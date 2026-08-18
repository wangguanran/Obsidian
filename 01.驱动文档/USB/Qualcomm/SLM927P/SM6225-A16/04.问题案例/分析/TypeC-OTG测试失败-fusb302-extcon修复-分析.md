# 分析：TypeC OTG 测试失败 - fusb302 extcon 修复

**版本号：v1.0**
**对应文档：** TypeC-OTG测试失败-fusb302-extcon修复.md

## 技术背景

SLM927P_A16 (SM6225/khaje) 的 Type-C 口硬件由 FUSB302 做 CC 检测，DWC3 做 USB 控制器。TypeC OTG 要工作必须满足：

1. FUSB302 节点存在且可 probe（I2C + INT + 供电）；
2. usb0/dwc3 的 extcon 指向 fusb302（线缆/role 检测来源）；
3. dwc3 使能 usb-role-switch 且 dr_mode=otg（动态切换 host/device）；
4. OTG VBUS 有供电来源（nopmi 板：GPIO108 boost）。

原 DT 中 1~4 均缺失（nopmi 板甚至删掉 extcon 并锁死 peripheral），因此 OTG 测试失败。

## 代码改动分析（逐文件）

### 1. khaje-idp.dtsi（+27/-0）

**新增 `&qupv3_se1_i2c` 下 fusb302 节点：**

- `compatible = "fairchild,fusb302"` → 匹配内核 `drivers/usb/typec/fusb302/fusb302.c`；
- `reg = <0x22>` → I2C 地址 0x22；
- `vdda18-supply = <&pm6125_l9>`、`vdda33-supply = <&pm6125_l15>` → 两路供电；
- `int-n-gpios = <&tlmm 36 0x00>` → 中断脚 GPIO36（低有效）；
- `pinctrl-0 = <&fusb302_default>` → INT 引脚默认配置；
- 注释掉的 `aux-switch/aux-en/aux-sel/vbus-5v-gpios` 表明该板未使用 DisplayPort AUX 与 GPIO VBUS（VBUS 走 GPIO108 boost）。

**新增 `&usb0` 覆盖块（文件末尾）：**

- `usb-role-switch`：usb0 与 dwc3 均使能 role switch；
- `extcon = <&fusb302>`：usb0 的 extcon 指向 fusb302；
- `qcom,otg-vbus-gpio = <&tlmm 108 0>`：OTG 时 GPIO boost 供 5V；
- `dwc3@4e00000 { usb-role-switch; dr_mode = "otg"; }`：DWC3 切 OTG 模式。

### 2. khaje-idp-nopmi.dtsi（+7/-2）

原 nopmi 板 `&usb0` 为：

```dts
/delete-property/ extcon;        /* 删除 extcon（无线缆检测） */
dwc3@4e00000 { dr_mode = "peripheral"; };  /* 锁死 device */
```

改为与 khaje-idp.dtsi 相同的 usb-role-switch + otg-vbus-gpio + extcon=fusb302 + dr_mode="otg" 配置（`///delete-property/` 注释掉删除逻辑）。

### 3. khaje-usb.dtsi（+2/-0）

`&usb0` 中补 `extcon = <&eud>`：保留 eud（Embedded USB Debugger）作为第二 extcon 来源，eud 与 fusb302 可叠加通知。

## 潜在风险

1. **`fusb302_default` pinctrl 未在本补丁定义**：若 khaje-pinctrl 系列 dtsi 中不存在该节点，probe 时 pinctrl 解析失败会导致 FUSB302 不工作 —— 需确认平台已有定义（引用自既有文件，未在 patch 中新增）；
2. **usb0 extcon 多来源叠加**：khaje-usb.dtsi 的 `extcon = <&eud>` 与 khaje-idp 的 `extcon = <&fusb302>` 在 overlay 合并时，以最后覆盖者为准（idp 覆盖为 fusb302）；若某板同时 include 两文件且顺序不定，行为可能不一致；
3. **GPIO108 复用冲突**：nopmi 板 GPIO108 作为 OTG VBUS boost，需确认与其它功能（如 FUSB302 的 vbus-5v 注释掉的 GPIO118、或平台其它使用）无冲突；
4. **OTG 电流能力**：GPIO boost 方案电流有限，大电流 OTG 设备可能供电不足（对比 pm7250b 板载升压方案）。

## 回归测试建议

- 编译 dtbo（`make dtboimage`）确认 DTS 语法与节点合并正确；
- OTG：插入 U 盘 → `lsusb` 枚举、读写文件正常；
- device：插 PC → gadget 枚举（ADB）正常；
- 反复插拔 10+ 次确认 role 切换稳定；
- 充电回归：Type-C 充电（D+ 连接）不受影响；
- nopmi 与带 PMIC 板两套配置分别验证。

## 与现有驱动架构的关系

- 沿用内核既有 fusb302 驱动与 extcon 框架，无内核代码改动，纯 DT 修复；
- 与 [[01.驱动文档/USB/Qualcomm/SLM927P/SM6225-A16/01.原理与架构/TypeC-OTG驱动架构分析.md|TypeC-OTG 驱动架构分析]] 描述的链路一致：FUSB302 检测 CC → extcon → dwc3 role switch；
- khaje 平台其它项目（如 GPIO/SLM927P 平台目录）共用同一套 khaje dtsi，本改动影响所有 include khaje-idp.dtsi 的板型。

---

_Author: wangguanran_