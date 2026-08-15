/******************************************************************************
 *
 *  Copyright (C) 2011-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

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

#include "emvco_config.h"
#include "emvco_log.h"
#include "emvco_osal_common.h"
#include "osal_memory.h"
#include <emvco_ncif.h>
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

struct map *fp_config_map = NULL;
uint8_t *fp_buffer = NULL;
pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;

struct map *config_map = NULL;
uint8_t cmd_idle_pwr_off_cfg[] = {0x04, 0xA0, 0x44, 0x01, 0x02};

static size_t readConfigFile(const char *fileName, uint8_t **p_data) {
  FILE *fd = fopen(fileName, "rb");
  if (fd == NULL)
    return 0;
  fseek(fd, 0L, SEEK_END);
  const size_t file_size = ftell(fd);
  rewind(fd);

  if ((long)file_size < 0) {
    LOG_EXTNS_E("%s Invalid file size file_size = %zu\n", __func__, file_size);
    fclose(fd);
    return 0;
  }

  fp_buffer = (uint8_t *)malloc(file_size + 1);
  if (!fp_buffer) {
    fclose(fd);
    return 0;
  }
  size_t read = fread(fp_buffer, file_size, 1, fd);
  fclose(fd);

  if (read == 1) {
    fp_buffer[file_size] = '\n';
    *p_data = fp_buffer;
    return file_size + 1;
  }
  return 0;
}

/**
 * @brief Determines if a given character is a digit in the specified number
 * base.
 * @param c The character to be evaluated.
 * @param base The number base to be used for evaluation (valid values are 2 to
 * 36).
 * @return True if the character is a digit in the specified number base, false
 * otherwise.
 */
bool isDigit(char c, int base) {
  if ('0' <= c && c <= '9')
    return true;
  if (base == 16) {
    if (('A' <= c && c <= 'F') || ('a' <= c && c <= 'f'))
      return true;
  }
  return false;
}

/**
 * @brief Converts a character representation of a digit to its integer value in
 * the specified number base.
 *
 * @param c The character representation of a digit.
 * @param base The number base to use for the conversion.
 *
 * @return The integer value of the digit in the specified number base.
 */
int getDigitValue(char c, int base) {
  if ('0' <= c && c <= '9')
    return c - '0';
  if (base == 16) {
    if ('A' <= c && c <= 'F')
      return c - 'A' + 10;
    else if ('a' <= c && c <= 'f')
      return c - 'a' + 10;
  }
  return 0;
}

/**
 * @brief Determines if the input character is printable.
 *
 * @param c The character to check.
 *
 * @return True if the character is printable, false otherwise.
 */
bool isPrintable(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
         (c >= '0' && c <= '9') || c == '/' || c == '_' || c == '-' || c == '.';
}

bool read_config(const char *name) {
  pthread_mutex_lock(&config_mutex);
  fp_config_map = map_create();
  enum {
    BEGIN_LINE = 1,
    TOKEN,
    STR_VALUE,
    NUM_VALUE,
    BEGIN_HEX,
    BEGIN_QUOTE,
    END_LINE
  };

  uint8_t *p_config = NULL;
  size_t config_size = readConfigFile(name, &p_config);

  if (p_config == NULL) {
    LOG_EXTNS_E("%s Cannot open config file %s\n", __func__, name);
    pthread_mutex_unlock(&config_mutex);
    return false;
  }

  char token[255] = {'\0'};
  char strValue[255] = {'\0'};
  int numValue = 0;
  int i = 0;
  int base = 0;
  char c;
  int bflag = 0;
  int token_index = 0;
  int strValue_index = 0;
  int state = BEGIN_LINE;

  for (size_t offset = 0; offset != config_size; ++offset) {
    c = p_config[offset];

    switch (state & 0xff) {
    case BEGIN_LINE:
      if (c == '#')
        state = END_LINE;
      else if (isPrintable(c)) {
        i = 0;
        token_index = 0;
        strValue_index = 0;
        osal_memset(token, '\0', sizeof(token));
        osal_memset(strValue, '\0', sizeof(strValue));
        strValue[0] = '\0';
        set_string_val(false);
        state = TOKEN;
        token[token_index++] = c;
      }
      break;
    case TOKEN:
      if (c == '=') {
        token[token_index] = '\0';
        state = BEGIN_QUOTE;
      } else if (isPrintable(c))
        token[token_index++] = c;
      else
        state = END_LINE;
      break;
    case BEGIN_QUOTE:
      if (c == '"') {
        state = STR_VALUE;
        base = 0;
      } else if (c == '0')
        state = BEGIN_HEX;
      else if (isDigit(c, 10)) {
        state = NUM_VALUE;
        base = 10;
        numValue = getDigitValue(c, base);
        i = 0;
      } else if (c == '{') {
        state = NUM_VALUE;
        bflag = 1;
        base = 16;
        i = 0;
        set_string_val(true);
      } else
        state = END_LINE;
      break;
    case BEGIN_HEX:
      if (c == 'x' || c == 'X') {
        state = NUM_VALUE;
        base = 16;
        numValue = 0;
        i = 0;
        break;
      } else if (isDigit(c, 10)) {
        state = NUM_VALUE;
        base = 10;
        numValue = getDigitValue(c, base);
        break;
      } else if (c != '\n' && c != '\r') {
        state = END_LINE;
        break;
      }
    case NUM_VALUE:
      if (isDigit(c, base)) {
        numValue *= base;
        numValue += getDigitValue(c, base);
        ++i;
      } else if (bflag == 1 &&
                 (c == ' ' || c == '\r' || c == '\n' || c == '\t')) {
        break;
      } else if (base == 16 &&
                 (c == ',' || c == ':' || c == '-' || c == ' ' || c == '}')) {
        if (c == '}') {
          bflag = 0;
        }
        if (i > 0) {
          int n = (i + 1) / 2;
          while (n-- > 0) {
            numValue = numValue >> (n * 8);
            unsigned char c = (numValue)&0xFF;
            strValue[strValue_index++] = c;
          }
          strValue[strValue_index] = '\0';
        }
        set_string_val(true);
        numValue = 0;
        i = 0;
      } else {
        if (c == '\n' || c == '\r') {
          if (bflag == 0) {
            state = BEGIN_LINE;
          }
        } else {
          if (bflag == 0) {
            state = END_LINE;
          }
        }
        if (is_string_val() && base == 16 && i > 0) {
          int n = (i + 1) / 2;
          while (n-- > 0) {
            strValue[strValue_index++] = (numValue >> (n * 8)) & 0xFF;
          }
        }
        if (strlen(strValue) > 0) {
          LOG_EXTNS_D("%s map_set key:%s, value:%s \n", __func__, token,
                      strValue);
          map_set(fp_config_map, &token, strlen(token) + 1, &strValue,
                  strValue_index + 1);
        } else {
          int num_len = floor(log10(abs(numValue))) + 1;
          LOG_EXTNS_D("%s map_set key:%s, value:%d", __func__, token, numValue);
          map_set(fp_config_map, &token, strlen(token) + 1, &numValue, num_len);
        }
        numValue = 0;
        osal_memset(token, '\0', sizeof(token));
        osal_memset(strValue, '\0', sizeof(strValue));
        strValue[0] = '\0';
      }
      break;
    case STR_VALUE:
      if (c == '"') {
        strValue[strValue_index] = '\0';
        state = END_LINE;
        LOG_EXTNS_D("%s map_set key:%s, value:%s \n ", __func__, token,
                    strValue);
        map_set(fp_config_map, &token, strlen(token) + 1, &strValue,
                strlen(strValue) + 1);
      } else if (isPrintable(c))
        strValue[strValue_index++] = c;
      break;
    case END_LINE:
      if (c == '\n' || c == '\r')
        state = BEGIN_LINE;
      break;
    default:
      break;
    }
  }
  if (p_config != NULL) {
    free(p_config);
    p_config = NULL;
  }
  pthread_mutex_unlock(&config_mutex);
  return true;
}

int get_byte_array_value(const char *key, char **p_value,
                         unsigned int *value_len) {
  pthread_mutex_lock(&config_mutex);
  unsigned int value_size;
  void *value = map_get_value(fp_config_map, key, strlen(key) + 1, &value_size);
  if (value != NULL) {
    *p_value = osal_malloc(value_size);
    if (!(*p_value)) {
      LOG_EXTNS_E("%s: osal_malloc failed for key %s", __func__, key);
      pthread_mutex_unlock(&config_mutex);
      return FALSE;
    }
    memcpy(*p_value, value, value_size);
    *value_len = value_size - 1;
    pthread_mutex_unlock(&config_mutex);
    return TRUE;
  } else {
    LOG_EXTNS_E("%s: key %s not found in configuration map", __func__, key);
    pthread_mutex_unlock(&config_mutex);
    return FALSE;
  }
}

int get_byte_value(char *key, unsigned long *p_value, unsigned int *value_len) {
  pthread_mutex_lock(&config_mutex);
  unsigned long *num_value = (unsigned long *)map_get_value(
      fp_config_map, key, strlen(key) + 1, value_len);
  if (num_value != NULL) {
    *p_value = *num_value;
    pthread_mutex_unlock(&config_mutex);
    return TRUE;
  } else {
    LOG_EXTNS_E("%s: Key:%s not found in configuration map\n", __func__, key);
    pthread_mutex_unlock(&config_mutex);
    return FALSE;
  }
}

struct map *get_config_map(void) {
  if (config_map == NULL) {
    LOG_EXTNS_E("%s: config_map is NULL calling map_create \n", __func__);
    config_map = map_create();
  }
  return config_map;
}

static void send_poll_profile_sel_set_config() {
  LOG_EXTNS_D("%s:  \n", __func__);
  int set_config_retry_cnt = 0;
  uint8_t *p_cmd_idle_pwr_off_cfg = (uint8_t *)cmd_idle_pwr_off_cfg;
  do {
    if (EMVCO_STATUS_SUCCESS ==
        send_core_set_config(&p_cmd_idle_pwr_off_cfg[1],
                             p_cmd_idle_pwr_off_cfg[0])) {
      break;
    } else {
      LOG_EMVCOHAL_E("NCI_SET_CONFIG_PROFILE_SELECTION: Failed");
      ++set_config_retry_cnt;
    }
  } while (set_config_retry_cnt < 3);
}

void send_dynamic_set_config(void) {
  struct map *m = get_config_map();
  struct map_node *node = m->head;

  while (node != NULL) {
    int key = atoi((char *)node->key);
    const int8_t *value = (int8_t *)node->value;
    LOG_EXTNS_E("%s: POLL_PROFILE_SEL str_key:%s, value:%d \n", __func__,
                (char *)node->key, *value);
    switch (key) {
    case POLL_PROFILE_SEL:
      cmd_idle_pwr_off_cfg[4] = *value;
      break;
    default:
      break;
    }
    node = node->next;
  }

  send_poll_profile_sel_set_config();

  LOG_EXTNS_D("%s: end \n", __func__);
}