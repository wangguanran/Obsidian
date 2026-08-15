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

#include "Emvco.h"
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
//#include <cstring>

using ::aidl::vendor::nxp::emvco::Emvco;

int main() {
  LOG(INFO) << "EMVCO HAL starting up";
  if (!ABinderProcess_setThreadPoolMaxThreadCount(1)) {
    LOG(INFO) << "failed to set thread pool max thread count";
    return 1;
  }
  std::shared_ptr<Emvco> emvco_service = Emvco::getInstance();

  const std::string instance = std::string() + Emvco::descriptor + "/default";
  ALOGD_IF(EMVCO_HAL_DEBUG, "EMVCo Registering service: %s", instance.c_str());
  binder_status_t status = AServiceManager_addService(
      emvco_service->asBinder().get(), instance.c_str());
  ALOGD_IF(EMVCO_HAL_DEBUG, "EMVCo Registered IEMVCo service status: %d",
           status);
  CHECK(status == STATUS_OK);
  ABinderProcess_joinThreadPool();

  return EXIT_FAILURE; // should not reach
}
