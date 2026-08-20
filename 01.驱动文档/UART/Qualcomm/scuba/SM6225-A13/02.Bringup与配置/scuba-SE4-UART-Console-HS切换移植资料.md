# Qualcomm scuba (SM6225) SE4 UART Console/HS 切换移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：Qualcomm scuba（SM6225）
- **平台**：SM6225-A13（LA.VENDOR.13.2.1）
- **功能**：SE4 UART Console/HS 模式切换

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| SE4 UART | 通用异步收发器，支持 Console/HS 双模式 |
| alias | scuba.dtsi 中 ttyHS0（SE4 UART） |

## 关键配置

### 1. Bootloader 命令

```bash
# 启用 UART 调试控制台模式（Console）
fastboot oem uartdebug console
# 启用 UART 高速模式（HS）
fastboot oem uartdebug hs
```

### 2. cmdline 传递

ABL 构造 cmdline 参数 `uart_console`，geni serial 驱动通过解析该参数决定工作模式：

- **Console 模式**：UART 端口作为内核调试控制台输出
- **HS 模式**：UART 端口切换到高速数据传输模式

### 3. DTS aliases（scuba.dtsi）

```dts
aliases {
    serial7 = &qupv3_se4_2uart;   // SE4 UART 定义（示例）
    ...
};
```

## 编译与验证

```bash
# 编译 bootloader（ABL）与 kernel
# 进入 fastboot 执行命令切换模式

# 确认内核日志输出到 UART 控制台（Console 模式）
# 确认 UART 工作在高速模式（HS 模式，收发测试）
```

## 移植注意事项

- 需要同时更新 ABL 命令、geni serial 驱动解析、DTS aliases 三侧
- 若 SE4 在目标平台已有别名占用（如 BT UART），需确认 alias 冲突

## 引用文件索引

- [[01.驱动文档/UART/Qualcomm/scuba/SM6225-A13/03.需求实现/SCUBA-UART-SE4-Console-HS-Switch.md|SCUBA-UART-SE4-Console-HS-Switch]]（补丁内容）
- `scuba.dtsi`（远程）

---

_Author: wangguanran_
