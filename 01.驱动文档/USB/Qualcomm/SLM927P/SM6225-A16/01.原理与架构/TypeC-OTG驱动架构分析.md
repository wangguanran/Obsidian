# Qualcomm SM6225 (khaje) TypeC-OTG 驱动架构分析

> **版本号：v1.0**

## 平台概述

SLM927P_A16 (SM6225/khaje) 的 USB Type-C 口通过 **FUSB302** Type-C 控制器芯片做 CC 检测，USB 控制器为 DWC3（`dwc3@4e00000`）。

```
USB Type-C 接口 (CC1/CC2)
   │
   ▼
FUSB302 (fairchild,fusb302, I2C 0x22, INT=GPIO36)
   │  CC 检测 → Type-C 状态机（DRP/UFP/DFP）→ role 上报
   ▼
extcon 通知 (extcon = <&fusb302>)
   │
   ▼
dwc3 (usb-role-switch, dr_mode = "otg")
   ├─ host 模式: qcom,otg-vbus-gpio (GPIO108) boost 输出 5V
   └─ device 模式: 由主机供电，USB gadget 枚举
```

## USB host/device 切换链路

```
FUSB302 检测 CC 状态（RDFP=host / RDFN=device / 断开）
   │  INT (GPIO36) 中断 → fusb302 驱动解析
   ▼
fusb302 驱动更新 role（作为 extcon provider 上报）
   ├─ EXTCON_USB_HOST → dwc3 切 host，otg-vbus-gpio 输出 5V
   └─ EXTCON_USB      → dwc3 切 device（gadget）
   ▼
USB 控制器角色切换完成
```

关键 DT 属性：

| 属性 | 作用 |
|:---|:---|
| `usb-role-switch` (usb0 与 dwc3) | 使能 USB role switch 框架，dwc3 按 extcon/role 状态动态切换 host/device |
| `dr_mode = "otg"` (dwc3) | DWC3 运行在 OTG 模式（可切 host/device） |
| `extcon = <&fusb302>` (usb0) | 将 FUSB302 注册为 extcon provider，替代原先的 phy/eud 线缆检测 |
| `qcom,otg-vbus-gpio = <&tlmm 108 0>` | OTG 时用 GPIO boost 输出 VBUS 5V（nopmi 板无 PMIC 升压，用 GPIO） |
| `///delete-property/ extcon` | 注释掉的旧删除逻辑：原 khaje-idp-nopmi 曾删除 usb0 的 extcon 属性，现恢复 |

## extcon 与 phy 的双路径

- **khaje-usb.dtsi**：usb0 保留 `extcon = <&eud>`（eud 为调试场景），Type-C 控制由 FUSB302 提供；
- **khaje-idp.dtsi / khaje-idp-nopmi.dtsi**：usb0 的 extcon 指向 `&fusb302`，dwc3 切 OTG 模式。

## FUSB302 节点配置

```dts
&qupv3_se1_i2c {
	fusb302: typec-portc@22 {
		compatible = "fairchild,fusb302";
		reg = <0x22>;
		vdda18-supply = <&pm6125_l9>;    /* 1.8V 供电 */
		vdda33-supply = <&pm6125_l15>;   /* 3.3V 供电 */
		pinctrl-names = "default";
		pinctrl-0 = <&fusb302_default>;  /* INT 引脚配置 */
		int-n-gpios = <&tlmm 36 0x00>;   /* 中断 GPIO */
		status = "okay";
	};
};
```

## 相关文件索引

- [[01.驱动文档/USB/Qualcomm/SLM927P/SM6225-A16/91.源码与补丁索引/dt_config/khaje-idp.dtsi|khaje-idp.dtsi]]
- [[01.驱动文档/USB/Qualcomm/SLM927P/SM6225-A16/91.源码与补丁索引/dt_config/khaje-idp-nopmi.dtsi|khaje-idp-nopmi.dtsi]]
- [[01.驱动文档/USB/Qualcomm/SLM927P/SM6225-A16/91.源码与补丁索引/dt_config/khaje-usb.dtsi|khaje-usb.dtsi]]

---

_Author: wangguanran_