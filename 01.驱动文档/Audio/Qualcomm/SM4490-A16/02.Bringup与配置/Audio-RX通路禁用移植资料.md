# [项目代号] (parrot) Audio RX 通路禁用移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：[项目代号]（Qualcomm parrot/QCM4490 衍生）
- **功能**：LPASS 内建 codec 音频通路控制
- **平台**：SM4490-A16（LA.VENDOR.1.0.R1）

## 背景

[项目代号] 板未使用 LPASS RX SWR 通路（无 WCD RX / Awinic I2C PA），但 RX macro 及 RX SWR master 仍按默认使能，导致无意义初始化、功耗与启动延迟增加。解决方案为三层禁用。

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| 音频通路 | LPASS 内建 codec（lpass-cdc），VA 通路保留 |
| 外置 PA | 无（无 WCD RX、无 Awinic I2C PA） |
| RX 通路 | RX macro + SWR master（已禁用） |

## 修改文件

| 文件 | 变更 | 说明 |
|:---|:---|:---|
| `vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi` | +7/−7 | 禁用 rx-macro 及其 swr-mstr |
| `vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi` | +0/−4 | 删除无效 RX_TX DEC 路由 |
| `sound/soc/codecs/lpass-cdc-rx-macro.c` | +31/−13 | RX macro 禁用适配 |
| `sound/soc/codecs/waipio.c` | +10/−2 | `#if 0` 屏蔽 RX CDC DMA DAI link |

## 配置方式

### DTS 配置

在 `parrot-audio-overlay.dtsi` 中禁用 RX macro 及 SWR master：

```dts
&rx_macro {
    status = "disabled";   // 无实际硬件连接，不应 probe
};
&rx_swr_master {
    status = "disabled";
};
```

在 `parrot-audio-qrd.dtsi` 中删除无效的 RX_TX DEC 路由。

### 驱动编译屏蔽

```c
#if 0
/* RX CDC DMA links above are disabled (no RX SWR / Awinic I2C PA). */
... RX CDC DMA DAI link 定义 ...
#endif
```

## 编译与验证

```bash
# 编译 kernel 与 DTS，确认无新增 warning
mmm kernel_platform/msm-kernel  # 或项目全量脚本

# 启动后确认 RX macro 未被 probe
adb shell dmesg | grep -i "rx-macro"   # 预期：无相关 probe 日志

# 查看 audio codec 电源状态，确认可进入低功耗
adb shell cat /sys/power/runtime_status
```

## 移植注意事项

- 其他 parrot 衍生板型不受影响；仅 [项目代号] 需此修改
- 若需启用 RX 通路（后续加外置 PA），反向恢复 DTS 与驱动修改
- 注意 `num-macros` 计数与使能的 macro 数量一致

## 引用文件索引

- [[01.驱动文档/Audio/Qualcomm/SM4490-A16/03.需求实现/Audio禁用RX-macro-SWR通路.md|Audio禁用RX-macro-SWR通路]]（完整补丁内容与源码归档）
- `vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi`（远程源码树）
- `vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi`（远程源码树）
- `kernel_platform/msm-kernel/sound/soc/codecs/lpass-cdc-rx-macro.c`（远程源码树）
- `kernel_platform/msm-kernel/sound/soc/codecs/waipio.c`（远程源码树）

---

_Author: wangguanran_
