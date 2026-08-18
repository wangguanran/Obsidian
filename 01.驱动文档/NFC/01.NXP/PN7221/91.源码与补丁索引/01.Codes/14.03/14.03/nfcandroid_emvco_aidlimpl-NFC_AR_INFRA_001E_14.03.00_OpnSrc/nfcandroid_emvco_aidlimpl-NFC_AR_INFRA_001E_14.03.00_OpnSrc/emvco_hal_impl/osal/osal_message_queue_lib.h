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

#ifndef _OSAL_MESSAGE_QUEUE_LIB_H_
#define _OSAL_MESSAGE_QUEUE_LIB_H_
/** \addtogroup EMVCO_STACK_OSAL_API_INTERFACE
 *  @brief      Interface for message queue, log, dynamic memory management,
 * thread, timer and transport mapping layer implementation in Android
 */
#include <emvco_types.h>
#include <linux/ipc.h>

/**
 *
 * @brief            Allocates message queue
 *
 * @param[in]       Ignored, included only for Linux queue API compatibility
 *
 * @return           (int) value of pQueue if successful
 *                  -1, if failed to allocate memory or to init mutex
 *
 */
intptr_t osal_msg_get(key_t key, int msgflg);

/**
 *
 * @brief       Releases message queue
 *
 * @param[in]        msqid - message queue handle
 *
 * @return           None
 *
 */
void osal_msg_release(intptr_t msqid);

/**
 *
 * @brief      Destroys message queue
 *
 * @param[in]       msqid - message queue handle
 *                  cmd, buf - ignored, included only for Linux queue API
 *                  compatibility
 *
 * @return          0,  if successful
 *                  -1, if invalid handle is passed
 *
 */
int osal_msg_ctl(intptr_t msqid, int cmd, void *buf);

/**
 *
 * @brief      Sends a message to the queue. The message will be added at
 *                  the end of the queue as appropriate for FIFO policy
 *
 * @param[in]       msqid  - message queue handle
 *                  msgp   - message to be sent
 *                  msgsz  - message size
 *                  msgflg - ignored
 *
 * @return          0,  if successful
 *                  -1, if invalid parameter passed or failed to allocate memory
 *
 */
intptr_t osal_msg_snd(intptr_t msqid, lib_emvco_message_t *msg, int msgflg);

/**
 *
 * @brief       Gets the oldest message from the queue.
 *                  If the queue is empty the function waits (blocks on a mutex)
 *                  until a message is posted to the queue with
 * phDal4EMVCo_msgsnd
 *
 * @param[in]        msqid  - message queue handle
 *                  msgp   - message to be received
 *                  msgsz  - message size
 *                  msgtyp - ignored
 *                  msgflg - ignored
 *
 * @return           0,  if successful
 *                  -1, if invalid parameter passed
 *
 */
int osal_msg_rcv(intptr_t msqid, lib_emvco_message_t *msg, long msgtyp,
                 int msgflg);

/** @}*/
#endif /*  _OSAL_MESSAGE_QUEUE_LIB_H_  */
