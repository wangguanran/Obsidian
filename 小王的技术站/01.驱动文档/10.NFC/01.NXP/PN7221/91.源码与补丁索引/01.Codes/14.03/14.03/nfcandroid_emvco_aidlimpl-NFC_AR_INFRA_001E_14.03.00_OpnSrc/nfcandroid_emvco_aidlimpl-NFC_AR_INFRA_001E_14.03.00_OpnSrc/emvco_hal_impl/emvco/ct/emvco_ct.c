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

#include <emvco_config.h>
#include <emvco_ct.h>
#include <emvco_dm.h>
#include <emvco_log.h>
#include <string.h>

extern tda_control_t g_tda_ctrl;
extern fp_ct_open_t fp_ct_open;
extern fp_ct_close_t fp_ct_close;
extern fp_transceive_t fp_transceive;
extern fp_ct_discover_tda_t fp_ct_discover_tda;
extern fp_set_max_wtx_timeout_value_t fp_set_max_wtx_timeout_value;

EMVCO_STATUS discover_tda_slots(tda_control_t *tda_control) {
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;
  if (fp_ct_discover_tda != NULL) {
    status = fp_ct_discover_tda(tda_control);
  } else {
    LOG_EMVCOHAL_D("%s", __func__);
    status = EMVCO_STATUS_FEATURE_NOT_SUPPORTED;
  }
  return status;
}

EMVCO_STATUS open_tda_slot(int8_t tda_id, bool in_standBy, int8_t *conn_id) {
  LOG_EMVCOHAL_D("%s", __func__);
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;
  if (fp_ct_open != NULL) {
    status = fp_ct_open(tda_id, in_standBy, conn_id);
  } else {
    LOG_EMVCOHAL_D("%s", __func__);
    status = EMVCO_STATUS_FEATURE_NOT_SUPPORTED;
  }
  return status;
}

EMVCO_STATUS close_tda_slot(int8_t tda_id, bool in_standBy) {
  LOG_EMVCOHAL_D("%s", __func__);
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;
  if (fp_ct_close != NULL) {
    status = fp_ct_close(tda_id, in_standBy);
  } else {
    status = EMVCO_STATUS_FEATURE_NOT_SUPPORTED;
  }
  return status;
}

EMVCO_STATUS transceive_tda_slot(tda_data *cmd_apdu, tda_data *rsp_apdu) {
  LOG_EMVCOHAL_D("%s", __func__);
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;
  if (fp_transceive != NULL) {
    status = fp_transceive(cmd_apdu, rsp_apdu);
  } else {
    status = EMVCO_STATUS_FEATURE_NOT_SUPPORTED;
  }
  return status;
}

void initialize_max_wtx_timeout_value(void) {
  LOG_EMVCOHAL_D("%s", __func__);
  unsigned long num = 0;
  unsigned int num_len;

  if (get_byte_value(NAME_NXP_CT_MAX_WTX_WAIT_TIME, &num, &num_len)) {
    LOG_EMVCOHAL_D(
        "%s NAME_NXP_CT_MAX_WTX_WAIT_TIME property num_len:%d, num:%lu",
        __func__, num_len, num);
    if (fp_set_max_wtx_timeout_value != NULL) {
      fp_set_max_wtx_timeout_value(num);
    }
  } else {
    LOG_EMVCOHAL_E("%s NAME_NXP_CT_MAX_WTX_WAIT_TIME property not found",
                   __func__);
  }
}