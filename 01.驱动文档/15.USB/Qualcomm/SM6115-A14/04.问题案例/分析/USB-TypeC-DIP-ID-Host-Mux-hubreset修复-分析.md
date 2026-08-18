# 分析：USB Type-C DIP ID host mux hubreset 修复

**版本号：v1.0**
**对应文档：** USB-TypeC-DIP-ID-Host-Mux-hubreset修复.md

## 技术背景

MT5205 (SM6115/scuba) 硬件通过 DIP 开关在两条 USB 通路间选择：
- **D1 (ID low)**：USB Host 模式，外接 USB HUB（HUB 的 5V 由 GPIO19/103 控制，复位由 GPIO108 控制）；
- **D2 (ID high)**：Type-C 模式，交给标准 Type-C/eud 链路。

DIP 开关输出接到 PMIC GPIO3 作为 USB_ID。原始代码用 `pm2250_charger` 的 `usb_id_irq`（TLMM 89）检测 ID，与硬件不符，且 host VBUS/HUB 复位无联动，导致两种模式切换后 USB 工作异常。

## 代码改动分析

### 1. extcon-usb-gpio.c（+27/-3）

结构体 `usb_extcon_info` 新增两个成员：

```c
bool id_high_is_none;          /* ID high 语义 = none（非 gadget） */
struct gpio_desc *hub_reset_gpiod;  /* HUB 复位脚，跟随 ID */
```

`usb_extcon_detect_cable()` 的 VBUS 判定逻辑重构：

```c
if (info->vbus_gpiod)
	vbus = gpiod_get_value_cansleep(info->vbus_gpiod);
else if (info->id_high_is_none)
	vbus = 0;                 /* ID-only + id-high-is-none：VBUS 恒 0 */
else
	vbus = id;                /* 原逻辑：ID-only 时 VBUS=ID */
```

效果：`id-high-is-none` 场景下，ID high 时 `id=1, vbus=0` → 既不上报 `EXTCON_USB`（gadget）也不上报 `EXTCON_USB_HOST`，即上报 **none**；ID low 时 `id=0` → 上报 `EXTCON_USB_HOST`（host）。

检测末尾新增 HUB 复位联动：

```c
if (info->hub_reset_gpiod)
	gpiod_set_value_cansleep(info->hub_reset_gpiod, !!id);
```

即 ID low（host）→ hub-reset 拉低（复位 HUB），ID high（type-c）→ 拉高。

probe 中新增：

```c
info->hub_reset_gpiod = devm_gpiod_get_optional(&pdev->dev, "hub-reset", GPIOD_OUT_HIGH);
info->id_high_is_none = of_property_read_bool(np, "id-high-is-none");
```

### 2. dwc3-msm-core.c（+19/-0）

- 新增 include `<linux/gpio/consumer.h>`；
- 结构体 `dwc3_msm` 新增 `struct gpio_descs *vbus_en_gpiods`；
- probe 中 `devm_gpiod_get_array_optional(dev, "vbus-en", GPIOD_OUT_LOW)` 获取 vbus-en GPIO 数组（GPIO19 + GPIO103）；
- `vbus_regulator_toggle()` 开头遍历数组逐个 `gpiod_set_value_cansleep(desc, on)`：host 模式置高输出 5V，退出 host 置低。

`vbus_regulator_toggle` 被 host/device 切换路径调用，因此 VBUS 使能随 DWC3 角色切换自动联动。

### 3. extcon-usb-gpio.txt（+5/-0）

补充两个可选属性文档：`id-high-is-none`、`hub-reset-gpios`。

### 4. scuba-iot-idp.dtsi（+77/-1）

- `&pm2250_gpios`：`usb_id_default`，GPIO3 `power-source = <1>`（1.8V VIN，保证 1.7V ID 判高）；
- `&soc`：`extcon_usb_id` 节点（`linux,extcon-usb-gpio` + `id-high-is-none` + `hub-reset-gpios=<&tlmm 108>`）；
- `&tlmm`：`usba_pwr_en_default`(GPIO19)、`otg_5v_en_default`(GPIO103)、`hub_reset_default`(GPIO108, output-high)；
- `&usb0`：extcon 追加 `&extcon_usb_id`，新增 `vbus-en-gpios`（GPIO19/103）与 pinctrl。

### 5. scuba.dtsi / scuba_auto-pmic.dtsi / scuba-thermal.dtsi

删除 `pm2250_vadc` 中 `ADC5_GPIO3_100K_PU` 的 skin_therm 通道（两处：vadc 子节点 + adc_tm 子节点 reg 4），thermal 中 `chg-skin-therm` 置 `status="disabled"`，释放 GPIO3 给 USB_ID 使用。

### 6. bengal_GKI.config（+1/-0）

`CONFIG_EXTCON_USB_GPIO=m` 使能 extcon-usb-gpio 模块。

## 潜在风险

1. **VBUS 判定语义变化**：`id-high-is-none` 仅在本节点（scuba-iot-idp）开启，其它平台不受影响；但若其它 dtsi 复用了 extcon-usb-gpio 且未设该属性，行为与原来一致。
2. **HUB 复位时序**：ID low 时 hub-reset 立即拉低，若 HUB 需要上电稳定后再复位，可能存在时序问题（当前实现未做延时）。
3. **GPIO3 复用**：删除 skin_therm 后，`scuba.dtsi` 中 `pinctrl-0 = <&conn_therm_default &skin_therm_default>` 仍引用 `skin_therm_default`，若该 pinctrl 状态仍配置 GPIO3 的 ADC 功能，需确认无残留冲突（补丁注释说明 GPIO5 Hi-Z、GPIO3 为 USB_ID）。
4. **vbus-en-gpios 与 charger 的 OTG 控制并存**：host 模式 VBUS 由 dwc3 直接驱动 GPIO，需确认与 pm2250_charger OTG 路径不打架。

## 回归测试建议

- DIP 拨到 D1（ID low）：确认 USB host 枚举 HUB 及下游设备正常，`dmesg` 有 `EXTCON_USB_HOST` 上报；
- DIP 拨到 D2（ID high）：确认 Type-C 模式正常（U 盘/充电），HUB 不复位；
- 反复拨动 DIP 10+ 次，确认模式切换无残留状态；
- 充电功能回归：Type-C 充电不受影响；
- 温度采集回归：确认无 skin_therm 相关报错（该通道已被禁用）。

## 与现有驱动架构的关系

- `extcon-usb-gpio` 是内核主线通用驱动，本次为 MT5205 场景做了向后兼容的小扩展（可选属性），不影响主线行为；
- `dwc3-msm-core` 是 Qualcomm 平台封装，`vbus-en-gpios` 与现有 `vbus_regulator` 逻辑并存（GPIO 优先，regulator 仍按原逻辑）；
- 与 `pm2250_charger` 的 `usb_id_irq` 路径并行存在：ID 检测已改由 extcon 负责，charger 的 `usb_id_irq`（TLMM 89）保留但不再承担模式判定。

---

_Author: wangguanran_