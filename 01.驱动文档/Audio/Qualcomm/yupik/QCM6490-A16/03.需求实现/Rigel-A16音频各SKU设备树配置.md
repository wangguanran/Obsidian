# Rigel A16 音频各 SKU 设备树配置

> **模块**: Audio | **厂商**: Qualcomm | **芯片**: QCM6490 (yupik/lahaina)
> **平台**: QCM6490-A16 (Develop_QCM6490.LA.6.0) | **类型**: 需求
> **Change**: #196739 | **作者**: weirong | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196739 |
| 项目 | meigla/platform/vendor/qcom/opensource/audio-devicetree |
| 分支 | Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_Rigel |
| 作者 | weirong |
| 类型 | 需求（Rigel A16 各 SKU 音频设备树配置） |
| 芯片 | Qualcomm QCM6490 (yupik/lahaina) |
| 平台 | QCM6490-A16 |
| 模块 | Audio（audio-devicetree / yupik_snd / bolero / WSA / DMIC） |
| 提交标题 | `[119404][rigel_A16][Audio]modify devicetree for Rigel A16 SKUs[Owner]weirong` |
| 任务 | Task 119404 |

## 需求描述

Rigel A16 项目包含多个 SKU（MC933 / MC934 / MC936 / MC937 / MC938 / MC9392），各 SKU 硬件差异（扬声器通路、DMIC 数量与位置、是否带 WSA 功放、是否使用 SWR 接口）需要分别落到音频设备树 overlay 中。本次提交为各 SKU 补齐：

- `qcom,model` 声卡名（`lahaina-yupikidp-mcXXX-snd-card`）
- MIC BIAS 供电 GPIO（`vcc-micbias1-gpio` = TLMM 96，`vcc-micbias2-gpio` = TLMM 97）
- `fsa4480-i2c-handle = <0>`（Type-C 音频开关无效化，走板载模拟通路）
- 扬声器路由 `SpL IN`→`WSA_SPK1 OUT`、`SpR IN`→`WSA_SPK2 OUT`
- DMIC0~3 路由（`TX DMICx`/`VA DMICx` → `Digital Micx`，BIAS 接 `VCC MIC BIAS1`）
- MC934 变体：关闭 WSA 通路（`wsa_macro`/`wsa883x_0221/0222` disabled、`qcom,num-macros = <3>`、VA/RX macro 取消 SWR GPIO）
- 公共 overlay：`qcom,ext-disp-audio-rx = <0>`（关闭外部显示音频接收）
- Kbuild：移除不再使用的 `kera-audio-mtp-wcn7750.dtbo`、`kera-audio-evk-wcd9378-dmic.dtbo`

## 方案

按 SKU 维护独立 overlay 文件，公共配置放 `yupik-iot-audio-overlay.dtsi`，每个 SKU overlay 内仅描述差异；编译清单（`Kbuild`）只保留实际生产的 dtbo。MC933/MC936/MC937/MC938/MC9392 为 WSA+DMIC 通路变体，MC934 为去 WSA（SWR 改普通 GPIO）变体。

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/Kbuild\|Kbuild]] | 4 行改动 | 移除 wcn7750 / wcd9378-dmic 两个 dtbo 目标 |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc933.dtsi\|yupik-audio-overlay-mc933.dtsi]] | +30/-3 | model / micbias / routing（WSA + DMIC0~3） |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc934.dtsi\|yupik-audio-overlay-mc934.dtsi]] | +48/-6 | 无 WSA 变体：bolero num-macros=3、wsa_macro/wsa883x disabled、VA/RX 去 SWR |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc936.dtsi\|yupik-audio-overlay-mc936.dtsi]] | +30/-3 | 同 MC933 模式 |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc937.dtsi\|yupik-audio-overlay-mc937.dtsi]] | +30/-3 | 同 MC933 模式 |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc938.dtsi\|yupik-audio-overlay-mc938.dtsi]] | +28/-3 | 同 MC933 模式 |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc9392.dtsi\|yupik-audio-overlay-mc9392.dtsi]] | +30/-3 | 同 MC933 模式 |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-iot-audio-overlay.dtsi\|yupik-iot-audio-overlay.dtsi]] | 1 行 | `qcom,ext-disp-audio-rx = <0>` |

## 配置方式

以 MC937 为例的 SKU overlay 关键配置：

```dts
&yupik_snd {
    qcom,model = "lahaina-yupikidp-mc937-snd-card";
    qcom,wcd-disabled = <1>;
    asoc-codec  = <&stub_codec>, <&bolero>,
                  <&wsa883x_0221>, <&wsa883x_0222>;
    asoc-codec-names = "msm-stub-codec.1", "bolero_codec",
                       "wsa-codec1", "wsa-codec2";
    vcc-micbias1-gpio = <&tlmm 96 0>;
    vcc-micbias2-gpio = <&tlmm 97 0>;
    fsa4480-i2c-handle = <0>;
    qcom,audio-routing =
        "RX_TX DEC0_INP", "TX DEC0 MUX",
        ...
        "SpL IN", "WSA_SPK1 OUT",
        "SpR IN", "WSA_SPK2 OUT",
        /* Right DMIC1 @ GPIO150/151 */
        "TX DMIC0", "Digital Mic0",
        "TX DMIC0", "VCC MIC BIAS1",
        ...
        "VA DMIC3", "VCC MIC BIAS1";
};
```

MC934（无 WSA）变体：

```dts
&bolero {
    qcom,num-macros = <3>;
};
&wsa_macro { status = "disabled"; };
&wsa883x_0221 { status = "disabled"; };
&wsa883x_0222 { status = "disabled"; };
&va_macro {
    qcom,is-used-swr-gpio = <0>;
    /delete-property/ qcom,va-swr-gpios;
};
&rx_macro {
    qcom,is-used-swr-gpio = <0>;
    /delete-property/ qcom,rx-swr-gpios;
};
```

Kbuild 编译清单（dtbo-y）：

```makefile
dtbo-y += kera-audio.dtbo \
        kera-audio-mtp-qmp1000.dtbo \
        kera-audio-qrd.dtbo \
        kera-audio-rcm.dtbo \
        kera-audio-rcm-orne.dtbo
```

## 验证方式

- 编译验证：audio-devicetree 仓库 `make dtbo`（或整包编译）确认 8 个文件语法与 dtbo 生成正常；本 Change 已合入主干，CI 编译通过。
- 实机验证（建议）：各 SKU 烧录后：
  - `cat /proc/asound/cards` 确认声卡名为 `lahaina-yupikidp-mcXXX-snd-card`
  - 播放测试音确认 WSA 扬声器有声（MC933/936/937/938/9392）
  - `tinymix` 检查 `Digital Mic0~3` 通路，录音验证 DMIC 拾音（GPIO150~153）
  - MC934 确认扬声器无输出属预期（无 WSA 硬件）

## 结论

通过按 SKU 拆分 overlay 并补齐 model/micbias/audio-routing，Rigel A16 各 SKU 音频通路（WSA 扬声器 + 4 路 DMIC）在设备树层配置完成；MC934 走无 WSA 精简通路。公共层关闭 ext-disp-audio-rx，避免未接 HDMI 音频时的错误路由。

## 补丁内容

```diff
Subject: [PATCH] [119404][rigel_A16][Audio]modify devicetree for Rigel A16
 SKUs[Owner]weirong

---
 Kbuild                          |  4 +--
 yupik-audio-overlay-mc933.dtsi  | 30 ++++++++++++++++++---
 yupik-audio-overlay-mc934.dtsi  | 48 +++++++++++++++++++++++++++------
 yupik-audio-overlay-mc936.dtsi  | 30 ++++++++++++++++++---
 yupik-audio-overlay-mc937.dtsi  | 30 ++++++++++++++++++---
 yupik-audio-overlay-mc938.dtsi  | 28 ++++++++++++++++---
 yupik-audio-overlay-mc9392.dtsi | 30 ++++++++++++++++++---
 yupik-iot-audio-overlay.dtsi    |  2 +-
 8 files changed, 171 insertions(+), 31 deletions(-)

diff --git a/Kbuild b/Kbuild
index ce11d2c..8136e3d 100644
--- a/Kbuild
+++ b/Kbuild
@@ -120,9 +120,7 @@ dtbo-y += kera-audio.dtbo \
                 kera-audio-mtp-qmp1000.dtbo \
                 kera-audio-qrd.dtbo \
                 kera-audio-rcm.dtbo \
-                kera-audio-rcm-orne.dtbo \
-                kera-audio-mtp-wcn7750.dtbo \
-                kera-audio-evk-wcd9378-dmic.dtbo
+                kera-audio-rcm-orne.dtbo
 
 endif
 
diff --git a/yupik-audio-overlay-mc933.dtsi b/yupik-audio-overlay-mc933.dtsi
index 48bd7c7..88f4d52 100755
--- a/yupik-audio-overlay-mc933.dtsi
+++ b/yupik-audio-overlay-mc933.dtsi
@@ -18,6 +18,7 @@
 };
 
 &yupik_snd {
+	qcom,model = "lahaina-yupikidp-mc933-snd-card";
     qcom,wcd-disabled = <1>;
     asoc-codec  = <&stub_codec>, <&bolero>,
                     //<&wcd937x_codec>,
@@ -25,7 +26,10 @@
     asoc-codec-names = "msm-stub-codec.1", "bolero_codec",
                         // "wcd937x_codec",
                         "wsa-codec1", "wsa-codec2";
-    qcom,audio-routing = 
+	vcc-micbias1-gpio = <&tlmm 96 0>;
+	vcc-micbias2-gpio = <&tlmm 97 0>;
+	fsa4480-i2c-handle = <0>;
+    qcom,audio-routing =
 		/*"HAP_IN", "PCM_OUT",*/
 		"WSA SRC0_INP", "SRC0",
 		"WSA_TX DEC0_INP", "TX DEC0 MUX",
@@ -33,9 +37,27 @@
 		"RX_TX DEC0_INP", "TX DEC0 MUX",
 		"RX_TX DEC1_INP", "TX DEC1 MUX",
 		"RX_TX DEC2_INP", "TX DEC2 MUX",
-		"RX_TX DEC3_INP", "TX DEC3 MUX";
+		"RX_TX DEC3_INP", "TX DEC3 MUX",
 		//"SpkrLeft IN", "WSA_SPK1 OUT",
 		//"SpkrRight IN", "WSA_SPK2 OUT",
-		//"SpL IN", "WSA_SPK1 OUT",
-		//"SpR IN", "WSA_SPK2 OUT";
+		"SpL IN", "WSA_SPK1 OUT",
+		"SpR IN", "WSA_SPK2 OUT",
+		/* Right DMIC1 @ GPIO150/151 */
+		"TX DMIC0", "Digital Mic0",
+		"TX DMIC0", "VCC MIC BIAS1",
+		"TX DMIC1", "Digital Mic1",
+		"TX DMIC1", "VCC MIC BIAS1",
+		/* Left DMIC2 @ GPIO152/153 */
+		"TX DMIC2", "Digital Mic2",
+		"TX DMIC2", "VCC MIC BIAS1",
+		"TX DMIC3", "Digital Mic3",
+		"TX DMIC3", "VCC MIC BIAS1",
+		"VA DMIC0", "Digital Mic0",
+		"VA DMIC0", "VCC MIC BIAS1",
+		"VA DMIC1", "Digital Mic1",
+		"VA DMIC1", "VCC MIC BIAS1",
+		"VA DMIC2", "Digital Mic2",
+		"VA DMIC2", "VCC MIC BIAS1",
+		"VA DMIC3", "Digital Mic3",
+		"VA DMIC3", "VCC MIC BIAS1";
 };
diff --git a/yupik-audio-overlay-mc934.dtsi b/yupik-audio-overlay-mc934.dtsi
index 48bd7c7..e62efcb 100755
--- a/yupik-audio-overlay-mc934.dtsi
+++ b/yupik-audio-overlay-mc934.dtsi
@@ -1,5 +1,31 @@
 #include "yupik-iot-audio-overlay.dtsi"
 
+&bolero {
+	qcom,num-macros = <3>;
+};
+
+&wsa_macro {
+    status = "disabled";
+};
+
+&va_macro {
+	qcom,is-used-swr-gpio = <0>;
+	/delete-property/ qcom,va-swr-gpios;
+};
+
+&rx_macro {
+	qcom,is-used-swr-gpio = <0>;
+	/delete-property/ qcom,rx-swr-gpios;
+};
+
+&wsa883x_0221 {
+    status = "disabled";
+};
+
+&wsa883x_0222 {
+    status = "disabled";
+};
+
 &wcd937x_tx_slave {
     status = "disabled";
 };
@@ -18,24 +44,30 @@
 };
 
 &yupik_snd {
+	qcom,model = "lahaina-yupikidp-mc934-snd-card";
     qcom,wcd-disabled = <1>;
-    asoc-codec  = <&stub_codec>, <&bolero>,
+    asoc-codec  = <&stub_codec>, <&bolero>;
                     //<&wcd937x_codec>,
-                    <&wsa883x_0221>, <&wsa883x_0222>;
-    asoc-codec-names = "msm-stub-codec.1", "bolero_codec",
+                    //<&wsa883x_0221>, <&wsa883x_0222>;
+    asoc-codec-names = "msm-stub-codec.1", "bolero_codec";
                         // "wcd937x_codec",
-                        "wsa-codec1", "wsa-codec2";
+                        //"wsa-codec1", "wsa-codec2";
+	vcc-micbias1-gpio = <&tlmm 96 0>;
+	fsa4480-i2c-handle = <0>;
     qcom,audio-routing = 
 		/*"HAP_IN", "PCM_OUT",*/
-		"WSA SRC0_INP", "SRC0",
-		"WSA_TX DEC0_INP", "TX DEC0 MUX",
-		"WSA_TX DEC1_INP", "TX DEC1 MUX",
 		"RX_TX DEC0_INP", "TX DEC0 MUX",
 		"RX_TX DEC1_INP", "TX DEC1 MUX",
 		"RX_TX DEC2_INP", "TX DEC2 MUX",
-		"RX_TX DEC3_INP", "TX DEC3 MUX";
+		"RX_TX DEC3_INP", "TX DEC3 MUX",
 		//"SpkrLeft IN", "WSA_SPK1 OUT",
 		//"SpkrRight IN", "WSA_SPK2 OUT",
 		//"SpL IN", "WSA_SPK1 OUT",
 		//"SpR IN", "WSA_SPK2 OUT";
+		/* Single DMIC1 @ GPIO150/151 */
+		"TX DMIC0", "Digital Mic0",
+		"TX DMIC0", "VCC MIC BIAS1",
+		"TX DMIC1", "Digital Mic1",
+		"TX DMIC1", "VCC MIC BIAS1";
+	qcom,wsa-max-devs = <0>;
 };
diff --git a/yupik-audio-overlay-mc936.dtsi b/yupik-audio-overlay-mc936.dtsi
index 48bd7c7..3d70c49 100755
--- a/yupik-audio-overlay-mc936.dtsi
+++ b/yupik-audio-overlay-mc936.dtsi
@@ -18,6 +18,7 @@
 };
 
 &yupik_snd {
+	qcom,model = "lahaina-yupikidp-mc936-snd-card";
     qcom,wcd-disabled = <1>;
     asoc-codec  = <&stub_codec>, <&bolero>,
                     //<&wcd937x_codec>,
@@ -25,7 +26,10 @@
     asoc-codec-names = "msm-stub-codec.1", "bolero_codec",
                         // "wcd937x_codec",
                         "wsa-codec1", "wsa-codec2";
-    qcom,audio-routing = 
+	vcc-micbias1-gpio = <&tlmm 96 0>;
+	vcc-micbias2-gpio = <&tlmm 97 0>;
+	fsa4480-i2c-handle = <0>;
+    qcom,audio-routing =
 		/*"HAP_IN", "PCM_OUT",*/
 		"WSA SRC0_INP", "SRC0",
 		"WSA_TX DEC0_INP", "TX DEC0 MUX",
@@ -33,9 +37,27 @@
 		"RX_TX DEC0_INP", "TX DEC0 MUX",
 		"RX_TX DEC1_INP", "TX DEC1 MUX",
 		"RX_TX DEC2_INP", "TX DEC2 MUX",
-		"RX_TX DEC3_INP", "TX DEC3 MUX";
+		"RX_TX DEC3_INP", "TX DEC3 MUX",
 		//"SpkrLeft IN", "WSA_SPK1 OUT",
 		//"SpkrRight IN", "WSA_SPK2 OUT",
-		//"SpL IN", "WSA_SPK1 OUT",
-		//"SpR IN", "WSA_SPK2 OUT";
+		"SpL IN", "WSA_SPK1 OUT",
+		"SpR IN", "WSA_SPK2 OUT",
+		/* Right DMIC1 @ GPIO150/151 */
+		"TX DMIC0", "Digital Mic0",
+		"TX DMIC0", "VCC MIC BIAS1",
+		"TX DMIC1", "Digital Mic1",
+		"TX DMIC1", "VCC MIC BIAS1",
+		/* Left DMIC2 @ GPIO152/153 */
+		"TX DMIC2", "Digital Mic2",
+		"TX DMIC2", "VCC MIC BIAS1",
+		"TX DMIC3", "Digital Mic3",
+		"TX DMIC3", "VCC MIC BIAS1",
+		"VA DMIC0", "Digital Mic0",
+		"VA DMIC0", "VCC MIC BIAS1",
+		"VA DMIC1", "Digital Mic1",
+		"VA DMIC1", "VCC MIC BIAS1",
+		"VA DMIC2", "Digital Mic2",
+		"VA DMIC2", "VCC MIC BIAS1",
+		"VA DMIC3", "Digital Mic3",
+		"VA DMIC3", "VCC MIC BIAS1";
 };
diff --git a/yupik-audio-overlay-mc937.dtsi b/yupik-audio-overlay-mc937.dtsi
index 48bd7c7..fbdbb8d 100755
--- a/yupik-audio-overlay-mc937.dtsi
+++ b/yupik-audio-overlay-mc937.dtsi
@@ -18,6 +18,7 @@
 };
 
 &yupik_snd {
+	qcom,model = "lahaina-yupikidp-mc937-snd-card";
     qcom,wcd-disabled = <1>;
     asoc-codec  = <&stub_codec>, <&bolero>,
                     //<&wcd937x_codec>,
@@ -25,7 +26,10 @@
     asoc-codec-names = "msm-stub-codec.1", "bolero_codec",
                         // "wcd937x_codec",
                         "wsa-codec1", "wsa-codec2";
-    qcom,audio-routing = 
+	vcc-micbias1-gpio = <&tlmm 96 0>;
+	vcc-micbias2-gpio = <&tlmm 97 0>;
+	fsa4480-i2c-handle = <0>;
+    qcom,audio-routing =
 		/*"HAP_IN", "PCM_OUT",*/
 		"WSA SRC0_INP", "SRC0",
 		"WSA_TX DEC0_INP", "TX DEC0 MUX",
@@ -33,9 +37,27 @@
 		"RX_TX DEC0_INP", "TX DEC0 MUX",
 		"RX_TX DEC1_INP", "TX DEC1 MUX",
 		"RX_TX DEC2_INP", "TX DEC2 MUX",
-		"RX_TX DEC3_INP", "TX DEC3 MUX";
+		"RX_TX DEC3_INP", "TX DEC3 MUX",
 		//"SpkrLeft IN", "WSA_SPK1 OUT",
 		//"SpkrRight IN", "WSA_SPK2 OUT",
-		//"SpL IN", "WSA_SPK1 OUT",
-		//"SpR IN", "WSA_SPK2 OUT";
+		"SpL IN", "WSA_SPK1 OUT",
+		"SpR IN", "WSA_SPK2 OUT",
+		/* Right DMIC1 @ GPIO150/151 */
+		"TX DMIC0", "Digital Mic0",
+		"TX DMIC0", "VCC MIC BIAS1",
+		"TX DMIC1", "Digital Mic1",
+		"TX DMIC1", "VCC MIC BIAS1",
+		/* Left DMIC2 @ GPIO152/153 */
+		"TX DMIC2", "Digital Mic2",
+		"TX DMIC2", "VCC MIC BIAS1",
+		"TX DMIC3", "Digital Mic3",
+		"TX DMIC3", "VCC MIC BIAS1",
+		"VA DMIC0", "Digital Mic0",
+		"VA DMIC0", "VCC MIC BIAS1",
+		"VA DMIC1", "Digital Mic1",
+		"VA DMIC1", "VCC MIC BIAS1",
+		"VA DMIC2", "Digital Mic2",
+		"VA DMIC2", "VCC MIC BIAS1",
+		"VA DMIC3", "Digital Mic3",
+		"VA DMIC3", "VCC MIC BIAS1";
 };
diff --git a/yupik-audio-overlay-mc938.dtsi b/yupik-audio-overlay-mc938.dtsi
index 48bd7c7..54b889e 100755
--- a/yupik-audio-overlay-mc938.dtsi
+++ b/yupik-audio-overlay-mc938.dtsi
@@ -18,6 +18,7 @@
 };
 
 &yupik_snd {
+	qcom,model = "lahaina-yupikidp-mc938-snd-card";
     qcom,wcd-disabled = <1>;
     asoc-codec  = <&stub_codec>, <&bolero>,
                     //<&wcd937x_codec>,
@@ -25,6 +26,9 @@
     asoc-codec-names = "msm-stub-codec.1", "bolero_codec",
                         // "wcd937x_codec",
                         "wsa-codec1", "wsa-codec2";
+	vcc-micbias1-gpio = <&tlmm 96 0>;
+	vcc-micbias2-gpio = <&tlmm 97 0>;
+	fsa4480-i2c-handle = <0>;
     qcom,audio-routing = 
 		/*"HAP_IN", "PCM_OUT",*/
 		"WSA SRC0_INP", "SRC0",
@@ -33,9 +37,27 @@
 		"RX_TX DEC0_INP", "TX DEC0 MUX",
 		"RX_TX DEC1_INP", "TX DEC1 MUX",
 		"RX_TX DEC2_INP", "TX DEC2 MUX",
-		"RX_TX DEC3_INP", "TX DEC3 MUX";
+		"RX_TX DEC3_INP", "TX DEC3 MUX",
 		//"SpkrLeft IN", "WSA_SPK1 OUT",
 		//"SpkrRight IN", "WSA_SPK2 OUT",
-		//"SpL IN", "WSA_SPK1 OUT",
-		//"SpR IN", "WSA_SPK2 OUT";
+		"SpL IN", "WSA_SPK1 OUT",
+		"SpR IN", "WSA_SPK2 OUT",
+		/* Right DMIC1 @ GPIO150/151 */
+		"TX DMIC0", "Digital Mic0",
+		"TX DMIC0", "VCC MIC BIAS1",
+		"TX DMIC1", "Digital Mic1",
+		"TX DMIC1", "VCC MIC BIAS1",
+		/* Left DMIC2 @ GPIO152/153 */
+		"TX DMIC2", "Digital Mic2",
+		"TX DMIC2", "VCC MIC BIAS1",
+		"TX DMIC3", "Digital Mic3",
+		"TX DMIC3", "VCC MIC BIAS1",
+		"VA DMIC0", "Digital Mic0",
+		"VA DMIC0", "VCC MIC BIAS1",
+		"VA DMIC1", "Digital Mic1",
+		"VA DMIC1", "VCC MIC BIAS1",
+		"VA DMIC2", "Digital Mic2",
+		"VA DMIC2", "VCC MIC BIAS1",
+		"VA DMIC3", "Digital Mic3",
+		"VA DMIC3", "VCC MIC BIAS1";
 };
diff --git a/yupik-audio-overlay-mc9392.dtsi b/yupik-audio-overlay-mc9392.dtsi
index 48bd7c7..c3833f1 100755
--- a/yupik-audio-overlay-mc9392.dtsi
+++ b/yupik-audio-overlay-mc9392.dtsi
@@ -18,6 +18,7 @@
 };
 
 &yupik_snd {
+	qcom,model = "lahaina-yupikidp-mc9392-snd-card";
     qcom,wcd-disabled = <1>;
     asoc-codec  = <&stub_codec>, <&bolero>,
                     //<&wcd937x_codec>,
@@ -25,7 +26,10 @@
     asoc-codec-names = "msm-stub-codec.1", "bolero_codec",
                         // "wcd937x_codec",
                         "wsa-codec1", "wsa-codec2";
-    qcom,audio-routing = 
+	vcc-micbias1-gpio = <&tlmm 96 0>;
+	vcc-micbias2-gpio = <&tlmm 97 0>;
+	fsa4480-i2c-handle = <0>;
+    qcom,audio-routing =
 		/*"HAP_IN", "PCM_OUT",*/
 		"WSA SRC0_INP", "SRC0",
 		"WSA_TX DEC0_INP", "TX DEC0 MUX",
@@ -33,9 +37,27 @@
 		"RX_TX DEC0_INP", "TX DEC0 MUX",
 		"RX_TX DEC1_INP", "TX DEC1 MUX",
 		"RX_TX DEC2_INP", "TX DEC2 MUX",
-		"RX_TX DEC3_INP", "TX DEC3 MUX";
+		"RX_TX DEC3_INP", "TX DEC3 MUX",
 		//"SpkrLeft IN", "WSA_SPK1 OUT",
 		//"SpkrRight IN", "WSA_SPK2 OUT",
-		//"SpL IN", "WSA_SPK1 OUT",
-		//"SpR IN", "WSA_SPK2 OUT";
+		"SpL IN", "WSA_SPK1 OUT",
+		"SpR IN", "WSA_SPK2 OUT",
+		/* Right DMIC1 @ GPIO150/151 */
+		"TX DMIC0", "Digital Mic0",
+		"TX DMIC0", "VCC MIC BIAS1",
+		"TX DMIC1", "Digital Mic1",
+		"TX DMIC1", "VCC MIC BIAS1",
+		/* Left DMIC2 @ GPIO152/153 */
+		"TX DMIC2", "Digital Mic2",
+		"TX DMIC2", "VCC MIC BIAS1",
+		"TX DMIC3", "Digital Mic3",
+		"TX DMIC3", "VCC MIC BIAS1",
+		"VA DMIC0", "Digital Mic0",
+		"VA DMIC0", "VCC MIC BIAS1",
+		"VA DMIC1", "Digital Mic1",
+		"VA DMIC1", "VCC MIC BIAS1",
+		"VA DMIC2", "Digital Mic2",
+		"VA DMIC2", "VCC MIC BIAS1",
+		"VA DMIC3", "Digital Mic3",
+		"VA DMIC3", "VCC MIC BIAS1";
 };
diff --git a/yupik-iot-audio-overlay.dtsi b/yupik-iot-audio-overlay.dtsi
index 352ddb1..ff07e93 100644
--- a/yupik-iot-audio-overlay.dtsi
+++ b/yupik-iot-audio-overlay.dtsi
@@ -340,7 +340,7 @@
 	qcom,va-bolero-codec = <1>;
 	qcom,rxtx-bolero-codec = <1>;
 	qcom,wcn-btfm = <0>;
-	qcom,ext-disp-audio-rx = <1>;
+	qcom,ext-disp-audio-rx = <0>;
 	qcom,audio-routing =
 		"AMIC1", "Analog Mic1",
 		"Analog Mic1", "MIC BIAS1",
-- 
2.34.1
```

## 补丁验证

- 验证方式：134 服务器 `/tmp/p196739` 仓库 `git checkout FETCH_HEAD~1` 后 `git apply --check /tmp/196739.patch`
- 结果：✅ 可干净应用（父提交重建验证）

## 源码归档

| 内容 | 路径 | 说明 |
|------|------|------|
| dt_config/ | [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/\|dt_config]] | 7 个 SKU/公共 dtsi（补丁后合并版本） |
| kernel_driver/ | [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/Kbuild\|Kbuild]] | audio-devicetree 编译清单 |
| patches/ | [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/patches/196739.patch\|196739.patch]] | 完整补丁（已清隐私） |

## 引用文件索引

| 文件 | 完整路径 | 说明 |
|------|---------|------|
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/kernel_driver/Kbuild\|Kbuild]] | audio-devicetree/Kbuild | dtbo-y 编译清单 |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc933.dtsi\|yupik-audio-overlay-mc933.dtsi]] | audio-devicetree/yupik-audio-overlay-mc933.dtsi | MC933 SKU 音频 overlay |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc934.dtsi\|yupik-audio-overlay-mc934.dtsi]] | audio-devicetree/yupik-audio-overlay-mc934.dtsi | MC934 SKU（无 WSA） |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc936.dtsi\|yupik-audio-overlay-mc936.dtsi]] | audio-devicetree/yupik-audio-overlay-mc936.dtsi | MC936 SKU 音频 overlay |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc937.dtsi\|yupik-audio-overlay-mc937.dtsi]] | audio-devicetree/yupik-audio-overlay-mc937.dtsi | MC937 SKU 音频 overlay |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc938.dtsi\|yupik-audio-overlay-mc938.dtsi]] | audio-devicetree/yupik-audio-overlay-mc938.dtsi | MC938 SKU 音频 overlay |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-audio-overlay-mc9392.dtsi\|yupik-audio-overlay-mc9392.dtsi]] | audio-devicetree/yupik-audio-overlay-mc9392.dtsi | MC9392 SKU 音频 overlay |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-iot-audio-overlay.dtsi\|yupik-iot-audio-overlay.dtsi]] | audio-devicetree/yupik-iot-audio-overlay.dtsi | 公共音频 overlay |
| [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/patches/196739.patch\|196739.patch]] | 91.源码与补丁索引/patches/ | 补丁（已清隐私） |

_Author: wangguanran_
