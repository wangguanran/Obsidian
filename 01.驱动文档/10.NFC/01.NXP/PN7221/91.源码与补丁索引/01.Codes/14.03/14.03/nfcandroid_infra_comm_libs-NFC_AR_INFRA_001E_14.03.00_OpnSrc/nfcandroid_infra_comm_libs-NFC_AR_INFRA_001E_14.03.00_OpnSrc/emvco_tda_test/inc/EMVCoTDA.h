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
#pragma once
#include "StateMachine.h"
#include "TDAState.h"
#include <aidl/Gtest.h>
#include <aidl/vendor/nxp/emvco/BnNxpEmvco.h>
#include <aidl/vendor/nxp/emvco/BnNxpEmvcoTDACallback.h>
#include <aidl/vendor/nxp/emvco/INxpEmvco.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoContactlessCard.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoProfileDiscovery.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoTDA.h>
#include <aidl/vendor/nxp/emvco/NxpCardTLVInfo.h>
#include <aidl/vendor/nxp/emvco/NxpConfigType.h>
#include <aidl/vendor/nxp/emvco/NxpProtocols.h>
#include <android-base/stringprintf.h>
#include <android/binder_auto_utils.h>
#include <android/binder_enums.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <chrono>
#include <condition_variable>
#include <future>
#include <log/log.h>

namespace aidl::vendor::nxp::emvco::tda::test {
using aidl::vendor::nxp::emvco::INxpEmvco;
using aidl::vendor::nxp::emvco::INxpEmvcoContactlessCard;
using aidl::vendor::nxp::emvco::INxpEmvcoProfileDiscovery;
using aidl::vendor::nxp::emvco::INxpEmvcoTDA;
using aidl::vendor::nxp::emvco::INxpEmvcoTDACallback;
using aidl::vendor::nxp::emvco::NxpCardTLVInfo;
using aidl::vendor::nxp::emvco::NxpConfigType;
using aidl::vendor::nxp::emvco::NxpDeactivationType;
using aidl::vendor::nxp::emvco::NxpEmvcoEvent;
using aidl::vendor::nxp::emvco::NxpEmvcoState;
using aidl::vendor::nxp::emvco::NxpEmvcoStatus;
using aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo;
using aidl::vendor::nxp::emvco::NxpEmvcoTDAStatus;
using aidl::vendor::nxp::emvco::NxpLedControl;
using aidl::vendor::nxp::emvco::NxpProtocols;
using ndk::SpAIBinder;
using namespace std;

/* #define NCI_SEND_PPSE \
  {                                                                            \
    0x00, 0xA4, 0x04, 0x00, 0x0E, 0x31, 0x50, 0x41, 0x59, 0x2E, 0x53, 0x59,    \
        0x53, 0x2E, 0x44, 0x44, 0x46, 0x30, 0x31, 0x00                         \
  }
*/

#define CT_CARD_TDA_ID 0x20
#define SAM1_CARD_TDA_ID 0x21
#define SAM2_CARD_TDA_ID 0x22
#define UNKNOWN_TDA_SLOT -1

class EMVCoTDA {
private:
  EMVCoTDA();
  EMVCoTDA(const EMVCoTDA &other);
  EMVCoTDA &operator=(const EMVCoTDA &other);
  static void EmvcoHalBinderDied(void *cookie);
  static EMVCoTDA *mEMVCoTDA;
  int config_len = 0;
  std::shared_ptr<INxpEmvcoTDA> nxp_emvco_ct_service_;
  std::shared_ptr<INxpEmvcoContactlessCard> nxp_emvco_cl_service_;
  std::shared_ptr<INxpEmvcoProfileDiscovery> nxp_emvco_prof_disc_service_;
  std::shared_ptr<INxpEmvco> nxp_emvco_service_;
  ::ndk::ScopedAIBinder_DeathRecipient mDeathRecipient;
  std::condition_variable tda_cv;
  std::mutex tda_cv_m;
  volatile bool isTDAReady;
  std::condition_variable rsp_cv;
  std::mutex rsp_cv_m;
  volatile bool isRSPReady;
  int8_t mTdaSlot;
  Response mResponse;
  bool isCardAvailable();
  void initEMVCoHal();
  std::thread mRspHandlerThread;

public:
  static EMVCoTDA &getEMVCoTDAInstance();
  static volatile uint8_t pollingConfiguration;
  void discoverTDA(std::vector<NxpEmvcoTDAInfo> *contactCards);
  void openTDA(bool standBy, int8_t *channelId);
  void transceive(std::vector<uint8_t> cmdApdu, std::vector<uint8_t> *rspApdu);
  void closeTDA(bool standBy);
  void displayTDA();
  std::vector<NxpEmvcoTDAInfo> getTDAInfos();
  void wait();
  void startTDALoopback();
  bool stopTDALoopback();
  void setTDAReady(bool isTDAReady);
  void setTDASlot(int8_t tdaSlot) { mTdaSlot = tdaSlot; };
  int8_t getTDASlot() { return mTdaSlot; };
  void responseHandler();
  const static string CT_CARD_TDA_SLOT_NAME;
  const static string SAM1_CARD_TDA_SLOT_NAME;
  const static string SAM2_CARD_TDA_SLOT_NAME;
  const static int pollProfileSelectionVal;
};
} // namespace aidl::vendor::nxp::emvco::tda::test