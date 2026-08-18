# 分析：UIC-Pulse-GPIO32检测与GPIO33输出驱动

**版本号：v1.0**
**对应文档：** UIC-Pulse-GPIO32检测与GPIO33输出驱动.md

## 技术背景

UIC（POS 收银机）场景下，IO 接口板（Pulse 接口-1）通过 GPIO32 向 AP 上报脉冲信号，AP 通过 GPIO33 向 MDB 小板（Pulse 接口-2）发出脉冲。两条走线为直连同相（same-phase），空闲低电平、脉冲为高电平。脉冲宽度与间隔由外设决定，驱动需要在去抖、有效性窗口、批次合并、卡死超时几个维度上做滤波与规整，并通过字符设备把事件交给用户态。

Linux 侧实现此类脉冲检测的常用组合是 GPIO IRQ + hrtimer（高精度定时器），配合 kfifo 做事件缓冲、waitqueue/poll 做阻塞读取。本驱动即采用该经典结构，并额外提供 GPIO33 的 ioctl 脉冲发射能力。

## 代码改动分析（逐文件）

### 1. drivers/misc/meig_gpio_pulse.c（+987 行，新文件）

- **数据结构** `struct uic_pulse_dev`：in/out 两个 `struct gpio_desc`、irq、`struct uic_pulse_config cfg`、三个 hrtimer（debounce_timer / stuck_timer / batch_timer）、kfifo（32 深度）、miscdevice、spinlock + mutex + waitqueue。
- **极性模型**：
  - `uic_pulse_raw_is_active()`：按 `cfg->active_level`（ACTIVE_HIGH=raw!=0 / ACTIVE_LOW=raw==0）判断 IN 有效电平；
  - OUT 侧 `uic_pulse_out_active_raw()/uic_pulse_out_idle_raw()`：ACTIVE_HIGH 时 active=1、idle=0，ACTIVE_LOW 反之。
- **检测流程**（ISR → debounce → 宽度统计 → 批次）：
  - `uic_pulse_isr()`：记录 `t_last_edge`，与上次边沿间隔小于 debounce_ms 则丢弃；否则启动/重启 debounce_timer；
  - `uic_pulse_debounce_fn()`：去抖结束后读原始电平，若与 `in_active` 状态翻转则更新；激活沿启动 stuck_timer 并取消未到期 batch_timer（新脉冲到达，批次窗口重新计时）；非激活沿计算宽度 `width_ms`，落在 [min_valid_ms, max_valid_ms] 内且未 stuck 才计数：
    - batch_gap_ms==0 → 立即 `uic_pulse_push_event()`；
    - 否则启动 batch_timer，到期后按 `batch_count` 合并上报；
  - `uic_pulse_stuck_fn()`：IN 有效时间超过 stuck_timeout_ms 置 `stuck=true`，该脉冲判无效。
- **发射流程** `uic_pulse_emit()`：按 simulate 参数（count/pulse_width_ms/interval_ms/out_active_level，0 或 CFG 时取 config）循环拉高/拉低 OUT；`uic_pulse_wait_until()` 基于 monotonic deadline 补偿 usleep 超调（剩余 <2ms 用 udelay 忙等）。
- **用户态接口**：misc 字符设备 /dev/uic_pulse（`uic_pulse_fops`）：
  - ioctl：SET_CONFIG（校验+应用）、GET_CONFIG、START/STOP/RESET、FLUSH_EVENT（清空 kfifo）、OUTPUT（发射）；
  - read()：阻塞（wait_event_interruptible）或 O_NONBLOCK（-EAGAIN），返回 `struct uic_pulse_event`；
  - poll()：fifo 非空 → EPOLLIN，有 out → EPOLLOUT。
  - sysfs 属性（`/sys/class/misc/uic_pulse/`）：config（10 参数读写）、各字段快捷属性、start/stop/reset/flush_event/output（写）、event/started/in_raw（读）。
- **DT 解析** `uic_pulse_parse_dt()`：min-width-us/max-width-us/debounce-us/default-emit-us（us→ms）/meig,in-active-high；probe 取第一个 available child node；in-gpios 要求 MMIO（gpiod_cansleep 检查），out-gpios 允许任意；`compatible = "meig,gpio-pulse"`。

### 2. include/uapi/linux/uic_pulse.h（+88 行，新文件）

定义 `uic_pulse_config`（10 字段）、`uic_pulse_simulate`（count/width/interval/level）、`uic_pulse_event`（pulse_count/last_width_ms/timestamp_ns）与 ioctl 命令（magic 'U'，0x01~0x07）。

### 3. bindings/misc/meig,gpio-pulse.txt（+85 行，新文件）

binding 文档：compatible、channel 子节点属性（label/in-gpios/out-gpios/debounce-us/min-width-us/max-width-us/default-emit-us/meig,in-active-high/meig,mask-irq-on-emit/pinctrl）、运行时参数、ioctl/sysfs 说明、MT5205 直连示例与极性说明。

### 4. scuba-iot-idp-overlay.dts（+46/-0）

新增 `mt5205_pulse_default` pinctrl（GPIO32 输入 + bias-pull-up、GPIO33 输出 drive-strength 8 + output-low）与 `meig_pulse` 节点（in-gpios GPIO32 + meig,in-active-high、out-gpios GPIO33、debounce 2000us、min 10ms、max 500ms、default-emit 50ms）。

### 5. Kconfig / Makefile / bengal_GKI.config

`CONFIG_MEIG_GPIO_PULSE` tristate（depends on OF && GPIOLIB），模块名 meig_gpio_pulse，`=m` 写入 bengal_GKI.config。

## 潜在风险

- **发射与检测互斥**：OUT 发射为忙等/睡眠式（mutex 串行），发射期间 IN 检测仍进行，直连回环时 OUT 高电平会被 IN 当作输入脉冲；binding 提供 `meig,mask-irq-on-emit` 选项但当前驱动未实现，若回环误报需在用户态忽略或后续补充。
- **debounce 与宽度窗口**：debounce_ms 上限 100ms，min/max_valid_ms 上限 10s，stuck_timeout 上限 60s；极端配置（debounce 接近 min_valid）会滤掉合法脉冲，需按实际信号调参。
- **hrtimer 与 spinlock**：timer 回调在软中断/进程上下文，全部通过 spin_lock_irqsave 保护共享状态，未使用 sleepable API，结构正确；但 `uic_pulse_debounce_fn` 内调用 `uic_pulse_in_raw()`（gpiod_get_raw_value）在持锁前执行，避免 GPIO 慢访问持锁。
- **批量窗口语义**：batch_gap_ms 期间持续有脉冲会不断推迟上报（新脉冲取消 batch_timer），高频率长脉冲串会延迟事件，用户态需考虑实时性要求。
- **UAPI 稳定性**：ioctl 结构体为固定 __u32 布局，扩展字段需走新命令号，避免破坏 ABI。

## 回归测试建议

1. 回环测试：GPIO33 发射 1/3/10 个脉冲（宽度 25/50/100ms），GPIO32 侧 read() 应收到对应 batch（batch_gap_ms=200 时一次 10 个脉冲合并为 1 条事件）。
2. 极性回归：分别以 ACTIVE_HIGH / ACTIVE_LOW 配置，确认 in_raw 空闲值与事件极性。
3. 边界：宽度 <min_valid_ms、>max_valid_ms、持续拉高超过 stuck_timeout_ms 均不应上报。
4. 并发：read + ioctl SET_CONFIG 同时进行，确认无死锁、无 kfifo 溢出（32 深度满后丢弃新事件，poll 行为正确）。
5. 休眠唤醒：echo mem > /sys/power/state 后 GPIO32 脉冲应唤醒并正常上报。

## 与现有驱动架构的关系

- 属于 misc 字符设备类驱动（与同树 gpio-userspace 并列于 drivers/misc/），共享 bengal_GKI.config 的模块化编译方式；
- pinctrl 部分与 #195886（MDB nRST/GPIO14）同文件（scuba-iot-idp-overlay.dts），两变更叠加后 overlay 为最终版本；
- 与 gpio-userspace 的 GPIO36/GPIO102 复位脚管理互不冲突（不同 GPIO）。

---

_Author: wangguanran_
