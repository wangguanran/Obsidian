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
#include "LinkedCallback.h"
#include <emvco_common.h>
#include <emvco_hal.h>
#define CHK_STATUS(x)                                                          \
  ((x) == EMVCO_STATUS_SUCCESS)                                                \
      ? (::ndk::ScopedAStatus::ok())                                           \
      : (::ndk::ScopedAStatus::fromStatus(STATUS_FAILED_TRANSACTION))

namespace aidl {
namespace vendor {
namespace nxp {
namespace emvco {

using ::aidl::vendor::nxp::emvco::NxpDiscoveryMode;
using ::aidl::vendor::nxp::emvco::NxpEmvcoEvent;
using ::aidl::vendor::nxp::emvco::NxpEmvcoStatus;

std::vector<std::unique_ptr<LinkedCallback>> Emvco::cl_callbacks_;
std::vector<std::unique_ptr<LinkedCallback>> Emvco::ct_callbacks_;
std::mutex Emvco::callbacks_lock_;
std::shared_ptr<Emvco> Emvco::emvco_service_;
std::shared_ptr<INxpNfcStateChangeRequestCallback>
    Emvco::nfc_State_change_callback = nullptr;
std::shared_ptr<INxpEmvcoTDACallback> Emvco::emvco_tda_callback = nullptr;

void Emvco::setNxpNfcState(bool enableNfc) {
  if (Emvco::nfc_State_change_callback != nullptr) {
    auto ret = Emvco::nfc_State_change_callback->enableNfc(enableNfc);
    if (!ret.isOk()) {
      LOG(ERROR) << "Failed to send event!";
    }
  }
}

void Emvco::eventCallback(uint8_t event, uint8_t status) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  std::lock_guard<std::mutex> lock(callbacks_lock_);
  for (auto &it : cl_callbacks_) {
    it->callback()->sendEvent((NxpEmvcoEvent)event, (NxpEmvcoStatus)status);
  }
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Exit", __func__);
}

void Emvco::dataCallback(uint16_t data_len, uint8_t *p_data) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  std::lock_guard<std::mutex> lock(callbacks_lock_);
  std::vector<uint8_t> data(p_data, p_data + data_len);
  for (auto &it : cl_callbacks_) {
    it->callback()->sendData(data);
  }
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Exit", __func__);
}

void Emvco::onCLStateChange(uint8_t state, char *debugReason) {
  std::string dbgReason = debugReason;
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s state:%d, dbgReason:%s", __func__, state,
           dbgReason.c_str());
  NxpEmvcoState emvcoState = (NxpEmvcoState)(state);
  if (emvco_tda_callback != nullptr) {
    emvco_tda_callback->onEMVCoCLStateChange(emvcoState, dbgReason);
  }
}

void Emvco::onTDAStateChange(void *tda_info, char *debugReason) {
  std::string dbgReason = debugReason;
  tda_t *tda = (tda_t *)tda_info;
  aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo tdaInfo;

  tdaInfo.id = tda->id;
  tdaInfo.status = (NxpEmvcoTDAStatus)tda->status;
  tdaInfo.numberOfProtocols = tda->number_of_protocols;
  std::vector<::aidl::vendor::nxp::emvco::NxpProtocols> protocols;
  if (tdaInfo.numberOfProtocols > 0) {
    for (int j = 0; j < tdaInfo.numberOfProtocols; j++) {
      int protocolVal = (int)(*((uint8_t *)(tda->protocols_t) + j));
      ::aidl::vendor::nxp::emvco::NxpProtocols protocol =
          NxpProtocols(protocolVal);
      protocols.push_back(protocol);
    }
  }
  tdaInfo.protocols = std::move(protocols);
  tdaInfo.numberOfCardInfo = tda->number_of_card_info;
  std::vector<::aidl::vendor::nxp::emvco::NxpCardTLVInfo> cardTLVInfos;
  if (tdaInfo.numberOfCardInfo > 0) {
    for (int k = 0; k < tdaInfo.numberOfCardInfo; k++) {
      ::aidl::vendor::nxp::emvco::NxpCardTLVInfo cardtlvInfo;
      cardtlvInfo.type = tda->card_tlv_info->type;
      cardtlvInfo.length = tda->card_tlv_info->length;
      if (cardtlvInfo.length > 0) {
        std::vector<uint8_t> values;
        for (int l = 0; l < cardtlvInfo.length; l++) {
          uint8_t *value = ((uint8_t *)(tda->card_tlv_info->value) + l);
          values.push_back(*value);
        }
        cardtlvInfo.value = std::move(values);
      }
      cardTLVInfos.push_back(cardtlvInfo);
    }
  }
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: adding cardTLVInfos", __func__);
  tdaInfo.cardTLVInfo = std::move(cardTLVInfos);

  if (emvco_tda_callback != nullptr) {
    emvco_tda_callback->onTDAStateChange(tdaInfo, dbgReason);
  }
}

void OnCallbackDiedWrapped(void *cookie) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  LinkedCallback *linked = reinterpret_cast<LinkedCallback *>(cookie);
  linked->OnCallbackDied();
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Exit", __func__);
}

std::shared_ptr<Emvco> Emvco::getInstance() {
  if (emvco_service_ == nullptr) {
    emvco_service_ = ::ndk::SharedRefBase::make<Emvco>();
    ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  }
  return emvco_service_;
}

Emvco::Emvco()
    : death_recipient_(AIBinder_DeathRecipient_new(&OnCallbackDiedWrapped)) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  open();
}

::ndk::ScopedAStatus Emvco::getEmvcoProfileDiscoveryInterface(
    std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoProfileDiscovery>
        *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (emvco_profile_discovery_ == nullptr) {
    emvco_profile_discovery_ =
        ::ndk::SharedRefBase::make<EmvcoProfileDiscovery>();
  }
  *_aidl_return = emvco_profile_discovery_;
  return ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus Emvco::getEmvcoContactlessCard(
    std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoContactlessCard>
        *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (emvco_contactless_card == nullptr) {
    emvco_contactless_card = ::ndk::SharedRefBase::make<EmvcoContactlessCard>();
  }
  *_aidl_return = emvco_contactless_card;
  return ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus Emvco::getEmvcoTDA(
    std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoTDA> *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (nxp_emvco_tda_ == nullptr) {
    nxp_emvco_tda_ = ::ndk::SharedRefBase::make<EmvcoTDA>();
    ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter created instance", __func__);
  }
  *_aidl_return = nxp_emvco_tda_;
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::registerEMVCoEventListener(
    const std::shared_ptr<::aidl::vendor::nxp::emvco::INxpEmvcoClientCallback>
        &in_clientCallback,
    bool *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  registerCallback(in_clientCallback);
  *_aidl_return = true;
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::getCurrentDiscoveryMode(
    ::aidl::vendor::nxp::emvco::NxpDiscoveryMode *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  *_aidl_return = (NxpDiscoveryMode)get_current_discovery_mode();
  return ndk::ScopedAStatus::ok();
}

binder_status_t Emvco::dump(int fd, const char **p, uint32_t q) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s", __func__);
  (void)fd;
  (void)p;
  (void)q;
  return STATUS_OK;
}

::ndk::ScopedAStatus Emvco::onNfcStateChange(NxpNfcState in_nfcState) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s onNfcStateChange nfcState:%d", __func__,
           (int)in_nfcState);
  on_nfc_state_change((int)in_nfcState);
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::registerNFCStateChangeCallback(
    const std::shared_ptr<
        ::aidl::vendor::nxp::emvco::INxpNfcStateChangeRequestCallback>
        &in_nfcStateChangeCallback,
    bool *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s", __func__);
  Emvco::nfc_State_change_callback = in_nfcStateChangeCallback;
  *_aidl_return = true;
  return ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus Emvco::setEMVCoMode(int8_t in_config,
                                         bool in_isStartEMVCo) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter in_config:%d, in_isStartEMVCo:%d",
           __func__, in_config, in_isStartEMVCo);

  set_emvco_mode(in_config, in_isStartEMVCo);
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: phNxpNciHal_doSetEMVCoMode returned",
           __func__);
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::open() {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  EMVCO_STATUS status =
      open_emvco_app_data_channel(eventCallback, dataCallback, setNxpNfcState,
                                  onTDAStateChange, onCLStateChange);

  return CHK_STATUS(status);
}
::ndk::ScopedAStatus Emvco::transceive(const std::vector<uint8_t> &in_data,
                                       int32_t *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  std::vector<uint8_t> data(in_data.begin(), in_data.end());
  *_aidl_return = send_emvco_app_data(data.size(), (uint8_t *)data.data());
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::close(
    const std::shared_ptr<INxpEmvcoClientCallback> &in_clientCallback) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (in_clientCallback == nullptr) {
    LOG(ERROR) << __func__ << "Client callback is NULL";
  } else {
    unregisterCallback(in_clientCallback);
  }
  EMVCO_STATUS status = close_emvco_app_data_channel(false);
  return CHK_STATUS(status);
}

void Emvco::registerCallback(
    const std::shared_ptr<INxpEmvcoClientCallback> &callback) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (callback == nullptr) {
    LOG(ERROR) << __func__ << "Client callback is NULL";
    return;
  }
  {
    std::lock_guard<std::mutex> lock(callbacks_lock_);
    cl_callbacks_.emplace_back(LinkedCallback::Make(ref<Emvco>(), callback));
    ALOGD_IF(EMVCO_HAL_DEBUG, "Total clients=%d", (int)cl_callbacks_.size());
  }
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Exit", __func__);
}

void Emvco::unregisterCallback(
    const std::shared_ptr<INxpEmvcoClientCallback> &callback) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (callback == nullptr) {
    LOG(ERROR) << __func__ << "Client callback is NULL";
    return;
  }
  std::lock_guard<std::mutex> lock(callbacks_lock_);
  auto matches = [callback](const auto &linked) {
    return linked->callback()->asBinder() == callback->asBinder();
  };
  auto it1 = std::remove_if(cl_callbacks_.begin(), cl_callbacks_.end(),
                            std::move(matches));
  bool removed = (it1 != cl_callbacks_.end());
  if (removed) {
    cl_callbacks_.erase(it1, cl_callbacks_.end());
  } else {
    LOG(ERROR) << __func__ << "unregisterCallback failed";
  }
  ALOGD_IF(EMVCO_HAL_DEBUG, "Total clients = %d", (int)cl_callbacks_.size());
}

void Emvco::registerCTCallback(
    const std::shared_ptr<INxpEmvcoTDACallback> &callback) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (callback == nullptr) {
    LOG(ERROR) << __func__ << "Client callback is NULL";
    return;
  }
  emvco_tda_callback = callback;
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Exit", __func__);
}

void Emvco::unregisterCTCallback(
    const std::shared_ptr<INxpEmvcoTDACallback> &callback) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  if (callback == nullptr) {
    LOG(ERROR) << __func__ << "Client callback is NULL";
    return;
  }
  emvco_tda_callback = nullptr;
}

::ndk::ScopedAStatus Emvco::stopRFDisovery(
    ::aidl::vendor::nxp::emvco::NxpDeactivationType in_deactivationType,
    ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *emvco_status) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: in_deactivationType:%d", __func__,
           in_deactivationType);
  *emvco_status = (NxpEmvcoStatus)stop_rf_discovery((int)in_deactivationType);
  return ndk::ScopedAStatus::ok();
}
::ndk::ScopedAStatus
Emvco::setLed(::aidl::vendor::nxp::emvco::NxpLedControl in_ledControl,
              ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *emvco_status) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: in_ledControl:%d", __func__, in_ledControl);

  *emvco_status = (NxpEmvcoStatus)set_led((int)in_ledControl);
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus
Emvco::setByteConfig(::aidl::vendor::nxp::emvco::NxpConfigType in_type,
                     int32_t in_length, int8_t in_value,
                     ::aidl::vendor::nxp::emvco::NxpEmvcoStatus *emvco_status) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: in_value:%d", __func__, in_value);

  *emvco_status = (NxpEmvcoStatus)set_byte_config((config_type_t)in_type,
                                                  in_length, in_value);
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::registerEMVCoCTListener(
    const std::shared_ptr<INxpEmvcoTDACallback> &in_clientCallback,
    bool *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  Emvco::emvco_tda_callback = in_clientCallback;
  *_aidl_return = true;
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::discoverTDA(
    std::vector<::aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo> *_aidl_return) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  tda_control_t tda_control;
  EMVCO_STATUS status = discover_tda(&tda_control);
  if (status != EMVCO_STATUS_SUCCESS) {
    return ndk::ScopedAStatus::fromServiceSpecificError(status);
  }

  std::vector<::aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo> tdas;
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter, tda_control->num_tda_supported:%d",
           __func__, tda_control.num_tda_supported);

  for (int i = 0; i < tda_control.num_tda_supported; i++) {
    if ((tda_control.p_tda + i) != nullptr) {
      aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo tda;
      tda.id = (tda_control.p_tda + i)->id;
      tda.status = (NxpEmvcoTDAStatus)(tda_control.p_tda + i)->status;
      tda.numberOfProtocols = (tda_control.p_tda + i)->number_of_protocols;
      std::vector<::aidl::vendor::nxp::emvco::NxpProtocols> protocols;
      if (tda.numberOfProtocols > 0) {
        for (int j = 0; j < tda.numberOfProtocols; j++) {
          int protocolVal =
              (int)(*((uint8_t *)((tda_control.p_tda + i)->protocols_t) + j));
          ::aidl::vendor::nxp::emvco::NxpProtocols protocol =
              NxpProtocols(protocolVal);
          protocols.push_back(protocol);
        }
      }
      tda.protocols = std::move(protocols);
      tda.numberOfCardInfo = (tda_control.p_tda + i)->number_of_card_info;

      std::vector<::aidl::vendor::nxp::emvco::NxpCardTLVInfo> cardTLVInfos;
      if (tda.numberOfCardInfo > 0) {
        for (int k = 0; k < tda.numberOfCardInfo; k++) {
          ::aidl::vendor::nxp::emvco::NxpCardTLVInfo cardtlvInfo;
          cardtlvInfo.type = (tda_control.p_tda + i)->card_tlv_info->type;
          cardtlvInfo.length = (tda_control.p_tda + i)->card_tlv_info->length;
          if (cardtlvInfo.length > 0) {
            std::vector<uint8_t> values;
            for (int l = 0; l < cardtlvInfo.length; l++) {
              uint8_t *value =
                  ((uint8_t *)((tda_control.p_tda + i)->card_tlv_info->value) +
                   l);
              values.push_back(*value);
            }
            cardtlvInfo.value = std::move(values);
          }
          cardTLVInfos.push_back(cardtlvInfo);
        }
      }
      tda.cardTLVInfo = std::move(cardTLVInfos);
      tdas.push_back(tda);
    }
  }

  for (aidl::vendor::nxp::emvco::NxpEmvcoTDAInfo tda : tdas) {
    ALOGI("tda ID:%d", tda.id);
    ALOGI("tda status:%d", tda.status);
    ALOGI("tda numberOfProtocols:%d", tda.numberOfProtocols);
    if (tda.numberOfProtocols > 0) {
      std::vector<NxpProtocols> protocols = tda.protocols;
      for (NxpProtocols protocol : protocols) {
        ALOGI("tda protocol:%d", protocol);
      }
    }
    ALOGI("tda numberOfCardInfo:%d", tda.numberOfCardInfo);
    if (tda.numberOfCardInfo > 0) {
      std::vector<NxpCardTLVInfo> cardTLVInfos = tda.cardTLVInfo;
      for (NxpCardTLVInfo cardTLVInfo : cardTLVInfos) {
        ALOGI("tda cardTLVInfo.type:%d", cardTLVInfo.type);
        ALOGI("tda cardTLVInfo.length:%d", cardTLVInfo.length);
        std::vector<uint8_t> values = cardTLVInfo.value;
        for (uint8_t value : values) {
          ALOGI("tda cardTLVInfo.value:%d", value);
        }
      }
    }
  }
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: before assigning full tda to _aidl_return ",
           __func__);
  *_aidl_return = std::move(tdas);
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: after assigning full tda", __func__);
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::openTDA(int8_t in_tdaID, bool in_standBy,
                                    int8_t *out_connID) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter in_tdaID:%d", __func__, in_tdaID);
  EMVCO_STATUS status = open_tda(in_tdaID, in_standBy, out_connID);
  if (status != EMVCO_STATUS_OK) {
    ALOGD_IF(EMVCO_HAL_DEBUG, "%s: status:%d", __func__, status);
    return ndk::ScopedAStatus::fromServiceSpecificError(status);
  }
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::closeTDA(int8_t in_tdaID, bool in_standBy) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  EMVCO_STATUS status = close_tda(in_tdaID, in_standBy);
  if (status != EMVCO_STATUS_OK) {
    ALOGD_IF(EMVCO_HAL_DEBUG, "%s: status:%d", __func__, status);
    return ndk::ScopedAStatus::fromServiceSpecificError(status);
  }
  return ndk::ScopedAStatus::ok();
}

::ndk::ScopedAStatus Emvco::transceive(const std::vector<uint8_t> &in_cmd_data,
                                       std::vector<uint8_t> *out_rsp_data) {
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: Enter", __func__);
  std::vector<uint8_t> result;
  tda_data cmd_apdu, rsp_apdu;
  memset(&cmd_apdu, 0x00, sizeof(cmd_apdu));
  memset(&rsp_apdu, 0x00, sizeof(rsp_apdu));
  cmd_apdu.len = (uint32_t)in_cmd_data.size();
  cmd_apdu.p_data = (uint8_t *)malloc(in_cmd_data.size() * sizeof(uint8_t));
  memcpy(cmd_apdu.p_data, in_cmd_data.data(), cmd_apdu.len);

  if (NULL == cmd_apdu.p_data) {
    ALOGD_IF(EMVCO_HAL_DEBUG, "%s: transceive failed to allocate the Memory!!!",
             __func__);
    /*Return empty vec*/
    *out_rsp_data = result;
    return ndk::ScopedAStatus::ok();
  }

  EMVCO_STATUS status = transceive_tda(&cmd_apdu, &rsp_apdu);
  if (status != EMVCO_STATUS_OK) {
    ALOGD_IF(EMVCO_HAL_DEBUG, "%s: status:%d", __func__, status);
    return ndk::ScopedAStatus::fromServiceSpecificError(status);
  }
  result.resize(rsp_apdu.len);
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: resp size:%d", __func__, rsp_apdu.len);
  memcpy(&result[0], rsp_apdu.p_data, rsp_apdu.len);
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: after memcpy resp size:%d", __func__,
           rsp_apdu.len);

  for (uint8_t data : result) {
    ALOGD_IF(EMVCO_HAL_DEBUG, "%s: data:%02x \t", __func__, data);
  }
  *out_rsp_data = std::move(result);
  ALOGD_IF(EMVCO_HAL_DEBUG, "%s: after rsp_data assignment resp size:%d",
           __func__, rsp_apdu.len);

  if (NULL != cmd_apdu.p_data) {
    free(cmd_apdu.p_data);
    cmd_apdu.p_data = NULL;
  }

  if (NULL != rsp_apdu.p_data) {
    free(rsp_apdu.p_data);
    rsp_apdu.p_data = NULL;
  }
  return ndk::ScopedAStatus::ok();
}

} // namespace emvco
} // namespace nxp
} // namespace vendor
} // namespace aidl
