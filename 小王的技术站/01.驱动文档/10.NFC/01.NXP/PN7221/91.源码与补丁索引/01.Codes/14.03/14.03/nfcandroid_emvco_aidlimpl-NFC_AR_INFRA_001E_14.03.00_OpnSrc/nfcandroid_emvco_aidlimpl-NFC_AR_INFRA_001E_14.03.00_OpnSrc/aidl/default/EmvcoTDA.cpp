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
#include "EmvcoTDA.h"
#include "Emvco.h"

namespace aidl {
namespace vendor {
namespace nxp {
namespace emvco {

::ndk::ScopedAStatus EmvcoTDA::registerEMVCoCTListener(
    const std::shared_ptr<INxpEmvcoTDACallback> &in_clientCallback,
    bool *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->registerEMVCoCTListener(in_clientCallback,
                                                       _aidl_return);
}

::ndk::ScopedAStatus EmvcoTDA::discoverTDA(
    std::vector<::aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo> *emvcoTDAInfo) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->discoverTDA(emvcoTDAInfo);
}

::ndk::ScopedAStatus EmvcoTDA::openTDA(int8_t in_tdaID, bool in_standBy,
                                       int8_t *out_connID) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->openTDA(in_tdaID, in_standBy, out_connID);
}
::ndk::ScopedAStatus
EmvcoTDA::transceive(const std::vector<uint8_t> &in_cmd_data,
                     std::vector<uint8_t> *out_rsp_data) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->transceive(in_cmd_data, out_rsp_data);
}
::ndk::ScopedAStatus EmvcoTDA::closeTDA(int8_t in_tdaID, bool in_standBy) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  return Emvco::getInstance()->closeTDA(in_tdaID, in_standBy);
}

} // namespace emvco
} // namespace nxp
} // namespace vendor
} // namespace aidl
