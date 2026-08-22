# ASIX ax88179a USB Ethernet 驱动架构分析

> **版本号：v1.0**

## 平台概述

在 [项目代号] 平台（QCM6490）上移植 ASIX ax88179a USB Ethernet 驱动，提供基于 USB 接口的有线网络：

```
应用层 (TCP/IP 协议栈)
   ↓
内核网络子系统 (netdev)
   └─ ax_usb_nic 驱动 (USB Ethernet)
         ├─ USB 批量传输端点 (bulk IN/OUT)
         ├─ 内部 MAC + PHY
         └─ PTP 硬件时间戳（ax88179a 支持）
              ↓
USB 主机控制器 (QCM6490 USB3/2)
```

## 驱动特性

| 特性 | 说明 |
|:---|:---|
| 芯片支持 | ax88179a、ax88179_178a |
| 接口 | USB（CDC ECM 变体，自定义量产端点） |
| PTP | 支持 IEEE 1588 时间同步 |
| 传输速率 | 支持 USB 3.0 SuperSpeed 千兆 |

## 内核集成方式

驱动以**外部模块**方式集成：

1. `Kconfig`：在 `drivers/net/Kconfig` 增加 `source "$(KCONFIG_EXT_PREFIX)drivers/net/usb/Kconfig"` 挂载外部 Kconfig
2. **defconfig**：`CONFIG_USB_NET_AX_USB_NIC=m`（模块方式编译）
3. **common 侧**（#196763）：在 kernel common 侧排除通用 USB Ethernet 驱动绑定，避免与 ax_usb_nic 冲突

## 关键配置

```kconfig
# drivers/net/usb/Kconfig
config USB_NET_AX_USB_NIC
    tristate "ASIX AX88179A USB 3.0/2.0 Gigabit Ethernet"
    depends on USB_USBNET
```

## 引用文件索引

- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/03.需求实现/ax88179a-USB-Ethernet驱动移植.md|ax88179a-USB-Ethernet驱动移植]]（补丁内容）
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/|91.源码与补丁索引]]

---

_Author: wangguanran_
