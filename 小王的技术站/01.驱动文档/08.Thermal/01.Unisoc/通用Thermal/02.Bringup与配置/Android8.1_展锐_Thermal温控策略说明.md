## Android 8.1（展锐）Thermal 温控策略说明（基于 `thermalSensorsConfig.xml`）

- **温度单位**: 配置中的温度为毫摄氏度（m°C），如 50000=50°C
- **采样**: 各 `sensor` 通过 `polling_delay` 轮询温度，并在 `AvgPeriod` 内做均值
- **分级**: `zone_level` 按温度阈值分级，进入级别后执行对应 `trip` 下的 `action`
- **保护**: 统一在 110°C 触发 `shutdown` 保护

### 1. 主板温区 `board-thmzone`
- **采样**: `polling_delay=2000ms`, `AvgPeriod=60000ms`
- **分级与动作**:
  - **Active (≥50°C)**
    - Trip0:
      - 设置温区开机阈值: 写 `70000` 至 `/sys/class/thermal/thermal_zone0/trip_point_0_temp`
      - 控温目标阈值: 写 `85000` 至 `/sys/class/thermal/thermal_zone0/trip_point_1_temp`
      - 屏幕亮度上限: 写 `255` 至 `/sys/class/backlight/sprd_backlight/max_brightness`
    - Trip2: 屏幕亮度上限降至 `191`
    - Trip4: 屏幕亮度上限降至 `172`
  - **Critical (≥110°C)**
    - Trip0: `shutdown`
- **说明**:
  - 通过提升/降低亮度上限控温；亮度台阶：`255 → 191 → 172`
  - 70/85°C 写入内核热区 trip 点，用于底层热控制协同

### 2. 充电温区 `chg-thmzone`
- **采样**: `polling_delay=2000ms`, `AvgPeriod=30000ms`
- **后备传感器**: `backup_sensor=board-thmzone`
- **分级与动作**:
  - **Active (≥50°C)**
    - Trip0: 写 `0` 至 `/sys/class/power_supply/battery/chg_cool_state`（关闭冷却充电策略）
    - Trip1:
      - 写 `500` 至 `/sys/class/power_supply/battery/adjust_cur_min`（最小充电电流 500mA）
      - 写 `1` 至 `/sys/class/power_supply/battery/chg_cool_state`（开启冷却充电）
  - **Hot (≥60°C)**
    - Trip0:
      - 写 `300` 至 `/sys/class/power_supply/battery/adjust_cur_min`（最小充电电流 300mA）
      - 写 `1` 至 `/sys/class/power_supply/battery/chg_cool_state`
  - **Critical (≥110°C)**
    - Trip0: `shutdown`
- **说明**:
  - 分级限流与冷却充电组合，50°C 开始干预，60°C 进一步降流

### 3. CPU 温区 `cpu-thmzone`
- **采样**: `polling_delay=2000ms`, `AvgPeriod=30000ms`
- **分级与动作**:
  - **Active (≥90°C)**
    - Trip0:
      - `/sys/class/thermal/cooling_device0/min_core_num = 4`
      - `/sys/class/thermal/cooling_device0/min_freq = 768000`（kHz）
    - Trip1:
      - `/sys/class/thermal/cooling_device0/min_core_num = 3`
  - **Hot (≥95°C)**
    - Trip0:
      - `/sys/class/thermal/cooling_device0/min_core_num = 2`
  - **Hot (≥100°C)**
    - Trip0:
      - `/sys/class/thermal/cooling_device0/min_core_num = 1`
  - **Critical (≥110°C)**
    - Trip0: `shutdown`
- **说明**:
  - 90/95/100°C 三档逐级减少最小在线核心数（4→3→2→1），并固定最低频 768MHz

### 4. 射频 PA 温区 `pa-thmzone`
- **采样**: `polling_delay=2000ms`, `AvgPeriod=60000ms`
- **分级与动作**:
  - **Active (≥60°C)**
    - Trip0: 发送 `AT+SPTPPB=0`（低级回退）
  - **Hot (≥70°C)**
    - Trip0: 发送 `AT+SPTPPB=2`（更强回退）
- **说明**:
  - 通过 AT 指令回退 PA 功率等级以降温

### 5. 行为汇总
- **亮度**: `255 → 191 → 172`
- **充电**: 50°C 启用冷却并限流（500mA），60°C 再降至 300mA
- **CPU**: 90/95/100°C 三档 `min_core_num: 4 → 3 → 2 → 1`，`min_freq = 768MHz`
- **PA**: 60/70°C 下发 `AT+SPTPPB`（0 → 2）
- **关机**: 任一温区 ≥110°C 触发 `shutdown`

### 6. 客制化建议（关键可调项，单位 m°C）
- `board-thmzone`: `trip_point_0_temp`（如 70000）、`trip_point_1_temp`（如 85000）、亮度台阶（255/191/172）
- `chg-thmzone`: 介入温度（50000/60000）、`adjust_cur_min`（500/300mA）、`chg_cool_state`
- `cpu-thmzone`: 核数/阈值（90/95/100°C）、`min_freq`（如 768000）
- `pa-thmzone`: AT 回退等级/阈值（60/70°C）

> 修改路径以 `action.file` 指向的 sysfs/控制接口为准；AT 指令需确保 Modem 支持。 