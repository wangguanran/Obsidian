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
#define LOG_TAG "emvco_tda_test_close_state"

/**
 * @brief  This class is responsible for calling closeTDA API
 *         and handles all the errors related to closeTDA API.
 *
 **/

#include "CloseState.h"
#include "DiscoverState.h"
#include "EMVCoTDA.h"
#include <log/log.h>

using namespace aidl::vendor::nxp::emvco::tda::test;
CloseState *CloseState::mCloseState = nullptr;

CloseState::CloseState() { ALOGI("%s \n", __func__); }

/**
 * @brief  Calls the closeTDA API on CloseState entry
 *
 **/
void CloseState::enter() {
  ALOGI("%s \n", __func__);
  EMVCoTDA::getEMVCoTDAInstance().closeTDA(true);
}

/**
 * @brief  handles all the error codes related to closeTDA API call
 *
 **/
void CloseState::handleFailures(Response res) {
  switch (res.error) {
  case static_cast<int8_t>(NxpEmvcoStatus::EMVCO_STATUS_CORE_CONN_CLOSE_FAILED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_CORE_CONN_CLOSED_ALREADY):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_TDA_CLOSED_ALREADY):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_NFCEE_MODE_SET_DISABLE_FAILED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED):
    ALOGI("%s Gracefully ignoring the close errors. EOS \n", __func__);
    break;
  default:
    ALOGE("%s State machine error. EOS \n", __func__);
    break;
  }
}

/**
 * @brief  process the close API response and moves to next state on
 *         sucess and handles error scenario's gracefully on failure
 *
 **/
void CloseState::processResponse(Response res) {
  if (res.status != 1 || res.exception != 0 || res.error != 0) {
    ALOGI("%s closeTDA Failed. res.error:%d \n", __func__, res.error);
    handleFailures(res);
  } else {
    ALOGI("%s End of statemachine cycle\n", __func__);
  }
  StateMachine::getInstance().setState(&DiscoverState::getInstance());
}

/**
 * @brief  Returns the singleton object the CloseState class
 *
 **/
TDAState &CloseState::getInstance() {
  if (mCloseState == nullptr) {
    mCloseState = new CloseState();
  }
  return *mCloseState;
}