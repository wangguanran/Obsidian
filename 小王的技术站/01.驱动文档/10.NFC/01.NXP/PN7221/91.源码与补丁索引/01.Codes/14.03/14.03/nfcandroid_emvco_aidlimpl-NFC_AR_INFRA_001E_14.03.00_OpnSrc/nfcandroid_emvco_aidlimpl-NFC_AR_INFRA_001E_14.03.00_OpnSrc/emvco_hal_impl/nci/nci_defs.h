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
/******************************************************************************
 *
 *  This file contains the definition from NCI specification
 *
 ******************************************************************************/

#ifndef NCI_DEFS_H
#define NCI_DEFS_H

#include <nci_parser.h>
#include <stdint.h>

/* Define the message header size for all NCI Commands and Notifications.
 */
#define NCI_MSG_HDR_SIZE 3  /* per NCI spec */
#define NCI_DATA_HDR_SIZE 3 /* per NCI spec */

#define NCI_MT_SHIFT 5

#define MT_CMD 1 /* (MT_CMD << NCI_MT_SHIFT) = 0x20 */
#define MT_RSP 2 /* (MT_RSP << NCI_MT_SHIFT) = 0x40 */
#define MT_NTF 3 /* (MT_NTF << NCI_MT_SHIFT) = 0x60 */

/* Status Codes */
#define NCI_STATUS_OK 0x00
#define NCI_STATUS_FAILED 0x03

/* GID: Group Identifier (byte 0) */
#define NCI_GID_MASK 0x0F
#define NCI_GID_CORE 0x00      /* 0000b NCI Core group */
#define NCI_GID_RF_MANAGE 0x01 /* 0001b RF Management group */
#define NCI_GID_EE_MANAGE 0x02 /* 0010b NFCEE Management group */
#define NCI_GID_PROP 0x0F      /* 1111b Proprietary */
/* 0111b - 1110b RFU */

/* OID: Opcode Identifier (byte 1) */
#define NCI_OID_MASK 0x3F
#define NCI_OID_SHIFT 0

/**********************************************
 * RF MANAGEMENT Group Opcode    - 1
 **********************************************/
#define NCI_MSG_RF_DISCOVER_MAP 0
#define NCI_MSG_RF_SET_ROUTING 1
#define NCI_MSG_RF_GET_ROUTING 2
#define NCI_MSG_RF_DISCOVER 3
#define NCI_MSG_RF_DISCOVER_SELECT 4
#define NCI_MSG_RF_INTF_ACTIVATED 5
#define NCI_MSG_RF_DEACTIVATE 6
#define NCI_MSG_RF_FIELD 7
#define NCI_DISCOVER_PARAM_SIZE_DEACT 0x01
#define NCI_MAX_DATA_LEN 300
/* builds byte0 of NCI Command and Notification packet */
#define NCI_MSG_BLD_HDR0(p, mt, gid)                                           \
  *(p)++ = (uint8_t)(((mt) << NCI_MT_SHIFT) | (gid));

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
#define STREAM_TO_ARRAY(a, p, len)                                             \
  {                                                                            \
    int ijk;                                                                   \
    for (ijk = 0; ijk < (len); ijk++)                                          \
      ((uint8_t *)(a))[ijk] = *(p)++;                                          \
  }
/**********************************************
 * NCI Core Group Opcode        - 0
 **********************************************/
#define MSG_CORE_RESET 0
#define MSG_CORE_INIT 1
#define MSG_CORE_SET_CONFIG 2
#define MSG_CORE_GET_CONFIG 3

/* Define the message header size for all NCI Commands and Notifications.
 */
#define NCI_MSG_HDR_SIZE 3 /* per NCI spec */

/**********************************************
 * NCI Core Group Params
 **********************************************/
#define NCI_CORE_PARAM_SIZE_RESET 0x01

#define NCI_VERSION_UNKNOWN 0x00
#define NCI_VERSION_1_0 0x10
#define NCI_VERSION_2_0 0x20
#define NCI_CORE_PARAM_SIZE_INIT(X) (((X) == NCI_VERSION_2_0) ? (0x02) : (0x00))
#define NCI2_0_CORE_INIT_CMD_BYTE_0 0x00
#define NCI2_0_CORE_INIT_CMD_BYTE_1 0x00

#endif /* NCI_DEFS_H */