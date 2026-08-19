# 分析：Rigel A16 音频各 SKU 设备树配置

**版本号：v1.0**
**对应文档：** [[01.驱动文档/Audio/Qualcomm/yupik/QCM6490-A16/03.需求实现/Rigel-A16音频各SKU设备树配置.md|Rigel-A16音频各SKU设备树配置]]

## 技术背景

Qualcomm yupik（QCM6490）平台音频使用 ASoC 框架：

- **yupik_snd**：machine driver 节点（qcom,audio-routing 描述 DAPM 路由），codec 侧为 bolero（WCD 家族集成 codec）+ WSA883x 智能功放（WSA 通路）；
- **WSA 通路**：`WSA_SPK1/2 OUT` → WSA883x → 扬声器；`SpL/SpR IN` 为 WSA 回环输入；
- **DMIC**：TX/VA macro 的 DMIC0~3 对应数字麦克风，BIAS 由 `vcc-micbias1/2-gpio`（TLMM 96/97）控制；
- **SWR**：va_macro/rx_macro 的 SoundWire 端口，MC934 无 WSA 时通过 `qcom,is-used-swr-gpio = <0>` + `/delete-property/ qcom,va-swr-gpios` 关闭；
- **fsa4480**：Type-C 音频开关；`fsa4480-i2c-handle = <0>` 表示不走 Type-C 音频路径（板载 3.5mm/模拟通路）。

## 代码改动分析

| 文件 | 改动内容 | 关键点 |
|------|---------|--------|
| Kbuild | 删除 `kera-audio-mtp-wcn7750.dtbo`、`kera-audio-evk-wcd9378-dmic.dtbo` | 收敛编译产物，避免生成无用 dtbo |
| yupik-audio-overlay-mc933/936/937/938/9392.dtsi | 结构相同：model + micbias + fsa4480 + routing（WSA 双声道 + DMIC0~3 TX/VA） | 各 SKU 仅 model 名不同；DMIC 注释标注 GPIO150~153 |
| yupik-audio-overlay-mc934.dtsi | 无 WSA 变体：bolero num-macros=3、wsa_macro/wsa883x disabled、VA/RX 去 SWR GPIO | 与带 WSA 变体结构差异最大 |
| yupik-iot-audio-overlay.dtsi | `qcom,ext-disp-audio-rx = <0>` | 关闭外部显示（HDMI/DP）音频接收，公共默认 |

## 潜在风险

1. **routing 交叉影响**：`qcom,audio-routing` 是全局 map，MC934 若误加载带 WSA routing 的公共配置会 probe 失败（WSA 节点 disabled）；各 SKU overlay 必须与硬件严格对应。
2. **GPIO 冲突**：TLMM 96/97 作为 micbias 使能，若与其他外设共用需核对 pinctrl。
3. **DMIC 增益/偏置**：routing 只解决通路，具体偏置电压/增益仍依赖 codec 驱动参数，实机需调测。
4. **Kbuild 收敛**：移除的 dtbo 若仍有工厂/量产镜像引用，会导致 dtbo 缺失——需确认下游无依赖。

## 回归测试建议

- 每个 SKU 冒烟：播放（WSA）、录音（DMIC0~3）、通话（如有）、声卡节点名正确；
- MC934 专项：确认无 WSA 设备节点（`wsa883x` 不 probe），系统无报错；
- HDMI/DP 音频：确认 ext-disp 音频不再占用（`qcom,ext-disp-audio-rx=0` 后 HDMI 音频不可用属预期）；
- 整包编译：确认 dtbo 列表与 Kbuild 一致，无缺失 dtbo 警告。

## 与现有驱动架构的关系

该提交是设备树层配置，与内核 ASoC 驱动（machine/bolero/wsa883x）解耦；同一 yupik 平台的其它项目（如 MC937 相机、Rigel 系列）可复用该 overlay 模式。与既有 QCM6490-A16 平台的差异主要体现在 SKU 级音频通路裁剪。

_Author: wangguanran_
