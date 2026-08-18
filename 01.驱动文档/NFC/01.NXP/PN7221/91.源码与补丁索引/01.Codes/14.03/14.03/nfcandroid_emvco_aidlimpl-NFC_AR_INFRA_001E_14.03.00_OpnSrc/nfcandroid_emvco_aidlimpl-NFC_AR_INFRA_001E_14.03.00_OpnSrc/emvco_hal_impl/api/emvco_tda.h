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

#ifndef _EMVCO_TDA_H_
#define _EMVCO_TDA_H_

#include "emvco_common.h"
#include "emvco_status.h"
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief MAX data length of CT fragmented data
 */
#define CT_FRAG_MAX_DATA_LEN 1024

/**
 * @brief tda_data defines the data buffer and length
 * can be send and received with controller
 */
typedef struct tda_data {
  uint32_t len;    /*!< length of the buffer */
  uint8_t *p_data; /*!< pointer to a buffer */
} tda_data;

/**
 * @brief tda_type provides the type of the smart
 * card connected over TDA
 */
typedef enum tda_type { CONTACT_CARD = 0, SAM, UN_KNOWN } tda_type_t;

/**
 * @brief card_tlv_info provides details of
 * smart card connected over TDA in TLV format
 */
typedef struct card_tlv_info {
  uint8_t type;
  uint8_t length;
  void *value;
} card_tlv_info_t;

typedef enum protocols {
  APDU = 0,
  RFU_01,
  T3T,
  TRANPARENT,
  RFU_04_7F,
  PROPRIETARY,
  RFU_FF
} protocols_t;

/**
 * @brief tda_status provides the status of the smart
 * card connected over TDA
 */
typedef enum tda_status {
  ENABLED = 0,
  DISABLED,
  UNRESPONSIVE,
  CC_STATUS_RFU
} tda_status_t;

/**
 * @brief tda provides all the details of
 * smart card connected over TDA
 */
typedef struct tda {
  int8_t id;
  tda_status_t status;
  uint8_t number_of_protocols;
  protocols_t *protocols_t;
  uint8_t number_of_card_info;
  card_tlv_info_t *card_tlv_info;
} tda_t;

/**
 * @brief mode_set_control has tda_id and mode(enable/disable)
 */
typedef struct mode_set_control {
  int8_t tda_id;
  int8_t mode;
} mode_set_control_t;

/**
 * @brief  defines the possible states of TDA
 *
 **/
typedef enum {
  INIT_STATE,
  DISCOVERED_STATE,
  MODE_SET_ENABLED_STATE,
  CORE_CONN_CREATED_STATE,
  CORE_CONN_CLOSED_STATE,
  LAST_STATE
} system_state_t;

/**
 * @brief    defines the possible API's and events of TDA
 *
 **/
typedef enum {
  NFCEE_DISCOVER_EVENT,
  DISCOVER_TDA_EVENT,
  OPEN_TDA_EVENT,
  CORE_CONN_CREATE_EVENT,
  TRANSCEIVE_EVENT,
  CLOSE_TDA_EVENT,
  CORE_CONN_CLOSE_EVENT,
  LAST_EVENT
} system_event_t;

typedef EMVCO_STATUS (*fp_event_handler_t)(void *);

/**
 * @brief    defines the state machine with state, event and
 *           corresponding event handler to process the data
 *
 **/
typedef struct {
  system_state_t state;
  system_event_t event;
  fp_event_handler_t handler;
} state_machine_t;

/**
 * @brief tda_channel_pair_t has tda_id and it's corresponding channel number
 */
typedef struct {
  int8_t tda_id;
  int8_t channel_num;
} tda_channel_pair_t;

/**
 * @brief transceive_buffer_t has command buffer and response buffer
 */
typedef struct {
  tda_data *cmd_apdu;
  tda_data *rsp_apdu;
} transceive_buffer_t;

/**
 * @brief emvco_state defines the EMVCo CT state
 */
typedef enum emvco_state {
  ON = 0,
  CL_TYPE_A_CARD_DETECTED,
  CL_TYPE_A_CARD_REMOVED,
  OFF
} emvco_state_t;

/**
 * @brief
 * The callback passed in from the EMVCo HAL that EMVCo
 * stack can use to pass emvco tda state change to EMVCo HAL.
 */
typedef void(emvco_tda_state_change_t)(void *tda_info, char *p_dbg_reason);

/**
 * @brief
 * The callback passed in from the EMVCo HAL that EMVCo
 * stack can use to pass emvco cl state change and card detection to EMVCo HAL.
 */
typedef void(emvco_cl_state_change_t)(uint8_t emvco_state, char *p_dbg_reason);

/**
 * @brief Structure representing Fragmented data and its properties.
 *
 * @param[in] p_data Pointer to the start of the Fragmented data.
 * @param[in] data_size Total size of the Fragmented data
 * @param[in] data_pos Current position in the Fragmented data.
 * @param[in] is_chained Indicates whether there is chained data or not.
 */

typedef struct ct_frag_rsp {
  uint8_t p_data[CT_FRAG_MAX_DATA_LEN];
  uint16_t data_size;
  uint16_t data_pos;
  uint8_t is_chained;
} ct_frag_rsp_t;

/**
 * @brief tda_control has information to handle the TDA functionality
 */
typedef struct tda_control {
  /* Points to the TDA Info structure */
  tda_t *p_tda;
  /* Holds the current TDA index */
  int8_t curr_tda;
  /* Holds the number of TDA supported by the system */
  int8_t num_tda_supported;
  /* Holds current state of the TDA */
  system_state_t tda_state;
  /* Holds current channel number of the opened TDA */
  int8_t curr_channel_num;
  /* Ensure to wait for API response and fills in the client API data buffer */
  sem_t sync_tda_write;
  /* Holds details about the tda_id and mode set request command */
  mode_set_control_t mode_set_ctrl;
  /* Holds latest client API status */
  uint16_t ret_status;
  /* Holds tda ID and it's correponding channel number */
  tda_channel_pair_t tda_ch_pr;
  /* Holds command and response APDU buffer and length */
  transceive_buffer_t trans_buf;
  /* Ensures mutual exclusion for all client API */
  pthread_mutex_t snd_lck;
  /* tda and cl state callbacks */
  emvco_tda_state_change_t *p_tda_state_change;
  emvco_cl_state_change_t *p_cl_state_change;
  /* Fragment data structure holds fragment response data information */
  ct_frag_rsp_t frag_rsp;
  /* specifies whether command data is fragmented or not */
  uint8_t is_chained_cmd;
} tda_control_t;

/* Macros to update the chained response data state */
#define CT_SET_CHAINED_RSP_DATA() (g_tda_ctrl.frag_rsp.is_chained = 1)
#define CT_RESET_CHAINED_RSP_DATA() (g_tda_ctrl.frag_rsp.is_chained = 0)
#define CT_IS_CHAINED_RSP_DATA() (1 == g_tda_ctrl.frag_rsp.is_chained)

/* Macros to update the chained command data state */
#define CT_SET_CHAINED_CMD_DATA() (g_tda_ctrl.is_chained_cmd = 1)
#define CT_RESET_CHAINED_CMD_DATA() (g_tda_ctrl.is_chained_cmd = 0)
#define CT_IS_CHAINED_CMD_DATA() (1 == g_tda_ctrl.is_chained_cmd)

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
#endif /* _EMVCO_TDA_H_ */
