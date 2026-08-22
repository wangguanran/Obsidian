============================================================================

  File: MDPPlatformLibPanelCommon.c

  MDP default platform panel functions

  Copyright (c) 2018-2021 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Protocol/EFIPmicGpio.h>
#include <Protocol/EFIPmicClkBuff.h>
#include <Protocol/EFITlmm.h>
#include <Protocol/EFIPmicLpg.h>
#include <Protocol/EFIPmicGpio.h>
#include <Protocol/EFIPmicVreg.h>
#include <Protocol/EFIPmicIbb.h>
#include <Protocol/EFIPmicLab.h>
#include <Protocol/EFIPmicAb.h>
#include <Protocol/EFIPmicLcdb.h>
#include <Protocol/EFIPmicPwm.h>
#include <Protocol/EFIPmicVersion.h>
#include <api/pmic/pm/pm_version.h>
#include "DDIChipInfo.h"
#include "MDPSystem.h"
#include "MDPPeripherals.h"
#include "MDPPlatformLibPanelCommon.h"
#include "DisplayUtils.h"
#include "pmapp_npa.h"
#include "DDITlmm.h"
#include "HALDSILib.h"
#include "odm_features.h"

#if defined(SM6115_PLATFORM_DISPLAY)
#include "i2c_api.h"
#include <Protocol/EFII2C.h>
#include "boot_logger.h"
#define PM8008_ENABLE_GPIO 26

extern EFI_GUID gQcomTokenSpaceGuid;
uint32_t pm8008Value;
VOID Get_Meige_Pm8008Value(VOID)
{
  DEBUG ((EFI_D_ERROR, "pm8008Value is %d\n", pm8008Value));

  gRT->SetVariable (L"MeiGPM8008ID", &gQcomTokenSpaceGuid,
    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
    (UINTN)sizeof(pm8008Value), &pm8008Value);
}

void pm_i2c_sid_config(void)
{
	uint32 read_write_count =0;
	i2c_slave_config cfg;
	i2c_status istatus = I2C_SUCCESS;
	EFI_STATUS Status = EFI_SUCCESS;
	VOID *pm8008_i2c_handle = NULL;
	uint8 L1I_enable[] =    {0x40, 0x46, 0x80};
	uint8 L2I_enable[] =    {0x41, 0x46, 0x80};
	uint8 L3I_enable[] =    {0x42, 0x46, 0x80};
	uint8 L4I_enable[] =    {0x43, 0x46, 0x80};
	/* VSET: write LB (0x40) first, then UB (0x41). PM8008 latches 16-bit VSET on VSET_UB write. */
	uint8 L5I_set_2P8_1[] = {0x44, 0x40, 0x28};
	uint8 L5I_set_2P8_2[] = {0x44, 0x41, 0x0B};
	uint8 L5I_set_2P8_3[] = {0x44, 0x45, 0x07};
	uint8 L5I_enable[] =    {0x44, 0x46, 0x80};
	uint8 L6I_set_3P0_1[] = {0x45, 0x40, 0x90};
	uint8 L6I_set_3P0_2[] = {0x45, 0x41, 0x0A};
	uint8 L6I_set_3P0_3[] = {0x45, 0x45, 0x07};
	uint8 L6I_enable[] =    {0x45, 0x46, 0x80};
	uint8 L7I_set_1P8_1[] = {0x46, 0x40, 0x08};
	uint8 L7I_set_1P8_2[] = {0x46, 0x41, 0x07};
	uint8 L7I_set_1P8_3[] = {0x46, 0x45, 0x07};
	uint8 L7I_enable[] =    {0x46, 0x46, 0x80};
	DEBUG ((EFI_D_WARN, "pm_i2c_sid_config start\n"));
	DEBUG ((EFI_D_WARN, "pm_i2c_sid_config init pm8008, pull up PM8008_ENABLE_GPIO%d\n", PM8008_ENABLE_GPIO));
	EFI_TLMM_PROTOCOL *TLMMProtocol;
	Status =gBS->LocateProtocol( &gEfiTLMMProtocolGuid,NULL,(void**)&TLMMProtocol);
	ASSERT_EFI_ERROR(Status);
	if(TLMMProtocol){
	Status = TLMMProtocol ->ConfigGpio( (UINT32) EFI_GPIO_CFG(PM8008_ENABLE_GPIO, 0, GPIO_OUTPUT,GPIO_NO_PULL,GPIO_16MA), TLMM_GPIO_ENABLE);
	ASSERT_EFI_ERROR(Status);
	if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(PM8008_ENABLE_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_HIGH_VALUE))
		DEBUG((EFI_D_WARN, "pm_i2c_sid_config pull up gpio PM8008_ENABLE_GPIO Failed!\n"));
	else
		DEBUG((EFI_D_WARN, "pm_i2c_sid_config pull up gpio PM8008_ENABLE_GPIO successfully!\n"));
	}
	DEBUG ((EFI_D_WARN, "pm_i2c_sid_config init pm8008, init /dev/i2c%d\n", I2C_INSTANCE_002));
	istatus = i2c_open(I2C_INSTANCE_002, &pm8008_i2c_handle);
	if (I2C_SUCCESS != istatus){
		DEBUG ((EFI_D_WARN, "Meig:pm_i2c_sid_config i2c_open failed\n"));
	}
	cfg.bus_frequency_khz = I2C_STANDARD_MODE_FREQ_KHZ;
	cfg.slave_address = 0x9;
	cfg.mode = I2C;
	cfg.slave_max_clock_stretch_us = 500;
	cfg.core_configuration1 = 0;
	cfg.core_configuration2 = 0;
	if ((istatus == I2C_SUCCESS) && (pm8008_i2c_handle != NULL))
	{
		DEBUG ((EFI_D_WARN, "Meig:pm_i2c_sid_config callback i2c_write start\n"));
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L1I_enable,     3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L2I_enable,     3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L3I_enable,     3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L4I_enable,     3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L5I_set_2P8_1,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L5I_set_2P8_2,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L5I_set_2P8_3,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L5I_enable,     3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L6I_set_3P0_1,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L6I_set_3P0_2,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L6I_set_3P0_3,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L6I_enable,     3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L7I_set_1P8_1,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L7I_set_1P8_2,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L7I_set_1P8_3,  3, &read_write_count, 2500);
		istatus |= i2c_write(pm8008_i2c_handle, &cfg, 0, 0, L7I_enable,     3, &read_write_count, 2500);
		DEBUG ((EFI_D_WARN, "Meig:pm_i2c_sid_config callback i2c_write stop\n"));
	}
	/* Any error experienced should cause the boot to stop */
	if (istatus != I2C_SUCCESS)
	{
		pm8008Value = 0;
		DEBUG ((EFI_D_ERROR, "Meig:pm_i2c_sid_config i2c_write failed\n"));
	}else{
		pm8008Value = 1;
		DEBUG ((EFI_D_ERROR, "Meig:pm_i2c_sid_config i2c_write succeed\n"));
	}
	DEBUG ((EFI_D_ERROR, "Meig:pm_i2c_sid_config stop\n"));
	Get_Meige_Pm8008Value();
}
#endif

#define LCD_1V8_GPIO 28
#if defined(ODM_PROJECT_HXB_SLM927M_RuiSiKe)
#include "cw221X_fuel_gauge_V1.h"
VOID ShutdownDevice (VOID)
{
  EFI_STATUS Status = EFI_INVALID_PARAMETER;
  //WaitForFlashFinished ();
  gRT->ResetSystem (EfiResetShutdown, Status, 0, NULL);
  /* Flow never comes here and is fatal if it comes here.*/
  ASSERT (0);
}
#endif

#if defined (ODM_PROJECT_HXB_SNM927D_TianBu)
#include "I2cApi.h"
#include "lt9211.h"
static i2c_config cfg;
static void *pI2cHandle = NULL;

/* i2c config */
i2c_status i2c_init(UINT32 SlaveAddr, UINT32 I2cFreq)
{
    i2c_status i2cstatus = I2C_SUCCESS;
    cfg.bus_frequency_khz = I2cFreq;
    cfg.slave_address = SlaveAddr;
    //cfg.mode = I2C;
    //cfg.slave_max_clock_stretch_us = 500;
    //cfg.core_configuration1 = 0;
    //cfg.core_configuration2 = 0;
    cfg.slave_address_type =  I2C_07_BIT_SLAVE_ADDRESS;
    i2cstatus = i2c_open((i2c_instance) (I2C_INSTANCE_003), &pI2cHandle);
    if (I2C_SUCCESS != i2cstatus){
        DEBUG((EFI_D_ERROR, "Failed to initialize I2C %d\n", i2cstatus));
    }
    return i2cstatus;
}

unsigned int i2c_read_reg(unsigned int addr)
{
    uint32 bRead = 0;
    //unsigned int getdata = 0;
    i2c_status i2cstatus = I2C_SUCCESS;
    unsigned char rdbuf;
    //gBS->Stall(600000);
    i2cstatus = i2c_read(pI2cHandle, &cfg, addr, 1, &rdbuf, 1, &bRead, 2500);
    if(I2C_SUCCESS != i2cstatus){
        DEBUG((EFI_D_ERROR, "Read addr:0x%X error\n", addr));
    }
    //else
        //DEBUG((EFI_D_ERROR, "Read addr:0x%X success, value=0x%X\n", addr, rdbuf));
    gBS->Stall(600);
    //getdata=rdbuf[0] & 0x00ff;
    //getdata<<= 8;
    //getdata |=rdbuf[1];
    DEBUG((EFI_D_ERROR, "lt9211 rdbuf: 0x%x = %x\n", addr, rdbuf));
    return (unsigned int)rdbuf;
}

unsigned int i2c_write_reg(unsigned char addr, unsigned int reg_data)
{
    uint32 bWrote = 0;
    i2c_status i2cstatus = I2C_SUCCESS;
    unsigned char wdbuf = (unsigned char)reg_data;
    //wdbuf[1] = (unsigned char)(reg_data & 0x00ff);
    //wdbuf[0] = (unsigned char)((reg_data & 0xff00)>>8);
    i2cstatus = i2c_write(pI2cHandle, &cfg, addr, 1, &wdbuf, 1, &bWrote, 2500);
    if(I2C_SUCCESS != i2cstatus){
        DEBUG((EFI_D_ERROR, "Write addr:0x%X data:0x%X error\n", addr, reg_data));
    }
    //else
        //DEBUG((EFI_D_ERROR, "Write addr:0x%X data:0x%X success\n", addr, reg_data));
    return bWrote;
}

unsigned int LT9211EXB_IIC_Write_byte(unsigned char addr, unsigned char reg_data)
{
    return i2c_write_reg(addr, reg_data);
}

unsigned int LT9211EXB_IIC_Read_byte(unsigned char addr)
{
    return i2c_read_reg(addr);
}

i2c_status i2c_deinit(void)
{
    return i2c_close(pI2cHandle);
}
#endif



/* General helper functions */
static void       Panel_LCDB_WaitForReady(EFI_QCOM_PMIC_LCDB_PROTOCOL   *PmicLCDBProtocol);
static MDP_Status GetPmicIBBLABMode(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams);


/**********************************************************************************************
*
* FUNCTION: Panel_Default_PowerUp()
*
* DESCRIPTION:
*   Panel power up sequence for default platform, such as CDP/MTP/QRD.
*
***********************************************************************************************/
MDP_Status Panel_Default_PowerUp(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams)
{
  MDP_Status                   Status           = MDP_STATUS_OK;
  EFI_TLMM_PROTOCOL           *TLMMProtocol     = NULL;
  EFI_QCOM_PMIC_GPIO_PROTOCOL *PmicGpioProtocol = NULL;
#if defined(SM6115_PLATFORM_DISPLAY)
  EFI_STATUS                   eLocalStatus     = EFI_SUCCESS;
#endif

#if defined(ODM_PROJECT_HXB_SLM927M_RuiSiKe)
  meig_battery_boot_check();
#endif

  if (MDP_STATUS_OK != (Status =  PlatformClientInit(eDisplayId, pPowerParams)))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Failed to initialize handle for Primary display NPA node.\n"));
  }
  else if (EFI_SUCCESS != gBS->LocateProtocol(&gEfiTLMMProtocolGuid, NULL, (void **)&TLMMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate TLMM protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicGpioProtocolGuid, NULL, (VOID **)&PmicGpioProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC GPIO protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else
  {
#if defined(SM6115_PLATFORM_DISPLAY)
    DEBUG((EFI_D_ERROR, "DisplayDxe: Panel_Default_PowerUp callback pm_i2c_sid_config start\n"));
    pm_i2c_sid_config();
    DEBUG((EFI_D_ERROR, "DisplayDxe: Panel_Default_PowerUp callback pm_i2c_sid_config stop\n"));
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(LCD_1V8_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure LCD_1V8_GPIO failed!\n"));
    }
         if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(LCD_1V8_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: LCD_1V8_GPIO gpioout failed!\n"));
    }
    eLocalStatus = PmicGpioProtocol->ConfigDigitalOutput(PMIC_A,
                                                    EFI_PM_GPIO_2,
                                                    EFI_PM_GPIO_OUT_BUFFER_CONFIG_CMOS,
                                                    EFI_PM_GPIO_VIN0,
                                                    EFI_PM_GPIO_SOURCE_GND,
                                                    EFI_PM_GPIO_OUT_BUFFER_HIGH, 1
                                                    );
    if (EFI_SUCCESS != eLocalStatus)
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe:lcm enable gpio (2) config fialed - %d!\n", eLocalStatus));
      Status = MDP_STATUS_FAILED;
    }
#endif
   /*
    * Voting for Display NPA node to be ON
    */
 
    /* TODO: Voting with STANDBY as MODE-1 is treated as ENABLE for LDO13A in SDMPkg/6150/Settings/PMIC/LA/pm_config_pam.cpm_config_pam.c
     * Once PMIC team updates the mode settings this has to be changed back to PMIC_NPA_MODE_ID_GENERIC_ACTIVE
     */
    npa_issue_required_request(pPowerParams->sNPAClient[eDisplayId], PMIC_NPA_MODE_ID_GENERIC_STANDBY);

    MDP_OSAL_DELAYMS(10);   /* delay 10ms to allow power grid to settle */

    /* Display TE pin 
	 * For MDSS 6.3.0 the special function's 1 and 2 are reserved for output, function 3 is used for TE input to the controller.
	 */
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio(EFI_GPIO_CFG(DEFAULT_DISP_TE_GPIO, 3, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure GPIO %d for TE line failed %d\n", DEFAULT_DISP_TE_GPIO));
    }

   if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(DEFAULT_TOUCH_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure DEFAULT_TOUCH_GPIO failed!\n"));
    } 
	 if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(DEFAULT_TOUCH_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: DEFAULT_TOUCH_GPIO gpioout failed!\n"));
    }

  }
#if defined(ODM_PROJECT_HXB_SLM927_RuiSiKe) || defined(ODM_PROJECT_HXB_SLM927M_RuiSiKe)
        /*add by xieduo*/
        /*en gpio*/
        if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(58, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), DAL_TLMM_GPIO_ENABLE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO58 as output\n"));
		  Status = MDP_STATUS_FAILED;
		}
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(58, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO58 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
#endif
#if defined(ODM_PROJECT_HXB_SNM927D_TianBu)
        /*add by xieduo*/
        /*en gpio*/
        if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(58, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), DAL_TLMM_GPIO_ENABLE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO58 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(58, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_LOW_VALUE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO58 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
        /*lcd reset*/
        if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(85, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), DAL_TLMM_GPIO_ENABLE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO85 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(85, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_LOW_VALUE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO85 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
        MDP_OSAL_DELAYMS(100);
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(85, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO85 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
        /*lt9211 reset*/
        if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), DAL_TLMM_GPIO_ENABLE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO82 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_LOW_VALUE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO82 as output\n"));
          Status = MDP_STATUS_FAILED;
        }
        DEBUG((EFI_D_ERROR, " lt8911 rst down \n"));
        MDP_OSAL_DELAYMS(100);
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
        {
          DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO82 as output\n"));
          Status = MDP_STATUS_FAILED;
        }

        DEBUG((EFI_D_ERROR, " lt8911 rst up \n"));
        MDP_OSAL_DELAYMS(1000);
        i2c_init(0x2d, 100);
        MDP_OSAL_DELAYMS(100);
        lt9211_init();
        MDP_OSAL_DELAYMS(50);
#endif
  return Status;
}


/**********************************************************************************************
*
* FUNCTION: Panel_Default_PowerDown()
*
* DESCRIPTION:
*   Panel power down sequence for default platform, such as CDP/MTP/QRD.
*
***********************************************************************************************/
MDP_Status Panel_Default_PowerDown(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams)
{
  MDP_Status                   Status           = MDP_STATUS_OK;
  EFI_TLMM_PROTOCOL           *TLMMProtocol     = NULL;
  EFI_QCOM_PMIC_GPIO_PROTOCOL *PmicGpioProtocol = NULL;

  if (NULL == pPowerParams->sNPAClient[eDisplayId])
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: NULL Handle for Primary display NPA node.\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else if (EFI_SUCCESS != gBS->LocateProtocol(&gEfiTLMMProtocolGuid, NULL, (void **)&TLMMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate TLMM protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicGpioProtocolGuid, NULL, (VOID **)&PmicGpioProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC GPIO protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else
  {
    /* 
     * GPIOs & VRegs
     */
    /* Display RESET_N pin */
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(pPowerParams->uResetGpio, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), TLMM_GPIO_DISABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Failed to disable GPIO %d for Reset_N line\n", pPowerParams->uResetGpio));
    }

    /* Display TE pin */
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio(EFI_GPIO_CFG(DEFAULT_DISP_TE_GPIO, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), TLMM_GPIO_DISABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Failed to disable GPIO %d for TE pin\n", DEFAULT_DISP_TE_GPIO));
    }


    /*
    * Voltage Regulators
    */
    npa_complete_request(pPowerParams->sNPAClient[eDisplayId]);           // Complete the request to power rails
  }

  return Status;
}


/**********************************************************************************************
*
* FUNCTION: Panel_ResetStateExec()
*
* DESCRIPTION:
*   Toggle reset gpio base on states transition.
*
***********************************************************************************************/
static void Panel_ResetStateExec(EFI_TLMM_PROTOCOL *tlmm, MDPPlatformPanelResetInfo *pResetInfo, uint32 state0, uint32 state1, uint32 state2)
{
    if (EFI_SUCCESS != tlmm->GpioOut((UINT32)EFI_GPIO_CFG(pResetInfo->uResetGpio, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), state0))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Reset_N line FAILED at state-0!\n"));
    }

    MDP_OSAL_DELAYUS(pResetInfo->uPreResetTimeUs);

    if (EFI_SUCCESS != tlmm->GpioOut((UINT32)EFI_GPIO_CFG(pResetInfo->uResetGpio, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), state1))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Reset_N line FAILED at state-1!\n"));
    }

    MDP_OSAL_DELAYUS(pResetInfo->uResetAssertTimeUs);

    if (EFI_SUCCESS != tlmm->GpioOut((UINT32)EFI_GPIO_CFG(pResetInfo->uResetGpio, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), state2))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Reset_N line FAILED at state-2!\n"));
    }
}


/**********************************************************************************************
*
* FUNCTION: Panel_Default_Reset()
*
* DESCRIPTION:
*   Panel reset sequence for default platform, such as CDP/MTP/QRD.
*
***********************************************************************************************/
MDP_Status Panel_Default_Reset(MDPPlatformPanelResetInfo *pResetInfo)
{
  MDP_Status                    Status           = MDP_STATUS_OK;
  EFI_TLMM_PROTOCOL            *TLMMProtocol     = NULL;

  if (EFI_SUCCESS != gBS->LocateProtocol(&gEfiTLMMProtocolGuid, NULL, (void **)&TLMMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate TLMM protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else
  {
    /* Enable Display Reset pin*/
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(pResetInfo->uResetGpio, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure GPIO %d for Reset_N line failed!\n", pResetInfo->uResetGpio));
    }

    switch (pResetInfo->uAssertState)
    {
      case MDP_RESET_STATE_EDGE_LOW2HIGH:
         Panel_ResetStateExec(TLMMProtocol, pResetInfo, GPIO_LOW_VALUE, GPIO_HIGH_VALUE, GPIO_HIGH_VALUE);
       break;
      case MDP_RESET_STATE_EDGE_HIGH2LOW:
         Panel_ResetStateExec(TLMMProtocol, pResetInfo, GPIO_HIGH_VALUE, GPIO_LOW_VALUE, GPIO_LOW_VALUE);
       break;
      case MDP_RESET_STATE_LEVEL_HIGH:
         Panel_ResetStateExec(TLMMProtocol, pResetInfo, GPIO_LOW_VALUE, GPIO_HIGH_VALUE, GPIO_LOW_VALUE);
       break;
      case MDP_RESET_STATE_LEVEL_LOW:
      default:
         Panel_ResetStateExec(TLMMProtocol, pResetInfo, GPIO_HIGH_VALUE, GPIO_LOW_VALUE, GPIO_HIGH_VALUE);
       break;
    }

    MDP_OSAL_DELAYUS(pResetInfo->uPostResetTimeUs);

    /*
     * post reset time: Timing space between end of reset to start sending init dcs cmds
     * NOTE:
     *   use DSIInitMasterTime to extend post reset time control if required
     */
  }

  return Status;
}


/**********************************************************************************************
*
* FUNCTION: GetPmicIBBLABMode()
*
* DESCRIPTION:
*   This function can get the PMIC AMOLED/LCD mode configuration.
*
***********************************************************************************************/
static MDP_Status GetPmicIBBLABMode(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams)
{
  EFI_QCOM_PMIC_VERSION_PROTOCOL *PmicVersionProtocol = NULL;
  EFI_PM_DEVICE_INFO_TYPE         PmicDeviceInfo;
  MDP_Status                      eStatus             = MDP_STATUS_OK;


  if (NULL == pPowerParams)
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: GetPmicIBBLABMode pPowerParams is NULL!\n"));
    eStatus = MDP_STATUS_NO_RESOURCES;
  }
  else if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicVersionProtocolGuid, NULL, (VOID **)&PmicVersionProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: GetPmicIBBLABMode Locate PMIC Version Protocol failed!\n"));
    eStatus = MDP_STATUS_NO_RESOURCES;
  }
  else
  {
     if (EFI_SUCCESS != PmicVersionProtocol->GetPmicInfo(PM632_INDEX, &PmicDeviceInfo))
     {
        DEBUG((EFI_D_ERROR, "DisplayDxe: GetPmicInfo failed!\n"));
        eStatus = MDP_STATUS_NO_RESOURCES;
     }
     else
     {
        if (PmicDeviceInfo.PmicModel == EFI_PMIC_IS_PMI632)
        {
          pPowerParams->ePMICSecondaryPower[eDisplayId] = MDP_PMIC_MODULE_CONTROLTYPE_IBB_LAB_LCD; 
        }
        else if (PmicDeviceInfo.PmicModel == EFI_PMIC_IS_UNKNOWN)
		{
		  pPowerParams->ePMICSecondaryPower[eDisplayId] = MDP_PMIC_MODULE_CONTROLTYPE_NONE; 
		}
        else 
		{
           pPowerParams->ePMICSecondaryPower[eDisplayId] = MDP_PMIC_MODULE_CONTROLTYPE_NONE; 
           eStatus = MDP_STATUS_NO_RESOURCES;
        }
     }
   }

  return eStatus;
}


/**********************************************************************************************
*
* FUNCTION: Panel_Default_Brightness_Enable()
*
* DESCRIPTION:
*   Configure PWM registers and enable it.
*
***********************************************************************************************/
MDP_Status Panel_Default_Brightness_Enable(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams)
{
  MDP_Status                   Status           = MDP_STATUS_OK;
  EFI_QCOM_PMIC_GPIO_PROTOCOL *PmicGpioProtocol = NULL;
  EFI_QCOM_PMIC_PWM_PROTOCOL  *PMICPWMProtocol  = NULL;
  EFI_STATUS                   eLocalStatus     = EFI_SUCCESS;
 
#if defined(ODM_PROJECT_HXB_SNM927D_TianBu)
   EFI_TLMM_PROTOCOL           *TLMMProtocol     = NULL;
   if (EFI_SUCCESS != gBS->LocateProtocol(&gEfiTLMMProtocolGuid, NULL, (void **)&TLMMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate TLMM protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  MDP_OSAL_DELAYMS(1000);
  if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(58, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
  {
    DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO58 as output\n"));
    Status = MDP_STATUS_FAILED;
  }
#endif

  if (MDP_STATUS_OK != (Status = GetPmicIBBLABMode(eDisplayId, pPowerParams)))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Getting PMIC mode failed with error(%d), skipping peripheral power configuration!\n", Status));
  }
  else if (pPowerParams->ePMICSecondaryPower[eDisplayId] != MDP_PMIC_MODULE_CONTROLTYPE_NONE)
  {
	 
    // Configure Backlight enable gpio on Nebula
    if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicGpioProtocolGuid, NULL, (VOID **)&PmicGpioProtocol))
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC GPIO protocol failed!\n"));
      Status = MDP_STATUS_NO_RESOURCES;
    } 
    else 
    {
#if defined(SM6115_PLATFORM_DISPLAY)
      eLocalStatus = PmicGpioProtocol->ConfigDigitalOutput(PMIC_A,
                                                    EFI_PM_GPIO_8,
                                                    EFI_PM_GPIO_OUT_BUFFER_CONFIG_CMOS,
                                                    EFI_PM_GPIO_VIN0,
                                                    EFI_PM_GPIO_SOURCE_SPECIAL_FUNCTION1,
                                                    EFI_PM_GPIO_OUT_BUFFER_HIGH, 1
                                                    );
      if (EFI_SUCCESS != eLocalStatus)
      {
        DEBUG((EFI_D_ERROR, "DisplayDxe: Backlight enable gpio (8) config fialed - %d!\n", eLocalStatus));
        Status = MDP_STATUS_FAILED;
      }
#else
      eLocalStatus = PmicGpioProtocol->ConfigDigitalOutput(PM632_INDEX,
                                                      EFI_PM_GPIO_6,
                                                      EFI_PM_GPIO_OUT_BUFFER_CONFIG_CMOS,
                                                      EFI_PM_GPIO_VIN0,
                                                      EFI_PM_GPIO_SOURCE_GND,
                                                      EFI_PM_GPIO_OUT_BUFFER_LOW, 1
                                                      );
      if (EFI_SUCCESS != eLocalStatus)
      {
        DEBUG((EFI_D_ERROR, "DisplayDxe: Backlight enable gpio (6) config fialed - %d!\n", eLocalStatus));
        Status = MDP_STATUS_FAILED;
      }
#endif
    }
  }
  
  /* Configure PWM */
  if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicPwmProtocolGuid, NULL, (void **)&PMICPWMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMICPWM Protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  } 
  else
  {
    eLocalStatus = PMICPWMProtocol->PwmConfig(0,
                                          EFI_PM_PWM_RSRC_PWM_1,
                                          0,
                                          1,
                                          2,
                                          EFI_PM_PWM_CLK_19_2_MHZ,
                                          EFI_PM_PWM_SIZE_9BIT);
    if (EFI_SUCCESS != eLocalStatus)
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe: PwmConfig failed error %d !\n", eLocalStatus));
	  Status = MDP_STATUS_FAILED;
    }
  } 
  return Status;
}


/**********************************************************************************************
*
* FUNCTION: Panel_Default_Peripheral_Power()
*
* DESCRIPTION:
*   Secondary power sequence for other PMIC modules such as IBB/LAB.
*
***********************************************************************************************/
MDP_Status Panel_Default_Peripheral_Power(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams, bool32 bPowerUp)
{
  MDP_Status Status = MDP_STATUS_OK;

  if (MDP_STATUS_OK != (Status = GetPmicIBBLABMode(eDisplayId, pPowerParams)))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Getting PMIC mode failed with error(%d), skipping peripheral power configuration!\n", Status));
  }
  else
  {
    EFI_QCOM_PMIC_IBB_PROTOCOL   *PmicIBBProtocol  = NULL;
    EFI_QCOM_PMIC_AB_PROTOCOL    *PmicABProtocol   = NULL;
    EFI_QCOM_PMIC_LCDB_PROTOCOL  *PmicLCDBProtocol = NULL;

    if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicIbbProtocolGuid, NULL, (VOID **)&PmicIBBProtocol))
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC IBB Protocol failed!\n"));
      Status = MDP_STATUS_NO_RESOURCES;
    }
    else if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicAbProtocolGuid, NULL, (VOID **)&PmicABProtocol))
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC AB Protocol failed!\n"));
      Status = MDP_STATUS_NO_RESOURCES;
    }
    else if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicLcdbProtocolGuid, NULL, (VOID **)&PmicLCDBProtocol))
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC LCDB Protocol failed!\n"));
      Status = MDP_STATUS_NO_RESOURCES;
    }
    else if (TRUE == bPowerUp)
    {
      // ********** Brightness Power-up Sequence **********

      // Configure the power grid based on the module type
      switch (pPowerParams->ePMICSecondaryPower[eDisplayId])
      {
      case MDP_PMIC_MODULE_CONTROLTYPE_IBB_LAB_LCD:
        {
          // Power up
          /************************************************************************/
          /* Turn On IBB(+5.5v) first, wait for 8ms, turn on LAB(-5.5v)           */
          /************************************************************************/
          // Enable LCD mode
          
          // Turn on LCDB
          if (EFI_SUCCESS != PmicLCDBProtocol->SetOutputSoftStart(PM632_INDEX,5500,-5500,TRUE))
          {
            DEBUG((EFI_D_ERROR, "DisplayDxe: Error to enable LCDB\n"));
          }

          // Wait for IBB/LAB to become ready
          Panel_LCDB_WaitForReady(PmicLCDBProtocol);
        }
        break;
      case MDP_PMIC_MODULE_CONTROLTYPE_IBB_LAB_OLED:
        {
          /* No need to setup LAB/IBB manually for OLED panel as that is controlled by swire */
        }
        break;
      default:
        break;
      }
    }
    else
    {
      // ********** Brightness Power-down Sequence **********

      // Configure the power grid based on the module type
      switch (pPowerParams->ePMICSecondaryPower[eDisplayId])
      {
      case MDP_PMIC_MODULE_CONTROLTYPE_IBB_LAB_LCD:
        // Power down
        if (EFI_SUCCESS != PmicLCDBProtocol->ConfigEnableCtrl(PM632_INDEX, EFI_PM_LCDB_ENABLE_CTL_MODULE_EN, FALSE))
        {
          DEBUG((EFI_D_ERROR, "DisplayDxe: Error to disable LCDB\n"));
        }

        break;

      case MDP_PMIC_MODULE_CONTROLTYPE_IBB_LAB_OLED:
      default:
        break;
      }
    }
  }
  return Status;
}


/**********************************************************************************************
*
* FUNCTION: Panel_Default_Brightness_Level()
*
* DESCRIPTION:
*   The function can control the panel brightness level.
*
***********************************************************************************************/
MDP_Status Panel_Default_Brightness_Level(MDP_Display_IDType eDisplayId, BacklightConfigType *pBacklightConfig)
{
  MDP_Status Status                             = MDP_STATUS_OK;
  EFI_QCOM_PMIC_PWM_PROTOCOL  *PMICPWMProtocol  = NULL;
  EFI_STATUS                   eLocalStatus     = EFI_SUCCESS;
  uint32 uBrightnessValue                       = (511 * pBacklightConfig->uLevel) / 100;
 
  if (MDP_PANEL_BACKLIGHTTYPE_PMIC == pBacklightConfig->eBacklightType)
  {
    /* Configure PWM */
    if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicPwmProtocolGuid, NULL, (void **)&PMICPWMProtocol))
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMICPWM Protocol failed!\n"));
      Status = MDP_STATUS_NO_RESOURCES;
    } 
    else
    {
      eLocalStatus = PMICPWMProtocol->PwmConfig(0,
                                            EFI_PM_PWM_RSRC_PWM_1,
                                            uBrightnessValue,
                                            1,
                                            2,
                                            EFI_PM_PWM_CLK_19_2_MHZ,
                                            EFI_PM_PWM_SIZE_9BIT);
      if (EFI_SUCCESS != eLocalStatus)
      {
        DEBUG((EFI_D_ERROR, "DisplayDxe: PwmConfig failed error %d !\n", eLocalStatus));
  	  Status = MDP_STATUS_FAILED;
      }
    }
  }
  else if (MDP_PANEL_BACKLIGHTTYPE_DSI == pBacklightConfig->eBacklightType)
  {
    if (pBacklightConfig->uNumBits > 16)
    {
      DEBUG((EFI_D_ERROR, "Backlight dcs bit width is not available\n"));
      Status = MDP_STATUS_BAD_PARAM;
    }
    else
    {
      PlatformPanelBacklightControl panelBacklightControl = {
        0x00,
        {0x51, 0x00, 0x00}, // Using standard MIPI DCS commands to control brightness
      };

      if (pBacklightConfig->uNumBits > 8)
      {
        // Calculate brightness
        panelBacklightControl.uCmdType   = 0x39; // Use dcs long write type, e.g. 10bits: first byte is bits 0:7, the second byte is bits 8:9.
        panelBacklightControl.command[1] = DISP_DIV_ROUND_UP(((1 << pBacklightConfig->uNumBits) - 1) * pBacklightConfig->uLevel, 100) &  0xFF;
        panelBacklightControl.command[2] = DISP_DIV_ROUND_UP(((1 << pBacklightConfig->uNumBits) - 1) * pBacklightConfig->uLevel, 100) >> 0x08;
      }
      else
      {
        // Default to calculate brightness based on 8 bit mode
        panelBacklightControl.uCmdType   = 0x15; // Use dcs short write type as only needs one byte.
        panelBacklightControl.command[1] = DISP_DIV_ROUND_UP(0xFF * pBacklightConfig->uLevel, 100) & 0xFF;
      }

      Status = DSIDriver_Write(panelBacklightControl.uCmdType,
                               panelBacklightControl.command,
                               sizeof(panelBacklightControl.command));
      if (MDP_STATUS_OK != Status)
      {
        DEBUG((EFI_D_ERROR, "Backlight brightness DCS command send failed\n"));
      }
	}
  }
  else
  {
    // Nothing to do for other configurations
  }
  return Status;
}


/**********************************************************************************************
*
* FUNCTION: Panel_LCDB_WaitForReady()
*
* DESCRIPTION:
*   Polling loop to wait for IBB/LAB modules to be ready.
*   VDISN, VDISP will reach the proper voltage upon ready.
*
***********************************************************************************************/
static void Panel_LCDB_WaitForReady(EFI_QCOM_PMIC_LCDB_PROTOCOL   *PmicLCDBProtocol)
{

  BOOLEAN lcdbStatus;
  uint32                 uCount;

  for (uCount = 0; uCount < PMIC_IBBLAB_READYTIME; uCount++)
  {
      if (EFI_SUCCESS == PmicLCDBProtocol->VregOk(PM632_INDEX, &lcdbStatus))
      {
          if (TRUE == lcdbStatus)
             break;
      }
      else
      {
        DEBUG((EFI_D_ERROR, "DisplayDxe: LCDB VregOK() failed\n"));
        break;
      }
      // Wait 1ms between checks
      MDP_OSAL_DELAYMS(1);
  }

}


/**********************************************************************************************
*
* FUNCTION: DP_USBPlugInvertedOrientation()
*
* DESCRIPTION:
*   Get DP plug polarity (TRUE = inverted, FALSE = normal).
*
***********************************************************************************************/
bool32 DP_USBPlugInvertedOrientation (void)
{ 
  /* USB driver support for detecting orientation is not available. 
   * Use Hardcoded cable polarity 
   */
  bool32  bPolarity = DP_PLUG_INVERTED_POLARITY;

  // For Klocwork - will never be true when this fn. is called
  if (PCD_EXTERNAL_DISPLAY_DP != PcdGet32(PcdExtDisplayType))
  {
     bPolarity = !bPolarity;
  }

  return bPolarity;
}


/**********************************************************************************************
*
* FUNCTION: ExternalPanel_Default_PowerUp()
*
* DESCRIPTION:
*   The default power up function for external display.
*
***********************************************************************************************/
MDP_Status ExternalPanel_Default_PowerUp(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams)
{
  MDP_Status                    Status            = MDP_STATUS_OK;
  EFI_TLMM_PROTOCOL            *TLMMProtocol      = NULL;

  if (MDP_STATUS_OK != (Status =  PlatformClientInit(eDisplayId, pPowerParams)))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Failed to initialize handle for External (HDMI) display NPA node.\n"));
  }
  else if (EFI_SUCCESS != gBS->LocateProtocol(&gEfiTLMMProtocolGuid, NULL, (void **)&TLMMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate TLMM protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else
  {
    /*
    * Voting for Display NPA node to be ON
    */
    npa_issue_required_request(pPowerParams->sNPAClient[eDisplayId], PMIC_NPA_MODE_ID_GENERIC_ACTIVE );

    switch (PcdGet32(PcdExtDisplayType))
    {
      case PCD_EXTERNAL_DISPLAY_DP:
        npa_issue_required_request(pPowerParams->sNPAClient[eDisplayId], PMIC_NPA_MODE_ID_GENERIC_ACTIVE );

        /* Configure USB_DP_OE_N (EN_N) GPIO */
        if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(43, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), DAL_TLMM_GPIO_ENABLE))
        {
          DEBUG((EFI_D_ERROR, "DP: failed to configure USB_DP_OE_N GPIO as output \n"));        
          Status = MDP_STATUS_FAILED;
        }
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(43, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), GPIO_LOW_VALUE))
        {
          DEBUG((EFI_D_ERROR, "DP: failed to drive USB_DP_OE_N GPIO \n"));  
          Status = MDP_STATUS_FAILED;
        }
           
        /* Configure gpio for chip select OE_N of USB-DP Mux logic (SBU_SW_OE) */      
        if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(51, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), DAL_TLMM_GPIO_ENABLE))
        {
          DEBUG((EFI_D_ERROR, "DP: failed to configure USB_EURO_SEL_DC GPIO as output \n"));      
          Status = MDP_STATUS_FAILED;
        }
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(51, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_LOW_VALUE))
        {
          DEBUG((EFI_D_ERROR, "DP: failed to drive USB_EURO_SEL_DC GPIO \n"));
          Status = MDP_STATUS_FAILED;
        }
           
        break;
  
      default:
        break;
    }

  }

  return Status;
}


/**********************************************************************************************
*
* FUNCTION: ExternalPanel_Default_PowerDown()
*
* DESCRIPTION:
*   The default power down function for external display.
*
***********************************************************************************************/
MDP_Status ExternalPanel_Default_PowerDown(MDP_Display_IDType eDisplayId, Panel_PowerCtrlParams *pPowerParams)
{
  MDP_Status                    Status            = MDP_STATUS_OK;
  EFI_TLMM_PROTOCOL             *TLMMProtocol     = NULL;

  if (NULL == pPowerParams->sNPAClient[eDisplayId])
  {
    DEBUG ((EFI_D_ERROR, "DisplayDxe: NULL Handle for HDMI NPA node.\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else if (EFI_SUCCESS != gBS->LocateProtocol(&gEfiTLMMProtocolGuid, NULL, (void **)&TLMMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate TLMM protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else
  {
    switch (PcdGet32(PcdExtDisplayType))
    {
      case PCD_EXTERNAL_DISPLAY_DP:
        break;
 
      default:
        break;
    }
  }

  return Status;
}

#ifdef __cplusplus
}
#endif
