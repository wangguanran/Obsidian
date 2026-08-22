# [项目代号] (parrot) Audio LPASS 通路架构分析

> **版本号：v1.0**

## 平台概述

[项目代号]（基于 parrot/QCM4490）的音频子系统使用 **LPASS（Low Power Audio SubSystem）** 内建 codec（lpass-cdc）。音频通路分为 VA（Voice Activity）与 RX（Playback）两大类：

```
应用层播放/录音
   ↓ Audio HAL (audio HAL → acdb/audio route)
LPASS 内建 codec (lpass-cdc)
   ├─ VA 通路: VA macro → ADSP 录音/唤醒处理   ← 保留
   ├─ RX 通路: RX macro → SWR master → 外置 PA   ← [项目代号] 未使用，已禁用
   └─ CDC DMA DAI link (waipio.c 定义)           ← RX 侧已 #if 0 屏蔽
```

## 通路组成

| 通路 | 组成 | [项目代号] 状态 |
|:---|:---|:---|
| VA（录音/唤醒） | VA macro + codec 电源管理 | ✅ 保留，移除 always-on 限制 |
| RX（播放） | RX macro + RX SWR master + CDC DMA | ❌ 禁用（无 WCD RX / Awinic I2C PA） |

## 关键机制

### 1. RX macro probe 流程（已禁用）

默认配置下 RX macro 在 probe 阶段会：

1. 执行 SWR（SoundWire）初始化
2. 调度 `add_child_devices` work，添加 SWR slave 子设备
3. 维持 `qcom,is-always-on` 供电，阻止 codec 电源管理释放

[项目代号] 板无对应硬件，上述操作无实际意义，还会增加功耗与启动延迟。

### 2. 禁用方式（三层联动）

| 层次 | 修改 | 作用 |
|:---|:---|:---|
| DTS overlay | `parrot-audio-overlay.dtsi` 中 `rx-macro` 节点 `status = "disabled"`，SWR master 一并禁用 | 阻止 RX macro probe |
| DTS qrd | `parrot-audio-qrd.dtsi` 删除无效 RX_TX DEC 路由（+0/−4） | 清理残留路由配置 |
| 驱动源码 | `waipio.c` 用 `#if 0` 编译屏蔽 RX CDC DMA DAI link | 移除播放侧 DAI 注册 |

### 3. 电源管理收益

- **修改前**：`qcom,is-always-on = <1>` 使 codec 持续供电，无法进入低功耗
- **修改后**：VA 通路移除 always-on 限制，由正常电源管理策略控制，codec 可释放

## 注意事项

- 禁用 RX macro 后，需确保 `num-macros` 计数与使能的 macro 数量一致，否则驱动 probe 可能失败
- 若后续硬件变更需启用 RX 通路，需反向恢复上述 DTS 和驱动修改（仅影响 [项目代号]/parrot，不涉及其他 parrot 衍生板型）

## 引用文件索引

- [[01.驱动文档/Audio/Qualcomm/SM4490-A16/03.需求实现/Audio禁用RX-macro-SWR通路.md|Audio禁用RX-macro-SWR通路]]（变更明细与补丁）
- `vendor/qcom/proprietary/audio-devicetree/parrot-audio-overlay.dtsi`（远程源码树）
- `vendor/qcom/proprietary/audio-devicetree/parrot-audio-qrd.dtsi`（远程源码树）
- `kernel_platform/msm-kernel/sound/soc/codecs/lpass-cdc-rx-macro.c`、`waipio.c`（远程源码树）

---

_Author: wangguanran_
