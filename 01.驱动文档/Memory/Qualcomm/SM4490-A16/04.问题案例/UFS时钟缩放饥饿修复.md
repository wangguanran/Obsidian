# UFS 时钟缩放饥饿修复（HOSINGLOBAL 2.2）

> **模块**: Memory | **厂商**: Qualcomm | **芯片**: [项目代号] (SM4490/parrot)
> **平台**: SM4490-A16 (LA.VENDOR.1.0.R1) | **类型**: Bug
> **Change**: #197041 | **作者**: [同事] | **状态**: MERGED

---

## 基本信息

| Change | 项目 | 分支 | 作者 | 类型 | 芯片 | 平台 | 模块 |
|--------|------|------|------|------|------|------|------|
| #197041 | LA.VENDOR.1.0.R1 | master_Snapdragon_Premium_High_2021.SPF.2.0.2_[项目代号] | [同事] | Bug | SM4490 (parrot) | SM4490-A16 | Memory/UFS |

## 现象

flashval 测试 **2.1 / 2.2 / 10.1 / 10.2**（要求 clock scale 切换次数 ≥ 1000）在 HOSINGLOBAL **UFS 2.2** 器件（型号 HBN1901280CHBC，manufacturer_id `0x0CD6`）上**失败**：实际只观察到 **28~50 次**时钟缩放切换；而 UFS 3.x 器件可以通过。

## 环境

- 芯片：Qualcomm SM4490（parrot）
- 平台：SM4490-A16（LA.VENDOR.1.0.R1，[项目代号] 分支）
- 内核：msm-kernel（GKI + `drivers/scsi/ufs`）
- 器件：HOSINGLOBAL UFS 2.2（JEDEC id `0x0CD6`），对比器件 UFS 3.x

## 根因分析

1. **simple_ondemand 降频阈值过严**：`ufs_qcom_config_scaling_param()` 固定 `downdifferential = 65`，而 `upthreshold = 70`。simple_ondemand 的规则是：当 busy% 高于 `(upthreshold - downdifferential) = 5` 时**保持当前频率**——即只有当 busy% ≤ 5 时才会降频。UFS 2.2 器件每笔 IO 延迟约为 3.x 器件的 2 倍，IO 突发结束后 busy% 仍停留在 10~47（ftrace 实测），导致 core_clk 长期锁在最高频，**时钟缩放被饿死**（无法降频，切换次数上不去）。
2. **JEDEC id 未匹配 quirk**：`0x0CD6` 不匹配 `UFS_VENDOR_SKHYNIX (0x1AD)`，因此 `ufs_qcom_dev_fixups[]` 中的 `UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM` 不会生效；idle 时 host 立即 suspend，进一步**抹掉了剩余的降频窗口**。

## 处理方案

1. **放宽降频阈值（仅限 UFS 2.x）**：在 `ufs_qcom_config_scaling_param()` 中读取 `host->limit_phy_submode`，为 **0（UFS 2.x）** 时将 `downdifferential` 从 65 放宽到 **45**（降频阈值从 busy% ≤ 5 提升到 busy% ≤ 25），给 UFS 2.x 器件留出降频空间；UFS 3.x（limit_phy_submode != 0）保持默认 65，**不受影响**。
2. **补齐 HOSINGLOBAL quirk**：在 `ufs_quirks.h` 新增 `UFS_VENDOR_HOSINGLOBAL 0x0CD6`，并在 `ufs_qcom_dev_fixups[]` 注册 `UFS_FIX(UFS_VENDOR_HOSINGLOBAL, UFS_ANY_MODEL, UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM)`，使 `0x0CD6` 器件获得与其他 Hynix 器件相同的 delay-before-LPM 处理，保留 idle 后的降频窗口。

## 修改文件清单

| # | 文件 | 改动 | 说明 |
|---|------|------|------|
| 1 | [[01.驱动文档/Memory/Qualcomm/SM4490-A16/91.源码与补丁索引/kernel_driver/drivers/scsi/ufs/ufs-qcom.c\|ufs-qcom.c]] | +15/-1 | 降频阈值 UFS 2.x 放宽 + HOSINGLOBAL quirk 注册 |
| 2 | [[01.驱动文档/Memory/Qualcomm/SM4490-A16/91.源码与补丁索引/kernel_driver/drivers/scsi/ufs/ufs_quirks.h\|ufs_quirks.h]] | +1 | 定义 UFS_VENDOR_HOSINGLOBAL 0x0CD6 |

## 配置方式

无 DTS/defconfig 改动，全部为内核代码级调整：

```c
/* ufs-qcom.c: ufs_qcom_config_scaling_param() */
host = ufshcd_get_variant(hba);
if (host && !host->limit_phy_submode)   /* UFS 2.x only */
    d->downdifferential = 45;           /* 降频阈值 busy% <= 25 */
else
    d->downdifferential = 65;           /* UFS 3.x 保持默认 */

/* ufs_quirks.h: 新增厂商 ID */
#define UFS_VENDOR_HOSINGLOBAL 0x0CD6

/* ufs-qcom.c: fixups 表新增条目 */
UFS_FIX(UFS_VENDOR_HOSINGLOBAL, UFS_ANY_MODEL,
        UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM),
```

注意：`host->limit_phy_submode` 为 0 即当前 PHY 工作在 UFS 2.x 模式（HS-G2 等），非 0 为 3.x 模式（HS-G4）。

## 验证方式

1. 重跑 flashval test 2.1 / 2.2 / 10.1 / 10.2：
   ```bash
   # 预期：clock scale 切换次数 >= 1000（此前仅 28~50）
   ```
2. 观察 UFS devfreq 频率变化：
   ```bash
   cat /sys/class/devfreq/1d84000.ufshc/trans_stat    # 各频率停留时间分布
   # 预期：低频率档位有实际停留（有降频发生）
   ```
3. UFS 2.2 器件 IOPS/读写带宽回归：确认放宽阈值不影响峰值性能；
4. UFS 3.x 器件回归：确认默认参数不变、无行为变化。

## 结论

通过「UFS 2.x 放宽降频阈值 + 补齐 HOSINGLOBAL LPM quirk」双管齐下，恢复了 UFS 2.2 器件的时钟缩放能力，flashval 中时钟缩放切换次数指标由 28~50 提升到通过标准（≥1000）。改动仅影响 UFS 2.x 器件，UFS 3.x 行为不变。

## 补丁内容

```diff
Subject: [PATCH] [项目代号][96242][UFS]Fix clock scaling starvation on high-latency HOSINGLOBAL UFS 2.x device

[Root Cause]
flashval test 2.1/2.2/10.1/10.2 (clock scale transitions >= 1000) fail on
the HOSINGLOBAL UFS 2.2 device (HBN1901280CHBC, manufacturer_id 0x0CD6):
only 28~50 transitions observed, while the UFS 3.x device passes.
1. simple_ondemand keeps the current frequency while busy% >
   (upthreshold - downdifferential) = 70 - 65 = 5, i.e. the clock can
   only scale DOWN when busy% <= 5. The UFS 2.2 part has ~2x per-IO
   latency, so after each IO burst its busy% lingers at 10~47, pinning
   core_clk at max and starving clock scaling.
2. JEDEC id 0x0CD6 does not match UFS_VENDOR_SKHYNIX (0x1AD) in
   ufs_qcom_dev_fixups[], so UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM is not
   applied and the host suspends immediately when idle, which further
   removes the remaining scale-down windows.

[Solution]
1. In ufs_qcom_config_scaling_param(), relax downdifferential from 65
   to 45 for UFS 2.x devices only (host->limit_phy_submode == 0),
   raising the scale-down threshold from busy% <= 5 to busy% <= 25.
   UFS 3.x devices keep the default and are unaffected.
2. Add UFS_FIX(UFS_VENDOR_HOSINGLOBAL, UFS_ANY_MODEL,
   UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM) so 0x0CD6 parts get the same
   delay-before-LPM treatment as other SKHynix parts.

---

diff --git a/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs-qcom.c b/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs-qcom.c
index ff57b9e..b145883 100644
--- a/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs-qcom.c
+++ b/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs-qcom.c
@@ -4481,6 +4481,7 @@
 					  void *data)
 {
 	static struct devfreq_simple_ondemand_data *d;
+	struct ufs_qcom_host *host;
 
 	if (!data)
 		return;
@@ -4489,7 +4490,21 @@
 	p->polling_ms = 60;
 	p->timer = DEVFREQ_TIMER_DELAYED;
 	d->upthreshold = 70;
-	d->downdifferential = 65;
+	/*
+	 * simple_ondemand keeps the current frequency while busy% is
+	 * above (upthreshold - downdifferential), so the scale-down
+	 * threshold is busy% <= 5 with the default 65.  UFS 2.x parts
+	 * with higher per-IO latency can hardly reach that after an IO
+	 * burst: their busy% lingers at 10~47 (ftrace measured), pinning
+	 * core_clk at max and starving clock scaling.  Relax to 45
+	 * (threshold busy% <= 25) for UFS 2.x only; UFS 3.x keeps the
+	 * default.
+	 */
+	host = ufshcd_get_variant(hba);
+	if (host && !host->limit_phy_submode)
+		d->downdifferential = 45;
+	else
+		d->downdifferential = 65;
 }
 
 static struct ufs_dev_fix ufs_qcom_dev_fixups[] = {
@@ -4500,6 +4515,14 @@
 		UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM),
 	UFS_FIX(UFS_VENDOR_SKHYNIX, UFS_ANY_MODEL,
 		UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM),
+	/*
+	 * Hynix UFS 2.2 parts populated on parrot report JEDEC id 0x0CD6,
+	 * which does not match UFS_VENDOR_SKHYNIX (0x1AD).  Give them the
+	 * same delay-before-LPM treatment, otherwise the host suspends
+	 * too eagerly when idle and clock scaling windows are lost.
+	 */
+	UFS_FIX(UFS_VENDOR_HOSINGLOBAL, UFS_ANY_MODEL,
+		UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM),
 	UFS_FIX(UFS_VENDOR_WDC, UFS_ANY_MODEL,
 		UFS_DEVICE_QUIRK_HOST_PA_TACTIVATE),
 	END_FIX
diff --git a/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs_quirks.h b/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs_quirks.h
index 59ce2db..c8297be 100644
--- a/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs_quirks.h
+++ b/kernel_platform/msm-kernel/drivers/scsi/ufs/ufs_quirks.h
@@ -17,6 +17,7 @@
 #define UFS_VENDOR_SKHYNIX     0x1AD
 #define UFS_VENDOR_TOSHIBA     0x198
 #define UFS_VENDOR_WDC         0x145
+#define UFS_VENDOR_HOSINGLOBAL 0x0CD6
 
 /**
  * ufs_dev_fix - ufs device quirk info

```

## 补丁验证

| 补丁 | 验证方式 | 结果 |
|------|----------|------|
| #197041 | 134 源码树父提交文件提取 + `patch --dry-run` | ✅ 可干净应用 |

## 源码归档

| 归档目录 | 文件 | 说明 |
|----------|------|------|
| kernel_driver/ | drivers/scsi/ufs/ufs-qcom.c | 合并后版本 |
| kernel_driver/ | drivers/scsi/ufs/ufs_quirks.h | 合并后版本 |
| patches/ | 197041.patch | 本变更补丁 |

## 引用文件索引

| 文件 | 路径 | 说明 |
|------|------|------|
| [[01.驱动文档/Memory/Qualcomm/SM4490-A16/91.源码与补丁索引/kernel_driver/drivers/scsi/ufs/ufs-qcom.c\|ufs-qcom.c]] | `kernel_platform/msm-kernel/drivers/scsi/ufs/ufs-qcom.c` | 时钟缩放参数 + quirk 注册 |
| [[01.驱动文档/Memory/Qualcomm/SM4490-A16/91.源码与补丁索引/kernel_driver/drivers/scsi/ufs/ufs_quirks.h\|ufs_quirks.h]] | `kernel_platform/msm-kernel/drivers/scsi/ufs/ufs_quirks.h` | UFS_VENDOR_HOSINGLOBAL 定义 |
| [[01.驱动文档/Memory/Qualcomm/SM4490-A16/91.源码与补丁索引/patches/197041.patch\|197041.patch]] | `patches/197041.patch` | 本变更补丁 |

---

_Author: wangguanran_
