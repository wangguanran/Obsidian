# SGM832 电源监控移植资料（A14→A16）

## 硬件接口

| 项 | 说明 |
|----|------|
| 总线 | I2C |
| 中断 | irq_gpio（DT 配置） |
| 电源 | 由板级提供 |

## DTS 配置

- SGM832 节点：compatible `sgm832`，I2C 地址、irq-gpio
- cash_drawer：compatible `vendor,cash-drawer`（GPIOLIB + INPUT）
- ext-adc-gpio：compatible `ext,adc-driver`（OF + IIO）

## 移植注意

- 原驱动来自 A14 msm-5.4，A16（5.15 GKI）需适配新内核 API
- `drivers/misc/Kconfig` 增加 EXT_ADC_GPIO / CASH_DRAWER_DRIVER 配置项
- `drivers/power/supply/Kconfig` 增加 SGM832

## 相关提交

- #196751（[项目代号] A16 bring up sgm832 + cash）

_Author: wangguanran_
