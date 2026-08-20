# 分析：电池保护功能客户 API（Max SoC limit）

**版本号：v1.0**
**对应文档：** 电池保护功能客户API.md

## 技术背景

SDM660 平台（Android 14, 4.19 内核）使用 Qualcomm SMB 充电框架：`qpnp-smb2.c`（SMB2 驱动）→ `smb-lib.c`（共享充电库）→ `power_supply` 核心。客户需要电池保护功能：设置充电上限（Max SoC limit），防止电池长期满充老化。

**votable 机制**：Qualcomm 充电框架用 votable（投票器）管理各参数（charging current, FCC, input current limit），各模块以投票方式参与，`rerun_election` 重新选举最终值。

## 代码改动分析

### 属性链路（sysfs → 回调）

```
/sys/class/power_supply/battery/restricted_charging_user
  → power_supply.h: POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER（枚举）
  → power_supply_sysfs.c: POWER_SUPPLY_ATTR(restricted_charging_user)
  → qpnp-smb2.c: smb2_batt_get_prop 分发
  → smb-lib.c: smblib_get_restricted_charging_capacity_user
```

### 核心控制（smblib_restricted_charging_work_user）

| 电量 vs 上限 | chg_disable | usb_icl | fcc | 效果 |
|-------------|------------|---------|-----|------|
| > limit | true (0A) | 0 | 0 | 完全停充 |
| == limit | false | 500mA | 0 | 涓流保电 |
| < limit | false | 放开 | 放开 | 正常充电 |

- 60s 轮询 + 设置后 100ms 首查：`schedule_delayed_work(100ms)` → 循环 60s
- `USER_VOTER` 作为投票者，用户取消时 `smblib_restore_all_charging_paras_user()` 全部解除

### uevent 补充（USB_CHARGER_PRESENT）

- `smblib_notify_usb_charger_present()` 在 usbin-plugin IRQ 中调用
- 通过 workqueue（`usb_charger_present_uevent_work`）异步发 `kobject_uevent_env` —— 避免在 `chg->lock` 持锁上下文直接发 uevent（死锁/调度风险）

## 潜在风险

1. **60s 轮询延迟**：容量跨过阈值后最多 60s 才反应；若客户要求实时，需缩短周期或改中断触发
2. **竞态**：`vote` + `rerun_election` 并发调用时，多个线程（set 与 work）可能竞争；代码每次先 `cancel_delayed_work_sync` 再调度，已基本规避
3. **属性 ABI**：在 `POWER_SUPPLY_PROP_SERIAL_NUMBER` 之后追加（注释明确 "Don't add below" 约束内），不破坏 HAL 枚举序
4. **uevent 频率**：USB 抖动（vbus_rising 反复）可能产生大量 uevent → 需确认 IRQ 消抖

## 回归测试建议

- 上限 80%：充到 80% 后停充；放电到 <80% 恢复充电
- 上限边界（100%/0%）
- 充电过程中动态修改上限值
- usb 插拔 uevent 捕获（`cat /dev/uevent` 或 logcat）
- 满充状态下长期充电发热

## 与现有驱动架构的关系

- 完全不触碰既有 QoS/OTG/无线充电路径，仅新增 USER_VOTER 投票通道
- 与 A14 其它平台（如 SM6225 的 SMB5 充电修复）机制同源，均基于 qpnp-smb2/smb-lib 框架
- MT578 项目在 134 无源码树，归档源码来自 Gerrit REST，后续如需深入可向 [同事] 索取完整树

_Author: wangguanran_
