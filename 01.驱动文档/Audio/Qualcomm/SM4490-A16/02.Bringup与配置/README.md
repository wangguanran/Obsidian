# [项目代号] SM4490-A16 音频 Bringup 与配置

> **版本号：v1.0**（持续补充）

## 1. 硬件接口

| 接口 | 说明 |
|:---|:---|
| MI2S（primary） | Speaker 通路 backend |
| MI2S（handset） | Handset/听筒通路 backend |
| SWR（SoundWire） | codec 与 DSP 间音频数据总线（未用通路可裁剪，见 [项目代号] 案例） |
| I2C | SmartPA / 外部 codec 控制 |

> 具体引脚分配以 [项目代号] 原理图为准（parrot/QCM4490 参考设计）。

## 2. 驱动/配置索引

| 路径 | 说明 |
|:---|:---|
| `vendor/qcom/opensource/audio-hal/primary-hal/configs/parrot/parrot.mk` | parrot 平台 audio HAL 产品属性（归档：[[01.驱动文档/Audio/Qualcomm/SM4490-A16/91.源码与补丁索引/vendor_hal/parrot.mk\|parrot.mk]]） |
| `vendor/qcom/opensource/audio-hal/primary-hal/hal/core/platform/Platform.cpp` | HAL 平台层（设备定制钩子，归档：[[01.驱动文档/Audio/Qualcomm/SM4490-A16/91.源码与补丁索引/vendor_hal/Platform.cpp\|Platform.cpp]]） |
| `vendor/qcom/opensource/audio-kernel/` | 音频内核驱动（asoc、lpass-cdc codec） |
| `vendor/qcom/proprietary/audio-devicetree/` | 音频 DTS（parrot-audio-overlay.dtsi 等） |

## 3. 常用配置

### 属性开关（parrot.mk / setprop）

```bash
# 媒体双通路（Speaker+Handset 同时播放）
getprop persist.vendor.audio.media.spk_rcv_dual
setprop persist.vendor.audio.media.spk_rcv_dual true|false
```

### A2DP offload（平台默认）

```makefile
ro.bluetooth.a2dp_offload.supported=true
```

## 4. 验证命令

```bash
# 播放测试
adb push test.wav /data/local/tmp/ && adb shell tinyplay /data/local/tmp/test.wav

# 录音测试
adb shell tinycap /data/local/tmp/record.wav

# 音频路由/策略查看
adb shell dumpsys media.audio_flinger
adb shell dumpsys audio

# HAL 日志（设备选择、通路切换）
adb logcat -d | grep -iE "audiohal|pal|spk_rcv_dual"

# 通路状态
adb shell cat /proc/asound/cards
```

## 5. 修改历史

见 [[01.驱动文档/Audio/Qualcomm/SM4490-A16/91.源码与补丁索引/modified_history.md|modified_history.md]]。

_Author: wangguanran_