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

/** \addtogroup EMVCO_STACK_OSAL_API_INTERFACE
 *  @{
 */

#ifndef _OSAL_TIMER_H_
#define _OSAL_TIMER_H_
#include <emvco_common.h>
/*
************************* Include Files ****************************************
*/

/**
 * @brief Timer callback interface which will be called once registered timer
 * time out expires.
 *  @param[in]       TimerId  - Timer Id for which callback is called.
 *  @param[in]       p_context - Parameter to be passed to the callback function
 */
typedef void (*p_osal_emvco_timer_callback_t)(uint32_t TimerId,
                                              void *p_context);

/*
 * The Timer could not be created due to a
 * system error */
#define PH_OSALNFC_TIMER_CREATE_ERROR (0X00E0)

/*
 * The Timer could not be started due to a
 * system error or invalid handle */
#define PH_OSALNFC_TIMER_START_ERROR (0X00E1)

/*
 * The Timer could not be stopped due to a
 * system error or invalid handle */
#define PH_OSALNFC_TIMER_STOP_ERROR (0X00E2)

/*
 * The Timer could not be deleted due to a
 * system error or invalid handle */
#define PH_OSALNFC_TIMER_DELETE_ERROR (0X00E3)

/*
 * Invalid timer ID type.This ID used indicate timer creation is failed */
#define PH_OSALNFC_TIMER_ID_INVALID (0xFFFF)

/*
 * OSAL timer message .This message type will be posted to
 * calling application thread.*/
#define PH_OSALNFC_TIMER_MSG (0x315)

/*
***************************Globals,Structure and Enumeration ******************
*/

/**
 *
 *
 * @brief           Creates a timer which shall call back the specified function
 *                  when the timer expires. Fails if OSAL module is not
 *                  initialized or timers are already occupied
 *
 * @param[in]       None
 *
 * @return          TimerId
 *                  TimerId value of PH_OSALEMVCO_TIMER_ID_INVALID indicates
 * that timer is not created
 *
 */
uint32_t osal_timer_create(void);

/**
 *
 *
 * @brief           Starts the requested, already created, timer.
 *                  If the timer is already running, timer stops and restarts
 *                  with the new timeout value and new callback function in case
 *                  any ??????
 *                  Creates a timer which shall call back the specified function
 *                  when the timer expires
 *
 * @param[in]       dwTimerId - valid timer ID obtained during timer creation
 *                  dwRegTimeCnt - requested timeout in milliseconds
 *                  pApplication_callback - application callback interface to be
 *                                          called when timer expires
 *                  pContext - caller context, to be passed to the application
 *                             callback function
 *
 * @return          EMVCO status:
 *                  EMVCOSTATUS_SUCCESS - the operation was successful
 *                  EMVCOSTATUS_NOT_INITIALISED - OSAL Module is not initialized
 *                  EMVCOSTATUS_INVALID_PARAMETER - invalid parameter passed to
 *                                                the function
 *                  PH_OSALEMVCO_TIMER_START_ERROR - timer could not be created
 *                                                 due to system error
 *
 */
EMVCO_STATUS
osal_timer_start(uint32_t dw_timer_id, uint32_t dwRegTimeCnt,
                 p_osal_emvco_timer_callback_t pApplication_callback,
                 void *p_context);
/**
 *
 * @brief           Stops already started timer
 *                  Allows to stop running timer. In case timer is stopped,
 *                  timer callback will not be notified any more
 *
 * @param[in]       dwTimerId - valid timer ID obtained during timer creation
 *
 * @return          EMVCO status:
 *                  EMVCOSTATUS_SUCCESS - the operation was successful
 *                  EMVCOSTATUS_NOT_INITIALISED - OSAL Module is not initialized
 *                  EMVCOSTATUS_INVALID_PARAMETER - invalid parameter passed to
 *                                                the function
 *                  PH_OSALEMVCO_TIMER_STOP_ERROR - timer could not be stopped
 * due to system error
 *
 */
EMVCO_STATUS osal_timer_stop(uint32_t dw_timer_id);

/**
 *
 * @brief           Deletes previously created timer
 *                  Allows to delete previously created timer. In case timer is
 *                  running, it is first stopped and then deleted
 *
 * @param[in]       dwTimerId - valid timer ID obtained during timer creation
 *
 * @return          EMVCO status:
 *                  EMVCOSTATUS_SUCCESS - the operation was successful
 *                  EMVCOSTATUS_NOT_INITIALISED - OSAL Module is not initialized
 *                  EMVCOSTATUS_INVALID_PARAMETER - invalid parameter passed to
 *                                                the function
 *                  PH_OSALEMVCO_TIMER_DELETE_ERROR - timer could not be stopped
 *                                                  due to system error
 *
 */
EMVCO_STATUS osal_timer_delete(uint32_t dw_timer_id);

/**
 *
 *
 * @brief           Deletes all previously created timers
 *                  Allows to delete previously created timers. In case timer is
 *                  running, it is first stopped and then deleted
 *
 * @param[in]        none
 *
 * @return          None
 *
 */
void osal_timer_cleanup(void);

/**
 *
 * @brief      Find an available timer id
 *
 * @param[in]       void
 *
 * @return          Available timer id
 *
 */
uint32_t osal_check_for_available_timer(void);

/**
 *
 * @brief       Checks the requested timer is present or not
 *
 * @param[in]       pObjectHandle - timer context
 *
 * @return          EMVCOSTATUS_SUCCESS if found
 *                  Other value if not found
 *
 */
EMVCO_STATUS osal_check_timer_presence(void *pobject_handle);

/** @}*/
#endif /* _OSAL_TIMER_H_ */
