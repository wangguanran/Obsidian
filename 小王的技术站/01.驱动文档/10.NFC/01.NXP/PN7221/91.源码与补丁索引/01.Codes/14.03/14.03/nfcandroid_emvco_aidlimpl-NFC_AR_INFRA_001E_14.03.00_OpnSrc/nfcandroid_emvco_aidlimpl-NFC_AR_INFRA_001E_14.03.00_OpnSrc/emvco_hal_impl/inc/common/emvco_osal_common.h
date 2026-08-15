/******************************************************************************
 * Copyright 2010-2021,2022-2023 NXP
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
 ******************************************************************************/

/*
 *  OSAL header files related to memory, debug, random, semaphore and mutex
 * functions.
 */

#ifndef _EMVCO_OSAL_COMMON_H_
#define _EMVCO_OSAL_COMMON_H_

/*
************************* Include Files ****************************************
*/

#include <emvco_comp_id.h>
#include <emvco_status.h>
#include <emvco_types.h>
#include <osal_timer.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef FW_LIB_ROOT_DIR
#if (defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64))
#define FW_LIB_ROOT_DIR "/vendor/lib64/"
#else
#define FW_LIB_ROOT_DIR "/vendor/lib/"
#endif
#endif

/*
 *  information to configure OSAL
 */
typedef struct osal_emvco_config {
  uint8_t *p_log_file;             /* Log File Name*/
  uintptr_t dw_callback_thread_id; /* Client ID to which message is posted */
} osal_emvco_config_t,
    *posal_emvco_config_t /* Pointer to #osal_emvco_config_t */;

/*
 * Deferred call declaration.
 * This type of API is called from ClientApplication (main thread) to notify
 * specific callback.
 */
typedef void (*osal_emvco_defer_func_pointer)(void *);

/*
 * Deferred message specific info declaration.
 */
typedef struct osal_emvco_defer_call_info {
  osal_emvco_defer_func_pointer pDeferedCall; /* pointer to Deferred callback */
  void *pParam; /* contains timer message specific details*/
} osal_emvco_defer_call_info_t;

/*
 * States in which a OSAL timer exist.
 */
typedef enum {
  eTimerIdle = 0,            /* Indicates Initial state of timer */
  eTimerRunning = 1,         /* Indicate timer state when started */
  eTimerStopped = 2          /* Indicates timer state when stopped */
} osal_emvco_timer_states_t; /* Variable representing State of timer */

/*
 **Timer Handle structure containing details of a timer.
 */
typedef struct osal_emvco_timer_handle {
  uint32_t timer_id;    /* ID of the timer */
  timer_t timer_handle; /* Handle of the timer */
  /* Timer callback function to be invoked */
  p_osal_emvco_timer_callback_t Application_callback;
  void *p_context; /* Parameter to be passed to the callback function */
  osal_emvco_timer_states_t eState; /* Timer states */
  /* Osal Timer message posted on User Thread */
  lib_emvco_message_t tOsalMessage;
  /* Deferred Call structure to Invoke Callback function */
  osal_emvco_defer_call_info_t tDeferedCallInfo;
  /* Variables for Structure Instance and Structure Ptr */
} osal_emvco_timer_handle_t, *posal_emvco_timer_handle_t;

#endif /*  _EMVCO_OSAL_COMMON_H_  */
