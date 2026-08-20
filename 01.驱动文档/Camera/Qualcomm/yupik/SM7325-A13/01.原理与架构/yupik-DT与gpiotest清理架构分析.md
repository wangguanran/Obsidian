# Qualcomm SM7325 (yupik) 设备树架构与 gpiotest 清理分析

> **版本号：v1.0**

## 平台概述

SM7325（yupik）的板级设备树分散在多个 proprietary 仓库，经 `Makefile` 编译为 DTBO 分区镜像：

```
devicetree/qcom/ (板级 DTS/DTSI)
   ├─ camera-devicetree/    ← Camera 传感器节点
   ├─ display-devicetree/   ← 显示管线节点
   └─ qcom/                 ← SoC/板级公共节点
        ↓ DTC 编译
   dtbo.img → DTBO 分区（bootloader 按 board ID 加载 overlay）
```

## gpiotest 问题根因（#195900）

项目分支遗留了大量仅用于硬件 GPIO 功能验证阶段的 gpiotest DTS 文件，量产固件中不需要，但通过三层机制进入固件：

| 机制 | 位置 | 问题 |
|:---|:---|:---|
| DTS 编译链包含 | `Makefile` 引用 `yupik-gpiotest.dtsi` 等 | 测试 DTS 被编译进 DTBO |
| Bootloader 板级检查 | `Board.c` 含 gpiotest board ID 检查 | 解析 DTB 时加载错误 overlay |
| DTB 解析冲突 | gpiotest overlay 节点与产品硬件节点冲突 | DTB 解析失败 → 启动卡死 |

## 清理方案（3 层）

1. **删除文件**：删除 camera-devicetree / devicetree/qcom / display-devicetree 下全部 gpiotest 相关 DTSI/DTS（19 个文件删除，约 15,006 行）
2. **编译配置**：`Makefile` 移除 gpiotest DTS 编译条目（+1/−3）
3. **Bootloader**：`Board.c` 移除 gpiotest board ID 检查（+2/−2）；`vendor_sparseimage.sh` 移除 gpiotest 镜像构建（+0/−4）

## 影响范围

| 模块 | 文件数 | 删除行数 |
|:---|:---|:---|
| camera-devicetree | 3 | ~2,413 |
| devicetree/qcom | 10 | ~11,016 |
| display-devicetree | 4 | ~1,024 |
| Bootloader Board.c | 1 | 2 行修改 |
| 构建脚本/编译配置 | 2 | 7 行修改 |

## 引用文件索引

- [[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/04.问题案例/yupik删除gpiotest-DTS修复启动失败.md|yupik删除gpiotest-DTS修复启动失败]]（完整补丁与文件清单）
- `bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/Board.c`（远程）
- `vendor/qcom/proprietary/devicetree/qcom/Makefile`（远程）
- `vendor/vendorcode/build/vendor_sparseimage.sh`（远程）

---

_Author: wangguanran_
