# SDM660 电池保护（Max SoC limit）架构

## 功能概述

新增 `restricted_charging_user`（受限充电）机制：客户可设置电池充电上限（Max SoC limit），达到上限后停止充电（或限流 500mA），防止电池过充、延长寿命。

## 实现分层

```
用户态 / 客户 API
   ↓ sysfs / power_supply 属性
POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER
   ↓ qpnp-smb2.c smb2_batt_get_prop()
smblib_get_restricted_charging_capacity_user()
   ↓ smb-lib.c
restricted_charging_work_user（60s 周期轮询 CAPACITY）
   ↓ vote()
chg_disable_votable / usb_icl_votable / fcc_votable
```

## 核心逻辑（smblib_restricted_charging_work_user）

| 电池电量 vs 上限 | 动作 |
|-----------------|------|
| capacity > limit | 禁用充电（chg_disable）+ 挂起 USB（usb_icl=0）+ FCC=0 |
| capacity == limit | 恢复充电使能，但 USB 输入限 500mA |
| capacity < limit | 全部恢复（解除 vote） |

## 其他改动

- `smblib_notify_usb_charger_present()`：USB_CHARGER_PRESENT uevent（workqueue 异步发送，避免锁内 uevent）

_Author: wangguanran_
