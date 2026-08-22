# SM6225-A16 (khaje) [项目代号] USB 平台移植资料

> **版本号：v1.0**

## 平台版本

| 项目 | 内容 |
|------|------|
| SoC | Qualcomm SM6225 (khaje) |
| Android | A16 (LA.VENDOR.13.2.1 / master_IOT_High_Mid_2024.SPF.3.0_SLM927x_SLM550x) |
| 内核 | msm-kernel 5.10 |
| 项目 | [项目代号]_A16 |

## 硬件接口

| 信号 | 引脚/地址 | 说明 |
|:---|:---|:---|
| FUSB302 I2C | 0x22 (qupv3_se1_i2c) | Type-C 控制器，CC 检测 |
| FUSB302 INT | TLMM GPIO36 | 中断（int-n-gpios，低有效） |
| FUSB302 1.8V | pm6125_l9 | vdda18-supply |
| FUSB302 3.3V | pm6125_l15 | vdda33-supply |
| OTG VBUS | TLMM GPIO108 | GPIO boost 输出 5V（nopmi 板） |
| USB D+/D- | — | qusb2 phy (usb2_phy0) + eud |

## 驱动索引

| 驱动/文件 | 路径（源码树内） | 说明 |
|:---|:---|:---|
| fusb302 | `kernel_platform/msm-kernel/drivers/usb/typec/fusb302/fusb302.c` | FUSB302 Type-C 控制器驱动（内核既有） |
| dwc3 | `kernel_platform/msm-kernel/drivers/usb/dwc3/` | USB 控制器，usb-role-switch |
| khaje-idp.dtsi | `.../devicetree/qcom/khaje-idp.dtsi` | fusb302 节点 + usb0 配置 |
| khaje-idp-nopmi.dtsi | `.../devicetree/qcom/khaje-idp-nopmi.dtsi` | nopmi 板 usb0 配置 |
| khaje-usb.dtsi | `.../devicetree/qcom/khaje-usb.dtsi` | 通用 usb0/eud 配置 |

## DTS 配置要点

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

&usb0 {
	usb-role-switch;
	qcom,otg-vbus-gpio = <&tlmm 108 0>;
	extcon = <&fusb302>;
	dwc3@4e00000 {
		usb-role-switch;
		dr_mode = "otg";
	};
};
```

注意：`fusb302_default` pinctrl 节点需在对应 pinctrl dtsi（如 khaje-pinctrl.dtsi）中定义 INT GPIO36 的配置，本补丁未包含。

## 内核配置

```
CONFIG_TYPEC=y
CONFIG_TYPEC_FUSB302=y
CONFIG_EXTCON=y
CONFIG_USB_DWC3=y
```

## 编译命令

```bash
# 134 源码树 (LA.VENDOR.13.2.1 / SLM927x 分支)
cd <tree>/kernel_platform
# 编译 dtbo（DTS 改动验证）
make dtboimage
# 或全量编译
./build.sh
```

## 相关文档

- [[01.驱动文档/USB/Qualcomm/SM6225-A16/04.问题案例/TypeC-OTG测试失败-fusb302-extcon修复.md|TypeC-OTG测试失败-fusb302-extcon修复]]
- [[01.驱动文档/USB/Qualcomm/SM6225-A16/01.原理与架构/TypeC-OTG驱动架构分析.md|TypeC-OTG 驱动架构分析]]

---

_Author: wangguanran_