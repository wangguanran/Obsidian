# 分析：UEFI 回退 pcie-usb qcom 补丁

**版本号：v1.0**
**对应文档：** [[01.驱动文档/USB/Qualcomm/yupik/QCM6490-A16/04.问题案例/UEFI回退pcie-usb补丁.md|UEFI回退pcie-usb补丁]]

## 技术背景

UEFI（BOOT.MXF.1.0.c1，EDK2 派生）中 USB 与 PCIe 子系统：

- **UsbBusDxe**：USB 总线枚举（root hub、设备地址分配）；
- **XhciDxe**：XHCI 控制器驱动（DriverBinding → Start → CreateUsbHc → 调度器/中断）；
- **PcieConfigLib**：PCIe root port 使能/配置（`PcieConfigLibEnableRootPorts`）；
- **PmicLib**：PMIC 协议栈（`pm_install_target_protocols` 安装 charger/LDO 等协议）；
- **Core.fdf / CoreAux.fdf.inc / Apriori.fdf.inc**：固件镜像驱动清单与加载顺序；
- **mg_build.py**：构建规则（`SUB_SYS_BUILD_RULES`、`-r RELEASE,DEBUG` 变体）。

## 代码改动分析（Revert 恢复的内容）

| 文件 | revert 恢复点 |
|------|--------------|
| UsbEnumer.c | 移除 `UsbEnumNewDevRootHub` 中 vid/pid/Addr 调试打印（-1 行） |
| Xhci.c | 恢复 DriverBindingSupported/Start 的原始打印与错误分支；`XhcCreateUsbHc` 的 ControlPollTimer 错误文案恢复 `[XHCI]` 前缀风格 |
| Core.fdf | 恢复驱动 INF 清单与 `NumBlocks` 等布局参数（含 AX88179UsbEthDxe 条目回归） |
| CoreAux.fdf.inc | 恢复模块列表 |
| uefiplat.cfg | 恢复平台参数 |
| pm_core.c | 恢复 `pm_install_target_protocols` 中约 140 行协议安装（原补丁大量删除） |
| PcieConfigLib.c | 恢复 `PcieConfig_isPort1Supported` 与 `PcieConfigLibEnableRootPorts` 原逻辑（约 150 行） |
| boot_uart.c | 恢复 UART 初始化条件 |
| Tftp.c / inf | 恢复 HiiPackage 初始化分支 |
| mg_build.py | `-r RELEASE,DEBUG` 恢复双变体 |

## 潜在风险

1. **功能倒退**：若原 pcie-usb 补丁曾修复过真实问题（如 PCIe 网卡枚举、USB 引导），revert 后该问题可能复现——需确认原补丁合入动机是否已被其它手段替代；
2. **上下文漂移**：revert 基于当前主干状态，若中途有其它提交改过同一区域，revert 可能有冲突或语义偏差（本提交为干净 revert，无冲突）；
3. **AX88179UsbEthDxe**：Core.fdf 中该驱动条目恢复，若产品不再使用 USB 网卡，属冗余但无害。

## 回归测试建议

- UEFI 阶段：USB 键盘/存储枚举、PCIe 外设枚举（SSD/网卡）、PMIC 充电/电压协议；
- OS 启动：整机冷启动、fastboot/adb、以太网（如走 USB 网卡）；
- 构建回归：`mg_build.py -v WP -t kodiak,QcomToolsPkg -v LAA -r RELEASE,DEBUG` 双变体产物齐全。

## 与现有驱动架构的关系

UEFI 驱动与 Linux 内核驱动是两套独立实现（EDK2 vs kernel/drivers）。该 revert 只影响 UEFI 侧 USB/PCIe/PMIC 行为；与内核侧 USB 驱动（如 dwc3）无直接耦合，但 UEFI 枚举结果会影响 OS 引导设备顺序。[项目代号] Linux（Ubuntu）平台后续如需 pcie-usb 能力，建议拆分为小粒度补丁重新评审。

_Author: wangguanran_
