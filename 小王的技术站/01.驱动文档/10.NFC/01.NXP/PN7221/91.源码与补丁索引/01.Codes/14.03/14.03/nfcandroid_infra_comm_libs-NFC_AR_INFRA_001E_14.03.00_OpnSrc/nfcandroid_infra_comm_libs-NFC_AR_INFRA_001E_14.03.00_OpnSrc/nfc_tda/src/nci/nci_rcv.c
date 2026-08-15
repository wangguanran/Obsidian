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

#include "nci_rcv.h"
#include "fsm.h"
#include "pal.h"
#include "tda.h"
#include "tda_nci_defs.h"

extern tda_control_t g_tda_ctrl;

/**
 * @brief process TDA discover response and updates the TDA info
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_discover_rsp(uint8_t *p_rsp, uint16_t p_len) {
  uint8_t status = (*(p_rsp + 3) & 0xFF);
  OSAL_LOG_NFCHAL_D("%s status:%02x\n", __func__, status);
  if (p_len != MSG_CORE_NFCEE_DISCOVER_RSP_LEN || status != 0x00) {
    g_tda_ctrl.ret_status = NFC_STATUS_NCI_RESPONSE_ERR;
    release_ct_lock();
  } else {
    g_tda_ctrl.num_tda_supported = (*(p_rsp + 4) & 0xFF);
    OSAL_LOG_NFCHAL_D("%s MAX SUPPORTED TDA:%02x\n", __func__,
                      g_tda_ctrl.num_tda_supported);
    if (g_tda_ctrl.num_tda_supported > 0) {
      g_tda_ctrl.p_tda = (tda_t *)ct_osal_malloc(
          (g_tda_ctrl.num_tda_supported + 1) * sizeof(tda_t));
    }
  }
}

/**
 * @brief process TDA core connection close response and updates the connection
 * ID
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_core_conn_close_rsp(uint8_t *p_rsp, uint16_t p_len) {
  uint8_t status = (*(p_rsp + 3) & 0xFF);
  OSAL_LOG_NFCHAL_D("%s status:%02x, p_len:%d\n", __func__, status, p_len);
  if (status != NFC_STATUS_SUCCESS) {
    g_tda_ctrl.ret_status = NFC_STATUS_NCI_RESPONSE_ERR;
  }
  g_tda_ctrl.curr_tda = INVALID_NUM;
  g_tda_ctrl.curr_channel_num = INVALID_NUM;
  release_ct_lock();
}

/**
 * @brief process TDA core connection  create response and updates the
 * connection ID
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_tda_core_conn_create_rsp(uint8_t *p_rsp, uint16_t p_len) {
  if (p_len == 7) {
    uint8_t status = (*(p_rsp + 3) & 0xFF);
    OSAL_LOG_NFCHAL_D("%s status:%02x\n", __func__, status);
    uint8_t payload_size = (*(p_rsp + 4) & 0xFF);
    OSAL_LOG_NFCHAL_D("%s payload_size:%02x\n", __func__, payload_size);
    uint8_t initial_credit = (*(p_rsp + 5) & 0xFF);
    OSAL_LOG_NFCHAL_D("%s initial_credit:%02x\n", __func__, initial_credit);
    uint8_t connection_id = (*(p_rsp + 6) & 0xFF);
    OSAL_LOG_NFCHAL_D("%s connection_id:%02x\n", __func__, connection_id);
    g_tda_ctrl.curr_channel_num = connection_id;
  } else {
    OSAL_LOG_NFCHAL_D("%s invalid core create res len:%02x\n", __func__, p_len);
    g_tda_ctrl.ret_status = NFC_STATUS_CORE_CONN_CREATE_FAILED;
  }
  release_ct_lock();
}

/**
 * @brief updates NFCEE mode set specific error codes
 * @param[in] status: error code
 *
 * @return void
 *
 */
static void update_nfcee_error_status(uint8_t status) {
  switch (status) {
  case NFCEE_INTERFACE_ACTIVATION_FAILED:
    g_tda_ctrl.ret_status = NFC_STATUS_NFCEE_INTERFACE_ACTIVATION_FAILED;
    break;
  case NFCEE_TRANSMISSION_ERROR:
    g_tda_ctrl.ret_status = NFC_STATUS_NFCEE_TRANSMISSION_ERROR;
    break;
  case NFCEE_PROTOCOL_ERROR:
    g_tda_ctrl.ret_status = NFC_STATUS_NFCEE_PROTOCOL_ERROR;
    break;
  case NFCEE_TIMEOUT_ERROR:
    g_tda_ctrl.ret_status = NFC_STATUS_NFCEE_TIMEOUT_ERROR;
    break;
  default:
    g_tda_ctrl.ret_status = NFC_STATUS_NCI_RESPONSE_ERR;
    break;
  }
}

/**
 * @brief process mode set notification and handles the errors
 * @param[in] p_rsp data buffer
 *
 * @return void
 *
 */
void process_tda_mode_set_ntf(uint8_t *p_rsp) {
  uint8_t status = (*(p_rsp + 3) & 0xFF);
  OSAL_LOG_NFCHAL_D("%s status:%02x \n", __func__, status);
  g_tda_ctrl.ret_status = status;
  if (status != 0x00) {
    update_nfcee_error_status(status);
    g_tda_ctrl.mode_set_ctrl.mode = INVALID_NUM;
    g_tda_ctrl.mode_set_ctrl.tda_id = INVALID_NUM;
    release_ct_lock();
  } else if (g_tda_ctrl.mode_set_ctrl.mode == NCI_NFCEE_MD_DEACTIVATE) {
    remove_tda_info_of_tda(g_tda_ctrl.mode_set_ctrl.tda_id);
    g_tda_ctrl.mode_set_ctrl.mode = INVALID_NUM;
    g_tda_ctrl.mode_set_ctrl.tda_id = INVALID_NUM;
    update_state(DISCOVERED_STATE);
    release_ct_lock();
  }
}

/**
 * @brief process mode set response and handles the errors
 * @param[in] p_rsp data buffer
 *
 * @return void
 *
 */
void process_tda_mode_set_rsp(uint8_t *p_rsp) {
  uint8_t tda_id = (*(p_rsp + 2) & 0xFF);
  uint8_t status = (*(p_rsp + 3) & 0xFF);
  OSAL_LOG_NFCHAL_D("%s status:%02x, tda_id:%02x\n", __func__, status, tda_id);
  g_tda_ctrl.ret_status = status;
  if (status != 0x00) {
    update_nfcee_error_status(status);
    release_ct_lock();
  }
}

/**
 * @brief process TDA discover response and updates the TDA info
 *
 * @param[in] p_ntf data buffer
 * @return void
 *
 */
void process_tda_discover_ntf(uint8_t *p_ntf) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  uint8_t *p_ntf_tmp, op_code;
  p_ntf_tmp = p_ntf + 1;
  NCI_MSG_PRS_HDR_BYTE1(p_ntf_tmp, op_code);
  OSAL_LOG_NFCHAL_D("%s op_code:0x%x\n", __func__, op_code);
  p_ntf_tmp += 1;
  int current_id = *p_ntf_tmp;
  OSAL_LOG_NFCHAL_D("%s current_id:%02x, g_tda_ctrl.mode_set_ctrl.tda_id:%d, "
                    "g_tda_ctrl.curr_tda:%d",
                    __func__, current_id, g_tda_ctrl.mode_set_ctrl.tda_id,
                    g_tda_ctrl.curr_tda);
  int status = *(p_ntf_tmp + 1);
  OSAL_LOG_NFCHAL_D("%s status:%02x", __func__, status);
  for (int i = 0; i < g_tda_ctrl.num_tda_supported; i++) {
    if (current_id == (g_tda_ctrl.p_tda + i)->id) {
      OSAL_LOG_NFCHAL_D("%s current_id:%02x exists", __func__, current_id);
      g_tda_ctrl.curr_tda = i;
    }
  }
  OSAL_LOG_NFCHAL_D("%s current index:%02x", __func__, g_tda_ctrl.curr_tda);
  tda_t *p_tda = g_tda_ctrl.p_tda + g_tda_ctrl.curr_tda;

  if (op_code == NCI_MSG_NFCEE_DISCOVER) {
    switch (g_tda_ctrl.tda_state) {
    case INIT_STATE:
      OSAL_LOG_NFCHAL_D("%s TDA_STATE_INIT \n", __func__);
      add_tda_info(p_ntf_tmp, p_tda);
      g_tda_ctrl.curr_channel_num = INVALID_NUM;
      g_tda_ctrl.curr_tda += 1;
      if (g_tda_ctrl.curr_tda == g_tda_ctrl.num_tda_supported) {
        g_tda_ctrl.curr_tda = 0;
        release_ct_lock();
        ct_osal_init_completed();
      }
      break;
    case DISCOVERED_STATE:
    case MODE_SET_ENABLED_STATE:
    case CORE_CONN_CREATED_STATE:
    case CORE_CONN_CLOSED_STATE:
    case LAST_STATE:
      OSAL_LOG_NFCHAL_D("%s DISCOVERED_STATE \n", __func__);
      remove_tda_info(p_ntf_tmp, p_tda);
      add_tda_info(p_ntf_tmp, p_tda);
      if (current_id == g_tda_ctrl.mode_set_ctrl.tda_id) {
        g_tda_ctrl.mode_set_ctrl.tda_id = INVALID_NUM;
        g_tda_ctrl.mode_set_ctrl.mode = INVALID_NUM;
        release_ct_lock();
      }
      // (ct_osal_get_nci_hal_ctrl().p_tda_state_change)(p_tda, "TDA Updated");
      break;
    }
  } else {
    OSAL_LOG_NFCHAL_D("%s unknown opcode:0x%x\n", __func__, op_code);
  }
}

/**
 * @brief Forms the NCI packet by including NCI header and retruns the data
 *
 * @param[in] pbf_n_conn_id packet boundary and connection ID
 * @param[in] p_data data buffer
 * @param[in] data_len data length
 * @return NCI packet buffer
 *
 */
static uint8_t *get_nci_ct_loopback_data(uint8_t pbf_n_conn_id, uint8_t *p_data,
                                         int data_len) {
  g_tda_ctrl.trans_buf.rsp_apdu->p_data =
      ct_osal_malloc((data_len + NCI_PKT_HDR_SIZE) * sizeof(uint8_t));
  OSAL_LOG_NFCHAL_D("%s get_nci_ct_loopback_data pbf_n_conn_id:%02x\n",
                    __func__, pbf_n_conn_id);
  *(g_tda_ctrl.trans_buf.rsp_apdu->p_data + 0) = pbf_n_conn_id;
  *(g_tda_ctrl.trans_buf.rsp_apdu->p_data + 1) = 0x00;
  *(g_tda_ctrl.trans_buf.rsp_apdu->p_data + 2) = data_len;
  ct_osal_memcpy((g_tda_ctrl.trans_buf.rsp_apdu->p_data + NCI_PKT_HDR_SIZE),
                 p_data, data_len);
  return g_tda_ctrl.trans_buf.rsp_apdu->p_data;
}

/**
 * @brief process CT data with reassembles and send to upper layer
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return void
 *
 */
void process_nfc_ct_data(uint8_t *p_ntf, uint16_t p_len) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  if (p_len < 1) {
    OSAL_LOG_NFCHAL_E(
        "Not valid Non fragment APDU received length less than 1");
  }

  int apdu_len = (int)p_ntf[2];
  if ((p_ntf[0] & PBF_SEGMENT_MSG) == PBF_SEGMENT_MSG) {
    CT_OSAL_SET_CHAINED_DATA();
    ct_osal_memcpy(ct_osal_get_nci_hal_ctrl().frag_rsp.p_data +
                       ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos,
                   (p_ntf + NCI_PKT_HDR_SIZE), (p_len - NCI_PKT_HDR_SIZE));
    ct_osal_set_frag_data_pos(ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos +
                              (p_len - NCI_PKT_HDR_SIZE));
  } else if ((p_ntf[0] & PBF_COMPLETE_MSG) == PBF_COMPLETE_MSG) {
    if (CT_OSAL_IS_CHAINED_DATA()) {
      if (ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos > 0 &&
          (ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos + apdu_len) <
              NCI_FRAG_MAX_DATA_LEN) {
        ct_osal_memcpy(ct_osal_get_nci_hal_ctrl().frag_rsp.p_data +
                           ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos,
                       p_ntf + NCI_PKT_HDR_SIZE, apdu_len);
        ct_osal_set_frag_data_pos(ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos +
                                  apdu_len);
        uint8_t pbf_n_conn_id = PBF_SEGMENT_MSG | p_ntf[0];
        OSAL_LOG_NFCHAL_D("%s pbf_n_conn_id:%d\n", __func__, pbf_n_conn_id);
        g_tda_ctrl.trans_buf.rsp_apdu->p_data = get_nci_ct_loopback_data(
            pbf_n_conn_id, ct_osal_get_nci_hal_ctrl().frag_rsp.p_data,
            ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos);
        g_tda_ctrl.trans_buf.rsp_apdu->len =
            ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos + NCI_PKT_HDR_SIZE;
        release_ct_lock();
      } else {
        OSAL_LOG_NFCHAL_E("Invalid APDU data length:%d received",
                          ct_osal_get_nci_hal_ctrl().frag_rsp.data_pos +
                              apdu_len);
      }
      ct_osal_set_frag_data_pos(0);
      CT_OSAL_RESET_CHAINED_DATA();
    } else {
      uint8_t pbf_n_conn_id = PBF_COMPLETE_MSG | p_ntf[0];
      OSAL_LOG_NFCHAL_D("%s pbf_n_conn_id:%d\n", __func__, pbf_n_conn_id);
      g_tda_ctrl.trans_buf.rsp_apdu->p_data = get_nci_ct_loopback_data(
          pbf_n_conn_id, p_ntf + NCI_PKT_HDR_SIZE, p_len - NCI_PKT_HDR_SIZE);
      g_tda_ctrl.trans_buf.rsp_apdu->len = p_len;
      release_ct_lock();
    }
  } else {
    OSAL_LOG_NFCHAL_D("%s Non CT data packet, Not processing. "
                      "issegmentData:%02x,isFulldata:%02x \n",
                      __func__, p_ntf[0] & PBF_SEGMENT_MSG,
                      p_ntf[0] & PBF_COMPLETE_MSG);
  }
}

/**
 * @brief process the nci packet related to CT
 *
 * @param[in] p_ntf data buffer
 * @param[in] p_len data length
 *
 * @return NFC_STATUS indicates success or failure
 * Refer nfc_status.h file for more specific error code
 * incase of failure.
 *
 */
NFC_STATUS proc_tda_rsp_ntf(uint8_t *p_ntf, uint16_t p_len) {
  OSAL_LOG_NFCHAL_D("%s p_len:%d", __func__, p_len);
  uint8_t msg_type, pbf, group_id, op_code, *p_data;

  p_data = p_ntf;
  NCI_MSG_PRS_HDR_BYTE0(p_data, msg_type, pbf, group_id);
  OSAL_LOG_NFCHAL_D("%s msg_type:%d group_id:%d ", __func__, msg_type,
                    group_id);

  NCI_MSG_PRS_HDR_BYTE1(p_data, op_code);
  OSAL_LOG_NFCHAL_D("%s op_code:%d", __func__, op_code);
  p_data = p_ntf;

  switch (msg_type) {
  case NCI_MSG_TYPE_RSP_VAL: {
    switch (group_id) {
    case NCI_GID_EE_MANAGE_VAL:
      switch (op_code) {
      case MSG_CORE_NFCEE_DISCOVER_RSP_NTF_VAL: {
        process_tda_discover_rsp(p_data, p_len);
        break;
      }
      case MSG_CORE_NFCEE_MODESET_RSP_NTF: {
        process_tda_mode_set_rsp(p_data);
        break;
      }
      }
    break;
    case NCI_GID_CORE_VAL:
      switch (op_code) {
      case NCI_MSG_CORE_CONN_CREATE_VAL: {
        process_tda_core_conn_create_rsp(p_data, p_len);
        break;
      }
      case NCI_MSG_CORE_CONN_CLOSE_VAL: {
        process_tda_core_conn_close_rsp(p_data, p_len);
        break;
      }
      }
      break;
    }
    break;
  }
  case NCI_MSG_TYPE_NTF_VAL: {
    switch (group_id) {
    case NCI_GID_EE_MANAGE_VAL:
      switch (op_code) {
      case MSG_CORE_NFCEE_DISCOVER_RSP_NTF_VAL: {
        process_tda_discover_ntf(p_ntf);
        break;
      }
      case MSG_CORE_NFCEE_MODESET_RSP_NTF: {
        process_tda_mode_set_ntf(p_data);
        break;
      }
      }
    }
    break;
  }
  case NCI_MSG_TYPE_DATA_VAL: {
    int channel_num = get_tda_channel_num();
    OSAL_LOG_NFCHAL_D("%s channel_num:%d\n", __func__, channel_num);
    if (group_id == channel_num) {
      OSAL_LOG_NFCHAL_D("%s p_len:%d\n", __func__, p_len);
      process_nfc_ct_data(p_ntf, p_len);
    }
    break;
  }
  }
  return NFC_STATUS_SUCCESS;
}