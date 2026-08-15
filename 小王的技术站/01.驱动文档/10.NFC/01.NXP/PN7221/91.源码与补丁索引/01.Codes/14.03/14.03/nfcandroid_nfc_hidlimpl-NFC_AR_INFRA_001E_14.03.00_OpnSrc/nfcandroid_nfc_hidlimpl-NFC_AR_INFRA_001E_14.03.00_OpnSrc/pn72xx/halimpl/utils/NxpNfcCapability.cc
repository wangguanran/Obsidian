/******************************************************************************
 *
 *  Copyright 2015-2018,2020-2024 NXP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
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
#define LOG_TAG "NxpHal"
#include "NxpNfcCapability.h"
#include <phNxpLog.h>

capability *capability::instance = NULL;
tNFC_chipType capability::chipType = pn7220;
tNfc_featureList nfcFL;

capability::capability() {}

capability *capability::getInstance() {
  if (NULL == instance) {
    instance = new capability();
  }
  return instance;
}

tNFC_chipType capability::processChipType(uint8_t *msg, uint16_t msg_len) {
  if ((msg != NULL) && (msg_len != 0)) {
    if (msg[0] == 0x60 && msg[1] == 0x00) {
      if ((msg[msg_len - 3] == 0x03) && (msg[msg_len - 2] == 0x00) &&
          ((msg[msg_len - 5] == 0x20) || (msg[msg_len - 5] == 0x00))) {
        chipType = pn7220;
      } else if ((msg[msg_len - 3] == 0x03) && (msg[msg_len - 2] == 0x00) &&
                 (msg[msg_len - 5] == 0x21)) {
        chipType = pn7221;
      } else if (((msg[msg_len - 3] == 0x12)) && ((msg[msg_len - 4] == 0x71) ||
                 (msg[msg_len - 4] == 0x61) || (msg[msg_len - 4] == 0x41))) {
        chipType = pn7160;
      } else {
        ALOGD("%s Setting Default ChiptType pn7220", __func__);
        chipType = pn7220;
      }
    } else if (msg[0] == 0x00) {
      if ((msg[offsetFwRomCodeVersion] == 0x03) &&
          ((msg[offsetModelID] == 0x20) || (msg[offsetModelID] == 0x00))) {
        chipType = pn7220;
      } else if ((msg[offsetFwRomCodeVersion] == 0x03) &&
                 (msg[offsetModelID] == 0x21)) {
        chipType = pn7221;
      } else if ((msg[offsetFwRomCodeVersion] == 0x12) &&
                 ((msg[offsetHwVersionpn716x] == 0x71) ||
                  (msg[offsetHwVersionpn716x] == 0x61) ||
                  (msg[offsetHwVersionpn716x] == 0x41))) {
        chipType = pn7160;
      } else {
        ALOGD("%s Setting Default ChiptType pn7220 in FW DNLD Mode", __func__);
        chipType = pn7220;
      }
    } else if (((msg[msg_len - 4]) == 0x61) || ((msg[msg_len - 4]) == 0x71) ||
               ((msg[msg_len - 4]) == 0x41)) {
      chipType = pn7160;
    } else if (offsetHwVersion < msg_len) {
      ALOGD("%s HwVersion : 0x%02x  Product ID : 0x%02x", __func__,
            msg[msg_len - 4], msg[msg_len - 5]);
      switch (msg[msg_len - 5]) {
      case 0x20:
      case 0x00:
        chipType = pn7220;
        break;
      case 0x21:
        chipType = pn7221;
        break;
      default:
        chipType = pn7220;
        break;
      }
    } else {
      ALOGD("%s Wrong msg_len. Setting Default ChiptType pn7220", __func__);
      chipType = pn7220;
    }
  }
  ALOGD("%s Product : %s", __func__, product[chipType]);
  return chipType;
}

uint32_t capability::getFWVersionInfo(uint8_t *msg, uint16_t msg_len) {
  uint32_t versionInfo = 0;
  if ((msg != NULL) && (msg_len != 0)) {
    if (msg[0] == 0x00) {
      versionInfo = msg[offsetFwRomCodeVersion] << 16;
      versionInfo |= msg[offsetFwMajorVersion] << 8;
      versionInfo |= msg[offsetFwMinorVersion];
    }
  }
  return versionInfo;
}
