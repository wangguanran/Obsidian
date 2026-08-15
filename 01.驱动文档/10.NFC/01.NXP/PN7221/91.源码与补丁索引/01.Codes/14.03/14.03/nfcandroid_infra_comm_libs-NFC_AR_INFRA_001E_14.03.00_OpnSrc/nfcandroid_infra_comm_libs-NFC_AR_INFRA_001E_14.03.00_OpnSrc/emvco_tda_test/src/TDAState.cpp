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
#define LOG_TAG "emvco_tda_test_tda_base_state"

/**
 * @brief  This base class is reponsible for handling the common/generic
 *         errors applicable for different states.
 *
 **/

#include "TDAState.h"
#include "CloseState.h"
#include "EMVCoTDA.h"
#include <log/log.h>

using namespace aidl::vendor::nxp::emvco::tda::test;

/**
 * @brief  handles the generic errors related to different states
 *
 **/
void TDAState::handleFailures(Response res) {
  ALOGI("%s res.error:%d\n", __func__, res.error);
  switch (res.error) {
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::
          EMVCO_STATUS_INVALID_STATE_CORE_CONN_CREATE_NOT_COMPLETED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_TDA_OPENED_ALREADY):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_CORE_CONN_CREATED_ALREADY):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_TDA_IN_CLOSED):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_CORE_CONN_CLOSED_ALREADY):
  case static_cast<int8_t>(
      NxpEmvcoStatus::EMVCO_STATUS_INVALID_STATE_TDA_CLOSED_ALREADY):
  case static_cast<int8_t>(NxpEmvcoStatus::EMVCO_STATUS_NCI_RESPONSE_ERR):
    ALOGI("%s Invalid state. close and retry", __func__);
    StateMachine::getInstance().setState(&CloseState::getInstance());
    break;
  default:
    ALOGI("%s Un-handled error.close and retry", __func__);
    StateMachine::getInstance().setState(&CloseState::getInstance());
    break;
  }
}
