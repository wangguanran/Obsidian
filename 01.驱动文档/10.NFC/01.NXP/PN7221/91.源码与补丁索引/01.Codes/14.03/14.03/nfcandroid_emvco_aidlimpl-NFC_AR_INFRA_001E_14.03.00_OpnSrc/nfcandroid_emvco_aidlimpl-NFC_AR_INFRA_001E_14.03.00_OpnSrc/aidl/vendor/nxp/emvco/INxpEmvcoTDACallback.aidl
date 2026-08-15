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

import vendor.nxp.emvco.NxpEmvcoState;
import vendor.nxp.emvco.NxpEmvcoTDAInfo;
/** \addtogroup EMVCO_HAL_API_INTERFACE
 *  @{
 */

/**
 * @brief Provides a contact card callback functionality to App.
 *        dynamic insertion and removal card status will be received
 *        as part of this callback
 */
@VintfStability
interface INxpEmvcoTDACallback {
    /**
     * Used to inform the client about changes in the state of the TDA
     * 
     * @param emvcoState state of EMVCo
     * @param debugReason provides additional data why there was a change in state
     *               This is used only for debugging purpose to understand
     *               in-field issues.
     *
     */
    oneway void onEMVCoCLStateChange(in NxpEmvcoState emvcoState, in String debugReason);

    /**
     * Used to inform the client about changes in the state of the TDA
     * 
     * @param emvcoTDAInfo indicates the information of the the smart card connected over TDA
     * @param debugReason provides additional data why there was a change in state
     *               ex. initialization error, smart card removed etc
     *               This is used only for debugging purpose to understand
     *               in-field issues.
     */
    oneway void onTDAStateChange(in NxpEmvcoTDAInfo emvcoTDAInfo, in String debugReason);
}
/** @}*/