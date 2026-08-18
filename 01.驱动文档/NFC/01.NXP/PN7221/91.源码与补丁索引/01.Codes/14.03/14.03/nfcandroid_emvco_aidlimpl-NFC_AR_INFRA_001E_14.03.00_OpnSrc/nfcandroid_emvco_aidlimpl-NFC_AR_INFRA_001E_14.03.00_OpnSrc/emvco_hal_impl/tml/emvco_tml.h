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

#ifndef _EMVCO_TML_H_
#define _EMVCO_TML_H_

/** \addtogroup EMVCO_STACK_TML_API_INTERFACE
 *  @brief      Transport Mapping Layer header files containing APIs related to
 * initializing, reading and writing data into files provided by the driver
 * interface.
 *
 * API listed here encompasses Transport Mapping Layer interfaces required to be
 * mapped
 * to different Interfaces and Platforms.
 *  @{
 */

#include <emvco_osal_common.h>

/*
 * Message posted by Reader thread upon
 * completion of requested operation
 */
#define TMLNFC_READ_MESSAGE (0xAA)

/*
 * Message posted by Writer thread upon
 * completion of requested operation
 */
#define TMLNFC_WRITE_MESSAGE (0x55)

/*
 * Value indicates to reset device
 */
#define TMLNFC_RESETDEVICE (0x00008001)

/*
***************************Globals,Structure and Enumeration ******************
*/

/**
 * @brief Transaction (Tx/Rx) completion information structure of TML
 *
 * This structure holds the completion callback information of the
 * transaction passed from the TML layer to the Upper layer
 * along with the completion callback.
 *
 * The value of field w_status can be interpreted as:
 *
 *     - EMVCO_STATUS_SUCCESS                    Transaction performed
 * successfully.
 *     - EMVCO_STATUS_FAILED                     Failed to wait on Read/Write
 * operation.
 */

typedef struct osal_transact_info {
  EMVCO_STATUS w_status; /* Status of the Transaction Completion*/
  uint8_t *p_buff;       /* Response Data of the Transaction*/
  uint16_t w_length;     /* Data size of the Transaction*/
} osal_transact_info_t;  /* Instance of Transaction structure */

/*nfc state flags*/
enum emvco_state_flags {
  /*nfc in unknown state */
  EMVCO_STATE_UNKNOWN = 0,
  /*nfc booted in download mode */
  EMVCO_STATE_FW_DWL = 0x1,
  /*nfc booted in NCI mode */
  EMVCO_STATE_NCI = 0x2,
};
/**
 * @brief TML transreceive completion callback to Upper Layer
 *
 * p_context - Context provided by upper layer
 * pInfo    - Transaction info. See osal_transact_info
 */
typedef void (*transact_completion_callback_t)(void *p_context,
                                               osal_transact_info_t *pInfo);

/**
 * @brief TML Deferred callback interface structure invoked by upper layer
 *
 * This could be used for read/write operations
 *
 * dw_msg_posted_thread Message source identifier
 * pParams Parameters for the deferred call processing
 */
typedef void (*defer_func_pointer_t)(uint32_t dw_msg_posted_thread,
                                     void *pParams);

/**
 * @brief Enum definition contains  supported ioctl control codes.
 *
 * tml_ioctl
 */
typedef enum {
  Invalid = 0,
  ResetDevice = TMLNFC_RESETDEVICE, /* Reset the device */
  EnableDownloadMode,               /* Do the hardware setting to enter into
                                                  download mode */
  EnableNormalMode, /* Hardware setting for normal mode of operation
                     */
  RedLedOff,        /* Turns off red led */
  RedLedOn,         /* Turns on red led */
  GreenLedOff,      /* Turns off green led */
  GreenLedOn,       /* Turns on green led */
  NFCCModeSwitchOn,
  NFCCModeSwitchOff,
  SMCUModeSwitchOn,
  SMCUModeSwitchOff,
} emvco_control_code_t; /* Control code for IOCTL call */

/*
 * Enable / Disable Re-Transmission of Packets
 *
 * tml_emvco_ConfigNciPktReTx
 */
typedef enum {
  enable_retrans = 0x00, /*Enable retransmission of Nci packet */
  disable_retrans = 0x01 /*Disable retransmission of Nci packet */
} config_retrans_t;      /* Configuration for Retransmission */

/*
 * Structure containing details related to read and write operations
 *
 */
typedef struct tml_read_write_info {
  volatile uint8_t b_enable; /*This flag shall decide whether to perform
                               Write/Read operation */
  uint8_t b_thread_busy;     /*Flag to indicate thread is busy on respective
                                operation */
  /* Transaction completion Callback function */
  transact_completion_callback_t p_thread_callback;
  void *p_context;          /*Context passed while invocation of operation */
  uint8_t *p_buffer;        /*Buffer passed while invocation of operation */
  uint16_t w_length;        /*Length of data read/written */
  EMVCO_STATUS work_status; /*Status of the transaction performed */
} tml_read_write_info_t;

/*
 *Base Context Structure containing members required for entire session
 */
typedef struct tml_emvco_context {
  pthread_t reader_thread; /*Handle to the thread which handles write and read
                             operations */
  volatile uint8_t
      b_thread_done; /*Flag to decide whether to run or abort the thread */
  config_retrans_t e_config; /*Retransmission of Nci Packet during timeout */
  uint8_t b_retry_count;     /*Number of times retransmission shall happen */
  uint32_t dw_timer_id;      /* Timer used to retransmit nci packet */
  tml_read_write_info_t t_read_info; /*Pointer to Reader Thread Structure */
  void *p_dev_handle;                /* Pointer to Device Handle */
  uintptr_t dw_callback_thread_id; /* Thread ID to which message to be posted */
  uint8_t b_enable_crc; /*Flag to validate/not CRC for input buffer */
  sem_t rx_semaphore;
  sem_t post_msg_semaphore; /* Semaphore to post message atomically by Reader &
                             writer thread */
  pthread_mutex_t
      read_info_update_mutex; /*Mutex to synchronize read Info update*/
} tml_emvco_context_t;

/**
 * @brief
 * TML Configuration exposed to upper layer.
 */
typedef struct tml_emvco_Config {
  /* Port name connected to PN72X
   *
   * Platform specific canonical device name to which PN72X is connected.
   *
   * e.g. On Linux based systems this would be /dev/PN72X
   */
  int8_t *p_dev_name;
  /* Callback Thread ID
   *
   * This is the thread ID on which the Reader & Writer thread posts message. */
  uintptr_t dw_get_msg_thread_id;
  /* Communication speed between DH and PN72X
   *
   * This is the baudrate of the bus for communication between DH and PN72X */
  uint32_t dw_baud_rate;
} tml_emvco_Config_t, *ptml_emvco_Config_t; /* pointer to tml_emvco_Config_t */

/*
 * TML Deferred Callback structure used to invoke Upper layer Callback function.
 */
typedef struct {
  /* Deferred callback function to be invoked */
  defer_func_pointer_t p_def_call;
  /* Source identifier
   *
   * Identifier of the source which posted the message
   */
  uint32_t dw_msg_posted_thread;
  /** Actual Message
   *
   * This is passed as a parameter passed to the deferred callback function
   * pDef_call. */
  void *pParams;
} tml_emvco_defer_msg_t; /* DeferMsg structure passed to User Thread */

/* Function declarations */

/**
 * @brief           Provides initialization of TML layer and hardware interface
 *                  Configures given hardware interface and sends handle to the
 *                  caller
 *
 * @param[in]       pConfig - TML configuration details as provided by the upper
 *                            layer
 *
 * @return          NFC status:
 *                  EMVCO_STATUS_SUCCESS - initialization successful
 *                  EMVCO_STATUS_INVALID_PARAMETER - at least one parameter is
 *                                                invalid
 *                  EMVCO_STATUS_FAILED - initialization failed (for example,
 *                                     unable to open hardware interface)
 *                  EMVCO_STATUS_INVALID_DEVICE - device has not been opened or
 *                  has been disconnected
 *
 */
EMVCO_STATUS tml_init(ptml_emvco_Config_t pConfig);
/**
 * @brief       Uninitializes TML layer and hardware interface
 *
 * @param[in]       None
 *
 *  @return          NFC status:
 *                  EMVCO_STATUS_SUCCESS - TML configuration released
 * successfully
 *                  EMVCO_STATUS_INVALID_PARAMETER - at least one parameter is
 *                                                invalid
 *                  EMVCO_STATUS_FAILED - un-initialization failed (example:
 * unable
 *                                     to close interface)
 *
 */
EMVCO_STATUS tml_shutdown(void);

/**
 *
 * @brief       wrapper function  for shutdown  and cleanup of resources
 *
 * @param[in]       None
 *
 * @return           EMVCO_STATUS
 *
 */
EMVCO_STATUS tml_shutdown_cleanup();

/**
 *
 * @brief      Clears all handles opened during TML initialization
 *
 * @param[in]       None
 *
 * @return          None
 *
 */
void tml_cleanup(void);

/**
 *
 * @brief      Asynchronously writes given data block to hardware
 *                  interface/driver. Enables writer thread if there are no
 *                  write requests pending. Returns successfully once writer
 *                  thread completes write operation. Notifies upper layer using
 *                  callback mechanism.
 *
 *                  NOTE:
 *                  * it is important to post a message with id
 *                    TMLNFC_WRITE_MESSAGE to IntegrationThread after data
 *                    has been written to PN72X
 *                  * if CRC needs to be computed, then input buffer should be
 *                    capable to store two more bytes apart from length of
 *                    packet
 *
 * @param[in]       p_buffer - data to be sent
 * @param[in]       w_length - length of data buffer
 * @param[in]       pTmlWriteComplete - pointer to the function to be invoked
 *                                      upon completion
 * @param[in]       p_context - context provided by upper layer
 *
 * @return          NFC status:
 *                  EMVCO_STATUS_PENDING - command is yet to be processed
 *                  EMVCO_STATUS_INVALID_PARAMETER - at least one parameter is
 *                                                invalid
 *                  EMVCO_STATUS_BUSY - write request is already in progress
 *
 */
EMVCO_STATUS tml_write(uint8_t *p_buffer, uint16_t w_length);
/**
 *
 * @brief           Asynchronously reads data from the driver
 *                  Number of bytes to be read and buffer are passed by upper
 *                  layer.
 *                  Enables reader thread if there are no read requests pending
 *                  Returns successfully once read operation is completed
 *                  Notifies upper layer using callback mechanism
 *
 * @param[in]       p_buffer - data to be sent
 * @param[in]       w_length - length of data buffer
 * @param[in]       pTmlWriteComplete  pointer to the function to be invoked
 upon
 * completion
 * @param[in]       p_context        context provided by upper layer
 *
 * @return          NFC status:
 *                  EMVCO_STATUS_PENDING - command is yet to be processed
 *                  EMVCO_STATUS_INVALID_PARAMETER - at least one parameter is
 *                                                invalid
 *                  EMVCO_STATUS_BUSY - read request is already in progress

 *
 *
 */
EMVCO_STATUS tml_read(uint8_t *p_buffer, uint16_t w_length,
                      transact_completion_callback_t pTmlReadComplete,
                      void *p_context);
/**
 *
 * @brief       Aborts pending read request (if any)
 *
 * @param[in]        None
 *
 * @return           NFC status:
 *                  EMVCO_STATUS_SUCCESS - ongoing read operation aborted
 *                  EMVCO_STATUS_INVALID_PARAMETER - at least one parameter is
 *                                                invalid
 *                  EMVCO_STATUS_NOT_INITIALIZED - TML layer is not initialized
 *                  EMVCO_STATUS_BOARD_COMMUNICATION_ERROR - unable to cancel
 *read
 *                                                        operation
 *
 */
EMVCO_STATUS tml_read_abort(void);
/**
 * @brief       Resets device when insisted by upper layer
 *                  Number of bytes to be read and buffer are passed by upper
 *                  layer
 *                  Enables reader thread if there are no read requests pending
 *                  Returns successfully once read operation is completed
 *                  Notifies upper layer using callback mechanism
 *
 * @param[in]       eControlCode       - control code for a specific operation
 *
 * @return           NFC status:
 *                  EMVCO_STATUS_SUCCESS  - ioctl command completed successfully
 *                  EMVCO_STATUS_FAILED   - ioctl command request failed
 *
 */
EMVCO_STATUS tml_ioctl(emvco_control_code_t eControlCode);

/**
 * @brief       Updates the callback to be invoked after read completed
 *
 * @param[in]       pTmlReadComplete - pointer to the function to be invoked
 *                                     upon completion of read operation
 *
 * @return           NFC status:
 *                  EMVCO_STATUS_SUCCESS - if TmlNfc context available
 *                  EMVCO_STATUS_FAILED - otherwise
 *
 */
EMVCO_STATUS tml_update_read_complete_callback(
    transact_completion_callback_t pTmlReadComplete);

/**
 * @brief       Posts message on upper layer thread
 *                  upon successful read or write operation
 *
 * @param[in]       dwThreadId  - id of the thread posting message
 *                  ptWorkerMsg - message to be posted
 *
 * @return           None
 *
 */
void tml_deferred_call(uintptr_t dwThreadId, lib_emvco_message_t *ptWorkerMsg);

/** @}*/
#endif /*  _EMVCO_TML_H_  */
