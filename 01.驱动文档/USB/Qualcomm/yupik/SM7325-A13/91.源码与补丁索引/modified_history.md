# SM7325-A13 (yupik) USB 修改历史

## Change #196525 (2026-08-19)
- 提交: [MC9371][121129][USB] Restore ttyACM after usbfs release[Owner][同事]
- 涉及文件: kernel/msm-5.4/drivers/usb/class/cdc-acm.c、kernel/msm-5.4/drivers/usb/core/devio.c
- 补丁验证: ✅ 可干净应用（合并后源文件反向 apply --check 通过；源码树在 134 无工作副本，经 Gerrit REST 获取）

### 相关提交（同分支 USB 历史）
- 196525 [MC9371][121129][USB] Restore ttyACM after usbfs release（本 Change）
- 195508 [MC9371][120336][USB] Fix unstable ttyACM minor allocation for WCH CH343 multi-interface device
- 172375 [MC9371] Fix mcu ttyACM port

_Author: wangguanran_
