# 分析：ttyACM 节点恢复修复

**版本号：v1.0**
**对应文档：** [[01.驱动文档/USB/Qualcomm/yupik/SM7325-A13/04.问题案例/ttyACM节点恢复修复.md|ttyACM节点恢复修复]]

## 技术背景

- **cdc_acm**：USB CDC-ACM 串口类驱动。probe 时通过 `acm_alloc_minor()` 在全局 idr（`acm_minors`）分配 tty minor；`tty_unregister_device()` 移除节点，但 idr 槽要等 `acm_port_destruct()`（tty 引用计数归零）才释放。
- **usbfs（devio）**：用户态通过 `/dev/bus/usb` 控制设备。`USBDEVFS_DISCONNECT_CLAIM` 使 usbfs 驱动绑定接口、把内核驱动解绑；`releaseintf()` 释放接口。
- **固定 minor 定制**：本产品对 WCH CH343（`1a86:55da` devpath 1.3）固定 minor 11/12、`040b:a6b` 固定 minor 0，保证用户态串口号稳定。

## 代码改动分析

**devio.c**：

- 新增 `usbfs_rebind_interfaces(udev)`：设备处于 `USB_STATE_CONFIGURED` 时遍历所有接口，对 `!intf->dev.driver` 的接口 `device_attach()` 重绑；
- `releaseintf()`：在 `ps->ifclaimed == 0`（该 handle 已释放全部接口）时调用重绑，避免兄弟接口仍被 usbfs 持有时 cdc_acm probe 因 `-EBUSY` 失败。

**cdc-acm.c**：

- 新增 `acm_alloc_fixed_minor(acm, want)`：`idr_find` 命中且旧实例 `disconnected` 时先 `idr_remove` 并置 `ACM_MINOR_INVALID`，再 `idr_alloc` 固定槽——回收 stale 槽位；
- `acm_alloc_minor()`：WCH 分支改用固定 minor 回收函数，ifnum 0x00→11、0x02→12、其它→`-ENODEV`（原实现 else 分支给 13，与硬件不符，顺带修正）；日志统一带 ifnum；
- `acm_release_minor()`：增加 `ACM_MINOR_INVALID` 保护，幂等；
- `acm_disconnect()`：`tty_unregister_device()` 后立即 `acm_release_minor()`，不再等用户态关闭 fd。

## 潜在风险

1. **重绑时机竞态**：`usbfs_rebind_interfaces` 在 handle 释放最后一个接口时执行；若多进程/多 handle 同时 claim，`ps->ifclaimed` 只反映当前 handle，仍可能提前重绑——当前场景单进程串行使用，风险可控；
2. **idr 复用安全**：回收 disconnected 槽位时若旧实例仍被引用（`old->disconnected` 为真但 tty 未完全析构），新实例复用 minor 后，旧 fd 上的 ioctl 可能命中新实例——需要用户态关闭旧 fd（本补丁设计即要求用户态最终 close）；
3. **`-ENODEV` 变更**：WCH 非 0x00/0x02 接口从"分配 minor 13"改为 `-ENODEV`，若实际设备有第三个 ACM 接口会被拒绝 probe——需确认 WCH 硬件接口数量；
4. **其它 VID/PID**：固定 minor 判断基于 devpath（`1.3`/`1.2`），hub 拓扑变化时 devpath 会变，属既有定制约束。

## 回归测试建议

- WCH CH343：open ttyACM11 → DISCONNECT_CLAIM → close → 节点恢复；连续 20 次循环无 ENODEV；
- 热插拔：正常插拔（不经过 usbfs）时 ttyACM 注册/注销无异常；
- 其它 USB 串口（`040b:a6b`、普通 ACM 设备）回归：minor 分配不受影响；
- usbfs 批量传输/claim 常规操作回归（`USBDEVFS_CLAIMINTERFACE` 等）。

## 与现有驱动架构的关系

该修复是内核 USB 子系统级改动（usbfs + cdc_acm），不涉及平台特定代码；`usbfs_rebind_interfaces` 的重绑模式对任意"usbfs 解绑后可重绑"的多接口驱动（如 cdc_ncm、usbnet）通用。本项目 WCH 固定 minor 定制逻辑保留并增强，其它项目可参考 `acm_alloc_fixed_minor` 的 stale 回收模式。

_Author: wangguanran_
