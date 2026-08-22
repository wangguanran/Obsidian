# [项目代号] Sensor 裁剪 SSC vendor 库 — 分析

## 变更信息

| 项目 | 内容 |
|------|------|
| Change | #196014 |
| 项目 | snapdragon-premium-high-2021-spf-2-0-2_amss_standard_oem_apq |
| 分支 | master_Snapdragon_Premium_High_2021.SPF.2.0.2_[项目代号] |
| 作者 | [同事] |
| 状态 | MERGED |
| 类型 | 需求 (trim) |

## 根因分析

Netrani SSC（Sensors SubSystem）的 POR（Power On Reset）配置脚本 `por.py` 中，`include_sensor_vendor_libs` 列表默认包含了多种传感器 vendor 库。这些库对应的是参考设计平台所支持的传感器型号，但 [项目代号] 项目的实际 BOM 中仅使用了 STMicroelectronics 的 `sns_sc7a20` 加速度传感器。

被移除的未使用传感器库及其对应传感器型号：

| 被移除的库 | 对应传感器 |
|-----------|-----------|
| `lsm6dst` | ST 六轴 IMU（加速度+陀螺仪） |
| `ak0991x` | AKM 电子罗盘/磁力计 |
| `tmd2725` | AMS 环境光/接近传感器 |
| `lps22hx` | ST 气压传感器 |
| `sx932x` | Semtech 电容式接近传感器 |
| `bu52053nvx` | ROHM 霍尔效应传感器 |

这些库虽然在编译阶段被链接至 ADSP SSC 镜像，但由于对应硬件未在 BOM 中搭载，运行时并不会被使用，属于冗余代码。

## 解决方案

在 `por.py` 中将 `include_sensor_vendor_libs` 列表精简为仅包含 `sns_sc7a20`，其余六项 vendor 库引用全部移除。

## 影响范围

- **镜像体积**：ADSP SSC 镜像体积减小，不再包含多余传感器驱动代码。
- **编译时间**：无显著影响，仅减少了链接步骤中的目标文件数量。
- **运行时**：对功能无影响，因被移除的传感器硬件未在 BOM 中使用。
- **回退风险**：低。若后续 BOM 变更需要增加传感器，只需在列表中重新添加对应库即可。

## 参考链接

- [[../Sensor裁剪SSC-vendor库|[项目代号] Sensor 裁剪 SSC vendor 库 — 主文档]]

---

_Author: wangguanran_