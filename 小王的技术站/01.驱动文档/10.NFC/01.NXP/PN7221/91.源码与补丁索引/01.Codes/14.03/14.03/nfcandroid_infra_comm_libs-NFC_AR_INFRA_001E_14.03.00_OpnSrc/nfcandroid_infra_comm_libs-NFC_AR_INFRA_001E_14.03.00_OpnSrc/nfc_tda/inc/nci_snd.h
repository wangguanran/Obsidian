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

#ifndef NCI_SND_H_
#define NCI_SND_H_

/** \addtogroup NCI_SND_INTERFACE
 *  @brief  interface to form and send the NCI command to controller
 *  @{
 */

#include "pal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @brief           This function is called to enable or disable NFCEE
 *                  Discovery.
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_nfcee_discover();

/**
 *
 * @brief           This function is called to activate or de-activate an NFCEE
 *                  connected to the NFCC.
 *
 * @param[in]       nfcee_id - the NFCEE to activate or de-activate.
 *                  nfcee_mode - NFC_MODE_ACTIVATE to activate NFCEE,
 *                  NFC_MODE_DEACTIVATE to de-activate.
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_nfcee_mode_set(uint8_t nfcee_id, uint8_t nfcee_mode);

/**
 *
 * @brief           compose and send CORE CONN_CREATE command to command queue
 *
 * @param[in]       conn_id - Connection ID
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_core_conn_create(uint8_t conn_id);

/**
 *
 * @brief           compose and send CORE CONN_CLOSE command to command queue
 *
 * @param[in]       channel_num - logical connection ID
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_core_conn_close(uint8_t channel_num);

/**
 *
 * @brief           Internal function to send raw APDU to controller
 *
 * @param[in]       p_data - data buffer pointer
 * @param[in]       data_len - data buffer length
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_nfc_ct_data(uint8_t *p_data, uint16_t data_len);

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
/** @}*/
#endif /* NCI_SND_H_ */
