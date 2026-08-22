# 源码与补丁索引 — kernel_driver/

## 目录说明

本目录存放 ax88179a USB Ethernet 驱动移植的原始源码，与 `patches/196024.patch`（Gerrit Change #196024，[项目代号]_A16）配对。

## 文件来源

源码从 134 服务器 [项目代号] A16 源码树检索归档：

```
[内网路径]/LA.VENDOR.16.2.1/kernel_platform/soc-repo/
```

（2026-08-20 归档；此前该树未在 134 常规 workspace 中，故 README 曾标注"源码不在本地归档"。）

### 驱动源码 (`drivers/net/usb/ax_usb_nic/`)

| 文件 | 说明 |
|------|------|
| ax88179_178a.c / .h | ax88179/178a 标准版驱动 |
| ax88179a_772d.c / .h | 772d 版本驱动 |
| ax_ioctl.h | ioctl 接口定义 |
| ax_main.c / .h | 主驱动文件 |
| ax_ptp.c / .h | PTP 时间同步 |
| Kconfig | 模块配置选项 |
| Makefile | 模块编译规则 |
| modules.bzl | Bazel 构建配置 |

### 构建/配置修改

| 文件 | 说明 |
|------|------|
| drivers/net/Kconfig | 添加 source 引用 |
| drivers/net/Makefile | 添加 obj-y |
| drivers/net/modules.bzl | 添加 ax_usb_nic 模块 |
| drivers/net/usb/Kconfig | 添加 source |
| drivers/net/usb/Makefile | 添加 obj-y |
| configs/lahaina_consolidate.bzl | 平台构建配置 `CONFIG_USB_NET_AX_USB_NIC=m` |
| configs/lahaina_perf.bzl | 平台构建配置 `CONFIG_USB_NET_AX_USB_NIC=m` |

## 补丁归档

| 文件 | 大小 | 说明 |
|------|------|------|
| [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/patches/196024.patch\|196024.patch]] | 272 KB | Gerrit Change #196024 完整补丁 (+10205/-19) |

## 相关文档

- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/03.需求实现/ax88179a-USB-Ethernet驱动移植.md|主文档：ax88179a USB Ethernet 驱动移植]]
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/03.需求实现/ax88179a-USB-Ethernet驱动移植-分析.md|分析文档：ax88179a USB Ethernet 驱动移植分析]]

---

_Author: wangguanran_
