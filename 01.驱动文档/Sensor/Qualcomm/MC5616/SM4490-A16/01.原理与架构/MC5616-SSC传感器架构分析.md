# MC5616 (parrot) ADSP SSC 传感器架构分析

> **版本号：v1.0**

## 平台概述

MC5616（基于 parrot/QCM4490）的传感器由 **ADSP SSC**（Sensors SubSystem）统一管理：

```
Sensor 硬件 (I2C)
   ↓ sns_sc7a20 等驱动库（编译进 SSC 镜像）
ADSP SSC (Sensors SubSystem)
   ├─ 传感器驱动库（vendor libs）
   ├─ 数据采集/融合 (SSI/SSE 框架)
   └─ 对上提供 /sys/bus/iio 与 sensors HAL 接口
```

## SSC 镜像与 vendor 库机制

SSC（Sensor SubSystem）为每个平台编译独立的 ADSP 镜像，镜像内容由 **POR（Power On Reset）配置脚本 `por.py`** 控制。`por.py` 中的 `include_sensor_vendor_libs` 列表列出了编译进镜像的传感器 vendor 库：

```python
env.Replace(SSC_INCLUDE_SENS_VEND_LIBS=include_sensor_vendor_libs)
```

- 列表对应**参考设计平台支持的传感器型号**（tmd3702/tmd2725/shtw2 等）
- MC5616 的 BOM **仅搭载 sc7a20 加速度传感器**，其余库对应硬件不存在
- 多余库虽被链接进镜像，运行时不会使用，纯属冗余代码，增大镜像体积

## 裁剪方案

仅保留 BOM 实际使用的传感器库（`sns_sc7a20`），从 `por.py` 的默认 vendor 库列表中移除其余项。

## 平台差异（移植注意）

| 芯片平台 | 默认 vendor 库差异 |
|:---|:---|
| netrani（本平台） | 本文修改对象 |
| fillmore / waipio | 使用 `sns_tmd3702` 而非 `sns_tmd2725`；waipio 还含 `sns_shtw2` |

移植到其他平台前必须确认目标平台 BOM 传感器列表。

## 引用文件索引

- [[01.驱动文档/Sensor/Qualcomm/MC5616/SM4490-A16/03.需求实现/MC5616-Sensor裁剪SSC-vendor库.md|MC5616-Sensor裁剪SSC-vendor库]]（补丁内容与源码归档）
- [[01.驱动文档/Sensor/Qualcomm/MC5616/SM4490-A16/91.源码与补丁索引/kernel_driver/ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py|por.py]]（已归档）

---

_Author: wangguanran_
