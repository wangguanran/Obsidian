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
#ifndef _EMVCO_UTILS_H_
#define _EMVCO_UTILS_H_

#include "osal_thread.h"
#include <assert.h>
#include <emvco_status.h>
#include <pthread.h>
#include <semaphore.h>
/********************* Definitions and structures *****************************/

/* List structures */
struct list_node {
  void *p_data;
  struct list_node *p_next;
};

struct list_head {
  struct list_node *p_first;
  pthread_mutex_t mutex;
};

/* Semaphore handling structure */
typedef struct nci_hal_sem_t {
  /* Semaphore used to wait for callback */
  sem_t sem;

  /* Used to store the status sent by the callback */
  EMVCO_STATUS status;

  /* Used to provide a local context to the callback */
  void *p_context;

} nci_hal_sem;

/* Semaphore helper macros */
#define SEM_WAIT(cb_data) osal_sem_wait(&((cb_data).sem))
#define SEM_POST(p_cb_data) osal_sem_post(&((p_cb_data)->sem))

/* Semaphore and mutex monitor */
typedef struct phNxpNciHal_Monitor {
  /* Mutex protecting native library against reentrance */
  pthread_mutex_t reentrance_mutex;

  /* Mutex protecting native library against concurrency */
  pthread_mutex_t concurrency_mutex;

  /* List used to track pending semaphores waiting for callback */
  struct list_head sem_list;

} phNxpNciHal_Monitor_t;

/************************ Exposed functions ***********************************/
/* List functions */
int listInit(struct list_head *pList);
int listDestroy(struct list_head *pList);
int listAdd(struct list_head *pList, void *p_data);
int listRemove(struct list_head *pList, void *p_data);
int listGetAndRemoveNext(struct list_head *pList, void **pp_data);
void listDump(struct list_head *pList);

/* NXP NCI HAL utility functions */
phNxpNciHal_Monitor_t *init_monitor(void);
void cleanup_monitor(void);
phNxpNciHal_Monitor_t *get_monitor(void);
EMVCO_STATUS init_cb_data(nci_hal_sem *pCallbackData, void *p_context);
void cleanup_cb_data(nci_hal_sem *pCallbackData);
void releaseall_cb_data(void);
void print_packet(const char *pString, const uint8_t *p_data, uint16_t len);
void emergency_recovery(void);

/* Lock unlock helper macros */
/* Lock unlock helper macros */
#define REENTRANCE_LOCK()                                                      \
  if (get_monitor())                                                           \
  pthread_mutex_lock(&get_monitor()->reentrance_mutex)
#define REENTRANCE_UNLOCK()                                                    \
  if (get_monitor())                                                           \
  pthread_mutex_unlock(&get_monitor()->reentrance_mutex)
#define CONCURRENCY_LOCK()                                                     \
  if (get_monitor())                                                           \
  pthread_mutex_lock(&get_monitor()->concurrency_mutex)
#define CONCURRENCY_UNLOCK()                                                   \
  if (get_monitor())                                                           \
  pthread_mutex_unlock(&get_monitor()->concurrency_mutex)

#endif /* _EMVCO_UTILS_H_ */
