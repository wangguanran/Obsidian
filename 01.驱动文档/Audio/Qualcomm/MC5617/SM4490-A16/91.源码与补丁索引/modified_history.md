# MC5617 SM4490-A16 修改历史

## Change #196282 (2026-08-18 归档)

- 提交: [MC5617][TaskID]118650[Description]Add a feature that speaker and handset can play at the same time.[owner][同事]
- 涉及文件:
  - `vendor/qcom/opensource/audio-hal/primary-hal/configs/parrot/parrot.mk`（+5/−0，新增 `persist.vendor.audio.media.spk_rcv_dual=false`）
  - `vendor/qcom/opensource/audio-hal/primary-hal/hal/core/platform/Platform.cpp`（+52/−0，新增 `maybeAppendHandsetForMediaSpkRcvDual()`）
- 补丁验证: ✅ 可干净应用（134 源码树直接 git apply --check）

### 相关提交基线

| 文件 | 提交 | 基线版本 |
|:---|:---|:---|
| parrot.mk / Platform.cpp | bf78b5f27f9 | QCM4490.LA.4.0 \| Post-CS2 0.0.006.1a \| LA.VENDOR.15.4.5.r1-02200-WAIPIO.QISI16.0-1 |
| parrot.mk / Platform.cpp | 38a3ee8325c | QCM4490.LA.4.0 \| CS 0.0.004.1a \| LA.VENDOR.15.4.5.r1-01500-WAIPIO.QISI16.0-1 |

_Author: wangguanran_