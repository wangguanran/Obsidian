# SGM832 电源监控驱动架构

## 芯片功能

SGM832 为 SG Micro 电源监控芯片，通过 I2C 读取电源参数（电压/电流等），支持中断通知（irq_gpio），并提供 power_supply 注册（usb_psy）。

## 驱动结构

```
sgm832_data
├── i2c_client       # I2C 通信
├── delayed_work     # 周期采样（delaywork_func 读 0x01 寄存器）
├── kobject (bl_kobj)# sysfs 背光/状态导出
├── irq_gpio         # 中断 GPIO
└── power_supply *usb_psy  # 上报 USB 电源状态
```

## 关键函数

| 函数 | 作用 |
|------|------|
| `delaywork_func()` | 周期读寄存器 0x01 采样电源数据 |
| `swapEndianness()` | 字节序交换（大端读取） |

## 配套外设

- **cash_drawer.c**：钱箱 GPIO 驱动，char device + INPUT 事件，检测/控制钱箱开合
- **ext-adc-gpio.c**：外部 ADC GPIO 通道，sysfs 导出，compatible `ext,adc-driver`

## 配置宏

- `CONFIG_SGM832=m`
- `CONFIG_EXT_ADC_GPIO=m`
- `CONFIG_CASH_DRAWER_DRIVER=m`

_Author: wangguanran_
