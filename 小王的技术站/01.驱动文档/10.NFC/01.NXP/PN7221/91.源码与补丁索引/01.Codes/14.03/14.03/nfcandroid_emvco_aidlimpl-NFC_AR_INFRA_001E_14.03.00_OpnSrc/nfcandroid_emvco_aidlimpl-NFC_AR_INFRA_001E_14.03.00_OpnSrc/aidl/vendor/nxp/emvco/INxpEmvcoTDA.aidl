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

package vendor.nxp.emvco;

import vendor.nxp.emvco.INxpEmvcoClientCallback;
import vendor.nxp.emvco.NxpEmvcoTDAInfo;
import vendor.nxp.emvco.INxpEmvcoTDACallback;

/** \addtogroup EMVCO_HAL_API_INTERFACE
 *  @{
 */

/**
  * @brief Interface to EMVCo contact card HAL.
  *
  * The android application calls this interface to utilize EMVCo
  * Contact card functionality of the underlying device.
  *
  */
 
@VintfStability
interface INxpEmvcoTDA {
    
    /**
    *
    * @brief Register callback function to receive the events needed for CT functionality.
    *
    * @note This function is must to call before invoking any other api.
    *
    * @param[in]  *in_clientCallback has EMVCo contact card client HAL callback
    *
    * @return boolean returns true, if success and returns false, if failed to register
    */
    boolean registerEMVCoCTListener(in INxpEmvcoTDACallback in_clientCallback);

    /**
    *
    * @brief discoverTDA provides all the details of smart card connected over TDA
    *
    *
    * @throws ServiceSpecificException with code
    *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not supported.
    *
    * @return NxpEmvcoTDAInfo[] returns all the smart card connected over TDA.
    *         valid emvcoTDAInfo received only when status is EMVCO_STATUS_OK
    */
    NxpEmvcoTDAInfo[] discoverTDA();

    /**
    *
    * @brief opens the smart card connected over TDA. It internally calls
    *        mode set enable and core connection create. mode set enable is 
    *        called based on standBy flag.
    *
    * @note  Please gothrough closeTDA API to understand the usage of standBy flag
    *
    * @param[in]  tdaID tda id of the smard card received through discoverTDA
    * @param[in]  standBy false, opens the communication with TDA freshely and mode set enable command is sent to controller.
    *         standBy true, resumes the communication from partial close and does not send mode set enable command to controller
    * 
    * @note       use standby false, if you are opening the TDA for first time.
    *             use standby true, if you are opening the TDA followed by partial close of another TDA
    *
    *
    * @throws ServiceSpecificException with codes
    *   - EMVCO_STATUS_INVALID_PARAMETER, if provided tdaID is in-valid
    *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not supported.
    *
    * @return byte returns connection id of the smard card.
    *         valid connection id received only when status is EMVCO_STATUS_OK
    */
    byte openTDA(in byte tdaID, boolean standBy);

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
    *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not supported.
    *
    * @return Response APDU received from controller.
    *         valid Response APDU received only when status is EMVCO_STATUS_OK
    */
    byte[] transceive(in byte[] in_cmd_data);

    /**
    *
    * @brief closes the smart card connected over TDA.It internally calls
    *        core connection close and mode set disable. mode set disable is 
    *        called based on standBy flag.
    *
    * @param[in]  tdaID tda ID of the tda slot to be closed
    * @param[in]  standBy true, closes the communication with TDA fully and allows the system to go in standbymode 
    *         standBy false, closes the communication partially and does not allow the system to go in standbymode.
    * 
    * @note       use standby false, If you are closing the current TDA to open another TDA for communication then use false to get better performance
    *             use standby true, If you are closing the current TDA to stop the communication with it fully and allow system to enter standby mode
    *
    * @throws ServiceSpecificException with codes
    *   - EMVCO_STATUS_INVALID_PARAMETER, if provided tdaID is in-valid
    *   - EMVCO_STATUS_FEATURE_NOT_SUPPORTED when the contact card feature is not supported.
    *
    * @return void
    */
    void closeTDA(in byte tdaID, boolean standBy);

}
