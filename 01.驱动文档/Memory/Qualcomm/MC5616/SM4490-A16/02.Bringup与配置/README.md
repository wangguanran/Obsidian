# UFS Bringup 与配置说明（MC5616 / SM4490）

> 平台：SM4490-A16（parrot）| VENDOR：LA.VENDOR.1.0.R1

## 硬件接口

- UFS 器件接 UFS PHY（HS-G2/G3/G4 视器件而定）；
- 本项目典型器件：HOSINGLOBAL UFS 2.2（JEDEC id 0x0CD6，型号 HBN1901280CHBC）、UFS 3.x 器件。

## 内核配置

UFS 相关配置在内核 defconfig（GKI vendor 侧）启用，本平台涉及：

- `CONFIG_SCSI_UFS_QCOM`：高通 UFS 主机控制器驱动；
- `CONFIG_DEVFREQ_GOV_SIMPLE_ONDEMAND`：simple_ondemand governor（时钟缩放用）。

## UFS 器件 quirk 要求

器件请在 `ufs_qcom_dev_fixups[]` 中登记（厂商 ID + quirk），否则默认行为可能不适用：

- 高延迟 UFS 2.x 器件：`UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM`（idle 延迟进 LPM，保留降频窗口）；
- 时钟缩放验收（flashval 2.1/2.2/10.1/10.2）需保证 UFS 2.x 器件降频阈值放宽（downdifferential=45）。

## 调试命令

```bash
# 查看 UFS 设备
ls /sys/bus/platform/devices/*ufshc*/  | head
# devfreq 频率切换统计
cat /sys/class/devfreq/*ufshc*/trans_stat
# 时钟缩放参数（若导出）
cat /sys/kernel/debug/ufshcd0/ 2>/dev/null | head
# IO 压力测试后观察降频是否发生
# 器件厂商 ID
cat /sys/class/scsi_device/*/device/vendor /sys/class/scsi_device/*/device/model
```

## 验证 checklist

- [ ] flashval 2.1/2.2/10.1/10.2 通过（时钟缩放切换 ≥ 1000 次）
- [ ] UFS 2.x 与 3.x 器件读写性能无回退
- [ ] idle→active 循环 devfreq 正常降频

## 相关归档

- 问题案例：[[01.驱动文档/Memory/Qualcomm/MC5616/SM4490-A16/04.问题案例/UFS时钟缩放饥饿修复.md\|UFS 时钟缩放饥饿修复]]

_Author: wangguanran_
