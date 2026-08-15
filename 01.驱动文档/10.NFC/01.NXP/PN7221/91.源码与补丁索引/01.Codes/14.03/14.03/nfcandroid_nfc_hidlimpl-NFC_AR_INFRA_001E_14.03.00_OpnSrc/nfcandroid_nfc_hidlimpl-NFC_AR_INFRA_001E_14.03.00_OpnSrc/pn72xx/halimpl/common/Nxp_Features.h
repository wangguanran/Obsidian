/******************************************************************************
 *
 *  Copyright 2018-2024 NXP
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
#include <stdint.h>
#else
#include <unistd.h>
#endif
#include <string>
#ifndef NXP_FEATURES_H
#define NXP_FEATURES_H

#define STRMAX_2 100
#define FW_MOBILE_MAJOR_NUMBER_SN100U 0x010
#define FW_MOBILE_MAJOR_NUMBER_SN220U 0x02
#define FW_MOBILE_MAJOR_NUMBER_PN7220 0x00
#define FW_MOBILE_MAJOR_NUMBER_PN7160 0x50

#define EMVCO_MODE_SWITCH 0x01
#define NFC_MODE_SWITCH 0x02
#define SMCU_FW_DNLD_MODE_SWITCH 0x03

/*Including T4T NFCEE by incrementing 1*/
#define NFA_EE_MAX_EE_SUPPORTED 5

#ifndef FW_LIB_ROOT_DIR
#if (defined(__arm64__) || defined(__aarch64__) || defined(_M_ARM64))
#define FW_LIB_ROOT_DIR "/vendor/lib64/"
#else
#define FW_LIB_ROOT_DIR "/vendor/lib/"
#endif
#endif
#ifndef FW_BIN_ROOT_DIR
#define FW_BIN_ROOT_DIR "/vendor/firmware/"
#endif
#ifndef FW_LIB_EXTENSION
#define FW_LIB_EXTENSION ".so"
#endif
#ifndef FW_BIN_EXTENSION
#define FW_BIN_EXTENSION ".bin"
#endif
using namespace std;
typedef enum {
  NFCC_DWNLD_WITH_VEN_RESET,
  NFCC_DWNLD_WITH_NCI_CMD
} tNFCC_DnldType;

typedef enum {
  DEFAULT_CHIP_TYPE = 0x00,
  pn7160,
  sn100u,
  sn220u,
  pn7220,
  pn7221
} tNFC_chipType;

typedef struct {
  /*Flags common to all chip types*/
  uint8_t _NXP_NFCC_EMPTY_DATA_PACKET : 1;
  uint8_t _GEMALTO_SE_SUPPORT : 1;
  uint8_t _NFCC_I2C_READ_WRITE_IMPROVEMENT : 1;
  uint8_t _NFCC_MIFARE_TIANJIN : 1;
  uint8_t _NFCC_MW_RCVRY_BLK_FW_DNLD : 1;
  uint8_t _NFC_NXP_STAT_DUAL_UICC_EXT_SWITCH : 1;
  uint8_t _NFC_NXP_STAT_DUAL_UICC_WO_EXT_SWITCH : 1;
  uint8_t _NFCC_FW_WA : 1;
  uint8_t _NFCC_FORCE_NCI1_0_INIT : 1;
  uint8_t _NFCC_ROUTING_BLOCK_BIT : 1;
  uint8_t _NFCC_SPI_FW_DOWNLOAD_SYNC : 1;
  uint8_t _HW_ANTENNA_LOOP4_SELF_TEST : 1;
  uint8_t _NFCEE_REMOVED_NTF_RECOVERY : 1;
  uint8_t _NFCC_FORCE_FW_DOWNLOAD : 1;
  uint8_t _UICC_CREATE_CONNECTIVITY_PIPE : 1;
  uint8_t _NFCC_AID_MATCHING_PLATFORM_CONFIG : 1;
  uint8_t _NFCC_ROUTING_BLOCK_BIT_PROP : 1;
  uint8_t _NXP_NFC_UICC_ETSI12 : 1;
  uint8_t _NFA_EE_MAX_EE_SUPPORTED : 3;
  uint8_t _NFCC_DWNLD_MODE : 1;
} tNfc_nfccFeatureList;

typedef struct {
  uint8_t _NFCC_RESET_RSP_LEN;
} tNfc_platformFeatureList;

typedef struct {
  uint8_t _NCI_INTERFACE_UICC_DIRECT;
  uint8_t _NCI_INTERFACE_ESE_DIRECT;
  uint8_t _NCI_PWR_LINK_PARAM_CMD_SIZE;
  uint8_t _NCI_EE_PWR_LINK_ALWAYS_ON;
  uint8_t _NFA_EE_MAX_AID_ENTRIES;
  uint8_t _NFC_NXP_AID_MAX_SIZE_DYN : 1;
} tNfc_nfcMwFeatureList;

typedef struct {
  tNFC_chipType chipType;
  std::string _FW_LIB_PATH;
  std::string _PLATFORM_LIB_PATH;
  std::string _PKU_LIB_PATH;
  std::string _FW_BIN_PATH;
  uint16_t _PHDNLDNFC_USERDATA_EEPROM_OFFSET;
  uint16_t _PHDNLDNFC_USERDATA_EEPROM_LEN;
  uint8_t _FW_MOBILE_MAJOR_NUMBER;
  tNfc_nfccFeatureList nfccFL;
  tNfc_platformFeatureList platformFL;
  tNfc_nfcMwFeatureList nfcMwFL;
} tNfc_featureList;

extern tNfc_featureList nfcFL;


#define CONFIGURE_FEATURELIST(chipType)                                         \
  {                                                                             \
    nfcFL.chipType = chipType;                                                  \
    if ((chipType == sn100u) || (chipType == sn220u)) {                         \
      CONFIGURE_FEATURELIST_NFCC_WITH_ESE(chipType)                             \
    } else {                                                                    \
      CONFIGURE_FEATURELIST_NFCC(chipType)                                      \
    }                                                                           \
  }

#define CONFIGURE_FEATURELIST_NFCC_WITH_ESE(chipType)                           \
  {                                                                             \
      CONFIGURE_FEATURELIST_NFCC(chipType)                                      \
  }

#define CONFIGURE_FEATURELIST_NFCC(chipType)                                    \
  {                                                                             \
    nfcFL.nfccFL._NXP_NFC_UICC_ETSI12 = false;                                  \
    nfcFL.nfccFL._NFCC_SPI_FW_DOWNLOAD_SYNC = false;                            \
                                                                                \
    nfcFL.platformFL._NFCC_RESET_RSP_LEN = 0;                                   \
                                                                                \
    nfcFL.nfcMwFL._NCI_INTERFACE_UICC_DIRECT = 0x00;                            \
    nfcFL.nfcMwFL._NCI_INTERFACE_ESE_DIRECT = 0x00;                             \
    nfcFL.nfcMwFL._NCI_PWR_LINK_PARAM_CMD_SIZE = 0x02;                          \
    nfcFL.nfcMwFL._NCI_EE_PWR_LINK_ALWAYS_ON = 0x01;                            \
    nfcFL._PHDNLDNFC_USERDATA_EEPROM_OFFSET = 0x023CU;                          \
    nfcFL._PHDNLDNFC_USERDATA_EEPROM_LEN = 0x0C80U;                             \
    nfcFL._FW_MOBILE_MAJOR_NUMBER = FW_MOBILE_MAJOR_NUMBER_PN7220;              \
    nfcFL.nfccFL._NFCC_DWNLD_MODE = NFCC_DWNLD_WITH_VEN_RESET;                  \
    switch (chipType) {                                                         \
      case pn7220:                                                              \
      case pn7221:                                                              \
        nfcFL.nfccFL._NFCC_I2C_READ_WRITE_IMPROVEMENT = true;                   \
        nfcFL.nfccFL._NFCC_MIFARE_TIANJIN = false;                              \
        nfcFL.nfccFL._NFCC_MW_RCVRY_BLK_FW_DNLD = true;                         \
        nfcFL.nfccFL._NFC_NXP_STAT_DUAL_UICC_EXT_SWITCH = false;                \
        nfcFL.nfccFL._NFC_NXP_STAT_DUAL_UICC_WO_EXT_SWITCH = true;              \
        nfcFL.nfccFL._NFCC_FW_WA = true;                                        \
        nfcFL.nfccFL._NFCC_FORCE_NCI1_0_INIT = false;                           \
        nfcFL.nfccFL._NFCC_SPI_FW_DOWNLOAD_SYNC = true;                         \
        nfcFL.nfccFL._HW_ANTENNA_LOOP4_SELF_TEST = false;                       \
        nfcFL.nfccFL._NFCEE_REMOVED_NTF_RECOVERY = true;                        \
        nfcFL.nfccFL._NFCC_FORCE_FW_DOWNLOAD = true;                            \
        nfcFL.nfccFL._UICC_CREATE_CONNECTIVITY_PIPE = true;                     \
        nfcFL.nfccFL._NXP_NFC_UICC_ETSI12 = false;                              \
        nfcFL.nfccFL._NFA_EE_MAX_EE_SUPPORTED = 3;                              \
        nfcFL.platformFL._NFCC_RESET_RSP_LEN = 0x10U;                           \
        nfcFL.nfcMwFL._NCI_INTERFACE_UICC_DIRECT = 0x82;                        \
        nfcFL.nfcMwFL._NCI_INTERFACE_ESE_DIRECT = 0x83;                         \
        SRTCPY_FW("libpn72xx_fw", "libpn72xx_fw_platform", "libpn72xx_fw_pku")  \
        STRCPY_FW_BIN("pn72xx")                                                 \
        break;                                                                  \
      case pn7160:                                                              \
        nfcFL._PHDNLDNFC_USERDATA_EEPROM_OFFSET = 0x023CU;                      \
        nfcFL._PHDNLDNFC_USERDATA_EEPROM_LEN = 0x0C80U;                         \
        STRCPY_FW("libpn7160_fw")                                               \
        STRCPY_FW_BIN("pn7160")                                                 \
        nfcFL._FW_MOBILE_MAJOR_NUMBER = FW_MOBILE_MAJOR_NUMBER_PN7160;          \
        nfcFL.nfccFL._NFCC_I2C_READ_WRITE_IMPROVEMENT = true;                   \
        break;                                                                  \
      default:                                                                  \
        nfcFL.nfccFL._NFCC_FORCE_FW_DOWNLOAD = true;                            \
        break;                                                                  \
      }                                                                         \
  }
#define STRCPY_FW_BIN(str)                                                      \
  {                                                                             \
    nfcFL._FW_BIN_PATH.clear();                                                 \
    nfcFL._FW_BIN_PATH.append(FW_BIN_ROOT_DIR);                                 \
    nfcFL._FW_BIN_PATH.append(str);                                             \
    nfcFL._FW_BIN_PATH.append(FW_BIN_EXTENSION);                                \
  }
#define SRTCPY_FW(str1, str2, str3)                                             \
  {                                                                             \
    nfcFL._FW_LIB_PATH.clear();                                                 \
    nfcFL._FW_LIB_PATH.append(FW_LIB_ROOT_DIR);                                 \
    nfcFL._FW_LIB_PATH.append(str1);                                            \
    nfcFL._FW_LIB_PATH.append(FW_LIB_EXTENSION);                                \
    nfcFL._PLATFORM_LIB_PATH.clear();                                           \
    nfcFL._PLATFORM_LIB_PATH.append(FW_LIB_ROOT_DIR);                           \
    nfcFL._PLATFORM_LIB_PATH.append(str2);                                      \
    nfcFL._PLATFORM_LIB_PATH.append(FW_LIB_EXTENSION);                          \
    nfcFL._PKU_LIB_PATH.clear();                                                \
    nfcFL._PKU_LIB_PATH.append(FW_LIB_ROOT_DIR);                                \
    nfcFL._PKU_LIB_PATH.append(str3);                                           \
    nfcFL._PKU_LIB_PATH.append(FW_LIB_EXTENSION);                               \
  }
#define STRCPY_FW(str1) {                                                       \
  nfcFL._FW_LIB_PATH.clear();                                                   \
  nfcFL._FW_LIB_PATH.append(FW_LIB_ROOT_DIR);                                   \
  nfcFL._FW_LIB_PATH.append(str1);                                              \
  nfcFL._FW_LIB_PATH.append(FW_LIB_EXTENSION);                                  \
}
#endif
