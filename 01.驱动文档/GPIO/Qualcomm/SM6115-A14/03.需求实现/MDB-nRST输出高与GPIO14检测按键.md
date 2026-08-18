# MDB nRST 输出高与 GPIO14 检测按键

> **模块**: GPIO | **厂商**: Qualcomm | **芯片**: SM6115 (scuba)
> **平台**: SM6115-A14 (LA.VENDOR.13.2.1) | **类型**: 需求
> **Change**: #195886 | **作者**: wangguanran | **状态**: MERGED

---

## 基本信息

| Change | 项目 | 分支 | 作者 | 类型 | 芯片 | 平台 | 模块 |
|--------|------|------|------|------|------|------|------|
| #195886 | LA.VENDOR.13.2.1 | MT5205 | wangguanran | 需求 | SM6115 (scuba) | SM6115-A14 | GPIO |

## 需求描述

MDB 小板（STM32F103）nRST 复位脚在空闲状态下没有稳定输出高电平，导致 MDB 主控可能处于复位/不稳定状态；同时 GPIO14 原本被 pinctrl 保留（reserved），无法用作 DB 检测按键输入。

## 环境

- 芯片：SM6115 (scuba)
- 平台：SM6115-A14（LA.VENDOR.13.2.1，MT5205 分支）
- 设备：Scuba IOT IDP（overlay：scuba-iot-idp-overlay.dts）
- 相关任务：Task 120572
- 关联改动：#195883（UIC Pulse，同一 overlay 后续变更）、#195883 之前的 gpio-userspace 驱动（#195832 前后，SE_RESET/MDB_RESET default-high）

## 背景与设计

1. **MDB nRST 空闲电平**：MDB 小板上的 STM32F103 nRST 为低有效复位。主板上没有上拉电阻（no board pull-up on MB），若 pinctrl 不显式输出高电平，nRST 会浮空或处于不确定状态，MDB 主控可能反复复位。需要在 pinctrl 中将 GPIO36（mdb_reset）配置为 `output-high`，空闲时钳位为高电平。
2. **GPIO14 被保留**：`pinctrl-scuba.c` 的 `scuba_reserved_gpios[]` 中 GPIO14/15 与 0~3 一起被标记为 reserved（此前为 MDB SE5 UART 预留 16/17 时把 14/15 一并保留）。GPIO14 需要用于 DB 检测按键，必须从保留列表中解除。
3. **按键节点缺失**：DB 检测需要 gpio-keys 节点上报按键事件（KEY_F1）；scuba.dtsi 中已有的 gpio_keys 节点没有 phandle，无法直接引用，需要在 overlay 中单独新增 `mdb_db_keys` 节点。

## 方案

1. **pinctrl-scuba.c**（+2/-2）：`scuba_reserved_gpios[]` 从 `0, 1, 2, 3, 14, 15, -1` 改为 `0, 1, 2, 3, 15, -1`，解除 GPIO14 保留（GPIO15 保持保留）。
2. **scuba-iot-idp-overlay.dts**（+39/-2）：
   - 注释更新：GPIO36 MDB_RESET 说明改为 idle output-high；
   - `mt5205_mdb_reset` 节点增加注释"无板级上拉，空闲输出高"（配置已含 `output-high`）；
   - 新增 `mt5205_mdb_db_detect` pinctrl（GPIO14，drive-strength 2，bias-pull-up，input-enable）；
   - 新增 `mdb_db_keys` gpio-keys 节点：GPIO14 低有效（GPIO_ACTIVE_LOW），上报 `KEY_F1`，debounce-interval 15ms，支持 wakeup。

## 修改文件清单

| # | 文件 | 改动 | 说明 |
|---|------|------|------|
| 1 | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/pinctrl-scuba.c\|pinctrl-scuba.c]] | +2/-2 | scuba_reserved_gpios 解除 GPIO14 保留 |
| 2 | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp-overlay.dts\|scuba-iot-idp-overlay.dts]] | +39/-2 | MDB nRST 空闲输出高 + GPIO14 gpio-keys KEY_F1 节点 |

## 配置方式

### 1. pinctrl 保留列表（kernel）

```c
static const int scuba_reserved_gpios[] = {
    /* MT5205: 16/17 for MDB UART; 14 released for MDB DET; 15 stays reserved */
    0, 1, 2, 3, 15, -1
};
```

### 2. 设备树 overlay（scuba-iot-idp-overlay.dts）

```dts
&tlmm {
    /* MDB STM32F103 nRST: idle output-high (no board pull-up on MB) */
    mt5205_mdb_reset: mt5205_mdb_reset {
        mux {
            pins = "gpio36";
            function = "gpio";
        };
        config {
            pins = "gpio36";
            drive-strength = <2>;
            bias-disable;
            output-high;   /* 空闲输出高 */
        };
    };

    mt5205_mdb_db_detect: mt5205_mdb_db_detect {
        mux {
            pins = "gpio14";
            function = "gpio";
        };
        config {
            pins = "gpio14";
            drive-strength = <2>;
            bias-pull-up;
            input-enable;
        };
    };
};

&soc {
    /* Separate node: scuba.dtsi gpio_keys has no phandle */
    mdb_db_keys {
        compatible = "gpio-keys";
        label = "mdb-db-keys";
        pinctrl-names = "default";
        pinctrl-0 = <&mt5205_mdb_db_detect>;
        status = "okay";

        mdb_db {
            label = "mdb_db_detect";
            gpios = <&tlmm 14 GPIO_ACTIVE_LOW>;
            linux,input-type = <1>;
            linux,code = <KEY_F1>;
            debounce-interval = <15>;
            gpio-key,wakeup;
        };
    };
};
```

## 验证方式

1. 编译内核（pinctrl 改动）与 DT overlay，确认无 reserved GPIO 冲突：
   ```bash
   # pinctrl probe 阶段无 GPIO14 request 失败
   dmesg | grep -i "gpio14\|pinctrl"
   ```
2. MDB nRST 空闲电平：
   ```bash
   cat /sys/class/gpio_userspace/mdb_reset/value   # 预期 1（空闲高）
   echo 0 > /sys/class/gpio_userspace/mdb_reset/value   # 复位 MDB
   echo 1 > /sys/class/gpio_userspace/mdb_reset/value   # 释放复位
   ```
3. DB 检测按键：
   ```bash
   getevent -c 3   # 插入/拔出 DB 小板应上报 KEY_F1
   ```
   预期：GPIO14 被拉低时上报 `KEY_F1` 事件，且可唤醒系统。

## 结论

通过解除 GPIO14 的 reserved 状态并新增 gpio-keys 节点，实现了 DB 检测按键功能；MDB nRST 空闲输出高电平保证 STM32F103 稳定运行。改动仅涉及 pinctrl 保留列表与 overlay，无驱动代码变更，风险低。

## 补丁内容

```diff
Subject: [PATCH] [MT5205][TaskID]120572[Description]MDB nRST output-high and GPIO14 DB detect key[Solution]idle mdb_reset output-high, unreserve GPIO14, gpio-keys KEY_F1[Owner]wangguanran

---

diff --git a/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c b/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c
index 0e24381..1d4a3e5 100644
--- a/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c
+++ b/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c
@@ -1586,8 +1586,8 @@
 };
 
 static const int scuba_reserved_gpios[] = {
-		/* MT5205: release 16/17 for MDB SE5 UART (gpio14/15 stay reserved) */
-		0, 1, 2, 3, 14, 15, -1
+		/* MT5205: 16/17 for MDB UART; 14 released for MDB DET; 15 stays reserved */
+		0, 1, 2, 3, 15, -1
 };
 
 static const struct msm_gpio_wakeirq_map scuba_mpm_map[] = {
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
index 6c52b71..4b0d987 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
@@ -3,6 +3,7 @@
 
 #include <dt-bindings/interrupt-controller/arm-gic.h>
 #include <dt-bindings/gpio/gpio.h>
+#include <dt-bindings/input/input.h>
 #include "scuba-iot-idp.dtsi"
 
 / {
@@ -15,10 +16,13 @@
 /*
  * MT5205 gpio-userspace exports
  *   GPIO102 SE_RESET -> Secure Element (SE) NRST (active-low), default high
- *   GPIO36  MDB_RESET  -> STM32F103 NRST (active-low), default high
+ *   GPIO36  MDB_RESET -> STM32F103 NRST (active-low), idle output-high
  *
  * Sysfs: /sys/class/gpio_userspace/<label>/value
- * default-state: 0-low, 1-high, 2-input
+ * default-state: 0-low, 1-high, 2-input(high-Z)
+ * Assert nRST: echo 0 > value
+ *
+ * MT5205 MDB-DB detect: GPIO14 gpio-keys KEY_F1
  */
 &tlmm {
 	mt5205_se_reset: mt5205_se_reset {
@@ -35,6 +39,7 @@
 		};
 	};
 
+	/* MDB STM32F103 nRST: idle output-high (no board pull-up on MB) */
 	mt5205_mdb_reset: mt5205_mdb_reset {
 		mux {
 			pins = "gpio36";
@@ -48,6 +53,20 @@
 			output-high;
 		};
 	};
+
+	mt5205_mdb_db_detect: mt5205_mdb_db_detect {
+		mux {
+			pins = "gpio14";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio14";
+			drive-strength = <2>;
+			bias-pull-up;
+			input-enable;
+		};
+	};
 };
 
 &soc {
@@ -69,4 +88,22 @@
 			default-state = <1>;
 		};
 	};
+
+	/* Separate node: scuba.dtsi gpio_keys has no phandle */
+	mdb_db_keys {
+		compatible = "gpio-keys";
+		label = "mdb-db-keys";
+		pinctrl-names = "default";
+		pinctrl-0 = <&mt5205_mdb_db_detect>;
+		status = "okay";
+
+		mdb_db {
+			label = "mdb_db_detect";
+			gpios = <&tlmm 14 GPIO_ACTIVE_LOW>;
+			linux,input-type = <1>;
+			linux,code = <KEY_F1>;
+			debounce-interval = <15>;
+			gpio-key,wakeup;
+		};
+	};
 };

```

## 补丁验证

✅ 可干净应用（134 源码树父提交重建验证）。

## 源码归档

| 归档目录 | 文件 | 说明 |
|----------|------|------|
| kernel_driver/ | pinctrl-scuba.c | scuba pinctrl（合并后版本） |
| dt_config/ | scuba-iot-idp-overlay-195886.dts | overlay（Change #195886 时点版本，109 行） |
| dt_config/ | scuba-iot-idp-overlay.dts | overlay 最终版（#195883 归档，含 pulse + MDB 全部改动） |
| patches/ | 195886.patch | 本变更补丁 |

## 引用文件索引

| 文件 | 路径 | 说明 |
|------|------|------|
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/pinctrl-scuba.c\|pinctrl-scuba.c]] | `kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c` | scuba_reserved_gpios 解除 GPIO14（+2/-2） |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp-overlay-195886.dts\|scuba-iot-idp-overlay-195886.dts]] | `kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts` | #195886 时点版本（+39/-2） |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp-overlay.dts\|scuba-iot-idp-overlay.dts]] | `kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts` | overlay 最终版（含 #195883 pulse 改动） |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/195886.patch\|195886.patch]] | `patches/195886.patch` | 本变更补丁 |

---

_Author: wangguanran_
