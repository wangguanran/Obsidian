# [项目代号] SM4490-A16 显示（LCD）Bringup 与配置

> **版本号：v1.0**（持续补充）

## 1. 硬件接口

| 接口 | 说明 |
|:---|:---|
| DSI（4-lane） | 面板数据通路（video/cmd 模式） |
| DSI 时钟 | pll_byte_clk0 / pll_dsi_clk0（面板节点 qcom,dsi-select-clocks） |
| 电源 | 面板 VDDI/VCI/VDD 等（LDO/regulator 配置） |
| 背光 | PMIC WLED / I2C 背光驱动（按面板规格） |
| TE/复位 | cmd 模式 TE 信号、reset GPIO（按面板规格） |

> 具体引脚与电源分配以 [项目代号] 原理图为准。

## 2. 驱动/配置索引

| 路径 | 说明 |
|:---|:---|
| `vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi` | parrot 显示公共配置 + 面板节点（归档：[[01.驱动文档/LCD/Qualcomm/SM4490-A16/91.源码与补丁索引/dt_config/parrot-sde-display-common.dtsi\|parrot-sde-display-common.dtsi]]） |
| `vendor/qcom/proprietary/display-devicetree/display/dsi-panel-r66451*.dtsi` | r66451 面板初始化序列/属性定义 |
| `vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-idp.dtsi` | 板级显示引用/panel 选择 |
| `msm/drm/sde/`（kernel） | SDE 驱动（DPU/DSI/panel，含 ESD 检测实现） |
| UEFI Panel xml（BOOT.XF.4.1） | XBL 阶段面板初始化（AP 侧显示） |

## 3. 常用配置

### ESD 检测开关（面板节点）

```dts
&dsi_r66451_amoled_video {
    qcom,esd-check-enabled;        /* 使能（默认） */
    /* qcom,esd-check-enabled; */  /* 禁用（Change #196185 处理方式） */
};
```

### 面板时钟选择

```dts
qcom,dsi-select-clocks = "pll_byte_clk0", "pll_dsi_clk0";
```

## 4. 验证命令

```bash
# 显示链路状态（drm）
adb shell dumpsys SurfaceFlinger --display-id
adb shell dumpsys display

# 内核日志（面板 probe / ESD / recovery）
adb shell dmesg | grep -iE "sde|dsi|panel|esd"

# 亮屏/灭屏循环压力测试
for i in $(seq 1 100); do adb shell input keyevent KEYCODE_POWER; sleep 2; adb shell input keyevent KEYCODE_POWER; sleep 2; done

# 显示测试图/视频长稳
adb push test.mp4 /data/local/tmp/ && adb shell am start -a android.intent.action.VIEW -d file:///data/local/tmp/test.mp4 -t video/mp4
```

## 5. 修改历史

见 [[01.驱动文档/LCD/Qualcomm/SM4490-A16/91.源码与补丁索引/modified_history.md|modified_history.md]]。

_Author: wangguanran_