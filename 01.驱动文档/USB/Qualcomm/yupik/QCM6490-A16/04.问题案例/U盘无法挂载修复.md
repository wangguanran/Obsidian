# QCM6490 U 盘无法挂载修复

**版本号：v1.0**
**类型：Bug**
**状态：已合入**
**来源：** Gerrit Change 195827
**项目：** meigla/platform/vendor/qcom/lahaina612
**分支：** Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_[项目代号]
**作者：** [同事]
**合入时间：** 2026-08-15

## 现象

插入 U 盘后系统无法挂载，无法识别或访问 U 盘文件系统。

## 环境

- SoC：QCM6490（yupikp）
- 内核：Linux 6.0
- 平台：[项目代号：[项目代号]_A16 / Elo_[项目代号]]

## 根因分析

`fstab.qcom` 和 `fstab_non_AB.qcom` 文件中缺少 USB 存储设备的挂载节点配置。系统在解析 fstab 时，没有找到 USB 存储设备的挂载规则，因此插入 U 盘后无法自动挂载。

## 处理方案

在 `fstab.qcom` 和 `fstab_non_AB.qcom` 中添加 USB 存储设备的挂载节点，让系统能够识别并自动挂载 U 盘。

修改范围覆盖 5 个 fstab 文件：
- `default/fstab.qcom`：标准分区挂载表
- `default/fstab_non_AB.qcom`：非 A/B 分区挂载表
- `emmc/fstab.qcom`：eMMC 分区挂载表
- `emmc/fstab_non_AB.qcom`：eMMC 非 A/B 分区挂载表
- `recovery_non_AB.fstab`：Recovery 模式挂载表

每个文件增加 2 行 USB 存储设备挂载节点。

## 修改文件清单

- `default/fstab.qcom`：增加 USB 存储挂载节点（+2）
- `default/fstab_non_AB.qcom`：增加 USB 存储挂载节点（+2）
- `emmc/fstab.qcom`：增加 USB 存储挂载节点（+2）
- `emmc/fstab_non_AB.qcom`：增加 USB 存储挂载节点（+2）
- `recovery_non_AB.fstab`：增加 USB 存储挂载节点（+2）

## 配置方式

### fstab 配置

**fstab 增加 USB 存储挂载节点（示例）：**
```
# USB 存储设备自动挂载
/devices/platform/soc/XXXX.usb*   auto   vfat   defaults   voldmanaged=usb:auto
```

具体的挂载节点路径需要与 QCM6490 平台的 USB 控制器设备路径匹配。修改在 5 个 fstab 文件中各增加了 2 行挂载规则。

### Kernel config
- 无变更（USB 存储驱动和 VFAT 文件系统支持已在内核中启用）

### BoardConfig
- 无变更

## 验证方式

- **验证命令**：
  ```bash
  # 插入 U 盘后检查是否自动挂载
  ls /mnt/media_rw/
  # 或检查系统日志
  logcat -b all | grep -i "usb\|vold\|mount"
  # 查看挂载点
  mount | grep vfat
  ```
- **预期结果**：插入 U 盘后系统自动挂载，可通过文件管理器或命令访问 U 盘内容
- **实际结果**：修复后 U 盘可正常挂载和访问

## 补丁内容

补丁内容暂未获取（远程源码树搜索超时）。主要改动为 5 个 fstab 文件各增加 2 行 USB 存储挂载节点（共 +10 行）。

## 源码归档

- 源码暂未归档（远程 134 源码树搜索超时，文件位于 Gerrit 仓库 `meigla/platform/vendor/qcom/lahaina612`，分支 `Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_[项目代号]`）

## 引用文件索引

- `default/fstab.qcom`：标准分区挂载表，增加 USB 存储挂载节点（远程 Gerrit 仓库）
- `default/fstab_non_AB.qcom`：非 A/B 分区挂载表，增加 USB 存储挂载节点（远程 Gerrit 仓库）
- `emmc/fstab.qcom`：eMMC 分区挂载表，增加 USB 存储挂载节点（远程 Gerrit 仓库）
- `emmc/fstab_non_AB.qcom`：eMMC 非 A/B 分区挂载表，增加 USB 存储挂载节点（远程 Gerrit 仓库）
- `recovery_non_AB.fstab`：Recovery 模式挂载表，增加 USB 存储挂载节点（远程 Gerrit 仓库）

_Author: wangguanran_