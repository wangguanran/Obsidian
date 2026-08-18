# SM6115-A14 GPIO Bringup 与配置

> **模块**: GPIO | **芯片**: SM6115 (scuba) | **平台**: SM6115-A14（LA.VENDOR.13.2.1 / MT5205）

## 1. 硬件接口 GPIO 对照表（MT5205）

| GPIO | 功能 | 方向 | 电平/极性 | 驱动/节点 | 说明 |
|------|------|------|-----------|-----------|------|
| 14 | MDB-DB detect | IN | 低有效（GPIO_ACTIVE_LOW），pull-up，wakeup | gpio-keys `mdb_db_keys` → KEY_F1 | #195886 解除 reserved |
| 15 | — | — | reserved | pinctrl-scuba reserved 列表 | 保留 |
| 16/17 | MDB SE5 UART | — | — | UART（更早变更释放） | |
| 32 | Pulse 接口-1 | IN | ACTIVE_HIGH（idle 低、检高），pull-up | meig_gpio_pulse in-gpios | IRQ + hrtimer 检测 |
| 33 | Pulse 接口-2 | OUT | ACTIVE_HIGH（idle 低、发高），8mA | meig_gpio_pulse out-gpios | 脉冲发射 25~500ms |
| 36 | MDB nRST | OUT | 低有效，空闲输出高 | gpio-userspace `mdb_reset` | STM32F103 复位；`echo 0 > value` 复位 |
| 102 | SE nRST | OUT | 低有效，默认高 | gpio-userspace `se_reset` | Secure Element 复位 |

> 实机 model：`Qualcomm Technologies, Inc. Scuba IOT IDP`（overlay 用 scuba-iot-idp-overlay.dts，不是 Bengal）。

## 2. 驱动源码索引

| 驱动 | 源码（归档） | 设备节点 | 说明 |
|------|--------------|----------|------|
| meig_gpio_pulse | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/meig_gpio_pulse.c\|meig_gpio_pulse.c]] | /dev/uic_pulse、/sys/class/misc/uic_pulse | UIC 脉冲检测/发射字符设备 |
| gpio-userspace | （高通基线） | /sys/class/gpio_userspace/<label>/value | 复位脚 sysfs 导出 |
| pinctrl-scuba | [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/pinctrl-scuba.c\|pinctrl-scuba.c]] | — | 引脚复用/保留管理 |
| gpio-keys | （内核标准） | /dev/input/eventX | DB 检测按键（KEY_F1） |

## 3. DTS 配置要点

- overlay：`kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp-overlay.dts` → [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-iot-idp-overlay.dts|scuba-iot-idp-overlay.dts]]
- binding：`kernel_platform/qcom/proprietary/devicetree/bindings/misc/meig,gpio-pulse.txt` → [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/meig,gpio-pulse.txt|meig,gpio-pulse.txt]]

关键片段：

```dts
&tlmm {
    mt5205_pulse_default: mt5205_pulse_default { /* GPIO32 IN pull-up / GPIO33 OUT 8mA */ };
    mt5205_mdb_reset: mt5205_mdb_reset { /* GPIO36 output-high */ };
    mt5205_mdb_db_detect: mt5205_mdb_db_detect { /* GPIO14 IN pull-up */ };
};

&soc {
    meig_pulse: meig_pulse {
        compatible = "meig,gpio-pulse";
        pulse0: pulse@0 {
            in-gpios = <&tlmm 32 GPIO_ACTIVE_HIGH>;
            meig,in-active-high;
            out-gpios = <&tlmm 33 GPIO_ACTIVE_HIGH>;
            debounce-us = <2000>;
            min-width-us = <10000>;
            max-width-us = <500000>;
            default-emit-us = <50000>;
        };
    };
};
```

## 4. 内核配置

```text
CONFIG_MEIG_GPIO_PULSE=m        # bengal_GKI.config
CONFIG_GPIO_USERSPACE=m
```

## 5. 编译命令

```bash
# 平台：SM6115-A14 / MT5205，源码树 LA.VENDOR.13.2.1
# userdebug：consolidate 配置
./prepare_vendor.sh bengal consolidate
# user：gki 配置
./prepare_vendor.sh bengal gki
```

- Stage1 内核编译约 39 min，PASS 后 exit 0；
- 驱动编译为模块 meig_gpio_pulse.ko（Stage2/3 打包进 vendor 镜像）。

## 6. 设备端验证

```bash
insmod meig_gpio_pulse.ko
ls /dev/uic_pulse /sys/class/misc/uic_pulse/
cat /sys/class/misc/uic_pulse/in_raw        # 空闲 0
echo 50000 > /dev/meig_pulse0               # 旧接口发 50ms 脉冲
# 新接口：ioctl UIC_PULSE_IOC_OUTPUT（count/width/interval/level）
cat /sys/class/gpio_userspace/mdb_reset/value   # 1（空闲高）
getevent -c 3                                # 插拔 DB 上报 KEY_F1
```

## 7. 修改历史

见 [[01.驱动文档/GPIO/Qualcomm/SM6115-A14/91.源码与补丁索引/modified_history.md|modified_history]]（#195883 / #195886，均 ✅ 可干净应用）。

---

_Author: wangguanran_