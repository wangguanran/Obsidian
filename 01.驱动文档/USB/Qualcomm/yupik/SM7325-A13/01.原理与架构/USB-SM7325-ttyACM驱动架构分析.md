# USB-SM7325 ttyACM 驱动架构分析

> **芯片**: Qualcomm SM7325 (yupik) | **平台**: SM7325-A13

## 分层结构

```
应用（open /dev/ttyACM11）
   │
   ▼
tty 层（tty_register_device → /dev/ttyACM*）
   │
   ▼
cdc_acm 驱动（probe → acm_alloc_minor 分配 idr 槽 → tty_port）
   │
   ▼
USB core（usbfs devio：/dev/bus/usb，USBDEVFS_DISCONNECT_CLAIM）
   │
   ▼
WCH CH343 设备（1a86:55da，接口 0/2）
```

## cdc_acm 生命周期

1. **probe**：`acm_alloc_minor()` 从全局 idr `acm_minors` 分配 minor（产品定制：WCH 固定 11/12、040b:a6b 固定 0）；
2. **disconnect**：`tty_unregister_device()` 移除节点；旧实现等 `acm_port_destruct()`（tty 引用归零）才 `idr_remove`；
3. **usbfs 介入**：`USBDEVFS_DISCONNECT_CLAIM` 使 usbfs 驱动接管接口、cdc_acm 被解绑 → 节点消失；`releaseintf()` 释放接口。

## 关键修复点（#196525）

| 函数 | 位置 | 作用 |
|------|------|------|
| `usbfs_rebind_interfaces` | devio.c | handle 释放全部接口后遍历 `device_attach()` 重绑内核驱动 |
| `acm_alloc_fixed_minor` | cdc-acm.c | 固定 minor 槽被 disconnected 旧实例占用时先回收再分配 |
| `acm_release_minor` | cdc-acm.c | `ACM_MINOR_INVALID` 幂等保护 |
| `acm_disconnect` | cdc-acm.c | 注销节点后立即释放 minor |

## 数据通路（串口）

```
应用读写 /dev/ttyACM11 → tty core → cdc_acm port ops
   → USB bulk IN/OUT → CH343 → 外设
```

## 固定 minor 定制规则（产品约束）

- `1a86:55da` + devpath `1.3`：接口 0 → minor 11，接口 2 → minor 12；
- `040b:a6b` + devpath `1.2` → minor 0；
- 其余设备从 minor 3 起动态分配。

---

_Author: wangguanran_
