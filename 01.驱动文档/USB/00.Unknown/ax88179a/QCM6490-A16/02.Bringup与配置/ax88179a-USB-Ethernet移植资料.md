# ASIX ax88179a USB Ethernet 驱动移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：ASIX ax88179a（USB 3.0/2.0 Gigabit Ethernet 控制器）
- **平台**：QCM6490（Rigel）/ Android 16
- **支持型号**：ax88179a、ax88179_178a（含 PTP 时间同步）

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| 接口 | USB（连接 USB Host 控制器） |
| 网络 | 千兆以太网（内部 MAC+PHY） |
| PTP | IEEE 1588 时间同步 |

## 关键配置

### Kconfig 挂载

在 `drivers/net/Kconfig` 中增加：

```kconfig
source "$(KCONFIG_EXT_PREFIX)drivers/net/usb/Kconfig"
```

### defconfig 使能

```kconfig
CONFIG_USB_NET_AX_USB_NIC=m
```

（与 `CONFIG_USB_LAN78XX=m` 等并列）

### common 侧排除通用驱动（#196763）

在 kernel common 侧排除通用 USB Ethernet 驱动绑定，避免与 ax_usb_nic 竞争绑定。

## 编译与验证

```bash
# 全量编译 kernel（含模块）
# 确认生成 ax_usb_nic.ko

# 插入 USB 网卡，确认识别
adb shell lsusb        # 应看到 0b95:1790 (ASIX)
adb shell ifconfig     # 应出现 ethX 网络接口

# 网络连通与 PTP 验证
adb shell ping <网关>
```

## 移植注意事项

- 驱动为 ASIX 官方代码，需确认授权与版本
- common 侧排除绑定（#196763）必须一并合入，否则通用驱动抢先绑定导致 ax_usb_nic 不生效

## 引用文件索引

- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/03.需求实现/ax88179a-USB-Ethernet驱动移植.md|ax88179a-USB-Ethernet驱动移植]]（补丁内容）
- [[01.驱动文档/USB/00.Unknown/ax88179a/QCM6490-A16/91.源码与补丁索引/patches/|patches]]（补丁索引）

---

_Author: wangguanran_
