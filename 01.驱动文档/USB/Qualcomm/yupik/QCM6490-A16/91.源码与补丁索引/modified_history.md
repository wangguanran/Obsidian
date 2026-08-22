# QCM6490-A16 (yupik) USB 修改历史

## Change #196565 (2026-08-19)
- 提交: Revert "[MC936_Linux][TaskID]113211[Description]pcie-usb qcom patch[Solution]do it.[owner][同事]"
- 涉及文件: UsbEnumer.c、Xhci.c、XhciDxe.inf、Apriori.fdf.inc、Core.dsc.inc、Core.fdf、CoreAux.fdf.inc、uefiplat.cfg、PmicCoreLib.inf、pm_core.c、PcieConfigLib.c、boot_uart.c、Tftp.c、TftpDynamicCommand.inf、mg_build.py（共 16 文件）
- 补丁验证: ✅ 可干净应用（format-patch 与 FETCH_HEAD~1..FETCH_HEAD 提交 diff 逐字节一致；工作树含本地改动未做工作树级 apply）

### 相关提交（UEFI 历史）
- 460aafd Revert "[MC936_Linux] pcie-usb qcom patch"（#196565）
- 7998575 [119387][[项目代号]_Ubuntu][LCD]Fix issue that dp can not bringup
- cc0e3a2 [MC936_Linux][TaskID]113211 pcie-usb qcom patch（被回退对象）
- 4837dfd [[项目代号]_Ubuntu][TaskID]117628 add odm_rw sysfs nodes for SN and MAC
- ee09c25 [92106][[项目代号]_Ubuntu][LCD]Fix issue that MC937 cannot bring up edp panel when resume

_Author: wangguanran_
