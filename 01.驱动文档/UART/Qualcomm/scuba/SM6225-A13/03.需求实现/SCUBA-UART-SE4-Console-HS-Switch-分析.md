# SCUBA UART SE4 Console/HS Switch 实现 — 分析文档

> **Change**: #196150 | **类型**: 需求 (feature) | **分析日期**: 2024

---

## 一、变更概要

### 提交信息

| 项目 | 内容 |
|------|------|
| Change | #196150 |
| 项目 | LA.VENDOR.13.2.1 |
| 分支 | [项目代号] |
| 作者 | [同事] |
| 状态 | MERGED |
| 类型 | 需求 (feature) |
| 标题 | [[项目代号]][120574][uart] fastboot oem uartdebug SE4 console/HS switch |

### 摘要

在 ABL 中实现 `fastboot oem uartdebug` 命令，支持 SE4 UART 的 Console/HS switch 功能。通过 cmdline 传递 `uart_console` 信息，geni serial 驱动添加 HS fallback 支持。

---

## 二、架构分析

### 2.1 整体架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        Fastboot 用户                             │
│              fastboot oem uartdebug console/hs                   │
└─────────────────────────┬───────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                    ABL (Android Bootloader)                      │
│  ┌──────────────────────┐  ┌────────────────────────────────┐   │
│  │   FastbootCmds.c     │  │       DeviceInfo.c             │   │
│  │  oem uartdebug 命令  │──▶│   uart_console 变量存取       │   │
│  └──────────────────────┘  └───────────────┬────────────────┘   │
│                                            │                    │
│                                            ▼                    │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │                 UpdateCmdLine.c                          │   │
│  │          uart_console=console/hs  → cmdline              │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────┬───────────────────────────────────────┘
                          │  cmdline 传递
                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Kernel (geni serial 驱动)                     │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │              qcom_geni_serial.c                          │   │
│  │         解析 cmdline → 选择 Console/HS 模式              │   │
│  │         HS fallback 支持                                 │   │
│  └──────────────────────────────────────────────────────────┘   │
│                            ▲                                     │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │              scuba.dtsi (Device Tree)                     │   │
│  │          aliases 节点定义 SE4 UART                        │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

### 2.2 数据流

```
Fastboot 命令
    │
    ▼
FastbootCmds.oem_uartdebug()     ← 解析参数 console/hs
    │
    ▼
DeviceInfo.SetUartConsole()      ← 写入 uart_console 变量
    │
    ▼
UpdateCmdLine.UpdateCmdLine()    ← 读取变量，构造 cmdline
    │
    ▼
Kernel cmdline: uart_console=xxx
    │
    ▼
qcom_geni_serial.c: probe()      ← 解析 cmdline，配置模式
    │
    ├── Console 模式 → 注册为控制台
    └── HS 模式     → 启用高速传输
```

---

## 三、文件修改详解

### 3.1 DeviceInfo.h（+15 行）

**作用**：定义 `uart_console` 变量相关的宏和接口声明。

**关键点**：
- 新增 `UART_CONSOLE_VAR` 宏定义，用于标识变量名称
- 声明 `uart_console` 变量的读写接口
- 该变量将持久化存储，重启后仍可读取

### 3.2 DeviceInfo.c（+42 行）

**作用**：实现 `uart_console` 变量的存取逻辑。

**关键点**：
- 实现变量存储（通常使用 ABL 的 NVRAM 或环境变量机制）
- 提供读取接口供 UpdateCmdLine 使用
- 提供写入接口供 FastbootCmd 使用

### 3.3 UpdateCmdLine.c（+30 行）

**作用**：在构造 kernel cmdline 时插入 `uart_console` 参数。

**关键点**：
- 在 cmdline 构造流程中插入 `uart_console` 参数读取
- 根据 DeviceInfo 中存储的值拼接 `uart_console=console` 或 `uart_console=hs`
- 确保 cmdline 参数正确传递给 kernel

### 3.4 FastbootCmds.c（+39 行）

**作用**：实现 `fastboot oem uartdebug` 命令的解析和执行。

**关键点**：
- 注册 `oem uartdebug` 子命令
- 支持 `console` 和 `hs` 两个参数
- 调用 DeviceInfo 接口写入用户选择的模式
- 返回执行结果给 fastboot 客户端

### 3.5 qcom_geni_serial.c（common + msm-kernel，各 +16/-15）

**作用**：kernel 侧 geni serial 驱动添加 HS fallback 支持。

**关键点**：
- 解析 cmdline 中 `uart_console` 参数
- 根据参数值选择 Console 或 HS 模式初始化
- HS fallback 机制：在 HS 模式下遇到错误时降级到 Console 模式
- common 和 msm-kernel 两处同步修改（共 16 行新增，15 行删除/修改）

### 3.6 scuba.dtsi（+12/-7）

**作用**：Device Tree 中定义 SE4 UART 的 aliases。

**关键点**：
- 在 aliases 节点中添加 SE4 UART 的引用
- 使系统能够通过别名正确识别 SE4 UART 端口
- 修改 12 行 / 删除 7 行，包含对现有配置的调整

---

## 四、技术要点

### 4.1 Console vs HS 模式

| 模式 | 用途 | 特点 |
|------|------|------|
| Console | 内核调试控制台 | 输出内核日志，交互式调试，速度较慢 |
| HS (High-Speed) | 高速数据传输 | 高速率通信，适用于数据传输场景 |

### 4.2 HS Fallback 机制

在 geni serial 驱动中实现了 HS fallback 机制：
- 当 UART 配置为 HS 模式但硬件初始化失败时
- 自动降级到 Console 模式，保证基本通信功能可用
- 提高系统的健壮性和容错能力

### 4.3 双树同步修改

`qcom_geni_serial.c` 在 `common/` 和 `msm-kernel/` 两处同步修改：
- `common/` 路径：通用的 kernel 公共代码
- `msm-kernel/` 路径：MSM 专用 kernel 代码
- 两处修改保持一致，确保无论使用哪个 kernel 树都能获得 HS fallback 支持

---

## 五、影响范围

### 5.1 涉及模块

| 模块 | 影响 |
|------|------|
| Fastboot 协议 | 新增 `oem uartdebug` 命令 |
| ABL 启动流程 | cmdline 增加 `uart_console` 参数 |
| geni serial 驱动 | 新增 HS fallback 逻辑 |
| Device Tree | SE4 UART aliases 调整 |

### 5.2 兼容性

- 向后兼容：未修改 `uart_console` 变量时，默认行为不变
- 新增命令不影响现有 fastboot 命令
- HS fallback 机制仅在 HS 模式失败时触发

---

## 六、注意事项

### 6.1 使用注意事项

1. **模式选择时机**：需要在 bootloader 阶段（fastboot 模式下）选择 UART 模式，重启后生效
2. **Console 模式优先**：调试阶段建议使用 Console 模式，确保内核日志可正常输出
3. **HS 模式验证**：切换 HS 模式前需确认硬件支持和连接稳定性

### 6.2 补丁验证

> ⚠️ 补丁内容请参考 Gerrit Change #196150
> ⚠️ 补丁验证：无法直接获取，需在对应硬件平台上进行实际测试

### 6.3 隐私审查

- 本分析文档不包含敏感信息
- 所有文件路径和接口描述均为公开技术信息
- 无个人身份信息或未公开的内部接口

---

## 参考文档

- [[SCUBA UART SE4 Console/HS Switch 实现]]
- [[Gerrit Change #196150]]
- [[SM6225 UART 驱动文档索引]]

---

_Author: wangguanran_