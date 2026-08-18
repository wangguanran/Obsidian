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
#include "tda.h"
#include "pal.h"

extern tda_control_t g_tda_ctrl;

void process_nfc_ct_data(uint8_t *p_ntf, uint16_t p_len);

/**
 *
 * @brief           releases the lock to send next data to controller.
 *
 *
 * @return          returns void
 *
 **/
void release_ct_lock() {
  int sem_val;
  ct_osal_sem_getvalue(&(g_tda_ctrl.sync_tda_write), &sem_val);
  OSAL_LOG_NFCHAL_D("%s sem_val:%d\n", __func__, sem_val);
  if (sem_val == 0) {
    ct_osal_sem_post(&(g_tda_ctrl.sync_tda_write));
  }
}

/**
 *
 * @brief           adds the TDA information for given TDA.
 * @param[in]       p_ntf data buffer received from NFCC
 * @param[in]       p_tda tda structure pointer to be added/updated
 *
 * @return          returns void
 *
 **/
void add_tda_info(uint8_t *p_ntf, tda_t *p_tda) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  p_tda->id = *(p_ntf)++;
  OSAL_LOG_NFCHAL_D("%s p_tda->id:%02x", __func__, p_tda->id);
  p_tda->status = *(p_ntf)++;
  OSAL_LOG_NFCHAL_D("%s p_tda->status:%02x", __func__, p_tda->status);
  p_tda->number_of_protocols = *(p_ntf)++;
  OSAL_LOG_NFCHAL_D("%s p_tda->number_of_protocols:%02x", __func__,
                    p_tda->number_of_protocols);
  uint8_t num_of_protocols = p_tda->number_of_protocols;
  if (num_of_protocols >= 1) {
    p_tda->protocols_t =
        (protocols_t *)malloc(sizeof(protocols_t) * num_of_protocols);
    for (int i = 0; i < num_of_protocols; i++) {
      int protocol_val = *(p_ntf)++;
      p_tda->protocols_t[i] = (protocols_t)protocol_val;
      OSAL_LOG_NFCHAL_D("%s p_tda->protocols_t[i]:%02x", __func__,
                        p_tda->protocols_t[i]);
    }
  }
  p_tda->number_of_card_info = *(p_ntf)++;
  uint8_t num_of_card_info = p_tda->number_of_card_info;
  OSAL_LOG_NFCHAL_D("%s p_tda->number_of_card_info:%02x", __func__,
                    p_tda->number_of_card_info);
  if (num_of_card_info >= 1) {
    p_tda->card_tlv_info =
        (card_tlv_info_t *)malloc(sizeof(card_tlv_info_t) * num_of_card_info);
    for (int i = 0; i < num_of_card_info; i++) {
      (p_tda->card_tlv_info + i)->type = *(p_ntf)++;
      OSAL_LOG_NFCHAL_D("%s (p_tda->card_tlv_info[i])->type:%02x", __func__,
                        (p_tda->card_tlv_info + i)->type);
      (p_tda->card_tlv_info + i)->length = *(p_ntf)++;
      OSAL_LOG_NFCHAL_D("%s (p_tda->card_tlv_info[i])->length:%02x", __func__,
                        (p_tda->card_tlv_info + i)->length);
      uint8_t value_len = (p_tda->card_tlv_info + i)->length;
      if (value_len >= 1) {
        (p_tda->card_tlv_info + i)->value =
            (uint8_t *)malloc(sizeof(uint8_t) * value_len);
        TDA_STREAM_TO_ARRAY((p_tda->card_tlv_info + i)->value, p_ntf,
                            value_len);
        for (int j = 0; j < value_len; j++) {
          OSAL_LOG_NFCHAL_D(
              "%s (p_tda->card_tlv_info[i])->value[j]:%02x", __func__,
              (uint8_t)(*((uint8_t *)((p_tda->card_tlv_info + i)->value) + j)));
        }
      }
    }
  }
}

/**
 *
 * @brief           removes the TDA information for given TDA.
 * @param[in]       tda_id id of the TDA
 *
 * @return          returns void
 *
 **/
void remove_tda_info_of_tda(uint8_t tda_id) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  int tda_index = -1;
  for (int i = 0; i < g_tda_ctrl.num_tda_supported; i++) {
    if (tda_id == (g_tda_ctrl.p_tda + i)->id) {
      OSAL_LOG_NFCHAL_D("%s tda_id:%02x exists", __func__, tda_id);
      tda_index = i;
      break;
    }
  }
  if (tda_index == -1) {
    return;
  }
  tda_t *p_tda = g_tda_ctrl.p_tda + tda_index;

  p_tda->status = DISABLED;
  OSAL_LOG_NFCHAL_D("%s p_tda->status:%02x", __func__, p_tda->status);

  for (int i = 0; i < p_tda->number_of_card_info; i++) {
    if ((p_tda->card_tlv_info + i)->value != NULL) {
      free((p_tda->card_tlv_info + i)->value);
    }
  }
  if ((p_tda->card_tlv_info) != NULL) {
    free(p_tda->card_tlv_info);
  }
  p_tda->number_of_card_info = 0;
}

/**
 *
 * @brief           removes the TDA information based on TDA received NCI NTF.
 * @param[in]       p_ntf data buffer received from NFCC
 * @param[in]       p_tda tda structure pointer to be removed/updated
 *
 * @return          returns void
 *
 **/
void remove_tda_info(uint8_t *p_ntf, tda_t *p_tda) {
  OSAL_LOG_NFCHAL_D("%s \n", __func__);
  p_tda->id = *(p_ntf)++;
  OSAL_LOG_NFCHAL_D("%s p_tda->id:%02x", __func__, p_tda->id);
  p_tda->status = *(p_ntf)++;
  OSAL_LOG_NFCHAL_D("%s p_tda->status:%02x, p_tda->number_of_card_info:%02x",
                    __func__, p_tda->status, p_tda->number_of_card_info);
  if (p_tda->number_of_card_info > 0) {
    for (int i = 0; i < p_tda->number_of_card_info; i++) {
      if ((p_tda->card_tlv_info + i)->value != NULL) {
        free((uint8_t *)(p_tda->card_tlv_info + i)->value);
      }
    }
    if ((p_tda->card_tlv_info) != NULL) {
      free(p_tda->card_tlv_info);
    }
    p_tda->number_of_card_info = 0;
  }
}

/**
 *
 * @brief           returns the channel number of the currently opened TDA
 *
 *
 * @return          returns valid channel number, if found or returns
 *(-1)INVALID_NUM
 *
 **/
uint8_t get_tda_channel_num() {
  LOG_NFCHAL_D("%s curr_channel_num:%d\n", __func__,
               g_tda_ctrl.curr_channel_num);
  return g_tda_ctrl.curr_channel_num;
}