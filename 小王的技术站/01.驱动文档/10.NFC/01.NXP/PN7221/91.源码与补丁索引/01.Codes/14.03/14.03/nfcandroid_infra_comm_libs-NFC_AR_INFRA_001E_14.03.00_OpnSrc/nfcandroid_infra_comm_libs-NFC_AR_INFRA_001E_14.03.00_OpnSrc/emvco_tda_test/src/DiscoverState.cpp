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

#define LOG_TAG "emvco_tda_test_discover_state"

/**
 * @brief  This class is responsible for calling DiscoverState API
 *         and handles all the errors related to DiscoverState API.
 *
 **/
#include "DiscoverState.h"
#include "CloseState.h"
#include "EMVCoTDA.h"
#include "OpenState.h"
#include <log/log.h>

using namespace aidl::vendor::nxp::emvco::tda::test;
using namespace aidl::vendor::nxp::emvco;

DiscoverState *DiscoverState::mDiscoverState = nullptr;

DiscoverState::DiscoverState() { ALOGI("%s \n", __func__); }

/**
 * @brief  Calls the discoverTDA API on DiscoverState enrty
 *
 **/
void DiscoverState::enter() {
  ALOGI("%s \n", __func__);
  EMVCoTDA::getEMVCoTDAInstance().discoverTDA(&mContactCards);
}

/**
 * @brief  checks the smard card status of the TDA slot for loopback requested
 *
 **/
bool checkTDAStatus(std::vector<NxpEmvcoTDAInfo> tdas) {
  bool isCardConnected = true;
  ALOGI("TDA size:%d", (int)tdas.size());
  for (NxpEmvcoTDAInfo tda : tdas) {
    ALOGI("TDA:%s", tda.toString().c_str());
    if (EMVCoTDA::getEMVCoTDAInstance().getTDASlot() == tda.id &&
        NxpEmvcoTDAStatus::UN_RESPONSIVE_CARD_NOT_INSERTED == tda.status) {
      isCardConnected = false;
    }
  }
  return isCardConnected;
}

/**
 * @brief  handles all the error codes related to discoverTDA API call
 *
 **/
void DiscoverState::handleFailures(Response res) {
  switch (res.error) {
  case static_cast<int8_t>(NxpEmvcoStatus::EMVCO_STATUS_TDA_INIT_FAILED):
    EMVCoTDA::getEMVCoTDAInstance().stopTDALoopback();
    break;

  default:
    ALOGI("%s checking for state machine error \n", __func__);
    TDAState::handleFailures(res);
    break;
  }
}

/**
 * @brief  process the discoverTDA API response and moves to next state on
 *         sucess and handles error scenario's gracefully on failure
 *
 **/
void DiscoverState::processResponse(Response res) {
  if (res.status != 1 || res.exception != 0 || res.error != 0) {
    ALOGI("%s DiscoverTDA Failed. res.error:%d \n", __func__, res.error);
    handleFailures(res);
  } else {
    if (!checkTDAStatus(mContactCards)) {
      ALOGI("%s Smart card is not present. Waiting for card to be connected \n",
            __func__);
      if (EMVCoTDA::getEMVCoTDAInstance().getTDASlot() == SAM1_CARD_TDA_ID ||
          EMVCoTDA::getEMVCoTDAInstance().getTDASlot() == SAM2_CARD_TDA_ID) {
        ALOGI("SAM Card not present in discoverTDA. Exit the application");
        EMVCoTDA::getEMVCoTDAInstance().stopTDALoopback();
        exit(1);
      } else {
        EMVCoTDA::getEMVCoTDAInstance().setTDAReady(false);
        EMVCoTDA::getEMVCoTDAInstance().wait();
      }
    }
    StateMachine::getInstance().setState(&OpenState::getInstance());
  }
}

/**
 * @brief  Returns the singleton object the CloseState class
 *
 **/
TDAState &DiscoverState::getInstance() {
  if (mDiscoverState == nullptr) {
    mDiscoverState = new DiscoverState();
  }
  return *mDiscoverState;
}