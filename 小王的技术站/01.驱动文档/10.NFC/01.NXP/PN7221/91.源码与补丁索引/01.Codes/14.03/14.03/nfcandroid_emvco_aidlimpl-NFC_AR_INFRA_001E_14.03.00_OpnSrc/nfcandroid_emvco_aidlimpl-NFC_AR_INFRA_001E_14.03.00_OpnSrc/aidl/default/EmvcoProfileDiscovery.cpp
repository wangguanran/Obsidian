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
#include "EmvcoProfileDiscovery.h"

namespace aidl {
namespace vendor {
namespace nxp {
namespace emvco {

::ndk::ScopedAStatus EmvcoProfileDiscovery::registerEMVCoEventListener(
    const std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoClientCallback>
        &in_clientCallback,
    bool *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->registerEMVCoEventListener(in_clientCallback,
                                                          _aidl_return);
}

::ndk::ScopedAStatus EmvcoProfileDiscovery::setEMVCoMode(int8_t in_config,
                                                         bool in_isStartEMVCo) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->setEMVCoMode(in_config, in_isStartEMVCo);
}

::ndk::ScopedAStatus EmvcoProfileDiscovery::registerNFCStateChangeCallback(
    const std::shared_ptr<
        ::aidl::vendor::nxp::emvco::INxpNfcStateChangeRequestCallback>
        &in_nfcStateChangeCallback,
    bool *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->registerNFCStateChangeCallback(
      in_nfcStateChangeCallback, _aidl_return);
}
::ndk::ScopedAStatus
EmvcoProfileDiscovery::onNfcStateChange(NxpNfcState in_nfcState) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->onNfcStateChange(in_nfcState);
}

::ndk::ScopedAStatus EmvcoProfileDiscovery::getCurrentDiscoveryMode(
    ::aidl::vendor::nxp::emvco::NxpDiscoveryMode *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->getCurrentDiscoveryMode(_aidl_return);
}

::ndk::ScopedAStatus EmvcoProfileDiscovery::setLed(
    ::aidl::vendor::nxp::emvco::NxpLedControl in_ledControl,
    ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *emvco_status) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->setLed(in_ledControl, emvco_status);
}
::ndk::ScopedAStatus EmvcoProfileDiscovery::setByteConfig(
    ::aidl::vendor::nxp::emvco::NxpConfigType in_type, int32_t in_length,
    int8_t in_value, ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->setByteConfig(in_type, in_length, in_value,
                                             _aidl_return);
}

} // namespace emvco
} // namespace nxp
} // namespace vendor
} // namespace aidl
