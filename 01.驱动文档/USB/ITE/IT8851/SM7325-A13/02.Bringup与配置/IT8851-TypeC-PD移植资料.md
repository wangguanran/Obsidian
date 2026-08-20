# ITE IT8851 Type-C/PD 控制器移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：ITE IT8851（Type-C/PD 控制器）
- **平台**：SM7325-A13（yupik），项目代号 MT912 / RIG5EM-3701
- **驱动**：`kernel/msm-5.4/drivers/meig-tools/it8851.c`

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| Type-C | CC 检测、PD 协商 |
| 电源 | VBUS/VCONN 控制 |
| 屏供电 | CFD 屏应用中承担屏供电控制 |
| 通信 | I2C（连接 SM7325 SoC） |

## 关键配置

- **DTS 配置**：无变更（该修复完全在驱动代码层面）
- **Kernel config**：无变更（CONFIG 已包含 it8851 驱动）
- **BoardConfig**：无变更

## 驱动修改（suspend/resume 复位逻辑）

`it8851.c` 关键函数中增加 12 行复位/初始化逻辑（+12/−0），确保 suspend/resume 后 IT8851 正确恢复工作状态：

```c
// 新增 12 行复位/初始化逻辑，确保 suspend/resume 后 IT8851 正常工作
// resume 阶段重新初始化芯片寄存器
```

## 编译与验证

```bash
# 编译 kernel（含 it8851 驱动）
mmm kernel/msm-5.4  # 或模块编译

# 功能验证
# 1. 插入 USB/Type-C 设备，确认 PD 协商正常
# 2. 休眠后按电源键唤醒，观察屏是否正常亮起
# 3. 反复休眠唤醒 10+ 次，确认无偶发不亮
```

## 移植注意事项

- 若目标平台 CFD 屏供电不经过 it8851，本复位逻辑不适用
- 修改为驱动层修复，不影响 DTS 与其他配置

## 引用文件索引

- [[01.驱动文档/USB/ITE/IT8851/SM7325-A13/04.问题案例/CFD屏休眠唤醒不亮.md|CFD屏休眠唤醒不亮]]（补丁内容）
- `kernel/msm-5.4/drivers/meig-tools/it8851.c`（远程）

---

_Author: wangguanran_
