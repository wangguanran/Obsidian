---
创建时间: 2026年03月02日 晚上 19:39:05 星期一
最后修改时间: 2026年03月04日 上午 09:14:03 星期三
version: "1.0"
---

## NFC abort链路与处理逻辑归档（PN7221）

更新时间：2026-03-02

### 1. 现象
压测（每3秒开关NFC并验证读卡）过程中出现两类崩溃：
- `com.android.nfc` 进程 `SIGABRT`
- `nfc_pn72xx@1.2-service` 进程 `SIGABRT`

### 2. 证据
日志样本：
- `Desktop/log-582---开机失败且压测过程中hal层崩溃.log`
- `Desktop/log-582-0227-02----不上报内容.log`

关键时间线（示例窗口）：
- 随后出现 `NFA_DM_NFCC_TIMEOUT_EVT; abort`
- 然后 `com.android.nfc` `SIGABRT`
- 重启恢复阶段连续 `write_unlocked failed ... max count = 0x6` + `i2c ret(-107)`
- 最后 `nfc_pn72xx@1.2-service` `SIGABRT`

### 3. 触发 abort 的调用链

#### 3.1 Framework 侧 abort 链路（com.android.nfc）
1. NFCC上报 `CORE_RESET_NTF`，reason=`0xA7`
2. HAL扩展解析到非白名单reason，走异常恢复入口：
   - `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/hal/phNxpNciHal_ext.cc:523`
   - `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/hal/phNxpNciHal_ext.cc:555`
3. `phNxpNciHal_emergency_recovery(0xA7)` 在当前版本走 default，仅记录 invalid status：
   - `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/utils/phNxpNciHal_utils.cc:493`
4. NCI stack 对 `CORE_RESET_NTF` 只把 `0x01/0x02` 视为正常，其余走错误恢复并触发 `nfc_ncif_cmd_timeout()`：
   - `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/system/nfc/src/nfc/nfc/nfc_ncif.cc:1668`
   - `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/system/nfc/src/nfc/nfc/nfc_ncif.cc:1684`
5. 上层回调进入 `NFA_DM_NFCC_TIMEOUT_EVT; abort`，最终 `com.android.nfc` `SIGABRT`（日志已证实）。

补充：Framework还有独立 watchdog 主动 abort 通道（与上面 timeout 链可叠加）：
- `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/packages/apps/Nfc/src/com/android/nfc/NfcService.java:2231`
- `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/packages/apps/Nfc/nci/jni/NativeNfcManager.cpp:1783`

#### 3.2 HAL service 侧 abort 链路（nfc_pn72xx@1.2-service）
1. 恢复/关闭阶段出现连续 I2C 失败（`ret(-107)`），`write_unlocked` 重试耗尽：
   - `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/hal/phNxpNciHal.cc:1328`
   - `workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/hal/phNxpNciHal.cc:1342`
2. 进入 reset/recovery 后仍无法恢复通信，日志落到 `SIGABRT`（service进程）。

### 4. 0xA7 的定义（文档来源）
在以下文档中有明确定义：
- `Obsidian/小王的技术站/01.驱动文档/NFC/01.NXP PN7221/02.Doc/UM11810.pdf`
- Section `7.3.8`, Table 24 (`Proprietary reason codes in CORE_RESET_NTF`)

定义摘要：
- `0xA7` 表示 POR 后接口层（HIF1_I2C/HIF_SPI/HIF2_I2C）收到不符合期望的包序列（如非 `CORE_RESET_CMD` 包或非法包）。

### 5. 直接原因 vs 实际原因
- 直接原因：
  - 收到 `CORE_RESET_NTF(0xA7)` 后，NCI stack 将其判为错误 reset，触发 timeout/recovery，最终进入 abort。
  - 恢复阶段 I2C 连续失败导致 HAL service 侧也出现 abort。
- 实际原因：
  - 压测反复开关NFC时，开关电/复位窗口与读写并发重叠，导致 POR 后包序不满足芯片期望（触发 `0xA7`）并进一步劣化为通信失败。

### 6. 验证判据
满足以下链路即判定同类问题：
1. 日志中出现 `CORE_RESET_NTF` 且 reason=`0xA7`
2. 随后出现 `NFA_DM_NFCC_TIMEOUT_EVT` 或等价 timeout 恢复日志
3. 进程出现 `com.android.nfc` 或 `nfc_pn72xx@1.2-service` 的 `SIGABRT`
4. 同窗口伴随 `write_unlocked failed`、`i2c read/write ret(-107)`

### 7. 处理策略（当前共识）
- 收紧关断窗口：NFC `power off/closing` 置位后，HAL/驱动立即阻断新I/O。
- 串行化开关与读写：禁止 `disable` 窗口残留读写穿透到NFCC。
- 避免POR后早期错误包进入芯片，降低 `0xA7` 触发概率。

_Author: wangguanran_
