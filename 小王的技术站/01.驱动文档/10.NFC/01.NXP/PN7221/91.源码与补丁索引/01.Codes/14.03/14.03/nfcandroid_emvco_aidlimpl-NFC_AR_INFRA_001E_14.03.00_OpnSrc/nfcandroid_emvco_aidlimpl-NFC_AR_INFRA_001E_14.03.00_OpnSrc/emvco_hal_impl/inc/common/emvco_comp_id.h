/******************************************************************************
 * Copyright 2022-2023 NXP
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

/*
 * EMVCO Component ID Values - Used for Function Return Codes
 */

#ifndef _EMVCO_COMP_ID_H_
#define _EMVCO_COMP_ID_H_

/*
 *  Component IDs
 *
 *  IDs for all EMVCO components. Combined with the Status Code they build the
 * value (status)
 *  returned by each function.
 *
 *  ID Number Spaces:
 *  - 01..1F: HAL
 *  - 20..3F: EMVCO-MW (Local Device)
 *  - 40..5F: EMVCO-MW (Remote Device)
 *  .
 *
 *         The value CID_EMVCO_NONE does not exist for Component IDs. Do not use
 * this value except
 *         for EMVCO_STATUS_SUCCESS. The enumeration function uses
 * CID_EMVCO_NONE to mark unassigned "References".
 */
/* Unassigned or doesn't apply (see #EMVCO_STATUS_SUCCESS) */
#define CID_EMVCO_NONE 0x00
#define CID_EMVCO_TML 0x01 /* Transport Mapping Layer */
#define CID_EMVCO_LLC 0x07 /* Logical Link Control Layer */
/* EMVCO Controller Interface Layer */
#define CID_EMVCO_NCI 0x08
/* Firmware Download Management Layer */
#define CID_EMVCO_DNLD 0x09
#define CID_EMVCO_HAL 0x10 /* Hardware Abstraction Layer */
/* Operating System Abstraction Layer*/
#define CID_EMVCO_OSAL CID_EMVCO_NONE
#define CID_FRI_EMVCO_OVR_HAL 0x20       /* EMVCO-Device, HAL-based */
#define CID_FRI_EMVCO_NDEF_RECORD 0x22   /* NDEF Record Tools Library. */
#define CID_FRI_EMVCO_NDEF_MAP 0x23      /* NDEF Mapping. */
#define CID_FRI_EMVCO_NDEF_REGISTRY 0x24 /* NDEF_REGISTRY. */
/* Automatic Device Discovery. */
#define CID_FRI_EMVCO_AUTO_DEV_DIS 0x25
#define CID_FRI_EMVCO_NDEF_SMTCRDFMT 0x26 /* Smart Card Formatting */
#define CID_EMVCO_LIB 0x30                /* EMVCO Library Layer*/
/* The maximum CID value that is defined. */
#define CID_MAX_VALUE 0xF0
/* Logical Link Control Protocol */
#define CID_FRI_EMVCO_LLCP 0x40
#define CID_FRI_EMVCO_LLCP_TRANSPORT 0x50
#define CID_FRI_EMVCO_LLCP_MAC 0x60

#endif /* _EMVCO_COMP_ID_H_ */
