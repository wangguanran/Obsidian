# Dock 连接影响充电功能修复

> **模块**: Charger | **厂商**: Qualcomm | **芯片**: SM6225 (khaje)
> **平台**: SM6225-A14 (LA.VENDOR.13.2.1) | **类型**: Bug
> **Change**: #195273 | **作者**: [同事] | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #195273 |
| 项目 | LA.VENDOR.13.2.1 |
| 分支 | master_IOT_High_Mid_2024.SPF.1.0_[项目代号] |
| 作者 | [同事] |
| 类型 | Bug（Dock 连接影响充电功能） |
| 芯片 | Qualcomm SM6225 (khaje) |
| 平台 | SM6225-A14 (LA.VENDOR.13.2.1) |
| 模块 | Charger (smb5-lib POGO/Dock) |
| 提交标题 | `[项目代号][TaskID]119595[Description]When the dock is connected to the host, the charging function will be affected[owner][同事]` |
| 任务 | Task 119595 |

## 现象

设备通过 POGO 接口连接 Dock（Dock 作为 USB host）时，**充电功能受影响**：Dock 后接 host 时充电异常，无法在 Dock USB host 工作期间正常充电。

## 根因分析

Dock 插入时原有处理逻辑为：

```c
/* Switch usb mode to host - notify USB driver via extcon */
smblib_notify_usb_host(chg, true);
```

`smblib_notify_usb_host(enable=true)` 内部除了通知 USB 控制器外，还会**打开充电器的 OTG_EN**（OTG 模式），OTG 与 DCIN 输入路径互斥 —— OTG_EN 开启后充电器认为正通过自身输出 5V VBUS，DCIN/Dock 输入路径被禁用，因此 **Dock host 工作时无法同时充电**。

而硬件上 Dock HUB 的 5V 是由 `pogo_en_hub_vcc_gpio`（EN4）直接供电的，**不需要充电器 OTG_EN 输出 VBUS**。所以正确做法是：只通过 extcon 通知 USB 控制器切换 host 模式，不打开 OTG_EN，DCIN 充电即可与 Dock host 并存。

## 处理方案

1. 新增 `smblib_pogo_notify_usb_host_role(struct smb_charger *chg, bool enable)`：

   - enable 时调用 `smblib_notify_extcon_props(chg, EXTCON_USB_HOST)` 预通知（下发 role props），再 `extcon_set_state_sync(chg->extcon, EXTCON_USB_HOST, enable)` 设置状态；
   - **不调用** `smblib_notify_usb_host()`，即不开 OTG_EN，HUB VBUS 由 EN4 提供。

2. `smblib_pogo_irq_back_det_delayed_work` 中：
   - Dock 插入分支：补上 **EN2 (pogo_en_typea_5v_gpio) 置 1**（Type-A 口 5V 使能），并将 `smblib_notify_usb_host(chg, true)` 替换为 `smblib_pogo_notify_usb_host_role(chg, true)`；
   - Dock 拔出分支：`smblib_notify_usb_host(chg, false)` 替换为 `smblib_pogo_notify_usb_host_role(chg, false)`。

3. 验证：编译 PASS。

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/Charger/Qualcomm/SM6225-A14/91.源码与补丁索引/kernel_driver/smb5-lib.c\|smb5-lib.c]] | +26/-4 | 新增 `smblib_pogo_notify_usb_host_role()`；Dock 插入/拔出改为仅 extcon 通知；插入时补 EN2 使能 |

具体改动点：

- 新增函数 `smblib_pogo_notify_usb_host_role()`（仅 extcon 通知，不开 OTG_EN，HUB 5V 由 EN4 供）；
- `smblib_pogo_irq_back_det_delayed_work` Dock 插入分支：`pogo_en_typea_5v_gpio=1`（EN2 on）＋ 新函数替换 `smblib_notify_usb_host(chg, true)`；
- 拔出分支：新函数替换 `smblib_notify_usb_host(chg, false)`。

## 配置方式

本补丁为纯代码修改，无 DTS / Kconfig 配置变更。

日志关键字（dmesg 可查）：

```
[POGO_USB]smblib_pogo_notify_usb_host_role: EXTCON_USB_HOST=1 (no OTG_EN, hub via EN4)
[POGO_USB]smblib_pogo_irq_back_det_delayed_work:xxx set pogo_en_typea_5v_gpio=1 (EN2 on)
[POGO_USB]smblib_pogo_irq_back_det_delayed_work:xxx set pogo_en_hub_vcc_gpio=1 (EN4 on)
```

## 验证方式

| 项目 | 内容 |
|------|------|
| 编译 | ✅ 编译 PASS |
| 功能 | Dock 连接 host 时，DCIN 充电正常进行（充电电流 > 0） |
| 日志 | `[POGO_USB]` 打印 `EXTCON_USB_HOST=1 (no OTG_EN, hub via EN4)`，无 OTG_EN 相关动作 |
| 回归 | Dock USB host 枚举正常（HUB 5V 由 EN4 供电） |

## 结论

Dock host 模式下不再打开充电器 OTG_EN（避免与 DCIN 输入互斥），HUB 5V 由 EN4 GPIO 直接供电，仅通过 extcon 通知 USB 控制器切换 host/device，实现 Dock host 与 DCIN 充电并行工作。补丁可在由 REST API 重建的父提交文件内容上干净应用。

## 补丁内容

```diff
Subject: [PATCH] [项目代号][TaskID]119595[Description]When the dock is connected to the host, the charging function will be affected[owner][同事]

---
diff --git a/kernel_platform/msm-kernel/drivers/power/supply/qcom/smb5-lib.c b/kernel_platform/msm-kernel/drivers/power/supply/qcom/smb5-lib.c
index 518be0b..eb46a97 100644
--- a/kernel_platform/msm-kernel/drivers/power/supply/qcom/smb5-lib.c
+++ b/kernel_platform/msm-kernel/drivers/power/supply/qcom/smb5-lib.c
@@ -8880,6 +8880,22 @@
     }
 }
 
+/*
+ * Dock USB host role only: notify USB controller via extcon, do NOT enable
+ * charger OTG_EN. Hub VBUS is supplied by pogo_en_hub_vcc_gpio (EN4), so DCIN
+ * charging can work at the same time. Do not call smblib_notify_usb_host() here.
+ */
+static void smblib_pogo_notify_usb_host_role(struct smb_charger *chg, bool enable)
+{
+	if (enable)
+		smblib_notify_extcon_props(chg, EXTCON_USB_HOST);
+
+	pr_info("[POGO_USB]%s: EXTCON_USB_HOST=%d (no OTG_EN, hub via EN4)\n",
+		__func__, enable);
+
+	extcon_set_state_sync(chg->extcon, EXTCON_USB_HOST, enable);
+}
+
 void smblib_pogo_irq_back_det_delayed_work(struct work_struct *work)
 {
     struct smb_charger *chg = container_of(work, struct smb_charger,
@@ -8908,6 +8924,13 @@
             pr_info("[POGO_USB]%s:%d set pogo_sw_uart_usb_gpio=1\n",
                      __func__, __LINE__);
         }
+
+		/* Enable EN2 (pogo_en_typea_5v_gpio) */
+        if (gpio_is_valid(chg->pogo_en_typea_5v_gpio)) {
+            gpio_direction_output(chg->pogo_en_typea_5v_gpio, 1);
+            pr_info("[POGO_USB]%s:%d set pogo_en_typea_5v_gpio=1 (EN2 on)\n",
+                     __func__, __LINE__);
+        }
         
         /* Enable EN4 (pogo_en_hub_vcc_gpio) */
         if (gpio_is_valid(chg->pogo_en_hub_vcc_gpio)) {
@@ -8916,8 +8939,8 @@
                      __func__, __LINE__);
         }
 
-        /* Switch usb mode to host - notify USB driver via extcon */
-        smblib_notify_usb_host(chg, true);
+        /* USB2.0 host via hub; keep OTG_EN off for DCIN charge */
+        smblib_pogo_notify_usb_host_role(chg, true);
         pr_info("[POGO_USB]%s:%d switched USB to host mode (dock inserted)\n",
                  __func__, __LINE__);
     } else {
@@ -8957,8 +8980,7 @@
             pr_info("[POGO_USB]%s:%d set pogo_en_hub_vcc_gpio=0 (EN4 off)\n", __func__, __LINE__);
         }
 
-        /* Switch USB mode back to device mode - notify USB driver */
-        smblib_notify_usb_host(chg, false);
+        smblib_pogo_notify_usb_host_role(chg, false);
         pr_info("[POGO_USB]%s:%d switched USB back to device mode (dock removed)\n",
                  __func__, __LINE__);
     }
```

## 补丁验证

✅ **可干净应用**（REST API 父提交文件内容重建验证）

## 源码归档

已归档到 [[01.驱动文档/Charger/Qualcomm/SM6225-A14/91.源码与补丁索引/|91.源码与补丁索引/]]：

| 目录 | 内容 |
|:---|:---|
| `kernel_driver/` | smb5-lib.c（合并后版本，242KB） |
| `patches/` | 195273.patch |
| `modified_history.md` | 修改历史摘要 |

## 引用文件索引

| 序号 | 文件 | 完整路径（源码树内） | 说明 |
|:---|:---|:---|:---|
| 1 | [[01.驱动文档/Charger/Qualcomm/SM6225-A14/91.源码与补丁索引/kernel_driver/smb5-lib.c\|smb5-lib.c]] | `kernel_platform/msm-kernel/drivers/power/supply/qcom/smb5-lib.c` | Dock host 仅 extcon 通知、不开 OTG_EN（+26/-4） |

---

_Author: wangguanran_