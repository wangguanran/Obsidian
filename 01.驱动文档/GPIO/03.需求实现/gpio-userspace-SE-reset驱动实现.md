# ProjectC gpio-userspace 驱动实现（SE reset / MDB reset）

> **版本号：v1.0**

## 基本信息

| 字段 | 值 |
|:---|:---|
| 文档类型 | 需求实现 |
| 项目 | ProjectC（bengal_515_32go） |
| 驱动模块 | `kernel_platform/msm-kernel/drivers/misc/gpio-userspace.c` |
| 芯片 | Qualcomm bengal（scuba-iot） |
| 报告日期 | 2026-08-12 |

## 需求描述

Secure Element（SE）和 MDB 通信需要系统开机后保持固定的 GPIO 复位状态：

- **SE_RESET（GPIO102）**：Secure Element NRST，active-low，默认高电平
- **MDB_RESET（GPIO36）**：STM32F103 NRST，active-low，默认高电平

需要将这些 GPIO 导出到用户空间，使上层应用（SE 服务、MDB 服务）能在运行时控制复位时序。

## 方案

新增 `gpio-userspace` 内核驱动，通过 Device Tree 配置 label/gpios/default-state 后，自动在 `/sys/class/gpio_userspace/<label>/value` 创建 sysfs 节点，用户空间通过读写 value 文件控制 GPIO 电平。

### 驱动架构

```
gpio-userspace.ko
  └─ /sys/class/gpio_userspace/
       ├── se_reset/value     ← GPIO102 SE_RESET (default-high)
       └── mdb_reset/value    ← GPIO36  MDB_RESET  (default-high)
```

## 修改文件清单

| 文件 | 修改类型 | 说明 |
|:---|:---|:---|
| `[[#gpio-userspace.c]]` | 新增 | 272 行 gpio-userspace 驱动源码 |
| `[[#Kconfig]]` | 新增配置 | `CONFIG_GPIO_USERSPACE` 定义 |
| `[[#Makefile]]` | 新增编译项 | `obj-$(CONFIG_GPIO_USERSPACE) += gpio-userspace.o` |
| `[[#bengal_GKI.config]]` | 新增配置项 | `CONFIG_GPIO_USERSPACE=m` |
| `[[#scuba-iot-idp-overlay.dts]]` | 新增 DT | GPIO102/GPIO36 pinctrl + gpio-userspace 子节点 |
| `[[#init.target.rc]]` | 新增权限 | `chown system system` + `chmod 0666` |

### gpio-userspace.c 驱动核心

驱动通过解析 DT 中的 `label`、`gpios`、`default-state` 属性，自动创建 sysfs 节点：

```c
// DT 子节点示例
se-reset {
    label = "se_reset";
    gpios = <&tlmm 102 GPIO_ACTIVE_HIGH>;
    default-state = <1>;  // 0-low, 1-high, 2-input
};
```

- 支持 `default-state = 0`（输出低）、`1`（输出高）、`2`（输入）
- 导出 `value` 读写（sysfs）和 `direction` 写（切换 IN/OUT）
- 提供 `get_userspacegpio_value()` 内核符号供其他模块调用
- 支持 `-EPROBE_DEFER` 重试机制（最多 8 次，每次 500ms）

### DT 配置

```dts
&tlmm {
    [项目代号]_se_reset: [项目代号]_se_reset {
        mux { pins = "gpio102"; function = "gpio"; };
        config { pins = "gpio102"; drive-strength = <2>;
                bias-disable; output-high; };
    };
    [项目代号]_mdb_reset: [项目代号]_mdb_reset {
        mux { pins = "gpio36"; function = "gpio"; };
        config { pins = "gpio36"; drive-strength = <2>;
                bias-disable; output-high; };
    };
};

&soc {
    gpio-userspace {
        compatible = "gpio-userspace";
        status = "okay";
        pinctrl-names = "default";
        pinctrl-0 = <&[项目代号]_se_reset &[项目代号]_mdb_reset>;

        se-reset { label = "se_reset"; gpios = <&tlmm 102 GPIO_ACTIVE_HIGH>; default-state = <1>; };
        mdb-reset { label = "mdb_reset"; gpios = <&tlmm 36 GPIO_ACTIVE_HIGH>; default-state = <1>; };
    };
};
```

### init.target.rc 权限

```rc
# ProjectC gpio-userspace
chown system system /sys/class/gpio_userspace/se_reset/value
chmod 0666 /sys/class/gpio_userspace/se_reset/value
chown system system /sys/class/gpio_userspace/mdb_reset/value
chmod 0666 /sys/class/gpio_userspace/mdb_reset/value
```

## 编译验证

### 编译命令

```bash
cd <ProjectC 源码根目录>
JOBS=32 TMPDIR=<临时目录> ./build_boot_vendorboot_dtbo.sh
```

### 预期产物

- `out/target/product/bengal_515_32go/boot.img`
- `out/target/product/bengal_515_32go/vendor_boot.img`
- `out/target/product/bengal_515_32go/dtbo.img`
- `gpio-userspace.ko`（consolidate dist 输出）

## 结论

通过新增 `gpio-userspace` 内核驱动，将 SE_RESET（GPIO102）和 MDB_RESET（GPIO36）导出到用户空间 sysfs 节点，满足上层服务控制复位时序的需求。驱动支持 DT 配置 label/gpios/default-state，编译通过，冒烟验证通过。

## 补丁内容

```diff
---
 device/qcom/bengal_515_32go/init.target.rc    |   7 +
 .../arm64/configs/vendor/bengal_GKI.config    |   1 +
 .../msm-kernel/drivers/misc/Kconfig           |  10 +
 .../msm-kernel/drivers/misc/Makefile          |   1 +
 .../msm-kernel/drivers/misc/gpio-userspace.c  | 272 ++++++++++++++++++
 .../devicetree/qcom/scuba-iot-idp-overlay.dts |  60 ++++
 6 files changed, 351 insertions(+)
 create mode 100644 kernel_platform/msm-kernel/drivers/misc/gpio-userspace.c

diff --git a/device/qcom/bengal_515_32go/init.target.rc b/device/qcom/bengal_515_32go/init.target.rc
index 352b84557fc..e8feddd4183 100644
--- a/device/qcom/bengal_515_32go/init.target.rc
+++ b/device/qcom/bengal_515_32go/init.target.rc
@@ -89,6 +89,13 @@ on boot
     setprop vendor.usb.qdss.inst.name "qdss_sw"
     setprop vendor.usb.controller 4e00000.dwc3
 
+    # ProjectC gpio-userspace
+    chown system system /sys/class/gpio_userspace/se_reset/value
+    chmod 0666 /sys/class/gpio_userspace/se_reset/value
+    chown system system /sys/class/gpio_userspace/mdb_reset/value
+    chmod 0666 /sys/class/gpio_userspace/mdb_reset/value
+
+
 on boot && property:persist.vendor.usb.controller.default=*
     setprop vendor.usb.controller ${persist.vendor.usb.controller.default}
 
diff --git a/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config b/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
index b4badbb1a5e..72ad2d17dd6 100644
--- a/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
+++ b/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
@@ -105,6 +105,7 @@ CONFIG_POWER_RESET_MSM=m
 # CONFIG_POWER_RESET_QCOM_DOWNLOAD_MODE_DEFAULT is not set
 CONFIG_POWER_RESET_QCOM_PON=m
 CONFIG_PWM_QTI_LPG=m
+CONFIG_GPIO_USERSPACE=m
 CONFIG_QCOM_APCS_IPC=m
 CONFIG_QCOM_BALANCE_ANON_FILE_RECLAIM=y
 CONFIG_QCOM_BAM_DMA=m
diff --git a/kernel_platform/msm-kernel/drivers/misc/Kconfig b/kernel_platform/msm-kernel/drivers/misc/Kconfig
index b75b23a4dda..6ee9c50416b 100644
--- a/kernel_platform/msm-kernel/drivers/misc/Kconfig
+++ b/kernel_platform/msm-kernel/drivers/misc/Kconfig
@@ -527,6 +527,16 @@ config HDMI_INPUT_MUX
 
 	  If unsure, say N.
 
+config GPIO_USERSPACE
+	tristate "gpio user space,export gpio for userspace"
+	depends on OF && GPIOLIB
+	help
+	  Export named GPIOs to /sys/class/gpio_userspace/<label>/value
+	  for userspace control. Parse label/gpios/default-state from DT.
+
+	  To compile this driver as a module, choose M here: the
+	  module will be called gpio-userspace.
+
 source "drivers/misc/c2port/Kconfig"
 source "drivers/misc/eeprom/Kconfig"
 source "drivers/misc/cb710/Kconfig"
diff --git a/kernel_platform/msm-kernel/drivers/misc/Makefile b/kernel_platform/msm-kernel/drivers/misc/Makefile
index d7c5ed90a53..99f7f67f8db 100644
--- a/kernel_platform/msm-kernel/drivers/misc/Makefile
+++ b/kernel_platform/msm-kernel/drivers/misc/Makefile
@@ -66,3 +66,4 @@ obj-$(CONFIG_QPNP_MISC)		+= qpnp-misc.o
 obj-$(CONFIG_PROFILER)		+= profiler.o
 obj-$(CONFIG_QRC)		+= qrc/
 obj-$(CONFIG_HDMI_INPUT_MUX)	+= hdmi_input_mux.o
+obj-$(CONFIG_GPIO_USERSPACE)	+= gpio-userspace.o
diff --git a/kernel_platform/msm-kernel/drivers/misc/gpio-userspace.c b/kernel_platform/msm-kernel/drivers/misc/gpio-userspace.c
new file mode 100644
index 00000000000..c09e3c3e237
--- /dev/null
+++ b/kernel_platform/msm-kernel/drivers/misc/gpio-userspace.c
@@ -0,0 +1,272 @@
+// SPDX-License-Identifier: GPL-2.0-only
+/*
+ * Configure "label", "gpios" and "default-state" in dts.
+ * gpio-userspace.c automatically parses and creates the directory "label".
+ *
+ * for example:
+ * add the following code to dts, the directory /sys/class/gpio_userspace/rfid_pwr_en
+ * user can write "1" or "0" to "/sys/class/gpio_userspace/rfid_pwr_en/value" to set gpio high or low.
+ *
+ * &soc {
+ *      gpio-userspace {
+ *       status = "ok";
+ *       compatible = "gpio-userspace";
+ *       pinctrl-names = "default";
+ *       pinctrl-0 = <&gpio_userspace_default>;
+ *
+ *       rfid-pwr-en {
+ *           label = "rfid_pwr_en"; //"module" + "_" + "function" + "_" + "en"
+ *           gpios = <&tlmm 153 0>;
+ *           default-state = <0>; //0 -low, 1-high, 2-input
+ *       };
+ *
+ *     };
+ *  };
+ *
+ */
+
+#include <linux/module.h>
+#include <linux/err.h>
+#include <linux/init.h>
+#include <linux/platform_device.h>
+#include <linux/of.h>
+#include <linux/of_gpio.h>
+#include <linux/gpio.h>
+#include <linux/delay.h>
+
+#define EXPORT_NAME_SIZE 64
+#define DEFFER_COUNT_MAX 8
+static struct class *gpio_userspace_class;
+struct gpio_userspace {
+	int gpio;
+	int gpio_default_state;
+	struct device *dev;
+        char name[EXPORT_NAME_SIZE];
+};
+
+static struct gpio_userspace *gpio_userspace;
+static unsigned int gpio_userspace_count;
+
+int get_userspacegpio_value(char *str)
+{
+	struct gpio_userspace *userspace;
+	int i;
+	int value = 0;
+
+	for (i = 0; i < gpio_userspace_count; i++) {
+		userspace = &gpio_userspace[i];
+
+		if (strlen(userspace->name) == 0 || str == NULL)
+			return -1;
+		if (strcmp(userspace->name, str) == 0) {
+			value = gpio_get_value(userspace->gpio);
+            break;
+		}
+	}
+
+	return value;
+}
+EXPORT_SYMBOL(get_userspacegpio_value);
+
+static ssize_t value_show(struct device *dev,
+                                          struct device_attribute *attr, char *buf)
+{
+    struct gpio_userspace *userspace = dev_get_drvdata(dev);
+    int value;
+
+    value = gpio_get_value(userspace->gpio);
+
+    return scnprintf(buf, PAGE_SIZE, "%d\n", value);
+}
+
+static ssize_t value_store(struct device *dev,
+                                           struct device_attribute *attr,
+                                           const char *buf, size_t count)
+{
+    struct gpio_userspace *userspace = dev_get_drvdata(dev);
+    int value;
+
+    if (kstrtoint(buf, 10, &value)) {
+        return -EINVAL;
+    }
+
+    gpio_set_value(userspace->gpio, value);
+
+    return count;
+}
+
+static DEVICE_ATTR_RW(value);
+//<!- add by meiglink for task42361 20230830 begin
+static ssize_t direction_store(struct device *dev,
+                                           struct device_attribute *attr,
+                                           const char *buf, size_t count)
+{
+    struct gpio_userspace *userspace = dev_get_drvdata(dev);
+
+    if (strncmp(buf, "OUT", 3) == 0) {
+        gpio_direction_output(userspace->gpio, 0);
+    } else if (strncmp(buf, "IN", 2) == 0) {
+        gpio_direction_input(userspace->gpio);
+    } else {
+        pr_err("set gpio %s:direction err\n", userspace->name);
+    }
+
+    return count;
+}
+
+static DEVICE_ATTR_WO(direction);
+// add by meiglink for task42361 20230830 end ->
+static const struct of_device_id gpio_userspace_of_match[] = {
+    { .compatible = "gpio-userspace", },
+    {},
+};
+MODULE_DEVICE_TABLE(of, gpio_userspace_of_match);
+
+static int parse_gpio_dts(struct platform_device *pdev, struct device_node *np)
+{
+    struct device_node *child;
+    int ret = 0;
+    u32 flag;
+
+    if (pdev == NULL || np == NULL)
+        return -EINVAL;
+
+    for_each_child_of_node(np, child) {
+        struct gpio_userspace *userspace = &gpio_userspace[gpio_userspace_count];
+        const char *label;
+        int rc;
+
+        label = of_get_property(child, "label", NULL);
+        if (!label) {
+            dev_err(&pdev->dev, "child %pOF has no label property\n", child);
+            ret = -EINVAL;
+            goto out_put_child;
+        }
+        snprintf(userspace->name, EXPORT_NAME_SIZE, "%s",  label);
+
+        userspace->gpio = of_get_named_gpio_flags(child, "gpios", 0, &flag);
+        if (!gpio_is_valid(userspace->gpio)) {
+			pr_err("gpio %s: err\n", userspace->name);
+			ret = -EINVAL;
+            goto out_put_child;
+        }
+        ret = gpio_request(userspace->gpio, userspace->name);
+        if (ret < 0) {
+                pr_err("Request %s GPIO failed, ret = %d\n", userspace->name, ret);
+                goto out_put_child;
+        }
+        rc = of_property_read_u32(child,
+                                "default-state", &userspace->gpio_default_state);
+        if (rc < 0){
+                userspace->gpio_default_state = 0;
+        }
+
+		if (userspace->gpio_default_state == 2) {
+			gpio_direction_input(userspace->gpio);
+		} else {
+			gpio_direction_output(userspace->gpio, userspace->gpio_default_state);
+		}
+        userspace->dev = device_create_with_groups(gpio_userspace_class, &pdev->dev, 0,
+                        userspace, NULL, "%s", userspace->name);
+
+        if (IS_ERR(userspace->dev)) {
+                ret = PTR_ERR(userspace->dev);
+                dev_err(&pdev->dev, "device_create_with_groups failed ret %d\n", ret);
+                goto out_put_child;
+        }
+        dev_info(&pdev->dev, "added GPIO %s %d,default %d\n", userspace->name, userspace->gpio, userspace->gpio_default_state );
+        gpio_userspace_count++;
+        continue;
+
+        out_put_child:
+           of_node_put(child);
+           break;
+    }
+
+    return ret;
+}
+
+static int gpio_userspace_probe(struct platform_device *pdev)
+{
+	int ret = -1;
+	static int deffer_count = 0;
+	struct device_node *np = pdev->dev.of_node;
+
+	pr_err("%s probe start\n", __func__);
+	if (IS_ERR(gpio_userspace_class))
+		return PTR_ERR(gpio_userspace_class);
+
+	gpio_userspace = devm_kcalloc(&pdev->dev, of_get_child_count(np), sizeof(struct gpio_userspace), GFP_KERNEL);
+	if (!gpio_userspace)
+		return -ENOMEM;
+
+	ret = parse_gpio_dts(pdev, np);
+	if(ret && deffer_count < DEFFER_COUNT_MAX){
+		deffer_count++;
+		ret = -EPROBE_DEFER;
+		pr_err("%s probe need to deffer deffer_count=%d\n", __func__,deffer_count);
+		msleep(500);
+	}
+	return ret;
+}
+
+static int gpio_userspace_remove(struct platform_device *pdev)
+{
+    struct gpio_userspace *userspace;
+    int i;
+
+    for (i = 0; i < gpio_userspace_count; i++) {
+        userspace = &gpio_userspace[i];
+
+       // device_remove_file(&userspace->gpiod->gdev->dev, &dev_attr_value);
+    }
+
+    return 0;
+}
+
+static struct platform_driver gpio_userspace_driver = {
+    .driver = {
+        .name = "gpio-userspace",
+        .of_match_table = gpio_userspace_of_match,
+    },
+    .probe = gpio_userspace_probe,
+    .remove = gpio_userspace_remove,
+};
+
+static struct attribute *gpio_userspace_class_attrs[] = {
+	&dev_attr_value.attr,
+//<!- add by meiglink for task42361 20230830 begin
+	&dev_attr_direction.attr,
+// add by meiglink for task42361 20230830 end ->
+	NULL,
+};
+
+static const struct attribute_group gpio_userspace_group = {
+	.attrs = gpio_userspace_class_attrs,
+};
+
+static const struct attribute_group *gpio_userspace_groups[] = {
+	&gpio_userspace_group,
+	NULL,
+};
+static int __init gpio_userspace_init(void)
+{
+	pr_err("%s %d init\n", __func__, __LINE__);
+	gpio_userspace_class = class_create(THIS_MODULE, "gpio_userspace");
+	if (IS_ERR(gpio_userspace_class))
+		return PTR_ERR(gpio_userspace_class);
+
+	gpio_userspace_class->dev_groups = gpio_userspace_groups;
+	return platform_driver_register(&gpio_userspace_driver);
+}
+
+static void __exit gpio_userspace_exit(void)
+{
+	class_destroy(gpio_userspace_class);
+	platform_driver_unregister(&gpio_userspace_driver);
+}
+
+module_init(gpio_userspace_init);
+module_exit(gpio_userspace_exit);
+MODULE_AUTHOR("meiglink");
+MODULE_DESCRIPTION("Linux GPIO Userspace Driver");
+MODULE_LICENSE("GPL");
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
index 96adfa675a1..6c52b71a3ec 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
@@ -2,6 +2,7 @@
 /plugin/;
 
 #include <dt-bindings/interrupt-controller/arm-gic.h>
+#include <dt-bindings/gpio/gpio.h>
 #include "scuba-iot-idp.dtsi"
 
 / {
@@ -10,3 +11,62 @@
 	qcom,msm-id = <473 0x10000>, <474 0x10000>;
 	qcom,board-id = <34 0>;
 };
+
+/*
+ * ProjectC gpio-userspace exports
+ *   GPIO102 SE_RESET -> Secure Element (SE) NRST (active-low), default high
+ *   GPIO36  MDB_RESET  -> STM32F103 NRST (active-low), default high
+ *
+ * Sysfs: /sys/class/gpio_userspace/<label>/value
+ * default-state: 0-low, 1-high, 2-input
+ */
+&tlmm {
+	[项目代号]_se_reset: [项目代号]_se_reset {
+		mux {
+			pins = "gpio102";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio102";
+			drive-strength = <2>;
+			bias-disable;
+			output-high;
+		};
+	};
+
+	[项目代号]_mdb_reset: [项目代号]_mdb_reset {
+		mux {
+			pins = "gpio36";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio36";
+			drive-strength = <2>;
+			bias-disable;
+			output-high;
+		};
+	};
+};
+
+&soc {
+	gpio-userspace {
+		compatible = "gpio-userspace";
+		status = "okay";
+		pinctrl-names = "default";
+		pinctrl-0 = <&[项目代号]_se_reset &[项目代号]_mdb_reset>;
+
+		se-reset {
+			label = "se_reset";
+			gpios = <&tlmm 102 GPIO_ACTIVE_HIGH>;
+			default-state = <1>;
+		};
+
+		mdb-reset {
+			label = "mdb_reset";
+			gpios = <&tlmm 36 GPIO_ACTIVE_HIGH>;
+			default-state = <1>;
+		};
+	};
+};
-- 
2.34.1
```

## 引用文件索引

### gpio-userspace.c
路径：`kernel_platform/msm-kernel/drivers/misc/gpio-userspace.c`
新增 272 行 gpio-userspace 驱动源码。核心功能：解析 DT label/gpios/default-state，创建 `/sys/class/gpio_userspace/<label>/value` sysfs 节点。支持 `value` 读写、`direction` 切换、`get_userspacegpio_value()` 内核符号导出。

### Kconfig
路径：`kernel_platform/msm-kernel/drivers/misc/Kconfig`
新增 `CONFIG_GPIO_USERSPACE` 配置项，`tristate "gpio user space,export gpio for userspace"`，依赖 `OF && GPIOLIB`。

### Makefile
路径：`kernel_platform/msm-kernel/drivers/misc/Makefile`
新增 `obj-$(CONFIG_GPIO_USERSPACE) += gpio-userspace.o`。

### bengal_GKI.config
路径：`kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config`
新增 `CONFIG_GPIO_USERSPACE=m`。

### scuba-iot-idp-overlay.dts
路径：`kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts`
新增 pinctrl 节点（`[项目代号]_se_reset`、`[项目代号]_mdb_reset`）和 `gpio-userspace` 子节点（`se-reset` label=se_reset, gpios=102, default-high；`mdb-reset` label=mdb_reset, gpios=36, default-high）。

### init.target.rc
路径：`device/qcom/bengal_515_32go/init.target.rc`
新增 se_reset/mdb_reset value 文件权限：`chown system system` + `chmod 0666`。

_Author: 艾达_