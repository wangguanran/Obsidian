# 修改历史

> **模块**: UART | **厂商**: Qualcomm | **芯片**: scuba (SM6225)
> **平台**: SM6225-A13 (LA.VENDOR.13.2.1) | **分支**: [项目代号]

---

## 当前状态

当前仓库 HEAD 处于 `a3da5a8833b0`，补丁 #196150 **尚未合入**。以下为已提交的 Gerrit 修改记录。

---

## Gerrit Change #196150

| 属性 | 值 |
|------|-----|
| Change | #196150 |
| 任务 | 120574 |
| 标题 | [[项目代号]][120574][uart] fastboot oem uartdebug SE4 console/HS switch |
| 作者 | [同事] |
| 状态 | MERGED |
| 类型 | 需求 (feature) |
| 分支 | [项目代号] |

### 涉及文件

| # | 文件路径 | 改动 | 说明 |
|---|---------|------|------|
| 1 | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Include/Library/DeviceInfo.h` | +15/-0 | 添加 `ConsoleLoggingMode` 变量声明及宏定义 |
| 2 | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/DeviceInfo.c` | +42/-0 | 实现 `GetConsoleLoggingMode` / `IsConsoleLoggingEffectiveOn` / `SetConsoleLoggingMode` |
| 3 | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/UpdateCmdLine.c` | +30/-0 | 根据 console logging 模式追加 cmdline 参数 |
| 4 | `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/FastbootLib/FastbootCmds.c` | +39/-0 | 注册 `oem uartdebug Qon/Qof` 命令 |
| 5 | `kernel_platform/common/drivers/tty/serial/qcom_geni_serial.c` | +16/-15 | geni serial 驱动 HS fallback 支持 |
| 6 | `kernel_platform/msm-kernel/drivers/tty/serial/qcom_geni_serial.c` | +16/-15 | geni serial HS fallback（从 common 同步） |
| 7 | `kernel_platform/qcom/proprietary/devicetree/qcom/scuba.dtsi` | +12/-7 | aliases 调整：SE4 UART 映射为 ttyHS2 |

### 变更摘要

总计: **7 files changed, 170 insertions(+), 37 deletions(-)**

---

## 相关提交

| 提交 | 描述 | 作者 |
|------|------|------|
| `1a96e40e9f50` | [[项目代号]][TaskID]118750/118744 config gpio69/70 gpio16/17 as uart (前置依赖) | [同事] |
| `223bd95b8dcc` | IOT_High_Mid_2024.SPF.1.0 FC 0.0.008.0 (基线) | - |

---

## 补丁验证结果

| 项目 | 结果 |
|------|------|
| 补丁文件 | `patches/196150.patch` |
| 源码匹配 | ✅ 补丁可成功应用到预修改源码 |
| 新增行数 | 170 |
| 删除行数 | 37 |
| 涉及文件 | 7 |
| 注意事项 | 补丁尾部缺少换行符，应用时需使用 `git apply --recount` |

---

*归档日期: 2026-08-18*

_Author: wangguanran_
