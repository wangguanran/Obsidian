# UEFI 回退 pcie-usb qcom 补丁

> **模块**: USB | **厂商**: Qualcomm | **芯片**: QCM6490 (yupik/Kodiak)
> **平台**: QCM6490-A16 (rigle_linux_1.6) | **类型**: Bug
> **Change**: #196565 | **作者**: lixianghui | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196565 |
| 项目 | qualcomm-linux-spf-1-0_ap_standard_oem_nm-qimpsdk |
| 分支 | rigle_linux_1.6 |
| 作者 | lixianghui |
| 类型 | Bug（Revert：回退 pcie-usb qcom 补丁） |
| 芯片 | Qualcomm QCM6490 (yupik/Kodiak) |
| 平台 | QCM6490-A16（Rigel Linux / Ubuntu） |
| 模块 | USB（UEFI：XhciDxe / UsbBusDxe / PcieConfigLib / PmicLib / Core.fdf） |
| 提交标题 | `Revert "[MC936_Linux][TaskID]113211[Description]pcie-usb qcom patch[Solution]do it.[owner]qianyiping"` |
| 任务 | 对应原 Task 113211（回退对象） |

## 现象

原 `pcie-usb qcom patch`（commit `cc0e3a22`，Task 113211）对 UEFI USB/PCIe 链路做了较大范围修改：XHCI 驱动绑定/启动流程中插入大量 `DEBUG` 打印与逻辑微调、`PcieConfigLibEnableRootPorts` 端口使能逻辑变更、PMIC 目标协议安装裁剪、Core.fdf 驱动清单调整（含 AX88179UsbEthDxe 条目）等。该补丁合入后对 USB 枚举/PCIe 端口/构建流程造成回归，需要整体回退。

> 注：本提交为纯 revert，未附带单独修复；回退后恢复至原上游行为，若仍有功能需求需另行评估补丁。

## 环境

| 项 | 内容 |
|----|------|
| UEFI 源码 | `BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/`（Kodiak SoC 平台） |
| 构建工具 | `mg_build_tools/mg_build.py`（构建规则含 SUB_SYS_BUILD_RULES） |
| 回退对象 | commit `cc0e3a22b556267ac6fe62de9865d916a3f0ec6f` |

## 根因分析

回退补丁本身不引入新逻辑，问题根因在原始 pcie-usb 补丁（#113211）：

1. **XHCI 驱动流程被大量 DEBUG 打印与分支调整污染**：`XhcDriverBindingSupported` / `XhcDriverBindingStart` 中插入了调试打印和错误处理改动（`XhcCreateUsbHc`、ControlPollTimer、async monitor 启动等），存在掩盖真实错误状态、改变控制流顺序的风险。
2. **PCIe 配置使能逻辑变更**：`PcieConfigLibEnableRootPorts`（约 140 行）与 `PcieConfig_isPort1Supported` 被改写，可能改变 root port 上电/枚举时序。
3. **PMIC 协议安装裁剪**：`pm_core.c` 中 `pm_install_target_protocols` 大量协议安装被删除，影响 PMIC 驱动的协议可用性。
4. **构建与固件清单变更**：`Core.fdf`/`CoreAux.fdf.inc` 增删驱动 INF、`uefiplat.cfg` 参数调整、`mg_build.py` 构建规则变化，影响产物结构与流程。

由于修改面覆盖 USB 枚举、PCIe、PMIC、构建四个领域且无独立修复手段，选择整体 revert 回到合入前基线。

## 处理方案

`git revert cc0e3a22`（保留 revert 提交记录），恢复以下文件至上游状态：

- `UsbBusDxe/UsbEnumer.c`：移除根集线器枚举调试打印
- `XhciDxe/Xhci.c` / `XhciDxe.inf`：恢复 XHCI 绑定/启动原始逻辑与打印
- `SocPkg/Kodiak/Common/{Core.fdf, Core.dsc.inc, CoreAux.fdf.inc, Apriori.fdf.inc, uefiplat.cfg}`：恢复固件清单与参数
- `PmicLib/core/la/{pm_core.c, PmicCoreLib.inf}`：恢复 PMIC 目标协议安装
- `PcieConfigLib/PcieConfigLib.c`：恢复 root port 使能逻辑
- `XBLLoader/ExtDrivers/boot_uart.c`、`ShellPkg Tftp*`：恢复 UART/网络 shell 原始实现
- `mg_build_tools/mg_build.py`：恢复构建规则（`-v WP -t kodiak` 恢复 `RELEASE,DEBUG` 双变体）

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/UsbBusDxe/UsbEnumer.c\|UsbEnumer.c]] | -1 | 移除 root hub 枚举 DEBUG 打印 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/Xhci.c\|Xhci.c]] | 大量 +/- | 恢复 XHCI 绑定/启动原始逻辑 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/XhciDxe.inf\|XhciDxe.inf]] | 少量 | INF 内容恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Apriori.fdf.inc\|Apriori.fdf.inc]] | 少量 | Apriori 恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.dsc.inc\|Core.dsc.inc]] | 少量 | DSC 恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.fdf\|Core.fdf]] | 多文件条目 | 恢复驱动清单（含 AX88179UsbEthDxe） |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/CoreAux.fdf.inc\|CoreAux.fdf.inc]] | 少量 | Aux FDF 恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/uefiplat.cfg\|uefiplat.cfg]] | 少量 | 平台参数恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/pm_core.c\|pm_core.c]] | 大量 - | 恢复 PMIC 协议安装 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/PmicCoreLib.inf\|PmicCoreLib.inf]] | 少量 | INF 恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.c\|PcieConfigLib.c]] | 大量 +/- | 恢复 root port 使能逻辑 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/XBLLoader/ExtDrivers/boot_uart.c\|boot_uart.c]] | 少量 | UART 初始化恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/Tftp.c\|Tftp.c]] | 少量 | TFTP HiiPackage 恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf\|TftpDynamicCommand.inf]] | 少量 | INF 恢复 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/mg_build_tools/mg_build.py\|mg_build.py]] | 1 行 | 构建规则恢复 RELEASE,DEBUG |

## 配置方式

无新增配置项；revert 恢复的 UEFI 构建参数（`uefiplat.cfg`）与驱动清单（`Core.fdf`/`CoreAux.fdf.inc`）为上游默认。构建命令示例：

```bash
# 在 UEFI 仓库根目录
python -u mg_build_tools/mg_build.py -v WP -t kodiak,QcomToolsPkg -v LAA -r RELEASE,DEBUG
```

## 验证方式

- 构建验证：`mg_build.py` 恢复 `RELEASE,DEBUG` 双变体构建，确认 xbl.elf 产物生成正常（原补丁曾裁剪为仅 RELEASE）。
- 实机验证（建议）：
  - 开机进 UEFI shell，确认 USB 键盘/存储枚举正常（`UsbEnumer`/XHCI 恢复后）
  - `pcie` 相关外设（如有）在 OS 内可枚举
  - PMIC 相关 UEFI 协议（充电显示等）行为正常
- 补丁层验证：本补丁与 Gerrit 合入提交的 diff 逐字节一致（见下）。

## 结论

通过整体 revert 消除 pcie-usb 补丁对 USB 枚举、PCIe 使能、PMIC 协议与构建流程的连带影响，恢复至合入前基线。此类跨领域大补丁后续建议拆分评审、逐项验证。

## 补丁内容

```diff
Subject: [PATCH] Revert "[MC936_Linux][TaskID]113211[Description]pcie-usb qcom
 patch[Solution]do it.[owner]qianyiping"

This reverts commit cc0e3a22b556267ac6fe62de9865d916a3f0ec6f.

---
 .../QcomPkg/Drivers/UsbBusDxe/UsbEnumer.c     |   1 -
 .../boot/QcomPkg/Drivers/XhciDxe/Xhci.c       |  50 ++----
 .../boot/QcomPkg/Drivers/XhciDxe/XhciDxe.inf  |   1 -
 .../SocPkg/Kodiak/Common/Apriori.fdf.inc      |   8 +-
 .../QcomPkg/SocPkg/Kodiak/Common/Core.dsc.inc |  50 +-----
 .../QcomPkg/SocPkg/Kodiak/Common/Core.fdf     |  37 +---
 .../SocPkg/Kodiak/Common/CoreAux.fdf.inc      |   8 +-
 .../QcomPkg/SocPkg/Kodiak/Common/uefiplat.cfg |   2 +-
 .../Library/PmicLib/core/la/PmicCoreLib.inf   |   1 -
 .../Kodiak/Library/PmicLib/core/la/pm_core.c  | 160 ------------------
 .../Library/PcieConfigLib/PcieConfigLib.c     | 150 +---------------
 .../QcomPkg/XBLLoader/ExtDrivers/boot_uart.c  |   2 +-
 .../DynamicCommand/TftpDynamicCommand/Tftp.c  |  46 +++--
 .../TftpDynamicCommand/TftpDynamicCommand.inf |   5 +-
 mg_build_tools/mg_build.py                    |   2 +-
 15 files changed, 72 insertions(+), 451 deletions(-)

diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/UsbBusDxe/UsbEnumer.c b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/UsbBusDxe/UsbEnumer.c
index b26057155c..55349a88a2 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/UsbBusDxe/UsbEnumer.c
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/UsbBusDxe/UsbEnumer.c
@@ -1354,7 +1354,6 @@ UsbEnumNewDevRootHub (
           }
         }
       }
-      DEBUG ((EFI_D_ERROR, " UsbEnumNewDevRootHub: vid = 0x%04x pid = 0x%04x, Addr = %d\n", Child->DevDesc->Desc.IdVendor, Child->DevDesc->Desc.IdProduct, Child->Address));
       //
       // Select a default configuration: UEFI must set the configuration
       // before the driver can connect to the device.
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/Xhci.c b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/Xhci.c
index a3fe36cd57..a1061a6954 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/Xhci.c
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/Xhci.c
@@ -2310,8 +2310,6 @@ XhcDriverBindingSupported (
   USB_CLASSC              UsbClassCReg;
   UINT32                  PciID;
 
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingSupported: Entry\n"));
-
   //
   // Test whether there is PCI IO Protocol attached on the controller handle.
   //
@@ -2325,7 +2323,6 @@ XhcDriverBindingSupported (
                   );
 
   if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingSupported: PCI IO Protocol not found\n"));
     return EFI_UNSUPPORTED;
   }
 
@@ -2348,13 +2345,10 @@ XhcDriverBindingSupported (
   if ((UsbClassCReg.BaseCode != PCI_CLASS_SERIAL) ||
       (UsbClassCReg.SubClassCode != PCI_CLASS_SERIAL_USB) ||
       (UsbClassCReg.ProgInterface != PCI_IF_XHCI)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingSupported: Not XHCI controller (Class %x/%x/%x)\n", 
-            UsbClassCReg.BaseCode, UsbClassCReg.SubClassCode, UsbClassCReg.ProgInterface));
     Status = EFI_UNSUPPORTED;
   }
   else
   {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingSupported: XHCI controller detected\n"));
     Status = PciIo->Pci.Read (
                           PciIo,
                           EfiPciIoWidthUint8,
@@ -2384,7 +2378,7 @@ ON_EXIT:
          This->DriverBindingHandle,
          Controller
          );
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingSupported: Exit - %r\n", Status));
+
   return Status;
 }
 
@@ -2412,12 +2406,9 @@ XhcCreateUsbHc (
   UINT16                  ExtCapReg;
   UINTN                   CoreNum;
 
-  DEBUG ((EFI_D_ERROR, "[XHCI] CreateUsbHc: Entry\n"));
-
   Xhc = AllocateZeroPool (sizeof (USB_XHCI_INSTANCE));
 
   if (Xhc == NULL) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] CreateUsbHc: Memory allocation failed\n"));
     return NULL;
   }
 
@@ -2473,8 +2464,6 @@ XhcCreateUsbHc (
   Xhc->UsbLegSupOffset   = XhcGetCapabilityAddr (Xhc, XHC_CAP_USB_LEGACY);
   Xhc->DebugCapSupOffset = XhcGetCapabilityAddr (Xhc, XHC_CAP_USB_DEBUG);
 
-  DEBUG ((EFI_D_ERROR, "[XHCI] CreateUsbHc: MaxSlots=%d MaxPorts=%d 64bit=%d\n", 
-          Xhc->HcSParams1.Data.MaxSlots, Xhc->HcSParams1.Data.MaxPorts, Xhc->HcCParams.Data.Ac64));
   DEBUG ((EFI_D_INFO, "XhcCreateUsb3Hc: Capability length 0x%x\n", Xhc->CapLength));
   DEBUG ((EFI_D_INFO, "XhcCreateUsb3Hc: HcSParams1 0x%x\n", Xhc->HcSParams1));
   DEBUG ((EFI_D_INFO, "XhcCreateUsb3Hc: HcSParams2 0x%x\n", Xhc->HcSParams2));
@@ -2527,15 +2516,14 @@ XhcCreateUsbHc (
                     );
 
     if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] CreateUsbHc: ControlPollTimer creation failed - %r\n", Status));
+    DEBUG ((EFI_D_ERROR, "XhcCreateUsbHc: ControlPollTimer creation failed with status %r\n", Status));
       goto ON_ERROR;
     }
 
-  DEBUG ((EFI_D_ERROR, "[XHCI] CreateUsbHc: Exit - Success\n"));
+
   return Xhc;
 
 ON_ERROR:
-  DEBUG ((EFI_D_ERROR, "[XHCI] CreateUsbHc: Exit - Error\n"));
   FreePool (Xhc);
   return NULL;
 }
@@ -2630,10 +2618,10 @@ XhcDriverBindingStart (
   USB_XHCI_INSTANCE         *Xhc;
   EFI_DEVICE_PATH_PROTOCOL  *HcDevicePath;
   UINTN                     DeviceNum;
+
   //
   // Open the PciIo Protocol, then enable the USB host controller
   //
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Opening PCI IO Protocol\n"));
   Status = gBS->OpenProtocol (
                   Controller,
                   &gEfiPciIoProtocolGuid,
@@ -2644,7 +2632,6 @@ XhcDriverBindingStart (
                   );
 
   if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Failed to open PCI IO - %r\n", Status));
     return Status;
   }
 
@@ -2694,25 +2681,22 @@ XhcDriverBindingStart (
   }
 
   if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Failed to enable controller - %r\n", Status));
+    DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: failed to enable controller\n"));
     goto CLOSE_PCIIO;
   }
 
   //
   // Create then install USB2_HC_PROTOCOL
   //
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Creating USB HC instance\n"));
   Xhc = XhcCreateUsbHc (PciIo, HcDevicePath, OriginalPciAttributes);
 
   if (Xhc == NULL) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Failed to create USB HC\n"));
+    DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: failed to create USB2_HC\n"));
     return EFI_OUT_OF_RESOURCES;
   }
-  
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Setting BIOS ownership\n"));
+
   XhcSetBiosOwnership (Xhc);
-  
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Resetting HC\n"));
+
   XhcResetHC (Xhc, XHC_RESET_TIMEOUT);
   USB_ASSERT_GOTO_SETSTS ((XhcIsHalt (Xhc)), FREE_POOL, EFI_NOT_READY, Status);
 
@@ -2725,22 +2709,19 @@ XhcDriverBindingStart (
   //
   // Initialize the schedule
   //
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Initializing scheduler\n"));
   XhcInitSched (Xhc);
 
   //
   // Start the Host Controller
   //
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Starting HC\n"));
   XhcRunHC(Xhc, XHC_GENERIC_TIMEOUT);
 
   //
   // Start the asynchronous interrupt monitor
   //
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Starting async monitor\n"));
   Status = gBS->SetTimer (Xhc->PollTimer, TimerPeriodic, XHC_ASYNC_TIMER_INTERVAL);
   if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Failed to start async monitor - %r\n", Status));
+    DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: failed to start async interrupt monitor\n"));
     XhcHaltHC (Xhc, XHC_GENERIC_TIMEOUT);
     goto FREE_POOL;
   }
@@ -2782,7 +2763,6 @@ XhcDriverBindingStart (
   //
   // Install the Usb2Hc and UsbPortTest protocols
   //
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Installing USB2 HC Protocol\n"));
   Status = gBS->InstallMultipleProtocolInterfaces (
                   &Controller,
                   &gEfiUsb2HcProtocolGuid,
@@ -2790,11 +2770,10 @@ XhcDriverBindingStart (
                   NULL
                   );
   if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Failed to install USB2 HC Protocol - %r\n", Status));
+    DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: failed to install USB2_HC Protocol\n"));
     goto FREE_POOL;
   }
 
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Installing USB Port Test Protocol\n"));
   Status = gBS->InstallMultipleProtocolInterfaces (
                   &Controller,
                   &gQcomUsbPortTestProtocolGuid, 
@@ -2803,7 +2782,7 @@ XhcDriverBindingStart (
                   );
 
   if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Failed to install Port Test Protocol - %r\n", Status));
+    DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: failed to install USB Port test Protocol\n"));
     goto FREE_POOL;
   }
 
@@ -2813,11 +2792,10 @@ XhcDriverBindingStart (
   }
 
   Xhc->CoreNum = GET_CORENUM (DeviceNum);
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: CoreNum=%d DeviceNum=%d\n", Xhc->CoreNum, DeviceNum));
+  DEBUG ((EFI_D_INFO, "XhcDriverBindingStart: CoreNum = %d\n",Xhc->CoreNum));
 
 // Enable Vbus via config usb protocol
 // Locate Config USB protocol here and get handle for that
-DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Locating USB Config Protocol\n"));
   Status = gBS->LocateProtocol (
                   &gQcomUsbConfigProtocolGuid,
                   NULL,
@@ -2825,11 +2803,11 @@ DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Locating USB Config Protocol\n"
                   );
 
   if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Failed to locate USB Config Protocol - %r\n", Status));
+    DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: failed to locate UsbConfig protocol\n"));
     goto FREE_POOL;
   }  
 
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: SUCCESS - Driver fully initialized\n"));
+  DEBUG ((EFI_D_INFO, "XhcDriverBindingStart: XHCI started for controller @ %x\n", Controller));
   return EFI_SUCCESS;
 
 FREE_POOL:
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/XhciDxe.inf b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/XhciDxe.inf
index b3138a6d83..3fa808a92d 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/XhciDxe.inf
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/XhciDxe.inf
@@ -83,7 +83,6 @@
   gQcomUsbPortTestProtocolGuid                  ## PRODUCES
   gQcomUsbConfigProtocolGuid                    ## CONSUMES
   gEfiUsbMsdPeripheralProtocolGuid              ## CONSUMES
-  gEfiTLMMProtocolGuid                          ## CONSUMES
 
 [Pcd]
   gQcomTokenSpaceGuid.InitUsbControllerOnBoot
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Apriori.fdf.inc b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Apriori.fdf.inc
index 94fb50aa2d..18599c5f41 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Apriori.fdf.inc
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Apriori.fdf.inc
@@ -132,11 +132,11 @@
         #INF QcomPkg/Drivers/ChargerExDxe/NullLib/ChargerExDxeNull.inf
 INF QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
         INF QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
-        INF QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
-        INF QcomPkg/Drivers/XhciDxe/XhciDxe.inf
-        INF QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
+        #INF QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
+        #INF QcomPkg/Drivers/XhciDxe/XhciDxe.inf
+        #INF QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
         #INF QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
-        INF QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
+        #INF QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
         INF QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
         INF QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
         INF QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.dsc.inc b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.dsc.inc
index 06b5893c45..93c968e3c9 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.dsc.inc
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.dsc.inc
@@ -1,7 +1,6 @@
 #==============================================================================
 # @file Core.dsc
 # Kodiak Core package.
-# Network stack additions for Elo PXE boot (QCS6490 / Kodiak LA).
 #
 # Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 # All rights reserved.
@@ -270,17 +269,6 @@
   PcieConfigLib|QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.inf
   PcieCoreConfigLib|QcomPkg/SocPkg/Kodiak/Settings/PCIE/core/PcieCoreConfigLib.inf
 
-  # ── Network Stack Libraries (Elo PXE boot / QCS6490) ─────────────────────
-  # Required by MnpDxe, Ip4Dxe, Ip6Dxe, TcpDxe, UdpDxe, UefiPxeBcDxe
-  DpcLib|NetworkPkg/Library/DxeDpcLib/DxeDpcLib.inf
-  NetLib|NetworkPkg/Library/DxeNetLib/DxeNetLib.inf
-  IpIoLib|NetworkPkg/Library/DxeIpIoLib/DxeIpIoLib.inf
-  UdpIoLib|NetworkPkg/Library/DxeUdpIoLib/DxeUdpIoLib.inf
-  TcpIoLib|NetworkPkg/Library/DxeTcpIoLib/DxeTcpIoLib.inf
-
-  # Required by ShellPkg TftpDynamicCommand
-  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
-  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
 
  # meige boardid Library
   MeigeBoardIdLib|QcomPkg/Library/MeigeBoardIdLib/MeigeBoardIdLib.inf
@@ -706,7 +694,6 @@
   ## FD Base offset (refer to .fdf for FD size)
   #################################
   gQcomTokenSpaceGuid.PcdEmbeddedFdBaseAddress|0x9FB00000
-  gQcomTokenSpaceGuid.PcdEmbeddedFdSize|0x004F0000
 
   #################################
   ## Shared Memory Base offset and size.
@@ -1098,11 +1085,11 @@
     <LibraryClasses>
     UsbfnDwc3Lib|QcomPkg/SocPkg/Kodiak/Library/UsbfnDwc3Lib/UsbfnDwc3Lib.inf
   }
-  QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
-  QcomPkg/Drivers/XhciDxe/XhciDxe.inf
-  QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
+  #QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
+  #QcomPkg/Drivers/XhciDxe/XhciDxe.inf
+  #QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
   #QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
-    QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
+  #QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
   QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
   QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
   QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf  {
@@ -1293,32 +1280,3 @@
   # Qcom Board info DT fixup Driver
   #
   QcomPkg/Drivers/BoardInfoDxe/BoardInfoDxe.inf
-
-  # ── Network Stack Components (Elo PXE boot / QCS6490) ────────────────────
-  # Full UEFI network stack above EFI_SIMPLE_NETWORK_PROTOCOL.
-  # AX88179A USB NIC (Ax88179.efi) already produces SNP directly;
-  # SnpDxe is included for any UNDI-based NIC present via PCIe.
-  NetworkPkg/SnpDxe/SnpDxe.inf
-  NetworkPkg/DpcDxe/DpcDxe.inf
-  NetworkPkg/MnpDxe/MnpDxe.inf
-  NetworkPkg/ArpDxe/ArpDxe.inf
-  NetworkPkg/Dhcp4Dxe/Dhcp4Dxe.inf
-  NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
-  NetworkPkg/Ip4Dxe/Ip4Dxe.inf
-  NetworkPkg/Ip6Dxe/Ip6Dxe.inf
-  NetworkPkg/Udp4Dxe/Udp4Dxe.inf
-  NetworkPkg/Udp6Dxe/Udp6Dxe.inf
-  NetworkPkg/Mtftp4Dxe/Mtftp4Dxe.inf
-  NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
-  NetworkPkg/TcpDxe/TcpDxe.inf
-  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf
-  NetworkPkg/VlanConfigDxe/VlanConfigDxe.inf
-
-  # -- UEFI Shell tftp command (Elo QCS6490 / Kodiak LA) --------------------
-  # PcdShellLibAutoInitialize=FALSE prevents UefiShellLib from calling
-  # ShellInitialize() at DXE entry, which would ASSERT because the Shell
-  # protocol is not yet installed when dynamic-command drivers load.
-  ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf {
-    <PcdsFixedAtBuild>
-      gEfiShellPkgTokenSpaceGuid.PcdShellLibAutoInitialize|FALSE
-  }
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.fdf b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.fdf
index 7db083c3bb..3ac0630732 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.fdf
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.fdf
@@ -1,6 +1,5 @@
 #/** @file Core.fdf
 # FLASH layout file for LA Kodiak
-# Network stack additions for Elo PXE boot (QCS6490 / Kodiak LA).
 #
 # Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 # All rights reserved.
@@ -33,10 +32,10 @@
 
 [FD.KODIAK_EFI]
 BaseAddress   = 0x9FB00000|gEmbeddedTokenSpaceGuid.PcdEmbeddedFdBaseAddress  #The base address of the FLASH Device.
-Size          = 0x004F0000|gQcomTokenSpaceGuid.PcdEmbeddedFdSize         #The size in bytes of the FLASH Device
+Size          = 0x00410000|gQcomTokenSpaceGuid.PcdEmbeddedFdSize         #The size in bytes of the FLASH Device
 ErasePolarity = 1
 BlockSize     = 0x1000
-NumBlocks     = 0x4F0
+NumBlocks     = 0x410
 
 ################################################################################
 #
@@ -55,7 +54,7 @@ NumBlocks     = 0x4F0
 ################################################################################
 
 # 512 bytes of configuration header & 8 bytes of image header
-0x00000000|0x004F0000
+0x00000000|0x00410000
 gEmbeddedTokenSpaceGuid.PcdFlashFvMainBase|gEmbeddedTokenSpaceGuid.PcdFlashFvMainSize
 FV = FVMAIN_COMPACT
 
@@ -320,11 +319,11 @@ INF QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
   # USB Support
   #
 INF QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
-INF QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
-INF QcomPkg/Drivers/XhciDxe/XhciDxe.inf
-INF QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
+#INF QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
+#INF QcomPkg/Drivers/XhciDxe/XhciDxe.inf
+#INF QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
 #INF QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
-INF QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
+#INF QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
 INF QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
 INF QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
 INF QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
@@ -492,28 +491,6 @@ INF QcomPkg/Drivers/AX88179UsbEthDxe/AX88179UsbEthDxe.inf
   }
 #endif
 
-  # ── Network Stack (Elo PXE boot / QCS6490) ───────────────────────────────
-  # Load order: network stack drivers must be loaded BEFORE Ax88179.efi
-  # so that UefiPxeBcDxe can bind to the SNP interface it produces.
-  INF  NetworkPkg/SnpDxe/SnpDxe.inf
-  INF  NetworkPkg/DpcDxe/DpcDxe.inf
-  INF  NetworkPkg/MnpDxe/MnpDxe.inf
-  INF  NetworkPkg/ArpDxe/ArpDxe.inf
-  INF  NetworkPkg/Dhcp4Dxe/Dhcp4Dxe.inf
-  INF  NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
-  INF  NetworkPkg/Ip4Dxe/Ip4Dxe.inf
-  INF  NetworkPkg/Ip6Dxe/Ip6Dxe.inf
-  INF  NetworkPkg/Udp4Dxe/Udp4Dxe.inf
-  INF  NetworkPkg/Udp6Dxe/Udp6Dxe.inf
-  INF  NetworkPkg/Mtftp4Dxe/Mtftp4Dxe.inf
-  INF  NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
-  INF  NetworkPkg/TcpDxe/TcpDxe.inf
-  INF  NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf
-  INF  NetworkPkg/VlanConfigDxe/VlanConfigDxe.inf
-
-  # UEFI Shell tftp command
-  INF  ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
-
   INF EmbeddedPkg/Ebl/Ebl.inf
 
 
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/CoreAux.fdf.inc b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/CoreAux.fdf.inc
index 1e83100339..62d94926fb 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/CoreAux.fdf.inc
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/CoreAux.fdf.inc
@@ -74,11 +74,11 @@ INF QcomPkg/Drivers/LimitsDxe/LimitsDxe.inf
 #
 #  USB Support
 #
-INF QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
-INF QcomPkg/Drivers/XhciDxe/XhciDxe.inf
-INF QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
+#INF QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
+#INF QcomPkg/Drivers/XhciDxe/XhciDxe.inf
+#INF QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
 #INF QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
-INF QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
+#INF QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
 #INF QcomPkg/Drivers/Usb4Dxe/Usb4Dxe.inf
 #INF QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
 #INF QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/uefiplat.cfg b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/uefiplat.cfg
index d02e747acc..4d4963684f 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/uefiplat.cfg
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/uefiplat.cfg
@@ -75,7 +75,7 @@ NumCpusFuseAddr = 0x5C04C
 EnableShell = 0x1
 
 # Tune based on heap needs for Initial tables, FV decompression and MMU tables through Dxe/BDS
-InitialPagePoolCount = 0xD00
+InitialPagePoolCount = 0xB00
 
 ## Shared IMEM (Cookies, Offsets)
 SharedIMEMBaseAddr    = 0x146AA000
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/PmicCoreLib.inf b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/PmicCoreLib.inf
index 19f18e868b..c6ae54ea70 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/PmicCoreLib.inf
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/PmicCoreLib.inf
@@ -70,7 +70,6 @@
 
 [Protocols]
  gQcomPmicPonProtocolGuid
- gQcomPmicGpioProtocolGuid
  gEfiTLMMProtocolGuid
 
 [Depex]
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/pm_core.c b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/pm_core.c
index 36dfcf8c9c..92da002b3a 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/pm_core.c
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/pm_core.c
@@ -110,7 +110,6 @@ extern EFI_QCOM_PMIC_SCHG_PROTOCOL        PmicSchg_P_ProtocolImplementation;
 
 ===========================================================================*/
 static pm_err_flag_type pm_i2c_sid_config(void);
-static pm_err_flag_type pm_platform_usb_hub_gpio_init(void);
 
 /*===========================================================================
 
@@ -147,169 +146,10 @@ pm_err_flag_type pm_install_target_protocols(EFI_HANDLE ImageHandle, EFI_SYSTEM_
 
   err_flag = (Status == EFI_SUCCESS) ? PM_ERR_FLAG_SUCCESS : PM_ERR_FLAG_FAILURE;
 
-  if (err_flag == PM_ERR_FLAG_SUCCESS) {
-    err_flag |= pm_platform_usb_hub_gpio_init();
-  }
-
   return err_flag;
 }
 
 
-static pm_err_flag_type pm_platform_usb_hub_gpio_init(void)
-{
-  EFI_TLMM_PROTOCOL             *TLMMProtocol = NULL;
-  EFI_QCOM_PMIC_GPIO_PROTOCOL   *PmicGpioProtocol = NULL;
-  EFI_STATUS                     Status;
-  UINT32                         GpioConfig;
-  STATIC BOOLEAN                 Initialized = FALSE;
-
-  if (Initialized) {
-    return PM_ERR_FLAG_SUCCESS;
-  }
-
-  Status = gBS->LocateProtocol (&gEfiTLMMProtocolGuid, NULL, (VOID **)&TLMMProtocol);
-  if (EFI_ERROR (Status) || (TLMMProtocol == NULL)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to locate TLMM for USB hub GPIO - %r\n", Status));
-    return PM_ERR_FLAG_FAILURE;
-  }
-
-  // PCIe hub power: GPIO 99
-  GpioConfig = (UINT32)EFI_GPIO_CFG (99, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 99 - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_HIGH_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 99 high - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 99 configured and powered up\n"));
-    }
-  }
-
-  // PCIe hub power: GPIO 100
-  GpioConfig = (UINT32)EFI_GPIO_CFG (100, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 100 - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_HIGH_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 100 high - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 100 configured and powered up\n"));
-    }
-  }
-
-  // PCIe hub power: GPIO 106
-  GpioConfig = (UINT32)EFI_GPIO_CFG (106, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 106 - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_HIGH_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 106 high - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 106 configured and powered up\n"));
-    }
-  }
-
-  // USB_SW1_SEL: GPIO 50, default high (select Type-C)
-  GpioConfig = (UINT32)EFI_GPIO_CFG (50, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 50 (USB_SW1_SEL) - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_HIGH_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 50 high - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 50 (USB_SW1_SEL) configured and set high\n"));
-    }
-  }
-
-  // USB_SW2_SEL: GPIO 15, default high
-  GpioConfig = (UINT32)EFI_GPIO_CFG (15, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 15 (USB_SW2_SEL) - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_HIGH_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 15 high - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 15 (USB_SW2_SEL) configured and set high\n"));
-    }
-  }
-
-  // USB_SW3_SEL: GPIO 14, default low
-  GpioConfig = (UINT32)EFI_GPIO_CFG (14, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 14 (USB_SW3_SEL) - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_LOW_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 14 low - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 14 (USB_SW3_SEL) configured and set low\n"));
-    }
-  }
-
-  //GPIO 6
-  GpioConfig = (UINT32)EFI_GPIO_CFG (6, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 6 (hub_5v_en) - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_HIGH_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 6 high - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 6 (hub_5v_en) configured and set high\n"));
-    }
-  }
-
-  //GPIO 104
-  GpioConfig = (UINT32)EFI_GPIO_CFG (104, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-  Status = TLMMProtocol->ConfigGpio (GpioConfig, TLMM_GPIO_ENABLE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure GPIO 104 (AX88179A) - %r\n", Status));
-  } else {
-    Status = TLMMProtocol->GpioOut (GpioConfig, GPIO_HIGH_VALUE);
-    if (EFI_ERROR (Status)) {
-      DEBUG ((EFI_D_ERROR, "pm_core: Failed to set GPIO 104 high - %r\n", Status));
-    } else {
-      DEBUG ((EFI_D_ERROR, "pm_core: GPIO 104 (AX88179A) configured and set high\n"));
-    }
-  }
-
-  // pcie-hub-typeAadb: PM7325 GPIO 9, default high (TYPE-A ADB)
-  Status = gBS->LocateProtocol (&gQcomPmicGpioProtocolGuid, NULL, (VOID **)&PmicGpioProtocol);
-  if (EFI_ERROR (Status) || (PmicGpioProtocol == NULL)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to locate PMIC GPIO for pcie-hub-typeAadb - %r\n", Status));
-    return PM_ERR_FLAG_FAILURE;
-  }
-
-  Status = PmicGpioProtocol->CfgMode (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_DIG_OUT);
-  Status |= PmicGpioProtocol->SetVoltageSource (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_VIN0);
-  Status |= PmicGpioProtocol->SetOutBufCfg (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_OUT_BUF_CFG_CMOS);
-  Status |= PmicGpioProtocol->SetOutSrcCfg (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_SRC_GND);
-  Status |= PmicGpioProtocol->SetOutDrvStr (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_OUT_DRV_STR_MEDIUM);
-  Status |= PmicGpioProtocol->SetOutputLevel (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_LEVEL_HIGH);
-  Status |= PmicGpioProtocol->Enable (PMIC_B, EFI_PM_GPIO_9, TRUE);
-  if (EFI_ERROR (Status)) {
-    DEBUG ((EFI_D_ERROR, "pm_core: Failed to configure PM7325 GPIO 9 (pcie-hub-typeAadb) - %r\n", Status));
-    return PM_ERR_FLAG_FAILURE;
-  }
-
-  DEBUG ((EFI_D_ERROR, "pm_core: PM7325 GPIO 9 (pcie-hub-typeAadb) configured and set high\n"));
-  Initialized = TRUE;
-  return PM_ERR_FLAG_SUCCESS;
-}
-
-
 pm_err_flag_type pm_post_pmic_initialization(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
 {
   pm_err_flag_type err_flag = PM_ERR_FLAG_SUCCESS;
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.c b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.c
index cdbfd50195..a6283fd405 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.c
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.c
@@ -67,11 +67,6 @@
 #include <Library/PcieConfigLib.h>
 #include <Library/DebugLib.h>
 #include <Library/UefiCfgLib.h>
-#include <Protocol/EFITlmm.h>
-#if 0  // USB hub GPIO moved to pm_core.c pm_platform_usb_hub_gpio_init()
-#include <Protocol/EFIPmicGpio.h>
-#include <api/pmic/pm/pm_version.h>
-#endif
 
 #include "pcie_rp_cfg_svc.h"
 #include "pcie_osal.h"
@@ -233,8 +228,7 @@ EFI_STATUS PcieConfig_isPort1Supported(UINT8* flag)
   }
 
   /* Check boot_from_nvme() first */
-  //BootFromNvmeFlag = boot_from_nvme();
-  BootFromNvmeFlag = 1;
+  BootFromNvmeFlag = boot_from_nvme();
 
   /* Only check config values if boot_from_nvme is FALSE */
   if (!BootFromNvmeFlag)
@@ -428,148 +422,6 @@ PcieConfigLibEnableRootPorts (VOID)
    UINT32            TotalRootPorts = 0;
    uint32            enabled_ports;
    pcie_status_t     pStatus;
-   //EFI_STATUS                Status;
-
-  DEBUG ((EFI_D_ERROR, "[XHCI] DriverBindingStart: Entry\n"));
-
-#if 0  // Platform USB/PCIe GPIO: configured in pm_core.c pm_platform_usb_hub_gpio_init()
-   STATIC BOOLEAN          EnablePCIePowerGpios = FALSE;
-
-  if (!EnablePCIePowerGpios) {
-    EFI_TLMM_PROTOCOL       *TLMMProtocol = NULL;
-    // Locate TLMM protocol for GPIO configuration
-    Status = gBS->LocateProtocol (
-                    &gEfiTLMMProtocolGuid,
-                    NULL,
-                    (VOID **)&TLMMProtocol
-                    );
-    
-    if (!EFI_ERROR (Status) && (TLMMProtocol != NULL)) {
-      UINT32 GpioConfig99, GpioConfig100, GpioConfig106;
-      
-      // Configure GPIO 99 as output, pull up, 2MA
-      GpioConfig99 = (UINT32)EFI_GPIO_CFG(99, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-      Status = TLMMProtocol->ConfigGpio(GpioConfig99, TLMM_GPIO_ENABLE);
-      if (EFI_ERROR (Status)) {
-        DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: Failed to configure GPIO 99 - %r\n", Status));
-      } else {
-        // Set GPIO 99 to high (power up)
-        Status = TLMMProtocol->GpioOut(GpioConfig99, GPIO_HIGH_VALUE);
-        if (EFI_ERROR (Status)) {
-          DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: Failed to set GPIO 99 high - %r\n", Status));
-        } else {
-          DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: GPIO 99 configured and powered up\n"));
-        }
-      }
-      
-      // Configure GPIO 100 as output, pull up, 2MA
-      GpioConfig100 = (UINT32)EFI_GPIO_CFG(100, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-      Status = TLMMProtocol->ConfigGpio(GpioConfig100, TLMM_GPIO_ENABLE);
-      if (EFI_ERROR (Status)) {
-        DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: Failed to configure GPIO 100 - %r\n", Status));
-      } else {
-        // Set GPIO 100 to high (power up)
-        Status = TLMMProtocol->GpioOut(GpioConfig100, GPIO_HIGH_VALUE);
-        if (EFI_ERROR (Status)) {
-          DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: Failed to set GPIO 100 high - %r\n", Status));
-        } else {
-          DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: GPIO 100 configured and powered up\n"));
-        }
-      }
-      
-      // Configure GPIO 106 as output, pull up, 2MA
-      GpioConfig106 = (UINT32)EFI_GPIO_CFG(106, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-      Status = TLMMProtocol->ConfigGpio(GpioConfig106, TLMM_GPIO_ENABLE);
-      if (EFI_ERROR (Status)) {
-        DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: Failed to configure GPIO 106 - %r\n", Status));
-      } else {
-        // Set GPIO 106 to high (power up)
-        Status = TLMMProtocol->GpioOut(GpioConfig106, GPIO_HIGH_VALUE);
-        if (EFI_ERROR (Status)) {
-          DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: Failed to set GPIO 106 high - %r\n", Status));
-        } else {
-          DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: GPIO 106 configured and powered up\n"));
-        }
-      }
-
-      UINT32 GpioConfig50, GpioConfig15, GpioConfig14;
-
-      // USB_SW1_SEL: GPIO 50, default high (select Type-C)
-      GpioConfig50 = (UINT32)EFI_GPIO_CFG(50, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-      Status = TLMMProtocol->ConfigGpio(GpioConfig50, TLMM_GPIO_ENABLE);
-      if (EFI_ERROR (Status)) {
-        DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to configure GPIO 50 (USB_SW1_SEL) - %r\n", Status));
-      } else {
-        Status = TLMMProtocol->GpioOut(GpioConfig50, GPIO_HIGH_VALUE);
-        if (EFI_ERROR (Status)) {
-          DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to set GPIO 50 high - %r\n", Status));
-        } else {
-          DEBUG ((EFI_D_ERROR, "PcieConfigLib: GPIO 50 (USB_SW1_SEL) configured and set high\n"));
-        }
-      }
-
-      // USB_SW2_SEL: GPIO 15, default high
-      GpioConfig15 = (UINT32)EFI_GPIO_CFG(15, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-      Status = TLMMProtocol->ConfigGpio(GpioConfig15, TLMM_GPIO_ENABLE);
-      if (EFI_ERROR (Status)) {
-        DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to configure GPIO 15 (USB_SW2_SEL) - %r\n", Status));
-      } else {
-        Status = TLMMProtocol->GpioOut(GpioConfig15, GPIO_HIGH_VALUE);
-        if (EFI_ERROR (Status)) {
-          DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to set GPIO 15 high - %r\n", Status));
-        } else {
-          DEBUG ((EFI_D_ERROR, "PcieConfigLib: GPIO 15 (USB_SW2_SEL) configured and set high\n"));
-        }
-      }
-
-      // USB_SW3_SEL: GPIO 14, default low
-      GpioConfig14 = (UINT32)EFI_GPIO_CFG(14, 0, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA);
-      Status = TLMMProtocol->ConfigGpio(GpioConfig14, TLMM_GPIO_ENABLE);
-      if (EFI_ERROR (Status)) {
-        DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to configure GPIO 14 (USB_SW3_SEL) - %r\n", Status));
-      } else {
-        Status = TLMMProtocol->GpioOut(GpioConfig14, GPIO_LOW_VALUE);
-        if (EFI_ERROR (Status)) {
-          DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to set GPIO 14 low - %r\n", Status));
-        } else {
-          DEBUG ((EFI_D_ERROR, "PcieConfigLib: GPIO 14 (USB_SW3_SEL) configured and set low\n"));
-        }
-      }
-
-      // pcie-hub-typeAadb: PM7325 GPIO 9, default high (TYPE-A ADB)
-      {
-        EFI_QCOM_PMIC_GPIO_PROTOCOL  *PmicGpioProtocol = NULL;
-        EFI_STATUS                    PmicStatus;
-
-        PmicStatus = gBS->LocateProtocol (
-                            &gQcomPmicGpioProtocolGuid,
-                            NULL,
-                            (VOID **)&PmicGpioProtocol
-                            );
-        if (!EFI_ERROR (PmicStatus) && (PmicGpioProtocol != NULL)) {
-          PmicStatus = PmicGpioProtocol->CfgMode (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_DIG_OUT);
-          PmicStatus |= PmicGpioProtocol->SetVoltageSource (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_VIN0);
-          PmicStatus |= PmicGpioProtocol->SetOutBufCfg (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_OUT_BUF_CFG_CMOS);
-          PmicStatus |= PmicGpioProtocol->SetOutSrcCfg (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_SRC_GND);
-          PmicStatus |= PmicGpioProtocol->SetOutDrvStr (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_OUT_DRV_STR_MEDIUM);
-          PmicStatus |= PmicGpioProtocol->SetOutputLevel (PMIC_B, EFI_PM_GPIO_9, EFI_PM_GPIO_LEVEL_HIGH);
-          PmicStatus |= PmicGpioProtocol->Enable (PMIC_B, EFI_PM_GPIO_9, TRUE);
-          if (EFI_ERROR (PmicStatus)) {
-            DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to configure PM7325 GPIO 9 (pcie-hub-typeAadb) - %r\n", PmicStatus));
-          } else {
-            DEBUG ((EFI_D_ERROR, "PcieConfigLib: PM7325 GPIO 9 (pcie-hub-typeAadb) configured and set high\n"));
-          }
-        } else {
-          DEBUG ((EFI_D_ERROR, "PcieConfigLib: Failed to locate PMIC GPIO protocol for pcie-hub-typeAadb - %r\n", PmicStatus));
-        }
-      }
-
-      EnablePCIePowerGpios = TRUE;
-    } else {
-      DEBUG ((EFI_D_ERROR, "XhcDriverBindingStart: Failed to locate TLMM protocol for GPIO configuration - %r\n", Status));
-    }
-  }
-#endif
 
    /* Enable default config options before pcie_rp_lib_init which loads the
     * enabled configurations */
diff --git a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/XBLLoader/ExtDrivers/boot_uart.c b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/XBLLoader/ExtDrivers/boot_uart.c
index c158e8774c..966495b3ad 100755
--- a/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/XBLLoader/ExtDrivers/boot_uart.c
+++ b/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/XBLLoader/ExtDrivers/boot_uart.c
@@ -94,7 +94,7 @@ boolean boot_uart_init(void)
 
   do {
 	uart_connection_status = uart_is_cable_connected();
-
+	 
     if (uart_connection_status == FALSE) {                                                                  		 
 	  break;
 	}
diff --git a/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/Tftp.c b/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/Tftp.c
index 8ecc433d76..589c227d8b 100755
--- a/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/Tftp.c
+++ b/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/Tftp.c
@@ -1081,10 +1081,7 @@ CheckPacket (
 }
 
 /**
-  Publish HII string packages to HII database.
-
-  Uses HiiAddPackages instead of UEFI_HII_RESOURCE_SECTION because
-  CLANG140LINUX arm-elfcopy cannot produce elf64-littleaarch64 objects.
+  Retrieve HII package list from ImageHandle and publish to HII database.
 
   @param ImageHandle            The image handle of the process.
 
@@ -1095,15 +1092,38 @@ InitializeHiiPackage (
   EFI_HANDLE                  ImageHandle
   )
 {
+  EFI_STATUS                  Status;
+  EFI_HII_PACKAGE_LIST_HEADER *PackageList;
+  EFI_HII_HANDLE              HiiHandle;
+
   //
-  // CLANG140LINUX/QCOM: arm-elfcopy does not support elf64-littleaarch64,
-  // so UEFI_HII_RESOURCE_SECTION is disabled. Use HiiAddPackages instead,
-  // which embeds the string array directly without a PE HII resource section.
+  // Retrieve HII package list from ImageHandle
   //
-  return HiiAddPackages (
-           &gEfiCallerIdGuid,
-           ImageHandle,
-           STRING_ARRAY_NAME,
-           NULL
-           );
+  Status = gBS->OpenProtocol (
+                  ImageHandle,
+                  &gEfiHiiPackageListProtocolGuid,
+                  (VOID **)&PackageList,
+                  ImageHandle,
+                  NULL,
+                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
+                  );
+  ASSERT_EFI_ERROR (Status);
+  if (EFI_ERROR (Status)) {
+    return NULL;
+  }
+
+  //
+  // Publish HII package list to HII Database.
+  //
+  Status = gHiiDatabase->NewPackageList (
+                           gHiiDatabase,
+                           PackageList,
+                           NULL,
+                           &HiiHandle
+                           );
+  ASSERT_EFI_ERROR (Status);
+  if (EFI_ERROR (Status)) {
+    return NULL;
+  }
+  return HiiHandle;
 }
diff --git a/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf b/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
index 8028af8666..695af5559a 100755
--- a/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
+++ b/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf
@@ -19,10 +19,8 @@
   UNLOAD_IMAGE                   = TftpUnload
 #
 #  This flag specifies whether HII resource section is generated into PE image.
-#  Disabled for CLANG140LINUX/QCOM: arm-elfcopy does not support elf64-littleaarch64
-#  (HII strings still work via HiiAddPackages + STRING_ARRAY_NAME).
 #
-  UEFI_HII_RESOURCE_SECTION      = FALSE
+  UEFI_HII_RESOURCE_SECTION      = TRUE
 
 [Sources.common]
   Tftp.uni
@@ -55,6 +53,7 @@
   gEfiManagedNetworkServiceBindingProtocolGuid   ## CONSUMES
   gEfiMtftp4ServiceBindingProtocolGuid           ## CONSUMES
   gEfiMtftp4ProtocolGuid                         ## CONSUMES
+  gEfiHiiPackageListProtocolGuid                 ## CONSUMES
   gEfiShellDynamicCommandProtocolGuid            ## PRODUCES
 
 [DEPEX]
diff --git a/mg_build_tools/mg_build.py b/mg_build_tools/mg_build.py
index 4f5e9369ac..89ba4eaeb9 100755
--- a/mg_build_tools/mg_build.py
+++ b/mg_build_tools/mg_build.py
@@ -89,7 +89,7 @@ SUB_SYS_BUILD_RULES = {
         "depends": ["SECTOOL", "PYTHON"],
         "cmd": """
         cp ${MG_WORK_ROOT}/PROJECT/odm_features.h ${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Include/
-        python -u "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot_tools/buildex.py" -v WP -t kodiak,QcomToolsPkg -v LAA -r RELEASE,DEBUG
+        python -u "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot_tools/buildex.py" -v WP -t kodiak,QcomToolsPkg -v LAA -r RELEASE
 
         source_files=(
                     "${MG_WORK_ROOT}/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Bin/LAA/RELEASE/xbl.elf"
-- 
2.34.1
```

## 补丁验证

- 验证方式：134 服务器 qimpsdk 工作树 `git diff FETCH_HEAD~1 FETCH_HEAD` 与 format-patch 主体逐字节比对（排除 git 2.34.1 签名尾行）
- 结果：✅ 可干净应用（补丁内容与已合入提交 diff 一致；工作树含本地未提交改动，未做工作树级 apply）

## 源码归档

| 内容 | 路径 | 说明 |
|------|------|------|
| kernel_driver/ | [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/\|BOOT.MXF.1.0.c1/]] | 15 个 UEFI 源文件（补丁后合并版本，保留原相对路径） |
| kernel_driver/ | [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/mg_build_tools/mg_build.py\|mg_build.py]] | 构建工具脚本 |
| patches/ | [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/patches/196565.patch\|196565.patch]] | 完整补丁（已清隐私） |

## 引用文件索引

| 文件 | 完整路径 | 说明 |
|------|---------|------|
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/UsbBusDxe/UsbEnumer.c\|UsbEnumer.c]] | BOOT.MXF.1.0.c1/.../QcomPkg/Drivers/UsbBusDxe/ | USB 总线枚举驱动 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/Xhci.c\|Xhci.c]] | BOOT.MXF.1.0.c1/.../Drivers/XhciDxe/ | XHCI 控制器驱动 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/Drivers/XhciDxe/XhciDxe.inf\|XhciDxe.inf]] | BOOT.MXF.1.0.c1/.../Drivers/XhciDxe/ | XHCI INF |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Apriori.fdf.inc\|Apriori.fdf.inc]] | BOOT.MXF.1.0.c1/.../SocPkg/Kodiak/Common/ | Apriori 驱动列表 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.dsc.inc\|Core.dsc.inc]] | BOOT.MXF.1.0.c1/.../SocPkg/Kodiak/Common/ | DSC 模块列表 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/Core.fdf\|Core.fdf]] | BOOT.MXF.1.0.c1/.../SocPkg/Kodiak/Common/ | FDF 固件清单 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/CoreAux.fdf.inc\|CoreAux.fdf.inc]] | BOOT.MXF.1.0.c1/.../SocPkg/Kodiak/Common/ | Aux FDF |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Common/uefiplat.cfg\|uefiplat.cfg]] | BOOT.MXF.1.0.c1/.../SocPkg/Kodiak/Common/ | 平台配置 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/pm_core.c\|pm_core.c]] | BOOT.MXF.1.0.c1/.../PmicLib/core/la/ | PMIC 核心库 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Kodiak/Library/PmicLib/core/la/PmicCoreLib.inf\|PmicCoreLib.inf]] | BOOT.MXF.1.0.c1/.../PmicLib/core/la/ | PMIC 库 INF |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/SocPkg/Library/PcieConfigLib/PcieConfigLib.c\|PcieConfigLib.c]] | BOOT.MXF.1.0.c1/.../SocPkg/Library/PcieConfigLib/ | PCIe 配置库 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/boot/QcomPkg/XBLLoader/ExtDrivers/boot_uart.c\|boot_uart.c]] | BOOT.MXF.1.0.c1/.../XBLLoader/ExtDrivers/ | XBL UART 初始化 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/Tftp.c\|Tftp.c]] | BOOT.MXF.1.0.c1/.../edk2/ShellPkg/.../TftpDynamicCommand/ | TFTP 动态命令 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/BOOT.MXF.1.0.c1/boot_images/edk2/ShellPkg/DynamicCommand/TftpDynamicCommand/TftpDynamicCommand.inf\|TftpDynamicCommand.inf]] | BOOT.MXF.1.0.c1/.../edk2/ShellPkg/.../TftpDynamicCommand/ | TFTP INF |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/mg_build_tools/mg_build.py\|mg_build.py]] | mg_build_tools/ | 构建工具脚本 |
| [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/patches/196565.patch\|196565.patch]] | 91.源码与补丁索引/patches/ | 补丁（已清隐私） |

_Author: wangguanran_
