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
 * TML Implementation.
 */

#include <emvco_config.h>
#include <emvco_dm.h>
#include <emvco_log.h>
#include <emvco_tml.h>
#include <emvco_tml_i2c.h>
#include <emvco_util.h>
#include <osal_message_queue_lib.h>
#include <osal_thread.h>
#include <osal_timer.h>

/*
 * Duration of Timer to wait after sending an Nci packet
 */
#define PHTMLNFC_MAXTIME_RETRANSMIT (200U)
#define MAX_WRITE_RETRY_COUNT 0x03
#define MAX_READ_RETRY_DELAY_IN_MILLISEC (150U)

/* Value to reset variables of TML  */
#define TML_EMVCO_RESET_VALUE (0x00)

/* Initialize Context structure pointer used to access context structure */
tml_emvco_context_t *gptml_emvco_context = NULL;
/* Local Function prototypes */
static EMVCO_STATUS tml_emvco_start_thread(void);
static void tml_readDeferredCb(void *pParams);
static void *emvco_tml_thread(void *pParam);

/* Function definitions */

EMVCO_STATUS tml_init(ptml_emvco_Config_t pConfig) {
  EMVCO_STATUS wInitStatus = EMVCO_STATUS_SUCCESS;

  /* Check if TML layer is already Initialized */
  if (NULL != gptml_emvco_context) {
    /* TML initialization is already completed */
    wInitStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_ALREADY_INITIALISED);
  }
  /* Validate Input parameters */
  else if ((NULL == pConfig) ||
           (TML_EMVCO_RESET_VALUE == pConfig->dw_get_msg_thread_id)) {
    /*Parameters passed to TML init are wrong */
    wInitStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_INVALID_PARAMETER);
  } else {
    /* Allocate memory for TML context */
    gptml_emvco_context =
        (tml_emvco_context_t *)malloc(sizeof(tml_emvco_context_t));

    if (NULL == gptml_emvco_context) {
      wInitStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_FAILED);
    } else {
      /* Initialise all the internal TML variables */
      memset(gptml_emvco_context, TML_EMVCO_RESET_VALUE,
             sizeof(tml_emvco_context_t));
      /* Make sure that the thread runs once it is created */
      gptml_emvco_context->b_thread_done = 1;

      /* Open the device file to which data is read/written */
      wInitStatus =
          i2c_open_and_configure(pConfig, &(gptml_emvco_context->p_dev_handle));

      if (EMVCO_STATUS_SUCCESS != wInitStatus) {
        wInitStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_INVALID_DEVICE);
        gptml_emvco_context->p_dev_handle = NULL;
      } else {
        gptml_emvco_context->t_read_info.b_enable = 0;
        gptml_emvco_context->t_read_info.b_thread_busy = false;
        if (osal_mutex_init(&gptml_emvco_context->read_info_update_mutex) !=
            0) {
          wInitStatus = EMVCO_STATUS_FAILED;
        } else if (0 != sem_init(&gptml_emvco_context->rx_semaphore, 0, 0)) {
          wInitStatus = EMVCO_STATUS_FAILED;
        } else if (0 !=
                   sem_init(&gptml_emvco_context->post_msg_semaphore, 0, 0)) {
          wInitStatus = EMVCO_STATUS_FAILED;
        } else {
          sem_post(&gptml_emvco_context->post_msg_semaphore);
          /* Start TML thread (to handle write and read operations) */
          if (EMVCO_STATUS_SUCCESS != tml_emvco_start_thread()) {
            wInitStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_FAILED);
          } else {
            /* Create Timer used for Retransmission of NCI packets */
            gptml_emvco_context->dw_timer_id = osal_timer_create();
            if (PH_OSALNFC_TIMER_ID_INVALID !=
                gptml_emvco_context->dw_timer_id) {
              /* Store the Thread Identifier to which Message is to be posted */
              gptml_emvco_context->dw_callback_thread_id =
                  pConfig->dw_get_msg_thread_id;
              /* Enable retransmission of Nci packet & set retry count to
               * default */
              gptml_emvco_context->e_config = disable_retrans;
              /* Retry Count = Standby Recovery time of NFCC / Retransmission
               * time + 1 */
              gptml_emvco_context->b_retry_count =
                  (2000 / PHTMLNFC_MAXTIME_RETRANSMIT) + 1;
            } else {
              wInitStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_FAILED);
            }
          }
        }
      }
    }
  }
  /* Clean up all the TML resources if any error */
  if (EMVCO_STATUS_SUCCESS != wInitStatus) {
    /* Clear all handles and memory locations initialized during init */
    tml_shutdown_cleanup();
  }

  return wInitStatus;
}

/*******************************************************************************
**
** Function         tml_emvco_start_thread
**
** Description      Initializes comport, reader and writer threads
**
** Parameters       None
**
** Returns          NFC status:
**                  EMVCO_STATUS_SUCCESS - threads initialized successfully
**                  EMVCO_STATUS_FAILED - initialization failed due to system
*error
**
*******************************************************************************/
static EMVCO_STATUS tml_emvco_start_thread(void) {
  EMVCO_STATUS wStartStatus = EMVCO_STATUS_SUCCESS;
  void *h_threadsEvent = 0x00;
  int pthread_create_status = 0;

  /* Create Reader and Writer threads */
  pthread_create_status =
      osal_thread_create(&gptml_emvco_context->reader_thread, NULL,
                         &emvco_tml_thread, (void *)h_threadsEvent);
  if (0 != pthread_create_status) {
    wStartStatus = EMVCO_STATUS_FAILED;
  }

  return wStartStatus;
}

/*******************************************************************************
**
** Function         emvco_tml_thread
**
** Description      Read the data from the lower layer driver
**
** Parameters       pParam  - parameters for Writer thread function
**
** Returns          None
**
*******************************************************************************/
static void *emvco_tml_thread(void *pParam) {
  EMVCO_STATUS w_status = EMVCO_STATUS_SUCCESS;
  int32_t dwNoBytesWrRd = TML_EMVCO_RESET_VALUE;
  uint8_t temp[260];
  uint8_t readRetryDelay = 0;
  /* Transaction info buffer to be passed to Callback Thread */
  static osal_transact_info_t tTransactionInfo;
  /* Structure containing Tml callback function and parameters to be invoked
     by the callback thread */
  static lib_emvco_deferred_call_t tDeferredInfo;
  /* Initialize Message structure to post message onto Callback Thread */
  static lib_emvco_message_t tMsg;
  UNUSED(pParam);
  LOG_EMVCO_TML_D("PN72X - Tml Reader Thread Started................\n");

  /* Writer thread loop shall be running till shutdown is invoked */
  while (gptml_emvco_context->b_thread_done) {
    /* If Tml write is requested */
    /* Set the variable to success initially */
    w_status = EMVCO_STATUS_SUCCESS;
    if (-1 == sem_wait(&gptml_emvco_context->rx_semaphore)) {
      LOG_EMVCO_TML_E("sem_wait didn't return success \n");
    }
    /* If Tml read is requested */
    if (1 == gptml_emvco_context->t_read_info.b_enable) {
      LOG_EMVCO_TML_D("PN72X - Read requested.....\n");
      /* Set the variable to success initially */
      w_status = EMVCO_STATUS_SUCCESS;

      /* Variable to fetch the actual number of bytes read */
      dwNoBytesWrRd = TML_EMVCO_RESET_VALUE;

      /* Read the data from the file onto the buffer */
      if (NULL != gptml_emvco_context->p_dev_handle) {
        LOG_EMVCO_TML_D("PN72X - Invoking I2C Read.....\n");
        dwNoBytesWrRd = i2c_read(gptml_emvco_context->p_dev_handle, temp, 260);
        if (-1 == dwNoBytesWrRd) {
          LOG_EMVCO_TML_E("PN72X - Error in I2C Read.....\n");
          if (readRetryDelay < MAX_READ_RETRY_DELAY_IN_MILLISEC) {
            /*sleep for 30/60/90/120/150 msec between each read trial incase of
             * read error*/
            readRetryDelay += 30;
          }
          usleep(readRetryDelay * 1000);
          sem_post(&gptml_emvco_context->rx_semaphore);
        } else if (dwNoBytesWrRd > 260) {
          LOG_EMVCO_TML_E("Numer of bytes read exceeds the limit 260.....\n");
          readRetryDelay = 0;
          sem_post(&gptml_emvco_context->rx_semaphore);
        } else {
          osal_mutex_lock(&gptml_emvco_context->read_info_update_mutex);
          memcpy(gptml_emvco_context->t_read_info.p_buffer, temp,
                 dwNoBytesWrRd);
          readRetryDelay = 0;

          LOG_EMVCO_TML_D("PN72X - I2C Read successful.....\n");
          /* This has to be reset only after a successful read */
          gptml_emvco_context->t_read_info.b_enable = 0;
          if ((enable_retrans == gptml_emvco_context->e_config) &&
              (0x00 != (gptml_emvco_context->t_read_info.p_buffer[0] & 0xE0))) {
            LOG_EMVCO_TML_D("PN72X - Retransmission timer stopped.....\n");
            /* Stop Timer to prevent Retransmission */
            uint32_t timerStatus =
                osal_timer_stop(gptml_emvco_context->dw_timer_id);
            if (EMVCO_STATUS_SUCCESS != timerStatus) {
              LOG_EMVCO_TML_E("PN72X - timer stopped returned failure.....\n");
            }
          }
          /* Update the actual number of bytes read including header */
          gptml_emvco_context->t_read_info.w_length = (uint16_t)(dwNoBytesWrRd);
          print_packet("RECV", gptml_emvco_context->t_read_info.p_buffer,
                       gptml_emvco_context->t_read_info.w_length);

          dwNoBytesWrRd = TML_EMVCO_RESET_VALUE;

          /* Fill the Transaction info structure to be passed to Callback
           * Function */
          tTransactionInfo.w_status = w_status;
          memcpy(tMsg.data, gptml_emvco_context->t_read_info.p_buffer,
                 gptml_emvco_context->t_read_info.w_length);

          tTransactionInfo.w_length = gptml_emvco_context->t_read_info.w_length;
          /* Read operation completed successfully. Post a Message onto Callback
           * Thread*/
          /* Prepare the message to be posted on User thread */
          tDeferredInfo.p_callback = &tml_readDeferredCb;
          tDeferredInfo.p_parameter = &tTransactionInfo;
          tMsg.e_msgType = EMVCO_DEFERRED_CALL_MSG;
          tMsg.p_msg_data = &tDeferredInfo;
          /* Actual number of bytes read is filled in the structure */
          tMsg.size = gptml_emvco_context->t_read_info.w_length;
          tMsg.w_status = tTransactionInfo.w_status;
          LOG_EMVCO_TML_D("PN72X - Posting read message.....\n");
          tml_deferred_call(gptml_emvco_context->dw_callback_thread_id, &tMsg);
          osal_mutex_unlock(&gptml_emvco_context->read_info_update_mutex);
        }
      } else {
        LOG_EMVCO_TML_D("PN72X -gptml_emvco_context->p_dev_handle is NULL");
      }
    } else {
      LOG_EMVCO_TML_D("PN72X - read request NOT enabled");
      usleep(10 * 1000);
    }
  } /* End of While loop */

  return NULL;
}

void tml_cleanup(void) {
  if (NULL == gptml_emvco_context) {
    return;
  }
  if (NULL != gptml_emvco_context->p_dev_handle) {
    (void)i2c_nfcc_reset(gptml_emvco_context->p_dev_handle, MODE_POWER_OFF);
    gptml_emvco_context->b_thread_done = 0;
  }
  sem_destroy(&gptml_emvco_context->rx_semaphore);
  sem_destroy(&gptml_emvco_context->post_msg_semaphore);
  i2c_close(gptml_emvco_context->p_dev_handle);
  gptml_emvco_context->p_dev_handle = NULL;
  /* Clear memory allocated for storing Context variables */
  free((void *)gptml_emvco_context);
  /* Set the pointer to NULL to indicate De-Initialization */
  gptml_emvco_context = NULL;

  return;
}

EMVCO_STATUS tml_shutdown(void) {
  EMVCO_STATUS wShutdownStatus = EMVCO_STATUS_SUCCESS;

  /* Check whether TML is Initialized */
  if (NULL != gptml_emvco_context) {
    /* Reset thread variable to terminate the thread */
    gptml_emvco_context->b_thread_done = 0;
    usleep(1000);
    /* Clear All the resources allocated during initialization */
    sem_post(&gptml_emvco_context->rx_semaphore);
    usleep(1000);
    sem_post(&gptml_emvco_context->post_msg_semaphore);
    usleep(1000);
    sem_post(&gptml_emvco_context->post_msg_semaphore);
    usleep(1000);
    osal_mutex_destroy(&gptml_emvco_context->read_info_update_mutex);
    if (0 !=
        osal_thread_join(gptml_emvco_context->reader_thread, (void **)NULL)) {
      LOG_EMVCO_TML_E("Fail to kill reader thread!");
    }
    LOG_EMVCO_TML_D("b_thread_done == 0");

  } else {
    wShutdownStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_NOT_INITIALISED);
  }

  return wShutdownStatus;
}

EMVCO_STATUS tml_write(uint8_t *p_buffer, uint16_t w_length) {
  EMVCO_STATUS write_status = EMVCO_STATUS_SUCCESS;
  if ((NULL != gptml_emvco_context->p_dev_handle) && (NULL != p_buffer) &&
      (TML_EMVCO_RESET_VALUE != w_length)) {
    LOG_EMVCO_TML_D("PN72X - Invoking I2C Write.....\n");
    int32_t num_of_bytes_wrote =
        i2c_write(gptml_emvco_context->p_dev_handle, p_buffer, w_length);
    if (-1 == num_of_bytes_wrote) {
      LOG_EMVCO_TML_D("PN72X - Error in I2C Write.....\n");
      write_status = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_FAILED);
    } else {
      print_packet("SEND", p_buffer, w_length);
    }

    if (EMVCO_STATUS_SUCCESS == write_status) {
      LOG_EMVCO_TML_D("PN72X - I2C Write successful.....\n");
    }
  } else {
    LOG_EMVCO_TML_D("PN72X - Write error invalid parameter");
    write_status = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_INVALID_PARAMETER);
  }

  return write_status;
}

EMVCO_STATUS tml_update_read_complete_callback(
    transact_completion_callback_t pTmlReadComplete) {
  EMVCO_STATUS w_status = EMVCO_STATUS_FAILED;
  if ((NULL != gptml_emvco_context) && (NULL != pTmlReadComplete)) {
    gptml_emvco_context->t_read_info.p_thread_callback = pTmlReadComplete;
    w_status = EMVCO_STATUS_SUCCESS;
  }
  return w_status;
}

EMVCO_STATUS tml_read(uint8_t *p_buffer, uint16_t w_length,
                      transact_completion_callback_t pTmlReadComplete,
                      void *p_context) {
  EMVCO_STATUS wReadStatus;

  /* Check whether TML is Initialized */
  if (NULL != gptml_emvco_context) {
    if ((gptml_emvco_context->p_dev_handle != NULL) && (NULL != p_buffer) &&
        (TML_EMVCO_RESET_VALUE != w_length) && (NULL != pTmlReadComplete)) {
      if (!gptml_emvco_context->t_read_info.b_thread_busy) {
        osal_mutex_lock(&gptml_emvco_context->read_info_update_mutex);
        /* Setting the flag marks beginning of a Read Operation */
        gptml_emvco_context->t_read_info.b_thread_busy = true;
        /* Copy the buffer, length and Callback function,
           This shall be utilized while invoking the Callback function in thread
           */
        gptml_emvco_context->t_read_info.p_buffer = p_buffer;
        gptml_emvco_context->t_read_info.w_length = w_length;
        gptml_emvco_context->t_read_info.p_thread_callback = pTmlReadComplete;
        gptml_emvco_context->t_read_info.p_context = p_context;
        wReadStatus = EMVCO_STATUS_PENDING;

        /* Set event to invoke Reader Thread */
        gptml_emvco_context->t_read_info.b_enable = 1;
        osal_mutex_unlock(&gptml_emvco_context->read_info_update_mutex);
        sem_post(&gptml_emvco_context->rx_semaphore);
      } else {
        wReadStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_BUSY);
      }
    } else {
      wReadStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_INVALID_PARAMETER);
    }
  } else {
    wReadStatus = EMVCOSTVAL(CID_EMVCO_TML, EMVCO_STATUS_NOT_INITIALISED);
  }

  return wReadStatus;
}

EMVCO_STATUS tml_read_abort(void) {
  EMVCO_STATUS w_status = EMVCO_STATUS_INVALID_PARAMETER;
  gptml_emvco_context->t_read_info.b_enable = 0;

  /*Reset the flag to accept another Read Request */
  gptml_emvco_context->t_read_info.b_thread_busy = false;
  w_status = EMVCO_STATUS_SUCCESS;

  return w_status;
}

EMVCO_STATUS tml_ioctl(emvco_control_code_t eControlCode) {
  EMVCO_STATUS w_status = EMVCO_STATUS_SUCCESS;

  if (NULL == gptml_emvco_context) {
    w_status = EMVCO_STATUS_FAILED;
  } else {
    switch (eControlCode) {
    case ResetDevice: {
      /*Reset PN72X*/
      i2c_nfcc_reset(gptml_emvco_context->p_dev_handle, MODE_POWER_ON);
      usleep(100 * 1000);
      i2c_nfcc_reset(gptml_emvco_context->p_dev_handle, MODE_POWER_OFF);
      usleep(100 * 1000);
      i2c_nfcc_reset(gptml_emvco_context->p_dev_handle, MODE_POWER_ON);
      break;
    }
    case EnableNormalMode: {
      /*Reset PN72X*/
      i2c_nfcc_reset(gptml_emvco_context->p_dev_handle, MODE_POWER_OFF);
      usleep(10 * 1000);
      i2c_nfcc_reset(gptml_emvco_context->p_dev_handle, MODE_POWER_ON);
      usleep(100 * 1000);
      break;
    }
    case RedLedOff: {
      i2c_led_control(gptml_emvco_context->p_dev_handle, RED_LED_OFF);
      break;
    }
    case RedLedOn: {
      i2c_led_control(gptml_emvco_context->p_dev_handle, RED_LED_ON);
      break;
    }
    case GreenLedOff: {
      i2c_led_control(gptml_emvco_context->p_dev_handle, GREEN_LED_OFF);
      break;
    }
    case GreenLedOn: {
      i2c_led_control(gptml_emvco_context->p_dev_handle, GREEN_LED_ON);
      break;
    }
    case NFCCModeSwitchOn: {
      i2c_nfcc_profile_switch(gptml_emvco_context->p_dev_handle, EMVCO_MODE);
      break;
    }
    case NFCCModeSwitchOff: {
      i2c_nfcc_profile_switch(gptml_emvco_context->p_dev_handle, NCI_MODE);
      break;
    }
    case SMCUModeSwitchOn: {
      i2c_smcu_profile_switch(gptml_emvco_context->p_dev_handle, EMVCO_MODE);
      break;
    }
    case SMCUModeSwitchOff: {
      i2c_smcu_profile_switch(gptml_emvco_context->p_dev_handle, NCI_MODE);
      break;
    }
    default: {
      w_status = EMVCO_STATUS_INVALID_PARAMETER;
      break;
    }
    }
  }

  return w_status;
}

void tml_deferred_call(uintptr_t dwThreadId, lib_emvco_message_t *ptWorkerMsg) {
  intptr_t bPostStatus;
  UNUSED(dwThreadId);
  /* Post message on the user thread to invoke the callback function */
  if (-1 == sem_wait(&gptml_emvco_context->post_msg_semaphore)) {
    LOG_EMVCO_TML_E("sem_wait didn't return success \n");
  }
  bPostStatus =
      osal_msg_snd(gptml_emvco_context->dw_callback_thread_id, ptWorkerMsg, 0);
  sem_post(&gptml_emvco_context->post_msg_semaphore);
}

/*******************************************************************************
**
** Function         tml_readDeferredCb
**
** Description      Read thread call back function
**
** Parameters       pParams - context provided by upper layer
**
** Returns          None
**
*******************************************************************************/
static void tml_readDeferredCb(void *pParams) {
  /* Transaction info buffer to be passed to Callback Function */
  osal_transact_info_t *pTransactionInfo = (osal_transact_info_t *)pParams;

  /* Reset the flag to accept another Read Request */
  gptml_emvco_context->t_read_info.b_thread_busy = false;
  enable_tml_read();
  gptml_emvco_context->t_read_info.p_thread_callback(
      gptml_emvco_context->t_read_info.p_context, pTransactionInfo);

  return;
}

EMVCO_STATUS tml_shutdown_cleanup() {
  EMVCO_STATUS wShutdownStatus = tml_shutdown();
  tml_cleanup();
  return wShutdownStatus;
}
