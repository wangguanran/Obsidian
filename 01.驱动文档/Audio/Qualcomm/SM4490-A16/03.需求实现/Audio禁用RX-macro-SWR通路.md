# [项目代号] Audio 禁用 RX macro/SWR 通路降低功耗

## 概述

- **Change**: #196025
- **项目**: LA.VENDOR.1.0.R1
- **分支**: master_Snapdragon_Premium_High_2021.SPF.2.0.2_[项目代号]
- **作者**: [同事]
- **状态**: MERGED
- **类型**: 需求 (降功耗优化)
- **类型**: 需求 (power optimization)
- **芯片**: [项目代号] (基于 parrot/QCM4490)
- **SoC-Android**: SM4490-A16

## 背景

[项目代号] 板未使用 LPASS RX SWR 通路（无 WCD RX / Awinic I2C PA），但 RX macro 及 RX SWR master 仍按默认使能，导致以下问题：

1. **RX macro 无意义初始化**：probe 阶段执行无意义的 SWR 初始化与 `add_child_devices` work 调度，增加不必要的功耗和启动延迟。
2. **VA SWR master 无法释放**：VA SWR master 保留 `always-on` / `irq-wakeup` 属性，造成 codeccore 无法释放。
3. **无效音频路由残留**：DTS 中残留无效的 RX_TX DEC 音频路由配置。

## 解决方案

### DTS 配置

1. 将 `rx-macro` 及其 `swr-mstr` 设为 `disabled`
2. `num-macros` 由 3 改为 2
3. VA SWR master 的 `irq-wakeup-capable` 与 `is-always-on` 置 0
4. 删除无效 RX_TX DEC 路由

### Kernel 驱动修改

- `waipio.c`：用 `#if 0` 禁用 RX CDC DMA DAI link
- `lpass-cdc-rx-macro.c`：修改 RX macro 禁用逻辑

## 修改文件清单

| 文件 | 变更 | 说明 |
|------|------|------|
| `vendor/qcom/opensource/audio-kernel/asoc/codecs/lpass-cdc/lpass-cdc-rx-macro.c` | +18/−13 | 修改 RX macro 禁用 |
| `vendor/qcom/opensource/audio-kernel/asoc/waipio.c` | +8/−2 | 禁用 RX CDC DMA DAI link |
| `vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi` | +7/−7 | 禁用 rx-macro 及其 swr-mstr |
| `vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi` | +0/−4 | 删除无效 RX_TX DEC 路由 |

> 补丁内容请参考 Gerrit Change #196025

## 配置方式

### DTS 配置

在 `parrot-audio-overlay.dtsi` 中禁用 RX macro 及 SWR master：

```dts
&rx_macro {
    status = "disabled";
};

&swr3 {
    status = "disabled";
};
```

调整 `lpass-cdc` 节点中 macro 数量：

```dts
&lpass_cdc {
    qcom,num-macros = <2>;  /* 由 3 改为 2 */
};
```

清除 VA SWR master 的 always-on / wakeup 属性：

```dts
&swr0 {
    qcom,irq-wakeup-capable = <0>;
    qcom,is-always-on = <0>;
};
```

在 `parrot-audio-qrd.dtsi` 中删除无效的 RX_TX DEC 路由。

### Kernel 配置

- 无需额外 kernel config 修改
- 通过 `#if 0` 在 `waipio.c` 中编译屏蔽 RX CDC DMA DAI link

## 验证方式

### 1. 编译验证

确保 kernel 和 DTS 编译通过，无新增 warning：

```bash
# 编译 kernel
make ARCH=arm64 ...
# 编译 DTS
make dtbs
```

### 2. 启动日志验证

检查 kernel 启动日志，确认 RX macro 不再 probe：

```bash
# 确认 RX macro 未被 probe
dmesg | grep -i "rx-macro"
# 预期：无相关 probe 日志输出
```

### 3. 功耗验证

通过以下方式确认低功耗状态：

```bash
# 查看 audio codec 电源状态
cat /sys/kernel/debug/regulator/...

# 查看 SWR 总线状态
cat /sys/kernel/debug/swr_master/...
```

### 4. 功能验证

验证音频播放/录音功能正常：

```bash
# 播放测试
tinyplay /data/test.wav

# 录音测试
tinycap /data/record.wav
```

### 5. 补丁验证结果

| 验证项目 | 结果 | 说明 |
|---------|------|------|
| 补丁可应用性 | ✅ 干净应用 | 已通过 git apply 验证，在 134 HEAD 基线无冲突 |
| 变更文件 | 4 个 | lpass-cdc-rx-macro.c (+31/-13), waipio.c (+10/-2), overlay.dtsi (+7/-7), qrd.dtsi (+0/-4) |
| 134 仓库状态 | ⚠️ 未合入 | 当前 HEAD 6ebcb463ebb 未包含该补丁 |
| 文件权限 | ⚠️ 仅警告 | 100644 vs 100755 差异，不影响功能 |

> 补丁文件：`/tmp/gerrit-patches-rest/196025.patch`

## 补丁内容

```diff
[PATCH] [[项目代号]][93821][Audio] Disable RX macro/SWR path, drop VA always-on [Owner][同事]

diff --git a/vendor/qcom/opensource/audio-kernel/asoc/codecs/lpass-cdc/lpass-cdc-rx-macro.c b/vendor/qcom/opensource/audio-kernel/asoc/codecs/lpass-cdc/lpass-cdc-rx-macro.c
index 058154b..543e672 100644
--- a/vendor/qcom/opensource/audio-kernel/asoc/codecs/lpass-cdc/lpass-cdc-rx-macro.c
+++ b/vendor/qcom/opensource/audio-kernel/asoc/codecs/lpass-cdc/lpass-cdc-rx-macro.c
@@ -4719,8 +4719,10 @@
 			__func__);
 		return -EPROBE_DEFER;
 	}
-	msm_cdc_pinctrl_set_wakeup_capable(
-				rx_priv->rx_swr_gpio_p, false);
+	if (is_used_rx_swr_gpio){
+		msm_cdc_pinctrl_set_wakeup_capable(
+					rx_priv->rx_swr_gpio_p, false);
+	}
 
 	rx_io_base = devm_ioremap(&pdev->dev, rx_base_addr,
 				  LPASS_CDC_RX_MACRO_MAX_OFFSET);
@@ -4736,16 +4738,18 @@
 		return -ENOMEM;
 	}
 	rx_priv->rx_mclk_mode_muxsel = muxsel_io;
-	rx_priv->reset_swr = true;
-	INIT_WORK(&rx_priv->lpass_cdc_rx_macro_add_child_devices_work,
-		  lpass_cdc_rx_macro_add_child_devices);
-	rx_priv->swr_plat_data.handle = (void *) rx_priv;
-	rx_priv->swr_plat_data.read = NULL;
-	rx_priv->swr_plat_data.write = NULL;
-	rx_priv->swr_plat_data.bulk_write = NULL;
-	rx_priv->swr_plat_data.clk = rx_swrm_clock;
-	rx_priv->swr_plat_data.core_vote = lpass_cdc_rx_macro_core_vote;
-	rx_priv->swr_plat_data.handle_irq = NULL;
+	if (is_used_rx_swr_gpio){
+		rx_priv->reset_swr = true;
+		INIT_WORK(&rx_priv->lpass_cdc_rx_macro_add_child_devices_work,
+			  lpass_cdc_rx_macro_add_child_devices);
+		rx_priv->swr_plat_data.handle = (void *) rx_priv;
+		rx_priv->swr_plat_data.read = NULL;
+		rx_priv->swr_plat_data.write = NULL;
+		rx_priv->swr_plat_data.bulk_write = NULL;
+		rx_priv->swr_plat_data.clk = rx_swrm_clock;
+		rx_priv->swr_plat_data.core_vote = lpass_cdc_rx_macro_core_vote;
+		rx_priv->swr_plat_data.handle_irq = NULL;
+	}
 
 	rx_priv->clk_id = default_clk_id;
 	rx_priv->default_clk_id  = default_clk_id;
@@ -4779,7 +4783,8 @@
 	pm_runtime_set_suspended(&pdev->dev);
 	pm_suspend_ignore_children(&pdev->dev, true);
 	pm_runtime_enable(&pdev->dev);
-	schedule_work(&rx_priv->lpass_cdc_rx_macro_add_child_devices_work);
+	if (is_used_rx_swr_gpio)
+		schedule_work(&rx_priv->lpass_cdc_rx_macro_add_child_devices_work);
 	return 0;
 
 err_reg_macro:
diff --git a/vendor/qcom/opensource/audio-kernel/asoc/waipio.c b/vendor/qcom/opensource/audio-kernel/asoc/waipio.c
index 42036a2..eb9d31d 100644
--- a/vendor/qcom/opensource/audio-kernel/asoc/waipio.c
+++ b/vendor/qcom/opensource/audio-kernel/asoc/waipio.c
@@ -841,6 +841,7 @@
 };
 
 static struct snd_soc_dai_link msm_rx_tx_cdc_dma_be_dai_links[] = {
+#if 0
 	/* RX CDC DMA Backend DAI Links */
 	{
 		.name = LPASS_BE_RX_CDC_DMA_RX_0,
@@ -909,7 +910,10 @@
 		.ops = &msm_common_be_ops,
 		SND_SOC_DAILINK_REG(rx_dma_rx6),
 	},
-	/* TX CDC DMA Backend DAI Links */
+#endif
+	/* TX CDC DMA Backend DAI Links — keep TX_3/TX_4 for DMIC capture.
+	 * RX CDC DMA links above are disabled (no RX SWR / Awinic I2C PA).
+	 * Use msm_tx_codec_init: msm_rx_tx_codec_init touches RX FIR and needs rx_macro. */
 	{
 		.name = LPASS_BE_TX_CDC_DMA_TX_3,
 		.stream_name = LPASS_BE_TX_CDC_DMA_TX_3,
@@ -919,6 +923,7 @@
 		.ignore_suspend = 1,
 		.ops = &msm_common_be_ops,
 		SND_SOC_DAILINK_REG(tx_dma_tx3),
+		.init = &msm_tx_codec_init,
 	},
 	{
 		.name = LPASS_BE_TX_CDC_DMA_TX_4,
@@ -1908,7 +1913,8 @@
 }
 
 
-static int msm_rx_tx_codec_init(struct snd_soc_pcm_runtime *rtd)
+/* Kept for re-enabling RX CDC DMA links; unused while RX is #if 0'd. */
+static int __maybe_unused msm_rx_tx_codec_init(struct snd_soc_pcm_runtime *rtd)
 {
 	int codec_variant = -1;
 	struct snd_soc_component *component = NULL;
diff --git a/vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi b/vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi
index bfb5391..0d6d4f4 100755
--- a/vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi
+++ b/vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi
@@ -5,7 +5,7 @@
 #include "waipio-lpi.dtsi"
 
 &lpass_cdc {
-	qcom,num-macros = <3>;
+	qcom,num-macros = <2>;
 	qcom,lpass-cdc-version = <6>;
 	#address-cells = <1>;
 	#size-cells = <1>;
@@ -59,8 +59,10 @@
 				<3 SWRM_TX3_CH3 0x4>, <3 SWRM_TX3_CH4 0x8>;
 			qcom,swr-num-dev = <5>;
 			qcom,swr-clock-stop-mode0 = <1>;
-			qcom,swr-mstr-irq-wakeup-capable = <1>;
-			qcom,is-always-on = <1>;
+			/* No AON/wake-word on this board (wcd-disabled, no VA SWR slaves).
+			 * Keep always-on and irq-wakeup both off so codeccore can release. */
+			qcom,swr-mstr-irq-wakeup-capable = <0>;
+			qcom,is-always-on = <0>;
 			wcd938x_tx_slave: wcd938x-tx-slave {
 				compatible = "qcom,wcd938x-slave";
 				reg = <0x0D 0x01170223>;
@@ -82,6 +84,7 @@
 	};
 
 	rx_macro: rx-macro@3200000 {
+		status = "disabled";
 		compatible = "qcom,lpass-cdc-rx-macro";
 		reg = <0x3200000 0x0>;
 		qcom,is-used-swr-gpio = <0>;
@@ -92,6 +95,7 @@
 		clock-names = "rx_mclk2_2x_clk";
 		clocks = <&clock_audio_rx_mclk2_2x_clk 0>;
 		swr1: rx_swr_master {
+			status = "disabled";
 			compatible = "qcom,swr-mstr";
 			#address-cells = <2>;
 			#size-cells = <0>;
@@ -400,10 +404,6 @@
 			"WSA SRC0_INP", "SRC0",
 			"WSA_TX DEC0_INP", "TX DEC0 MUX",
 			"WSA_TX DEC1_INP", "TX DEC1 MUX",
-			"RX_TX DEC0_INP", "TX DEC0 MUX",
-			"RX_TX DEC1_INP", "TX DEC1 MUX",
-			"RX_TX DEC2_INP", "TX DEC2 MUX",
-			"RX_TX DEC3_INP", "TX DEC3 MUX",
 			"SpkrLeft IN", "WSA_SPK1 OUT",
 			"SpkrRight IN", "WSA_SPK2 OUT",
 			"TX SWR_INPUT", "WCD_TX_OUTPUT",
diff --git a/vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi b/vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi
index e5e178b..9341cd9 100755
--- a/vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi
+++ b/vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi
@@ -21,10 +21,6 @@
 			"TX DMIC4", "VCC MIC BIAS1",
 			"TX DMIC5", "Digital Mic5",
 			"TX DMIC5", "VCC MIC BIAS1",
-			"RX_TX DEC0_INP", "TX DEC0 MUX",
-			"RX_TX DEC1_INP", "TX DEC1 MUX",
-			"RX_TX DEC2_INP", "TX DEC2 MUX",
-			"RX_TX DEC3_INP", "TX DEC3 MUX",
 			"VA SWR_INPUT", "VA_SWR_CLK",
 			"VA_AIF1 CAP", "VA_SWR_CLK",
 			"VA_AIF2 CAP", "VA_SWR_CLK",
```

## 源码归档

| 文件 | 归档路径 | 说明 |
|------|---------|------|
| `lpass-cdc-rx-macro.c` | [[04.问题案例/源码归档/lpass-cdc-rx-macro.c\\|lpass-cdc-rx-macro.c]] | 补丁前基线，4846行，来自 134 HEAD |
| `waipio.c` | [[04.问题案例/源码归档/waipio.c\\|waipio.c]] | 补丁前基线，2453行，来自 134 HEAD |
| `parrot-audio-overlay.dtsi` | [[04.问题案例/源码归档/parrot-audio-overlay.dtsi\\|parrot-audio-overlay.dtsi]] | 补丁前基线，700行，来自 134 HEAD |
| `parrot-audio-qrd.dtsi` | [[04.问题案例/源码归档/parrot-audio-qrd.dtsi\\|parrot-audio-qrd.dtsi]] | 补丁前基线，32行，来自 134 HEAD |
| 归档索引 | [[04.问题案例/源码归档/README.md\\|README.md]] | 含 commit 历史、补丁验证状态 |

- **134 仓库**: `/home3/[同事]/workspace/[项目代号]/LA.VENDOR.1.0.R1` (HEAD `6ebcb463ebb`)
- **分支**: `master_Snapdragon_Premium_High_2021.SPF.2.0.2_[项目代号]`
- **补丁状态**: Gerrit #196025 MERGED，134 仓库 **未合入**

## 引用文件索引

- [[01.驱动文档/Audio/Qualcomm/SM4490-A16/04.问题案例/分析/Audio禁用RX-macro-SWR通路-分析|分析文档]]
- [[04.问题案例/源码归档/README.md|源码归档索引]]
- 关联模块：[[01.驱动文档/Audio/Qualcomm/|Qualcomm Audio 驱动文档]]

## 移植文档状态

| 检查项 | 状态 | 说明 |
|--------|------|------|
| 移植文档 (02.Bringup与配置/) | ❌ 缺失 | SM4490-A16 目录下无移植文档，需补充 Audio Bringup 整体流程 |
| 源码与补丁索引 (02.Bringup与配置/91.源码与补丁索引/) | ❌ 缺失 | 当前仅归档于 04.问题案例/源码归档/，需按规范迁移 |
| 00.总览.md | ❌ 缺失 | 无平台 Audio 总览文档 |

> 根据 Obsidian 驱动文档规范，移植文档应存放于 `02.Bringup与配置/` 目录，当前所有内容仅归档于 `04.问题案例/`。

---

_Author: wangguanran_