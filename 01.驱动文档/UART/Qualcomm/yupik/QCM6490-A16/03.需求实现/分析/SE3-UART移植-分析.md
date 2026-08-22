# 分析：QCM6490 SE3 UART 移植

**版本号：v1.0**
**对应文档：** QCM6490 SE3 UART 移植

## 技术背景

### QCM6490 平台 UART 架构

QCM6490 基于 Qualcomm Kryo 670 CPU（ARM Cortex-A78/A55），其外设接口通过 QUPv3（Qualcomm Universal Peripheral v3）控制器管理。QUPv3 是一个可配置的串行接口控制器，支持 UART、I2C、SPI 等多种协议。每个 QUP 实例可独立配置为不同的协议模式。

### SE3 UART

SE3（Serial Engine 3）是 QUPv3 的第三个串行引擎实例。在 QCM6490 上，默认的 UART 实例用于调试串口（通常为 SE0 或 SE1），SE3 用于扩展的外部 UART。通过配置 SE3 为 2-wire UART 模式（TX/RX）或 4-wire UART 模式（TX/RX/CTS/RTS），可以连接外部串口设备。

### TrustZone QUP 访问控制

Qualcomm 平台的 TrustZone（TZ）对 QUP 等外设实施访问控制。QUPAC（QUP Access Control）机制在 TZ 侧配置每个 QUP 通道的安全属性：
- **非安全世界（Non-Secure, NS）**：Linux/Android 运行环境，可访问 UART
- **安全世界（Secure）**：TZ/TEE 运行环境，通常用于安全外设

如果 TZ 侧的 QUPAC 配置未将 SE3 通道标记为 NS 可达，Linux 内核在访问该 QUP 时会被 TZ 拦截，导致 UART 无法工作。

## 代码改动分析

### 方案 1：DTS 配置（#195829）

**4 个文件修改，共 +93 行：**

- **yupik-pinctrl.dtsi**（+42）：定义 SE3 2-wire UART 的引脚复用组（tx_active、rx_active、sleep），配置引脚功能、驱动强度和偏置。
- **yupik-qupv3.dtsi**（+36）：在 QUPv3 控制器节点下添加 SE3 UART 子节点，配置时钟、中断、DMA 等资源。
- **yupik-qrd.dtsi**（+5）：在板级 DTS 中引用 SE3 UART 节点并设置 `status = "okay"`。
- **yupik.dtsi**（+10）：在 SoC 级 DTS 中注册 UART 总线别名（`aliases`），为 SE3 串口分配固定的设备名（如 `serial3`）。

### 方案 2：产品 overlay 使能外部 UART（#195831）

**5 个产品 overlay，每个修改 1-2 行：**

将外部 UART 节点从 `status = "disabled"` 改为 `"okay"`。这些 overlay 对应不同的产品变体（lisbonmc9392、stdmc936、stdmc937、stdmc937mipi、stdmc938），每个变体都有独立的 DTS 配置。

### 方案 3：TZ QUP 访问控制（#195834）

**1 个文件修改，+10/-6：**

在 `QUPAC_Access_[项目代号].c` 中增加 SE3 UART 对应的 QUP 通道配置条目，将通道权限设置为 NS（Non-Secure）可读写。原配置中可能缺少该通道或配置为 Secure 模式。

## 配置分析

### DTS 节点变更

SE3 UART 的 DTS 配置遵循 QCM6490 平台的 QUPv3 设备树绑定规范。关键配置包括：
- 引脚复用：选择正确的 GPIO 引脚和 QUP 功能序号
- 时钟源：配置 QUP 时钟源（常为 `gcc_qupv3_wrap0_s3_clk`）
- 中断：绑定 QUP 中断号
- 功耗管理：配置 pinctrl sleep 状态

### TZ 配置变更

QUPAC 配置是 Qualcomm 平台特有的安全机制，在 TZ 侧编译。`QUPAC_Access_[项目代号].c` 中定义了每个 QUP 通道的访问权限数组，修改后需要重新编译 TZ 镜像并刷入。

## 潜在风险

- DTS 引脚配置与硬件原理图不一致会导致 UART 无法通信
- TZ QUPAC 配置错误会导致系统 panic 或安全异常
- 多个产品 overlay 的配置需要同步验证，避免遗漏某个变体
- 外部 UART 默认使能会增加功耗（~mW 级别）

## 回归测试建议

- 验证所有 5 个产品变体的外部 UART 功能
- 测试不同波特率下的 UART 通信稳定性（115200、9600、921600 等）
- 验证 TZ 安全启动后 UART 是否正常工作
- 测试 UART 中断在高负载下的响应

## 与现有驱动架构的关系

QCM6490 的 UART 驱动使用 `msm_geni_serial` 驱动（Generic Interface (GENI) Serial Engine），该驱动位于 `drivers/soc/qcom/qcom-geni-se.c` 和 `drivers/tty/serial/qcom_geni_serial.c`。SE3 UART 的配置不涉及驱动代码修改，完全通过 DTS 和 TZ 配置完成。

_Author: 艾达_