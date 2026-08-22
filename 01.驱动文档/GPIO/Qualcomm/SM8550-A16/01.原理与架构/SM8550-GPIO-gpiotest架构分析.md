# SM8550 GPIO 驱动原理与架构分析（gpiotest 视角）

## 平台概况

- **SoC**: Qualcomm SM8550（kalama）
- **Android**: 16（SRM969_A16_AutoTest 产测分支）
- **GPIO 体系**:
  - SoC TLMM GPIO：系统 GPIO 偏移约 300~500+（含 PMIC 扩展）
  - PMIC GPIO：PM8550 系列（PM8550VE/PM8550VW/PM8550VS/PMK8550），经 SPMI 访问
  - ADC：PMK8550 VADC（`c42d000.qcom,spmi/.../vadc@9000/iio:device0/in_voltage_pm8550_gpio*_adc_input`）

## GPIO 测试通路（gpiotest）

```
产测 APK/工具
   │ start meig_gpio_test (init service, late_start, root)
   ▼
/vendor/bin/SRM969_gpio.sh gpio_test
   │ echo N > /sys/class/gpio/export
   │ echo out/in > /sys/class/gpio/gpioN/direction
   │ echo 0/1 > /sys/class/gpio/gpioN/value
   ├── 成对环回：gpio_num1(out) ⇄ gpio_num2(in) 电平互验
   ├── 单引脚组：gpio_single_input1/2/3/4/6
   ├── SD 卡检测：/sys/devices/platform/soc/soc:meig_sd_and_sim_auto/sdcard_auto
   ├── PMIC ADC：vadc@9000 in_voltage_pm8550_gpio2/12_adc_input 阈值判断
   ├── LED：/sys/class/gpio/gpio318 + /sys/class/leds/blue/brightness
   ▼
/mnt/vendor/persist/gpiotest/gpio_test_final.ini（test_result=true/false）
```

## 关键设计点

1. **环回矩阵互斥**：已纳入环回矩阵的引脚（gpio389<->407、gpio317<->310）不再单独 export 做 LED 测试，避免 busy/方向覆盖，保证环回结果可信。
2. **偏移三段结构**：TLMM 主 GPIO、PMIC GPIO 扩展、单引脚特殊组分开管理，便于按网表维护。
3. **结果持久化**：写入 persist 分区，产测后可追溯。

_Author: wangguanran_
