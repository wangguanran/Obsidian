# 分析：SM8550 gpiotest GPIO 全引脚测试脚本需求实现

**版本号：v1.0**
**对应文档：** SM8550-gpiotest-GPIO全引脚测试脚本.md

## 技术背景

SM8550（kalama）是高通旗舰平台，SoC 侧含 TLMM GPIO（约 200 个编号到 500+ 的系统 GPIO 偏移）与 PMIC 侧 PM8550 系列 GPIO。产测场景需要对全部 GPIO 做开短路环回测试：将成对 GPIO 一个配置为输出、另一个配置为输入，输出端拉高后输入端回读，验证板级连线的电气完整性。

本平台的 gpiotest 以 shell 脚本形式集成在 vendor 分区（`/vendor/bin/gpiotest_gpio.sh`），由 init 服务 `meig_gpio_test` 在产测模式触发，测试结果写入 persist 分区 `gpio_test_final.ini` 供产测工具解析。

## 代码改动分析

### device/qcom/kalama/gpiotest_gpio.sh（RENAMED +105/-72）

核心改动：

1. **引脚表重排**：`gpio_num1`/`gpio_num2` 更新为 [项目代号] 平台的系统 GPIO 偏移（303~500 区间），并加入 PM8550VE/PMK8550 等 PMIC GPIO 项（284~289、255~268 等偏移段），对应 SPMI 侧 PMIC GPIO 映射。
2. **测试段结构**（与 [项目代号] 相同）：
   - `gpioAllTest_PullUp`：num1 组输出 1，num2 组输入回读
   - `gpioAllTest_PullDown`：输出 0 后回读
   - `gpio_single_input*`：单引脚分组验证
   - `sdcard_auto`：SD 卡自动检测节点（`meig_sd_and_sim_auto`）状态验证
   - `[ADCTest]`：PM8550 GPIO2/GPIO12 ADC 电压阈值判断（2 < 1.9V、12 < 1.8V）
   - `[LEDTest]`：blue LED（gpio318）点亮/熄灭验证
   - `[final_result]`：汇总 test_result
3. **逻辑修正**：删除 green(gpio389)/red(gpio317) LED 测试——389/317 已加入环回矩阵（389<->407、317<->310），重复 export 会 busy 并覆盖方向，导致环回结果不可信。blue(gpio318) 不在矩阵中保留 LED 测试。这是本次改动中**最有价值的行为修正**，解决了两个测试段互相干扰的问题。

### device/qcom/kalama/AndroidBoard.mk / kalama.mk / init.target.rc

三处 +1/-1 的机械替换：脚本模块名从 `gpiotest_gpio.sh` 统一改为 `gpiotest_gpio.sh`，保证打包产物、PRODUCT_PACKAGES 与 init service 引用一致。

## 潜在风险

1. **引脚表准确性依赖网表**：替换后的引脚偏移（303~500+）若与 [项目代号] 实际 TLMM/PMIC 映射不符，会产生误判 false 或测试死等（export 失败）。需要产线首台样机实测校准。
2. **PMIC GPIO 依赖 SPMI 节点路径**：`c42d000.qcom,spmi` 路径为 kalama 平台固定地址，若后续平台变体改动地址，脚本会静默读取失败（`cat` 返回非数字导致 `[` 判断报错）。
3. **`sdcard_auto` 节点依赖自研驱动**：`meig_sd_and_sim_auto` 为美格自研 misc 驱动节点，若该驱动未合入则测试段报错。
4. **shell 无 set -e**：单个 GPIO 失败不会中止脚本，多失败时仅 test_result=0 汇总，定位需要逐条看 ini 文件。

## 回归测试建议

- 产线全引脚环回测试冒烟（PullUp/PullDown 两轮全通过）
- PM8550 GPIO2/GPIO12 ADC 阈值验证
- blue LED 点亮/熄灭
- SD 卡自动检测节点
- 与环回矩阵的 green/red LED 互斥性验证（确保无 busy）

## 与现有驱动架构的关系

- 纯用户态脚本，不涉及内核驱动改动
- 依赖内核侧：GPIO sysfs（`/sys/class/gpio`）、SPMI VADC（`in_voltage_pm8550_gpio*_adc_input`）、自研 misc 驱动（`meig_sd_and_sim_auto`）
- 与 SM6115-A14 平台的 gpiotest 机制同源（同为 shell + sysfs 环回方案），SM8550-A16 为首份归档

_Author: wangguanran_
