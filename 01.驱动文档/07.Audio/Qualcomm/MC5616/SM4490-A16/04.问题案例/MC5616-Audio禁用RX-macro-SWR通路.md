# MC5616 Audio 禁用 RX macro/SWR 通路降低功耗

## 概述

- **Change**: #196025
- **项目**: LA.VENDOR.1.0.R1
- **分支**: master_Snapdragon_Premium_High_2021.SPF.2.0.2_MC5616
- **作者**: zhourulei
- **状态**: MERGED
- **类型**: 需求 (power optimization)
- **芯片**: MC5616 (基于 parrot/QCM4490)
- **SoC-Android**: SM4490-A16

## 背景

MC5616 板未使用 LPASS RX SWR 通路（无 WCD RX / Awinic I2C PA），但 RX macro 及 RX SWR master 仍按默认使能，导致以下问题：

1. **RX macro 无意义初始化**：probe 阶段执行无意义的 SWR 初始化与 `add_child_devices` work 调度，增加不必要的功耗和启动延迟。
2. **VA SWR master 无法释放**：VA SWR master 保留 `always-on` / `irq-wakeup` 属性，造成 codeccore 无法释放。
3. **无效音频路由残留**：DTS 中残留无效的 RX_TX DEC 音频路由配置。

## 解决方案

### DTS 配置

1. 将 `rx-macro` 及其 `swr-mstr` 设为 `disabled`
2. `num-macros` 由 3 改为 2
3. VA SWR master 的 `irq-wakeup-capable` 与 `is-always-on` 置 0
4. 删除无效 RX_TX DEC 路由

### Kernel 驱动修改

- `waipio.c`：用 `#if 0` 禁用 RX CDC DMA DAI link
- `lpass-cdc-rx-macro.c`：修改 RX macro 禁用逻辑

## 修改文件清单

| 文件 | 变更 | 说明 |
|------|------|------|
| `vendor/qcom/opensource/audio-kernel/asoc/codecs/lpass-cdc/lpass-cdc-rx-macro.c` | +18/−13 | 修改 RX macro 禁用 |
| `vendor/qcom/opensource/audio-kernel/asoc/waipio.c` | +8/−2 | 禁用 RX CDC DMA DAI link |
| `vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi` | +7/−7 | 禁用 rx-macro 及其 swr-mstr |
| `vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi` | +0/−4 | 删除无效 RX_TX DEC 路由 |

> 补丁内容请参考 Gerrit Change #196025

## 配置方式

### DTS 配置

在 `parrot-audio-overlay.dtsi` 中禁用 RX macro 及 SWR master：

```dts
&rx_macro {
    status = "disabled";
};

&swr3 {
    status = "disabled";
};
```

调整 `lpass-cdc` 节点中 macro 数量：

```dts
&lpass_cdc {
    qcom,num-macros = <2>;  /* 由 3 改为 2 */
};
```

清除 VA SWR master 的 always-on / wakeup 属性：

```dts
&swr0 {
    qcom,irq-wakeup-capable = <0>;
    qcom,is-always-on = <0>;
};
```

在 `parrot-audio-qrd.dtsi` 中删除无效的 RX_TX DEC 路由。

### Kernel 配置

- 无需额外 kernel config 修改
- 通过 `#if 0` 在 `waipio.c` 中编译屏蔽 RX CDC DMA DAI link

## 验证方式

### 1. 编译验证

确保 kernel 和 DTS 编译通过，无新增 warning：

```bash
# 编译 kernel
make ARCH=arm64 ...
# 编译 DTS
make dtbs
```

### 2. 启动日志验证

检查 kernel 启动日志，确认 RX macro 不再 probe：

```bash
# 确认 RX macro 未被 probe
dmesg | grep -i "rx-macro"
# 预期：无相关 probe 日志输出
```

### 3. 功耗验证

通过以下方式确认低功耗状态：

```bash
# 查看 audio codec 电源状态
cat /sys/kernel/debug/regulator/...

# 查看 SWR 总线状态
cat /sys/kernel/debug/swr_master/...
```

### 4. 功能验证

验证音频播放/录音功能正常：

```bash
# 播放测试
tinyplay /data/test.wav

# 录音测试
tinycap /data/record.wav
```

> ⚠️ 补丁验证：无法直接获取，请在实际硬件上按上述步骤验证。

## 源码归档

- **Gerrit**: #196025（暂无法直接获取补丁内容）
- **分支**: master_Snapdragon_Premium_High_2021.SPF.2.0.2_MC5616
- **项目**: LA.VENDOR.1.0.R1

## 引用文件索引

- [[01.驱动文档/07.Audio/Qualcomm/MC5616/SM4490-A16/04.问题案例/分析/MC5616-Audio禁用RX-macro-SWR通路-分析|分析文档]]
- 关联模块：[[01.驱动文档/07.Audio/Qualcomm/|Qualcomm Audio 驱动文档]]

---

_Author: wangguanran_