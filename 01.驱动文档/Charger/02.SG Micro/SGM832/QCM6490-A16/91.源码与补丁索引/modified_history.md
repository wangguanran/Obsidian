# 修改历史摘要

> 来源：meigla/kernel/qcom（Gerrit 提交记录）
> 134 服务器无该仓库源码树，历史以 Gerrit 归档为准。

## 提交记录

| 提交 | 任务 | 描述 | 状态 | 补丁验证 |
|------|------|------|------|---------|
| 2bc79eb416 (change 196751) | Task 121287/121288/121289 | bring up sgm832 and lisbon cash | MERGED | ✅ 与归档源码一致（REST） |

## 归档源码清单

- `kernel_driver/drivers/power/supply/sgm832.c` — SGM832 电源监控
- `kernel_driver/drivers/misc/cash_drawer.c` — 钱箱驱动
- `kernel_driver/drivers/misc/ext-adc-gpio.c` — 外部 ADC GPIO
- `kernel_driver/drivers/misc/Kconfig/Makefile/modules.bzl`
- `kernel_driver/drivers/power/supply/Kconfig/Makefile/modules.bzl`
- `kernel_driver/configs/lahaina_consolidate.bzl` / `lahaina_perf.bzl`

## 备注

- 仓库不在 134，源码经 Gerrit REST 拉取 current revision
- 补丁文件：`patches/196751.patch`

_Author: wangguanran_
