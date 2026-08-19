# 分析：PM8008 LDO 输出电压异常修复

**版本号：v1.0**
**对应文档：** [[01.驱动文档/LCD/Qualcomm/SLM927M/SM6225-A16/04.问题案例/PM8008-LDO输出电压异常修复.md|PM8008-LDO输出电压异常修复]]

## 技术背景

- **PM8008**：Qualcomm 配套 PMIC（I2C SID 0x8/0x9），为相机/显示外设提供 LDO 电源；LDO5~LDO7 为可编程输出。
- **16-bit VSET**：PM8008 LDO 电压由 `VSET_LB`（低字节，地址偏移 +0x00）与 `VSET_UB`（高字节，+0x01）组成，**写 VSET_UB 时锁存完整 16 位电压值**。
- **UEFI MDPPlatformLib**：KamortaPkg 中负责显示面板初始化的库，`pm_i2c_sid_config` 按数组顺序下发 I2C 写命令；面板电源（LDO5/6/7）在 UEFI 阶段使能，早于 Linux。

## 代码改动分析

改动文件：`MDPPlatformLibPanelCommon.c`，仅调整 6 个初始化数组的元素顺序 + 1 条注释：

| LDO | 目标电压 | 修复前顺序 | 修复后顺序 |
|-----|---------|-----------|-----------|
| LDO5 | 2.8V | UB(0x41)=0x0B → LB(0x40)=0x28 | LB=0x28 → UB=0x0B |
| LDO6 | 3.0V | UB=0x0A → LB=0x90 | LB=0x90 → UB=0x0A |
| LDO7 | 1.8V | UB=0x07 → LB=0x08 | LB=0x08 → UB=0x07 |

电压值本身未变（UB 高字节 × 256 + LB 低字节 = 目标电压），仅保证 UB 写触发锁存时 LB 已是目标值。

## 潜在风险

1. **影响面小但直接**：仅 UEFI 面板电源序列；若某面板依赖"先 UB 后 LB"的中间态电压做软启动，改序可能改变上电波形（正常场景下无影响）；
2. **其它 LDO/项目复制**：同文件其它 LDO（L1~L4）若也是 16-bit VSET 且顺序有误，应一并核查（本次提交未改动，可能其写入顺序本就正确）；
3. **与 Linux 侧重复配置**：UEFI 设好后 Linux PMIC 驱动会再次配置，若两侧电压定义不一致会覆盖——本提交未涉及 Linux 侧。

## 回归测试建议

- UEFI 串口日志确认 `pm_i2c_sid_config` 序列无 I2C NACK；
- 万用表实测 LDO5/6/7 输出 2.8V/3.0V/1.8V（±2%）；
- 面板上电/显示冒烟：logo 正常、无闪烁；
- 冷启动/重启各 10 次确认电源序列稳定。

## 与现有驱动架构的关系

UEFI MDPPlatformLib 与 Linux 显示驱动（SDE）是独立的两个电源初始化阶段；该改动只影响 UEFI 阶段。同类锁存型 16-bit VSET 的 PMIC（如 SMB 系列）初始化代码都应遵循 LB→UB 顺序，可作为平台级经验沉淀到 FAQ。

_Author: wangguanran_
