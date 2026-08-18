# 分析：U 盘无法挂载修复

**版本号：v1.0**
**对应文档：** U盘无法挂载修复.md

## 技术背景

Android 系统通过 vold（Volume Daemon）管理外部存储设备的挂载。fstab 文件定义了系统启动时以及热插拔时的分区/设备挂载规则。对于 USB 存储设备，vold 通过 fstab 中的 `voldmanaged` 属性识别设备类型并自动挂载。

Qualcomm 平台的 fstab 文件通常分为：
- `fstab.qcom`：标准挂载表
- `fstab_non_AB.qcom`：非 A/B 分区版本的挂载表
- `recovery_non_AB.fstab`：Recovery 模式的挂载表

当 U 盘插入后无法挂载时，通常是因为 fstab 中缺少对应的 USB 存储设备节点规则，导致 vold 无法识别该设备。

## 代码改动分析

该提交在 5 个 fstab 文件中各增加了 2 行 USB 存储挂载节点配置（共 +10 行），覆盖：

1. `default/fstab.qcom`：正常开机模式的挂载配置
2. `default/fstab_non_AB.qcom`：非 A/B 启动模式的配置
3. `emmc/fstab.qcom` 和 `emmc/fstab_non_AB.qcom`：eMMC 存储方案的配置
4. `recovery_non_AB.fstab`：Recovery 模式下的配置

新增的挂载节点通常包含：
- USB 控制器设备路径（`/devices/platform/soc/*/usb*`）
- 文件系统类型（vfat、exfat 等）
- `voldmanaged=usb:auto` 属性，让 vold 自动管理

## 潜在风险

- 挂载节点路径若与设备树中的 USB 控制器路径不匹配，可能导致挂载失败
- 对 USB 设备的访问权限配置不当可能导致安全风险
- Recovery 模式下增加挂载节点可能引入额外的恢复流程依赖

## 回归测试建议

- 插入不同容量和格式（FAT32/exFAT/NTFS）的 U 盘验证挂载
- 验证 U 盘热插拔（插入、拔出、再插入）
- 验证 Recovery 模式下的 U 盘挂载
- 验证系统休眠/唤醒后 U 盘挂载状态

## 与现有驱动架构的关系

该修改属于 QCM6490 平台（Rigel 项目）的系统配置层面，与 USB 驱动、vold 存储管理框架配合工作。USB 驱动负责设备枚举，vold 根据 fstab 规则完成挂载。

_Author: 艾达_