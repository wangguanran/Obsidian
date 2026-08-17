# MC5616 Sensor 裁剪 SSC vendor 库

## 概述

- **Change**: #196014
- **项目**: snapdragon-premium-high-2021-spf-2-0-2_amss_standard_oem_apq
- **分支**: master_Snapdragon_Premium_High_2021.SPF.2.0.2_MC5616
- **作者**: zhourulei
- **状态**: MERGED
- **类型**: 需求 (trim)
- **芯片**: MC5616（基于 parrot/QCM4490）
- **SoC-Android**: SM4490-A16

## 背景与动机

Netrani SSC POR 默认编入了本项目 BOM 未使用的多家传感器 vendor 库，包括：

- `lsm6dst`
- `ak0991x`
- `tmd2725`
- `lps22hx`
- `sx932x`
- `bu52053nvx`

这些多余的传感器驱动库被链接进 ADSP SSC 镜像，导致镜像体积增大且包含不必要的驱动代码。为精简镜像体积，需在 SSC 编译配置中移除上述未使用的 vendor 库，仅保留 BOM 实际使用的传感器库。

## 修改内容

仅修改一个文件：

| 文件 | 修改 |
|------|------|
| `ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py` | `+1 / -7` |

### 修改详情

在 `por.py` 的 `include_sensor_vendor_libs` 列表中，移除 `lsm6dst`、`ak0991x`、`tmd2725`、`lps22hx`、`sx932x`、`bu52053nvx` 六项，仅保留 `sns_sc7a20` 传感器库引用。

## 配置方式

将 `por.py` 中 `include_sensor_vendor_libs` 列表修改为仅包含 `sns_sc7a20`：

```python
include_sensor_vendor_libs = [
    "sns_sc7a20",
]
```

移除以下六项未使用的 vendor 库引用：

```python
# 已移除
# "lsm6dst",
# "ak0991x",
# "tmd2725",
# "lps22hx",
# "sx932x",
# "bu52053nvx",
```

重新编译 ADSP SSC 镜像后，镜像中将不再包含上述传感器驱动库。

## 验证方式

1. **编译验证**：执行 ADSP SSC 编译，确认编译通过且无链接错误。
2. **镜像体积检查**：对比修改前后 ADSP SSC 镜像体积，确认镜像体积减小。
3. **运行时验证**：将生成的镜像烧录至目标设备，启动后检查设备节点 `/sys/bus/iio/devices/` 或 `sensors` 相关接口，确认仅 `sns_sc7a20` 加速度传感器正常工作，其余已移除传感器设备节点不再出现。
4. **日志检查**：使用 `adb logcat` 或 DSP 侧日志确认 SSC 模块加载正常，无缺失 vendor 库相关的错误日志。

> ⚠️ 补丁验证：无法直接获取验证结果，请参照上述步骤在实际设备上验证。

## 源码归档

补丁内容请参考 Gerrit Change #196014。

修改后的关键代码片段 (`por.py`)：

```python
include_sensor_vendor_libs = [
    "sns_sc7a20",
]
```

## 引用文件索引

- [[../../../ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py|por.py]] — SSC Netrani 平台 POR 编译配置文件，定义了传感器 vendor 库的包含列表

## 相关文档

- [[../MC5616-Sensor裁剪SSC-vendor库-分析|MC5616 Sensor 裁剪 SSC vendor 库 - 分析文档]]

---

_Author: wangguanran_