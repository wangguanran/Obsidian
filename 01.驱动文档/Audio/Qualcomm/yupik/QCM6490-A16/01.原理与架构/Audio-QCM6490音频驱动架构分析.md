# Audio-QCM6490 音频驱动架构分析

> **芯片**: Qualcomm QCM6490 (yupik) | **平台**: QCM6490-A16

## 分层结构

```
应用层 (AudioService / HAL)
   │
   ▼
AUDIO HAL (audio.primary) ── ALSA mixer/pcm
   │
   ▼
ALSA / ASoC
   ├── machine: yupik_snd（驱动 qcom,audio-routing 等属性）
   ├── codec:  bolero（内置集成 codec，WCD 家族）
   ├── DSP:    ADSP（TX/VA/RX macro 走 SoundWire）
   └── 功放:   wsa883x_0221/0222（SoundWire 智能功放，WSA 通路）
   │
   ▼
硬件：扬声器（WSA）、DMIC0~3（GPIO150~153）、MIC BIAS（TLMM 96/97）
```

## 初始化流程

1. **UEFI/ABL 阶段**：无音频配置，音频由 Linux 侧接管；
2. **内核 probe**：machine 驱动（yupik_snd）读取 `qcom,model`、`asoc-codec`、`qcom,audio-routing`，注册 snd soc card；
3. **codec 注册**：bolero 按 `qcom,num-macros` 初始化 macro（TX/RX/VA/WSA）；MC934 关闭 WSA、VA/RX 走非 SWR 模式（`qcom,is-used-swr-gpio=<0>`）；
4. **声卡就绪**：`/proc/asound/cards` 出现 `lahaina-yupikidp-mcXXX-snd-card`；
5. **通路建立**：上层按 routing 使能 `WSA_SPK1/2 OUT`、`Digital Mic0~3`。

## 数据通路（播放）

```
AudioFlinger → HAL → ALSA pcm → machine(dai_link) → bolero RX path
   → SoundWire → wsa883x (DAC+功放) → 扬声器
```

## 数据通路（录音 DMIC）

```
DMIC0~3 (GPIO150~153) → TX/VA macro (Digital Micx) → ADSP
   → ALSA capture pcm → HAL → 应用
BIAS: vcc-micbias1/2-gpio (TLMM 96/97) → VCC MIC BIAS1
```

## 关键配置属性

| 属性 | 作用 |
|------|------|
| `qcom,model` | 声卡名（决定 /proc/asound/cards 显示） |
| `qcom,audio-routing` | DAPM 路由表（扬声器/DMIC/BIAS 通路） |
| `vcc-micbias1/2-gpio` | DMIC 偏置电源 GPIO |
| `fsa4480-i2c-handle` | Type-C 音频开关句柄（0=禁用） |
| `qcom,wcd-disabled` | 禁用外置 WCD codec |
| `qcom,num-macros` | bolero macro 数量（3=无 WSA） |
| `qcom,is-used-swr-gpio` | macro 是否使用 SWR GPIO 复位 |

## 与同平台其他模块的关系

音频 overlay 与相机（camera-devicetree 仓库，同分支 Rigel）共用 TLMM GPIO；GPIO96/97（micbias）与 150~153（DMIC）需避开相机/显示占用。per-SKU 结构与 [[01.驱动文档/Audio/Qualcomm/MC5617/SM4490-A16/03.需求实现/听筒与扬声器同时播放功能.md|MC5617 听筒+扬声器同播]] 的需求实现思路一致：机器级配置驱动通路差异。

---

_Author: wangguanran_
