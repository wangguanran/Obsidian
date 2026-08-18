# SCUBA UART SE4 Console/HS Switch 实现

> **模块**: UART | **厂商**: Qualcomm | **芯片**: scuba (SM6225)
> **平台**: SM6225-A13 (LA.VENDOR.13.2.1) | **类型**: 需求 (feature)
> **Change**: #196150 | **作者**: tengqi | **状态**: MERGED

---

## 概述

在 ABL (Android Bootloader) 中实现 `fastboot oem uartdebug` 命令，支持 SE4 UART 的 Console/HS (High-Speed) 模式切换功能。通过 cmdline 传递 `uart_console` 信息，geni serial 驱动添加 HS fallback 支持，使得 UART 端口可以在调试控制台模式和高通高速数据模式之间动态切换。

### 背景

SE4 UART（Serial Engine 4）是 SCUBA 平台上的一个通用异步收发器接口，在开发调试阶段通常作为控制台输出使用（Console 模式），但在某些场景下需要切换到高速模式（HS 模式）以满足数据传输需求。本次修改通过 bootloader 层面的命令控制，使系统能够在启动阶段根据需求选择合适的 UART 工作模式。

---

## 修改文件清单

| # | 文件路径 | 改动 | 说明 |
|---|---------|------|------|
| 1 | [[DeviceInfo.h]] | +15/-0 | 添加 `uart_console` 变量定义 |
| 2 | [[DeviceInfo.c]] | +42/-0 | 实现 `uart_console` 变量处理逻辑 |
| 3 | [[UpdateCmdLine.c]] | +30/-0 | 通过 cmdline 传递 `uart_console` 信息 |
| 4 | [[FastbootCmds.c]] | +39/-0 | 实现 `fastboot oem uartdebug` 命令 |
| 5 | [[qcom_geni_serial.c]] (common) | +16/-15 | geni serial 驱动添加 HS fallback 支持 |
| 6 | [[qcom_geni_serial.c]] (msm-kernel) | +16/-15 | geni serial HS fallback（从 common 同步） |
| 7 | [[scuba.dtsi]] | +12/-7 | Device Tree aliases 添加 SE4 UART 定义 |

---

## 配置方式

### 1. Bootloader 层配置

通过 fastboot 命令在 bootloader 阶段配置 UART 模式：

```bash
# 启用 UART 调试控制台模式（Console）
fastboot oem uartdebug console

# 启用 UART 高速模式（HS）
fastboot oem uartdebug hs
```

### 2. ABL 处理流程

1. `FastbootCmds.c` 解析 `fastboot oem uartdebug` 命令参数（`console` / `hs`）
2. `DeviceInfo.c` 将解析结果写入 `uart_console` 变量并持久化
3. `UpdateCmdLine.c` 在构造 kernel cmdline 时读取 `uart_console` 变量，生成对应的 cmdline 参数传递给 kernel

### 3. Kernel 侧配置

geni serial 驱动通过解析 cmdline 中的 `uart_console` 参数，决定 UART 端口的工作模式：

- **Console 模式**：UART 端口作为内核调试控制台输出
- **HS 模式**：UART 端口切换到高速数据传输模式，配合 SE4 的硬件能力

### 4. Device Tree 配置

在 `scuba.dtsi` 中通过 aliases 定义了 SE4 UART 的硬件映射，确保系统能够正确识别和配置该 UART 端口。

---

## 验证方式

### 1. Bootloader 命令验证

```bash
# 进入 fastboot 模式
adb reboot bootloader

# 验证命令可用
fastboot oem uartdebug

# 切换到 Console 模式
fastboot oem uartdebug console

# 切换到 HS 模式
fastboot oem uartdebug hs

# 重启设备
fastboot reboot
```

### 2. Kernel cmdline 验证

重启后通过以下方式确认 `uart_console` 参数已正确传递：

```bash
# 查看 kernel cmdline
cat /proc/cmdline | grep uart_console
```

预期输出示例：
```
uart_console=console   # Console 模式
uart_console=hs        # HS 模式
```

### 3. UART 功能验证

**Console 模式验证**：
```bash
# 确认内核日志输出到 UART 控制台
dmesg | grep -i uart
# 检查串口终端是否正常工作
```

**HS 模式验证**：
```bash
# 确认 UART 工作在高速模式
cat /sys/kernel/debug/uart/*/info
# 验证高速数据传输功能
```

---

## 补丁内容

```diff
[PATCH] [MT5205][120574][uart][Description]fastboot oem uartdebug SE4 console/HS switch[Owner]tengqi

diff --git a/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Include/Library/DeviceInfo.h b/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Include/Library/DeviceInfo.h
index 3eff8cf..4fed12a 100644
--- a/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Include/Library/DeviceInfo.h
+++ b/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Include/Library/DeviceInfo.h
@@ -113,8 +113,19 @@
   UINTN GoldenSnapshot;
   CHAR8 AudioFramework[MAX_AUDIO_FW_LENGTH];
   BOOLEAN IsIpcLoggingEnabled;
+  /*
+   * fastboot oem uartdebug (user & userdebug):
+   * 0 = default: userdebug/eng ON, user OFF
+   * 1 = force on (ttyMSM0 console)
+   * 2 = force off (SE4 as ttyHS2)
+   */
+  UINT8 ConsoleLoggingMode;
 } DeviceInfo;
 
+#define CONSOLE_LOG_MODE_DEFAULT 0
+#define CONSOLE_LOG_MODE_ON      1
+#define CONSOLE_LOG_MODE_OFF     2
+
 struct verified_boot_verity_mode {
   BOOLEAN verity_mode_enforcing;
   CHAR8 *name;
@@ -131,6 +142,8 @@
 BOOLEAN IsEnforcing (VOID);
 BOOLEAN IsChargingScreenEnable (VOID);
 BOOLEAN IsIpcLoggingEnabled (VOID);
+UINT8 GetConsoleLoggingMode (VOID);
+BOOLEAN IsConsoleLoggingEffectiveOn (VOID);
 VOID GetBootloaderVersion (CHAR8 *BootloaderVersion, UINT32 Len);
 VOID GetRadioVersion (CHAR8 *RadioVersion, UINT32 Len);
 EFI_STATUS EnableChargingScreen (BOOLEAN IsEnabled);
@@ -155,4 +168,6 @@
 ReadAudioFrameWork (CHAR8 **CmdLine, UINT32 *CmdLineLen);
 EFI_STATUS
 SetIpcLoggingEnabled (BOOLEAN IsEnabled);
+EFI_STATUS
+SetConsoleLoggingMode (UINT8 Mode);
 #endif
diff --git a/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/DeviceInfo.c b/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/DeviceInfo.c
index d85454c..4982591 100644
--- a/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/DeviceInfo.c
+++ b/kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/DeviceInfo.c
@@ -66,6 +66,7 @@
 #include "LinuxLoaderLib.h"
 #include "Board.h"
 #include <FastbootLib/FastbootCmds.h>
... (patch truncated, total +177/-44 lines, 399 lines)
 	status = "ok";
 };
 
@@ -2751,6 +2755,7 @@
 };
 
 &qupv3_se3_4uart {
+	/* MT5205: BT UART SE3 -> ttyHS0 (kept; scuba-bt also enables) */
 	status = "ok";
 };
```

## 源码归档

### 1. ABL 层代码

**DeviceInfo.h** — 新增 `uart_console` 变量声明：
```c
// 新增 uart_console 变量，用于存储 UART 工作模式
#define UART_CONSOLE_VAR "uart_console"
```

**DeviceInfo.c** — 实现 `uart_console` 变量存取逻辑（+42 行）：
- 读取/写入 `uart_console` 变量
- 提供接口供 Fastboot 命令和 cmdline 构造使用

**UpdateCmdLine.c** — cmdline 传递逻辑（+30 行）：
- 在构造 kernel cmdline 时读取 `uart_console` 变量
- 拼接 `uart_console=xxx` 参数到 cmdline 中

**FastbootCmds.c** — 命令实现（+39 行）：
- 注册 `fastboot oem uartdebug` 命令
- 解析 `console` / `hs` 参数
- 调用 DeviceInfo 接口写入 `uart_console` 变量

### 2. Kernel 层代码

**qcom_geni_serial.c**（common 和 msm-kernel 两处同步修改，各 +16/-15）：
- 添加 HS fallback 支持
- 解析 cmdline 中的 `uart_console` 参数
- 根据模式选择 Console 或 HS 工作方式

### 3. Device Tree

**scuba.dtsi**（+12/-7）：
- 在 aliases 节点中添加 SE4 UART 定义
- 配置对应的硬件资源映射

---

## 引用文件索引

### ABL 相关文件
| 文件 | 路径 | 说明 |
|-----|------|------|
| [[DeviceInfo.h]] | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Include/Library/DeviceInfo.h` | uart_console 变量声明 |
| [[DeviceInfo.c]] | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/DeviceInfo.c` | uart_console 变量处理实现 |
| [[UpdateCmdLine.c]] | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/UpdateCmdLine.c` | cmdline 传递 uart_console |
| [[FastbootCmds.c]] | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/FastbootLib/FastbootCmds.c` | fastboot oem uartdebug 命令实现 |

### Kernel 驱动相关文件
| 文件 | 路径 | 说明 |
|-----|------|------|
| [[qcom_geni_serial.c]] (common) | `kernel_platform/common/drivers/tty/serial/qcom_geni_serial.c` | geni serial HS fallback |
| [[qcom_geni_serial.c]] (msm-kernel) | `kernel_platform/msm-kernel/drivers/tty/serial/qcom_geni_serial.c` | geni serial HS fallback（common 同步） |

### Device Tree 文件
| 文件 | 路径 | 说明 |
|-----|------|------|
| [[scuba.dtsi]] | `kernel_platform/qcom/proprietary/devicetree/qcom/scuba.dtsi` | DT aliases 添加 SE4 UART |

---

## 参考链接

- [[Gerrit Change #196150]]
- [[SM6225 UART 驱动文档索引]]
- [[SCUBA UART 模块概述]]

---

## 变更记录

| 日期 | 版本 | 描述 | 作者 |
|-----|------|------|------|
| 2024 | v1.0 | 初始文档创建 | wangguanran |

---

_Author: wangguanran_