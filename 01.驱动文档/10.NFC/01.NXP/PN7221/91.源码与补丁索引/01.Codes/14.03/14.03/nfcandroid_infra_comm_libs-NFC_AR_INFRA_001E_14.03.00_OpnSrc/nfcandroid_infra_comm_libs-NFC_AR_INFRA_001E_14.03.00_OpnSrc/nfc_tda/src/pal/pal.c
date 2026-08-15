/******************************************************************************
 *
 *  Copyright 2023 NXP
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

#include "pal.h"
#include "NfcTdaAdaptation.h"
#include <semaphore.h>

nci_hal_ctrl_t nci_hal_ctrl;

/**
 *
 * @brief           Writes the data to controller
 *
 * @param[in]       p_data - data buffer pointer
 * @param[in]       data_len - data buffer length
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS ct_osal_write(uint8_t *p_data, uint16_t data_len, bool is_tda) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  (void)is_tda;
  TdaHalWrite(data_len, p_data);
  return 1;
}

/**
 *
 * @brief  Allocates some memoryAllocates some memory
 *
 * @param[in] dwSize   Size, in uint32_t, to be allocated
 *
 * @return            NON-NULL value:  The memory is successfully allocated ;
 *                    the return value is a pointer to the allocated memory
 * location NULL:The operation is not successful.
 *
 */
void *ct_osal_malloc(int size) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  return malloc(size);
}

/**
 * @brief                Copies the values stored in the source memory to the
 *                       values stored in the destination memory.
 *
 * @param[in] pDest     Pointer to the Destination Memory
 * @param[in] pSrc      Pointer to the Source Memory
 * @param[in] dwSize    Number of bytes to be copied.
 *
 * @return    void
 */
void ct_osal_memcpy(void *pDest, const void *pSrc, int size) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  memcpy(pDest, pSrc, size);
  return;
}

/**
 *
 * @brief get the value of a semaphore
 *
 * @param[in] sem_t          Semaphore.
 *
 * @return #0                on success
 * @return #EINVAL           sem is not a valid semaphore
 *
 */
int ct_osal_sem_getvalue(sem_t *sem, int *sval) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  return sem_getvalue(sem, sval);
}

/**
 *
 * @brief unlock a semaphore
 *
 * @param[in] sem_t                          Semaphore.
 *
 * @return #0                on success
 * @return #EINVAL           sem is not a valid semaphore..
 * @return #EOVERFLOW        The maximum allowable value for a semaphore would
 *                           be exceeded
 */
int ct_osal_sem_post(sem_t *sem) {
  OSAL_LOG_NFCHAL_D("%s\n", __func__);
  return sem_post(sem);
}

/**
 *
 * @brief lock a semaphore with monotonic timeout
 *
 * @param[in] sem_t                          Semaphore.
 *
 * @return #0             on success
 * @return #EAGAIN        The operation could not be performed
 *                        without blocking
 *
 * @return #EINTR         The call was interrupted by
 *                        a signal handler.
 * @return #EINVAL        sem is not a valid semaphore..
 * @return #EINVAL        The value of abs_timeout.tv_nsecs is
 *                        less than 0, or greater than or
 *                        equal to 1000 million
 * @return #ETIMEDOUT     The call timed out before the semaphore
 *                        could be locked.
 */
int ct_osal_sem_timedwait_monotonic_np(sem_t *__sem,
                                       const struct timespec *__ts) {
  OSAL_LOG_NFCHAL_D("%s\n", __func__);
  return sem_timedwait_monotonic_np(__sem, __ts);
}

/**
 *
 * @brief Informs the CT initialization complete to EMVCo stack
 * @param[in] void
 * @return void
 */
void ct_osal_init_completed() {
  OSAL_LOG_NFCHAL_D("%s\n", __func__);
  //  ct_init_completed();
}

/**
 *
 * @brief gets the EMVCo stack HAL control
 * @param[in] void
 * @return nci_hal_ctrl_t EMVCo HAL Control
 */
nci_hal_ctrl_t ct_osal_get_nci_hal_ctrl() {
  OSAL_LOG_NFCHAL_D("%s\n", __func__);
  return nci_hal_ctrl;
}

/**
 *
 * @brief sets the fragment data position of EMVCo stack HAL control
 * @param[in] void
 * @return void
 */
void ct_osal_set_frag_data_pos(uint8_t pos) {
  OSAL_LOG_NFCHAL_D("%s  :%d\n", __func__, pos);
  nci_hal_ctrl.frag_rsp.data_pos = pos;
}

/**
 *
 * @brief sets chained data flag as true
 * @param[in] void
 * @return void
 */
void CT_OSAL_SET_CHAINED_DATA() {
  OSAL_LOG_NFCHAL_D("%s\n", __func__);
  SET_CHAINED_DATA();
}
/**
 *
 * @brief sets chained data flag as false
 * @param[in] void
 * @return void
 */
void CT_OSAL_RESET_CHAINED_DATA() {
  OSAL_LOG_NFCHAL_D("%s\n", __func__);
  RESET_CHAINED_DATA();
}

/**
 *
 * @brief returns chained data flag
 * @param[in] void
 * @return bool
 */
bool CT_OSAL_IS_CHAINED_DATA() {
  OSAL_LOG_NFCHAL_D("%s\n", __func__);
  return IS_CHAINED_DATA();
  return true;
}