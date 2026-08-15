/******************************************************************************
 *
 *  Copyright 2022-2024 NXP
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

#include "emvco_tml_i2c.h"
#include <cutils/properties.h>
#include <dlfcn.h>
#include <emvco_cl.h>
#include <emvco_config.h>
#include <emvco_ct.h>
#include <emvco_dm.h>
#include <emvco_log.h>
#include <emvco_nci_ext.h>
#include <emvco_ncif.h>
#include <emvco_tml.h>
#include <errno.h>
#include <log/log.h>
#include <nci_parser.h>
#include <osal_memory.h>
#include <osal_message_queue_lib.h>
#include <osal_thread.h>
#include <peripherals.h>
#include <stdlib.h>
#include <sys/stat.h>
/*********************** Global Variables *************************************/
#define PN547C2_CLOCK_SETTING
#define CORE_RES_STATUS_BYTE 3

void *p_emvco_ecp_vas_handle = NULL;
fp_init_ecp_vas_t fp_init_ecp_vas = NULL;

void *p_emvco_ct_one_bin_handle = NULL;
fp_ct_init_ext_t fp_ct_init_ext = NULL;
fp_ct_de_init_ext_t fp_ct_de_init_ext = NULL;
fp_ct_nfcee_discover_t fp_ct_nfcee_discover = NULL;
fp_ct_process_emvco_mode_rsp_t fp_ct_process_emvco_mode_rsp = NULL;
fp_ct_open_t fp_ct_open = NULL;
fp_ct_close_t fp_ct_close = NULL;
fp_is_ct_send_app_data_t fp_is_ct_send_app_data = NULL;
fp_transceive_t fp_transceive = NULL;
fp_ct_discover_tda_t fp_ct_discover_tda = NULL;
fp_is_ct_data_credit_received_t fp_is_ct_data_credit_received = NULL;
fp_is_ct_data_rsp_t fp_is_ct_data_rsp = NULL;
fp_on_emvco_rf_pool_start_t fp_on_emvco_rf_pool_start = NULL;
fp_set_max_wtx_timeout_value_t fp_set_max_wtx_timeout_value = NULL;

/* Processing of ISO 15693 EOF */
extern uint8_t icode_send_eof;
extern uint8_t icode_detected;
static uint8_t cmd_icode_eof[] = {0x00, 0x00, 0x00};

static uint8_t config_access = false;
static uint8_t config_success = true;

extern nci_clock_t nci_clock;
extern emvco_args_t *modeSwitchArgs;

/* NCI HAL Control structure */
nci_hal_ctrl_t nci_hal_ctrl;

/* NXP Poll Profile structure */
nci_profile_Control_t nxpprofile_ctrl;

/* TML Context */
extern tml_emvco_context_t *gptml_emvco_context;
uint32_t wFwVerRsp;

extern uint16_t wFwVer;
extern uint16_t rom_version;

uint32_t timeoutTimerId = 0;
bool emvco_debug_enabled;

/*  Used to send Callback Transceive data during Mifare Write.
 *  If this flag is enabled, no need to send response to Upper layer */
bool sendRspToUpperLayer = true;

nci_hal_sem config_data;

nci_clock_t nci_clock = {0, {0}, false};

rf_setting_t phNxpNciRfSet = {false, {0}};

eeprom_area_t eeprom_area = {false, {0}};

/**************** local methods used in this file only ************************/
static void open_app_data_channel_complete(EMVCO_STATUS status);
static int min_open_app_data_channel();
static int handle_power_cycle(void);
int min_close_app_data_channel(void);
static void min_open_app_data_channel_complete(EMVCO_STATUS status);
static int check_ncicmd_write_window(uint16_t cmd_len, uint8_t *p_cmd);
static void read_app_data_complete(void *p_context,
                                   osal_transact_info_t *pInfo);
static void close_app_data_channel_complete(EMVCO_STATUS status);
static void power_cycle_complete(EMVCO_STATUS status);
static void kill_emvco_hal_client_thread(nci_hal_ctrl_t *p_nxpncihal_ctrl);
static void *emvco_hal_client_thread(void *arg);
static void print_res_status(uint8_t *p_rx_data, uint16_t *p_len);

static void initialize_debug_enabled_flag();

/******************************************************************************
 * Function         initialize_debug_enabled_flag
 *
 * Description      This function gets the value for emvco_debug_enabled
 *
 * Returns          void
 *
 ******************************************************************************/
static void initialize_debug_enabled_flag() {
  unsigned long num = 0;
  unsigned int num_len = 0;
  if (get_byte_value(NAME_NXP_EMVCO_DEBUG_ENABLED, &num, &num_len)) {
    LOG_EMVCOHAL_D("emvco_debug_enabled conf from file: %lu", num);
    emvco_debug_enabled = (num == 0) ? false : true;
  }
  LOG_EMVCOHAL_D("emvco_debug_enabled : %d", emvco_debug_enabled);
}

/******************************************************************************
 * Function         emvco_hal_client_thread
 *
 * Description      This function is a thread handler which handles all TML and
 *                  NCI messages.
 *
 * Returns          void
 *
 ******************************************************************************/
static void *emvco_hal_client_thread(void *arg) {
  nci_hal_ctrl_t *p_nci_hal_ctrl = (nci_hal_ctrl_t *)arg;
  lib_emvco_message_t msg;

  LOG_EMVCOHAL_D("thread started");

  p_nci_hal_ctrl->thread_running = 1;

  while (p_nci_hal_ctrl->thread_running == 1) {
    /* Fetch next message from the NFC stack message queue */
    if (osal_msg_rcv(p_nci_hal_ctrl->gDrvCfg.n_client_id, &msg, 0, 0) == -1) {
      LOG_EMVCOHAL_E("NFC client received bad message");
      continue;
    }

    if (p_nci_hal_ctrl->thread_running == 0) {
      break;
    }

    switch (msg.e_msgType) {
    case EMVCO_DEFERRED_CALL_MSG: {
      REENTRANCE_LOCK();
      lib_emvco_deferred_call_t *deferCall =
          (lib_emvco_deferred_call_t *)(msg.p_msg_data);

      osal_transact_info_t osal_transact_info;
      osal_transact_info_t *pTransactionInfo = &osal_transact_info;
      if (pTransactionInfo != NULL && msg.size > 0) {
        pTransactionInfo->p_buff = msg.data;
        pTransactionInfo->w_length = msg.size;
        pTransactionInfo->w_status = msg.w_status;
        deferCall->p_callback(pTransactionInfo);
        process_emvco_mode_rsp(pTransactionInfo);
      } else {
        LOG_EMVCOHAL_E("TransInfo is invalid");
      }
      REENTRANCE_UNLOCK();

      break;
    }

    case EMVCO_OPEN_CHNL_CPLT_MSG: {
      REENTRANCE_LOCK();
      if (nci_hal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_OPEN_CHNL_CPLT_EVT, STATUS_OK);
      }
      REENTRANCE_UNLOCK();
      break;
    }

    case EMVCO_CLOSE_CHNL_CPLT_MSG: {
      REENTRANCE_LOCK();
      modeSwitchArgs->is_start_emvco = false;
      if (nci_hal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_POLLING_STOP_EVT, STATUS_OK);
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_CLOSE_CHNL_CPLT_EVT, STATUS_OK);
      }
      if (nci_hal_ctrl.p_nfc_state_cback != NULL) {
        (*nci_hal_ctrl.p_nfc_state_cback)(true);
      }
      kill_emvco_hal_client_thread(&nci_hal_ctrl);
      REENTRANCE_UNLOCK();
      break;
    }

    case EMVCO_OPEN_CHNL_ERROR_MSG: {
      REENTRANCE_LOCK();
      if (nci_hal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_OPEN_CHNL_ERROR_EVT,
                                          STATUS_FAILED);
      }
      REENTRANCE_UNLOCK();
      break;
    }

    case EMVCO_DATA_RX_EVT: {
      REENTRANCE_LOCK();
      if (nci_hal_ctrl.p_nfc_stack_data_cback != NULL) {
        (*nci_hal_ctrl.p_nfc_stack_data_cback)(nci_hal_ctrl.rsp_len,
                                               nci_hal_ctrl.p_rsp_data);
      }
      REENTRANCE_UNLOCK();
      break;
    }
    case EMVCO_POLLING_STARTED_MSG: {
      REENTRANCE_LOCK();
      if (nci_hal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        LOG_EMVCOHAL_E("EMVCO_POLLING_STARTED_EVT propagted to upper layer");
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_POLLING_STARTED_EVT, STATUS_OK);
        if (fp_on_emvco_rf_pool_start != NULL) {
          fp_on_emvco_rf_pool_start();
        }
      }
      REENTRANCE_UNLOCK();
      break;
    }
    case EMVCO_POOLING_STARTING_MSG: {
      REENTRANCE_LOCK();
      if (nci_hal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_POOLING_START_EVT, STATUS_OK);
      }
      REENTRANCE_UNLOCK();
      break;
    }
    case EMVCO_POOLING_START_FAILED_MSG: {
      REENTRANCE_LOCK();
      if (nci_hal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_POOLING_START_EVT,
                                          STATUS_FAILED);
      }
      REENTRANCE_UNLOCK();
      break;
    }
    case EMVCO_UN_SUPPORTED_CARD_MSG: {
      REENTRANCE_LOCK();
      if (nci_hal_ctrl.p_nfc_stack_cback != NULL) {
        (*nci_hal_ctrl.p_nfc_stack_cback)(EMVCO_UN_SUPPORTED_CARD_EVT,
                                          STATUS_OK);
      }
      REENTRANCE_UNLOCK();
      break;
    }
    }
  }

  return NULL;
}

/******************************************************************************
 * Function         kill_emvco_hal_client_thread
 *
 * Description      This function safely kill the client thread and clean all
 *                  resources.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void kill_emvco_hal_client_thread(nci_hal_ctrl_t *p_nci_hal_ctrl) {
  LOG_EMVCOHAL_D("Terminating phNxpNciHal client thread...");
  p_nci_hal_ctrl->p_nfc_stack_cback = NULL;
  p_nci_hal_ctrl->p_nfc_stack_data_cback = NULL;
  p_nci_hal_ctrl->p_nfc_state_cback = NULL;
  p_nci_hal_ctrl->thread_running = 0;
  return;
}

int open_app_data_channelImpl(emvco_stack_callback_t *p_cback,
                              emvco_stack_data_callback_t *p_data_cback,
                              emvco_state_change_callback_t *p_nfc_state_cback,
                              emvco_tda_state_change_t *p_tda_state_change,
                              emvco_cl_state_change_t *p_cl_state_change) {
  EMVCO_STATUS wConfigStatus = EMVCO_STATUS_SUCCESS;
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;
  LOG_EMVCOHAL_D("open_app_data_channel nfc_status:%d", nfc_status);
  read_config(emvco_hal_config_path);

  if (nci_hal_ctrl.halStatus == HAL_STATUS_OPEN) {
    LOG_EMVCOHAL_D("open_app_data_channel already open");
    return EMVCO_STATUS_SUCCESS;
  } else if (nci_hal_ctrl.halStatus == HAL_STATUS_CLOSE) {
    status = min_open_app_data_channel();
    if (status != EMVCO_STATUS_SUCCESS) {
      LOG_EMVCOHAL_E("min_open_app_data_channel failed");
      goto clean_and_return;
    }
  }
  nci_hal_ctrl.p_nfc_stack_cback = p_cback;
  nci_hal_ctrl.p_nfc_stack_data_cback = p_data_cback;
  nci_hal_ctrl.p_nfc_state_cback = p_nfc_state_cback;
  m_p_tda_state_change = p_tda_state_change;
  m_p_cl_state_change = p_cl_state_change;

  open_app_data_channel_complete(wConfigStatus);

  return wConfigStatus;

clean_and_return:
  CONCURRENCY_UNLOCK();
  if (p_cback != NULL) {
    (*p_cback)(EMVCO_OPEN_CHNL_CPLT_EVT, STATUS_FAILED);
  }

  nci_hal_ctrl.p_nfc_stack_cback = NULL;
  nci_hal_ctrl.p_nfc_stack_data_cback = NULL;
  cleanup_monitor();
  nci_hal_ctrl.halStatus = HAL_STATUS_CLOSE;
  return EMVCO_STATUS_FAILED;
}

void initialize_emvco_ecp_vas() {
  LOG_EMVCOHAL_D("%s", __func__);
  p_emvco_ecp_vas_handle =
      dlopen("/system/vendor/lib64/emvco_ecp_vas.so", RTLD_NOW);
  if (p_emvco_ecp_vas_handle == NULL) {
    LOG_EMVCOHAL_D(
        "Error : opening (/system/vendor/lib64/emvco_ecp_vas.so) !!");
    return;
  }
  if ((fp_init_ecp_vas = (fp_init_ecp_vas_t)dlsym(p_emvco_ecp_vas_handle,
                                                  "init_ecp_vas")) == NULL) {
    LOG_EMVCOHAL_D("Error while linking (init_ecp_vas) !!");
    return;
  }
}

void initialize_emvco_ct() {
  LOG_EMVCOHAL_D("%s", __func__);
  p_emvco_ct_one_bin_handle =
      dlopen("/system/vendor/lib64/emvco_tda.so", RTLD_NOW);
  if (p_emvco_ct_one_bin_handle == NULL) {
    LOG_EMVCOHAL_E("Error : opening (/system/vendor/lib64/emvco_tda.so) !!");
    return;
  }
  if ((fp_ct_init_ext = (fp_ct_init_ext_t)dlsym(p_emvco_ct_one_bin_handle,
                                                "ct_init_ext")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (ct_init_ext) !!");
  }
  if ((fp_ct_de_init_ext = (fp_ct_de_init_ext_t)dlsym(
           p_emvco_ct_one_bin_handle, "ct_de_init_ext")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (ct_de_init_ext) !!");
  }
  if ((fp_ct_nfcee_discover = (fp_ct_nfcee_discover_t)dlsym(
           p_emvco_ct_one_bin_handle, "ct_nfcee_discover")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (ct_nfcee_discover) !!");
  }

  if ((fp_ct_discover_tda = (fp_ct_discover_tda_t)dlsym(
           p_emvco_ct_one_bin_handle, "ct_discover_tda")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (ct_discover_tda) !!");
  }

  if ((fp_ct_process_emvco_mode_rsp = (fp_ct_process_emvco_mode_rsp_t)dlsym(
           p_emvco_ct_one_bin_handle, "process_tda_rsp_ntf")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (process_tda_rsp_ntf) !!");
  }
  if ((fp_ct_open =
           (fp_ct_open_t)dlsym(p_emvco_ct_one_bin_handle, "ct_open")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (ct_open) !!");
  }
  if ((fp_ct_close = (fp_ct_close_t)dlsym(p_emvco_ct_one_bin_handle,
                                          "ct_close")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (ct_close) !!");
  }

  if ((fp_is_ct_data_credit_received = (fp_is_ct_data_credit_received_t)dlsym(
           p_emvco_ct_one_bin_handle, "is_ct_data_credit_received")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (is_ct_data_credit_received) !!");
  }
  if ((fp_is_ct_send_app_data = (fp_is_ct_send_app_data_t)dlsym(
           p_emvco_ct_one_bin_handle, "is_ct_send_app_data")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (is_ct_send_app_data) !!");
  }

  if ((fp_transceive = (fp_transceive_t)dlsym(p_emvco_ct_one_bin_handle,
                                              "ct_transceive")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (transceive) !!");
  }
  if ((fp_is_ct_data_rsp = (fp_is_ct_data_rsp_t)dlsym(
           p_emvco_ct_one_bin_handle, "is_ct_data_rsp")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (is_ct_data_rsp) !!");
  }
  if ((fp_on_emvco_rf_pool_start = (fp_on_emvco_rf_pool_start_t)dlsym(
           p_emvco_ct_one_bin_handle, "on_emvco_rf_pool_start")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (on_emvco_rf_pool_start) !!");
  }
  if ((fp_set_max_wtx_timeout_value = (fp_set_max_wtx_timeout_value_t)dlsym(
           p_emvco_ct_one_bin_handle, "set_max_wtx_timeout_value")) == NULL) {
    LOG_EMVCOHAL_E("Error while linking (set_max_wtx_timeout_value) !!");
  }
}
/******************************************************************************
 * Function         min_open_app_data_channel
 *
 * Description      This function initializes the least required resources to
 *                  communicate to NFCC.This is mainly used to communicate to
 *                  NFCC when NFC service is not available.
 *
 *
 * Returns          This function return EMVCO_STATUS_SUCCESS (0) in case of
 *                  success. In case of failure returns other failure value.
 *
 ******************************************************************************/
int min_open_app_data_channel() {
  osal_emvco_config_t tOsalConfig;
  tml_emvco_Config_t tTmlConfig;
  char *p_nfc_dev_node = NULL;
  unsigned int dev_node_size = 0;
  uint32_t mode_switch_status;
  EMVCO_STATUS wConfigStatus = EMVCO_STATUS_SUCCESS;
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;
  LOG_EMVCOHAL_D("phNxpNci_MinOpen(): enter");

  if (nci_hal_ctrl.halStatus == HAL_STATUS_MIN_OPEN) {
    LOG_EMVCOHAL_D("min_open_app_data_channel(): already open");
    return EMVCO_STATUS_SUCCESS;
  }

  int init_retry_cnt = 0;
  int8_t ret_val = 0x00;

  /*Create the timer for extns write response*/
  timeoutTimerId = osal_timer_create();

  if (init_monitor() == NULL) {
    LOG_EMVCOHAL_E("Init monitor failed");
    return EMVCO_STATUS_FAILED;
  }

  CONCURRENCY_LOCK();
  osal_memset(&nci_hal_ctrl, 0x00, sizeof(nci_hal_ctrl));
  osal_memset(&tOsalConfig, 0x00, sizeof(tOsalConfig));
  osal_memset(&tTmlConfig, 0x00, sizeof(tTmlConfig));
  osal_memset(&nxpprofile_ctrl, 0, sizeof(nci_profile_Control_t));

  /*Init binary semaphore to handle back to back NCI write synchronization*/
  if (0 != osal_sem_init(&nci_hal_ctrl.sync_nci_write, 0, 1)) {
    LOG_EMVCOHAL_E("osal_sem_init() FAiled for sync_nci_write, errno = 0x%02X",
                   errno);
    goto clean_and_return;
  }

  /* By default HAL status is HAL_STATUS_OPEN */
  nci_hal_ctrl.halStatus = HAL_STATUS_OPEN;

  /*nci version NCI_VERSION_UNKNOWN version by default*/
  nci_hal_ctrl.nci_info.nci_version = NCI_VERSION_UNKNOWN;

  initialize_emvco_ecp_vas();
  initialize_emvco_ct();
  initialize_max_wtx_timeout_value();
  if (fp_ct_init_ext != NULL) {
    EMVCO_STATUS status =
        fp_ct_init_ext(m_p_tda_state_change, m_p_cl_state_change);
    if (status != EMVCO_STATUS_SUCCESS) {
      LOG_EMVCOHAL_E("CT Initialization failed");
    }
  }
  /* Read the nfc device node name */
  if (!get_byte_array_value(NAME_NXP_EMVCO_DEV_NODE, &p_nfc_dev_node,
                            &dev_node_size)) {
    LOG_EMVCOHAL_D(
        "Invalid nfc device node name keeping the default device node "
        "/dev/nxpnfc");
    int len = (sizeof(char) * strlen(DEF_NFC_DEV_NODE) + 1);
    p_nfc_dev_node = (char *)osal_malloc(len);
    if (p_nfc_dev_node == NULL) {
      LOG_EMVCOHAL_E("malloc of p_nfc_dev_node failed ");
      goto clean_and_return;
    }
    strlcpy(p_nfc_dev_node, DEF_NFC_DEV_NODE, len);
  }

  /* Configure hardware link */
  nci_hal_ctrl.gDrvCfg.n_client_id = osal_msg_get(0, 0600);
  nci_hal_ctrl.gDrvCfg.n_link_type = ENUM_LINK_TYPE_I2C;
  tTmlConfig.p_dev_name = (int8_t *)p_nfc_dev_node;
  tOsalConfig.dw_callback_thread_id =
      (uintptr_t)nci_hal_ctrl.gDrvCfg.n_client_id;
  tOsalConfig.p_log_file = NULL;
  tTmlConfig.dw_get_msg_thread_id = (uintptr_t)nci_hal_ctrl.gDrvCfg.n_client_id;

  /* Initialize TML layer */
  wConfigStatus = tml_init(&tTmlConfig);
  if (wConfigStatus != EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_E("tml_init Failed");
    goto clean_and_return;
  } else {
    if (p_nfc_dev_node != NULL) {
      osal_free(p_nfc_dev_node);
      p_nfc_dev_node = NULL;
    }
  }

  /* Create the client thread */
  ret_val = osal_thread_create(&nci_hal_ctrl.emvco_hal_client_thread, NULL,
                               emvco_hal_client_thread, &nci_hal_ctrl);
  if (ret_val != 0) {
    LOG_EMVCOHAL_E("osal_thread_create failed");
    wConfigStatus = tml_shutdown_cleanup();
    goto clean_and_return;
  }

  CONCURRENCY_UNLOCK();

  /* call read pending */
  status =
      tml_read(nci_hal_ctrl.p_rsp_data, NCI_MAX_DATA_LEN,
               (transact_completion_callback_t)&read_app_data_complete, NULL);
  if (status != EMVCO_STATUS_PENDING) {
    LOG_EMVCOHAL_E("TML Read status error status = %x", status);
    wConfigStatus = tml_shutdown_cleanup();
    wConfigStatus = EMVCO_STATUS_FAILED;
    goto clean_and_return;
  }

  nci_ext_init();

init_retry:

  mode_switch_status = tml_ioctl(NFCCModeSwitchOn);
  LOG_EMVCOHAL_D("%s modeswitch IOCTL status:%d", __func__, mode_switch_status);
  led_switch_control(GREEN_LED_ON);
  status = send_core_reset(NCI_RESET_TYPE_KEEP_CFG);
  if (status != EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_E("NCI_CORE_RESET: Failed");
    if (init_retry_cnt < 3) {
      init_retry_cnt++;
      (void)handle_power_cycle();
      goto init_retry;
    } else
      init_retry_cnt = 0;
    wConfigStatus = tml_shutdown_cleanup();
    wConfigStatus = EMVCO_STATUS_FAILED;
    goto clean_and_return;
  }

  if (nci_hal_ctrl.nci_info.nci_version == NCI_VERSION_2_0) {
    status = send_core_init(NCI_VERSION_2_0);
  } else {
    status = send_core_init(NCI_VERSION_UNKNOWN);
  }
  if (status != EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_E("NCI_CORE_INIT : Failed");
    if (init_retry_cnt < 3) {
      init_retry_cnt++;
      (void)handle_power_cycle();
      goto init_retry;
    } else
      init_retry_cnt = 0;
    wConfigStatus = tml_shutdown_cleanup();
    wConfigStatus = EMVCO_STATUS_FAILED;
    goto clean_and_return;
  }

  //NAME_NXP_PCD_SETTINGS configuration applied as part of NFC initialization
  get_set_config(NAME_NXP_SET_CONFIG);
  get_set_config(NAME_NXP_GET_CONFIG);

  /* Call open complete */
  min_open_app_data_channel_complete(wConfigStatus);
  LOG_EMVCOHAL_D("min_open_app_data_channel(): exit");
  return wConfigStatus;

clean_and_return:
  min_close_app_data_channel();
  CONCURRENCY_UNLOCK();
  if (p_nfc_dev_node != NULL) {
    osal_free(p_nfc_dev_node);
    p_nfc_dev_node = NULL;
  }
  return EMVCO_STATUS_FAILED;
}

/******************************************************************************
 * Function         get_set_config
 *
 * Description      This function gets/sets the configuration command using
 *config key and sends the set config command to controller
 *
 * Parameter        Takes the configuration key as input
 *
 * Returns          void.
 *
 ******************************************************************************/
void get_set_config(const char *p_conf_key) {
  LOG_EMVCOHAL_D("%s", __func__);
  int retry_cnt = 0;
  char *buffer = NULL;
  unsigned int bufflen = 0;

  if ((get_byte_array_value(p_conf_key, &buffer, &bufflen)) &&
      (buffer != NULL)) {
    do {
      if (EMVCO_STATUS_SUCCESS == send_ext_cmd(bufflen, (uint8_t *)buffer)) {
        retry_cnt = 0;
        break;
      } else {
        LOG_EMVCOHAL_E("Failed to set %s configuration\n ", p_conf_key);
        ++retry_cnt;
      }
    } while (retry_cnt < 3);
  } else {
    LOG_EMVCOHAL_E("Failed to set %s configuration. Please re-try by updating "
                   "the configuration file with proper value \n ",
                   p_conf_key);
  }
  free(buffer);
  buffer = NULL;
}

int open_app_data_channel(emvco_stack_callback_t *p_cback,
                          emvco_stack_data_callback_t *p_data_cback,
                          emvco_state_change_callback_t *p_nfc_state_cback,
                          emvco_tda_state_change_t *p_tda_state_change,
                          emvco_cl_state_change_t *p_cl_state_change) {
  LOG_EMVCOHAL_D("%s:", __func__);
  read_config(emvco_hal_config_path);
  if (modeSwitchArgs == NULL) {
    modeSwitchArgs =
        (struct emvco_args *)osal_malloc(sizeof(struct emvco_args));
    modeSwitchArgs->current_discovery_mode = UNKNOWN;
    modeSwitchArgs->emvco_config = -1;
    modeSwitchArgs->is_start_emvco = false;
  }
  m_p_nfc_stack_cback = p_cback;
  m_p_nfc_stack_data_cback = p_data_cback;
  m_p_nfc_state_cback = p_nfc_state_cback;
  m_p_tda_state_change = p_tda_state_change;
  m_p_cl_state_change = p_cl_state_change;

  /* initialize log levels */
  initialize_debug_enabled_flag();
  initialize_log_level();
  return EMVCO_STATUS_SUCCESS;
}

/******************************************************************************
 * Function         min_open_app_data_channel_complete
 *
 * Description      This function updates the status of
 *min_open_app_data_channel_complete to halstatus.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void min_open_app_data_channel_complete(EMVCO_STATUS status) {
  if (status == EMVCO_STATUS_SUCCESS) {
    nci_hal_ctrl.halStatus = HAL_STATUS_MIN_OPEN;
  }

  return;
}

/******************************************************************************
 * Function         open_app_data_channel_complete
 *
 * Description      This function inform the status of open_app_data_channel
 *                  function to libnfc-nci.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void open_app_data_channel_complete(EMVCO_STATUS status) {
  static lib_emvco_message_t msg;

  if (status == EMVCO_STATUS_SUCCESS) {
    msg.e_msgType = EMVCO_OPEN_CHNL_CPLT_MSG;
    nci_hal_ctrl.hal_open_status = true;
    nci_hal_ctrl.halStatus = HAL_STATUS_OPEN;
  } else {
    msg.e_msgType = EMVCO_OPEN_CHNL_ERROR_MSG;
  }

  msg.p_msg_data = NULL;
  msg.size = 0;

  tml_deferred_call(gptml_emvco_context->dw_callback_thread_id,
                    (lib_emvco_message_t *)&msg);

  return;
}

int send_app_data(uint16_t data_len, const uint8_t *p_data, bool is_tda) {
  return send_app_data_internal(data_len, p_data, is_tda);
}

int send_app_data_internal(uint16_t data_len, const uint8_t *p_data,
                           bool is_tda) {
  EMVCO_STATUS status = EMVCO_STATUS_FAILED;
  static lib_emvco_message_t msg;
  if (nci_hal_ctrl.halStatus != HAL_STATUS_OPEN) {
    LOG_EMVCOHAL_E("cmd_len exceeds limit NCI_MAX_DATA_LEN");
    return EMVCO_STATUS_FAILED;
  }
  int msg_type = p_data[0] & (NCI_MSG_TYPE_RSP << NCI_MT_SHIFT);
  if (NCI_MSG_CMD_ABS_VAL == msg_type) {
    if (fp_is_ct_send_app_data != NULL) {
      if (fp_is_ct_send_app_data(p_data, data_len, is_tda) == false) {
        LOG_EMVCOHAL_E("NCI command not allowed to send to controller");
        return 0; // Zero bytes written
      } else {
        LOG_EMVCOHAL_D("NCI command initiated from CT");
      }
    } else {
      LOG_EMVCOHAL_E("NCI command not allowed to send to controller");
      return 0; // Zero bytes written
    }
  }

  /* Create local copy of cmd_data */
  osal_memcpy(nci_hal_ctrl.p_cmd_data, p_data, data_len);
  nci_hal_ctrl.cmd_len = data_len;

  /* Check for NXP ext before sending write */
  status = send_app_data_ext(&nci_hal_ctrl.cmd_len, nci_hal_ctrl.p_cmd_data,
                             &nci_hal_ctrl.rsp_len, nci_hal_ctrl.p_rsp_data);
  if (status != EMVCO_STATUS_SUCCESS) {
    /* Do not send packet to controller, send response directly */
    msg.e_msgType = EMVCO_DATA_RX_EVT;
    msg.p_msg_data = NULL;
    msg.size = 0;

    tml_deferred_call(gptml_emvco_context->dw_callback_thread_id,
                      (lib_emvco_message_t *)&msg);
    goto clean_and_return;
  }

  CONCURRENCY_LOCK();
  data_len =
      send_app_data_unlocked(nci_hal_ctrl.cmd_len, nci_hal_ctrl.p_cmd_data);
  CONCURRENCY_UNLOCK();

  if (icode_send_eof == 1) {
    usleep(10000);
    icode_send_eof = 2;
    status = send_ext_cmd(3, cmd_icode_eof);
    if (status != EMVCO_STATUS_SUCCESS) {
      LOG_EMVCOHAL_E("ICODE end of frame command failed");
    }
  }

clean_and_return:
  /* No data written */
  return data_len;
}

int send_app_data_unlocked(uint16_t data_len, const uint8_t *p_data) {
  nci_hal_sem cb_data;
  /* Create the local semaphore */
  if (init_cb_data(&cb_data, NULL) != EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_D("send_app_data_unlocked Create cb data failed");
    data_len = 0;
    goto clean_and_return;
  }

  /* Create local copy of cmd_data */
  osal_memcpy(nci_hal_ctrl.p_cmd_data, p_data, data_len);
  nci_hal_ctrl.cmd_len = data_len;

  /* check for write synchronyztion */
  if (check_ncicmd_write_window(nci_hal_ctrl.cmd_len,
                                nci_hal_ctrl.p_cmd_data) !=
      EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_D("send_app_data_unlocked check nci write window failed");
    data_len = 0;
    osal_sem_post(&(nci_hal_ctrl.sync_nci_write));
    goto clean_and_return;
  }
  send_emvco_data(nci_hal_ctrl.p_cmd_data, nci_hal_ctrl.cmd_len);

clean_and_return:
  cleanup_cb_data(&cb_data);
  return data_len;
}
static bool is_data_credit_received(osal_transact_info_t *pInfo) {
  if ((6 == pInfo->w_length) &&
      (pInfo->p_buff[0] == NCI_MT_NTF &&
       pInfo->p_buff[1] == NCI_CORE_CONN_CREDITS_NTF &&
       pInfo->p_buff[2] == NCI_CORE_CONN_CREDITS_NTF_LEN &&
       pInfo->p_buff[3] == NCI_CORE_CONN_CREDITS_NTF_NO_OF_ENTRY &&
       pInfo->p_buff[4] == NCI_CORE_CONN_CREDITS_NTF_CONN_ID &&
       pInfo->p_buff[5] == NCI_CORE_CONN_CREDITS_NTF_CONN_CREDITS)) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief process the nci packet and checks for core interface error
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is CT packet otherwise returns false
 *
 */
static bool is_core_inf_err_ntf(osal_transact_info_t *pInfo) {
  if ((5 == pInfo->w_length) &&
      (pInfo->p_buff[0] == NCI_MT_NTF &&
       pInfo->p_buff[1] == NCI_CORE_INTERFACE_ERROR_NTF &&
       pInfo->p_buff[2] == NCI_CORE_INTERFACE_ERROR_NTF_LEN)) {
    return true;
  } else {
    return false;
  }
}

void enable_tml_read() {
  if (nci_hal_ctrl.halStatus == HAL_STATUS_CLOSE &&
      nci_hal_ctrl.nci_info.wait_for_rsp == FALSE) {
    return;
  }
  /* Read again because read must be pending always.*/
  gptml_emvco_context->t_read_info.b_thread_busy = false;

  EMVCO_STATUS status =
      tml_read(nci_hal_ctrl.p_rsp_data, NCI_MAX_DATA_LEN,
               (transact_completion_callback_t)&read_app_data_complete, NULL);
  if (status != EMVCO_STATUS_PENDING) {
    LOG_EMVCOHAL_E("read status error status = %x", status);
  }
}
static bool is_rf_link_loss_received(osal_transact_info_t *pInfo) {
  if ((5 == pInfo->w_length) &&
      (pInfo->p_buff[0] == NCI_RF_NTF &&
       pInfo->p_buff[1] == NCI_RF_DEACTIVATE_RES_NTF &&
       pInfo->p_buff[2] == NCI_DEACTIVATE_NTF_LEN &&
       pInfo->p_buff[3] == NCI_DEACT_TYPE_DISCOVERY &&
       pInfo->p_buff[4] == NCI_DEACT_RESON_RF_LINK_LOSS)) {
    nci_hal_ctrl.frag_rsp.data_pos = 0;
    RESET_CHAINED_DATA();
    return true;
  } else {
    return false;
  }
}

static void read_app_data_complete(void *p_context,
                                   osal_transact_info_t *pInfo) {
  EMVCO_STATUS status = EMVCO_STATUS_FAILED;
  int sem_val;
  UNUSED(p_context);
  if (nci_hal_ctrl.read_retry_cnt == 1) {
    nci_hal_ctrl.read_retry_cnt = 0;
  }
  if (pInfo->w_status == EMVCO_STATUS_SUCCESS) {

    osal_sem_getvalue(&(nci_hal_ctrl.sync_nci_write), &sem_val);

    if (fp_is_ct_data_credit_received != NULL) {
      if (fp_is_ct_data_credit_received(pInfo->p_buff, pInfo->w_length) ==
          true) {
        LOG_EMVCOHAL_D("CT data received. Unlocking")
        osal_sem_post(&(nci_hal_ctrl.sync_nci_write));
      }
    }
    if (((pInfo->p_buff[0] & NCI_MT_MASK) == NCI_MT_RSP ||
         is_data_credit_received(pInfo) || is_rf_link_loss_received(pInfo) ||
         is_core_inf_err_ntf(pInfo)) &&
        sem_val == 0) {
      LOG_EMVCOHAL_D("CT credit ntf received. Unlocking")
      osal_sem_post(&(nci_hal_ctrl.sync_nci_write));
    }
    /*Check the Omapi command response and store in dedicated buffer to solve
     * sync issue*/
    if (pInfo->p_buff[0] == 0x4F && pInfo->p_buff[1] == 0x01 &&
        pInfo->p_buff[2] == 0x01) {
      nci_hal_ctrl.p_rx_ese_data = pInfo->p_buff;
      nci_hal_ctrl.rx_ese_data_len = pInfo->w_length;
      SEM_POST(&(nci_hal_ctrl.ext_cb_data));
    } else {
      nci_hal_ctrl.p_rx_data = pInfo->p_buff;
      nci_hal_ctrl.rx_data_len = pInfo->w_length;
      status =
          process_ext_rsp(nci_hal_ctrl.p_rx_data, &nci_hal_ctrl.rx_data_len);
    }

    print_res_status(pInfo->p_buff, &pInfo->w_length);
    /* Check if response should go to hal module only */
    if (nci_hal_ctrl.hal_ext_enabled == TRUE &&
        (nci_hal_ctrl.p_rx_data[0x00] & NCI_MT_MASK) == NCI_MT_RSP) {
      if (status == EMVCO_STATUS_FAILED) {
        nci_hal_ctrl.ext_cb_data.status = status;
      }
      /* Unlock semaphore only for responses*/
      if ((nci_hal_ctrl.p_rx_data[0x00] & NCI_MT_MASK) == NCI_MT_RSP ||
          ((icode_detected == true) && (icode_send_eof == 3))) {
        /* Unlock semaphore */
        nci_hal_ctrl.nci_info.wait_for_rsp = FALSE;
        SEM_POST(&(nci_hal_ctrl.ext_cb_data));
      }
    } // Notification Checking
    else if ((nci_hal_ctrl.hal_ext_enabled == TRUE) &&
             ((nci_hal_ctrl.p_rx_data[0x00] & NCI_MT_MASK) == NCI_MT_NTF) &&
             (nci_hal_ctrl.nci_info.wait_for_ntf == TRUE)) {
      /* Unlock semaphore waiting for only  ntf*/
      SEM_POST(&(nci_hal_ctrl.ext_cb_data));
      nci_hal_ctrl.nci_info.wait_for_ntf = FALSE;
    }
    /* Read successful send the event to higher layer */
    else if ((nci_hal_ctrl.p_nfc_stack_data_cback != NULL) &&
             (status == EMVCO_STATUS_SUCCESS)) {

      if (fp_is_ct_data_rsp != NULL) {
        if (fp_is_ct_data_rsp(pInfo->p_buff, pInfo->w_length) == false) {
          process_emvco_data(nci_hal_ctrl.p_rx_data, nci_hal_ctrl.rx_data_len);
        } else {
          LOG_EMVCOHAL_D("%s CT data not processing.", __func__);
        }
      } else {
        process_emvco_data(nci_hal_ctrl.p_rx_data, nci_hal_ctrl.rx_data_len);
      }
    }
  } else {
    LOG_EMVCOHAL_E("read error status = 0x%x", pInfo->w_status);
  }


  return;
}

int close_app_data_channel(bool bShutdown) {
  EMVCO_STATUS status;
  const uint8_t cmd_ven_disable_nci[] = {0x05, 0x01, 0xA0, 0x07, 0x01, 0x02};
  uint8_t *p_cmd_ven_disable_nci = (uint8_t *)cmd_ven_disable_nci;
  if (nci_hal_ctrl.halStatus == HAL_STATUS_CLOSE) {
    LOG_EMVCOHAL_D("close_app_data_channel is already closed, ignoring close");
    return EMVCO_STATUS_FAILED;
  }

  CONCURRENCY_LOCK();

  int sem_val;
  osal_sem_getvalue(&(nci_hal_ctrl.sync_nci_write), &sem_val);
  if (sem_val == 0) {
    osal_sem_post(&(nci_hal_ctrl.sync_nci_write));
  }
  if (!bShutdown) {
    status = send_core_set_config(&p_cmd_ven_disable_nci[0],
                                  p_cmd_ven_disable_nci[1]);
    if (status != EMVCO_STATUS_SUCCESS) {
      LOG_EMVCOHAL_E("CMD_VEN_DISABLE_NCI: Failed");
    }
  }
  nci_hal_ctrl.nci_info.wait_for_rsp = true;
  nci_hal_ctrl.halStatus = HAL_STATUS_CLOSE;
  status = send_core_reset(NCI_RESET_TYPE_KEEP_CFG);
  if (status != EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_E("NCI_CORE_RESET: Failed");
  }
  osal_sem_destroy(&nci_hal_ctrl.sync_nci_write);
  if (NULL != gptml_emvco_context->p_dev_handle) {
    close_app_data_channel_complete(EMVCO_STATUS_SUCCESS);
    /* Abort any pending read and write */
    status = tml_read_abort();

    osal_timer_cleanup();

    status = tml_shutdown();

    if (0 !=
        osal_thread_join(nci_hal_ctrl.emvco_hal_client_thread, (void **)NULL)) {
      LOG_EMVCO_TML_E("Fail to kill client thread!");
    }

    tml_cleanup();

    osal_msg_release(nci_hal_ctrl.gDrvCfg.n_client_id);

    osal_memset(&nci_hal_ctrl, 0x00, sizeof(nci_hal_ctrl));

    LOG_EMVCOHAL_D("close_app_data_channel - phOsalNfc_DeInit completed");
  }
  CONCURRENCY_UNLOCK();
  cleanup_monitor();
  /* Return success always */
  return EMVCO_STATUS_SUCCESS;
}
/******************************************************************************
 * Function         min_close_app_data_channel
 *
 * Description      This function close the NFCC interface and free all
 *                  resources.This is called by libnfc-nci on NFC service stop.
 *
 * Returns          Always return EMVCO_STATUS_SUCCESS (0).
 *
 ******************************************************************************/
int min_close_app_data_channel(void) {
  EMVCO_STATUS status;
  /*NCI_RESET_CMD*/
  uint8_t cmd_reset_nci[] = {0x20, 0x00, 0x01, 0x00};
  CONCURRENCY_LOCK();
  nci_hal_ctrl.halStatus = HAL_STATUS_CLOSE;
  status = send_ext_cmd(sizeof(cmd_reset_nci), cmd_reset_nci);
  if (status != EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_E("NCI_CORE_RESET: Failed");
  }
  osal_sem_destroy(&nci_hal_ctrl.sync_nci_write);
  if (NULL != gptml_emvco_context->p_dev_handle) {
    close_app_data_channel_complete(EMVCO_STATUS_SUCCESS);
    /* Abort any pending read and write */
    status = tml_read_abort();

    osal_timer_cleanup();

    status = tml_shutdown();

    if (0 !=
        pthread_join(nci_hal_ctrl.emvco_hal_client_thread, (void **)NULL)) {
      LOG_EMVCO_TML_E("Fail to kill client thread!");
    }

    tml_cleanup();

    osal_msg_release(nci_hal_ctrl.gDrvCfg.n_client_id);

    osal_memset(&nci_hal_ctrl, 0x00, sizeof(nci_hal_ctrl));

    LOG_EMVCOHAL_D("close_app_data_channel - phOsalNfc_DeInit completed");
  }

  CONCURRENCY_UNLOCK();

  cleanup_monitor();

  return EMVCO_STATUS_SUCCESS;
}
/******************************************************************************
 * Function         close_app_data_channel_complete
 *
 * Description      This function inform libnfc-nci about result of
 *                  close_app_data_channel.
 *
 * Returns          void.
 *
 ******************************************************************************/
void close_app_data_channel_complete(EMVCO_STATUS status) {
  static lib_emvco_message_t msg;

  if (status == EMVCO_STATUS_SUCCESS) {
    msg.e_msgType = EMVCO_CLOSE_CHNL_CPLT_MSG;
  } else {
    msg.e_msgType = EMVCO_OPEN_CHNL_ERROR_MSG;
  }
  msg.p_msg_data = NULL;
  msg.size = 0;

  tml_deferred_call(gptml_emvco_context->dw_callback_thread_id, &msg);

  return;
}

/******************************************************************************
 * Function         handle_power_cycle
 *
 * Description      This function is called by libnfc-nci when power cycling is
 *                  performed. When processing is complete it is notified to
 *                  libnfc-nci through power_cycle_complete.
 *
 * Returns          Always return EMVCO_STATUS_SUCCESS (0).
 *
 ******************************************************************************/
int handle_power_cycle(void) {
  LOG_EMVCOHAL_D("Power Cycle");
  EMVCO_STATUS status = EMVCO_STATUS_FAILED;
  if (nci_hal_ctrl.halStatus != HAL_STATUS_OPEN) {
    LOG_EMVCOHAL_D("Power Cycle failed due to hal status not open");
    return EMVCO_STATUS_FAILED;
  }
  status = tml_ioctl(ResetDevice);

  if (EMVCO_STATUS_SUCCESS == status) {
    LOG_EMVCOHAL_D("Controller Reset - SUCCESS\n");
  } else {
    LOG_EMVCOHAL_D("Controller Reset - FAILED\n");
  }
  power_cycle_complete(EMVCO_STATUS_SUCCESS);
  return EMVCO_STATUS_SUCCESS;
}

/******************************************************************************
 * Function         power_cycle_complete
 *
 * Description      This function is called to provide the status of
 *                  handle_power_cycle to libnfc-nci through callback.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void power_cycle_complete(EMVCO_STATUS status) {
  static lib_emvco_message_t msg;

  if (status == EMVCO_STATUS_SUCCESS) {
    msg.e_msgType = EMVCO_OPEN_CHNL_CPLT_MSG;
  } else {
    msg.e_msgType = EMVCO_OPEN_CHNL_ERROR_MSG;
  }
  msg.p_msg_data = NULL;
  msg.size = 0;

  tml_deferred_call(gptml_emvco_context->dw_callback_thread_id, &msg);

  return;
}
/******************************************************************************
 * Function         check_ncicmd_write_window
 *
 * Description      This function is called to check the write synchroniztion
 *                  status if write already aquired then wait for corresponding
                    read to complete.
 *
 * Returns          return 0 on success and -1 on fail.
 *
 ******************************************************************************/

int check_ncicmd_write_window(uint16_t cmd_len, uint8_t *p_cmd) {
  EMVCO_STATUS status = EMVCO_STATUS_FAILED;
  int sem_timedout = 2, s;
  struct timespec ts;

  if (cmd_len < 1) {
    android_errorWriteLog(0x534e4554, "153880357");
    return EMVCO_STATUS_FAILED;
  }

  uint8_t cmd_pbf = (p_cmd[0] & 0xE0);
  if (cmd_pbf == 0x20 || cmd_pbf == 0x00) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += sem_timedout;

    while ((s = osal_sem_timedwait_monotonic_np(&nci_hal_ctrl.sync_nci_write,
                                                &ts)) == -1 &&
           errno == EINTR)
      continue; /* Restart if interrupted by handler */

    if (s != -1) {
      status = EMVCO_STATUS_SUCCESS;
    }
  } else {
    /* cmd window check not required for writing non nci cmd and data packet */
    status = EMVCO_STATUS_SUCCESS;
  }
  return status;
}

/******************************************************************************
 * Function         print_res_status
 *
 * Description      This function is called to process the response status
 *                  and print the status byte.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void print_res_status(uint8_t *p_rx_data, uint16_t *p_len) {
  static uint8_t response_buf[][30] = {"STATUS_OK",
                                       "STATUS_REJECTED",
                                       "STATUS_RF_FRAME_CORRUPTED",
                                       "STATUS_FAILED",
                                       "STATUS_NOT_INITIALIZED",
                                       "STATUS_SYNTAX_ERROR",
                                       "STATUS_SEMANTIC_ERROR",
                                       "RFU",
                                       "RFU",
                                       "STATUS_INVALID_PARAM",
                                       "STATUS_MESSAGE_SIZE_EXCEEDED",
                                       "STATUS_UNDEFINED"};
  int status_byte;
  if (p_rx_data[0] == 0x40 && (p_rx_data[1] == 0x02 || p_rx_data[1] == 0x03)) {
    if (p_rx_data[2] && p_rx_data[3] <= 10) {
      status_byte = p_rx_data[CORE_RES_STATUS_BYTE];
      LOG_EMVCOHAL_D("%s: response status =%s", __func__,
                     response_buf[status_byte]);
    } else {
      LOG_EMVCOHAL_D("%s: response status =%s", __func__, response_buf[11]);
    }
    if (nci_clock.is_clock_set) {
      int i, len = sizeof(nci_clock.p_rx_data);
      if (*p_len > len) {
        __osal_log_error_write(0x534e4554, "169257710");
      } else {
        len = *p_len;
      }
      for (i = 0; i < len; i++) {
        nci_clock.p_rx_data[i] = p_rx_data[i];
      }
    }

    else if (phNxpNciRfSet.is_get_config) {
      int i, len = sizeof(phNxpNciRfSet.p_rx_data);
      if (*p_len > len) {
        __osal_log_error_write(0x534e4554, "169258733");
      } else {
        len = *p_len;
      }
      for (i = 0; i < len; i++) {
        phNxpNciRfSet.p_rx_data[i] = p_rx_data[i];
        // LOG_EMVCOHAL_D("%s: response status =0x%x",__func__,p_rx_data[i]);
      }
    } else if (eeprom_area.is_get_eeprom_area) {
      int i, len = sizeof(eeprom_area.p_rx_data) + 8;
      if (*p_len > len) {
        __osal_log_error_write(0x534e4554, "169258884");
      } else {
        len = *p_len;
      }
      for (i = 8; i < len; i++) {
        eeprom_area.p_rx_data[i - 8] = p_rx_data[i];
      }
    } else if (nci_hal_ctrl.gpio_info.state == GPIO_STORE) {
      LOG_EMVCOHAL_D("%s: Storing GPIO Values...", __func__);
      nci_hal_ctrl.gpio_info.values[0] = p_rx_data[9];
      nci_hal_ctrl.gpio_info.values[1] = p_rx_data[8];
    } else if (nci_hal_ctrl.gpio_info.state == GPIO_RESTORE) {
      LOG_EMVCOHAL_D("%s: Restoring GPIO Values...", __func__);
      nci_hal_ctrl.gpio_info.values[0] = p_rx_data[9];
      nci_hal_ctrl.gpio_info.values[1] = p_rx_data[8];
    }
  }

  if (p_rx_data[2] && (config_access == true)) {
    if (p_rx_data[3] != EMVCO_STATUS_SUCCESS) {
      LOG_EMVCOHAL_W("Invalid Data from config file.");
      config_success = false;
    }
  }
}

void ct_process_emvco_mode_rsp_impl(osal_transact_info_t *pTransactionInfo) {
  if (fp_ct_process_emvco_mode_rsp != NULL) {
    fp_ct_process_emvco_mode_rsp(pTransactionInfo->p_buff,
                                 pTransactionInfo->w_length);
  } else {
    LOG_EMVCOHAL_W("CT not supported");
  }
}