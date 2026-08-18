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
#define LOG_TAG "emvco_tda_test_transceive_state"

/**
 * @brief  This class is responsible for calling transceive API
 *         and handles all the errors related to transceive API.
 *
 **/

#include "TransceiveState.h"
#include "CloseState.h"
#include "EMVCoTDA.h"
#include "OpenState.h"
#include <algorithm>
#include <log/log.h>

using namespace aidl::vendor::nxp::emvco::tda::test;
TransceiveState *TransceiveState::mTransceiveState = nullptr;

/**
 * @brief  Calls the transceive API on TransceiveState entry
 *
 **/
void TransceiveState::enter() {
  ALOGI("%s \n", __func__);
  sendPPSE();
}

/**
 * @brief  Forms the NCI packet by including NCI header and payload together
 *
 **/
std::vector<uint8_t> getNCILoopbackData(uint8_t nfceeID, uint8_t coreCredit,
                                        std::vector<uint8_t> apduData,
                                        int dataLength) {
  ALOGI("%s\n dataLength", __func__);
  std::vector<uint8_t> nci_send_loopback_;
  nci_send_loopback_.insert(nci_send_loopback_.begin(), nfceeID);
  nci_send_loopback_.insert(nci_send_loopback_.begin() + 1, coreCredit);
  nci_send_loopback_.insert(nci_send_loopback_.begin() + 2, dataLength);
  nci_send_loopback_.insert(nci_send_loopback_.begin() + 3, apduData.begin(),
                            apduData.end());
  return nci_send_loopback_;
}

TransceiveState::TransceiveState() { ALOGI("%s \n", __func__); }

/**
 * @brief  sends the PPSE APDU to controller
 *
 **/
void TransceiveState::sendPPSE() {
  ALOGI("%s \n", __func__);
  mCmdApdu.clear();
  mRspApdu.clear();
  OpenState *pOpenState = static_cast<OpenState *>(&OpenState::getInstance());
  mCmdApdu =
      getNCILoopbackData(pOpenState->getConnectionID(), 0, nci_send_ppse, 20);
  for (uint8_t apdu : mCmdApdu) {
    ALOGI("%s mCmdApdu:%02x ", __func__, apdu);
  }
  EMVCoTDA::getEMVCoTDAInstance().transceive(mCmdApdu, &mRspApdu);
}

/**
 * @brief  handles the EMVCo CT APDU loopback.
 *         on success, sends the APDU by removing the status byte
 *         on EOT, ends the loopback
 *         on failure, handles the error scenario gracefully
 *
 **/
void TransceiveState::handleLoopBack() {
  ALOGI("%s rspApdu.size():%zu\n", __func__, mRspApdu.size());
  for (uint8_t apdu : mRspApdu) {
    ALOGI("%s resApdu:%02x ", __func__, apdu);
  }
  if (mRspApdu.size() > 0) {
    if (mRspApdu.size() >= 6) {
      if (EOT_INDICATOR == mRspApdu[4]) {
        ALOGI("%s End of test case\n", __func__);
        StateMachine::getInstance().setState(&CloseState::getInstance());
      } else {
        ALOGI("%s Valid Response. Do loopback the data to NFCC \n", __func__);
        mCmdApdu.clear();
        copy(mRspApdu.begin() + 3, mRspApdu.end() - 2, back_inserter(mCmdApdu));
        mRspApdu.clear();
        OpenState *pOpenState =
            static_cast<OpenState *>(&OpenState::getInstance());
        ALOGI("%s conn_id:%02x\n", __func__, pOpenState->getConnectionID());
        mCmdApdu = getNCILoopbackData(pOpenState->getConnectionID(), 0,
                                      mCmdApdu, mCmdApdu.size());
        for (uint8_t apdu : mCmdApdu) {
          ALOGI("%s mCmdApdu:%02x ", __func__, apdu);
        }
        EMVCoTDA::getEMVCoTDAInstance().transceive(mCmdApdu, &mRspApdu);
      }
    } else {
      ALOGI("%s response <=6 byte. sending PPSE again\n", __func__);
      sendPPSE();
    }
  } else {
    ALOGI("%s transceive Failed. Invalid response received \n", __func__);
    StateMachine::getInstance().setState(&CloseState::getInstance());
  }
}

/**
 * @brief  handles all the error codes related to transceive API call
 *
 **/
void TransceiveState::handleFailures(Response res) {
  ALOGI("%s res.error:%d\n", __func__, res.error);
  switch (res.error) {
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_TRANSCEIVE_FAILED_WRITE_ERR):
  case static_cast<int8_t>(NxpEmvcoStatus::EMVCO_STATUS_NCI_RESPONSE_ERR):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_TRANSCEIVE_FAILED_WTX_TIMED_OUT):
    ALOGI("%s transceive Failed. Try close and discover again\n", __func__);
    StateMachine::getInstance().setState(&CloseState::getInstance());
    break;
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_TRANSCEIVE_FAILED_INVALID_CONN_ID):
    ALOGI("%s In-Valid connection ID. Exit the application \n", __func__);
    /* Application which supports writing to multiple TDA needs to close the
     * existing the TDA and open the TDA slot which it needs to send the data.
     * Since this is CT compliance app which supports only CT slot, aborting the
     * application
     */
    exit(1);
    break;
  default:
    ALOGI("%s checking for state machine error \n", __func__);
    TDAState::handleFailures(res);
    break;
  }
}

/**
 * @brief  process the transceive API response and moves to next state on
 *         sucess and handles error scenario's gracefully on failure
 *
 **/
void TransceiveState::processResponse(Response res) {
  ALOGI("%s res.status:%d,res.exception:%d,res.error:%d, mRspApdu.size():%zu\n",
        __func__, res.status, res.exception, res.error, mRspApdu.size());
  if (res.status != 1 || res.exception != 0 || res.error != 0) {
    handleFailures(res);
  } else {
    handleLoopBack();
  }
}

/**
 * @brief  Returns the singleton object the TransceiveState class
 *
 **/
TDAState &TransceiveState::getInstance() {
  ALOGI("%s \n", __func__);
  if (mTransceiveState == nullptr) {
    mTransceiveState = new TransceiveState();
  }
  return *mTransceiveState;
}