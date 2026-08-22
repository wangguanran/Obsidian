# USB Type-C DIP ID host mux hubreset 修复

> **模块**: USB | **厂商**: Qualcomm | **芯片**: SM6115 (scuba)
> **平台**: SM6115-A14 (LA.VENDOR.13.2.1) | **类型**: Bug
> **Change**: #196403 | **作者**: [同事] | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196403 |
| 项目 | LA.VENDOR.13.2.1 |
| 分支 | [项目代号] |
| 作者 | [同事] |
| 类型 | Bug（USB Type-C DIP ID host mux hubreset 修复） |
| 芯片 | Qualcomm SM6115 (scuba) |
| 平台 | SM6115-A14 (LA.VENDOR.13.2.1) |
| 模块 | USB (extcon / dwc3 / devicetree) |
| 提交标题 | `[项目代号][120577][usb][Description]USB Type-C DIP ID host mux hubreset[Owner][同事]` |
| 任务 | Task 120577 |

## 现象

[项目代号] 项目使用 DIP 开关（D1/D2）在「USB Host（通过 HUB）」与「Type-C」两种模式间切换硬件连接。原有实现存在以下问题：

- USB ID 检测依赖 `pm2250_charger` 的 `usb_id_irq`（TLMM 89），而非专用 extcon 驱动，ID 电平判定不可靠（PMIC GPIO3 的 1.7V ID 电平在 1.8V VIN 选择错误时会被误判为低）；
- Host 模式 VBUS 使能、HUB 复位等 GPIO 无统一控制逻辑；
- HUB 复位（hubreset）不随 ID 状态联动，Host/Type-C 切换后 HUB 状态错误，导致 USB 枚举/工作异常。

## 根因分析

硬件上 DIP 开关把 PMIC GPIO3 作为 USB_ID：

- **DIP OFF / D1**：ID low → 期望进入 **Host 模式**（外接 HUB，GPIO19/103 供 5V，GPIO108 复位 HUB）；
- **DIP ON / D2**：ID high → 期望进入 **Type-C 模式**（交给 eud/phy 处理，不上报 gadget）。

原实现的问题：

1. **ID 检测路径错误**：`pm2250_charger` 的 `usb_id_irq` 绑定 TLMM 89，与实际硬件（PMIC GPIO3 = U900F E9 的 USB_ID）不一致；
2. **缺少 host mux 联动**：没有在 ID 变化时驱动 host VBUS 使能 GPIO（GPIO19/103）与 HUB_RESET（GPIO108）；
3. **skin_therm 占用 GPIO3**：`pm2250_vadc` 中 `ADC5_GPIO3_100K_PU` 把 GPIO3 配成 ADC 采样（skin_therm），与 USB_ID 功能冲突。

## 处理方案

1. 用 **extcon-usb-gpio 驱动**接管 PMIC GPIO3 的 ID 检测，扩展驱动支持：
   - `id-high-is-none`：ID-only 场景下 ID high 上报 none（Type-C 交接），ID low 上报 USB-HOST；
   - `hub-reset-gpios`：hub reset 跟随 ID（ID low → low，ID high → high）。
2. **dwc3-msm-core** 增加 `vbus-en-gpios`（数组）支持，`vbus_regulator_toggle()` 时同步驱动 GPIO19/103，Host 模式输出 5V。
3. **scuba-iot-idp.dtsi** 添加 `extcon_usb_id` 节点、PMIC GPIO3 pinctrl（1.8V 上拉）、TLMM GPIO19/103/108 pinctrl，并把 `extcon_usb_id` 挂到 `&usb0`。
4. **释放 GPIO3**：从 `scuba.dtsi` / `scuba_auto-pmic.dtsi` 删除 skin_therm ADC 通道，`scuba-thermal.dtsi` 禁用 chg-skin-therm。
5. **bengal_GKI.config** 使能 `CONFIG_EXTCON_USB_GPIO=m`。

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/extcon/extcon-usb-gpio.c\|extcon-usb-gpio.c]] | +27/-3 | DIP ID 检测扩展：`id_high_is_none`、`hub_reset_gpiod`、VBUS 判定逻辑 |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/dwc3/dwc3-msm-core.c\|dwc3-msm-core.c]] | +19/-0 | hubreset/VBUS 处理：`vbus_en_gpiods` 数组、`vbus_regulator_toggle()` 驱动 GPIO |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/extcon-usb-gpio.txt\|extcon-usb-gpio.txt]] | +5/-0 | binding 文档：`id-high-is-none`、`hub-reset-gpios` |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp.dtsi\|scuba-iot-idp.dtsi]] | +77/-1 | extcon 节点、pinctrl、usb0 配置（vbus-en-gpios、extcon 挂载） |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba.dtsi\|scuba.dtsi]] | +2/-13 | 移除 skin_therm（ADC5_GPIO3） |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba_auto-pmic.dtsi\|scuba_auto-pmic.dtsi]] | +1/-13 | 移除 skin_therm（ADC5_GPIO3） |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-thermal.dtsi\|scuba-thermal.dtsi]] | +2/-0 | chg-skin-therm 禁用 |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/bengal_GKI.config\|bengal_GKI.config]] | +1/-0 | `CONFIG_EXTCON_USB_GPIO=m` |

## 配置方式

### DTS：extcon 节点（scuba-iot-idp.dtsi）

```dts
&pm2250_gpios {
	usb_id {
		usb_id_default: usb_id_default {
			pins = "gpio3";
			function = "normal";
			input-enable;
			bias-pull-up;
			/* 1.7V ID level needs 1.8V VIN; vin-0 (VPH) makes 1.7V read as low */
			power-source = <1>;
		};
	};
};

&soc {
	extcon_usb_id: extcon_usb_id {
		compatible = "linux,extcon-usb-gpio";
		id-gpio = <&pm2250_gpios 3 GPIO_ACTIVE_HIGH>;
		/* DIP ON/D2: ID high → Type-C (none); DIP OFF/D1: ID low → host */
		id-high-is-none;
		/* GPIO108 HUB_RESET: ID low/host → low; ID high/type-c → high */
		hub-reset-gpios = <&tlmm 108 GPIO_ACTIVE_HIGH>;
		pinctrl-names = "default";
		pinctrl-0 = <&usb_id_default &hub_reset_default>;
	};
};
```

### DTS：usb0 挂载（scuba-iot-idp.dtsi）

```dts
&usb0 {
	extcon = <&qusb_phy0>, <&eud>, <&extcon_usb_id>;
	/* GPIO19_USBA_POWER_EN + GPIO103_OTG_5V_POWER_EN: host=high, none/device=low */
	vbus-en-gpios = <&tlmm 19 GPIO_ACTIVE_HIGH>,
			<&tlmm 103 GPIO_ACTIVE_HIGH>;
	pinctrl-names = "default";
	pinctrl-0 = <&usba_pwr_en_default &otg_5v_en_default>;
};
```

### DTS：TLMM pinctrl（scuba-iot-idp.dtsi）

```dts
&tlmm {
	usba_pwr_en_default: usba_pwr_en_default {
		mux { pins = "gpio19"; function = "gpio"; };
		config { pins = "gpio19"; drive-strength = <2>; bias-disable; output-low; };
	};
	otg_5v_en_default: otg_5v_en_default {
		mux { pins = "gpio103"; function = "gpio"; };
		config { pins = "gpio103"; drive-strength = <2>; bias-disable; output-low; };
	};
	hub_reset_default: hub_reset_default {
		mux { pins = "gpio108"; function = "gpio"; };
		config { pins = "gpio108"; drive-strength = <2>; bias-disable; output-high; };
	};
};
```

### 内核配置（bengal_GKI.config）

```
CONFIG_EXTCON_USB_GPIO=m
```

### 机制总结

```
DIP D1 (ID low)  → extcon 上报 EXTCON_USB_HOST
                 → hub-reset(GPIO108) = low  → HUB 复位
                 → vbus-en-gpios(GPIO19/103) = high → HOST 5V 输出
DIP D2 (ID high) → extcon 上报 none（id-high-is-none）
                 → hub-reset(GPIO108) = high
                 → vbus-en-gpios = low → Type-C 交接给 eud/phy
```

## 验证方式

| 项目 | 内容 |
|------|------|
| 编译 | ✅ 内核编译 PASS（bengal 配置） |
| DIP OFF/D1 (ID low) | 设备进入 Host 模式：`lsusb` 可见 HUB/设备，GPIO108 为低，GPIO19/103 输出 5V |
| DIP ON/D2 (ID high) | Type-C 模式：可识别 U 盘/充电，HUB 不复位 |
| dmesg 检查 | `extcon-usb-gpio` 上报 `EXTCON_USB_HOST` 状态 |

## 结论

通过 extcon-usb-gpio 驱动接管 DIP ID 检测，并联动 host VBUS 使能（GPIO19/103）与 HUB_RESET（GPIO108），实现 DIP 开关控制的 Host/Type-C 双模式正确切换；同时释放被 skin_therm 占用的 PMIC GPIO3。补丁可在 134 源码树父提交上干净应用。

## 补丁内容

```diff
Subject: [PATCH] [项目代号][120577][usb][Description]USB Type-C DIP ID host mux hubreset[Owner][同事]

[Solution]extcon-usb-gpio on PMIC GPIO3; GPIO19/103 host VBUS; GPIO108 hubreset follows ID

---
diff --git a/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config b/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
index 77bb378..3cd796d 100644
--- a/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
+++ b/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
@@ -26,6 +26,7 @@
 CONFIG_CPU_FREQ_GOV_ONDEMAND=m
 CONFIG_CPU_FREQ_GOV_USERSPACE=m
 CONFIG_CPU_IDLE_GOV_QCOM_LPM=m
+CONFIG_EXTCON_USB_GPIO=m
 CONFIG_HWSPINLOCK_QCOM=m
 CONFIG_I2C_MSM_GENI=m
 CONFIG_INPUT_PM8941_PWRKEY=m
diff --git a/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c b/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c
index f06be6d..8ec9d5d 100644
--- a/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c
+++ b/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c
@@ -33,6 +33,10 @@
 
 	unsigned long debounce_jiffies;
 	struct delayed_work wq_detcable;
+	/* ID-only: ID high = none (not gadget). Used by mux/DIP to Type-C. */
+	bool id_high_is_none;
+	/* DIP host (ID low) → drive low; DIP type-c (ID high) → drive high */
+	struct gpio_desc *hub_reset_gpiod;
 };
 
 static const unsigned int usb_extcon_cable[] = {
@@ -57,7 +61,8 @@
  * In case we have only one of these signals:
  * - VBUS only - we want to distinguish between [1] and [2], so ID is always 1.
  * - ID only - we want to distinguish between [1] and [4], so VBUS = ID.
-*/
+ * - ID only + id-high-is-none: distinguish [2] none vs [4] USB-HOST (VBUS = 0).
+ */
 static void usb_extcon_detect_cable(struct work_struct *work)
 {
 	int id, vbus;
@@ -68,8 +73,12 @@
 	/* check ID and VBUS and update cable state */
 	id = info->id_gpiod ?
 		gpiod_get_value_cansleep(info->id_gpiod) : 1;
-	vbus = info->vbus_gpiod ?
-		gpiod_get_value_cansleep(info->vbus_gpiod) : id;
+	if (info->vbus_gpiod)
+		vbus = gpiod_get_value_cansleep(info->vbus_gpiod);
+	else if (info->id_high_is_none)
+		vbus = 0;
+	else
+		vbus = id;
 
 	/* at first we clean states which are no longer active */
 	if (id)
@@ -83,6 +92,10 @@
 		if (vbus)
 			extcon_set_state_sync(info->edev, EXTCON_USB, true);
 	}
+
+	/* ID low (host/D1): hub reset low; ID high (type-c/D2): hub reset high */
+	if (info->hub_reset_gpiod)
+		gpiod_set_value_cansleep(info->hub_reset_gpiod, !!id);
 }
 
 static irqreturn_t usb_irq_handler(int irq, void *dev_id)
@@ -125,6 +138,17 @@
 	if (IS_ERR(info->vbus_gpiod))
 		return PTR_ERR(info->vbus_gpiod);
 
+	info->hub_reset_gpiod = devm_gpiod_get_optional(&pdev->dev, "hub-reset",
+							GPIOD_OUT_HIGH);
+	if (IS_ERR(info->hub_reset_gpiod))
+		return PTR_ERR(info->hub_reset_gpiod);
+	if (info->hub_reset_gpiod)
+		dev_info(dev, "hub-reset: ID low=low, ID high=high\n");
+
+	info->id_high_is_none = of_property_read_bool(np, "id-high-is-none");
+	if (info->id_high_is_none)
+		dev_info(dev, "id-high-is-none: ID low=host, ID high=none\n");
+
 	info->edev = devm_extcon_dev_allocate(dev, usb_extcon_cable);
 	if (IS_ERR(info->edev)) {
 		dev_err(dev, "failed to allocate extcon device\n");
diff --git a/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c b/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c
index 7d64422..bab869e 100644
--- a/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c
+++ b/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c
@@ -25,6 +25,7 @@
 #include <linux/of.h>
 #include <linux/of_platform.h>
 #include <linux/of_gpio.h>
+#include <linux/gpio/consumer.h>
 #include <linux/list.h>
 #include <linux/uaccess.h>
 #include <linux/usb/ch9.h>
@@ -620,6 +621,7 @@
 	bool			read_u1u2;
 
 	struct gpio_desc	*oc_gpiod;
+	struct gpio_descs	*vbus_en_gpiods;
 };
 
 #define USB_HSPHY_3P3_VOL_MIN		3050000 /* uV */
@@ -5872,6 +5874,14 @@
 		goto err;
 	}
 
+	mdwc->vbus_en_gpiods = devm_gpiod_get_array_optional(mdwc->dev,
+					"vbus-en", GPIOD_OUT_LOW);
+	if (IS_ERR(mdwc->vbus_en_gpiods)) {
+		ret = PTR_ERR(mdwc->vbus_en_gpiods);
+		dev_err(mdwc->dev, "Error %d extracting vbus-en gpios\n", ret);
+		goto err;
+	}
+
 	if (mdwc->oc_gpiod) {
 		oc_irq = gpiod_to_irq(mdwc->oc_gpiod);
 		if (oc_irq < 0) {
@@ -6467,6 +6477,15 @@
 
 static int vbus_regulator_toggle(struct dwc3_msm *mdwc, bool on)
 {
+	int i;
+
+	if (mdwc->vbus_en_gpiods) {
+		for (i = 0; i < mdwc->vbus_en_gpiods->ndescs; i++)
+			gpiod_set_value_cansleep(
+				mdwc->vbus_en_gpiods->desc[i], on);
+		dev_dbg(mdwc->dev, "vbus-en gpios %s\n", on ? "high" : "low");
+	}
+
 	if (!mdwc->vbus_reg)
 		return 0;
 
diff --git a/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt b/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt
index dfc14f7..79a19a8 100755
--- a/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt
+++ b/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt
@@ -9,6 +9,11 @@
 Either one of id-gpio or vbus-gpio must be present. Both can be present as well.
 - id-gpio: gpio for USB ID pin. See gpio binding.
 - vbus-gpio: gpio for USB VBUS pin.
+- id-high-is-none: optional. For id-gpio only, ID high reports none instead
+  of USB gadget (ID low still reports USB-HOST). Use when ID selects host vs
+  a mux/Type-C handoff rather than classic OTG B-device.
+- hub-reset-gpios: optional. Driven low when ID is low (host), high when ID
+  is high.
 
 Example: Examples of extcon-usb-gpio node in dra7-evm.dts as listed below:
 	extcon_usb1 {
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi
index ab4ebe1..0897664 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi
@@ -77,6 +77,33 @@
 	status = "ok";
 };
 
+/* [项目代号]: PM4125/PM2250 GPIO3 = USB_ID (U900F E9), via extcon-usb-gpio */
+&pm2250_gpios {
+	usb_id {
+		usb_id_default: usb_id_default {
+			pins = "gpio3";
+			function = "normal";
+			input-enable;
+			bias-pull-up;
+			/* 1.7V ID level needs 1.8V VIN; vin-0 (VPH) makes 1.7V read as low */
+			power-source = <1>;
+		};
+	};
+};
+
+&soc {
+	extcon_usb_id: extcon_usb_id {
+		compatible = "linux,extcon-usb-gpio";
+		id-gpio = <&pm2250_gpios 3 GPIO_ACTIVE_HIGH>;
+		/* DIP ON/D2: ID high → Type-C (none); DIP OFF/D1: ID low → host */
+		id-high-is-none;
+		/* GPIO108 HUB_RESET: ID low/host → low; ID high/type-c → high */
+		hub-reset-gpios = <&tlmm 108 GPIO_ACTIVE_HIGH>;
+		pinctrl-names = "default";
+		pinctrl-0 = <&usb_id_default &hub_reset_default>;
+	};
+};
+
 &pm2250_charger {
 	interrupts-extended = <&tlmm 89 0>;
 	interrupt-names = "usb_id_irq";
@@ -183,8 +210,57 @@
 	extcon = <&pm2250_charger>;
 };
 
+&tlmm {
+	usba_pwr_en_default: usba_pwr_en_default {
+		mux {
+			pins = "gpio19";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio19";
+			drive-strength = <2>;
+			bias-disable;
+			output-low;
+		};
+	};
+
+	otg_5v_en_default: otg_5v_en_default {
+		mux {
+			pins = "gpio103";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio103";
+			drive-strength = <2>;
+			bias-disable;
+			output-low;
+		};
+	};
+
+	hub_reset_default: hub_reset_default {
+		mux {
+			pins = "gpio108";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio108";
+			drive-strength = <2>;
+			bias-disable;
+			output-high;
+		};
+	};
+};
+
 &usb0 {
-	extcon = <&qusb_phy0>, <&eud>;
+	extcon = <&qusb_phy0>, <&eud>, <&extcon_usb_id>;
+	/* GPIO19_USBA_POWER_EN + GPIO103_OTG_5V_POWER_EN: host=high, none/device=low */
+	vbus-en-gpios = <&tlmm 19 GPIO_ACTIVE_HIGH>,
+			<&tlmm 103 GPIO_ACTIVE_HIGH>;
+	pinctrl-names = "default";
+	pinctrl-0 = <&usba_pwr_en_default &otg_5v_en_default>;
 };
 
 &qupv3_se1_i2c {
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-thermal.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-thermal.dtsi
index 02ea4b1..232b5d2 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-thermal.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-thermal.dtsi
@@ -565,7 +565,9 @@
 		};
 	};
 
+	/* Disabled: PM2250 has no valid ADC GPIO for skin_therm (GPIO3=USB_ID, GPIO5 not in adc5 table) */
 	chg-skin-therm {
+		status = "disabled";
 		polling-delay-passive = <0>;
 		polling-delay = <0>;
 		thermal-sensors = <&pm2250_adc_tm_iio 4>;
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba.dtsi
index dab86fe..b8b56b9 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba.dtsi
@@ -2590,6 +2590,7 @@
 	#address-cells = <1>;
 	#size-cells = <0>;
 	pinctrl-names = "default";
+	/* GPIO5 Hi-Z only: PM2250 adc5-lite has no ADC5_GPIO5; GPIO3 is USB_ID */
 	pinctrl-0 = <&conn_therm_default &skin_therm_default>;
 
 	xo_therm {
@@ -2624,14 +2625,6 @@
 		qcom,pre-scaling = <1 1>;
 	};
 
-	skin_therm {
-		reg = <ADC5_GPIO3_100K_PU>;
-		label = "skin_therm";
-		qcom,ratiometric;
-		qcom,hw-settle-time = <200>;
-		qcom,pre-scaling = <1 1>;
-	};
-
 	conn_therm {
 		reg = <ADC5_GPIO4_100K_PU>;
 		label = "conn_therm";
@@ -2686,11 +2679,7 @@
 				io-channels = <&pm2250_vadc ADC5_AMUX_THM3_100K_PU>;
 			};
 
-			skin_therm {
-				reg = <4>;
-				io-channels = <&pm2250_vadc ADC5_GPIO3_100K_PU>;
-			};
-
+			/* reg 4 was skin_therm (ADC5_GPIO5 unsupported; GPIO3 is USB_ID) */
 			conn_therm {
 				reg = <5>;
 				io-channels = <&pm2250_vadc ADC5_GPIO4_100K_PU>;
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba_auto-pmic.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba_auto-pmic.dtsi
index 492c4e0..60d3d09 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba_auto-pmic.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba_auto-pmic.dtsi
@@ -54,14 +54,6 @@
 		qcom,pre-scaling = <1 1>;
 	};
 
-	skin_therm {
-		reg = <ADC5_GPIO3_100K_PU>;
-		label = "skin_therm";
-		qcom,ratiometric;
-		qcom,hw-settle-time = <200>;
-		qcom,pre-scaling = <1 1>;
-	};
-
 	conn_therm {
 		reg = <ADC5_GPIO4_100K_PU>;
 		label = "conn_therm";
@@ -110,11 +102,7 @@
 				io-channels = <&pm2250_vadc ADC5_AMUX_THM3_100K_PU>;
 			};
 
-			skin_therm {
-				reg = <4>;
-				io-channels = <&pm2250_vadc ADC5_GPIO3_100K_PU>;
-			};
-
+			/* reg 4 was skin_therm (ADC5_GPIO5 unsupported; GPIO3 is USB_ID) */
 			conn_therm {
 				reg = <5>;
 				io-channels = <&pm2250_vadc ADC5_GPIO4_100K_PU>;
```

## 补丁验证

✅ **可干净应用**（134 源码树父提交重建验证）

## 源码归档

已归档到 [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/|91.源码与补丁索引/]]：

| 目录 | 内容 |
|:---|:---|
| `kernel_driver/` | extcon-usb-gpio.c、dwc3-msm-core.c、bengal_GKI.config（合并后版本） |
| `dt_config/` | extcon-usb-gpio.txt、scuba-iot-idp.dtsi、scuba.dtsi、scuba_auto-pmic.dtsi、scuba-thermal.dtsi（合并后版本） |
| `patches/` | 196403.patch |
| `modified_history.md` | 修改历史摘要 |

## 引用文件索引

| 序号 | 文件 | 完整路径（源码树内） | 说明 |
|:---|:---|:---|:---|
| 1 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/extcon/extcon-usb-gpio.c\|extcon-usb-gpio.c]] | `kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c` | DIP ID 检测扩展（+27/-3） |
| 2 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/dwc3/dwc3-msm-core.c\|dwc3-msm-core.c]] | `kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c` | hubreset/VBUS 处理（+19/-0） |
| 3 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/extcon-usb-gpio.txt\|extcon-usb-gpio.txt]] | `.../devicetree/bindings/extcon/extcon-usb-gpio.txt` | binding 文档（+5/-0） |
| 4 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp.dtsi\|scuba-iot-idp.dtsi]] | `.../devicetree/qcom/scuba-iot-idp.dtsi` | extcon 节点 + pinctrl + usb0（+77/-1） |
| 5 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba.dtsi\|scuba.dtsi]] | `.../devicetree/qcom/scuba.dtsi` | 移除 skin_therm（+2/-13） |
| 6 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba_auto-pmic.dtsi\|scuba_auto-pmic.dtsi]] | `.../devicetree/qcom/scuba_auto-pmic.dtsi` | 移除 skin_therm（+1/-13） |
| 7 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-thermal.dtsi\|scuba-thermal.dtsi]] | `.../devicetree/qcom/scuba-thermal.dtsi` | chg-skin-therm 禁用（+2/-0） |
| 8 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/bengal_GKI.config\|bengal_GKI.config]] | `kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config` | CONFIG_EXTCON_USB_GPIO=m（+1/-0） |

---


---

## 补充：DIP Host/Type-C 隔离与编译修复（#196756 + #196543）

**Change**: #196756（[同事]）+ #196543（[同事]）| **项目**: LA.VENDOR.13.2.1 | **分支**: [项目代号] | **状态**: MERGED
**任务**: Task 120577（与 #196403 同一任务，合并归档）

### 需求/问题描述

#196403 实现 DIP ID host mux hubreset 后，量产验证发现两个问题：

1. **DIP Host 模式与 Type-C 模式互斥不足（#196756）**：USB-A（DIP Host）插入时，`qusb_phy0` 的 charger role extcon 仍会向 dwc3 发角色通知，与 `extcon_usb_id` 的 Host 角色冲突；HUB_RESET（GPIO108，CH334 RESET# 低有效）极性也与实际硬件相反（原实现 ID low→low 复位了 HUB，应为 ID low→high 释放 RESET#）。
2. **编译错误（#196543）**：#196403 引入的 `vbus-en-gpios` 数组在无该属性的 DT 上 `devm_gpiod_get_array_optional` 返回 -ENOENT 时被当作错误 goto err，导致编译/运行异常。

### 方案

**#196756（DIP Host/Type-C isolation + hub reset polarity）**：

1. `phy-msm-qusb.c`：新增 `qcom,role-mute-extcon` 机制：
   - `role_mute_edev` / `role_mute_nb` / `role_mute` 字段；
   - `qusb_phy_role_mute_setup()`：解析 phandle 并注册 EXTCON_USB_HOST notifier；
   - `qusb_phy_role_mute_apply()`：mute 时清 `EXTCON_USB/USB_HOST` 状态（抑制 qusb→dwc3 角色通知），unmute 时 `port_state=PORT_UNKNOWN` 重新跑角色检测；
   - `qusb_phy_role_mute_notifier()`：跟随 `extcon_usb_id` 的 HOST 状态切换 mute；
   - extcon 通知回调开头增加 `if (qphy->role_mute) return;` 拦截。
2. `extcon-usb-gpio.c`：HUB_RESET 极性修正：ID low（Host）→ high（释放 RESET#），ID high（Type-C）→ low（复位）；`GPIOD_OUT_HIGH` 改 `GPIOD_OUT_LOW` 默认；日志注释同步更新。
3. `extcon-usb-gpio.txt`（binding）：更新 hub-reset-gpios 说明（CH334-style RESET# 低有效）。
4. `scuba-iot-idp.dtsi`：`&qusb_phy0` 增加 `qcom,role-mute-extcon = <&extcon_usb_id>`；GPIO108 pinctrl 默认 `output-low`；注释更新。

**#196543（编译修复）**：

5. `dwc3-msm-core.c`：`devm_gpiod_get_array_optional` 改 `devm_gpiod_get_array`，-ENOENT 时置 NULL 继续（属性缺失是合法场景，不是错误）。

### 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/phy/phy-msm-qusb.c\|kernel_driver/drivers/usb/phy/phy-msm-qusb.c]] | +98/-0 | role-mute 机制（#196756） |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/extcon/extcon-usb-gpio.c\|kernel_driver/drivers/extcon/extcon-usb-gpio.c]] | +6/-5 | HUB_RESET 极性修正（#196756） |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp.dtsi\|dt_config/scuba-iot-idp.dtsi]] | +11/-2 | role-mute-extcon 挂接 + GPIO108 默认低（#196756） |
| [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/dwc3/dwc3-msm-core.c\|kernel_driver/drivers/usb/dwc3/dwc3-msm-core.c]] | +7/-3 | vbus-en-gpios -ENOENT 容忍（#196543） |

### 配置方式

```dts
/* scuba-iot-idp.dtsi */
&qusb_phy0 {
	extcon = <&pm2250_charger>;
	qcom,role-mute-extcon = <&extcon_usb_id>;  /* DIP Host 时抑制 qusb role */
};

/* GPIO108 pinctrl：默认低（RESET# 断言直到 ID 检测） */
hub_reset_default {
	pins = "gpio108";
	drive-strength = <2>;
	bias-disable;
	output-low;
};
```

### 验证方式

```bash
# 1. DIP Host 模式插 USB-A：确认 qusb role 被 mute（日志 "USB role mute ON"）
dmesg | grep -i "role mute"
# 2. HUB 枚举正常（CH334 RESET# 释放）
lsusb | grep -i hub
# 3. Type-C 模式：mute 清除且角色从 charger 重新同步
dmesg | grep -i "role mute OFF"
# 4. 编译：无 vbus-en 属性的 DT 不再报 -ENOENT 错误
```

**预期**：DIP Host 与 Type-C 互斥隔离，HUB 复位极性正确，编译通过。
**实际**：#196756/#196543 已 MERGED；134 上 `git apply --check --reverse` 确认补丁已在分支 HEAD 中（ALREADY-APPLIED）。

### 补丁内容

### 补丁 1/2：#196756（DIP Host/Type-C isolation）

```diff
From fdb313f8a4c9d03eff5b441b828d3ba870e8ef05 Mon Sep 17 00:00:00 2001

[Solution]Add qusb role-mute on extcon_usb_id HOST to suppress qusb_phy0
role extcon on USB-A; resync on Type-C. GPIO108 hub reset default low,
high on USB-A to release CH334 RESET#.

---

diff --git a/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c b/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c
index 8ec9d5d..3b1fa26 100644
--- a/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c
+++ b/kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c
@@ -35,7 +35,7 @@
 	struct delayed_work wq_detcable;
 	/* ID-only: ID high = none (not gadget). Used by mux/DIP to Type-C. */
 	bool id_high_is_none;
-	/* DIP host (ID low) → drive low; DIP type-c (ID high) → drive high */
+	/* DIP host (ID low) → drive high (release RESET#); Type-C → drive low */
 	struct gpio_desc *hub_reset_gpiod;
 };
 
@@ -93,9 +93,9 @@
 			extcon_set_state_sync(info->edev, EXTCON_USB, true);
 	}
 
-	/* ID low (host/D1): hub reset low; ID high (type-c/D2): hub reset high */
+	/* DIP host (ID low): release RESET# (high); Type-C (ID high): assert (low) */
 	if (info->hub_reset_gpiod)
-		gpiod_set_value_cansleep(info->hub_reset_gpiod, !!id);
+		gpiod_set_value_cansleep(info->hub_reset_gpiod, !id);
 }
 
 static irqreturn_t usb_irq_handler(int irq, void *dev_id)
@@ -139,11 +139,12 @@
 		return PTR_ERR(info->vbus_gpiod);
 
 	info->hub_reset_gpiod = devm_gpiod_get_optional(&pdev->dev, "hub-reset",
-							GPIOD_OUT_HIGH);
+							GPIOD_OUT_LOW);
 	if (IS_ERR(info->hub_reset_gpiod))
 		return PTR_ERR(info->hub_reset_gpiod);
 	if (info->hub_reset_gpiod)
-		dev_info(dev, "hub-reset: ID low=low, ID high=high\n");
+		dev_info(dev,
+			 "hub-reset: ID low(host)=high(release), ID high=low\n");
 
 	info->id_high_is_none = of_property_read_bool(np, "id-high-is-none");
 	if (info->id_high_is_none)
diff --git a/kernel_platform/msm-kernel/drivers/usb/phy/phy-msm-qusb.c b/kernel_platform/msm-kernel/drivers/usb/phy/phy-msm-qusb.c
index 675e33e..4bf66fb 100644
--- a/kernel_platform/msm-kernel/drivers/usb/phy/phy-msm-qusb.c
+++ b/kernel_platform/msm-kernel/drivers/usb/phy/phy-msm-qusb.c
@@ -186,6 +186,15 @@
 	enum port_state		port_state;
 	unsigned int		dcd_timeout;
 
+	/*
+	 * DIP Host / Type-C isolation (qcom,role-mute-extcon):
+	 * when mute edev reports EXTCON_USB_HOST, suppress qusb→dwc3
+	 * role notifications so only the DIP extcon drives Host.
+	 */
+	struct extcon_dev	*role_mute_edev;
+	struct notifier_block	role_mute_nb;
+	bool			role_mute;
+
 	/* debugfs entries */
 	struct dentry		*root;
 	u8			tune1;
@@ -1105,6 +1114,14 @@
 	dev_dbg(qphy->phy.dev, "Notify event: %d for extcon_id: %d\n",
 					event, extcon_id);
 
+	/* USB-A / DIP host: do not publish charger/qusb role to dwc3 */
+	if (qphy->role_mute) {
+		dev_dbg(qphy->phy.dev,
+			"role mute: skip extcon_id=%d event=%d\n",
+			extcon_id, event);
+		return;
+	}
+
 	if (event) {
 		ret = extcon_get_property(edev, extcon_id,
 					EXTCON_PROP_USB_TYPEC_POLARITY, &val);
@@ -1126,6 +1143,84 @@
 	extcon_set_state_sync(qphy->usb_extcon, extcon_id, event);
 }
 
+static void qusb_phy_role_mute_resync(struct qusb_phy *qphy)
+{
+	if (qphy->role_mute || !qphy->usb_extcon)
+		return;
+
+	/* Re-run port SM so Type-C path re-announces role from charger */
+	qphy->port_state = PORT_UNKNOWN;
+	queue_delayed_work(system_freezable_wq, &qphy->port_det_w, 0);
+}
+
+static void qusb_phy_role_mute_apply(struct qusb_phy *qphy, bool mute)
+{
+	if (mute == qphy->role_mute)
+		return;
+
+	qphy->role_mute = mute;
+	if (mute) {
+		if (qphy->usb_extcon) {
+			extcon_set_state_sync(qphy->usb_extcon,
+					      EXTCON_USB, false);
+			extcon_set_state_sync(qphy->usb_extcon,
+					      EXTCON_USB_HOST, false);
+		}
+		dev_info(qphy->phy.dev,
+			 "USB role mute ON (DIP host): qusb extcon suppressed\n");
+	} else {
+		dev_info(qphy->phy.dev,
+			 "USB role mute OFF (Type-C): resync from charger\n");
+		qusb_phy_role_mute_resync(qphy);
+	}
+}
+
+static int qusb_phy_role_mute_notifier(struct notifier_block *nb,
+				       unsigned long event, void *ptr)
+{
+	struct qusb_phy *qphy = container_of(nb, struct qusb_phy, role_mute_nb);
+	bool mute;
+
+	if (!qphy->role_mute_edev)
+		return NOTIFY_DONE;
+
+	mute = !!extcon_get_state(qphy->role_mute_edev, EXTCON_USB_HOST);
+	qusb_phy_role_mute_apply(qphy, mute);
+
+	return NOTIFY_DONE;
+}
+
+static int qusb_phy_role_mute_setup(struct qusb_phy *qphy)
+{
+	struct device *dev = qphy->phy.dev;
+	struct device_node *np;
+	struct extcon_dev *edev;
+	int ret;
+
+	np = of_parse_phandle(dev->of_node, "qcom,role-mute-extcon", 0);
+	if (!np)
+		return 0;
+
+	edev = extcon_find_edev_by_node(np);
+	of_node_put(np);
+	if (IS_ERR_OR_NULL(edev))
+		return edev ? PTR_ERR(edev) : -EPROBE_DEFER;
+
+	qphy->role_mute_edev = edev;
+	qphy->role_mute_nb.notifier_call = qusb_phy_role_mute_notifier;
+	ret = devm_extcon_register_notifier(dev, edev, EXTCON_USB_HOST,
+					    &qphy->role_mute_nb);
+	if (ret) {
+		dev_err(dev, "role-mute extcon notifier failed: %d\n", ret);
+		return ret;
+	}
+
+	qusb_phy_role_mute_apply(qphy,
+		!!extcon_get_state(edev, EXTCON_USB_HOST));
+	dev_info(dev, "role-mute-extcon bound (DIP Host/Type-C isolation)\n");
+	return 0;
+}
+
 static bool qusb_phy_chg_det_status(struct qusb_phy *qphy,
 						enum chg_det_state state)
 {
@@ -1683,6 +1778,9 @@
 		if (ret)
 			return ret;
 
+		ret = qusb_phy_role_mute_setup(qphy);
+		if (ret)
+			return ret;
 	}
 
 	ret = usb_add_phy_dev(&qphy->phy);
diff --git a/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt b/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt
index 79a19a8..3f065b2 100755
--- a/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt
+++ b/kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt
@@ -12,8 +12,8 @@
 - id-high-is-none: optional. For id-gpio only, ID high reports none instead
   of USB gadget (ID low still reports USB-HOST). Use when ID selects host vs
   a mux/Type-C handoff rather than classic OTG B-device.
-- hub-reset-gpios: optional. Driven low when ID is low (host), high when ID
-  is high.
+- hub-reset-gpios: optional. CH334-style RESET# (active-low): driven high
+  (released) when ID is low (host), driven low (held in reset) when ID is high.
 
 Example: Examples of extcon-usb-gpio node in dra7-evm.dts as listed below:
 	extcon_usb1 {
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi
index 0897664..8e85682 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi
@@ -97,7 +97,10 @@
 		id-gpio = <&pm2250_gpios 3 GPIO_ACTIVE_HIGH>;
 		/* DIP ON/D2: ID high → Type-C (none); DIP OFF/D1: ID low → host */
 		id-high-is-none;
-		/* GPIO108 HUB_RESET: ID low/host → low; ID high/type-c → high */
+		/*
+		 * GPIO108 HUB_RESET (CH334 RESET# active-low, 1.8↔3.3 shifter):
+		 * default/Type-C/ID high → low; USB-A/ID low → high (release)
+		 */
 		hub-reset-gpios = <&tlmm 108 GPIO_ACTIVE_HIGH>;
 		pinctrl-names = "default";
 		pinctrl-0 = <&usb_id_default &hub_reset_default>;
@@ -208,6 +211,11 @@
 
 &qusb_phy0 {
 	extcon = <&pm2250_charger>;
+	/*
+	 * [项目代号] DIP Host/Type-C isolation: USB-A (extcon_usb_id HOST)
+	 * suppresses qusb charger role extcon; Type-C clears mute + resync.
+	 */
+	qcom,role-mute-extcon = <&extcon_usb_id>;
 };
 
 &tlmm {
@@ -249,7 +257,8 @@
 			pins = "gpio108";
 			drive-strength = <2>;
 			bias-disable;
-			output-high;
+			/* default low (Type-C / RESET# asserted until ID detect) */
+			output-low;
 		};
 	};
 };
```

### 补丁 2/2：#196543（Compilation error）

```diff
From 8cb7b23068aeb28611631db781e2994a5f9d7350 Mon Sep 17 00:00:00 2001

---

diff --git a/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c b/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c
index bab869e..04549d1 100644
--- a/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c
+++ b/kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c
@@ -5874,12 +5874,16 @@
 		goto err;
 	}
 
-	mdwc->vbus_en_gpiods = devm_gpiod_get_array_optional(mdwc->dev,
+	mdwc->vbus_en_gpiods = devm_gpiod_get_array(mdwc->dev,
 					"vbus-en", GPIOD_OUT_LOW);
 	if (IS_ERR(mdwc->vbus_en_gpiods)) {
 		ret = PTR_ERR(mdwc->vbus_en_gpiods);
-		dev_err(mdwc->dev, "Error %d extracting vbus-en gpios\n", ret);
-		goto err;
+		if (ret == -ENOENT) {
+			mdwc->vbus_en_gpiods = NULL;
+		} else {
+			dev_err(mdwc->dev, "Error %d extracting vbus-en gpios\n", ret);
+			goto err;
+		}
 	}
 
 	if (mdwc->oc_gpiod) {
```

### 补丁验证

| Change | 验证方式 | 结果 |
|--------|---------|------|
| #196756 | 134 [项目代号] LA.VENDOR.13.2.1 `git apply --check --reverse` | ✅ 已在分支 HEAD（MERGED） |
| #196543 | 同上 | ✅ 已在分支 HEAD（MERGED） |

### 源码归档

- [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/phy/phy-msm-qusb.c|phy-msm-qusb.c]]（134 拉取，最终版）
- [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/extcon/extcon-usb-gpio.c|extcon-usb-gpio.c]]（134 拉取，最终版）
- [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/dwc3/dwc3-msm-core.c|dwc3-msm-core.c]]（134 拉取，最终版）
- [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp.dtsi|scuba-iot-idp.dtsi]]（134 拉取，最终版）
- 补丁：[[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/196756.patch|196756.patch]]、[[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/196543.patch|196543.patch]]

### 引用文件索引（#196756/#196543 补充）

| 序号 | 文件 | 说明 |
|------|------|------|
| 1 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/phy/phy-msm-qusb.c\|phy-msm-qusb.c]] | qusb PHY role-mute（#196756） |
| 2 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/extcon/extcon-usb-gpio.c\|extcon-usb-gpio.c]] | HUB_RESET 极性（#196756） |
| 3 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/drivers/usb/dwc3/dwc3-msm-core.c\|dwc3-msm-core.c]] | vbus-en -ENOENT 容忍（#196543） |
| 4 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp.dtsi\|scuba-iot-idp.dtsi]] | DTS 配置（#196756） |
| 5 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/196756.patch\|196756.patch]] | #196756 补丁 |
| 6 | [[01.驱动文档/USB/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/196543.patch\|196543.patch]] | #196543 补丁 |


_Author: wangguanran_