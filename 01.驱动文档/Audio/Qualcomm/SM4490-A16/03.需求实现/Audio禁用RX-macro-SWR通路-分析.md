# [项目代号] Audio 禁用 RX macro/SWR 通路降低功耗 — 分析

## 变更概要

| 项目 | 内容 |
|------|------|
| Change | #196025 |
| 标题 | [[项目代号]][93821][Audio] Disable RX macro/SWR path, drop VA always-on |
| 作者 | [同事] |
| 类型 | 需求 (power optimization) |
| 芯片 | [项目代号] (parrot/QCM4490) |
| 平台 | SM4490-A16 |

## 根因分析

[项目代号] 板硬件设计上未使用 LPASS RX SWR 通路，具体表现为：

- **无 WCD RX 器件**：未外接 WCD 系列音频编解码器
- **无 Awinic I2C PA**：未使用 Awinic 系列 I2C 功放

但软件配置中以下组件仍按默认使能，导致不必要的功耗开销：

### 1. RX macro 及 RX SWR master 无意义使能

```dts
// 修改前：默认使能
&rx_macro {
    status = "okay";  // 无实际硬件连接，不应 probe
};
&swr3 {
    status = "okay";  // RX SWR master，无从设备
};
```

probe 阶段执行无意义的操作：
- SWR 总线初始化与枚举
- `add_child_devices` work 调度
- 增加不必要的启动延迟和静态功耗

### 2. VA SWR master 阻止 codeccore 释放

```dts
// 修改前：always-on 阻止电源管理
&swr0 {
    qcom,irq-wakeup-capable = <1>;  // 保持唤醒
    qcom,is-always-on = <1>;        // 持续供电
};
```

`always-on` 和 `irq-wakeup` 属性导致 codeccore 驱动无法进入低功耗状态，即使 VA 通路未在使用中。

### 3. 无效音频路由残留

```dts
// 修改前：存在 RX_TX DEC 路由（实际无 RX 通路可用）
rx-tx-dec;  // 无效路由配置
```

## 解决方案

### DTS 修改

**parrot-audio-overlay.dtsi**（+7/−7）：

1. 禁用 RX macro 节点：
   ```dts
   &rx_macro {
       status = "disabled";
   };
   ```

2. 禁用 RX SWR master（swr3）：
   ```dts
   &swr3 {
       status = "disabled";
   };
   ```

3. 调整 macro 数量（3 → 2）：
   ```dts
   &lpass_cdc {
       qcom,num-macros = <2>;
   };
   ```

4. 清除 VA SWR master 的 always-on 属性：
   ```dts
   &swr0 {
       qcom,irq-wakeup-capable = <0>;
       qcom,is-always-on = <0>;
   };
   ```

**parrot-audio-qrd.dtsi**（+0/−4）：

- 删除无效的 RX_TX DEC 音频路由

### Kernel 驱动修改

**lpass-cdc-rx-macro.c**（+18/−13）：

- 修改 RX macro 驱动，使其在 disabled 状态下跳过初始化

**waipio.c**（+8/−2）：

- 用 `#if 0` 编译屏蔽 RX CDC DMA DAI link 的定义

## 功耗收益分析

| 项目 | 修改前 | 修改后 |
|------|--------|--------|
| RX macro probe | 执行（无意义） | 跳过 |
| RX SWR 初始化 | 执行（无意义） | 跳过 |
| VA SWR always-on | 是 | 否 |
| codeccore 电源管理 | 无法释放 | 正常释放 |
| 无效音频路由 | 存在 | 清理 |

## 影响范围

- **RX 通路**：完全禁用，不影响 [项目代号] 板功能（无 RX 硬件）
- **VA 通路**：功能保留，仅移除 always-on 限制，由正常电源管理策略控制
- **TX 通路**：不受影响
- **其他平台**：仅影响 [项目代号]（parrot），不涉及其他 parrot 衍生板型

## 注意事项

- 若后续硬件变更需启用 RX 通路，需反向恢复上述 DTS 和驱动修改
- 禁用 RX macro 后，需确保 `num-macros` 计数与使能的 macro 数量一致，否则驱动 probe 可能失败
- VA SWR master 的 `is-always-on` 清除后，需验证 VA 通路在需要时能正常唤醒

## 参考文档

- 主文档：[[01.驱动文档/Audio/Qualcomm/SM4490-A16/03.需求实现/Audio禁用RX-macro-SWR通路|[项目代号] Audio 禁用 RX macro/SWR 通路降低功耗]]
- 关联模块：[[01.驱动文档/Audio/Qualcomm/|Qualcomm Audio 驱动文档]]

---

_Author: wangguanran_