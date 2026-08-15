---
创建时间: 2026年03月02日 晚上 19:17:48 星期一
最后修改时间: 2026年03月23日 上午 09:02:03 星期一
---

## MT5825 NFC HAL主动崩溃触发逻辑说明

### 结论速览
NFC HAL 的“主动崩溃”由 `phNxpNciHal_emergency_recovery()` 内部显式 `abort()` 触发，不是随机崩溃。

### 1. 现象
压测中出现 NFC 相关进程 `SIGABRT`，表现为 NFC 服务重启或 NFC 功能中断。

### 2. 证据
- HAL 内显式 `abort()`：
  - `/home1/wangguanran/workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/utils/phNxpNciHal_utils.cc:484`
  - `/home1/wangguanran/workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/utils/phNxpNciHal_utils.cc:490`
- 触发入口（CORE_RESET_NTF 分支）：
  - `/home1/wangguanran/workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/hal/phNxpNciHal_ext.cc:517`
  - `/home1/wangguanran/workspace/MT582_A14/LA.VENDOR.13.2.1.R2/hardware/nxp/nfc/pn72xx/halimpl/hal/phNxpNciHal_ext.cc:555`
- Framework 侧也存在独立 abort 路径（用于 watchdog）：
  - `/home1/wangguanran/workspace/MT582_A14/LA.VENDOR.13.2.1.R2/packages/apps/Nfc/src/com/android/nfc/NfcService.java:2231`
  - `/home1/wangguanran/workspace/MT582_A14/LA.VENDOR.13.2.1.R2/packages/apps/Nfc/nci/jni/NativeNfcManager.cpp:1783`

### 3. 调用逻辑
1. HAL 收到 `CORE_RESET_NTF`，在 `phNxpNciHal_ext_process_nfc_init_rsp()` 解析。
2. 当 `p_ntf[3]` 不属于“允许继续流程”的 3 类原因时：
   - `CORE_RESET_CMD_RECEIVED (0x02)`
   - `MODE_SWITCH_TO_NFC_FORUM (0xA8)`
   - `MODE_SWITCH_TO_EMVCO (0xA9)`
3. 进入 `phNxpNciHal_emergency_recovery(p_ntf[3])`。
4. 在 `phNxpNciHal_emergency_recovery()` 根据 reason code 执行 `abort()`。

### 4. 直接原因
`phNxpNciHal_emergency_recovery()` 命中以下状态码时直接 `abort()`：
- `0xA1` Over temperature
- `0xA0` FW assert
- `0xA3` Watchdog reset
- `0xA4` Input clock lost
- `0x00` Unrecoverable error

此外：
- `0x01` Powered on 仅在 `hal_open_status == true` 时也会 `abort()`。

### 5. 实际原因
当前可确认到：
- HAL 主动崩溃是“策略性 fail-fast”，由 NFCC 上报的 `CORE_RESET_NTF reason` 触发。

当前待继续确认项：
- 这些 reason code 的上游硬件/驱动根因（I2C 时序、电源/时钟稳定性、并发关断窗口）是哪一条主因。

### 6. 验证方法
1. 在异常窗口抓取 HAL 日志，定位 `CORE_RESET_NTF` 原始帧并提取 `p_ntf[3]`。
2. 对照本文件 reason code 表，确认是否命中 `emergency_recovery -> abort()` 分支。
3. 同时区分是否为 Framework watchdog 分支：
   - 若出现 `Watchdog triggered, aborting.`，是 Framework 路径。
4. 若需判定根因，继续向上游核对同时间窗口：
   - I2C read/write 失败点
   - VEN/IRQ 电平状态
   - power off 与读写并发时序

### 7. 补充说明（避免误判）
- `HAL abort` 与 `Framework watchdog abort` 是两条独立路径，可能先后出现，不能混为一个根因。
- 本文只确认“触发崩溃的确切代码逻辑”，不等于已完成“驱动通信失败根因”闭环。
