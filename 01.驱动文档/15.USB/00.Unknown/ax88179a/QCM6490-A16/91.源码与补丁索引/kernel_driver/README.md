# 源码与补丁索引 — kernel_driver/

## 目录说明

本目录用于存放 ax88179a USB Ethernet 驱动移植的原始源码和补丁文件。

## 文件来源

所有文件来源于 Gerrit Change #196024。

### 驱动源码 (`drivers/net/usb/ax_usb_nic/`)

| 文件 | 说明 |
|------|------|
| ax88179_178a.c | ax88179/178a 标准版驱动 (1238行) |
| ax88179_178a.h | ax88179/178a 头文件 (35行) |
| ax88179a_772d.c | 772d 版本驱动 (2701行) |
| ax88179a_772d.h | 772d 版头文件 (246行) |
| ax_ioctl.h | ioctl 接口定义 (212行) |
| ax_main.c | 主驱动文件 (2640行) |
| ax_main.h | 主头文件 (707行) |
| ax_ptp.c | PTP 时间同步 (1168行) |
| ax_ptp.h | PTP 头文件 (1168行) |
| Kconfig | 模块配置选项 (17行) |
| Makefile | 模块编译规则 (12行) |
| modules.bzl | Bazel 构建配置 (24行) |

### 配置文件

| 文件 | 说明 |
|------|------|
| drivers/net/Kconfig | 添加 source 引用 (+1) |
| drivers/net/Makefile | 添加 obj-y (+1) |
| drivers/net/modules.bzl | 添加 ax_usb_nic 模块 (+2) |
| drivers/net/usb/Kconfig | 添加 source (+6) |
| drivers/net/usb/Makefile | 添加 obj-y (+6) |
| configs/lahaina_consolidate.bzl | 平台构建配置 (+1) |
| configs/lahaina_perf.bzl | 平台构建配置 (+1) |

## 获取方式

```bash
# 从 Gerrit 拉取补丁（需内网访问权限）
git fetch <gerrit_remote> refs/changes/24/196024/1 && git checkout FETCH_HEAD
```

## 相关文档

- [[01.驱动文档/15.USB/00.Unknown/ax88179a/QCM6490-A16/04.问题案例/ax88179a-USB-Ethernet驱动移植.md|主文档：ax88179a USB Ethernet 驱动移植]]
- [[01.驱动文档/15.USB/00.Unknown/ax88179a/QCM6490-A16/04.问题案例/分析/ax88179a-USB-Ethernet驱动移植-分析.md|分析文档：ax88179a USB Ethernet 驱动移植分析]]

---

_Author: wangguanran_