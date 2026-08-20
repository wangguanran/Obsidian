# QCM6490 触摸与手势唤醒移植资料

## 硬件接口（触摸 IC）

| 项目 | eGalax | SiS |
|------|--------|-----|
| 驱动文件 | egalax_i2c.c | sis_i2c.c |
| 总线 | I2C | I2C |
| 使能 GPIO | — | en_gpio（gpio_is_valid 校验） |
| 复位 GPIO | — | rst_gpio（gpio_is_valid 校验） |
| 中断 | client->irq | client->irq |

## DTS 配置要点

- 触摸节点需配置 `qcom,panel-notifier-support`（显示侧）以建立触摸↔显示事件链
- eDP 默认面板 phandle：`qcom,edp-default-panel`
- 手势唤醒：`gesture_mode_enable`（驱动内可调）

## 编译与模块

- 触摸驱动：`CONFIG_TOUCHSCREEN_EGALAX_SERIAL` / SiS 相关 config（vendor dlkm 模块）
- 显示驱动：`CONFIG_DRM_MSM_DP`（yupik DP 子系统）

## 相关提交

- #196796（touch-drivers）：手势唤醒 TP 驱动支持
- #196795（display-drivers）：手势唤醒显示驱动支持（drm_panel 注册）

_Author: wangguanran_
