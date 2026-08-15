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
#include "emvco_cl.h"
#include "emvco_ncif.h"
#include <emvco_config.h>
#include <emvco_dm.h>
#include <emvco_log.h>
#include <emvco_nci_ext.h>
#include <emvco_tml.h>
#include <log/log.h>
#include <nci_parser.h>
#include <osal_log.h>
#include <osal_memory.h>
#include <osal_message_queue_lib.h>
#include <pthread.h>

/* Timeout value to wait for response from PN548AD */
#define HAL_EXTNS_WRITE_RSP_TIMEOUT (1000)

#define INDEX_OF_NFCID1_LEN 12
#define SEL_RES_LEN 1
#define SEL_RES_DATA_LEN 1
#define PROTOCOL_T3T 0x03
#define PROTOCOL_ISO_DEP 0x04
#define TYPE_4A_TAG_MASK 0x20
#define TYPE_4B_TAG_MASK 0x01
#define RF_TECH_MODE_NFC_A_PASSIVE_POLL 0x00
#define RF_TECH_MODE_NFC_B_PASSIVE_POLL 0x01
#define SENSB_RESP_PROTOCOL_TYPE_INDEX 20
#define RF_PROTOCOL_INDEX 5
#define RF_TECH_MODE_INDEX 6
#define RF_INTF_ACTIVATED_NTF_BYTE_0 0x61
#define RF_INTF_ACTIVATED_NTF_BYTE_1 0x05
#undef P2P_PRIO_LOGIC_HAL_IMP

/******************* Global variables *****************************************/
extern nci_hal_ctrl_t nci_hal_ctrl;
extern nci_profile_Control_t nxpprofile_ctrl;
extern tml_emvco_context_t *gptml_emvco_context;
// extern uint32_t cleanup_timer;
extern bool emvco_debug_enabled;
extern emvco_args_t *modeSwitchArgs;
extern fp_ct_init_ext_t fp_ct_init_ext;

uint8_t icode_detected = 0x00;
uint8_t icode_send_eof = 0x00;
uint8_t nfcdep_detected = 0x00;
static uint8_t ee_disc_done = 0x00;
uint8_t EnableP2P_PrioLogic = false;

/* NFCEE Set mode */
static uint8_t setEEModeDone = 0x00;
uint32_t wFwVerRsp;
uint16_t wFwVer;

uint16_t rom_version;
extern uint32_t timeoutTimerId;

/************** HAL extension functions ***************************************/
static void hal_extns_write_rsp_timeout_cb(uint32_t timer_id, void *p_context);

/*Proprietary cmd sent to HAL to send reader mode flag
 * Last byte of 4 byte proprietary cmd data contains ReaderMode flag
 * If this flag is enabled, NFC-DEP protocol is modified to T3T protocol
 * if FrameRF interface is selected. This needs to be done as the FW
 * always sends Ntf for FrameRF with NFC-DEP even though FrameRF with T3T is
 * previously selected with DISCOVER_SELECT_CMD
 */
#define PROPRIETARY_CMD_FELICA_READER_MODE 0xFE
static uint8_t gFelicaReaderMode;

static EMVCO_STATUS process_emvco_init_rsp(uint8_t *p_ntf, uint16_t *p_len);
/*******************************************************************************
**
** Function         nci_ext_init
**
** Description      initialize extension function
**
*******************************************************************************/
void nci_ext_init(void) {
  icode_detected = 0x00;
  icode_send_eof = 0x00;
  setEEModeDone = 0x00;
  EnableP2P_PrioLogic = false;
}

static void phNxpNciHal_ext_process_non_emvco_card_ntf() {
  lib_emvco_message_t msg;
  msg.e_msgType = EMVCO_UN_SUPPORTED_CARD_MSG;
  msg.p_msg_data = NULL;
  msg.size = 0;
  memset(msg.data, 0, sizeof(msg.data));
  msg.w_status = EMVCO_STATUS_SUCCESS;
  tml_deferred_call(gptml_emvco_context->dw_callback_thread_id, &msg);
  rf_deactivate(DISCOVER);
}

EMVCO_STATUS process_ext_rsp(uint8_t *p_ntf, uint16_t *p_len) {
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;
  int select_index = 0;

  if (p_ntf[0] == 0x61 && p_ntf[1] == 0x05 && *p_len < 14) {

    if (*p_len <= 6) {
      __osal_log_error_write(0x534e4554, "118152591");
    }
    LOG_EMVCOHAL_E("RF_INTF_ACTIVATED_NTF length error!");
    status = EMVCO_STATUS_FAILED;
    return status;
  }

  if (p_ntf[0] == RF_INTF_ACTIVATED_NTF_BYTE_0 &&
      p_ntf[1] == RF_INTF_ACTIVATED_NTF_BYTE_1) {
    if (p_ntf[RF_PROTOCOL_INDEX] == PROTOCOL_T3T) {
      LOG_EMVCOHAL_D("T3T EMVCo card detected in EmvCo profile");
    } else if ((p_ntf[RF_PROTOCOL_INDEX] == PROTOCOL_ISO_DEP) &&
               (p_ntf[RF_TECH_MODE_INDEX] == RF_TECH_MODE_NFC_A_PASSIVE_POLL) &&
               (*p_len >= INDEX_OF_NFCID1_LEN)) {
      select_index = INDEX_OF_NFCID1_LEN + p_ntf[INDEX_OF_NFCID1_LEN] +
                     SEL_RES_LEN + SEL_RES_DATA_LEN;
      /* Checking the SEL_RES Response format byte to confirm T4A Tag Platform
       * Enablement */
      if (p_ntf[select_index] & TYPE_4A_TAG_MASK) {
        LOG_EMVCOHAL_D("Type-A  ISO DEP EMVCo card detected in EmvCo profile");
      } else {
        LOG_EMVCOHAL_D("InValid SAK. Non EMVCo card detected in EmvCo profile "
                       "- Restart polling");
        phNxpNciHal_ext_process_non_emvco_card_ntf();
        status = EMVCO_STATUS_FAILED;
        return status;
      }
    } else if ((p_ntf[RF_PROTOCOL_INDEX] == PROTOCOL_ISO_DEP) &&
               (p_ntf[RF_TECH_MODE_INDEX] == RF_TECH_MODE_NFC_B_PASSIVE_POLL) &&
               (*p_len >= INDEX_OF_NFCID1_LEN)) {
      /* Checking the SENSB_RES Response format byte to confirm T4B Tag Platform
       * Enablement */
      if (p_ntf[SENSB_RESP_PROTOCOL_TYPE_INDEX] & TYPE_4B_TAG_MASK) {
        LOG_EMVCOHAL_D("Type-B ISO DEP EMVCo card detected in EmvCo profile");
      } else {
        LOG_EMVCOHAL_D("InValid SAK. Non EMVCo card detected in EmvCo profile "
                       "- Restart polling");
        phNxpNciHal_ext_process_non_emvco_card_ntf();
        status = EMVCO_STATUS_FAILED;
        return status;
      }
    } else {
      LOG_EMVCOHAL_D("RF_INTF_ACTIVATED_NTF length error. Non EMVCo card "
                     "detected in EmvCo profile - Restart polling");
      phNxpNciHal_ext_process_non_emvco_card_ntf();
      status = EMVCO_STATUS_FAILED;
      return status;
    }
  }

  if (p_ntf[0] == 0x61 && p_ntf[1] == 0x05 && p_ntf[4] == 0x03 &&
      p_ntf[5] == 0x05 && nxpprofile_ctrl.profile_type == EMV_CO_PROFILE) {
    p_ntf[4] = 0xFF;
    p_ntf[5] = 0xFF;
    p_ntf[6] = 0xFF;
    LOG_EMVCOHAL_D("Nfc-Dep Detect in EmvCo profile - Restart polling");
  }

  if (p_ntf[0] == 0x61 && p_ntf[1] == 0x05 && p_ntf[4] == 0x01 &&
      p_ntf[5] == 0x05 && p_ntf[6] == 0x02 && gFelicaReaderMode) {
    /*If FelicaReaderMode is enabled,Change Protocol to T3T from NFC-DEP
     * when FrameRF interface is selected*/
    p_ntf[5] = 0x03;
    LOG_EMVCOHAL_D("FelicaReaderMode:Activity 1.1");
  }

#ifdef P2P_PRIO_LOGIC_HAL_IMP
  if (p_ntf[0] == 0x61 && p_ntf[1] == 0x05 && p_ntf[4] == 0x02 &&
      p_ntf[5] == 0x04 && nxpprofile_ctrl.profile_type == NFC_FORUM_PROFILE) {
    EnableP2P_PrioLogic = true;
  }

  LOG_EMVCOHAL_D("Is EnableP2P_PrioLogic: 0x0%X", EnableP2P_PrioLogic);

#endif

  status = EMVCO_STATUS_SUCCESS;

  if (p_ntf[0] == 0x61 && p_ntf[1] == 0x05) {
    switch (p_ntf[4]) {
    case 0x00:
      LOG_EMVCOHAL_D("NxpNci: RF Interface = NFCEE Direct RF");
      break;
    case 0x01:
      LOG_EMVCOHAL_D("NxpNci: RF Interface = Frame RF");
      break;
    case 0x02:
      LOG_EMVCOHAL_D("NxpNci: RF Interface = ISO-DEP");
      break;
    case 0x03:
      LOG_EMVCOHAL_D("NxpNci: RF Interface = NFC-DEP");
      break;
    case 0x80:
      LOG_EMVCOHAL_D("NxpNci: RF Interface = MIFARE");
      break;
    default:
      LOG_EMVCOHAL_D("NxpNci: RF Interface = Unknown");
      break;
    }

    switch (p_ntf[5]) {
    case 0x01:
      LOG_EMVCOHAL_D("NxpNci: Protocol = T1T");
      break;
    case 0x02:
      LOG_EMVCOHAL_D("NxpNci: Protocol = T2T");
      break;
    case 0x03:
      LOG_EMVCOHAL_D("NxpNci: Protocol = T3T");
      break;
    case 0x04:
      LOG_EMVCOHAL_D("NxpNci: Protocol = ISO-DEP");
      break;
    case 0x05:
      LOG_EMVCOHAL_D("NxpNci: Protocol = NFC-DEP");
      break;
    case 0x06:
      LOG_EMVCOHAL_D("NxpNci: Protocol = 15693");
      break;
    case 0x80:
      LOG_EMVCOHAL_D("NxpNci: Protocol = MIFARE");
      break;
    case 0x81:
      LOG_EMVCOHAL_D("NxpNci: Protocol = Kovio");
      break;
    default:
      LOG_EMVCOHAL_D("NxpNci: Protocol = Unknown");
      break;
    }

    switch (p_ntf[6]) {
    case 0x00:
      LOG_EMVCOHAL_D("NxpNci: Mode = A Passive Poll");
      break;
    case 0x01:
      LOG_EMVCOHAL_D("NxpNci: Mode = B Passive Poll");
      break;
    case 0x02:
      LOG_EMVCOHAL_D("NxpNci: Mode = F Passive Poll");
      break;
    case 0x03:
      LOG_EMVCOHAL_D("NxpNci: Mode = A Active Poll");
      break;
    case 0x05:
      LOG_EMVCOHAL_D("NxpNci: Mode = F Active Poll");
      break;
    case 0x06:
      LOG_EMVCOHAL_D("NxpNci: Mode = 15693 Passive Poll");
      break;
    case 0x70:
      LOG_EMVCOHAL_D("NxpNci: Mode = Kovio");
      break;
    case 0x80:
      LOG_EMVCOHAL_D("NxpNci: Mode = A Passive Listen");
      break;
    case 0x81:
      LOG_EMVCOHAL_D("NxpNci: Mode = B Passive Listen");
      break;
    case 0x82:
      LOG_EMVCOHAL_D("NxpNci: Mode = F Passive Listen");
      break;
    case 0x83:
      LOG_EMVCOHAL_D("NxpNci: Mode = A Active Listen");
      break;
    case 0x85:
      LOG_EMVCOHAL_D("NxpNci: Mode = F Active Listen");
      break;
    case 0x86:
      LOG_EMVCOHAL_D("NxpNci: Mode = 15693 Passive Listen");
      break;
    default:
      LOG_EMVCOHAL_D("NxpNci: Mode = Unknown");
      break;
    }
  }
  process_emvco_init_rsp(p_ntf, p_len);

  if (p_ntf[0] == 0x61 && p_ntf[1] == 0x05 && p_ntf[2] == 0x15 &&
      p_ntf[4] == 0x01 && p_ntf[5] == 0x06 && p_ntf[6] == 0x06) {
    LOG_EMVCOHAL_D("> Going through workaround - notification of ISO 15693");
    icode_detected = 0x01;
    p_ntf[21] = 0x01;
    p_ntf[22] = 0x01;
  } else if (icode_detected == 1 && icode_send_eof == 2) {
    icode_send_eof = 3;
  } else if (p_ntf[0] == 0x00 && p_ntf[1] == 0x00 && icode_detected == 1) {
    if (icode_send_eof == 3) {
      icode_send_eof = 0;
    }
    if (nci_hal_ctrl.nci_info.nci_version != NCI_VERSION_2_0) {
      if (*p_len <= (p_ntf[2] + 2)) {
        __osal_log_error_write(0x534e4554, "181660091");
        LOG_EMVCOHAL_E("length error!");
        return EMVCO_STATUS_FAILED;
      }
      if (p_ntf[p_ntf[2] + 2] == 0x00) {
        LOG_EMVCOHAL_D("> Going through workaround - data of ISO 15693");
        p_ntf[2]--;
        (*p_len)--;
      } else {
        p_ntf[p_ntf[2] + 2] |= 0x01;
      }
    }
  } else if (p_ntf[2] == 0x02 && p_ntf[1] == 0x00 && icode_detected == 1) {
    LOG_EMVCOHAL_D("> ICODE EOF response do not send to upper layer");
  } else if (p_ntf[0] == 0x61 && p_ntf[1] == 0x06 && icode_detected == 1) {
    LOG_EMVCOHAL_D("> Polling Loop Re-Started");
    icode_detected = 0;
    icode_send_eof = 0;
  } else if (*p_len == 4 && p_ntf[0] == 0x40 && p_ntf[1] == 0x02 &&
             p_ntf[2] == 0x01 && p_ntf[3] == 0x06) {
    LOG_EMVCOHAL_D("> Deinit workaround for LLCP set_config 0x%x 0x%x 0x%x",
                   p_ntf[21], p_ntf[22], p_ntf[23]);
    p_ntf[0] = 0x40;
    p_ntf[1] = 0x02;
    p_ntf[2] = 0x02;
    p_ntf[3] = 0x00;
    p_ntf[4] = 0x00;
    *p_len = 5;
  }
  // 4200 02 00 01
  else if (p_ntf[0] == 0x42 && p_ntf[1] == 0x00 && ee_disc_done == 0x01) {
    LOG_EMVCOHAL_D("Going through workaround - NFCEE_DISCOVER_RSP");
    if (p_ntf[4] == 0x01) {
      p_ntf[4] = 0x00;

      ee_disc_done = 0x00;
    }
    LOG_EMVCOHAL_D("Going through workaround - NFCEE_DISCOVER_RSP - END");

  } else if (*p_len == 4 && p_ntf[0] == 0x4F && p_ntf[1] == 0x11 &&
             p_ntf[2] == 0x01) {
    if (p_ntf[3] == 0x00) {
      LOG_EMVCOHAL_D(
          ">  Workaround for ISO-DEP Presence Check, ignore response and wait "
          "for notification");
      p_ntf[0] = 0x60;
      p_ntf[1] = 0x06;
      p_ntf[2] = 0x03;
      p_ntf[3] = 0x01;
      p_ntf[4] = 0x00;
      p_ntf[5] = 0x01;
      *p_len = 6;
    } else {
      LOG_EMVCOHAL_D(
          ">  Workaround for ISO-DEP Presence Check, presence check return "
          "failed");
      p_ntf[0] = 0x60;
      p_ntf[1] = 0x08;
      p_ntf[2] = 0x02;
      p_ntf[3] = 0xB2;
      p_ntf[4] = 0x00;
      *p_len = 5;
    }
  } else if (*p_len == 4 && p_ntf[0] == 0x6F && p_ntf[1] == 0x11 &&
             p_ntf[2] == 0x01) {
    if (p_ntf[3] == 0x01) {
      LOG_EMVCOHAL_D(
          ">  Workaround for ISO-DEP Presence Check - Card still in field");
      p_ntf[0] = 0x00;
      p_ntf[1] = 0x00;
      p_ntf[2] = 0x01;
      p_ntf[3] = 0x7E;
    } else {
      LOG_EMVCOHAL_D(
          ">  Workaround for ISO-DEP Presence Check - Card not in field");
      p_ntf[0] = 0x60;
      p_ntf[1] = 0x08;
      p_ntf[2] = 0x02;
      p_ntf[3] = 0xB2;
      p_ntf[4] = 0x00;
      *p_len = 5;
    }
  }

  return status;
}

/******************************************************************************
 * Function         process_emvco_init_rsp
 *
 * Description      This function is used to process the HAL NFC core reset rsp
 *                  and ntf and core init rsp of NCI 1.0 or NCI2.0 and update
 *                  NCI version.
 *                  It also handles error response such as core_reset_ntf with
 *                  error status in both NCI2.0 and NCI1.0.
 *
 * Returns          Returns EMVCO_STATUS_SUCCESS if parsing response is
 *successful or returns failure.
 *
 *******************************************************************************/
static EMVCO_STATUS process_emvco_init_rsp(uint8_t *p_ntf, uint16_t *p_len) {
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;

  /* Parsing CORE_RESET_RSP and CORE_RESET_NTF to update NCI version.*/
  if (p_ntf == NULL || *p_len < 2) {
    return EMVCO_STATUS_FAILED;
  }
  if (p_ntf[0] == NCI_MT_RSP &&
      ((p_ntf[1] & NCI_OID_MASK) == NCI_MSG_CORE_RESET)) {
    if (*p_len < 4) {
      __osal_log_error_write(0x534e4554, "169258455");
      return EMVCO_STATUS_FAILED;
    }
    if (p_ntf[2] == 0x01 && p_ntf[3] == 0x00) {
      LOG_EMVCOHAL_D("CORE_RESET_RSP NCI2.0");
      if (nci_hal_ctrl.hal_ext_enabled == TRUE) {
        nci_hal_ctrl.nci_info.wait_for_ntf = TRUE;
      }
    } else if (p_ntf[2] == 0x03 && p_ntf[3] == 0x00) {
      if (*p_len < 5) {
        __osal_log_error_write(0x534e4554, "169258455");
        return EMVCO_STATUS_FAILED;
      }
      LOG_EMVCOHAL_D("CORE_RESET_RSP NCI1.0");
      nci_hal_ctrl.nci_info.nci_version = p_ntf[4];
    }
  } else if (p_ntf[0] == NCI_MT_NTF &&
             ((p_ntf[1] & NCI_OID_MASK) == NCI_MSG_CORE_RESET)) {
    if (*p_len < 4) {
      __osal_log_error_write(0x534e4554, "169258455");
      return EMVCO_STATUS_FAILED;
    }
    if (p_ntf[3] == CORE_RESET_TRIGGER_TYPE_CORE_RESET_CMD_RECEIVED ||
        p_ntf[3] == CORE_RESET_TRIGGER_TYPE_POWERED_ON) {
      if (*p_len < 6) {
        __osal_log_error_write(0x534e4554, "169258455");
        return EMVCO_STATUS_FAILED;
      }
      LOG_EMVCOHAL_D("CORE_RESET_NTF NCI2.0 reason CORE_RESET_CMD received !");
      nci_hal_ctrl.nci_info.nci_version = p_ntf[5];
      LOG_EMVCOHAL_D("nci_version : 0x%02x", nci_hal_ctrl.nci_info.nci_version);
      int len = p_ntf[2] + 2; /*include 2 byte header*/
      if (len != *p_len - 1) {
        LOG_EMVCOHAL_E("process_emvco_init_rsp invalid NTF length");
        __osal_log_error_write(0x534e4554, "121263487");
        return EMVCO_STATUS_FAILED;
      }
      wFwVerRsp = (((uint32_t)p_ntf[len - 2]) << 16U) |
                  (((uint32_t)p_ntf[len - 1]) << 8U) | p_ntf[len];
      LOG_EMVCOHAL_D("NxpNci> FW Version: %x.%x.%x", p_ntf[len - 2],
                     p_ntf[len - 1], p_ntf[len]);
      rom_version = p_ntf[len - 2];
    } else if (p_ntf[3] == CORE_RESET_NTF_MODE_SWITCH_TO_NFC_FORUM ||
               p_ntf[3] == CORE_RESET_NTF_MODE_SWITCH_TO_EMVCO) {
      LOG_EMVCOHAL_D("NFCC MODE SWITCH STATE: %x", p_ntf[3]);
    } else {
      uint32_t i;
      char print_buffer[*p_len * 3 + 1];

      osal_memset(print_buffer, 0, sizeof(print_buffer));
      for (i = 0; i < *p_len; i++) {
        snprintf(&print_buffer[i * 2], 3, "%02X", p_ntf[i]);
      }
      LOG_EMVCOHAL_D("CORE_RESET_NTF received !");
      LOG_EMVCOR_E("len = %3d > %s", *p_len, print_buffer);
      emergency_recovery();
      status = EMVCO_STATUS_FAILED;
    } /* Parsing CORE_INIT_RSP*/
  } else if (p_ntf[0] == NCI_MT_RSP &&
             ((p_ntf[1] & NCI_OID_MASK) == NCI_MSG_CORE_INIT)) {
    if (nci_hal_ctrl.nci_info.nci_version == NCI_VERSION_2_0) {
      LOG_EMVCOHAL_D("CORE_INIT_RSP NCI2.0 received !");
    } else {
      LOG_EMVCOHAL_D("CORE_INIT_RSP NCI1.0 received !");
      if (*p_len < 3) {
        __osal_log_error_write(0x534e4554, "169258455");
        return EMVCO_STATUS_FAILED;
      }
      int len = p_ntf[2] + 2; /*include 2 byte header*/
      if (len != *p_len - 1) {
        LOG_EMVCOHAL_E("process_emvco_init_rsp invalid NTF length");
        __osal_log_error_write(0x534e4554, "121263487");
        return EMVCO_STATUS_FAILED;
      }
      wFwVerRsp = (((uint32_t)p_ntf[len - 2]) << 16U) |
                  (((uint32_t)p_ntf[len - 1]) << 8U) | p_ntf[len];
      if (wFwVerRsp == 0)
        status = EMVCO_STATUS_FAILED;
      LOG_EMVCOHAL_D("NxpNci> FW Version: %x.%x.%x", p_ntf[len - 2],
                     p_ntf[len - 1], p_ntf[len]);
      rom_version = p_ntf[len - 2];
    }
  }
  return status;
}

/******************************************************************************
 * Function         process_ext_cmd_rsp
 *
 * Description      This function process the extension command response. It
 *                  also checks the received response to expected response.
 *
 * Returns          returns EMVCO_STATUS_SUCCESS if response is as expected else
 *                  returns failure.
 *
 ******************************************************************************/
static EMVCO_STATUS process_ext_cmd_rsp(uint16_t cmd_len, uint8_t *p_cmd) {
  EMVCO_STATUS status = EMVCO_STATUS_FAILED;
  uint16_t data_written = 0;

  /* Create the local semaphore */
  if (init_cb_data(&nci_hal_ctrl.ext_cb_data, NULL) != EMVCO_STATUS_SUCCESS) {
    LOG_EMVCOHAL_D("Create ext_cb_data failed");
    return EMVCO_STATUS_FAILED;
  }

  nci_hal_ctrl.ext_cb_data.status = EMVCO_STATUS_SUCCESS;

  /* Send ext command */
  data_written = send_app_data_unlocked(cmd_len, p_cmd);
  if (data_written != cmd_len) {
    LOG_EMVCOHAL_D("send_app_data failed for hal ext");
    goto clean_and_return;
  }

  /* Start timer */
  status = osal_timer_start(timeoutTimerId, HAL_EXTNS_WRITE_RSP_TIMEOUT,
                            &hal_extns_write_rsp_timeout_cb, NULL);
  if (EMVCO_STATUS_SUCCESS == status) {
    LOG_EMVCOHAL_D("Response timer started");
  } else {
    LOG_EMVCOHAL_E("Response timer not started!!!");
    status = EMVCO_STATUS_FAILED;
    goto clean_and_return;
  }

  /* Wait for rsp */
  LOG_EMVCOHAL_D("Waiting after ext cmd sent");
  if (SEM_WAIT(nci_hal_ctrl.ext_cb_data)) {
    LOG_EMVCOHAL_E("p_hal_ext->ext_cb_data.sem semaphore error");
    goto clean_and_return;
  }

  /* Stop Timer */
  status = osal_timer_stop(timeoutTimerId);
  if (EMVCO_STATUS_SUCCESS == status) {
    LOG_EMVCOHAL_D("Response timer stopped");
  } else {
    LOG_EMVCOHAL_E("Response timer stop ERROR!!!");
    status = EMVCO_STATUS_FAILED;
    goto clean_and_return;
  }

  if (cmd_len < 3) {
    __osal_log_error_write(0x534e4554, "153880630");
    status = EMVCO_STATUS_FAILED;
    goto clean_and_return;
  }

  /* No NTF expected for OMAPI command */
  if (p_cmd[0] == 0x2F && p_cmd[1] == 0x1 && p_cmd[2] == 0x01) {
    nci_hal_ctrl.nci_info.wait_for_ntf = FALSE;
  }
  /* Start timer to wait for NTF*/
  if (nci_hal_ctrl.nci_info.wait_for_ntf == TRUE) {
    status = osal_timer_start(timeoutTimerId, HAL_EXTNS_WRITE_RSP_TIMEOUT,
                              &hal_extns_write_rsp_timeout_cb, NULL);
    if (EMVCO_STATUS_SUCCESS == status) {
      LOG_EMVCOHAL_D("Response timer started");
    } else {
      LOG_EMVCOHAL_E("Response timer not started!!!");
      status = EMVCO_STATUS_FAILED;
      goto clean_and_return;
    }
    if (SEM_WAIT(nci_hal_ctrl.ext_cb_data)) {
      LOG_EMVCOHAL_E("p_hal_ext->ext_cb_data.sem semaphore error");
      /* Stop Timer */
      status = osal_timer_stop(timeoutTimerId);
      goto clean_and_return;
    }
    status = osal_timer_stop(timeoutTimerId);
    if (EMVCO_STATUS_SUCCESS == status) {
      LOG_EMVCOHAL_D("Response timer stopped");
    } else {
      LOG_EMVCOHAL_E("Response timer stop ERROR!!!");
      status = EMVCO_STATUS_FAILED;
      goto clean_and_return;
    }
  }

  if (nci_hal_ctrl.ext_cb_data.status != EMVCO_STATUS_SUCCESS &&
      p_cmd[0] != 0x2F && p_cmd[1] != 0x1 && p_cmd[2] == 0x01) {
    LOG_EMVCOHAL_E(
        "Callback Status is failed!! Timer Expired!! Couldn't read it! 0x%x",
        nci_hal_ctrl.ext_cb_data.status);
    status = EMVCO_STATUS_FAILED;
    goto clean_and_return;
  }

  LOG_EMVCOHAL_D("Checking response");
  status = EMVCO_STATUS_SUCCESS;

clean_and_return:
  cleanup_cb_data(&nci_hal_ctrl.ext_cb_data);
  nci_hal_ctrl.nci_info.wait_for_ntf = FALSE;
  return status;
}

EMVCO_STATUS send_app_data_ext(uint16_t *cmd_len, uint8_t *p_cmd_data,
                               uint16_t *rsp_len, uint8_t *p_rsp_data) {
  EMVCO_STATUS status = EMVCO_STATUS_SUCCESS;

  if (p_cmd_data[0] == PROPRIETARY_CMD_FELICA_READER_MODE &&
      p_cmd_data[1] == PROPRIETARY_CMD_FELICA_READER_MODE &&
      p_cmd_data[2] == PROPRIETARY_CMD_FELICA_READER_MODE) {
    LOG_EMVCOHAL_D("Received proprietary command to set Felica Reader mode:%d",
                   p_cmd_data[3]);
    gFelicaReaderMode = p_cmd_data[3];
    /* frame the dummy response */
    *rsp_len = 4;
    p_rsp_data[0] = 0x00;
    p_rsp_data[1] = 0x00;
    p_rsp_data[2] = 0x00;
    p_rsp_data[3] = 0x00;
    status = EMVCO_STATUS_FAILED;
  } else if (p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x02 &&
             p_cmd_data[2] == 0x05 && p_cmd_data[3] == 0x01 &&
             p_cmd_data[4] == 0xA0 && p_cmd_data[5] == 0x44 &&
             p_cmd_data[6] == 0x01 && p_cmd_data[7] == 0x01) {
    nxpprofile_ctrl.profile_type = EMV_CO_PROFILE;
    LOG_EMVCOHAL_D("EMV_CO_PROFILE mode - Enabled");
    status = EMVCO_STATUS_SUCCESS;
  } else if (p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x02 &&
             p_cmd_data[2] == 0x05 && p_cmd_data[3] == 0x01 &&
             p_cmd_data[4] == 0xA0 && p_cmd_data[5] == 0x44 &&
             p_cmd_data[6] == 0x01 && p_cmd_data[7] == 0x00) {
    LOG_EMVCOHAL_D("NFC_FORUM_PROFILE mode - Enabled");
    nxpprofile_ctrl.profile_type = NFC_FORUM_PROFILE;
    status = EMVCO_STATUS_SUCCESS;
  }

  if (nxpprofile_ctrl.profile_type == EMV_CO_PROFILE) {
    if (p_cmd_data[0] == 0x21 && p_cmd_data[1] == 0x06 &&
        p_cmd_data[2] == 0x01 && p_cmd_data[3] == 0x03) {
#if 0
            //Needs clarification whether to keep it or not
            LOG_EMVCOHAL_D ("EmvCo Poll mode - RF Deactivate discard");
            print_packet("SEND", p_cmd_data, *cmd_len);
            *rsp_len = 4;
            p_rsp_data[0] = 0x41;
            p_rsp_data[1] = 0x06;
            p_rsp_data[2] = 0x01;
            p_rsp_data[3] = 0x00;
            print_packet("RECV", p_rsp_data, 4);
            status = EMVCO_STATUS_FAILED;
#endif
    } else if (p_cmd_data[0] == 0x21 && p_cmd_data[1] == 0x03) {
      LOG_EMVCOHAL_D("Commented EmvCo Poll mode workaround - Discover map "
                     "only for A and B");
      /*p_cmd_data[2] = 0x05;
      p_cmd_data[3] = 0x02;
      p_cmd_data[4] = 0x00;
      p_cmd_data[5] = 0x01;
      p_cmd_data[6] = 0x01;
      p_cmd_data[7] = 0x01;
      *cmd_len = 8;*/
    }
  }

  if (p_cmd_data[3] == 0x81 && p_cmd_data[4] == 0x01 && p_cmd_data[5] == 0x03) {
    if (nci_hal_ctrl.nci_info.nci_version != NCI_VERSION_2_0) {
      LOG_EMVCOHAL_D("> Going through workaround - set host list");

      *cmd_len = 8;

      p_cmd_data[2] = 0x05;
      p_cmd_data[6] = 0x02;
      p_cmd_data[7] = 0xC0;

      LOG_EMVCOHAL_D("> Going through workaround - set host list - END");
      status = EMVCO_STATUS_SUCCESS;
    }
  } else if (icode_detected) {
    if ((p_cmd_data[3] & 0x40) == 0x40 &&
        (p_cmd_data[4] == 0x21 || p_cmd_data[4] == 0x22 ||
         p_cmd_data[4] == 0x24 || p_cmd_data[4] == 0x27 ||
         p_cmd_data[4] == 0x28 || p_cmd_data[4] == 0x29 ||
         p_cmd_data[4] == 0x2a)) {
      LOG_EMVCOHAL_D("> Send EOF set");
      icode_send_eof = 1;
    }

    if (p_cmd_data[3] == 0x20 || p_cmd_data[3] == 0x24 ||
        p_cmd_data[3] == 0x60) {
      LOG_EMVCOHAL_D("> NFC ISO_15693 Proprietary CMD ");
      p_cmd_data[3] += 0x02;
    }
  } else if (p_cmd_data[0] == 0x21 && p_cmd_data[1] == 0x03) {
    LOG_EMVCOHAL_D("> Polling Loop Started");
    icode_detected = 0;
    icode_send_eof = 0;
  }
  // 22000100
  else if (p_cmd_data[0] == 0x22 && p_cmd_data[1] == 0x00 &&
           p_cmd_data[2] == 0x01 && p_cmd_data[3] == 0x00) {
    // ee_disc_done = 0x01;//Reader Over SWP event getting
    *rsp_len = 0x05;
    p_rsp_data[0] = 0x42;
    p_rsp_data[1] = 0x00;
    p_rsp_data[2] = 0x02;
    p_rsp_data[3] = 0x00;
    p_rsp_data[4] = 0x00;
    print_packet("RECV", p_rsp_data, 5);
    status = EMVCO_STATUS_FAILED;
  }
  // 2002 0904 3000 3100 3200 5000
  else if ((p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x02) &&
           ((p_cmd_data[2] == 0x09 && p_cmd_data[3] == 0x04) /*||
            (p_cmd_data[2] == 0x0D && p_cmd_data[3] == 0x04)*/
            )) {
    *cmd_len += 0x01;
    p_cmd_data[2] += 0x01;
    p_cmd_data[9] = 0x01;
    p_cmd_data[10] = 0x40;
    p_cmd_data[11] = 0x50;
    p_cmd_data[12] = 0x00;

    LOG_EMVCOHAL_D("> Going through workaround - Dirty Set Config ");
    //        print_packet("SEND", p_cmd_data, *cmd_len);
    LOG_EMVCOHAL_D("> Going through workaround - Dirty Set Config - End ");
  }
  //    20020703300031003200
  //    2002 0301 3200
  else if ((p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x02) &&
           ((p_cmd_data[2] == 0x07 && p_cmd_data[3] == 0x03) ||
            (p_cmd_data[2] == 0x03 && p_cmd_data[3] == 0x01 &&
             p_cmd_data[4] == 0x32))) {
    LOG_EMVCOHAL_D("> Going through workaround - Dirty Set Config ");
    print_packet("SEND", p_cmd_data, *cmd_len);
    *rsp_len = 5;
    p_rsp_data[0] = 0x40;
    p_rsp_data[1] = 0x02;
    p_rsp_data[2] = 0x02;
    p_rsp_data[3] = 0x00;
    p_rsp_data[4] = 0x00;

    print_packet("RECV", p_rsp_data, 5);
    status = EMVCO_STATUS_FAILED;
    LOG_EMVCOHAL_D("> Going through workaround - Dirty Set Config - End ");
  }

  // 2002 0D04 300104 310100 320100 500100
  // 2002 0401 320100
  else if ((p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x02) &&
           (
               /*(p_cmd_data[2] == 0x0D && p_cmd_data[3] == 0x04)*/
               (p_cmd_data[2] == 0x04 && p_cmd_data[3] == 0x01 &&
                p_cmd_data[4] == 0x32 && p_cmd_data[5] == 0x00))) {
    //        p_cmd_data[12] = 0x40;

    LOG_EMVCOHAL_D("> Going through workaround - Dirty Set Config ");
    print_packet("SEND", p_cmd_data, *cmd_len);
    p_cmd_data[6] = 0x60;

    print_packet("RECV", p_rsp_data, 5);
    //        status = EMVCO_STATUS_FAILED;
    LOG_EMVCOHAL_D("> Going through workaround - Dirty Set Config - End ");
  } else if (*cmd_len <= (NCI_MAX_DATA_LEN - 3) && p_cmd_data[0] == 0x21 &&
             p_cmd_data[1] == 0x00) {
    LOG_EMVCOHAL_D(
        "> Going through workaround - Add Mifare Classic in Discovery Map");
    p_cmd_data[*cmd_len] = 0x80;
    p_cmd_data[*cmd_len + 1] = 0x01;
    p_cmd_data[*cmd_len + 2] = 0x80;
    p_cmd_data[5] = 0x01;
    p_cmd_data[6] = 0x01;
    p_cmd_data[2] += 3;
    p_cmd_data[3] += 1;
    *cmd_len += 3;
  } else if (*cmd_len == 3 && p_cmd_data[0] == 0x00 && p_cmd_data[1] == 0x00 &&
             p_cmd_data[2] == 0x00) {
    LOG_EMVCOHAL_D("> Going through workaround - ISO-DEP Presence Check ");
    p_cmd_data[0] = 0x2F;
    p_cmd_data[1] = 0x11;
    p_cmd_data[2] = 0x00;
    status = EMVCO_STATUS_SUCCESS;
    LOG_EMVCOHAL_D("> Going through workaround - ISO-DEP Presence Check - End");
  }
#if 0
    else if ( (p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x02 ) &&
                 ((p_cmd_data[2] == 0x09 && p_cmd_data[3] == 0x04) ||
                     (p_cmd_data[2] == 0x0B && p_cmd_data[3] == 0x05) ||
                     (p_cmd_data[2] == 0x07 && p_cmd_data[3] == 0x02) ||
                     (p_cmd_data[2] == 0x0A && p_cmd_data[3] == 0x03) ||
                     (p_cmd_data[2] == 0x0A && p_cmd_data[3] == 0x04) ||
                     (p_cmd_data[2] == 0x05 && p_cmd_data[3] == 0x02))
             )
    {
        LOG_EMVCOHAL_D ("> Going through workaround - Dirty Set Config ");
        print_packet("SEND", p_cmd_data, *cmd_len);
        *rsp_len = 5;
        p_rsp_data[0] = 0x40;
        p_rsp_data[1] = 0x02;
        p_rsp_data[2] = 0x02;
        p_rsp_data[3] = 0x00;
        p_rsp_data[4] = 0x00;

        print_packet("RECV", p_rsp_data, 5);
        status = EMVCO_STATUS_FAILED;
        LOG_EMVCOHAL_D ("> Going through workaround - Dirty Set Config - End ");
    }

    else if((p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x02) &&
           ((p_cmd_data[3] == 0x00) ||
           ((*cmd_len >= 0x06) && (p_cmd_data[5] == 0x00)))) /*If the length of the first param id is zero don't allow*/
    {
        LOG_EMVCOHAL_D ("> Going through workaround - Dirty Set Config ");
        print_packet("SEND", p_cmd_data, *cmd_len);
        *rsp_len = 5;
        p_rsp_data[0] = 0x40;
        p_rsp_data[1] = 0x02;
        p_rsp_data[2] = 0x02;
        p_rsp_data[3] = 0x00;
        p_rsp_data[4] = 0x00;

        print_packet("RECV", p_rsp_data, 5);
        status = EMVCO_STATUS_FAILED;
        LOG_EMVCOHAL_D ("> Going through workaround - Dirty Set Config - End ");
    }
#endif
  else if ((wFwVerRsp & 0x0000FFFF) == wFwVer) {
    /* skip CORE_RESET and CORE_INIT from Brcm */
    if (p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x00 &&
        p_cmd_data[2] == 0x01 && p_cmd_data[3] == 0x01) {
      //            *rsp_len = 6;
      //
      //            LOG_EMVCOHAL_D("> Going - core reset optimization");
      //
      //            p_rsp_data[0] = 0x40;
      //            p_rsp_data[1] = 0x00;
      //            p_rsp_data[2] = 0x03;
      //            p_rsp_data[3] = 0x00;
      //            p_rsp_data[4] = 0x10;
      //            p_rsp_data[5] = 0x01;
      //
      //            status = EMVCO_STATUS_FAILED;
      //            LOG_EMVCOHAL_D("> Going - core reset optimization - END");
    }
    /* CORE_INIT */
    else if (p_cmd_data[0] == 0x20 && p_cmd_data[1] == 0x01 &&
             p_cmd_data[2] == 0x00) {
    }
  }

  return status;
}

/******************************************************************************
 * Function         send_ext_cmd
 *
 * Description      This function send the extension command to NFCC. No
 *                  response is checked by this function but it waits for
 *                  the response to come.
 *
 * Returns          Returns EMVCO_STATUS_SUCCESS if sending cmd is successful
 *and response is received.
 *
 ******************************************************************************/
EMVCO_STATUS send_ext_cmd(uint16_t cmd_len, uint8_t *p_cmd) {
  EMVCO_STATUS status = EMVCO_STATUS_FAILED;
  HAL_ENABLE_EXT();
  nci_hal_ctrl.cmd_len = cmd_len;
  memcpy(nci_hal_ctrl.p_cmd_data, p_cmd, cmd_len);
  status = process_ext_cmd_rsp(nci_hal_ctrl.cmd_len, nci_hal_ctrl.p_cmd_data);
  HAL_DISABLE_EXT();

  return status;
}

/******************************************************************************
 * Function         hal_extns_write_rsp_timeout_cb
 *
 * Description      Timer call back function
 *
 * Returns          None
 *
 ******************************************************************************/
static void hal_extns_write_rsp_timeout_cb(uint32_t timerId, void *p_context) {
  UNUSED(timerId);
  UNUSED(p_context);
  LOG_EMVCOHAL_D("hal_extns_write_rsp_timeout_cb - write timeout!!!");
  nci_hal_ctrl.ext_cb_data.status = EMVCO_STATUS_FAILED;
  usleep(1);
  osal_sem_post(&(nci_hal_ctrl.sync_nci_write));
  SEM_POST(&(nci_hal_ctrl.ext_cb_data));

  return;
}

void static send_poll_event_to_upper_layer() {
  LOG_EMVCOHAL_D("EMVCO_POLLING_STARTED_MSG");
  nci_hal_ctrl.frag_rsp.data_pos = 0;
  RESET_CHAINED_DATA();

  modeSwitchArgs->current_discovery_mode = EMVCO;
  lib_emvco_message_t msg;
  msg.e_msgType = EMVCO_POLLING_STARTED_MSG;
  msg.p_msg_data = NULL;
  memset(msg.data, 0, sizeof(msg.data));
  msg.size = 0;
  msg.w_status = EMVCO_STATUS_SUCCESS;
  tml_deferred_call(gptml_emvco_context->dw_callback_thread_id, &msg);
}

EMVCO_STATUS process_emvco_mode_rsp(osal_transact_info_t *pTransactionInfo) {
  LOG_EMVCOHAL_D("process_emvco_mode_rsp");
  uint8_t *p_ntf = pTransactionInfo->p_buff;
  uint16_t p_len = pTransactionInfo->w_length;
  if (!modeSwitchArgs->is_start_emvco) {
    return EMVCO_STATUS_FAILED;
  }
  LOG_EMVCOHAL_D("process_emvco_mode_rsp data_len:%d "
                 "modeSwitchArgs->is_start_emvco:%d",
                 p_len, modeSwitchArgs->is_start_emvco);
  uint8_t msg_type, pbf, group_id, op_code, *p_data;

  p_data = p_ntf;
  NCI_MSG_PRS_HDR0(p_data, msg_type, pbf, group_id);
  LOG_EMVCOHAL_D("process_emvco_mode_rsp msg_type:%d group_id:%d ", msg_type,
                 group_id);

  NCI_MSG_PRS_HDR1(p_data, op_code);
  LOG_EMVCOHAL_D("process_emvco_mode_rsp op_code:%d", op_code);
  p_data = p_ntf;

  ct_process_emvco_mode_rsp_impl(pTransactionInfo);

  switch (msg_type) {
  case NCI_MSG_TYPE_RSP:
    switch (group_id) {
    case NCI_GID_PROP:
      switch (op_code) {
      case MSG_CORE_PROPRIETARY_RSP: {
        if (p_len == MSG_CORE_PROPRIETARY_STANDBY_RSP) {
          uint8_t num_params;
          tEMVCO_DISCOVER_PARAMS disc_params[MAX_DISC_PARAMS];
          num_params = get_rf_discover_config(modeSwitchArgs->emvco_config,
                                              disc_params, MAX_DISC_PARAMS);
          LOG_EMVCOHAL_D("RFDiscover num_params:%d", num_params);
          send_discover_cmd(num_params, disc_params);
        }
        break;
      }
      }
      break;
    case NCI_GID_RF_MANAGE:
      switch (op_code) {
      case RF_DEACTIVATE_NTF: {
        /** Send poll event to upper layer only, if deactivate with discover mode
            received */
        if ((p_data[2] == NCI_DEACTIVATE_NTF_LEN) &&
            (NCI_DEACT_TYPE_DISCOVERY == p_data[3])) {
          send_poll_event_to_upper_layer();
        }
      } break;
      case MSG_RF_DISCOVER_RSP: {
        send_poll_event_to_upper_layer();
      } break;
      }
    }
    break;
  }
  return EMVCO_STATUS_SUCCESS;
}