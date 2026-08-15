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

#define LOG_TAG "emvco_compliance_test"

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
using aidl::vendor::nxp::emvco::NxpLedControl;
using ndk::SpAIBinder;

#define MIN_VALID_DATA_SIZE 9
#define NCI_HEADER_SIZE 3
#define NCI_SEND_PPSE                                                          \
  {                                                                            \
    0x00, 0xA4, 0x04, 0x00, 0x0E, 0x32, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59,    \
        0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00                         \
  }
#define DEACTIVATE_IDLE_TIMEOUT 15000
#define MIN_LED_GLOW_DURATION 500
#define MAX_LED_GLOW_DURATION 3000
#define EMVCO_POLLING_STARTED_EVT 4

const std::vector<uint8_t> nci_send_ppse = NCI_SEND_PPSE;
::ndk::ScopedAIBinder_DeathRecipient mDeathRecipient;
static std::promise<void> psse_cb_promise_;
static std::future<void> psse_cb_future_ = psse_cb_promise_.get_future();
unsigned long long start_ts_, end_ts_;

static std::vector<uint8_t> nci_send_loopback_;
static volatile bool is_aborted_ = false;
static volatile bool is_removal_procedure = false;
static volatile uint8_t pollingConfiguration = 0;
// Set the led glow duration to 500 milli second by default
static volatile uint32_t led_glow_duration = 500;

const int NFC_A_PASSIVE_POLL_MODE = 0;
const int NFC_B_PASSIVE_POLL_MODE = 1;
const int NFC_F_PASSIVE_POLL_MODE = 2;
const int NFC_VAS_PASSIVE_POLL_MODE = 3;

const int NFC_AB_PASSIVE_POLL_MODE_SUPPORTED = 3;
const int NFC_F_PASSIVE_POLL_MODE_SUPPORTED = 4;
const int NFC_ABF_PASSIVE_POLL_MODE_SUPPORTED = 7;
const int NFC_ABVAS_PASSIVE_POLL_MODE_SUPPORTED = 11;
const int NFC_ABFVAS_PASSIVE_POLL_MODE_SUPPORTED = 15;
const int pollProfileSelectionVal = 0b00000010;
int config_len = 0;

std::mutex data_mutex_;
std::mutex led_mutex_;
int32_t aidl_return;

std::shared_ptr<INxpEmvco> nxp_emvco_service_;
std::shared_ptr<INxpEmvcoContactlessCard> nxp_emvco_cl_service_;
std::shared_ptr<INxpEmvcoProfileDiscovery> nxp_emvco_prof_disc_service_;

static void
send(std::shared_ptr<INxpEmvcoContactlessCard> nxp_emvco_cl_service_,
     const std::vector<uint8_t> data, int32_t aidl_return, std::string dataTag);

void setLedState(NxpLedControl ledControl) {
  const std::lock_guard<std::mutex> lock(led_mutex_);
  aidl::vendor::nxp::emvco::NxpEmvcoStatus _aidl_return;
  nxp_emvco_prof_disc_service_->setLed(ledControl, &_aidl_return);
}

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

static void controlRedLed() {
  setLedState(NxpLedControl::RED_LED_ON);
  std::chrono::milliseconds sleepTime(led_glow_duration);
  std::this_thread::sleep_for(sleepTime);
  setLedState(NxpLedControl::RED_LED_OFF);
}

static bool isEndOfTest(std::vector<unsigned char> &data, int received_size) {
  ALOGI("%s received_size:%d\n", __func__, received_size);
  bool isEOT = false;
  if (data.at(0) == 0x60 && data.at(1) == 0x08) {
    ALOGI("Device Error");
    switch (data.at(3)) {
    case 0xB0:
      ALOGI("Device lost - transmission error\n");
      controlRedLed();
      break;
    case 0xB1:
      ALOGI("Device lost - protocol error\n");
      controlRedLed();
      break;
    case 0xB2:
      ALOGI("Device lost - timeout error\n");
      controlRedLed();
      break;
    default:
      ALOGI("Default\n");
      break;
    }
  } else if ((received_size >= 5) &&
             (data.at(4) == 0x70 || data.at(4) == 0x72)) {
    ALOGI("Device lost APDU received with NCI header\n");
    if (data.at(4) == 0x70) {
      is_removal_procedure = true;
    }
    isEOT = true;

    setLedState(NxpLedControl::GREEN_LED_ON);
    std::chrono::milliseconds sleepTime(led_glow_duration);
    std::this_thread::sleep_for(sleepTime);
    setLedState(NxpLedControl::GREEN_LED_OFF);
  } else if ((data.at(0) == 0x00) && (data.at(received_size - 1) != 0x00) &&
             (data.at(received_size - 2) != 0x90)) {
    ALOGI("InValid status byte - starting removal procedure\n");
    is_removal_procedure = true;
    isEOT = true;
  }
  return isEOT;
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

static void
send(std::shared_ptr<INxpEmvcoContactlessCard> nxp_emvco_cl_service_,
     const std::vector<uint8_t> data, int32_t aidl_return,
     std::string dataTag) {
  ALOGI("%s\n transceive data:%s", __func__, dataTag.c_str());
  nxp_emvco_cl_service_->transceive(data, &aidl_return);
}

void signal_callback_handler(int signum) {
  ALOGI("%s Self test App abort requested, signum:%d", __func__, signum);
  is_aborted_ = true;
  if (nxp_emvco_cl_service_ != nullptr) {
    nxp_emvco_cl_service_->setEMVCoMode(pollingConfiguration, false);
  }
  psse_cb_promise_.set_value();
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
  try {
    ALOGI("%s Entered %d arguments", __func__, argc);
    for (int i = 0; i < argc; ++i) {
      ALOGI("%s argv:", argv[i]);
    }
    if (argc > 1) {
      if (strstr(argv[1], "INTEROP") != 0 || strstr(argv[1], "interop") != 0) {
        if ((argc > 3) && argv[3] != NULL) {
          std::string duration_str = std::string(argv[3]);
          if (!duration_str.empty()) {
            int duration = std::stoi(argv[3]);
            ALOGI("User provided duration:%d", duration);
            if (duration >= MIN_LED_GLOW_DURATION &&
                duration <= MAX_LED_GLOW_DURATION) {
              led_glow_duration = duration;
            } else {
              printf(
                  "Invalid duration. 500ms to 3sec is the supported LED glow "
                  "duration \n. setting glow duration as 500ms \n");
            }
          }
        }
        ALOGI("Executing digital compliance with LED glow duration:%d",
              led_glow_duration);
      } else {
        led_glow_duration = 0;
        ALOGI("Executing digital compliance with out LED glow duration:%d",
              led_glow_duration);
      }
    }
    if (argc > 2) {
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
          "Example#1: \"./EMVCoAidlHalComplianceTest Type/interop A 600\" "
          "will enable Type A "
          "for polling with LED glow duration of 600ms \n Example#2: "
          "\"./EMVCoAidlHalComplianceTest Type/interop AB 600\" "
          "will enable Type/interop AB for polling with LED glow duration of "
          "600ms\n \n ");
      return 0;
    }

    if (pollingConfiguration == NFC_AB_PASSIVE_POLL_MODE_SUPPORTED ||
        pollingConfiguration == NFC_F_PASSIVE_POLL_MODE_SUPPORTED ||
        pollingConfiguration == NFC_ABF_PASSIVE_POLL_MODE_SUPPORTED ||
        pollingConfiguration == NFC_ABVAS_PASSIVE_POLL_MODE_SUPPORTED ||
        pollingConfiguration == NFC_ABFVAS_PASSIVE_POLL_MODE_SUPPORTED) {
      printf("\n Valid Technology selected for polling\n ");
    } else {
      printf(
          "\n Select supported polling technolgy (AB) to enable EMVCo mode\n "
          "Example: \"./EMVCoAidlHalComplianceTest Type/interop "
          "AB 600 \" will enable Type/interop AB for polling with LED glow "
          "duration of 600ms\n \n ");
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
    int tempPollProfileSelection = pollProfileSelectionVal;
    while (tempPollProfileSelection != 0) {
      tempPollProfileSelection /= 10;
      ++config_len;
    }
    ALOGI("setByteConfig called with pollProfileSelectionVal:%d, config_len:%d",
          pollProfileSelectionVal, config_len);

    nxp_emvco_prof_disc_service_->setByteConfig(
        NxpConfigType::POLL_PROFILE_SEL, config_len, pollProfileSelectionVal,
        &_aidl_return);

    auto mCallback = ::ndk::SharedRefBase::make<EmvcoClientCallback>(
        [](auto event, auto status) {
          ALOGI("Event callback event:%d", event);
          (void)status;
          if (EMVCO_POLLING_STARTED_EVT == (int)event) {
            setLedState(NxpLedControl::RED_LED_OFF);
            setLedState(NxpLedControl::GREEN_LED_OFF);
          }
        },
        [](auto &in_data) {
          ALOGI("Data callback");
          const std::lock_guard<std::mutex> lock(data_mutex_);
          ALOGI("Data callback after mutex lock");
          try {
            std::vector<uint8_t> data(in_data.begin(), in_data.end());
            int received_size = data.size();
            if (data.at(0) == 0x00 && received_size < MIN_VALID_DATA_SIZE) {
              ALOGI("InValid Data length Packet - Sending PPSE\n");
              send(nxp_emvco_cl_service_,
                   getNCILoopbackData(0x00, nci_send_ppse, 20), aidl_return,
                   "LOOPBACK_APDU");

            } else if (isEndOfTest(data, received_size)) {
              ALOGI("End of Test\n");
              if (is_removal_procedure) {
                is_removal_procedure = false;
                ALOGI("stopRFDisovery with DISCOVER");
                NxpEmvcoStatus emvcoStatus;
                nxp_emvco_cl_service_->stopRFDisovery(
                    NxpDeactivationType::DISCOVER, &emvcoStatus);
              } else {
                ALOGI("stopRFDisovery with IDLE");
                NxpEmvcoStatus emvcoStatus;
                nxp_emvco_cl_service_->stopRFDisovery(NxpDeactivationType::IDLE,
                                                      &emvcoStatus);
                std::chrono::microseconds sleepTime(DEACTIVATE_IDLE_TIMEOUT);
                std::this_thread::sleep_for(sleepTime);
                ALOGI("Poll again through setEMVCoMode immediately");
                nxp_emvco_cl_service_->setEMVCoMode(pollingConfiguration, true);
              }
              ALOGI("\n Loopback application running - waiting for test "
                    "equipment "
                    "discovery ...\n");
            } else if (data.at(0) == 0x00) {
              std::vector<uint8_t> apduData(in_data.begin() + NCI_HEADER_SIZE,
                                            in_data.end() - 2);
              send(nxp_emvco_cl_service_,
                   getNCILoopbackData(0x00, apduData,
                                      received_size - NCI_HEADER_SIZE - 2),
                   aidl_return, "LOOPBACK_APDU");
            } else if (data.at(0) == 0x61 &&
                       data.at(1) == 0x05) { // RF_ACTIVATION_NTF - 0x61 && 0x05
              ALOGI("RF_ACTIVATION_NTF VERIFIED");

              send(nxp_emvco_cl_service_,
                   getNCILoopbackData(0x00, nci_send_ppse, 20), aidl_return,
                   "LOOPBACK_APDU");
            }
          } catch (const std::future_error &e) {
            ALOGE("%s data future_error", e.what());
          }
        });
    bool register_status;
    EXPECT_TRUE((*(nxp_emvco_cl_service_))
                    .registerEMVCoEventListener(mCallback, &register_status)
                    .isOk());
    nxp_emvco_cl_service_->setEMVCoMode(pollingConfiguration, true);
    psse_cb_future_.wait();

  } catch (const std::length_error &e) {
    ALOGE("%s std::length_error", e.what());
  }
  ALOGI("TEST APP EXITED");
}
