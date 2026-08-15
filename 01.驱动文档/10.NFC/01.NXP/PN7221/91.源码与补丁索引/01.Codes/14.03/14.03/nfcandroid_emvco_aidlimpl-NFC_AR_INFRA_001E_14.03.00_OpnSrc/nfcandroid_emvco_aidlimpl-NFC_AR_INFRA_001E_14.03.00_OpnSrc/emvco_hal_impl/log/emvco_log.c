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
#define LOG_TAG "NxpNfcHal"
#include <stdio.h>
#include <string.h>
#if !defined(LOG__H_INCLUDED)
#include "emvco_config.h"
#include "emvco_log.h"
#endif
#include <cutils/properties.h>
#include <log/log.h>

const char *LOG_ITEM_EXTNS = "EMVCoExtns";
const char *LOG_ITEM_NCIHAL = "EMVCoHal";
const char *LOG_ITEM_NCIX = "EMVCoNciX";
const char *LOG_ITEM_NCIR = "EMVCoNciR";
const char *LOG_ITEM_TML = "EMVCoTml";

/* global log level structure */
nci_log_level_t gLog_level;

extern bool emvco_debug_enabled;

/*******************************************************************************
 *
 * Function         phNxpLog_SetGlobalLogLevel
 *
 * Description      Sets the global log level for all modules.
 *                  This value is set by Android property
 *nfc.log_level_global.
 *                  If value can be overridden by module log level.
 *
 * Returns          The value of global log level
 *
 ******************************************************************************/
static uint8_t set_global_log_level(void) {
  uint8_t level = LOG_DEFAULT_LOGLEVEL;
  unsigned long num = 0;
  char valueStr[PROPERTY_VALUE_MAX] = {0};

  int len = property_get(PROP_NAME_LOG_GLOBAL_LOGLEVEL, valueStr, "");
  if (len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    level = (unsigned char)num;
  }
  memset(&gLog_level, level, sizeof(nci_log_level_t));
  return level;
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetHALLogLevel
 *
 * Description      Sets the HAL layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void set_hal_log_level(uint8_t level) {
  unsigned long num = 0;
  unsigned int num_len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};

  if (get_byte_value(NAME_NXP_LOG_NCIHAL_LOGLEVEL, &num, &num_len)) {
    gLog_level.hal_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
  }

  LOG_EMVCOHAL_D("gLog_level.hal_log_level : %d", gLog_level.hal_log_level);

  num_len = property_get(PROP_NAME_LOG_HAL_LOGLEVEL, valueStr, "");
  if (num_len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.hal_log_level = (unsigned char)num;
    LOG_EMVCOHAL_D("Android prop gLog_level.hal_log_level : %d",
                   gLog_level.hal_log_level);
  }
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetExtnsLogLevel
 *
 * Description      Sets the Extensions layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void set_extn_log_level(uint8_t level) {
  unsigned long num = 0;
  unsigned int num_len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (get_byte_value(NAME_NXP_LOG_EXTNS_LOGLEVEL, &num, &num_len)) {
    gLog_level.extns_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
    ;
  }
  LOG_EMVCOHAL_D("gLog_level.extns_log_level : %d", gLog_level.extns_log_level);

  num_len = property_get(PROP_NAME_LOG_EXTNS_LOGLEVEL, valueStr, "");
  if (num_len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.extns_log_level = (unsigned char)num;
    LOG_EMVCOHAL_D("Android prop gLog_level.extns_log_level : %d",
                   gLog_level.extns_log_level);
  }
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetTmlLogLevel
 *
 * Description      Sets the Tml layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void set_tml_log_level(uint8_t level) {
  unsigned long num = 0;
  unsigned int num_len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (get_byte_value(NAME_NXP_LOG_TML_LOGLEVEL, &num, &num_len)) {
    gLog_level.tml_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
    ;
  }
  LOG_EMVCOHAL_D("gLog_level.tml_log_level : %d", gLog_level.tml_log_level);
  num_len = property_get(PROP_NAME_LOG_TML_LOGLEVEL, valueStr, "");
  if (num_len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.tml_log_level = (unsigned char)num;
    LOG_EMVCOHAL_D("Android prop gLog_level.tml_log_level : %d",
                   gLog_level.tml_log_level);
  }
}

/*******************************************************************************
 *
 * Function         phNxpLog_SetNciTxLogLevel
 *
 * Description      Sets the NCI transaction layer log level.
 *
 * Returns          void
 *
 ******************************************************************************/
static void set_nci_tx_log_level(uint8_t level) {
  unsigned long num = 0;
  unsigned int num_len;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (get_byte_value(NAME_NXP_LOG_NCIX_LOGLEVEL, &num, &num_len)) {
    gLog_level.ncix_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
  }

  if (get_byte_value(NAME_NXP_LOG_NCIR_LOGLEVEL, &num, &num_len)) {
    gLog_level.ncir_log_level =
        (level > (unsigned char)num) ? level : (unsigned char)num;
  }
  LOG_EMVCOHAL_D("ncix_log_level : %d , gLog_level.ncir_log_level : %d",
                 gLog_level.ncix_log_level, gLog_level.ncir_log_level);
  num_len = property_get(PROP_NAME_LOG_NCI_LOGLEVEL, valueStr, "");
  if (num_len > 0) {
    /* let Android property override .conf variable */
    sscanf(valueStr, "%lu", &num);
    gLog_level.ncix_log_level = (unsigned char)num;
    gLog_level.ncir_log_level = (unsigned char)num;
    LOG_EMVCOHAL_D(
        "Android ncix_log_level : %d , gLog_level.ncir_log_level : %d",
        gLog_level.ncix_log_level, gLog_level.ncir_log_level);
  }
}

void initialize_log_level(void) {
  uint8_t level = set_global_log_level();
  set_hal_log_level(level);
  set_extn_log_level(level);
  set_tml_log_level(level);
  set_nci_tx_log_level(level);

  ALOGD_IF(emvco_debug_enabled, "%s: global =%u, Fwdnld =%u, extns =%u, \
                hal =%u, tml =%u, ncir =%u, \
                ncix =%u",
           __func__, gLog_level.global_log_level, gLog_level.dnld_log_level,
           gLog_level.extns_log_level, gLog_level.hal_log_level,
           gLog_level.tml_log_level, gLog_level.ncir_log_level,
           gLog_level.ncix_log_level);
}
