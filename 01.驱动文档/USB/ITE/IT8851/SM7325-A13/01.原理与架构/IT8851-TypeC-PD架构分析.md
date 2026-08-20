# ITE IT8851 Type-C/PD 控制器架构分析

> **版本号：v1.0**

## 芯片概述

IT8851 是 ITE 的 Type-C/PD 控制器，负责：

1. **插入方向检测**：CC 引脚检测 USB 插入方向
2. **PD 协议协商**：与对端协商供电/数据角色
3. **电源控制**：控制 VBUS 和 VCONN 供电
4. **屏供电**（CFD 屏应用）：承担屏供电控制功能

## 系统接入

```
USB Type-C 接口
   ↓ CC 检测 / PD 协商
IT8851 (Type-C/PD 控制器)
   ├─ VBUS/VCONN 电源控制
   ├─ 屏供电控制（CFD 屏）
   └─ I2C 接口 → SoC (SM7325)
         ↓
it8851.c 驱动 (kernel/msm-5.4/drivers/meig-tools/)
```

## suspend/resume 问题（CFD 屏休眠唤醒不亮）

- **现象**：设备休眠唤醒后，CFD 屏无法点亮
- **根因**：IT8851 在 suspend/resume 流程中，驱动初始化或复位时序不完整，寄存器状态未正确恢复，导致屏供电未打开或通信链路未恢复
- **修复**：在 `it8851.c` 的关键函数中增加 12 行复位/初始化逻辑，resume 阶段重新初始化芯片（+12/−0）

## 关键要点

- 该修复**完全在驱动代码层面**：无 DTS 变更、无 Kernel config 变更、无 BoardConfig 变更
- 寄存器状态恢复是 Type-C/PD 控制器 suspend/resume 的通用关注点

## 引用文件索引

- [[01.驱动文档/USB/ITE/IT8851/SM7325-A13/04.问题案例/CFD屏休眠唤醒不亮.md|CFD屏休眠唤醒不亮]]（补丁内容）
- `kernel/msm-5.4/drivers/meig-tools/it8851.c`（远程）

---

_Author: wangguanran_
