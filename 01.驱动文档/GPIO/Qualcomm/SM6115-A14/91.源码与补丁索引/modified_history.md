# SM6115-A14 修改历史

## Change #195883
- 提交: [MT5205][TaskID]118743[Description]fix UIC pulse GPIO32/33 direct-wire detect and sysfs[Solution]ACTIVE_HIGH IN, batch_timer cancel, debounce default 2ms, update binding and DT overlay[Owner]wangguanran
- 涉及文件: bengal_GKI.config、drivers/misc/Kconfig、drivers/misc/Makefile、drivers/misc/meig_gpio_pulse.c、include/uapi/linux/uic_pulse.h、devicetree/bindings/misc/meig,gpio-pulse.txt、devicetree/qcom/scuba-iot-idp-overlay.dts
- 补丁验证: ✅ 可干净应用（134 源码树父提交重建，新文件由补丁创建）

## Change #195886
- 提交: [MT5205][TaskID]120572[Description]MDB nRST output-high and GPIO14 DB detect key[Solution]idle mdb_reset output-high, unreserve GPIO14, gpio-keys KEY_F1[Owner]wangguanran
- 涉及文件: drivers/pinctrl/qcom/pinctrl-scuba.c、devicetree/qcom/scuba-iot-idp-overlay.dts
- 补丁验证: ✅ 可干净应用（134 源码树父提交重建）

---

_Author: wangguanran_
