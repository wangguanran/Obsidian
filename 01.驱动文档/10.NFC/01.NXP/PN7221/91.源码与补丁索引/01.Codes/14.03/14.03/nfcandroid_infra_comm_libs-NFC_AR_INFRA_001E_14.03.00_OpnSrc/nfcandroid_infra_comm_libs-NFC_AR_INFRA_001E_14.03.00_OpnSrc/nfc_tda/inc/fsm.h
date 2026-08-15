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
#ifndef CT_NFC_FSM_H_
#define CT_NFC_FSM_H_

#include "pal.h"
/** \addtogroup NFC_FSM_API_INTERFACE
 *  @brief  has states, events and functions needed to handle FSM
 *  @{
 */

/**
 * @brief  updates the state of TDA
 *
 **/
void update_state(system_state_t state);

/**
 * @brief  handles the incoming event based on the current state
 *         and calls the respective event handler to process the
 *         event
 *
 **/
fp_event_handler_t handle_event(system_event_t sys_evt);

NFC_STATUS init_nfcee_discover(void *);
NFC_STATUS init_discover_tda(void *);
NFC_STATUS init_open_tda(void *);
NFC_STATUS init_core_conn_create(void *);
NFC_STATUS init_transceive(void *);
NFC_STATUS init_close_tda(void *);
NFC_STATUS init_core_conn_close(void *);
NFC_STATUS discovered_nfcee_discover(void *);
NFC_STATUS discovered_discover_tda(void *);
NFC_STATUS discovered_open_tda(void *);
NFC_STATUS discovered_core_conn_create(void *);
NFC_STATUS discovered_transceive(void *);
NFC_STATUS discovered_close_tda(void *);
NFC_STATUS discovered_core_conn_close(void *);
NFC_STATUS mode_set_enabled_nfcee_discover(void *);
NFC_STATUS mode_set_enabled_discover_tda(void *);
NFC_STATUS mode_set_enabled_open_tda(void *);
NFC_STATUS mode_set_enabled_core_conn_create(void *);
NFC_STATUS mode_set_enabled_transceive(void *);
NFC_STATUS mode_set_enabled_close_tda(void *);
NFC_STATUS mode_set_enabled_core_conn_close(void *);
NFC_STATUS core_conn_created_nfcee_discover(void *);
NFC_STATUS core_conn_created_discover_tda(void *);
NFC_STATUS core_conn_created_open_tda(void *);
NFC_STATUS core_conn_created_core_conn_create(void *);
NFC_STATUS core_conn_created_transceive(void *);
NFC_STATUS core_conn_created_close_tda(void *);
NFC_STATUS core_conn_created_core_conn_close(void *);
NFC_STATUS core_conn_closed_nfcee_discover(void *);
NFC_STATUS core_conn_closed_discover_tda(void *);
NFC_STATUS core_conn_closed_open_tda(void *);
NFC_STATUS core_conn_closed_core_conn_create(void *);
NFC_STATUS core_conn_closed_transceive(void *);
NFC_STATUS core_conn_closed_close_tda(void *);
NFC_STATUS core_conn_closed_core_conn_close(void *);
/** @}*/
#endif /* CT_NFC_FSM_H_ */
