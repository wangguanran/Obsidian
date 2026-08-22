# SM8550-A16 gpiotest 移植/Bringup 配置资料

> 本平台首份文档：GPIO 全引脚测试脚本（Change #197519）需求实现

## 芯片信息

- **型号**: Qualcomm SM8550（kalama）
- **厂商**: Qualcomm
- **功能**: 旗舰 SoC，产测分支 SRM969_A16_AutoTest

## 平台信息

- **SoC**: SM8550 | **Android**: 16 | **内核**: LA.VENDOR.13.2.9
- **134 源码树**: ⚠️ 不可用（LA.VENDOR.13.2.9 未部署）

## GPIO 测试相关配置

| 配置项 | 位置 | 说明 |
|--------|------|------|
| 脚本打包 | device/qcom/kalama/AndroidBoard.mk | LOCAL_MODULE := SRM969_gpio.sh |
| 产品集成 | device/qcom/kalama/kalama.mk | PRODUCT_PACKAGES += SRM969_gpio.sh |
| init 服务 | device/qcom/kalama/init.target.rc | service meig_gpio_test（late_start/disabled/oneshot） |
| 结果路径 | /mnt/vendor/persist/gpiotest/gpio_test_final.ini | 测试结果 |

## 测试触发命令

```bash
adb shell start meig_gpio_test
# 或
adb root && adb shell /vendor/bin/sh /vendor/bin/SRM969_gpio.sh gpio_test
```

## 参考资料

- 主文档：[[01.驱动文档/GPIO/Qualcomm/SM8550-A16/03.需求实现/SM8550-gpiotest-GPIO全引脚测试脚本.md|SM8550-gpiotest-GPIO全引脚测试脚本.md]]
- 补丁：[[01.驱动文档/GPIO/Qualcomm/SM8550-A16/91.源码与补丁索引/patches/197519.patch|197519.patch]]

_Author: wangguanran_
