# 分析：SE SPI0 spidev 与 SE 握手 GPIO 使能

**版本号：v1.0**
**对应文档：** SE-SPI0-spidev与SE握手GPIO

## 技术背景

QUPv3（Qualcomm Universal Peripheral）SE0 是 AP 侧的一组复用串行引擎，可配置为 SPI/I2C/UART。在 Secure MCU（STM32U585）场景下，SE0 SPI（GPIO0~3）承担 AP 与安全芯片间的数据通道。

三道关卡需要同时打通：

1. **pinctrl 保留（scuba_reserved_gpios）**：平台模板默认保留 GPIO0~3（供 eSE/NFC 等用途），`pinctrl-scuba.c` 的保留列表决定这些引脚是否允许被普通驱动请求；
2. **TZ QUP 访问控制（QUPAC）**：TrustZone 侧的 `QUPv3_se_security_permissions_type` 表定义每个 SE 的协议、模式与归属（AC_TZ=仅 TZ 可用，AC_HLOS=HLOS 可用）。即使 DT 使能，若 TZ 不给权限，HLOS 访问会 bus error 或 timeout；
3. **spidev 用户态接口**：内核需编译 `CONFIG_SPI_SPIDEV`，并在 DT 中挂 compatible 匹配 spidev 白名单的 dummy 节点，用户态才能通过 `/dev/spidevX.Y` 访问。

## 代码改动分析

### bengal_GKI.config
- 新增 `CONFIG_SPI_SPIDEV=m`：使能 spidev 模块。注意 GKI 框架下该配置在 vendor 侧生效，需随 vendor_boot 打包。

### pinctrl-scuba.c
- `scuba_reserved_gpios[]` 从 `0,1,2,3,15,-1` 收窄到 `15,-1`。GPIO0~3 释放给 SE0 SPI；GPIO15 保持保留（MDB 用途预留）。释放后，SE0 SPI 的 `pinctrl-0`（qupv3_se0_spi 节点）才能正常请求引脚。

### scuba-iot-idp-overlay.dts
- 新增握手 GPIO pinctrl：`mt5205_se_ack`（GPIO37 输出低）、`mt5205_se_rdy`（GPIO63 输入高阻），并入 `gpio-userspace` 节点 pinctrl-0；
- `se-ack`/`se-rdy` 子节点：`default-state` 0（输出低）/2（输入 high-Z）；
- `&qupv3_se0_spi`：`status="ok"`、10MHz、`qcom,disable-autosuspend`（Secure MCU 通信不允许 runtime suspend 打断），`spidev@0` 使用 `qcom,spi-msm-codec-slave` 这个 spidev 白名单内的 compatible（否则 spidev 拒绝 probe）；
- 显式禁用 `qupv3_se0_i2c` 与 `qupv3_se0_4uart`：SE0 同一时刻只能一种协议，防止 DT 层面功能冲突。

### QUPAC_Access_MT5205.c（TZ 侧，新增 215 行）
- 复制自平台模板并做产品化裁剪，`ODM_PROJECT_MT5205` 条件编译：
  - SE0 → SPI FIFO AC_HLOS：核心改动，TZ 释放 SE0 SPI 给 HLOS；
  - SE1 → UART_4W（GPIO69/70）：本板 RS232 需求（非默认 I2C）；
  - SE5 → UART_4W（GPIO16/17）：MDB UART（非默认 SPI Fingerprint，且原为 AC_TZ）；
- 其他 SE 权限表（rumi/QRB/Genoa/2W/SKU2）原样保留，保证不同 SKU/仿真平台可用。

## 潜在风险

1. **TZ 权限与 DT 不一致**：QUPAC 表与 DT 必须配套；若 SE0 在 TZ 侧配成 AC_TZ 而 DT 使能，HLOS 访问会 hang；反之 TZ 释放但 DT 未使能无影响；
2. **SE0 协议互斥**：SPI 使能后 I2C/UART 功能永久让出（禁用节点），后续如需复用需回改；
3. **spidev 安全性**：`/dev/spidev0.0` 暴露给用户态，Secure MCU 场景需关注访问权限控制；
4. **autosuspend 禁用**：`qcom,disable-autosuspend` 会增加空闲功耗；
5. **后续修正**：merge 后确认 GPIO37/63 握手脚位并非本板所需（se_reset 为 GPIO102），se_ack/se_rdy 节点已从 overlay 移除（另行提交），SE0 SPI 通路不受影响。

## 回归测试建议

- 刷机后确认 `dmesg | grep spidev` 与 `/dev/spidev0.0` 存在；
- SE MCU 上电时序：se_reset 释放 → SPI 读写 1000 次无超时；
- 系统 suspend/resume 后 SPI 通信正常（验证 disable-autosuspend 无副作用）；
- MDB UART（SE5）与 RS232（SE1）回环测试，确认 TZ 侧配置未破坏已有功能。

## 与现有驱动架构的关系

- 与既有归档（UIC Pulse #195883、MDB nRST #195886）同处 `scuba-iot-idp-overlay.dts`，均为该平台 overlay 的增量演进；本单释放 GPIO0-3 后，后续 overlay 改动需在最新版本基础上进行；
- TZ 侧 QUPAC 与 AP 侧 DT 双改是本平台 Secure MCU 通信链路的标准组合，后续类似 SE 资源释放可复用此模式。

_Author: wangguanran_
