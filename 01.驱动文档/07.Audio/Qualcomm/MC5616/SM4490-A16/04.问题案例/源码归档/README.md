# 源码归档 — #196025 Disable RX macro/SWR path

## 归档信息

| 项目 | 内容 |
|------|------|
| **Change** | #196025 |
| **标题** | [MC5616][93821][Audio] Disable RX macro/SWR path, drop VA always-on |
| **作者** | zhourulei |
| **状态** | MERGED (Gerrit) / 未合入 134 仓库 |
| **归档日期** | 2026-08-18 |
| **源码来源** | 134 服务器 (`/home3/wangguanran/workspace/MC5616/LA.VENDOR.1.0.R1`) |
| **134 HEAD** | `6ebcb463ebb` — V13 compilation script update |

## 文件清单

| 文件 | 路径 | 大小 | 说明 |
|------|------|------|------|
| `lpass-cdc-rx-macro.c` | `vendor/qcom/opensource/audio-kernel/asoc/codecs/lpass-cdc/` | 154KB / 4846行 | RX macro 驱动源码（补丁前基线） |
| `waipio.c` | `vendor/qcom/opensource/audio-kernel/asoc/` | 67KB / 2453行 | SoC 音频平台驱动（补丁前基线） |
| `parrot-audio-overlay.dtsi` | `vendor/qcom/proprietary/audio-devicetree/` | 21KB / 700行 | 主 DTS 配置（补丁前基线） |
| `parrot-audio-qrd.dtsi` | `vendor/qcom/proprietary/audio-devicetree/` | 983B / 32行 | QRD DTS 配置（补丁前基线） |

## 补丁状态

- **134 仓库当前 HEAD**: `6ebcb463ebb` — 未包含 #196025 修改
- **补丁基线**: c2094387615 (SPF 2.0.1)
- **补丁已通过 Git 验证**：可干净应用到 134 HEAD 基线，无冲突
- **补丁合并状态**: 已提交 Gerrit 但未合入 134 仓库

## 文件修改历史

### lpass-cdc-rx-macro.c
```
aecee77cdeb SPF 2.0.2 baseline
c2094387615 SPF 2.0.1 baseline
```

### waipio.c
```
d27849eab23 [96634/96633] audio bring up speaker and mic
c2094387615 SPF 2.0.1 baseline
```

### parrot-audio-overlay.dtsi
```
6d0a4964b8e [114574] audio bring up speaker for cat1 (weirong)
d27849eab23 [96634/96633] audio bring up speaker and mic
100851c1c85 [NULL] bring up sound card — remove wcd and wsa
c2094387615 SPF 2.0.1 baseline
```

### parrot-audio-qrd.dtsi
```
8267868a5c6 [114575] audio bring up mic for cat1 (weirong)
d27849eab23 [96634/96633] audio bring up speaker and mic
100851c1c85 [NULL] bring up sound card — remove wcd and wsa
c2094387615 SPF 2.0.1 baseline
```