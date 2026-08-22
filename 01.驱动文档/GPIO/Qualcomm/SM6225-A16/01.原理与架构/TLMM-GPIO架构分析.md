# Qualcomm SM6225 ([项目代号]/khaje) TLMM GPIO 架构分析

> **版本号：v1.0**

## 平台概述

[项目代号]（基于 khaje/SM6225）的 GPIO 由 **TLMM**（Top Level Mode Multiplexer）统一管理：

```
应用/内核驱动
   ↓ gpiolib / pinctrl API
TLMM (pinctrl-msm)
   ├─ pinmux: 引脚功能复用（gpio/i2c/uart/spi/...）
   ├─ bias:   上拉/下拉/高阻
   └─ drive:  驱动强度
        ↓
SoC 引脚 (GPIO0~GPIO130+)
```

## 引脚配置机制

DTS 中通过 pinctrl 节点定义引脚状态：

```dts
&tlmm {
    <功能>_active: <功能>_active {
        mux {
            pins = "gpioXX";
            function = "xxx";      // 复用功能
        };
        config {
            pins = "gpioXX";
            bias-pull-up;          // 或 bias-disable / bias-pull-down
            drive-strength = <2>;
        };
    };
};
```

## GPIO 全引脚测试机制

`[项目代号]_gpio.sh` 遍历所有 GPIO 引脚做功能验证。测试失败的两类根因：

| 根因 | 说明 |
|:---|:---|
| DTS pinmux 与硬件不匹配 | 引脚 function/bias 配置与实际设计不一致，引脚无法正常输出预期电平 |
| 测试脚本预期值错误 | 脚本未识别 DTS 中已配置的特殊功能引脚（如 I2C/UART/SPI 复用脚），误判为失败 |

## 修复要点（#195832）

1. `khaje.dtsi`：修正目标 GPIO 引脚的 pinmux 配置（function/bias 与硬件一致，+1/−1）
2. `[项目代号]_gpio.sh`：补充正确测试逻辑（+5 行），排除特殊功能引脚误判（+5/−27 净调整）

## 引用文件索引

- [[01.驱动文档/GPIO/Qualcomm/SM6225-A16/04.问题案例/GPIO全引脚测试失败修复.md|GPIO全引脚测试失败修复]]（补丁内容）
- `kernel_platform/qcom/proprietary/devicetree/qcom/khaje.dtsi`（远程）
- `device/qcom/bengal_515/[项目代号]_gpio.sh`（远程）

---

_Author: wangguanran_
