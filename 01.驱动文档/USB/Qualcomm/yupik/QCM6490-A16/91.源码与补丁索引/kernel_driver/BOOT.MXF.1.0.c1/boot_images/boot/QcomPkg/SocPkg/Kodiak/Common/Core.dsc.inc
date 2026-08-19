#==============================================================================
# @file Core.dsc
# Kodiak Core package.
#
# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# All rights reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.
# Portions Copyright (c) 2009 - 2010, Apple Inc. All rights reserved.
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#==============================================================================

[LibraryClasses.common]
  #
  # EDK2 Libraries
  #
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf
  NULL|QcomPkg/Library/QcomBaseLib/QcomBaseLib.inf
  NULL|QcomPkg/Library/StackCanaryBaseLib/StackCanaryBaseLibCore.inf
  NULL|QcomPkg/Library/MuslLib/MuslLib.inf

  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  RngLib|QcomPkg/SocPkg/Kodiak/Library/RNGLib/RNGLibHw_core.inf
  HashApiLib|CryptoPkg/Library/BaseHashApiLib/BaseHashApiLib.inf
  TlsLib|CryptoPkg/Library/TlsLib/TlsLib.inf
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  ArmGenericTimerCounterLib|ArmPkg/Library/ArmGenericTimerVirtCounterLib/ArmGenericTimerVirtCounterLib.inf
  ArmGicArchLib|ArmPkg/Library/ArmGicArchLib/ArmGicArchLib.inf
  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf
  ArmMmuLib|ArmPkg/Library/ArmMmuLib/ArmMmuBaseLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  BaseSynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  CpuExceptionHandlerLib|ArmPkg/Library/ArmExceptionLib/ArmRelocateExceptionLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DebugLibRuntime|QcomPkg/Library/DebugLib/DebugLibRuntime.inf
  RuntimeLoggerLib|QcomPkg/Library/RuntimeLoggerLib/RuntimeLogger.inf
  DebugLib|QcomPkg/Library/DebugLib/DebugLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DTBExtnLib|QcomPkg/Library/DTBExtnUEFILib/DTBExtnUEFILib.inf
  EblAddExternalCommandLib|EmbeddedPkg/Library/EblAddExternalCommandLib/EblAddExternalCommandLib.inf
  EblNetworkLib|EmbeddedPkg/Library/EblNetworkLib/EblNetworkLib.inf
  EfiFileLib|EmbeddedPkg/Library/EfiFileLib/EfiFileLib.inf
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf
  Zlib|QcomPkg/Library/Zlib/zlib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  LzmaDecompressLib|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  ZlibDecompressLib|QcomPkg/Library/ZlibDecompressLib/ZlibDecompressLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  #PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PowerSleepTargetLib|QcomPkg/SocPkg/Kodiak/Library/PowerSleepTargetLib/PowerSleepTargetLib.inf
  PrePiLib|EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UncachedMemoryAllocationLib|ArmPkg/Library/UncachedMemoryAllocationLib/UncachedMemoryAllocationLib.inf
  UsbSharedLib|QcomPkg/SocPkg/Kodiak/Library/UsbSharedLib/UsbSharedLibCore.inf
  UsbConfigLib|QcomPkg/SocPkg/Kodiak/Library/UsbConfigLib/UsbConfigLib.inf
  LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
 # DxeImageAuthenticationStatusLib|SecurityPkg/Library/DxeImageAuthenticationStatusLib/DxeImageAuthenticationStatusLib.inf
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
  Tpm2DeviceLib|QcomPkg/Library/Tpm2DeviceLib/Tpm2DeviceLib.inf
  Tpm2CommandLib|SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  IPCCLib|QcomPkg/Library/IPCCLib/IPCCLib.inf
#LockLib|QcomPkg/Library/LockLib/LockLibNull.inf
  LockLib|QcomPkg/Library/LockLib/LockLib.inf
  KernelLib|QcomPkg/Library/KernelLib/KernelLib.inf
  WatchDogLib|QcomPkg/Library/KernelLib/KernWDogLib.inf
  AslrLib|QcomPkg/Library/AslrLib/AslrLib.inf
  DbiDumpLib|QcomPkg/Library/DbiDumpLib/DbiDumpLib.inf
  MuslLib|QcomPkg/Library/MuslLib/MuslLib.inf
  UefiCfgLib|QcomPkg/Library/UefiConfigLib/UefiCfgLib.inf
  ShLib|QcomPkg/Library/ShLib/ShLib.inf
  RamPartitionTableLib|QcomPkg/Library/RamPartitionTableLib/RamPartitionTableLib.inf
  DDRInfoPartitionLib|QcomPkg/Library/DDRInfoPartitionLib/DDRInfoPartitionLib.inf
  PeCoffExtraActionLib|QcomPkg/Library/DebugPeCoffExtraActionLib/DebugPeCoffExtraActionLib.inf
  CookieLib|QcomPkg/Library/CookieLib/CookieLib.inf
  SDILib|QcomPkg/Library/SDILib/SDILib.inf
  DtPlatformDtbLoaderLib|QcomPkg/Override/EmbeddedPkg/Library/DxeDtPlatformDtbLoaderLibDefault/DxeDtPlatformDtbLoaderLibDefault.inf
  LibUfdt|QcomPkg/Library/LibUfdt/LibUfdt.inf
  VirtCpuLib|QcomPkg/Library/VirtCpuLib/VirtCpuLib.inf

# Minidump Trust App Loader Lib
  MinidumpTALib|QcomPkg/Library/MinidumpTALib/MinidumpTALib.inf

  UefiFeatureEnablerLoaderLib|QcomPkg/Library/UefiFeatureEnablerLoaderLib/UefiFeatureEnablerLoaderLib.inf
  AptLib|QcomPkg/Library/NullLibs/AptLibNull/AptLibNull.inf
# --PRESIL--
  #BootConfigLib|QcomPkg/Library/NullLibs/BootConfigLibNull/BootConfigLibNull.inf
  BootConfigLib|QcomPkg/Library/BootConfigLib/BootConfigLib.inf
  BdsInputLib|QcomPkg/Library/BdsInputLib/BdsInputLib.inf
  BusyWaitLib|QcomPkg/Library/BusyWaitLib/BusyWaitCoreLib.inf
  ChargerLib|QcomPkg/SocPkg/Kodiak/Library/ChargerLib/ChargerLibLa.inf
  UsbPwrCtrlLib|QcomPkg/Library/UsbPwrCtrlLib/UsbPwrCtrlLib.inf
  ChipInfoDalFwkLib|QcomPkg/Library/ChipInfoLib/ChipInfoDalFwk.inf
  ChipInfoCommonLib|QcomPkg/Library/ChipInfoLib/ChipInfoCommon.inf
  ChipInfoCoreLib|QcomPkg/Library/ChipInfoLib/ChipInfoCore.inf
  DDRTargetLib|QcomPkg/Library/NullLibs/DDRTargetLibNull/DDRTargetLibNull.inf
  EblCmdLib|QcomPkg/Library/EblCmdLib/EblCmdLib.inf
# --PRESIL--
#  EfiResetSystemLib|QcomPkg/Library/ResetSystemLib/Null/ResetSystemLib.inf
  EfiResetSystemLib|QcomPkg/Library/ResetSystemLib/LA/ResetSystemLib.inf
  FuseControlLib|QcomPkg/Library/FuseControlLib/FuseControlLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  FwCommonLib|QcomPkg/Library/FwCommonLib/FwCommonLib.inf
  FwProvisionLib|QcomPkg/Library/FwProvisionLib/FwProvisionLib.inf
  FwUpdateLib|QcomPkg/Library/FwUpdateLib/FwUpdateLib.inf
  GuidedFvLib|QcomPkg/Library/GuidedFvLib/GuidedFvLib.inf
  TrialBootInfoLib|QcomPkg/Library/TrialBootInfoLib/TrialBootInfoLib.inf
  RecoveryInfoPartitionRW|QcomPkg/Library/RecoveryInfoPartitionRW/RecoveryInfoPartitionRWCore.inf
  HALclkSharedLib|QcomPkg/Library/HALclkSharedLib/HALclkSharedLibUEFI.inf
  VcsSharedLib|QcomPkg/Library/VcsSharedLib/VcsSharedLib.inf
  HFATLib|QcomPkg/Library/HFATLib/HFATLib.inf
  ColdplugEfsLib|QcomPkg/Library/ColdplugLib/ColdplugEfsLib.inf
  ColdplugLib|QcomPkg/Library/ColdplugLib/ColdplugLib.inf
  IcbCfgLib|QcomPkg/Library/NullLibs/BusesLibNull/IcbCfgLibNull.inf
  ImageVersionLib|QcomPkg/Library/ImageVersionLib/ImageVersionLib.inf
  ImagePathUtilsLib|QcomPkg/Library/ImagePathUtilsLib/ImagePathUtilsLib.inf
  IORTLib|QcomPkg/Library/IORTLib/IORTLib.inf
  ParserLib|QcomPkg/Library/ParserLib/ParserLib.inf
  PlatformInfoCoreLib|QcomPkg/Library/PlatformInfoLib/PlatformInfoCore.inf
  PmicApiLib|QcomPkg/Library/PmicApiLib/PmicApiLib.inf
  #PrintLib|QcomPkg/Library/PrintShLib/PrintShLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  ProcLib|QcomPkg/Library/ProcLib/ProcLib.inf
  QcomBaseLib|QcomPkg/Library/QcomBaseLib/QcomBaseLib.inf
  QcomLib|QcomPkg/Library/QcomLib/QcomLib.inf
  MountFvLib|QcomPkg/Library/MountFvLib/MountFvLib.inf
  FvParserLib|QcomPkg/Library/FvParserLib/FvParserLib.inf
  FvDecompressLib|QcomPkg/Library/FvDecompressLib/FvDecompressLib.inf
  BMPLib|QcomPkg/Library/BMPLib/BMPLib.inf
  QcomUtilsLib|QcomPkg/Library/QcomUtilsLib/QcomUtilsLib.inf
  QusbLib|QcomPkg/Library/QusbLib/QusbLib.inf
  RFSLib|QcomPkg/Library/RFSLib/RFSLib.inf
  ROMFSLib|QcomPkg/Library/EfsLib/ROMFSLib.inf
  RandomStackCanaryLib|QcomPkg/Library/RandomStackCanaryLib/RandomStackCanaryLib.inf
  RealTimeClockLib|QcomPkg/Library/RealTimeClockLibLegacy/RealTimeClockLib.inf
  RpmbCgLib|QcomPkg/Library/RpmbCgLib/RpmbCgLib.inf
  RpmbLib|QcomPkg/Library/RpmbLib/RpmbLib.inf
  RpmbListenerLib|QcomPkg/Library/RpmbLib/RpmbListenerLib.inf
  ScmCmdLib|QcomPkg/Library/ScmCmdLib/ScmCmdLib.inf
  SdccCommonLib|QcomPkg/Library/SdccCommonLib/SdccCommonLib.inf
  SdccHalLib|QcomPkg/Library/SdccCommonLib/SdccHalLib/SdccHalLib.inf
  CreateSecurebootVariableLib|QcomPkg/Library/CreateSecureBootVariableLib/CreateSecurebootVariableLib.inf
  SecBootBaseLib|QcomPkg/Library/SecBootBaseLib/SecBootBaseLib.inf
  SecBootSigLib|QcomPkg/Library/SecBootSigLib/SecBootSigLib.inf
  SecHwTestLib|QcomPkg/Library/SecHwTestLib/SecHwTestLib.inf
  SerialPortLib|QcomPkg/Library/SerialPortLib/SerialPortShLib.inf
  ICBLib|QcomPkg/Library/ICBLib/ICBLib.inf
  ICBArbLib|QcomPkg/Library/ICBArbLib/ICBArbLib.inf
  CoreInitTargetLib|QcomPkg/SocPkg/Kodiak/Library/CoreInitTargetLib/CoreInitTargetLib.inf
  ServicesUtilsLib|QcomPkg/Library/ServicesLib/ServicesUtilsLib.inf
  SmBiosLib|QcomPkg/Library/SmBiosLib/SmBiosLib.inf
  SmemLib|QcomPkg/Library/SmemLib/build/SmemLibXBLCore.inf
  SpiNorCommonLib|QcomPkg/Library/SpiNorCommonLib/SpiNorCommonLib.inf
  TargetResetLib|QcomPkg/Library/TargetResetLib/TargetResetLib.inf
  TestInterfaceLib|QcomPkg/Library/TestInterfaceLib/TestInterfaceLib.inf
  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf
  TimerSleepLib|QcomPkg/Library/TimerSleepLib/TimerSleepLib.inf
  TimetickLib|QcomPkg/Library/TimetickLibB/TimetickLib.inf
  TimetickRunTimeLib|QcomPkg/Library/TimetickLibB/TimetickRunTimeLib.inf
  TlmmLib|QcomPkg/Library/TlmmLib/TlmmLib.inf
  ToolSupportLib|QcomPkg/Library/ToolSupportLib/ToolSupportLib.inf
  TzRngLib|QcomPkg/Library/TzRngLib/TzRngLib.inf
  TzRuntimeLib|QcomPkg/Library/TzRuntimeLib/TzRuntimeLib.inf
  UfsCommonLib|QcomPkg/Library/UfsCommonLib/UfsCommonLib.inf
  StorageProfileLib|QcomPkg/Library/StorageCommonLib/StorageProfile.inf
  StorageBootCommonLib|QcomPkg/Library/StorageCommonLib/StorageCoreCommon.inf
  ShmBridgeLib|QcomPkg/Library/ShmBridgeLib/ShmBridgeLib.inf
  BamLib|QcomPkg/Library/BamLib/BamCoreLib.inf
  BamCoreLib|QcomPkg/Library/BamLib/BamCoreLib.inf
  SPMIApiLib|QcomPkg/Library/SPMIApiLib/SPMIApiLib.inf
  SPMIRuntimeLib|QcomPkg/Library/SPMIRuntimeLib/SPMIRuntime.inf
  ULogApiLib|QcomPkg/Library/ULogApiLib/ULogApiLib.inf
  CoreUtilsLib|QcomPkg/Library/CoreUtilsLib/CoreUtilsLib.inf
  DALModEnvLib|QcomPkg/Library/DALModEnvLib/DALModEnvLib.inf
  DALCommonDxeLib|QcomPkg/Library/DALCommonLib/DALCommonDxeLib.inf
  DALFwkLib|QcomPkg/Library/DALFwkLib/DALFwkLib.inf
  MDPLib|QcomPkg/Library/MDPLib/MDPLib.inf
  HALMDSSLib|QcomPkg/Library/HALMDSSLib/HALMDSS72x.inf
  HALDSILib|QcomPkg/Library/HALDSILib/HALDSILib72x.inf
  ExternalDisplayLib|QcomPkg/Library/ExternalDisplayLib/ExternalDisplayLib.inf
  GpiLib|QcomPkg/Library/GpiLib/GpiLib.inf
  GpiDrvLib|QcomPkg/SocPkg/Library/GpiDrvLib/GpiDrvLib.inf
  QupCommonLib|QcomPkg/Library/QupCommonLib/QupCommonLib.inf
  QUPFWImageLib|QcomPkg/SocPkg/Kodiak/Settings/QUP/QUPFWImageLib.inf
  I2CLib|QcomPkg/Library/I2CLib/I2CLib.inf
  I2CApiLib|QcomPkg/Library/I2CApiLib/I2CApiLib.inf
  SPILib|QcomPkg/Library/SPILib/SPILib.inf
  SPIApiLib|QcomPkg/Library/SPIApiLib/SPIApiLib.inf
  HSUartCoreLib|QcomPkg/Library/HSUartQupv3Lib/HSUartCoreLib.inf
  NpaApiLib|QcomPkg/Library/NpaApiLib/NpaApiLib.inf
  ClockApiLib|QcomPkg/Library/ClockApiLib/ClockApiLib.inf
  CmdDbApiLib|QcomPkg/Library/CmdDbApiLib/CmdDbApiLib.inf
  PwrUtilsApiLib|QcomPkg/Library/PwrUtilsApiLib/PwrUtilsApiLib.inf
  RpmhApiLib|QcomPkg/Library/RpmhApiLib/RpmhApiLib.inf
  QdssLib|QcomPkg/Library/QdssLib/QdssLib.inf
  AdcDxeWrapperLib|QcomPkg/Library/PmicLib/adc/build/AdcDxeWrapperLib.inf
  HWConfigLib|QcomPkg/Library/HWConfigLib/HWConfigLib.inf
  UCLib|QcomPkg/Library/UCLib/UCLib_core.inf
  PmicLib|QcomPkg/SocPkg/Kodiak/Library/PmicLib/PmicLib.inf
  PrngLib|QcomPkg/SocPkg/Kodiak/Library/RNGLib/RNGLibHw_core.inf
#QdssStmTargetLib|QcomPkg/SocPkg/Library/QdssStmTargetLib/QdssStmTargetLib.inf
#QdssTFunnelTargetLib|QcomPkg/SocPkg/Library/QdssTmcTargetLib/QdssTmcTargetLib.inf
#QdssTmcTargetLib|QcomPkg/SocPkg/Library/QdssTFunnelTargetLib/QdssTFunnelTargetLib.inf
  QusbDciCommonLib|QcomPkg/SocPkg/Kodiak/Library/QusbDciCommonLib/QusbDciCommonLib.inf
  QusbLdrLib|QcomPkg/SocPkg/Kodiak/Library/QusbLdrLib/QusbLdrLib.inf
  QusbTargetLib|QcomPkg/SocPkg/Kodiak/Library/QusbTargetLib/QusbTargetLib.inf
  SdccClockLib|QcomPkg/SocPkg/Kodiak/Library/SdccClockLib/SdccCoreClockLib.inf
  SdccTargetLib|QcomPkg/SocPkg/Kodiak/Library/SdccTargetLib/SdccCoreTargetLib.inf
  SpinorCoreTargetLib|QcomPkg/SocPkg/Library/SpinorTargetLib/SpinorCoreTargetLib.inf
  SpinorTargetLib|QcomPkg/SocPkg/Kodiak/Library/SpinorTargetLib/loader/SpinorTargetLib.inf
  TLMMTargetLib|QcomPkg/SocPkg/Kodiak/Library/TLMMTargetLib/TLMMTargetLib.inf
  UfsTargetUefiLib|QcomPkg/SocPkg/Kodiak/Library/UfsTargetLib/UfsTargetUefiLib.inf
#UsbWrapperLayerLib|QcomPkg/SocPkg/Library/UsbWrapperLayerLib/UsbWrapperLayerLib.inf
  HALclkLib|QcomPkg/SocPkg/Kodiak/Library/HALclkLib/HALclkLibXBL.inf
  PmicShutdownLibBoot|QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/shutdown/pm_shutdown.inf
  SmemTargetLib|QcomPkg/SocPkg/Library/SmemTargetLib/SmemTargetLib.inf
  DALGenLibCore|QcomPkg/SocPkg/Kodiak/Settings/DALConfig/DALGenLibCore_LA.inf
  DALGenLibTools|QcomPkg/SocPkg/Kodiak/Settings/DALConfig/DALGenLibTools.inf
  DALConfigLib|QcomPkg/SocPkg/Kodiak/Settings/DALConfig/DALConfigCoreLib_LA.inf
  DALConfigToolsLib|QcomPkg/SocPkg/Kodiak/Settings/DALConfig/DALConfigToolsLib.inf
  MDPPlatformLib|QcomPkg/SocPkg/Kodiak/Library/MDPPlatformLib/MDPPlatformLib.inf
  BamTargetLib|QcomPkg/SocPkg/Library/BamTargetLib/BamTargetLib.inf
  SPMILib|QcomPkg/SocPkg/Kodiak/Library/SPMILib/SPMILib.inf
  PilProxyVoteLib|QcomPkg/SocPkg/Kodiak/Library/PilProxyVoteLib/PilProxyVoteLib.inf
  BlkCryptoLib|QcomPkg/Library/BlkCryptoLib/BlkCryptoLib.inf
  BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  FmpDeviceLib|QcomPkg/Library/FmpDeviceLib/FmpDeviceLib.inf
  EdkiiSystemCapsuleLib|QcomPkg/Library/EdkiiSystemCapsuleLib/EdkiiSystemCapsuleLib.inf
  #FmpAuthenticationLib|MdeModulePkg/Library/FmpAuthenticationLibNull/FmpAuthenticationLibNull.inf
  FmpAuthenticationLib|QcomPkg/Library/FmpAuthenticationLibPkcs7/FmpAuthenticationLibPkcs7.inf
  #DisplayUpdateProgressLib|MdeModulePkg/Library/DisplayUpdateProgressLibGraphics/DisplayUpdateProgressLibGraphics.inf

  # Variables Helper
  VariablesHelperLib|QcomPkg/Library/VariablesHelperLib/VariablesHelperLibLA.inf
 # ADSP Target Library
  ADSPTargetLib|QcomPkg/SocPkg/Library/ADSPTargetLib/ADSPTargetLib.inf

# Pcie Library
  PcieConfigLib|QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.inf
  PcieCoreConfigLib|QcomPkg/SocPkg/Kodiak/Settings/PCIE/core/PcieCoreConfigLib.inf


 # meige boardid Library
  MeigeBoardIdLib|QcomPkg/Library/MeigeBoardIdLib/MeigeBoardIdLib.inf

[LibraryClasses.Arm]
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf

[LibraryClasses.AARCH64]
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf

[LibraryClasses.common.SEC]
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLibBase.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  HobLib|EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  PrePiHobListPointerLib|EmbeddedPkg/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  BootConfigLib|QcomPkg/Library/BootConfigLib/BootConfigLibSec.inf
  DebugLib|QcomPkg/Library/DebugLib/DebugLib.inf
  PrintHostLib|QcomPkg/Library/PrintHostLib/PrintHostLib.inf
  ProcLib|QcomPkg/Library/ProcLib/ProcLib.inf
  MinidumpLib|QcomPkg/Library/MinidumpLib/MinidumpLibCore.inf
  DTBExtnSecLib|QcomPkg/Library/DTBExtnLib/DTBExtnSecLib.inf
# --PRESIL--
  #SecFuseLib|QcomPkg/Library/NullLibs/SecFuseLibNull/SecFuseLibNull.inf
  SecFuseLib|QcomPkg/Library/SecFuseLib/SecFuseLib.inf
# --PRESIL--
#  SerialPortLib|QcomPkg/Library/SerialPortLibT32/SerialPortLib.inf
  SerialPortLib|QcomPkg/Library/SerialPortLib/SerialPortLib.inf
  UartLib|QcomPkg/Library/UartQupv3Lib/UartCoreLib.inf
  UartSettingsLib|QcomPkg/SocPkg/Kodiak/Settings/UART/UartSettingsLib.inf
  ShLib|QcomPkg/Library/ShLib/ShLibMgr.inf
  DepLib|QcomPkg/Library/DepLib/DepLib.inf
  PrePiMemoryAllocationLib|QcomPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  MemoryAllocationLib|QcomPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  AllocPagePoolLib|QcomPkg/Library/AllocPagePoolLib/AllocPagePoolLib.inf
  TargetInitLib|QcomPkg/SocPkg/Kodiak/Library/TargetInitLib/TargetInitLib.inf

# --PRESIL--
#SplitBootLib|QcomPkg/Library/SplitBootLib/SplitBootNullLib.inf
  SplitBootLib|QcomPkg/Library/SplitBootLib/SplitBootLib.inf
  SystemSleepLib|QcomPkg/Library/SystemSleepLib/SystemSleepLib.inf
  #WatchDogLib|QcomPkg/SocPkg/Library/WatchDogLib/WatchDogLib.inf
  WatchDogLib|QcomPkg/SocPkg/Library/VirtualWatchDogLib/VirtualWatchDogLib.inf

[LibraryClasses.ARM.SEC]
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf

[LibraryClasses.AARCH64.SEC]
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf

[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

[LibraryClasses.common.DXE_CORE]
  PeCoffExtraActionLib|ArmPkg/Library/DebugPeCoffExtraActionLib/DebugPeCoffExtraActionLib.inf
  PeCoffLib|EmbeddedPkg/Library/DxeHobPeCoffLib/DxeHobPeCoffLib.inf
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  PerformanceLib|MdeModulePkg/Library/DxeCorePerformanceLib/DxeCorePerformanceLib.inf
  CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
  DebugLib|QcomPkg/Library/DebugLib/DebugLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf


[LibraryClasses.common.DXE_DRIVER]
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  GenericBdsLib|QcomPkg/Library/GenericBdsLib/GenericBdsLib.inf
  PlatformBdsLib|QcomPkg/Library/PlatformBdsLib/PlatformBdsLib.inf
  BdsTargetLib|QcomPkg/SocPkg/Library/BdsTargetLib/BdsTargetLib.inf
  QcomBdsLib|QcomPkg/Library/QcomBdsLib/QcomBdsLib.inf
  #CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  CapsuleLib|QcomPkg/Library/DxeCapsuleLibFmp/DxeCapsuleLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  OfflineCrashDumpLib|QcomPkg/Library/OfflineCrashDumpLib/OfflineCrashDumpDxeLib.inf
#VirtCpuLib|QcomPkg/Library/VirtCpuLib/VirtCpuNullLib.inf
  VirtCpuLib|QcomPkg/Library/VirtCpuLib/VirtCpuLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  DebugLibRuntime|QcomPkg/Library/DebugLib/DebugLibRuntime.inf
  RuntimeLoggerLib|QcomPkg/Library/RuntimeLoggerLib/RuntimeLogger.inf

[LibraryClasses.common.UEFI_APPLICATION]
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  OfflineCrashDumpLib|QcomPkg/Library/OfflineCrashDumpLib/OfflineCrashDumpDxeLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  #CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  CapsuleLib|QcomPkg/Library/DxeCapsuleLibFmp/DxeRuntimeCapsuleLib.inf
  GenericBdsLib|QcomPkg/Library/GenericBdsLib/GenericBdsLib.inf
  PeCoffLib|EmbeddedPkg/Library/DxeHobPeCoffLib/DxeHobPeCoffLib.inf
  OfflineCrashDumpLib|QcomPkg/Library/OfflineCrashDumpLib/OfflineCrashDumpDxeLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/RuntimeDxeReportStatusCodeLib/RuntimeDxeReportStatusCodeLib.inf

[BuildOptions.ARM]
  # to enable thumb we need to evaluate if size is more advantage or speed
  RVCT:*_*_ARM_ARCHCC_FLAGS  == --cpu Cortex-A53 --thumb --fpu vfpv3
  RVCT:*_*_ARM_ARCHASM_FLAGS == --cpu Cortex-A53

[BuildOptions.AARCH64]
# --PRESIL-- Disable ASLR
  GCC:*_*_AARCH64_ARCHCC_FLAGS  == -fno-common -mtune=cortex-a53 -I$(WORKSPACE)/QcomPkg/SocPkg/Include -I$(WORKSPACE)/QcomPkg/SocPkg/Kodiak/Include -include $(WORKSPACE)/QcomPkg/Include/Library/DebugLib.h -DQCOM_EDK2_PATCH -DDISABLE_DEP $(FEATURE_FLAGS) -DENABLE_XN -DENABLE_DEP_64 -DENABLE_ASLR -DENABLE_EXEC_CODE_READY_TO_BOOT -DRUNTIME_ENABLED
  GCC:*_*_AARCH64_ARCHPP_FLAGS  == -I$(WORKSPACE)/QcomPkg/SocPkg/Include -I$(WORKSPACE)/QcomPkg/SocPkg/Kodiak/Include -include $(WORKSPACE)/QcomPkg/Include/Library/DebugLib.h -DQCOM_EDK2_PATCH -DDISABLE_DEP $(FEATURE_FLAGS) -DENABLE_XN -DENABLE_DEP_64 -DENABLE_ASLR -DENABLE_EXEC_CODE_READY_TO_BOOT -DRUNTIME_ENABLED

!if $(PRODMODE) == "PRODMODE"
  RVCT:*_*_*_CC_FLAGS = -DPRODMODE
  GCC:*_*_*_CC_FLAGS = -DPRODMODE
!endif

!ifdef $(PRE_SIL)
  GCC:*_*_*_CC_FLAGS = -DPRE_SIL
!endif

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]
  gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnostics2Disable|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwarePerformanceDataTableS3Support|FALSE

  #
  # Control what commands are supported from the UI
  # Turn these on and off to add features or save size
  #
  gEmbeddedTokenSpaceGuid.PcdEmbeddedDirCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedHobCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedHwDebugCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPciDebugCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedIoEnable|FALSE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedScriptCmd|FALSE

  gEmbeddedTokenSpaceGuid.PcdCacheEnable|TRUE

  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdTurnOffUsbLegacySupport|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|TRUE

  ## Indicates if StatusCode is reported via Serial port.
  #   TRUE  - Reports StatusCode via Serial port.
  #   FALSE - Does not report StatusCode via Serial port.
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseSerial|TRUE

  ## Indicates if StatusCode is stored in memory.
  #  The memory is boot time memory in PEI Phase and is runtime memory in DXE Phase.
  #   TRUE  - Stores StatusCode in memory.
  #   FALSE - Does not store StatusCode in memory.
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeUseMemory|FALSE

  ## Indicates if PEI phase StatusCode will be replayed in DXE phase.
  #   TRUE  - Replays PEI phase StatusCode in DXE phased.
  #   FALSE - Does not replay PEI phase StatusCode in DXE phase.
  gEfiMdeModulePkgTokenSpaceGuid.PcdStatusCodeReplayIn|FALSE

  # HW Watchdog
  gQcomTokenSpaceGuid.ForceEnableHWWdog|FALSE

  #
  # Target board Specific PCDs
  #

# TIANOCORE UPDATE: ShellPkg Disabled
#  gEfiShellPkgTokenSpaceGuid.PcdShellForceNoMapNoStartup|TRUE

  gArmTokenSpaceGuid.PcdArmGicV3WithV2Legacy|FALSE

[PcdsFixedAtBuild.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"Qualcomm Technologies, Inc."
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"6.0"

  gEmbeddedTokenSpaceGuid.PcdEmbeddedPrompt|"EBL"
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuMemorySize|48
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|0
  gQcomTokenSpaceGuid.PcdCpuStackTotal|0x40000
  gQcomTokenSpaceGuid.PcdMaxSupportedCpu|0x8
  gQcomTokenSpaceGuid.PcdPerClusterCoreMask|0x8
  gQcomTokenSpaceGuid.PcdMaxCoreMask|0xFF
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdSpinLockTimeout|10000000
  gEfiMdePkgTokenSpaceGuid.PcdDebugClearMemoryValue|0xAF

  # Disable perf
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdPostCodePropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|320

  # PcdDebugPropertyMask - From DebugLib.h:
  #   DEBUG_ASSERT_ENABLED       0x01
  #   DEBUG_PRINT_ENABLED        0x02
  #   DEBUG_CODE_ENABLED         0x04
  #   CLEAR_MEMORY_ENABLED       0x08
  #   ASSERT_BREAKPOINT_ENABLED  0x10
  #   ASSERT_DEADLOOP_ENABLED    0x20
!if $(PRODMODE) == "PRODMODE"
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2b
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2f
!endif

  # Pcd{Fixed}DebugPrintErrorLevel - From DebugLib.h:
  #   DEBUG_INIT      0x00000001  // Initialization
  #   DEBUG_WARN      0x00000002  // Warnings
  #   DEBUG_LOAD      0x00000004  // Load events
  #   DEBUG_FS        0x00000008  // EFI File system
  #   DEBUG_POOL      0x00000010  // Alloc & Free's
  #   DEBUG_PAGE      0x00000020  // Alloc & Free's
  #   DEBUG_INFO      0x00000040  // Verbose
  #   DEBUG_DISPATCH  0x00000080  // PEI/DXE Dispatchers
  #   DEBUG_VARIABLE  0x00000100  // Variable
  #   DEBUG_BM        0x00000400  // Boot Manager
  #   DEBUG_BLKIO     0x00001000  // BlkIo Driver
  #   DEBUG_NET       0x00004000  // SNI Driver
  #   DEBUG_UNDI      0x00010000  // UNDI Driver
  #   DEBUG_LOADFILE  0x00020000  // UNDI Driver
  #   DEBUG_EVENT     0x00080000  // Event messages
  #   DEBUG_GCD       0x00100000  // Global Coherency Database changes
  #   DEBUG_CACHE     0x00200000  // Memory range cachability changes
  #   DEBUG_VERBOSE   0x00400000  // Detailed debug messages that may significantly impact boot performance
  #   DEBUG_ERROR     0x80000000  // Error
!if $(PRODMODE) == "PRODMODE"
  # Only enable Errors and Boot Manager if Production Mode Flag is set
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000400
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0x80000400
!else
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x800fee0f
  gEfiMdePkgTokenSpaceGuid.PcdFixedDebugPrintErrorLevel|0x800fee0f
!endif

  # PcdReportStatusCodePropertyMask - From ReportStatusCodeLib.h:
  #   REPORT_STATUS_CODE_PROPERTY_PROGRESS_CODE_ENABLED   0x00000001
  #   REPORT_STATUS_CODE_PROPERTY_ERROR_CODE_ENABLED      0x00000002
  #   REPORT_STATUS_CODE_PROPERTY_DEBUG_CODE_ENABLED      0x00000004
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x06

  gEmbeddedTokenSpaceGuid.PcdEmbeddedAutomaticBootCommand|""
  gEmbeddedTokenSpaceGuid.PcdEmbeddedDefaultTextColor|0x07
  gEmbeddedTokenSpaceGuid.PcdEmbeddedMemVariableStoreSize|0x10000

  gEmbeddedTokenSpaceGuid.PcdFlashFvMainBase|0
  gEmbeddedTokenSpaceGuid.PcdFlashFvMainSize|0

# Disable UEFI DEP
  gEfiMdeModulePkgTokenSpaceGuid.PcdImageProtectionPolicy|0x0

#
# Optional feature to help prevent EFI memory map fragments
# Turned on and off via: PcdPrePiProduceMemoryTypeInformationHob
# Values are in EFI Pages (4K). DXE Core will make sure that
# at least this much of each type of memory can be allocated
# from a single memory range. This way you only end up with
# maximum of two fragements for each type in the memory map
# (the memory used, and the free memory that was prereserved
# but not used).
#
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|80
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|40
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|800
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|10
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0

#
# SMBIOS related
#
  # OEM's can set their product info here
  gQcomTokenSpaceGuid.PcdSystemMfrStr|"Qualcomm"
  gQcomTokenSpaceGuid.PcdSystemProductNameStr|""
  gQcomTokenSpaceGuid.PcdSystemProductFamilyStr|""

#
# ACPI related
#
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiExposedTableVersions|0xC
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|"QCOM  "
  #Signature_64('Q','C','O','M','E','D','K','2') = 0x324B44454D4F4351 = '2','K','D','E','M','O','C','Q'
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemTableId|0x324B44454D4F4351
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00000001
  #Signature_32('Q', 'C', 'O', 'M') = 0x4D4F4351
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorId|0x4D4F4351
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorRevision|0x00000001
  gQcomTokenSpaceGuid.DisableWriteProtect|TRUE


#
# Display PCDs
# Read QcomPkg/Docs/DisplayConfiguration.txt for PCDs configuration details.
#
  gQcomTokenSpaceGuid.PcdDisplayForceSwRenderer|FALSE
  # gFBResLimit Table 0: 4k, 1: UHD, 2: WQHD, 3: WUXGA, 4: FHD, 5: HD
  gQcomTokenSpaceGuid.PcdFrameBufMaxRes|0
  # External display type:  0: No external display, 1: DisplayPort.
  gQcomTokenSpaceGuid.PcdExtDisplayType|0
  gQcomTokenSpaceGuid.PcdBacklightLevel|50
  gQcomTokenSpaceGuid.PcdPresentationMode|0
  # No charger policy used for display
  gQcomTokenSpaceGuid.PcdDisplayChargerPolicy|0
  # Enable PFS for display
  gQcomTokenSpaceGuid.PcdPrimaryFlags|0x11000000

#
# LPM PCDs
#

 gQcomTokenSpaceGuid.PcdLpm|0x030580A5
  #
  # Crypto PCDs
  #
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.HmacSha256.Family                        | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.HmacSha384.Family                        | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Pkcs.Services.Pkcs1v2Encrypt             | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Pkcs.Services.Pkcs5HashPassword          | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Pkcs.Services.Pkcs7Verify                | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Pkcs.Services.VerifyEKUsInPkcs7Signature | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Pkcs.Services.Pkcs7GetSigners            | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Pkcs.Services.Pkcs7FreeSigners           | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Pkcs.Services.AuthenticodeVerify         | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Random.Family                            | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Rsa.Services.Pkcs1Verify                 | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Rsa.Services.New                         | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Rsa.Services.Free                        | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Rsa.Services.SetKey                      | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Rsa.Services.GetPublicKeyFromX509        | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Sha1.Family                              | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Sha256.Family                            | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Sha384.Family                            | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Sha512.Family                            | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Sha256.Services.HashAll                  | FALSE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.X509.Services.GetSubjectName             | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.X509.Services.GetCommonName              | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.X509.Services.GetOrganizationName        | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.X509.Services.GetTBSCert                 | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Tls.Family                               | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.TlsSet.Family                            | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.TlsGet.Family                            | PCD_CRYPTO_SERVICE_ENABLE_FAMILY
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Aes.Services.GetContextSize              | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Aes.Services.Init                        | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Aes.Services.CbcEncrypt                  | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Aes.Services.CbcDecrypt                  | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.AeadAesGcm.Services.Encrypt              | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.AeadAesGcm.Services.Decrypt              | TRUE
  gEfiCryptoPkgTokenSpaceGuid.PcdCryptoServiceFamilyEnable.Hkdf.Family                              | PCD_CRYPTO_SERVICE_ENABLE_FAMILY


#
# Target Specific PCDs
#

  #########################################################################
  #                               Memory Map
  #########################################################################

  gQcomTokenSpaceGuid.PcdMaxMemory|0x0

  #################################
  ## Presilicon definitions.
  #################################
  gQcomTokenSpaceGuid.PcdMemoryBase|0x80000000    # Starting address
  gQcomTokenSpaceGuid.PcdMemorySize|0x60000000    # 1.5 GB

  #################################
  ## HLOS Base Offset and size.
  #################################
  gQcomTokenSpaceGuid.PcdHLOSMemoryBaseOffset|0x98200000 # remaining memory after this is HLOS

  #################################
  ## UEFI Memory Base Offset and Size.
  #################################
  gQcomTokenSpaceGuid.PcdUefiMemPoolBaseOffset|0x94E00000
  gQcomTokenSpaceGuid.PcdUefiMemPoolSize|0x03300000     # 51 MB
  #Offset to UEFI stack base
  gQcomTokenSpaceGuid.PcdStackBaseOffset|0x00590000

  # The HOB list will start here.
  # NOTE: Hob.c is still referencing PcdPrePiHobBase
  gQcomTokenSpaceGuid.PcdPrePiHobBase|0x94E00000

  # Offset to UEFI memory
  gQcomTokenSpaceGuid.PcdPreAllocatedMemorySize|0x00000000


  #################################
  ## FD Base offset (refer to .fdf for FD size)
  #################################
  gQcomTokenSpaceGuid.PcdEmbeddedFdBaseAddress|0x9FB00000

  #################################
  ## Shared Memory Base offset and size.
  ## TODO: PcdSmemBaseAddress is currently used by smem library
  #################################
  gQcomTokenSpaceGuid.PcdSmemBaseAddress|0x80900000  #64-bit smem addr
  gQcomTokenSpaceGuid.PcdSmemSize|0x00200000     # 2MB
  ## Generic SMEM information that SMEM lib might need
  ## LSB represents chip family, other bytes are unused for now.
  gQcomTokenSpaceGuid.PcdSmemInformation|0x00000005  # 5 = Nazgul

  # Maximum number of memory regions defined in the above memory map
  gQcomTokenSpaceGuid.PcdMaxMemRegions|74

  #################################
  ## IMem Base Address and size.
  #################################

  gQcomTokenSpaceGuid.PcdIMemCookiesBase|0x146AA000
  gQcomTokenSpaceGuid.PcdIMemCookiesSize|0x00001000

  # Cookie (memory capture mode) Address for crash dump support
  gQcomTokenSpaceGuid.PcdHLOSCrashCookieAddr|0x146AA018

  # Uefi Debug Cookie Address and value
  gQcomTokenSpaceGuid.PcdUefiDebugCookieOffset|0x944
  gQcomTokenSpaceGuid.PcdUefiDebugCookie|0x55444247

  gArmTokenSpaceGuid.PcdCpuResetAddress|0x08000000    # warm reset should use this

  gArmPlatformTokenSpaceGuid.PcdCoreCount|8

  # Secure boot efuse register address
  gQcomTokenSpaceGuid.PcdSecbootRegAddr|0x0005E030
  gQcomTokenSpaceGuid.PcdSecbootAuthEnMask|0x20


  # Timer Debug
  gQcomTokenSpaceGuid.PcdTimerDxeDebug|FALSE

  # Timers
  gEmbeddedTokenSpaceGuid.PcdInterruptBaseAddress|0x9BC0000

  # Timers 
#--PRESIL--
  #gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|400000 
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|19200000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|29
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|30

  gArmTokenSpaceGuid.PcdGicDistributorBase|0x17A00000      # APSS_GICD_CTLR 
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x17A60000   # APSS_ALIAS0_GICR_CTLR

  gQcomTokenSpaceGuid.PcdInterruptTimerIndex|3
  gQcomTokenSpaceGuid.PcdInterruptTimerPeriod|100000
  gQcomTokenSpaceGuid.PcdTimer1IRQ|0x28
  gQcomTokenSpaceGuid.PcdTimerTestClockFreq|918000000 # Pcycles runs at 918 MHz
  gQcomTokenSpaceGuid.PcdTimerTestClockFreqDiv|64 # Pcycles divided by value
  gQcomTokenSpaceGuid.PcdQTimerAC_BasePhys|0x17C20000      # APSS_QTMR0_QTMR_AC
  gQcomTokenSpaceGuid.PcdQTimerV1BasePhys|0x17C21000       # APSS_QTMR0_F0V1_QTMR_V1
  gQcomTokenSpaceGuid.PcdQTimerV2BasePhys|0x17C22000       # APSS_QTMR0_F0V2_QTMR_V2
  gQcomTokenSpaceGuid.PcdQTimerFrame|0
  gQcomTokenSpaceGuid.PcdQTimerEnabled|1
  gQcomTokenSpaceGuid.PcdPerfomanceCounterEnd|0xFFFFFFFF
  gQcomTokenSpaceGuid.PcdPerfomanceCounterStart|0x00000000
  gQcomTokenSpaceGuid.PcdPerformanceCounterIndex|3
  gQcomTokenSpaceGuid.PcdRealTimeClockResolution|1
  gQcomTokenSpaceGuid.PcdRealTimeClockAccuracy|200000000
  gQcomTokenSpaceGuid.PcdRealTimeClockSetToZero|1

  #TODO: Need to correct based on frequency plan
  gQcomTokenSpaceGuid.PcdAppsProcFrequencyMhz|1190

  #MPM_SLEEP_TIMETICK_COUNT_VAL for time from power on
  gQcomTokenSpaceGuid.PcdTimeTickClkAddress|0x0C221000

  gQcomTokenSpaceGuid.PcdPsHoldAddress|0xC264000


 #Default target Panic behavior - Shutdown
  gQcomTokenSpaceGuid.PcdTargetPanic|0x0

  #I2C
  gQcomTokenSpaceGuid.EEPROMI2CCore|15
  gQcomTokenSpaceGuid.EEPROMSlaveAddress|0x52

  # SPI Loopback
  gQcomTokenSpaceGuid.SPILoopbackCore|10


  # SDCC
  gQcomTokenSpaceGuid.SDCCExtSlotNumber|1
  gQcomTokenSpaceGuid.SDCCEmmcSlotNumber|0
  gQcomTokenSpaceGuid.SDCCRpmbKey|{0x20, 0xAA, 0x9C, 0xF4, 0x99, 0x4D, 0xD4, 0xFE, 0xA5, 0x85, 0xBE, 0x96, 0x6, 0x1, 0xD1, 0xA9, 0xC3, 0x3, 0x4F, 0x91, 0x62, 0x7C, 0x64, 0x53, 0x38, 0xC1, 0x1F, 0xF2, 0x76, 0x4D, 0x2E, 0xC4}
  gQcomTokenSpaceGuid.SDCCEventBasedSDDetectEnabled|TRUE
  gQcomTokenSpaceGuid.SDCCDmaEnabled|TRUE
  gQcomTokenSpaceGuid.SDCCEmmcHwResetEnabled|TRUE
  gQcomTokenSpaceGuid.SDCCMaxSlotNumber|2
  gQcomTokenSpaceGuid.SDCCSdhciEnable|TRUE
  ## SDCC eMMC speed mode - DDR50: 0, HS200: 1, HS400: 2 SDR: 3 ##
  gQcomTokenSpaceGuid.SDCCEmmcSpeedMode|2
  gQcomTokenSpaceGuid.SDCCRegBase|"+PERIPH_SS_SDC1_SDCC_SDCC5_HC"

  #
  # ARM Pcds
  #
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0xA008C000

  # Timer sync not required
  gQcomTokenSpaceGuid.PcdSyncTimerToMPP|FALSE

  # UART
  gQcomTokenSpaceGuid.UartPlatform|"SDM855"

  # USB
  gQcomTokenSpaceGuid.InitUsbControllerOnBoot|FALSE
  gQcomTokenSpaceGuid.HostModeSupported|FALSE
  gQcomTokenSpaceGuid.USBSmmuStage1Supported|TRUE
  gQcomTokenSpaceGuid.UsbNoPmic|FALSE
  gQcomTokenSpaceGuid.FastbootSecPort|FALSE

#!if ( gQcomTokenSpaceGuid.PcdExtDisplayType == 1 )
  # DP over USB type-C: Enable high speed USB configuration
  gQcomTokenSpaceGuid.HighSpeedOnlySupported|FALSE
#!endif
 gQcomTokenSpaceGuid.USBU1U2Supported|FALSE

!if $(PRODMODE) == "PRODMODE"
  # Version info needed on LCD Display
  gQcomTokenSpaceGuid.VersionDisplay|FALSE
!else
  # Version info needed on debug port
  gQcomTokenSpaceGuid.VersionDisplay|TRUE
!endif

  # offset to 4KB shared memory in System IMEM             : 0x8603F000
  # offset to 200 bytes HLOS region within this 4KB memory : 0x658
  # offset 0  : 4 byte physical address for DebugLog Override
  gQcomTokenSpaceGuid.PcdDebugLogOverrideAddress|0x08642658

  # Magic signature value that is used to indicate DebugLog Override
  # capability in DebugLib of DXE
  gQcomTokenSpaceGuid.PcdDebugLogOverrideMagic|0x534B4950

  # Hard/Warm Reset
  gQcomTokenSpaceGuid.PcdUefiHardResetConfigure|TRUE
  gQcomTokenSpaceGuid.PcdUefiWarmResetConfigure|TRUE
  gQcomTokenSpaceGuid.PcdUefiHardResetEnabled|TRUE
  gQcomTokenSpaceGuid.PcdUefiWarmResetEnabled|TRUE

  # PON Debounce
  gQcomTokenSpaceGuid.PcdPONDebounce|0x00000001 # max value 0x07

  gEfiMdeModulePkgTokenSpaceGuid.PcdProgressCodeOsLoaderLoad | 0x03058000
  gEfiMdeModulePkgTokenSpaceGuid.PcdProgressCodeOsLoaderStart| 0x03058001

  # ChargerTaskSupported
  gQcomTokenSpaceGuid.PcdChargerTaskSupported|FALSE

  # Default to EL1
  gQcomTokenSpaceGuid.PcdSwitchToEL1|TRUE

[PcdsPatchableInModule]
  ## This PCD defines the Console output column and the default value is 25 according to UEFI spec
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|53

  ## This PCD defines the Console output row and the default value is 80 according to UEFI spec
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|80

  # This PCD should be set to 0 then video resolution could be at highest resolution.
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|0
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|0


[LibraryClasses.common]

  # External Display Interface Common Libraries
!if ( gQcomTokenSpaceGuid.PcdExtDisplayType == 1 )
  HALDPLib|QcomPkg/Library/HALDPLib/HALDPLib.inf
!else
  HALDPLib|QcomPkg/Library/HALDPLib/HALDPLibStub.inf
!endif


################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]
#
# SEC
#
  QcomPkg/XBLCore/XBLCore.inf {
    <LibraryClasses>
      SchedulerLib|QcomPkg/Library/SchedulerLib/SchedulerLib.inf
      FvDecompressLib|QcomPkg/Library/FvDecompressLib/PrePiFvDecompressLib.inf
      KernelLib|QcomPkg/Library/KernelLib/PrePiKernelLib.inf
  }

#
# PEI - not used on QC platform
#

#
# DXE
#
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      DebugLib|QcomPkg/Library/DebugLib/DebugLib.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
      NULL|EmbeddedPkg/Library/LzmaHobCustomDecompressLib/LzmaHobCustomDecompressLib.inf
  }
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/RscRtDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/SCHandlerRtDxe.inf
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf{
    <LibraryClasses>
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
      NULL|SecurityPkg/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf
      PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
      PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
   <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2b
  }
  QcomPkg/Drivers/SecurityDxe/SecurityDxe.inf
  SecurityPkg/Pkcs7Verify/Pkcs7VerifyDxe/Pkcs7VerifyDxe.inf
#  QcomPkg/Drivers/MorPpiDxe/MorPpiDxe.inf {
#    <LibraryClasses>
#      Tcg2PhysicalPresenceLib|SecurityPkg/Library/DxeTcg2PhysicalPresenceLib/DxeTcg2PhysicalPresenceLib.inf
#      Tcg2PpVendorLib|SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
#   <PcdsFixedAtBuild>
#      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2b
#  }
#  QcomPkg/Drivers/TrEEDxe/TpmDxe.inf
#  SecurityPkg/Tcg/Tcg2Dxe/Tcg2Dxe.inf {
#    <LibraryClasses>
#      NULL|SecurityPkg/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
#      HashLib|SecurityPkg/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterDxe.inf
#      PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
#      PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
#      Tcg2PhysicalPresenceLib|SecurityPkg/Library/DxeTcg2PhysicalPresenceLib/DxeTcg2PhysicalPresenceLib.inf
#      Tcg2PpVendorLib|SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
#   <PcdsFixedAtBuild>
#      gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2b
#  }
  QcomPkg/Drivers/SecRSADxe/SecRSADxe.inf
# QcomPkg/Drivers/ASN1X509Dxe/ASN1X509Dxe.inf
# QcomPkg/Drivers/VerifiedBootDxe/VerifiedBootDxe.inf
  QcomPkg/Drivers/VerifiedBootNullDxe/VerifiedBootNullDxe.inf
  QcomPkg/Drivers/TzDxe/ScmDxeCompat.inf
  QcomPkg/Drivers/TzDxe/TzDxeLA.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  #MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  QcomPkg/Drivers/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf
  QcomPkg/Drivers/VariableDxe/VariableDxe.inf {
    <LibraryClasses>
      DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  }  
  QcomPkg/Drivers/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf
  QcomPkg/Drivers/ResetRuntimeDxe/ResetRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf{
    <LibraryClasses>
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  }
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  QcomPkg/Drivers/FontDxe/FontDxe.inf
  QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf{
    <LibraryClasses>
	TLMMTargetLib|QcomPkg/SocPkg/Kodiak/Library/TLMMTargetLib/TLMMTargetLibCore.inf
  }
  QcomPkg/Drivers/EnvDxe/EnvDxe.inf {
  <LibraryClasses>
    DebugLib|QcomPkg/Library/DebugLib/DebugLib.inf
  }
  QcomPkg/Drivers/QcomWDogDxe/QcomWDogDxe.inf
  QcomPkg/Drivers/OSConfigDxe/OSConfigDxe.inf
  QcomPkg/Drivers/SmemDxe/SmemDxe.inf
  QcomPkg/Drivers/IPCCDxe/IPCCDxe.inf {
  <LibraryClasses>
  IPCCTargetLib|QcomPkg/SocPkg/Kodiak/Library/IPCCTargetLib/IPCCTargetLib.inf
  IPCCLib|QcomPkg/Library/IPCCLib/IPCCLib.inf
  }
  QcomPkg/Override/EmbeddedPkg/Drivers/DtPlatformDxe/DtPlatformDxe.inf
  QcomPkg/Drivers/HypDtFixupDxe/HypDtFixupDxe.inf
  QcomPkg/Drivers/CryptoDtFixupDxe/CryptoDtFixup.inf
  #
  # ESRTDXE
  #
  MdeModulePkg/Universal/EsrtDxe/EsrtDxe.inf
  
  #
  # Runtime Enablement
  #
  QcomPkg/Drivers/RuntimeInitDxe/RuntimeInitDxe.inf

  #
  # Various definitions related to different instances of FmpDxe
  #
  # Define ESRT GUIDs for Firmware Management Protocol instances
  DEFINE FMP_SYSTEM_FW_DEFAULT_ESRT_GUID = 8875B9DF-6D2B-4973-A9B7-47D2722DD5C0
  FmpDevicePkg/FmpDxe/QcomSystemFwFmpDxe.inf {
    <Defines>
      #
      # ESRT and FMP GUID for EDK2 device capsule update
      #
    ###  TODO: Uncomment below line and use FmpDxe.inf above once we resolve FMP guid
    ###  compilation issue while overriding the FMpDxe GUID like below.
    ###  Original:
    ###  FILE_GUID = $(FMP_SYSTEM_FW_DEFAULT_ESRT_GUID)

    <PcdsFixedAtBuild>
      #
      # Unicode name string that is used to populate FMP Image Descriptor for this capsule update module
      #
      gFmpDevicePkgTokenSpaceGuid.PcdFmpDeviceImageIdName|L"Qualcomm System Firmware Update Driver"

      # TestRoot.cer.gFmpDevicePkgTokenSpaceGuid.PcdFmpDevicePkcs7CertBufferXdr.inc
	  
      #gFmpDevicePkgTokenSpaceGuid.PcdFmpDevicePkcs7CertBufferXdr|{0x00, 0x00, 0x03, 0xC2, 0x30, 0x82, 0x03, 0xBE, 0x30, 0x82, 0x02, 0xA6, 0xA0, 0x03, 0x02, 0x01, 0x02, 0x02, 0x14, 0x7F, 0xDA, 0x38, 0x0D, 0xFC, 0x8C, 0x45, 0xDF, 0x8D, 0xD5, 0xE1, 0x27, 0x0E, 0x75, 0x8C, 0x80, 0xE9, 0xEA, 0x10, 0xBF, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x05, 0x00, 0x30, 0x7A, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x4E, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x09, 0x4B, 0x61, 0x72, 0x6E, 0x61, 0x74, 0x61, 0x6B, 0x61, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x09, 0x42, 0x61, 0x6E, 0x67, 0x61, 0x6C, 0x6F, 0x72, 0x65, 0x31, 0x22, 0x30, 0x20, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x13, 0x19, 0x51, 0x75, 0x61, 0x6C, 0x63, 0x6F, 0x6D, 0x6D, 0x20, 0x54, 0x65, 0x63, 0x68, 0x6E, 0x6F, 0x6C, 0x6F, 0x67, 0x69, 0x65, 0x73, 0x20, 0x49, 0x6E, 0x63, 0x31, 0x0C, 0x30, 0x0A, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x13, 0x03, 0x51, 0x43, 0x54, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x08, 0x72, 0x6F, 0x6F, 0x74, 0x75, 0x73, 0x65, 0x72, 0x30, 0x1E, 0x17, 0x0D, 0x32, 0x34, 0x30, 0x39, 0x31, 0x38, 0x30, 0x35, 0x32, 0x30, 0x31, 0x30, 0x5A, 0x17, 0x0D, 0x33, 0x34, 0x30, 0x39, 0x31, 0x36, 0x30, 0x35, 0x32, 0x30, 0x31, 0x30, 0x5A, 0x30, 0x7A, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x49, 0x4E, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x08, 0x13, 0x09, 0x4B, 0x61, 0x72, 0x6E, 0x61, 0x74, 0x61, 0x6B, 0x61, 0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x07, 0x13, 0x09, 0x42, 0x61, 0x6E, 0x67, 0x61, 0x6C, 0x6F, 0x72, 0x65, 0x31, 0x22, 0x30, 0x20, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x13, 0x19, 0x51, 0x75, 0x61, 0x6C, 0x63, 0x6F, 0x6D, 0x6D, 0x20, 0x54, 0x65, 0x63, 0x68, 0x6E, 0x6F, 0x6C, 0x6F, 0x67, 0x69, 0x65, 0x73, 0x20, 0x49, 0x6E, 0x63, 0x31, 0x0C, 0x30, 0x0A, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x13, 0x03, 0x51, 0x43, 0x54, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13, 0x08, 0x72, 0x6F, 0x6F, 0x74, 0x75, 0x73, 0x65, 0x72, 0x30, 0x82, 0x01, 0x22, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0F, 0x00, 0x30, 0x82, 0x01, 0x0A, 0x02, 0x82, 0x01, 0x01, 0x00, 0xCC, 0x50, 0x39, 0x8F, 0xF5, 0x68, 0xA2, 0x88, 0x8C, 0x90, 0xA0, 0x1E, 0xB2, 0x73, 0xD9, 0x14, 0xEF, 0x09, 0x50, 0x80, 0x5D, 0x22, 0x79, 0x54, 0xE2, 0xF6, 0x98, 0x71, 0xDD, 0x1B, 0x2E, 0x4C, 0x80, 0xF0, 0xEC, 0x94, 0xB2, 0xC5, 0x11, 0x3E, 0x1A, 0x7E, 0xE7, 0x40, 0x3D, 0xF9, 0xCF, 0xC9, 0xD1, 0x43, 0xD4, 0x44, 0x84, 0xA6, 0x01, 0xEE, 0xA8, 0x6A, 0xCF, 0x54, 0xE9, 0x34, 0x3A, 0x6C, 0x01, 0x6D, 0x27, 0xD5, 0x4F, 0x39, 0x5C, 0xC0, 0x62, 0x52, 0xEB, 0x21, 0x89, 0xBE, 0x52, 0x5B, 0x38, 0x7A, 0xCD, 0xF4, 0xCB, 0x07, 0xA0, 0x72, 0x7B, 0x99, 0xDA, 0x6F, 0xB5, 0x28, 0x22, 0x7D, 0x9E, 0x74, 0x8C, 0x35, 0xE6, 0x94, 0x61, 0x1E, 0xD4, 0x73, 0xA2, 0xEE, 0xA7, 0x78, 0x88, 0xC4, 0x71, 0x1D, 0x2C, 0x89, 0x42, 0x67, 0x88, 0x0D, 0x04, 0x12, 0x11, 0x31, 0x2E, 0x7C, 0x90, 0xDF, 0x01, 0xEC, 0x7E, 0xFE, 0x21, 0x58, 0xFA, 0xFD, 0xBE, 0x7A, 0x3F, 0xB8, 0xE6, 0xBD, 0x1C, 0x85, 0xEE, 0x70, 0xBB, 0x1F, 0xA9, 0xF2, 0x49, 0xC2, 0x42, 0xB5, 0x50, 0xA0, 0x5C, 0x0C, 0x4D, 0x4A, 0xE9, 0x18, 0x6E, 0x7F, 0x23, 0xB8, 0xAA, 0xE9, 0x6B, 0x9F, 0x5C, 0xA3, 0xB1, 0xB9, 0x22, 0xE5, 0x63, 0x23, 0x11, 0x07, 0xB0, 0x46, 0x8E, 0xFF, 0x5E, 0x5E, 0x2F, 0xE4, 0xD8, 0x38, 0xC9, 0x3C, 0x49, 0xD4, 0xE4, 0xCD, 0xB0, 0xA2, 0x28, 0x5F, 0xA9, 0x94, 0xD8, 0xE3, 0x5A, 0x92, 0x94, 0xF3, 0x8E, 0xFA, 0x7C, 0x34, 0xD0, 0x0E, 0xD6, 0x79, 0x6E, 0x62, 0xE0, 0xA6, 0xDD, 0xB9, 0xB7, 0x7F, 0x47, 0xEE, 0x80, 0xDF, 0xF6, 0xA2, 0xC2, 0xBD, 0x71, 0x77, 0x9C, 0x54, 0x36, 0x51, 0xD6, 0x04, 0x72, 0x72, 0xB1, 0x23, 0xBE, 0x3E, 0xF6, 0xE2, 0x65, 0xA7, 0x04, 0x88, 0x22, 0xA3, 0xA8, 0x09, 0x02, 0x03, 0x01, 0x00, 0x01, 0xA3, 0x3C, 0x30, 0x3A, 0x30, 0x1D, 0x06, 0x03, 0x55, 0x1D, 0x0E, 0x04, 0x16, 0x04, 0x14, 0x33, 0x68, 0xBC, 0xFB, 0xB5, 0x26, 0x1F, 0x24, 0x10, 0xB2, 0x76, 0x3A, 0xB3, 0x18, 0x32, 0x35, 0x10, 0x91, 0xE8, 0x0F, 0x30, 0x0C, 0x06, 0x03, 0x55, 0x1D, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF, 0x30, 0x0B, 0x06, 0x03, 0x55, 0x1D, 0x0F, 0x04, 0x04, 0x03, 0x02, 0x01, 0x06, 0x30, 0x0D, 0x06, 0x09, 0x2A, 0x86, 0x48, 0x86, 0xF7, 0x0D, 0x01, 0x01, 0x0B, 0x05, 0x00, 0x03, 0x82, 0x01, 0x01, 0x00, 0x53, 0xA9, 0x05, 0x71, 0x3B, 0x12, 0x5C, 0x86, 0x83, 0x2D, 0x6D, 0x4C, 0x1C, 0x33, 0x76, 0xF2, 0x79, 0xE0, 0xF4, 0x1E, 0xD3, 0xC9, 0x33, 0x7A, 0x76, 0x00, 0x73, 0xC7, 0xC4, 0xDF, 0x8A, 0x44, 0x54, 0x7F, 0xF1, 0x70, 0x52, 0x6D, 0x58, 0xFF, 0xA0, 0x2B, 0x32, 0x3C, 0xAE, 0xDD, 0x72, 0xAC, 0xA4, 0x63, 0xF1, 0x5A, 0x32, 0xCC, 0x25, 0x70, 0x36, 0x97, 0x95, 0xA1, 0x76, 0x95, 0xBB, 0xAB, 0xA3, 0x0F, 0x0B, 0xA6, 0xE3, 0xAC, 0x4C, 0xA0, 0x30, 0xF2, 0xED, 0xFC, 0xC4, 0x9A, 0xC6, 0x17, 0xE3, 0xA7, 0xA1, 0x9E, 0xDF, 0x3B, 0x17, 0x49, 0x1B, 0xC0, 0xFC, 0x84, 0xE2, 0x22, 0xD0, 0xDE, 0x07, 0xEC, 0xBE, 0x0C, 0x44, 0x17, 0x44, 0x92, 0x0D, 0x58, 0xE3, 0x0F, 0x92, 0x5A, 0x90, 0x64, 0xCD, 0x36, 0x68, 0x85, 0xE4, 0x54, 0x29, 0x30, 0x61, 0xDD, 0x73, 0x35, 0xCD, 0x31, 0x45, 0x9E, 0xBE, 0x12, 0x94, 0xE3, 0x9F, 0xEF, 0x07, 0xEE, 0x27, 0xD1, 0xDA, 0x32, 0x1E, 0xFE, 0xD4, 0xBB, 0xE4, 0x70, 0x19, 0x5F, 0xDA, 0x63, 0x5B, 0xBB, 0xD2, 0x19, 0x45, 0x76, 0x77, 0x28, 0xD4, 0x9C, 0xE5, 0x9C, 0xA7, 0x7C, 0x73, 0x0A, 0xD6, 0x42, 0x9E, 0x80, 0x04, 0xDA, 0x7E, 0x0A, 0xAD, 0x3D, 0x39, 0x42, 0x08, 0x7E, 0x88, 0x6F, 0xE8, 0xF4, 0x67, 0x4D, 0x40, 0x54, 0x9D, 0xB9, 0x38, 0xCC, 0x2E, 0x34, 0x32, 0xF3, 0xFB, 0x0C, 0xF1, 0xF9, 0xAF, 0x74, 0x9D, 0x78, 0x17, 0x16, 0x03, 0xEC, 0xE7, 0x60, 0x11, 0xAA, 0xCB, 0x10, 0x35, 0x06, 0xF4, 0xF7, 0x0E, 0xF6, 0xC1, 0xAA, 0x86, 0x87, 0x28, 0xCE, 0xCD, 0x25, 0x88, 0xFE, 0xE5, 0xE9, 0xF5, 0x3D, 0x9F, 0xF0, 0xA3, 0x17, 0x28, 0xB5, 0x3D, 0xFF, 0xF0, 0x30, 0xF5, 0x4E, 0xBB, 0xFD, 0x5B, 0xB9, 0x5A, 0x23, 0xD2, 0xDC, 0x17, 0x1A, 0x00, 0x00}      ## An event GUID that locks the firmware device as well as UEFI FMP variables when the event is signaled.
      
	  #  If this PCD is not a valid GUID value, then the firmware device and UEFI FMP variables are locked
      #  when gEfiEndOfDxeEventGroupGuid (End of DXE Phase) is signaled. Default value is gEfiEventReadyToBootGuid,so by default the firmware device is locked at the booting to any device/file.
      #  Making this PCD value 0, var lock will happen at End of Dxe event.
      #  @Prompt Firmware Device Lock Event GUID.
      gFmpDevicePkgTokenSpaceGuid.PcdFmpDeviceLockEventGuid|{0xB3, 0x8F, 0xE8, 0x7C, 0xD7, 0x4B, 0x79, 0x46, 0x87, 0xA8, 0xA8, 0xD8, 0xDE,  0xE5, 0x0D, 0x2B}

      #
      # ESRT and FMP Lowest Support Version for this capsule update module
      # 0000.0000
      #
      gFmpDevicePkgTokenSpaceGuid.PcdFmpDeviceBuildTimeLowestSupportedVersion|0x00000000

      gFmpDevicePkgTokenSpaceGuid.PcdFmpDeviceProgressWatchdogTimeInSeconds|0

      # Capsule Update Progress Bar Color.  Set to Green (RGB) (0, 255, 0)
      #
      gFmpDevicePkgTokenSpaceGuid.PcdFmpDeviceProgressColor|0x000000FF

    <LibraryClasses>
      #
      # Generic libraries that are used "as is" by all FMP modules
      #
      FmpPayloadHeaderLib|FmpDevicePkg/Library/FmpPayloadHeaderLibV1/FmpPayloadHeaderLibV1.inf
      #FmpDependencyLib|FmpDevicePkg/Library/FmpDependencyLib/FmpDependencyLib.inf
      #FmpDependencyCheckLib|FmpDevicePkg/Library/FmpDependencyCheckLibNull/FmpDependencyCheckLibNull.inf
      #FmpDependencyDeviceLib|FmpDevicePkg/Library/FmpDependencyDeviceLibNull/FmpDependencyDeviceLibNull.inf

      #
      # Platform specific capsule policy library
      #
      CapsuleUpdatePolicyLib|QcomPkg/Library/CapsuleUpdatePolicyLib/CapsuleUpdatePolicyLib.inf

      #
      # Device specific library that processes a capsule and updates the FW storage device
      #
      FmpDeviceLib|QcomPkg/Library/FmpDeviceLib/FmpDeviceLib.inf
  }
  
  
  
  
  
  QcomPkg/Drivers/GLinkDxe/GLinkDxe.inf {
  <LibraryClasses>
  SmdLib|QcomPkg/Library/SmdLib/SmdLib.inf
  GLinkLib|QcomPkg/Library/GLinkLib/GLinkLib.inf
  }
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf
  QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
  QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf {
    #<PcdsFixedAtBuild>
    #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
    <LibraryClasses>
    UsbfnDwc3Lib|QcomPkg/SocPkg/Kodiak/Library/UsbfnDwc3Lib/UsbfnDwc3Lib.inf
  }
  #QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
  #QcomPkg/Drivers/XhciDxe/XhciDxe.inf
  #QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
  #QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
  #QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
  QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
  QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
  QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf  {
    #<PcdsFixedAtBuild>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xFFFFFFFF
    <LibraryClasses>
      UsbConfigLib|QcomPkg/SocPkg/Kodiak/Library/UsbConfigLib/UsbConfigLib.inf
      #UsbConfigLib|QcomPkg/Library/NullLibs/UsbConfigLibNull/UsbConfigLibNull.inf
  }
  QcomPkg/Drivers/UsbFastbootTransportDxe/UsbFastbootTransport.inf
  QcomPkg/Drivers/UsbInitDxe/UsbInitDxe.inf
  QcomPkg/Drivers/FastbootPlatformDxe/FastbootPlatformDxe.inf
  QcomPkg/Drivers/DALSYSDxe/DALSYSDxe.inf
  QcomPkg/Drivers/ICBDxe/ICBDxe.inf
  QcomPkg/Drivers/ClockDxe/ClockDxe.inf {
  <LibraryClasses>
     ClockLib|QcomPkg/SocPkg/Kodiak/Library/ClockLib/ClockLibXBL.inf
  }
  #QcomPkg/Drivers/NULLClockDxe/NULLClockDxe.inf
  QcomPkg/Drivers/VcsDxe/VcsDxe.inf {
    <LibraryClasses>
      VcsLib|QcomPkg/SocPkg/Kodiak/Library/VcsLib/VcsLib.inf
  }

  #
  # EDK II Core PCI Bus Driver
  #
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf {
    <LibraryClasses>
      PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  }

  #
  # PCI HostBridge Driver
  #
  QcomPkg/Drivers/PciHostBridgeDxe/PciHostBridgeDxe.inf

  QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.inf
  QcomPkg/Drivers/HWIODxe/HWIODxe.inf
  QcomPkg/Drivers/UCDxe/UCDxe.inf
  QcomPkg/Drivers/RNGDxe/RngDxe.inf
  CryptoPkg/Driver/CryptoDxe.inf
  QcomPkg/Drivers/SoftSKUDxe/SoftSKUDxe.inf
  QcomPkg/Drivers/FeatureEnablerDxe/FeatureEnablerDxe.inf
  QcomPkg/Drivers/MinidumpTADxe/MinidumpTADxe.inf
  QcomPkg/Drivers/DisplayDxe/DisplayDxe.inf
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
#QcomPkg/Drivers/QdssDxe/QdssDxe.inf
  #
  # GPI Driver
  #
  QcomPkg/Drivers/GpiDxe/GpiDxe.inf

  #
  # I2C Driver
  #
  QcomPkg/Drivers/I2CDxe/I2CDxe.inf

  #
  # SPI Driver
  #
  QcomPkg/Drivers/SPIDxe/SPIDxe.inf
  #
  # QUPCOMMON Driver
  #
  QcomPkg/Drivers/QupDxe/QupDxe.inf

  #
  # HSUART Driver
  # 
  QcomPkg/Drivers/HSUartDxe/HSUartDxe.inf
  
  
  QcomPkg/Drivers/NpaDxe/NpaDxe.inf {
    <LibraryClasses>
      RpmTargetLib|QcomPkg/SocPkg/Kodiak/Library/RpmTargetLib/RpmTargetLib.inf
      #SmdLib|QcomPkg/Library/SmdLib/SmdLib.inf
      NpaTargetLib|QcomPkg/SocPkg/Kodiak/Library/NpaTargetLib/NpaTargetLib.inf
  }
  QcomPkg/Drivers/ULogDxe/ULogDxe.inf {
    <LibraryClasses>
      ULogLib|QcomPkg/Library/ULogLib/ULogLib.inf
  }
  QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf {
    <LibraryClasses>
      CmdDbLib|QcomPkg/Library/CmdDbLib/CmdDbLib.inf
      CmdDbTargetLib|QcomPkg/SocPkg/Kodiak/Library/CmdDbTargetLib/CmdDbTargetLib.inf
  }
  QcomPkg/Drivers/PwrUtilsDxe/PwrUtilsDxe.inf {
    <LibraryClasses>
      PwrUtilsLib|QcomPkg/Library/PwrUtilsLib/PwrUtilsLib.inf
  }
  QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf {
    <LibraryClasses>
      RpmhLib|QcomPkg/SocPkg/Kodiak/Library/RpmhTargetLib/RpmhLib.inf
  }
  #QcomPkg/Drivers/PdcDxe/PdcDxe.inf {
  #  <LibraryClasses>
  #    PdcTargetLib|QcomPkg/SocPkg/Kodiak/Library/PdcTargetLib/PdcTargetLib.inf
  #}
  QcomPkg/Drivers/CPRDxe/CPRDxe.inf {
    <LibraryClasses>
      CPRLib|QcomPkg/SocPkg/Kodiak/Library/CPRTargetLib/Core/CPRTargetLib.inf
  }
  QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
  QcomPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
  QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf {
   <LibraryClasses>
     ButtonsLib|QcomPkg/SocPkg/Kodiak/Library/ButtonsTargetLib/ButtonsLib.inf
  }
# --PRESIL--
  #QcomPkg/Drivers/PmicDxe/NullLib/PmicDxeNull.inf{
   #<LibraryClasses>
    #PmicCoreLib|QcomPkg/Library/NullLibs/PmicLibNull/PmicLibNull.inf
   #}
  QcomPkg/Drivers/PmicDxe/PmicDxeLa.inf {
   <LibraryClasses>
    PmicCoreLib|QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/PmicCoreLib.inf
    AdcCoreLib|QcomPkg/Library/PmicLib/adc/build/AdcCoreLib.inf
   }
  
  # Pmic GLink Driver
  QcomPkg/Drivers/PmicGlinkDxe/PmicGlinkDxe.inf
# --PRESIL--
  #QcomPkg/Drivers/ChargerExDxe/NullLib/ChargerExDxeNull.inf
  QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
  QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf
  QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
  #QcomPkg/Drivers/UsbPwrCtrlDxe/NullLib/UsbPwrCtrlDxeNull.inf
# --PRESIL--
  QcomPkg/Drivers/TsensDxe/TsensDxe.inf {
    <LibraryClasses>
      TsensCoreLib|QcomPkg/Library/TsensLib/TsensCoreLib.inf
     TsensTargetLib|QcomPkg/SocPkg/Kodiak/Library/TsensTargetLib/TsensTargetLib.inf
  }
 QcomPkg/Drivers/LimitsDxe/LimitsDxe.inf {
    <LibraryClasses>
    LimitsTargetLib|QcomPkg/SocPkg/Kodiak/Library/LimitsTargetLib/LimitsTargetLib.inf
  }
# --PRESIL--
#QcomPkg/Drivers/SdccDxe/SdccDxe_SDBoot.inf
  QcomPkg/Drivers/SdccDxe/SdccDxe.inf
  QcomPkg/Drivers/UFSDxe/UFSDxe.inf
  #QcomPkg/Drivers/SpiNorDxe/SpiNorDxe.inf
  QcomPkg/Drivers/SerialDxe/SerialDxe.inf {
    <LibraryClasses>
      SerialPortLib|QcomPkg/Library/SerialPortLib/SerialPortLib.inf
      UartLib|QcomPkg/Library/UartQupv3Lib/UartCoreLib.inf
      UartSettingsLib|QcomPkg/SocPkg/Kodiak/Settings/UART/UartSettingsLib.inf
  }
  QcomPkg/Drivers/PILDxe/PILDxe.inf
  QcomPkg/Drivers/PILProxyDxe/PILProxyDxe.inf
  QcomPkg/Drivers/QcomBds/QcomBds.inf
 # QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
   QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxe.inf
  #QcomPkg/Application/FastbootApp/FastbootApp.inf
  EmbeddedPkg/Application/AndroidFastboot/AndroidFastbootApp.inf
  
  QcomPkg/Application/QcomChargerApp/QcomChargerApp.inf {
   <PcdsFixedAtBuild>
     gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07
  }
  EmbeddedPkg/Ebl/Ebl.inf

  #
  # FV Utils Driver
  #
  QcomPkg/Drivers/FvUtilsDxe/FvUtilsDxe.inf
  
  #
  # Qcom Parser Driver
  #
  QcomPkg/Drivers/ParserDxe/ParserDxe.inf
  
  #
  # Qcom Serial Port Driver
  #
  QcomPkg/Drivers/SerialPortDxe/SerialPortDxe.inf
  
  #
  # Qcom Subset Parts DT fixup Driver
  #
  QcomPkg/Drivers/SubsetPartsDxe/SubsetPartsDxe.inf

  #
  # Qcom Board info DT fixup Driver
  #
  QcomPkg/Drivers/BoardInfoDxe/BoardInfoDxe.inf
