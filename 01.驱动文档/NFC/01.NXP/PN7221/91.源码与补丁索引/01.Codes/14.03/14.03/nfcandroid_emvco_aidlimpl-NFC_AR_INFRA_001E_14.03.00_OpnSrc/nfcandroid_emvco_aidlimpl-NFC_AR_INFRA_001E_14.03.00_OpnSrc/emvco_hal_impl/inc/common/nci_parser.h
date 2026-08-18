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

#ifndef _NCI_PARSER_H_
#define _NCI_PARSER_H_

#define NCI_MT_MASK 0xE0
#define NCI_MT_SHIFT 5

/* PBF: Packet Boundary Flag (byte 0) */
#define NCI_PBF_MASK 0x10
#define NCI_PBF_SHIFT 4

/* GID: Group Identifier (byte 0) */
#define NCI_GID_MASK 0x0F
#define NCI_GID_CORE 0x00      /* 0000b NCI Core group */
#define NCI_GID_RF_MANAGE 0x01 /* 0001b RF Management group */
#define NCI_GID_EE_MANAGE 0x02 /* 0010b NFCEE Management group */
#define NCI_GID_PROP 0x0F      /* 1111b Proprietary */
/* 0111b - 1110b RFU */

/* OID: Opcode Identifier (byte 1) */
#define NCI_OID_MASK 0x3F

#define NCI_BYTE_0_MASK 0x0F
#define NCI_BYTE_1_MASK 0x1F
#define NCI_BYTE_2_MASK 0x2F
#define NCI_BYTE_3_MASK 0x3F

#define NCI_MSG_CMD_ABS_VAL 0x20

#define NCI_MSG_TYPE_DATA 0
#define NCI_MSG_TYPE_CMD 1 /* (NCI_MSG_TYPE_CMD << NCI_MT_SHIFT) = 0x20 */
#define NCI_MSG_TYPE_RSP 2 /* (NCI_MSG_TYPE_RSP << NCI_MT_SHIFT) = 0x40 */
#define NCI_MSG_TYPE_NTF 3 /* (NCI_MSG_TYPE_NTF << NCI_MT_SHIFT) = 0x60 */

/* parse byte0 of NCI packet */
#define NCI_MSG_PRS_HDR0(p, mt, pbf, gid)                                      \
  mt = (*(p)&NCI_MT_MASK) >> NCI_MT_SHIFT;                                     \
  (pbf) = (*(p)&NCI_PBF_MASK) >> NCI_PBF_SHIFT;                                \
  (gid) = *(p)++ & NCI_GID_MASK;

/* parse byte1 of NCI Cmd/Ntf */
#define NCI_MSG_PRS_HDR1(p, oid)                                               \
  (oid) = (*(p)&NCI_OID_MASK);                                                 \
  (p)++;

#define NCI_MSG_PRS_BYTE(p, oid)                                               \
  (oid) = (*(p)&NCI_OID_MASK);                                                 \
  (p)++;

#define MSG_CORE_PROPRIETARY_RSP 2
#define MSG_RF_DISCOVER_RSP 3
#define RF_DEACTIVATE_NTF 6
#define NCI_MSG_CORE_CONN_CREATE 4
#define NCI_MSG_CORE_CONN_CLOSE 5
#define NCI_MSG_MODE_SET 1
#define MSG_CORE_PROPRIETARY_STANDBY_RSP 8
#endif /* _NCI_PARSER_H_ */
