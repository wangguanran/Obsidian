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
#ifndef _EMVCO_DM_H_
#define _EMVCO_DM_H_

/** \addtogroup EMVCO_DATA_EXCHANGE_STACK_DM_API_INTERFACE
 *  @brief interface to EMVCO stack HAL to realize the EMVCo functionality.
 *  @{
 */

#include "emvco_tda.h"
#include <emvco_hal.h>
#include <emvco_tml.h>
#include <emvco_util.h>
#include <nci_defs.h>
#include <pthread.h>
/********************* Definitions and structures *****************************/
#define DEF_NFC_DEV_NODE "/dev/nxpnfc"
#define MAX_INIT_RETRY_COUNT 3
#define MAX_RETRY_COUNT 5
#define FRAG_MAX_DATA_LEN 1024
#define NCI_POLL_DURATION 500
#define HAL_EMVCO_ENABLE_I2C_FRAGMENTATION_EVT 0x07
#undef P2P_PRIO_LOGIC_HAL_IMP
#define NCI_VERSION_2_0 0x20
#define NCI_VERSION_1_1 0x11
#define NCI_VERSION_1_0 0x10
#define NCI_VERSION_UNKNOWN 0x00
typedef void(control_granted_callback_t)();

/*ROM CODE VERSION FW*/
#define FW_MOBILE_ROM_VERSION_PN548AD 0x10
#define FW_MOBILE_ROM_VERSION_PN551 0x10
#define FW_MOBILE_ROM_VERSION_PN553 0x11
#define FW_MOBILE_ROM_VERSION_PN557 0x12

#define CORE_RESET_TRIGGER_TYPE_CORE_RESET_CMD_RECEIVED 0x02
#define CORE_RESET_TRIGGER_TYPE_POWERED_ON 0x01
#define CORE_RESET_NTF_MODE_SWITCH_TO_NFC_FORUM 0xA8
#define CORE_RESET_NTF_MODE_SWITCH_TO_EMVCO 0xA9

#define NCI_MSG_CORE_RESET 0x00
#define NCI_MSG_CORE_INIT 0x01
#define NCI_MT_MASK 0xE0
#define NCI_OID_MASK 0x3F

#define NCI_CONN_ID_MASK 0x0F

#define NXP_MAX_CONFIG_STRING_LEN 260
#define NCI_HEADER_SIZE 3

#define NFC_A_PASSIVE_POLL_MODE 1
#define NFC_B_PASSIVE_POLL_MODE 2
#define NFC_AB_PASSIVE_POLL_MODE 3
#define NFC_F_PASSIVE_POLL_MODE 4
#define NFC_AF_PASSIVE_POLL_MODE 5
#define NFC_BF_PASSIVE_POLL_MODE 6
#define NFC_ABF_PASSIVE_POLL_MODE 7
#define NFC_VAS_PASSIVE_POLL_MODE 8
#define NFC_AVAS_PASSIVE_POLL_MODE 9
#define NFC_BVAS_PASSIVE_POLL_MODE 10
#define NFC_ABVAS_PASSIVE_POLL_MODE 11
#define NFC_FVAS_PASSIVE_POLL_MODE 12
#define NFC_AFVAS_PASSIVE_POLL_MODE 13
#define NFC_BFVAS_PASSIVE_POLL_MODE 14
#define NFC_ABFVAS_PASSIVE_POLL_MODE 15

/**********************************************
 * NCI Core Group Params
 **********************************************/
#define NCI_CORE_PARAM_SIZE_RESET 0x01
#define NCI_RESET_TYPE_KEEP_CFG 0x00

sem_t nfc_status_semaphore;

/*
 * EMVCo stack events that can be passed back to the EMVCo HAL
 */
enum {
  /** @brief  Event to indicate EMVCo HAL open complete state*/
  EMVCO_OPEN_CHNL_CPLT_EVT = 0u,
  /** @brief  Event to indicate EMVCo HAL open error state*/
  EMVCO_OPEN_CHNL_ERROR_EVT = 1u,
  /** @brief  Event to indicate EMVCo HAL close complete state*/
  EMVCO_CLOSE_CHNL_CPLT_EVT = 2u,
  /** @brief  Event to indicate the start of EMVCo mode*/
  EMVCO_POOLING_START_EVT = 3u,
  /** @brief  Event to indicate EMVCo polling activated state*/
  EMVCO_POLLING_STARTED_EVT = 4u,
  /** @brief  Event to indicate the stop of EMVCo mode*/
  EMVCO_POLLING_STOP_EVT = 5u,
  /** @brief  Event to indicate the Non EMV card*/
  EMVCO_UN_SUPPORTED_CARD_EVT = 6u,
};

/*
 * Status corresponds to EMVCo stack events
 */
enum {
  STATUS_OK = 0u,
  STATUS_FAILED = 1u,
};

emvco_stack_callback_t *m_p_nfc_stack_cback;
emvco_stack_data_callback_t *m_p_nfc_stack_data_cback;
emvco_state_change_callback_t *m_p_nfc_state_cback;
emvco_tda_state_change_t *m_p_tda_state_change;
emvco_cl_state_change_t *m_p_cl_state_change;

typedef struct nci_data {
  uint16_t len;
  uint8_t p_data[NCI_MAX_DATA_LEN];
} nci_data_t;

typedef enum {
  HAL_STATUS_CLOSE = 0,
  HAL_STATUS_OPEN,
  HAL_STATUS_MIN_OPEN
} emvco_hal_status;

typedef enum {
  GPIO_UNKNOWN = 0x00,
  GPIO_STORE = 0x01,
  GPIO_STORE_DONE = 0x02,
  GPIO_RESTORE = 0x10,
  GPIO_RESTORE_DONE = 0x20,
  GPIO_CLEAR = 0xFF
} gpio_info_state;

typedef struct gpio_info {
  gpio_info_state state;
  uint8_t values[2];
} gpio_info_t;

/* Macros to enable and disable extensions */
#define HAL_ENABLE_EXT() (nci_hal_ctrl.hal_ext_enabled = 1)
#define HAL_DISABLE_EXT() (nci_hal_ctrl.hal_ext_enabled = 0)

/**
 * @brief Structure representing Fragmented data and its properties.
 *
 * @param[in] p_data Pointer to the start of the Fragmented data.
 * @param[in] data_size Total size of the Fragmented data
 * @param[in] data_pos Current position in the Fragmented data.
 * @param[in] is_chained Indicates whether there is chained data or not.
 */

typedef struct frag_rsp {
  uint8_t p_data[FRAG_MAX_DATA_LEN];
  uint16_t data_size;
  uint16_t data_pos;
  uint8_t is_chained;
} frag_rsp_t;

typedef struct nci_info {
  uint8_t nci_version;
  bool_t wait_for_ntf;
  bool_t wait_for_rsp;
} nci_info_t;
/* NCI Control structure */
typedef struct nci_hal_ctrl {
  emvco_hal_status halStatus;        /* Indicate if hal is open or closed */
  pthread_t emvco_hal_client_thread; /* Integration thread handle */
  uint8_t thread_running;     /* Thread running if set to 1, else set to 0 */
  driver_config_data gDrvCfg; /* Driver config data */

  /* Rx data */
  uint8_t *p_rx_data;
  uint16_t rx_data_len;

  /* Rx data */
  uint8_t *p_rx_ese_data;
  uint16_t rx_ese_data_len;

  frag_rsp_t frag_rsp;

  /* libnfc-nci callbacks */
  emvco_stack_callback_t *p_nfc_stack_cback;
  emvco_stack_data_callback_t *p_nfc_stack_data_cback;
  emvco_state_change_callback_t *p_nfc_state_cback;

  /* control granted callback */
  control_granted_callback_t *p_control_granted_cback;

  /* HAL open status */
  bool_t hal_open_status;

  /* HAL extensions */
  uint8_t hal_ext_enabled;

  /* Waiting semaphore */
  nci_hal_sem ext_cb_data;
  sem_t sync_nci_write;

  uint16_t cmd_len;
  uint8_t p_cmd_data[NCI_MAX_DATA_LEN];
  uint16_t rsp_len;
  uint8_t p_rsp_data[NCI_MAX_DATA_LEN];

  /* retry count used to force download */
  uint16_t retry_cnt;
  uint8_t read_retry_cnt;
  nci_info_t nci_info;

  /* to store and restore gpio values */
  gpio_info_t gpio_info;
} nci_hal_ctrl_t;

typedef struct nci_clock {
  bool_t is_clock_set;
  uint8_t p_rx_data[20];
  bool_t is_set_config;
} nci_clock_t;

typedef struct rf_setting {
  bool_t is_get_config;
  uint8_t p_rx_data[20];
} rf_setting_t;

typedef struct eeprom_area {
  bool_t is_get_eeprom_area;
  uint8_t p_rx_data[32];
} eeprom_area_t;

typedef enum {
  NFC_FORUM_PROFILE,
  EMV_CO_PROFILE,
  INVALID_PROFILe
} nci_profile_t;
/* NXP Poll Profile control structure */
typedef struct nci_profile_Control {
  nci_profile_t profile_type;
  uint8_t
      b_clk_src_val; /* Holds the System clock source read from config file */
  uint8_t b_clk_freq_val; /* Holds the System clock frequency read from config
                             file */
  uint8_t b_timeout;      /* Holds the Timeout Value */
} nci_profile_Control_t;

/* Internal messages to handle callbacks */
#define EMVCO_OPEN_CHNL_CPLT_MSG 0x411
#define EMVCO_OPEN_CHNL_ERROR_MSG 0x412
#define EMVCO_CLOSE_CHNL_CPLT_MSG 0x413
#define EMVCO_POOLING_STARTING_MSG 0x414
#define EMVCO_POOLING_START_FAILED_MSG 0x415
#define EMVCO_POLLING_STARTED_MSG 0x416
#define EMVCO_UN_SUPPORTED_CARD_MSG 0x417

#define EMVCO_DATA_RX_EVT 0xF01

#define NCIHAL_CMD_CODE_LEN_BYTE_OFFSET (2U)
#define NCIHAL_CMD_CODE_BYTE_LEN (3U)

/**
 *
 * @brief           This function is called by EMVCo HAL during the
 *                  initialization of the NFCC. It opens the physical connection
 *                  with NFCC (PN7220) and creates required client thread for
 *                  operation.
 *                  After open is complete, status is informed to EMVCo HAL
 *                  through callback function.
 *
 * @param[in]       p_cback provides EMVCo event status to client
 * @param[in]       p_data_cback provides EMVCo data to client
 * @param[in]       p_nfc_state_change_req_cback EMVCo HAL requests NFC module
 * to turn ON or OFF the NFC through this callback
 *
 * @return          int status of the operation performed
 *
 */
int open_app_data_channel(emvco_stack_callback_t *p_cback,
                          emvco_stack_data_callback_t *p_data_cback,
                          emvco_state_change_callback_t *p_nfc_state_cback,
                          emvco_tda_state_change_t *p_tda_state_change,
                          emvco_cl_state_change_t *p_cl_state_change);

/**
 *
 * @brief       This function close the NFCC interface and free all
 *              resources.This is called by EMVCo HAL on EMVCo service stop.
 *
 * @param[in]   bShutdown true, if host is going to shutdown mode. false if host
 * is not going to shutdown mode.
 *
 * @return      int status of the close operation performed
 *
 */
int close_app_data_channel(bool shutdown);

/**
 * @brief           This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the driver interface.
 *                  Before sending the data to NFCC, send_app_data_ext
 *                  is called to check if there is any extension processing
 *                  is required for the NCI packet being sent out.
 *
 * @return          int It returns number of bytes successfully written to NFCC.
 *
 ******************************************************************************/
int send_app_data_internal(uint16_t data_len, const uint8_t *p_data,
                           bool is_tda);

/**
 *
 * @brief           This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN7220 driver interface.
 *                  Before sending the data to NFCC, phEMVCoHal_write_ext
 *                  is called to check if there is any extension processing
 *                  is required for the NCI packet being sent out.
 *
 * @param[in]       data_len length of the data to be written
 * @param[in]       p_data actual data to be written
 * @param[in]       is_tda specifies data received from CT or CL
 *
 * @return          int status of the write operation performed
 *
 */
int send_app_data(uint16_t data_len, const uint8_t *p_data, bool is_tda);

/**
 * @brief           This is the actual function which is being called by
 *                  send_app_data. This function writes the data to NFCC.
 *                  It waits till write callback provide the result of write
 *                  process.
 *
 * @param[in]       data_len length of the data to be written
 * @param[in]       p_data actual data to be written
 *
 * @return          int It returns number of bytes successfully written to NFCC.
 *
 ******************************************************************************/
int send_app_data_unlocked(uint16_t data_len, const uint8_t *p_data);

/**
 * @brief           Invokes tml_read Read to make sure always read thread is
 *                  pending
 *
 * @param[in]       void
 *
 * @return          void
 *
 ******************************************************************************/
void enable_tml_read();

int open_app_data_channelImpl(emvco_stack_callback_t *p_cback,
                              emvco_stack_data_callback_t *p_data_cback,
                              emvco_state_change_callback_t *p_nfc_state_cback,
                              emvco_tda_state_change_t *p_tda_state_change,
                              emvco_cl_state_change_t *p_cl_state_change);

void get_set_config(const char *p_nxp_conf);

void ct_process_emvco_mode_rsp_impl(osal_transact_info_t *pTransactionInfo);

typedef void (*fp_init_ecp_vas_t)();
typedef EMVCO_STATUS (*fp_ct_init_ext_t)(emvco_tda_state_change_t *,
                                         emvco_cl_state_change_t *);
typedef EMVCO_STATUS (*fp_ct_de_init_ext_t)();
typedef EMVCO_STATUS (*fp_ct_nfcee_discover_t)();
typedef void (*fp_ct_process_emvco_mode_rsp_t)(uint8_t *, uint16_t);
typedef EMVCO_STATUS (*fp_ct_open_t)(int8_t, int8_t, int8_t *);
typedef EMVCO_STATUS (*fp_ct_close_t)(int8_t, int8_t);
typedef EMVCO_STATUS (*fp_is_ct_send_app_data_t)(const uint8_t *, uint16_t,
                                                 bool);
typedef EMVCO_STATUS (*fp_transceive_t)(tda_data *, tda_data *);

typedef EMVCO_STATUS (*fp_ct_discover_tda_t)(tda_control_t *);
typedef bool (*fp_is_ct_data_credit_received_t)(uint8_t *, uint16_t);
typedef bool (*fp_is_ct_data_rsp_t)(uint8_t *, uint16_t);
typedef void (*fp_on_emvco_rf_pool_start_t)();
typedef void (*fp_set_max_wtx_timeout_value_t)();
#endif /* _EMVCO_DM_H_ */
