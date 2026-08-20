# CellWise CW221X XBL 电量计检测移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：CellWise CW221X 系列电量计（CW2215/CW2217 等）
- **平台**：SM6225-A13（divar），项目代号 HXB_SLM927_YunZhuoKeJi
- **功能**：电池电量计，I2C 通信，XBL 阶段识别型号

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| 通信接口 | I2C（XBL 阶段直接读芯片 ID） |
| 芯片识别 | 读 ID 寄存器与匹配表比对，区分 CW221X 变体 |

## 关键配置

### 芯片 ID 匹配表（MDPPlatformLib）

完善匹配表，覆盖实际使用的 CW221X 型号：

```c
// 完善芯片 ID 检测逻辑，增加对不同 CW221X 型号的准确识别
static const struct cw221x_id_match cw221x_ids[] = {
    /* CW2215 */ { .id = 0xXX, .name = "CW2215" },
    /* CW2217 */ { .id = 0xXX, .name = "CW2217" },
    ...
};
```

### 配置说明

- **DTS 配置**：无变更（XBL 阶段不依赖 DTS）
- **Kernel config**：无变更（XBL 使用独立编译配置）
- **BoardConfig**：无变更

## 编译与验证

```bash
# 编译 XBL（bootloader）镜像，确认编译通过
# 烧录 XBL + 完整固件，开机观察串口日志中电量计型号识别结果
# 预期：正确输出 CW221X 实际型号，电池信息读取正常
```

## 移植注意事项

- 若更换 CW221X 具体型号，需同步更新 ID 匹配表
- XBL 侧修改与内核侧电量计驱动独立，两侧识别结果需一致

## 引用文件索引

- [[01.驱动文档/Charger/CellWise/CW221X/SM6225-A13/04.问题案例/XBL阶段电量计类型检测修复.md|XBL阶段电量计类型检测修复]]（补丁内容）
- `BOOT.XF.*/boot_images/QcomPkg/.../MDPPlatformLib`（远程源码树）

---

_Author: wangguanran_
