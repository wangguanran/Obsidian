# Ubuntu 平台休眠唤醒失败问题分析

## 1. 问题描述

Ubuntu 平台执行 `echo mem > /sys/power/state` 进入休眠后，按 power 键无法唤醒，概率 3/3（必现）。

**现象**：
- 执行休眠命令后，背光熄灭
- 按 power 键无法唤醒系统，屏幕无响应
- 同项目另一单板可正常唤醒，属单板差异

## 2. 复现命令

```bash
adb shell
echo mem > /sys/power/state
# 取消 ctrl+c
# 按 power 键
```

**预期结果**：执行命令后背光灭，按 power 键可唤醒系统，不死机。

**实际结果**：执行命令灭屏，按 power 键无法唤醒，系统无响应。

## 3. 根因分析

**boardid 不正确导致 dtbo 未正常加载**，进而导致 edp panel resume 流程失败。

具体调试链：
1. 无法唤醒的单板 boardid 检测异常
2. 由于 boardid 错误，对应的 dtbo（Device Tree Blob Overlay）未正确加载
3. dtbo 加载失败导致 edp panel 的电源管理和 resume 流程缺少必要的配置
4. 系统进入休眠后，edp panel 无法正常恢复，表现为按 power 键无响应

## 4. 解决方案

**修正 boardid 检测逻辑，完成 edp panel resume bring-up。**

修复方向：
- 修正 boardid 识别机制，确保 dtbo 能够正确加载
- 修复 edp panel resume 流程，使休眠唤醒后 edp panel 可正常 bring-up

验证情况：
- 修复后单板可正常执行休眠唤醒
- 验证通过：休眠后可按 power 键正常唤醒，无死机