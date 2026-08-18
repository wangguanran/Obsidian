# SM6115 GPIO 驱动分析

> **模块**: 13.GPIO | **芯片**: SM6115 (scuba) | **平台**: SM6115-A14

## 概述

SM6115 平台 GPIO 子系统由四层构成：

1. **TLMM pinctrl 驱动**（`drivers/pinctrl/qcom/pinctrl-scuba.c`）：管理引脚复用（mux）、上下拉/驱动强度（config）与保留引脚（reserved）；
2. **GPIO 控制器**（gpiolib + msm gpio chip）：提供 gpiod 抽象，供各驱动与 DTS 引用；
3. **消费方驱动**：meig_gpio_pulse（UIC 脉冲字符设备）、gpio-userspace（复位脚 sysfs 导出）、gpio-keys（按键）；
4. **DT overlay**（scuba-iot-idp-overlay.dts）：MT5205 项目引脚功能分配与 pinctrl 状态定义。

## pinctrl-scuba.c 关键点

- `scuba_reserved_gpios[]`：定义不可被普通 GPIO 请求的保留引脚。MT5205 现状：`0, 1, 2, 3, 15, -1`（GPIO14 已释放给 MDB DET，GPIO16/17 已释放给 MDB UART）；
- 引脚复用数据表（scuba_pinctrl 等）由高通基线生成，MT5205 仅在保留列表上做裁剪。

## meig_gpio_pulse 驱动架构（/dev/uic_pulse）

### 分层结构

```
用户态
  ├── ioctl（SET_CONFIG/GET_CONFIG/START/STOP/RESET/FLUSH_EVENT/OUTPUT）
  ├── read() → struct uic_pulse_event（阻塞/非阻塞）
  ├── poll() → POLLIN / POLLOUT
  └── sysfs（/sys/class/misc/uic_pulse/：config/start/stop/output/event/in_raw...）
内核
  ├── uic_pulse_dev（in/out gpio_desc、irq、3 × hrtimer、kfifo(32)、miscdevice）
  ├── ISR → debounce_timer → 宽度统计 → batch_timer → kfifo 上报
  ├── stuck_timer（IN 持续有效超时判定）
  └── uic_pulse_emit（OUT 脉冲发射，mutex 串行 + monotonic deadline 等待）
DT
  └── compatible "meig,gpio-pulse" + in-gpios/out-gpios + 通道子节点
```

### probe 流程

1. `uic_pulse_probe()` 取第一个 available child node；
2. `uic_pulse_parse_dt()` 解析 min/max-width-us、debounce-us、default-emit-us、meig,in-active-high，填充默认 config；
3. 获取 in-gpios（要求 MMIO）/ out-gpios（GPIOD_OUT_LOW）；
4. in 存在时 `gpiod_to_irq()` + `devm_request_irq()`（IRQF_TRIGGER 由 irq_edge 决定）；
5. OUT 初始化为 idle 电平；
6. `misc_register()` → /dev/uic_pulse，注册 sysfs 属性组。

### 关键函数

| 函数 | 作用 |
|------|------|
| uic_pulse_isr | 边沿记录 + debounce 过滤，启动 debounce_timer |
| uic_pulse_debounce_fn | 去抖后确认有效电平；激活沿启动 stuck、重置 batch；非激活沿统计宽度并计批次 |
| uic_pulse_stuck_fn | 卡死超时置 stuck，该脉冲判无效 |
| uic_pulse_batch_fn | batch_gap_ms 到期合并上报 batch_count 条脉冲 |
| uic_pulse_emit | 按 simulate/config 参数在 OUT 上发出 count 个脉冲 |
| uic_pulse_apply_config | 校验 + 应用新配置（改 IRQ 触发类型、重置状态、OUT idle） |
| uic_pulse_validate_config | 参数合法性校验（宽度 25~500、debounce ≤100、stuck ≤60000 等） |

### 事件流

GPIO32 脉冲 → IRQ → debounce(2ms) → 宽度在 [10,500]ms 且未 stuck → batch 计数 → batch_gap(200ms) 空闲 → kfifo 写入 `{pulse_count, last_width_ms, timestamp_ns}` → wake_up → read()/poll() 返回。

## gpio-userspace 驱动

`drivers/misc/gpio-userspace.c`：将 DT 中配置的 GPIO 导出为 `/sys/class/gpio_userspace/<label>/value`，default-state（0-low/1-high/2-input）。MT5205 使用：

- se_reset（GPIO102，默认高）、mdb_reset（GPIO36，空闲输出高）——均低有效复位脚。

## DT overlay 引脚分配要点（scuba-iot-idp-overlay.dts）

- pinctrl 状态：`mt5205_se_reset`、`mt5205_mdb_reset`（output-high）、`mt5205_mdb_db_detect`（GPIO14 输入 pull-up）、`mt5205_pulse_default`（GPIO32 输入 pull-up / GPIO33 输出 8mA）；
- 根节点：`model = "Qualcomm Technologies, Inc. Scuba IOT IDP"`（实机为 Scuba IOT IDP，不是 Bengal）；
- overlay 以 `/plugin/` 方式编译，节点挂在 &tlmm / &soc 下。

## 参考

- [[01.驱动文档/13.GPIO/Qualcomm/SM6115-A14/03.需求实现/UIC-Pulse-GPIO32检测与GPIO33输出驱动.md|UIC-Pulse-GPIO32检测与GPIO33输出驱动]]
- [[01.驱动文档/13.GPIO/Qualcomm/SM6115-A14/03.需求实现/MDB-nRST输出高与GPIO14检测按键.md|MDB-nRST输出高与GPIO14检测按键]]
- 源码：[[01.驱动文档/13.GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/meig_gpio_pulse.c|meig_gpio_pulse.c]] | [[01.驱动文档/13.GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/pinctrl-scuba.c|pinctrl-scuba.c]] | [[01.驱动文档/13.GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/uic_pulse.h|uic_pulse.h]]

---

_Author: wangguanran_