# yupik 删除gpiotest DTS修复启动失败

## 现象

设备在烧录固件后无法正常启动，表现为开机卡死、无法进入系统。具体现象包括：

- 设备上电后停留在 bootloader 阶段，无法进入内核
- 串口日志显示 DTB（Device Tree Blob）解析失败
- 系统无法挂载根文件系统，设备无法完成初始化流程

## 根因分析

经过排查，问题根因如下：

### 1. 问题定位

在 [[yupik删除gpiotest-DTS修复启动失败-分析|详细分析]] 中确认，设备无法启动的直接原因是 **gpiotest 相关的 DTS overlay 文件被错误地编译进了 DTBO 分区**。

### 2. 根本原因

该平台基于 Qualcomm SM7325（yupik）芯片，项目分支中遗留了大量用于 GPIO 测试的 DTS 文件（gpiotest 系列）。这些文件原本仅用于硬件 GPIO 功能验证阶段，在产品量产固件中完全不需要。

然而，由于以下机制导致问题：

1. **DTS 编译链包含**：`Makefile` 中引用了 `yupik-gpiotest.dtsi` 等文件，导致这些测试用的 DTS 被编译进 DTBO 镜像
2. **Bootloader 板级检查**：`Board.c` 中存在 gpiotest 的 board ID 检查逻辑，引导加载程序在解析 DTB 时加载了错误的 overlay
3. **DTB 解析冲突**：gpiotest overlay 中定义的节点与产品实际硬件节点存在冲突，导致 DTB 解析过程失败

### 3. 涉及文件统计

本次提交共涉及 **20 个文件**，删除约 **15,006 行** 冗余代码，分布在以下模块：

| 模块 | 文件数 | 删除行数 |
|------|--------|----------|
| 摄像头设备树（camera-devicetree） | 3 | ~2,413 |
| 设备树主目录（devicetree/qcom） | 10 | ~11,016 |
| 显示设备树（display-devicetree） | 4 | ~1,024 |
| Bootloader 板级代码 | 1 | 2行修改 |
| 构建脚本 | 1 | 4行修改 |
| Makefile 编译配置 | 1 | 3行修改 |

## 处理方案

### 方案概述

彻底删除所有 gpiotest 相关的 DTS 文件及引用，消除测试代码对产品固件的干扰。

### 具体修改

**1. 删除 gpiotest DTS 文件（19个文件删除）**

- 删除 `vendor/qcom/proprietary/camera-devicetree/` 下所有 gpiotest 相关 DTSI 文件
- 删除 `vendor/qcom/proprietary/devicetree/qcom/` 下所有 gpiotest 相关 DTSI/DTS 文件
- 删除 `vendor/qcom/proprietary/display-devicetree/display/` 下所有 gpiotest 相关 DTSI 文件

**2. 修改编译配置**

- `Makefile`：移除 gpiotest DTS 的编译条目（+1/-3）

**3. 修改 Bootloader 板级代码**

- `Board.c`：移除 gpiotest board ID 检查逻辑（+2/-2）

**4. 修改构建脚本**

- `vendor_sparseimage.sh`：移除 gpiotest 镜像的构建步骤（+0/-4）

## 配置方式

本修补丁为全局性删除操作，无需额外配置。正常编译即可：

```bash
# 正常编译 DTBO，不再包含 gpiotest overlay
./build.sh -T LA.UM.9.14.1
```

如果在其他分支中需要保留 gpiotest 功能，请确认以下内容：

- 确保 `Makefile` 中不包含 gpiotest 的 `DTC_FLAGS` 和 `dtbo-y` 条目
- 确保 `vendor_sparseimage.sh` 中不包含 gpiotest 镜像构建步骤
- 确保 `Board.c` 中不包含 gpiotest 的 board ID 检查逻辑

## 验证方式

### 编译验证

```bash
# 全量编译，确认 DTBO 分区不再包含 gpiotest overlay
./build.sh -T LA.UM.9.14.1

# 检查 DTBO 镜像中是否包含 gpiotest 节点
dtc -I dtb -O dts dtbo.img | grep -i gpiotest
# 应无输出
```

### 启动验证

1. 烧录编译后的完整固件到设备
2. 设备上电，观察串口日志
3. 确认设备正常进入 Android 系统
4. 确认 Camera、Display 等模块功能正常

### 回归验证

- 确认摄像头功能正常
- 确认显示功能正常
- 确认 GPIO 相关功能（sensor、按键等）正常

## 源码归档

### 提交信息

| 项目 | 内容 |
|------|------|
| Change-Id | #195900 |
| 项目名 | LA.UM.9.14.1 |
| 分支 | master_LA.4.0_HXB_SNM932_PinMingTong_7a07b9e |
| 作者 | weirong（实际提交：[同事]） |
| 状态 | MERGED |
| 类型 | Bug（boot failure） |
| 提交标题 | [HXB_SNM932_PinMingTong][TaskID]120654 device cannot boot - remove useless gpiotest dts |

### 补丁内容

补丁详情请参考 Gerrit Change #195900。

### 补丁验证

⚠️ **无法直接获取**，因 Gerrit 服务器访问受限。

## 引用文件索引

### 新增/修改文件

| 文件路径 | 变更类型 | 变更内容 |
|----------|----------|----------|
| `bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/Board.c` | 修改 | 移除 gpiotest board ID 检查（+2/-2） |
| `vendor/qcom/proprietary/devicetree/qcom/Makefile` | 修改 | 移除 gpiotest DTS 编译条目（+1/-3） |
| `vendor/vendorcode/build/vendor_sparseimage.sh` | 修改 | 移除 gpiotest 镜像构建（+0/-4） |

### 删除文件

**摄像头设备树（camera-devicetree）：**

- `vendor/qcom/proprietary/camera-devicetree/[[yupik-camera-gpiotest.dtsi]]`（-1,651 行）
- `vendor/qcom/proprietary/camera-devicetree/[[yupik-camera-sensor-idp-gpiotest.dtsi]]`（-506 行）
- `vendor/qcom/proprietary/camera-devicetree/[[yupik-camera-sensor-qrd-gpiotest.dtsi]]`（-256 行）

**设备树主目录（devicetree/qcom）：**

- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-gpiotest.dtsi]]`（-5,567 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-iot-gpiotest.dtsi]]`（-51 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-iot-qrd-gpiotest.dtsi]]`（-61 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-iot-qrd-meige-gpiotest.dts]]`（-10 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-iot-qrd-overlay-gpiotest.dts]]`（-11 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-pinctrl-gpiotest.dtsi]]`（-3,366 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-pmic-overlay-gpiotest.dtsi]]`（-690 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-qrd-gpiotest.dtsi]]`（-1,048 行）
- `vendor/qcom/proprietary/devicetree/qcom/[[yupik-qupv3-gpiotest.dtsi]]`（-756 行）

**显示设备树（display-devicetree）：**

- `vendor/qcom/proprietary/display-devicetree/display/[[yupik-sde-display-common-gpiotest.dtsi]]`（-388 行）
- `vendor/qcom/proprietary/display-devicetree/display/[[yupik-sde-display-gpiotest.dtsi]]`（-159 行）
- `vendor/qcom/proprietary/display-devicetree/display/[[yupik-sde-display-qrd-gpiotest.dtsi]]`（-120 行）
- `vendor/qcom/proprietary/display-devicetree/display/[[yupik-sde-gpiotest.dtsi]]`（-357 行）

---

_Author: wangguanran_