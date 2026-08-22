# Qualcomm SM6225 ([项目代号]/divar) ADSP SSC Sensor POR 机制分析

> **版本号：v1.0**

## 平台概述

[项目代号]（divar/SM6225）的传感器由 ADSP SSC 管理。SSC 在 ADSP 侧通过 **POR（Power-On Reset Sequence）配置**控制各传感器的上电时序：

```
传感器硬件 (I2C + GPIO)
   ↑ 上电信号/复位时序（由 por.py 定义）
ADSP SSC (por.py)
   ├─ 传感器 GPIO 控制
   ├─ 上电延迟 / 复位时序参数
   └─ 初始化序列 → 传感器就绪
```

## por.py 的作用

`ADSP.VT.5.4.3.c1/adsp_proc/ssc/chipset/divar/por.py` 是 ADSP 侧 SSC 的传感器上电时序配置文件，定义了各传感器的：

1. **上电序列**（Power-On Reset Sequence）
2. **GPIO 控制逻辑**（传感器供电/复位引脚）
3. **时序参数**（上电延迟、复位时长）

## 问题与修复（#195840）

- **问题**：sensor test failed —— por.py 中的 POR 配置与实际传感器硬件不匹配，上电时序不正确、初始化失败
- **修复**：调整 `por.py` 传感器上电/复位时序参数 + GPIO 控制逻辑（+10/−8），匹配实际硬件

## 关键要点

- **生效方式**：por.py 在 ADSP 固件编译时生效，**不经过 DTS**
- **调试入口**：ADSP 日志（`logcat -b all | grep -i sensor` 或 ADSP 端日志）确认传感器初始化状态

## 引用文件索引

- [[01.驱动文档/Sensor/Qualcomm/SM6225-A16/04.问题案例/SensorPOR配置修复.md|SensorPOR配置修复]]（补丁内容）
- `ADSP.VT.5.4.3.c1/adsp_proc/ssc/chipset/divar/por.py`（远程 Gerrit 仓库 `iot-high-mid-2024-spf-3-0_amss_standard_oem`）

---

_Author: wangguanran_
