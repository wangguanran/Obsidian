# SM6115-A14 修改历史

## Change #196371 (AP 侧)
- 提交: [[项目代号]][TaskID]118727[Description]fix JD9365DA init data for display error in AP[Owner][同事]
- 涉及文件: display/bengal-sde-display-idp.dtsi、display/bengal-sde-display-pinctrl.dtsi（DELETED）、display/dsi-panel-jd9365da-video.dtsi、display/scuba-sde-display-idp.dtsi、display/scuba-sde-display.dtsi
- 补丁验证: ✅ 可干净应用（134 源码树父提交重建，CRLF 转换后）

## Change #196374 (BP/UEFI 侧)
- 提交: [[项目代号]][TaskID]118727[Description]fix JD9365DA UEFI init data for display error in BP[Owner][同事]
- 涉及文件: BOOT.XF.4.1/boot_images/QcomPkg/Settings/Panel/Panel_jd9365da_720p_vid.xml
- 补丁验证: ✅ 可干净应用（134 源码树父提交重建）

## Change #196216 (UEFI 修复)
- 提交: [[项目代号]][TaskID]118727[Description] fix unused PmicGpioProtocol breaking other UEFI builds[Owner][同事]
- 涉及文件: BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/AgattiPkg/LAA/Core.fdf、BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/AgattiPkg/Library/MDPPlatformLib/MDPPlatformLib.c、BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/AgattiPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c
- 补丁验证: ✅ 可干净应用（134 源码树父提交重建）

## Change #196443 (需求)
- 提交: [[项目代号]][TaskID]120699[Description]Increase the display PWM frequency[Owner][同事]
- 涉及文件: display/scuba-sde-display-idp.dtsi
- 补丁验证: ✅ 可干净应用（134 源码树父提交重建）

---

_Author: wangguanran_
