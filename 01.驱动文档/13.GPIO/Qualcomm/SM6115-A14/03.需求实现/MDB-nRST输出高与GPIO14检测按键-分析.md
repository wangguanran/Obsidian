# 分析：MDB-nRST输出高与GPIO14检测按键

**版本号：v1.0**
**对应文档：** MDB-nRST输出高与GPIO14检测按键.md

## 技术背景

MT5205 主板上 MDB 小板以 STM32F103 作为主控，其 nRST（低有效复位）由 AP 侧 GPIO36（mdb_reset）控制。MDB 小板在主板上无上拉电阻，若 AP 不主动驱动高电平，nRST 浮空将导致主控复位状态不确定，因此 pinctrl 必须 `output-high` 钳位。

DB（Dispenser/外设）检测通过 GPIO14 实现：DB 插入时拉低 GPIO14，AP 需以 gpio-keys 方式上报按键事件（KEY_F1），支持 wakeup。此前 GPIO14 被 scuba pinctrl 的 reserved 列表占用，无法作为普通 GPIO 使用，必须先解除保留。

## 代码改动分析（逐文件）

### 1. drivers/pinctrl/qcom/pinctrl-scuba.c（+2/-2）

`scuba_reserved_gpios[]` 从 `0, 1, 2, 3, 14, 15, -1` 改为 `0, 1, 2, 3, 15, -1`：

- 解除 GPIO14 保留，允许 gpio-keys / gpiod 请求；
- GPIO15 维持保留（SE5 相关预留）；
- 注释同步更新：16/17 供 MDB UART，14 释放供 MDB DET。

### 2. devicetree/qcom/scuba-iot-idp-overlay.dts（+39/-2）

- **注释更新**：GPIO36 MDB_RESET 说明改为"idle output-high"，并注明 Assert nRST 方式（echo 0 > value）。
- **mt5205_mdb_reset 节点**：保持 `output-high` 配置不变（该配置由更早的 gpio-userspace 引入，#195886 仅补充注释说明"无板级上拉"的根因）。
- **新增 mt5205_mdb_db_detect pinctrl**：GPIO14 复用为 gpio 功能，drive-strength 2，bias-pull-up（空闲高），input-enable。
- **新增 mdb_db_keys 节点**：gpio-keys 驱动，label "mdb-db-keys"，`gpios = <&tlmm 14 GPIO_ACTIVE_LOW>`（低有效：插入拉低 → 触发），`linux,code = <KEY_F1>`，debounce-interval 15ms，`gpio-key,wakeup` 支持唤醒。

注释特别说明：scuba.dtsi 里已有 gpio_keys 节点但无 phandle，无法在其上扩展，故新建独立节点。

## 潜在风险

- GPIO14 释放后若有其他驱动隐式请求（如早期 MDB SE5 UART 配置残留），会与 gpio-keys 冲突；当前 reserved 列表已明确移除，需在 bringup 时核对无重复 request。
- KEY_F1 为功能键，用户态需有对应监听（如 input 服务）；若上层无消费，事件仅体现在 getevent。
- GPIO14 低有效 + pull-up：若 DB 小板内部对地阻抗设计不当，可能触发误按键，需实测 debounce。

## 回归测试建议

1. 冷启动后 `cat /sys/kernel/debug/gpio` 确认 GPIO14 方向为输入、GPIO36 输出高。
2. 反复插拔 DB 小板 100 次，getevent 每次均应收到且仅收到一次 KEY_F1。
3. 休眠后插拔 DB，确认可唤醒（gpio-key,wakeup 生效）。
4. MDB 通信功能回归：复位释放后 MDB 正常应答。

## 与现有驱动架构的关系

- 与 #195883（UIC Pulse）作用于同一 overlay 文件，两个变更叠加为最终版本（归档为 scuba-iot-idp-overlay.dts）；
- MDB_RESET 的 default-high 能力由更早的 gpio-userspace 驱动（SE_RESET/MDB_RESET）提供，本变更修正其空闲电平语义并补充 DB 检测；
- 属纯配置类改动（pinctrl 数据 + DT），不涉及内核驱动代码逻辑。

---

_Author: wangguanran_
