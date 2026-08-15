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
#include "nfa_tda_api.h"
#include <android-base/stringprintf.h>
#include <base/logging.h>
#include <string.h>

using android::base::StringPrintf;
extern bool nfc_debug_enabled;

void NFA_Discover_tda_slots(tda_control_t *tda_control,
                            fp_ct_lib_evt_cb_t pCtLib_CB) {
  tNFA_TDA_OPERATION *p_msg;
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s : Enter ", __func__);

  if ((p_msg = (tNFA_TDA_OPERATION *)GKI_getbuf(
           (uint16_t)(sizeof(tNFA_TDA_OPERATION)))) != NULL) {
    p_msg->hdr.event = NFA_DISCOVER_TDA_REQUEST_EVT;
    p_msg->getTdaInfo = tda_control;
    p_msg->pCtNotify = pCtLib_CB;
    nfa_sys_sendmsg(p_msg);
  }
  return;
}

void NFA_Open_tda_slot(int8_t tda_id, int8_t *conn_id, bool standBy,
                       fp_ct_lib_evt_cb_t *pCtLib_CB) {
  tNFA_TDA_OPERATION *p_msg;
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s : Enter ", __func__);

  if ((p_msg = (tNFA_TDA_OPERATION *)GKI_getbuf(
           (uint16_t)(sizeof(tNFA_TDA_OPERATION)))) != NULL) {
    p_msg->hdr.event = NFA_OPEN_TDA_REQUEST_EVT;
    p_msg->p_TdaId = tda_id;
    p_msg->p_CID = conn_id;
    p_msg->p_StandBy = standBy;
    p_msg->pCtNotify = pCtLib_CB;
    nfa_sys_sendmsg(p_msg);
  }
  return;
}

void NFA_Close_tda_slot(int8_t tda_id, bool standBy,
                        fp_ct_lib_evt_cb_t pCtLib_CB) {
  tNFA_TDA_OPERATION *p_msg;
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s : Enter ", __func__);

  if ((p_msg = (tNFA_TDA_OPERATION *)GKI_getbuf(
           (uint16_t)(sizeof(tNFA_TDA_OPERATION)))) != NULL) {
    p_msg->hdr.event = NFA_CLOSE_TDA_REQUEST_EVT;
    p_msg->p_TdaId = tda_id;
    p_msg->p_StandBy = standBy;
    p_msg->pCtNotify = pCtLib_CB;
    nfa_sys_sendmsg(p_msg);
  }
  return;
}

void NFA_transceive_tda(tda_data *cmd_apdu, tda_data *rsp_apdu,
                        fp_ct_lib_evt_cb_t pCtLib_CB) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s : Enter ", __func__);
  tNFA_TDA_OPERATION *p_msg;

  if ((p_msg = (tNFA_TDA_OPERATION *)GKI_getbuf(
           (uint16_t)(sizeof(tNFA_TDA_OPERATION)))) != NULL) {
    p_msg->hdr.event = NFA_TRANSCEIVE_TDA_REQUEST_EVT;
    p_msg->pCmd_apdu = cmd_apdu;
    p_msg->pRsp_apdu = rsp_apdu;
    p_msg->pCtNotify = pCtLib_CB;
    nfa_sys_sendmsg(p_msg);
  }
  return;
}
#endif