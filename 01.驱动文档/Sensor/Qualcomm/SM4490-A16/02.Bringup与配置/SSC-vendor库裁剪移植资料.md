# [项目代号] (parrot) SSC vendor 库裁剪移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：[项目代号]（parrot/QCM4490 衍生）
- **SSC 平台**：Netrani
- **平台**：SM4490-A16
- **功能**：裁剪 ADSP SSC 镜像中的冗余传感器 vendor 库

## 背景

`include_sensor_vendor_libs` 默认列表包含多种参考设计传感器库，但 [项目代号] BOM 仅搭载 sc7a20，多余库导致 SSC 镜像体积增大。

## 关键配置（por.py）

```python
env.Replace(SSC_INCLUDE_SENS_VEND_LIBS=include_sensor_vendor_libs)
# 修改 include_sensor_vendor_libs: 移除 BOM 未搭载的传感器库，仅保留 sns_sc7a20
```

## 编译与验证

```bash
# 编译 ADSP SSC 镜像
# → 编译通过且无链接错误

# 烧录镜像后验证
adb shell ls /sys/bus/iio/devices/   # 仅 sns_sc7a20 相关设备节点
adb shell getprop | grep -i sensor   # 确认传感器服务正常
```

## 移植注意事项

- 仅修改 netrani 芯片平台的 `por.py`，其他平台 `por.py` 内容不同
- `fillmore`/`waipio` 使用 `sns_tmd3702`；`waipio` 额外含 `sns_shtw2` —— 移植前确认目标平台 BOM 列表
- 修改后需完整重编 ADSP SSC 镜像并烧录验证

## 引用文件索引

- [[01.驱动文档/Sensor/Qualcomm/SM4490-A16/91.源码与补丁索引/kernel_driver/ADSP.HT.5.7/adsp_proc/ssc/chipset/netrani/por.py|por.py]]（已归档源码）
- [[01.驱动文档/Sensor/Qualcomm/SM4490-A16/03.需求实现/源码/por.py.patch|por.py.patch]]（补丁）
- [[01.驱动文档/Sensor/Qualcomm/SM4490-A16/03.需求实现/Sensor裁剪SSC-vendor库.md|Sensor裁剪SSC-vendor库]]（完整需求文档）

---

_Author: wangguanran_
