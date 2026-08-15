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

#ifndef _NFC_COMMON_H_
#define _NFC_COMMON_H_

#include "nfc_status.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NCI_OID_SHIFT 0
#define NCI_MT_SHIFT_VAL 5

/* builds byte0 of NCI Command and Notification packet */
#define NCI_MSG_BLD_HDR0_TDA(p, mt, gid)                                       \
  *(p)++ = (uint8_t)(((mt) << NCI_MT_SHIFT_VAL) | (gid));

/* builds byte1 of NCI Command and Notification packet */
#define NCI_MSG_BLD_HDR1(p, oid) *(p)++ = (uint8_t)(((oid) << NCI_OID_SHIFT));

#define UINT8_TO_STREAM(p, u8)                                                 \
  { *(p)++ = (uint8_t)(u8); }

#define ARRAY_TO_STREAM(p, a, len)                                             \
  {                                                                            \
    int ijk;                                                                   \
    for (ijk = 0; ijk < (len); ijk++)                                          \
      *(p)++ = (uint8_t)(a)[ijk];                                              \
  }
#define TDA_STREAM_TO_ARRAY(a, p, len)                                         \
  {                                                                            \
    int ijk;                                                                   \
    for (ijk = 0; ijk < (len); ijk++)                                          \
      ((uint8_t *)(a))[ijk] = *(p)++;                                          \
  }

enum NfcStatus {
  NFC_STATUS__OK = 0,
  NFC_STATUS_INVALID_PARAMETER = 1,
  NFC_STATUS_INVALID_DEVICE = 2,
  NFC_STATUS_PENDING = 3,
  NFC_STATUS_NOT_INITIALISED = 4,
  NFC_STATUS_ALREADY_INITIALISED = 5,
  NFC_STATUS_FEATURE_NOT_SUPPORTED = 6,
  NFC_STATUS_BUSY_ = 7,
  NFC_STATUS_WRITE_FAILED = 8,
  NFC_STATUS_TDA_INIT_FAILED = 9,
  NFC_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED = 10,
  NFC_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY = 11,
  NFC_STATUS_CORE_CONN_CREATE_FAILED = 12,
  NFC_STATUS_CORE_CONN_CREATED_ALREADY = 13,
  NFC_STATUS_NFCEE_MODE_SET_ENABLE_FAILED = 14,
  NFC_STATUS_NFCEE_MODE_SET_ENABLE_TIMEOUT = 15,
  NFC_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED = 16,
  NFC_STATUS_INVALID_STATE_CORE_CONN_CREATE_NOT_COMPLETED = 17,
  NFC_STATUS_INVALID_STATE_TDA_OPENED_ALREADY = 18,
  NFC_STATUS_INVALID_STATE_CORE_CONN_CREATED_ALREADY = 19,
  NFC_STATUS_TRANSCEIVE_FAILED = 20,
  NFC_STATUS_CORE_CONN_CLOSE_FAILED = 21,
  NFC_STATUS_NFCEE_MODE_SET_DISABLE_FAILED = 22,
  NFC_STATUS_NFCEE_INTERFACE_ACTIVATION_FAILED = 23,
  NFC_STATUS_NFCEE_TRANSMISSION_ERROR = 24,
  NFC_STATUS_INVALID_STATE_TDA_IN_CLOSED = 25,
  NFC_STATUS_INVALID_STATE_CORE_CONN_CLOSED_ALREADY = 26,
  NFC_STATUS_INVALID_STATE_TDA_CLOSED_ALREADY = 27,
  NFC_STATUS_NFCEE_PROTOCOL_ERROR = 28,
  NFC_STATUS_NFCEE_TIMEOUT_ERROR = 29,
  NFC_STATUS_NCI_RESPONSE_ERR = 30,
  NFC_STATUS_FAIL = 255,
};

#ifdef __cplusplus
}
#endif /*  C++ Compilation guard */
#endif /* _NFC_COMMON_H_ */
