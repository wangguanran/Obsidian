# SE SPI0 spidev 与 SE 握手 GPIO 使能

> **模块**: GPIO | **厂商**: Qualcomm | **芯片**: SM6115 (scuba)
> **平台**: SM6115-A14 (LA.VENDOR.13.2.1) | **类型**: 需求
> **Change**: #195885 + #197063（Task 118732 合并 2 个 Change） | **作者**: wangguanran | **状态**: MERGED

---

## 基本信息

| Change | 项目 | 分支 | 作者 | 类型 | 芯片 | 平台 | 模块 |
|--------|------|------|------|------|------|------|------|
| #195885 | LA.VENDOR.13.2.1 | MT5205 | wangguanran | 需求 | SM6115 (scuba) | SM6115-A14 | GPIO/SPI |
| #197063 | iot-high-mid-2024-spf-1-0_amss_standard_oem | master_meig | wangguanran | 需求 | SM6115 (scuba) | SM6115-A14 | TZ QUP 访问控制 |

## 需求描述

Secure MCU（STM32U585）需要经 QUP SE0 SPI（10MHz）与 AP 通信，用于业务数据交互。当前存在三重阻碍：

1. **GPIO0~3 被 pinctrl 保留**：`pinctrl-scuba.c` 的 `scuba_reserved_gpios[]` 把 GPIO0~3 标记为 reserved，SE0 SPI（QUPv3 SE0 默认映射 GPIO0~3）无法被内核正常使用；
2. **TZ 默认持有 SE0**：TZ 侧 QUP 访问控制（QUPAC）默认把 SE0 配成其他用途/AC_TZ，HLOS 侧即使 DT 使能也无法真正访问硬件；
3. **缺少握手 GPIO 导出**：SE_ACK（GPIO37）/SE_RDY（GPIO63）需要导出到 gpio_userspace，便于主机控制/监控 Secure MCU 握手时序。

## 环境

- 芯片：SM6115 (scuba)
- 平台：SM6115-A14（LA.VENDOR.13.2.1，MT5205 分支）
- 设备：Scuba IOT IDP（overlay：scuba-iot-idp-overlay.dts）
- Secure MCU：STM32U585（SE_RESET = GPIO102，见已有归档）
- 相关任务：Task 118732（SE SPI 通信链路）
- 关联改动：#195883（UIC Pulse，同一 overlay 的其他变更）

## 方案

### AP 侧（#195885，kernel + DT）

1. **bengal_GKI.config**（+1）：使能 `CONFIG_SPI_SPIDEV=m`，提供 spidev 用户态接口；
2. **pinctrl-scuba.c**（+2/-2）：`scuba_reserved_gpios[]` 从 `0, 1, 2, 3, 15, -1` 改为 `15, -1`，释放 GPIO0~3 给 SE0 SPI；
3. **scuba-iot-idp-overlay.dts**（+70）：
   - 新增 `mt5205_se_ack`（GPIO37，output-low，bias-disable）与 `mt5205_se_rdy`（GPIO63，input-enable，bias-disable）pinctrl 节点；
   - gpio_userspace 节点新增 `se-ack`（default-state 0）与 `se-rdy`（default-state 2）子节点；
   - `&qupv3_se0_spi` 使能：`spi-max-frequency = <10000000>`、`qcom,disable-autosuspend`，新增 `spidev@0`（compatible `qcom,spi-msm-codec-slave`，spidev 内建 dummy）；
   - 禁用冲突的 `&qupv3_se0_i2c` 与 `&qupv3_se0_4uart`。

### TZ 侧（#197063，AMSS/TrustZone QUP 访问控制）

新增 `QUPAC_Access_MT5205.c`（产品级 QUPAC overlay，`ODM_PROJECT_MT5205` 条件编译）：

- **SE0**：`QUPV3_PROTOCOL_SPI` + `QUPV3_MODE_FIFO` + `AC_HLOS`（bAllowFifo=TRUE）→ HLOS 获得 SE0 SPI 访问权；
- **SE1**：本平台配成 `UART_4W`（GPIO69/70 RS232），替代默认 I2C；
- **SE5**：本平台配成 `UART_4W`（GPIO16/17 MDB UART），替代默认 SPI Fingerprint（AC_TZ）；
- 其余 SE2（Touch I2C GSI）、SE3（BT HCI UART）、SE4（Debug UART）保持默认；
- 沿用平台既有 `qupv3_perms_rumi / _QRB / _QRB_V2 / _Genoa / _2W / _2W_SKU2` 等备选表。

## 修改文件清单

| # | 文件 | 改动 | 说明 |
|---|------|------|------|
| 1 | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/bengal_GKI.config\|bengal_GKI.config]] | +1 | 使能 CONFIG_SPI_SPIDEV=m |
| 2 | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/pinctrl-scuba.c\|pinctrl-scuba.c]] | +2/-2 | scuba_reserved_gpios 释放 GPIO0-3 |
| 3 | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp-overlay-195885.dts\|scuba-iot-idp-overlay-195885.dts]] | +70 | SE0 SPI spidev + se_ack/se_rdy 握手 GPIO |
| 4 | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/TZ.XF.5.1/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access_MT5205.c\|QUPAC_Access_MT5205.c]] | +215（新增） | TZ 释放 SE0 SPI 给 HLOS（AC_HLOS） |

## 配置方式

### 1. Kernel config（bengal_GKI.config）

```text
CONFIG_SPI_SPIDEV=m
```

### 2. pinctrl 保留列表（pinctrl-scuba.c）

```c
static const int scuba_reserved_gpios[] = {
    /* MT5205: 0-3 released for SE0 SPI; 16/17 MDB UART; 14 MDB DET; 15 stays reserved */
    15, -1
};
```

### 3. DT overlay（scuba-iot-idp-overlay.dts）

```dts
&qupv3_se0_spi {
    status = "ok";
    spi-max-frequency = <10000000>;
    qcom,disable-autosuspend;

    spidev@0 {
        compatible = "qcom,spi-msm-codec-slave";   /* spidev 内建 dummy compatible */
        reg = <0>;
        spi-max-frequency = <10000000>;
    };
};

&qupv3_se0_i2c  { status = "disabled"; };
&qupv3_se0_4uart { status = "disabled"; };
```

gpio_userspace 握手节点：

```dts
se-ack { label = "se_ack"; gpios = <&tlmm 37 GPIO_ACTIVE_HIGH>; default-state = <0>; };
se-rdy { label = "se_rdy"; gpios = <&tlmm 63 GPIO_ACTIVE_HIGH>; default-state = <2>; };
```

### 4. TZ QUPAC（AMSS 侧）

- 使能 `ODM_PROJECT_MT5205` 宏后编译 TrustZone 镜像（QUPAC_Access_MT5205.c 生效）；
- SE0 权限行：`{ QUPV3_0_SE0, QUPV3_PROTOCOL_SPI, QUPV3_MODE_FIFO, AC_HLOS, TRUE, TRUE, TRUE }`。

## 验证方式

1. 刷机（fastboot）：`flash.bat` 刷 boot/vendor_boot/dtbo 均 OKAY（ISSUE-2026-0819-001 验证记录）；
2. spidev 节点：
   ```bash
   ls /dev/spidev*                     # 预期出现 spidev0.0
   cat /sys/class/spi_master/spi0/device/spidev0.0/of_node/compatible
   ```
3. 握手 GPIO 导出：
   ```bash
   cat /sys/class/gpio_userspace/se_ack/value   # 预期 0（AC 输出低）
   cat /sys/class/gpio_userspace/se_rdy/value   # 预期输入态
   ```
4. SE SPI 通信：Secure MCU 侧发起 SPI 读写，AP 侧用 spidev_test 验证数据通路；
5. gerrit 评审静态检查通过（runtime 验证在后续迭代中完成——本单 merge 后仍有 overlay 微调，见后续归档记录）。

> 注：本需求 merge 后，实测确认 GPIO102 才是 SE MCU 复位（se_reset），se_ack/se_rdy 的 pinctrl 与 userspace 节点后续已从 overlay 移除（另行提交），最终保留 SE0 SPI + se_reset 通路。

## 补丁内容

### 补丁 1/2：#195885（AP 侧 kernel + DT）

```diff
Subject: [PATCH] [MT5205][TaskID]118732[Description]enable SE SPI0 spidev and
 handshake GPIOs [Solution]overlay SE0 10MHz dummy spidev, unreserve GPIO0-3,
 export GPIO37/63/102 [Owner]wangguanran

---
 .../arm64/configs/vendor/bengal_GKI.config    |  1 +
 .../drivers/pinctrl/qcom/pinctrl-scuba.c      |  4 +-
 .../devicetree/qcom/scuba-iot-idp-overlay.dts | 70 ++++++++++++++++++-
 3 files changed, 72 insertions(+), 3 deletions(-)

diff --git a/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config b/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
index 3cd796d3b63..97c815be97d 100644
--- a/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
+++ b/kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config
@@ -273,6 +273,7 @@ CONFIG_SM_GPUCC_SCUBA=m
 CONFIG_SM_LPASS_AUDIOCC_6115=m
 CONFIG_SND_USB_AUDIO_QMI=m
 CONFIG_SPI_MSM_GENI=m
+CONFIG_SPI_SPIDEV=m
 CONFIG_SPMI_MSM_PMIC_ARB=m
 CONFIG_SPS=m
 CONFIG_SPS_SUPPORT_NDP_BAM=y
diff --git a/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c b/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c
index 1d4a3e5d7cf..28d2a3a7d06 100644
--- a/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c
+++ b/kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c
@@ -1586,8 +1586,8 @@ static const struct msm_pingroup scuba_groups[] = {
 };
 
 static const int scuba_reserved_gpios[] = {
-		/* MT5205: 16/17 for MDB UART; 14 released for MDB DET; 15 stays reserved */
-		0, 1, 2, 3, 15, -1
+		/* MT5205: 0-3 released for SE0 SPI; 16/17 MDB UART; 14 MDB DET; 15 stays reserved */
+		15, -1
 };
 
 static const struct msm_gpio_wakeirq_map scuba_mpm_map[] = {
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
index 3d1654f1658..e985688633f 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts
@@ -16,6 +16,8 @@
 /*
  * MT5205 gpio-userspace exports
  *   GPIO102 SE_RESET -> Secure Element (SE) NRST (active-low), default high
+ *   GPIO37  se_ack   -> Secure MCU ACK (output, bias-disable)
+ *   GPIO63  se_rdy   -> Secure MCU RDY (input, bias-disable)
  *   GPIO36  MDB_RESET -> STM32F103 NRST (active-low), idle output-high
  *
  * Sysfs: /sys/class/gpio_userspace/<label>/value
@@ -66,6 +68,36 @@
 		};
 	};
 
+	/* Secure MCU ACK to SP */
+	mt5205_se_ack: mt5205_se_ack {
+		mux {
+			pins = "gpio37";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio37";
+			drive-strength = <2>;
+			bias-disable;
+			output-low;
+		};
+	};
+
+	/* Secure MCU RDY from SP */
+	mt5205_se_rdy: mt5205_se_rdy {
+		mux {
+			pins = "gpio63";
+			function = "gpio";
+		};
+
+		config {
+			pins = "gpio63";
+			drive-strength = <2>;
+			bias-disable;
+			input-enable;
+		};
+	};
+
 	/* MDB STM32F103 nRST: idle output-high (no board pull-up on MB) */
 	mt5205_mdb_reset: mt5205_mdb_reset {
 		mux {
@@ -120,7 +152,8 @@
 		compatible = "gpio-userspace";
 		status = "okay";
 		pinctrl-names = "default";
-		pinctrl-0 = <&mt5205_se_reset &mt5205_mdb_reset>;
+		pinctrl-0 = <&mt5205_se_reset &mt5205_se_ack &mt5205_se_rdy
+			      &mt5205_mdb_reset>;
 
 		se-reset {
 			label = "se_reset";
@@ -128,6 +161,18 @@
 			default-state = <1>;
 		};
 
+		se-ack {
+			label = "se_ack";
+			gpios = <&tlmm 37 GPIO_ACTIVE_HIGH>;
+			default-state = <0>;
+		};
+
+		se-rdy {
+			label = "se_rdy";
+			gpios = <&tlmm 63 GPIO_ACTIVE_HIGH>;
+			default-state = <2>;
+		};
+
 		mdb-reset {
 			label = "mdb_reset";
 			gpios = <&tlmm 36 GPIO_ACTIVE_HIGH>;
@@ -153,3 +198,26 @@
 		};
 	};
 };
+
+/* MT5205 Secure MCU STM32U585: SE0 SPI0 GPIO0~3 @ 10MHz, no runtime sleep.
+ * spidev@0 uses qcom,spi-msm-codec-slave dummy compatible from drivers/spi/spidev.c.
+ */
+&qupv3_se0_spi {
+	status = "ok";
+	spi-max-frequency = <10000000>;
+	qcom,disable-autosuspend;
+
+	spidev@0 {
+		compatible = "qcom,spi-msm-codec-slave";
+		reg = <0>;
+		spi-max-frequency = <10000000>;
+	};
+};
+
+&qupv3_se0_i2c {
+	status = "disabled";
+};
+
+&qupv3_se0_4uart {
+	status = "disabled";
+};
-- 
2.34.1


```

### 补丁 2/2：#197063（TZ QUPAC）

```diff
Subject: [PATCH] [MT5205][TaskID]118732[Description]release QUP SE0 SPI to
 HLOS for Secure MCU[Solution]add QUPAC_Access_MT5205 product overlay with SE0
 FIFO AC_HLOS[Owner]wangguanran

---
 .../qupv3/config/agatti/QUPAC_Access_MT5205.c | 215 ++++++++++++++++++
 1 file changed, 215 insertions(+)
 create mode 100644 TZ.XF.5.1/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access_MT5205.c

diff --git a/TZ.XF.5.1/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access_MT5205.c b/TZ.XF.5.1/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access_MT5205.c
new file mode 100644
index 0000000000..9e5567066a
--- /dev/null
+++ b/TZ.XF.5.1/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access_MT5205.c
@@ -0,0 +1,215 @@
+//===========================================================================
+//
+// FILE:         QUPAC_Access.xml
+//
+// DESCRIPTION:  This file lists access permission for all QUPS
+//
+//===========================================================================
+//
+//                             Edit History
+//
+// $Header: //components/rel/core.tz/2.1/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access.c#6 $
+//
+// when       who     what, where, why
+// 10/09/24   ABH     Updated settings for 2W SKU2
+// 03/14/23   PCR     Updated settings for 2W 
+// 03/02/23   RK      Added AC settings for RB1-V2
+// 03/17/22   RK      Added AC settings for Genoa
+// 11/22/21   RK      Added AC settings for RB1
+// 08/01/19   PCR     Created
+//
+//===========================================================================
+//             Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
+//             All rights reserved.
+//             Confidential and Proprietary - Qualcomm Technologies, Inc.
+//===========================================================================
+#include "QupACCommonIds.h"
+#include "odm_features.h"
+
+/* OEMs are expected to modify this .c to suit their board design. The uAC 
+   specifies the owners of the SE resource. It is initially populated
+   according to System IO GPIO allocation */
+
+//All SEs have to be listed below. Any SE not present cannot be accessed by any subsystem. 
+//It's designed to be flexible enough to list only available SEs on a particular platform.
+
+const QUPv3_se_security_permissions_type qupv3_perms_default[] =
+{
+  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  TRUE  }, // NFC eSE
+/* <!--! modify this to set gpio69 70 as 4 wire uart start */
+#if defined(ODM_PROJECT_MT5205)
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // RS232 UART GPIO69/70
+/* modify this to set gpio69 70 as 4 wire uart stop --> */
+#else
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
+#endif
+  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
+  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
+  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
+#if defined(ODM_PROJECT_MT5205)
+  { QUPV3_0_SE5, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // MDB UART GPIO16/17
+#else
+  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_TZ,          FALSE,      TRUE,  TRUE  }, // Fingerprint
+#endif
+};
+
+const uint32 qupv3_perms_size_default = sizeof(qupv3_perms_default)/sizeof(qupv3_perms_default[0]);
+
+const QUPv3_se_security_permissions_type ssc_qupv3_perms_default[] =
+{
+  /*PeriphID,      ProtocolID,             Mode,           NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_SSC_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  { QUPV3_SSC_SE1, QUPV3_PROTOCOL_I3C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  /*QUPV3_SSC_SE2*/
+  /*QUPV3_SSC_SE3*/
+  /*QUPV3_SSC_SE4*/
+  { QUPV3_SSC_SE5, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  { QUPV3_SSC_SE6, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  /*QUPV3_SSC_SE7*/
+};
+const uint32 ssc_qupv3_perms_size_default = sizeof(ssc_qupv3_perms_default)/sizeof(ssc_qupv3_perms_default[0]);
+
+const QUPv3_se_security_permissions_type qupv3_perms_rumi[] =
+{
+  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_FIFO,  AC_TZ,          TRUE,       TRUE,  TRUE  }, // NFC eSE
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
+  /*QUPV3_0_SE2*/
+  /*QUPV3_0_SE3*/
+  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        FALSE,      FALSE, FALSE }, // Debug UART
+  /*QUPV3_0_SE5*/
+};
+const uint32 qupv3_perms_size_rumi = sizeof(qupv3_perms_rumi)/sizeof(qupv3_perms_rumi[0]);
+
+const QUPv3_se_security_permissions_type qupv3_perms_default_QRB[] =
+{
+  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_0_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // NFC eSE
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
+  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
+  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
+  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
+  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Fingerprint
+};
+
+const uint32 qupv3_perms_size_default_QRB = sizeof(qupv3_perms_default_QRB)/sizeof(qupv3_perms_default_QRB[0]);
+
+const QUPv3_se_security_permissions_type qupv3_perms_default_QRB_V2[] =
+{
+  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_0_SE0, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // Diag UART
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
+  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
+  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
+  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
+  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Fingerprint
+};
+
+const uint32 qupv3_perms_size_default_QRB_V2 = sizeof(qupv3_perms_default_QRB_V2)/sizeof(qupv3_perms_default_QRB_V2[0]);
+
+const QUPv3_se_security_permissions_type qupv3_perms_default_Genoa[] =
+{
+  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_0_SE0, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // Diag UART
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
+  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
+  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
+  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
+  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_TZ,          FALSE,      TRUE,  TRUE  }, // Fingerprint
+};
+
+const uint32 qupv3_perms_size_default_Genoa = sizeof(qupv3_perms_default_Genoa)/sizeof(qupv3_perms_default_Genoa[0]);
+
+const QUPv3_se_security_permissions_type qupv3_perms_2W[] =
+{
+  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_TZ,          FALSE,      TRUE,  TRUE  }, // NFC eSE
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // SMB/NFC/EEPROM/PM8008
+  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
+  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
+  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
+  { QUPV3_0_SE5, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  TRUE  }, // Fingerprint
+};
+
+const uint32 qupv3_perms_size_2W = sizeof(qupv3_perms_2W)/sizeof(qupv3_perms_2W[0]);
+
+const QUPv3_se_security_permissions_type ssc_qupv3_perms_2W[] =
+{
+  /*PeriphID,      ProtocolID,             Mode,           NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_SSC_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  { QUPV3_SSC_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  /*QUPV3_SSC_SE2*/
+  /*QUPV3_SSC_SE3*/
+  /*QUPV3_SSC_SE4*/
+  { QUPV3_SSC_SE5, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  { QUPV3_SSC_SE6, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  /*QUPV3_SSC_SE7*/
+};
+
+const uint32 ssc_qupv3_perms_size_2W = sizeof(ssc_qupv3_perms_2W)/sizeof(ssc_qupv3_perms_2W[0]);
+
+const QUPv3_se_security_permissions_type qupv3_perms_2W_SKU2[] =
+{
+  /*PeriphID,    ProtocolID,             Mode,             NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_0_SE0, QUPV3_PROTOCOL_SPI,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // LVDS & CODEC
+  { QUPV3_0_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // PMIC & CODEC
+  { QUPV3_0_SE2, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI,   AC_HLOS,        FALSE,      TRUE,  FALSE }, // Legacy Touch
+  { QUPV3_0_SE3, QUPV3_PROTOCOL_UART_4W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // BT HCI
+  { QUPV3_0_SE4, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       FALSE, FALSE }, // Debug UART
+  { QUPV3_0_SE5, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_FIFO,  AC_HLOS,        TRUE,       TRUE,  FALSE }, // 4Wire UART
+};
+
+const uint32 qupv3_perms_size_2W_SKU2 = sizeof(qupv3_perms_2W_SKU2)/sizeof(qupv3_perms_2W_SKU2[0]);
+
+const QUPv3_se_security_permissions_type ssc_qupv3_perms_2W_SKU2[] =
+{
+  /*PeriphID,      ProtocolID,             Mode,           NsOwner,        bAllowFifo, bLoad, bModExcl  */
+  { QUPV3_SSC_SE0, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  { QUPV3_SSC_SE1, QUPV3_PROTOCOL_I2C,     QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  /*QUPV3_SSC_SE2*/
+  /*QUPV3_SSC_SE3*/
+  /*QUPV3_SSC_SE4*/
+  { QUPV3_SSC_SE5, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  { QUPV3_SSC_SE6, QUPV3_PROTOCOL_UART_2W, QUPV3_MODE_GSI, AC_ADSP_Q6_ELF, FALSE,      FALSE, FALSE },
+  /*QUPV3_SSC_SE7*/
+};
+
+const uint32 ssc_qupv3_perms_size_2W_SKU2 = sizeof(ssc_qupv3_perms_2W_SKU2)/sizeof(ssc_qupv3_perms_2W_SKU2[0]);
+
+const QUPv3_gpii_security_permissions_type qupv3_gpii_perms[] =
+{
+  { QUPV3_0_GPII0,  AC_HLOS, AC_HLOS_GSI },
+  { QUPV3_0_GPII1,  AC_HLOS, AC_HLOS_GSI },
+  { QUPV3_0_GPII2,  AC_HLOS, AC_HLOS_GSI },
+  { QUPV3_0_GPII3,  AC_HLOS, AC_HLOS_GSI },
+  { QUPV3_0_GPII4,  AC_TZ },
+  { QUPV3_0_GPII5,  AC_TZ },
+  { QUPV3_0_GPII6,  AC_TZ },
+  { QUPV3_0_GPII7,  AC_ADSP_Q6_ELF },
+  { QUPV3_0_GPII8,  AC_ADSP_Q6_ELF },
+  { QUPV3_0_GPII9,  AC_MSS_MSA },  
+};
+const uint32 qupv3_gpii_perms_size = sizeof(qupv3_gpii_perms)/sizeof(qupv3_gpii_perms[0]);
+
+const QUPv3_gpii_security_permissions_type ssc_qupv3_gpii_perms[] =
+{
+  { QUPV3_SSC_GPII0,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII1,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII2,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII3,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII4,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII5,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII6,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII7,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII8,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII9,  AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII10, AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII11, AC_ADSP_Q6_ELF },
+  { QUPV3_SSC_GPII12, AC_ADSP_Q6_ELF }, 
+  { QUPV3_SSC_GPII13, AC_ADSP_Q6_ELF }, 
+  { QUPV3_SSC_GPII14, AC_ADSP_Q6_ELF }, 
+  { QUPV3_SSC_GPII15, AC_ADSP_Q6_ELF }, 
+};
+const uint32 ssc_qupv3_gpii_perms_size = sizeof(ssc_qupv3_gpii_perms)/sizeof(ssc_qupv3_gpii_perms[0]);
+
-- 
2.34.1


```

## 补丁验证

| 补丁 | 验证方式 | 结果 |
|------|----------|------|
| #195885 | 134 源码树父提交文件提取 + `patch --dry-run` | ✅ 可干净应用 |
| #197063 | 134 AMSS 树工作区 `git apply --reverse --check` | ✅ 可干净应用 |

## 源码归档

| 归档目录 | 文件 | 说明 |
|----------|------|------|
| kernel_driver/ | bengal_GKI.config | 合并后版本（含 SPI_SPIDEV） |
| kernel_driver/ | pinctrl-scuba.c | 合并后版本（GPIO0-3 已释放） |
| dt_config/ | scuba-iot-idp-overlay-195885.dts | #195885 时点 overlay 快照 |
| kernel_driver/ | TZ.XF.5.1/.../QUPAC_Access_MT5205.c | TZ QUPAC 新增文件（保留原相对路径） |
| patches/ | 195885.patch、197063.patch | 本任务全部补丁 |

## 引用文件索引

| 文件 | 路径 | 说明 |
|------|------|------|
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/bengal_GKI.config\|bengal_GKI.config]] | `kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config` | CONFIG_SPI_SPIDEV=m |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/pinctrl-scuba.c\|pinctrl-scuba.c]] | `kernel_platform/msm-kernel/drivers/pinctrl/qcom/pinctrl-scuba.c` | reserved 列表释放 GPIO0-3 |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp-overlay-195885.dts\|scuba-iot-idp-overlay-195885.dts]] | `kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts` | #195885 时点版本（spidev + 握手 GPIO） |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/TZ.XF.5.1/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/agatti/QUPAC_Access_MT5205.c\|QUPAC_Access_MT5205.c]] | `TZ.XF.5.1/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/agatti/` | SE0 SPI AC_HLOS 释放 |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/195885.patch\|195885.patch]] | Gerrit #195885 | AP 侧补丁 |
| [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/197063.patch\|197063.patch]] | Gerrit #197063 | TZ QUPAC 补丁 |

---

_Author: wangguanran_
