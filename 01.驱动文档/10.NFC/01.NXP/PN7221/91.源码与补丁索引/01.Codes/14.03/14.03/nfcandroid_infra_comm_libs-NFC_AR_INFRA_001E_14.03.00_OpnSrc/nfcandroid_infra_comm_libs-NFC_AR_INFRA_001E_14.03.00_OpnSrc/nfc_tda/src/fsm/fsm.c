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
#include "fsm.h"
#include "nci_snd.h"
#include "pal.h"
#include "tda.h"
#include "tda_nci_defs.h"

extern tda_control_t g_tda_ctrl;

state_machine_t state_machine[] = {
    {INIT_STATE, NFCEE_DISCOVER_EVENT, init_nfcee_discover},
    {INIT_STATE, DISCOVER_TDA_EVENT, init_discover_tda},
    {INIT_STATE, OPEN_TDA_EVENT, init_open_tda},
    {INIT_STATE, CORE_CONN_CREATE_EVENT, init_core_conn_create},
    {INIT_STATE, TRANSCEIVE_EVENT, init_transceive},
    {INIT_STATE, CLOSE_TDA_EVENT, init_close_tda},
    {INIT_STATE, CORE_CONN_CLOSE_EVENT, init_core_conn_close},
    {DISCOVERED_STATE, NFCEE_DISCOVER_EVENT, discovered_nfcee_discover},
    {DISCOVERED_STATE, DISCOVER_TDA_EVENT, discovered_discover_tda},
    {DISCOVERED_STATE, OPEN_TDA_EVENT, discovered_open_tda},
    {DISCOVERED_STATE, CORE_CONN_CREATE_EVENT, discovered_core_conn_create},
    {DISCOVERED_STATE, TRANSCEIVE_EVENT, discovered_transceive},
    {DISCOVERED_STATE, CLOSE_TDA_EVENT, discovered_close_tda},
    {DISCOVERED_STATE, CORE_CONN_CLOSE_EVENT, discovered_core_conn_close},
    {MODE_SET_ENABLED_STATE, NFCEE_DISCOVER_EVENT,
     mode_set_enabled_nfcee_discover},
    {MODE_SET_ENABLED_STATE, DISCOVER_TDA_EVENT, mode_set_enabled_discover_tda},
    {MODE_SET_ENABLED_STATE, OPEN_TDA_EVENT, mode_set_enabled_open_tda},
    {MODE_SET_ENABLED_STATE, CORE_CONN_CREATE_EVENT,
     mode_set_enabled_core_conn_create},
    {MODE_SET_ENABLED_STATE, TRANSCEIVE_EVENT, mode_set_enabled_transceive},
    {MODE_SET_ENABLED_STATE, CLOSE_TDA_EVENT, mode_set_enabled_close_tda},
    {MODE_SET_ENABLED_STATE, CORE_CONN_CLOSE_EVENT,
     mode_set_enabled_core_conn_close},
    {CORE_CONN_CREATED_STATE, NFCEE_DISCOVER_EVENT,
     core_conn_created_nfcee_discover},
    {CORE_CONN_CREATED_STATE, DISCOVER_TDA_EVENT,
     core_conn_created_discover_tda},
    {CORE_CONN_CREATED_STATE, OPEN_TDA_EVENT, core_conn_created_open_tda},
    {CORE_CONN_CREATED_STATE, CORE_CONN_CREATE_EVENT,
     core_conn_created_core_conn_create},
    {CORE_CONN_CREATED_STATE, TRANSCEIVE_EVENT, core_conn_created_transceive},
    {CORE_CONN_CREATED_STATE, CLOSE_TDA_EVENT, core_conn_created_close_tda},
    {CORE_CONN_CREATED_STATE, CORE_CONN_CLOSE_EVENT,
     core_conn_created_core_conn_close},
    {CORE_CONN_CLOSED_STATE, NFCEE_DISCOVER_EVENT,
     core_conn_closed_nfcee_discover},
    {CORE_CONN_CLOSED_STATE, DISCOVER_TDA_EVENT, core_conn_closed_discover_tda},
    {CORE_CONN_CLOSED_STATE, OPEN_TDA_EVENT, core_conn_closed_open_tda},
    {CORE_CONN_CLOSED_STATE, CORE_CONN_CREATE_EVENT,
     core_conn_closed_core_conn_create},
    {CORE_CONN_CLOSED_STATE, TRANSCEIVE_EVENT, core_conn_closed_transceive},
    {CORE_CONN_CLOSED_STATE, CLOSE_TDA_EVENT, core_conn_closed_close_tda},
    {CORE_CONN_CLOSED_STATE, CORE_CONN_CLOSE_EVENT,
     core_conn_closed_core_conn_close},
};

/**
 * @brief  handles the incoming event based on the current state
 *         and calls the respective event handler to process the
 *         event
 *
 **/
fp_event_handler_t handle_event(system_event_t new_event) {
  int index = (g_tda_ctrl.tda_state * LAST_EVENT) + new_event;
  OSAL_LOG_NFCHAL_D("%s g_tda_ctrl.tda_state:%d, new_event:%d,index:%d, "
                    "state_machine[index].event:%d",
                    __func__, g_tda_ctrl.tda_state, new_event, index,
                    state_machine[index].event);
  if ((g_tda_ctrl.tda_state < LAST_STATE) && (new_event < LAST_EVENT) &&
      (state_machine[index].event == new_event) &&
      (state_machine[index].handler != NULL)) {
    return state_machine[index].handler;
  } else {
    OSAL_LOG_NFCHAL_D("%s Invalid event,returning NULL", __func__);
  }
  return NULL;
}

/**
 * @brief  updates the state of TDA
 *
 **/
void update_state(system_state_t state) {
  OSAL_LOG_NFCHAL_D("%s previous_state:%d, current_state:%d", __func__,
                      g_tda_ctrl.tda_state, state);
  g_tda_ctrl.tda_state = state;
}
/**
 * @brief   sends the nfcee discover command returs the response
 *
 * @return  NFC_STATUS indicates success or failure
 *          Refer nfc_status.h file for more specific error code
 *          incase of failure.
 **/
NFC_STATUS init_nfcee_discover(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  NFC_STATUS status = NFC_STATUS_FAIL;
  status = send_nfcee_discover();
  if (status == NFC_STATUS_SUCCESS) {
    OSAL_LOG_NFCHAL_D("%s updated state as DISCOVERED_STATE", __func__);
    update_state(DISCOVERED_STATE);
  }
  return status;
}
NFC_STATUS init_discover_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED;
}

NFC_STATUS init_open_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED;
}
NFC_STATUS init_core_conn_create(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED;
}
NFC_STATUS init_transceive(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED;
}

NFC_STATUS init_close_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED;
}

NFC_STATUS init_core_conn_close(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_INIT_NOT_COMPLETED;
}

/**
 * @brief         returns the TDA Info stored on TDA structure
 *                to the given TDA control
 *
 * @param[out]    tda_ctrl in which TDA info been copied
 * @return        NFC_STATUS indicates success or failure
 *                Refer nfc_status.h file for more specific error code
 *                incase of failure.
 **/
NFC_STATUS discover_tda_impl(void *tda_ctrl) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  NFC_STATUS status = NFC_STATUS_SUCCESS;
  tda_control_t *tda_control = (tda_control_t *)tda_ctrl;

  if (g_tda_ctrl.p_tda != NULL) {
    tda_control->p_tda = g_tda_ctrl.p_tda;
    tda_control->num_tda_supported = g_tda_ctrl.num_tda_supported;
    OSAL_LOG_NFCHAL_D("%s, g_tda_ctrl.num_tda_supported:%d", __func__,
                      g_tda_ctrl.num_tda_supported);
    status = NFC_STATUS_SUCCESS;
  } else {
    OSAL_LOG_NFCHAL_D("%s", __func__);
    status = NFC_STATUS_TDA_INIT_FAILED;
  }
  return status;
}

NFC_STATUS discovered_nfcee_discover(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY;
}

NFC_STATUS discovered_discover_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return discover_tda_impl(input);
}

/**
 * @brief         opens the TDA by sending modeset enable and core connection
 *                create command
 *
 * @param[inout]    tda_channel_pair_t contains the TDA ID as input
 *                  channel_num to send back the logical connection ID
 *
 * @return          NFC_STATUS indicates success or failure
 *                  Refer nfc_status.h file for more specific error code
 *                  incase of failure.
 **/
NFC_STATUS discovered_open_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  g_tda_ctrl.tda_ch_pr = (*(tda_channel_pair_t *)input);
  int8_t tda_id = g_tda_ctrl.tda_ch_pr.tda_id;
  OSAL_LOG_NFCHAL_D("%s tda_id:%02x", __func__, tda_id);
  NFC_STATUS status = send_nfcee_mode_set(tda_id, NCI_NFCEE_MD_ACTIVATE);
  if (status != NFC_STATUS_SUCCESS) {
    OSAL_LOG_NFCHAL_D("%s MODESET_ENABLE_FAILED", __func__);
    return status;
  }
  update_state(MODE_SET_ENABLED_STATE);
  fp_event_handler_t fp_event_handler = handle_event(CORE_CONN_CREATE_EVENT);
  status = fp_event_handler(&g_tda_ctrl.tda_ch_pr);
  if (status != NFC_STATUS_SUCCESS) {
    OSAL_LOG_NFCHAL_D("%s MODESET_FAILED", __func__);
    status = NFC_STATUS_CORE_CONN_CREATE_FAILED;
    return status;
  }
  return status;
}

/**
 * @brief         sends the core connection create command and return logical
 *connection on success
 *
 * @param[inout]    tda_channel_pair_t contains the TDA ID as input
 *                  channel_num to send back the logical connection ID
 *
 * @return          NFC_STATUS indicates success or failure
 *                  Refer nfc_status.h file for more specific error code
 *                  incase of failure.
 **/
NFC_STATUS core_conn_create_impl(void *input) {

  g_tda_ctrl.tda_ch_pr = (*(tda_channel_pair_t *)input);
  OSAL_LOG_NFCHAL_D("%s g_tda_ctrl.tda_ch_pr.tda_id:%02x", __func__, g_tda_ctrl.tda_ch_pr.tda_id);
  NFC_STATUS status = send_core_conn_create(g_tda_ctrl.tda_ch_pr.tda_id);
  g_tda_ctrl.tda_ch_pr.channel_num = get_tda_channel_num();
  OSAL_LOG_NFCHAL_D("%s status:%02x, channel_num:%02x \n", __func__, status,
                    g_tda_ctrl.tda_ch_pr.channel_num);
  if (status != NFC_STATUS_SUCCESS || g_tda_ctrl.tda_ch_pr.channel_num == INVALID_NUM) {
    OSAL_LOG_NFCHAL_D("%s CORE_CONN_CREATE_FAILED", __func__);
    status = NFC_STATUS_CORE_CONN_CREATE_FAILED;
    return status;
  }

  update_state(CORE_CONN_CREATED_STATE);
  return status;
}
NFC_STATUS discovered_core_conn_create(void *input) {
  return core_conn_create_impl(input);
}

NFC_STATUS discovered_transceive(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED;
}
NFC_STATUS discovered_close_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED;
}

NFC_STATUS discovered_core_conn_close(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_OPEN_NOT_COMPLETED;
}

NFC_STATUS mode_set_enabled_nfcee_discover(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY;
}
NFC_STATUS mode_set_enabled_discover_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return discover_tda_impl(input);
}

NFC_STATUS mode_set_enabled_open_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return core_conn_create_impl(input);
}

NFC_STATUS mode_set_enabled_core_conn_create(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return core_conn_create_impl(input);
}

NFC_STATUS mode_set_enabled_transceive(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_CORE_CONN_CREATE_NOT_COMPLETED;
}

/**
 * @brief         sends the mode set disable command and returns status
 *
 * @param[inout]    input ID of the TDA slot
 *
 * @return          NFC_STATUS indicates success or failure
 *                  Refer nfc_status.h file for more specific error code
 *                  incase of failure.
 **/
NFC_STATUS mode_set_disable_impl(void *input) {
  int8_t tda_id = *((int8_t *)input);
  OSAL_LOG_NFCHAL_D("%s tda_id:%02x", __func__, tda_id);
  NFC_STATUS status = send_nfcee_mode_set(tda_id, NCI_NFCEE_MD_DEACTIVATE);
  if (status != NFC_STATUS_SUCCESS) {
    OSAL_LOG_NFCHAL_D("%s MODE_SET_DISABLE_FAILED", __func__);
    status = NFC_STATUS_NFCEE_MODE_SET_DISABLE_FAILED;
    return status;
  }
  update_state(DISCOVERED_STATE);
  return status;
}

NFC_STATUS mode_set_enabled_close_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return mode_set_disable_impl(input);
}

NFC_STATUS mode_set_enabled_core_conn_close(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_CORE_CONN_CREATE_NOT_COMPLETED;
}

NFC_STATUS core_conn_created_nfcee_discover(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY;
}
NFC_STATUS core_conn_created_discover_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return discover_tda_impl(input);
}
NFC_STATUS core_conn_created_open_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_OPENED_ALREADY;
}
NFC_STATUS core_conn_created_core_conn_create(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_CORE_CONN_CREATED_ALREADY;
}
/**
 * @brief         sends the raw APDU to controller and returns the response APDU
 *on success
 *
 * @param[inout]    transceive_buffer_t containts command and response APUD
 *
 * @return          NFC_STATUS indicates success or failure
 *                  Refer nfc_status.h file for more specific error code
 *                  incase of failure.
 **/
NFC_STATUS core_conn_created_transceive(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  g_tda_ctrl.trans_buf = (*(transceive_buffer_t *)input);
  OSAL_LOG_NFCHAL_D("%s g_tda_ctrl.trans_buf.cmd_apdu->len:%d", __func__,
                     g_tda_ctrl.trans_buf.cmd_apdu->len);
  g_tda_ctrl.trans_buf.rsp_apdu->len = 0;
  NFC_STATUS status =
      send_nfc_ct_data(g_tda_ctrl.trans_buf.cmd_apdu->p_data, g_tda_ctrl.trans_buf.cmd_apdu->len);
  if (status != NFC_STATUS_SUCCESS) {
    OSAL_LOG_NFCHAL_D("%s NFC_STATUS_TRANSCEIVE_FAILED", __func__);
    status = NFC_STATUS_TRANSCEIVE_FAILED;
    return status;
  }
  OSAL_LOG_NFCHAL_D("%s resp len:%d\n", __func__,  g_tda_ctrl.trans_buf.rsp_apdu->len);
  if (g_tda_ctrl.trans_buf.rsp_apdu->len <= 0) {
    OSAL_LOG_NFCHAL_D("%s No data to copy", __func__);
  }
  return NFC_STATUS_SUCCESS;
}

/**
 * @brief         sends the core connection close command and return logical
 *connection on success
 *
 * @param[inout]    tda_channel_pair_t contains the TDA ID as input
 *
 * @return          NFC_STATUS indicates success or failure
 *                  Refer nfc_status.h file for more specific error code
 *                  incase of failure.
 **/
NFC_STATUS core_conn_close_tda_impl(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  int8_t tda_id = *((int *)input);
  int8_t channel_num = get_tda_channel_num();
  OSAL_LOG_NFCHAL_D("%s tda_id:%02x, channel_num:%02x", __func__, tda_id,
                    channel_num);
  if (channel_num == INVALID_NUM) {
    return NFC_STATUS_CORE_CONN_CLOSE_FAILED;
  }
  NFC_STATUS status = send_core_conn_close(channel_num);
  if (status != NFC_STATUS_SUCCESS) {
    OSAL_LOG_NFCHAL_D("%s CORE_CONN_CLOSE_FAILED", __func__);
    status = NFC_STATUS_CORE_CONN_CLOSE_FAILED;
    return status;
  }

  update_state(CORE_CONN_CLOSED_STATE);
  return status;
}
NFC_STATUS core_conn_created_close_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  NFC_STATUS status = core_conn_close_tda_impl(input);
  if (status == NFC_STATUS_SUCCESS) {
    int8_t tda_id = *((int *)input);
    fp_event_handler_t fp_event_handler = handle_event(CLOSE_TDA_EVENT);
    status = fp_event_handler(&tda_id);
  }
  return status;
}

NFC_STATUS core_conn_created_core_conn_close(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return core_conn_close_tda_impl(input);
}

NFC_STATUS core_conn_closed_nfcee_discover(void *input) {
  (void)input;
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return NFC_STATUS_INVALID_STATE_TDA_DISCOVERED_ALREADY;
}
NFC_STATUS core_conn_closed_discover_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return discover_tda_impl(input);
}
NFC_STATUS core_conn_closed_open_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return core_conn_create_impl(input);
}
NFC_STATUS core_conn_closed_core_conn_create(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return core_conn_create_impl(input);
}
NFC_STATUS core_conn_closed_transceive(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  (void)input;
  return NFC_STATUS_INVALID_STATE_CORE_CONN_CLOSED_ALREADY;
}
NFC_STATUS core_conn_closed_close_tda(void *input) {
  OSAL_LOG_NFCHAL_D("%s", __func__);
  return mode_set_disable_impl(input);
}

NFC_STATUS core_conn_closed_core_conn_close(void *input) {
  (void)input;
  return NFC_STATUS_INVALID_STATE_TDA_CLOSED_ALREADY;
}