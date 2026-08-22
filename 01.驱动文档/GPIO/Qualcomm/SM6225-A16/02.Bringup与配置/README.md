# Qualcomm SM6225 ([项目代号]/khaje) GPIO 移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：[项目代号]（基于 khaje/SM6225）
- **平台**：SM6225-A16（LA.VENDOR.13.2.1）
- **功能**：TLMM GPIO 引脚复用配置与全引脚测试

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| GPIO 控制器 | TLMM（pinctrl-msm） |
| 特殊功能引脚 | 部分 GPIO 复用为 I2C/UART/SPI 等功能 |
| 测试脚本 | `device/qcom/bengal_515/[项目代号]_gpio.sh` |

## 关键配置

### DTS pinmux（khaje.dtsi）

GPIO 引脚功能与 bias 配置必须与硬件设计一致：

```dts
&tlmm {
    <功能>_active: <功能>_active {
        mux {
            pins = "gpioXX";
            function = "xxx";
        };
        config {
            pins = "gpioXX";
            bias-pull-up;          // 或 bias-disable / bias-pull-down
            drive-strength = <2>;
        };
    };
};
```

### GPIO 测试脚本（[项目代号]_gpio.sh）

- 遍历所有 GPIO 引脚做功能验证
- 特殊功能引脚（I2C/UART/SPI 复用）需排除或特殊处理，否则误判失败
- 测试预期电平必须与 DTS 配置一致

## 编译与验证

```bash
# 编译 DTS（确认 khaje.dtsi 修改生效）
# 运行 GPIO 全引脚测试
adb shell sh /data/local/tmp/[项目代号]_gpio.sh

# 预期：全部引脚测试通过，特殊功能引脚不误报
```

## 常见问题排查

| 现象 | 排查方向 |
|:---|:---|
| 引脚无法输出预期电平 | 检查 DTS pinmux 的 function/bias 是否与硬件一致 |
| 特殊引脚误报失败 | 检查测试脚本是否排除 I2C/UART/SPI 等复用引脚 |
| 测试预期与配置矛盾 | 同步核对测试脚本预期值与 DTS 实际配置 |

## 相关资源

- 设备树配置：`[[01.驱动文档/GPIO/Qualcomm/SM6225-A16/02.Bringup与配置/91.源码与补丁索引/dt_config/khaje.dtsi|khaje.dtsi]]`
- 补丁历史：`[[01.驱动文档/GPIO/Qualcomm/SM6225-A16/02.Bringup与配置/91.源码与补丁索引/patches/|patches]]`
- 问题案例：[[01.驱动文档/GPIO/Qualcomm/SM6225-A16/04.问题案例/GPIO全引脚测试失败修复.md|GPIO全引脚测试失败修复]]

---

_Author: wangguanran_
