# 分析：禁用 r66451 AMOLED 面板 ESD 检查

**版本号：v1.0**
**对应文档：** 禁用r66451-AMOLED面板ESD检查.md

## 技术背景

Qualcomm 显示栈中，**SDE（Snapdragon Display Engine）** 驱动（msm/drm/sde）负责 DPU 与 DSI 显示链路。SDE 的面板框架支持 **ESD 检测（Panel ESD check）** 机制：驱动周期性下发 DSI 命令读取面板状态寄存器（`panel-status-command`，如读 0x0A 寄存器），将读回值与 `panel-status-value` 比对，不一致则判定面板异常（静电/掉电），触发 **panel recovery**——重置 DSI 链路并重新发送面板初始化序列恢复显示。

面板节点在 DTS 中通过以下属性组合控制 ESD 检测：

| 属性 | 作用 |
|------|------|
| `qcom,esd-check-enabled` | 使能该面板的 ESD 周期检查（存在即使能） |
| `qcom,mdss-dsi-panel-status-check-mode` | 检测方式：`reg_read`（读寄存器）等 |
| `qcom,mdss-dsi-panel-status-command` | 读取命令序列（`06 01 00 01 00 00 01 0a` = DCS 读 0x0A） |
| `qcom,mdss-dsi-panel-status-value` | 期望的寄存器值（`0x1c`） |
| `qcom,mdss-dsi-panel-status-read-length` | 读取字节长度（1） |

r66451 是三星/Synaptics 系的 AMOLED DSI 面板（常见于 parrot/QCM4490 参考设计），支持 video 与 cmd 模式；本案例为 **r66451 AMOLED video panel**（`dsi_r66451_amoled_video` 节点）。

## 代码改动分析

### parrot-sde-display-common.dtsi（+1/−1）

`&dsi_r66451_amoled_video` 节点第 380 行：

```dts
-	qcom,esd-check-enabled;
+	/* qcom,esd-check-enabled; */
```

- **改动本质**：将属性注释，驱动解析 DTS 时不再看到该属性，ESD 检查定时器不注册，不再周期性读 status 寄存器 → 误检路径整体消除；
- **保留项**：`status-check-mode / status-command / status-value / status-read-length` 全部原样保留。这些属性在 `esd-check-enabled` 缺失时不会被使用（SDE 驱动的 esd 配置入口以 `qcom,esd-check-enabled` 存在为前提），保留是为了**后续复开零成本**（取消注释即可，无需再核对寄存器值）；
- 改动点位于该文件的公共显示配置区，仅影响 `dsi_r66451_amoled_video` 这一个面板节点，其他面板（cmd 模式 r66451、144hz 等）不受影响。

## 潜在风险

1. **失去真实 ESD 保护**：禁用后若面板发生真实 ESD 事件，驱动无法自动 recovery——显示可能停在异常状态，需人为重启。需结合产品使用环境评估（消费/工业场景差异大）；
2. **video 面板无 TE 信号辅助**：video 模式面板本身命令通道利用率低，若后续固件升级改变寄存器语义，status-value 可能再次失配——复开 ESD 前需重新实测 status 值；
3. **方案是绕开而非根治**：未从面板固件/时序上解决"读回值不符"的根因，若后续屏幕供应商修复固件，可考虑恢复 ESD 检查。

## 回归测试建议

1. **长稳**：72h 亮屏/灭屏循环、休眠唤醒循环、热插拔电源场景，确认无闪屏、无恢复异常；
2. **显示功能**：开机、视频播放、图像滚动、色彩渐变各场景正常；
3. **对比回归**：有条件时在恢复 ESD 检查（取消注释）的版本上复测，确认复开路径可用（status 属性保留的价值验证）；
4. **日志回归**：确认无 `esd` 误报日志、无 `panel recovery` 周期触发日志。

## 与现有驱动架构的关系

- 该改动位于 **display-devicetree**（vendor 树），属于 DTS 配置层修复，未涉及 SDE 内核驱动代码；
- 与同平台既有 LCD 改动（如 JD9365DA init data 修复、PWM 频率调整）同属 parrot 显示链路 bringup/调试范畴，问题类型不同（本案例为面板检测机制与面板匹配问题）；
- 该文件 `parrot-sde-display-common.dtsi` 为 parrot 平台显示公共配置，被 parrot 系列产品共用，后续其他 parrot 项目若使用 r66451 video 面板可参考此处理方式。

_Author: wangguanran_