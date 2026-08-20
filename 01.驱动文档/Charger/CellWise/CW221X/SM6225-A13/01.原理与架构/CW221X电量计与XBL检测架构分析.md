# CellWise CW221X 电量计工作原理与 XBL 检测架构分析

> **版本号：v1.0**

## 芯片概述

CW221X 是 CellWise 的低功耗电池电量计（Fuel Gauge）系列，包含多个型号（CW2215、CW2217 等）。不同型号的**寄存器地址与 ID 寄存器内容不同**，系统需正确识别型号后才能按对应寄存器配置读取电池信息（电压、电流、容量）。

## 检测链路（XBL 阶段）

```
设备上电
   ↓
XBL (eXtensible Boot Loader)
   └─ MDPPlatformLib 电量计检测函数
         ├─ 通过 I2C 读取芯片 ID 寄存器
         ├─ 与 ID 匹配表比对 → 确定 CW221X 型号
         └─ 按型号配置读取电池信息 → 开机流程
```

## 问题根因

`MDPPlatformLib` 中的类型判断逻辑不完善，仅覆盖部分型号，导致：

1. **读取了错误的芯片 ID 寄存器**（型号判断失败时 fallback 错误）
2. 电池信息读取异常，影响开机流程

## 修复方案

完善芯片 ID 匹配表，增加对不同 CW221X 型号（CW2215/CW2217 等）的准确识别，使 XBL 阶段能正确匹配实际使用的变体。

## 关键要点

- XBL 阶段**不依赖 DTS**：电量计信息通过 I2C 直接读取芯片 ID，配置独立于内核
- 修改位于 bootloader 侧（BOOT.XF / MDPPlatformLib），与内核驱动无耦合

## 引用文件索引

- [[01.驱动文档/Charger/CellWise/CW221X/SM6225-A13/04.问题案例/XBL阶段电量计类型检测修复.md|XBL阶段电量计类型检测修复]]（补丁内容）
- `BOOT.XF.*/boot_images/QcomPkg/.../MDPPlatformLib`（远程源码树，XBL 电量计检测逻辑）

---

_Author: wangguanran_
