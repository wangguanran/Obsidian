# Qualcomm SM6225 (khaje) SMB5 充电驱动架构分析

> **版本号：v1.0**

## 平台概述

SM6225 (khaje) 平台的充电管理由 **smb5 驱动**（`drivers/power/supply/qcom/smb5-lib.c` + `smb5.c`）实现，管理 PM6125/PM7250B 的 SMB5 充电器硬件：输入检测（DCIN/USBIN）、电池充电、OTG、无线充/背充（POGO）等。

## 充电路径

```
电源输入（DCIN / USBIN / POGO）
   │
   ▼
SMB5 充电器 (pm6125_charger)
   ├─ 输入源检测 (smblib_dc_detect / usb_detect)
   ├─ 充电状态机 (smblib_update_usb_status / charging)
   ├─ OTG / USB host 供电 (smblib_notify_usb_host / otg)
   └─ 温度监测 (adc_tm, skin_therm 等)
```

## POGO / Dock 路径（MT5825 定制）

POGO 接口（背夹/Dock）由 smb5-lib 中 `smblib_pogo_*` 系列函数处理，关键信号：

| 信号 | 说明 |
|:---|:---|
| `pogo_en_hub_vcc_gpio` (EN4) | Dock HUB 的 VBUS 5V 供电使能 |
| `pogo_en_typea_5v_gpio` (EN2) | Type-A 口 5V 使能 |
| `pogo_sw_uart_usb_gpio` | UART/USB 切换 |
| `pogo_irq_back_det` | Dock 插入/拔出检测中断 |

## Dock 插入处理（smblib_pogo_irq_back_det_delayed_work）

```c
Dock 插入:
  ├─ pogo_sw_uart_usb_gpio = 1    (切到 USB)
  ├─ pogo_en_typea_5v_gpio = 1    (EN2 on)
  ├─ pogo_en_hub_vcc_gpio = 1     (EN4 on, HUB 5V)
  └─ smblib_pogo_notify_usb_host_role(chg, true)   ← 仅 extcon 通知，不开 OTG_EN
Dock 拔出:
  ├─ pogo_en_typea_5v_gpio = 0
  ├─ pogo_en_hub_vcc_gpio = 0
  └─ smblib_pogo_notify_usb_host_role(chg, false)
```

**关键设计**：Dock host 模式下**不调用 `smblib_notify_usb_host()`**（该函数会打开充电器 OTG_EN，占用 DCIN 输入路径导致无法同时充电），而是通过 `smblib_pogo_notify_usb_host_role()` 只做 extcon 通知。HUB 的 5V 由 EN4 GPIO 直接供电，因此 DCIN 充电可与 Dock USB host 同时工作。

## extcon 通知链

```
smblib_pogo_notify_usb_host_role(chg, enable)
  ├─ enable → smblib_notify_extcon_props(chg, EXTCON_USB_HOST)
  └─ extcon_set_state_sync(chg->extcon, EXTCON_USB_HOST, enable)
          │
          ▼
USB 控制器 (dwc3) 切换 host/device 模式
```

## 相关文件索引

- [[01.驱动文档/Charger/Qualcomm/SM6225-A14/91.源码与补丁索引/kernel_driver/smb5-lib.c|smb5-lib.c]]

---

_Author: wangguanran_