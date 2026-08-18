# SM6115-A14 (scuba) 修改历史

## Change #196403 (2026-08)

- 提交: `[MT5205][120577][usb][Description]USB Type-C DIP ID host mux hubreset[Owner]tengqi`
- 涉及文件:
  - `kernel_platform/msm-kernel/arch/arm64/configs/vendor/bengal_GKI.config` (+1/-0)
  - `kernel_platform/msm-kernel/drivers/extcon/extcon-usb-gpio.c` (+27/-3)
  - `kernel_platform/msm-kernel/drivers/usb/dwc3/dwc3-msm-core.c` (+19/-0)
  - `kernel_platform/qcom/proprietary/devicetree/bindings/extcon/extcon-usb-gpio.txt` (+5/-0)
  - `kernel_platform/qcom/proprietary/devicetree/qcom/scuba-iot-idp.dtsi` (+77/-1)
  - `kernel_platform/qcom/proprietary/devicetree/qcom/scuba-thermal.dtsi` (+2/-0)
  - `kernel_platform/qcom/proprietary/devicetree/qcom/scuba.dtsi` (+2/-13)
  - `kernel_platform/qcom/proprietary/devicetree/qcom/scuba_auto-pmic.dtsi` (+1/-13)
- 补丁验证: ✅ 可干净应用（134 源码树父提交重建验证）
- 补丁归档: [[01.驱动文档/15.USB/Qualcomm/SM6115-A14/91.源码与补丁索引/patches/196403.patch|196403.patch]]

---

_Author: wangguanran_