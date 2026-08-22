# 禁用 r66451 AMOLED 面板 ESD 检查

> **模块**: LCD | **厂商**: Qualcomm | **芯片**: SM4490 (parrot/QCM4490)
> **平台**: SM4490-A16 (LA.VENDOR.1.0.R1) | **类型**: Bug
> **Change**: #196185 | **作者**: [同事] | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196185 |
| 项目 | LA.VENDOR.1.0.R1 |
| 分支 | master_Snapdragon_Premium_High_2021.SPF.2.0.2_MC5616 |
| 作者 | [同事] |
| 类型 | Bug（显示异常） |
| 芯片 | SM4490（parrot/QCM4490） |
| 平台 | SM4490-A16 |
| 模块 | LCD 显示（display-devicetree） |
| BugID | 96455 |

## 现象

MC5616 平台使用 r66451 AMOLED video 面板，开机及使用过程中出现**屏闪烁/显示异常**。排查为面板 ESD 检测误触发所致：

- 驱动按 `qcom,esd-check-enabled` 使能 ESD 检查后，会按 `panel-status-command` 周期性读取面板 status 寄存器；
- r66451 video 面板在特定时序/状态下读回的 status 值与预期不符，触发 ESD 误判 → 面板被强制做 panel recovery（重启显示链路），表现为闪屏、显示异常。

## 环境

- 平台：SM4490-A16（Qualcomm parrot/QCM4490）
- 项目：LA.VENDOR.1.0.R1（MC5616，Snapdragon Premium High 2021）
- 面板：r66451 AMOLED video panel
- 修改文件：`vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi`

## 关键日志

（无保留原始日志，现象以 BugID 96455 复现记录为准：面板 ESD 误检触发 recovery，表现为周期性闪屏。）

## 调用链

```
显示驱动（SDE）定时器
   → 读取 dsibase 寄存器 / 下发 panel-status-command（reg_read 0x0A）
   → status 值 != qcom,mdss-dsi-panel-status-value (<0x1c>)
   → ESD 误判 → 触发 panel recovery（显示链路重置）
   → 屏幕闪烁/显示异常
```

## 根因分析

`parrot-sde-display-common.dtsi` 中 `&dsi_r66451_amoled_video` 面板节点**默认使能**了 `qcom,esd-check-enabled` 属性，并配置了：

```dts
qcom,mdss-dsi-panel-status-check-mode = "reg_read";
qcom,mdss-dsi-panel-status-command = [06 01 00 01 00 00 01 0a];   /* 读 0x0A 寄存器 */
qcom,mdss-dsi-panel-status-command-state = "dsi_lp_mode";
qcom,mdss-dsi-panel-status-value = <0x1c>;
qcom,mdss-dsi-panel-status-read-length = <1>;
```

即驱动会周期性通过 DSI 命令读面板 0x0A 寄存器并比对 `0x1c`。r66451 **video 模式**面板在部分状态下读回的寄存器值不符合预期（video 面板命令通道时序/面板固件响应差异），导致 ESD 检测**误触发**，驱动误以为面板异常而执行 panel recovery，最终表现为屏闪/显示异常。

该面板本身硬件正常（非真实 ESD 事件），属于**检测机制与面板匹配问题**，而非面板故障。

## 处理方案

在 `parrot-sde-display-common.dtsi` 的 `&dsi_r66451_amoled_video` 节点中**注释掉 `qcom,esd-check-enabled`**，禁用该面板的 ESD 检查，避免误检触发 recovery；**保留** status 相关属性（check-mode/status-command/value 等），便于后续如需恢复 ESD 检查可直接取消注释。

```dts
&dsi_r66451_amoled_video {
	qcom,dsi-select-clocks = "pll_byte_clk0", "pll_dsi_clk0";

	/* qcom,esd-check-enabled; */
	qcom,mdss-dsi-panel-status-check-mode = "reg_read";
	qcom,mdss-dsi-panel-status-command = [06 01 00 01 00 00 01 0a];
	qcom,mdss-dsi-panel-status-command-state = "dsi_lp_mode";
	qcom,mdss-dsi-panel-status-value = <0x1c>;
	qcom,mdss-dsi-panel-status-read-length = <1>;
	...
};
```

改动量为 +1/−1（一行注释）。

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/LCD/Qualcomm/MC5616/SM4490-A16/91.源码与补丁索引/dt_config/parrot-sde-display-common.dtsi\|parrot-sde-display-common.dtsi]] | +1/−1 | `&dsi_r66451_amoled_video` 节点中注释 `qcom,esd-check-enabled` |

## 配置方式

DTS 节点片段（合并后形态）：

```dts
&dsi_r66451_amoled_video {
	qcom,dsi-select-clocks = "pll_byte_clk0", "pll_dsi_clk0";

	/* qcom,esd-check-enabled; */          /* ← 本补丁注释掉，禁用 ESD 检查 */
	qcom,mdss-dsi-panel-status-check-mode = "reg_read";
	qcom,mdss-dsi-panel-status-command = [06 01 00 01 00 00 01 0a];
	qcom,mdss-dsi-panel-status-command-state = "dsi_lp_mode";
	qcom,mdss-dsi-panel-status-value = <0x1c>;
	qcom,mdss-dsi-panel-status-read-length = <1>;

	qcom,mdss-dsi-display-timings {
		timing@0 {
			qcom,mdss-dsi-panel-phy-timings = [00 1c 08 07 17 22 07
				07 08 02 04 00 19 0c];
			qcom,display-topology = <1 1 1>;
			qcom,default-topology-index = <0>;
		};
	};
};
```

说明：

- 注释掉 `qcom,esd-check-enabled` 后，SDE 驱动不再为 r66451 面板注册 ESD 检测定时器，不会周期性读 status 寄存器；
- **status 相关属性刻意保留**，后续若要恢复 ESD 检查，只需取消注释该行（status 属性无需改动，降低回归风险）。

## 验证方式

### 1. 长稳测试

```bash
# 长时间亮屏/灭屏循环、上电/断电循环
# 预期：全程无闪屏、无显示异常、无面板 recovery 日志
```

### 2. 日志验证

```bash
# 确认无 ESD 误检/panel recovery 触发
adb logcat -d | grep -iE "esd|panel.*recovery|drm_panel"
# 预期：无 esd check 误报、无 periodic panel recovery
```

### 3. 显示功能测试

- 开机进系统正常显示，无闪烁；
- 播放视频/图片、切屏、休眠唤醒各场景显示正常；
- 显示色彩、亮度无异常。

## 结论

r66451 AMOLED video 面板的 ESD 检查在 MC5616 平台误触发，导致驱动周期性执行 panel recovery 造成屏闪。通过在 DTS 节点中注释 `qcom,esd-check-enabled` 禁用该面板 ESD 检测（保留 status 属性便于复开），问题闭环。改动为单行配置，风险低、可回退。

## 补丁内容（完整粘贴，不截断）

```diff
Subject: [PATCH] [MC5616][96455][Display]Disable ESD check for r66451 AMOLED video panel [Owner][同事]

[Root Cause]
	&dsi_r66451_amoled_video 面板节点默认使能 qcom,esd-check-enabled，驱动会按 panel-status-command 周期性读取 status 寄存器并可能触发 panel recovery。
[Solution  ]
	注释掉该节点中的 qcom,esd-check-enabled，保留 status 相关属性便于后续复开。

---

diff --git a/vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi b/vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi
index 44a8651..16c8f07 100755
--- a/vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi
+++ b/vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi
@@ -377,7 +377,7 @@
 &dsi_r66451_amoled_video {
 	qcom,dsi-select-clocks = "pll_byte_clk0", "pll_dsi_clk0";
 
-	qcom,esd-check-enabled;
+	/* qcom,esd-check-enabled; */
 	qcom,mdss-dsi-panel-status-check-mode = "reg_read";
 	qcom,mdss-dsi-panel-status-command = [06 01 00 01 00 00 01 0a];
 	qcom,mdss-dsi-panel-status-command-state = "dsi_lp_mode";
```

## 补丁验证

| 验证方式 | 结果 |
|:---|:---|
| 134 源码树直接 git apply --check | ✅ 可干净应用 |

## 源码归档

已归档文件清单（91.源码与补丁索引）：

| 文件 | 归档位置 |
|------|------|
| parrot-sde-display-common.dtsi（合并后版本，895 行） | `91.源码与补丁索引/dt_config/parrot-sde-display-common.dtsi` |
| 196185.patch | `91.源码与补丁索引/patches/196185.patch` |
| modified_history.md | `91.源码与补丁索引/modified_history.md` |

> **来源路径说明**：该文件属 vendor 树（`vendor/qcom/proprietary/display-devicetree/display/`），按归档规则 `.dtsi` 归入 `dt_config/` 子目录（不放入 `vendor_hal/`）。

## 引用文件索引

- [[01.驱动文档/LCD/Qualcomm/MC5616/SM4490-A16/91.源码与补丁索引/dt_config/parrot-sde-display-common.dtsi|parrot-sde-display-common.dtsi]] — `vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi`，SDE 显示通用配置（含 r66451 AMOLED 面板节点）
- [[01.驱动文档/LCD/Qualcomm/MC5616/SM4490-A16/91.源码与补丁索引/patches/196185.patch|196185.patch]] — Change #196185 完整补丁
- [[01.驱动文档/LCD/Qualcomm/MC5616/SM4490-A16/91.源码与补丁索引/modified_history.md|modified_history.md]] — 本平台修改历史

_Author: wangguanran_