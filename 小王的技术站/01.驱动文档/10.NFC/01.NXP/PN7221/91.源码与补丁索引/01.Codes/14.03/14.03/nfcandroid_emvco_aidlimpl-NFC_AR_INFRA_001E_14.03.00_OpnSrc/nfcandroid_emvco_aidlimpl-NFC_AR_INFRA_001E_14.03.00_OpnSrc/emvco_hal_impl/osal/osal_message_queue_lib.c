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

/*
 * DAL independent message queue implementation for Android (can be used under
 * Linux too)
 */

#include <emvco_log.h>
#include <emvco_types.h>
#include <errno.h>
#include <linux/ipc.h>
#include <osal_message_queue_lib.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct message_queue_item {
  lib_emvco_message_t n_msg;
  struct message_queue_item *p_prev;
  struct message_queue_item *p_next;
} message_queue_item_t;

typedef struct message_queue {
  message_queue_item_t *p_items;
  pthread_mutex_t n_critical_section_mutex;
  sem_t n_process_semaphore;

} message_queue_t;

/*******************************************************************************
**
** Function         osal_msg_get
**
** Description      Allocates message queue
**
** Parameters       Ignored, included only for Linux queue API compatibility
**
** Returns          (int) value of pQueue if successful
**                  -1, if failed to allocate memory or to init mutex
**
*******************************************************************************/
intptr_t osal_msg_get(key_t key, int msgflg) {
  message_queue_t *pQueue;
  UNUSED(key);
  UNUSED(msgflg);
  pQueue = (message_queue_t *)malloc(sizeof(message_queue_t));
  if (pQueue == NULL)
    return -1;
  memset(pQueue, 0, sizeof(message_queue_t));
  if (pthread_mutex_init(&pQueue->n_critical_section_mutex, NULL) != 0) {
    free(pQueue);
    return -1;
  }
  if (sem_init(&pQueue->n_process_semaphore, 0, 0) == -1) {
    free(pQueue);
    return -1;
  }

  return ((intptr_t)pQueue);
}

/*******************************************************************************
**
** Function         osal_msg_release
**
** Description      Releases message queue
**
** Parameters       msqid - message queue handle
**
** Returns          None
**
*******************************************************************************/
void osal_msg_release(intptr_t msqid) {
  message_queue_t *pQueue = (message_queue_t *)msqid;

  if (pQueue != NULL) {
    sem_post(&pQueue->n_process_semaphore);
    usleep(3000);
    if (sem_destroy(&pQueue->n_process_semaphore)) {
      LOG_EMVCO_TML_E("Failed to destroy semaphore (errno=0x%08x)", errno);
    }
    pthread_mutex_destroy(&pQueue->n_critical_section_mutex);

    free(pQueue);
  }

  return;
}

/*******************************************************************************
**
** Function         osal_msg_ctl
**
** Description      Destroys message queue
**
** Parameters       msqid - message queue handle
**                  cmd, buf - ignored, included only for Linux queue API
**                  compatibility
**
** Returns          0,  if successful
**                  -1, if invalid handle is passed
**
*******************************************************************************/
int osal_msg_ctl(intptr_t msqid, int cmd, void *buf) {
  message_queue_t *pQueue;
  message_queue_item_t *p;
  UNUSED(cmd);
  UNUSED(buf);
  if (msqid == 0)
    return -1;

  pQueue = (message_queue_t *)msqid;
  pthread_mutex_lock(&pQueue->n_critical_section_mutex);
  if (pQueue->p_items != NULL) {
    p = pQueue->p_items;
    while (p->p_next != NULL) {
      p = p->p_next;
    }
    while (p->p_prev != NULL) {
      p = p->p_prev;
      free(p->p_next);
      p->p_next = NULL;
    }
    free(p);
  }
  pQueue->p_items = NULL;
  pthread_mutex_unlock(&pQueue->n_critical_section_mutex);
  pthread_mutex_destroy(&pQueue->n_critical_section_mutex);
  free(pQueue);

  return 0;
}

/*******************************************************************************
**
** Function         osal_msg_snd
**
** Description      Sends a message to the queue. The message will be added at
**                  the end of the queue as appropriate for FIFO policy
**
** Parameters       msqid  - message queue handle
**                  msgp   - message to be sent
**                  msgsz  - message size
**                  msgflg - ignored
**
** Returns          0,  if successful
**                  -1, if invalid parameter passed or failed to allocate memory
**
*******************************************************************************/
intptr_t osal_msg_snd(intptr_t msqid, lib_emvco_message_t *msg, int msgflg) {
  message_queue_t *pQueue;
  message_queue_item_t *p;
  message_queue_item_t *pNew;
  UNUSED(msgflg);
  if ((msqid == 0) || (msg == NULL))
    return -1;

  pQueue = (message_queue_t *)msqid;
  pNew = (message_queue_item_t *)malloc(sizeof(message_queue_item_t));
  if (pNew == NULL)
    return -1;
  memset(pNew, 0, sizeof(message_queue_item_t));
  memcpy(&pNew->n_msg, msg, sizeof(lib_emvco_message_t));
  pthread_mutex_lock(&pQueue->n_critical_section_mutex);

  if (pQueue->p_items != NULL) {
    p = pQueue->p_items;
    while (p->p_next != NULL) {
      p = p->p_next;
    }
    p->p_next = pNew;
    pNew->p_prev = p;
  } else {
    pQueue->p_items = pNew;
  }
  pthread_mutex_unlock(&pQueue->n_critical_section_mutex);
  sem_post(&pQueue->n_process_semaphore);

  return 0;
}

/*******************************************************************************
**
** Function         osal_msg_rcv
**
** Description      Gets the oldest message from the queue.
**                  If the queue is empty the function waits (blocks on a mutex)
**                  until a message is posted to the queue with osal_msg_snd
**
** Parameters       msqid  - message queue handle
**                  msgp   - message to be received
**                  msgsz  - message size
**                  msgtyp - ignored
**                  msgflg - ignored
**
** Returns          0,  if successful
**                  -1, if invalid parameter passed
**
*******************************************************************************/
int osal_msg_rcv(intptr_t msqid, lib_emvco_message_t *msg, long msgtyp,
                 int msgflg) {
  message_queue_t *pQueue;
  message_queue_item_t *p;
  UNUSED(msgflg);
  UNUSED(msgtyp);
  if ((msqid == 0) || (msg == NULL))
    return -1;

  pQueue = (message_queue_t *)msqid;

  if (-1 == sem_wait(&pQueue->n_process_semaphore)) {
    LOG_EMVCO_TML_E("sem_wait didn't return success\n");
  }

  pthread_mutex_lock(&pQueue->n_critical_section_mutex);

  if (pQueue->p_items != NULL) {
    memcpy(msg, &(pQueue->p_items)->n_msg, sizeof(lib_emvco_message_t));
    p = pQueue->p_items->p_next;
    free(pQueue->p_items);
    pQueue->p_items = p;
  }
  pthread_mutex_unlock(&pQueue->n_critical_section_mutex);

  return 0;
}
