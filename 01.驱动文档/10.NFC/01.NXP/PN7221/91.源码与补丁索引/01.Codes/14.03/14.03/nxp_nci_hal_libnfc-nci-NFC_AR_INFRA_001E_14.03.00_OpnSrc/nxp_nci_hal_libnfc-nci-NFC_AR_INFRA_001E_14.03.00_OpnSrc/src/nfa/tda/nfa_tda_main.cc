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
#if (NXP_EXTNS == TRUE)
#include "nfa_dm_int.h"
#include "nfa_ee_int.h"
#include "nfa_rw_int.h"
#include "nfa_tda_api.h"
#include "nfc_config.h"
#include "nfc_status.h"
#include <android-base/stringprintf.h>
#include <base/logging.h>
#include <csignal>
#include <dlfcn.h>
#include <string.h>
using android::base::StringPrintf;

extern bool nfc_debug_enabled;
static void nfa_tda_sys_enable(void);
static void nfa_tda_sys_disable(void);
static void initialize_ct_lib();

void *p_nfc_ct_one_bin_handle = NULL;
fp_ct_init_ext_t fp_ct_init_ext = NULL;
fp_ct_de_init_ext_t fp_ct_de_init_ext = NULL;
fp_ct_process_nfc_mode_rsp_t fp_ct_process_nfc_mode_rsp = NULL;
fp_ct_open_t fp_ct_open = NULL;
fp_ct_close_t fp_ct_close = NULL;
fp_is_ct_send_app_data_t fp_is_ct_send_app_data = NULL;
fp_transceive_t fp_transceive = NULL;
fp_ct_discover_tda_t fp_ct_discover_tda = NULL;
fp_is_ct_data_credit_received_t fp_is_ct_data_credit_received = NULL;
fp_is_ct_data_rsp_t fp_is_ct_data_rsp = NULL;
extern fp_process_tda_rsp_ntf_t fp_process_tda_rsp_ntf;
extern tda_control_t g_tda_ctrl;

/*****************************************************************************
** Constants and types
*****************************************************************************/
static const tNFA_SYS_REG nfa_tda_sys_reg = {
    nfa_tda_sys_enable, nfa_tda_handle_event, nfa_tda_sys_disable, NULL};

/* NFA_TDA actions */
const tNFA_TDA_ACTION nfa_tda_action_tbl[] = {
    nfa_open_tda_handle_req,       /* NFA_OPEN_TDA_REQUEST_EVT       */
    nfa_close_tda_handle_req,      /* NFA_CLOSE_TDA_REQUEST_EVT      */
    nfa_transceive_tda_handle_req, /* NFA_TRANSCEIVE_TDA_REQUEST_EVT */
    nfa_discover_tda_handle_req,   /* NFA_DISCOVER_TDA_REQUEST_EVT   */
};

/*******************************************************************************
**
** Function         nfa_tda_init
**
** Description      Initialize and register NFA TDA sub-system
**
** Returns          None
**
*******************************************************************************/
void nfa_tda_init(void) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);
  /* register message handler on NFA SYS */
  nfa_sys_register(NFA_ID_TDA, &nfa_tda_sys_reg);
}

/*******************************************************************************
**
** Function         nfa_tda_evt_2_str
**
** Description      convert nfa_tda evt to string
**
*******************************************************************************/
static std::string nfa_tda_evt_2_str(uint16_t event) {
  switch (event) {
  case NFA_OPEN_TDA_REQUEST_EVT:
    return "NFA_OPEN_TDA_REQUEST_EVT";
  case NFA_CLOSE_TDA_REQUEST_EVT:
    return "NFA_CLOSE_TDA_REQUEST_EVT";
  case NFA_TRANSCEIVE_TDA_REQUEST_EVT:
    return "NFA_TRANSCEIVE_TDA_REQUEST_EVT";
  case NFA_DISCOVER_TDA_REQUEST_EVT:
    return "NFA_DISCOVER_TDA_REQUEST_EVT";
  default:
    break;
  }
  return "Unknown";
}

/*******************************************************************************
**
** Function         ct_init
**
** Description      Initialize the CT Lib
**
** Returns          None
**
*******************************************************************************/
static void *ct_init(void *x) {
  (void)x;
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Enter", __func__);
  if (fp_ct_init_ext != NULL) {
    NFC_STATUS status = fp_ct_init_ext();
    if (status != NFC_STATUS_SUCCESS) {
      DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Failed", __func__);
    }
  }
  return NULL;
}

/*******************************************************************************
**
** Function         ct_deinit
**
** Description      Deinitialize the CT Lib
**
** Returns          None
**
*******************************************************************************/
static void *ct_deinit(void *x) {
  (void)x;
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Enter", __func__);
  if (fp_ct_de_init_ext != NULL) {
    NFC_STATUS status = fp_ct_de_init_ext();
    if (status != NFC_STATUS_SUCCESS) {
      DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Failed", __func__);
    }
  }
  return NULL;
}
/*******************************************************************************
**
** Function         nfa_tda_sys_enable
**
** Description      Enable NFA TDA sub-system and Initialize the CT Lib
**
** Returns          None
**
*******************************************************************************/
void nfa_tda_sys_enable(void) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);
  initialize_ct_lib();
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, ct_init, NULL);
}

/*******************************************************************************
**
** Function         initialize_ct_lib
**
** Description      Initialize the dynamic CT Lib and get the function link of
**                  If CT library pluged in.
**
** Returns          None
**
*******************************************************************************/
void initialize_ct_lib() {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);

  p_nfc_ct_one_bin_handle = dlopen("/system/lib64/nfc_tda.so", RTLD_NOW);
  if (p_nfc_ct_one_bin_handle == NULL) {
    LOG(ERROR) << StringPrintf("Error : opening (/system/lib64/nfc_tda.so) !!");
    return;
  }

  if ((fp_ct_init_ext = (fp_ct_init_ext_t)dlsym(p_nfc_ct_one_bin_handle,
                                                "ct_init_ext")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (ct_init_ext) !!");
  }
  if ((fp_ct_de_init_ext = (fp_ct_de_init_ext_t)dlsym(
           p_nfc_ct_one_bin_handle, "ct_de_init_ext")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (ct_de_init_ext) !!");
  }
  if ((fp_ct_discover_tda = (fp_ct_discover_tda_t)dlsym(
           p_nfc_ct_one_bin_handle, "ct_discover_tda")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (ct_discover_tda) !!");
  }

  if ((fp_ct_process_nfc_mode_rsp = (fp_ct_process_nfc_mode_rsp_t)dlsym(
           p_nfc_ct_one_bin_handle, "process_tda_rsp_ntf")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (process_tda_rsp_ntf) !!");
  }
  if ((fp_ct_open = (fp_ct_open_t)dlsym(p_nfc_ct_one_bin_handle, "ct_open")) ==
      NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (ct_open) !!");
  }
  if ((fp_ct_close =
           (fp_ct_close_t)dlsym(p_nfc_ct_one_bin_handle, "ct_close")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (ct_close) !!");
  }
  if ((fp_is_ct_data_credit_received = (fp_is_ct_data_credit_received_t)dlsym(
           p_nfc_ct_one_bin_handle, "is_ct_data_credit_received")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (is_ct_data_credit_received) !!");
  }
  if ((fp_is_ct_send_app_data = (fp_is_ct_send_app_data_t)dlsym(
           p_nfc_ct_one_bin_handle, "is_ct_send_app_data")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (is_ct_send_app_data) !!");
  }

  if ((fp_transceive = (fp_transceive_t)dlsym(p_nfc_ct_one_bin_handle,
                                              "ct_transceive")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (transceive) !!");
  }

  if ((fp_is_ct_data_rsp = (fp_is_ct_data_rsp_t)dlsym(
           p_nfc_ct_one_bin_handle, "is_ct_data_rsp")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (is_ct_data_rsp) !!");
  }

  if ((fp_process_tda_rsp_ntf = (fp_process_tda_rsp_ntf_t)dlsym(
           p_nfc_ct_one_bin_handle, "process_tda_rsp_ntf")) == NULL) {
    LOG(ERROR) << StringPrintf("Error while linking (process_tda_rsp_ntf) !!");
  }
}

/*******************************************************************************
**
** Function         nfa_tda_handle_event
**
** Description      nfa tda main event handling function.
**
** Returns          true if caller should free p_msg buffer
**
*******************************************************************************/
bool nfa_tda_handle_event(NFC_HDR *p_msg) {
  uint16_t act_idx;

  DLOG_IF(INFO, nfc_debug_enabled)
      << StringPrintf("nfa_tda_handle_event event: %s (0x%02x)",
                      nfa_tda_evt_2_str(p_msg->event).c_str(), p_msg->event);

  /* Get NFA_TDA sub-event */
  if ((act_idx = (p_msg->event & 0x00FF)) < (NFA_TDA_MAX_EVT & 0xFF)) {
    return (*nfa_tda_action_tbl[act_idx])((tNFA_TDA_MSG *)p_msg);
  } else {
    DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf(
        "nfa_tda_handle_event: unhandled event 0x%02X", p_msg->event);
    return true;
  }
}

/*******************************************************************************
**
** Function         nfa_tda_sys_disable
**
** Description      Clean up tda sub-system
**
**
** Returns          void
**
*******************************************************************************/
void nfa_tda_sys_disable(void) {
  nfa_sys_deregister(NFA_ID_TDA);

  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);
  pthread_t thread_id;
  pthread_create(&thread_id, NULL, ct_deinit, NULL);
}
#endif
