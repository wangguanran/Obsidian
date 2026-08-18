/*=============================================================================

  File: MDPPlatformLibPanelCommon.c

  MDP default platform panel functions

  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
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
#include "DDIChipInfo.h"
#include "MDPSystem.h"
#include "MDPPeripherals.h"
#include "MDPPlatformLibPanelCommon.h"
#include "DisplayUtils.h"
#include "pmapp_npa.h"
#include "DDITlmm.h"
#include "HALDSILib.h"
#include "api/pmic/pm/pm_version.h"
#if defined(ODM_PROJECT_HXB_SNM550D_BaoJian)
#include "odm_features.h"
#include "lt8912.h"
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
#if defined(ODM_PROJECT_SLM560D)
  EFI_STATUS                   eLocalStatus     = EFI_SUCCESS;
#endif
    DEBUG((EFI_D_ERROR, "AgattiPkg Panel_Default_PowerUp: enter!\n"));
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
   /*
    * Voting for Display NPA node to be ON
    */
 
    /* TODO: Voting with STANDBY as MODE-1 is treated as ENABLE for LDO13A in SDMPkg/6150/Settings/PMIC/LA/pm_config_pam.cpm_config_pam.c
     * Once PMIC team updates the mode settings this has to be changed back to PMIC_NPA_MODE_ID_GENERIC_ACTIVE
     */
    npa_issue_required_request(pPowerParams->sNPAClient[eDisplayId], PMIC_NPA_MODE_ID_GENERIC_ACTIVE);

    MDP_OSAL_DELAYMS(10);   /* delay 10ms to allow power grid to settle */
#if defined(ODM_PROJECT_SLM550) || defined(ODM_PROJECT_SNM550) || defined(ODM_PROJECT_SNM550GO)
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(SLM550_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure SLM550_LCD_1P8_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(SLM550_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: SLM550_LCD_1P8_EN_GPIO gpioout failed!\n"));
    }
#endif

#if defined(ODM_PROJECT_SLM560D)
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(SLM560D_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure SLM560D_LCD_1P8_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(SLM560D_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: SLM560D_LCD_1P8_EN_GPIO gpioout failed!\n"));
    }

    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(SLM560D_LCD_2P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure SLM560D_LCD_2P8_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(SLM560D_LCD_2P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: SLM560D_LCD_2P8_EN_GPIO gpioout failed!\n"));
    }

    if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicGpioProtocolGuid, NULL, (VOID **)&PmicGpioProtocol))
      {
        DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC GPIO protocol failed!\n"));
      }
      else
      {
        eLocalStatus = PmicGpioProtocol->ConfigDigitalOutput(
                    0,
                    EFI_PM_GPIO_3,
                    EFI_PM_GPIO_OUT_BUFFER_CONFIG_CMOS,
                    EFI_PM_GPIO_VIN1,
                    EFI_PM_GPIO_SOURCE_GND,
                    EFI_PM_GPIO_OUT_BUFFER_MEDIUM,
                    FALSE
                  );

        if (EFI_SUCCESS != eLocalStatus)
        {
          DEBUG((EFI_D_WARN, "DisplayDxe: PMIC_A GPIO3 ConfigDigitalOutput failed! Status=%r\n", eLocalStatus));
        }
        else
        {
          DEBUG((EFI_D_INFO, "DisplayDxe: PMIC_A GPIO3 configured OK (VIN1 for 1.8V)\n"));
        }
      }
#endif
#if defined(ODM_PROJECT_HXB_SLM560D_Analogics) 
  if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(60, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), TLMM_GPIO_ENABLE))
    {   
      DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO%d as output\n", 60));
      Status = MDP_STATUS_FAILED;
    }   
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(60, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
    {   
      DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to drive GPIO%d\n", 60));
      Status = MDP_STATUS_FAILED;
    } 
    DEBUG((EFI_D_ERROR, " GPIO60 END\n"));
    MDP_OSAL_DELAYMS(50);

    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(71, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO%d as output\n", 71));
      Status = MDP_STATUS_FAILED;
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(71, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
    {   
      DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to drive GPIO%d\n", 71));
      Status = MDP_STATUS_FAILED;
    }
    MDP_OSAL_DELAYMS(20);
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(71, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_LOW_VALUE))
    {
       DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to drive GPIO%d\n", 71));
      Status = MDP_STATUS_FAILED;
    }
    MDP_OSAL_DELAYMS(50);
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(71, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to drive GPIO%d\n", 71));
      Status = MDP_STATUS_FAILED;
    }
    DEBUG((EFI_D_ERROR, " GPIO71 TP_RST END\n"));

#endif
#if defined(ODM_PROJECT_HXB_SLM550_TianBo) || defined(ODM_PROJECT_HXB_SLM550D_TianBo)
  if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(14, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to configure GPIO%d as output\n", 14));
      Status = MDP_STATUS_FAILED;
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(14, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_ERROR, "display>>>Panel_Default_PowerUp pull: failed to drive GPIO%d\n", 14));
      Status = MDP_STATUS_FAILED;
    }
	if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), TLMM_GPIO_ENABLE))
	{
		DEBUG((EFI_D_ERROR, "DisplayDxe: Configure GPIO %d for gpio100 Failed!\n", 82));
	}
	MDP_OSAL_DELAYMS(100);
	if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_HIGH_VALUE))
	{
		DEBUG((EFI_D_ERROR, "DisplayDxe: gpio82 line HIGH failed!\n"));
	}
	DEBUG((EFI_D_ERROR, " lt8911 reset gpio down \n"));
	MDP_OSAL_DELAYMS(100);
  
  if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), GPIO_LOW_VALUE))
	{
	      DEBUG((EFI_D_ERROR, "DisplayDxe: gpio82 line LOW failed!\n"));
	}
#endif


#if defined(ODM_PROJECT_MT5205)
    /* MT5205 power rails: IOVCC(111) -> VDD2V8(112) -> BIAS(105) later */
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(MT5205_LCD_IOVCC_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure MT5205_LCD_IOVCC_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(MT5205_LCD_IOVCC_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: MT5205_LCD_IOVCC_EN_GPIO gpioout failed!\n"));
    }
    MDP_OSAL_DELAYMS(10);

    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(MT5205_LCM_VDD2V8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure MT5205_LCM_VDD2V8_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(MT5205_LCM_VDD2V8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: MT5205_LCM_VDD2V8_EN_GPIO gpioout failed!\n"));
    }
    MDP_OSAL_DELAYMS(10);
#endif
    /* Display TE pin */
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio(EFI_GPIO_CFG(DEFAULT_DISP_TE_GPIO, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure GPIO %d for TE line failed %d\n", DEFAULT_DISP_TE_GPIO));
    }

    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(DEFAULT_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure DEFAULT_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(DEFAULT_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_HIGH_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: DEFAULT_EN_GPIO gpioout failed!\n"));
    }
  DEBUG((EFI_D_ERROR, "AgattiPkg Panel_Default_PowerUp: end....!\n"));
  }

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

#if defined(ODM_PROJECT_MT5205)
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(DEFAULT_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure DEFAULT_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(DEFAULT_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), GPIO_LOW_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: DEFAULT_EN_GPIO gpioout failed!\n"));
    }
    MDP_OSAL_DELAYMS(5);

    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(MT5205_LCM_VDD2V8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), GPIO_LOW_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: MT5205_LCM_VDD2V8_EN_GPIO gpioout failed!\n"));
    }
    MDP_OSAL_DELAYMS(5);

    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(MT5205_LCD_IOVCC_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA), GPIO_LOW_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: MT5205_LCD_IOVCC_EN_GPIO gpioout failed!\n"));
    }
#endif
    npa_complete_request(pPowerParams->sNPAClient[eDisplayId]);           // Complete the request to power rails
#if defined(ODM_PROJECT_SLM550) || defined(ODM_PROJECT_MC598) || defined(ODM_PROJECT_SNM550) || defined(ODM_PROJECT_SNM550GO)
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(SLM550_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure SLM550_LCD_1P8_EN_GPIO failed!\n"));
    } 
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(SLM550_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_LOW_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: SLM550_LCD_1P8_EN_GPIO gpioout failed!\n"));
    }
#endif

#if defined(ODM_PROJECT_SLM560D)
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(SLM560D_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure SLM560D_LCD_1P8_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(SLM560D_LCD_1P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_LOW_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: SLM560D_LCD_1P8_EN_GPIO gpioout failed!\n"));
    }

    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(SLM560D_LCD_2P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA), TLMM_GPIO_ENABLE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: Configure SLM560D_LCD_2P8_EN_GPIO failed!\n"));
    }
    if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(SLM560D_LCD_2P8_EN_GPIO, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_16MA), GPIO_LOW_VALUE))
    {
      DEBUG((EFI_D_WARN, "DisplayDxe: SLM560D_LCD_2P8_EN_GPIO gpioout failed!\n"));
    }
#endif
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

#if defined(ODM_PROJECT_HXB_SNM550D_BaoJian)
    /* GPIO82 reset done above; init LT8912 over I2C once */
    if (EFI_SUCCESS != TLMMProtocol->ConfigGpio((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), TLMM_GPIO_ENABLE))
        {
                DEBUG((EFI_D_ERROR, "DisplayDxe: Configure GPIO %d for gpio82 Failed!\n", 82));
        }
        MDP_OSAL_DELAYMS(100);
        if (EFI_SUCCESS != TLMMProtocol->GpioOut((UINT32)EFI_GPIO_CFG(82, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_HIGH_VALUE))
        {
                DEBUG((EFI_D_ERROR, "DisplayDxe: gpio82 line HIGH failed!\n"));
        }
        DEBUG((EFI_D_ERROR, " lt8912 reset gpio up \n"));
        MDP_OSAL_DELAYMS(100);
    lt8912_init();
#endif

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
     if (EFI_SUCCESS != PmicVersionProtocol->GetPmicInfo(PMIC_A, &PmicDeviceInfo))
     {
        DEBUG((EFI_D_ERROR, "DisplayDxe: GetPmicInfo failed!\n"));
        eStatus = MDP_STATUS_NO_RESOURCES;
     }
     else
     {
        if (PmicDeviceInfo.PmicModel == EFI_PMIC_IS_PM2250)
        {
          pPowerParams->ePMICSecondaryPower[eDisplayId] = MDP_PMIC_MODULE_CONTROLTYPE_IBB_LAB_LCD; 
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
  EFI_QCOM_PMIC_PWM_PROTOCOL  *PMICPWMProtocol  = NULL;
  EFI_STATUS                   eLocalStatus     = EFI_SUCCESS;

#if defined(ODM_PROJECT_MT5205)
  EFI_QCOM_PMIC_GPIO_PROTOCOL *PmicGpioProtocol = NULL;
  /* MT5205: mux PM4125 GPIO2 -> PWM SPECIAL_FUNCTION1 (not PMI632 GPIO6 WLED) */
  if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicGpioProtocolGuid, NULL, (VOID **)&PmicGpioProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMIC GPIO protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  }
  else
  {
    eLocalStatus = PmicGpioProtocol->ConfigDigitalOutput(PMIC_A,
                                                    EFI_PM_GPIO_2,
                                                    EFI_PM_GPIO_OUT_BUFFER_CONFIG_CMOS,
                                                    EFI_PM_GPIO_VIN0,
                                                    EFI_PM_GPIO_SOURCE_SPECIAL_FUNCTION1,
                                                    EFI_PM_GPIO_OUT_BUFFER_HIGH, 0);
    if (EFI_SUCCESS != eLocalStatus)
    {
      DEBUG((EFI_D_ERROR, "DisplayDxe: MT5205 PM4125 GPIO2 SPECIAL_FUNCTION1 failed - %d!\n", eLocalStatus));
      Status = MDP_STATUS_FAILED;
    }
  }
#endif

  /* Configure PWM */
  if (EFI_SUCCESS != gBS->LocateProtocol(&gQcomPmicPwmProtocolGuid, NULL, (void **)&PMICPWMProtocol))
  {
    DEBUG((EFI_D_ERROR, "DisplayDxe: Locate PMICPWM Protocol failed!\n"));
    Status = MDP_STATUS_NO_RESOURCES;
  } 
  else
  {
    eLocalStatus = PMICPWMProtocol->PwmConfig(0,
                                              EFI_PM_PWM_RSRC_PWM_3,
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
          if (EFI_SUCCESS != PmicLCDBProtocol->SetOutput(PMIC_A,5500,-5500,TRUE))
          {
            DEBUG((EFI_D_ERROR, "DisplayDxe: Error to enable LCDB\n"));
          }

          if (EFI_SUCCESS != PmicLCDBProtocol->ConfigEnableCtrl(PMIC_A, EFI_PM_LCDB_ENABLE_CTL_MODULE_EN, TRUE))
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
        if (EFI_SUCCESS != PmicLCDBProtocol->ConfigEnableCtrl(PMIC_A, EFI_PM_LCDB_ENABLE_CTL_MODULE_EN, FALSE))
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
                                                EFI_PM_PWM_RSRC_PWM_3,
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
      if (EFI_SUCCESS == PmicLCDBProtocol->VregOk(PMIC_A, &lcdbStatus))
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
