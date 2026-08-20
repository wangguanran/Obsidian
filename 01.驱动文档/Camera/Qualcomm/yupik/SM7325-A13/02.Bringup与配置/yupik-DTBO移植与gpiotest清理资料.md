# Qualcomm SM7325 (yupik) DTBO 移植与 gpiotest 清理资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：Qualcomm SM7325（yupik）
- **平台**：SM7325-A13（LA.UM.9.14.1）
- **说明**：清理 gpiotest 测试 DTS，修复量产固件无法启动问题

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| Camera | CAMSS + CCI，sensor 走 CCI I2C |
| Display | sde 显示管线（display-devicetree） |
| DTBO | dtbo.img 分区，bootloader 按 board ID 加载 |

## 关键配置

### DTBO 编译（Makefile）

移除 gpiotest 的 `DTC_FLAGS` 与 `dtbo-y` 编译条目（+1/−3）。

### Bootloader（Board.c）

移除 gpiotest board ID 检查逻辑（+2/−2）。

### 构建脚本（vendor_sparseimage.sh）

移除 gpiotest 镜像构建步骤（+0/−4）。

## 编译与验证

```bash
# 正常编译 DTBO，不再包含 gpiotest overlay
./build.sh -T LA.UM.9.14.1

# 检查 DTBO 镜像中是否包含 gpiotest 节点（应无输出）
dtc -I dtb -O dts dtbo.img | grep -i gpiotest
```

启动验证：

1. 烧录编译后的完整固件
2. 设备上电，观察串口日志，确认进入 Android 系统
3. 回归验证：Camera、Display、GPIO（sensor/按键）功能正常

## 移植注意事项

- 若其他分支需保留 gpiotest 功能：确保 Makefile 无 gpiotest `DTC_FLAGS`/`dtbo-y` 条目、vendor_sparseimage.sh 无 gpiotest 构建步骤、Board.c 无 gpiotest board ID 检查
- 本次补丁删除量大（20 文件，约 15,006 行），移植时以 `[[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/02.Bringup与配置/91.源码与补丁索引/patches/195900-remove-gpiotest-dts.patch|195900-remove-gpiotest-dts.patch]]` 为准

## 引用文件索引

- [[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/04.问题案例/yupik删除gpiotest-DTS修复启动失败.md|yupik删除gpiotest-DTS修复启动失败]]（补丁内容）
- [[01.驱动文档/Camera/Qualcomm/yupik/SM7325-A13/02.Bringup与配置/91.源码与补丁索引/patches/195900-remove-gpiotest-dts.patch|195900-remove-gpiotest-dts.patch]]
- `bootable/bootloader/edk2/QcomModulePkg/Library/BootLib/Board.c`（远程）
- `vendor/qcom/proprietary/devicetree/qcom/Makefile`（远程）
- `vendor/vendorcode/build/vendor_sparseimage.sh`（远程）

---

_Author: wangguanran_
