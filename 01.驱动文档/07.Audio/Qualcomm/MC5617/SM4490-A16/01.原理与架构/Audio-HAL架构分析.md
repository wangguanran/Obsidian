# Audio HAL 架构分析（primary-hal 分层/路由）

> **版本号：v1.0**（基于 LA.VENDOR.15.4.5 / QCM4490.LA.4.0 parrot 平台）

## 1. 音频软件栈分层

Qualcomm 音频栈自顶向下：

```
应用层（MediaPlayer / AudioTrack）
        │
AudioFlinger（音频服务）
        │
AudioPolicyService（APM：路由决策，选定输出设备/策略）
        │  AIDL
        ▼
audio-hal（primary-hal）★ 本平台归档重点
        ├── AudioPrimaryHal（AIDL HAL 实现）
        ├── Platform（hal/core/platform/Platform.cpp）★ customizePalDevices 钩子
        └── configs/parrot/parrot.mk（平台产品属性）★
        │  PAL API
        ▼
PAL（Primitive Audio Library，libpal）
        │
ADSP / APM（Audio Processing Manager，固件侧）
        │
MI2S / SWR 等 backend → Codec / SmartPA / Amp
        ▼
Speaker / Handset / 耳机 等物理设备
```

关键点：

- **APM 决定"播什么、走哪"**：根据音频策略（audio_policy_configuration.xml 等）与当前设备状态选定输出 device（如 SPEAKER）；
- **HAL 决定"最终下发哪些设备给 PAL"**：`Platform::customizePalDevices()` 是设备列表下发前的最后修改点，可增删改 `std::vector<pal_device>`；
- **PAL 负责底层通路**：按设备 ID 映射到 DSP graph（MI2S/SWR 后端），同一路流可在多个后端同时输出。

## 2. primary-hal 目录结构（涉及归档文件）

```
vendor/qcom/opensource/audio-hal/primary-hal/
├── configs/parrot/parrot.mk     ← parrot 平台产品属性（PRODUCT_PROPERTY_OVERRIDES）
└── hal/core/platform/
    └── Platform.cpp             ← 平台层：设备转换、customizePalDevices 钩子
```

- **parrot.mk**：parrot 平台的 audio HAL 属性集中地（deep buffer、offload、A2DP offload、RAS、Dolby 等开关）。本平台新增的 `persist.vendor.audio.media.spk_rcv_dual` 即在此定义。
- **Platform.cpp**：`convertToPalDevices()`（AudioDevice → pal_device 转换）、`customizePalDevices()`（下发前定制：HAC 标记、HiFi PCM filter、媒体双通路等）。本平台的双通路功能全部实现在此。

## 3. 路由与设备模型

- 设备枚举：`PAL_DEVICE_OUT_SPEAKER`（外放）、`PAL_DEVICE_OUT_HANDSET`（听筒）、`PAL_DEVICE_OUT_WIRED_HEADSET/HEADPHONE`、A2DP 等；
- parrot 平台 Speaker 与 Handset 分属不同 MI2S 后端通路，设备列表同时包含两者时 PAL 会**多路同时输出**（本平台"听筒与扬声器同时播放"功能即基于此）；
- usecase 分类：媒体播放（PRIMARY / LOW_LATENCY / DEEP_BUFFER / ULL / MMAP / COMPRESS_OFFLOAD / PCM_OFFLOAD / SPATIAL）、语音（VOIP/Telephony）、录音等——HAL 层可按 usecase 差异化处理设备列表。

## 4. 与内核/固件的关系

- audio-kernel（vendor/qcom/opensource/audio-kernel）：asoc 驱动、lpass-cdc codec 等，MC5616 平台有 RX macro 裁剪案例（见平台总览）；
- ADSP 固件：PAL 与其通过 APM 命令交互，HAL 只做设备/属性层决策，不直接操作硬件寄存器。

_Author: wangguanran_