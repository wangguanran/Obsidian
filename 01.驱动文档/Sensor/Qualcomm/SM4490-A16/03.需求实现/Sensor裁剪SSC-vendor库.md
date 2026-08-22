# [项目代号] Sensor 裁剪 SSC vendor 库

## 概述

- **Change**: #196014
- **项目**: snapdragon-premium-high-2021-spf-2-0-2_amss_standard_oem_apq
- **分支**: master_Snapdragon_Premium_High_2021.SPF.2.0.2_[项目代号]
- **作者**: [同事]
- **状态**: MERGED
- **类型**: 需求 (trim)
- **芯片**: [项目代号]（基于 parrot/QCM4490）
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

## 补丁验证

### 补丁信息

| 项目 | 内容 |
|------|------|
| Change-Id | #196014 |
| 文件 | `ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py` |
| 大小 | 1082 bytes |
| 修改类型 | +1 / -7（仅修改 `include_sensor_vendor_libs` 列表） |
| 状态 | MERGED |

### 验证结果

✅ **补丁文件完整性**：补丁文件 `/tmp/gerrit-patches-rest/196014.patch` 大小 1082 bytes，diff 格式正确（标准 unified diff），无格式错误。

✅ **源码一致性验证**：从 134 服务器读取的 `netrani/por.py` 确认补丁已合入。文件中的 `include_sensor_vendor_libs` 列表已精简为仅包含 `sns_sc7a20`，与 patch 内容完全一致。

✅ **预补丁版本复现**：根据补丁反向推导，修改前列表包含 7 项（`lsm6dst`, `sns_ak0991x`, `sns_tmd2725`, `sns_lps22hx`, `sns_sx932x`, `sns_bu52053nvx`, `sns_sc7a20`），修改后仅保留 `sns_sc7a20`，与上下文中的功能注释完全吻合。

✅ **Git 索引一致性**：补丁中 `index a9ab0fa..9de8eab` 显示新旧 blob 哈希不同，符合文件修改预期。

> 补丁已验证通过，无语法错误或逻辑冲突。

## 补丁内容

```diff
[PATCH] [项目代号][93821][Sensor]Trim SSC netrani vendor libs to sns_sc7a20 [Owner][同事]

diff --git a/ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py b/ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py
index a9ab0fa..9de8eab 100755
--- a/ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py
+++ b/ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py
@@ -22,13 +22,7 @@
     
     if 'SSC_TARGET_X86' not in env['CPPDEFINES']:
         # POR sensors list
-        include_sensor_vendor_libs.extend(['lsm6dst',
-                                           'sns_ak0991x',
-                                           'sns_tmd2725',
-                                           'sns_lps22hx',
-                                           'sns_sx932x',                 
-                                           'sns_bu52053nvx',
-                                           'sns_sc7a20'])
+        include_sensor_vendor_libs.extend(['sns_sc7a20'])
         env.Replace(SSC_INCLUDE_SENS_VEND_LIBS=include_sensor_vendor_libs)
 
         # TODO : Disable close to WAIPIO CS (Enable Registry Debug)
```

## 源码归档

### 归档文件

源码已从 134 服务器归档至本地，包含以下文件：

| 文件 | 说明 | 路径 |
|------|------|------|
| `por.py` | 补丁合入后的 `netrani/por.py`（完整文件） | [[./源码/por.py]] |
| `por.py.patch` | 补丁文件（来自 Gerrit #196014） | [[./源码/por.py.patch]] |

### 修改前后对比

**修改前**（7 项 vendor 库）：
```python
include_sensor_vendor_libs.extend(['lsm6dst',
                                   'sns_ak0991x',
                                   'sns_tmd2725',
                                   'sns_lps22hx',
                                   'sns_sx932x',                 
                                   'sns_bu52053nvx',
                                   'sns_sc7a20'])
```

**修改后**（仅保留 `sns_sc7a20`）：
```python
include_sensor_vendor_libs.extend(['sns_sc7a20'])
```

## 移植注意事项

### 跨芯片平台对比

此补丁仅修改 `netrani` 芯片平台的 `por.py`。其他芯片平台的 `por.py` 内容不同，如需移植到其他平台需注意：

| 芯片平台 | 文件路径 | 默认 vendor 库列表 | 差异点 |
|---------|---------|-------------------|--------|
| **netrani** (本补丁) | `ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py` | `lsm6dst, sns_ak0991x, sns_tmd2725, sns_lps22hx, sns_sx932x, sns_bu52053nvx, sns_sc7a20` | ✅ 已修剪，仅保留 `sns_sc7a20` |
| **fillmore** | `ADSP.HT.5.7/adsp_proc/ssc/chipset/fillmore/por.py` | `lsm6dst, sns_ak0991x, sns_tmd3702, sns_lps22hx, sns_sx932x, sns_bu52053nvx` | 使用 `sns_tmd3702`（而非 `tmd2725`），无 `sns_sc7a20` |
| **waipio** | `ADSP.HT.5.7/adsp_proc/ssc/chipset/waipio/por.py` | `lsm6dst, sns_lps22hx, sns_sx932x, sns_ak0991x, sns_tmd3702, sns_shtw2, sns_bu52053nvx` | 多一个 `sns_shtw2`，无 `sns_sc7a20` |
| **clarence** | `ADSP.HT.5.7/adsp_proc/ssc/chipset/clarence/por.py` | `lsm6dst, sns_ak0991x, sns_tmd2725, sns_lps22hx, sns_sx932x, sns_bu52053nvx` | 与 netrani 原始列表接近但无 `sns_sc7a20` |
| **waipio (SLPI)** | `SLPI.HY.5.0/slpi_proc/ssc/chipset/waipio/por.py` | (独立代码库，SLPI 侧) | 不在此次修改范围内 |

> ⚠️ **移植提醒**：`fillmore` 和 `waipio` 使用 `sns_tmd3702` 而非 `sns_tmd2725`；`waipio` 还包含 `sns_shtw2`。直接移植此补丁前需确认目标平台的 BOM 传感器列表。

## 引用文件索引

- `por.py` — SSC Netrani 平台 POR 编译配置文件（已归档至 [[./源码/por.py]]）
- `por.py.patch` — Gerrit #196014 补丁文件（已归档至 [[./源码/por.py.patch]]）

## 相关文档

- [[../Sensor裁剪SSC-vendor库-分析|[项目代号] Sensor 裁剪 SSC vendor 库 - 分析文档]]

---

_Author: wangguanran_