/******************************************************************************
 *
 *  Copyright 2023-2024 NXP
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
#if (NXP_EXTNS == TRUE)
#include "NfcAdaptation.h"
#include "NfcTdaAdaptation.h"
#include <android-base/stringprintf.h>
#include <base/logging.h>

using android::base::StringPrintf;
extern bool nfc_debug_enabled;

extern "C" {
void TdaHalWrite(uint16_t data_len, uint8_t *p_data) {
  DLOG_IF(INFO, nfc_debug_enabled) << StringPrintf("%s Enter", __func__);
  tHAL_NFC_ENTRY *halFuncEntries =
      NfcAdaptation::GetInstance().GetHalEntryFuncs();
  halFuncEntries->write(data_len, p_data);
}
}
#endif
