# 分析：IT8851 Type-C CFD屏休眠唤醒不亮

**版本号：v1.0**
**对应文档：** IT8851 Type-C CFD屏休眠唤醒不亮

## 技术背景

IT8851 是 ITE Tech 推出的一款 USB Type-C/PD（Power Delivery）控制器，支持 DRP（Dual Role Port）、Source/Sink 模式切换，以及 VCONN 控制。在嵌入式系统中，IT8851 常用于：

- USB Type-C 插入方向检测（CC 逻辑）
- PD 协议协商（电压/电流能力协商）
- VBUS 和 VCONN 供电控制
- 外设电源管理

在 CFD 屏的应用场景中，IT8851 通过其 GPIO 或 I2C 接口控制屏的供电链路。当系统进入 suspend 状态时，SoC 会切断部分外设供电或时钟，IT8851 的寄存器状态可能丢失或进入低功耗模式。若 resume 时驱动未能正确恢复寄存器状态，屏供电链路无法正常建立，导致屏不亮。

## 代码改动分析

- **文件**：`kernel/msm-5.4/drivers/meig-tools/it8851.c`
- **改动量**：+12 行，-0 行
- **改动内容**：在驱动中添加了 resume 后的复位/初始化逻辑，推测是在 `it8851_resume()` 或类似回调函数中增加寄存器恢复操作。
- **关键函数**：驱动中已有的 `it8851_init()` 或 `it8851_reset()` 函数，在 resume 时重新调用。

## 潜在风险

- 增加 resume 初始化可能会延长 resume 时间（~ms 级别），对低功耗场景有轻微影响
- 若复位时序不当，可能与其他共享同一电源域的外设产生交互影响
- 需要在不同批次的硬件上验证修复的稳定性

## 回归测试建议

- 连续 100 次以上 suspend/resume 循环测试
- 在高温/低温环境下测试 suspend/resume 稳定性
- 同时测试 USB 功能是否正常（IT8851 同时负责 USB Type-C/PD）
- 验证其他外设（如触摸屏、LCD）在 suspend/resume 后是否正常

## 与现有驱动架构的关系

it8851 驱动位于 `meig-tools` 目录下，这是一个自研驱动模块集合，属于内核驱动层。该驱动通过 I2C 总线与 SoC 通信，挂载在 I2C 控制器下。驱动遵循 Linux 内核驱动模型，实现了 probe、suspend、resume 等标准回调函数。

_Author: 艾达_