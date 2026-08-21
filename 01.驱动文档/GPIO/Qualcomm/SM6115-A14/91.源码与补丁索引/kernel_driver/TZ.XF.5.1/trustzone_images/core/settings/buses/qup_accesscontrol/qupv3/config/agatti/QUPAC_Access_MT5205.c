//===========================================================================
//
// FILE:         QUPAC_Access.xml
//
// DESCRIPTION:  This file lists access permission for all QUPS
//
//===========================================================================
//
//                             Edit History
//
// $Header: //components/rel/core.tz/2.1/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access.c#6 $
//
// when       who     what, where, why
// 10/09/24   ABH     Updated settings for 2W SKU2
// 03/14/23   PCR     Updated settings for 2W 
// 03/02/23   RK      Added AC settings for RB1-V2
// 03/17/22   RK      Added AC settings for Genoa
// 11/22/21   RK      Added AC settings for RB1
// 08/01/19   PCR     Created
//
//===========================================================================
//             Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
//             All rights reserved.
//             Confidential and Proprietary - Qualcomm Technologies, Inc.
//===========================================================================
#include "QupACCommonIds.h"
#include "odm_features.h"

/* OEMs are expected to modify this .c to suit their board design. The uAC 
   specifies the owners of the SE resource. It is initially populated
   according to System IO GPIO allocation */

//All SEs have to be listed below. Any SE not present cannot be accessed by any subsystem. 
//It's designed to be flexible enough to list only available SEs on a particular platform.

const QUPv3_se_security_permissions_type qupv3_perms_default[] =
{
  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  TRUE  }, // NFC eSE
/* <!--! modify this to set gpio69 70 as 4 wire uart start */
#if defined(ODM_PROJECT_MT5205)
  { QUPV3_0_SE1, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // RS232 UART GPIO69/70
/* modify this to set gpio69 70 as 4 wire uart stop --> */
#else
  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
#endif
  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
#if defined(ODM_PROJECT_MT5205)
  { QUPV3_0_SE5, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // MDB UART GPIO16/17
#else
  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_TZ,          FALSE,      TRUE,  TRUE  }, // Fingerprint
#endif
};

const uint32 qupv3_perms_size_default = sizeof(qupv3_perms_default)/sizeof(qupv3_perms_default[0]);

const QUPv3_se_security_permissions_type ssc_qupv3_perms_default[] =
{
  /*PeriphID,      ProtocolID,             Mode,           NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_SSC_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  { QUPV3_SSC_SE1, QUPV3_PROTOCOL_I3C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  /*QUPV3_SSC_SE2*/
  /*QUPV3_SSC_SE3*/
  /*QUPV3_SSC_SE4*/
  { QUPV3_SSC_SE5, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  { QUPV3_SSC_SE6, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  /*QUPV3_SSC_SE7*/
};
const uint32 ssc_qupv3_perms_size_default = sizeof(ssc_qupv3_perms_default)/sizeof(ssc_qupv3_perms_default[0]);

const QUPv3_se_security_permissions_type qupv3_perms_rumi[] =
{
  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_FIFO,  AC_TZ,          TRUE,       TRUE,  TRUE  }, // NFC eSE
  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
  /*QUPV3_0_SE2*/
  /*QUPV3_0_SE3*/
  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        FALSE,      FALSE, FALSE }, // Debug UART
  /*QUPV3_0_SE5*/
};
const uint32 qupv3_perms_size_rumi = sizeof(qupv3_perms_rumi)/sizeof(qupv3_perms_rumi[0]);

const QUPv3_se_security_permissions_type qupv3_perms_default_QRB[] =
{
  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_0_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // NFC eSE
  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Fingerprint
};

const uint32 qupv3_perms_size_default_QRB = sizeof(qupv3_perms_default_QRB)/sizeof(qupv3_perms_default_QRB[0]);

const QUPv3_se_security_permissions_type qupv3_perms_default_QRB_V2[] =
{
  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_0_SE0, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // Diag UART
  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Fingerprint
};

const uint32 qupv3_perms_size_default_QRB_V2 = sizeof(qupv3_perms_default_QRB_V2)/sizeof(qupv3_perms_default_QRB_V2[0]);

const QUPv3_se_security_permissions_type qupv3_perms_default_Genoa[] =
{
  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_0_SE0, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // Diag UART
  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_TZ,          FALSE,      TRUE,  TRUE  }, // Fingerprint
};

const uint32 qupv3_perms_size_default_Genoa = sizeof(qupv3_perms_default_Genoa)/sizeof(qupv3_perms_default_Genoa[0]);

const QUPv3_se_security_permissions_type qupv3_perms_2W[] =
{
  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_TZ,          FALSE,      TRUE,  TRUE  }, // NFC eSE
  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  TRUE  }, // Fingerprint
};

const uint32 qupv3_perms_size_2W = sizeof(qupv3_perms_2W)/sizeof(qupv3_perms_2W[0]);

const QUPv3_se_security_permissions_type ssc_qupv3_perms_2W[] =
{
  /*PeriphID,      ProtocolID,             Mode,           NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_SSC_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  { QUPV3_SSC_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  /*QUPV3_SSC_SE2*/
  /*QUPV3_SSC_SE3*/
  /*QUPV3_SSC_SE4*/
  { QUPV3_SSC_SE5, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  { QUPV3_SSC_SE6, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  /*QUPV3_SSC_SE7*/
};

const uint32 ssc_qupv3_perms_size_2W = sizeof(ssc_qupv3_perms_2W)/sizeof(ssc_qupv3_perms_2W[0]);

const QUPv3_se_security_permissions_type qupv3_perms_2W_SKU2[] =
{
  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // LVDS & CODEC
  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // PMIC & CODEC
  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
  { QUPV3_0_SE5, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // 4Wire UART
};

const uint32 qupv3_perms_size_2W_SKU2 = sizeof(qupv3_perms_2W_SKU2)/sizeof(qupv3_perms_2W_SKU2[0]);

const QUPv3_se_security_permissions_type ssc_qupv3_perms_2W_SKU2[] =
{
  /*PeriphID,      ProtocolID,             Mode,           NsOwner,        bAllowFifo, bLoad, bModExcl  */
  { QUPV3_SSC_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  { QUPV3_SSC_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  /*QUPV3_SSC_SE2*/
  /*QUPV3_SSC_SE3*/
  /*QUPV3_SSC_SE4*/
  { QUPV3_SSC_SE5, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  { QUPV3_SSC_SE6, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
  /*QUPV3_SSC_SE7*/
};

const uint32 ssc_qupv3_perms_size_2W_SKU2 = sizeof(ssc_qupv3_perms_2W_SKU2)/sizeof(ssc_qupv3_perms_2W_SKU2[0]);

const QUPv3_gpii_security_permissions_type qupv3_gpii_perms[] =
{
  { QUPV3_0_GPII0,  AC_HLOS, AC_HLOS_GSI },
  { QUPV3_0_GPII1,  AC_HLOS, AC_HLOS_GSI },
  { QUPV3_0_GPII2,  AC_HLOS, AC_HLOS_GSI },
  { QUPV3_0_GPII3,  AC_HLOS, AC_HLOS_GSI },
  { QUPV3_0_GPII4,  AC_TZ },
  { QUPV3_0_GPII5,  AC_TZ },
  { QUPV3_0_GPII6,  AC_TZ },
  { QUPV3_0_GPII7,  AC_ADSP_Q6_ELF },
  { QUPV3_0_GPII8,  AC_ADSP_Q6_ELF },
  { QUPV3_0_GPII9,  AC_MSS_MSA },  
};
const uint32 qupv3_gpii_perms_size = sizeof(qupv3_gpii_perms)/sizeof(qupv3_gpii_perms[0]);

const QUPv3_gpii_security_permissions_type ssc_qupv3_gpii_perms[] =
{
  { QUPV3_SSC_GPII0,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII1,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII2,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII3,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII4,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII5,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII6,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII7,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII8,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII9,  AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII10, AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII11, AC_ADSP_Q6_ELF },
  { QUPV3_SSC_GPII12, AC_ADSP_Q6_ELF }, 
  { QUPV3_SSC_GPII13, AC_ADSP_Q6_ELF }, 
  { QUPV3_SSC_GPII14, AC_ADSP_Q6_ELF }, 
  { QUPV3_SSC_GPII15, AC_ADSP_Q6_ELF }, 
};
const uint32 ssc_qupv3_gpii_perms_size = sizeof(ssc_qupv3_gpii_perms)/sizeof(ssc_qupv3_gpii_perms[0]);

