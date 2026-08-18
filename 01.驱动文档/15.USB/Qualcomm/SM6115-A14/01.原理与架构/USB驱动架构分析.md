# Qualcomm SM6115 (scuba) USB 驱动架构分析

> **版本号：v1.0**

## 平台概述

SM6115 (scuba) 是骁龙 4 系 IoT/移动平台。USB 子系统由以下部分组成：

```
USB 控制器 (dwc3@4e00000, DWC3 + dwc3-msm 平台封装)
   ├─ qusb_phy0     — USB2.0 高速 PHY
   ├─ eud           — Embedded USB Debugger
   ├─ extcon        — 角色/线缆检测（extcon-usb-gpio、eud、phy）
   └─ 电源路径      — vbus-regulator / vbus-en-gpios / otg
```

## USB 角色切换的两条路径

1. **Type-C / OTG 标准路径**：`eud` / PHY 检测 VBUS+ID，通过 extcon 上报 `EXTCON_USB_HOST` / `EXTCON_USB`，dwc3-msm 据此切换 host/device 模式并开关 OTG VBUS。
2. **DIP ID 切换路径（本平台定制）**：硬件 DIP 开关把 PMIC GPIO3 拉到高/低作为 USB ID 信号，`extcon-usb-gpio` 驱动读取 ID：
   - **ID low = DIP OFF/D1** → host 模式（`EXTCON_USB_HOST`），HUB_RESET (GPIO108) 拉低复位 HUB，GPIO19/103 输出 5V 给 HOST VBUS；
   - **ID high = DIP ON/D2** → Type-C 交接（`id-high-is-none` 语义：ID high 上报 none 而非 gadget），HUB_RESET 拉高、VBUS 关闭。

## extcon-usb-gpio 驱动

通用 GPIO 版 extcon 驱动（`drivers/extcon/extcon-usb-gpio.c`），通过 `id-gpio` / `vbus-gpio` 检测线缆。本平台扩展了两个可选属性：

| 属性 | 作用 |
|:---|:---|
| `id-high-is-none` | 仅 ID 时，ID high 上报 **none**（而非 USB gadget），ID low 仍上报 USB-HOST。用于 ID 选择 host vs mux/Type-C 交接的场景 |
| `hub-reset-gpios` | ID low 时拉低（host 复位 HUB），ID high 时拉高 |

## dwc3-msm-core

`drivers/usb/dwc3/dwc3-msm-core.c` 是 Qualcomm 对 DWC3 的平台封装，负责：

- 注册 extcon 通知回调，随线缆状态切换 host/device；
- `vbus_regulator_toggle()` 控制 VBUS 电源（本平台扩展为同时驱动 `vbus-en-gpios` 数组：GPIO19_USBA_POWER_EN + GPIO103_OTG_5V_POWER_EN）；
- 过流保护（oc_gpiod）等。

## 扩展后的 USB 模式切换链路

```
DIP 开关 (D1/D2)
   │ PMIC GPIO3 (USB_ID, 上拉 1.8V)
   ▼
extcon-usb-gpio 驱动 (usb_extcon_detect_cable)
   ├─ ID low  → EXTCON_USB_HOST=true,  hub-reset(GPIO108)=low
   ├─ ID high → EXTCON_USB_HOST=false, hub-reset(GPIO108)=high
   ▼
dwc3-msm-core (extcon 通知回调)
   └─ vbus_regulator_toggle(on)
        ├─ vbus-en-gpios (GPIO19/103) = high  (host 时供 5V)
        └─ vbus_regulator (若配置)
   ▼
DWC3 控制器 host ⇄ device 模式切换
```

## 相关文件索引

- [[01.驱动文档/15.USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/extcon-usb-gpio.c|extcon-usb-gpio.c]]
- [[01.驱动文档/15.USB/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/dwc3-msm-core.c|dwc3-msm-core.c]]
- [[01.驱动文档/15.USB/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/extcon-usb-gpio.txt|extcon-usb-gpio.txt]]

---

_Author: wangguanran_