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

#include "osal_thread.h"
#include <emvco_log.h>
#include <semaphore.h>

EMVCO_STATUS osal_thread_create(pthread_t *thread, const pthread_attr_t *attr,
                                void *(*start_routine)(void *), void *arg) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return pthread_create(thread, attr, start_routine, arg);
}

EMVCO_STATUS osal_mutex_lock(void *hMutex) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return pthread_mutex_lock(hMutex);
}
EMVCO_STATUS osal_mutex_unlock(void *hMutex) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return pthread_mutex_unlock(hMutex);
}
EMVCO_STATUS osal_mutex_init(void *hMutex) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return pthread_mutex_init(hMutex, NULL);
}
EMVCO_STATUS osal_mutex_destroy(void *hMutex) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return pthread_mutex_destroy(hMutex);
}
EMVCO_STATUS osal_thread_join(pthread_t thread, void *retval) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return pthread_join(thread, retval);
}

int osal_sem_wait(sem_t *sem) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return sem_wait(sem);
}

int osal_sem_timedwait(sem_t *sem, const struct timespec *abs_timeout) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return sem_timedwait(sem, abs_timeout);
}

int osal_sem_post(sem_t *sem) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return sem_post(sem);
}

int osal_sem_destroy(sem_t *sem) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return sem_destroy(sem);
}

int osal_sem_init(sem_t *sem, int pshared, unsigned int value) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return sem_init(sem, pshared, value);
}

int osal_sem_getvalue(sem_t *sem, int *sval) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return sem_getvalue(sem, sval);
}

int osal_sem_timedwait_monotonic_np(sem_t *__sem, const struct timespec *__ts) {
  LOG_EMVCO_TML_D("%s\n", __func__);
  return sem_timedwait_monotonic_np(__sem, __ts);
}