/*
 * Copyright 2012-2024 NXP
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android-base/file.h>
#include <dlfcn.h>
#include <log/log.h>
#include <phDal4Nfc_messageQueueLib.h>
#include <phDnldNfc.h>
#include <phNxpConfig.h>
#include <phNxpLog.h>
#include <phNxpNciHal.h>
#include <phNxpNciHal_Adaptation.h>
#include <phNxpNciHal_Dnld.h>
#include <phNxpNciHal_ext.h>
#include <phTmlNfc.h>

#include <android-base/stringprintf.h>
#include <sys/stat.h>

#include "NfccTransportFactory.h"
#include "NxpNfcThreadMutex.h"
#include "phNxpNciHal_IoctlOperations.h"
#include "phNxpNciHal_extOperations.h"

#include <sys/system_properties.h>

using android::base::StringPrintf;
using namespace android::hardware::nfc::V1_1;
using namespace android::hardware::nfc::V1_2;
using android::base::WriteStringToFile;
using android::hardware::nfc::V1_1::NfcEvent;

/*********************** Global Variables *************************************/
#define PN547C2_CLOCK_SETTING
#define CORE_RES_STATUS_BYTE 3
#define MAX_NXP_HAL_EXTN_BYTES 10
#define DEFAULT_MINIMAL_FW_VERSION 0x0110DE
#define NFC_PROP_VALUE_MAX 2
bool bEnableMfcExtns = false;
bool bEnableMfcReader = false;
bool bDisableLegacyMfcExtns = true;

/* Processing of ISO 15693 EOF */
extern uint8_t icode_send_eof;
extern uint8_t icode_detected;
static uint8_t cmd_icode_eof[] = {0x00, 0x00, 0x00};
const char *rf_block_name = "NXP_RF_CONF_BLK_";
/* FW download success flag */
static uint8_t fw_download_success = 0;
/* Anti-tearing mechanism sucess flag */
uint8_t anti_tearing_recovery_success = 0;
static uint8_t config_access = false;
static uint8_t config_success = true;
static NfcHalThreadMutex sHalFnLock;

extern phNxpNciClock_t phNxpNciClock;

/* NCI HAL Control structure */
phNxpNciHal_Control_t nxpncihal_ctrl;

/* NXP Poll Profile structure */
phNxpNciProfile_Control_t nxpprofile_ctrl;

/* TML Context */
extern phTmlNfc_Context_t *gpphTmlNfc_Context;
extern spTransport gpTransportObj;

extern void
phTmlNfc_set_fragmentation_enabled(phTmlNfc_i2cfragmentation_t result);

extern NFCSTATUS phNxpNciHal_ext_send_sram_config_to_flash();
extern void phNxpNciHal_prop_conf_lpcd(bool enableLPCD);

nfc_stack_callback_t *p_nfc_stack_cback_backup;
phNxpNci_getCfg_info_t *mGetCfg_info = NULL;
/* global variable to get FW version from NCI response or dl get version
 * response*/
uint32_t wFwVerRsp;
#ifdef NXP_BOOTTIME_UPDATE
ese_update_state_t ese_update = ESE_UPDATE_COMPLETED;
#endif
/* External global variable to get FW version */
extern uint16_t wFwVer;
extern uint8_t gRecFWDwnld;
static uint8_t gRecFwRetryCount; // variable to hold recovery FW retry count
static uint8_t write_unlocked_status = NFCSTATUS_SUCCESS;
uint8_t wFwUpdateReq = false;
uint8_t wRfUpdateReq = false;
uint32_t timeoutTimerId = 0;
// This flag will be used to update the EEPROM if FW DNLD completed successfully
bool isFwDnldTriggered = false;
#ifndef FW_DWNLD_FLAG
uint8_t fw_dwnld_flag = false;
#endif
bool nfc_debug_enabled = true;

phNxpNci_LmNfcADiscCfg_t new_Cfg;

/*  Used to send Callback Transceive data during Mifare Write.
 *  If this flag is enabled, no need to send response to Upper layer */
bool sendRspToUpperLayer = true;

phNxpNciHal_Sem_t config_data;

phNxpNciClock_t phNxpNciClock = {0, {0}, false};

phNxpNciRfSetting_t phNxpNciRfSet = {false, {0}};

phNxpNciMwEepromArea_t phNxpNciMwEepromArea = {false, {0}};

volatile bool_t gsIsFirstHalMinOpen = true;
volatile bool_t gsIsFwRecoveryRequired = false;
void *RfFwRegionDnld_handle = NULL;
fpVerInfoStoreInEeprom_t fpVerInfoStoreInEeprom = NULL;
fpRegRfFwDndl_t fpRegRfFwDndl = NULL;
fpPropConfCover_t fpPropConfCover = NULL;
void *phNxpNciHal_client_thread(void *arg);
/**************** local methods used in this file only ************************/
static void phNxpNciHal_open_complete(NFCSTATUS status);
static void phNxpNciHal_MinOpen_complete(NFCSTATUS status);
static void phNxpNciHal_write_complete(void *pContext,
                                       phTmlNfc_TransactInfo_t *pInfo);
static void phNxpNciHal_read_complete(void *pContext,
                                      phTmlNfc_TransactInfo_t *pInfo);
static void phNxpNciHal_close_complete(NFCSTATUS status);
static void phNxpNciHal_core_initialized_complete(NFCSTATUS status);
static void phNxpNciHal_power_cycle_complete(NFCSTATUS status);
static void
phNxpNciHal_kill_client_thread(phNxpNciHal_Control_t *p_nxpncihal_ctrl);
static void phNxpNciHal_nfccClockCfgRead(void);
static bool phNxpNciHal_LmNfcADiscCfgRead(void);
static NFCSTATUS phNxpNciHal_nfccClockCfgApply(void);
static NFCSTATUS phNxpNciHal_nfccTdaPpsCfgApply(void);
static NFCSTATUS phNxpNciHal_nfccPcdSettingApply(void);
static NFCSTATUS phNxpNciHal_flushSramToFlash(void);
static NFCSTATUS phNxpNciHal_hceLmNfcADiscCfgApply(void);
static void phNxpNciHal_print_res_status(uint8_t *p_rx_data, uint16_t *p_len);
static void phNxpNciHal_enable_i2c_fragmentation();
static NFCSTATUS phNxpNciHal_get_mw_eeprom(void);
static NFCSTATUS phNxpNciHal_set_mw_eeprom(void);
static NFCSTATUS phNxpNciHal_config_t4t_ndef(uint8_t t4tFlag);
static void phNxpNciHal_initialize_debug_enabled_flag();
static void phNxpNciHal_initialize_mifare_flag();
static void phNxpNciHal_UpdateFwStatus(HalNfcFwUpdateStatus fwStatus);
static NFCSTATUS phNxpNciHal_resetDefaultSettings(uint8_t fw_update_req,
                                                  bool keep_config);
static NFCSTATUS phNxpNciHal_force_fw_download(uint8_t seq_handler_offset = 0);
static int phNxpNciHal_MinOpen_Clean(char *nfc_dev_node);
static void phNxpNciHal_CheckAndHandleFwTearDown(void);
static NFCSTATUS
phNxpNciHal_getChipInfoInFwDnldMode(bool bIsVenResetReqd = false);
static uint8_t phNxpNciHal_getSessionInfoInFwDnldMode();
static NFCSTATUS phNxpNciHal_dlResetInFwDnldMode();
static NFCSTATUS phNxpNciHal_enableTmlRead();
static NFCSTATUS phNxpNciHal_CheckRFCmdRespStatus();
static NFCSTATUS phNxpNciHalRFConfigCmdRecSequence();
NFCSTATUS phNxpNciHal_china_tianjin_rf_setting(void);
/******************************************************************************
 * Function         phNxpNciHal_initialize_debug_enabled_flag
 *
 * Description      This function gets the value for nfc_debug_enabled
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpNciHal_initialize_debug_enabled_flag() {
  unsigned long num = 0;
  char valueStr[PROPERTY_VALUE_MAX] = {0};
  if (GetNxpNumValue(NAME_NFC_DEBUG_ENABLED, &num, sizeof(num))) {
    nfc_debug_enabled = (num == 0) ? false : true;
  }

  int len = property_get("nfc.debug_enabled", valueStr, "");
  if (len > 0) {
    // let Android property override .conf variable
    unsigned debug_enabled = 0;
    sscanf(valueStr, "%u", &debug_enabled);
    nfc_debug_enabled = (debug_enabled == 0) ? false : true;
  }
  NXPLOG_NCIHAL_D("nfc_debug_enabled : %d", nfc_debug_enabled);
}

/******************************************************************************
 * Function         phNxpNciHal_client_thread
 *
 * Description      This function is a thread handler which handles all TML and
 *                  NCI messages.
 *
 * Returns          void
 *
 ******************************************************************************/
void *phNxpNciHal_client_thread(void *arg) {
  phNxpNciHal_Control_t *p_nxpncihal_ctrl = (phNxpNciHal_Control_t *)arg;
  phLibNfc_Message_t msg;

  NXPLOG_NCIHAL_D("thread started");

  p_nxpncihal_ctrl->thread_running = 1;

  while (p_nxpncihal_ctrl->thread_running == 1) {
    /* Fetch next message from the NFC stack message queue */
    if (phDal4Nfc_msgrcv(p_nxpncihal_ctrl->gDrvCfg.nClientId, &msg, 0, 0) ==
        -1) {
      NXPLOG_NCIHAL_E("NFC client received bad message");
      continue;
    }

    if (p_nxpncihal_ctrl->thread_running == 0) {
      break;
    }

    switch (msg.eMsgType) {
    case PH_LIBNFC_DEFERREDCALL_MSG: {
      phLibNfc_DeferredCall_t *deferCall =
          (phLibNfc_DeferredCall_t *)(msg.pMsgData);

      REENTRANCE_LOCK();
      deferCall->pCallback(deferCall->pParameter);
      REENTRANCE_UNLOCK();

      break;
    }

    case NCI_HAL_OPEN_CPLT_MSG: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_OPEN_CPLT_EVT,
                                            HAL_NFC_STATUS_OK);
      }
      REENTRANCE_UNLOCK();
      break;
    }

    case NCI_HAL_CLOSE_CPLT_MSG: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_CLOSE_CPLT_EVT,
                                            HAL_NFC_STATUS_OK);
      }
      phNxpNciHal_kill_client_thread(&nxpncihal_ctrl);
      REENTRANCE_UNLOCK();
      break;
    }

    case NCI_HAL_POST_INIT_CPLT_MSG: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_POST_INIT_CPLT_EVT,
                                            HAL_NFC_STATUS_OK);
      }
      REENTRANCE_UNLOCK();
      break;
    }

    case NCI_HAL_PRE_DISCOVER_CPLT_MSG: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_PRE_DISCOVER_CPLT_EVT,
                                            HAL_NFC_STATUS_OK);
      }
      REENTRANCE_UNLOCK();
      break;
    }

    case NCI_HAL_HCI_NETWORK_RESET_MSG: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nxpncihal_ctrl.p_nfc_stack_cback)(
            (uint32_t)NfcEvent::HCI_NETWORK_RESET, HAL_NFC_STATUS_OK);
      }
      REENTRANCE_UNLOCK();
      break;
    }

    case NCI_HAL_ERROR_MSG: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_ERROR_EVT,
                                            HAL_NFC_STATUS_FAILED);
      }
      REENTRANCE_UNLOCK();
      break;
    }

    case NCI_HAL_RX_MSG: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_data_cback != NULL) {
        (*nxpncihal_ctrl.p_nfc_stack_data_cback)(nxpncihal_ctrl.rsp_len,
                                                 nxpncihal_ctrl.p_rsp_data);
      }
      REENTRANCE_UNLOCK();
      break;
    }
    case HAL_NFC_FW_UPDATE_STATUS_EVT: {
      REENTRANCE_LOCK();
      if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
        /* Send the event */
        (*nxpncihal_ctrl.p_nfc_stack_cback)(msg.eMsgType,
                                            *((uint8_t *)msg.pMsgData));
      }
      REENTRANCE_UNLOCK();
      break;
    }
    }
  }

  NXPLOG_NCIHAL_D("NxpNciHal thread stopped");

  return NULL;
}

/******************************************************************************
 * Function         phNxpNciHal_kill_client_thread
 *
 * Description      This function safely kill the client thread and clean all
 *                  resources.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void
phNxpNciHal_kill_client_thread(phNxpNciHal_Control_t *p_nxpncihal_ctrl) {
  NXPLOG_NCIHAL_D("Terminating phNxpNciHal client thread...");

  p_nxpncihal_ctrl->p_nfc_stack_cback = NULL;
  p_nxpncihal_ctrl->p_nfc_stack_data_cback = NULL;
  p_nxpncihal_ctrl->thread_running = 0;

  return;
}

/******************************************************************************
 * Function         phNxpNciHal_CheckIntegrityRecovery
 *
 * Description     This function to enter in recovery if FW download fails with
 *                 check integrity.
 *
 * Returns         NFCSTATUS
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_CheckIntegrityRecovery() {
  NFCSTATUS status = NFCSTATUS_FAILED;
  if (phNxpNciHal_nfcc_core_reset_init(false) == NFCSTATUS_SUCCESS) {
    status = phNxpNciHal_fw_download();
  } else {
    status = NFCSTATUS_FW_CHECK_INTEGRITY_FAILED;
  }
  return status;
}
/******************************************************************************
 * Function         phNxpNciHal_force_fw_download
 *
 * Description     This function, based on the offset provided, will trigger
 *                 Secure FW download sequence.
 *                 It will retry the FW download in case the Check Integrity
 *                 has been failed.
 *
 * Parameters      Offset by which the FW dnld Seq handler shall be triggered.
 *                 e.g. if we want to send only the Check Integrity command,
 *                 then the offset shall be 7.
 *
 * Returns         SUCCESS if FW download is successful else FAIL.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_force_fw_download(uint8_t seq_handler_offset) {
  NFCSTATUS wConfigStatus = NFCSTATUS_SUCCESS;
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  /*Get FW version from device*/
  for (int retry = 1; retry >= 0; retry--) {
    if (phDnldNfc_InitImgInfo() == NFCSTATUS_SUCCESS) {
      break;
    } else {
      phDnldNfc_ReSetHwDevHandle();
      NXPLOG_NCIHAL_E("Image information extraction Failed!!");
      if (!retry)
        return NFCSTATUS_FAILED;
    }
  }

  NXPLOG_NCIHAL_D("FW version for FW file = 0x%x", wFwVer);
  NXPLOG_NCIHAL_D("FW version from device = 0x%x", wFwVerRsp);
  bool bIsNfccDlState = false;
  if (wFwVerRsp == 0) {
    status = phNxpNciHal_getChipInfoInFwDnldMode(true);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("phNxpNciHal_getChipInfoInFwDnldMode Failed");
    }
    bIsNfccDlState = true;
  }
  if (NFCSTATUS_SUCCESS == phNxpNciHal_CheckValidFwVersion()) {
    NXPLOG_NCIHAL_D("FW update required");
    nxpncihal_ctrl.phNxpNciGpioInfo.state = GPIO_UNKNOWN;
    fw_download_success = 0;
    /*We are expecting NFC to be either in NFC or in the FW Download state*/
    status = phNxpNciHal_fw_download(seq_handler_offset, bIsNfccDlState);
    if (status == NFCSTATUS_FW_CHECK_INTEGRITY_FAILED) {
      status = phNxpNciHal_CheckIntegrityRecovery();
    }
    property_set("nfc.fw.downloadmode_force", "0");
    if (status == NFCSTATUS_SUCCESS) {
      wConfigStatus = NFCSTATUS_SUCCESS;
      fw_download_success = TRUE;
    } else if (status == NFCSTATUS_FW_CHECK_INTEGRITY_FAILED ||
               (phNxpNciHal_fw_mw_ver_check() != NFCSTATUS_SUCCESS)) {
      phOsalNfc_Timer_Cleanup();
      phTmlNfc_Shutdown_CleanUp();
      return NFCSTATUS_CMD_ABORTED;
    }

    status = phNxpNciHal_nfcc_core_reset_init();
  }
  return wConfigStatus;
}

/******************************************************************************
 * Function         phNxpNciHal_fw_download
 *
 * Description      This function download the PN72xx secure firmware to IC. If
 *                  firmware version in Android filesystem and firmware in the
 *                  IC is same then firmware download will return with success
 *                  without downloading the firmware.
 *
 * Returns          NFCSTATUS_SUCCESS if firmware download successful
 *                  NFCSTATUS_FAILED in case of failure
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_fw_download(uint8_t seq_handler_offset,
                                  bool bIsNfccDlState) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  phNxpNciHal_UpdateFwStatus(HAL_NFC_FW_UPDATE_START);
  phNxpNciHal_nfccClockCfgRead();

  if (!bIsNfccDlState) {
    NXPLOG_NCIHAL_D("nfcFL.nfccFL._NFCC_DWNLD_MODE %x\n",
                    nfcFL.nfccFL._NFCC_DWNLD_MODE);
    if (nfcFL.chipType != pn7160) {
      status = phTmlNfc_IoCtl(phTmlNfc_e_EnableDownloadMode);
    } else {
      status = phTmlNfc_IoCtl(phTmlNfc_e_EnableDownloadModeWithVenRst);
    }
    if (NFCSTATUS_SUCCESS != status) {
      nxpncihal_ctrl.fwdnld_mode_reqd = FALSE;
      phNxpNciHal_UpdateFwStatus(HAL_NFC_FW_UPDATE_FAILED);
      return NFCSTATUS_FAILED;
    }
  }

  /* Make sure read thread is pending before updating fwdnld_mode_reqd to true*/
  NFCSTATUS readStatus = phNxpNciHal_enableTmlRead();
  if (readStatus != PHNFCSTVAL(CID_NFC_TML, NFCSTATUS_BUSY)) {
    NXPLOG_NCIHAL_E("Read Thread is not pending already. status = 0x%x \n",
                    readStatus);
  }

  nxpncihal_ctrl.fwdnld_mode_reqd = TRUE;
  if (nfcFL.nfccFL._NFCC_DWNLD_MODE == NFCC_DWNLD_WITH_NCI_CMD &&
      (!bIsNfccDlState)) {
    /*NCI_RESET_CMD*/
    static uint8_t cmd_reset_nci_dwnld[] = {0x20, 0x00, 0x01, 0x80};
    status = phNxpNciHal_send_ext_cmd(sizeof(cmd_reset_nci_dwnld),
                                      cmd_reset_nci_dwnld);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Core reset FW download command failed \n");
    }
  }

  if (NFCSTATUS_SUCCESS == status) {
    phTmlNfc_EnableFwDnldMode(true);
    /* Set the obtained device handle to download module */

    phDnldNfc_SetHwDevHandle();
    if (nfcFL.chipType >= sn100u) {
      phDnldNfc_SetI2CFragmentLength(NCI_CMDRESP_MAX_BUFF_SIZE_SNXXX);
    } else {
      phDnldNfc_SetI2CFragmentLength(NCI_CMDRESP_MAX_BUFF_SIZE_PN557);
    }

    NXPLOG_NCIHAL_D("Calling Seq handler for FW Download \n");
    status = phNxpNciHal_fw_download_seq(nxpprofile_ctrl.bClkSrcVal,
                                         nxpprofile_ctrl.bClkFreqVal,
                                         seq_handler_offset);

    if (phNxpNciHal_dlResetInFwDnldMode() != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("DL Reset failed in FW DN mode");
    }

    /* FW download done.Therefore if previous I2C write failed then we can
     * change the state to NFCSTATUS_SUCCESS*/
    write_unlocked_status = NFCSTATUS_SUCCESS;
  } else {
    nxpncihal_ctrl.fwdnld_mode_reqd = FALSE;
    status = NFCSTATUS_FAILED;
  }
  if (NFCSTATUS_SUCCESS == status) {
    isFwDnldTriggered = true;
    phNxpNciHal_UpdateFwStatus(HAL_NFC_FW_UPDATE_SCUCCESS);
  } else {
    phNxpNciHal_UpdateFwStatus(HAL_NFC_FW_UPDATE_FAILED);
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_CheckValidFwVersion
 *
 * Description      This function checks the valid FW for Mobile device.
 *                  If the FW doesn't belong the Mobile device it further
 *                  checks nxp config file to override.
 *
 * Returns          NFCSTATUS_SUCCESS if valid fw version found
 *                  NFCSTATUS_NOT_ALLOWED in case of FW not valid for mobile
 *                  device
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_CheckValidFwVersion(void) {
  NFCSTATUS status = NFCSTATUS_NOT_ALLOWED;
  const unsigned char sfw_infra_major_no = 0x02;
  unsigned char ufw_current_major_no = 0x00;
  uint8_t rom_version = 0xFF & (wFwVerRsp >> 16);
  uint8_t fw_maj_ver = 0xFF & (wFwVerRsp >> 8);

  /* extract the firmware's major no */
  ufw_current_major_no = ((0x00FF) & (wFwVer >> 8U));
  NXPLOG_NCIHAL_D("HAL %s current_major_no = 0x%x", __func__,
                  ufw_current_major_no);
  NXPLOG_NCIHAL_D("%s fw_maj_ver = 0x%x", __func__, fw_maj_ver);
  if (nfcFL.chipType == pn7220 || nfcFL.chipType == pn7160) {
    if (ufw_current_major_no >= fw_maj_ver) {
      /* if file major version is grater than the one from the
         Nfc init command allow FW download
      */
      status = NFCSTATUS_SUCCESS;
    }
    return status;
  }

  if (wFwVerRsp == 0) {
    NXPLOG_NCIHAL_E(
        "FW Version not received by NCI command >>> Force Firmware download");
    status = NFCSTATUS_SUCCESS;
  } else if (ufw_current_major_no == nfcFL._FW_MOBILE_MAJOR_NUMBER) {
    NXPLOG_NCIHAL_E("FW Version 2");
    status = NFCSTATUS_SUCCESS;
  } else if (ufw_current_major_no == sfw_infra_major_no) {
    if (rom_version == FW_MOBILE_ROM_VERSION_PN7720) {
      NXPLOG_NCIHAL_D(" Pn72xx  allow Fw download with major number =  0x%x",
                      ufw_current_major_no);
      status = NFCSTATUS_SUCCESS;
    } else {
      status = NFCSTATUS_NOT_ALLOWED;
    }
  } else {
    NXPLOG_NCIHAL_E("Wrong FW Version >>> Firmware download not allowed");
  }

  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_MinOpen_Clean
 *
 * Description      This function shall be called from phNxpNciHal_MinOpen when
 *                  any unrecoverable error has encountered which needs to mark
 *                  min open as failed, HAL status as closed & deallocate any
 *                  memory if allocated.
 *
 * Returns          This function always returns Failure
 *
 ******************************************************************************/
static int phNxpNciHal_MinOpen_Clean(char *nfc_dev_node) {
  if (nfc_dev_node != NULL) {
    free(nfc_dev_node);
    nfc_dev_node = NULL;
  }
  if (mGetCfg_info != NULL) {
    free(mGetCfg_info);
    mGetCfg_info = NULL;
  }
  /* Report error status */
  phNxpNciHal_cleanup_monitor();
  nxpncihal_ctrl.halStatus = HAL_STATUS_CLOSE;
  return NFCSTATUS_FAILED;
}

/******************************************************************************
 * Function         phNxpNciHal_MinOpen
 *
 * Description      This function initializes the least required resources to
 *                  communicate to NFCC.This is mainly used to communicate to
 *                  NFCC when NFC service is not available.
 *
 *
 * Returns          This function return NFCSTATUS_SUCCESS (0) in case of
 *success In case of failure returns other failure value.
 *
 ******************************************************************************/
int phNxpNciHal_MinOpen() {
  phOsalNfc_Config_t tOsalConfig;
  phTmlNfc_Config_t tTmlConfig;
  char *nfc_dev_node = NULL;
  const uint16_t max_len = 260;
  NFCSTATUS wConfigStatus = NFCSTATUS_SUCCESS;
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  int dnld_retry_cnt = 0;
  NXPLOG_NCIHAL_D("phNxpNci_MinOpen(): enter");

  NfcHalAutoThreadMutex a(sHalFnLock);
  if (nxpncihal_ctrl.halStatus == HAL_STATUS_MIN_OPEN) {
    NXPLOG_NCIHAL_D("phNxpNciHal_MinOpen(): already open");
    return NFCSTATUS_SUCCESS;
  }

  int8_t ret_val = 0x00;

  phNxpNciHal_initialize_debug_enabled_flag();
  /* initialize trace level */
  phNxpLog_InitializeLogLevel();

  /* initialize Mifare flags*/
  phNxpNciHal_initialize_mifare_flag();

  /*Create the timer for extns write response*/
  timeoutTimerId = phOsalNfc_Timer_Create();

  if (phNxpNciHal_init_monitor() == NULL) {
    NXPLOG_NCIHAL_E("Init monitor failed");
    return NFCSTATUS_FAILED;
  }

  CONCURRENCY_LOCK();
  memset(&tOsalConfig, 0x00, sizeof(tOsalConfig));
  memset(&tTmlConfig, 0x00, sizeof(tTmlConfig));
  memset(&nxpprofile_ctrl, 0, sizeof(phNxpNciProfile_Control_t));

  /*Init binary semaphore for Spi Nfc synchronization*/
  if (0 != sem_init(&nxpncihal_ctrl.syncSpiNfc, 0, 1)) {
    NXPLOG_NCIHAL_E("sem_init() FAiled, errno = 0x%02X", errno);
    CONCURRENCY_UNLOCK();
    return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
  }

  /* By default HAL status is HAL_STATUS_OPEN */
  nxpncihal_ctrl.halStatus = HAL_STATUS_OPEN;

  /*nci version NCI_VERSION_2_0 version by default for SN100 chip type*/
  nxpncihal_ctrl.nci_info.nci_version = NCI_VERSION_2_0;
  /* Read the nfc device node name */
  nfc_dev_node = (char *)malloc(max_len * sizeof(char));
  if (nfc_dev_node == NULL) {
    NXPLOG_NCIHAL_D("malloc of nfc_dev_node failed ");
    CONCURRENCY_UNLOCK();
    return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
  } else if (!GetNxpStrValue(NAME_NXP_NFC_DEV_NODE, nfc_dev_node, max_len)) {
    NXPLOG_NCIHAL_D(
        "Invalid nfc device node name keeping the default device node "
        "/dev/nxpnfc");
    strlcpy(nfc_dev_node, "/dev/nxpnfc", (max_len * sizeof(char)));
  }
  unsigned long retval = 0;
  if (!GetNxpNumValue(NAME_NXP_CHIP_TYPE, &retval, sizeof(unsigned long))) {
    NXPLOG_NCIHAL_E("Reading of NXP_CHIP_TYPE failed. Default retval = %lu",
                    retval);
  }
  if (retval == 0x01) {
    nfcFL.chipType = pn7160;
  } else if (retval == 0x04) {
    nfcFL.chipType = pn7220;
  } else {
    nfcFL.chipType = pn7220;
  }
  /* Configure hardware link */
  nxpncihal_ctrl.gDrvCfg.nClientId = phDal4Nfc_msgget(0, 0600);
  nxpncihal_ctrl.gDrvCfg.nLinkType = ENUM_LINK_TYPE_I2C; /* For PN72xx */
  tTmlConfig.pDevName = (int8_t *)nfc_dev_node;
  tOsalConfig.dwCallbackThreadId = (uintptr_t)nxpncihal_ctrl.gDrvCfg.nClientId;
  tOsalConfig.pLogFile = NULL;
  tTmlConfig.dwGetMsgThreadId = (uintptr_t)nxpncihal_ctrl.gDrvCfg.nClientId;
  mGetCfg_info = NULL;
  mGetCfg_info =
      (phNxpNci_getCfg_info_t *)nxp_malloc(sizeof(phNxpNci_getCfg_info_t));
  if (mGetCfg_info == NULL) {
    CONCURRENCY_UNLOCK();
    return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
  }
  memset(mGetCfg_info, 0x00, sizeof(phNxpNci_getCfg_info_t));

  /* Set Default Fragment Length */
  tTmlConfig.fragment_len = NCI_CMDRESP_MAX_BUFF_SIZE_PN557;

  /* Initialize TML layer */
  wConfigStatus = phTmlNfc_Init(&tTmlConfig);
  if (wConfigStatus != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("phTmlNfc_Init Failed");
    CONCURRENCY_UNLOCK();
    return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
  } else {
    if (nfc_dev_node != NULL) {
      free(nfc_dev_node);
      nfc_dev_node = NULL;
    }
  }

  /* Create the client thread */
  ret_val = pthread_create(&nxpncihal_ctrl.client_thread, NULL,
                           phNxpNciHal_client_thread, &nxpncihal_ctrl);
  if (ret_val != 0) {
    NXPLOG_NCIHAL_E("pthread_create failed");
    wConfigStatus = phTmlNfc_Shutdown_CleanUp();
    CONCURRENCY_UNLOCK();
    return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
  }

  CONCURRENCY_UNLOCK();
  /* call read pending */
  status = phTmlNfc_Read(
      nxpncihal_ctrl.p_rsp_data, NCI_MAX_DATA_LEN,
      (pphTmlNfc_TransactCompletionCb_t)&phNxpNciHal_read_complete, NULL);
  if (status != NFCSTATUS_PENDING) {
    NXPLOG_NCIHAL_E("TML Read status error status = %x", status);
    wConfigStatus = phTmlNfc_Shutdown_CleanUp();
    wConfigStatus = NFCSTATUS_FAILED;
    return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
  }

  if ((gsIsFirstHalMinOpen) && (isDualCpuConfigure() == false)) {
    phNxpNciHal_CheckAndHandleFwTearDown();
  }

  uint8_t seq_handler_offset = 0x00;
  uint8_t fw_update_req = 1;
  uint8_t rf_update_req;
  phNxpNciHal_ext_init();

  phTmlNfc_IoCtl(phTmlNfc_e_EnableVen);

#if (NXP_EXTNS == TRUE)
  if (nfcFL.chipType != pn7160) {
  /*mode switch gpio triggers core reset notification. Avoid core reset
   * notification propagating to upper layer since it is initiated from HAL*/
  nxpncihal_ctrl.nci_info.wait_for_ntf = TRUE;
  HAL_ENABLE_EXT();

  if (NFCSTATUS_SUCCESS == phTmlNfc_IoCtl(phTmlNfc_e_ModeSwitchOff)) {
    NXPLOG_NCIHAL_D("phTmlNfc_e_ModeSwitchOff - SUCCESS\n");
  } else {
    NXPLOG_NCIHAL_D("phTmlNfc_e_ModeSwitchOff - FAILED\n");
  }

  if (NFCSTATUS_SUCCESS == phTmlNfc_IoCtl(phTmlNfc_e_SmcuModeSwitchOff)) {
    NXPLOG_NCIHAL_D("phTmlNfc_e_SmcuModeSwitchOff - SUCCESS\n");
  } else {
    NXPLOG_NCIHAL_D("phTmlNfc_e_SmcuModeSwitchOff - FAILED\n");
  }
}
#endif
  /* reset version info new version info will be fetch */
  wFwVerRsp = 0x00;
  wFwVer = 0x00;
  if (NFCSTATUS_SUCCESS == phNxpNciHal_nfcc_core_reset_init(true)) {

    setNxpFwConfigPath();
    if (nfcFL.chipType == pn7160) {
      phNxpNciHal_enable_i2c_fragmentation();
    }
    status = phNxpNciHal_CheckFwRegFlashRequired(&fw_update_req, &rf_update_req,
                                                 false);
    if (status != NFCSTATUS_OK) {
      NXPLOG_NCIHAL_D(
          "phNxpNciHal_CheckFwRegFlashRequired() failed:exit status = %x",
          status);
      fw_update_req = FALSE;
      rf_update_req = FALSE;
    }

    if (!wFwUpdateReq) {
      NXPLOG_NCIHAL_D("FW update not required");
      property_set("nfc.fw.downloadmode_force", "0");
      phDnldNfc_ReSetHwDevHandle();
    }
  } else {
    phNxpNciHal_getChipInfoInFwDnldMode(true);
  }

  if (gsIsFirstHalMinOpen && gsIsFwRecoveryRequired) {
    NXPLOG_NCIHAL_E("FW Recovery is required");
    fw_update_req = true;
  }

  do {
    if (fw_update_req && !fw_download_success) {
      gsIsFwRecoveryRequired = false;
      status = phNxpNciHal_force_fw_download(seq_handler_offset);
      if (status == NFCSTATUS_CMD_ABORTED) {
        return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
      } else if (fw_download_success) {
        wConfigStatus = NFCSTATUS_SUCCESS;
      }
    }
    status = phNxpNciHal_resetDefaultSettings(
        fw_update_req, fw_download_success ? false : true);

    if ((status != NFCSTATUS_SUCCESS && fw_download_success) ||
        (gsIsFwRecoveryRequired && fw_update_req)) {
      NXPLOG_NCIHAL_E("FW Recovery required, Perform Force FW Download "
                      "gsIsFwRecoveryRequired %d",
                      gsIsFwRecoveryRequired);
      fw_update_req = 1;
      dnld_retry_cnt++;
    } else if (status != NFCSTATUS_SUCCESS) {
      return phNxpNciHal_MinOpen_Clean(nfc_dev_node);
    } else {
      break;
    }

    if (dnld_retry_cnt > 1) {
      wConfigStatus = NFCSTATUS_FAILED;
      break;
    }

  } while (status != NFCSTATUS_SUCCESS || gsIsFwRecoveryRequired);

  /* Update the EEPROM area if libnfc-nxp-eeprom.conf modified*/
  if (isNxpEepromConfigModified() || (isFwDnldTriggered == true)) {
    unsigned long num = 0;
    int ret = 0;

    if (phNxpNciHal_nfccClockCfgApply() != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("phNxpNciHal_nfccClockCfgApply failed");
    }

    if (phNxpNciHal_hceLmNfcADiscCfgApply() != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("phNxpNciHal_hceLmNfcADiscCfgApply failed");
    }

    if ((nfcFL.chipType >= pn7220) &&
        (phNxpNciHal_nfccTdaPpsCfgApply() != NFCSTATUS_SUCCESS)) {
      NXPLOG_NCIHAL_E("phNxpNciHal_nfccTdaPpsCfgApply failed");
    }

    if ((nfcFL.chipType >= pn7220) &&
        (phNxpNciHal_nfccPcdSettingApply() != NFCSTATUS_SUCCESS)) {
      NXPLOG_NCIHAL_E("phNxpNciHal_nfccPcdSettingApply failed");
    }

    ret = GetNxpNumValue(NAME_NXP_ENABLE_DISABLE_LPCD, &num, sizeof(num));
    if (ret && num == 1) {
      ret = GetNxpNumValue(NAME_NXP_ENABLE_DISABLE_STANBY, &num, sizeof(num));
      if (ret && num == 1) {
        phNxpNciHal_prop_conf_lpcd(true);
      } else {
        NXPLOG_NCIHAL_E(
            "Failed to enable LPCD as Standby config is not enabled");
      }
    } else if (ret && num == 0) {
      phNxpNciHal_prop_conf_lpcd(false);
    }

    /* VEN Reset is mandatory post EEPROM (0xA2 Config) update */
    if (NFCSTATUS_SUCCESS == phTmlNfc_IoCtl(phTmlNfc_e_ResetDevice)) {
      NXPLOG_NCIHAL_D("VEN Reset - SUCCESS\n");
    } else {
      NXPLOG_NCIHAL_D("VEN Reset - FAILED\n");
    }
    /* Core reset and core init must be perform post VEN reset */
    if (NFCSTATUS_SUCCESS != phNxpNciHal_nfcc_core_reset_init(true)) {
      NXPLOG_NCIHAL_E("Fail to perform core reset post ven reset\n");
    }
  }

  /* Update if libnfc-nxp-rf-ext.conf modified*/
  if (isNxpRfExtConfigModified() || (isFwDnldTriggered == true)) {
    unsigned long numOfRfExtConfig = 0;
    int ret = 0;
    uint8_t buff[260] = {0};
    long rlen = 0;
    long bufflen = 260;
    int isfound;
    char baseName[] = "NXP_RFEXT_CONFIG_";
    char rfConfigName[25];

    ret = GetNxpNumValue(NAME_NXP_NUM_OF_RFEXT_CONFIG, &numOfRfExtConfig,
                         sizeof(numOfRfExtConfig));
    if (ret) {
      for (int i = 1; i <= (int)numOfRfExtConfig; i++) {

        snprintf(rfConfigName, sizeof(rfConfigName), "%s%d", baseName, i);
        isfound =
            GetNxpByteArrayValue(rfConfigName, (char *)buff, bufflen, &rlen);
        if ((isfound == 1) && (rlen > 0)) {
          status = phNxpNciHal_send_ext_cmd(rlen, buff);
          if (status != NFCSTATUS_SUCCESS) {
            NXPLOG_NCIHAL_E("NXP RF Ext failed");
          }
        }
      }
    }
  }

  /* Call open complete */
  phNxpNciHal_MinOpen_complete(wConfigStatus);
  NXPLOG_NCIHAL_D("phNxpNciHal_MinOpen(): exit");
  return wConfigStatus;
}

/******************************************************************************
 * Function         phNxpNciHal_open
 *
 * Description      This function is called by libnfc-nci during the
 *                  initialization of the NFCC. It opens the physical connection
 *                  with NFCC (PN72xx) and creates required client thread for
 *                  operation.
 *                  After open is complete, status is informed to libnfc-nci
 *                  through callback function.
 *
 * Returns          This function return NFCSTATUS_SUCCESS (0) in case of
 *success In case of failure returns other failure value.
 *
 ******************************************************************************/
int phNxpNciHal_open(nfc_stack_callback_t *p_cback,
                     nfc_stack_data_callback_t *p_data_cback) {
  NFCSTATUS wConfigStatus = NFCSTATUS_SUCCESS;
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  NXPLOG_NCIHAL_E("HAL imple phNxpNciHal_open NFC HAL OPEN");
  if (nxpncihal_ctrl.halStatus == HAL_STATUS_OPEN) {
    NXPLOG_NCIHAL_D("phNxpNciHal_open already open");
    phNxpNciHal_open_complete(wConfigStatus);
    return wConfigStatus;
  } else if (nxpncihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
    memset(&nxpncihal_ctrl, 0x00, sizeof(nxpncihal_ctrl));
    nxpncihal_ctrl.p_nfc_stack_cback = p_cback;
    nxpncihal_ctrl.p_nfc_stack_data_cback = p_data_cback;
    status = phNxpNciHal_MinOpen();
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("phNxpNciHal_MinOpen failed");
      goto clean_and_return;
    } /*else its already in MIN_OPEN state. continue with rest of
         functionality*/
  } else {
    nxpncihal_ctrl.p_nfc_stack_cback = p_cback;
    nxpncihal_ctrl.p_nfc_stack_data_cback = p_data_cback;
  }
  /* Call open complete */
  phNxpNciHal_open_complete(wConfigStatus);

  return wConfigStatus;

clean_and_return:
  CONCURRENCY_UNLOCK();
  /* Report error status */
  if (p_cback != NULL) {
    (*p_cback)(HAL_NFC_OPEN_CPLT_EVT, HAL_NFC_STATUS_FAILED);
  }

  nxpncihal_ctrl.p_nfc_stack_cback = NULL;
  nxpncihal_ctrl.p_nfc_stack_data_cback = NULL;
  phNxpNciHal_cleanup_monitor();
  nxpncihal_ctrl.halStatus = HAL_STATUS_CLOSE;
  return NFCSTATUS_FAILED;
}

/******************************************************************************
 * Function         phNxpNciHal_fw_mw_check
 *
 * Description      This function inform the status of phNxpNciHal_fw_mw_check
 *                  function to libnfc-nci.
 *
 * Returns          int.
 *
 ******************************************************************************/
int phNxpNciHal_fw_mw_ver_check() {
  NFCSTATUS status = NFCSTATUS_FAILED;
  uint8_t rom_version = 0xFF & (wFwVerRsp >> 16);
  uint8_t fw_maj_ver = 0xFF & (wFwVerRsp >> 8);

  if ((nfcFL.chipType == sn100u) && (rom_version == SN1XX_ROM_VERSION) &&
      (fw_maj_ver == SN1XX_FW_MAJOR_VERSION)) {
    status = NFCSTATUS_SUCCESS;
#if (NXP_EXTNS == TRUE)
  } else if ((nfcFL.chipType >= pn7220) &&
             (rom_version == FW_MOBILE_ROM_VERSION_PN7720) &&
             (fw_maj_ver == 0x00)) {
    status = NFCSTATUS_SUCCESS;
#endif
  } else if ((nfcFL.chipType == sn220u) && (rom_version == SN2XX_ROM_VERSION) &&
             (fw_maj_ver == SN2XX_FW_MAJOR_VERSION)) {
    status = NFCSTATUS_SUCCESS;
  } else if ((nfcFL.chipType == pn7160) && (rom_version == FW_MOBILE_ROM_VERSION_PN7160) &&
      (fw_maj_ver == 0x50)) {
        status = NFCSTATUS_SUCCESS;
  }
  if (NFCSTATUS_SUCCESS != status) {
    NXPLOG_NCIHAL_D("Chip Version Middleware Version mismatch!!!!");
  }
  return status;
}
/******************************************************************************
 * Function         phNxpNciHal_MinOpen_complete
 *
 * Description      This function updates the status of
 *phNxpNciHal_MinOpen_complete to halstatus.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_MinOpen_complete(NFCSTATUS status) {
  gsIsFirstHalMinOpen = false;
  if (status == NFCSTATUS_SUCCESS) {
    nxpncihal_ctrl.halStatus = HAL_STATUS_MIN_OPEN;
  }

  return;
}

/******************************************************************************
 * Function         phNxpNciHal_open_complete
 *
 * Description      This function inform the status of phNxpNciHal_open
 *                  function to libnfc-nci.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_open_complete(NFCSTATUS status) {
  static phLibNfc_Message_t msg;

  if (status == NFCSTATUS_SUCCESS) {
    msg.eMsgType = NCI_HAL_OPEN_CPLT_MSG;
    nxpncihal_ctrl.hal_open_status = true;
    nxpncihal_ctrl.halStatus = HAL_STATUS_OPEN;
  } else {
    msg.eMsgType = NCI_HAL_ERROR_MSG;
  }

  msg.pMsgData = NULL;
  msg.Size = 0;

  phTmlNfc_DeferredCall(gpphTmlNfc_Context->dwCallbackThreadId,
                        (phLibNfc_Message_t *)&msg);

  return;
}

/******************************************************************************
 * Function         phNxpNciHal_write
 *
 * Description      This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN72xx driver interface.
 *                  Before sending the data to NFCC, phNxpNciHal_write_ext
 *                  is called to check if there is any extension processing
 *                  is required for the NCI packet being sent out.
 *
 * Returns          It returns number of bytes successfully written to NFCC.
 *
 ******************************************************************************/
int phNxpNciHal_write(uint16_t data_len, const uint8_t *p_data) {
  if (bDisableLegacyMfcExtns && bEnableMfcExtns && p_data[0] == 0x00) {
    return NxpMfcReaderInstance.Write(data_len, p_data);
  }
  return phNxpNciHal_write_internal(data_len, p_data);
}
/******************************************************************************
 * Function         phNxpNciHal_txNfccClockSetCmd
 *
 * Description      This function is called after successfull download
 *                  to apply the clock setting provided in config file
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpNciHal_txNfccClockSetCmd(void) {
  NFCSTATUS status = NFCSTATUS_FAILED;
  uint8_t set_clock_cmd[] = {0x20, 0x02, 0x05, 0x01,
                                    0xA0, 0x03, 0x01, 0x08};
  uint8_t setClkCmdLen = sizeof(set_clock_cmd);
  unsigned long clockSource = 0;
  unsigned long frequency = 0;
  uint32_t pllSetRetryCount = 3, dpllSetRetryCount = 3,
           setClockCmdWriteRetryCnt = 0;
  uint8_t *pCmd4PllSetting = NULL;
  uint8_t *pCmd4DpllSetting = NULL;
  uint32_t pllCmdLen = 0, dpllCmdLen = 0;
  int srcCfgFound = 0, freqCfgFound = 0;
  srcCfgFound = (GetNxpNumValue(NAME_NXP_SYS_CLK_SRC_SEL, &clockSource,
                                sizeof(clockSource)) > 0);
  freqCfgFound = (GetNxpNumValue(NAME_NXP_SYS_CLK_FREQ_SEL, &frequency,
                                 sizeof(frequency)) > 0);
  NXPLOG_NCIHAL_D("%s : clock source = %lu, frequency = %lu", __FUNCTION__,
                  clockSource, frequency);
  if (srcCfgFound && freqCfgFound && (clockSource == CLK_SRC_PLL)) {
    phNxpNciClock.isClockSet = TRUE;
    switch (frequency) {
    case CLK_FREQ_13MHZ: {
      NXPLOG_NCIHAL_D("PLL setting for CLK_FREQ_13MHZ");
      pCmd4PllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_PLL_13MHZ;
      pllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_PLL_13MHZ);
      pCmd4DpllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_DPLL_13MHZ;
      dpllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_DPLL_13MHZ);
      break;
    }
    case CLK_FREQ_19_2MHZ: {
      NXPLOG_NCIHAL_D("PLL setting for CLK_FREQ_19_2MHZ");
      pCmd4PllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_PLL_19_2MHZ;
      pllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_PLL_19_2MHZ);
      pCmd4DpllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_DPLL_19_2MHZ;
      dpllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_DPLL_19_2MHZ);
      break;
    }
    case CLK_FREQ_24MHZ: {
      NXPLOG_NCIHAL_D("PLL setting for CLK_FREQ_24MHZ");
      pCmd4PllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_PLL_24MHZ;
      pllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_PLL_24MHZ);
      pCmd4DpllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_DPLL_24MHZ;
      dpllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_DPLL_24MHZ);
      break;
    }
    case CLK_FREQ_26MHZ: {
      NXPLOG_NCIHAL_D("PLL setting for CLK_FREQ_26MHZ");
      pCmd4PllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_PLL_26MHZ;
      pllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_PLL_26MHZ);
      pCmd4DpllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_DPLL_26MHZ;
      dpllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_DPLL_26MHZ);
      break;
    }
    case CLK_FREQ_38_4MHZ: {
      NXPLOG_NCIHAL_D("PLL setting for CLK_FREQ_38_4MHZ");
      pCmd4PllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_PLL_38_4MHZ;
      pllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_PLL_38_4MHZ);
      pCmd4DpllSetting = (uint8_t *)PN7160_SET_CONFIG_CMD_DPLL_38_4MHZ;
      dpllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_DPLL_38_4MHZ);
      break;
    }
    case CLK_FREQ_48MHZ: {
        NXPLOG_NCIHAL_D("PLL setting for CLK_FREQ_48MHZ");
        pCmd4PllSetting = (uint8_t*)PN7160_SET_CONFIG_CMD_PLL_48MHZ;
        pllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_PLL_48MHZ);
        pCmd4DpllSetting = (uint8_t*)PN7160_SET_CONFIG_CMD_DPLL_48MHZ;
        dpllCmdLen = sizeof(PN7160_SET_CONFIG_CMD_DPLL_48MHZ);
        break;
    }
    default:
      phNxpNciClock.isClockSet = FALSE;
      NXPLOG_NCIHAL_E("ERROR: Invalid clock frequency!!");
      return;
    }
  }
  switch (clockSource) {
  case CLK_SRC_PLL: {
    set_clock_cmd[setClkCmdLen - 1] = 0x00;
    while (status != NFCSTATUS_SUCCESS &&
           setClockCmdWriteRetryCnt++ < MAX_RETRY_COUNT)
      status = phNxpNciHal_send_ext_cmd(setClkCmdLen, set_clock_cmd);
    status = NFCSTATUS_FAILED;
    while (status != NFCSTATUS_SUCCESS && pllSetRetryCount-- > 0)
      status = phNxpNciHal_send_ext_cmd(pllCmdLen, pCmd4PllSetting);
    status = NFCSTATUS_FAILED;
    while (status != NFCSTATUS_SUCCESS && dpllSetRetryCount-- > 0)
      status = phNxpNciHal_send_ext_cmd(dpllCmdLen, pCmd4DpllSetting);
    break;
  }
  case CLK_SRC_XTAL: {
    status = phNxpNciHal_send_ext_cmd(setClkCmdLen, set_clock_cmd);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("XTAL clock setting failed !!");
    }
    break;
  }
  default:
    NXPLOG_NCIHAL_E("Wrong clock source. Dont apply any modification");
    return;
  }
  phNxpNciClock.isClockSet = FALSE;
  if (status == NFCSTATUS_SUCCESS &&
      phNxpNciClock.p_rx_data[3] == NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("PLL and DPLL settings applied successfully");
  }
  return;
}

/******************************************************************************
 * Function         phNxpNciHal_write_internal
 *
 * Description      This function write the data to NFCC through physical
 *                  interface (e.g. I2C) using the PN72xx driver interface.
 *                  Before sending the data to NFCC, phNxpNciHal_write_ext
 *                  is called to check if there is any extension processing
 *                  is required for the NCI packet being sent out.
 *
 * Returns          It returns number of bytes successfully written to NFCC.
 *
 ******************************************************************************/
int phNxpNciHal_write_internal(uint16_t data_len, const uint8_t *p_data) {
  NFCSTATUS status = NFCSTATUS_FAILED;
  static phLibNfc_Message_t msg;
  if (nxpncihal_ctrl.halStatus != HAL_STATUS_OPEN) {
    return NFCSTATUS_FAILED;
  }
  /* Create local copy of cmd_data */
  memcpy(nxpncihal_ctrl.p_cmd_data, p_data, data_len);
  nxpncihal_ctrl.cmd_len = data_len;
  if ((nxpncihal_ctrl.cmd_len + MAX_NXP_HAL_EXTN_BYTES) > NCI_MAX_DATA_LEN) {
    NXPLOG_NCIHAL_D("cmd_len exceeds limit NCI_MAX_DATA_LEN");
    goto clean_and_return;
  }
#ifdef P2P_PRIO_LOGIC_HAL_IMP
  /* Specific logic to block RF disable when P2P priority logic is busy */
  if (p_data[0] == 0x21 && p_data[1] == 0x06 && p_data[2] == 0x01 &&
      EnableP2P_PrioLogic == true) {
    NXPLOG_NCIHAL_D("P2P priority logic busy: Disable it.");
    phNxpNciHal_clean_P2P_Prio();
  }
#endif

  /* Check for NXP ext before sending write */
  status =
      phNxpNciHal_write_ext(&nxpncihal_ctrl.cmd_len, nxpncihal_ctrl.p_cmd_data,
                            &nxpncihal_ctrl.rsp_len, nxpncihal_ctrl.p_rsp_data);
  if (status != NFCSTATUS_SUCCESS) {
    /* Do not send packet to PN72xx, send response directly */
    msg.eMsgType = NCI_HAL_RX_MSG;
    msg.pMsgData = NULL;
    msg.Size = 0;

    phTmlNfc_DeferredCall(gpphTmlNfc_Context->dwCallbackThreadId,
                          (phLibNfc_Message_t *)&msg);
    goto clean_and_return;
  }

  CONCURRENCY_LOCK();
  data_len = phNxpNciHal_write_unlocked(nxpncihal_ctrl.cmd_len,
                                        nxpncihal_ctrl.p_cmd_data, ORIG_LIBNFC);
  CONCURRENCY_UNLOCK();
  if (nfcFL.chipType < sn100u && icode_send_eof == 1) {
    usleep(10000);
    icode_send_eof = 2;
    status = phNxpNciHal_send_ext_cmd(3, cmd_icode_eof);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("ICODE end of frame command failed");
    }
  }

clean_and_return:
  /* No data written */
  return data_len;
}

/******************************************************************************
 * Function         phNxpNciHal_write_unlocked
 *
 * Description      This is the actual function which is being called by
 *                  phNxpNciHal_write. This function writes the data to NFCC.
 *                  It waits till write callback provide the result of write
 *                  process.
 *
 * Returns          It returns number of bytes successfully written to NFCC.
 *
 ******************************************************************************/
int phNxpNciHal_write_unlocked(uint16_t data_len, const uint8_t *p_data,
                               int origin) {
  NFCSTATUS status = NFCSTATUS_INVALID_PARAMETER;
  phNxpNciHal_Sem_t cb_data;
  nxpncihal_ctrl.retry_cnt = 0;
  int sem_val = 0;
  static uint8_t reset_ntf[] = {0x60, 0x00, 0x06, 0xA0, 0x00,
                                0xC7, 0xD4, 0x00, 0x00};
  /* Create the local semaphore */
  if (phNxpNciHal_init_cb_data(&cb_data, NULL) != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("phNxpNciHal_write_unlocked Create cb data failed");
    data_len = 0;
    goto clean_and_return;
  }

  /* Create local copy of cmd_data */
  memcpy(nxpncihal_ctrl.p_cmd_data, p_data, data_len);
  nxpncihal_ctrl.cmd_len = data_len;
  write_unlocked_status = NFCSTATUS_FAILED;
  /* check for write synchronyztion */
  if (phNxpNciHal_check_ncicmd_write_window(nxpncihal_ctrl.cmd_len,
                                            nxpncihal_ctrl.p_cmd_data) !=
      NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("phNxpNciHal_write_unlocked  CMD window  check failed");
    data_len = 0;
    goto clean_and_return;
  }

  if (origin == ORIG_NXPHAL)
    HAL_ENABLE_EXT();

retry:

  data_len = nxpncihal_ctrl.cmd_len;

  status = phTmlNfc_Write(
      (uint8_t *)nxpncihal_ctrl.p_cmd_data, (uint16_t)nxpncihal_ctrl.cmd_len,
      (pphTmlNfc_TransactCompletionCb_t)&phNxpNciHal_write_complete,
      (void *)&cb_data);
  if (status != NFCSTATUS_PENDING) {
    NXPLOG_NCIHAL_E("write_unlocked status error");
    data_len = 0;
    goto clean_and_return;
  }

  /* Wait for callback response */
  if (SEM_WAIT(cb_data)) {
    NXPLOG_NCIHAL_E("write_unlocked semaphore error");
    data_len = 0;
    goto clean_and_return;
  }

  if (cb_data.status != NFCSTATUS_SUCCESS) {
    data_len = 0;
    if (nxpncihal_ctrl.retry_cnt++ < MAX_RETRY_COUNT) {
      NXPLOG_NCIHAL_D(
          "write_unlocked failed - PN72xx Maybe in Standby Mode - Retry");
      /* 10ms delay to give NFCC wake up delay */
      usleep(1000 * 10);
      goto retry;
    } else {
      NXPLOG_NCIHAL_E(
          "write_unlocked failed - PN72xx Maybe in Standby Mode (max count = "
          "0x%x)",
          nxpncihal_ctrl.retry_cnt);

      status = phTmlNfc_IoCtl(phTmlNfc_e_ResetDevice);

      if (NFCSTATUS_SUCCESS == status) {
        NXPLOG_NCIHAL_D("PN72xx Reset - SUCCESS\n");
      } else {
        NXPLOG_NCIHAL_D("PN72xx Reset - FAILED\n");
      }
      if (nxpncihal_ctrl.p_nfc_stack_data_cback != NULL &&
          nxpncihal_ctrl.hal_open_status == true) {
        if (nxpncihal_ctrl.p_rx_data != NULL) {
          NXPLOG_NCIHAL_D(
              "Send the Core Reset NTF to upper layer, which will trigger the "
              "recovery\n");
          // Send the Core Reset NTF to upper layer, which will trigger the
          // recovery.
#if (NXP_EXTNS == TRUE)
          abort();
#endif
          nxpncihal_ctrl.rx_data_len = sizeof(reset_ntf);
          memcpy(nxpncihal_ctrl.p_rx_data, reset_ntf, sizeof(reset_ntf));
          (*nxpncihal_ctrl.p_nfc_stack_data_cback)(nxpncihal_ctrl.rx_data_len,
                                                   nxpncihal_ctrl.p_rx_data);
        } else {
          (*nxpncihal_ctrl.p_nfc_stack_data_cback)(0x00, NULL);
        }
        write_unlocked_status = NFCSTATUS_FAILED;
      }
    }
  } else {
    write_unlocked_status = NFCSTATUS_SUCCESS;
  }

clean_and_return:
  if (write_unlocked_status == NFCSTATUS_FAILED) {
    sem_getvalue(&(nxpncihal_ctrl.syncSpiNfc), &sem_val);
    if (((nxpncihal_ctrl.p_cmd_data[0] & NCI_MT_MASK) == NCI_MT_CMD) &&
        sem_val == 0) {
      sem_post(&(nxpncihal_ctrl.syncSpiNfc));
      NXPLOG_NCIHAL_D("HAL write  failed CMD window check releasing \n");
    }
  }
  phNxpNciHal_cleanup_cb_data(&cb_data);
  return data_len;
}

/******************************************************************************
 * Function         phNxpNciHal_write_complete
 *
 * Description      This function handles write callback.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_write_complete(void *pContext,
                                       phTmlNfc_TransactInfo_t *pInfo) {
  phNxpNciHal_Sem_t *p_cb_data = (phNxpNciHal_Sem_t *)pContext;
  if (pInfo->wStatus == NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("write successful status = 0x%x", pInfo->wStatus);
  } else {
    NXPLOG_NCIHAL_D("write error status = 0x%x", pInfo->wStatus);
  }

  p_cb_data->status = pInfo->wStatus;

  SEM_POST(p_cb_data);

  return;
}

/******************************************************************************
 * Function         phNxpNciHal_read_complete
 *
 * Description      This function is called whenever there is an NCI packet
 *                  received from NFCC. It could be RSP or NTF packet. This
 *                  function provide the received NCI packet to libnfc-nci
 *                  using data callback of libnfc-nci.
 *                  There is a pending read called from each
 *                  phNxpNciHal_read_complete so each a packet received from
 *                  NFCC can be provide to libnfc-nci.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_read_complete(void *pContext,
                                      phTmlNfc_TransactInfo_t *pInfo) {
  NFCSTATUS status = NFCSTATUS_FAILED;
  int sem_val;
  UNUSED_PROP(pContext);
  if (nxpncihal_ctrl.read_retry_cnt == 1) {
    nxpncihal_ctrl.read_retry_cnt = 0;
  }
  if (pInfo->wStatus == NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("read successful status = 0x%x", pInfo->wStatus);

    /*Check the Omapi command response and store in dedicated buffer to solve
     * sync issue*/
    if (nfcFL.chipType <= sn100u && pInfo->pBuff[0] == 0x4F &&
        pInfo->pBuff[1] == 0x01 && pInfo->pBuff[2] == 0x01) {
      nxpncihal_ctrl.p_rx_ese_data = pInfo->pBuff;
      nxpncihal_ctrl.rx_ese_data_len = pInfo->wLength;
      SEM_POST(&(nxpncihal_ctrl.ext_cb_data));
    } else {
      nxpncihal_ctrl.p_rx_data = pInfo->pBuff;
      nxpncihal_ctrl.rx_data_len = pInfo->wLength;
      status = phNxpNciHal_process_ext_rsp(nxpncihal_ctrl.p_rx_data,
                                           &nxpncihal_ctrl.rx_data_len);
      if (nxpncihal_ctrl.hal_ext_enabled && phTmlNfc_IsFwDnldModeEnabled()) {
        SEM_POST(&(nxpncihal_ctrl.ext_cb_data));
      }
    }
    phNxpNciHal_print_res_status(pInfo->pBuff, &pInfo->wLength);

    /* Check if response should go to hal module only */
    if (nxpncihal_ctrl.hal_ext_enabled == TRUE &&
        (nxpncihal_ctrl.p_rx_data[0x00] & NCI_MT_MASK) == NCI_MT_RSP) {
      if (status == NFCSTATUS_FAILED) {
        NXPLOG_NCIHAL_D("enter into NFCC init recovery");
        nxpncihal_ctrl.ext_cb_data.status = status;
      }
      /* Unlock semaphore only for responses*/
      if ((nxpncihal_ctrl.p_rx_data[0x00] & NCI_MT_MASK) == NCI_MT_RSP ||
          ((nfcFL.chipType < sn100u) && (icode_detected == true) &&
           (icode_send_eof == 3))) {
        /* Unlock semaphore */
        SEM_POST(&(nxpncihal_ctrl.ext_cb_data));
      }
    } // Notification Checking
    else if ((nxpncihal_ctrl.hal_ext_enabled == TRUE) &&
             ((nxpncihal_ctrl.p_rx_data[0x00] & NCI_MT_MASK) == NCI_MT_NTF) &&
#if (NXP_EXTNS == TRUE)
             ((nxpncihal_ctrl.p_cmd_data[0x00] & NCI_GID_MASK) ==
              (nxpncihal_ctrl.p_rx_data[0x00] & NCI_GID_MASK)) &&
             ((nxpncihal_ctrl.p_cmd_data[0x01] & NCI_OID_MASK) ==
              (nxpncihal_ctrl.p_rx_data[0x01] & NCI_OID_MASK)) &&
#endif
             (nxpncihal_ctrl.nci_info.wait_for_ntf == TRUE)) {
      /* Unlock semaphore waiting for only  ntf*/
      nxpncihal_ctrl.nci_info.wait_for_ntf = FALSE;
      SEM_POST(&(nxpncihal_ctrl.ext_cb_data));
    } else if (bDisableLegacyMfcExtns && !sendRspToUpperLayer &&
               (nxpncihal_ctrl.p_rx_data[0x00] == 0x00)) {
      sendRspToUpperLayer = true;
      NFCSTATUS mfcRspStatus = NxpMfcReaderInstance.CheckMfcResponse(
          nxpncihal_ctrl.p_rx_data, nxpncihal_ctrl.rx_data_len);
      NXPLOG_NCIHAL_D("Mfc Response Status = 0x%x", mfcRspStatus);
      SEM_POST(&(nxpncihal_ctrl.ext_cb_data));
    }
    /* Read successful send the event to higher layer */
    else if ((nxpncihal_ctrl.p_nfc_stack_data_cback != NULL) &&
             (status == NFCSTATUS_SUCCESS)) {
      NxpMfcReaderInstance.MfcNotifyOnAckReceived(nxpncihal_ctrl.p_rx_data);
      (*nxpncihal_ctrl.p_nfc_stack_data_cback)(nxpncihal_ctrl.rx_data_len,
                                               nxpncihal_ctrl.p_rx_data);
    }
    /* Unblock next Write Command Window */
    sem_getvalue(&(nxpncihal_ctrl.syncSpiNfc), &sem_val);
    if (((pInfo->pBuff[0] & NCI_MT_MASK) == NCI_MT_RSP) && sem_val == 0) {
      sem_post(&(nxpncihal_ctrl.syncSpiNfc));
    }
  } else {
    NXPLOG_NCIHAL_E("read error status = 0x%x", pInfo->wStatus);
  }

  if (nxpncihal_ctrl.halStatus == HAL_STATUS_CLOSE &&
#if (NXP_EXTNS == TRUE)
      (nxpncihal_ctrl.p_cmd_data[0x00] & NCI_GID_MASK) ==
          (nxpncihal_ctrl.p_rx_data[0x00] & NCI_GID_MASK) &&
      (nxpncihal_ctrl.p_cmd_data[0x01] & NCI_OID_MASK) ==
          (nxpncihal_ctrl.p_rx_data[0x01] & NCI_OID_MASK) &&
#endif
      nxpncihal_ctrl.nci_info.wait_for_ntf == FALSE) {
    NXPLOG_NCIHAL_D(" Ignoring read , HAL close triggered");
    return;
  }
  /* Read again because read must be pending always except FWDNLD.*/
  if (TRUE != nxpncihal_ctrl.fwdnld_mode_reqd) {
    status = phTmlNfc_Read(
        nxpncihal_ctrl.p_rsp_data, NCI_MAX_DATA_LEN,
        (pphTmlNfc_TransactCompletionCb_t)&phNxpNciHal_read_complete, NULL);
    if (status != NFCSTATUS_PENDING) {
      NXPLOG_NCIHAL_E("read status error status = %x", status);
      /* TODO: Not sure how to handle this ? */
    }
  }
  return;
}

/******************************************************************************
 * Function         phNxpNciHal_enableTmlRead
 *
 * Description      Invokes TmlNfc Read to make sure always read thread is
 *                  pending
 *
 * Returns          Returns read status
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_enableTmlRead() {
  /* Read again because read must be pending always.*/
  NFCSTATUS status = phTmlNfc_Read(
      nxpncihal_ctrl.p_rsp_data, NCI_MAX_DATA_LEN,
      (pphTmlNfc_TransactCompletionCb_t)&phNxpNciHal_read_complete, NULL);
  if (status != NFCSTATUS_PENDING) {
    NXPLOG_NCIHAL_E("read status error status = %x", status);
  }
  return status;
}
/******************************************************************************
 * Function         phNxpNciHal_core_initialized
 *
 * Description      This function is called by libnfc-nci after successful open
 *                  of NFCC. All proprietary setting for PN72xx are done here.
 *                  After completion of proprietary settings notification is
 *                  provided to libnfc-nci through callback function.
 *
 * Returns          Always returns NFCSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpNciHal_core_initialized(uint16_t core_init_rsp_params_len,
                                 uint8_t *p_core_init_rsp_params) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
if (nfcFL.chipType != pn7160) {
  core_init_rsp_params_len = 10;
  p_core_init_rsp_params = NULL;
  uint8_t *buffer = NULL;
  long bufflen = 260;
  uint8_t isfound = 0;
  long retlen = 0;
  unsigned long num = 0;
  uint8_t setConfigAlways = false;

  buffer = (uint8_t *)malloc(bufflen * sizeof(uint8_t));
  if (NULL == buffer) {
    return NFCSTATUS_FAILED;
  }

  setConfigAlways = false;
  isfound = GetNxpNumValue(NAME_NXP_SET_CONFIG_ALWAYS, &num, sizeof(num));
  if (isfound > 0) {
    setConfigAlways = num;
  }

  fw_dwnld_flag |= (bool)fw_download_success;

  if (isNxpConfigModified() || (fw_dwnld_flag == true)) {
    fw_download_success = 0;
  }

  if (GetNxpNumValue(NAME_NXP_ENABLE_DISABLE_STANBY, &num, sizeof(num))) {
    if (num == 0 || num == 1) {
      uint8_t coreStandBy[] = {0x2F, 0x00, 0x01, 0x00};
      coreStandBy[3] = num;
      status = phNxpNciHal_send_ext_cmd(sizeof(coreStandBy), coreStandBy);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("Failed to enable/disable NFCC Standby");
      }
    }
  }

  // Update eeprom value
  status = phNxpNciHal_get_mw_eeprom();
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("NXP GET MW EEPROM failed");
  } else {
    for (int i = 0; i < nxpncihal_ctrl.p_rx_data[7]; i++) {
      if (nxpncihal_ctrl.p_rx_data[8 + i] != 0x00) {
        status = phNxpNciHal_set_mw_eeprom();
        if (status != NFCSTATUS_SUCCESS) {
          NXPLOG_NCIHAL_E("NXP Update MW EEPROM Proprietary Ext failed");
        }
        break;
      }
    }
  }

  if (isNxpEepromConfigModified() || (isFwDnldTriggered == true)) {
    retlen = 0;
    NXPLOG_NCIHAL_D("Performing NAME_NXP_CORE_CONF_EXTN Settings");
    isfound = GetNxpByteArrayValue(NAME_NXP_CORE_CONF_EXTN, (char *)buffer,
                                   bufflen, &retlen);
    if (isfound > 0 && retlen > 0) {
      /* NXP ACT Proprietary Ext */
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("NXP Core configuration failed");
      }
    }
  }

  if ((true == fw_dwnld_flag) || (true == setConfigAlways) ||
      isNxpConfigModified() || (wRfUpdateReq == true)) {
    retlen = 0;
    NXPLOG_NCIHAL_D("Performing NAME_NXP_CORE_CONF Settings");
    isfound = GetNxpByteArrayValue(NAME_NXP_CORE_CONF, (char *)buffer, bufflen,
                                   &retlen);
    if (isfound > 0 && retlen > 0) {
      /* NXP ACT Proprietary Ext */
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("Core Set Config failed");
      }
    }
  }

  if ((isNxpEepromConfigModified() || (isFwDnldTriggered == true)) &&
      (phNxpNciHal_flushSramToFlash() != NFCSTATUS_SUCCESS)) {
    NXPLOG_NCIHAL_E("phNxpNciHal_flushSramToFlash failed");
  }

  if (buffer) {
    free(buffer);
    buffer = NULL;
  }

  // initialize recovery FW variables
  gRecFWDwnld = 0;
  gRecFwRetryCount = 0;

  phNxpNciHal_core_initialized_complete(status);

  if (nfcFL.chipType >= pn7220) {
    status = phTmlNfc_IoCtl(phTmlNfc_e_RedLedOn);
    if (NFCSTATUS_SUCCESS == status) {
      NXPLOG_NCIHAL_D("phTmlNfc_e_RedLedOn - SUCCESS\n");
    } else {
      NXPLOG_NCIHAL_D("phTmlNfc_e_RedLedOn - FAILED\n");
    }
  }
  if (isNxpConfigModified()) {
    updateNxpConfigTimestamp();
  }
  if (isNxpEepromConfigModified()) {
    updateNxpEepromConfigTimestamp();
  }

  if (isNxpRfExtConfigModified()) {
    updateNxpRfExtConfigTimestamp();
  }
} else {
      status = phNxpNciHal_core_initialized_pn7160(core_init_rsp_params_len,
      p_core_init_rsp_params);
    }
  /* Reset fw dnld trigger flag */
  isFwDnldTriggered = false;

  return NFCSTATUS_SUCCESS;
}
#if (NXP_EXTNS == TRUE)
/******************************************************************************
 * Function         phNxpNciHal_core_initialized_pn7160
 *
 * Description      This function is called by libnfc-nci after successful open
 *                  of NFCC. All proprietary setting for PN54X are done here.
 *                  After completion of proprietary settings notification is
 *                  provided to libnfc-nci through callback function.
 *
 * Returns          Always returns NFCSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpNciHal_core_initialized_pn7160(uint16_t core_init_rsp_params_len,
                                 uint8_t* p_core_init_rsp_params) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  static uint8_t p2p_listen_mode_routing_cmd[] = {0x21, 0x01, 0x07, 0x00, 0x01,
                                                  0x01, 0x03, 0x00, 0x01, 0x05};

  uint8_t swp_full_pwr_mode_on_cmd[] = {0x20, 0x02, 0x05, 0x01,
                                        0xA0, 0xF1, 0x01, 0x01};

  static uint8_t cmd_ven_pulld_enable_nci[] = {0x20, 0x02, 0x05, 0x01,
                                               0xA0, 0x07, 0x01, 0x03};

  static uint8_t swp_switch_timeout_cmd[] = {0x20, 0x02, 0x06, 0x01, 0xA0,
                                             0xF3, 0x02, 0x00, 0x00};
  config_success = true;
  uint8_t* buffer = NULL;
  long bufflen = 260;
  long retlen = 0;
  int isfound;
#if (NFC_NXP_HFO_SETTINGS == TRUE)
  /* Temp fix to re-apply the proper clock setting */
  int temp_fix = 1;
#endif
  unsigned long num = 0;
  // initialize dummy FW recovery variables
  gRecFwRetryCount = 0;
  gRecFWDwnld = 0;
  // recovery --start
  /*NCI_INIT_CMD*/
  static uint8_t cmd_init_nci[] = {0x20, 0x01, 0x00};
  /*NCI_RESET_CMD*/
  static uint8_t cmd_reset_nci[] = {0x20, 0x00, 0x01,
                                    0x00};  // keep configuration
  static uint8_t cmd_init_nci2_0[] = {0x20, 0x01, 0x02, 0x00, 0x00};
  /* reset config cache */
  static uint8_t retry_core_init_cnt;

  if (nxpncihal_ctrl.halStatus != HAL_STATUS_OPEN) {
    return NFCSTATUS_FAILED;
  }

  if (core_init_rsp_params_len >= 1 && (*p_core_init_rsp_params > 0) &&
      (*p_core_init_rsp_params < 4))  // initializing for recovery.
  {
  retry_core_init:
    config_access = false;
    if (buffer != NULL) {
      free(buffer);
      buffer = NULL;
    }
    if (retry_core_init_cnt > 3) {
      return NFCSTATUS_FAILED;
    }

    status = phTmlNfc_IoCtl(phTmlNfc_e_ResetDevice);
    if (NFCSTATUS_SUCCESS == status) {
      NXPLOG_NCIHAL_D("PN54X Reset - SUCCESS\n");
    } else {
      NXPLOG_NCIHAL_D("PN54X Reset - FAILED\n");
    }

    status = phNxpNciHal_send_ext_cmd(sizeof(cmd_reset_nci), cmd_reset_nci);
    if ((status != NFCSTATUS_SUCCESS) &&
        (nxpncihal_ctrl.retry_cnt >= MAX_RETRY_COUNT)) {
      NXPLOG_NCIHAL_E("Force FW Download, NFCC not coming out from Standby");
      retry_core_init_cnt++;
      goto retry_core_init;
    } else if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("NCI_CORE_RESET: Failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }

    if (core_init_rsp_params_len >= 1 && *p_core_init_rsp_params == 2) {
      NXPLOG_NCIHAL_E(" Last command is CORE_RESET!!");
 //     goto invoke_callback;
    }
    if (nxpncihal_ctrl.nci_info.nci_version == NCI_VERSION_2_0) {
      status =
          phNxpNciHal_send_ext_cmd(sizeof(cmd_init_nci2_0), cmd_init_nci2_0);
    } else {
      status = phNxpNciHal_send_ext_cmd(sizeof(cmd_init_nci), cmd_init_nci);
    }
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("NCI_CORE_INIT : Failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }

    if (core_init_rsp_params_len >= 1 && *p_core_init_rsp_params == 3) {
      NXPLOG_NCIHAL_E(" Last command is CORE_INIT!!");
//      goto invoke_callback;
    }
  }
  // recovery --end

  buffer = (uint8_t*)malloc(bufflen * sizeof(uint8_t));
  if (NULL == buffer) {
    return NFCSTATUS_FAILED;
  }
  config_access = true;
  retlen = 0;
  isfound = GetNxpByteArrayValue(NAME_NXP_ACT_PROP_EXTN, (char*)buffer, bufflen,
                                 &retlen);
  if ((isfound == 1) && (retlen > 0)) {
    /* NXP ACT Proprietary Ext */
    status = phNxpNciHal_send_ext_cmd(retlen, buffer);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("NXP ACT Proprietary Ext failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }
  }
  status = phNxpNciHal_send_ext_cmd(sizeof(cmd_ven_pulld_enable_nci),
                                    cmd_ven_pulld_enable_nci);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("cmd_ven_pulld_enable_nci: Failed");
    retry_core_init_cnt++;
    goto retry_core_init;
  }

  // Check if firmware download success
  status = phNxpNciHal_get_mw_eeprom();
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("NXP GET MW EEPROM AREA Proprietary Ext failed");
    retry_core_init_cnt++;
    goto retry_core_init;
  }
  retlen = 0;
  config_access = true;
  isfound = GetNxpByteArrayValue(NAME_NXP_NFC_PROFILE_EXTN, (char*)buffer,
                                 bufflen, &retlen);
  if (retlen > 0) {
    /* NXP ACT Proprietary Ext */
    status = phNxpNciHal_send_ext_cmd(retlen, buffer);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("NXP ACT Proprietary Ext failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }
  }

  if (!GetNxpNumValue(NAME_NXP_T4T_NFCEE_ENABLE, (void*)&retlen,
                      sizeof(retlen))) {
    retlen = 0x00;
    NXPLOG_NCIHAL_D(
        "T4T_NFCEE_ENABLE not found. Taking default value : 0x%02lx", retlen);
  }

  // Configure t4t ndef emulation
  status = phNxpNciHal_config_t4t_ndef((uint8_t)retlen);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("NXP Update MW EEPROM Proprietary Ext failed");
  }

  if (isNxpConfigModified() || (fw_download_success == 1)  || (anti_tearing_recovery_success == 1)) {
    NXPLOG_NCIHAL_D("Applying Settings: isNxpConfigModified()=%d, fw_download_success=%d, anti_tearing_recovery_success=%d",
      isNxpConfigModified(), fw_download_success, anti_tearing_recovery_success);

    retlen = 0;
    fw_download_success = 0;

    NXPLOG_NCIHAL_D("Performing TVDD Settings");
    isfound = GetNxpNumValue(NAME_NXP_EXT_TVDD_CFG, &num, sizeof(num));
    if (isfound > 0) {
      if (num == 1) {
        isfound = GetNxpByteArrayValue(NAME_NXP_EXT_TVDD_CFG_1, (char*)buffer,
                                       bufflen, &retlen);
        if ((isfound == 1) && (retlen > 0)) {
          status = phNxpNciHal_send_ext_cmd(retlen, buffer);
          if (status != NFCSTATUS_SUCCESS) {
            NXPLOG_NCIHAL_E("EXT TVDD CFG 1 Settings failed");
            retry_core_init_cnt++;
            goto retry_core_init;
          }
        }
      } else if (num == 2) {
        isfound = GetNxpByteArrayValue(NAME_NXP_EXT_TVDD_CFG_2, (char*)buffer,
                                       bufflen, &retlen);
        if ((isfound == 1) && (retlen > 0)) {
          status = phNxpNciHal_send_ext_cmd(retlen, buffer);
          if (status != NFCSTATUS_SUCCESS) {
            NXPLOG_NCIHAL_E("EXT TVDD CFG 2 Settings failed");
            retry_core_init_cnt++;
            goto retry_core_init;
          }
        }
      } else if (num == 3) {
        isfound = GetNxpByteArrayValue(NAME_NXP_EXT_TVDD_CFG_3, (char*)buffer,
                                       bufflen, &retlen);
        if ((isfound == 1) && (retlen > 0)) {
          status = phNxpNciHal_send_ext_cmd(retlen, buffer);
          if (status != NFCSTATUS_SUCCESS) {
            NXPLOG_NCIHAL_E("EXT TVDD CFG 3 Settings failed");
            retry_core_init_cnt++;
            goto retry_core_init;
          }
        }
      } else {
        NXPLOG_NCIHAL_E("Wrong Configuration Value %ld", num);
      }
    }
    retlen = 0;
    config_access = false;
    NXPLOG_NCIHAL_D("Performing RF Settings BLK 1");
    isfound = GetNxpByteArrayValue(NAME_NXP_RF_CONF_BLK_1, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status == NFCSTATUS_SUCCESS) {
        status = phNxpNciHal_CheckRFCmdRespStatus();
        /*STATUS INVALID PARAM 0x09*/
        if (status == 0x09) {
          phNxpNciHalRFConfigCmdRecSequence();
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("RF Settings BLK 1 failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
    retlen = 0;

    NXPLOG_NCIHAL_D("Performing RF Settings BLK 2");
    isfound = GetNxpByteArrayValue(NAME_NXP_RF_CONF_BLK_2, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status == NFCSTATUS_SUCCESS) {
        status = phNxpNciHal_CheckRFCmdRespStatus();
        /*STATUS INVALID PARAM 0x09*/
        if (status == 0x09) {
          phNxpNciHalRFConfigCmdRecSequence();
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("RF Settings BLK 2 failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
    retlen = 0;

    NXPLOG_NCIHAL_D("Performing RF Settings BLK 3");
    isfound = GetNxpByteArrayValue(NAME_NXP_RF_CONF_BLK_3, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status == NFCSTATUS_SUCCESS) {
        status = phNxpNciHal_CheckRFCmdRespStatus();
        /*STATUS INVALID PARAM 0x09*/
        if (status == 0x09) {
          phNxpNciHalRFConfigCmdRecSequence();
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("RF Settings BLK 3 failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
    retlen = 0;

    NXPLOG_NCIHAL_D("Performing RF Settings BLK 4");
    isfound = GetNxpByteArrayValue(NAME_NXP_RF_CONF_BLK_4, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status == NFCSTATUS_SUCCESS) {
        status = phNxpNciHal_CheckRFCmdRespStatus();
        /*STATUS INVALID PARAM 0x09*/
        if (status == 0x09) {
          phNxpNciHalRFConfigCmdRecSequence();
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("RF Settings BLK 4 failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
    retlen = 0;

    NXPLOG_NCIHAL_D("Performing RF Settings BLK 5");
    isfound = GetNxpByteArrayValue(NAME_NXP_RF_CONF_BLK_5, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status == NFCSTATUS_SUCCESS) {
        status = phNxpNciHal_CheckRFCmdRespStatus();
        /*STATUS INVALID PARAM 0x09*/
        if (status == 0x09) {
          phNxpNciHalRFConfigCmdRecSequence();
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("RF Settings BLK 5 failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
    retlen = 0;

    NXPLOG_NCIHAL_D("Performing RF Settings BLK 6");
    isfound = GetNxpByteArrayValue(NAME_NXP_RF_CONF_BLK_6, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status == NFCSTATUS_SUCCESS) {
        status = phNxpNciHal_CheckRFCmdRespStatus();
        /*STATUS INVALID PARAM 0x09*/
        if (status == 0x09) {
          phNxpNciHalRFConfigCmdRecSequence();
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("RF Settings BLK 6 failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
    retlen = 0;
    config_access = true;
    NXPLOG_NCIHAL_D("Performing NAME_NXP_CORE_CONF_EXTN Settings");
    isfound = GetNxpByteArrayValue(NAME_NXP_CORE_CONF_EXTN, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      /* NXP ACT Proprietary Ext */
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("NXP Core configuration failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }

    retlen = 0;
    config_access = false;
    isfound = GetNxpByteArrayValue(NAME_NXP_CORE_RF_FIELD, (char*)buffer,
                                   bufflen, &retlen);
    if ((isfound == 1) && (retlen > 0)) {
      /* NXP ACT Proprietary Ext */
      status = phNxpNciHal_send_ext_cmd(retlen, buffer);
      if (status == NFCSTATUS_SUCCESS) {
        status = phNxpNciHal_CheckRFCmdRespStatus();
        /*STATUS INVALID PARAM 0x09*/
        if (status == 0x09) {
          phNxpNciHalRFConfigCmdRecSequence();
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("Setting NXP_CORE_RF_FIELD status failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }

    config_access = true;
    retlen = 0;
    /* NXP SWP switch timeout Setting*/
    if (GetNxpNumValue(NAME_NXP_SWP_SWITCH_TIMEOUT, (void*)&retlen,
                       sizeof(retlen))) {
      // Check the permissible range [0 - 60]
      if (0 <= retlen && retlen <= 60) {
        if (0 < retlen) {
          unsigned int timeout = (uint32_t)retlen * 1000;
          unsigned int timeoutHx = 0x0000;

          char tmpbuffer[10] = {0};
          snprintf((char*)tmpbuffer, 10, "%04x", timeout);
          sscanf((char*)tmpbuffer, "%x", &timeoutHx);

          swp_switch_timeout_cmd[7] = (timeoutHx & 0xFF);
          swp_switch_timeout_cmd[8] = ((timeoutHx & 0xFF00) >> 8);
        }

        status = phNxpNciHal_send_ext_cmd(sizeof(swp_switch_timeout_cmd),
                                          swp_switch_timeout_cmd);
        if (status != NFCSTATUS_SUCCESS) {
          NXPLOG_NCIHAL_E("SWP switch timeout Setting Failed");
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      } else {
        NXPLOG_NCIHAL_E("SWP switch timeout Setting Failed - out of range!");
      }
    }

    status = phNxpNciHal_china_tianjin_rf_setting();
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("phNxpNciHal_china_tianjin_rf_setting failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }

    // Update eeprom value
    status = phNxpNciHal_set_mw_eeprom();
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("NXP Update MW EEPROM Proprietary Ext failed");
    }

    anti_tearing_recovery_success = 0;
  }

  retlen = 0;
  NXPLOG_NCIHAL_D("Performing NAME_NXP_CORE_CONF Settings");
  isfound =
      GetNxpByteArrayValue(NAME_NXP_CORE_CONF, (char*)buffer, bufflen, &retlen);
  NXPLOG_NCIHAL_D("NAME_NXP_CORE_CONF Settings Found - %d Len: %ld", isfound, retlen);
  if ((isfound == 1) && (retlen > 0)) {
    /* NXP ACT Proprietary Ext */
    status = phNxpNciHal_send_ext_cmd(retlen, buffer);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Core Set Config failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }
  }

  if (phNxpNciHal_nfccClockCfgApply() != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("phNxpNciHal_nfccClockCfgApply failed");
    retry_core_init_cnt++;
    goto retry_core_init;
  }

  config_access = false;
  // if recovery mode and length of last command is 0 then only reset the P2P
  // listen mode routing.
  if (core_init_rsp_params_len >= 1 && core_init_rsp_params_len >= 36 && (*p_core_init_rsp_params > 0) &&
      (*p_core_init_rsp_params < 4) && p_core_init_rsp_params[35] == 0) {
    /* P2P listen mode routing */
    status = phNxpNciHal_send_ext_cmd(sizeof(p2p_listen_mode_routing_cmd),
                                      p2p_listen_mode_routing_cmd);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("P2P listen mode routing failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }
  }

  retlen = 0;

  /* SWP FULL PWR MODE SETTING ON */
  if (GetNxpNumValue(NAME_NXP_SWP_FULL_PWR_ON, (void*)&retlen,
                     sizeof(retlen))) {
    if (1 == retlen) {
      status = phNxpNciHal_send_ext_cmd(sizeof(swp_full_pwr_mode_on_cmd),
                                        swp_full_pwr_mode_on_cmd);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("SWP FULL PWR MODE SETTING ON CMD FAILED");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    } else {
      swp_full_pwr_mode_on_cmd[7] = 0x00;
      status = phNxpNciHal_send_ext_cmd(sizeof(swp_full_pwr_mode_on_cmd),
                                        swp_full_pwr_mode_on_cmd);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("SWP FULL PWR MODE SETTING OFF CMD FAILED");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
  }

  if (core_init_rsp_params_len >= 1 && (*p_core_init_rsp_params > 0) &&
      (*p_core_init_rsp_params < 4)) {
    static phLibNfc_Message_t msg;
    uint16_t tmp_len = 0;
    uint8_t uicc_set_mode[] = {0x22, 0x01, 0x02, 0x02, 0x01};
    uint8_t set_screen_state[] = {0x2F, 0x15, 01, 00};  // SCREEN ON
    uint8_t nfcc_core_conn_create[] = {0x20, 0x04, 0x06, 0x03, 0x01,
                                       0x01, 0x02, 0x01, 0x01};
    uint8_t nfcc_mode_set_on[] = {0x22, 0x01, 0x02, 0x01, 0x01};

    NXPLOG_NCIHAL_W(
        "Sending DH and NFCC core connection command as raw packet!!");
    status = phNxpNciHal_send_ext_cmd(sizeof(nfcc_core_conn_create),
                                      nfcc_core_conn_create);

    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E(
          "Sending DH and NFCC core connection command as raw packet!! Failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }

    NXPLOG_NCIHAL_W("Sending DH and NFCC mode set as raw packet!!");
    status =
        phNxpNciHal_send_ext_cmd(sizeof(nfcc_mode_set_on), nfcc_mode_set_on);

    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Sending DH and NFCC mode set as raw packet!! Failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }

    NXPLOG_NCIHAL_W("Sending UICC Select Command as raw packet!!");
    status = phNxpNciHal_send_ext_cmd(sizeof(uicc_set_mode), uicc_set_mode);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Sending UICC Select Command as raw packet!! Failed");
      retry_core_init_cnt++;
      goto retry_core_init;
    }

    if (core_init_rsp_params_len >= 4 &&
        *(p_core_init_rsp_params + 1) == 1)  // RF state is Discovery!!
    {
      NXPLOG_NCIHAL_W("Sending Set Screen ON State Command as raw packet!!");
      status =
          phNxpNciHal_send_ext_cmd(sizeof(set_screen_state), set_screen_state);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E(
            "Sending Set Screen ON State Command as raw packet!! Failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }

      if (p_core_init_rsp_params[2] > (core_init_rsp_params_len - 3)) {
        if (buffer) {
          free(buffer);
          buffer = NULL;
        }
        return NFCSTATUS_FAILED;
      }
      NXPLOG_NCIHAL_W("Sending discovery as raw packet!!");
      status = phNxpNciHal_send_ext_cmd(p_core_init_rsp_params[2],
                                        (uint8_t*)&p_core_init_rsp_params[3]);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("Sending discovery as raw packet Failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }

    } else {
      NXPLOG_NCIHAL_W("Sending Set Screen OFF State Command as raw packet!!");
      set_screen_state[3] = 0x01;  // Screen OFF
      status =
          phNxpNciHal_send_ext_cmd(sizeof(set_screen_state), set_screen_state);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E(
            "Sending Set Screen OFF State Command as raw packet!! Failed");
        retry_core_init_cnt++;
        goto retry_core_init;
      }
    }
    NXPLOG_NCIHAL_W("Sending last command for Recovery ");

    if (core_init_rsp_params_len >= 40 &&
        p_core_init_rsp_params[35] > 0) {  // if length of last command is 0
                                           // then it doesn't need to send last
                                           // command.
      if (!(((p_core_init_rsp_params[36] == 0x21) &&
             (p_core_init_rsp_params[37] == 0x03)) &&
            (*(p_core_init_rsp_params + 1) == 1)) &&
          !((p_core_init_rsp_params[36] == 0x21) &&
            (p_core_init_rsp_params[37] == 0x06) &&
            (p_core_init_rsp_params[39] == 0x00) &&
            (*(p_core_init_rsp_params + 1) == 0x00)))
      // if last command is discovery and RF status is also discovery state,
      // then it doesn't need to execute or similarly
      // if the last command is deactivate to idle and RF status is also idle ,
      // no need to execute the command .
      {
        tmp_len = p_core_init_rsp_params[35];

        /* Check for NXP ext before sending write */
        status = phNxpNciHal_write_ext(
            &tmp_len, (uint8_t*)&p_core_init_rsp_params[36],
            &nxpncihal_ctrl.rsp_len, nxpncihal_ctrl.p_rsp_data);
        if (status != NFCSTATUS_SUCCESS) {
          if (buffer) {
            free(buffer);
            buffer = NULL;
          }
          /* Do not send packet to PN54X, send response directly */
          msg.eMsgType = NCI_HAL_RX_MSG;
          msg.pMsgData = NULL;
          msg.Size = 0;

          phTmlNfc_DeferredCall(gpphTmlNfc_Context->dwCallbackThreadId,
                                (phLibNfc_Message_t*)&msg);
          return NFCSTATUS_SUCCESS;
        }

        p_core_init_rsp_params[35] = (uint8_t)tmp_len;
        if (p_core_init_rsp_params[35] > (core_init_rsp_params_len - 36)) {
          if (buffer) {
            free(buffer);
            buffer = NULL;
          }
          return NFCSTATUS_FAILED;
        }
        status = phNxpNciHal_send_ext_cmd(
            p_core_init_rsp_params[35], (uint8_t*)&p_core_init_rsp_params[36]);
        if (status != NFCSTATUS_SUCCESS) {
          NXPLOG_NCIHAL_E("Sending last command for Recovery Failed");
          retry_core_init_cnt++;
          goto retry_core_init;
        }
      }
    }
  }

  retry_core_init_cnt = 0;

  if (buffer) {
    free(buffer);
    buffer = NULL;
  }
  // initialize dummy FW recovery variables
  gRecFWDwnld = 0;
  gRecFwRetryCount = 0;
  /*TO DO fix as part of this artf1170009 */
 /* if (core_init_rsp_params_len >= 1 &&
      !((*p_core_init_rsp_params > 0) && (*p_core_init_rsp_params < 4)))
    phNxpNciHal_core_initialized_complete(status);
  else {
  invoke_callback:
    config_access = false;
    if (nxpncihal_ctrl.p_nfc_stack_data_cback != NULL) {
      if (core_init_rsp_params_len) *p_core_init_rsp_params = 0;
      NXPLOG_NCIHAL_W("Invoking data callback!!");
      (*nxpncihal_ctrl.p_nfc_stack_data_cback)(nxpncihal_ctrl.rx_data_len,
                                               nxpncihal_ctrl.p_rx_data);
    }
  }*/

  if (config_success == false) return NFCSTATUS_FAILED;
#ifdef PN547C2_CLOCK_SETTING
  if (isNxpConfigModified()) {
    updateNxpConfigTimestamp();
  }
#endif
  phNxpNciHal_core_initialized_complete(status);
  return NFCSTATUS_SUCCESS;
}
#endif
/******************************************************************************
 * Function         phNxpNciHal_CheckRFCmdRespStatus
 *
 * Description      This function is called to check the resp status of
 *                  RF update commands.
 *
 * Returns          NFCSTATUS_SUCCESS           if successful,
 *                  NFCSTATUS_INVALID_PARAMETER if parameter is inavlid
 *                  NFCSTATUS_FAILED            if failed response
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_CheckRFCmdRespStatus() {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  static uint16_t INVALID_PARAM = 0x09;
  if ((nxpncihal_ctrl.rx_data_len > 0) && (nxpncihal_ctrl.p_rx_data[2] > 0)) {
    if (nxpncihal_ctrl.p_rx_data[3] == 0x09) {
      status = INVALID_PARAM;
    } else if (nxpncihal_ctrl.p_rx_data[3] != NFCSTATUS_SUCCESS) {
      status = NFCSTATUS_FAILED;
    }
  }
  return status;
}
/******************************************************************************
 * Function         phNxpNciHalRFConfigCmdRecSequence
 *
 * Description      This function is called to handle dummy FW recovery sequence
 *                  Whenever RF settings are failed to apply with invalid param
 *                  response, recovery mechanism includes dummy firmware
 *download
 *                  followed by firmware download and then config settings. The
 *dummy
 *                  firmware changes the major number of the firmware inside
 *NFCC.
 *                  Then actual firmware dowenload will be successful. This can
 *be
 *                  retried maximum three times.
 *
 * Returns          Always returns NFCSTATUS_SUCCESS
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHalRFConfigCmdRecSequence() {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  uint16_t recFWState = 1;
  gRecFWDwnld = true;
  gRecFwRetryCount++;
  if (gRecFwRetryCount > 0x03) {
    NXPLOG_NCIHAL_D("Max retry count for RF config FW recovery exceeded ");
    gRecFWDwnld = false;
    return NFCSTATUS_FAILED;
  }
  do {
    status = phTmlNfc_IoCtl(phTmlNfc_e_ResetDevice);
    phDnldNfc_InitImgInfo();
    if (NFCSTATUS_SUCCESS == phNxpNciHal_CheckValidFwVersion()) {
      fw_download_success = 0;
      status = phNxpNciHal_fw_download();
      if (status == NFCSTATUS_SUCCESS) {
        fw_download_success = 1;
      }
      status = phTmlNfc_Read(
          nxpncihal_ctrl.p_cmd_data, NCI_MAX_DATA_LEN,
          (pphTmlNfc_TransactCompletionCb_t)&phNxpNciHal_read_complete, NULL);
      if (status != NFCSTATUS_PENDING) {
        NXPLOG_NCIHAL_E("TML Read status error status = %x", status);
        phOsalNfc_Timer_Cleanup();
        phTmlNfc_Shutdown();
        status = NFCSTATUS_FAILED;
      }
      break;
    }
    gRecFWDwnld = false;
  } while (recFWState--);
  gRecFWDwnld = false;
  return status;
}
/******************************************************************************
 * Function         phNxpNciHal_core_initialized_complete
 *
 * Description      This function is called when phNxpNciHal_core_initialized
 *                  complete all proprietary command exchanges. This function
 *                  informs libnfc-nci about completion of core initialize
 *                  and result of that through callback.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_core_initialized_complete(NFCSTATUS status) {
  static phLibNfc_Message_t msg;

  if (status == NFCSTATUS_SUCCESS) {
    msg.eMsgType = NCI_HAL_POST_INIT_CPLT_MSG;
  } else {
    msg.eMsgType = NCI_HAL_ERROR_MSG;
  }
  msg.pMsgData = NULL;
  msg.Size = 0;

  phTmlNfc_DeferredCall(gpphTmlNfc_Context->dwCallbackThreadId,
                        (phLibNfc_Message_t *)&msg);
  return;
}

/******************************************************************************
 * Function         phNxpNciHal_pre_discover
 *
 * Description      This function is called by libnfc-nci to perform any
 *                  proprietary exchange before RF discovery.
 *
 * Returns          It always returns NFCSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpNciHal_pre_discover(void) {
  /* Nothing to do here for initial version */
  // This is set to return Failed as no vendor specific pre-discovery action is
  // needed in case of HalPrediscover
  return NFCSTATUS_FAILED;
}

/******************************************************************************
 * Function         phNxpNciHal_release_info
 *
 * Description      This function frees allocated memory for mGetCfg_info
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_release_info(void) {
  NXPLOG_NCIHAL_D("phNxpNciHal_release_info mGetCfg_info");
  if (mGetCfg_info != NULL) {
    free(mGetCfg_info);
    mGetCfg_info = NULL;
  }
}
/******************************************************************************
 * Function         phNxpNciHal_close
 *
 * Description      This function close the NFCC interface and free all
 *                  resources.This is called by libnfc-nci on NFC service stop.
 *
 * Returns          Always return NFCSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpNciHal_close(bool bShutdown) {
  NFCSTATUS status = NFCSTATUS_FAILED;

  uint8_t cmd_reset_nci[] = {0x20, 0x00, 0x01, 0x00};
  uint8_t dummyGetConfig[] = {0x20, 0x03, 0x03, 0x01, 0xA0, 0x0F};
  uint8_t retry = 0;

  NfcHalAutoThreadMutex a(sHalFnLock);
  if (nxpncihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
    NXPLOG_NCIHAL_D("phNxpNciHal_close is already closed, ignoring close");
    return NFCSTATUS_FAILED;
  }

  CONCURRENCY_LOCK();
  int sem_val;
  sem_getvalue(&(nxpncihal_ctrl.syncSpiNfc), &sem_val);
  if (sem_val == 0) {
    sem_post(&(nxpncihal_ctrl.syncSpiNfc));
  }

  if (write_unlocked_status == NFCSTATUS_FAILED) {
    NXPLOG_NCIHAL_D("phNxpNciHal_close i2c write failed .Clean and Return");
    goto close_and_return;
  }

close_and_return:
  /* Send a dummy get config cmd to prevent the abrupt HAL close before reading
     all pending data from i2c line  */
  status = phNxpNciHal_send_ext_cmd(sizeof(dummyGetConfig), dummyGetConfig);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("Get config failed ");
  }

  if ((nfcFL.chipType < sn220u) || (nfcFL.chipType >= pn7220) || bShutdown) {
    nxpncihal_ctrl.halStatus = HAL_STATUS_CLOSE;
  }
  do { /*This is NXP_EXTNS code for retry*/
    status = phNxpNciHal_send_ext_cmd(sizeof(cmd_reset_nci), cmd_reset_nci);

    if (status == NFCSTATUS_SUCCESS) {
      break;
    } else {
      NXPLOG_NCIHAL_E("NCI_CORE_RESET: Failed, perform retry after delay");
      usleep(1000 * 1000);
      retry++;
      if (retry > 3) {
        NXPLOG_NCIHAL_E(
            "Maximum retries performed, shall restart HAL to recover");
        abort();
      }
    }
  } while (retry < 3);

#if (NXP_EXTNS == TRUE)
  if (nfcFL.chipType >= pn7220) {
    status = phTmlNfc_IoCtl(phTmlNfc_e_RedLedOff);
    if (NFCSTATUS_SUCCESS == status) {
      NXPLOG_NCIHAL_D("phTmlNfc_e_RedLedOn - SUCCESS\n");
    } else {
      NXPLOG_NCIHAL_D("phTmlNfc_e_RedLedOn- FAILED\n");
    }
  }
#endif

  sem_destroy(&nxpncihal_ctrl.syncSpiNfc);

  if (NULL != gpphTmlNfc_Context->pDevHandle) {
    phNxpNciHal_close_complete(NFCSTATUS_SUCCESS);
    /* Abort any pending read and write */
    status = phTmlNfc_ReadAbort();
    status = phTmlNfc_WriteAbort();

    phOsalNfc_Timer_Cleanup();

    status = phTmlNfc_Shutdown();

    if (0 != pthread_join(nxpncihal_ctrl.client_thread, (void **)NULL)) {
      NXPLOG_TML_E("Fail to kill client thread!");
    }

    phTmlNfc_CleanUp();

    phDal4Nfc_msgrelease(nxpncihal_ctrl.gDrvCfg.nClientId);

    memset(&nxpncihal_ctrl, 0x00, sizeof(nxpncihal_ctrl));

    NXPLOG_NCIHAL_D("phNxpNciHal_close - phOsalNfc_DeInit completed");
  }

  CONCURRENCY_UNLOCK();

  phNxpNciHal_cleanup_monitor();
  write_unlocked_status = NFCSTATUS_SUCCESS;
  phNxpNciHal_release_info();
  /* reset config cache */
  resetNxpConfig();
  /* Return success always */
  return NFCSTATUS_SUCCESS;
}

/******************************************************************************
 * Function         phNxpNciHal_close_complete
 *
 * Description      This function inform libnfc-nci about result of
 *                  phNxpNciHal_close.
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpNciHal_close_complete(NFCSTATUS status) {
  static phLibNfc_Message_t msg;

  if (status == NFCSTATUS_SUCCESS) {
    msg.eMsgType = NCI_HAL_CLOSE_CPLT_MSG;
  } else {
    msg.eMsgType = NCI_HAL_ERROR_MSG;
  }
  msg.pMsgData = NULL;
  msg.Size = 0;
  nxpncihal_ctrl.hal_open_status = false;
  phTmlNfc_DeferredCall(gpphTmlNfc_Context->dwCallbackThreadId, &msg);

  return;
}

/******************************************************************************
 * Function         phNxpNciHal_configDiscShutdown
 *
 * Description      Enable the CE and VEN config during shutdown.
 *
 * Returns          Always return NFCSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpNciHal_configDiscShutdown(void) {
  NFCSTATUS status;
  /*NCI_RESET_CMD*/

  uint8_t cmd_disable_disc[] = {0x21, 0x06, 0x01, 0x00};

  uint8_t cmd_ce_disc_nci[] = {0x21, 0x03, 0x07, 0x03, 0x80,
                               0x01, 0x81, 0x01, 0x82, 0x01};

  /* Discover map - PROTOCOL_ISO_DEP, PROTOCOL_T3T and MIFARE Classic*/
  uint8_t cmd_disc_map[] = {0x21, 0x00, 0x0A, 0x03, 0x04, 0x03, 0x02,
                            0x03, 0x02, 0x01, 0x80, 0x01, 0x80};
  CONCURRENCY_LOCK();

  status = phNxpNciHal_send_ext_cmd(sizeof(cmd_disable_disc), cmd_disable_disc);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("CMD_DISABLE_DISCOVERY: Failed");
  }

  if (nfcFL.chipType >= sn100u) {
    status = phNxpNciHal_send_ext_cmd(sizeof(cmd_disc_map), cmd_disc_map);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Discovery Map command: Failed");
    }
    status = phNxpNciHal_ext_send_sram_config_to_flash();
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Updation of the SRAM contents failed");
    }
  }
  status = phNxpNciHal_send_ext_cmd(sizeof(cmd_ce_disc_nci), cmd_ce_disc_nci);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("CMD_CE_DISC_NCI: Failed");
  }

  CONCURRENCY_UNLOCK();

  status = phNxpNciHal_close(true);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("NCI_HAL_CLOSE: Failed");
  }

  /* Return success always */
  return NFCSTATUS_SUCCESS;
}

/******************************************************************************
 * Function         phNxpNciHal_getVendorConfig
 *
 * Description      This function can be used by HAL to inform
 *                 to update vendor configuration parametres
 *
 * Returns          void.
 *
 ******************************************************************************/

void phNxpNciHal_getVendorConfig(
    android::hardware::nfc::V1_1::NfcConfig &config) {
  unsigned long num = 0;
  std::array<uint8_t, NXP_MAX_CONFIG_STRING_LEN> buffer;
  buffer.fill(0);
  long retlen = 0;
  memset(&config, 0x00, sizeof(android::hardware::nfc::V1_1::NfcConfig));
  memset(&config_ext, 0x00, sizeof(nxp_nfc_config_ext_t));

  if ((GetNxpNumValue(NAME_NXP_AUTONOMOUS_ENABLE, &num, sizeof(num)))) {
    config_ext.autonomous_mode = (uint8_t)num;
  }
  if ((GetNxpNumValue(NAME_NXP_GUARD_TIMER_VALUE, &num, sizeof(num)))) {
    config_ext.guard_timer_value = (uint8_t)num;
  }
  if (GetNxpNumValue(NAME_NFA_POLL_BAIL_OUT_MODE, &num, sizeof(num))) {
    config.nfaPollBailOutMode = (bool)num;
  }
  if (GetNxpNumValue(NAME_ISO_DEP_MAX_TRANSCEIVE, &num, sizeof(num))) {
    config.maxIsoDepTransceiveLength = (uint32_t)num;
  }
  if (GetNxpNumValue(NAME_DEFAULT_OFFHOST_ROUTE, &num, sizeof(num))) {
    config.defaultOffHostRoute = (uint8_t)num;
  }
  if (GetNxpNumValue(NAME_DEFAULT_NFCF_ROUTE, &num, sizeof(num))) {
    config.defaultOffHostRouteFelica = (uint8_t)num;
  }
  if (GetNxpNumValue(NAME_DEFAULT_SYS_CODE_ROUTE, &num, sizeof(num))) {
    config.defaultSystemCodeRoute = (uint8_t)num;
  }
  if (GetNxpNumValue(NAME_DEFAULT_SYS_CODE_PWR_STATE, &num, sizeof(num))) {
    config.defaultSystemCodePowerState =
        phNxpNciHal_updateAutonomousPwrState((uint8_t)num);
  }
  if (GetNxpNumValue(NAME_DEFAULT_ROUTE, &num, sizeof(num))) {
    config.defaultRoute = (uint8_t)num;
  }
  if (GetNxpByteArrayValue(NAME_DEVICE_HOST_WHITE_LIST, (char *)buffer.data(),
                           buffer.size(), &retlen)) {
    config.hostWhitelist.resize(retlen);
    for (long i = 0; i < retlen; i++)
      config.hostWhitelist[i] = buffer[i];
  }
  if (GetNxpNumValue(NAME_OFF_HOST_ESE_PIPE_ID, &num, sizeof(num))) {
    config.offHostESEPipeId = (uint8_t)num;
  }
  if (GetNxpNumValue(NAME_OFF_HOST_SIM_PIPE_ID, &num, sizeof(num))) {
    config.offHostSIMPipeId = (uint8_t)num;
  }
  if ((GetNxpByteArrayValue(NAME_NFA_PROPRIETARY_CFG, (char *)buffer.data(),
                            buffer.size(), &retlen)) &&
      (retlen == 9)) {
    config.nfaProprietaryCfg.protocol18092Active = (uint8_t)buffer[0];
    config.nfaProprietaryCfg.protocolBPrime = (uint8_t)buffer[1];
    config.nfaProprietaryCfg.protocolDual = (uint8_t)buffer[2];
    config.nfaProprietaryCfg.protocol15693 = (uint8_t)buffer[3];
    config.nfaProprietaryCfg.protocolKovio = (uint8_t)buffer[4];
    config.nfaProprietaryCfg.protocolMifare = (uint8_t)buffer[5];
    config.nfaProprietaryCfg.discoveryPollKovio = (uint8_t)buffer[6];
    config.nfaProprietaryCfg.discoveryPollBPrime = (uint8_t)buffer[7];
    config.nfaProprietaryCfg.discoveryListenBPrime = (uint8_t)buffer[8];
  } else {
    memset(&config.nfaProprietaryCfg, 0xFF, sizeof(ProtocolDiscoveryConfig));
  }
  if ((GetNxpNumValue(NAME_PRESENCE_CHECK_ALGORITHM, &num, sizeof(num))) &&
      (num <= 2)) {
    config.presenceCheckAlgorithm = (PresenceCheckAlgorithm)num;
  }
}

/******************************************************************************
 * Function         phNxpNciHal_getVendorConfig_1_2
 *
 * Description      This function can be used by HAL to inform
 *                 to update vendor configuration parametres
 *
 * Returns          void.
 *
 ******************************************************************************/

void phNxpNciHal_getVendorConfig_1_2(
    android::hardware::nfc::V1_2::NfcConfig &config) {
  unsigned long num = 0;
  std::array<uint8_t, NXP_MAX_CONFIG_STRING_LEN> buffer;
  buffer.fill(0);
  long retlen = 0;
  memset(&config, 0x00, sizeof(android::hardware::nfc::V1_2::NfcConfig));
  phNxpNciHal_getVendorConfig(config.v1_1);

  if (GetNxpByteArrayValue(NAME_OFFHOST_ROUTE_UICC, (char *)buffer.data(),
                           buffer.size(), &retlen)) {
    config.offHostRouteUicc.resize(retlen);
    for (long i = 0; i < retlen; i++)
      config.offHostRouteUicc[i] = buffer[i];
  }

  if (GetNxpByteArrayValue(NAME_OFFHOST_ROUTE_ESE, (char *)buffer.data(),
                           buffer.size(), &retlen)) {
    config.offHostRouteEse.resize(retlen);
    for (long i = 0; i < retlen; i++)
      config.offHostRouteEse[i] = buffer[i];
  }

  if (GetNxpNumValue(NAME_DEFAULT_ISODEP_ROUTE, &num, sizeof(num))) {
    config.defaultIsoDepRoute = num;
  }
}

/******************************************************************************
 * Function         phNxpNciHal_notify_i2c_fragmentation
 *
 * Description      This function can be used by HAL to inform
 *                 libnfc-nci that i2c fragmentation is enabled/disabled
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpNciHal_notify_i2c_fragmentation(void) {
  if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
    /*inform libnfc-nci that i2c fragmentation is enabled/disabled */
    (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_ENABLE_I2C_FRAGMENTATION_EVT,
                                        HAL_NFC_STATUS_OK);
  }
}
/******************************************************************************
 * Function         phNxpNciHal_control_granted
 *
 * Description      Called by libnfc-nci when NFCC control is granted to HAL.
 *
 * Returns          Always returns NFCSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpNciHal_control_granted(void) {
  /* Take the concurrency lock so no other calls from upper layer
   * will be allowed
   */
  CONCURRENCY_LOCK();

  if (NULL != nxpncihal_ctrl.p_control_granted_cback) {
    (*nxpncihal_ctrl.p_control_granted_cback)();
  }
  /* At the end concurrency unlock so calls from upper layer will
   * be allowed
   */
  CONCURRENCY_UNLOCK();
  return NFCSTATUS_SUCCESS;
}

/******************************************************************************
 * Function         phNxpNciHal_request_control
 *
 * Description      This function can be used by HAL to request control of
 *                  NFCC to libnfc-nci. When control is provided to HAL it is
 *                  notified through phNxpNciHal_control_granted.
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpNciHal_request_control(void) {
  if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
    /* Request Control of NCI Controller from NCI NFC Stack */
    (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_REQUEST_CONTROL_EVT,
                                        HAL_NFC_STATUS_OK);
  }

  return;
}

/******************************************************************************
 * Function         phNxpNciHal_release_control
 *
 * Description      This function can be used by HAL to release the control of
 *                  NFCC back to libnfc-nci.
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpNciHal_release_control(void) {
  if (nxpncihal_ctrl.p_nfc_stack_cback != NULL) {
    /* Release Control of NCI Controller to NCI NFC Stack */
    (*nxpncihal_ctrl.p_nfc_stack_cback)(HAL_NFC_RELEASE_CONTROL_EVT,
                                        HAL_NFC_STATUS_OK);
  }

  return;
}

/******************************************************************************
 * Function         phNxpNciHal_power_cycle
 *
 * Description      This function is called by libnfc-nci when power cycling is
 *                  performed. When processing is complete it is notified to
 *                  libnfc-nci through phNxpNciHal_power_cycle_complete.
 *
 * Returns          Always return NFCSTATUS_SUCCESS (0).
 *
 ******************************************************************************/
int phNxpNciHal_power_cycle(void) {
  NXPLOG_NCIHAL_D("Power Cycle");
  NFCSTATUS status = NFCSTATUS_FAILED;
  if (nxpncihal_ctrl.halStatus != HAL_STATUS_OPEN) {
    NXPLOG_NCIHAL_D("Power Cycle failed due to hal status not open");
    return NFCSTATUS_FAILED;
  }
  status = phTmlNfc_IoCtl(phTmlNfc_e_ResetDevice);

  if (NFCSTATUS_SUCCESS == status) {
    NXPLOG_NCIHAL_D("PN72xx Reset - SUCCESS\n");
  } else {
    NXPLOG_NCIHAL_D("PN72xx Reset - FAILED\n");
  }

  phNxpNciHal_power_cycle_complete(NFCSTATUS_SUCCESS);
  return NFCSTATUS_SUCCESS;
}

/******************************************************************************
 * Function         phNxpNciHal_power_cycle_complete
 *
 * Description      This function is called to provide the status of
 *                  phNxpNciHal_power_cycle to libnfc-nci through callback.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_power_cycle_complete(NFCSTATUS status) {
  static phLibNfc_Message_t msg;

  if (status == NFCSTATUS_SUCCESS) {
    msg.eMsgType = NCI_HAL_OPEN_CPLT_MSG;
  } else {
    msg.eMsgType = NCI_HAL_ERROR_MSG;
  }
  msg.pMsgData = NULL;
  msg.Size = 0;

  phTmlNfc_DeferredCall(gpphTmlNfc_Context->dwCallbackThreadId, &msg);

  return;
}
/******************************************************************************
 * Function         phNxpNciHal_check_ncicmd_write_window
 *
 * Description      This function is called to check the write synchroniztion
 *                  status if write already acquired then wait for corresponding
                    read to complete.
 *
 * Returns          return 0 on success and -1 on fail.
 *
 ******************************************************************************/

int phNxpNciHal_check_ncicmd_write_window(uint16_t cmd_len, uint8_t *p_cmd) {
  UNUSED_PROP(cmd_len);
  NFCSTATUS status = NFCSTATUS_FAILED;
  int sem_timedout = 2, s;
  struct timespec ts;

  if (cmd_len < 1) {
    android_errorWriteLog(0x534e4554, "153880357");
    return NFCSTATUS_FAILED;
  }

  if ((p_cmd[0] & 0xF0) == 0x20) {
    clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += sem_timedout;
    while ((s = sem_timedwait_monotonic_np(&nxpncihal_ctrl.syncSpiNfc, &ts)) ==
               -1 &&
           errno == EINTR) {
      continue; /* Restart if interrupted by handler */
    }
    if (s != -1) {
      status = NFCSTATUS_SUCCESS;
    }
  } else {
    /* cmd window check not required for writing data packet */
    status = NFCSTATUS_SUCCESS;
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_nfccClockCfgRead
 *
 * Description      This function is called for loading a data strcuture from
 *                  the config file with clock source and clock frequency values
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_nfccClockCfgRead(void) {
  unsigned long num = 0;
  int isfound = 0;

  nxpprofile_ctrl.bClkSrcVal = 0;
  nxpprofile_ctrl.bClkFreqVal = 0;
  if (nfcFL.chipType == pn7160) {
        nxpprofile_ctrl.bTimeout = 0;
  }

  isfound = GetNxpNumValue(NAME_NXP_SYS_CLK_SRC_SEL, &num, sizeof(num));
  if (isfound > 0) {
    nxpprofile_ctrl.bClkSrcVal = num;
  }

  num = 0;
  isfound = 0;
  isfound = GetNxpNumValue(NAME_NXP_SYS_CLK_FREQ_SEL, &num, sizeof(num));
  if (isfound > 0) {
    nxpprofile_ctrl.bClkFreqVal = num;
  }

  if(nfcFL.chipType == pn7160) {
    num = 0;
    isfound = 0;
    isfound = GetNxpNumValue(NAME_NXP_SYS_CLOCK_TO_CFG, &num, sizeof(num));
    if (isfound > 0)
    {
        nxpprofile_ctrl.bTimeout = num;
    }
  }

  NXPLOG_FWDNLD_D("gphNxpNciHal_fw_IoctlCtx.bClkSrcVal = 0x%x",
                  nxpprofile_ctrl.bClkSrcVal);
  NXPLOG_FWDNLD_D("gphNxpNciHal_fw_IoctlCtx.bClkFreqVal = 0x%x",
                  nxpprofile_ctrl.bClkFreqVal);

  if ((nxpprofile_ctrl.bClkSrcVal < CLK_SRC_XTAL) ||
      (nxpprofile_ctrl.bClkSrcVal > CLK_SRC_PLL)) {
    NXPLOG_FWDNLD_E("Clock source value is wrong in config file, setting it as "
                    "default XTAL");
    nxpprofile_ctrl.bClkSrcVal = NXP_SYS_CLK_SRC_SEL;
  }
  if (nfcFL.chipType == pn7160) {
    if ((nxpprofile_ctrl.bClkFreqVal < CLK_FREQ_13MHZ) ||
            (nxpprofile_ctrl.bClkFreqVal > CLK_FREQ_52MHZ))
    {
        NXPLOG_FWDNLD_E("Clock frequency value is wrong in config file, setting it as default");
        nxpprofile_ctrl.bClkFreqVal = NXP_SYS_CLK_FREQ_SEL;
    }
    if ((nxpprofile_ctrl.bTimeout < CLK_TO_CFG_DEF) || (nxpprofile_ctrl.bTimeout > CLK_TO_CFG_MAX))
    {
        NXPLOG_FWDNLD_E("Clock timeout value is wrong in config file, setting it as default");
        nxpprofile_ctrl.bTimeout = CLK_TO_CFG_DEF;
    }
  }
}

/******************************************************************************
 * Function         phNxpNciHal_LmNfcADiscCfgRead
 *
 * Description      This function is called for loading HCE Listen Mode - NFC-A
 *                  Discovery Parameters from the config file
 *
 * Returns          bool.
 *
 ******************************************************************************/
static bool phNxpNciHal_LmNfcADiscCfgRead(void) {
  uint8_t isfound = 0;
  long retlen = 0;

  memset(&new_Cfg, 0x00, sizeof(phNxpNci_LmNfcADiscCfg_t));

  isfound =
      GetNxpByteArrayValue(NAME_NXP_HCE_SENS_RES, (char *)&new_Cfg.bSensRes[0],
                           sizeof(new_Cfg.bSensRes), &retlen);
  if (isfound == 0x00) {
    NXPLOG_NCIHAL_E("New Config: NAME_NXP_HCE_SENS_RES Not found");
    return false;
  }

  isfound =
      GetNxpByteArrayValue(NAME_NXP_HCE_NFC_ID1, (char *)&new_Cfg.bNfcID1[0],
                           sizeof(new_Cfg.bNfcID1), &retlen);
  if (isfound == 0x00) {
    NXPLOG_NCIHAL_E("New Config: NAME_NXP_HCE_NFC_ID1 Not found");
    return false;
  }

  isfound = GetNxpNumValue(NAME_NXP_HCE_SEL_RES, &new_Cfg.bSelRes,
                           sizeof(new_Cfg.bSelRes));
  if (isfound == 0x00) {
    NXPLOG_NCIHAL_E("New Config: NAME_NXP_HCE_SEL_RES Not found");
    return false;
  }

  isfound = GetNxpNumValue(NAME_NXP_HCE_RNDM_UID_ENB, &new_Cfg.bRandomUIDEnable,
                           sizeof(new_Cfg.bRandomUIDEnable));
  if (isfound == 0x00) {
    NXPLOG_NCIHAL_E("New Config: NAME_NXP_HCE_RNDM_UID_ENB Not found");
    return false;
  }
  return true;
}

/******************************************************************************
 * Function         phNxpNciHal_nfccClockCfgApply
 *
 * Description      This function is called after successful download
 *                  to check if clock settings in config file and chip
 *                  is same
 *
 * Returns          status.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_nfccClockCfgApply(void) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  static uint8_t* get_clock_cmd;
  uint8_t get_clk_size = 0;
  uint8_t set_clck_cmd[] = {0x20, 0x02, 0x0C, 0x01, 0xA2, 0x02, 0x08, 0x00,
                            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t get_clck_cmd[] = {0x20, 0x03, 0x03, 0x01, 0xA2, 0x02};
  uint8_t get_clck_cmd_pn7160[] = {0x20, 0x03, 0x07, 0x03, 0xA0,
                                   0x02, 0xA0, 0x03, 0xA0, 0x04};
  uint8_t nfcc_cfg_clock_src, nfcc_cur_clock_src;
  uint8_t nfcc_clock_set_needed;
  if (nfcFL.chipType != pn7160) {
    get_clock_cmd = get_clck_cmd;
    get_clk_size = sizeof(get_clck_cmd);
    NXPLOG_NCIHAL_E("pn7220 unable to retrieve get_clk_src_sel %d", get_clk_size);

  } else {
    get_clock_cmd = get_clck_cmd_pn7160;
    get_clk_size = sizeof(get_clck_cmd_pn7160);
    NXPLOG_NCIHAL_E("pn7160 unable to retrieve get_clk_src_sel %d",get_clk_size);
  }
  phNxpNciHal_nfccClockCfgRead();
  phNxpNciClock.isClockSet = true;
  status = phNxpNciHal_send_ext_cmd(get_clk_size, get_clock_cmd);
  phNxpNciClock.isClockSet = false;

  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("unable to retrieve get_clk_src_sel");
    return status;
  }

  if (nfcFL.chipType != pn7160) {
    /* Set the system frequency only if, it's missmatch with current sys clk */
    if (nxpprofile_ctrl.bClkFreqVal != phNxpNciClock.p_rx_data[9]) {
      /*Read the preset value from FW*/
      memcpy(&set_clck_cmd[7], &phNxpNciClock.p_rx_data[8],
             phNxpNciClock.p_rx_data[7]);
      /*Update clock source and frequency as per DH configuration*/
      set_clck_cmd[8] = nxpprofile_ctrl.bClkFreqVal;
      status = phNxpNciHal_send_ext_cmd(sizeof(set_clck_cmd), set_clck_cmd);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("Failed to set system Frequency");
        return status;
      }
    }
  } else {
      nfcc_cfg_clock_src = phNxpNciHal_determineConfiguredClockSrc();
      nfcc_cur_clock_src = phNxpNciClock.p_rx_data[12];

      nfcc_clock_set_needed = (nfcc_cfg_clock_src != nfcc_cur_clock_src ||
                                  phNxpNciClock.p_rx_data[16] == nxpprofile_ctrl.bTimeout) ?\
                                  true : false;

      if(nfcc_clock_set_needed) {
        NXPLOG_NCIHAL_D ("Setting Clock Source and Frequency");
        phNxpNciHal_txNfccClockSetCmd();
        }
    }

    return status;
}

/******************************************************************************
 * Function         phNxpNciHal_nfccTdaPpsCfgApply
 *
 * Description      This function is called to check the CT Single slot without
 *                  TDA configuration is required. if required and configure it.
 *
 * Returns          status.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_nfccTdaPpsCfgApply(void) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  unsigned long isTdaPrsnt = 0x00;
  unsigned long isPPSEnabled = 0x00;
  int isTdaCongfound = 0x00;
  int isPpsCongfound = 0x00;
  uint8_t dutTdaPPSCfgVal = 0x00;
  bool isCfgUpdateNeeded = false;

  NXPLOG_NCIHAL_D("%s Enter", __func__);

  uint8_t getTdaPpsCnfgCmd[] = {0x20, 0x03, 0x03, 0x01, 0xA2, 0xE0};

  uint8_t setTdaPpsCnfgCmd[] = {0x20, 0x02, 0x07, 0x01, 0xA2,
                                0xE0, 0x03, 0x00, 0x00, 0x00};

  isTdaCongfound = GetNxpNumValue(NAME_NXP_IS_TDA_CHIP_PRESENT, &isTdaPrsnt,
                           sizeof(isTdaPrsnt));

  isPpsCongfound = GetNxpNumValue(NAME_NXP_ENABLE_DISBLE_PPS_EXCHANGE, &isPPSEnabled,
                           sizeof(isPPSEnabled));

  if((isTdaCongfound == 0x00) && (isPpsCongfound == 0x00)){
    NXPLOG_NCIHAL_E("CT_TDA_PPS_PROT_CONFIG not found\n");
    return NFCSTATUS_FAILED;
  }

  status = phNxpNciHal_send_ext_cmd(sizeof(getTdaPpsCnfgCmd), getTdaPpsCnfgCmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("unable to retrieve getTdaPpsCnfgCmd");
    return status;
  }

  // DUT CT_TDA_PPS_PROT_CONFIG value
  dutTdaPPSCfgVal = nxpncihal_ctrl.p_rx_data[NXP_TDA_PPS_RSP_CONFIG_INDEX];
  memcpy(&setTdaPpsCnfgCmd[7], &nxpncihal_ctrl.p_rx_data[8], setTdaPpsCnfgCmd[6]);

  if (isTdaCongfound) {
    // Check if 1st bit (TDA) is set
    uint8_t currentTda = (dutTdaPPSCfgVal & 0x01);
    NXPLOG_NCIHAL_D("Current val: TDA[%d]\n", currentTda);

    // Check if changes are needed
    if (currentTda != isTdaPrsnt) {
      isCfgUpdateNeeded = true;    // Update is needed for the 1st bit (TDA)
      dutTdaPPSCfgVal ^= 0x01; // toggle the 1st bit
    }
  }

  if (isPpsCongfound) {
    // Check if 2nd bit (PPA) is set
    uint8_t currentPpa = ((dutTdaPPSCfgVal >> NXP_PPS_CONFIG_BIT_POS) & 0x01);
    NXPLOG_NCIHAL_D("Current val: PPS[%d]\n", currentPpa);

    if (currentPpa != isPPSEnabled) {
      isCfgUpdateNeeded = true; // Update is needed for the 2nd bit (PPA)
      dutTdaPPSCfgVal ^=
          (0x01 << NXP_PPS_CONFIG_BIT_POS); // toggle the 2nd bit
    }
  }

  if (isCfgUpdateNeeded) {
    setTdaPpsCnfgCmd[NXP_TDA_PPS_CMD_CONFIG_INDEX] = dutTdaPPSCfgVal;
    status =
        phNxpNciHal_send_ext_cmd(sizeof(setTdaPpsCnfgCmd), setTdaPpsCnfgCmd);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("unable to retrieve setTdaPpsCnfgCmd");
      return status;
    }
  } else {
    NXPLOG_NCIHAL_D("%s : No update needed", __func__);
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_nfccPcdSettingApply
 *
 * Description      This function is called to apply the PCD Setting.
 *
 * Returns          status.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_nfccPcdSettingApply(void) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  uint8_t isfound = 0;
  uint8_t buff[260] = {0};
  long rlen = 0;
  long bufflen = 260;

  NXPLOG_NCIHAL_D("Performing NAME_NXP_PCD_SETTINGS");
  isfound = GetNxpByteArrayValue(NAME_NXP_PCD_SETTINGS,
                            (char *)buff, bufflen, &rlen);
  if ((isfound == 1) && (rlen > 0)) {
    status = phNxpNciHal_send_ext_cmd(rlen, buff);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("PCD Setting apply failed");
    }
  } else {
    NXPLOG_NCIHAL_E("NAME_NXP_PCD_SETTINGS Not found");
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_flushSramToFlash
 *
 * Description      This function is called to check and send the NCI cmd to
 *                  Flush the SRAM data to FLASH memory
 *
 * Returns          status.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_flushSramToFlash(void){
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  int isfound = 0x00;
  unsigned long isSramFlushReq = 0x00;
  NXPLOG_NCIHAL_D("Performing NAME_NXP_FLUSH_SRAM_TO_FLASH_ENABLE");
  isfound = GetNxpNumValue(NAME_NXP_FLUSH_SRAM_TO_FLASH_ENABLE, &isSramFlushReq,
                       sizeof(isSramFlushReq));
  if((isfound == 0x00) || (isSramFlushReq == 0x00)){
    NXPLOG_NCIHAL_E("Flush SRAM A0 to Flash is not required\n");
    return NFCSTATUS_FAILED;
  }

  status = phNxpNciHal_ext_send_sram_config_to_flash();
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("Updation of the SRAM contents failed");
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_hceLmNfcADiscCfgApply
 *
 * Description      This function is called to check if Listen Mode - NFC-A
 *                  Discovery Parameters in config file and chip are same or
 *not. if not then set the new Parameters from config file.
 *
 * Returns          status.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_hceLmNfcADiscCfgApply(void) {

  NXPLOG_NCIHAL_D("%s", __func__);
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  uint8_t get_a2_cfg_cmd[] = {0x20, 0x03, 0x03, 0x01, 0xA2, 0x78};
  uint8_t set_a2_cfg_cmd[] = {0x20, 0x02, 0x0C, 0x01, 0xA2, 0x78, 0x08, 0x00,
                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  if (phNxpNciHal_LmNfcADiscCfgRead() == false) {
    NXPLOG_NCIHAL_E("Failed to read the config file");
    return NFCSTATUS_FAILED;
  }

  status = phNxpNciHal_send_ext_cmd(sizeof(get_a2_cfg_cmd), get_a2_cfg_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("unable to retrieve bSensRes & bNfcID1");
    return status;
  }

  if ((NFCSTATUS_SUCCESS == nxpncihal_ctrl.p_rx_data[3]) &&
      ((nxpncihal_ctrl.p_rx_data[13] != new_Cfg.bSensRes[0]) ||
       (nxpncihal_ctrl.p_rx_data[14] != new_Cfg.bSensRes[1]) ||
       (nxpncihal_ctrl.p_rx_data[15] != new_Cfg.bNfcID1[0]))) {

    memcpy(&set_a2_cfg_cmd[7], &nxpncihal_ctrl.p_rx_data[8],
           nxpncihal_ctrl.p_rx_data[7]);

    set_a2_cfg_cmd[12] = new_Cfg.bSensRes[0];
    set_a2_cfg_cmd[13] = new_Cfg.bSensRes[1];
    set_a2_cfg_cmd[14] = new_Cfg.bNfcID1[0];

    status = phNxpNciHal_send_ext_cmd(sizeof(set_a2_cfg_cmd), set_a2_cfg_cmd);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Failed to set bSensRes & bNfcID1");
      return status;
    }
  }

  // EEPROM area config A2xx : xx - Offset Address
  get_a2_cfg_cmd[5] = 0x79;
  set_a2_cfg_cmd[5] = 0x79;

  status = phNxpNciHal_send_ext_cmd(sizeof(get_a2_cfg_cmd), get_a2_cfg_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("unable to retrieve bNfcID1 & bSelRes");
    return status;
  }

  if ((NFCSTATUS_SUCCESS == nxpncihal_ctrl.p_rx_data[3]) &&
      ((nxpncihal_ctrl.p_rx_data[8] != new_Cfg.bNfcID1[1]) ||
       (nxpncihal_ctrl.p_rx_data[9] != new_Cfg.bNfcID1[2]) ||
       (nxpncihal_ctrl.p_rx_data[10] != new_Cfg.bSelRes))) {

    memcpy(&set_a2_cfg_cmd[7], &nxpncihal_ctrl.p_rx_data[8],
           nxpncihal_ctrl.p_rx_data[7]);

    set_a2_cfg_cmd[7] = new_Cfg.bNfcID1[1];
    set_a2_cfg_cmd[8] = new_Cfg.bNfcID1[2];
    set_a2_cfg_cmd[9] = new_Cfg.bSelRes;

    status = phNxpNciHal_send_ext_cmd(sizeof(set_a2_cfg_cmd), set_a2_cfg_cmd);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Failed to set bNfcID1 & bSelRes");
      return status;
    }
  }

  // EEPROM area config A2xx : xx - Offset Address
  get_a2_cfg_cmd[5] = 0x7B;
  set_a2_cfg_cmd[5] = 0x7B;

  status = phNxpNciHal_send_ext_cmd(sizeof(get_a2_cfg_cmd), get_a2_cfg_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("unable to retrieve bRandomUIDEnable");
    return status;
  }

  if ((NFCSTATUS_SUCCESS == nxpncihal_ctrl.p_rx_data[3]) &&
      (nxpncihal_ctrl.p_rx_data[13] != new_Cfg.bRandomUIDEnable)) {

    memcpy(&set_a2_cfg_cmd[7], &nxpncihal_ctrl.p_rx_data[8],
           nxpncihal_ctrl.p_rx_data[7]);

    set_a2_cfg_cmd[12] = new_Cfg.bRandomUIDEnable;

    status = phNxpNciHal_send_ext_cmd(sizeof(set_a2_cfg_cmd), set_a2_cfg_cmd);
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("Failed to set bRandomUIDEnable");
      return status;
    }
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_get_mw_eeprom
 *
 * Description      This function is called to retrieve data in mw eeprom area
 *
 * Returns          NFCSTATUS.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_get_mw_eeprom(void) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  uint8_t retry_cnt = 0;
  static uint8_t get_mw_eeprom_cmd[] = {0x20, 0x03, 0x03, 0x01, 0xA0, 0x0F};

retry_send_ext:
  if (retry_cnt > 3) {
    return NFCSTATUS_FAILED;
  }

  phNxpNciMwEepromArea.isGetEepromArea = true;
  status =
      phNxpNciHal_send_ext_cmd(sizeof(get_mw_eeprom_cmd), get_mw_eeprom_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("unable to get the mw eeprom data");
    phNxpNciMwEepromArea.isGetEepromArea = false;
    retry_cnt++;
    goto retry_send_ext;
  }
  phNxpNciMwEepromArea.isGetEepromArea = false;

  if (phNxpNciMwEepromArea.p_rx_data[12]) {
    fw_download_success = 1;
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_set_mw_eeprom
 *
 * Description      This function is called to update data in mw eeprom area
 *
 * Returns          void.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_set_mw_eeprom(void) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  uint8_t retry_cnt = 0;
  uint8_t set_mw_eeprom_cmd[39] = {0};
  uint8_t cmd_header[] = {0x20, 0x02, 0x24, 0x01, 0xA0, 0x0F, 0x20};

  memcpy(set_mw_eeprom_cmd, cmd_header, sizeof(cmd_header));
  phNxpNciMwEepromArea.p_rx_data[12] = 0;
  memcpy(set_mw_eeprom_cmd + sizeof(cmd_header), phNxpNciMwEepromArea.p_rx_data,
         sizeof(phNxpNciMwEepromArea.p_rx_data));

retry_send_ext:
  if (retry_cnt > 3) {
    return NFCSTATUS_FAILED;
  }

  status =
      phNxpNciHal_send_ext_cmd(sizeof(set_mw_eeprom_cmd), set_mw_eeprom_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("unable to update the mw eeprom data");
    retry_cnt++;
    goto retry_send_ext;
  }
  return status;
}
/******************************************************************************
 * Function         phNxpNciHal_config_t4t_ndef
 *
 * Description      This function is called to configure T4T Ndef emulation
 *
 * Returns          void.
 *
 ******************************************************************************/
static NFCSTATUS phNxpNciHal_config_t4t_ndef(uint8_t t4tFlag) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  NXPLOG_NCIHAL_D("NxpNci phNxpNciHal_enable_ndef");
  uint8_t retry_cnt = 0;
  uint8_t set_mw_eeprom_cmd[8] = {0};
  uint8_t cmd_header[] = {0x20, 0x02, 0x05, 0x01, 0xA0, 0x95, 0x01, t4tFlag};

  memcpy(set_mw_eeprom_cmd, cmd_header, sizeof(cmd_header));

retry_send_ext:
  if (retry_cnt > 3) {
    return NFCSTATUS_FAILED;
  }

  status =
      phNxpNciHal_send_ext_cmd(sizeof(set_mw_eeprom_cmd), set_mw_eeprom_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_D("unable to update the mw eeprom data");
    retry_cnt++;
    goto retry_send_ext;
  }
  return status;
}
/******************************************************************************
 * Function         phNxpNciHal_china_tianjin_rf_setting
 *
 * Description      This function is called to check RF Setting
 *
 * Returns          Status.
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_china_tianjin_rf_setting(void) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  int isfound = 0;
  int rf_enable = false;
  int rf_val = 0;
  int send_flag;
  uint8_t retry_cnt = 0;
  int enable_bit = 0;
  static uint8_t get_rf_cmd[] = {0x20, 0x03, 0x03, 0x01, 0xA0, 0x85};

retry_send_ext:
  if (retry_cnt > 3) {
    return NFCSTATUS_FAILED;
  }
  send_flag = true;
  phNxpNciRfSet.isGetRfSetting = true;
  status = phNxpNciHal_send_ext_cmd(sizeof(get_rf_cmd), get_rf_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("unable to get the RF setting");
    phNxpNciRfSet.isGetRfSetting = false;
    retry_cnt++;
    goto retry_send_ext;
  }
  phNxpNciRfSet.isGetRfSetting = false;
  if (phNxpNciRfSet.p_rx_data[3] != 0x00) {
    NXPLOG_NCIHAL_E("GET_CONFIG_RSP is FAILED for CHINA TIANJIN");
    return status;
  }
  rf_val = phNxpNciRfSet.p_rx_data[10];
  isfound = (GetNxpNumValue(NAME_NXP_CHINA_TIANJIN_RF_ENABLED,
                            (void*)&rf_enable, sizeof(rf_enable)));
  if (isfound > 0) {
    enable_bit = rf_val & 0x40;
    if ((enable_bit != 0x40) && (rf_enable == 1)) {
      phNxpNciRfSet.p_rx_data[10] |= 0x40;  // Enable if it is disabled
    } else if ((enable_bit == 0x40) && (rf_enable == 0)) {
      phNxpNciRfSet.p_rx_data[10] &= 0xBF;  // Disable if it is Enabled
    } else {
      send_flag = false;  // No need to change in RF setting
    }

    if (send_flag == true) {
      static uint8_t set_rf_cmd[] = {0x20, 0x02, 0x08, 0x01, 0xA0, 0x85,
                                     0x04, 0x50, 0x08, 0x68, 0x00};
      memcpy(&set_rf_cmd[4], &phNxpNciRfSet.p_rx_data[5], 7);
      status = phNxpNciHal_send_ext_cmd(sizeof(set_rf_cmd), set_rf_cmd);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("unable to set the RF setting");
        retry_cnt++;
        goto retry_send_ext;
      }
    }
  }

  return status;
}
/******************************************************************************
 * Function         phNxpNciHal_CheckAndHandleFwTearDown
 *
 * Description      Check Whether chip is in FW download mode, If chip is in
 *                  Download mode and previous session is not complete, then
 *                  Do force FW update.
 *
 * Returns          Status
 *
 ******************************************************************************/
void phNxpNciHal_CheckAndHandleFwTearDown() {
  NFCSTATUS status = NFCSTATUS_FAILED;
  uint8_t session_state = -1;
  unsigned long minimal_fw_version = DEFAULT_MINIMAL_FW_VERSION;
#if (NXP_EXTNS == TRUE)
  status = phNxpNciHal_getChipInfoInFwDnldMode(false);
#else
  status = phNxpNciHal_getChipInfoInFwDnldMode();
#endif
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("Get Chip Info Failed");
    usleep(150 * 1000);
    return;
  }
  if (!GetNxpNumValue(NAME_NXP_MINIMAL_FW_VERSION, &minimal_fw_version,
                      sizeof(minimal_fw_version))) {
    /* If config file doesn't contain the info use default */
    minimal_fw_version = DEFAULT_MINIMAL_FW_VERSION;
  }
  if (wFwVerRsp != minimal_fw_version) {
    session_state = phNxpNciHal_getSessionInfoInFwDnldMode();
    if (session_state == 0) {
      NXPLOG_NCIHAL_E("NFC not in the teared state, boot NFCC in NCI mode");
      return;
    }
  }
  if (nfcFL.chipType != pn7160)
    phTmlNfc_IoCtl(phTmlNfc_e_EnableDownloadMode);
  else
    phTmlNfc_IoCtl(phTmlNfc_e_EnableDownloadModeWithVenRst);
  if (wFwVerRsp == minimal_fw_version) {
    /* since minimal fw required dlreset
     * to boot in Download mode */
    status = phNxpNciHal_dlResetInFwDnldMode();
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("DL Reset failed for minimal fw");
    }
  }
  phTmlNfc_EnableFwDnldMode(true);
  nxpncihal_ctrl.fwdnld_mode_reqd = TRUE;

  /* Set the obtained device handle to download module */
  phDnldNfc_SetHwDevHandle();
  NXPLOG_NCIHAL_D("Calling Seq handler for FW Download \n");
  status = phNxpNciHal_fw_download_seq(nxpprofile_ctrl.bClkSrcVal,
                                       nxpprofile_ctrl.bClkFreqVal);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("FW Download Sequence Handler Failed.");
  } else {
    property_set("nfc.fw.force_download", "0");
    fw_download_success = 1;
    isFwDnldTriggered = true;
  }

  status = phNxpNciHal_dlResetInFwDnldMode();
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("DL Reset failed in FW DN mode");
  }
}

/******************************************************************************
 * Function         phNxpNciHal_getChipInfoInFwDnldMode
 *
 * Description      Helper function to get the chip info in download mode
 *
 * Returns          Status
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_getChipInfoInFwDnldMode(bool bIsVenResetReqd) {


  uint8_t get_chip_info_cmd[] = {0x00, 0x04, 0xE1, 0x00,
                                 0x00, 0x00, 0x75, 0x48};
  uint8_t get_chip_info_cmd_pn716x[] = {0x00, 0x04, 0xF1, 0x00,
                                        0x00, 0x00, 0x6E, 0xEF};
  NFCSTATUS status = NFCSTATUS_FAILED;
  int retry_cnt = 0;
  if (nfcFL.chipType != pn7160) {
    if (bIsVenResetReqd) {
      status = phTmlNfc_IoCtl(phTmlNfc_e_EnableVen);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("Enable Download mode failed");
        return status;
      }
    }
  } else {
    if (bIsVenResetReqd) {
      status = phTmlNfc_IoCtl(phTmlNfc_e_EnableDownloadModeWithVenRst);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("Enable Download mode failed");
        return status;
      }
    }
  }
  phTmlNfc_EnableFwDnldMode(true);
  nxpncihal_ctrl.fwdnld_mode_reqd = TRUE;
  do {
    if (nfcFL.chipType != pn7160) {
      status = phNxpNciHal_send_ext_cmd(sizeof(get_chip_info_cmd),
                                        get_chip_info_cmd);
    } else {
      status = phNxpNciHal_send_ext_cmd(sizeof(get_chip_info_cmd_pn716x),
                                        get_chip_info_cmd_pn716x);
    }
    if (status != NFCSTATUS_SUCCESS) {
      /* break the loop if HAL write failed or response Timeout */
      break;
    } else {
      /* Check FW getResponse command response status byte */
      if (nxpncihal_ctrl.p_rx_data[0] == 0x00) {
        if (nxpncihal_ctrl.p_rx_data[2] != 0x00) {
          status = NFCSTATUS_FAILED;
          if (retry_cnt < MAX_RETRY_COUNT) {
            retry_cnt++;
            /*reset NFCC state to avoid any failures
             *such as DL_PROTOCOL_ERROR
             */
            status = phNxpNciHal_dlResetInFwDnldMode();
            if (status != NFCSTATUS_SUCCESS) {
              NXPLOG_NCIHAL_E("DL Reset failed in FW DN mode");
            }
          }
        }
      } else {
        status = NFCSTATUS_FAILED;
        break;
      }
    }
  } while ((status != NFCSTATUS_SUCCESS) && (retry_cnt < MAX_RETRY_COUNT));

  nxpncihal_ctrl.fwdnld_mode_reqd = FALSE;
  phTmlNfc_EnableFwDnldMode(false);
  if (phNxpNciHal_enableTmlRead() != NFCSTATUS_PENDING) {
    NXPLOG_NCIHAL_E("%s read status error status", __FUNCTION__);
  }
  if (status == NFCSTATUS_SUCCESS) {
    phNxpNciHal_configFeatureList(nxpncihal_ctrl.p_rx_data,
                                  nxpncihal_ctrl.rx_data_len);
    setNxpFwConfigPath();
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_getSessionInfoInFwDnldMode
 *
 * Description      Helper function to get the session info in download mode
 *
 * Returns          0 means session closed
 *
 ******************************************************************************/
uint8_t phNxpNciHal_getSessionInfoInFwDnldMode() {
  uint8_t session_status = -1;
  NFCSTATUS status = NFCSTATUS_FAILED;
#if (NXP_EXTNS == TRUE)
  uint8_t get_session_info_cmd[] = {0x00, 0x04, 0xDB, 0x00,
                                    0x00, 0x00, 0x31, 0x0A};
  uint8_t get_session_info_cmd_pn716x[] = {0x00, 0x04, 0xF2, 0x00,
                                           0x00, 0x00, 0xF5, 0x33};
#else
  uint8_t get_session_info_cmd[] = {0x00, 0x04, 0xF2, 0x00,
                                    0x00, 0x00, 0xF5, 0x33};
#endif
  phTmlNfc_EnableFwDnldMode(true);
  nxpncihal_ctrl.fwdnld_mode_reqd = TRUE;
  if (nfcFL.chipType != pn7160) {
    status = phNxpNciHal_send_ext_cmd(sizeof(get_session_info_cmd),
                                      get_session_info_cmd);
  } else {
    status = phNxpNciHal_send_ext_cmd(sizeof(get_session_info_cmd_pn716x),
                                      get_session_info_cmd_pn716x);
  }
  if (status == NFCSTATUS_SUCCESS) {
    /* Check FW getResponse command response status byte */
    if (nxpncihal_ctrl.p_rx_data[2] == 0x00 &&
        nxpncihal_ctrl.p_rx_data[0] == 0x00) {
      if (nxpncihal_ctrl.p_rx_data[3] == 0x00) {
        session_status = 0;
      }
    } else {
      NXPLOG_NCIHAL_D("get session info Failed !!!");
      usleep(150 * 1000);
    }
  }
  status = phNxpNciHal_dlResetInFwDnldMode();
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("DL Reset failed in FW DN mode");
  }
  return session_status;
}

/******************************************************************************
 * Function         phNxpNciHal_dlResetInFwDnldMode
 *
 * Description      Helper function to change the mode from FW to NCI
 *
 * Returns          Status
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_dlResetInFwDnldMode() {
  NFCSTATUS status = NFCSTATUS_FAILED;
  phTmlNfc_EnableFwDnldMode(true);
  nxpncihal_ctrl.fwdnld_mode_reqd = TRUE;
  NXPLOG_NCIHAL_D("Sending DL Reset for NFCC soft reboot");
  phDnldNfc_SetHwDevHandle();
  if (nfcFL.chipType >= sn100u) {
    phDnldNfc_SetI2CFragmentLength(NCI_CMDRESP_MAX_BUFF_SIZE_SNXXX);
  } else {
    phDnldNfc_SetI2CFragmentLength(NCI_CMDRESP_MAX_BUFF_SIZE_PN557);
  }

  status = phNxpNciHal_fw_dnld_switch_normal_mode();

  nxpncihal_ctrl.fwdnld_mode_reqd = FALSE;
  phTmlNfc_EnableFwDnldMode(false);
  phDnldNfc_ReSetHwDevHandle();
  if (phNxpNciHal_enableTmlRead() != NFCSTATUS_PENDING) {
    NXPLOG_NCIHAL_E("%s read status error status", __FUNCTION__);
    status = NFCSTATUS_FAILED;
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_nfcc_core_reset_init
 *
 * Description      Helper function to do nfcc core reset & core init
 *
 * Returns          Status
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_nfcc_core_reset_init(bool keep_config) {
  NFCSTATUS status = NFCSTATUS_FAILED;
  uint8_t retry_cnt = 0;
  uint8_t cmd_reset_nci[] = {0x20, 0x00, 0x01, 0x01};

  if (keep_config) {
    cmd_reset_nci[3] = 0x00;
  }
retry_core_reset:
  status = phNxpNciHal_send_ext_cmd(sizeof(cmd_reset_nci), cmd_reset_nci);
  if ((status != NFCSTATUS_SUCCESS) && (retry_cnt < 3)) {
    NXPLOG_NCIHAL_D("Retry: NCI_CORE_RESET");
    retry_cnt++;
    goto retry_core_reset;
  } else if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("NCI_CORE_RESET failed!!!\n");
    return status;
  }

  retry_cnt = 0;
  uint8_t cmd_init_nci[] = {0x20, 0x01, 0x00};
  uint8_t cmd_init_nci2_0[] = {0x20, 0x01, 0x02, 0x00, 0x00};
retry_core_init:
  if (nxpncihal_ctrl.nci_info.nci_version == NCI_VERSION_2_0) {
    status = phNxpNciHal_send_ext_cmd(sizeof(cmd_init_nci2_0), cmd_init_nci2_0);
  } else {
    status = phNxpNciHal_send_ext_cmd(sizeof(cmd_init_nci), cmd_init_nci);
  }

  if ((status != NFCSTATUS_SUCCESS) && (retry_cnt < 3)) {
    NXPLOG_NCIHAL_D("Retry: NCI_CORE_INIT\n");
    retry_cnt++;
    goto retry_core_init;
  } else if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("NCI_CORE_INIT failed!!!\n");
    return status;
  }

  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_resetDefaultSettings
 *
 * Description      Helper function to do nfcc core reset, core init
 *                  (if previously firmware update was triggered) and
 *                  apply default NFC settings
 *
 * Returns          Status
 *
 ******************************************************************************/
NFCSTATUS phNxpNciHal_resetDefaultSettings(uint8_t fw_update_req,
                                           bool keep_config) {
  NFCSTATUS status = NFCSTATUS_SUCCESS;
  if (fw_update_req) {
    status = phNxpNciHal_nfcc_core_reset_init(keep_config);
  }
  return status;
}

/******************************************************************************
 * Function         phNxpNciHal_enable_i2c_fragmentation
 *
 * Description      This function is called to process the response status
 *                  and print the status byte.
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpNciHal_enable_i2c_fragmentation() {
  NFCSTATUS status = NFCSTATUS_FAILED;
  static uint8_t fragmentation_enable_config_cmd[] = {0x20, 0x02, 0x05, 0x01,
                                                      0xA0, 0x05, 0x01, 0x10};
  long i2c_status = 0x00;
  long config_i2c_vlaue = 0xff;
  /*NCI_RESET_CMD*/
  static uint8_t cmd_reset_nci[] = {0x20, 0x00, 0x01, 0x00};
  /*NCI_INIT_CMD*/
  static uint8_t cmd_init_nci[] = {0x20, 0x01, 0x00};
  static uint8_t cmd_init_nci2_0[] = {0x20, 0x01, 0x02, 0x00, 0x00};
  static uint8_t get_i2c_fragmentation_cmd[] = {0x20, 0x03, 0x03,
                                                0x01, 0xA0, 0x05};
  if (GetNxpNumValue(NAME_NXP_I2C_FRAGMENTATION_ENABLED, (void*)&i2c_status,
                     sizeof(i2c_status)) == true) {
    NXPLOG_FWDNLD_D("I2C status : %ld", i2c_status);
  } else {
    NXPLOG_FWDNLD_E("I2C status read not succeeded. Default value : %ld",
                    i2c_status);
  }
  status = phNxpNciHal_send_ext_cmd(sizeof(get_i2c_fragmentation_cmd),
                                    get_i2c_fragmentation_cmd);
  if (status != NFCSTATUS_SUCCESS) {
    NXPLOG_NCIHAL_E("unable to retrieve  get_i2c_fragmentation_cmd");
  } else {
    if (nxpncihal_ctrl.p_rx_data[8] == 0x10) {
      config_i2c_vlaue = 0x01;
      phNxpNciHal_notify_i2c_fragmentation();
      phTmlNfc_set_fragmentation_enabled(I2C_FRAGMENTATION_ENABLED);
    } else if (nxpncihal_ctrl.p_rx_data[8] == 0x00) {
      config_i2c_vlaue = 0x00;
    }
    // if the value already matches, nothing to be done
    if (config_i2c_vlaue != i2c_status) {
      if (i2c_status == 0x01) {
        /* NXP I2C fragmenation enabled*/
        status =
            phNxpNciHal_send_ext_cmd(sizeof(fragmentation_enable_config_cmd),
                                     fragmentation_enable_config_cmd);
        if (status != NFCSTATUS_SUCCESS) {
          NXPLOG_NCIHAL_E("NXP fragmentation enable failed");
        }
      } else if (i2c_status == 0x00 || config_i2c_vlaue == 0xff) {
        fragmentation_enable_config_cmd[7] = 0x00;
        /* NXP I2C fragmentation disabled*/
        status =
            phNxpNciHal_send_ext_cmd(sizeof(fragmentation_enable_config_cmd),
                                     fragmentation_enable_config_cmd);
        if (status != NFCSTATUS_SUCCESS) {
          NXPLOG_NCIHAL_E("NXP fragmentation disable failed");
        }
      }
      status = phNxpNciHal_send_ext_cmd(sizeof(cmd_reset_nci), cmd_reset_nci);
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("NCI_CORE_RESET: Failed");
      }
      if (nxpncihal_ctrl.nci_info.nci_version == NCI_VERSION_2_0) {
        status =
            phNxpNciHal_send_ext_cmd(sizeof(cmd_init_nci2_0), cmd_init_nci2_0);
      } else {
        status = phNxpNciHal_send_ext_cmd(sizeof(cmd_init_nci), cmd_init_nci);
      }
      if (status != NFCSTATUS_SUCCESS) {
        NXPLOG_NCIHAL_E("NCI_CORE_INIT : Failed");
      } else if (i2c_status == 0x01) {
        phNxpNciHal_notify_i2c_fragmentation();
        phTmlNfc_set_fragmentation_enabled(I2C_FRAGMENTATION_ENABLED);
      }
    }
  }
}

/******************************************************************************
 * Function         phNxpNciHal_do_factory_reset
 *
 * Description      This function is called during factory reset to clear/reset
 *                  nfc sub-system persistent data.
 *
 * Returns          void.
 *
 ******************************************************************************/
void phNxpNciHal_do_factory_reset(void) {
  NFCSTATUS status = NFCSTATUS_FAILED;
  if (nxpncihal_ctrl.halStatus == HAL_STATUS_CLOSE) {
    status = phNxpNciHal_MinOpen();
    if (status != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_E("%s: NXP Nfc Open failed", __func__);
      return;
    }
  }
}
/******************************************************************************
 * Function         phNxpNciHal_print_res_status
 *
 * Description      This function is called to process the response status
 *                  and print the status byte.
 *
 * Returns          void.
 *
 ******************************************************************************/
static void phNxpNciHal_print_res_status(uint8_t *p_rx_data, uint16_t *p_len) {
  static uint8_t response_buf[][30] = {"STATUS_OK",
                                       "STATUS_REJECTED",
                                       "STATUS_RF_FRAME_CORRUPTED",
                                       "STATUS_FAILED",
                                       "STATUS_NOT_INITIALIZED",
                                       "STATUS_SYNTAX_ERROR",
                                       "STATUS_SEMANTIC_ERROR",
                                       "RFU",
                                       "RFU",
                                       "STATUS_INVALID_PARAM",
                                       "STATUS_MESSAGE_SIZE_EXCEEDED",
                                       "STATUS_UNDEFINED"};
  int status_byte;
  if (p_rx_data[0] == 0x40 && (p_rx_data[1] == 0x02 || p_rx_data[1] == 0x03)) {
    if (p_rx_data[2] && p_rx_data[3] <= 10) {
      status_byte = p_rx_data[CORE_RES_STATUS_BYTE];
      NXPLOG_NCIHAL_D("%s: response status =%s", __func__,
                      response_buf[status_byte]);
    } else {
      NXPLOG_NCIHAL_D("%s: response status =%s", __func__, response_buf[11]);
    }
    if (phNxpNciClock.isClockSet) {
      int i, len = sizeof(phNxpNciClock.p_rx_data);
      if (*p_len > len) {
        android_errorWriteLog(0x534e4554, "169257710");
      } else {
        len = *p_len;
      }
      for (i = 0; i < len; i++) {
        phNxpNciClock.p_rx_data[i] = p_rx_data[i];
      }
    }

    else if (phNxpNciRfSet.isGetRfSetting) {
      int i, len = sizeof(phNxpNciRfSet.p_rx_data);
      if (*p_len > len) {
        android_errorWriteLog(0x534e4554, "169258733");
      } else {
        len = *p_len;
      }
      for (i = 0; i < len; i++) {
        phNxpNciRfSet.p_rx_data[i] = p_rx_data[i];
        // NXPLOG_NCIHAL_D("%s: response status =0x%x",__func__,p_rx_data[i]);
      }
    } else if (phNxpNciMwEepromArea.isGetEepromArea) {
      int i, len = sizeof(phNxpNciMwEepromArea.p_rx_data) + 8;
      if (*p_len > len) {
        android_errorWriteLog(0x534e4554, "169258884");
      } else {
        len = *p_len;
      }
      for (i = 8; i < len; i++) {
        phNxpNciMwEepromArea.p_rx_data[i - 8] = p_rx_data[i];
      }
    } else if (nxpncihal_ctrl.phNxpNciGpioInfo.state == GPIO_STORE) {
      NXPLOG_NCIHAL_D("%s: Storing GPIO Values...", __func__);
      nxpncihal_ctrl.phNxpNciGpioInfo.values[0] = p_rx_data[9];
      nxpncihal_ctrl.phNxpNciGpioInfo.values[1] = p_rx_data[8];
    } else if (nxpncihal_ctrl.phNxpNciGpioInfo.state == GPIO_RESTORE) {
      NXPLOG_NCIHAL_D("%s: Restoring GPIO Values...", __func__);
      nxpncihal_ctrl.phNxpNciGpioInfo.values[0] = p_rx_data[9];
      nxpncihal_ctrl.phNxpNciGpioInfo.values[1] = p_rx_data[8];
    }
  }

  if (p_rx_data[2] && (config_access == true)) {
    if (p_rx_data[3] != NFCSTATUS_SUCCESS) {
      NXPLOG_NCIHAL_W("Invalid Data from config file.");
      config_success = false;
    }
  }
}
/******************************************************************************
 * Function         phNxpNciHal_initialize_mifare_flag
 *
 * Description      This function gets the value for Mfc flags.
 *
 * Returns          void
 *
 ******************************************************************************/
static void phNxpNciHal_initialize_mifare_flag() {
  unsigned long num = 0;
  bEnableMfcReader = false;
  bDisableLegacyMfcExtns = true;
  // 1: Enable Mifare Classic protocol in RF Discovery.
  // 0: Remove Mifare Classic protocol in RF Discovery.
  if (GetNxpNumValue(NAME_MIFARE_READER_ENABLE, &num, sizeof(num))) {
    bEnableMfcReader = (num == 0) ? false : true;
  }
  // 1: Use legacy JNI MFC extns.
  // 0: Disable legacy JNI MFC extns, use hal MFC Extns instead.
  if (GetNxpNumValue(NAME_LEGACY_MIFARE_READER, &num, sizeof(num))) {
    bDisableLegacyMfcExtns = (num == 0) ? true : false;
  }
}

/*******************************************************************************
**
** Function         phNxpNciHal_configFeatureList
**
** Description      Configures the featureList based on chip type &
**                  Configure fragmentation length based on chip type.
**                  HW Version information number will provide chipType.
**                  HW Version can be obtained from CORE_INIT_RESPONSE(NCI 1.0)
**                  or CORE_RST_NTF(NCI 2.0)
**
** Parameters       CORE_INIT_RESPONSE/CORE_RST_NTF, len
**
** Returns          none
*******************************************************************************/
void phNxpNciHal_configFeatureList(uint8_t *init_rsp, uint16_t rsp_len) {
  nxpncihal_ctrl.chipType = pConfigFL->processChipType(init_rsp, rsp_len);
  tNFC_chipType chipType = nxpncihal_ctrl.chipType;
  NXPLOG_NCIHAL_D("phNxpNciHal_configFeatureList ()chipType = %d", chipType);
  CONFIGURE_FEATURELIST(chipType);
  /* update fragment len based on the chip type.*/
  phTmlNfc_IoCtl(phTmlNfc_e_setFragmentSize);
}

/*******************************************************************************
**
** Function         phNxpNciHal_UpdateFwStatus
**
** Description      It shall be called to update the FW download status to the
**                  libnfc-nci.
**
** Parameters       fwStatus: FW update status
**
** Returns          void
*******************************************************************************/
static void phNxpNciHal_UpdateFwStatus(HalNfcFwUpdateStatus fwStatus) {
  static phLibNfc_Message_t msg;
  static uint8_t status;
  NXPLOG_NCIHAL_D("phNxpNciHal_UpdateFwStatus Enter");

  status = (uint8_t)fwStatus;
  msg.eMsgType = HAL_NFC_FW_UPDATE_STATUS_EVT;
  msg.pMsgData = &status;
  msg.Size = sizeof(status);
  phTmlNfc_DeferredCall(gpphTmlNfc_Context->dwCallbackThreadId,
                        (phLibNfc_Message_t *)&msg);
  return;
}

/******************************************************************************
 * Function         phNxpNciHal_determineConfiguredClockSrc
 *
 * Description      This function determines and encodes clock source based on
 *                  clock frequency
 *
 * Returns          encoded form of clock source
 *
 *****************************************************************************/
int  phNxpNciHal_determineConfiguredClockSrc()
{
    uint8_t param_clock_src = CLK_SRC_PLL;
    if (nxpprofile_ctrl.bClkSrcVal == CLK_SRC_PLL)
    {

        param_clock_src = param_clock_src << 3;

        if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_13MHZ)
        {
            param_clock_src |= 0x00;
        }
        else if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_19_2MHZ)
        {
            param_clock_src |= 0x01;
        }
        else if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_24MHZ)
        {
            param_clock_src |= 0x02;
        }
        else if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_26MHZ)
        {
            param_clock_src |= 0x03;
        }
        else if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_38_4MHZ)
        {
            param_clock_src |= 0x04;
        }
        else if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_52MHZ)
        {
            param_clock_src |= 0x05;
        }
        else if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_32MHZ)
        {
            param_clock_src |= 0x06;
        }
        else if (nxpprofile_ctrl.bClkFreqVal == CLK_FREQ_48MHZ)
        {
            param_clock_src |= 0x0A;
        }
        else
        {
            NXPLOG_NCIHAL_E("Wrong clock freq, send default PLL@19.2MHz");
                param_clock_src = 0x11;
        }
    }
    else if(nxpprofile_ctrl.bClkSrcVal == CLK_SRC_XTAL)
    {
        param_clock_src = 0x08;

    }
    else
    {
        NXPLOG_NCIHAL_E("Wrong clock source. Dont apply any modification");
    }
    return param_clock_src;
  }

  /******************************************************************************
 * Function         phNxpNciHal_setVerboseLogging
 *
 * Description      This function enables the nfc_debug_enabled
 *
 * Returns          void
 *
 *****************************************************************************/

void phNxpNciHal_setVerboseLogging(bool enable) { nfc_debug_enabled = enable; }

/******************************************************************************
 * Function         phNxpNciHal_getVerboseLogging
 *
 * Description      This function returns the value of nfc_debug_enabled
 *
 * Returns          void
 *
 *****************************************************************************/

bool phNxpNciHal_getVerboseLogging() { return nfc_debug_enabled; }