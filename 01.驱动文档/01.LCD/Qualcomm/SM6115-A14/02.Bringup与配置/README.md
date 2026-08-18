# SM6115-A14 LCD Bringup 与配置

> **模块**: 01.LCD | **芯片**: SM6115 (scuba) | **平台**: SM6115-A14（LA.VENDOR.13.2.1 / MT5205）

## 1. 硬件接口对照表（MT5205 LCM）

| 信号 | GPIO/资源 | 电平 | 说明 |
|------|-----------|------|------|
| LCD_IOVCC_EN | GPIO111 | 1.8V | IOVDD（vddio） |
| LCM_VDD2V8_EN | GPIO112 | 2.8V | VCC（avdd） |
| DISP_BIAS_EN | GPIO105 | 2.8V | avee |
| LCD_RST_N | GPIO82 | 低有效 | 面板复位（platform-reset-gpio） |
| LCM_TE | GPIO81 | — | TE 同步（mdp_vsync / platform-te-gpio） |
| BL_FB | GPIO24 | — | 背光电流档位（4inch LOW/40mA，5inch HIGH/80mA，由 BP 设置） |
| BL_PWM | PM2250 PWM3 / PM4125 GPIO2 | 20kHz | 背光 PWM（周期 50us，亮度 1~4095） |

## 2. 源码索引

| 侧 | 文件（归档） | 说明 |
|----|--------------|------|
| AP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/scuba-sde-display-idp.dtsi|scuba-sde-display-idp.dtsi]] | MT5205 显示装配（电源/面板/背光 PWM） |
| AP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/dsi-panel-jd9365da-video.dtsi|dsi-panel-jd9365da-video.dtsi]] | JD9365DA 面板与 init data |
| AP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/bengal-sde-display-idp.dtsi|bengal-sde-display-idp.dtsi]] | Bengal 参考板（td4330，无 MT5205 面板） |
| AP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/dt_config/bengal-sde-display-pinctrl.dtsi|bengal-sde-display-pinctrl.dtsi]] | 已删除文件（父提交版本 437B） |
| BP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/Settings/Panel/Panel_jd9365da_720p_vid.xml|Panel_jd9365da_720p_vid.xml]] | UEFI init data |
| BP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/AgattiPkg/LAA/Core.fdf|Core.fdf]] | XBL FREEFORM 段（打包 Panel XML） |
| BP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/AgattiPkg/Library/MDPPlatformLib/MDPPlatformLib.c|MDPPlatformLib.c]] | 面板注册表（ODM_PROJECT_MT5205） |
| BP | [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/AgattiPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c|MDPPlatformLibPanelCommon.c]] | PWM 背光初始化 |

## 3. DTS 配置要点（AP 侧）

```dts
/* scuba-sde-display-idp.dtsi — MT5205 电源组 */
dsi_panel_pwr_supply_mt5205: dsi_panel_pwr_supply_mt5205 {
    qcom,panel-supply-entry@0 { qcom,supply-name = "vddio"; /* 1.8V, 62mA */ };
    qcom,panel-supply-entry@1 { qcom,supply-name = "avdd";  /* 2.8V, 100mA */ };
    qcom,panel-supply-entry@2 { qcom,supply-name = "avee";  /* 2.8V, 100mA */ };
};

/* 面板节点（关键属性） */
&dsi_jd9365da_video {
    qcom,panel-supply-entries = <&dsi_panel_pwr_supply_mt5205>;
    qcom,mdss-dsi-bl-pmic-control-type = "bl_ctrl_pwm";
    pwms = <&pm2250_pwm3 0 0>;
    qcom,bl-pmic-pwm-period-usecs = <50>;        /* 20kHz */
    qcom,mdss-dsi-bl-min-level = <1>;
    qcom,mdss-dsi-bl-max-level = <4095>;
    qcom,platform-te-gpio = <&tlmm 81 0>;
    qcom,platform-reset-gpio = <&tlmm 82 0>;
};
```

> 注意：`qcom,panel-supply-entries` 引用在 scuba 侧定义；bengal 侧不承载 MT5205 面板。splash_region（scuba-sde-display.dtsi）为 0x5c000000 起 15MB cont_splash_region。

## 4. UEFI 配置要点（BP 侧）

- 面板 XML：`QcomPkg/Settings/Panel/Panel_jd9365da_720p_vid.xml`，init data 命令类型 29/15，结束序列 `FF C8`；
- Core.fdf：`FILE FREEFORM = 9bae75d9-... { SECTION UI/RAW = Panel_jd9365da_720p_vid.xml }`，缩进层级正确；
- MDPPlatformLib.c：JD9365DA 条目 `PANEL_CREATE_ENTRY("dsi_jd9365da_720p_video", MDPPLATFORM_PANEL_JD9365DA_720P_VIDEO, "qcom,mdss_dsi_jd9365da_video:", ...)` 位于 `#if defined(ODM_PROJECT_MT5205)` 内；
- 背光 PWM mux：MT5205 将 PM4125 GPIO2 mux 为 PWM SPECIAL_FUNCTION1（非 PMI632 GPIO6 WLED）——见 MDPPlatformLibPanelCommon.c 的 `#if defined(ODM_PROJECT_MT5205)` 分支。

## 5. 编译命令

```bash
# AP 侧（LA.VENDOR.13.2.1 / MT5205）
./prepare_vendor.sh bengal consolidate      # userdebug
./prepare_vendor.sh bengal gki              # user

# BP 侧（iot-high-mid-2024-spf-1-0_amss_standard_oem / master_meig）
# BOOT.XF.4.1 单独构建 XBL（eg. python build.py tgt=... 或 CI 流水线）
```

## 6. 验证

1. UEFI 阶段：splash 正常，无花屏；
2. Kernel：`dmesg | grep -i mdss` 无 DSI init 错误；亮度 1~4095 线性；
3. 背光实测：PWM 周期约 50us（20kHz），无可闻噪声；
4. 回归：st7701s 与 jd9365da 两块屏、suspend/resume 各 50 次。

## 7. 修改历史

见 [[01.驱动文档/01.LCD/Qualcomm/SM6115-A14/91.源码与补丁索引/modified_history.md|modified_history]]：#196371 / #196374 / #196216 / #196443 均 ✅ 可干净应用。

---

_Author: wangguanran_