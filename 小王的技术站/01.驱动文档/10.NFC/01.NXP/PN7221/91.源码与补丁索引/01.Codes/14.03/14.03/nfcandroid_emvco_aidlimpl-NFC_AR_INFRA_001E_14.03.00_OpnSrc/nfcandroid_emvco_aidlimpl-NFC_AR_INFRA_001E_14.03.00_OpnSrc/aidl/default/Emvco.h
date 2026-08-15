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
#pragma once
#ifndef EMVCO_HAL_DEBUG
#define EMVCO_HAL_DEBUG true
#endif
/** @defgroup EMVCO_HAL_API_INTERFACE.
 * The android application calls this interface to get the EMVCo mode switch and
 * Contact less Transaction functionality
 */

/** \addtogroup EMVCO_STACK_HAL_API_INTERFACE
 *  @brief interface to EMVCo mode switch, contact and contact interfaces.
 *           The android device, which supports EMVCo feature, implements
 * INxpEmvcoProfileDiscovery, INxpEmvcoContactlessCard, INxpEmvcoTDA interface
 * as
 * @ref EMVCo_POLLER_LIB to provide functionalities to Android application.
 *  @{
 */

#include "EmvcoContactlessCard.h"
#include "EmvcoProfileDiscovery.h"
#include "EmvcoTDA.h"
#include <aidl/vendor/nxp/emvco/BnNxpEmvco.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoClientCallback.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoTDACallback.h>
#include <aidl/vendor/nxp/emvco/INxpNfcStateChangeRequestCallback.h>
#include <aidl/vendor/nxp/emvco/NxpDiscoveryMode.h>
#include <android-base/logging.h>
#include <log/log.h>

namespace aidl {
namespace vendor {
namespace nxp {
namespace emvco {

using ::ndk::ICInterface;
class LinkedCallback;

class Emvco : public BnNxpEmvco {
public:
  Emvco();

  /**
   * @brief Provides the EMVCo mode switch HAL interface.
   *
   * @param  none
   *
   * @return INxpEmvcoProfileDiscovery EMVCo mode switch HAL interface
   *
   */
  ::ndk::ScopedAStatus getEmvcoProfileDiscoveryInterface(
      std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoProfileDiscovery>
          *_aidl_return) override;

  /**
   * @brief Provides the EMVCo contact less HAL interface.
   *
   * @param  none
   *
   * @return INxpEmvcoContactlessCard EMVCo contact less HAL interface
   *
   */
  ::ndk::ScopedAStatus getEmvcoContactlessCard(
      std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoContactlessCard>
          *_aidl_return) override;

  /**
   * @brief Provides the EMVCo contact HAL interface.
   *
   * @param  none
   *
   * @return INxpEmvcoTDA EMVCo contact HAL interface
   *
   */
  ::ndk::ScopedAStatus getEmvcoTDA(
      std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoTDA> *_aidl_return)
      override;
  ::ndk::ScopedAStatus onNfcStateChange(NxpNfcState in_nfcState);

  ::ndk::ScopedAStatus registerEMVCoEventListener(
      const std::shared_ptr<INxpEmvcoClientCallback> &in_clientCallback,
      bool *_aidl_return);
  ::ndk::ScopedAStatus setEMVCoMode(int8_t in_config, bool in_isStartEMVCo);
  ::ndk::ScopedAStatus transceive(const std::vector<uint8_t> &in_data,
                                  int32_t *_aidl_return);
  binder_status_t dump(int fd, const char **args, uint32_t num_args) override;
  ::ndk::ScopedAStatus open();
  ::ndk::ScopedAStatus
  close(const std::shared_ptr<INxpEmvcoClientCallback> &in_clientCallback);
  ::ndk::ScopedAStatus registerNFCStateChangeCallback(
      const std::shared_ptr<
          ::aidl::vendor::nxp::emvco::INxpNfcStateChangeRequestCallback>
          &in_nfcStateChangeCallback,
      bool *_aidl_return);
  ::ndk::ScopedAStatus getCurrentDiscoveryMode(
      ::aidl::vendor::nxp::emvco::NxpDiscoveryMode *_aidl_return);
  ::ndk::ScopedAStatus stopRFDisovery(
      ::aidl::vendor::nxp::emvco::NxpDeactivationType in_deactivationType,
      ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *emvco_status);
  ::ndk::ScopedAStatus
  setLed(::aidl::vendor::nxp::emvco::NxpLedControl in_ledControl,
         ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *emvco_status);
  ::ndk::ScopedAStatus
  setByteConfig(::aidl::vendor::nxp::emvco::NxpConfigType in_type,
                int32_t in_length, int8_t in_value,
                ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *_aidl_return);
  ::ndk::ScopedAStatus registerEMVCoCTListener(
      const std::shared_ptr<INxpEmvcoTDACallback> &in_clientCallback,
      bool *_aidl_return);
  ::ndk::ScopedAStatus discoverTDA(
      std::vector<::aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo> *emvcoTDAInfo);
  ::ndk::ScopedAStatus openTDA(int8_t in_tdaID, bool in_standBy,
                               int8_t *out_connID);
  ::ndk::ScopedAStatus transceive(const std::vector<uint8_t> &in_cmd_data,
                                  std::vector<uint8_t> *out_rsp_data);
  ::ndk::ScopedAStatus closeTDA(int8_t in_tdaID, bool in_standBy);

  static void eventCallback(uint8_t event, uint8_t status);
  static void dataCallback(uint16_t data_len, uint8_t *p_data);
  static void onCLStateChange(uint8_t state, char *debugReason);
  static void onTDAStateChange(void *tda, char *debugReason);
  static std::shared_ptr<Emvco> getInstance();
  static void setNxpNfcState(bool enableNfc);
  static std::shared_ptr<INxpNfcStateChangeRequestCallback>
      nfc_State_change_callback;

private:
  friend LinkedCallback;
  static std::shared_ptr<Emvco> emvco_service_;
  static std::vector<std::unique_ptr<LinkedCallback>> cl_callbacks_;
  static std::vector<std::unique_ptr<LinkedCallback>> ct_callbacks_;
  static std::mutex callbacks_lock_;
  ndk::ScopedAIBinder_DeathRecipient death_recipient_;
  std::shared_ptr<INxpEmvcoProfileDiscovery> emvco_profile_discovery_;
  std::shared_ptr<INxpEmvcoTDA> nxp_emvco_tda_;
  std::shared_ptr<INxpEmvcoContactlessCard> emvco_contactless_card;
  static std::shared_ptr<INxpEmvcoTDACallback> emvco_tda_callback;

  void
  registerCallback(const std::shared_ptr<INxpEmvcoClientCallback> &callback);
  void
  unregisterCallback(const std::shared_ptr<INxpEmvcoClientCallback> &callback);
  void
  registerCTCallback(const std::shared_ptr<INxpEmvcoTDACallback> &callback);
  void
  unregisterCTCallback(const std::shared_ptr<INxpEmvcoTDACallback> &callback);
  void setEMVCoModeImpl(int8_t in_config);
};

} // namespace emvco
} // namespace nxp
} // namespace vendor
} // namespace aidl

/** @}*/
