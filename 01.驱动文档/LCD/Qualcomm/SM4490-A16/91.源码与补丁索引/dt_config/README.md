# dt_config 归档说明

本目录存放该平台归档的 **DTS/DTSI 设备树配置**文件（合并后版本）。

## 归档文件

| 文件 | 来源路径（repo 相对路径） | 说明 |
|:---|:---|:---|
| [[01.驱动文档/LCD/Qualcomm/SM4490-A16/91.源码与补丁索引/dt_config/parrot-sde-display-common.dtsi\|parrot-sde-display-common.dtsi]] | `vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi` | parrot 显示公共配置 + 面板节点（含 r66451 AMOLED video） |

## 来源路径说明

> 该文件原始位于 **vendor 树**（`vendor/qcom/proprietary/display-devicetree/`），按归档规则 `.dtsi` 归入 `dt_config/` 子目录（未放入 `vendor_hal/`，`vendor_hal/` 仅存放 HAL/平台层 .cpp/.mk 等文件）。

## 补丁索引

- [[01.驱动文档/LCD/Qualcomm/SM4490-A16/91.源码与补丁索引/patches/196185.patch|196185.patch]] — 对应修改补丁（Change #196185）

_Author: wangguanran_