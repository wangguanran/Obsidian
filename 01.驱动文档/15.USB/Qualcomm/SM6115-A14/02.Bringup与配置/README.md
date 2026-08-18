# SM6115-A14 (scuba) USB 平台移植资料

> **版本号：v1.0**

## 平台版本

| 项目 | 内容 |
|------|------|
| SoC | Qualcomm SM6115 (scuba) |
| Android | A14 (LA.VENDOR.13.2.1 / MT5205 分支) |
| 内核 | msm-kernel 5.10 (KERNEL.PLATFORM.2.0) |
| DT | qcom/proprietary/devicetree (scuba 系列 dtsi) |

## 硬件接口

| 信号 | 引脚 | 说明 |
|:---|:---|:---|
| USB_ID | PM2250 GPIO3 (`pm2250_gpios 3`) | 接 DIP 开关，1.8V 上拉读 ID（需 `power-source=<1>` 选 1.8V VIN，否则 1.7V ID 电平被误判为低） |
| USBA_POWER_EN | TLMM GPIO19 | host 模式 VBUS 5V 使能 1/2 |
| OTG_5V_POWER_EN | TLMM GPIO103 | host 模式 VBUS 5V 使能 2/2 |
| HUB_RESET | TLMM GPIO108 | HUB 复位：ID low→低（复位），ID high→高 |
| USB D+/D- | — | qusb_phy0 + eud |

## 驱动索引

| 驱动/文件 | 路径（源码树内） | 说明 |
|:---|:---|:---|
| extcon-usb-gpio | `kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c` | DIP ID 检测 → host/type-c 判定 + HUB_RESET 跟随 |
| dwc3-msm-core | `kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c` | DWC3 平台封装，vbus-en-gpios 驱动 |
| extcon binding | `.../devicetree/bindings/extcon/extcon-usb-gpio.txt` | `id-high-is-none`、`hub-reset-gpios` 属性说明 |
| scuba-iot-idp.dtsi | `.../devicetree/qcom/scuba-iot-idp.dtsi` | extcon_usb_id 节点、pinctrl、usb0 配置 |
| scuba.dtsi / scuba_auto-pmic.dtsi | `.../devicetree/qcom/` | 移除 skin_therm（GPIO3 改作 USB_ID） |
| scuba-thermal.dtsi | `.../devicetree/qcom/scuba-thermal.dtsi` | chg-skin-therm 禁用 |
| bengal_GKI.config | `kernel_platform/msm-kernel/arch/arm64/configs/vendor/` | `CONFIG_EXTCON_USB_GPIO=m` |

## DTS 配置要点

```dts
&pm2250_gpios {
    usb_id {
        usb_id_default: usb_id_default {
            pins = "gpio3";
            function = "normal";
            input-enable;
            bias-pull-up;
            power-source = <1>;   /* 1.8V VIN：保证 1.7V ID 电平判高 */
        };
    };
};

&soc {
    extcon_usb_id: extcon_usb_id {
        compatible = "linux,extcon-usb-gpio";
        id-gpio = <&pm2250_gpios 3 GPIO_ACTIVE_HIGH>;
        id-high-is-none;                       /* ID high = Type-C(none) */
        hub-reset-gpios = <&tlmm 108 GPIO_ACTIVE_HIGH>;
        pinctrl-names = "default";
        pinctrl-0 = <&usb_id_default &hub_reset_default>;
    };
};

&usb0 {
    extcon = <&qusb_phy0>, <&eud>, <&extcon_usb_id>;
    vbus-en-gpios = <&tlmm 19 GPIO_ACTIVE_HIGH>,
                    <&tlmm 103 GPIO_ACTIVE_HIGH>;
    pinctrl-names = "default";
    pinctrl-0 = <&usba_pwr_en_default &otg_5v_en_default>;
};
```

## 内核配置

```
CONFIG_EXTCON_USB_GPIO=m
```

## 编译命令

```bash
# 134 源码树 MT5205 (LA.VENDOR.13.2.1)
cd <tree>/kernel_platform
# 编译内核（bengal 配置）
source build/envsetup.sh
./build.sh -k msm-kernel  # 或平台既有编译脚本
# 仅编译 dtbo 验证 DTS 语法
make dtboimage
```

## 相关文档

- [[01.驱动文档/15.USB/Qualcomm/SM6115-A14/04.问题案例/USB-TypeC-DIP-ID-Host-Mux-hubreset修复.md|USB-TypeC-DIP-ID-Host-Mux-hubreset修复]]
- [[01.驱动文档/15.USB/Qualcomm/SM6115-A14/01.原理与架构/USB驱动架构分析.md|USB 驱动架构分析]]

---

_Author: wangguanran_