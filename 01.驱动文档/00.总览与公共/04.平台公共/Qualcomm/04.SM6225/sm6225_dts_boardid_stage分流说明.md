# SM6225 DTS BoardID/Stage 分流说明

适用范围：
- Qualcomm SM6225（Khaje）平台
- 同一软件基线支持多个硬件小版本（如 v101/v102）
- 目标是通过 DTS/DTBO 分流，而不是在驱动 probe 中做版本分支

关键词：
- board-id
- stage
- dtbo
- overlay
- LocateDeviceTree
- DTB_MAJOR_MINOR
- best-fit

面向读者：初学者 / 平台维护 / 驱动调试

## 1. 问题或场景描述

这类问题通常表现为：
1. 同一功能在不同硬件版本表现不一致。
2. 某版本功能节点生效，另一版本不生效。
3. 一处 DTS 修改影响多个硬件版本。

典型触发条件：
1. 多硬件版本共用同一 base dtsi。
2. 硬件差异节点未隔离到独立 overlay。
3. 分流逻辑分散在 DTS 和驱动中，造成双重判定。

不适用场景：
1. 只有单一硬件版本。
2. 差异仅在用户空间策略，不涉及 DTS 资源绑定。

## 2. 背景知识

模块职责：
1. bootloader 决定最终加载哪个 DTB/DTBO。
2. DTS/overlay 决定具体硬件资源绑定（gpio/pinctrl/regulator/功能节点）。
3. 驱动应消费“已经分流后”的标准资源，不应承担版本路由。

上下游关系：
1. stage 来源：板级识别（`Identify_Stage_ProjectId()`）。
2. 匹配键：`board-id` 的 major/minor（在 bootloader 中参与比较）。
3. 输出结果：命中某个 overlay，系统按该配置启动。

关键输入：
1. `BoardTargetId()`
2. `Identify_Stage_ProjectId()`
3. DTS `qcom,board-id`

关键输出：
1. 最终匹配的 dt entry / overlay
2. 生效的硬件资源描述

## 3. 方案概览

修改目标：
1. 把“版本分流”前置到 bootloader+DT 匹配层。
2. 把“版本差异配置”收敛到独立 overlay 文件。
3. 保持驱动代码与版本分流解耦。

修改思路：
1. 在 `LocateDeviceTree.c` 增加 `GetBoardTargetIdForDtbMatch()`，将 stage 写入 board-id minor 位域（白名单控制）。
2. DT 匹配阶段统一使用 `BoardIdForMatch`。
3. Makefile 改为构建版本化 overlay（v101/v102）。
4. base dtsi 删除版本差异节点，overlay 承载完整差异配置。

预期收益：
1. 分流点单一、可审计。
2. 新版本扩展成本低。
3. 驱动代码更稳定，回归面更小。

## 4. 修改方案详解

改动文件（核心）：
1. `kernel_platform/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/LocateDeviceTree.c`
2. `kernel_platform/qcom/proprietary/devicetree/qcom/Makefile`
3. `kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-pm7250b.dtsi`
4. `kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v101-overlay.dts`
5. `kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v102-overlay.dts`

改动点：
1. bootloader 注入 stage 到 board-id minor：
   - `GetBoardTargetIdForDtbMatch()`（`LocateDeviceTree.c`）
2. 匹配链路改用 `BoardIdForMatch`：
   - `GetBoardMatchDtb()`
   - `platform_dt_absolute_match()`
   - `update_dtb_entry_node()`
3. DTS 构建入口从“通用 overlay”切换到“版本 overlay”：
   - Makefile 中 `khaje-idp-v101-overlay.dtbo` / `khaje-idp-v102-overlay.dtbo`
4. 差异节点迁移到版本 overlay，base dtsi 保留公共层。

为什么这样改：
1. 版本判定只在最早匹配点做一次。
2. DTS 结构清晰，降低跨版本串扰。
3. 驱动不再写硬件版本 if-else，减少维护复杂度。

风险点：
1. stage 值与 overlay minor 规划不一致会触发 best-fit 回落。
2. 白名单 stage 未覆盖新硬件版本时，会回退到原始 board-id 行为。

## 5. 修改 demo

### demo-1：bootloader 分流入口（伪代码）

```c
BoardIdForMatch = BoardTargetId();
if (BoardPlatformType() == KHAJE) {
    stage = Identify_Stage_ProjectId();
    if (stage in {1,3}) {
        BoardIdForMatch &= ~VARIANT_MINOR_MASK;
        BoardIdForMatch |= (stage << 8) & VARIANT_MINOR_MASK;
    }
}
```

### demo-2：Makefile 版本化 overlay 入口

```make
KHAJE_BOARDS += \
    khaje-idp-v101-overlay.dtbo \
    khaje-idp-v102-overlay.dtbo
```

### demo-3：overlay 差异承载模板

```dts
/ {
    qcom,board-id = <...>;
};

&soc {
    /* 仅本版本差异节点 */
};
```

## 6. 详细代码流程分析

入口函数：
1. `GetBoardTargetIdForDtbMatch`（`LocateDeviceTree.c`）
2. `platform_dt_absolute_match`
3. `GetBoardMatchDtb`
4. `update_dtb_entry_node`
5. `platform_dt_match_best`

关键调用链：
1. `platform_dt_match_best` 依次执行：
   - foundry compat
   - ddr compat
   - pmic model compat
   - `update_dtb_entry_node(DTB_SOC)`
   - `update_dtb_entry_node(DTB_MAJOR_MINOR)`（分流关键）
   - `update_dtb_entry_node(DTB_PMIC0..3)`
2. `DTB_MAJOR_MINOR` 维度中，版本比较使用 `BoardIdForMatch`。

配置入口：
1. DTS 构建入口：`qcom/Makefile`
2. 版本差异承载：`khaje-idp-v101-overlay.dts` / `khaje-idp-v102-overlay.dts`
3. 公共层：`khaje-idp-pm7250b.dtsi`

关键规则：
1. `platform_dt_absolute_match` 是候选过滤：`entry <= board`。
2. `update_dtb_entry_node(DTB_MAJOR_MINOR)` 是 best-fit 选择：
   - 优先 exact
   - 否则选择 `< board` 的最大值

正常路径：
1. stage 注入成功
2. 命中对应版本 overlay
3. 仅该版本差异节点生效

异常路径：
1. stage 不在白名单 -> 不注入 minor -> 使用原始 board-id
2. 目标 minor 无 exact overlay -> best-fit 回落到较小 minor
3. base dtsi 残留差异节点 -> 覆盖/冲突风险上升

## 7. 调试与验证方法

关键观测点：
1. `GetBoardTargetIdForDtbMatch` 打印的 stage 和 board-id（改写前后）
2. `platform_dt_absolute_match` 候选进入情况
3. `update_dtb_entry_node(DTB_MAJOR_MINOR)` 的 best_info 收敛过程

建议验证动作：
1. 用不同 stage 触发启动，确认命中 overlay 是否符合预期。
2. 检查最终 dtbo 列表与 board-id 对应关系。
3. 在目标节点上验证资源绑定（pinctrl/regulator/gpio）是否来自期望 overlay。

如何判断“根因已解决”：
1. 不是“功能偶现恢复”，而是“stage->board-id->overlay 命中链路稳定可复现”。
2. 跨版本改动不再互相影响。

## 8. 常见问题 / 变体 / 易错点

1. overlay board-id 设计与 stage 映射不一致：
   - 可能会被 best-fit 回落“看起来正常”，但不是精确命中。
2. 只改 DTS 不改 bootloader 匹配：
   - 新 overlay 可能永远不会被选中。
3. 差异节点留在 base dtsi：
   - 分流后仍有隐式覆盖，造成“改了没生效/错版本生效”。

## 9. 参考案例

案例来源：
- 某次 SM6225 多版本分流改造（已抽象，不绑定单一问题单正文）

可复用结论：
1. 分流入口应固定在 DT 匹配层。
2. 版本差异应固定在 overlay 层。
3. 驱动应只消费分流后的资源，不再二次分流。

---

## 附录：DTS-only patch（去除 nfc 修改）

```diff
From 82337bf7a36917fdaf482d7be1630683e9a480d7 Mon Sep 17 00:00:00 2001
From: wangguanran <wangguanran@example.com>
Date: Tue, 3 Mar 2026 13:10:11 +0000
Subject: [PATCH] [MT5825][TaskID]87133[Description]buzzer and NFC share se5v
 for v1.01/v1.02[Solution]stage hw102 overlay + regulator refcount power
 control[Owner]wangguanran

Change-Id: I25859e9b4ba8c03eb7a3c06af2c3720c7540ed15
---
 .../qcom/proprietary/devicetree/qcom/Makefile |  4 +-
 .../devicetree/qcom/khaje-idp-pm7250b.dtsi    | 34 +---------
 .../qcom/khaje-idp-v101-overlay.dts           | 63 +++++++++++++++++++
 .../qcom/khaje-idp-v102-overlay.dts           | 50 +++++++++++++++
 4 files changed, 117 insertions(+), 34 deletions(-)
 create mode 100644 kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v101-overlay.dts
 create mode 100644 kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v102-overlay.dts

diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/Makefile b/kernel_platform/qcom/proprietary/devicetree/qcom/Makefile
index 84f4cdffc55..17ecd662369 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/Makefile
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/Makefile
@@ -117,13 +117,13 @@ dtb-y += $(kalama-dtb-y)
 KHAJE_BASE_DTB += khaje.dtb khajep.dtb khajeq.dtb khajeg.dtb khaje-3gb.dtb
 
 KHAJE_BOARDS += \
-                 khaje-idp-overlay.dtbo \
+                 khaje-idp-v101-overlay.dtbo \
                  khaje-qrd-overlay.dtbo \
 		 khaje-qrd-hvdcp3p5-overlay.dtbo \
 		 khaje-qrd-nowcd9375-overlay.dtbo \
                  khaje-idp-nopmi-overlay.dtbo \
                  khaje-qrd-nopmi-overlay.dtbo \
-                 khaje-idps-display-90hz-overlay.dtbo \
+                 khaje-idp-v102-overlay.dtbo \
                  khaje-atp-overlay.dtbo \
                  khaje-idp-usbc-overlay.dtbo \
                  khaje-idp-pm8010-overlay.dtbo \
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-pm7250b.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-pm7250b.dtsi
index 59988e2278f..03ec319c227 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-pm7250b.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-pm7250b.dtsi
@@ -9,21 +9,6 @@
 		#include "qg-batterydata-alium-3600mah.dtsi"
 	};
 
-/* Modify:wangguanran on: Tue, 06 Jan 2026 08:55:57 +0000
- * GPIO simulated PWM causes crackling noise in buzzer, changed to hardware PWM control
-        gpio-pwm{
-                status = "ok";
-                compatible = "gpio-pwm";
-                pinctrl-names = "default";
-                pinctrl-0 = <&pm7250b_gpio8_buzzer_gpio_default>;
-                meig,gpio-pwm = <&pm7250b_gpios 8 GPIO_ACTIVE_HIGH>;
-        };
- */
-    pwm_beeper: pwm-beeper {
-        compatible = "pwm-beeper";
-        pwms = <&pm6125_pwm 0 1000000>;
-    };
-// End of Modify: wangguanran
         usb-speed-show {
             status = "ok";
             compatible = "usb-speed-show";
@@ -35,7 +20,7 @@
 		compatible = "gpio-userspace";
 /* Modify:wangguanran on: Thu, 29 Jan 2026 10:28:14 +0000 */
 		pinctrl-names = "default";
-		pinctrl-0 = <&pm6125_gpio6_redrive_vcc_enable &pm7250b_gpio8_se_5v_enable &pm7250b_gpio12_buzzer_detect>;
+		pinctrl-0 = <&pm6125_gpio6_redrive_vcc_enable &pm7250b_gpio12_buzzer_detect>;
 // End of Modify: wangguanran
 
 		scr-led {
@@ -54,12 +39,6 @@
 			gpios = <&pm6125_gpios 6 GPIO_ACTIVE_HIGH>;
 			default-state = <1>; // 0-low, 1-high
 		};
-		se-5v-enable {
-			label = "se-5v-enable";
-			gpios = <&pm7250b_gpios 8 GPIO_ACTIVE_HIGH>;
-			default-state = <0>; // 0-low, 1-high
-		};
-
  		/* add for buzzer detect input gpio */
 		buzzer-detect {
 			label = "buzzer-detect";
@@ -95,16 +74,7 @@
 		power-source = <1>;
 	 };
 
-    pm7250b_gpio8_se_5v_enable: pm7250b_gpio8_se_5v_enable {
-        pins = "gpio8";
-        function = "normal";
-        output-enable;
-        output-low;
-        bias-disable;
-        power-source = <1>;
-	};
-
-    pm7250b_gpio12_buzzer_detect: pm7250b_gpio12_buzzer_detect {
+	pm7250b_gpio12_buzzer_detect: pm7250b_gpio12_buzzer_detect {
         pins = "gpio12";
         function = "normal";
         input-enable;
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v101-overlay.dts b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v101-overlay.dts
new file mode 100644
index 00000000000..2b1205483c0
--- /dev/null
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v101-overlay.dts
@@ -0,0 +1,63 @@
+/dts-v1/;
+/plugin/;
+
+#include <dt-bindings/interrupt-controller/arm-gic.h>
+#include "khaje-idp.dtsi"
+#include "khaje-idp-pm7250b.dtsi"
+
+/ {
+	model = "Qualcomm Technologies, Inc. Khaje IDP";
+	compatible = "qcom,khaje-idp", "qcom,khaje", "qcom,idp";
+	qcom,msm-id = <518 0x10000>, <586 0x10000>;
+	/* board-id minor=0x00, mapped from androidboot.boardid.stage=0 (v101) */
+	qcom,board-id = <0x10022 0>;
+	qcom,pmic-id = <0x2D 0x2E 0x0 0x0>;
+};
+
+&soc {
+	se5v: se5v {
+		compatible = "regulator-fixed";
+		regulator-name = "se5v";
+		regulator-min-microvolt = <5000000>;
+		regulator-max-microvolt = <5000000>;
+		enable-active-high;
+		gpio = <&pm6125_gpios 8 GPIO_ACTIVE_HIGH>;
+		pinctrl-names = "default";
+		pinctrl-0 = <&pm6125_gpio8_se_5v_enable>;
+	};
+
+	gpio_pwm: gpio-pwm {
+		compatible = "gpio-pwm";
+		pinctrl-names = "default";
+		pinctrl-0 = <&pm7250b_gpio8_gpio_pwm_default>;
+		meig,gpio-pwm = <&pm7250b_gpios 8 GPIO_ACTIVE_HIGH>;
+		amp-supply = <&se5v>;
+	};
+};
+
+&pm6125_gpios {
+	pm6125_gpio8_se_5v_enable: pm6125_gpio8_se_5v_enable {
+		pins = "gpio8";
+		function = "normal";
+		input-disable;
+		output-enable;
+		output-low;
+		bias-disable;
+		power-source = <1>;
+	};
+};
+
+&pm7250b_gpios {
+	pm7250b_gpio8_gpio_pwm_default: pm7250b_gpio8_gpio_pwm_default {
+		pins = "gpio8";
+		function = "normal";
+		output-enable;
+		output-low;
+		bias-disable;
+		power-source = <1>;
+	};
+};
+
+&nfc {
+	vdd-supply = <&se5v>;
+};
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v102-overlay.dts b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v102-overlay.dts
new file mode 100644
index 00000000000..574cb210cb5
--- /dev/null
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-v102-overlay.dts
@@ -0,0 +1,50 @@
+/dts-v1/;
+/plugin/;
+
+#include <dt-bindings/interrupt-controller/arm-gic.h>
+#include "khaje-idp.dtsi"
+#include "khaje-idp-pm7250b.dtsi"
+#include "khaje-idps-display-90hz.dtsi"
+
+/ {
+	model = "Qualcomm Technologies, Inc. KHAJE IDPS + 90Hz";
+	compatible = "qcom,khaje-idp", "qcom,khaje", "qcom,idp";
+	qcom,msm-id = <518 0x10000>, <586 0x10000>;
+	/* board-id minor=0x03, mapped from androidboot.boardid.stage=3 (v102) */
+	qcom,board-id = <0x10322 0>;
+	qcom,pmic-id = <0x2D 0x2E 0x0 0x0>;
+};
+
+&soc {
+	se5v: se5v {
+		compatible = "regulator-fixed";
+		regulator-name = "se5v";
+		regulator-min-microvolt = <5000000>;
+		regulator-max-microvolt = <5000000>;
+		enable-active-high;
+		gpio = <&pm7250b_gpios 8 GPIO_ACTIVE_HIGH>;
+		pinctrl-names = "default";
+		pinctrl-0 = <&pm7250b_gpio8_se_5v_enable>;
+	};
+
+	pwm_beeper: pwm-beeper {
+		compatible = "pwm-beeper";
+		pwms = <&pm6125_pwm 0 1000000>;
+		amp-supply = <&se5v>;
+	};
+};
+
+&nfc {
+	vdd-supply = <&se5v>;
+};
+
+&pm7250b_gpios {
+	pm7250b_gpio8_se_5v_enable: pm7250b_gpio8_se_5v_enable {
+		pins = "gpio8";
+		function = "normal";
+		output-enable;
+		output-low;
+		bias-disable;
+		power-source = <1>;
+	};
+};
-- 
2.34.1
```


## 文档修订（2026-03-05）

对应提交更新：
1. Gerrit：`http://[内网Gerrit]/c/LA.VENDOR.13.2.1/+/168795`
2. Commit：`82337bf7a36917fdaf482d7be1630683e9a480d7`

相对 `e13b6e...` 的补充变更：
1. `kernel_platform/msm-kernel/drivers/input/misc/pwm-beeper.c` 回退到提交前状态（仅移除历史多余空行）。
2. `kernel_platform/msm-kernel/drivers/misc/gpio-pwm.c` 清理尾随空格（无功能变化）。
3. `kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp.dtsi` 中 `nfc` 段清理多余空行（无功能变化）。

说明：
1. 本文主干描述的 DTS 分流流程与匹配逻辑不变。
2. 本次属于提交收敛与文档同步修订。
