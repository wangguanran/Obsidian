# Qualcomm scuba (SM6225) SE4 UART Console/HS 模式切换架构分析

> **版本号：v1.0**

## 平台概述

scuba 平台的 **SE4 UART**（Serial Engine 4）通过 bootloader 命令在两种模式间切换：

```
fastboot oem uartdebug  (ABL 命令)
   ↓ 构造 cmdline: uart_console=<console|hs>
内核启动
   ↓ geni serial 驱动解析 uart_console
SE4 UART 工作模式
   ├─ Console 模式: 内核调试控制台输出
   └─ HS 模式:      高速数据传输（配合 SE4 硬件能力）
```

## 关键链路

| 环节 | 组件 | 作用 |
|:---|:---|:---|
| 命令入口 | ABL `fastboot oem uartdebug` | 配置 UART 模式 |
| 传递 | cmdline `uart_console` 变量 | 从 bootloader 传参给内核 |
| 解析 | geni serial 驱动 `UART_CONSOLE_VAR` 宏 | 决定 UART 端口工作模式 |
| 硬件映射 | `scuba.dtsi` aliases | SE4 UART 总线别名（ttyHS0） |

## 修改文件（摘要）

| 文件 | 变更 | 说明 |
|:---|:---|:---|
| ABL bootloader | 新增 `fastboot oem uartdebug` | 命令处理与 cmdline 构造 |
| geni serial 驱动 | `#define UART_CONSOLE_VAR "uart_console"` | 解析 cmdline 变量 |
| `scuba.dtsi` | +12/−7 | aliases 添加 SE4 UART 定义 |

## 引用文件索引

- [[01.驱动文档/UART/Qualcomm/scuba/SM6225-A13/03.需求实现/SCUBA-UART-SE4-Console-HS-Switch.md|SCUBA-UART-SE4-Console-HS-Switch]]（补丁内容）
- `scuba.dtsi`（远程）

---

_Author: wangguanran_
