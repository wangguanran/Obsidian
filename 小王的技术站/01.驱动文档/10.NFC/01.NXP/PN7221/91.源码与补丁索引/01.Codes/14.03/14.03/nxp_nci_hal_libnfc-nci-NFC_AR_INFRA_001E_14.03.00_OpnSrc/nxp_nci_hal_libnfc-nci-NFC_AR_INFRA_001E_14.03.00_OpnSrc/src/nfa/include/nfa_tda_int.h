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
#ifndef _TDA_INIT_H_
#define _TDA_INIT_H_

#include "nfa_sys.h"
#include "nfc_tda.h"
#include <android-base/stringprintf.h>
#include <base/logging.h>
using namespace std;
using android::base::StringPrintf;
extern bool nfc_debug_enabled;

/* Deactivate the connected NFCEE */
#define NCI_NFCEE_MD_DEACTIVATE 0x00
/* Activate the connected NFCEE */
#define NCI_NFCEE_MD_ACTIVATE 0x01

typedef void(fp_ct_lib_evt_cb_t)(uint8_t status);

enum {
  NFA_OPEN_TDA_REQUEST_EVT = NFA_SYS_EVT_START(NFA_ID_TDA),
  NFA_CLOSE_TDA_REQUEST_EVT,
  NFA_TRANSCEIVE_TDA_REQUEST_EVT,
  NFA_DISCOVER_TDA_REQUEST_EVT,
  NFA_TDA_MAX_EVT
};

typedef struct {
  NFC_HDR hdr;
  int8_t p_TdaId;
  int8_t *p_CID;
  bool p_StandBy;
  tda_data *pCmd_apdu;
  tda_data *pRsp_apdu;
  tda_control_t *getTdaInfo;
  fp_ct_lib_evt_cb_t *pCtNotify;
} tNFA_TDA_OPERATION;

/* union of all data types */
typedef union {
  /* GKI event buffer header */
  NFC_HDR hdr;
  tNFA_TDA_OPERATION op_req;
} tNFA_TDA_MSG;

/* type definition for action functions */
typedef bool (*tNFA_TDA_ACTION)(tNFA_TDA_MSG *p_data);
typedef NFC_STATUS (*fp_ct_init_ext_t)(void);
typedef NFC_STATUS (*fp_ct_de_init_ext_t)(void);
typedef NFC_STATUS (*fp_ct_open_t)(int8_t, bool, int8_t *);
typedef NFC_STATUS (*fp_ct_close_t)(int8_t, bool);
typedef NFC_STATUS (*fp_is_ct_send_app_data_t)(const uint8_t *, uint16_t, bool);
typedef NFC_STATUS (*fp_transceive_t)(tda_data *, tda_data *);
typedef NFC_STATUS (*fp_ct_discover_tda_t)(tda_control_t *);
typedef bool (*fp_is_ct_data_credit_received_t)(uint8_t *, uint16_t);
typedef bool (*fp_is_ct_data_rsp_t)(uint8_t *, uint16_t);
typedef void (*fp_ct_process_nfc_mode_rsp_t)(uint8_t *, uint16_t);
typedef NFC_STATUS (*fp_process_tda_rsp_ntf_t)(uint8_t *, uint16_t);

bool nfa_tda_handle_event(NFC_HDR *p_msg);
bool nfa_open_tda_handle_req(tNFA_TDA_MSG *p_data);
bool nfa_close_tda_handle_req(tNFA_TDA_MSG *p_data);
bool nfa_transceive_tda_handle_req(tNFA_TDA_MSG *p_data);
bool nfa_discover_tda_handle_req(tNFA_TDA_MSG *p_data);
#endif
