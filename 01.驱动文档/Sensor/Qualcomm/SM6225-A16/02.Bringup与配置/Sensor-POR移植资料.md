# Qualcomm SM6225 ([项目代号]/divar) Sensor POR 移植资料

> **版本号：v1.0**

## 芯片信息

- **平台**：[项目代号]（divar/SM6225），Android 16
- **ADSP 版本**：VT.5.4.3.c1
- **功能**：ADSP SSC 传感器上电时序（POR）配置

## 关键配置（por.py）

文件：`ADSP.VT.5.4.3.c1/adsp_proc/ssc/chipset/divar/por.py`

```python
# 传感器上电/复位时序参数
# GPIO 控制逻辑（供电/复位引脚）
# 各传感器 POR 序列定义
```

## 修改内容（#195840）

| 项目 | 内容 |
|:---|:---|
| 变更 | por.py 传感器 POR 配置（+10/−8） |
| 内容 | 修正上电/复位时序参数、调整 GPIO 控制逻辑、匹配实际硬件 |
| DTS | 无变更（ADSP 侧配置） |
| Kernel config | 无变更 |

## 编译与验证

```bash
# 编译 ADSP 镜像（含 SSC）并烧录
# 运行 sensor 测试，预期全部通过

# 通过 ADSP 日志确认传感器初始化状态
logcat -b all | grep -i sensor
# 或查看 ADSP 端日志
```

## 移植注意事项

- por.py 按 chipset 分目录（divar/netrani 等），改错平台文件不生效
- 传感器型号变更时需同步调整 GPIO 控制与上电延迟参数

## 引用文件索引

- [[01.驱动文档/Sensor/Qualcomm/SM6225-A16/04.问题案例/SensorPOR配置修复.md|SensorPOR配置修复]]（补丁内容）
- `ADSP.VT.5.4.3.c1/adsp_proc/ssc/chipset/divar/por.py`（远程 Gerrit 仓库）

---

_Author: wangguanran_
