/**
* @file PcieConfigLib.c
* Shim file to bridge between PCIe root port lib and UEFI drivers
*
* Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
* All rights reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*
*/


/*=============================================================================
                              EDIT HISTORY


 when       who      what, where, why
 --------   ---      ----------------------------------------------------------
 05/10/24   sh       Pcie port 0 enablement
 04/19/21   AS       Created (Derived from Makena and updated for Kodiak)
 12/18/20   yg       Migrated to new interface
 07/31/20   yg       Keep EP MSI's disabled
 04/10/20   ts       First port for SC8280X
 06/25/19   ts       Optimized boot time with just one root port wait time
                     Included for CLS subtype 1 platform type
 01/31/19   ts       Register for PCIe ExitBootServices callback
 10/23/18   ts       Register for a callback to update AML variable based on
                     the bifurcation configuration
 07/16/18   ts       Supporting PCIe LPM mode for off mode charging
 05/16/18   ts       Deferred L1ss config to a 100 ms timer callback
 11/15/17   ts       called SMMU Config from PcieConfigLibEnableRootports
 05/22/17   ts       Fixed UpdatePortInfo function for possible memory
                     corruption
 05/05/17   ts       Updated the global port info structure to take in account
                     the removed ports
 07/28/16   ts       Register for a callback to update AML variable on PCIe link
                     state and powered down port PHY in case of link training
                     failure
 07/28/16   ts       Included I/O port access support and removed Address
                     resource configuration for End points
 07/15/16   ts       Formatted strings to %a in debug prints and Implemented
                     PcieConfigLibUpdatePortInfo to update global port info
                     and as well as disable clocks and LDOs(if necessary)
 07/08/16   ts       Renamed file to PcieConfigLib.c & made changes to init
                     root ports one by one
 06/08/16   ts       merged the PERST and link training in one function
 02/08/16   ts       Added pcie xml config support
 01/11/16   ts       Register for PCIe ExitBootServices callback
 06/15/15   ts       Simplified PCIeConfigLibRead_Config function.
                     Calling setup L1ss after iATU setup.
 05/13/15   ts       Unified code for multi platforms
 03/18/15   ts       Fixed Copyrights header
 03/16/15   ts       Added warning messages and split PHY code
                     into common and port
 01/26/15   ts       Updated the code as per 8994 changes
 10/07/14   ah       Changes for 8996
 06/16/14   hk       Add PcieInit protocol
 05/07/14   hk       Turn on PCie during exitbootservices
 04/18/14   hk       First checkin for 8994
=============================================================================*/


#include <Uefi.h>
#include "pcie_dtypes.h"

#include <Library/BootConfig.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PcieConfigLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiCfgLib.h>

#include "pcie_rp_cfg_svc.h"
#include "pcie_osal.h"
#include "pcie_rp.h"
#include "PcieConfig_i.h"
#include "pcie_cap_config_utils.h"

//#define ENABLE_PERF_LOGGING
#ifdef ENABLE_PERF_LOGGING
#define PCIE_PROFILE_PERF(x)     DEBUG(x)
#else
#define PCIE_PROFILE_PERF(x)     do{}while(0)
#endif

struct pcie_nvme_controller_registers * pcie_nvme_mmio = NULL;

#define TIME_MS_TO_TIMER_100NS_UNITS(a)   (a * 10000)

/**
 *  Subtypes for H/W Variant : Kodiak IOT IDP
 *	Soc_Target  CDT  Platform_ID  Subtypes
 *  6490        IDP    34         0 - Moselle attach
 *                                1 - HSP attach
 *
 *  Subtypes for H/W Variant : RB3Gen2
 *  Soc_Target  CDT  Platform_ID  Subtypes
 *	6490        IOT	   32         1 - RB3Gen2 Video Collab Kit ( Moselle attach)
 *                                2 - RB3Gen2 Core / Vision Kit (Moselle attach)
 *                                3 - RB3Gen2 Core Kit (Moselle attach)
 *                                5 - RB3Gen2 Core Kit (Moselle attach)
 *                                6 - RB3Gen2 Core Kit (HSP attach)
 *                                7 - RB3Gen2 Vision Kit (HSP attach)
 *                                8 - RB3Gen2 Video Collab Kit (HSP attach)
 *                                9 - RB3Gen2 Industial Kit with HSP offboard on a m.2 card
 **/
static UINT32 Port0_IOTSubTypes[] = {0, 6, 7, 8, 9};
static UINT32 Port0_IDPSubTypes[] = {1};

/* Platform info with port 0 support in uefi */
static struct pcie_port_platform_config PCIE0_SupportedConfig[] = {
    { EFI_PLATFORMINFO_TYPE_IOT, sizeof(Port0_IOTSubTypes)/sizeof(UINT32), Port0_IOTSubTypes },
    { EFI_PLATFORMINFO_TYPE_IDP, sizeof(Port0_IDPSubTypes)/sizeof(UINT32), Port0_IDPSubTypes }};

BOOLEAN PcieConfigLibIsPortLinkUp (UINT32 RootPortIndex)
{
  return (pcie_is_port_link_up(RootPortIndex) != 0);
}

/**
 * The API parses through PCIE0_SupportedConfig
 * against current platform information to check if port is supported
 * */
EFI_STATUS PcieConfig_isPort0Supported(UINT8* flag)
{
#if FORCE_PORT_0_SUPPORT
  #if FORCE_PORT_0_EN
    *flag = 1;
  #else
    *flag = 0;
  #endif
  return EFI_SUCCESS;
#else
  static uint8 isPort0Supported = 0xFF;
  uint32 PlatformSubType = 0xFFFFFFFF;
  uint32 i = 0;
  struct pcie_port_platform_config* Config = NULL;

  EFI_STATUS                          Status;
  EFI_PLATFORMINFO_PLATFORM_TYPE      PlatformType = EFI_PLATFORMINFO_TYPE_UNKNOWN;
  EFI_PLATFORMINFO_PROTOCOL           *pPlatformInfo;
  EFI_PLATFORMINFO_PLATFORM_INFO_TYPE PlatformInfo;

  if(isPort0Supported != 0xFF)
  {
    *flag = isPort0Supported;
    return EFI_SUCCESS;
  }

  /* Get Platform Info */
  Status = gBS->LocateProtocol (&gEfiPlatformInfoProtocolGuid, NULL,
                                (VOID **) &pPlatformInfo);
  if (EFI_ERROR (Status)){
    DEBUG ((EFI_D_ERROR, "Error getting LocateProtocol\r\n"));
    return EFI_DEVICE_ERROR;
  }
  Status = pPlatformInfo->GetPlatformInfo (pPlatformInfo, &PlatformInfo);
  if (EFI_ERROR (Status)){
    DEBUG ((EFI_D_ERROR, "Error getting GetPlatformInfo\r\n"));
    return EFI_D_ERROR;
  }

  DEBUG ((EFI_D_INFO, "Platform type: %d subtype: %d\r\n",PlatformInfo.platform, PlatformInfo.subtype));

  PlatformType= PlatformInfo.platform;
  if ( (PlatformType >= EFI_PLATFORMINFO_NUM_TYPES) ||
        (PlatformType == EFI_PLATFORMINFO_TYPE_UNKNOWN))
  {
    DEBUG ((EFI_D_ERROR, " Error getting platform type %d\r\n", PlatformType));
    return EFI_DEVICE_ERROR;
  }

  for (i = 0; i < (sizeof(PCIE0_SupportedConfig)/sizeof(struct pcie_port_platform_config)); i++)
  {
    if(PlatformType == PCIE0_SupportedConfig[i].PlatformType)
    {
      /* Platform type matched */
      PlatformSubType = PlatformInfo.subtype;
      Config = &PCIE0_SupportedConfig[i];
      break;
    }
  }

  if(Config == NULL)
  {
    /* platform type not matched with table enrties */
    DEBUG ((EFI_D_ERROR, " Platform type = %x     Platform Sybtype = %x \r\n", PlatformType, PlatformSubType));
    *flag = isPort0Supported = 0;
    return EFI_SUCCESS;
  }

  for (i = 0; i < Config->NumSubTypes; i++)
  {
    if(PlatformSubType == Config->SupportedSubTypes[i])
    {
      *flag = isPort0Supported = 1;
      return EFI_SUCCESS;
    }
  }

  DEBUG ((EFI_D_ERROR, " Port 0 not supported! \r\n", PlatformType, PlatformSubType));
  *flag = isPort0Supported = 0;
  return EFI_SUCCESS;
#endif
}

/**
 * The API checks the current boot configuration to check if port1 is supported.
 * */
EFI_STATUS PcieConfig_isPort1Supported(UINT8* flag)
{
#if FORCE_PORT_1_SUPPORT
  #if FORCE_PORT_1_EN
    *flag = 1;
  #else
    *flag = 0;
  #endif
  return EFI_SUCCESS;
#else
  static uint8 isPort1Supported = 0xFF;
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32     PowerOnNvme = 0;
  UINT32     PowerOnPcieBridge = 0;
  BOOLEAN    BootFromNvmeFlag = FALSE;

  if(isPort1Supported != 0xFF)
  {
    *flag = isPort1Supported;
    return EFI_SUCCESS;
  }

  /* Check boot_from_nvme() first */
  BootFromNvmeFlag = boot_from_nvme();

  /* Only check config values if boot_from_nvme is FALSE */
  if (!BootFromNvmeFlag)
  {
    Status = GetConfigValue ("NvmePowerOn", &PowerOnNvme);
    if(EFI_SUCCESS != Status)
      PowerOnNvme = 0;

    Status = GetConfigValue ("PcieBridgePowerOn", &PowerOnPcieBridge);
    if(EFI_SUCCESS != Status)
      PowerOnPcieBridge = 0;
  }

  /* Port 1 is supported ONLY if any of these conditions are TRUE */
  if(BootFromNvmeFlag || PowerOnNvme || PowerOnPcieBridge)
  {
    DEBUG ((EFI_D_WARN, "PCIE Port 1 enabled: BootFromNvme=%d, PowerOnNvme=%d, PcieBridgePowerOn=%d\r\n",
            BootFromNvmeFlag, PowerOnNvme, PowerOnPcieBridge));
    isPort1Supported = 1;
  }
  else
  {
    DEBUG ((EFI_D_WARN, "PCIE Port 1 is disabled: NVMe boot not supported\r\n"));
    isPort1Supported = 0;
  }

  *flag = isPort1Supported;
  return EFI_SUCCESS;
#endif
}

VOID PcieEnableL1ssOnAllPorts (EFI_EVENT Event, VOID *Context)
{
   UINT32                   i;
   UINT32                   TotalRootPorts = 0;
   TotalRootPorts = pcie_rp_get_max_port_count ();

   for (i = 0; i < TotalRootPorts; i++)
   {
      if (pcie_is_port_link_up (i) == 0)
        continue;

      pcie_rp_enable_l1ss (i);
   }
}

/* TODO: does this really need to be timer callback based? */
EFI_STATUS
SetupL1ssConfig(void)
{
   EFI_STATUS        Status = EFI_SUCCESS;
   EFI_EVENT         L1ssEnableTimer;

   Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
                              PcieEnableL1ssOnAllPorts,
                              NULL, &L1ssEnableTimer);
   if (EFI_ERROR (Status))
   {
      DEBUG ((EFI_D_WARN, "L1ss CreateEvent Failed\r\n"));
      return Status;
   }

   Status = gBS->SetTimer (L1ssEnableTimer, TimerRelative,
                           TIME_MS_TO_TIMER_100NS_UNITS(100));

   if (EFI_ERROR (Status))
   {
      DEBUG ((EFI_D_WARN, "L1ss SetTimer failed\r\n"));
      return Status;
   }

   return Status;
}

EFI_STATUS SetupRootPort (UINT32 CurrentRootPort)
{
   EFI_STATUS Status = EFI_SUCCESS;

   Status = PcieConfigLibSetACPIPlatCallback (CurrentRootPort);

   if (pcie_rp_setup_aspm (CurrentRootPort) != PCIE_SUCCESS)
   {
      DEBUG ((EFI_D_WARN, "Setup ASPM Failed for %u\r\n", CurrentRootPort));
      return EFI_DEVICE_ERROR;
   }

   /* Initially keep MSI's disabled, later required module can enable it explicitly */
   Status = pcie_cap_disable_MSIs (CurrentRootPort);
   if (EFI_ERROR (Status))
      DEBUG ((EFI_D_WARN, "Disable MSI's Failed for %u\r\n", CurrentRootPort));

   return Status;
}

/* TODO: Target specific info, find a new home  */
#define TCSR_TCSR_REGS_REG_BASE                 (CORE_TOP_CSR_BASE            + 0x000c0000)
#define HWIO_PCIE_CTRL_4LN_CONFIG_SEL_ADDR      (TCSR_TCSR_REGS_REG_BASE          + 0xa044)

#define PCIE_PORT_0      0
#define PCIE_PORT_1      1

// Temp placeholder TODO: Move to some target/platform specific config
EFI_STATUS SetupConfig (VOID)
{
#if 0
   EFI_STATUS        Status = EFI_SUCCESS;

#define PHY_CONFIG_2A_X4    0x1
#define PHY_CONFIG_3A_X4    0x2

#define CURRENT_BOARD_LAYOUT_CONFIG    (PHY_CONFIG_2A_X4)
//#define CURRENT_BOARD_LAYOUT_CONFIG    (PHY_CONFIG_2A_X4 | PHY_CONFIG_3A_X4)

   /* Based on HW board layout if the PCIe device is connected to 4 Lanes of 2A and or 3A
    * set that config, failing to do so will result into link failure. If 4 lane config
    * is not desired, then just disabling the upper lane controllers will make sure the
    * link comes up with just 2 lanes. */

   /* Current board layout config Set 2A/3A as 4 Lanes */
   Status = write_secure_tcsr_reg_val ((UINT32*)HWIO_PCIE_CTRL_4LN_CONFIG_SEL_ADDR, PHY_CONFIG_2A_X4);
   if (EFI_ERROR (Status))
   {
      DEBUG ((EFI_D_ERROR, "Failed to set 4 Lane config reg val\r\n"));
      return Status;
   }

   /* Disable 2B,3B since the hardware board has been connected as lanes for both 2A and 3A
    * so, B ports cannot be used. In this case if we do the following above TCSR reg value
    * wouldn't matter (but enumerating in 4 lanes requires that setting) if we just want
    * to enumerate in 2 lane configuration. */
   //pcie_rp_disable_port (PCIE_PORT_2B);
   //pcie_rp_disable_port (PCIE_PORT_3B);

   return Status;
#else
   EFI_STATUS Status = EFI_SUCCESS;
   UINT8      IsPort0Enabled = 0;
   UINT8      IsPort1Enabled = 0;

   Status = PcieConfig_isPort0Supported(&IsPort0Enabled);
   if(Status != EFI_SUCCESS)
   {
     IsPort0Enabled = 0;
     DEBUG ((EFI_D_ERROR, "Unable to get port 0 hw config! \r\n"));
     Status = EFI_SUCCESS;  /* unable to get hw config : not asserting but disabling port 0 initialization */
   }

   if(IsPort0Enabled == 0)
   {
     /* Default Enabled */
     pcie_rp_disable_port (PCIE_PORT_0);
   }
   else
   {
     DEBUG ((EFI_D_ERROR, "PCIE Port 0 Configuration Found! \r\n"));
   }

   Status = PcieConfig_isPort1Supported(&IsPort1Enabled);
   if(Status != EFI_SUCCESS)
   {
     IsPort1Enabled = 0;
     DEBUG ((EFI_D_ERROR, "Unable to get port 1 hw config! \r\n"));
     Status = EFI_SUCCESS;  /* unable to get hw config : not asserting but disabling port 1 initialization */
   }

   if(IsPort1Enabled == 0)
   {
     /* Default Enabled */
     pcie_rp_disable_port (PCIE_PORT_1);
   }
   else
   {
     DEBUG ((EFI_D_ERROR, "PCIE Port 1 Configuration Found! \r\n"));
   }

   return Status;
#endif
}

/*******************************************************************************
 *
 *            Public functions
 *
 ******************************************************************************/

EFI_STATUS
EFIAPI
PcieConfigLibEnableRootPorts (VOID)
{
   UINT32            CurrentRootPort = 0;
   UINT32            TotalRootPorts = 0;
   uint32            enabled_ports;
   pcie_status_t     pStatus;

   /* Enable default config options before pcie_rp_lib_init which loads the
    * enabled configurations */
   if(SetupConfig() != EFI_SUCCESS)
   {
     PCIE_PROFILE_PERF ((EFI_D_ERROR, "SetupConfig Failed!! \r\n"));
     return EFI_DEVICE_ERROR;
   }

   /* Required config should be set now, enable lib and load config */
   if (pcie_rp_lib_init () != PCIE_SUCCESS)
      return EFI_DEVICE_ERROR;

   TotalRootPorts = pcie_rp_get_max_port_count ();

   enabled_ports = pcie_rp_get_enabled_ports_mask ();

   pStatus = pcie_rp_enumerate_ports (enabled_ports); // Pick all enabled ports to enumerate

   if (pStatus != PCIE_SUCCESS)
      return EFI_DEVICE_ERROR;

   PCIE_PROFILE_PERF ((EFI_D_ERROR, "Ports Root ports setup done : %ld\r\n", pcie_osal_get_time_ms ()));

   for (CurrentRootPort = 0; CurrentRootPort < TotalRootPorts; CurrentRootPort++)
   {
      if (((enabled_ports & (1 << CurrentRootPort)) == 0) || !pcie_is_port_link_up(CurrentRootPort))
         continue;

      SetupRootPort (CurrentRootPort);
   }

   PCIE_PROFILE_PERF ((EFI_D_ERROR, "Root ports setup done : %ld\r\n", pcie_osal_get_time_ms ()));

   PcieConfigLibSetupBattChargingHandler ();

   SetupL1ssConfig ();

   PCIE_PROFILE_PERF ((EFI_D_ERROR, "Root ports enable complete : %ld\r\n", pcie_osal_get_time_ms ()));

   return EFI_SUCCESS;
}

EFI_STATUS ConfigLibDeinitializeAllPcieCores (VOID)
{
   if (pcie_rp_powerdown_all_ports () != PCIE_SUCCESS)
      return EFI_DEVICE_ERROR;

   return EFI_SUCCESS;
}

UINT32
PcieConfigLibGetNumberOfHostBridges (VOID)
{
   return pcie_rp_get_max_port_count ();
}

UINT32
PcieConfigLibGetNumberOfRootBridges (UINT32 RootPortIndex)
{
   /*
   * Return 1 as of now
   * This can be changed in future, if we have more than one
   * root port sharing:
   *  o PCI I/O space
   *  o PCI memory space
   *  o PCI prefetchable memory space
   */
   return 0x1;
}

EFI_STATUS
PcieConfigLibGetResourceAperture (UINT32 RootPortIndex,
    PCI_ROOT_BRIDGE_RESOURCE_APERTURE * ResAperture)
{
   const mem_region_t*        port_mem;
   const pcie_port_config_t * port_cfg;
   UINT64                     EcamSize = 0x0;

   if (ResAperture == NULL)
      return EFI_INVALID_PARAMETER;

   port_mem = pcie_rp_get_port_mem_config (RootPortIndex);
   port_cfg = pcie_rp_get_port_config(RootPortIndex);

   if (port_mem == NULL || port_cfg == NULL)
      return EFI_INVALID_PARAMETER;

   EcamSize = (port_cfg->bus_number_max + 1) * PCIE_SINGLE_BUS_ADDR_SPACE_SIZE;

   /*
   * Before proceeding further, lets define some terms of resource aperture
   *
   * Base  - Base as seen from PCIe root complex after translation
   * Start - As seen from CPU before translation
   * End   - As seen from CPU before translation
   */

   /*
   * Bus number Information
   *
   * Base  - Start bus number of this segment
   * Start - Start bus number of this segment
   * End   - End bus number of this segment
   */
   ResAperture[PciRbResTypeBus].Base = 0;
   ResAperture[PciRbResTypeBus].Start = 0;
   ResAperture[PciRbResTypeBus].End = port_cfg->bus_number_max;

   /*
   * Config Mem Address information
   *
   * Base  - DBI Base
   * Start - DBI Base
   * End   - Ecam size of maximum available bus under this segment
   */
   ResAperture[PciRbResTypeCfg].Base = port_mem[axi_reg].pa;
   ResAperture[PciRbResTypeCfg].Start = port_mem[axi_reg].pa;
   ResAperture[PciRbResTypeCfg].End = port_mem[axi_reg].pa + EcamSize - 1;

   /* IO Mapped IO Information
   *
   * Base  - Device accessible address (arbitrary 20 bit value)
   * Start - CPU accessible DBI address
   * End   - DBI end
   */
   ResAperture[PciRbResTypeIo].Base = (0x10000 * (RootPortIndex + 1));
   ResAperture[PciRbResTypeIo].Start = ResAperture[PciRbResTypeCfg].End + 1;
   ResAperture[PciRbResTypeIo].End = ResAperture[PciRbResTypeIo].Start + PCIE_SINGLE_BUS_ADDR_SPACE_SIZE - 1;

   /* Mem Mapped IO Information
   *
   * Base  - Config Mem End for this segment
   * Start - Config Mem End for this segment
   * End   - Ecam size of maximum available bus under this segment
   */
   ResAperture[PciRbResTypeMem].Base = port_mem[non_prefetch_32_reg].pa;
   ResAperture[PciRbResTypeMem].Start = port_mem[non_prefetch_32_reg].pa;
   ResAperture[PciRbResTypeMem].End = port_mem[non_prefetch_32_reg].pa +  port_mem[non_prefetch_32_reg].sz - 1;

   return EFI_SUCCESS;
}

EFI_STATUS
PcieConfigLibGetSegmentNumber (UINT32 RootPortIndex, UINT32 *SegPtr)
{
   if (pcie_rp_get_hw_idx (RootPortIndex, SegPtr) == PCIE_SUCCESS)
      return EFI_SUCCESS;

   return EFI_NOT_FOUND;
}

EFI_STATUS
PcieConfigLibSetupIoSpace (UINT32 RootPortIndex, UINT64 Base, UINT64 Limit, UINT64 IOAddr)
{
   pcie_status_t        status;

   status = pcie_rp_setup_io_space_ATU (RootPortIndex, Base, Limit, IOAddr);

   if (status != PCIE_SUCCESS)
      DEBUG ((EFI_D_ERROR, "Pcie ATU setup for IO Space Failed\r\n"));

   return EFI_SUCCESS;
}


