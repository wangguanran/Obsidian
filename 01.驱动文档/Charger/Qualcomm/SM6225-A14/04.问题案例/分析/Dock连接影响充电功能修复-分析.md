# 分析：Dock 连接影响充电功能修复

**版本号：v1.0**
**对应文档：** Dock连接影响充电功能修复.md

## 技术背景

MT5825 (SM6225/khaje) 设备支持 POGO 接口连接 Dock。Dock 提供 USB host 功能（通过板载 HUB），同时设备需要继续通过 DCIN 充电。

SMB5 充电器有两种 VBUS 输出路径：
- **OTG_EN 路径**：充电器自身 boost 输出 5V（`smblib_notify_usb_host()` 内部会打开 OTG 并通知 USB 控制器）；
- **外部使能路径**：HUB 5V 由 `pogo_en_hub_vcc_gpio`（EN4）直接控制，不经充电器 boost。

充电器 OTG 与 DCIN 输入互斥：打开 OTG 后输入路径被占用，DCIN 无法同时充电。

## 代码改动分析（smb5-lib.c，+26/-4）

### 新增函数 `smblib_pogo_notify_usb_host_role()`

```c
static void smblib_pogo_notify_usb_host_role(struct smb_charger *chg, bool enable)
{
	if (enable)
		smblib_notify_extcon_props(chg, EXTCON_USB_HOST);

	pr_info("[POGO_USB]%s: EXTCON_USB_HOST=%d (no OTG_EN, hub via EN4)\n", ...);

	extcon_set_state_sync(chg->extcon, EXTCON_USB_HOST, enable);
}
```

- `enable=true` 时先 `smblib_notify_extcon_props()`（下发 role/driver props，USB 控制器据此切换），再 `extcon_set_state_sync()` 设 HOST 状态；
- **刻意不调用 `smblib_notify_usb_host()`**，避免打开充电器 OTG_EN。

### `smblib_pogo_irq_back_det_delayed_work` 修改

Dock 插入分支（原 `smblib_notify_usb_host(chg, true)` → 新 `smblib_pogo_notify_usb_host_role(chg, true)`），并新增 EN2 使能：

```c
/* Enable EN2 (pogo_en_typea_5v_gpio) */
if (gpio_is_valid(chg->pogo_en_typea_5v_gpio)) {
	gpio_direction_output(chg->pogo_en_typea_5v_gpio, 1);
	pr_info("[POGO_USB]... set pogo_en_typea_5v_gpio=1 (EN2 on)\n");
}
```

Dock 拔出分支（原 `smblib_notify_usb_host(chg, false)` → 新 `smblib_pogo_notify_usb_host_role(chg, false)`）。

### 关键函数调用链

```
pogo IRQ → smblib_pogo_irq_back_det_delayed_work()
  ├─ dock 插入: sw_uart=1 → EN2 on → EN4 on
  │             → smblib_pogo_notify_usb_host_role(true)
  │                 └─ smblib_notify_extcon_props(EXTCON_USB_HOST)
  │                 └─ extcon_set_state_sync(EXTCON_USB_HOST, true)
  │                 → dwc3 切 host（不开 OTG_EN，DCIN 可继续充电）
  └─ dock 拔出: EN2 off → EN4 off
                → smblib_pogo_notify_usb_host_role(false)
                    └─ extcon_set_state_sync(EXTCON_USB_HOST, false)
                    → dwc3 切回 device
```

## 潜在风险

1. **HUB 供电依赖 EN4**：若某些 Dock 硬件未接 EN4 供电或 EN4 GPIO 无效（`gpio_is_valid` 检查已覆盖），HUB 无 5V 时 USB 枚举失败 —— 需确认所有 Dock 硬件形态一致；
2. **extcon 通知与 dwc3 联动时序**：`smblib_notify_extcon_props` + `extcon_set_state_sync` 后 dwc3 立即切 host，若 HUB 复位/上电尚未完成，首插枚举可能失败（当前无延时）；
3. **不再走 charger OTG 的 ICL 管理**：OTG_EN 关闭后，USB host 电流由外部电源承担，充电器侧看不到 host 电流，输入电流限制（ICL）逻辑需依赖 DCIN 正常路径；
4. **回归面**：`smblib_notify_usb_host()` 还被其它路径使用（typec_det、otg sysfs 等，见 7652/6273/6532 行），本次只改 POGO 路径，不影响其它场景。

## 回归测试建议

- Dock 插入 → 确认 `[POGO_USB]` 日志 EN2/EN4 on、`EXTCON_USB_HOST=1 (no OTG_EN)`；
- 插入期间同时接 DCIN 适配器 → 确认充电电流 > 0（充电未被打断）；
- Dock USB host 枚举 U 盘/键鼠正常；
- Dock 拔出 → 确认 EN2/EN4 off、USB 回 device 模式；
- 充电异常回归：无 Dock 时 TypeC/USB 充电、POGO 背充（sibling 场景）正常。

## 与现有驱动架构的关系

- 本次改动仅影响 `smblib_pogo_*` 这一条定制路径，属 MT5825 平台特有逻辑（`[POGO_USB]` 日志前缀）；
- `smblib_notify_extcon_props` / `extcon_set_state_sync` 为 smb5-lib 既有机制，复用而无新增接口；
- 与 [[01.驱动文档/Charger/Qualcomm/SM6225-A14/01.原理与架构/SMB5充电驱动架构分析.md|SMB5 充电驱动架构]] 中描述的 extcon 通知链一致：充电器作为 extcon provider，dwc3 为 consumer。

---

_Author: wangguanran_