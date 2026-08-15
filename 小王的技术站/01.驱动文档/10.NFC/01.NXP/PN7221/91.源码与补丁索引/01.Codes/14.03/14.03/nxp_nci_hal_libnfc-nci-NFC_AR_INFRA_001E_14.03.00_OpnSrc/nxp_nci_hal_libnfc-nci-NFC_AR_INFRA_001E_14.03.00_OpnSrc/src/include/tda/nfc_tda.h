/******************************************************************************
 *
 *  Copyright 2023-2024 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#ifndef _NFC_TDA_H_
#define _NFC_TDA_H_

#include "nfc_common.h"
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief tda_data defines the data buffer and length
 * can be send and received with controller
 */
typedef struct tda_data {
  uint32_t len;    /*!< length of the buffer */
  uint8_t *p_data; /*!< pointer to a buffer */
} tda_data;

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


#define FRAG_MAX_DATA_LEN 1024
#define SET_CHAINED_DATA() (nci_hal_ctrl.frag_rsp.is_chained = 1)
#define RESET_CHAINED_DATA() (nci_hal_ctrl.frag_rsp.is_chained = 0)
#define IS_CHAINED_DATA() (1 == nci_hal_ctrl.frag_rsp.is_chained)

typedef struct frag_rsp {
  uint8_t p_data[FRAG_MAX_DATA_LEN];
  uint16_t data_size;
  uint16_t data_pos;
  uint8_t is_chained;
} frag_rsp_t;

/**
 * @brief
 * The callback passed in from the EMVCo HAL that EMVCo
 * stack can use to pass nfc tda state change to EMVCo HAL.
 */
typedef void(nfc_tda_state_change_t)(void *tda_info, char *p_dbg_reason);

/**
 * @brief
 * The callback passed in from the EMVCo HAL that EMVCo
 * stack can use to pass nfc cl state change and card detection to EMVCo HAL.
 */
typedef void(nfc_cl_state_change_t)(uint8_t nfc_state, char *p_dbg_reason);

/* NCI Control structure */
typedef struct nci_hal_ctrl {
  frag_rsp_t frag_rsp;
  nfc_tda_state_change_t *p_tda_state_change;
  nfc_cl_state_change_t *p_cl_state_change;
} nci_hal_ctrl_t;

enum { TDA_STATE_INIT = 0, TDA_STATE_DISOVERY_DONE, TDA_STATE_MODE_SET_DONE };

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

typedef NFC_STATUS (*fp_event_handler_t)(void *);

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
} tda_control_t;

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
#endif /* _NFC_TDA_H_ */
