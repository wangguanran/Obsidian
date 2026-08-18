# 提高显示 PWM 频率

> **模块**: LCD | **厂商**: Qualcomm | **芯片**: SM6115 (scuba)
> **平台**: SM6115-A14 (LA.VENDOR.13.2.1) | **类型**: 需求
> **Change**: #196443 | **作者**: lixianghui | **状态**: MERGED

---

## 基本信息

| Change | 项目 | 分支 | 作者 | 类型 | 芯片 | 平台 | 模块 |
|--------|------|------|------|------|------|------|------|
| #196443 | LA.VENDOR.13.2.1 | MT5205 | lixianghui | 需求 | SM6115 (scuba) | SM6115-A14 | LCD |

## 需求描述

Task 120699：提升显示背光 PWM 频率。当前 LCD 背光 PWM 周期配置为 100us（约 10kHz），存在可闻噪声/频闪风险，需求将 PWM 频率提升至 20kHz（周期 50us）。

## 方案

在 `scuba-sde-display-idp.dtsi` 中，两处 panel（st7701s 与 jd9365da 均使用 `qcom,bl-pmic-pwm-period-usecs` 属性）将周期由 100 改为 50：

- `qcom,bl-pmic-pwm-period-usecs = <100>` → `<50>`

PWM 周期 100us→50us，对应频率 10kHz→20kHz（PWM 输出由 PM2250 PWM3（pm2250_pwm3）产生，`bl_ctrl_pwm` 背光控制模式）。

## 修改文件清单

| # | 文件 | 改动 | 说明 |
|---|------|------|------|
| 1 | [[01.驱动文档/LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-sde-display-idp.dtsi\|scuba-sde-display-idp.dtsi]] | +2/-2 | 两处 `qcom,bl-pmic-pwm-period-usecs` 100→50 |

## 配置方式

```dts
/* scuba-sde-display-idp.dtsi — 两处 panel 节点 */
qcom,mdss-dsi-bl-pmic-control-type = "bl_ctrl_pwm";
pwms = <&pm2250_pwm3 0 0>;
qcom,bl-pmic-pwm-period-usecs = <50>;   /* 周期 50us → 20kHz（原 100us/10kHz） */
qcom,mdss-dsi-bl-min-level = <1>;
qcom,mdss-dsi-bl-max-level = <4095>;
```

## 验证方式

1. 编译验证：134 源码树父提交重建 ✅ 可干净应用。
2. 设备端验证：开机后背光亮度调节正常，无闪烁/可闻噪声；可用示波器在背光 PWM 输出端（PM2250 PWM3）实测周期约 50us（20kHz）。
3. 回归：st7701s 与 jd9365da 两块屏均验证亮度档位（min 1 ~ max 4095）线性无异常。

## 结论

通过调整 `qcom,bl-pmic-pwm-period-usecs` 将背光 PWM 频率从 10kHz 提升至 20kHz，消除低频可闻噪声风险。改动仅 2 行 DT 属性，风险低。

## 补丁内容

```diff
Subject: [PATCH]  [MT5205][TaskID]120699[Description]Increase the display PWM frequency[Owner]lixianghui

---

diff --git a/vendor/qcom/proprietary/display-devicetree/display/scuba-sde-display-idp.dtsi b/vendor/qcom/proprietary/display-devicetree/display/scuba-sde-display-idp.dtsi
index 627d415..124a383 100755
--- a/vendor/qcom/proprietary/display-devicetree/display/scuba-sde-display-idp.dtsi
+++ b/vendor/qcom/proprietary/display-devicetree/display/scuba-sde-display-idp.dtsi
@@ -112,7 +112,7 @@
 	qcom,panel-supply-entries = <&dsi_panel_pwr_supply_mt5205>;
 	qcom,mdss-dsi-bl-pmic-control-type = "bl_ctrl_pwm";
 	pwms = <&pm2250_pwm3 0 0>;
-	qcom,bl-pmic-pwm-period-usecs = <100>;
+	qcom,bl-pmic-pwm-period-usecs = <50>;
 	qcom,mdss-dsi-bl-min-level = <1>;
 	qcom,mdss-dsi-bl-max-level = <4095>;
 	qcom,platform-te-gpio = <&tlmm 81 0>;
@@ -136,7 +136,7 @@
 	qcom,panel-supply-entries = <&dsi_panel_pwr_supply_mt5205>;
 	qcom,mdss-dsi-bl-pmic-control-type = "bl_ctrl_pwm";
 	pwms = <&pm2250_pwm3 0 0>;
-	qcom,bl-pmic-pwm-period-usecs = <100>;
+	qcom,bl-pmic-pwm-period-usecs = <50>;
 	qcom,mdss-dsi-bl-min-level = <1>;
 	qcom,mdss-dsi-bl-max-level = <4095>;
 	qcom,platform-te-gpio = <&tlmm 81 0>;

```

## 补丁验证

✅ 可干净应用（134 源码树父提交重建验证）。

## 源码归档

| 归档目录 | 文件 | 说明 |
|----------|------|------|
| dt_config/ | scuba-sde-display-idp.dtsi | 最终版（含 #196443 PWM 改动与 #196371 JD9365DA 修正） |
| patches/ | 196443.patch | 本变更补丁 |

## 引用文件索引

| 文件 | 路径 | 说明 |
|------|------|------|
| [[01.驱动文档/LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-sde-display-idp.dtsi\|scuba-sde-display-idp.dtsi]] | `vendor/qcom/proprietary/display-devicetree/display/scuba-sde-display-idp.dtsi` | 最终版（PWM 周期 100→50，两处） |
| [[01.驱动文档/LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/196443.patch\|196443.patch]] | `patches/196443.patch` | 本变更补丁 |

---

_Author: wangguanran_
