# TypeC OTG 测试失败 - fusb302 extcon 修复

> **模块**: USB | **厂商**: Qualcomm | **芯片**: SM6225 (khaje)
> **平台**: SM6225-A16 (LA.VENDOR.13.2.1) | **类型**: Bug
> **Change**: #196214 | **作者**: zhangjinwei | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196214 |
| 项目 | LA.VENDOR.13.2.1 |
| 分支 | master_IOT_High_Mid_2024.SPF.3.0_SLM927x_SLM550x |
| 作者 | zhangjinwei |
| 类型 | Bug（TypeC OTG 测试失败） |
| 芯片 | Qualcomm SM6225 (khaje) |
| 平台 | SM6225-A16 (LA.VENDOR.13.2.1) |
| 项目 | [项目代号]_A16 |
| 模块 | USB (TypeC / OTG / fusb302 extcon) |
| 提交标题 | `[[项目代号]_A16][BugID]95891[Description] typec otg test failed[Solution]add fusb302 extcon [Owner]zhangjinwei` |
| BugID | 95891 |

## 现象

[项目代号]_A16 项目 **TypeC OTG 测试失败**：Type-C 口无法在 host/device 之间正确切换，插入 OTG 设备（U 盘/键鼠）时 USB 不枚举，OTG 功能不可用。

## 根因分析

硬件上 Type-C 口的 CC 检测由 **FUSB302** 芯片负责（I2C 0x22，中断 GPIO36），但设备树中：

1. **FUSB302 节点缺失**：`qupv3_se1_i2c` 下没有 `fairchild,fusb302` 节点，驱动无法 probe，CC 检测结果无法上报；
2. **usb0 extcon 未指向 fusb302**：`khaje-idp.dtsi` / `khaje-idp-nopmi.dtsi` 中 `&usb0` 没有 extcon 或挂错（nopmi 板甚至 `delete-property extcon` + `dr_mode = "peripheral"`，直接锁死 device 模式）；
3. **DWC3 未使能 role switch**：`usb0` 与 `dwc3@4e00000` 都缺 `usb-role-switch` 属性，`dr_mode` 为 peripheral，无法动态切换 host/device；
4. **OTG VBUS 无来源**：nopmi 板（无 PMIC 升压）需要 GPIO boost（GPIO108）提供 OTG 5V，但未配置 `qcom,otg-vbus-gpio`。

## 处理方案

1. **新增 FUSB302 节点**（khaje-idp.dtsi，+27）：

   ```dts
   &qupv3_se1_i2c {
       fusb302: typec-portc@22 {
           compatible = "fairchild,fusb302";
           reg = <0x22>;
           vdda18-supply = <&pm6125_l9>;
           vdda33-supply = <&pm6125_l15>;
           pinctrl-names = "default";
           pinctrl-0 = <&fusb302_default>;
           int-n-gpios = <&tlmm 36 0x00>;
           status = "okay";
       };
   };
   ```

2. **usb0 挂载 fusb302 extcon + 使能 role switch**（khaje-idp.dtsi / khaje-idp-nopmi.dtsi）：

   ```dts
   &usb0 {
       usb-role-switch;
       ///delete-property/ extcon;      /* 原 nopmi 删除逻辑注释掉 */
       qcom,otg-vbus-gpio = <&tlmm 108 0>;   /* nopmi: GPIO boost 供 OTG VBUS */
       extcon = <&fusb302>;             /* extcon 指向 FUSB302 */
       dwc3@4e00000 {
           usb-role-switch;
           dr_mode = "otg";             /* peripheral → otg */
       };
   };
   ```

3. **khaje-usb.dtsi** 中 usb0 补 `extcon = <&eud>`（+2），保留 eud 调试路径。

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/USB/Qualcomm/SM6225-A16/91.源码与补丁索引/dt_config/khaje-idp.dtsi\|khaje-idp.dtsi]] | +27/-0 | 新增 fusb302 节点（I2C 0x22、INT GPIO36）；usb0 挂 extcon=fusb302、usb-role-switch、otg-vbus-gpio |
| [[01.驱动文档/USB/Qualcomm/SM6225-A16/91.源码与补丁索引/dt_config/khaje-idp-nopmi.dtsi\|khaje-idp-nopmi.dtsi]] | +7/-2 | 恢复 usb0 extcon；dr_mode peripheral→otg；otg-vbus-gpio |
| [[01.驱动文档/USB/Qualcomm/SM6225-A16/91.源码与补丁索引/dt_config/khaje-usb.dtsi\|khaje-usb.dtsi]] | +2/-0 | usb0 增加 `extcon = <&eud>` |

## 配置方式

### 内核配置

FUSB302 驱动为内核既有驱动（`drivers/usb/typec/fusb302/fusb302.c`），需确保以下配置已使能（本补丁未改动 config，依赖平台既有配置）：

```
CONFIG_TYPEC=y
CONFIG_TYPEC_FUSB302=y
CONFIG_EXTCON=y
```

### OTG 链路（机制）

```
OTG 设备插入 Type-C 口
   │ CC 上拉电阻检测 (Rd/Rp)
   ▼
FUSB302 检测 CC → INT (GPIO36) 中断
   ▼
fusb302 驱动解析 role → 作为 extcon provider 上报
   ▼
dwc3 (usb-role-switch) 收到 EXTCON_USB_HOST → 切 host
   └─ qcom,otg-vbus-gpio (GPIO108) boost 输出 5V
   ▼
OTG 设备枚举成功
```

## 验证方式

| 项目 | 内容 |
|------|------|
| 编译 | ✅ 编译 PASS（dtbo 编译通过） |
| OTG 功能 | Type-C 插入 U 盘/键鼠 → `lsusb` 可见设备，`dmesg` 有 fusb302 role 切换日志 |
| host→device | 拔出 OTG 设备，插入 PC → device 模式（gadget 枚举） |
| VBUS | OTG 模式下 GPIO108 输出 5V |

## 结论

通过补齐 FUSB302 Type-C 控制器节点、将 usb0 extcon 指向 fusb302、使能 usb-role-switch 并把 dr_mode 改为 otg，修复 [项目代号]_A16 TypeC OTG 测试失败问题。补丁可在由 REST API 重建的父提交文件内容上干净应用。

## 补丁内容

```diff
Subject: [PATCH] [[项目代号]_A16][BugID]95891[Description] typec otg test failed[Solution]add fusb302 extcon [Owner]zhangjinwei

---
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-nopmi.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-nopmi.dtsi
index c2f9a63..608481d 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-nopmi.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-nopmi.dtsi
@@ -11,8 +11,13 @@
 };
 
 &usb0 {
-	/delete-property/ extcon;
+	usb-role-switch;
+	///delete-property/ extcon;
+	/* nopmi: use GPIO boost for OTG VBUS (same as bengal-usb.dtsi) */
+	qcom,otg-vbus-gpio = <&tlmm 108 0>;
+	extcon = <&fusb302>;
 	dwc3@4e00000 {
-		dr_mode = "peripheral";
+		usb-role-switch;
+		dr_mode = "otg";
 	};
 };
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp.dtsi
index 678f7ea..fe3536e 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp.dtsi
@@ -147,6 +147,21 @@
 };
 
 &qupv3_se1_i2c {
+	fusb302: typec-portc@22 {
+		 compatible = "fairchild,fusb302";
+		 reg = <0x22>;
+		 vdda18-supply = <&pm6125_l9>;
+		 vdda33-supply = <&pm6125_l15>;
+		 //aux_switch_vdda33-supply = <&pm7325_l2>;
+		 //aux-en-gpio = <&tlmm 59 0x00>;
+		 //aux-sel-gpio= <&tlmm 56 0x00>;
+		 pinctrl-names = "default";
+		pinctrl-0 = <&fusb302_default>;
+		 int-n-gpios = <&tlmm 36 0x00>;
+		 //vbus-5v-gpios = <&tlmm 118 0x00>;
+		 status = "okay";
+	 };
+
 	awinic@64 {
 		compatible = "awinic,aw2016_led";
 		reg = <0x64>;
@@ -309,3 +324,15 @@
 	};
 
 };
+
+&usb0 {
+	usb-role-switch;
+	///delete-property/ extcon;
+	/* nopmi: use GPIO boost for OTG VBUS (same as bengal-usb.dtsi) */
+	qcom,otg-vbus-gpio = <&tlmm 108 0>;
+	extcon = <&fusb302>;
+	dwc3@4e00000 {
+		usb-role-switch;
+		dr_mode = "otg";
+	};
+};
\ No newline at end of file
diff --git a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-usb.dtsi b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-usb.dtsi
index e658b75..44edbfd 100755
--- a/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-usb.dtsi
+++ b/kernel_platform/qcom/proprietary/devicetree/qcom/khaje-usb.dtsi
@@ -60,6 +60,8 @@
 				<0 2400>,
 				<0 40000>;
 
+		extcon = <&eud>;
+
 		dwc3@4e00000 {
 			compatible = "snps,dwc3";
 			reg = <0x4e00000 0xe000>;
```

## 补丁验证

✅ **可干净应用**（REST API 父提交文件内容重建验证）

## 源码归档

已归档到 [[01.驱动文档/USB/Qualcomm/SM6225-A16/91.源码与补丁索引/|91.源码与补丁索引/]]：

| 目录 | 内容 |
|:---|:---|
| `dt_config/` | khaje-idp.dtsi、khaje-idp-nopmi.dtsi、khaje-usb.dtsi（合并后版本） |
| `patches/` | 196214.patch |
| `modified_history.md` | 修改历史摘要 |

## 引用文件索引

| 序号 | 文件 | 完整路径（源码树内） | 说明 |
|:---|:---|:---|:---|
| 1 | [[01.驱动文档/USB/Qualcomm/SM6225-A16/91.源码与补丁索引/dt_config/khaje-idp.dtsi\|khaje-idp.dtsi]] | `kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp.dtsi` | fusb302 节点 + usb0 extcon/role-switch（+27/-0） |
| 2 | [[01.驱动文档/USB/Qualcomm/SM6225-A16/91.源码与补丁索引/dt_config/khaje-idp-nopmi.dtsi\|khaje-idp-nopmi.dtsi]] | `kernel_platform/qcom/proprietary/devicetree/qcom/khaje-idp-nopmi.dtsi` | usb0 恢复 extcon、otg、role-switch（+7/-2） |
| 3 | [[01.驱动文档/USB/Qualcomm/SM6225-A16/91.源码与补丁索引/dt_config/khaje-usb.dtsi\|khaje-usb.dtsi]] | `kernel_platform/qcom/proprietary/devicetree/qcom/khaje-usb.dtsi` | usb0 补 eud extcon（+2/-0） |

---

_Author: wangguanran_