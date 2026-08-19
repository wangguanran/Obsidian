#/** @file CoreAux.fdf.inc
# FLASH layout file for Makena
#
# Copyright (c) 2020 Qualcomm Technologies, Inc. All rights reserved.
# Portions Copyright (c) 2006 - 2010, Intel Corporation.
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

################################################################################
#     EFI's included in Auxillary FV
################################################################################
  
  INF QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf
  INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf

  INF QcomPkg/Drivers/GLinkDxe/GLinkDxe.inf
  INF QcomPkg/Drivers/IPCCDxe/IPCCDxe.inf  
  INF QcomPkg/Drivers/CPRDxe/CPRDxe.inf

  #
  # GPI Driver
  #
  INF QcomPkg/Drivers/GpiDxe/GpiDxe.inf

  #
  # I2C Driver
  #
#  INF QcomPkg/Drivers/I2CDxe/I2CDxe.inf

  #
  # Raw File System Driver
  #
  #INF QcomPkg/Drivers/RawFileSystemDxe/RawFileSystemDxe.inf
  
  #
  # RawFS Simple File System Driver
  #
  #INF QcomPkg/Drivers/RawFsSimpleFsDxe/RawFsSimpleFsDxe.inf



  #
  # SPI Driver
  #
  INF QcomPkg/Drivers/SPIDxe/SPIDxe.inf
  
  INF QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
  #INF QcomPkg/Drivers/SdccDxe/SdccDxe_EntpMgmt.inf

  INF QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf
#INF QcomPkg/Drivers/FeatureEnablerDxe/FeatureEnablerDxe.inf

#INF QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf
# --PRESIL--
  #INF QcomPkg/Drivers/PmicDxe/PmicDxeWp.inf
#INF QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf
#INF QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
#INF QcomPkg/Drivers/AdcDxe/AdcDxe.inf
INF QcomPkg/Drivers/TsensDxe/TsensDxe.inf
INF QcomPkg/Drivers/LimitsDxe/LimitsDxe.inf
  INF QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
#INF QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf

#
#  USB Support
#
#INF QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
#INF QcomPkg/Drivers/XhciDxe/XhciDxe.inf
#INF QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
#INF QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
#INF QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
#INF QcomPkg/Drivers/Usb4Dxe/Usb4Dxe.inf
#INF QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
#INF QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
#INF QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
#INF QcomPkg/Drivers/UsbInitDxe/UsbInitDxe.inf

INF QcomPkg/Drivers/UCDxe/UCDxe.inf
INF QcomPkg/Drivers/RNGDxe/RngDxe.inf
#INF QcomPkg/Drivers/TrEEDxe/TpmDxe.inf
#INF QcomPkg/Drivers/TrEEDxe/TrEEDxe.inf
#INF QcomPkg/Drivers/AdapterInformationDxe/AdapterInformationDxe.inf
#INF QcomPkg/Drivers/QdssDxe/QdssDxe.inf
  INF QcomPkg/Drivers/PILDxe/PILDxe.inf
  INF QcomPkg/Drivers/PILProxyDxe/PILProxyDxe.inf
  #
  # DPP Protocol
  #
  #INF QcomPkg/Drivers/DppDxe/DppDxe.inf

  INF QcomPkg/Drivers/SecRSADxe/SecRSADxe.inf
  INF QcomPkg/Drivers/ASN1X509Dxe/ASN1X509Dxe.inf
  #
  # QCOM FV Utils Driver
  #
  INF QcomPkg/Drivers/FvUtilsDxe/FvUtilsDxe.inf
  INF QcomPkg/Drivers/ParserDxe/ParserDxe.inf
  INF QcomPkg/Drivers/SerialPortDxe/SerialPortDxe.inf
#INF QcomPkg/Application/QcomChargerApp/QcomChargerApp.inf

  #INF MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  #
  # EDK II Core PCI Bus Driver
  #
  #INF MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf

  #
  # PCI Support
  #
  #INF QcomPkg/Drivers/PciHostBridgeDxe/PciHostBridgeDxe.inf

  FILE FREEFORM = a91d838e-a5fa-4138-825d-455e23030794 {
    SECTION UI = "logo1.bmp"
    SECTION RAW = QcomPkg/Logo/Common/logo1.bmp
  }

  #
  # Display panel configuration xml
  #

  FILE FREEFORM = 19b58acd-b1f8-4078-b9a5-a33584b680e2 {
    SECTION UI = "Panel_sharp_4k_dsc_vid.xml"
    SECTION RAW = QcomPkg/Settings/Panel/Panel_sharp_4k_dsc_vid.xml
  }


  FILE FREEFORM = 3982697d-4889-4433-b547-0121a1d2bdf0 {
    SECTION UI = "Panel_sharp_1080p_edp.xml"
    SECTION RAW = QcomPkg/Settings/Panel/Panel_sharp_1080p_edp.xml
  }

  #FILE FREEFORM = b5b1c960-f3f4-4512-a83e-507e603b551a {
  #  SECTION UI = "Panel_dual_dp.xml"
  #  SECTION RAW = QcomPkg/Settings/Panel/Panel_dual_dp.xml
  #}
  
  # End - Display panel configuration xml



  #
  # CFG needed for UEFI PIL
  #
  FILE FREEFORM = 21E9BDD9-6C3F-4F10-84A5-BBEC322741F1 {
    SECTION UI = "uefipil.cfg"
    SECTION RAW = QcomPkg/SocPkg/Kodiak/$(VAR)/uefipil.cfg
  }

#  FILE FREEFORM = 45FE4B7C-150C-45da-A021-4BEB2048EC6F {
#    SECTION UI = "QcomChargerCfg.cfg"
#    SECTION RAW = QcomPkg/SocPkg/Lahaina/Settings/Charger/QcomChargerConfig_VbattTh.cfg
#  }

  FILE FREEFORM = F780C779-DD7C-47CD-BD1A-5EB414C51704 {
    SECTION UI = "BATTERY.PROVISION"
    SECTION RAW = QcomPkg/Drivers/QcomChargerDxe/BATTERY.PROVISION
  }

  INF EmbeddedPkg/Ebl/Ebl.inf

