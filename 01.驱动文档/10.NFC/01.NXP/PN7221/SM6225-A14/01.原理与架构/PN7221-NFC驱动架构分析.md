# NXP PN7221 NFC 驱动架构分析（SM6225 平台）

> **版本号：v1.0**

## 芯片信息

- **型号**：NXP PN7221 (PN7220 系列，驱动共用 `pn7220/` 目录)
- **厂商**：NXP Semiconductors
- **功能**：NFC 控制器，支持 Reader/Writer、Card Emulation (CE)、P2P
- **通信接口**：I2C
- **固件**：NQ NFC Firmware (NQNFC)

## 平台信息

- **芯片平台**：Qualcomm SM6225 (khaje)
- **Android 版本**：Android 14 (API 34)
- **内核版本**：5.10
- **架构**：arm64
- **VENDOR 树**：`LA.VENDOR.13.2.1`
- **QSSI 树**：`LA.QSSI.14.0`

## 硬件接口

| 信号 | GPIO/接口 | 说明 |
|:---|:---|:---|
| I2C SDA | — | I2C 数据线 |
| I2C SCL | — | I2C 时钟线 |
| IRQ | GPIO_70 | NFC 中断（下降沿触发） |
| VEN (legacy) | 扩展 IO (et6416_21.3) | 旧 BOM 使能脚，依赖 I2C 扩展芯片 |
| VEN2 (primary) | GPIO_51 (AL5/CTLS_nPWDN) | 新 BOM 平台直驱使能脚，不依赖 I2C |
| MODE | 扩展 IO (et6416_21.2) | Forum/EMVCo 模式切换 |
| WAKEUP | GPIO_106 | NFC 唤醒 |
| 电源 | VDD_3V3 | 3.3V 供电 |

## 驱动架构

### 分层结构

NXP PN7221 驱动在 Android 平台由三层组成：

```
Android Framework (NfcService.java)
        │
        ▼
NFC HAL (vendor/nxp/nfc/)           ← 用户态
        │
        ▼
NFC 内核驱动 (drivers/nfc/pn7220/)  ← 内核态
        │
        ▼
硬件 (PN7221 I2C)
```

### 内核驱动层 (`pn7220/`)

驱动代码位于 `kernel_platform/msm-kernel/drivers/nfc/pn7220/`，包含以下关键文件：

| 文件 | 职责 |
|:---|:---|
| `i2c_drv.c` | I2C 总线驱动：probe、remove、IRQ handler、电源管理、GPIO 配置 |
| `i2c_drv.h` | I2C 驱动头文件 |
| `common.c` | 公共逻辑：NCI 协议处理、数据收发、GPIO 控制 |
| `common.h` | 公共头文件：GPIO 结构体、DTS 解析宏、NCI 状态 |
| `Kconfig` | 内核配置选项 |
| `Makefile` | 编译规则 |

SM6225 平台使用 **pn7220_i2cms** 变体（I2C Master-Slave 模式），驱动路径为 `kernel_platform/msm-kernel/drivers/nfc/pn7220_i2cms/`。

### 关键初始化流程

```
1. i2c_drv.c: nfc_i2c_dev_probe()
   ├─ 解析 DTS (nfc_parse_dt)
   │   ├─ IRQ (GPIO_70)
   │   ├─ VEN (扩展 IO)
   │   ├─ VEN2 (GPIO_51，可选)
   │   ├─ MODE (扩展 IO)
   │   └─ WAKEUP (GPIO_106)
   ├─ 配置 GPIO
   │   ├─ configure_gpio(irq, GPIO_IRQ)
   │   ├─ configure_gpio(ven, GPIO_OUTPUT)
   │   ├─ configure_gpio(ven2, GPIO_OUTPUT)  ← 可选，SM6225 特有
   │   └─ configure_gpio(wakeup, GPIO_OUTPUT)
   └─ NCI 初始化 (通过 IRQ handler 触发)
       ├─ CORE_RESET_CMD → CORE_RESET_NTF
       └─ CORE_INIT_CMD → CORE_INIT_NTF
```

### VEN 双路径控制（SM6225 平台特有）

SM6225 平台引入 VEN 双路径控制，解决扩展 IO 依赖 I2C 导致的自锁问题：

```
gpio_set_ven(nfc_dev, value)
  ├─ gpio_set_value(ven2, value)     ← 平台 GPIO51 直驱，不依赖 I2C
  └─ gpio_set_value(ven, value)      ← 扩展 IO 兼容旧 BOM
```

该设计确保 NFC 复位/使能操作不依赖共享 I2C 总线，避免总线挂死时无法复位 NFC。

### Vendor HAL 层

HAL 代码位于 `vendor/nxp/nfc/`，SM6225 平台使用 I2C MS 模式配置：

| 配置文件 | 用途 |
|:---|:---|
| `hw/pn7220_i2cms/libnfc-nci.conf` | NCI 协议参数（I2C MS 模式） |
| `hw/pn7220_i2cms/libnfc-nxp.conf` | NXP 扩展配置 |
| `hw/pn7220_i2cms/libnfc-nxp-rfExt.conf` | RF 扩展配置 |
| `hw/pn7220_i2cms/libnfc-nxp-eeprom.conf` | EEPROM 配置 |

### DTS 配置

SM6225 (khaje) 平台 NFC DTS 节点位于 `kernel_platform/qcom/proprietary/devicetree/qcom/`：

```
khaje-idp.dtsi:
  nfc: nxpnfc@38 {
      compatible = "nxp,nxpnfc_m";
      reg = <0x38>;
      pinctrl-names = "default";
      pinctrl-0 = <&nfc_ven_gpio51_active>;

      nxp,nxpnfc-irq = <&tlmm 70 0>;
      nxp,nxpnfc-ven = <&et6416_21 3 0>;     /* 扩展 IO (legacy BOM) */
      nxp,nxpnfc-ven2 = <&tlmm 51 0>;         /* 平台 GPIO51 (new BOM) */
      nxp,nxpnfc-mode_sw = <&et6416_21 2 0>;
      nxp,nxpnfc-wakeup = <&tlmm 106 0>;
  };
```

引脚复用配置：

```
khaje-pinctrl.dtsi:
  nfc_ven_gpio51_active:  GPIO51 output-high, bias-disable
  nfc_ven_gpio51_suspend: GPIO51 output-low, bias-pull-down
```

## 与 SM6115 平台的差异

| 项目 | SM6115 (bengal) | SM6225 (khaje) |
|:---|:---|:---|
| SoC | SM6115 | SM6225 |
| VEN 控制 | 单路（扩展 IO） | 双路（扩展 IO + 平台 GPIO51） |
| MODE 引脚 | GPIO_31 | 扩展 IO |
| 驱动路径 | `pn7220/` | `pn7220_i2cms/` |
| 固件 | PN557 | PN557 |

## 常见问题参考

- **VEN 双路径控制**：见 `04.问题案例/2026-08-14-NFC-VEN双路径GPIO51控制.md`
- **HAL 主动崩溃**：参考 SM6115 平台 `01.原理与架构/HAL主动崩溃触发逻辑说明.md`（NFC HAL 通用逻辑）

---

_Author: 艾达_