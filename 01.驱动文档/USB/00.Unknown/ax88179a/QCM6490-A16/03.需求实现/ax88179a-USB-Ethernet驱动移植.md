# ax88179a USB Ethernet 驱动移植

## 概述

| 项目 | 内容 |
|------|------|
| Change | #196024 |
| 项目 | meigla/kernel/qcom |
| 分支 | Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_Rigel |
| 作者 | qianyiping |
| 状态 | MERGED |
| 类型 | 需求 (bring up) |
| 芯片 | ASIX ax88179a |
| 平台 | QCM6490 (Rigel) / Android 16 |
| 模块 | USB Ethernet |

在 Rigel 平台 (QCM6490) 上移植 ASIX ax88179a USB Ethernet 驱动，支持基于 USB 接口的有线网络功能。该驱动由 ASIX 官方提供，支持 ax88179a 和 ax88179_178a 两款芯片，包含 PTP 时间同步功能。

## 补丁内容

```diff
[PATCH] [RIGEL_A16][TaskID]120099[Description]ax88179a bring up in Rigel A16[owner]qianyiping

diff --git a/configs/lahaina_consolidate.bzl b/configs/lahaina_consolidate.bzl
index 794089f..d1ea01a 100644
--- a/configs/lahaina_consolidate.bzl
+++ b/configs/lahaina_consolidate.bzl
@@ -27,5 +27,6 @@
     "CONFIG_SDHCI_MSM_DBG": "y",
     "CONFIG_UFS_DBG": "y",
     "CONFIG_USB_LINK_LAYER_TEST": "m",
+    "CONFIG_USB_NET_AX_USB_NIC": "m",
     "CONFIG_EXTCON": "y",
 }
diff --git a/configs/lahaina_perf.bzl b/configs/lahaina_perf.bzl
index 85a5b0d..eb6ad49 100644
--- a/configs/lahaina_perf.bzl
+++ b/configs/lahaina_perf.bzl
@@ -374,6 +374,7 @@
     "CONFIG_USB_G_WEBCAM": "m",
     "CONFIG_USB_LAN78XX": "m",
     "CONFIG_USB_MSM_SSPHY_QMP": "m",
+    "CONFIG_USB_NET_AX_USB_NIC": "m",
     "CONFIG_USB_QCOM_EMU_PHY": "m",
     "CONFIG_USB_REDRIVER": "m",
     "CONFIG_USB_REDRIVER_NB7VPQ904M": "m",
diff --git a/drivers/net/Kconfig b/drivers/net/Kconfig
index 5e0c906..3f69dc4 100644
--- a/drivers/net/Kconfig
+++ b/drivers/net/Kconfig
@@ -50,5 +50,6 @@
 source "$(KCONFIG_EXT_PREFIX)drivers/net/ethernet/Kconfig"
 source "$(KCONFIG_EXT_PREFIX)drivers/net/phy/Kconfig"
 source "$(KCONFIG_EXT_PREFIX)drivers/net/mdio_fe/Kconfig"
+source "$(KCONFIG_EXT_PREFIX)drivers/net/usb/Kconfig"
 
 endif # NETDEVICES
diff --git a/drivers/net/Makefile b/drivers/net/Makefile
index aeff3e5..5b64d068 100644
--- a/drivers/net/Makefile
+++ b/drivers/net/Makefile
@@ -6,6 +6,7 @@
 obj-y += ethernet/stmicro/stmmac/
 obj-y += pcs/
 obj-y += phy/aquantia/
+obj-y += usb/
 
 obj-$(CONFIG_FAILOVER) += failover.o
 failover-objs += ../../net/core/failover.o
diff --git a/drivers/net/modules.bzl b/drivers/net/modules.bzl
index dc5ec5e..dadef8e 100644
... (patch truncated, total +10205/-19 lines, 10332 lines)
+            "drivers/net/usb/ax_usb_nic/ax88179a_772d.h",
+            "drivers/net/usb/ax_usb_nic/ax_ioctl.h",
+            "drivers/net/usb/ax_usb_nic/ax_ptp.h",
+        ],
+        copts = [
+            "-DENABLE_IOCTL_DEBUG",
+            "-DENABLE_INT_POLLING",
+            "-DENABLE_AX88279",
+        ],
+    )
```
## 配置方式

### Kernel Kconfig 配置

新增 `drivers/net/usb/ax_usb_nic/Kconfig`，提供以下配置选项：

```
config AX_USB_NIC
    tristate "ASIX AX88179A USB 2.0/3.0 Ethernet Adapter support"
    depends on USB_USBNET
    select CRC32
    select PHYLIB
    ---help---
      This option adds support for ASIX AX88179A based USB 2.0/3.0 Ethernet
      adapters.
```

在以下 Kconfig 文件中添加了 source 引用：

- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|drivers/net/Kconfig]] — 添加 `source "drivers/net/usb/ax_usb_nic/Kconfig"`
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|drivers/net/usb/Kconfig]] — 添加 `source "drivers/net/usb/ax_usb_nic/Kconfig"`

### Makefile 配置

在以下 Makefile 中添加了模块编译路径：

- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|drivers/net/Makefile]] — 添加 `obj-y += ax_usb_nic/`
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|drivers/net/usb/Makefile]] — 添加 `obj-y += ax_usb_nic/`

### Bazel 模块配置

在以下 `.bzl` 文件中添加了模块定义：

- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|configs/lahaina_consolidate.bzl]] — 添加 ax88179a 模块配置
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|configs/lahaina_perf.bzl]] — 添加 ax88179a 模块配置
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|drivers/net/modules.bzl]] — 添加 ax_usb_nic 模块
- `drivers/net/usb/ax_usb_nic/modules.bzl` — 新增模块的 Bazel 构建配置

### 内核配置选项

在内核配置中启用以下选项：

```
CONFIG_AX_USB_NIC=y
CONFIG_USB_USBNET=y
```

### DTS 配置

由于 ax88179a 是 USB 设备，通过 USB 子系统自动枚举，无需额外 DTS 配置。驱动在 USB 设备插入时通过 USB VID/PID 自动匹配加载。

## 验证方式

### 验证环境

| 项目 | 内容 |
|------|------|
| 平台 | Rigel (QCM6490) |
| Android 版本 | A16 |
| 硬件 | ASIX ax88179a USB 3.0 to Gigabit Ethernet 适配器 |

### 验证命令

**1. 设备识别检查**

```bash
# 检查 USB 设备是否被识别
lsusb | grep -i asix

# 检查网络接口是否创建
ip link show

# 查看内核日志，确认驱动加载
dmesg | grep ax88
```

**2. 网络功能验证**

```bash
# 查看网络接口状态
ifconfig ethX

# 获取 IP 地址（DHCP）
dhcpcd ethX

# 测试网络连通性
ping -c 4 <网关地址>

# 查看传输速率
ethtool ethX
```

**3. PTP 时间同步验证**

```bash
# 查看 PTP 时钟设备
ls /dev/ptp*

# 使用 ptp4l 进行时间同步
ptp4l -i ethX -m
```

### 预期结果

- `lsusb` 应显示 ASIX ax88179a 设备
- 系统应创建新的网络接口（如 eth0 或 enx...）
- 内核日志应显示 `ax88179a` 驱动加载成功
- 网络接口应能正常获取 IP 地址并通信
- `ethtool` 应显示千兆以太网能力
- PTP 设备应出现在 `/dev/ptp*` 中

### 实际结果

⚠️ 无法直接验证，需从 Gerrit 拉取补丁后编译烧录验证。

## 源码归档

> ⚠️ **源码文件不在本地归档**
>
> 项目 `meigla/kernel/qcom` 不在 134 服务器上，无法从源码树直接检索驱动文件。
> 当前仅归档了 Gerrit patch 文件本身（`patches/196024.patch`），该 patch 包含完整的驱动源码（~10K 行，19 个文件）。
> 如需获取源码，请查看 [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/patches/196024.patch|patches/196024.patch]] 或从 Gerrit 拉取补丁。

驱动源码目录结构（来自 patch 内容）：

```
drivers/net/usb/ax_usb_nic/
├── Kconfig              # 模块配置选项
├── Makefile             # 模块编译规则
├── modules.bzl          # Bazel 构建配置
├── ax88179_178a.c       # ax88179/178a 标准版驱动 (1238行)
├── ax88179_178a.h       # ax88179/178a 头文件 (35行)
├── ax88179a_772d.c      # 772d 版本驱动 (2701行)
├── ax88179a_772d.h      # 772d 版头文件 (246行)
├── ax_ioctl.h           # ioctl 接口定义 (212行)
├── ax_main.c            # 主驱动文件 (2640行)
├── ax_main.h            # 主头文件 (707行)
├── ax_ptp.c             # PTP 时间同步 (1168行)
└── ax_ptp.h             # PTP 头文件 (1168行)
```

驱动源码归档路径：[[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/|91.源码与补丁索引/kernel_driver/]]

补丁文件归档路径：[[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/patches/196024.patch|patches/196024.patch]]

## 引用文件索引

> ⚠️ 以下所有文件均来自 Gerrit Change #196024 的 patch 内容，源码文件未在本地归档
> （项目 `meigla/kernel/qcom` 不在 134 服务器上）。完整差异见：
> [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/patches/196024.patch|patches/196024.patch]]

| 序号 | 文件路径 | 变更类型 | 行数变更 |
|------|----------|----------|----------|
| 1 | configs/lahaina_consolidate.bzl | ADDED | +1/-0 |
| 2 | configs/lahaina_perf.bzl | ADDED | +1/-0 |
| 3 | drivers/net/Kconfig | ADDED | +1/-0 |
| 4 | drivers/net/Makefile | ADDED | +1/-0 |
| 5 | drivers/net/modules.bzl | ADDED | +2/-0 |
| 6 | drivers/net/usb/Kconfig | ADDED | +6/-0 |
| 7 | drivers/net/usb/Makefile | ADDED | +6/-0 |
| 8 | drivers/net/usb/ax_usb_nic/Kconfig | ADDED | +17/-0 |
| 9 | drivers/net/usb/ax_usb_nic/Makefile | ADDED | +12/-0 |
| 10 | drivers/net/usb/ax_usb_nic/ax88179_178a.c | ADDED | +1238/-0 |
| 11 | drivers/net/usb/ax_usb_nic/ax88179_178a.h | ADDED | +35/-0 |
| 12 | drivers/net/usb/ax_usb_nic/ax88179a_772d.c | ADDED | +2701/-0 |
| 13 | drivers/net/usb/ax_usb_nic/ax88179a_772d.h | ADDED | +246/-0 |
| 14 | drivers/net/usb/ax_usb_nic/ax_ioctl.h | ADDED | +212/-0 |
| 15 | drivers/net/usb/ax_usb_nic/ax_main.c | ADDED | +2640/-0 |
| 16 | drivers/net/usb/ax_usb_nic/ax_main.h | ADDED | +707/-0 |
| 17 | drivers/net/usb/ax_usb_nic/ax_ptp.c | ADDED | +1168/-0 |
| 18 | drivers/net/usb/ax_usb_nic/ax_ptp.h | ADDED | +1168/-0 |
| 19 | drivers/net/usb/ax_usb_nic/modules.bzl | ADDED | +24/-0 |

**文件统计：** 新增 19 个文件，总计新增约 10,183 行代码。

---


---

## 补充：内核 common 侧排除通用驱动绑定（#196763）

**Change**: #196763 | **项目**: meigla/kernel/common | **分支**: Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_Rigel | **作者**: qianyiping | **状态**: MERGED
**任务**: Task 120099（与 #196024 同一任务，合并归档）

### 需求描述

Rigel A16 上内核 `kernel/common` 的通用网络驱动会与 vendor 的 `ax_usb_nic` 驱动**竞争绑定** AX88179/178A 设备：

- `drivers/net/usb/ax88179_178a.c`（内核自带版本）通过 `driver_info` 绑定 0x0b95:0x1790 / 0x178a；
- `drivers/net/usb/cdc_ncm.c` 的通用 CDC-NCM 匹配也可能命中（AX88179 支持 NCM 接口）。

两个通用驱动抢在 vendor 驱动之前 probe 时，vendor `ax_usb_nic` 无法绑定设备，导致带 PTP 等定制功能的驱动不生效。需要把这两个通用驱动对 AX88179/178A 的匹配**排除掉**（driver_info = 0 黑名单）。

### 方案

1. `ax88179_178a.c`：
   - `ax88179_info` / `ax88178a_info` 标记 `__maybe_unused`（不再被引用，避免编译告警）；
   - products 表中 AX88179/AX88178A 两项 `.driver_info = 0`（`usbnet_probe()` 对 driver_info==0 视为黑名单返回 -ENODEV）；
   - 注释明确"Exclude ASIX AX88179 so vendor ax_usb_nic can bind"。
2. `cdc_ncm.c`：CDC-NCM 设备表新增两条 `driver_info = 0` 的 AX88179/178A 黑名单项（`USB_DEVICE_AND_INTERFACE_INFO(0x0b95, 0x1790/0x178a, ...)`）。

### 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/drivers/net/usb/ax88179_178a.c\|drivers/net/usb/ax88179_178a.c]] | +8/-6 | AX88179/178A 黑名单（driver_info=0） |
| [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/drivers/net/usb/cdc_ncm.c\|drivers/net/usb/cdc_ncm.c]] | +16/-0 | CDC-NCM 黑名单项 |

### 配置方式

```c
/* ax88179_178a.c 产品表 */
USB_DEVICE_AND_INTERFACE_INFO(0x0b95, 0x1790, 0xff, 0xff, 0),
.driver_info = 0,   /* 黑名单：让 vendor ax_usb_nic 绑定 */

/* cdc_ncm.c 黑名单 */
{ USB_DEVICE_AND_INTERFACE_INFO(0x0b95, 0x1790, USB_CLASS_COMM, USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
  .driver_info = 0 },
```

### 验证方式

```bash
# 确认 vendor 驱动绑定（而非通用 ax88179_178a / cdc_ncm）
ls -l /sys/bus/usb/devices/*/net/
dmesg | grep -i "ax_usb_nic\|ax88179"
# 预期：ax_usb_nic 成功 bind，网卡功能 + PTP 正常
```

### 补丁内容

```diff
From 2046da1e03c2181f33c4da1a97ca1d194624c449 Mon Sep 17 00:00:00 2001

---

diff --git a/drivers/net/usb/ax88179_178a.c b/drivers/net/usb/ax88179_178a.c
index b034ef8..bac1a97 100644
--- a/drivers/net/usb/ax88179_178a.c
+++ b/drivers/net/usb/ax88179_178a.c
@@ -1713,7 +1713,7 @@
 	return 0;
 }
 
-static const struct driver_info ax88179_info = {
+static const struct driver_info ax88179_info __maybe_unused = {
 	.description = "ASIX AX88179 USB 3.0 Gigabit Ethernet",
 	.bind = ax88179_bind,
 	.unbind = ax88179_unbind,
@@ -1726,7 +1726,7 @@
 	.tx_fixup = ax88179_tx_fixup,
 };
 
-static const struct driver_info ax88178a_info = {
+static const struct driver_info ax88178a_info __maybe_unused = {
 	.description = "ASIX AX88178A USB 2.0 Gigabit Ethernet",
 	.bind = ax88179_bind,
 	.unbind = ax88179_unbind,
@@ -1884,13 +1884,15 @@
 
 static const struct usb_device_id products[] = {
 {
-	/* ASIX AX88179 10/100/1000 */
+	/* Exclude ASIX AX88179 so vendor ax_usb_nic can bind.
+	 * usbnet_probe() treats driver_info == 0 as a blacklist (-ENODEV).
+	 */
 	USB_DEVICE_AND_INTERFACE_INFO(0x0b95, 0x1790, 0xff, 0xff, 0),
-	.driver_info = (unsigned long)&ax88179_info,
+	.driver_info = 0,
 }, {
-	/* ASIX AX88178A 10/100/1000 */
+	/* Exclude ASIX AX88178A so vendor ax_usb_nic can bind. */
 	USB_DEVICE_AND_INTERFACE_INFO(0x0b95, 0x178a, 0xff, 0xff, 0),
-	.driver_info = (unsigned long)&ax88178a_info,
+	.driver_info = 0,
 }, {
 	/* Cypress GX3 SuperSpeed to Gigabit Ethernet Bridge Controller */
 	USB_DEVICE_AND_INTERFACE_INFO(0x04b4, 0x3610, 0xff, 0xff, 0),
diff --git a/drivers/net/usb/cdc_ncm.c b/drivers/net/usb/cdc_ncm.c
index 5c89e03..48a8e84 100644
--- a/drivers/net/usb/cdc_ncm.c
+++ b/drivers/net/usb/cdc_ncm.c
@@ -2105,6 +2105,22 @@
 	  .driver_info = (unsigned long)&cdc_ncm_zlp_info,
 	},
 
+	/*
+	 * Exclude ASIX AX88179/179A (0x0b95:0x1790 / 0x178a) from generic
+	 * CDC-NCM matching so the vendor ax_usb_nic driver can bind.
+	 * usbnet_probe() treats driver_info == 0 as a blacklist (-ENODEV).
+	 */
+	{ USB_DEVICE_AND_INTERFACE_INFO(0x0b95, 0x1790,
+		USB_CLASS_COMM,
+		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
+	  .driver_info = 0,
+	},
+	{ USB_DEVICE_AND_INTERFACE_INFO(0x0b95, 0x178a,
+		USB_CLASS_COMM,
+		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
+	  .driver_info = 0,
+	},
+
 	/* Generic CDC-NCM devices */
 	{ USB_INTERFACE_INFO(USB_CLASS_COMM,
 		USB_CDC_SUBCLASS_NCM, USB_CDC_PROTO_NONE),
```

### 补丁验证

| Change | 验证方式 | 结果 |
|--------|---------|------|
| #196763 | Gerrit REST 拉取 current revision 源码与补丁比对 | ✅ 与归档源码一致 |

> ⚠️ 项目 `meigla/kernel/common` 不在 134 服务器上，无法 `git apply --check`。已通过 Gerrit REST 拉取源码比对。

### 源码归档

- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/drivers/net/usb/ax88179_178a.c|ax88179_178a.c]]（REST，含 #196763 改动）
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/drivers/net/usb/cdc_ncm.c|cdc_ncm.c]]（REST，含 #196763 改动）
- 补丁：[[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/patches/196763.patch|196763.patch]]

### 引用文件索引（#196763 补充）

| 序号 | 文件 | 说明 |
|------|------|------|
| 1 | [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/drivers/net/usb/ax88179_178a.c\|ax88179_178a.c]] | 内核通用 AX88179 驱动（黑名单） |
| 2 | [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/kernel_driver/drivers/net/usb/cdc_ncm.c\|cdc_ncm.c]] | CDC-NCM 通用驱动（黑名单） |
| 3 | [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/patches/196763.patch\|196763.patch]] | #196763 补丁 |


_Author: wangguanran_