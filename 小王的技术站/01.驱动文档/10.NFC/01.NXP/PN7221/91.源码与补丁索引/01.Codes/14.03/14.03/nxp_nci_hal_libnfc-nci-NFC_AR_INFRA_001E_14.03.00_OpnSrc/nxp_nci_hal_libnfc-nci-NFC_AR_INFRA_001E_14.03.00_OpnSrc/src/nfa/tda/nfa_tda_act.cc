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
#include "ndef_utils.h"
#include "nfa_dm_int.h"
#include "nfa_mem_co.h"
#include "nfa_tda_api.h"
#include "nfc_status.h"
#include <android-base/stringprintf.h>
#include <base/logging.h>
#include <iomanip>
#include <pthread.h>
#include <string.h>
#include <unordered_map>

using android::base::StringPrintf;
extern bool nfc_debug_enabled;
extern void nfc_ncif_send_cmd(NFC_HDR *p_buf);
extern fp_ct_open_t fp_ct_open;
extern fp_ct_close_t fp_ct_close;
extern fp_transceive_t fp_transceive;
extern fp_ct_discover_tda_t fp_ct_discover_tda;
tNFA_TDA_MSG g_data;
tda_control_t g_tda_ctrl;

static void *discoverTDA(void *x) {
  (void)x;

  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);

  tNFC_STATUS status = NFC_STATUS_SUCCESS;
  if (fp_ct_open != NULL) {
    status = fp_ct_discover_tda(g_data.op_req.getTdaInfo);
  } else {
    status = NFC_STATUS_FEATURE_NOT_SUPPORTED;
  }
  (*(g_data.op_req.pCtNotify))(status);
  return NULL;
}

static void *openTDA(void *x) {
  (void)x;

  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);

  tNFC_STATUS status = NFC_STATUS_SUCCESS;
  if (fp_ct_open != NULL) {
    status = fp_ct_open(g_data.op_req.p_TdaId, g_data.op_req.p_StandBy,
                        g_data.op_req.p_CID);
  } else {
    status = NFC_STATUS_FEATURE_NOT_SUPPORTED;
  }
  (*(g_data.op_req.pCtNotify))(status);
  return NULL;
}

static void *closeTDA(void *x) {
  (void)x;

  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);

  tNFC_STATUS status = NFC_STATUS_SUCCESS;
  if (fp_ct_close != NULL) {
    status = fp_ct_close(g_data.op_req.p_TdaId, g_data.op_req.p_StandBy);
  } else {
    status = NFC_STATUS_FEATURE_NOT_SUPPORTED;
  }
  (*(g_data.op_req.pCtNotify))(status);
  return NULL;
}

static void *transceiveTDA(void *x) {
  (void)x;

  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s", __func__);

  tNFC_STATUS status = NFC_STATUS_SUCCESS;
  if (fp_transceive != NULL) {
    status = fp_transceive(g_data.op_req.pCmd_apdu, g_data.op_req.pRsp_apdu);
  } else {
    status = NFC_STATUS_FEATURE_NOT_SUPPORTED;
  }
  (*(g_data.op_req.pCtNotify))(status);
  return NULL;
}

bool nfa_open_tda_handle_req(tNFA_TDA_MSG *p_data) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Enter", __func__);
  pthread_t thread_id;
  g_data.op_req.pCtNotify = p_data->op_req.pCtNotify;
  g_data.op_req.p_StandBy = p_data->op_req.p_StandBy;
  g_data.op_req.p_CID = p_data->op_req.p_CID;
  g_data.op_req.p_TdaId = p_data->op_req.p_TdaId;
  pthread_create(&thread_id, NULL, openTDA, NULL);
  return true;
}

bool nfa_close_tda_handle_req(tNFA_TDA_MSG *p_data) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Enter", __func__);
  pthread_t thread_id;
  g_data.op_req.pCtNotify = p_data->op_req.pCtNotify;
  g_data.op_req.p_StandBy = p_data->op_req.p_StandBy;
  g_data.op_req.p_TdaId = p_data->op_req.p_TdaId;
  pthread_create(&thread_id, NULL, closeTDA, NULL);
  return true;
}

bool nfa_transceive_tda_handle_req(tNFA_TDA_MSG *p_data) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Enter", __func__);
  pthread_t thread_id;
  g_data.op_req.pCtNotify = p_data->op_req.pCtNotify;
  g_data.op_req.pCmd_apdu = p_data->op_req.pCmd_apdu;
  g_data.op_req.pRsp_apdu = p_data->op_req.pRsp_apdu;
  pthread_create(&thread_id, NULL, transceiveTDA, NULL);
  return true;
}

bool nfa_discover_tda_handle_req(tNFA_TDA_MSG *p_data) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Enter", __func__);
  pthread_t thread_id;
  g_data.op_req.pCtNotify = p_data->op_req.pCtNotify;
  g_data.op_req.getTdaInfo = p_data->op_req.getTdaInfo;
  pthread_create(&thread_id, NULL, discoverTDA, NULL);
  return true;
}
#endif
