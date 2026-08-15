/******************************************************************************
 *
 *  Copyright 2018-2021,2023-2024 NXP
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

#include <log/log.h>

#include "NxpNfc.h"
#include "phNxpNciHal.h"
#include "phNxpNciHal_Adaptation.h"

extern bool nfc_debug_enabled;

namespace vendor {
namespace nxp {
namespace nxpnfc {
namespace V2_0 {
namespace implementation {

Return<void> NxpNfc::getVendorParam(const ::android::hardware::hidl_string& key,
                                    getVendorParam_cb _hidl_cb) {
  string val = phNxpNciHal_getSystemProperty(key);
  _hidl_cb(val);
  return Void();
}

Return<bool> NxpNfc::setVendorParam(
    const ::android::hardware::hidl_string& key,
    const ::android::hardware::hidl_string& value) {
  return phNxpNciHal_setSystemProperty(key, value);
}

Return<bool> NxpNfc::resetEse(uint64_t resetType) {
  resetType = 0xff;
  return false;
}

Return<bool> NxpNfc::setEseUpdateState(NxpNfcHalEseState eSEState) {
  ALOGD("NxpNfc::setEseUpdateState Entry %lu ", eSEState);
  bool ret = false;

  if ((isDualCpuConfigure() == false)) {
    NXPLOG_NCIHAL_E("Modeswitch operation is not allowed on Single CPU");
    return ret;
  }

  if ((eSEState == (NxpNfcHalEseState)EMVCO_MODE_SWITCH) ||
      (eSEState == (NxpNfcHalEseState)NFC_MODE_SWITCH) ||
      (eSEState == (NxpNfcHalEseState)SMCU_FW_DNLD_MODE_SWITCH)) {
    ret = phNxpNciHal_DualCPU_modeSwitch((uint8_t)eSEState);
  } else {
    ALOGD("ERROR : %s Invalid option ", __func__);
    return ret;
  }
  ALOGD("NxpNfc::setEseUpdateState Exit ret %d", ret);
  return ret;
}

Return<bool> NxpNfc::setNxpTransitConfig(
    const ::android::hardware::hidl_string& strval) {
  bool status = true;
  ALOGD("NxpNfc::setNxpTransitConfig Entry");

  status = phNxpNciHal_setNxpTransitConfig((char*)strval.c_str());

  ALOGD("NxpNfc::setNxpTransitConfig Exit");
  return status;
}

Return<bool> NxpNfc::isJcopUpdateRequired() {
  return false;
}

Return<bool> NxpNfc::isLsUpdateRequired() {
  return false;
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace nxpnfc
}  // namespace nxp
}  // namespace vendor
