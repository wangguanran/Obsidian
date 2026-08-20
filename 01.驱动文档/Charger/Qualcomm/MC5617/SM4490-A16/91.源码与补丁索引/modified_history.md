# 修改历史摘要

> 来源：qcm4490-la-4-0_amss_standard_oem / _modem_sign（134 源码树 git 记录）

## 提交记录

| 提交 | 任务 | 描述 | 状态 | 补丁验证 |
|------|------|------|------|---------|
| e139caa6fc (change 196516) | Task 96276 | thermistor 846.6kΩ -15°C 无法开机（amss_standard_oem） | MERGED | ✅ 134 git apply --check 可干净应用 |
| eae3194b31 (change 196564) | Task 96276 | 同上（modem_sign 仓库） | MERGED | ✅ 134 git apply --check 可干净应用 |

## 归档源码清单

- `kernel_driver/ADSP.HT.5.7/adsp_proc/core/battman/pmic/platform/src/pm_peripheral_chgr.c` — BattMan 充电外设驱动（134 拉取）
- `kernel_driver/BOOT.MXF.2.0/boot_images/boot/QcomPkg/Library/PmicLib/target/clarence/psi/pm_config_target_sbl_sequence.h` — XBL PSI 序列表（134 拉取，含 #196564 改动）

## 备注

- 补丁文件：`patches/196516.patch`、`patches/196564.patch`

_Author: wangguanran_
