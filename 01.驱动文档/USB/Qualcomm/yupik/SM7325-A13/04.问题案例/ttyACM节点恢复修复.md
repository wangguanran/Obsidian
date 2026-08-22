# ttyACM 节点恢复（usbfs 释放接口后重新绑定 cdc_acm）

> **模块**: USB | **厂商**: Qualcomm | **芯片**: SM7325 (yupik)
> **平台**: SM7325-A13 (LA.UM.9.14.1) | **类型**: Bug
> **Change**: #196525 | **作者**: [同事] | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196525 |
| 项目 | LA.UM.9.14.1 |
| 分支 | master_LA.4.0_MC9371 |
| 作者 | [同事] |
| 类型 | Bug（usbfs 释放接口后 ttyACM 节点不恢复） |
| 芯片 | Qualcomm SM7325 (yupik) |
| 平台 | SM7325-A13（LA.UM.9.14.1，项目代号 MC9371） |
| 模块 | USB（cdc-acm 串口 / usbfs 用户态 USB） |
| 提交标题 | `[MC9371][121129][USB] Restore ttyACM after usbfs release[Owner][同事]` |
| 任务 | Task 121129 |

## 现象

应用对 WCH 芯片（`1a86:55da`，CH343 多接口）先 `open(/dev/ttyACM11)`，再通过 usbfs 对该设备执行 `DISCONNECT_CLAIM` 强制解绑内核驱动；`cdc_acm` 被拆掉后 `/dev/ttyACM11` 节点消失。应用关闭 usbfs handle 后节点**不会自动恢复**，后续再绑定失败，或 `open` 命中已 disconnected 的旧实例返回 `ENODEV`。

## 环境

| 项 | 内容 |
|----|------|
| 内核 | msm-5.4（`kernel/msm-5.4/drivers/usb/`） |
| 设备 | WCH CH343（`1a86:55da`，devpath `1.3`，接口 0x00/0x02），固定 minor 11/12 |
| 另一设备 | `040b:a6b`（devpath `1.2`，固定 minor 0） |
| 触发路径 | 用户态 open ttyACM → usbfs `DISCONNECT_CLAIM` → close usbfs fd |

## 关键日志

补丁新增的日志（预期输出）：

```
acm: reuse stale ttyACM11 (old instance disconnected)   # 固定 minor 回收
[acm_alloc_minor-XXX] alloc_minor:11 ifnum:0 pid:0x55da vid:0x1a86 devpath:1.3
```

问题现场典型表现（复现描述）：`releaseInterface` 只解绑 usbfs，不触发 `device_attach` 回 cdc_acm；用户态仍握着已删除的 tty fd 时，`tty_port` 不析构，固定 minor 11 一直被占用。

## 调用链

```
应用 open(/dev/ttyACM11) → cdc_acm probe → acm_alloc_minor(固定 11)
应用 ioctl(USBDEVFS_DISCONNECT_CLAIM) → usbfs_driver 绑定接口 → cdc_acm disconnect
    → tty_unregister_device()（tty 节点消失）
    → tty_port 因用户态 fd 未关闭而不析构 → acm->minor 一直占着 idr 槽 11
应用 close(usbfs fd) → releaseintf() → usb_driver_release_interface()
    → 旧逻辑结束（无重绑）→ ttyACM11 永久缺失
修复后 → if (!ps->ifclaimed) usbfs_rebind_interfaces(dev) → device_attach()
    → cdc_acm 重新 probe → acm_alloc_fixed_minor(11) 回收旧槽 → 节点恢复
```

## 根因分析

1. **usbfs 释放接口不触发内核驱动重绑**：`releaseintf()` 只调用 `usb_driver_release_interface()` 解绑 usbfs 驱动，不会把接口交还给原来的 `cdc_acm` 驱动；接口停在"无 driver"状态，设备节点不重建。
2. **cdc_acm 固定 minor 槽被 stale 实例长期占用**：`tty_unregister_device()` 之后，`acm->minor` 直到 `acm_port_destruct()`（依赖用户态关闭 fd 后的 tty 释放）才从 idr 中移除。用户态若一直握着 deleted fd，固定 minor（11/12）永远占着，重新 bind 时 `idr_alloc` 固定槽失败，或新 open 命中旧实例返回 `ENODEV`。
3. **多接口约束**：cdc_acm 是 control+data 双接口设备；若在兄弟接口仍被 usbfs 持有时立即重绑，`probe` 会因找不到配对接口返回 `-EBUSY`，所以重绑时机必须是该 handle 释放**全部**接口之后。

## 处理方案

1. **devio.c**：新增 `usbfs_rebind_interfaces()`——在 `releaseintf()` 中检测到该 usbfs handle 已无 claimed 接口（`!ps->ifclaimed`）时，遍历设备所有接口，对无 driver 的接口执行 `device_attach()`，让 `cdc_acm` 重新绑定。
2. **cdc-acm.c**：
   - 新增 `acm_alloc_fixed_minor()`：固定 minor 分配时若旧实例已 `disconnected`，先 `idr_remove` 回收槽位再分配；
   - `acm_release_minor()` 增加 `ACM_MINOR_INVALID` 保护；
   - `disconnect` 路径（`acm_disconnect` 内 `tty_unregister_device` 后）立即调用 `acm_release_minor()`，不再等用户态关 fd，保证固定 minor 尽快释放。

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/kernel_driver/kernel/msm-5.4/drivers/usb/class/cdc-acm.c\|cdc-acm.c]] | +85/-13 | 固定 minor 槽回收 + disconnect 立即释放 minor |
| [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/kernel_driver/kernel/msm-5.4/drivers/usb/core/devio.c\|devio.c]] | +34 | usbfs 释放全部接口后 device_attach 重绑 |

## 配置方式

无新增内核配置。固定 minor 分配规则沿用既有产品定制（按 VID/PID/devpath 命中）：

```c
if (usb_dev->descriptor.idProduct == 0x55da &&
    usb_dev->descriptor.idVendor == 0x1a86 &&
    strcmp(usb_dev->devpath, "1.3") == 0) {
    if (ifnum == 0x00)      minor = acm_alloc_fixed_minor(acm, 11);
    else if (ifnum == 0x02) minor = acm_alloc_fixed_minor(acm, 12);
    else                    minor = -ENODEV;
}
```

相关内核选项：`CONFIG_USB_ACM=y/m`、`CONFIG_USB_DEVICEFS`（usbfs `USBDEVFS_DISCONNECT_CLAIM`）。

## 验证方式

- 验证命令（复现+回归）：
  ```bash
  # 打开 ttyACM11 后强制解绑 cdc_acm
  python3 - <<'EOF'
  import usb.core, time
  dev = usb.core.find(idVendor=0x1a86, idProduct=0x55da)
  dev.detach_kernel_driver(0); dev.detach_kernel_driver(2)
  time.sleep(1)
  # 释放（close 句柄）后检查节点是否恢复
  EOF
  ls -l /dev/ttyACM*
  cat /proc/tty/drivers/usbserial  # 或 dmesg | grep acm
  ```
- 预期结果：close 后 `/dev/ttyACM11`、`/dev/ttyACM12` 重新出现；dmesg 见 `acm: reuse stale ttyACM11` 与 `alloc_minor:11`。
- 实际结果：本 Change 已合入主干；修复后节点恢复路径可用（需按项目整包编译验证）。

## 结论

根因是 usbfs 解绑后内核驱动不自动重绑 + cdc_acm 固定 minor 被 stale 实例长期占用。修复通过 usbfs 释放全部接口后 `device_attach()` 重绑，并在 cdc_acm 侧即时释放/回收固定 minor，闭环解决 ttyACM 节点消失问题。该修复对多接口 CDC/ACM（WCH CH343 等）类设备通用。

## 补丁内容

```diff
Subject: [PATCH] [MC9371][121129][USB] Restore ttyACM after usbfs release[Owner][同事]

[Root Cause]
	App 对 WCH(1a86:55da) 先 open(/dev/ttyACM11) 再 usbfs DISCONNECT_CLAIM，cdc_acm 被拆掉后节点消失。
	releaseInterface 只解绑 usbfs，不会 device_attach 回 cdc_acm。用户态若仍握着 deleted tty fd，tty_port 不析构，固定 minor 11 一直被占，再 bind 失败或 open 命中 disconnected 旧实例返回 ENODEV。
[Solution  ]
	usbfs 在该 handle 释放全部接口后 device_attach()，让 cdc_acm 重新绑定。
	cdc-acm 在 disconnect 时立即释放 minor；固定 minor 分配时回收已 disconnected 的旧 idr 槽，保证 ttyACM11/12 能重新注册。

---

diff --git a/kernel/msm-5.4/drivers/usb/class/cdc-acm.c b/kernel/msm-5.4/drivers/usb/class/cdc-acm.c
index 5ac7bc9..3be4cdc 100644
--- a/kernel/msm-5.4/drivers/usb/class/cdc-acm.c
+++ b/kernel/msm-5.4/drivers/usb/class/cdc-acm.c
@@ -83,6 +83,27 @@
 }
 
 /*
+ * Free a fixed minor even if a previous ACM instance still exists.
+ * Userspace may keep a deleted /dev/ttyACM* fd, which holds tty_port and
+ * would otherwise leak the minor until that process closes.
+ * Caller must hold acm_minors_lock.
+ */
+static int acm_alloc_fixed_minor(struct acm *acm, int want)
+{
+	struct acm *old = idr_find(&acm_minors, want);
+
+	if (old) {
+		if (!old->disconnected)
+			return -EBUSY;
+		pr_info("acm: reuse stale ttyACM%d (old instance disconnected)\n",
+			want);
+		idr_remove(&acm_minors, want);
+		old->minor = ACM_MINOR_INVALID;
+	}
+	return idr_alloc(&acm_minors, acm, want, want + 1, GFP_KERNEL);
+}
+
+/*
  * Try to find an available minor number and if found, associate it with 'acm'.
  */
 static int acm_alloc_minor(struct acm *acm)
@@ -90,23 +111,35 @@
 	int minor;
 	struct usb_interface *intf = acm->control;
 	struct usb_device *usb_dev = interface_to_usbdev(intf);
+	unsigned int ifnum = intf->cur_altsetting->desc.bInterfaceNumber;
 
 	mutex_lock(&acm_minors_lock);
-	if(usb_dev->descriptor.idProduct == 0x55da && usb_dev->descriptor.idVendor == 0x1a86 && strcmp(usb_dev->devpath,"1.3") == 0 ){
-		if(intf->cur_altsetting->desc.bInterfaceNumber == 0x00){
-			minor = idr_alloc(&acm_minors, acm, 11, ACM_TTY_MINORS, GFP_KERNEL);
-		}else if (intf->cur_altsetting->desc.bInterfaceNumber == 0x02){
-			minor = idr_alloc(&acm_minors, acm, 12, ACM_TTY_MINORS, GFP_KERNEL);
-		}else{
-			minor = idr_alloc(&acm_minors, acm, 13, ACM_TTY_MINORS, GFP_KERNEL);
-		}
-	}else if (usb_dev->descriptor.idProduct == 0xa6b && usb_dev->descriptor.idVendor == 0x40b && strcmp(usb_dev->devpath,"1.2") == 0) {
+	if (usb_dev->descriptor.idProduct == 0x55da &&
+	    usb_dev->descriptor.idVendor == 0x1a86 &&
+	    strcmp(usb_dev->devpath, "1.3") == 0) {
+		if (ifnum == 0x00)
+			minor = acm_alloc_fixed_minor(acm, 11);
+		else if (ifnum == 0x02)
+			minor = acm_alloc_fixed_minor(acm, 12);
+		else
+			minor = -ENODEV;
+	} else if (usb_dev->descriptor.idProduct == 0xa6b &&
+		   usb_dev->descriptor.idVendor == 0x40b &&
+		   strcmp(usb_dev->devpath, "1.2") == 0) {
 		minor = idr_alloc(&acm_minors, acm, 0, ACM_TTY_MINORS, GFP_KERNEL);
-	}
-	else{
+	} else {
 		minor = idr_alloc(&acm_minors, acm, 3, ACM_TTY_MINORS, GFP_KERNEL);
 	}
-	pr_info("[%s-%d] alloc_minor:%d pid:0x%x vid:0x%x devpath:%s \n",__func__,__LINE__,minor,usb_dev->descriptor.idProduct,usb_dev->descriptor.idVendor,usb_dev->devpath);
+	if (minor == -EBUSY)
+		pr_err("[%s-%d] fixed minor busy ifnum:%u pid:0x%x vid:0x%x devpath:%s\n",
+		       __func__, __LINE__, ifnum,
+		       usb_dev->descriptor.idProduct, usb_dev->descriptor.idVendor,
+		       usb_dev->devpath);
+	else
+		pr_info("[%s-%d] alloc_minor:%d ifnum:%u pid:0x%x vid:0x%x devpath:%s\n",
+			__func__, __LINE__, minor, ifnum,
+			usb_dev->descriptor.idProduct, usb_dev->descriptor.idVendor,
+			usb_dev->devpath);
 	mutex_unlock(&acm_minors_lock);
 
 	return minor;
@@ -116,7 +149,10 @@
 static void acm_release_minor(struct acm *acm)
 {
 	mutex_lock(&acm_minors_lock);
-	idr_remove(&acm_minors, acm->minor);
+	if (acm->minor != ACM_MINOR_INVALID) {
+		idr_remove(&acm_minors, acm->minor);
+		acm->minor = ACM_MINOR_INVALID;
+	}
 	mutex_unlock(&acm_minors_lock);
 }
 
@@ -1617,6 +1653,13 @@
 	cancel_delayed_work_sync(&acm->dwork);
 
 	tty_unregister_device(acm_tty_driver, acm->minor);
+	/*
+	 * Drop the minor immediately. A userspace process may keep the
+	 * deleted /dev/ttyACM* fd, which holds tty_port and would delay
+	 * acm_port_destruct() / idr_remove() until close(). Fixed-minor
+	 * devices (ttyACM11/12) cannot rebind until the number is free.
+	 */
+	acm_release_minor(acm);
 
 	usb_free_urb(acm->ctrlurb);
 	for (i = 0; i < ACM_NW; i++)
diff --git a/kernel/msm-5.4/drivers/usb/core/devio.c b/kernel/msm-5.4/drivers/usb/core/devio.c
index d037deb..2ec2fd8 100644
--- a/kernel/msm-5.4/drivers/usb/core/devio.c
+++ b/kernel/msm-5.4/drivers/usb/core/devio.c
@@ -787,6 +787,40 @@
 	return err;
 }
 
+/*
+ * Rebind kernel interface drivers after usbfs has released all interfaces
+ * claimed by one userspace handle. This is mainly needed for multi-interface
+ * CDC/ACM devices where userspace detached cdc_acm with force-claim and the
+ * tty nodes should come back after close().
+ *
+ * Must be called with the device lock held.
+ */
+static void usbfs_rebind_interfaces(struct usb_device *udev)
+{
+	struct usb_host_config *config;
+	int i;
+
+	if (!udev || udev->state != USB_STATE_CONFIGURED)
+		return;
+
+	config = udev->actconfig;
+	if (!config)
+		return;
+
+	for (i = 0; i < config->desc.bNumInterfaces; i++) {
+		struct usb_interface *intf = config->interface[i];
+		int rc;
+
+		if (!intf || intf->dev.driver)
+			continue;
+
+		rc = device_attach(&intf->dev);
+		if (rc < 0 && rc != -EPROBE_DEFER)
+			dev_dbg(&intf->dev,
+				"usbfs: rebind after release failed: %d\n", rc);
+	}
+}
+
 static int releaseintf(struct usb_dev_state *ps, unsigned int ifnum)
 {
 	struct usb_device *dev;
@@ -809,6 +843,15 @@
 		usb_driver_release_interface(&usbfs_driver, intf);
 		dev_set_uevent_suppress(&intf->dev, old_suppress);
 		err = 0;
+
+		/*
+		 * Only try to rebind after this userspace handle released all
+		 * of its claimed interfaces. This avoids cdc_acm probing while
+		 * its sibling data/control interface is still claimed, which
+		 * would otherwise fail with -EBUSY.
+		 */
+		if (!ps->ifclaimed)
+			usbfs_rebind_interfaces(dev);
 	}
 	return err;
 }
```

## 补丁验证

- 验证方式：134 服务器 REST 拉取补丁，对合并后源文件（cdc-acm.c / devio.c）执行 `git apply --check -R`（反向应用校验）
- 结果：✅ 可干净应用（补丁与已合入提交 diff 一致）

## 源码归档

| 内容 | 路径 | 说明 |
|------|------|------|
| kernel_driver/ | [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/kernel_driver/kernel/msm-5.4/drivers/usb/class/cdc-acm.c\|cdc-acm.c]] | cdc_acm 驱动（补丁后合并版本） |
| kernel_driver/ | [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/kernel_driver/kernel/msm-5.4/drivers/usb/core/devio.c\|devio.c]] | usbfs 实现（补丁后合并版本） |
| patches/ | [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/patches/196525.patch\|196525.patch]] | 完整补丁（已清隐私） |

## 引用文件索引

| 文件 | 完整路径 | 说明 |
|------|---------|------|
| [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/kernel_driver/kernel/msm-5.4/drivers/usb/class/cdc-acm.c\|cdc-acm.c]] | kernel/msm-5.4/drivers/usb/class/cdc-acm.c | cdc_acm 串口驱动 |
| [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/kernel_driver/kernel/msm-5.4/drivers/usb/core/devio.c\|devio.c]] | kernel/msm-5.4/drivers/usb/core/devio.c | usbfs 实现 |
| [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/91.源码与补丁索引/patches/196525.patch\|196525.patch]] | 91.源码与补丁索引/patches/ | 补丁（已清隐私） |

_Author: wangguanran_
