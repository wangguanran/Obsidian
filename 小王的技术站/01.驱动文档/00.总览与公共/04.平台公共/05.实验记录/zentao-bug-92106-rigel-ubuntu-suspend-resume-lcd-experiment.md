---
knowledge_key: "zentao:bug:92106"
title: "【MC937U】【MC936U】休眠后无法唤醒 3/3（问题定位实验记录）"
doc_type: experiment_record
stable_key: "zentao:bug:92106"
module: UNCLASSIFIED
module_path: null
publish_track: EXPERIMENT
vendor: null
chip: null
created_at: "2026-08-03"
---

# 【MC937U】【MC936U】休眠后无法唤醒 3/3（问题定位实验记录）

> 文档类型：实验记录（EXPERIMENT）
> 发布轨道：EXPERIMENT（05 审查 REVISION_NEEDED 保守降级；未合入 change 不满足 PUBLISH_CASE 语义，**不得升级**）
> 稳定键：`zentao:bug:92106`（942 Rigel_Ubuntu / 休眠后无法唤醒）
> 模块：UNCLASSIFIED（provisional，942 Ubuntu 平台 module-map 无匹配，module_path=null，最终由 Manager 确认）；vendor/chip：null（无证据，不猜测）
> 证据状态：01d ELIGIBLE（zentao-curator）+ 02b Gerrit confirmed reference（191663，status=NEW 未合入）；03b 源码 COMPLETE（hits=0，0 命中与 status=NEW 一致属预期不削弱 confirmed reference）；证据缺口见 §5

## 1. 问题描述

- **ZenTao bug**：92106
- **标题**：【MC937U】【MC936U】休眠后无法唤醒 3/3
- **项目/产品**：942（Rigel_Ubuntu）/ 302
- **严重级别/优先级**：severity=2 / pri=2
- **类型/os**：function / android
- **打开**：zhangmeng，2026-06-23 18:23:31
- **现象（steps 原文，命令逐字）**：
  - 【操作步骤】1、输入指令：`adb shell`；`echo mem > /sys/power/state`；`ctrl+c 取消`；2、按 power 键
  - 【预期结果】1、执行指令后背光灭；2、按 power 键可以唤醒、不死机
  - 【实际结果】MC937U：执行指令灭屏，按 power 键不唤醒；MC936U：执行指令灭一下自动亮屏随后再灭屏，按 power 键不唤醒（见附件视频）
  - 【概率】3/3
  - 【其它单板验证结果】MC938U 可以唤醒
- **附件**：2 个复现视频（fileID 226787 `936U唤醒.3gp`、226788 `937U唤醒.3gp`，zhangmeng / 2026-06-23）
- **影响**：MC937U/MC936U 执行 `echo mem > /sys/power/state` 进入休眠后按 power 键无法唤醒（必现 3/3）；同项目 MC938U 可正常唤醒，属单板差异。

## 2. 定位过程（时间线）

| 时间 | 动作 | 执行人 | 内容 |
|---|---|---|---|
| 2026-06-23 18:23:31 | opened | zhangmeng | 提交 bug（steps 见 §1；附件 2×3gp 复现视频） |
| 2026-06-26 16:04:08 | edited | jinnina | assignedTo→wangguanran |
| 2026-06-26 17:10:34 | commented | wuxiaolian | MC938_LINUX_524D57FDA5_20260624_V01_06 验证通过，休眠后可按 power 键唤醒 |
| 2026-07-01 20:20:06 | commented | wangguanran | 需 936/937/938 同步确认 Power 休眠唤醒是否正常 |
| 2026-07-01 20:20:15 | assigned | wangguanran | assignedTo→lixianghui |
| 2026-07-02 16:46:54 | commented | lixianghui | 根因调试：MC936 无法唤醒（boardid 不正确未正常加载 dtbo，换 boardid=1 后正常）；MC937 suspend 后 powerkey 可唤醒；MC938 可唤醒；均为 MC938_LINUX_524D57FDA5_20260624_V01_06 image |
| 2026-07-03 09:53:36 | commented | lixianghui | MC936/MC938 V6 image 可正常唤醒；MC937 测试结果差异待确认，若仍复现请提供串口 log |
| 2026-07-28 17:23:31 | commented | lixianghui | **代码已提交** + Gerrit 链接（见 §3，逐字引用） |
| 2026-07-28 17:29:30 | edited | wangguanran | difficulty/workload/reward |
| 2026-07-28 17:44:28 | edited | wangguanran | schedule/reward |
| 2026-07-28 17:45:19 | resolved | lixianghui | **resolution=fixed**、resolvedBuild=trunk、confirmed=1、assignedTo→zhangmeng |

**根因调试链（lixianghui 评论逐字）**：
> MC936 无法唤醒（boardid 不正确未正常加载 dtbo，换 boardid=1 后正常）；MC937 suspend 后 powerkey 可唤醒；MC938 可唤醒；均为 MC938_LINUX_524D57FDA5_20260624_V01_06 image

> MC936/MC938 V6 image 可正常唤醒；MC937 测试结果差异待确认，若仍复现请提供串口 log

## 3. 解决方案（Gerrit 引用逐字）

**来源**：2026-07-28 17:23:31 lixianghui 评论（逐字）：

> 代码已提交：https://[内网Gerrit]/c/qualcomm-linux-spf-1-0_ap_standard_oem_nm-qimpsdk/+/191663
> [92106][Rigel_Ubuntu][LCD]Fix issue that MC937 cannot bring up edp panel when resume
> [Owner]lixianghui (Ic0c7caf0)

**Gerrit change 191663（02b 检索确认，引用逐字）**：

| 项 | 值 |
|---|---|
| Gerrit change | 191663 |
| Change-Id（ZenTao 前缀） | **Ic0c7caf0** |
| Change-Id（完整，02b） | Ic0c7caf00e5c2df1a0742bf9ce52ac0aaa780632 |
| URL | https://[内网Gerrit]/c/qualcomm-linux-spf-1-0_ap_standard_oem_nm-qimpsdk/+/191663 |
| 标题（subject） | [92106][Rigel_Ubuntu][LCD]Fix issue that MC937 cannot bring up edp panel when resume |
| 项目（project） | qualcomm-linux-spf-1-0_ap_standard_oem_nm-qimpsdk |
| 分支（branch） | rigle_linux_1.6 |
| Owner | Li Xianghui（=ZenTao lixianghui） |
| **状态** | **NEW（open=true，未合入）**——如实标注，**不得写 merged/已合入** |
| patchset / revision / ref | 1 / ee09c2593fef6544522d6c4056c4c4c3d735c3a7 / refs/changes/63/191663/1 |
| 关联判定（02b） | **confirmed reference**（与 ZenTao 显式引用逐字一致：subject 含 [92106]、LCD/EDP 组件、owner 同名、仓库名一致、Change-Id 前缀匹配）；**非 confirmed fix**（diff 未验证且 status=NEW 未合入） |

**修复方向**：LCD/EDP panel resume bring-up（MC937 cannot bring up edp panel when resume）。ZenTao 根因调试链指向 MC936 boardid 不正确未正常加载 dtbo（换 boardid=1 后正常）；Gerrit 提交标题指向 EDP panel resume 修复。boardid/dtbo 调试与 edp resume 修复之间的对应关系以 02b diff 回填为准（见 §5 E2，不臆造）。

**注意**：本 change 状态为 NEW（未合入），仅记录修复方向与提交载体；「resolution=fixed」为 ZenTao 侧状态（§4），两者合入关系未验证（§5），不得据此断言该 change 已合入 trunk。

## 4. 验证

**依 ZenTao（resolution=fixed / confirmed=1 / resolvedBuild=trunk）**：

| 项 | 值 |
|---|---|
| resolution | **fixed** |
| confirmed | 1 |
| resolvedBuild | trunk |
| resolvedBy / resolvedDate | lixianghui / 2026-07-28 17:45:12 |
| 验证记录 | 2026-06-26 wuxiaolian：MC938_LINUX_524D57FDA5_20260624_V01_06 验证通过，休眠后可按 power 键唤醒；2026-07-03 lixianghui：MC936/MC938 V6 image 可正常唤醒 |

**如实并列（Gerrit 侧）**：Gerrit change 191663 状态为 **NEW（未合入）**；revision `ee09c259…` 与 commit SHA 对应关系未验证（ZenTao 未给显式 commit SHA）；status=NEW 与 resolvedBuild=trunk 的合入关系未验证。**不得写为「已验证」或「已合入」**。

## 5. 证据缺口（如实保留，不得补写）

1. **E2 diff 不可得**：gerrit-patch ["191663"] 已回填 **FAILED rc=142（宿主超时，与 R4 151686 同型）**，request_id=`gerrit-patch-20260803T082000Z-26879`；diff 无部分输出、**文件清单不可得**（LCD/EDP panel resume 文件、dtbo/boardid 配置无法从 Gerrit 侧确认）；稳定超时 ×1，不重试、不猜测 diff 内容。
2. **03b 源码 COMPLETE（hits=0）**：布局定位完成——qimpsdk（与 Gerrit 191663 项目名一致）→ LE.QCLINUX.1.0.R1（Yocto）→ sources/**graphics-kernel**（panel 驱动方向）+ **graphics-devicetree/gpu**（22 个平台 GPU dtso，**qcm6490-graphics.dtso 命中** Rigel 平台候选；无 MC936/MC937 panel overlay）；R5a `search graphics-kernel "edp"` rc=1 **0 命中**（-082417Z-27870）如实；Round 6 确认性收束已回填（-082554Z-28304）不改变结论；**0 命中与 191663 status=NEW 一致属预期，不削弱 confirmed reference**（02b Q1+Q3+显式引用足够，E2 diff 仅确认性证据）。
3. **revision↔commit SHA 未验证**：revision `ee09c2593fef6544522d6c4056c4c4c3d735c3a7` 与显式 commit 对应关系未验证（ZenTao 无显式 commit SHA）。
4. **status=NEW vs resolvedBuild=trunk 合入关系未验证**：不得断言 191663 已合入 trunk。
5. **短 Change-Id 前缀 message 检索 0 命中**（02b Q2 `message:"Ic0c7caf0"` NO_MATCH；Q1 `change:191663` 已确认命中，不影响结论）。

## 6. 结论

- **ELIGIBLE（01d 终判）**：显式 Gerrit change 引用（191663/Ic0c7caf0）+ 确认修复方案已提交 + 根因调试链完整（boardid/dtbo）。
- **可复现性**：复现命令逐字可复现（`adb shell`；`echo mem > /sys/power/state`；按 power 键，3/3）；Gerrit 引用可追踪（URL/Change-Id/subject 逐字）。
- **发布轨道**：**EXPERIMENT（05 审查 REVISION_NEEDED 保守降级，不得升级 PUBLISH_CASE）**——02b 判定 **confirmed reference 非 confirmed fix**（E2 diff rc=142 不可得 + SHA 未验证 + 191663 status=NEW 未合入）；PUBLISH_CASE 语义需确定修复，未合入 change 不满足；R4 78265 升级路径不适用（R4 依赖 Manager 5 点替代支撑含 MERGED + 显式 commit SHA a066b14b，R8 无 commit SHA、191663 未合入、无 Manager 级支撑）；R4 Round 1 先例 diff 未闭合时保守降级；不得将未验证状态反推为已修复能力。EXPERIMENT 轨道知识价值完整（问题 3/3 可复现命令/根因 boardid-dtbo/修复方向 edp resume 提交待合入/验证记录如实）。
- **约束**：status=NEW 未合入如实标注；SHA 未验证不得写「已验证」；E2 diff 不可得（rc=142）与源码 0 命中如实保留；不新增 KNOWLEDGE_REQUEST。
