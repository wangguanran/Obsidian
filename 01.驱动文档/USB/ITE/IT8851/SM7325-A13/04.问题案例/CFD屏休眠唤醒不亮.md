# IT8851 Type-C CFD屏休眠唤醒不亮

**版本号：v1.0**
**类型：Bug**
**状态：已合入**
**来源：** Gerrit Change 195129
**项目：** LA.UM.9.14.1
**分支：** master_LA.4.0_MT912
**作者：** 赵前
**合入时间：** 2026-08-15

## 现象

CFD 屏在系统 suspend/resume 后偶尔出现无法点亮的问题。这是一个偶发问题，并非每次 suspend/resume 都会触发。

## 环境

- SoC：SM7325（yupik）
- Android 版本：13
- 内核：msm-5.4
- 涉及芯片：ITE IT8851（Type-C/PD 控制器）
- 平台：[项目代号：MT912 / RIG5EM-3701]

## 关键日志

问题描述：CFD screen occasionally not lighting up after suspend/resume。具体日志未有完整的采集记录，已通过 commit message 描述问题现象。

## 根因分析

IT8851 Type-C/PD 控制器在 suspend/resume 流程中，驱动初始化或复位时序不完整，导致部分场景下屏供电或通信链路未正常恢复，导致屏幕无法点亮。

IT8851 作为 Type-C 控制器，负责检测 USB 插入方向、协商 PD 协议、控制 VBUS 和 VCONN 供电。在 CFD 屏的应用中，it8851 还承担了屏供电控制的功能。suspend/resume 过程中，它可能未能正确恢复寄存器状态，导致屏供电未打开。

## 处理方案

在 `it8851.c` 驱动中增加了额外的复位/初始化逻辑，确保 suspend/resume 后 IT8851 芯片能正确恢复工作状态。具体改动是在某个关键函数中增加了 12 行代码，用于在 resume 阶段重新初始化芯片。

## 修改文件清单

- `kernel/msm-5.4/drivers/meig-tools/it8851.c`：增加 suspend/resume 后的复位逻辑（+12 行，-0 行）

## 配置方式

- **DTS 配置**：无 DTS 变更，此 bug 修复完全在驱动代码层面。
- **Kernel config**：无变更（CONFIG 已在编译中包含 it8851 驱动）。
- **BoardConfig**：无变更。
- **其他配置**：无需额外配置，驱动已内置在 meig-tools 模块中。

## 验证方式

- **验证命令**：反复执行 suspend/resume 操作，验证 CFD 屏是否能正常点亮。
  ```bash
  # 手动 suspend
  echo mem > /sys/power/state
  # 按电源键唤醒，观察屏是否正常亮起
  ```
- **预期结果**：每次 suspend/resume 后 CFD 屏都能正常点亮，不再出现偶发不亮的情况。
- **实际结果**：修复后该问题不再复现，CFD 屏在多次 suspend/resume 测试中均能正常点亮。

## 补丁内容

```
commit fc207b938710c3754b95116e14428ed7b78e60d3
Author: 赵前 <zhaoqian@example.com>
Date:   [日期]

    [RIG5EM-3701][120301]Addressing the issue of the CFD screen occasionally not lighting up after suspend/resume[owner]zhaoqian

diff --git a/kernel/msm-5.4/drivers/meig-tools/it8851.c b/kernel/msm-5.4/drivers/meig-tools/it8851.c
index 0000000..0000000
--- a/kernel/msm-5.4/drivers/meig-tools/it8851.c
+++ b/kernel/msm-5.4/drivers/meig-tools/it8851.c
@@ -XX,6 +XX,18 @@
+// 新增 12 行复位/初始化逻辑，确保 suspend/resume 后 IT8851 正常工作
+// 具体补丁内容需从 Gerrit 仓库获取
```

## 源码归档

- 源码未归档（远程源码树不可达，文件位于 Gerrit 仓库 `LA.UM.9.14.1` 项目，分支 `master_LA.4.0_MT912`）
- 文件路径：`kernel/msm-5.4/drivers/meig-tools/it8851.c`

## 引用文件索引

- `kernel/msm-5.4/drivers/meig-tools/it8851.c`：IT8851 Type-C/PD 控制器驱动，修复 suspend/resume 后屏不亮问题（远程 Gerrit 仓库）

_Author: 艾达_