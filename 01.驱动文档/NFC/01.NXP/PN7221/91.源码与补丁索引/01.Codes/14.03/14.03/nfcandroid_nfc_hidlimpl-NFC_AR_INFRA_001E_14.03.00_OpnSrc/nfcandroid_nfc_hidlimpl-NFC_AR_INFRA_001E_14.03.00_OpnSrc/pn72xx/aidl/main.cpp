/******************************************************************************
 *
 *  Copyright 2022,2024 NXP
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


#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <vendor/nxp/nxpnfc/2.0/INxpNfc.h>
#include <thread>

#include "Nfc.h"
#include "NxpNfc.h"
#include "phNxpNciHal_Adaptation.h"
#include <hidl/LegacySupport.h>

using android::sp;
using ::aidl::android::hardware::nfc::Nfc;
using vendor::nxp::nxpnfc::V2_0::INxpNfc;
using vendor::nxp::nxpnfc::V2_0::implementation::NxpNfc;
using android::status_t;
using android::OK;
using namespace std;
using android::hardware::joinRpcThreadpool;
using android::hardware::configureRpcThreadpool;

void startNxpNfcAidlService() {
  ALOGI("NXP NFC Extn Service is starting.");
  configureRpcThreadpool(1, true /*callerWillJoin*/);

  sp<INxpNfc> nxp_nfc_service = new NxpNfc();
    if (nxp_nfc_service == nullptr) {
      ALOGE("Can not create an instance of NXP NFC Extn Iface, exiting.");
      return ;
    }
    status_t status = nxp_nfc_service->registerAsService();
    if (status != OK) {
      ALOGE("Could not register service for NXP NFC Extn Iface (%d).", status);
    }
    ALOGI("NFC service is ready");

    joinRpcThreadpool();
}

int main() {
  ALOGI("NFC AIDL HAL starting up");
  if (!ABinderProcess_setThreadPoolMaxThreadCount(1)) {
    ALOGE("failed to set thread pool max thread count");
    return 1;
  }
  std::shared_ptr<Nfc> nfc_service = ndk::SharedRefBase::make<Nfc>();

  const std::string nfcInstName = std::string() + Nfc::descriptor + "/default";
  binder_status_t status = AServiceManager_addService(
      nfc_service->asBinder().get(), nfcInstName.c_str());
  CHECK(status == STATUS_OK);

  thread t1(startNxpNfcAidlService);
  ABinderProcess_joinThreadPool();
  return 0;
}
