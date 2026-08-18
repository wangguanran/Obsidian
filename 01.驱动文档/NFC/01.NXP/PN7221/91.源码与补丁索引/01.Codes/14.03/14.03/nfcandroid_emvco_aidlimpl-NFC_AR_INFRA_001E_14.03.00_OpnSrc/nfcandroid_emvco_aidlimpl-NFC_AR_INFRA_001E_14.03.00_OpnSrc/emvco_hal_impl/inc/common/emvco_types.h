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

#ifndef _EMVCO_TYPES_H_
#define _EMVCO_TYPES_H_

#include <emvco_common.h>
#include <nci_defs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TRUE
#define TRUE (0x01) /* Logical True Value */
#endif
#ifndef FALSE
#define FALSE (0x00) /* Logical False Value */
#endif

#define STATIC static

#define EMVCO_MAX_UID_LENGTH 0x0AU /* Maximum UID length expected */
/* Maximum ATR_RES (General Bytes) length expected */
//#define EMVCO_MAX_ATR_LENGTH 0x30U
//#define EMVCO_NFCID_LENGTH 0x0AU /* Maximum length of NFCID 1.3*/
//#define EMVCO_ATQA_LENGTH 0x02U  /* ATQA length */

/*
 * Possible Hardware Configuration exposed to upper layer.
 * Typically this should be port name (Ex:"COM1","COM2") to which Controller is
 * connected.
 */
typedef enum {
  ENUM_LINK_TYPE_COM1,
  ENUM_LINK_TYPE_COM2,
  ENUM_LINK_TYPE_COM3,
  ENUM_LINK_TYPE_COM4,
  ENUM_LINK_TYPE_COM5,
  ENUM_LINK_TYPE_COM6,
  ENUM_LINK_TYPE_COM7,
  ENUM_LINK_TYPE_COM8,
  ENUM_LINK_TYPE_I2C,
  ENUM_LINK_TYPE_SPI,
  ENUM_LINK_TYPE_USB,
  ENUM_LINK_TYPE_TCP,
  ENUM_LINK_TYPE_NB
} config_link_type;

/*
 * Deferred message. This message type will be posted to the client application
 * thread
 * to notify that a deferred call must be invoked.
 */
#define EMVCO_DEFERRED_CALL_MSG (0x311)

/*
 * Deferred call declaration.
 * This type of API is called from ClientApplication ( main thread) to notify
 * specific callback.
 */
typedef void (*p_lib_emvco_deferred_callback_t)(void *);

/*
 * Deferred parameter declaration.
 * This type of data is passed as parameter from ClientApplication (main thread)
 * to the
 * callback.
 */
typedef void *p_lib_emvco_deferred_parameter_t;

/*
 * Possible Hardware Configuration exposed to upper layer.
 * Typically this should be at least the communication link (Ex:"COM1","COM2")
 * the controller is connected to.
 */
typedef struct lib_emvco_hw_conf {
  uint8_t *p_log_file; /* Log File Name*/
  /* Hardware communication link to the controller */
  config_link_type n_link_type;
  /* The client ID (thread ID or message queue ID) */
  uintptr_t n_client_id;
} driver_config_data, *pdriver_config_data;

/**
 * @brief EMVCO Message structure contains message specific details like
 *        message type, message specific data block details, etc.
 */
typedef struct lib_emvco_message {
  uint32_t e_msgType; /* Type of the message to be posted*/
  void *p_msg_data;   /* Pointer to message specific data block in case any*/
  uint32_t size;      /* Size of the datablock*/
  uint8_t data[NCI_MAX_DATA_LEN]; /* Message data maintained with MAX*/
  EMVCO_STATUS w_status;          /* Status of the Transaction Completion*/
} lib_emvco_message_t, *plib_emvco_message_t;

/**
 * @brief Deferred message specific info declaration.
 * This type of information is packed as message data when
 * EMVCO_DEFERRED_CALL_MSG
 * type message is posted to message handler thread.
 */
typedef struct lib_emvco_deferred_call {
  p_lib_emvco_deferred_callback_t p_callback; /* pointer to Deferred callback */
  p_lib_emvco_deferred_parameter_t
      p_parameter; /* pointer to Deferred parameter */
} lib_emvco_deferred_call_t;

/*
 *  Enumerated MIFARE Commands
 */

#define UNUSED(X) (void)(X);

#endif /* _EMVCO_TYPES_H_ */
