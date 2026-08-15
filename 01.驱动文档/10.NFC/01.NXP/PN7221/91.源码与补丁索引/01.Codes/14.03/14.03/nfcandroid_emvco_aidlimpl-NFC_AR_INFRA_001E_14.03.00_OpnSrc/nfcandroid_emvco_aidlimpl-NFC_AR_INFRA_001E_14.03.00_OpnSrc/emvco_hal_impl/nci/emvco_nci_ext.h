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
#ifndef _EMVCO_NCI_EXT_H_
#define _EMVCO_NCI_EXT_H_

/** \addtogroup EMVCO_STACK_NCI_API_INTERFACE
 *  @brief  interface to send and process the extension command
 *  @{
 */

#include <emvco_dm.h>
#include <emvco_tml.h>
#include <string.h>

#define NCI_MT_DATA 0x00
#define NCI_MT_CMD 0x20
#define NCI_MT_RSP 0x40
#define NCI_MT_NTF 0x60
#define NCI_CORE_CONN_CREDITS_NTF 0x06
#define NCI_CORE_CONN_CREDITS_NTF_LEN 0x03
#define NCI_CORE_CONN_CREDITS_NTF_NO_OF_ENTRY 0x01
#define NCI_CORE_CONN_CREDITS_NTF_CONN_ID 0x00
#define NCI_CORE_CONN_CREDITS_NTF_CONN_CREDITS 0x01

#define NCI_RF_NTF 0x61
#define NCI_RF_DEACTIVATE_RES_NTF 0x06
#define NCI_DEACTIVATE_NTF_LEN 0x02
#define NCI_DEACT_TYPE_DISCOVERY 0x03
#define NCI_DEACT_RESON_RF_LINK_LOSS 0x02
#define NCI_CORE_INTERFACE_ERROR_NTF 0x08
#define NCI_CORE_INTERFACE_ERROR_NTF_LEN 0x02

void nci_ext_init(void);

/**
 *
 * @brief        Process extension function response
 *
 * @param[in] p_cmd command to be processed
 * @param[in] p_len command length
 *
 * @return          EMVCO_STATUS_SUCCESS if success
 *
 */
EMVCO_STATUS process_ext_rsp(uint8_t *p_data, uint16_t *p_cmd);

/**
 *
 * @brief        sends the extension command
 *
 * @param[in] p_cmd command to be processed
 * @param[in] p_len command length
 *
 * @return          EMVCO_STATUS_SUCCESS if success
 *
 */
EMVCO_STATUS send_ext_cmd(uint16_t p_len, uint8_t *p_cmd);

/**
 * @brief      This function inform the status of open_app_data_channel
 *                  function to EMVCo HAL.
 *
 * @param[in] p_cmd_data command
 * @param[in] cmd_len command length
 * @param[in] p_rsp_data response data
 * @param[in] rsp_len response length
 *
 * @return          It return EMVCO_STATUS_SUCCESS then continue with send else
 *                  sends EMVCO_STATUS_FAILED direct response is prepared and
 *                  do not send anything to NFCC.
 *
 ******************************************************************************/
EMVCO_STATUS send_app_data_ext(uint16_t *cmd_len, uint8_t *p_cmd_data,
                               uint16_t *rsp_len, uint8_t *p_rsp_data);

/**
 * @brief process the NCI response and notification and ensures to do EMVCo
 * polling from EMVCo mode switch start.
 *
 * @param[in] osal_transact_info NCI response or ntf to be processed
 *
 * @return EMVCO_STATUS indicates success or failure
 *
 */
EMVCO_STATUS process_emvco_mode_rsp(osal_transact_info_t *osal_transact_info);
/** @}*/
#endif /* _PHNXPNICHAL_EXT_H_ */
