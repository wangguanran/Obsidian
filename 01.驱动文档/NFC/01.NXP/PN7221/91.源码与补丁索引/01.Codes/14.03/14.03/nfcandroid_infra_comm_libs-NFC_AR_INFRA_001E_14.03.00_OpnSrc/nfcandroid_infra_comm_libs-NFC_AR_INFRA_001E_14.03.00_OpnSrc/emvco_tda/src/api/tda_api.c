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
#include "tda_api.h"
#include "fsm.h"
#include "nci_rcv.h"
#include "nci_snd.h"
#include "pal.h"
#include "tda.h"
#include "tda_nci_defs.h"

tda_control_t g_tda_ctrl;

/**
 * @brief initialize CT and sends discovers the TDA connected
 *
 * @return EMVCO_STATUS indicates success or failure
 *
 */
EMVCO_STATUS ct_init_ext(emvco_tda_state_change_t *p_tda_state_change,
                         emvco_cl_state_change_t *p_cl_state_change) {
  OSAL_LOG_EMVCOHAL_D("%s", __func__);
  g_tda_ctrl.p_cl_state_change = p_cl_state_change;
  g_tda_ctrl.p_tda_state_change = p_tda_state_change;
  if (0 != osal_sem_init(&g_tda_ctrl.sync_tda_write, 0, 0)) {
    OSAL_LOG_EMVCOHAL_E(
        "osal_sem_init() Failed for sync_tda_write, errno = 0x%02X", errno);
  }

  if (pthread_mutex_init(&g_tda_ctrl.snd_lck, NULL) != 0) {
    OSAL_LOG_EMVCOHAL_E("osal_sem_init failed for snd_lck, errono = 0x%08x",
                        errno);
  }

  update_state(INIT_STATE);
  return EMVCO_STATUS_SUCCESS;
}

/**
 * @brief De-initializes CT and updates the EMVCo off state to clients
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS ct_de_init_ext() {
  OSAL_LOG_EMVCOHAL_D("%s", __func__);
  g_tda_ctrl.num_tda_supported = 0;
  g_tda_ctrl.curr_tda = 0;
  update_state(INIT_STATE);
  osal_sem_destroy(&g_tda_ctrl.sync_tda_write);
  pthread_mutex_destroy(&g_tda_ctrl.snd_lck);
  (g_tda_ctrl.p_cl_state_change)((uint8_t)OFF, "EMVCo OFF");
  return EMVCO_STATUS_SUCCESS;
}

/**
 * @brief sends nfcee discover command to controller.
 *
 * @param[in] void
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS ct_nfcee_discover() {
  OSAL_LOG_EMVCOHAL_D("%s", __func__);
  EMVCO_STATUS status = EMVCO_STATUS_OK;
  fp_event_handler_t fp_event_handler = handle_event(NFCEE_DISCOVER_EVENT);
  status = fp_event_handler(NULL);
  return status;
}

/**
 * @brief discovers the smart card connected to TDA and returns the smart card
 * control.
 *
 * @param[in] void
 * @param[out] tda_control provides the details of the smartcards present over
 * TDA
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS ct_discover_tda(tda_control_t *tda_control) {
  OSAL_LOG_EMVCOHAL_D("%s", __func__);
  EMVCO_STATUS status = EMVCO_STATUS_OK;
  fp_event_handler_t fp_event_handler = handle_event(DISCOVER_TDA_EVENT);
  status = fp_event_handler(tda_control);
  return status;
}

/**
 * @brief opens the contactcard.
 *
 * @param[in] tda_id id of the contact card to be opened
 * @param[in]  standBy false, opens the communication with TDA freshely and mode
 * set enable command is sent to controller. standBy true, resumes the
 * communication from partial close and does not send mode set enable command to
 * controller
 *
 * @note       use standby false, if you are opening the TDA for first time.
 *             use standby true, if you are opening the TDA followed by partial
 * close of another TDA
 * @param[out] channel_num returns the conn_id id of the contact card
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS ct_open(int8_t tda_id, bool in_standBy, int8_t *channel_num) {
  OSAL_LOG_EMVCOHAL_D("%s tda_id:%02x, in_standBy:%d", __func__, tda_id,
                      in_standBy);
  g_tda_ctrl.tda_ch_pr.tda_id = tda_id;
  g_tda_ctrl.tda_ch_pr.channel_num = INVALID_NUM;
  EMVCO_STATUS status = EMVCO_STATUS_OK;
  fp_event_handler_t fp_event_handler;
  if (!in_standBy) {
    fp_event_handler = handle_event(OPEN_TDA_EVENT);
  } else {
    fp_event_handler = handle_event(CORE_CONN_CREATE_EVENT);
  }
  status = fp_event_handler(&g_tda_ctrl.tda_ch_pr);
  *channel_num = g_tda_ctrl.tda_ch_pr.channel_num;
  OSAL_LOG_EMVCOHAL_D("%s g_tda_ctrl.tda_ch_pr.channel_num:%d,*channel_num:%d",
                      __func__, g_tda_ctrl.tda_ch_pr.channel_num, *channel_num);
  return status;
}

/**
 *
 * @brief           This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN7220 driver interface.
 *
 * @param[in]       cmd_apdu: Command to TDA
 * @param[out]      rsp_apdu: Command to TDA
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS ct_transceive(tda_data *cmd_apdu, tda_data *rsp_apdu) {
  OSAL_LOG_EMVCOHAL_D("%s", __func__);
  g_tda_ctrl.trans_buf.cmd_apdu = cmd_apdu;
  g_tda_ctrl.trans_buf.rsp_apdu = rsp_apdu;
  EMVCO_STATUS status = EMVCO_STATUS_OK;
  fp_event_handler_t fp_event_handler = handle_event(TRANSCEIVE_EVENT);
  status = fp_event_handler(&g_tda_ctrl.trans_buf);
  return status;
}

/**
 * @brief closes the contactcard.
 *
 * @param[in] tda_id id of the contact card to be closed
 * @param[in]  standBy true, closes the communication with TDA fully and allows
 * the system to go in standbymode standBy false, closes the communication
 * partially and does not allow the system to go in standbymode.
 *
 * @note       use standby false, If you are closing the current TDA to open
 * another TDA for communication then use false to get better performance use
 * standby true, If you are closing the current TDA to stop the communication
 * with it fully and allow system to enter standby mode
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS ct_close(int8_t tda_id, bool in_standBy) {
  OSAL_LOG_EMVCOHAL_D("%s tda_id:%02x", __func__, tda_id);
  EMVCO_STATUS status = EMVCO_STATUS_OK;
  fp_event_handler_t fp_event_handler;
  if (in_standBy) {
    fp_event_handler = handle_event(CLOSE_TDA_EVENT);
  } else {
    fp_event_handler = handle_event(CORE_CONN_CLOSE_EVENT);
  }
  status = fp_event_handler(&tda_id);
  return status;
}

/**
 * @brief process the nci packet and checks for core interface error
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is CT packet otherwise returns false
 *
 */
bool is_core_inf_err_ntf(uint8_t *p_ntf, uint16_t p_len) {
  OSAL_LOG_EMVCOHAL_D("%s p_len:%d", __func__, p_len);
  int channel_num = get_tda_channel_num();
  OSAL_LOG_EMVCOHAL_D("%s channel_num:%02x,p_ntf:%02x \n", __func__,
                      channel_num, *p_ntf);
  if ((5 == p_len) && (p_ntf[0] == NCI_MT_NTF_VAL &&
                       p_ntf[1] == NCI_CORE_INTERFACE_ERROR_NTF_VAL &&
                       p_ntf[2] == NCI_CORE_INTERFACE_ERROR_NTF_LEN_VAL)) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief process the nci packet and checks whether it is credit ntf received
 * for CT data packet
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is CT packet otherwise returns false
 *
 */
bool is_ct_data_credit_received(uint8_t *p_ntf, uint16_t p_len) {
  OSAL_LOG_EMVCOHAL_D("%s p_len:%d", __func__, p_len);
  int channel_num = get_tda_channel_num();
  OSAL_LOG_EMVCOHAL_D("%s channel_num:%02x,p_ntf:%02x \n", __func__,
                      channel_num, *p_ntf);
  if ((6 == p_len) &&
      (p_ntf[0] == NCI_MT_NTF_VAL &&
       p_ntf[1] == NCI_CORE_CONN_CREDITS_NTF_VAL &&
       p_ntf[2] == NCI_CORE_CONN_CREDITS_NTF_LEN_VAL &&
       p_ntf[3] == NCI_CORE_CONN_CREDITS_NTF_NO_OF_ENTRY_VAL &&
       p_ntf[4] == channel_num &&
       p_ntf[5] == NCI_CORE_CONN_CREDITS_NTF_CONN_CREDITS_VAL)) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief process the nci data/rsp/ntf packet related to CT
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return EMVCO_STATUS indicates success or failure
 * Refer emvco_status.h file for more specific error code
 * incase of failure.
 *
 */
EMVCO_STATUS process_tda_rsp_ntf(uint8_t *p_ntf, uint16_t p_len) {
  pthread_mutex_lock(&g_tda_ctrl.snd_lck);
  OSAL_LOG_EMVCOHAL_D("%s p_len:%d", __func__, p_len);
  EMVCO_STATUS status = proc_tda_rsp_ntf(p_ntf, p_len);
  pthread_mutex_unlock(&g_tda_ctrl.snd_lck);
  return status;
}

/**
 * @brief process the nci packet and checks the response is related to the NCI
 * command initiated by CT library
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is related to CT command otherwise returns false
 *
 */
bool is_ct_data_rsp(uint8_t *p_ntf, uint16_t p_len) {
  OSAL_LOG_EMVCOHAL_D("%s p_len:%d", __func__, p_len);
  int conn_id = get_tda_channel_num();
  LOG_EMVCOHAL_D("%s conn_id:%d, conn_id_mask:%02d", __func__, conn_id,
                 *p_ntf & NCI_CONN_ID_MASK_VAL);
  if (conn_id != INVALID_NUM && ((*p_ntf & NCI_CONN_ID_MASK_VAL) == conn_id)) {
    return true;
  }

  int oid = (*(p_ntf + 1) & NCI_OID_MASK_VAL);
  int gid = (*(p_ntf + 1) & NCI_GID_MASK_VAL);
  LOG_EMVCOHAL_D("%s oid:%d, gid:%d", __func__, oid, gid);

  if ((((*p_ntf) & (NCI_MSG_TYPE_RSP_VAL << NCI_MT_SHIFT_VAL)) ||
       ((*p_ntf) & (NCI_MSG_TYPE_NTF << NCI_MT_SHIFT_VAL))) &&
      ((gid == NCI_GID_EE) &&
       ((oid == NCI_MSG_CORE_CONN_CREATE_VAL ||
         oid == NCI_MSG_CORE_CONN_CLOSE_VAL || oid == NCI_MSG_MODE_SET_VAL)))) {
    LOG_EMVCOHAL_E("CT NCI response");
    return true;
  }
  return false;
}

/**
 * @brief process the nci packet and checks whether NCI command initiated by CT
 * library
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return true, if it is related to CT command otherwise returns false
 *
 */
bool is_ct_send_app_data(const uint8_t *p_ntf, uint16_t p_len, bool is_tda) {
  OSAL_LOG_EMVCOHAL_D("%s p_len:%d, is_tda:%d", __func__, p_len, is_tda);
  int msg_type = p_ntf[0] & (NCI_MSG_TYPE_RSP_VAL << NCI_MT_SHIFT_VAL);
  int oid = p_ntf[1] & NCI_OID_MASK_VAL;
  int gid = p_ntf[1] & NCI_GID_MASK_VAL;
  int conn_id = get_tda_channel_num();
  LOG_EMVCOHAL_D("%s msg_type:%02x, id:%d, conn_id:%d, gid:%d", __func__,
                 msg_type, oid, conn_id, gid);
  if ((msg_type == NCI_MSG_COMMAND_TYPE_VAL) &&
      (!(is_tda && ((gid == NCI_GID_EE) &&
                    (oid == NCI_MSG_CORE_CONN_CREATE_VAL ||
                     oid == NCI_MSG_CORE_CONN_CLOSE_VAL || oid == conn_id))))) {
    LOG_EMVCOHAL_E("NOT CT Command");
    return false;
  }
  return true;
}

/**
 * @brief informs the EMVCo ON state on EMVCo RF pool start
 *
 * @param[in] void
 *
 * @return void
 *
 */
void on_emvco_rf_pool_start() {
  (g_tda_ctrl.p_cl_state_change)((uint8_t)ON, "EMVCo ON");
}

/**
 *
 * @brief           sets the WTX time out value
 *
 * @param[in]       ime_out_val WTX time out value in secs
 *
 * @return void
 *
 */
void set_max_wtx_timeout_value(uint8_t time_out_val) {
  set_max_wtx_timeout_val(time_out_val);
}