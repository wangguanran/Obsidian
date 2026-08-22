# SM6225-A14 (khaje) Charger 平台移植资料

> **版本号：v1.0**

## 平台版本

| 项目 | 内容 |
|------|------|
| SoC | Qualcomm SM6225 (khaje) |
| Android | A14 (LA.VENDOR.13.2.1 / master_IOT_High_Mid_2024.SPF.1.0_[项目代号]) |
| 内核 | msm-kernel 5.10 |
| 充电器 | SMB5 (pm6125/pm7250b) |

## 硬件接口

| 信号 | 说明 |
|:---|:---|
| DCIN | Dock/POGO 直流输入（与 USB host 可并存充电） |
| USBIN | USB 充电输入 |
| EN2 (pogo_en_typea_5v_gpio) | Type-A 口 5V 使能 |
| EN4 (pogo_en_hub_vcc_gpio) | Dock HUB VBUS 5V 供电使能 |
| pogo_sw_uart_usb_gpio | POGO UART/USB 切换 |

## 驱动索引

| 驱动/文件 | 路径（源码树内） | 说明 |
|:---|:---|:---|
| smb5-lib | `kernel_platform/msm-kernel/drivers/power/supply/qcom/smb5-lib.c` | SMB5 充电核心库：输入检测、OTG、POGO/Dock 逻辑 |
| smb5 | `kernel_platform/msm-kernel/drivers/power/supply/qcom/smb5.c` | SMB5 平台驱动（probe、irq） |

## DTS 配置要点

POGO 相关 GPIO 与 charger 节点由既有 [项目代号] DT 配置，本补丁（#195273）不涉及 DTS 改动，仅修改 smb5-lib.c。

充电器节点 DT 结构（参考）：

```dts
&pm6125_charger {
    /* 既有配置：输入源、POGO GPIO 等 */
};
```

## 内核配置

SMB5 相关配置（既有，未在本补丁中改动）：

```
CONFIG_QPNP_SMB5=y
CONFIG_PMIC5_SMB5=y
```

## 编译命令

```bash
# 134 源码树 [项目代号] (LA.VENDOR.13.2.1)
cd <tree>/kernel_platform
# 编译内核
./build.sh -k msm-kernel    # 或平台既有编译脚本
```

## 问题案例经验

- **Dock 接 host 时充电异常**：根因是把充电器 OTG_EN 打开（占用 DCIN 输入路径），修复为仅 extcon 通知、HUB 5V 由 EN4 直接供电，详见 [[01.驱动文档/Charger/Qualcomm/SM6225-A14/04.问题案例/Dock连接影响充电功能修复.md|Dock连接影响充电功能修复]]。

## 相关文档

- [[01.驱动文档/Charger/Qualcomm/SM6225-A14/01.原理与架构/SMB5充电驱动架构分析.md|SMB5 充电驱动架构分析]]

---

_Author: wangguanran_