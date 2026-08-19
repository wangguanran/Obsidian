# USB-QCM6490 UEFI 与内核驱动架构分析

> **芯片**: Qualcomm QCM6490 (yupik/Kodiak) | **平台**: QCM6490-A16

## UEFI USB 分层

```
UEFI 应用/Shell（TFTP、U盘、USB 键盘）
   │
   ▼
UsbBusDxe（USB 总线枚举：root hub、设备寻址）
   │
   ▼
XhciDxe（XHCI 控制器：DriverBinding → CreateUsbHc → 调度/中断）
   │
   ▼
PCI Express（Xhci 经 PCI 接口挂接）← PcieConfigLib（root port 使能）
   │
   ▼
PMIC（PmicLib：电压/协议，如 charger 显示）
```

## 驱动绑定流程（XhciDxe）

1. `XhcDriverBindingSupported`：检查 PCI IO 协议 + Class/SubClass/ProgInterface（0x0C/0x03/0x30=XHCI）；
2. `XhcDriverBindingStart`：Open PCI IO → EnableController → `XhcCreateUsbHc` → 初始化调度器/ControlPollTimer → Start HC → 安装 `USB2_HC` 协议 → Port Test 协议；
3. `UsbEnumNewDevRootHub`：root hub 枚举子设备，分配地址（Addr）并读取描述符。

## 数据通路（U 盘启动）

```
UEFI Shell/Fastboot → UsbBusDxe 枚举 → UsbMassStorageDxe → FAT/APFS 文件系统 → 启动镜像
```

## 关键函数

| 函数 | 位置 | 说明 |
|------|------|------|
| `UsbEnumNewDevRootHub` | UsbEnumer.c | root hub 枚举、设备地址分配 |
| `XhcDriverBindingSupported/Start` | Xhci.c | XHCI 绑定/启动 |
| `XhcCreateUsbHc` | Xhci.c | HC 实例创建 |
| `PcieConfigLibEnableRootPorts` | PcieConfigLib.c | PCIe root port 使能 |
| `pm_install_target_protocols` | pm_core.c | PMIC 协议安装 |

## 与内核驱动的关系

UEFI 与内核（dwc3/ehci）为两套独立 USB 栈，但 UEFI 阶段枚举结果影响引导设备顺序与固件升级流程（fastboot/刷机）。UEFI 驱动改动需回归引导 + 刷机链路。

---

_Author: wangguanran_
