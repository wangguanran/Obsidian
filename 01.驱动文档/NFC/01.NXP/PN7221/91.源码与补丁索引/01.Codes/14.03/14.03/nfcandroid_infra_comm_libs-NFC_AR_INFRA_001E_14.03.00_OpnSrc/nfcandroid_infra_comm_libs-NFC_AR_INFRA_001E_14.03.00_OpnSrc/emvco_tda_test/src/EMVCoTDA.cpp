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

#define LOG_TAG "emvco_tda_test"

/**
 * @brief  This class is responsible for getting EMVCo HAL interface,
 *         registering the callbacks and calls the EMVCo HAL API's
 *
 **/

#include "EMVCoTDA.h"
#include "DiscoverState.h"
#include <string.h>

using aidl::vendor::nxp::emvco::INxpEmvco;
using aidl::vendor::nxp::emvco::NxpDeactivationType;
using aidl::vendor::nxp::emvco::NxpEmvcoState;
using aidl::vendor::nxp::emvco::NxpEmvcoStatus;
using aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo;
using aidl::vendor::nxp::emvco::tda::test::EMVCoTDA;
using namespace std;

EMVCoTDA *EMVCoTDA::mEMVCoTDA = nullptr;
volatile uint8_t EMVCoTDA::pollingConfiguration = 0;
const string EMVCoTDA::CT_CARD_TDA_SLOT_NAME = "CT";
const string EMVCoTDA::SAM1_CARD_TDA_SLOT_NAME = "SAM1";
const string EMVCoTDA::SAM2_CARD_TDA_SLOT_NAME = "SAM2";
const int EMVCoTDA::pollProfileSelectionVal = 0b00000010;

/**
 * @brief  Listens for EMVCo service binder status and
 *         exits the application
 **/
void EMVCoTDA::EmvcoHalBinderDied(void *cookie) {
  ALOGI("EmvcoHalBinderDied");
  (void)cookie;
  exit(1);
  ALOGI("Self test App aborted due to EMVCo HAL crash");
}

/**
 * @brief  EMVCo Service TDA callback listener class
 *         to receive TDA and EMVCo status updates
 **/
class EmvcoTDACallback
    : public aidl::vendor::nxp::emvco::BnNxpEmvcoTDACallback {
public:
  EmvcoTDACallback(
      const std::function<void(const NxpEmvcoState &in_emvcoState,
                               const std::string &in_debugReason)>
          &on_emvco_state_change_cb_,
      const std::function<void(const NxpEmvcoTDAInfo &in_emvcoTDAInfo,
                               const std::string &in_debugReason)>
          &on_tda_state_change_cb_)
      : on_emvco_state_change_cb_(on_emvco_state_change_cb_),
        on_tda_state_change_cb_(on_tda_state_change_cb_) {}
  virtual ~EmvcoTDACallback() = default;

  ::ndk::ScopedAStatus
  onEMVCoCLStateChange(NxpEmvcoState emvcoState,
                       const std::string &in_debugReason) override {
    on_emvco_state_change_cb_(emvcoState, in_debugReason);
    return ::ndk::ScopedAStatus::ok();
  }

  ::ndk::ScopedAStatus
  onTDAStateChange(const NxpEmvcoTDAInfo &in_emvcoTDAInfo,
                   const std::string &in_debugReason) override {
    on_tda_state_change_cb_(in_emvcoTDAInfo, in_debugReason);
    return ::ndk::ScopedAStatus::ok();
  }

private:
  std::function<void(const NxpEmvcoState &in_emvcoState,
                     const std::string &in_debugReason)>
      on_emvco_state_change_cb_;
  std::function<void(const NxpEmvcoTDAInfo &in_emvcoTDAInfo,
                     const std::string &in_debugReason)>
      on_tda_state_change_cb_;
};

/**
 * @brief  Waits for CT response and send the CT response
 *         to current state for processing.
 *
 **/
void EMVCoTDA::responseHandler() {
  ALOGI("%s  nowhile\n", __func__);
  while (1) {
    std::unique_lock<std::mutex> lk(rsp_cv_m);
    if (!isRSPReady) {
      ALOGI("%s waiting for EMVCo CT response \n", __func__);
      rsp_cv.wait(lk, [&isRSPReady = this->isRSPReady] { return isRSPReady; });
    }
    isRSPReady = false;
    lk.unlock();
    StateMachine::getInstance().getCurrentTDAState()->processResponse(
        mResponse);
  }
}

/**
 * @brief  Gets the EMVCo service instance,  listens for EMVCo TDA callbacks
 *         and starts the EMVCo mode
 **/
void EMVCoTDA::initEMVCoHal() {
  ALOGI("%s  \n", __func__);
  const std::string instance =
      std::string() + INxpEmvco::descriptor + "/default";
  SpAIBinder binder(AServiceManager_waitForService(instance.c_str()));
  nxp_emvco_service_ = INxpEmvco::fromBinder(binder);

  mDeathRecipient = ::ndk::ScopedAIBinder_DeathRecipient(
      AIBinder_DeathRecipient_new(EMVCoTDA::EmvcoHalBinderDied));
  AIBinder_linkToDeath(nxp_emvco_service_->asBinder().get(),
                       mDeathRecipient.get(), 0);

  nxp_emvco_service_->getEmvcoTDA(&nxp_emvco_ct_service_);
  nxp_emvco_service_->getEmvcoContactlessCard(&nxp_emvco_cl_service_);
  nxp_emvco_service_->getEmvcoProfileDiscoveryInterface(
      &nxp_emvco_prof_disc_service_);
  NxpEmvcoStatus _aidl_return = NxpEmvcoStatus::EMVCO_STATUS_FAILED;
  int tempPollProfileSelection = EMVCoTDA::pollProfileSelectionVal;
  while (tempPollProfileSelection != 0) {
    tempPollProfileSelection /= 10;
    ++config_len;
  }
  ALOGI("setByteConfig called with pollProfileSelectionVal:%d, config_len:%d",
        EMVCoTDA::pollProfileSelectionVal, config_len);

  nxp_emvco_prof_disc_service_->setByteConfig(
      NxpConfigType::POLL_PROFILE_SEL, config_len,
      EMVCoTDA::pollProfileSelectionVal, &_aidl_return);

  auto mCTCallback = ::ndk::SharedRefBase::make<EmvcoTDACallback>(
      [&](auto emvcoState, auto debugReason) {
        (void)debugReason;
        ALOGI("EMVCo StateMachine change emvcoState:%d", (int)emvcoState);
        if (NxpEmvcoState::ON == emvcoState) {
          NxpEmvcoStatus emvcoStatus;
          nxp_emvco_cl_service_->stopRFDisovery(NxpDeactivationType::IDLE,
                                                &emvcoStatus);
          std::unique_lock<std::mutex> lk(tda_cv_m);
          isTDAReady = true;
          tda_cv.notify_one();
        }
      },
      [&](auto emvcoTDAInfo, auto debugReason) {
        (void)debugReason;
        NxpEmvcoTDAInfo emvcoTDA = (NxpEmvcoTDAInfo)emvcoTDAInfo;
        ALOGI("TDA:%s", emvcoTDA.toString().c_str());
        if ((mTdaSlot == SAM1_CARD_TDA_ID || mTdaSlot == SAM2_CARD_TDA_ID) &&
            NxpEmvcoTDAStatus::UN_RESPONSIVE_CARD_NOT_INSERTED ==
                emvcoTDA.status) {
          ALOGI("SAM Card not present. exit the application");
          stopTDALoopback();
          exit(1);
        } else if (mTdaSlot == emvcoTDA.id &&
                   NxpEmvcoTDAStatus::CONNECTED_DISABLED == emvcoTDA.status) {
          std::unique_lock<std::mutex> lk(tda_cv_m);
          isTDAReady = true;
          tda_cv.notify_one();
        } else if (mTdaSlot == emvcoTDA.id &&
                   NxpEmvcoTDAStatus::CONNECTED_ENABLED != emvcoTDA.status) {
          std::unique_lock<std::mutex> lk(tda_cv_m);
          isTDAReady = false;
          tda_cv.notify_one();
        }
      });
  bool register_status;
  EXPECT_TRUE((*(EMVCoTDA::nxp_emvco_ct_service_))
                  .registerEMVCoCTListener(mCTCallback, &register_status)
                  .isOk());
  nxp_emvco_cl_service_->setEMVCoMode(pollingConfiguration, true);
}
/**
 * @brief  Initializes the resource needed for loopback application
 *         and starts the EMVCo mode
 **/
EMVCoTDA::EMVCoTDA() {
  ALOGI("%s \n", __func__);
  isTDAReady = false;
  isRSPReady = false;
  setTDASlot(UNKNOWN_TDA_SLOT);
  mResponse.status = 0;
  mResponse.exception = 0;
  mResponse.error = 0;
  mRspHandlerThread = std::thread(&EMVCoTDA::responseHandler, this);
  initEMVCoHal();
}

/**
 * @brief  Returns the singleton object the EMVCoTDA class
 *
 **/
EMVCoTDA &EMVCoTDA::getEMVCoTDAInstance() {
  if (mEMVCoTDA == nullptr) {
    mEMVCoTDA = new EMVCoTDA();
  }
  return *mEMVCoTDA;
}

/**
 * @brief  checks smart card is present on TDA slot
 *
 **/
bool EMVCoTDA::isCardAvailable() {
  std::unique_lock<std::mutex> lk(tda_cv_m);
  return isTDAReady;
}

/**
 * @brief  calls discoverTDA API of EMVCo and process the response
 *         through state machine
 *
 **/
void EMVCoTDA::discoverTDA(std::vector<NxpEmvcoTDAInfo> *contactCards) {
  ALOGI("%s \n", __func__);
  auto status = nxp_emvco_ct_service_->discoverTDA(contactCards);
  std::unique_lock<std::mutex> lk(rsp_cv_m);
  mResponse.status = status.isOk();
  mResponse.exception = status.getExceptionCode();
  mResponse.error = status.getServiceSpecificError();
  isRSPReady = true;
  rsp_cv.notify_one();
}

/**
 * @brief  calls openTDA API of EMVCo and process the response
 *         through state machine
 *
 **/
void EMVCoTDA::openTDA(bool standBy, int8_t *channelId) {
  ALOGI("%s \n", __func__);
  if (!isCardAvailable()) {
    ALOGI("%s waiting for card to be inserted\n", __func__);
    wait();
  }
  auto status = nxp_emvco_ct_service_->openTDA(mTdaSlot, standBy, channelId);
  ALOGI("openTDA channelId:%02x", *channelId);
  std::unique_lock<std::mutex> lk(rsp_cv_m);
  mResponse.status = status.isOk();
  mResponse.exception = status.getExceptionCode();
  mResponse.error = status.getServiceSpecificError();
  isRSPReady = true;
  rsp_cv.notify_one();
}

/**
 * @brief  calls transceive API of EMVCo and process the response
 *         through state machine
 *
 **/
void EMVCoTDA::transceive(std::vector<uint8_t> cmdApdu,
                          std::vector<uint8_t> *rspApdu) {
  ALOGI("%s \n", __func__);
  if (!isCardAvailable()) {
    ALOGI("%s waiting for card to be inserted\n", __func__);
    wait();
  }
  auto status = nxp_emvco_ct_service_->transceive(cmdApdu, rspApdu);
  std::unique_lock<std::mutex> lk(rsp_cv_m);
  mResponse.status = status.isOk();
  mResponse.exception = status.getExceptionCode();
  mResponse.error = status.getServiceSpecificError();
  isRSPReady = true;
  rsp_cv.notify_one();
}

/**
 * @brief  calls closeTDA API of EMVCo and process the response
 *         through state machine
 *
 **/
void EMVCoTDA::closeTDA(bool standBy) {
  ALOGI("%s \n", __func__);
  if (!isCardAvailable()) {
    ALOGI("%s waiting for card to be inserted\n", __func__);
    wait();
  }
  auto status = nxp_emvco_ct_service_->closeTDA(mTdaSlot, standBy);
  std::unique_lock<std::mutex> lk(rsp_cv_m);
  mResponse.status = status.isOk();
  mResponse.exception = status.getExceptionCode();
  mResponse.error = status.getServiceSpecificError();
  isRSPReady = true;
  rsp_cv.notify_one();
}

/**
 * @brief  waits until card been inserted to TDA slot
 *
 **/
void EMVCoTDA::wait() {
  ALOGI("%s \n", __func__);
  std::unique_lock<std::mutex> lk(tda_cv_m);
  tda_cv.wait(lk, [&isTDAReady = this->isTDAReady] { return isTDAReady; });
}

/**
 * @brief  sets the isTDAReady with true/false based on card availablity of TDA
 *Slot
 *
 **/
void EMVCoTDA::setTDAReady(bool isReady) {
  std::unique_lock<std::mutex> lk(tda_cv_m);
  ALOGI("%s isReady:%d\n", __func__, isReady);
  isTDAReady = isReady;
}

/**
 * @brief  checks the card availablity and if it is available, starts the
 *         loopback else waits for card to be inserted
 *
 **/
void EMVCoTDA::startTDALoopback() {
  ALOGI("%s\n", __func__);
  std::unique_lock<std::mutex> lk(tda_cv_m);
  if (!isTDAReady) {
    ALOGI("%s waiting for EMVCo CT ready \n", __func__);
    tda_cv.wait(lk, [&isTDAReady = this->isTDAReady] { return isTDAReady; });
  }
  lk.unlock();
  StateMachine &sm = StateMachine::getInstance();
  sm.setState(&DiscoverState::getInstance());
  mRspHandlerThread.join();
}

/**
 * @brief  Stops the EMVCo mode
 *
 **/
bool EMVCoTDA::stopTDALoopback() {
  std::unique_lock<std::mutex> lk(tda_cv_m);
  isTDAReady = false;
  ALOGI("%s Switch to NFC Mode\n", __func__);
  if (nxp_emvco_cl_service_ != nullptr) {
    nxp_emvco_cl_service_->setEMVCoMode(pollingConfiguration, false);
  }
  lk.unlock();
  return true;
}