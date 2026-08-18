# 分析：提高显示PWM频率

**版本号：v1.0**
**对应文档：** 提高显示PWM频率.md

## 技术背景

高通 SDE 显示框架的背光控制支持 `bl_ctrl_pwm` 模式：由 PMIC 的 PWM 通道产生 PWM 驱动背光 IC。`qcom,bl-pmic-pwm-period-usecs` 定义 PWM 周期（us），周期越小频率越高。人耳可听范围约 20Hz~20kHz，10kHz 的 PWM 与电感/陶瓷电容耦合后可能产生可闻噪声，因此显示类产品通常要求 PWM 频率 ≥ 20kHz（部分要求 25kHz+）。

MT5205 背光 PWM 通道为 PM2250 PWM3（`pm2250_pwm3`），面板为 st7701s 与 jd9365da 两块 720p 屏，均通过 `dsi_panel_pwr_supply_mt5205` 电源组与 `bl_ctrl_pwm` 控制。

## 代码改动分析

### vendor/qcom/proprietary/display-devicetree/display/scuba-sde-display-idp.dtsi（+2/-2）

- `&dsi_st7701s_boe397_video` 节点：`qcom,bl-pmic-pwm-period-usecs` 100 → 50；
- `&dsi_jd9365da_video` 节点：`qcom,bl-pmic-pwm-period-usecs` 100 → 50。

两处改动完全对称，均为周期减半（100us→50us，10kHz→20kHz），未涉及 min/max 亮度档位（1~4095）与 PWM duty 换算逻辑。

## 潜在风险

- 背光 IC 的 PWM 输入频率上限：若背光 IC 内部滤波器或使能逻辑对 20kHz 响应不佳，亮度曲线可能偏移；MT5205 实测无异常。
- duty 精度：周期减半后，同分辨率下步进精度降低（50us/4095 级 ≈ 12.2ns/级），对高精度调光有轻微影响，常规使用无感知。
- 仅修改 scuba 侧：bengal-sde-display-idp.dtsi（参考板）仍为 100us，若后续复用 Bengal 参考配置需同步评估。

## 回归测试建议

1. 亮度档位扫描（1/1024/2048/4095）确认无跳变、无闪烁；
2. 低亮度下听诊确认无可闻噪声（10kHz→20kHz 后应消失）；
3. st7701s 与 jd9365da 两块屏分别回归；
4. 功耗测试：20kHz 下 PWM 损耗无明显上升。

## 与现有驱动架构的关系

- 与 #196371（JD9365DA init data 修复）作用于同一文件，两变更叠加后为最终归档版本（scuba-sde-display-idp.dtsi）；
- 属性由 SDE/DSI panel 框架消费（qcom,panel-supply-entries + bl_ctrl_pwm），无需改驱动代码，纯 DT 配置类需求。

---

_Author: wangguanran_
