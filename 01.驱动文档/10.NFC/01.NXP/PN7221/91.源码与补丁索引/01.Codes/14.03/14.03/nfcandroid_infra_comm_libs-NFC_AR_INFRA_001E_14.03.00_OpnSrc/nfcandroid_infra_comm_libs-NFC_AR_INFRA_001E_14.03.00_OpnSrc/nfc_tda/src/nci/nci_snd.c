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
#include "nci_snd.h"
#include "pal.h"
#include "tda.h"
#include "tda_nci_defs.h"

extern tda_control_t g_tda_ctrl;

static int check_ncicmd_write_window(uint16_t cmd_len, uint8_t *p_cmd);
static uint8_t write_ct_data_internal(uint8_t *p_data, uint16_t data_len);
uint8_t *p_nci_data = NULL;

/**
 *
 * @brief           This function is called to enable or disable NFCEE
 *                  Discovery.
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_nfcee_discover() {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  uint8_t *pp, *p;
  int len = NCI_PKT_HDR_SIZE;
  uint8_t cmd_buf[len];
  p = cmd_buf;
  pp = p;

  NCI_MSG_BLD_HDR0_TDA(pp, NCI_MSG_TYPE_CMD, NCI_GID_EE);
  NCI_MSG_BLD_HDR1(pp, NCI_MSG_NFCEE_DISCOVER);
  UINT8_TO_STREAM(pp, NCI_PARAM_SIZE_DISCOVER_NFCEE);

  g_tda_ctrl.ret_status = NFC_STATUS_SUCCESS;
  if (0 == write_ct_data_internal(p, len)) {
    g_tda_ctrl.ret_status = NFC_STATUS_WRITE_FAILED;
  }
  return g_tda_ctrl.ret_status;
}

/**
 *
 * @brief           This function is called to activate or de-activate an NFCEE
 *                  connected to the NFCC.
 *
 * @param[in]       nfcee_id - the NFCEE to activate or de-activate.
 *                  nfcee_mode - NFC_MODE_ACTIVATE to activate NFCEE,
 *                  NFC_MODE_DEACTIVATE to de-activate.
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_nfcee_mode_set_impl(uint8_t tda_id, uint8_t nfcee_mode) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  uint8_t *pp, *p;
  int len = NCI_PKT_HDR_SIZE + NCI_CORE_PARAM_SIZE_NFCEE_MODE_SET;
  uint8_t cmd_buf[len];
  p = cmd_buf;
  pp = p;

  NCI_MSG_BLD_HDR0_TDA(pp, NCI_MSG_TYPE_CMD, NCI_GID_EE);
  NCI_MSG_BLD_HDR1(pp, NCI_MSG_NFCEE_MODE_SET);
  UINT8_TO_STREAM(pp, NCI_CORE_PARAM_SIZE_NFCEE_MODE_SET);
  UINT8_TO_STREAM(pp, tda_id);
  UINT8_TO_STREAM(pp, nfcee_mode);

  g_tda_ctrl.ret_status = NFC_STATUS_SUCCESS;
  // Since mode set have timeout callback, no need to update the error based on
  // write return value
  write_ct_data_internal(p, len);
  return g_tda_ctrl.ret_status;
}

/**
 *
 * @brief           Helper function to add the TLV and sends core connection
 *create command
 *
 * @param[in]       dest_type - destination type
 * @param[in]       num_tlv - Number of TLV
 * @param[in]       tlv_size - TLV size
 * @param[in]       p_param_tlvs - TLV data
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_core_conn_create_internal(uint8_t dest_type, uint8_t num_tlv,
                                          uint8_t tlv_size,
                                          uint8_t *p_param_tlvs) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  uint8_t *pp, *p;
  int len = NCI_PKT_HDR_SIZE + NCI_CORE_PARAM_SIZE_CON_CREATE + tlv_size;
  uint8_t cmd_buf[len];
  p = cmd_buf;
  pp = p;

  NCI_MSG_BLD_HDR0_TDA(pp, NCI_MSG_TYPE_CMD, NCI_CORE_GID);
  NCI_MSG_BLD_HDR1(pp, NCI_MSG_CORE_CON_CREATE);
  UINT8_TO_STREAM(pp, NCI_CORE_PARAM_SIZE_CON_CREATE + tlv_size);
  UINT8_TO_STREAM(pp, dest_type);
  UINT8_TO_STREAM(pp, num_tlv);
  if (tlv_size) {
    BYTE_ARRAY_TO_STREAM(pp, p_param_tlvs, tlv_size);
  }

  g_tda_ctrl.ret_status = NFC_STATUS_SUCCESS;
  if (0 == write_ct_data_internal(p, len)) {
    g_tda_ctrl.ret_status = NFC_STATUS_WRITE_FAILED;
  }
  return g_tda_ctrl.ret_status;
}

/**
 *
 * @brief           compose and send CORE CONN_CREATE command to command queue
 *
 * @param[in]       conn_id - Connection ID
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_core_conn_create(uint8_t tda_id) {
  uint8_t param_tlvs[4], *pp;
  uint8_t num_tlv = 1;
  int tlv_size = 4;
  pp = param_tlvs;
  UINT8_TO_STREAM(pp, NCI_CON_CREATE_TAG_NFCEE_VAL);
  UINT8_TO_STREAM(pp, 2);
  UINT8_TO_STREAM(pp, tda_id);
  UINT8_TO_STREAM(pp, NCI_NFCEE_INTERFACE_APDU);
  g_tda_ctrl.curr_tda = tda_id;
  return send_core_conn_create_internal(NCI_DEST_TYPE_NFCEE, num_tlv, tlv_size,
                                        param_tlvs);
}

/**
 *
 * @brief           compose and send CORE CONN_CLOSE command to command queue
 *
 * @param[in]       channel_num - logical connection ID
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_core_conn_close(uint8_t channel_num) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  uint8_t *pp, *p;
  int len = NCI_PKT_HDR_SIZE + NCI_CORE_PARAM_SIZE_CON_CLOSE;
  uint8_t cmd_buf[len];
  p = cmd_buf;
  pp = p;

  NCI_MSG_BLD_HDR0_TDA(pp, NCI_MSG_TYPE_CMD, NCI_CORE_GID);
  NCI_MSG_BLD_HDR1(pp, NCI_MSG_CORE_CON_CLOSE);
  UINT8_TO_STREAM(pp, NCI_CORE_PARAM_SIZE_CON_CLOSE);
  UINT8_TO_STREAM(pp, channel_num);
  g_tda_ctrl.ret_status = NFC_STATUS_SUCCESS;
  if (0 == write_ct_data_internal(p, len)) {
    g_tda_ctrl.ret_status = NFC_STATUS_WRITE_FAILED;
  }
  return g_tda_ctrl.ret_status;
}

void mode_set_ntf_timeout() {
  OSAL_LOG_NFCHAL_D("%s, g_tda_ctrl.mode_set_ctrl.tda_id:%d, "
                      "g_tda_ctrl.mode_set_ctrl.mode:%d \n",
                      __func__, g_tda_ctrl.mode_set_ctrl.tda_id,
                      g_tda_ctrl.mode_set_ctrl.mode);
  alarm(0);
  g_tda_ctrl.ret_status = NFC_STATUS_NFCEE_MODE_SET_ENABLE_TIMEOUT;
  g_tda_ctrl.mode_set_ctrl.tda_id = INVALID_NUM;
  g_tda_ctrl.mode_set_ctrl.mode = INVALID_NUM;
}

/**
 *
 * @brief           This function is called to activate or de-activate an NFCEE
 *                  connected to the NFCC.
 *
 * @param[in]       nfcee_id - the NFCEE to activate or de-activate.
 *                  nfcee_mode - NFC_MODE_ACTIVATE to activate NFCEE,
 *                  NFC_MODE_DEACTIVATE to de-activate.
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_nfcee_mode_set(uint8_t tda_id, uint8_t mode) {
  int time_in_sec = 100; // 100 sec as per FW CT Compliance test sequence
  signal(SIGALRM, mode_set_ntf_timeout);
  alarm(time_in_sec);
  g_tda_ctrl.mode_set_ctrl.tda_id = tda_id;
  g_tda_ctrl.mode_set_ctrl.mode = mode;
  OSAL_LOG_NFCHAL_D("%s send nfcee_mode_set enable for TDA:%d\n", __func__,
                    tda_id);
  return send_nfcee_mode_set_impl(tda_id, mode);
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
  p_nci_data = ct_osal_malloc((data_len + NCI_PKT_HDR_SIZE) * sizeof(uint8_t));
  OSAL_LOG_NFCHAL_D("%s get_nci_ct_loopback_data pbf_n_conn_id:%02x\n",
                    __func__, pbf_n_conn_id);
  *(p_nci_data + 0) = pbf_n_conn_id;
  *(p_nci_data + 1) = 0x00;
  *(p_nci_data + 2) = data_len;
  ct_osal_memcpy((g_tda_ctrl.trans_buf.cmd_apdu->p_data + NCI_PKT_HDR_SIZE),
                 p_data, data_len);
  return p_nci_data;
}

/**
 *
 * @brief           Internal function to send raw APDU to controller
 *
 * @param[in]       p_data - data buffer pointer
 * @param[in]       data_len - data buffer length
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
static uint8_t write_ct_data_internal(uint8_t *p_data, uint16_t data_len) {
  uint8_t no_bytes_wrote = 0;
  pthread_mutex_lock(&g_tda_ctrl.snd_lck);
  no_bytes_wrote = ct_osal_write(p_data, data_len, true);
  if (check_ncicmd_write_window(data_len, p_data) != NFC_STATUS_SUCCESS) {
    int sem_val;
    sem_getvalue(&(g_tda_ctrl.sync_tda_write), &sem_val);
    no_bytes_wrote = 0;
    return no_bytes_wrote;
  }
  return no_bytes_wrote;
}

/**
 *
 * @brief           Internal function to form the NCI data packet and call the
 *internal write API
 *
 * @param[in]       pbf - packet boundary flag
 * @param[in]       p_data - data buffer pointer
 * @param[in]       data_len - data buffer length
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
static uint8_t send_nfc_ct_data_impl(int pbf, uint8_t *p_data, int data_len) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  uint8_t no_bytes_wrote = 0;
  p_nci_data = get_nci_ct_loopback_data(pbf, p_data, data_len);
  no_bytes_wrote =
      write_ct_data_internal(p_nci_data, data_len + NCI_PKT_HDR_SIZE);
  if (p_nci_data != NULL) {
    free(p_nci_data);
    p_nci_data = NULL;
  }
  return no_bytes_wrote;
}

/**
 *
 * @brief           handles the segmentation and sends to controller
 *
 * @param[in]       p_data - data buffer pointer
 * @param[in]       data_len - data buffer length
 *
 * @return          NFC status:
 *                  NFCSTATUS_SUCCESS - command processed successfully
 *                  NFCSTATUS_FAILED - failed to process the command
 *
 **/
NFC_STATUS send_nfc_ct_data(uint8_t *p_data, uint16_t data_len) {
  uint8_t no_bytes_wrote = 0;
  if (data_len > MAX_FRAGMENT_SIZE) {
    while (data_len > MAX_FRAGMENT_SIZE) {
      OSAL_LOG_NFCHAL_D("%s sending segment packet data_len:%d\n", __func__,
                        data_len);
      OSAL_LOG_NFCHAL_D("%s *p_data:%02x\n", __func__, *p_data);
      OSAL_LOG_NFCHAL_D("%s PBF_SEGMENT_MSG:%02x\n", __func__, PBF_SEGMENT_MSG);
      uint8_t pbf_n_conn_id = PBF_SEGMENT_MSG | (*p_data);
      OSAL_LOG_NFCHAL_D("%s pbf_n_conn_id:%02x\n", __func__, pbf_n_conn_id);
      no_bytes_wrote =
          send_nfc_ct_data_impl(pbf_n_conn_id, p_data + NCI_PKT_HDR_SIZE,
                                MAX_FRAGMENT_SIZE - NCI_PKT_HDR_SIZE);
      data_len -= MAX_FRAGMENT_SIZE;
      p_data += MAX_FRAGMENT_SIZE;
      OSAL_LOG_NFCHAL_D("%s sending segment packet no_bytes_wrote:%d\n",
                        __func__, no_bytes_wrote);
    }
    OSAL_LOG_NFCHAL_D("%s sending segment packet after while data_len:%d\n",
                      __func__, data_len);
    if (data_len > 0) {
      int channel_num = get_tda_channel_num();
      OSAL_LOG_NFCHAL_D("%s channel_num:%d\n", __func__, channel_num);
      uint8_t pbf_n_conn_id = (PBF_COMPLETE_MSG | channel_num);
      OSAL_LOG_NFCHAL_D("%s pbf_n_conn_id:%d, data_len:%d\n", __func__,
                        pbf_n_conn_id, data_len);
      no_bytes_wrote = send_nfc_ct_data_impl(pbf_n_conn_id, p_data, data_len);
    }
  } else {
    no_bytes_wrote = write_ct_data_internal(p_data, data_len);
  }
  if (no_bytes_wrote == 0) {
    return NFC_STATUS_TRANSCEIVE_FAILED;
  }
  return NFC_STATUS_SUCCESS;
}

/******************************************************************************
 * Function         check_ncicmd_write_window
 *
 * Description      This function is called to wait for the
 *                  response/notication/data for the request
 *                  sent to controller
 *
 * Returns          return 0 on success and -1 on fail.
 *
 ******************************************************************************/

int check_ncicmd_write_window(uint16_t cmd_len, uint8_t *p_cmd) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  NFC_STATUS status = NFC_STATUS_FAIL;
  // 100 sec as per FW compliance test sequence
  int sem_timedout = 100, s;
  struct timespec ts;

  if (cmd_len < 1) {
    pthread_mutex_unlock(&g_tda_ctrl.snd_lck);
    return NFC_STATUS_FAIL;
  }
  uint8_t cmd_pbf = (p_cmd[0] & 0xE0);
  OSAL_LOG_NFCHAL_D("%s Ct (1st byte):%02x\n", __func__, cmd_pbf);
  if (cmd_pbf == 0x20 || cmd_pbf == 0x00) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += sem_timedout;
    int sem_val;
    sem_getvalue(&(g_tda_ctrl.sync_tda_write), &sem_val);
    OSAL_LOG_NFCHAL_D("%s sem_val:%d\n", __func__, sem_val);
    pthread_mutex_unlock(&g_tda_ctrl.snd_lck);
    while ((s = sem_timedwait_monotonic_np(&g_tda_ctrl.sync_tda_write, &ts)) ==
               -1 &&
           errno == EINTR) {
      OSAL_LOG_NFCHAL_D("%s continue\n", __func__);
      continue; /* Restart if interrupted by handler */
    }
    OSAL_LOG_NFCHAL_D("%s time:%d\n", __func__, s);
    if (s != -1) {
      status = NFC_STATUS_SUCCESS;
    }
  } else {
    /* cmd window check not required for writing non nci cmd and data packet */
    pthread_mutex_unlock(&g_tda_ctrl.snd_lck);
    status = NFC_STATUS_SUCCESS;
  }
  return status;
}