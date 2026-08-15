# 修改历史摘要

> 基线版本：IOT_High_Mid_2024.SPF.1.0 | ES 0.0.002.0 | LA.VENDOR.13.2.1.r2-05100-DIVAR.QSSI14.0-1 | LA.QSSI.14.0.r1-15100-qssi.0-1 | KERNEL.PLATFORM.2.0.r12-02200-kernel.0-2

## 修改列表

- **PN722x MODE pin Forum/EMVCo 切换支持**：添加 DT gpio31 mode_sw、nfc_mode sysfs 及 NFCC_PROFILE_SWITCH ioctl
- **NFC 固件版本更新**：固件版本非 3.2.5，更新 NFC firmware 至 3.2.5
- **Monkey 测试问题修复**：修复 monkey test 相关问题
- **PN7221 配置更新**：更新 libnfc-nxp-rfExt.conf
- **PN7221 高功耗修复**：启用 lpcd 降低待机功耗
- **NfcTdaTestApp 预装模式修改**：调整 NfcTdaTestApp 的 preinstall 模式
- **NFC 开关概率不显示修复**：修复 NFC 开关概率不显示的问题
- **PN7221 驱动移植**：vendor 层修改并更新 firmware
- **PN560 NFC 驱动 bring up**：添加 PN560 NFC 驱动
- **新增设备树配置**：添加平台 DTS 配置
- **新增 DTSI/XBL 配置**：添加多平台 DTSI 与 XBL 配置
