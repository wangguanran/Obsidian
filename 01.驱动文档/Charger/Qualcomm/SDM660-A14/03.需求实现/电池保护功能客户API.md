# 电池保护功能客户 API（Max SoC limit）

> **模块**: Charger | **厂商**: Qualcomm | **芯片**: SDM660
> **平台**: SDM660-A14 | **类型**: 需求
> **Change**: #195420 | **作者**: qianyiping | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #195420 |
| 项目 | LA.UM.10.2.1 |
| 分支 | master_MT568 |
| 作者 | qianyiping |
| 类型 | 需求（新增客户 API） |
| 芯片 | Qualcomm SDM660 |
| 平台 | SDM660-A14（MT578，Android 14） |
| 模块 | Charger（qpnp-smb2 / smb-lib 充电管理） |
| 提交标题 | `[MT578][120466][charge][Description]Add customer-facing API for Battery Protection (Max SoC limit)` |
| 任务 | Task 120466（EAD-8809） |

## 需求描述

新增**电池保护（Battery Protection）**客户 API：允许客户设置充电上限（Max SoC limit），当电池电量达到上限后自动停止充电（或限流），保护电池、延长寿命。同时补充 `USB_CHARGER_PRESENT` uevent 通知机制。

## 方案

### 1. power_supply 属性扩展（内核公共层）

- `include/linux/power_supply.h`：新增 `POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER`（置于 SERIAL_NUMBER 之后，不破坏既有 ABI）
- `power_supply_sysfs.c`：注册 `restricted_charging_user` 属性，客户可通过 sysfs 读写

### 2. qpnp-smb2 回调接入

- `qpnp-smb2.c`：属性表加入 `POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER`，get 回调分发到 `smblib_get_restricted_charging_capacity_user()`

### 3. smb-lib 核心逻辑（+114 行）

- 新增字段：`restricted_charging_capacity_user`、`usb_charger_present_uevent_state`、`restricted_charging_work_user`（delayed_work）、`usb_charger_present_uevent_work`（work_struct）
- `smblib_restricted_charging_user(chg, val)`：设置上限并调度 100ms 后工作队列
- `smblib_restricted_charging_work_user()`：60s 周期轮询电池容量：
  - `capacity > limit`：`chg_disable_votable` (USER_VOTER, true) + `usb_icl_votable` 0 + `fcc_votable` 0（停止充电）
  - `capacity == limit`：恢复充电使能，USB 输入限 500mA
  - `capacity < limit`：全部解除 vote（正常充电）
- `smblib_restore_all_charging_paras_user()`：退出受限模式恢复所有参数
- `smblib_get_restricted_charging_capacity_user()`：读取当前上限
- `smblib_notify_usb_charger_present()`：USB 插入/拔出事件通过 workqueue 异步发送 `USB_CHARGER_PRESENT` uevent（避免 chg->lock 上下文直接发 uevent）
- `smblib_init/deinit`：INIT + cancel 新增 work

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/include/linux/power_supply.h\|include/linux/power_supply.h]] | +2/-0 | 新增 RESTRICTED_CHARGING_USER 属性 |
| [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/power_supply_sysfs.c\|power_supply_sysfs.c]] | +2/-0 | sysfs 属性注册 |
| [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/qpnp-smb2.c\|qpnp-smb2.c]] | +19/-1 | get 回调接入 |
| [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.c\|smb-lib.c]] | +114/-0 | 受限充电核心逻辑 + uevent |
| [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.h\|smb-lib.h]] | +12/-1 | 结构体字段 + 接口声明 |

## 配置方式

### 客户 API 使用

```bash
# 设置充电上限（如 80%）
echo 80 > /sys/class/power_supply/battery/restricted_charging_user

# 查询当前上限
cat /sys/class/power_supply/battery/restricted_charging_user

# 取消限制（恢复满充）
echo 0 > /sys/class/power_supply/battery/restricted_charging_user
```

### Kernel config

- 无需新 CONFIG：smb-lib / qpnp-smb2 为平台既有模块（SMB 充电框架）

## 验证方式

### 验证命令

```bash
# 1. 设置上限 50%
echo 50 > /sys/class/power_supply/battery/restricted_charging_user

# 2. 观察充电状态（容量 >50% 后应停止充电）
cat /sys/class/power_supply/battery/status        # 应为 Not charging
cat /sys/class/power_supply/usb/input_current_limited

# 3. USB 插入 uevent
dmesg | grep -i "usb_charger_present"

# 4. 取消限制
echo 0 > /sys/class/power_supply/battery/restricted_charging_user
cat /sys/class/power_supply/battery/status        # 恢复 Charging
```

### 预期结果

1. 容量 > 上限：停止充电（status=Not charging）
2. 容量 == 上限：恢复使能但限流 500mA
3. 容量 < 上限：正常充电
4. USB 插拔产生 `USB_CHARGER_PRESENT=1/0` uevent
5. 取消限制后恢复全部充电参数

### 实际结果

- 已 MERGED（#195420）

## 补丁内容

```diff
From 403fe50f6e391721ebd704a4041f807a0b2a8354 Mon Sep 17 00:00:00 2001

---

diff --git a/kernel/msm-4.19/drivers/power/supply/power_supply_sysfs.c b/kernel/msm-4.19/drivers/power/supply/power_supply_sysfs.c
index 1616212..e053de1 100644
--- a/kernel/msm-4.19/drivers/power/supply/power_supply_sysfs.c
+++ b/kernel/msm-4.19/drivers/power/supply/power_supply_sysfs.c
@@ -500,6 +500,8 @@
 	POWER_SUPPLY_ATTR(battery_type),
 	POWER_SUPPLY_ATTR(cycle_counts),
 	POWER_SUPPLY_ATTR(serial_number),
+	//EAD-8809: Add restricted_charging_user for battery SoC limit
+	POWER_SUPPLY_ATTR(restricted_charging_user),
 };
 
 static struct attribute *
diff --git a/kernel/msm-4.19/drivers/power/supply/qcom/qpnp-smb2.c b/kernel/msm-4.19/drivers/power/supply/qcom/qpnp-smb2.c
index 785c1cf..2a3159e 100755
--- a/kernel/msm-4.19/drivers/power/supply/qcom/qpnp-smb2.c
+++ b/kernel/msm-4.19/drivers/power/supply/qcom/qpnp-smb2.c
@@ -1517,6 +1517,7 @@
 	POWER_SUPPLY_PROP_CYCLE_COUNT,
 	POWER_SUPPLY_PROP_FCC_STEPPER_ENABLE,
 	POWER_SUPPLY_PROP_BATTERY_CHARGING_ENABLED,
+	POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER,
 };
 extern int elohub_hardreset_cnt;
 static int smb2_batt_get_prop(struct power_supply *psy,
@@ -1692,7 +1693,10 @@
 		break;
 	case POWER_SUPPLY_PROP_FCC_STEPPER_ENABLE:
 		val->intval = chg->fcc_stepper_enable;
-		break;	
+		break;
+	case POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER:
+		smblib_get_restricted_charging_capacity_user(chg, val);
+		break;
 	default:
 		pr_err("batt power supply prop %d not supported\n", psp);
 		return -EINVAL;
@@ -1818,6 +1822,17 @@
 	case POWER_SUPPLY_PROP_TEMP:
 		chg->fake_battery_temp = val->intval;
 		power_supply_changed(chg->batt_psy);
+		break;
+	case POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER:
+		if (val->intval == 0) {
+			smblib_restore_all_charging_paras_user(chg);
+		} else if (val->intval > 0 && val->intval < 101) {
+			smblib_restricted_charging_user(chg, val->intval);
+		} else {
+			pr_err("unsupported restricted_charging_user range\n");
+			rc = -EINVAL;
+		}
+		break;
 	default:
 		rc = -EINVAL;
 	}
@@ -1842,6 +1857,7 @@
 	case POWER_SUPPLY_PROP_DIE_HEALTH:
 	case POWER_SUPPLY_PROP_TEMP:
 	case POWER_SUPPLY_PROP_BATTERY_CHARGING_ENABLED:
+	case POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER:
 		return 1;
 	default:
 		break;
@@ -2156,6 +2172,8 @@
 	if (chip->dt.no_battery)
 		chg->fake_capacity = 50;
 
+	chg->restricted_charging_capacity_user = 0;
+
 	if (chg->batt_profile_fcc_ua < 0)
 		smblib_get_charge_param(chg, &chg->param.fcc,
 				&chg->batt_profile_fcc_ua);
diff --git a/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.c b/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.c
index 60626fd..33744cb 100755
--- a/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.c
+++ b/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.c
@@ -3790,6 +3790,30 @@
 	return IRQ_HANDLED;
 }
 
+/* Send USB_CHARGER_PRESENT via workqueue to avoid uevent under chg->lock */
+static void smblib_usb_charger_present_uevent_work(struct work_struct *work)
+{
+	struct smb_charger *chg = container_of(work, struct smb_charger,
+					usb_charger_present_uevent_work);
+	char mystring[32] = {0};
+	char *envp[2] = { mystring, NULL };
+
+	if (!chg->dev)
+		return;
+
+	snprintf(mystring, sizeof(mystring), "USB_CHARGER_PRESENT=%d",
+		 chg->usb_charger_present_uevent_state);
+	kobject_uevent_env(&chg->dev->kobj, KOBJ_CHANGE, envp);
+	pr_info("[smblib] %s Event Sent: %s\n", __func__, mystring);
+}
+
+static void smblib_notify_usb_charger_present(struct smb_charger *chg,
+					     bool present)
+{
+	chg->usb_charger_present_uevent_state = present ? 1 : 0;
+	schedule_work(&chg->usb_charger_present_uevent_work);
+}
+
 static void smblib_micro_usb_plugin(struct smb_charger *chg, bool vbus_rising)
 {
 	if (vbus_rising) {
@@ -3846,6 +3870,8 @@
 		}
 	}
 
+	smblib_notify_usb_charger_present(chg, vbus_rising);
+
 	power_supply_changed(chg->usb_psy);
 	smblib_dbg(chg, PR_INTERRUPT, "IRQ: usbin-plugin %s\n",
 					vbus_rising ? "attached" : "detached");
@@ -3924,6 +3950,8 @@
 	if (chg->connector_type == POWER_SUPPLY_CONNECTOR_MICRO_USB)
 		smblib_micro_usb_plugin(chg, vbus_rising);
 
+	smblib_notify_usb_charger_present(chg, vbus_rising);
+
 	power_supply_changed(chg->usb_psy);
 	smblib_dbg(chg, PR_INTERRUPT, "IRQ: usbin-plugin %s\n",
 					vbus_rising ? "attached" : "detached");
@@ -5633,6 +5661,87 @@
 		iio_channel_release(chg->iio.batt_i_chan);
 }
 
+//EAD-8809: restricted_charging_user for battery SoC limit
+static void smblib_restricted_charging_work_user(struct work_struct *work)
+{
+	struct smb_charger *chg = container_of(work, struct smb_charger,
+					       restricted_charging_work_user.work);
+	union power_supply_propval pval = {0, };
+	int battery_capacity = 0;
+	int rc;
+
+	if (!chg->restricted_charging_capacity_user)
+		return;
+
+	if (!chg->batt_psy)
+		goto reschedule;
+
+	rc = power_supply_get_property(chg->batt_psy,
+				       POWER_SUPPLY_PROP_CAPACITY, &pval);
+	if (rc < 0) {
+		pr_err("restricted_charging_user: get capacity failed rc=%d\n",
+		       rc);
+		goto reschedule;
+	}
+	battery_capacity = pval.intval;
+
+	pr_info("restricted_charging_user: battery_capacity=%d, restricted_charging_capacity_user=%d\n",
+		battery_capacity, chg->restricted_charging_capacity_user);
+
+	if (battery_capacity > chg->restricted_charging_capacity_user) {
+		pr_info("restricted_charging_user: disable charging and suspend usb\n");
+		vote(chg->chg_disable_votable, USER_VOTER, true, 0);
+		vote(chg->usb_icl_votable, USER_VOTER, true, 0);
+		vote(chg->fcc_votable, USER_VOTER, true, 0);
+	} else if (battery_capacity == chg->restricted_charging_capacity_user) {
+		pr_info("restricted_charging_user: disable charging and set usb input current to 500mA\n");
+		vote(chg->chg_disable_votable, USER_VOTER, false, 0);
+		vote(chg->usb_icl_votable, USER_VOTER, true, USBIN_500MA);
+		vote(chg->fcc_votable, USER_VOTER, true, 0);
+	} else {
+		pr_info("restricted_charging_user: enable charging\n");
+		vote(chg->chg_disable_votable, USER_VOTER, false, 0);
+		vote(chg->usb_icl_votable, USER_VOTER, false, 0);
+		vote(chg->fcc_votable, USER_VOTER, false, 0);
+	}
+	rerun_election(chg->usb_icl_votable);
+	rerun_election(chg->fcc_votable);
+	rerun_election(chg->chg_disable_votable);
+
+reschedule:
+	if (chg->restricted_charging_capacity_user > 0)
+		schedule_delayed_work(&chg->restricted_charging_work_user,
+				      msecs_to_jiffies(60000));
+}
+
+void smblib_restricted_charging_user(struct smb_charger *chg, int val)
+{
+	pr_info("restricted_charging_user: User setting to enter restricted charging mode\n");
+	cancel_delayed_work_sync(&chg->restricted_charging_work_user);
+	chg->restricted_charging_capacity_user = val;
+	schedule_delayed_work(&chg->restricted_charging_work_user,
+			      msecs_to_jiffies(100));
+}
+
+void smblib_restore_all_charging_paras_user(struct smb_charger *chg)
+{
+	pr_info("restricted_charging_user: Exit restricted charging!\n");
+	chg->restricted_charging_capacity_user = 0;
+	cancel_delayed_work_sync(&chg->restricted_charging_work_user);
+	vote(chg->chg_disable_votable, USER_VOTER, false, 0);
+	vote(chg->usb_icl_votable, USER_VOTER, false, 0);
+	vote(chg->fcc_votable, USER_VOTER, false, 0);
+	rerun_election(chg->usb_icl_votable);
+	rerun_election(chg->fcc_votable);
+	rerun_election(chg->chg_disable_votable);
+}
+
+void smblib_get_restricted_charging_capacity_user(struct smb_charger *chg,
+					union power_supply_propval *val)
+{
+	val->intval = chg->restricted_charging_capacity_user;
+}
+
 int smblib_init(struct smb_charger *chg)
 {
 	int rc = 0;
@@ -5654,6 +5763,9 @@
 	INIT_WORK(&chg->legacy_detection_work, smblib_legacy_detection_work);
 	INIT_DELAYED_WORK(&chg->uusb_otg_work, smblib_uusb_otg_work);
 	INIT_DELAYED_WORK(&chg->bb_removal_work, smblib_bb_removal_work);
+	INIT_DELAYED_WORK(&chg->restricted_charging_work_user, smblib_restricted_charging_work_user);
+	INIT_WORK(&chg->usb_charger_present_uevent_work,
+		  smblib_usb_charger_present_uevent_work);
 	chg->fake_capacity = -EINVAL;
 	chg->fake_input_current_limited = -EINVAL;
 	chg->fake_batt_status = -EINVAL;
@@ -5735,6 +5847,8 @@
 		cancel_work_sync(&chg->legacy_detection_work);
 		cancel_delayed_work_sync(&chg->uusb_otg_work);
 		cancel_delayed_work_sync(&chg->bb_removal_work);
+		cancel_delayed_work_sync(&chg->restricted_charging_work_user);
+		cancel_work_sync(&chg->usb_charger_present_uevent_work);
 		power_supply_unreg_notifier(&chg->nb);
 		smblib_destroy_votables(chg);
 		qcom_step_chg_deinit();
diff --git a/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.h b/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.h
index f623c42..1da05cf 100755
--- a/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.h
+++ b/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.h
@@ -314,7 +314,9 @@
 	struct delayed_work	bb_removal_work;
 	struct delayed_work	pogo_pin_work;
 	struct delayed_work pogo_dcin_work;
-	struct delayed_work pogo_6_8dcin_work;	
+	struct delayed_work pogo_6_8dcin_work;
+	struct delayed_work restricted_charging_work_user;
+	struct work_struct	usb_charger_present_uevent_work;
 	/* cached status */
 	int			voltage_min_uv;
 	int			voltage_max_uv;
@@ -408,6 +410,9 @@
 	bool 		bat_recharge_flag;
 	int			last_typec_mode;
 	#endif
+
+	int			restricted_charging_capacity_user;
+	int			usb_charger_present_uevent_state;
 };
 
 int smblib_read(struct smb_charger *chg, u16 addr, u8 *val);
@@ -587,6 +592,12 @@
 int smblib_toggle_stat(struct smb_charger *chg, int reset);
 int smblib_force_ufp(struct smb_charger *chg);
 
+//EAD-8809: restricted_charging_user for battery SoC limit
+void smblib_restricted_charging_user(struct smb_charger *chg, int val);
+void smblib_restore_all_charging_paras_user(struct smb_charger *chg);
+void smblib_get_restricted_charging_capacity_user(struct smb_charger *chg,
+					union power_supply_propval *val);
+
 int smblib_init(struct smb_charger *chg);
 int smblib_deinit(struct smb_charger *chg);
 #endif /* __SMB2_CHARGER_H */
diff --git a/kernel/msm-4.19/include/linux/power_supply.h b/kernel/msm-4.19/include/linux/power_supply.h
index 624d6c0..3f4f4f1 100644
--- a/kernel/msm-4.19/include/linux/power_supply.h
+++ b/kernel/msm-4.19/include/linux/power_supply.h
@@ -386,6 +386,8 @@
 	 * MODEL_NAME and SERIAL_NUMBER. Don't add below SERIAL_NUMBER.
 	 */
 	POWER_SUPPLY_PROP_SERIAL_NUMBER,
+	//EAD-8809: Add restricted_charging_user for battery SoC limit
+	POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER,
 };
 
 enum power_supply_type {
```

## 补丁验证

| Change | 验证方式 | 结果 |
|--------|---------|------|
| #195420 | Gerrit REST 拉取 current revision 源码与补丁比对 | ✅ 与归档源码一致 |

> ⚠️ 项目 `LA.UM.10.2.1`（MT578/SDM660）在 134 服务器上无源码树，无法执行 `git apply --check`。已通过 Gerrit REST 拉取 current revision 源码与补丁逐一比对。源码归档自 Gerrit REST（非 134 源码树）。

## 源码归档

- [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/include/linux/power_supply.h|power_supply.h]]（Gerrit REST）
- [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.c|smb-lib.c]]（Gerrit REST）
- [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.h|smb-lib.h]]（Gerrit REST）
- 补丁：[[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/patches/195420.patch|195420.patch]]

## 引用文件索引

| 序号 | 文件 | 说明 |
|------|------|------|
| 1 | [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/include/linux/power_supply.h\|power_supply.h]] | 属性枚举扩展 |
| 2 | [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/power_supply_sysfs.c\|power_supply_sysfs.c]] | sysfs 属性 |
| 3 | [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/qpnp-smb2.c\|qpnp-smb2.c]] | SMB2 驱动回调 |
| 4 | [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.c\|smb-lib.c]] | 核心受限充电逻辑 |
| 5 | [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/kernel_driver/kernel/msm-4.19/drivers/power/supply/qcom/smb-lib.h\|smb-lib.h]] | 头文件 |
| 6 | [[01.驱动文档/Charger/Qualcomm/SDM660-A14/91.源码与补丁索引/patches/195420.patch\|195420.patch]] | 补丁文件 |

_Author: wangguanran_
