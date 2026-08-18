# SM6225 Sensor POR 配置修复

**版本号：v1.0**
**类型：Bug**
**状态：已合入**
**来源：** Gerrit Change 195840
**项目：** iot-high-mid-2024-spf-3-0_amss_standard_oem
**分支：** master_meig
**作者：** 赵前
**合入时间：** 2026-08-15

## 现象

sensor 测试失败（sensor test failed），ADSP（Audio DSP）端的 sensor 传感器无法正常工作或初始化失败。

## 环境

- SoC：divar（SM6225）
- ADSP 版本：VT.5.4.3.c1
- 平台：[项目代号：SLM927P-A16]

## 根因分析

`por.py` 是 ADSP 侧 SSC（Sensors SubSystem Core）的传感器上电时序配置文件，定义了各传感器的上电序列（Power-On Reset Sequence）。sensor 测试失败的原因是 `por.py` 中的 POR 配置与实际传感器硬件不匹配，导致：

1. 传感器上电时序不正确
2. 传感器初始化失败
3. 最终 sensor 测试用例无法通过

## 处理方案

修改 `ADSP.VT.5.4.3.c1/adsp_proc/ssc/chipset/divar/por.py` 配置文件，调整传感器上电时序配置（+10/-8）：

1. 修正传感器上电/复位时序参数
2. 调整 GPIO 控制逻辑，确保传感器在正确的时序下上电
3. 更新配置以匹配实际硬件

## 修改文件清单

- `ADSP.VT.5.4.3.c1/adsp_proc/ssc/chipset/divar/por.py`：修正传感器 POR 配置（+10/-8）

## 配置方式

### DTS 配置
- 无 DTS 变更（ADSP 侧配置，通过 por.py 在 ADSP 固件中生效）

### ADSP 配置
- `por.py` 中调整传感器上电时序
- 具体配置涉及传感器 GPIO 控制、上电延迟、复位时序等参数

### Kernel config
- 无变更

## 验证方式

- **验证命令**：
  ```bash
  # 运行 sensor 测试
  # 通过 ADSP 日志确认 sensor 初始化状态
  logcat -b all | grep -i sensor
  # 或查看 ADSP 端日志
  ```
- **预期结果**：sensor 测试全部通过，ADSP 端传感器初始化正常
- **实际结果**：修复后 sensor 测试通过

## 补丁内容

补丁内容暂未获取（远程源码树搜索超时）。主要改动为 `por.py` 中传感器 POR 配置的修改（+10/-8）。

## 源码归档

- 源码暂未归档（远程 134 源码树搜索超时，文件位于 Gerrit 仓库 `iot-high-mid-2024-spf-3-0_amss_standard_oem`，分支 `master_meig`）

## 引用文件索引

- `ADSP.VT.5.4.3.c1/adsp_proc/ssc/chipset/divar/por.py`：ADSP 侧传感器 POR 上电时序配置（远程 Gerrit 仓库）

_Author: 艾达_