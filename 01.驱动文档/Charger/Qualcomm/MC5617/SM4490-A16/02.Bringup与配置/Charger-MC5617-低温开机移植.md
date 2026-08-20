# MC5617 低温开机（JEITA 禁用）移植资料

## 问题场景

热敏电阻阻值 846.6kΩ 对应 -15°C 时样机无法正常开机（JEITA/AFP 低温保护阻断充电与启动）。

## 修改点

| 文件 | 改动 | 说明 |
|------|------|------|
| ADSP.../pm_peripheral_chgr.c | +12/-0 | `#if ODM_PROJECT_MC5617` 分支强制 JEITA_EN_CFG=0x00 |
| BOOT.../pm_config_target_sbl_sequence.h | +1/-1 / +4/-1 | PSI 表追加 0x1090=0x00 写项，引入 odm_features.h |

## 验证方式

- 冷启动验证：-15°C 环境（或等效热敏电阻阻值模拟）下开机，确认无 AFP 触发
- XBL 日志确认 PSI 序列执行 0x1090 写操作
- 正常温度下充电功能不受影响（JEITA 使能项被平台宏隔离）

## 相关提交

- #196516（amss_standard_oem 主仓）
- #196564（modem_sign 仓库，同一任务双提交）

_Author: wangguanran_
