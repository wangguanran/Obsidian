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

/** \addtogroup EMVCO_STACK_HAL_CT_API_INTERFACE
 *  @brief The android application calls this interface to use the EMVCo Contact
 *   card functionality.
 *  @{
 */
#include <aidl/vendor/nxp/emvco/BnNxpEmvcoTDA.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoClientCallback.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoTDACallback.h>
#include <android-base/logging.h>
#include <log/log.h>

namespace aidl {
namespace vendor {
namespace nxp {
namespace emvco {

class EmvcoTDA : public BnNxpEmvcoTDA {

public:
  /**
   *
   * @brief registers the EMVCoCT callback to EMVCo stack
   *
   * @param[in]  *in_in_clientCallback provides EMVCo state and TDA state as
   * callback
   *
   * @return void
   */
  ::ndk::ScopedAStatus registerEMVCoCTListener(
      const std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoTDACallback>
          &in_in_clientCallback,
      bool *_aidl_return) override;

  /**
   *
   * @brief discoverTDA provides all the details of smart card connected over
   * TDA
   *
   * @param[in]  *in_clientCallback provides EMVCo state and TDA state as
   * callback
   *
   * @throws ServiceSpecificException with code
   *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not
   * supported.
   *
   * @return NxpEmvcoTDAInfo[] returns all the smart card connected over TDA.
   *         valid emvcoTDAInfo received only when status is EMVCO_STATUS_OK
   */
  ::ndk::ScopedAStatus discoverTDA(
      std::vector<::aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo> *emvcoTDAInfo)
      override;
  /**
   *
   * @brief opens the smart card connected over TDA
   * @param[in]  tdaID tda id of the smard card received through discoverTDA
   *
   *
   * @throws ServiceSpecificException with codes
   *   - EMVCO_STATUS_INVALID_PARAMETER, if provided tdaID is in-valid
   *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not
   * supported.
   *
   * @return byte returns connection id of the smard card.
   *         valid connection id received only when status is EMVCO_STATUS_OK
   */
  ::ndk::ScopedAStatus openTDA(int8_t in_tdaID, bool in_standBy,
                               int8_t *out_connID) override;
  /**
   * @brief sends application data with the Device-Controller and
   *        receives response data from controller
   *
   * @note connection id of the TDA should be added as part of NCI header.
   *
   * @param[in] in_cmd_data Application command data buffer
   *
   * @throws ServiceSpecificException with codes
   *   - EMVCO_STATUS_INVALID_PARAMETER, if provided connection id is in-valid
   *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not
   * supported.
   *
   * @return Response APDU received from controller.
   *         valid Response APDU received only when status is EMVCO_STATUS_OK
   */
  ::ndk::ScopedAStatus transceive(const std::vector<uint8_t> &in_cmd_data,
                                  std::vector<uint8_t> *out_rsp_data) override;
  /**
   *
   * @brief closes the smart card connected over TDA
   * @param[in]  tdaID id of the tda slot to be closed
   *
   * @throws ServiceSpecificException with codes
   *   - EMVCO_STATUS_INVALID_PARAMETER, if provided tdaID is in-valid
   *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not
   * supported.
   *
   * @return void
   */
  ::ndk::ScopedAStatus closeTDA(int8_t in_tdaID, bool in_standBy) override;
};

} // namespace emvco
} // namespace nxp
} // namespace vendor
} // namespace aidl
  /** @}*/