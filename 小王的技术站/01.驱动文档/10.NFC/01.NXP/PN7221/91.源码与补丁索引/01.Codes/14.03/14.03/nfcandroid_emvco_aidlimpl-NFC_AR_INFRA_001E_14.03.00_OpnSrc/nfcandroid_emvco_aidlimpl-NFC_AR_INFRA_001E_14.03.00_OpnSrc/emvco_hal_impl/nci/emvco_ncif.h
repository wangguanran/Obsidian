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
/** \addtogroup EMVCO_STACK_NCI_API_INTERFACE
 *  @brief interface to send the NCI packets and process the fragmented data
 * received & loop back to NFCC
 *  @{
 */
#ifndef EMVCO_NCIF_H
#define EMVCO_NCIF_H

#include <emvco_cl.h>
#include <stdint.h>

#define MAX_FRAGMENT_SIZE 256
#define PBF_COMPLETE_MSG 0x00
#define PBF_SEGMENT_MSG 0x10
/* Macros to update the chained data state */
#define SET_CHAINED_DATA() (nci_hal_ctrl.frag_rsp.is_chained = 1)
#define RESET_CHAINED_DATA() (nci_hal_ctrl.frag_rsp.is_chained = 0)
#define IS_CHAINED_DATA() (1 == nci_hal_ctrl.frag_rsp.is_chained)
/**
 *
 * @brief      compose and send CORE RESET command to command queue
 * @param[in]  reset_type specfies to keep the configuration or not
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 */
uint8_t send_core_reset(uint8_t reset_type);

/**
 * @brief       compose and send CORE INIT command to command queue
 * @param[in]   nci_version specfies the nci version
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 */
uint8_t send_core_init(uint8_t nci_version);

/**
 *
 * @brief       compose and send CORE SET_CONFIG command to command queue
 * @param[in]       p_param_tlvs parameter in TLV format
 * @param[in]       tlv_size tlv parameter size
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 */
uint8_t send_core_set_config(uint8_t *p_param_tlvs, uint8_t tlv_size);

/**
 *
 * @brief           compose and send RF Management DISCOVER command to command
 *                  queue
 *
 * @param[in]       num number of polling types
 * @param[in]       p_param contains type and frequency of the polling type
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 */
uint8_t send_discover_cmd(uint8_t num, tEMVCO_DISCOVER_PARAMS *p_param);

/**
 *
 * @brief           compose and sends proprietary command to controller
 *
 * @param[in]       data_len command length
 * @param[in]       p_data   command data
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
uint8_t send_proprietary_act_cmd(uint16_t data_len, uint8_t *p_data);

/**
 *
 * @brief           Process the Received data packet from NFCC,
 *                  assembles and send it as complete packet
 *                  to upper layer
 *
 * @param[in]       data_len command length
 * @param[in]       p_data   command data
 *
 * @return          void
 *
 **/
void process_emvco_data(uint8_t *p_data, uint16_t data_len);

/**
 *
 * @brief           Process the Received data packet from Upper layer,
 *                  de-assembles, if length is more the Max fragment data
 *                  length and send it to NFCC
 *
 * @param[in]       data_len command length
 * @param[in]       p_data   command data
 *
 * @return          void
 *
 **/
void send_emvco_data(uint8_t *p_ntf, uint16_t p_len);

/**
 *
 * @brief           Deactivates the RF field and moves in to deactivation type
 *state
 *
 * @param[in]       de_act_type Idle or Discover state
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
uint8_t send_deactivate_cmd(uint8_t de_act_type);

/** @}*/
#endif /* EMVCO_NCIF_H */