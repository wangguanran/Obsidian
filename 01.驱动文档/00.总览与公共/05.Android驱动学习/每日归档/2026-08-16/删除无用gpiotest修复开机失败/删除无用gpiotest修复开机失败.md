# SM7325 删除无用 gpiotest DTS 修复开机失败

**版本号：v1.0**
**类型：Bug**
**状态：已合入**
**来源：** Gerrit Change 195900
**项目：** LA.UM.9.14.1
**分支：** master_LA.4.0_HXB_SNM932_PinMingTong_7a07b9e
**作者：** 魏荣
**合入时间：** 2026-08-15

## 现象

设备无法正常开机（device cannot boot），卡在启动阶段。

## 环境

- SoC：SM7325（yupik）
- 内核：msm-5.4 / LA.4.0
- 平台：[项目代号：HXB_SNM932_PinMingTong]

## 根因分析

在 DTS 编译过程中，`*-gpiotest.dtsi` 文件被包含到主 DTS 中。这些 gpiotest 文件原本用于 GPIO 全引脚测试，包含大量测试引脚配置（约 15000 行），但在正式产品中不应被包含。由于这些测试配置被错误地包含，导致 DTS 编译或解析过程中出现问题，进而导致设备无法正常启动。

具体受影响的文件包括：
- Camera 子系统：`yupik-camera-gpiotest.dtsi`、`yupik-camera-sensor-idp-gpiotest.dtsi`、`yupik-camera-sensor-qrd-gpiotest.dtsi`
- Display 子系统：`yupik-sde-display-common-gpiotest.dtsi`、`yupik-sde-display-gpiotest.dtsi`、`yupik-sde-display-qrd-gpiotest.dtsi`、`yupik-sde-gpiotest.dtsi`
- 平台级 GPIO 测试：`yupik-gpiotest.dtsi`、`yupik-iot-gpiotest.dtsi`、`yupik-iot-qrd-gpiotest.dts`、`yupik-iot-qrd-meige-gpiotest.dts`、`yupik-iot-qrd-overlay-gpiotest.dts`、`yupik-pinctrl-gpiotest.dtsi`、`yupik-pmic-overlay-gpiotest.dtsi`、`yupik-qrd-gpiotest.dtsi`、`yupik-qupv3-gpiotest.dtsi`

## 处理方案

删除所有 `*-gpiotest.dtsi` / `*-gpiotest.dts` 文件，并从 `Makefile` 和 `Board.c` 中移除相关引用，同时清理 `vendor_sparseimage.sh` 中的相关打包配置。

关键改动：
1. **删除 14 个 gpiotest 相关 DTS 文件**（-14861 行）
2. **修改 Makefile**：移除 gpiotest 文件的编译包含（+1/-3）
3. **修改 Board.c**：移除 gpiotest 相关初始化（+2/-2）
4. **修改 vendor_sparseimage.sh**：移除 gpiotest 相关打包配置（-4 行）

## 修改文件清单

- `bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/Board.c`：移除 gpiotest 相关初始化（+2/-2）
- `vendor/qcom/proprietary/camera-devicetree/yupik-camera-gpiotest.dtsi`：**删除**（-1651 行）
- `vendor/qcom/proprietary/camera-devicetree/yupik-camera-sensor-idp-gpiotest.dtsi`：**删除**（-506 行）
- `vendor/qcom/proprietary/camera-devicetree/yupik-camera-sensor-qrd-gpiotest.dtsi`：**删除**（-256 行）
- `vendor/qcom/proprietary/devicetree/qcom/Makefile`：移除 gpiotest 文件引用（+1/-3）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-gpiotest.dtsi`：**删除**（-5567 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-gpiotest.dtsi`：**删除**（-51 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-qrd-gpiotest.dtsi`：**删除**（-61 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-qrd-meige-gpiotest.dts`：**删除**（-10 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-qrd-overlay-gpiotest.dts`：**删除**（-11 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-pinctrl-gpiotest.dtsi`：**删除**（-3366 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-pmic-overlay-gpiotest.dtsi`：**删除**（-690 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-qrd-gpiotest.dtsi`：**删除**（-1048 行）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-qupv3-gpiotest.dtsi`：**删除**（-756 行）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-display-common-gpiotest.dtsi`：**删除**（-388 行）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-display-gpiotest.dtsi`：**删除**（-159 行）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-display-qrd-gpiotest.dtsi`：**删除**（-120 行）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-gpiotest.dtsi`：**删除**（-357 行）
- `vendor/vendorcode/build/vendor_sparseimage.sh`：移除 gpiotest 相关打包配置（-4 行）

## 配置方式

- **DTS 配置**：删除所有 `*-gpiotest.dtsi` 包含，确保正式 DTS 中不包含这些测试配置
- **Kernel config**：无变更
- **BoardConfig**：无变更
- **其他配置**：`vendor_sparseimage.sh` 中移除 gpiotest 相关镜像文件

## 验证方式

- **验证命令**：编译后烧录，验证设备能否正常开机
  ```bash
  # 编译验证
  source build/envsetup.sh && lunch [目标] && make -j
  # 烧录后验证开机
  fastboot flash boot boot.img
  fastboot reboot
  ```
- **预期结果**：设备正常开机，不再卡在启动阶段
- **实际结果**：问题修复后设备可正常开机

## 补丁内容

补丁内容暂未获取（远程源码树搜索超时）。改动主要是删除 14 个 gpiotest 相关 DTS 文件（约 15000 行），并修改 Makefile、Board.c 和 vendor_sparseimage.sh 中相关引用。

## 源码归档

- 源码暂未归档（远程 134 源码树搜索超时，文件位于 Gerrit 仓库 `LA.UM.9.14.1`，分支 `master_LA.4.0_HXB_SNM932_PinMingTong_7a07b9e`）

## 引用文件索引

- `bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/Board.c`：修改 gpiotest 相关初始化（远程 Gerrit 仓库）
- `vendor/qcom/proprietary/camera-devicetree/yupik-camera-gpiotest.dtsi`：Camera 子系统 gpiotest 配置（已删除）
- `vendor/qcom/proprietary/camera-devicetree/yupik-camera-sensor-idp-gpiotest.dtsi`：Camera sensor IDP gpiotest 配置（已删除）
- `vendor/qcom/proprietary/camera-devicetree/yupik-camera-sensor-qrd-gpiotest.dtsi`：Camera sensor QRD gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/Makefile`：移除 gpiotest 文件引用
- `vendor/qcom/proprietary/devicetree/qcom/yupik-gpiotest.dtsi`：平台级 GPIO 测试配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-gpiotest.dtsi`：IOT 平台 gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-qrd-gpiotest.dtsi`：IOT QRD gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-qrd-meige-gpiotest.dts`：IOT QRD Meige gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-iot-qrd-overlay-gpiotest.dts`：IOT QRD overlay gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-pinctrl-gpiotest.dtsi`：PinCtrl gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-pmic-overlay-gpiotest.dtsi`：PMIC overlay gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-qrd-gpiotest.dtsi`：QRD gpiotest 配置（已删除）
- `vendor/qcom/proprietary/devicetree/qcom/yupik-qupv3-gpiotest.dtsi`：QUPv3 gpiotest 配置（已删除）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-display-common-gpiotest.dtsi`：Display 通用 gpiotest 配置（已删除）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-display-gpiotest.dtsi`：Display gpiotest 配置（已删除）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-display-qrd-gpiotest.dtsi`：Display QRD gpiotest 配置（已删除）
- `vendor/qcom/proprietary/display-devicetree/display/yupik-sde-gpiotest.dtsi`：SDE gpiotest 配置（已删除）
- `vendor/vendorcode/build/vendor_sparseimage.sh`：移除 gpiotest 打包配置

_Author: 艾达_