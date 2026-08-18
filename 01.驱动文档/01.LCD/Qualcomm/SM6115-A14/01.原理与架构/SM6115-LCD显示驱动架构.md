# SM6115 LCD 显示驱动架构

> **模块**: 01.LCD | **芯片**: SM6115 (scuba) | **平台**: SM6115-A14

## 概述

SM6115（scuba）显示链路分为 AP 侧（SDE + DSI panel 框架）与 BP 侧（UEFI XBL 的 MDPPlatformLib）两套独立但同源的初始化路径：

```
AP 侧（Android/Kernel）
  SDE (MSM DRM disp/msm)
    ├── sde_dsi / dsi_host（DSI 控制器）
    ├── panel 驱动（由 dsi-panel-*.dtsi 描述）
    │     ├── qcom,mdss-dsi-on-command（init data，长写/短写）
    │     ├── power rails：dsi_panel_pwr_supply_mt5205（vddio/avdd/avee）
    │     └── 背光：bl_ctrl_pwm（PM2250 PWM3，周期 50us）
    └── DT 装配：scuba-sde-display-idp.dtsi / bengal-sde-display-idp.dtsi

BP 侧（UEFI XBL）
  MDPPlatformLib（AgattiPkg）
    ├── MDPPlatformLib.c：PANEL_CREATE_ENTRY 面板注册表（ODM_PROJECT_MT5205 条件编译）
    ├── MDPPlatformLibPanelCommon.c：PWM 背光初始化（PMIC GPIO mux）
    └── Panel_*.xml（QcomPkg/Settings/Panel/）：UEFI init data，经 Core.fdf FREEFORM 段打包
```

## 分层结构（AP 侧）

1. **DT 层**：`scuba-sde-display-idp.dtsi`（MT5205 装配，电源组/面板节点/背光 PWM）、`dsi-panel-jd9365da-video.dtsi`（面板时序 + on-command）、`bengal-sde-display-idp.dtsi`（参考板，保持 td4330）；
2. **电源层**：regulator-fixed（display_panel_vddio/avdd/avee）+ `dsi_panel_pwr_supply_mt5205` 供应组（supply-name vddio/avdd/avee，post-on-sleep 10ms）；
3. **panel 驱动层**：SDE panel 驱动解析 dtsi 中 `qcom,mdss-dsi-on-command`，通过 DSI host 发送 init 序列；读 TE（GPIO81）与复位脚（GPIO82）；
4. **背光层**：`qcom,mdss-dsi-bl-pmic-control-type = "bl_ctrl_pwm"` + `pwms = <&pm2250_pwm3 0 0>` + `qcom,bl-pmic-pwm-period-usecs`（周期，50us=20kHz），亮度 1~4095。

## Commanded panel init 流程（JD9365DA 720p video 为例）

1. UEFI（BP）阶段：XBL MDPPlatformLib 从 Panel XML 读取 on-command（0x29/0x15 长/短写混合），发送 DSI init；Core.fdf 将该 XML 作为 FREEFORM 段打包进 XBL 镜像；显示 splash 后交棒给 AP。
2. Kernel（AP）阶段：SDE panel probe → regulator 上电（vddio→avdd→avee，各 10ms startup-delay）→ 复位脚 GPIO82 拉高释放 → 发送 on-command → 背光 PWM 使能（PM4125 GPIO2 mux 由 UEFI 完成，AP 侧直接使用）→ 点亮。

**init data 要点（本次修复的核心）**：

- 页切换：`E0 00`（NVM 加载）→ `E0 01`（页 1 寄存器）→ `E0 02`（页 2 GIP）→ `E0 04`（页 4）→ `E0 00`；
- 命令类型：generic long write（29）用于带参寄存器设置；短写（15）用于单命令；05 仅限无参命令。用错类型会导致 IC 解析错位 → 花屏；
- 关键 GIP 参数：页 2 的 0x28/0x29（如 5F/41）决定 gamma 与面板特性；
- 结束序列：`05 11 00`（sleep out，等待 120ms）→ `05 29 00`（display on，5ms）→ `05 35 00`（TE on）。

## 关键函数（BP 侧 MDPPlatformLib）

| 函数/宏 | 作用 |
|---------|------|
| PANEL_CREATE_ENTRY(name, id, dt_compat, ...) | 面板注册表；MT5205 的 st7701s/jd9365da 条目用 `#if defined(ODM_PROJECT_MT5205)` 保护 |
| MDPPlatformGetPanelPwmPmicInfo（PanelCommon） | 获取背光 PWM PMIC 信息；MT5205 分支将 PM4125 GPIO2 mux 为 PWM SPECIAL_FUNCTION1 |

## DT 装配差异：bengal vs scuba

- `bengal-sde-display-idp.dtsi`：高通参考板配置，默认 td4330，MT5205 项目不再包含 MT5205 面板/pinctrl（#196371 回退）；
- `scuba-sde-display-idp.dtsi`：MT5205 实际装配（st7701s + jd9365da），电源组与背光 PWM 定义于此；
- `bengal-sde-display-pinctrl.dtsi` 已删除（TE pinctrl 由 scuba 侧管理）。

## 参考

- [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/04.问题案例/JD9365DA显示初始化数据修复.md|JD9365DA显示初始化数据修复]]
- [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/03.需求实现/提高显示PWM频率.md|提高显示PWM频率]]
- 源码：[[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/dsi-panel-jd9365da-video.dtsi|dsi-panel-jd9365da-video.dtsi]] | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/AgattiPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c|MDPPlatformLibPanelCommon.c]]

---

_Author: wangguanran_