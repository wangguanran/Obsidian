/******************************************************************************
 *
 *  Copyright 2022-2023 NXP
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

#define LOG_TAG "emvco_aidl_hal_test"

#include <aidl/Gtest.h>
#include <aidl/Vintf.h>
#include <aidl/vendor/nxp/emvco/BnNxpEmvco.h>
#include <aidl/vendor/nxp/emvco/BnNxpEmvcoClientCallback.h>
#include <aidl/vendor/nxp/emvco/INxpEmvco.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoContactlessCard.h>
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android/binder_auto_utils.h>
#include <android/binder_enums.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <binder/IServiceManager.h>
#include <chrono>
#include <future>
#include <log/log.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

using aidl::vendor::nxp::emvco::INxpEmvco;
using aidl::vendor::nxp::emvco::INxpEmvcoClientCallback;
using aidl::vendor::nxp::emvco::INxpEmvcoContactlessCard;
using aidl::vendor::nxp::emvco::NxpEmvcoEvent;
using aidl::vendor::nxp::emvco::NxpEmvcoStatus;

using ndk::ScopedAStatus;
using ndk::SharedRefBase;
using ndk::SpAIBinder;

using aidl::vendor::nxp::emvco::INxpEmvcoProfileDiscovery;
using aidl::vendor::nxp::emvco::NxpConfigType;
using android::getAidlHalInstanceNames;
using android::PrintInstanceNameToString;
using android::base::StringPrintf;
using ndk::enum_range;
using ndk::ScopedAStatus;
using ndk::SharedRefBase;
using ndk::SpAIBinder;
/* NCI Commands */

#define NCI_SEND_PPSE                                                          \
  {                                                                            \
    0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59,    \
        0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00                         \
  }

#define PPSE_SEND_MAX_TIMES 250

std::shared_ptr<INxpEmvco> iIEmvco_;
std::shared_ptr<INxpEmvcoContactlessCard> iEmvcoContactlessCard_;
constexpr static int kCallbackTimeoutMs = 100;
volatile static int index = 0;
static std::vector<std::promise<void>> psse_cb_promise;
unsigned long long start_ts, end_ts;
static volatile bool is_aborted_ = false;
static volatile uint8_t pollingConfiguration = 0;
const int NFC_A_PASSIVE_POLL_MODE = 0;
const int NFC_B_PASSIVE_POLL_MODE = 1;
const int NFC_F_PASSIVE_POLL_MODE = 2;
const int NFC_VAS_PASSIVE_POLL_MODE = 3;
const int pollProfileSelection = 0b00000010;
int count = 0;
::ndk::ScopedAIBinder_DeathRecipient mDeathRecipient;
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
    ALOGI("%s ", __func__);
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
  if (iEmvcoContactlessCard_ != nullptr) {
    iEmvcoContactlessCard_->setEMVCoMode(pollingConfiguration, false);
  }
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
  AIBinder_unlinkToDeath(iIEmvco_->asBinder().get(), mDeathRecipient.get(), 0);
  exit(1);
  ALOGI("Self test App aborted due to EMVCo HAL crash");
}

static std::vector<uint8_t> getNCILoopbackData(uint8_t packetBoundaryFlag,
                                               std::vector<uint8_t> apduData,
                                               int dataLength) {
  ALOGI("%s\n dataLength", __func__);
  std::vector<uint8_t> nci_send_loopback_;
  nci_send_loopback_.insert(nci_send_loopback_.begin(), packetBoundaryFlag);
  nci_send_loopback_.insert(nci_send_loopback_.begin() + 1, 0x00);
  nci_send_loopback_.insert(nci_send_loopback_.begin() + 2, dataLength);
  nci_send_loopback_.insert(nci_send_loopback_.begin() + 3, apduData.begin(),
                            apduData.end());
  return nci_send_loopback_;
}

int main(int argc, char **argv) {
  ABinderProcess_startThreadPool();
  try {
    ALOGI("%s Entered %d arguments", __func__, argc);
    for (int i = 0; i < argc; ++i) {
      ALOGI("%s argv:", argv[i]);
    }
    if (argc == 3) {
      if (strstr(argv[2], "A") != 0 || strstr(argv[2], "a") != 0) {
        setRFTechnologyMode(NFC_A_PASSIVE_POLL_MODE, true);
      }
      if (strstr(argv[2], "B") != 0 || strstr(argv[2], "b") != 0) {
        setRFTechnologyMode(NFC_B_PASSIVE_POLL_MODE, true);
      }
      if (strstr(argv[2], "F") != 0 || strstr(argv[2], "f") != 0) {
        setRFTechnologyMode(NFC_F_PASSIVE_POLL_MODE, true);
      }
      if (strstr(argv[2], "V") != 0 || strstr(argv[2], "v") != 0) {
        setRFTechnologyMode(NFC_VAS_PASSIVE_POLL_MODE, true);
      }
    } else {
      printf(
          "\n Select atleast one polling technolgy to enable EMVCo mode\n "
          "Example#1: \"./EMVCoAidlHalDesfireTest Type A\" will enable Type A "
          "for polling \n Example#2: \"./EMVCoAidlHalDesfireTest Type AB\" "
          "will enable Type AB for polling \n \n ");
      return 0;
    }

    if (pollingConfiguration == 0) {
      printf(
          "\n Select supported polling technolgy (A/B/F) to enable EMVCo "
          "mode\n "
          "Example#1: \"./EMVCoAidlHalDesfireTest Type A\" will enable "
          "Type A for polling \n Example#2: \"./EMVCoAidlHalDesfireTest Type "
          "AB\" will enable Type AB for polling \n \n ");
      return 0;
    }
    signal(SIGINT, signal_callback_handler);

    std::vector<uint8_t> nci_send_ppse = NCI_SEND_PPSE;
    std::promise<void> start_discovery_emvco_cb_promise;

    auto start_discovery_emvco_cb_future =
        start_discovery_emvco_cb_promise.get_future();
    std::vector<std::future<void>> psse_cb_future;

    std::chrono::milliseconds timeout{kCallbackTimeoutMs};

    for (int i = 0; i < PPSE_SEND_MAX_TIMES; i++) {
      std::promise<void> promise;
      psse_cb_future.push_back(promise.get_future());
      psse_cb_promise.push_back(std::move(promise));
    }

    auto mCallback = ::ndk::SharedRefBase::make<EmvcoClientCallback>(
        [](auto event, auto status) {
          ALOGI("%s event callback", __func__);
          (void)event;
          (void)status;
        },
        [&start_discovery_emvco_cb_promise](auto &in_data) {
          ALOGI("%s data callback", __func__);
          std::vector<uint8_t> data(in_data.begin(), in_data.end());
          size_t length = data.size();
          ALOGI("%s data callback data.size():%zu", __func__, length);
          // Validating RESET and NCI_DISABLE_STANDBY_MODE_CMD Response
          try {
            // RF_ACTIVATION_NTF - 0x61 && 0x05
            if (data.at(0) == 97 && data.at(1) == 5) {
              start_discovery_emvco_cb_promise.set_value();
              ALOGI("%s  RF_ACTIVATION_NTF VERIFIED", __func__);
            }
            // Validating NCI_SET_EMV_PROFILE and PPSE Response
            // PPSE 0x6A && 0x82
            if (data.at(3) == 106 && data.at(4) == 130) {
              ALOGI("%s  PPSE RESPONSE VERIFIED", __func__);
              psse_cb_promise.at(index).set_value();
            } else {
              ALOGI("%s  ELSE OF NCI_SET_EMV_PROFILE and PPSE Response",
                    __func__);
            }
          } catch (const std::future_error &e) {
            ALOGE("%s event future_error", e.what());
          }
        });

    const std::string instance =
        std::string() + INxpEmvco::descriptor + "/default";
    SpAIBinder binder(AServiceManager_waitForService(instance.c_str()));
    iIEmvco_ = INxpEmvco::fromBinder(binder);
    mDeathRecipient = ::ndk::ScopedAIBinder_DeathRecipient(
        AIBinder_DeathRecipient_new(EmvcoHalBinderDied));
    AIBinder_linkToDeath(iIEmvco_->asBinder().get(), mDeathRecipient.get(), 0);

    iIEmvco_->getEmvcoContactlessCard(&iEmvcoContactlessCard_);

    iIEmvco_->getEmvcoProfileDiscoveryInterface(&nxp_emvco_prof_disc_service_);
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

    // Open and wait for OPEN_CPLT
    ALOGI("registerEMVCoEventListener");
    bool register_status;
    EXPECT_TRUE((*(iEmvcoContactlessCard_))
                    .registerEMVCoEventListener(mCallback, &register_status)
                    .isOk());

    iEmvcoContactlessCard_->setEMVCoMode(pollingConfiguration, true);
    start_discovery_emvco_cb_future.wait();

    // NCI_SEND_PPSE
    while (true) {
      ALOGI("%s NCI_SEND_PPSE COUNT=%d", __func__, index);
      if (index <= PPSE_SEND_MAX_TIMES - 1) {
        const std::vector<uint8_t> &data6 = NCI_SEND_PPSE;
        int32_t ppse_aidl_return1;
        struct timespec tm;
        clock_gettime(CLOCK_MONOTONIC, &tm);
        start_ts = tm.tv_nsec * 1e-3 + tm.tv_sec * 1e+6;
        ALOGI("%s PPSE command Sent at:%llu", __func__, start_ts);
        iEmvcoContactlessCard_->transceive(getNCILoopbackData(0x00, data6, 20),
                                           &ppse_aidl_return1);
        EXPECT_EQ(psse_cb_future.at(index).wait_for(3 * timeout),
                  std::future_status::ready);
        clock_gettime(CLOCK_MONOTONIC, &tm);
        end_ts = tm.tv_nsec * 1e-3 + tm.tv_sec * 1e+6;
        ALOGI("%s PPSE response received at:%llu", __func__, end_ts);
        ALOGI("%s PPSE command and response duration :%llu microsec", __func__,
              end_ts - start_ts);
        ++index;
      } else {
        ALOGI("%s not sending NCI_SEND_PPSE_RSP as index> 99 ", __func__);
        break;
      }
    }

    iEmvcoContactlessCard_->setEMVCoMode(pollingConfiguration, false);
  } catch (const std::length_error &e) {
    ALOGE("%s std::length_error", e.what());
  }
  ALOGI("TEST APP EXITED");
}
