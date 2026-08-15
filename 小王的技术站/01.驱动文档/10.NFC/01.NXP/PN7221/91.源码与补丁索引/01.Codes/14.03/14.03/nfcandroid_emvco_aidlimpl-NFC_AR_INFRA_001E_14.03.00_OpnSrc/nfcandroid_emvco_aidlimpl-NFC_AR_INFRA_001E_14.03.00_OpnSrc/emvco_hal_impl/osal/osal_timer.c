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
 * OSAL Implementation for Timers.
 */

#include <emvco_dm.h>
#include <emvco_log.h>
#include <emvco_osal_common.h>
#include <emvco_types.h>
#include <osal_message_queue_lib.h>
#include <osal_timer.h>
#include <signal.h>

#define EMVCO_MAX_TIMER (5U)
static osal_emvco_timer_handle_t apTimerInfo[EMVCO_MAX_TIMER];

extern nci_hal_ctrl_t nci_hal_ctrl;

/*
 * Defines the base address for generating timerid.
 */
#define EMVCO_TIMER_BASE_ADDRESS (100U)

/*
 *  Defines the value for invalid timerid returned during timeSetEvent
 */
#define EMVCO_TIMER_ID_ZERO (0x00)

/*
 * Invalid timer ID type. This ID used indicate timer creation is failed */
#define EMVCO_TIMER_ID_INVALID (0xFFFF)

/* Forward declarations */
static void post_timer_msg(lib_emvco_message_t *pMsg);
static void osal_emvco_deferred_call(void *pParams);
static void osal_emvco_timer_expired(union sigval sv);

/*
 *************************** Function Definitions ******************************
 */

/*******************************************************************************
**
** Function         osal_timer_create
**
** Description      Creates a timer which shall call back the specified function
**                  when the timer expires. Fails if OSAL module is not
**                  initialized or timers are already occupied
**
** Parameters       None
**
** Returns          timer_id
**                  timer_id value of PH_OSALNFC_TIMER_ID_INVALID indicates that
**                  timer is not created
**
*******************************************************************************/
uint32_t osal_timer_create(void) {
  /* dw_timer_id is also used as an index at which timer object can be stored */
  uint32_t dw_timer_id = PH_OSALNFC_TIMER_ID_INVALID;
  static struct sigevent se;
  osal_emvco_timer_handle_t *pTimerHandle;
  /* Timer needs to be initialized for timer usage */

  se.sigev_notify = SIGEV_THREAD;
  se.sigev_notify_function = osal_emvco_timer_expired;
  se.sigev_notify_attributes = NULL;
  dw_timer_id = osal_check_for_available_timer();

  /* Check whether timers are available, if yes create a timer handle structure
   */
  if ((EMVCO_TIMER_ID_ZERO != dw_timer_id) &&
      (dw_timer_id <= EMVCO_MAX_TIMER)) {
    pTimerHandle = (osal_emvco_timer_handle_t *)&apTimerInfo[dw_timer_id - 1];
    /* Build the Timer Id to be returned to Caller Function */
    dw_timer_id += EMVCO_TIMER_BASE_ADDRESS;
    se.sigev_value.sival_int = (int)dw_timer_id;
    /* Create POSIX timer */
    if (timer_create(CLOCK_REALTIME, &se, &(pTimerHandle->timer_handle)) ==
        -1) {
      dw_timer_id = EMVCO_TIMER_ID_INVALID;
    } else {
      /* Set the state to indicate timer is ready */
      pTimerHandle->eState = eTimerIdle;
      /* Store the Timer Id which shall act as flag during check for timer
       * availability */
      pTimerHandle->timer_id = dw_timer_id;
    }
  } else {
    dw_timer_id = EMVCO_TIMER_ID_INVALID;
  }

  /* Timer ID invalid can be due to Uninitialized state,Non availability of
   * Timer */
  return dw_timer_id;
}

/*******************************************************************************
**
** Function         osal_timer_start
**
** Description      Starts the requested, already created, timer.
**                  If the timer is already running, timer stops and restarts
**                  with the new timeout value and new callback function in case
**                  any ??????
**                  Creates a timer which shall call back the specified function
**                  when the timer expires
**
** Parameters       dw_timer_id - valid timer ID obtained during timer creation
**                  dwRegTimeCnt - requested timeout in milliseconds
**                  pApplication_callback - application callback interface to be
**                                          called when timer expires
**                  p_context - caller context, to be passed to the application
**                             callback function
**
** Returns          NFC status:
**                  EMVCO_STATUS_SUCCESS - the operation was successful
**                  EMVCO_STATUS_NOT_INITIALISED - OSAL Module is not
*initialized
**                  EMVCO_STATUS_INVALID_PARAMETER - invalid parameter passed to
**                                                the function
**                  PH_OSALNFC_TIMER_START_ERROR - timer could not be created
**                                                 due to system error
**
*******************************************************************************/
EMVCO_STATUS
osal_timer_start(uint32_t dw_timer_id, uint32_t dwRegTimeCnt,
                 p_osal_emvco_timer_callback_t pApplication_callback,
                 void *p_context) {
  EMVCO_STATUS wStartStatus = EMVCO_STATUS_SUCCESS;

  struct itimerspec its;
  uint32_t dwIndex;
  osal_emvco_timer_handle_t *pTimerHandle;
  /* Retrieve the index at which the timer handle structure is stored */
  dwIndex = dw_timer_id - EMVCO_TIMER_BASE_ADDRESS - 0x01;
  pTimerHandle = (osal_emvco_timer_handle_t *)&apTimerInfo[dwIndex];
  /* OSAL Module needs to be initialized for timer usage */
  /* Check whether the handle provided by user is valid */
  if ((dwIndex < EMVCO_MAX_TIMER) && (0x00 != pTimerHandle->timer_id) &&
      (NULL != pApplication_callback)) {
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = dwRegTimeCnt / 1000;
    its.it_value.tv_nsec = 1000000 * (dwRegTimeCnt % 1000);
    if (its.it_value.tv_sec == 0 && its.it_value.tv_nsec == 0) {
      /* This would inadvertently stop the timer*/
      its.it_value.tv_nsec = 1;
    }
    pTimerHandle->Application_callback = pApplication_callback;
    pTimerHandle->p_context = p_context;
    pTimerHandle->eState = eTimerRunning;
    /* Arm the timer */
    if ((timer_settime(pTimerHandle->timer_handle, 0, &its, NULL)) == -1) {
      wStartStatus = EMVCOSTVAL(CID_EMVCO_OSAL, PH_OSALNFC_TIMER_START_ERROR);
    }
  } else {
    wStartStatus = EMVCOSTVAL(CID_EMVCO_OSAL, EMVCO_STATUS_INVALID_PARAMETER);
  }

  return wStartStatus;
}

/*******************************************************************************
**
** Function         osal_timer_stop
**
** Description      Stops already started timer
**                  Allows to stop running timer. In case timer is stopped,
**                  timer callback will not be notified any more
**
** Parameters       dw_timer_id - valid timer ID obtained during timer creation
**
** Returns          NFC status:
**                  EMVCO_STATUS_SUCCESS - the operation was successful
**                  EMVCO_STATUS_NOT_INITIALISED - OSAL Module is not
*initialized
**                  EMVCO_STATUS_INVALID_PARAMETER - invalid parameter passed to
**                                                the function
**                  PH_OSALNFC_TIMER_STOP_ERROR - timer could not be stopped due
**                                                to system error
**
*******************************************************************************/
EMVCO_STATUS osal_timer_stop(uint32_t dw_timer_id) {
  EMVCO_STATUS wStopStatus = EMVCO_STATUS_SUCCESS;
  static struct itimerspec its = {{0, 0}, {0, 0}};

  uint32_t dwIndex;
  osal_emvco_timer_handle_t *pTimerHandle;
  dwIndex = dw_timer_id - EMVCO_TIMER_BASE_ADDRESS - 0x01;
  pTimerHandle = (osal_emvco_timer_handle_t *)&apTimerInfo[dwIndex];
  /* OSAL Module and Timer needs to be initialized for timer usage */
  /* Check whether the timer_id provided by user is valid */
  if ((dwIndex < EMVCO_MAX_TIMER) && (0x00 != pTimerHandle->timer_id) &&
      (pTimerHandle->eState != eTimerIdle)) {
    /* Stop the timer only if the callback has not been invoked */
    if (pTimerHandle->eState == eTimerRunning) {
      if ((timer_settime(pTimerHandle->timer_handle, 0, &its, NULL)) == -1) {
        wStopStatus = EMVCOSTVAL(CID_EMVCO_OSAL, PH_OSALNFC_TIMER_STOP_ERROR);
      } else {
        /* Change the state of timer to Stopped */
        pTimerHandle->eState = eTimerStopped;
      }
    }
  } else {
    wStopStatus = EMVCOSTVAL(CID_EMVCO_OSAL, EMVCO_STATUS_INVALID_PARAMETER);
  }

  return wStopStatus;
}

/*******************************************************************************
**
** Function         osal_timer_delete
**
** Description      Deletes previously created timer
**                  Allows to delete previously created timer. In case timer is
**                  running, it is first stopped and then deleted
**
** Parameters       dw_timer_id - valid timer ID obtained during timer creation
**
** Returns          NFC status:
**                  EMVCO_STATUS_SUCCESS - the operation was successful
**                  EMVCO_STATUS_NOT_INITIALISED - OSAL Module is not
*initialized
**                  EMVCO_STATUS_INVALID_PARAMETER - invalid parameter passed to
**                                                the function
**                  PH_OSALNFC_TIMER_DELETE_ERROR - timer could not be stopped
**                                                  due to system error
**
*******************************************************************************/
EMVCO_STATUS osal_timer_delete(uint32_t dw_timer_id) {
  EMVCO_STATUS wDeleteStatus = EMVCO_STATUS_SUCCESS;

  uint32_t dwIndex;
  osal_emvco_timer_handle_t *pTimerHandle;
  dwIndex = dw_timer_id - EMVCO_TIMER_BASE_ADDRESS - 0x01;
  pTimerHandle = (osal_emvco_timer_handle_t *)&apTimerInfo[dwIndex];
  /* OSAL Module and Timer needs to be initialized for timer usage */

  /* Check whether the timer_id passed by user is valid and Deregistering of
   * timer is successful */
  if ((dwIndex < EMVCO_MAX_TIMER) && (0x00 != pTimerHandle->timer_id) &&
      (EMVCO_STATUS_SUCCESS == osal_check_timer_presence(pTimerHandle))) {
    /* Cancel the timer before deleting */
    if (timer_delete(pTimerHandle->timer_handle) == -1) {
      wDeleteStatus = EMVCOSTVAL(CID_EMVCO_OSAL, PH_OSALNFC_TIMER_DELETE_ERROR);
    }
    /* Clear Timer structure used to store timer related data */
    memset(pTimerHandle, (uint8_t)0x00, sizeof(osal_emvco_timer_handle_t));
  } else {
    wDeleteStatus = EMVCOSTVAL(CID_EMVCO_OSAL, EMVCO_STATUS_INVALID_PARAMETER);
  }
  return wDeleteStatus;
}

/*******************************************************************************
**
** Function         osal_timer_cleanup
**
** Description      Deletes all previously created timers
**                  Allows to delete previously created timers. In case timer is
**                  running, it is first stopped and then deleted
**
** Parameters       None
**
** Returns          None
**
*******************************************************************************/
void osal_timer_cleanup(void) {
  /* Delete all timers */
  uint32_t dwIndex;
  osal_emvco_timer_handle_t *pTimerHandle;
  for (dwIndex = 0; dwIndex < EMVCO_MAX_TIMER; dwIndex++) {
    pTimerHandle = (osal_emvco_timer_handle_t *)&apTimerInfo[dwIndex];
    /* OSAL Module and Timer needs to be initialized for timer usage */

    /* Check whether the timer_id passed by user is valid and Deregistering of
     * timer is successful */
    if ((0x00 != pTimerHandle->timer_id) &&
        (EMVCO_STATUS_SUCCESS == osal_check_timer_presence(pTimerHandle))) {
      /* Cancel the timer before deleting */
      if (timer_delete(pTimerHandle->timer_handle) == -1) {
        LOG_EMVCO_TML_E("timer %d delete error!", dwIndex);
      }
      /* Clear Timer structure used to store timer related data */
      memset(pTimerHandle, (uint8_t)0x00, sizeof(osal_emvco_timer_handle_t));
    }
  }

  return;
}

/*******************************************************************************
**
** Function         osal_emvco_deferred_call
**
** Description      Invokes the timer callback function after timer expiration.
**                  Shall invoke the callback function registered by the timer
**                  caller function
**
** Parameters       pParams - parameters indicating the ID of the timer
**
** Returns          None                -
**
*******************************************************************************/
static void osal_emvco_deferred_call(void *pParams) {
  /* Retrieve the timer id from the parameter */
  unsigned long dwIndex;
  osal_emvco_timer_handle_t *pTimerHandle;
  if (NULL != pParams) {
    /* Retrieve the index at which the timer handle structure is stored */
    dwIndex = (uintptr_t)pParams - EMVCO_TIMER_BASE_ADDRESS - 0x01;
    pTimerHandle = (osal_emvco_timer_handle_t *)&apTimerInfo[dwIndex];
    if (pTimerHandle->Application_callback != NULL) {
      /* Invoke the callback function with osal Timer ID */
      pTimerHandle->Application_callback((uintptr_t)pParams,
                                         pTimerHandle->p_context);
    }
  }

  return;
}

/*******************************************************************************
**
** Function         post_timer_msg
**
** Description      Posts message on the user thread
**                  Shall be invoked upon expiration of a timer
**                  Shall post message on user thread through which timer
**                  callback function shall be invoked
**
** Parameters       pMsg - pointer to the message structure posted on user
**                         thread
**
** Returns          None
**
*******************************************************************************/
static void post_timer_msg(lib_emvco_message_t *pMsg) {
  (void)osal_msg_snd(
      nci_hal_ctrl.gDrvCfg
          .n_client_id /*gpphOsalNfc_Context->dwCallbackThreadID*/,
      pMsg, 0);

  return;
}

/*******************************************************************************
**
** Function         osal_emvco_timer_expired
**
** Description      posts message upon expiration of timer
**                  Shall be invoked when any one timer is expired
**                  Shall post message on user thread to invoke respective
**                  callback function provided by the caller of Timer function
**
** Returns          None
**
*******************************************************************************/
static void osal_emvco_timer_expired(union sigval sv) {
  uint32_t dwIndex;
  osal_emvco_timer_handle_t *pTimerHandle;

  dwIndex = ((uint32_t)(sv.sival_int)) - EMVCO_TIMER_BASE_ADDRESS - 0x01;
  pTimerHandle = (osal_emvco_timer_handle_t *)&apTimerInfo[dwIndex];
  /* Timer is stopped when callback function is invoked */
  pTimerHandle->eState = eTimerStopped;

  pTimerHandle->tDeferedCallInfo.pDeferedCall = &osal_emvco_deferred_call;
  pTimerHandle->tDeferedCallInfo.pParam = (void *)((intptr_t)(sv.sival_int));

  pTimerHandle->tOsalMessage.e_msgType = EMVCO_DEFERRED_CALL_MSG;
  pTimerHandle->tOsalMessage.p_msg_data =
      (void *)&pTimerHandle->tDeferedCallInfo;

  /* Post a message on the queue to invoke the function */
  post_timer_msg((lib_emvco_message_t *)&pTimerHandle->tOsalMessage);

  return;
}

/*******************************************************************************
**
** Function         osal_check_for_available_timer
**
** Description      Find an available timer id
**
** Parameters       void
**
** Returns          Available timer id
**
*******************************************************************************/
uint32_t osal_check_for_available_timer(void) {
  /* Variable used to store the index at which the object structure details
     can be stored. Initialize it as not available. */
  uint32_t dwIndex = 0x00;
  uint32_t dwRetval = 0x00;

  /* Check whether Timer object can be created */
  for (dwIndex = 0x00; ((dwIndex < EMVCO_MAX_TIMER) && (0x00 == dwRetval));
       dwIndex++) {
    if (!(apTimerInfo[dwIndex].timer_id)) {
      dwRetval = (dwIndex + 0x01);
    }
  }

  return (dwRetval);
}

/*******************************************************************************
**
** Function         osal_check_timer_presence
**
** Description      Checks the requested timer is present or not
**
** Parameters       pobject_handle - timer context
**
** Returns          EMVCO_STATUS_SUCCESS if found
**                  Other value if not found
**
*******************************************************************************/
EMVCO_STATUS osal_check_timer_presence(void *pobject_handle) {
  uint32_t dwIndex;
  EMVCO_STATUS wRegisterStatus = EMVCO_STATUS_INVALID_PARAMETER;

  for (dwIndex = 0x00; ((dwIndex < EMVCO_MAX_TIMER) &&
                        (wRegisterStatus != EMVCO_STATUS_SUCCESS));
       dwIndex++) {
    /* For Timer, check whether the requested handle is present or not */
    if (((&apTimerInfo[dwIndex]) ==
         (osal_emvco_timer_handle_t *)pobject_handle) &&
        (apTimerInfo[dwIndex].timer_id)) {
      wRegisterStatus = EMVCO_STATUS_SUCCESS;
    }
  }
  return wRegisterStatus;
}
