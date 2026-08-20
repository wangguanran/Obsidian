# MC5617 低温启动充电管理架构

## 架构分层

```
XBL (BOOT.MXF.2.0)              BattMan (ADSP.HT.5.7)
pm_config_target_sbl_sequence.h  pm_peripheral_chgr.c
   PSI Sequence LUT                 JEITA 配置接口
        ↓                              ↓
   PMIC 寄存器                      SCHG_CHGR JEITA_EN_CFG (0x1090)
   (SBL 阶段写入)                   (运行阶段控制)
```

## 关键寄存器

| 寄存器 | 地址 | 作用 |
|--------|------|------|
| SCHG_CHGR JEITA_EN_CFG | 0x1090 | JEITA 温度保护使能配置（AFP_COLD/HOT） |

## JEITA 与低温启动

- JEITA（Japan Electronics and Information Technology Industries Association）温度保护：低温/高温下限制或禁止充电
- AFP（Automatic Feature Protection）COLD/HOT：硬件保护触发后会 hard-limit，阻断充电，进而影响冷启动
- MC5617 产品要求：-15°C（热敏电阻 846.6kΩ）时样机必须能开机，因此 SBL 阶段先写 0x1090=0x00 关 JEITA，BattMan 运行阶段也强制 JEITA 全关

## 平台差异

- `pm_peripheral_chgr.c` 的 JEITA 接口在 `#if defined(ODM_PROJECT_MC5617)` 分支下直接 `PM_OUT` 清 0x1090，而非按位掩码
- PSI Sequence 表内直接追加 `PM_SBL_WRITE 0x1090 0x00 0xFF` 表项（宏内不能用 #if）

_Author: wangguanran_
