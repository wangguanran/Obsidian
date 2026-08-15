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

package vendor.nxp.emvco;

import vendor.nxp.emvco.INxpEmvcoClientCallback;
import vendor.nxp.emvco.NxpDeactivationType;
import vendor.nxp.emvco.NxpEmvcoStatus;

/** \addtogroup EMVCO_HAL_API_INTERFACE
 *  @{
 */

 /**
  * @brief Interface to EMVCo Contactless card HAL.
  *
  * The android application calls this interface to utilize EMVCo
  * Contact less functionality of the underlying device.
  *
  */

@VintfStability
interface INxpEmvcoContactlessCard {
    /**
    *
    * @brief Register callback function to receive the events from a listener device.
    *
    * @note This function is must to call before invoking any other api.
    *
    * @param cb INxpEmvcoClientCallback the event callback function to be
             passed by caller
    *
    * @return boolean returns true, if success and returns false, if failed to register
    */
    boolean registerEMVCoEventListener(in INxpEmvcoClientCallback clientCallback);

    /**
    * @brief send application data with the Device-Controller.
    *
    * @note In case if send data is failed, Application shall again invoke
    *          @ref open "open()" before
    *          invoking @ref open "this" API.
    *
    * @param[in] in_data Application data buffer
    *
    * @return int indicating execution status
    *
    */
    int transceive(in byte[] in_data);

    /**
    * @brief starts the EMVCo mode with the Device-Controller.
    *
    * Once the @ref open "Application Data Channel is "
    * established, the Application may send start the EMVCo mode with the
    * Device-Controller.Ensure to call setByteConfig with POLL_PROFILE_SEL and 0b00000010 
    * combination to run EMVCo digital mode
    *
    * @param[in] in_config EMVCo polling technologies are configured through this parameter
    * @param[in] in_isStartEMVCo specifies to start or stop the EMVCo mode
    *
    * @return void
    *
    */
    void setEMVCoMode(in byte in_config,boolean in_isStartEMVCo);

    /**
    * @brief stops the RF field and moves in to the specified deactivation state.
    *
    * @param[in] deactivationType specifies the state to be in after RF deactivation
    *
    * @return NxpEmvcoStatus returns EMVCO_STATUS_OK, if command processed successfully and returns EMVCO_STATUS_FAILED, if command
    *         is not processed due to in-valid state. EMVCo mode should be ON to call this API
    *
    */
    NxpEmvcoStatus stopRFDisovery(in NxpDeactivationType deactivationType);
}

/** @}*/