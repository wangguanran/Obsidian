# SDE Display / parrot-sde 面板框架分析

> **版本号：v1.0**（基于 LA.VENDOR.1.0.R1 / Snapdragon_Premium_High_2021 parrot 平台）

## 1. 显示软件栈分层

```
应用 / SurfaceFlinger
        │
HWC（Hardware Composer）→ DRM/KMS
        │
SDE 内核驱动（msm/drm/sde）
        ├── DPU（Display Processing Unit）：图像处理/合成管线
        ├── DSI 控制器 + DSI PHY：链路物理层
        └── 面板框架（Panel Framework）：面板节点解析、初始化、ESD 检测、recovery
        │
DSI 命令/视频通路
        ▼
面板（r66451 AMOLED video / cmd 等）
```

关键点：

- **display-devicetree**（vendor/qcom/proprietary/display-devicetree/display/）集中管理 parrot 平台 SDE 显示 DTS：`parrot-sde-display-common.dtsi`（公共显示配置+面板节点）、各面板 dsi dtsi、`*-sde-display-idp.dtsi`（板级引用）；
- 面板节点（如 `&dsi_r66451_amoled_video`）定义时钟选择、初始化序列（timing@）、**ESD 检测属性**、显示拓扑（`qcom,display-topology`）等；
- 内核侧 SDE 驱动按 DTS 属性注册面板与显示链路。

## 2. 面板框架：ESD 检测机制（本平台案例核心）

| 属性 | 作用 |
|:---|:---|
| `qcom,esd-check-enabled` | 使能周期 ESD 检查（面板级） |
| `qcom,mdss-dsi-panel-status-check-mode` | 检测方式：`reg_read` / `irq_pulse` 等 |
| `qcom,mdss-dsi-panel-status-command` | DCS 读命令（如读 0x0A 寄存器） |
| `qcom,mdss-dsi-panel-status-command-state` | 命令发送模式（`dsi_lp_mode`） |
| `qcom,mdss-dsi-panel-status-value` | 期望读回值比对 |
| `qcom,mdss-dsi-panel-status-read-length` | 读取长度 |

流程：定时器 → 下发 status 命令读寄存器 → 比对 `status-value` → 不一致 → 判定 ESD → **panel recovery**（重置 DSI 链路 + 重发初始化序列）→ 显示恢复。

> 当检测阈值与面板实际行为不匹配时会**误触发** recovery，表现为周期性闪屏/显示异常——即 Change #196185 的根因（详见问题案例文档）。

## 3. 面板模式

- **video 模式**：DSI 连续视频流传输，无 TE 回帧信号辅助（本平台 r66451 video 面板所属）；
- **command（cmd）模式**：按帧写入，依赖 TE 信号，支持局部刷新。

## 4. 相关代码位置

| 路径 | 说明 |
|:---|:---|
| `vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi` | parrot 显示公共配置 + 面板节点（归档：[[01.驱动文档/LCD/Qualcomm/MC5616/SM4490-A16/91.源码与补丁索引/dt_config/parrot-sde-display-common.dtsi\|parrot-sde-display-common.dtsi]]） |
| `vendor/qcom/proprietary/display-devicetree/display/dsi-panel-*.dtsi` | 各面板初始化序列/属性 |
| `msm/drm/sde/`（kernel） | SDE 驱动：DPU、DSI、panel（esd 检测实现） |
| UEFI（BOOT.XF.4.1） | XBL 阶段显示初始化（面板 xml，见同芯片其他平台 UEFI 案例） |

_Author: wangguanran_