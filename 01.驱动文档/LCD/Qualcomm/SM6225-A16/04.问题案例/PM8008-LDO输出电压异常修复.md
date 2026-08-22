# PM8008 LDO 输出电压异常修复（UEFI 寄存器写入顺序）

> **模块**: LCD | **厂商**: Qualcomm | **芯片**: SM6225 (Kamorta)
> **平台**: SM6225-A16 (iot-high-mid-2024-spf-3-0) | **类型**: Bug
> **Change**: #196388 | **作者**: [同事] | **状态**: MERGED

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196388 |
| 项目 | iot-high-mid-2024-spf-3-0_amss_standard_oem |
| 分支 | master_meig |
| 作者 | [同事]（提交标题署名 shenbaotao） |
| 类型 | Bug（PM8008 LDO5/6/7 输出电压异常） |
| 芯片 | Qualcomm SM6225 (Kamorta) |
| 平台 | SM6225-A16（项目代号 HXB_[项目代号]_TianBo） |
| 模块 | LCD（UEFI MDPPlatformLib 面板电源初始化） |
| 提交标题 | `[HXB_[项目代号]_TianBo][TaskID][Description]PM8008 ldo7 Output voltage abnormal[Solution]Change the order of writing registers[Owner]shenbaotao` |

## 现象

UEFI 阶段初始化显示面板电源时，PM8008 的 LDO5/LDO6/LDO7 输出电压异常（表现为面板供电电压不对，可能引起上电时序或显示异常）。问题定位为 PM8008 LDO 电压配置寄存器的**写入顺序**错误。

## 环境

| 项 | 内容 |
|----|------|
| UEFI 源码 | `BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c` |
| 芯片 | SM6225 (Kamorta) |
| PMIC | PM8008（I2C SID，寄存器 0x40~0x46 段） |
| 面板电源 | LDO5 = 2.8V、LDO6 = 3.0V、LDO7 = 1.8V（初始化序列数组） |

## 关键代码（修复前）

```c
uint8 L5I_set_2P8_1[] = {0x44, 0x41, 0x0B};   /* 先写 VSET_UB(0x41) */
uint8 L5I_set_2P8_2[] = {0x44, 0x40, 0x28};   /* 后写 VSET_LB(0x40) */
uint8 L6I_set_3P0_1[] = {0x45, 0x41, 0x0A};
uint8 L6I_set_3P0_2[] = {0x45, 0x40, 0x90};
uint8 L7I_set_1P8_1[] = {0x46, 0x41, 0x07};
uint8 L7I_set_1P8_2[] = {0x46, 0x40, 0x08};
```

## 根因分析

PM8008 的 LDO 输出电压由 16-bit VSET 决定，跨两个寄存器：

- `VSET_LB`（0x40 / 0x45 / 0x46 的低字节寄存器，偏移 +0x00）
- `VSET_UB`（高字节寄存器，偏移 +0x01）

**PM8008 在写入 VSET_UB 时锁存 16 位电压值**。原实现先写 `0x41`（UB）再写 `0x40`（LB）：

1. 第一次写 UB：此时 LB 还是旧值/默认值，芯片立即锁存一个"UB×256 + 旧LB"的错误电压；
2. 第二次写 LB：虽然 LB 更新了，但锁存发生在 UB 写入时，本次 LB 写不会触发重新锁存（或触发但 UB 已是最新，取决于实现），最终 LDO 输出不是目标电压。

修复方式：**先写 LB（0x40）再写 UB（0x41）**，确保 UB 写入锁存时 LB 已是目标值，一次锁存即得到正确电压。

## 处理方案

交换 LDO5/LDO6/LDO7 的 VSET 写入顺序（LB 先行，UB 后行），并补充注释说明锁存机制：

```c
/* VSET: write LB (0x40) first, then UB (0x41). PM8008 latches 16-bit VSET on VSET_UB write. */
uint8 L5I_set_2P8_1[] = {0x44, 0x40, 0x28};   /* LB = 0x28 */
uint8 L5I_set_2P8_2[] = {0x44, 0x41, 0x0B};   /* UB = 0x0B → 2.8V */
uint8 L6I_set_3P0_1[] = {0x45, 0x40, 0x90};   /* LB = 0x90 */
uint8 L6I_set_3P0_2[] = {0x45, 0x41, 0x0A};   /* UB = 0x0A → 3.0V */
uint8 L7I_set_1P8_1[] = {0x46, 0x40, 0x08};   /* LB = 0x08 */
uint8 L7I_set_1P8_2[] = {0x46, 0x41, 0x07};   /* UB = 0x07 → 1.8V */
```

## 修改文件清单

| 文件 | 改动 | 说明 |
|------|------|------|
| [[01.驱动文档/LCD/Qualcomm/SM6225-A16/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c\|MDPPlatformLibPanelCommon.c]] | +7/-6 | LDO5/6/7 VSET 写入顺序调整（LB→UB）+ 注释 |

## 配置方式

无新增配置；电压值本身不变（LDO5=2.8V、LDO6=3.0V、LDO7=1.8V），仅调整写入顺序。I2C 命令格式：`{SID_偏移, 寄存器地址, 数据}`，该文件内通过 `pm_i2c_sid_config` 依次下发。

## 验证方式

- 编译验证：UEFI（BOOT.XF.4.1）全量编译通过（本 Change 已合入，CI Verified +1）。
- 实机验证（建议）：
  - UEFI 日志/串口确认 `pm_i2c_sid_config start` 后各 LDO 使能正常；
  - 万用表测量 PM8008 LDO5/LDO6/LDO7 输出，确认分别为 2.8V / 3.0V / 1.8V；
  - 面板上电后正常点亮，无电压异常导致的闪屏/黑屏。

## 结论

PM8008 16-bit VSET 在写 UB 时锁存，必须先写 LB 再写 UB。原顺序颠倒导致 LDO5/6/7 输出非目标电压，交换顺序后修复。同类 PMIC（锁存型 16 位 VSET）的初始化序列都应遵守 LB→UB 顺序。

## 补丁内容

```diff
Subject: [PATCH] [HXB_[项目代号]_TianBo][TaskID][Description]PM8008 ldo7 Output voltage abnormal[Solution]Change the order of writing registers[Owner]shenbaotao

---

diff --git a/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c b/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c
index 692bfee..47c8cd4 100755
--- a/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c
+++ b/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c
@@ -71,16 +71,17 @@
 	uint8 L2I_enable[] =    {0x41, 0x46, 0x80};
 	uint8 L3I_enable[] =    {0x42, 0x46, 0x80};
 	uint8 L4I_enable[] =    {0x43, 0x46, 0x80};
-	uint8 L5I_set_2P8_1[] = {0x44, 0x41, 0x0B};
-	uint8 L5I_set_2P8_2[] = {0x44, 0x40, 0x28};
+	/* VSET: write LB (0x40) first, then UB (0x41). PM8008 latches 16-bit VSET on VSET_UB write. */
+	uint8 L5I_set_2P8_1[] = {0x44, 0x40, 0x28};
+	uint8 L5I_set_2P8_2[] = {0x44, 0x41, 0x0B};
 	uint8 L5I_set_2P8_3[] = {0x44, 0x45, 0x07};
 	uint8 L5I_enable[] =    {0x44, 0x46, 0x80};
-	uint8 L6I_set_3P0_1[] = {0x45, 0x41, 0x0A};
-	uint8 L6I_set_3P0_2[] = {0x45, 0x40, 0x90};
+	uint8 L6I_set_3P0_1[] = {0x45, 0x40, 0x90};
+	uint8 L6I_set_3P0_2[] = {0x45, 0x41, 0x0A};
 	uint8 L6I_set_3P0_3[] = {0x45, 0x45, 0x07};
 	uint8 L6I_enable[] =    {0x45, 0x46, 0x80};
-	uint8 L7I_set_1P8_1[] = {0x46, 0x41, 0x07};
-	uint8 L7I_set_1P8_2[] = {0x46, 0x40, 0x08};
+	uint8 L7I_set_1P8_1[] = {0x46, 0x40, 0x08};
+	uint8 L7I_set_1P8_2[] = {0x46, 0x41, 0x07};
 	uint8 L7I_set_1P8_3[] = {0x46, 0x45, 0x07};
 	uint8 L7I_enable[] =    {0x46, 0x46, 0x80};
 	DEBUG ((EFI_D_WARN, "pm_i2c_sid_config start\n"));
```

## 补丁验证

- 验证方式：134 服务器 REST 拉取补丁，对合并后源文件执行 `git apply --check -R`（反向应用校验）
- 结果：✅ 可干净应用（补丁与已合入提交 diff 一致）

## 源码归档

| 内容 | 路径 | 说明 |
|------|------|------|
| kernel_driver/ | [[01.驱动文档/LCD/Qualcomm/SM6225-A16/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c\|MDPPlatformLibPanelCommon.c]] | UEFI 面板初始化库（补丁后合并版本） |
| patches/ | [[01.驱动文档/LCD/Qualcomm/SM6225-A16/91.源码与补丁索引/patches/196388.patch\|196388.patch]] | 完整补丁（已清隐私） |

## 引用文件索引

| 文件 | 完整路径 | 说明 |
|------|---------|------|
| [[01.驱动文档/LCD/Qualcomm/SM6225-A16/91.源码与补丁索引/kernel_driver/BOOT.XF.4.1/boot_images/QcomPkg/SocPkg/KamortaPkg/Library/MDPPlatformLib/MDPPlatformLibPanelCommon.c\|MDPPlatformLibPanelCommon.c]] | BOOT.XF.4.1/.../KamortaPkg/Library/MDPPlatformLib/ | UEFI 面板电源/初始化公共库 |
| [[01.驱动文档/LCD/Qualcomm/SM6225-A16/91.源码与补丁索引/patches/196388.patch\|196388.patch]] | 91.源码与补丁索引/patches/ | 补丁（已清隐私） |

_Author: wangguanran_
