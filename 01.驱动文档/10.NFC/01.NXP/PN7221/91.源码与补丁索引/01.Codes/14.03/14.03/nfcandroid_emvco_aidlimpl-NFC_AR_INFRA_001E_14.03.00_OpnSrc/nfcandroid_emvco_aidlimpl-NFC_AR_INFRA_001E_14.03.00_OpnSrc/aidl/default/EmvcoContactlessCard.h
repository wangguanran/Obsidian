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

/** \addtogroup EMVCO_STACK_HAL_CL_API_INTERFACE
 *  @brief The android application calls this interface to use the EMVCo
 *   Contactless card functionality.
 *  @{
 */

#include <aidl/vendor/nxp/emvco/BnNxpEmvcoContactlessCard.h>
#include <aidl/vendor/nxp/emvco/INxpEmvcoClientCallback.h>

namespace aidl {
namespace vendor {
namespace nxp {
namespace emvco {

class EmvcoContactlessCard : public BnNxpEmvcoContactlessCard {

public:
  /**
   *
   * @brief Register EMVCo callback function to receive the events from a
   * listener device.
   *
   * @note This function is must to call before invoking any other api.
   *
   * @param[in]  *in_clientCallback has EMVCo client HAL callback
   * @param[in]  *in_aidl_return indicates register status in return to caller
   *
   * @return boolean returns true, if success and returns false, if failed to
   * register
   */
  ::ndk::ScopedAStatus registerEMVCoEventListener(
      const std::shared_ptr<INxpEmvcoClientCallback> &in_clientCallback,
      bool *_aidl_return) override;

  /**
   * @brief send application data with the Device-Controller.
   *
   * @note In case if send data is failed, Application shall again invoke
   *          @ref open "open()" before
   *          invoking @ref open "this" API.
   *
   * @param[in] in_data Application data buffer
   *
   * @return NxpEmvcoStatus indicating execution status
   *
   */
  ::ndk::ScopedAStatus transceive(const std::vector<uint8_t> &in_data,
                                  int32_t *_aidl_return) override;

  /**
   * @brief starts the EMVCo mode with the Device-Controller.
   *
   * Once the @ref open "Application Data Channel is "
   * established, the Application may send start the EMVCo mode with the
   * Device-Controller.
   *
   * @param[in] in_config EMVCo polling technologies are configured through this
   * parameter
   * @param[in] in_isStartEMVCo specifies to start or stop the EMVCo mode
   *
   * @return void
   *
   */
  ::ndk::ScopedAStatus setEMVCoMode(int8_t in_config, bool in_isStartEMVCo);

  /**
   * @brief stops the RF field and moves in to the specified deactivation state.
   *
   * @param[in] in_deactivationType specifies the state to be in after RF
   * deactivation
   *
   * @return NxpEmvcoStatus returns EMVCO_STATUS_OK, if command processed
   * successfully and returns EMVCO_STATUS_FAILED, if command is not processed
   * due to in-valid state. EMVCo mode should be ON to call this API
   *
   */
  ::ndk::ScopedAStatus stopRFDisovery(
      ::aidl::vendor::nxp::emvco::NxpDeactivationType in_deactivationType,
      ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *emvco_status) override;
};

} // namespace emvco
} // namespace nxp
} // namespace vendor
} // namespace aidl
  /** @}*/