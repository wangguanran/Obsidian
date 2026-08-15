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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#include "emvco_tml.h"
#include "emvco_tml_i2c.h"
#include "emvco_util.h"
#include "osal_thread.h"
#include "osal_tml.h"
#include <emvco_log.h>
#include <emvco_status.h>
#include <string.h>

#define CRC_LEN 2
#define NORMAL_MODE_HEADER_LEN 3
#define FW_DNLD_HEADER_LEN 2
#define FW_DNLD_LEN_OFFSET 1
#define NORMAL_MODE_LEN_OFFSET 2
#define FRAGMENTSIZE_MAX PHNFC_I2C_FRAGMENT_SIZE

static struct osal_semaphore mTxRxSemaphore;

extern tml_emvco_context_t *gptml_emvco_context;

/*******************************************************************************
**
** Function         i2c_close
**
** Description      Closes NFCC device
**
** Parameters       p_dev_handle - device handle
**
** Returns          None
**
*******************************************************************************/
void i2c_close(void *p_dev_handle) {
  if (NULL != p_dev_handle) {
    osal_tml_close((intptr_t)p_dev_handle);
  }
  osal_sem_destroy(&mTxRxSemaphore.object_handle);
  return;
}

/*******************************************************************************
**
** Function         i2c_open_and_configure
**
** Description      Open and configure NFCC device
**
** Parameters       pConfig     - hardware information
**                  pLinkHandle - device handle
**
** Returns          NFC status:
**                  EMVCO_STATUS_SUCCESS - open_and_configure operation success
**                  EMVCO_STATUS_INVALID_DEVICE - device open operation failure
**
*******************************************************************************/
EMVCO_STATUS i2c_open_and_configure(ptml_emvco_Config_t pConfig,
                                    void **pLinkHandle) {
  int nHandle;

  LOG_EMVCO_TML_D("%s Opening port=%s\n", __func__, pConfig->p_dev_name);
  /* open port */
  nHandle = osal_tml_open((const char *)pConfig->p_dev_name, O_RDWR);
  if (nHandle < 0) {
    LOG_EMVCO_TML_E("_i2c_open() Failed: retval %x", nHandle);
    *pLinkHandle = NULL;
    return EMVCO_STATUS_INVALID_DEVICE;
  }

  *pLinkHandle = (void *)((intptr_t)nHandle);
  if (0 != osal_sem_init(&mTxRxSemaphore.object_handle, 0, 1)) {
    LOG_EMVCO_TML_E("%s Failed: reason osal_sem_init : retval %x", __func__,
                    nHandle);
  }
  /*Reset Controller*/
  i2c_nfcc_reset((void *)((intptr_t)nHandle), MODE_POWER_OFF);
  usleep(10 * 1000);
  i2c_nfcc_reset((void *)((intptr_t)nHandle), MODE_POWER_ON);

  return EMVCO_STATUS_SUCCESS;
}

int i2c_flush_data(void *p_dev_handle, uint8_t *p_buffer, int numRead) {
  int retRead = 0;
  uint16_t totalBtyesToRead =
      p_buffer[FW_DNLD_LEN_OFFSET] + FW_DNLD_HEADER_LEN + CRC_LEN;
  /* we shall read totalBtyesToRead-1 as one byte is already read by calling
   * function*/
  retRead = osal_tml_read((intptr_t)p_dev_handle, p_buffer + numRead,
                          totalBtyesToRead - 1);
  if (retRead > 0) {
    numRead += retRead;
    print_packet("RECV", p_buffer, numRead);
  } else if (retRead == 0) {
    LOG_EMVCO_TML_E("%s _i2c_read() [pyld] EOF", __func__);
  } else {
    LOG_EMVCO_TML_D("%s _i2c_read() [hdr] received", __func__);
    print_packet("RECV", p_buffer - numRead, NORMAL_MODE_HEADER_LEN);
    LOG_EMVCO_TML_E("%s _i2c_read() [pyld] errno : %x", __func__, errno);
  }
  i2c_sem_post();
  return -1;
}

/*******************************************************************************
**
** Function         phOsal_Read
**
** Description      Reads requested number of bytes from NFCC device into given
**                  buffer
**
** Parameters       p_dev_handle       - valid device handle
**                  p_buffer          - buffer for read data
**                  nNbBytesToRead   - number of bytes requested to be read
**
** Returns          numRead   - number of successfully read bytes
**                  -1        - read operation failure
**
*******************************************************************************/
int i2c_read(void *p_dev_handle, uint8_t *p_buffer, int nNbBytesToRead) {
  LOG_EMVCO_TML_D("%s i2c_read", __func__);
  int ret_Read;
  int ret_Select;
  int numRead = 0;
  struct timeval tv;
  fd_set rfds;
  uint16_t totalBtyesToRead = 0;

  UNUSED(nNbBytesToRead);
  if (NULL == p_dev_handle) {
    return -1;
  }

  totalBtyesToRead = NORMAL_MODE_HEADER_LEN;

  /* Read with 2 second timeout, so that the read thread can be aborted
     when the NFCC does not respond and we need to switch to FW download
     mode. This should be done via a control socket instead. */
  FD_ZERO(&rfds);
  FD_SET((intptr_t)p_dev_handle, &rfds);
  tv.tv_sec = 2;
  tv.tv_usec = 1;

  ret_Select =
      select((int)((intptr_t)p_dev_handle + (int)1), &rfds, NULL, NULL, &tv);
  if (ret_Select < 0) {
    LOG_EMVCO_TML_D("%s errno : %x", __func__, errno);
    return -1;
  } else if (ret_Select == 0) {
    LOG_EMVCO_TML_D("%s Timeout", __func__);
    return -1;
  } else {
    ret_Read = osal_tml_read((intptr_t)p_dev_handle, p_buffer,
                             totalBtyesToRead - numRead);
    if (ret_Read > 0 && !(p_buffer[0] == 0xFF && p_buffer[1] == 0xFF)) {
      i2c_sem_timed_wait();
      numRead += ret_Read;
    } else if (ret_Read == 0) {
      LOG_EMVCO_TML_E("%s [hdr]EOF", __func__);
      return -1;
    } else {
      LOG_EMVCO_TML_E("%s [hdr] errno : %x", __func__, errno);
      LOG_EMVCO_TML_E(" %s p_buffer[0] = %x p_buffer[1]= %x", __func__,
                      p_buffer[0], p_buffer[1]);
      return -1;
    }

    totalBtyesToRead = NORMAL_MODE_HEADER_LEN;
#if (NXP_EXTNS == TRUE)
    if (gptml_emvco_context->t_read_info.p_context != NULL &&
        !memcmp(gptml_emvco_context->t_read_info.p_context, "MinOpen", 0x07) &&
        !p_buffer[0] && p_buffer[1]) {
      return i2c_flush_data(p_dev_handle, p_buffer, numRead);
    }
#endif

    if (numRead < totalBtyesToRead) {
      ret_Read = osal_tml_read((intptr_t)p_dev_handle, (p_buffer + numRead),
                               totalBtyesToRead - numRead);

      if (ret_Read != totalBtyesToRead - numRead) {
        i2c_sem_post();
        LOG_EMVCO_TML_E("%s [hdr] errno : %x", __func__, errno);
        return -1;
      } else {
        numRead += ret_Read;
      }
    }
    totalBtyesToRead =
        p_buffer[NORMAL_MODE_LEN_OFFSET] + NORMAL_MODE_HEADER_LEN;
    if ((totalBtyesToRead - numRead) != 0) {
      ret_Read = osal_tml_read((intptr_t)p_dev_handle, (p_buffer + numRead),
                               totalBtyesToRead - numRead);
      if (ret_Read > 0) {
        numRead += ret_Read;
      } else if (ret_Read == 0) {
        i2c_sem_post();
        LOG_EMVCO_TML_E("%s [pyld] EOF", __func__);
        return -1;
      } else {
        LOG_EMVCO_TML_D("_i2c_read() [hdr] received");
        print_packet("RECV", p_buffer, NORMAL_MODE_HEADER_LEN);
        i2c_sem_post();
        LOG_EMVCO_TML_E("%s [pyld] errno : %x", __func__, errno);
        return -1;
      }
    } else {
      LOG_EMVCO_TML_E("%s _>>>>> Empty packet recieved !!", __func__);
    }
  }
  i2c_sem_post();
  return numRead;
}

/*******************************************************************************
**
** Function         i2c_write
**
** Description      Writes requested number of bytes from given buffer into
**                  NFCC device
**
** Parameters       p_dev_handle       - valid device handle
**                  p_buffer          - buffer for read data
**                  nNbBytesToWrite  - number of bytes requested to be written
**
** Returns          numWrote   - number of successfully written bytes
**                  -1         - i2c_write operation failure
**
*******************************************************************************/
int i2c_write(void *p_dev_handle, uint8_t *p_buffer, int nNbBytesToWrite) {
  int ret;
  int numWrote = 0;
  int numBytes = nNbBytesToWrite;
  if (NULL == p_dev_handle) {
    return -1;
  }
  while (numWrote < nNbBytesToWrite) {
    i2c_sem_timed_wait();
    ret = osal_tml_write((intptr_t)p_dev_handle, p_buffer + numWrote,
                         numBytes - numWrote);
    i2c_sem_post();
    if (ret > 0) {
      numWrote += ret;
    } else if (ret == 0) {
      LOG_EMVCO_TML_D("%s EOF", __func__);
      return -1;
    } else {
      LOG_EMVCO_TML_D("%s errno : %x", __func__, errno);
      if (errno == EINTR || errno == EAGAIN) {
        continue;
      }
      return -1;
    }
  }

  return numWrote;
}

/*******************************************************************************
**
** Function         i2c_nfcc_reset
**
** Description      Reset NFCC device, using VEN pin
**
** Parameters       p_dev_handle     - valid device handle
**                  eType          - reset level
**
** Returns           0   - reset operation success
**                  -1   - reset operation failure
**
*******************************************************************************/
int i2c_nfcc_reset(void *p_dev_handle, enum NfccResetType eType) {
  int ret = -1;
  LOG_EMVCO_TML_D("%s, VEN eType %u", __func__, eType);

  if (NULL == p_dev_handle) {
    return -1;
  }

  ret = osal_tml_ioctl((intptr_t)p_dev_handle, NFC_SET_PWR, eType, 2);

  if (ret < 0) {
    LOG_EMVCO_TML_E("%s :failed errno = 0x%x", __func__, errno);
  }
  return ret;
}

int i2c_led_control(void *p_dev_handle, enum LEDControl eType) {
  int ret = -1;
  LOG_EMVCO_TML_D("%s, LEDControl eType %u", __func__, eType);

  if (NULL == p_dev_handle) {
    return -1;
  }

  ret = osal_tml_ioctl((intptr_t)p_dev_handle, LEDS_CONTROL, eType, 2);

  if (ret < 0) {
    LOG_EMVCO_TML_E("%s :failed errno = 0x%x", __func__, errno);
  }
  return ret;
}

int i2c_nfcc_profile_switch(void *p_dev_handle, enum ProfileMode eType) {
  int ret = -1;
  LOG_EMVCO_TML_D("%s, LEDControl eType %u", __func__, eType);

  if (NULL == p_dev_handle) {
    return -1;
  }

  ret = osal_tml_ioctl((intptr_t)p_dev_handle, NFCC_PROFILE_SWITCH, eType, 2);

  if (ret < 0) {
    LOG_EMVCO_TML_E("%s :failed errno = 0x%x", __func__, errno);
  }
  return ret;
}

int i2c_smcu_profile_switch(void *p_dev_handle, enum ProfileMode eType) {
  int ret = -1;
  LOG_EMVCO_TML_D("%s, LEDControl eType %u", __func__, eType);

  if (NULL == p_dev_handle) {
    return -1;
  }

  ret = osal_tml_ioctl((intptr_t)p_dev_handle, SMCU_PROFILE_SWITCH, eType, 2);

  if (ret < 0) {
    LOG_EMVCO_TML_E("%s :failed errno = 0x%x", __func__, errno);
  }
  return ret;
}

/*******************************************************************************
**
** Function         i2c_sem_post
**
** Description      i2c_sem_post i2c_read / write
**
** Parameters       none
**
** Returns          none
*******************************************************************************/
void i2c_sem_post() {
  int sem_val = 0;
  osal_sem_getvalue(&mTxRxSemaphore.object_handle, &sem_val);
  if (sem_val == 0) {
    osal_sem_post(&mTxRxSemaphore.object_handle);
  }
}

/*******************************************************************************
**
** Function         i2c_sem_timed_wait
**
** Description      Timed sem_wait for avoiding i2c_read & write overlap
**
** Parameters       none
**
** Returns          Sem_wait return status
*******************************************************************************/
int i2c_sem_timed_wait() {
  EMVCO_STATUS status = EMVCO_STATUS_FAILED;
  long sem_timedout = 500 * 1000 * 1000;
  int s = 0;
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 0;
  ts.tv_nsec += sem_timedout;
  while ((s = osal_sem_timedwait(&mTxRxSemaphore.object_handle, &ts)) == -1 &&
         errno == EINTR) {
    continue; /* Restart if interrupted by handler */
  }
  if (s != -1) {
    status = EMVCO_STATUS_SUCCESS;
  } else if (errno == ETIMEDOUT && s == -1) {
    LOG_EMVCO_TML_E("%s :timed out errno = 0x%x", __func__, errno);
  }
  return status;
}
