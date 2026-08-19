# 源码与补丁索引

## 目录说明

本目录存放 SM7325-A13（yupik）Camera 平台 gpiotest 移除（patch #195900，HXB_SNM932 项目）涉及的源码存档，与 `patches/195900-remove-gpiotest-dts.patch` 配对。

## 文件来源

源码从 134 服务器 sm7325 源码树检索归档（2026-08-20）：

```
/home3/wangguanran/workspace/MC5616/LA.VENDOR.1.0.R1/
```

> 说明：patch 路径为 `vendor/qcom/proprietary/devicetree/qcom/Makefile`，134 树上该 git 仓库 checkout 位于 `kernel_platform/qcom/...`，归档时按 patch 相对路径存放。MC5616 为当前 134 上可用的 sm7325 树，文件为 gpiotest 移除后状态。

## 文件索引

| 分类 | 文件（patch 相对路径） | 说明 |
|------|------|------|
| kernel_driver | `bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/Board.c` | BootLib 启动逻辑，移除 gpiotest 引用 |
| dt_config | `vendor/qcom/proprietary/devicetree/qcom/Makefile` | devicetree 构建 Makefile，移除 gpiotest dtsi |
| vendor_hal | `vendor/vendorcode/build/vendor_sparseimage.sh` | vendor 镜像打包脚本，移除 gpiotest 分区引用 |

## 引用文件索引

- [[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/02.Bringup与配置/91.源码与补丁索引/kernel_driver/bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/Board.c|Board.c]]
- [[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/02.Bringup与配置/91.源码与补丁索引/dt_config/vendor/qcom/proprietary/devicetree/qcom/Makefile|Makefile]]
- [[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/02.Bringup与配置/91.源码与补丁索引/vendor_hal/vendor/vendorcode/build/vendor_sparseimage.sh|vendor_sparseimage.sh]]
- [[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/02.Bringup与配置/91.源码与补丁索引/patches/195900-remove-gpiotest-dts.patch|195900-remove-gpiotest-dts.patch]]

---

_Author: wangguanran_
