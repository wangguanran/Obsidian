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

/** \addtogroup EMVCO_HAL_API_INTERFACE
 *  @{
 */

/**
 * @brief Event codes to be handled by Application
 *
 * The EMVCo Hal notifies application with below event codes.
 */
@VintfStability
@Backing(type="int")
enum NxpEmvcoEvent {
   /** @brief  Event to indicate EMVCo HAL open complete state*/
   EMVCO_OPEN_CHNL_CPLT_EVT = 0,
   /** @brief  Event to indicate EMVCo HAL open error state*/
   EMVCO_OPEN_CHNL_ERROR_EVT = 1,
   /** @brief  Event to indicate EMVCo HAL close complete state*/
   EMVCO_CLOSE_CHNL_CPLT_EVT = 2,
   /** @brief  Event to indicate the start of EMVCo mode*/
   EMVCO_POOLING_START_EVT = 3,
   /** @brief  Event to indicate EMVCo polling activated state*/
   EMVCO_POLLING_STARTED_EVT = 4,
   /** @brief  Event to indicate the stop of EMVCo mode*/
   EMVCO_POLLING_STOP_EVT = 5,
   /** @brief  Event to indicate the Non EMV card*/
   EMVCO_UN_SUPPORTED_CARD_EVT = 6,
}
/** @}*/