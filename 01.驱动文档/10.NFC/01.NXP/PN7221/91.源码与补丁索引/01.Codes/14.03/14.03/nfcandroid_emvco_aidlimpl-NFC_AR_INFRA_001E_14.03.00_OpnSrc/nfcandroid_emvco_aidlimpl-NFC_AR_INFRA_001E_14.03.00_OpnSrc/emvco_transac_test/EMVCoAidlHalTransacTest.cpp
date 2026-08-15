/******************************************************************************
 *
 *  Copyright 2023 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of NXP nor the names of its contributors may be used
 * to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#define LOG_TAG "emvco_transac_test"

#include <aidl/Gtest.h>
#include <aidl/vendor/nxp/emvco/BnNxpEmvco.h>
#include <aidl/vendor/nxp/emvco/BnNxpEmvcoClientCallback.h>
#include <aidl/vendor/nxp/emvco/INxpEmvco.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoContactlessCard.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoProfileDiscovery.h>
#include <aidl/vendor/nxp/emvco/NxpConfigType.h>
#include <android-base/stringprintf.h>
#include <android/binder_auto_utils.h>
#include <android/binder_enums.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <chrono>
#include <future>
#include <log/log.h>

using aidl::vendor::nxp::emvco::INxpEmvco;
using aidl::vendor::nxp::emvco::INxpEmvcoClientCallback;
using aidl::vendor::nxp::emvco::INxpEmvcoContactlessCard;
using aidl::vendor::nxp::emvco::INxpEmvcoProfileDiscovery;
using aidl::vendor::nxp::emvco::NxpConfigType;
using aidl::vendor::nxp::emvco::NxpDeactivationType;
using aidl::vendor::nxp::emvco::NxpEmvcoEvent;
using aidl::vendor::nxp::emvco::NxpEmvcoStatus;
using ndk::SpAIBinder;

::ndk::ScopedAIBinder_DeathRecipient mDeathRecipient;

static volatile uint8_t pollingConfiguration = 0;

static volatile bool is_aborted_ = false;
const int NFC_A_PASSIVE_POLL_MASK = 0;
const int NFC_B_PASSIVE_POLL_MASK = 1;
const int NFC_F_PASSIVE_POLL_MASK = 2;
const int NFC_VAS_PASSIVE_POLL_MASK = 3;

const int NFC_A_PASSIVE_POLL_MODE = 1;
const int NFC_B_PASSIVE_POLL_MODE = 2;
const int NFC_AB_PASSIVE_POLL_MODE_SUPPORTED = 3;

int pollTypeAFirst = 0b00100000;
int pollProfileSelection = 0b01000010;
int count = 0;

std::mutex data_mutex_;
int32_t aidl_return;
std::future<void> transac_future;
std::promise<void> transac_promise;

std::shared_ptr<INxpEmvco> nxp_emvco_service_;
std::shared_ptr<INxpEmvcoContactlessCard> nxp_emvco_cl_service_;
std::shared_ptr<INxpEmvcoProfileDiscovery> nxp_emvco_prof_disc_service_;

class EmvcoClientCallback
    : public aidl::vendor::nxp::emvco::BnNxpEmvcoClientCallback {
public:
  EmvcoClientCallback(
      const std::function<void(NxpEmvcoEvent, NxpEmvcoStatus)> &on_hal_event_cb,
      const std::function<void(const std::vector<uint8_t> &)> &on_nci_data_cb)
      : on_nci_data_cb_(on_nci_data_cb), on_hal_event_cb_(on_hal_event_cb) {}
  virtual ~EmvcoClientCallback() = default;

  ::ndk::ScopedAStatus sendEvent(NxpEmvcoEvent event,
                                 NxpEmvcoStatus event_status) override {
    ALOGI("%s ", __func__);
    if (!is_aborted_) {
      on_hal_event_cb_(event, event_status);
    }
    return ::ndk::ScopedAStatus::ok();
  };
  ::ndk::ScopedAStatus sendData(const std::vector<uint8_t> &data) override {
    ALOGI("%s before mutex ", __func__);
    if (!is_aborted_) {
      on_nci_data_cb_(data);
    }
    return ::ndk::ScopedAStatus::ok();
  };

private:
  std::function<void(const std::vector<uint8_t> &)> on_nci_data_cb_;
  std::function<void(NxpEmvcoEvent, NxpEmvcoStatus)> on_hal_event_cb_;
};

void signal_callback_handler(int signum) {
  ALOGI("%s Self test App abort requested, signum:%d", __func__, signum);

  is_aborted_ = true;

  if (nxp_emvco_cl_service_ != nullptr) {
    nxp_emvco_cl_service_->setEMVCoMode(pollingConfiguration, false);
  }
  transac_promise.set_value();
  exit(signum);
  ALOGI("%s Self test App aborted", __func__);
}
void setRFTechnologyMode(int modeType, bool isSet) {
  ALOGI("%s Before set pollingConfiguration:%d\n", __func__,
        pollingConfiguration);
  if (isSet) {
    pollingConfiguration = 1 << modeType | pollingConfiguration;
  } else {
    pollingConfiguration = ~(1 << modeType) & pollingConfiguration;
  }
  ALOGI("%s after set pollingConfiguration:%d\n", __func__,
        pollingConfiguration);
}

void EmvcoHalBinderDied(void *cookie) {
  ALOGI("EmvcoHalBinderDied");
  (void)cookie;
  AIBinder_unlinkToDeath(nxp_emvco_service_->asBinder().get(),
                         mDeathRecipient.get(), 0);
  exit(1);
  ALOGI("Self test App aborted due to EMVCo HAL crash");
}

int main(int argc, char **argv) {
  ABinderProcess_startThreadPool();
  ALOGI("%s Entered %d arguments", __func__, argc);
  try {
    for (int i = 0; i < argc; ++i) {
      ALOGI("%s argv:", argv[i]);
    }

    if (argc > 2) {
      if (strstr(argv[2], "A") != 0 || strstr(argv[2], "a") != 0) {
        pollProfileSelection |= pollTypeAFirst;
        setRFTechnologyMode(NFC_A_PASSIVE_POLL_MASK, true);
      }
      if (strstr(argv[2], "B") != 0 || strstr(argv[2], "b") != 0) {
        setRFTechnologyMode(NFC_B_PASSIVE_POLL_MASK, true);
      }
      if (strstr(argv[2], "F") != 0 || strstr(argv[2], "f") != 0) {
        setRFTechnologyMode(NFC_F_PASSIVE_POLL_MASK, true);
      }
      if (strstr(argv[2], "V") != 0 || strstr(argv[2], "v") != 0) {
        setRFTechnologyMode(NFC_VAS_PASSIVE_POLL_MASK, true);
      }
    } else {
      printf("\n Select atleast one polling technolgy to enable EMVCo mode\n "
             "Example#1: \"./EMVCoAidlHalTransacTest Type A \" "
             "will enable Type A poll first"
             "for polling  \n Example#2: "
             "\"./EMVCoAidlHalTransacTest Type B \" "
             "will enable Type B poll first\n");
      return 0;
    }

    if (pollingConfiguration == NFC_A_PASSIVE_POLL_MODE ||
        pollingConfiguration == NFC_B_PASSIVE_POLL_MODE) {
      printf("\n Valid Technology selected for polling\n ");
    } else {
      printf(
          "\n Select supported polling technolgy (A/B) to enable EMVCo mode\n "
          "Example#1: \"./EMVCoAidlHalTransacTest Type A \" "
          "will enable Type A poll first"
          "for polling  \n Example#2: "
          "\"./EMVCoAidlHalTransacTest Type B \" "
          "will enable Type B poll first\n");
      return 0;
    }

    signal(SIGINT, signal_callback_handler);

    const std::string instance =
        std::string() + INxpEmvco::descriptor + "/default";
    SpAIBinder binder(AServiceManager_waitForService(instance.c_str()));
    nxp_emvco_service_ = INxpEmvco::fromBinder(binder);

    mDeathRecipient = ::ndk::ScopedAIBinder_DeathRecipient(
        AIBinder_DeathRecipient_new(EmvcoHalBinderDied));
    AIBinder_linkToDeath(nxp_emvco_service_->asBinder().get(),
                         mDeathRecipient.get(), 0);

    nxp_emvco_service_->getEmvcoContactlessCard(&nxp_emvco_cl_service_);
    nxp_emvco_service_->getEmvcoProfileDiscoveryInterface(
        &nxp_emvco_prof_disc_service_);
    aidl::vendor::nxp::emvco::NxpEmvcoStatus _aidl_return;
    int tempPollProfileSelection = pollProfileSelection;
    while (tempPollProfileSelection != 0) {
      tempPollProfileSelection /= 10;
      ++count;
    }
    ALOGI("setByteConfig called with pollProfileSelection:%d, count:%d",
          pollProfileSelection, count);

    nxp_emvco_prof_disc_service_->setByteConfig(NxpConfigType::POLL_PROFILE_SEL,
                                                count, pollProfileSelection,
                                                &_aidl_return);

    auto mCallback = ::ndk::SharedRefBase::make<EmvcoClientCallback>(
        [](auto event, auto status) {
          ALOGI("Event callback");
          (void)event;
          (void)status;
        },
        [](auto &in_data) {
          ALOGI("Data callback");
          (void)in_data;
        });
    bool register_status;
    EXPECT_TRUE((*(nxp_emvco_cl_service_))
                    .registerEMVCoEventListener(mCallback, &register_status)
                    .isOk());

    transac_future = transac_promise.get_future();
    nxp_emvco_cl_service_->setEMVCoMode(NFC_AB_PASSIVE_POLL_MODE_SUPPORTED,
                                        true);
    transac_future.wait();
  } catch (const std::length_error &e) {
    ALOGE("%s std::length_error on setByteConfig call", e.what());
  }
  ALOGI("TEST APP EXITED");
}
