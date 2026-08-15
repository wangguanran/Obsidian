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
#define LOG_TAG "emvco_tda_test_open_state"

/**
 * @brief  This class is responsible for calling OpenState API
 *         and handles all the errors related to OpenState API.
 *
 **/

#include "OpenState.h"
#include "CloseState.h"
#include "DiscoverState.h"
#include "EMVCoTDA.h"
#include "TransceiveState.h"
#include <log/log.h>
#include <string.h>

using aidl::vendor::nxp::emvco::NxpEmvcoStatus;
using namespace aidl::vendor::nxp::emvco::tda::test;
OpenState *OpenState::mOpenState = nullptr;

OpenState::OpenState() { ALOGI("%s \n", __func__); }

/**
 * @brief  Calls the openTDA API on OpenState entry
 *
 **/
void OpenState::enter() {
  ALOGI("%s \n", __func__);
  EMVCoTDA::getEMVCoTDAInstance().openTDA(false, &mConnID);
}

/**
 * @brief On openTDA API failure, retries to open with the gap of 1 sec sleep.
 *        after max retry count moves to close state to close the logical
 *connection and deactivate the TDA slot
 *
 **/
void OpenState::retryOpen(bool standby) {
  if (retryCount++ < MAX_RETRY_COUNT) {
    ALOGI("%s wait for 1sec and retry open\n", __func__);
    std::chrono::milliseconds sleepTime(OPEN_SLEEP_DURATION);
    std::this_thread::sleep_for(sleepTime);
    EMVCoTDA::getEMVCoTDAInstance().openTDA(standby, &mConnID);
  } else {
    retryCount = 0;
    ALOGI("%s Max open retried. close and retry\n", __func__);
    StateMachine::getInstance().setState(&CloseState::getInstance());
  }
}
/**
 * @brief  handles all the error codes related to openTDA API call
 *
 **/
void OpenState::handleFailures(Response res) {
  switch (res.error) {
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_NFCEE_INTERFACE_ACTIVATION_FAILED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_NFCEE_TRANSMISSION_ERROR):
  case static_cast<int8_t>(NxpEmvcoStatus::EMVCO_STATUS_NFCEE_PROTOCOL_ERROR):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_NFCEE_MODE_SET_ENABLE_FAILED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_NFCEE_MODE_SET_ENABLE_TIMEOUT):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_CORE_CONN_CREATE_FAILED):
  case static_cast<int8_t>(NxpEmvcoStatus::EMVCO_STATUS_NCI_RESPONSE_ERR):
    retryOpen(false);
    break;
  case static_cast<int8_t>(NxpEmvcoStatus::EMVCO_STATUS_NFCEE_TIMEOUT_ERROR):
    ALOGI("%s EMVCO_STATUS_NFCEE_TIMEOUT_ERROR.\n", __func__);
    if (EMVCoTDA::getEMVCoTDAInstance().getTDASlot() == CT_CARD_TDA_ID) {
      retryOpen(false);
    } else {
      if (retryCount++ < MAX_RETRY_COUNT) {
        StateMachine::getInstance().setState(&DiscoverState::getInstance());
      } else {
        ALOGI("%s Enable mode set failed three times. Exit the application \n",
              __func__);
        EMVCoTDA::getEMVCoTDAInstance().stopTDALoopback();
        exit(1);
      }
    }
    break;
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_TDA_OPENED_ALREADY):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_CORE_CONN_CREATED_ALREADY):
    retryCount = 0;
    if (0x0A == getConnectionID() || 0x0B == getConnectionID() ||
        0x0C == getConnectionID()) {
      StateMachine::getInstance().setState(&TransceiveState::getInstance());
    } else {
      ALOGI("%s In-valid channel ID. Exit the application \n", __func__);
      EMVCoTDA::getEMVCoTDAInstance().stopTDALoopback();
      exit(1);
    }
    break;
  default:
    ALOGI("%s checking for state machine error \n", __func__);
    TDAState::handleFailures(res);
    break;
  }
}

/**
 * @brief  process the openTDA API response and moves to next state on
 *         sucess and handles error scenario's gracefully on failure
 *
 **/
void OpenState::processResponse(Response res) {
  if (res.status != 1 || res.exception != 0 || res.error != 0) {
    ALOGI("%s openTDA Failed. res.error:%d\n", __func__, res.error);
    handleFailures(res);
  } else {
    retryCount = 0;
    ALOGI("%s openTDA Success. connectionID:%d\n", __func__, mConnID);
    StateMachine::getInstance().setState(&TransceiveState::getInstance());
  }
}

/**
 * @brief  Returns the singleton object the OpenState class
 *
 **/
TDAState &OpenState::getInstance() {
  if (mOpenState == nullptr) {
    mOpenState = new OpenState();
  }
  return *mOpenState;
}
