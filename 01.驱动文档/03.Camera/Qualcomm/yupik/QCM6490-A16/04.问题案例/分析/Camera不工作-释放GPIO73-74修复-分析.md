# 分析：Camera 不工作修复：释放 GPIO73/74（CCI2）

**版本号：v1.0**
**对应文档：** Camera不工作-释放GPIO73-74修复.md

## 技术背景

Qualcomm Camera 框架中，CCI（Camera Control Interface，I2C 主控制器）负责与 sensor/镜头/闪光灯等外设通信。QCM6490 (yupik) 平台提供多条 CCI 总线（CCI1/CCI2/CCI3...），每条 CCI 由一对 I2C 引脚（DATA/CLK）承载，引脚经 TLMM pinctrl 配置为 CCI 功能。

设备树中每个 CCI 控制器节点通过以下属性声明引脚使用：

- `pinctrl-0/pinctrl-1`：active/suspend 状态下启用的 pinctrl 状态（指向 `cciN_active` / `cciN_suspend` 引脚组）
- `gpios`：直接请求的 GPIO 列表（`<&tlmm N 0>`）
- `gpio-req-tbl-*`：GPIO 请求表（序号、标志、标签），对应 `gpios` 顺序

GPIO 请求由 camera devicetree 解析（`qcom-camera-cci` 驱动 / camera probe 阶段）执行，被请求的 GPIO 会通过 gpiolib 占用，重复/冲突占用会导致 `gpio_request` 失败或 xx 操作异常，最终表现为 camera 不工作。

## 代码改动分析

修改文件：`937/pvt/yupik-camera-937-pvt.dtsi`（+16/-10）

| 改动点 | 修改前 | 修改后 | 影响 |
|:-------|:-------|:-------|:-----|
| pinctrl-0 | `<&cci2_active &cci3_active>` | `<&cci3_active>` | CCI2 引脚组不再启用 |
| pinctrl-1 | `<&cci2_suspend &cci3_suspend>` | `<&cci3_suspend>` | CCI2 挂起态同步移除 |
| gpios | tlmm 73/74/75/76 | tlmm 75/76 | 释放 GPIO73/74 |
| gpio-req-tbl-num | 0 1 2 3 | 0 1 | 请求表从 4 项减为 2 项 |
| gpio-req-tbl-label | DATA2/CLK2/DATA3/CLK3 | DATA3/CLK3 | 移除 CCI2 标签 |
| 注释 | 无 | `//pinctrl-0 = <&cci3_active>;...` | 说明 cam_default 为 active 状态（转为注释，避免误会） |

关键逻辑：CCI1 节点不再占用 CCI2 的 GPIO73/74，仅保留 CCI3 的 GPIO75/76。GPIO73/74 释放后可供其他功能复用。

## 潜在风险

1. **GPIO75/76 依赖**：若产品实际还有第二路 CCI（挂 sensor 在 CCI2），此修改会导致 CCI2 无引脚可用——需确认摄像头只走 CCI3 单路
2. **pinctrl 引用完整性**：`cci2_active/cci2_suspend` 引脚组定义本身仍保留在 dtsi 中（未被删除），无悬空引用
3. **其他节点引用**：需确认没有其他节点（如 `cci2` 控制器）还引用 GPIO73/74，否则变向冲突
4. **GPIO 复用分配**：释放的 GPIO73/74 若被其他外设（如按键/传感器）申请，需在对应节点补配置

## 回归测试建议

- Camera 基础功能：前后摄预览、拍照、录像
- `dmesg | grep -iE "cci|gpio"` 无冲突/失败日志
- GPIO 占用状态检查：`cat /sys/kernel/debug/gpio` 确认 gpio73/74 不与 camera 关联
- 若 GPIO73/74 分配给新外设，验证新外设功能正常

## 与现有驱动架构的关系

- 本修改属于 camera-devicetree 仓库（`meigla/platform/vendor/opensource/camera-devicetree`），与 kernel 内的 `drivers/media/platform/qcom/camera` 配合
- 同类问题在 MC937 项目由 Rigel 平台（QCM6490）设计 -> 若其他 yupik 系产品使用相同 base，需检查是否有同样的 CCI2/CCI3 误占用
- 与 [[01.驱动文档/03.Camera/Qualcomm/yupik/QCM6490-A16/01.原理与架构/Camera-CCI驱动架构分析.md|Camera-CCI驱动架构分析]] 中的 CCI 引脚映射表关联

_Author: wangguanran_