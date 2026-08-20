# Qualcomm QCM6490 (yupik) SE3 UART 移植架构分析

> **版本号：v1.0**

## 平台概述

QCM6490（yupikp）SE3 UART 移植涉及三层：引脚复用（pinctrl）、QUPv3 控制器节点、TrustZone 访问控制：

```
Linux/Android（非安全世界）
   ↓ geni serial 驱动
QUPv3 SE3 通道 (qupv3_se3_2uart)
   ├─ pinctrl: qupv3_se3_2uart_pins (TX/RX active/sleep)
   ├─ DTS 节点: status="okay"
   └─ alias: ttyMSM 总线注册
        ↑ TZ 侧必须放行（QUPAC_Access_MT912.c）
TrustZone (QCOM 安全世界)
```

## 三层修改

| 层次 | 文件 | 内容 |
|:---|:---|:---|
| 引脚复用 | `yupik-pinctrl.dtsi` | SE3 UART TX/RX 引脚 mux/active/sleep 配置（+42） |
| 控制器节点 | `yupik-qupv3.dtsi` | `qupv3_se3_2uart` 节点 + pinctrl 引用（+36） |
| 板级引用 | `yupik-qrd.dtsi` | 引用 SE3 UART 节点（+5） |
| 总线别名 | `yupik.dtsi` | 注册 SE3 串口 alias（+10） |
| 产品使能 | 5 个 overlay dtsi | 外部 UART status → okay |
| TZ 放行 | `QUPAC_Access_MT912.c` | SE3 QUP 通道非安全世界访问权限（+10/−6） |

## 关键要点

- **TZ 是硬门槛**：QCM6490 的 QUP 通道默认由 TrustZone 管控，Linux 侧只配 DTS 不够，必须在 `QUPAC_Access_MT912.c` 放行对应通道，否则设备节点注册但访问被拒
- **alias 注册**：`yupik.dtsi` 中注册总线别名后，`/dev/ttyMSM*` 才会出现对应串口

## 引用文件索引

- [[01.驱动文档/UART/Qualcomm/yupik/QCM6490-A16/03.需求实现/SE3-UART移植.md|SE3-UART移植]]（补丁内容）
- `qcom/yupik-pinctrl.dtsi`、`yupik-qupv3.dtsi`、`yupik-qrd.dtsi`、`yupik.dtsi`（远程）
- `TZ.XF.5.35/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/kodiak/QUPAC_Access_MT912.c`（远程）

---

_Author: wangguanran_
