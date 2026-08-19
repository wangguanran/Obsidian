/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

                  PMIC Startup Services

GENERAL DESCRIPTION
  This file contains initialization functions and corresponding variable
  declarations to support interaction with the Qualcomm Pmic chips.

  Copyright (c) 2013-2021,2024 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*/

/*===========================================================================

                        EDIT HISTORY FOR MODULE

  This section contains comments describing changes made to the module.
  Notice that changes are listed in reverse chronological order.


when       who     what, where, why
--------   ---     ----------------------------------------------------------
09/04/18   sb      Added API for target specific cleanup
04/12/17   va      Enable FG,schg protocol
07/07/16   cm      Remove Gold cluster enable.
06/01/16   sm      Removed EFI_QCOM_CHARGER_EX_PROTOCOL related changes
05/10/16   al      Adding PON protocol 
05/06/16   sm      Added Pmic Charger Protocol to install 
04/09/16   al      Enable Gold cluster
03/30/16   va      enable protocol 
03/09/16   va      Moved common protocol install to pmic.c
03/08/16   al      Moving NPA to common
07/23/15   al      Adding NPA
07/21/14   va      Adding Fuel Gauge Protocol
07/03/14   al      Adding MIPI-BIF, RGB. Removing SW workaround since it doesn't solve issue
06/06/14   al      Adding workaround for HW issue 
04/28/14   va      Expose Npa Test protocol
05/09/14   al      Adding IBB and LAB 
04/18/14   al      Added SMBCHG 
11/22/13   va      PmicLib Dec Addition
10/02/13   al      New File
===========================================================================*/
/*===========================================================================

                    INCLUDE FILES FOR MODULE

===========================================================================*/
#include "com_dtypes.h"
#include "pm_uefi.h"
#include <Library/UefiBootServicesTableLib.h>

/**
  PMIC PROTOCOL interface
*/
#include "Protocol/EFIPmicSchg.h"
#include "Protocol/EFIPmicFg.h"
#include "Protocol/EFIPmicVreg.h"
#include "Protocol/EFIPmicPwrOn.h"
#include "Protocol/EFIPmicGpio.h"
#include "Protocol/EFIPlatformInfo.h"

#include "pm_comm.h"
#include "CoreVerify.h"

#include "pmapp_npa.h"
#include "npa_resource.h"
#include "pm_sdam.h"
#include "pm_log_utils.h"
#include "pm_version.h"


#include "i2c_api.h"
#include <Protocol/EFITlmm.h>

/*===========================================================================

                LOCAL CONSTANT AND MACRO DEFINITIONS

===========================================================================*/

#define PON_INT_MID_SEL_REG         0x81A
#define PMICA_SLAVE_ID_0            0x0
#define INT_MID_0_VAL               0x0
#define SPMI_PRIORITY               0x0
        
#define SDAM_MEM_013_ADDR           13
#define GPIO_GROUNDED               0x0
#define SINGLE_BYTE                 1
        
#define TIMEOUT_MAX                 2500

#define LEICA_I2C_INSTANCE          I2C_INSTANCE_002
#define LEICA_1_RESET_GPIO          EFI_PM_GPIO_3
#define LEICA_2_RESET_GPIO          EFI_PM_GPIO_4

/*===========================================================================

                VARIABLES DEFINITIONS

===========================================================================*/
extern EFI_QCOM_PMIC_SCHG_PROTOCOL        PmicSchgProtocolImplementation;
extern EFI_QCOM_PMIC_FG_BASIC_PROTOCOL    PmicFgBasicProtocolImplementation;
extern EFI_QCOM_PMIC_PON_PROTOCOL         PmicPonProtocolImplementation;
extern EFI_QCOM_PMIC_SCHG_PROTOCOL        PmicSchg_P_ProtocolImplementation;


/*===========================================================================

                LOCAL FUNCTION PROTOTYPES

===========================================================================*/
static pm_err_flag_type pm_i2c_sid_config(void);

/*===========================================================================

                EXTERNAL FUNCTION PROTOTYPES

===========================================================================*/

pm_err_flag_type pm_install_target_protocols(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

  pm_err_flag_type err_flag = PM_ERR_FLAG_SUCCESS;
  EFI_STATUS  Status = EFI_SUCCESS;
  
  pm_model_type model = pm_get_pmic_model(PMIC_I);
  if(model == PMIC_IS_PM7250B)
  {
    Status = gBS->InstallMultipleProtocolInterfaces(
      &ImageHandle,
      &gQcomPmicPonProtocolGuid,  &PmicPonProtocolImplementation,       /*installing gQcomPmicPonProtocolGuid for open source*/
      &gQcomPmicSchgProtocolGuid, &PmicSchg_P_ProtocolImplementation,
      &gQcomPmicFgProtocolGuid,   &PmicFgBasicProtocolImplementation,
      NULL
    );   
  }
  else{
    Status = gBS->InstallMultipleProtocolInterfaces(
      &ImageHandle,
      &gQcomPmicPonProtocolGuid,  &PmicPonProtocolImplementation,       /*installing gQcomPmicPonProtocolGuid for open source*/
      &gQcomPmicSchgProtocolGuid, &PmicSchgProtocolImplementation,
      &gQcomPmicFgProtocolGuid,   &PmicFgBasicProtocolImplementation,
      NULL
    );
  }

  err_flag = (Status == EFI_SUCCESS) ? PM_ERR_FLAG_SUCCESS : PM_ERR_FLAG_FAILURE;

  return err_flag;
}


pm_err_flag_type pm_post_pmic_initialization(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  pm_err_flag_type err_flag = PM_ERR_FLAG_SUCCESS;
  EFI_PLATFORMINFO_PROTOCOL *pPlatformInfoProtocol;
  EFI_PLATFORMINFO_PLATFORM_INFO_TYPE  platformInfo;
  EFI_PLATFORMINFO_PLATFORM_TYPE  PlatformType;
  EFI_STATUS Status;
  UINT32 nVal= 0;
UINT32 subType = 0;

  Status = gBS->LocateProtocol ( &gEfiPlatformInfoProtocolGuid,
  NULL,
  (VOID **)&pPlatformInfoProtocol
  );

  if ( Status != EFI_SUCCESS )
  {
    DEBUG(( EFI_D_ERROR, "Leica PMIC Init: Failed to locate PlatformInfo Protocol, Status =  (0x%x)\r\n", Status ));
    CORE_VERIFY(0);
  }

  if( !pPlatformInfoProtocol )
  {
    Status = EFI_INVALID_PARAMETER;
    DEBUG(( EFI_D_ERROR, "Leica PMIC Init: Failed pPlatformInfoProtocol = NULL "));
    CORE_VERIFY(0);
  }

  pPlatformInfoProtocol->GetPlatformInfo( pPlatformInfoProtocol, &platformInfo );
  PlatformType = platformInfo.platform;
  subType = platformInfo.subtype;
  pPlatformInfoProtocol->GetKeyValue( pPlatformInfoProtocol,EFI_PLATFORMINFO_KEY_PMIC, &nVal);

  if((EFI_PLATFORMINFO_TYPE_IOT == PlatformType) && ((nVal) || (subType == 0x1) || (subType == 0x2) || (subType == 0x5) || (subType == 0x6) || (subType == 0x7)|| (subType == 0x8) || (subType == 0x9)))
  {
  	return err_flag;   // to handle non leica SKU's
  }

  if(PlatformType != EFI_PLATFORMINFO_TYPE_ATP)
  {
    /*Two Leica PMICs with default SID as 0x8 and 0x9, So to communicate with them we are re-configuring one of Leica PMIC I2C SID to 0xC and 0xD
      To update only One Leica I2C SID, we are keeping One Leica Enabled and other in RESET state.
      So after this API, The Leica in RESET will be PM8008i with 0x8 & 0x9 as SID. and Enabled will be PM8008j with I2C SID 0xC & 0xD */
      
    err_flag = pm_i2c_sid_config();
  }
  
  return err_flag;
}


pm_err_flag_type pm_uefi_exit_target_specific_cleanup(void)
{
  pm_err_flag_type err_flag = PM_ERR_FLAG_SUCCESS;

  return err_flag;
}

static pm_err_flag_type pm_i2c_sid_config(void)
{
  pm_err_flag_type err_flag = PM_ERR_FLAG_SUCCESS;
  i2c_status istatus       = I2C_SUCCESS;
  void *i2c_handle        = NULL;
  uint32 read_write_count  = 0;
  uint8  packet_data[] = {0x06, 0x44, 0x03};      /* peripheral id, register address, register value */
  const uint32 packet_size = sizeof(packet_data);
  EFI_QCOM_PMIC_GPIO_PROTOCOL *PmicGpioProtocol = NULL;
  EFI_STATUS Status = EFI_SUCCESS;


  Status = gBS->LocateProtocol(&gQcomPmicGpioProtocolGuid, NULL, (VOID **)&PmicGpioProtocol);
  if EFI_ERROR (Status)
  {
    pm_log_message("Locate PMIC Protocol Failed!\n");
    return Status;
  }
  
  CORE_VERIFY(PmicGpioProtocol != NULL);

  Status |= PmicGpioProtocol->SetOutputLevel(PMIC_C, LEICA_1_RESET_GPIO, EFI_PM_GPIO_LEVEL_LOW);
  Status |= PmicGpioProtocol->SetOutputLevel(PMIC_C, LEICA_2_RESET_GPIO, EFI_PM_GPIO_LEVEL_HIGH);

  Status |= PmicGpioProtocol->Enable(PMIC_C, LEICA_1_RESET_GPIO, TRUE);
  Status |= PmicGpioProtocol->Enable(PMIC_C, LEICA_2_RESET_GPIO, TRUE);

  if (Status != EFI_SUCCESS)
  {
    CORE_VERIFY(0);
  }

  gBS->Stall(1000); /* Wait for LEICA to power up, 1mS */

  i2c_slave_config  cfg = {
    .bus_frequency_khz          = 400,
    .slave_address              = 0x08,
    .mode                       = I2C,
    .slave_max_clock_stretch_us = 500,
    .core_configuration1        = 0,
    .core_configuration2        = 0
  };

  istatus = i2c_open(LEICA_I2C_INSTANCE, &i2c_handle);

  if ((istatus == I2C_SUCCESS) && (i2c_handle != NULL))
  {
    istatus |= i2c_write(i2c_handle, &cfg, 0, 0, packet_data, packet_size, &read_write_count, TIMEOUT_MAX);
  }

  /* Any error experienced should cause the boot to stop */
  if (istatus != I2C_SUCCESS)
  {
    pm_log_message("Failed to detect external I2C PMIC with return value : %d. PMIC is left at default state \n", istatus);
    //CORE_VERIFY(0);
  }

  i2c_close(i2c_handle);

  return err_flag;
}
