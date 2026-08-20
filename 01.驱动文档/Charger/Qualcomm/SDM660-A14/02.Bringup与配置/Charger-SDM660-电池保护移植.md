# SDM660 电池保护（Max SoC limit）移植资料

## 修改点

| 文件 | 改动 | 说明 |
|------|------|------|
| drivers/power/supply/power_supply_sysfs.c | +2 | 新增 restricted_charging_user 属性 |
| drivers/power/supply/qcom/qpnp-smb2.c | +19/-1 | 属性支持 + get 回调 |
| drivers/power/supply/qcom/smb-lib.c | +114 | 受限充电工作队列与 vote 逻辑 |
| drivers/power/supply/qcom/smb-lib.h | +12/-1 | 结构体字段 + 接口声明 |
| include/linux/power_supply.h | +2 | POWER_SUPPLY_PROP_RESTRICTED_CHARGING_USER |

## 客户使用方式

```
# 设置充电上限 80%（示例）
echo 80 > /sys/class/power_supply/battery/restricted_charging_user
# 查询当前限制
cat /sys/class/power_supply/battery/restricted_charging_user
```

## 相关提交

- #195420（EAD-8809）

_Author: wangguanran_
