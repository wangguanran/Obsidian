# 分析：UFS 时钟缩放饥饿修复（HOSINGLOBAL 2.2）

**版本号：v1.0**
**对应文档：** UFS时钟缩放饥饿修复

## 技术背景

高通 UFS 主机控制器（ufs-qcom）通过 **devfreq + simple_ondemand governor** 做时钟缩放：按 IO 忙闲度（busy%）调整 core_clk 频率，省电的同时保证性能。

simple_ondemand 的参数语义：

- `upthreshold`（默认 70）：busy% 超过 70 升频；
- `downdifferential`（默认 65）：busy% 低于 `upthreshold - downdifferential`（即 70-65=5）才降频；**busy% 处于 5~70 之间时维持当前频率**。

因此降频阈值实际是 `busy% <= 5`。对每笔 IO 延迟高的器件（UFS 2.x），IO 突发结束后 busy% 衰减慢，长期高于 5%，core_clk 被钉在最高频——时钟缩放形同虚设。

## 代码改动分析

### ufs-qcom.c

1. `ufs_qcom_config_scaling_param()`：
   - 新增 `host = ufshcd_get_variant(hba)` 获取变体指针；
   - `if (host && !host->limit_phy_submode) d->downdifferential = 45;` —— `limit_phy_submode` 是 PHY 当前子模式（0 = UFS 2.x 速率族），以此区分器件代际，避免影响 UFS 3.x；
   - 降频阈值从 busy% ≤ 5 提升到 busy% ≤ 25（70-45），给高延迟器件留出降频窗口。
2. `ufs_qcom_dev_fixups[]`：
   - 新增 `UFS_FIX(UFS_VENDOR_HOSINGLOBAL, UFS_ANY_MODEL, UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM)`；
   - `UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM` 的作用是 idle 后延迟进入低功耗（让 devfreq 有机会在 LPM 前完成降频），此前只有 SKHynix(0x1AD)/Samsung 等厂商注册。

### ufs_quirks.h

- 新增 `#define UFS_VENDOR_HOSINGLOBAL 0x0CD6`，与现有厂商 ID 常量并列。

## 潜在风险

1. **阈值放宽影响功耗**：UFS 2.x 降频阈值放宽到 25%，高频停留时间缩短、低频停留增多，IOPS 峰值可能小幅下降；需确认业务场景无性能回退；
2. **厂商 ID 判定**：`0x0CD6` 与 SKHynix `0x1AD` 不同，按 JEDEC 标准 0xCD6 实际是 Hynix 系（旧编码），补丁按实际厂商命名 HOSINGLOBAL，后续若有同 ID 不同型号需在 fixups 中细化；
3. **limit_phy_submode 语义依赖**：该字段为私有字段，若未来内核重构其语义，需同步评估条件判断；
4. **影响面控制**：改动被限制在 UFS 2.x（submode=0），UFS 3.x 完全走默认分支，风险可控。

## 回归测试建议

- UFS 2.2 器件：flashval 2.1/2.2/10.1/10.2 通过（≥1000 次切换）；随机 4K 读写 IOPS、顺序读写带宽无显著回退；
- UFS 3.x 器件：跑同组用例，确认行为与改动前一致；
- 低电量/温升场景：确认降频阈值放宽不会造成过热（与温控策略叠加验证）；
- 长时间 idle→active 循环：观察 devfreq trans_stat 频率分布与 LPM 进入延迟。

## 与现有驱动架构的关系

- 属于 UFS 主机控制器驱动的调优类修复，与平台（SM4490/parrot）的存储性能验收强相关；
- 与同平台其他归档（Audio/LCD/Sensor 等）无代码交集；[项目代号] 平台目录下本单为 Memory 模块首篇归档。

_Author: wangguanran_
