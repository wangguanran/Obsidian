# SLM927P GPIO全引脚测试失败修复

**类型**：Bug (test failed)
**状态**：MERGED
**Gerrit Change**：#195832
**项目**：LA.VENDOR.13.2.1
**分支**：master_IOT_High_Mid_2024.SPF.3.0_SLM927x_SLM550x
**作者**：zhaoqian（实际解决：wanghao_sh）
**芯片平台**：SLM927P（基于 khaje/SM6225）
**SoC-Android**：SM6225-A16
**模块**：13.GPIO

---

## 现象

GPIO 全引脚测试（gpio full pin test）在 SLM927P 平台上测试失败。测试脚本遍历所有 GPIO 引脚进行功能验证时，部分 GPIO 引脚状态异常，导致测试用例无法通过。

具体表现为：

- 测试脚本执行过程中，特定 GPIO 引脚无法正确输出预期电平
- GPIO 引脚配置与硬件实际连接不匹配
- 测试脚本的预期判断逻辑与硬件实际状态不符

---

## 根因分析

经过排查，发现以下两个问题：

### 1. DTS 配置错误

在 `khaje.dtsi` 设备树文件中，某个 GPIO 引脚的引脚复用（pinmux）配置与实际硬件设计不一致。GPIO 的引脚功能（function）或上拉/下拉状态（bias）配置错误，导致该引脚在测试时无法正常工作。

### 2. 测试脚本逻辑问题

`SLM927P_gpio.sh` 测试脚本中，对某些 GPIO 引脚的测试预期值设置不正确，与 DTS 中的实际配置相矛盾。测试脚本未能正确识别 DTS 中已配置的特殊引脚（如用作特定功能而非普通 GPIO 的引脚），导致误判为失败。

---

## 处理方案

### 补丁概览

本次修复涉及 **2 个文件**的修改：

| 文件 | 修改 | 说明 |
|------|------|------|
| `device/qcom/bengal_515/SLM927P_gpio.sh` | +5 / -27 | 修复 GPIO 测试脚本，修正测试预期逻辑 |
| `kernel_platform/qcom/proprietary/devicetree/qcom/khaje.dtsi` | +1 / -1 | 修正 DTS 中 GPIO 引脚配置 |

### 详细修改说明

#### 1. DTS 修改（khaje.dtsi）

将目标 GPIO 引脚的设备树配置从错误状态修正为正确的引脚复用模式，确保硬件引脚的 function / bias 配置与实际硬件设计一致。

#### 2. 测试脚本修改（SLM927P_gpio.sh）

- 精简脚本，移除冗余或不正确的测试用例（-27 行）
- 补充正确的测试逻辑（+5 行），使测试预期值与 DTS 配置保持一致
- 修正对特殊功能引脚的判断逻辑，排除非普通 GPIO 引脚的误判

> 补丁详细内容请参考 Gerrit Change #195832。

---

## 配置方式

### 设备树配置

修改 `khaje.dtsi` 中对应 GPIO 节点的 pinmux 配置：

```dts
&tlmm {
    /* 目标 GPIO 引脚配置 */
    gpio_xxx {
        mux {
            pins = "gpio_xxx";
            function = "gpio";  /* 或其他正确功能 */
        };
        config {
            pins = "gpio_xxx";
            drive-strength = <2>;     /* 驱动强度 */
            bias-pull-down;           /* 下拉 / 上拉 / 无偏置 */
            output-low;               /* 默认输出低电平 */
        };
    };
};
```

> 具体引脚号和配置值请参考 Gerrit Change #195832 中的补丁内容。

### 测试脚本配置

确保 `SLM927P_gpio.sh` 中：

- 对特殊功能引脚（如 I2C、UART、SPI 等复用引脚）进行排除或特殊处理
- 测试预期电平与 DTS 配置一致
- 脚本遍历逻辑正确覆盖所有可测试 GPIO 引脚

---

## 验证方式

### 验证步骤

1. **编译内核并烧录镜像**
   - 编译修改后的 DTS，生成 dtb/dtbo
   - 烧录包含补丁的 boot 镜像到设备

2. **执行 GPIO 全引脚测试**
   ```bash
   # 进入测试脚本目录
   cd /device/qcom/bengal_515/
   # 执行 GPIO 测试脚本
   sh SLM927P_gpio.sh
   ```

3. **检查测试结果**
   - 所有 GPIO 引脚测试通过
   - 无 FAIL 或 ERROR 输出

### 预期结果

- GPIO 全引脚测试 100% 通过
- 特殊功能引脚正确排除或单独验证
- 测试日志中无异常报错

> ⚠️ 补丁验证：当前无法直接获取验证日志，请参考 Gerrit Change #195832 上的 CI 测试结果。

---

## 源码归档

### 涉及文件

- [[01.驱动文档/13.GPIO/Qualcomm/SLM927P/SM6225-A16/03.设备树/README|设备树配置]]
- [[01.驱动文档/13.GPIO/Qualcomm/SLM927P/SM6225-A16/02.驱动/GPIO驱动概述|GPIO 驱动概述]]

### 相关源码路径

| 文件路径 | 仓库 |
|----------|------|
| `device/qcom/bengal_515/SLM927P_gpio.sh` | device/qcom/bengal_515 |
| `kernel_platform/qcom/proprietary/devicetree/qcom/khaje.dtsi` | kernel_platform |

---

## 引用文件索引

- Gerrit Change #195832 — 补丁详情
- [[01.驱动文档/13.GPIO/Qualcomm/SLM927P/SM6225-A16/04.问题案例/分析/SLM927P-GPIO全引脚测试失败修复-分析|分析文档]]
- [[01.驱动文档/13.GPIO/Qualcomm/SLM927P/SM6225-A16/03.设备树/khaje.dtsi|khaje.dtsi 设备树]]
- [[01.驱动文档/13.GPIO/Qualcomm/SLM927P/SM6225-A16/02.驱动/GPIO测试脚本|GPIO 测试脚本]]

---

_Author: wangguanran_