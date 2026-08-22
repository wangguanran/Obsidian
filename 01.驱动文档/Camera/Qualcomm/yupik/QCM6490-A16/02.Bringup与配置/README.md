# QCM6490-A16 (yupik) Camera 平台移植资料

> 平台：QCM6490 (yupik) / Android 16 | 项目：MC937（[项目代号] 系列）| 内核树：meigla kernel (LA.6.0)

## 硬件接口

| 接口 | 说明 |
|:-----|:-----|
| CCI1 | camera 主 I2C 总线（GPIO75/76 = CCI_I2C_DATA3/CLK3） |
| CCI2 | 本产品未使用（GPIO73/74 已释放，可复用） |
| GPIO73/74 | 释放给其他外设（原错误占用为 CCI2） |
| GPIO75/76 | CCI3 数据/时钟，摄像头挂载 |

## 驱动源码索引

| 文件 | 位置 | 说明 |
|:-----|:-----|:-----|
| [[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-camera-937-pvt.dtsi\|yupik-camera-937-pvt.dtsi]] | camera-devicetree 仓库 `937/pvt/` | MC937 产品 camera devicetree（含 CCI/电源/pinctrl 配置） |

## 配置说明

- camera devicetree 仓库：`meigla/platform/vendor/opensource/camera-devicetree`（分支 `Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_[项目代号]`）
- 产品文件：`937/pvt/yupik-camera-937-pvt.dtsi`
- CCI1 节点已释放 CCI2（GPIO73/74），仅保留 CCI3（GPIO75/76）

## 已知问题案例

- [[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/04.问题案例/Camera不工作-释放GPIO73-74修复.md|Camera不工作修复：释放GPIO73-74（CCI2）]] — Change #196280，Task 120725

_Author: wangguanran_