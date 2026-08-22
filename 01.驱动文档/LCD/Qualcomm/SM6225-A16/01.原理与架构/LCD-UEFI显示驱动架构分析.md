# LCD-UEFI 显示驱动架构分析

> **芯片**: Qualcomm SM6225 (Kamorta) | **平台**: SM6225-A16

## UEFI 显示初始化分层

```
ABL → UEFI DXE 阶段
   │
   ▼
MDPPlatformLib（KamortaPkg/Library/MDPPlatformLib）
   ├── MDPPlatformLibPanelCommon.c：面板公共初始化
   │     └── pm_i2c_sid_config：PM8008 I2C 配置序列下发
   │           ├── LDO5 VSET（2.8V）
   │           ├── LDO6 VSET（3.0V）
   │           └── LDO7 VSET（1.8V）
   ├── MDPPlatformLib.c：平台级 display 入口
   └── Panel 参数（timing/init data）
   │
   ▼
MDP 硬件（DPU）→ DSI → 面板
```

## PM8008 LDO 电压配置机制

- 电压 = 16-bit VSET（`VSET_LB` + `VSET_UB`），I2C 地址格式 `{SID, REG, DATA}`；
- **锁存语义**：写 `VSET_UB` 时锁存完整 16 位数值；
- 因此正确顺序必须 **LB 先写、UB 后写**，否则 UB 写入瞬间锁存到"UB+旧LB"的错误组合（[[01.驱动文档/LCD/Qualcomm/SM6225-A16/04.问题案例/PM8008-LDO输出电压异常修复.md|#196388 修复点]]）。

## 初始化调用链

```
MDPPlatformLib 平台库入口
  → 面板探测/选型（DSI 接口）
  → MDPPlatformLibPanelCommon 上电序列
      → pm_i2c_sid_config(LDO5/LDO6/LDO7 set+enable)
          （LB→UB 顺序写 VSET，再写 enable 0x46/0x80）
  → 面板 backlight/复位时序
  → DPU 输出使能 → Logo 显示
```

## 关键数据结构

| 数组 | 含义 |
|------|------|
| `L{n}I_set_*` | LDO n 电压设置序列（SID 偏移 + REG + DATA） |
| `L{n}I_enable` | LDO n 使能命令（写 0x80 至 enable 寄存器） |

---

_Author: wangguanran_
