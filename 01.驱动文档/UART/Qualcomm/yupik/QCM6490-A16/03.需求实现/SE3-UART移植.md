# QCM6490 SE3 UART 移植

**版本号：v1.0**
**类型：需求**
**状态：已合入**
**来源：** Gerrit Change 195829 / 195831 / 195834（同一 TaskID 120615/120616）
**项目：** meigla/platform/vendor/qcom-opensource/devicetree / qcm6490-la-6-0_amss_standard_oem
**分支：** Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_[项目代号] / master_meig
**作者：** [同事]
**合入时间：** 2026-08-15

## 需求描述

在 [项目代号]_A16 平台（QCM6490）上移植 SE3 UART 功能，包括：

1. **SE3 UART 引脚配置和总线命名**（#195829）：在 DTS 中配置 SE3 UART 对应的 pinctrl、QUPv3 节点和总线别名，使能 SE3 串口功能。
2. **默认使能外部 UART**（#195831）：在多个产品 overlay 中默认开启外部 UART 功能。
3. **TrustZone QUP 访问控制**（#195834）：配置 TZ（TrustZone）侧 QUP 访问控制，允许 SE3 UART 的 QUP 通道在非安全世界访问。

## 环境

- SoC：QCM6490（yupikp）
- Android 版本：A16
- 内核：Linux 6.0
- 平台：[项目代号：[项目代号]_A16 / Elo_[项目代号]]

## 方案

### 1. DTS 配置（#195829）

通过修改 `yupik-pinctrl.dtsi`、`yupik-qupv3.dtsi`、`yupik-qrd.dtsi`、`yupik.dtsi` 四个文件，完成 SE3 UART 的引脚复用配置、QUPv3 控制器节点配置、以及总线别名注册。

### 2. 产品 overlay 使能外部 UART（#195831）

在 5 个产品 overlay DTSI 文件中，将外部 UART 的 `status` 从 `disabled` 改为 `okay`，默认使能外部 UART 功能。

### 3. TrustZone QUP 访问控制（#195834）

修改 `QUPAC_Access_MT912.c` 文件，在 QUP 访问控制配置中增加 SE3 UART 对应的通道权限，允许非安全世界（Linux/Android）访问该 QUP 通道。

## 修改文件清单

### #195829 — SE3 UART DTS 配置
- `qcom/yupik-pinctrl.dtsi`：增加 SE3 UART 引脚复用配置（+42 行）
- `qcom/yupik-qupv3.dtsi`：增加 SE3 UART 控制器节点（+36 行）
- `qcom/yupik-qrd.dtsi`：引用 SE3 UART 节点（+5 行）
- `qcom/yupik.dtsi`：注册总线别名，增加 SE3 串口（+10 行）

### #195831 — 默认使能外部 UART
- `qcom/yupikp-iot-qrd-overlay-lisbonmc9392-evt2.dtsi`：status 改为 okay（+1/-1）
- `qcom/yupikp-iot-qrd-overlay-stdmc936-evt2.dtsi`：status 改为 okay（+1/-1）
- `qcom/yupikp-iot-qrd-overlay-stdmc937-evt2.dtsi`：status 改为 okay（+2/-1）
- `qcom/yupikp-iot-qrd-overlay-stdmc937mipi-evt2.dtsi`：status 改为 okay（+2/-1）
- `qcom/yupikp-iot-qrd-overlay-stdmc938-evt2.dtsi`：status 改为 okay（+1/-1）

### #195834 — TrustZone QUP 访问控制
- `TZ.XF.5.35/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/kodiak/QUPAC_Access_MT912.c`：增加 SE3 UART QUP 通道权限（+10/-6）

## 配置方式

### DTS 配置

**pinctrl 配置（yupik-pinctrl.dtsi）：**
```dts
&qupv3_se3_2uart_pins {
    qupv3_se3_2uart_tx_active: qupv3_se3_2uart_tx_active {
        mux {
            pins = "gpioXX";
            function = "qup_se3";
        };
        config {
            pins = "gpioXX";
            drive-strength = <2>;
            bias-disable;
        };
    };
    // ... 其他引脚配置
};
```

**QUPv3 节点配置（yupik-qupv3.dtsi）：**
```dts
&qupv3_se3_2uart {
    status = "okay";
    pinctrl-0 = <&qupv3_se3_2uart_tx_active>;
    pinctrl-1 = <&qupv3_se3_2uart_sleep>;
    pinctrl-names = "default", "sleep";
};
```

**产品 overlay 中使能外部 UART：**
```dts
&qupv3_seX_4uart {
    status = "okay";
};
```

### Kernel config
- 无 Kconfig 变更（UART 驱动已在 QCM6490 内核中内置）

### TZ 配置
- `QUPAC_Access_MT912.c` 中为 SE3 UART 对应的 QUP 通道增加非安全世界访问权限

### 其他配置
- 无 BoardConfig 变更

## 验证方式

- **验证命令**：
  ```bash
  # 检查 SE3 UART 设备是否注册
  ls -l /dev/ttyMSM*
  # 查看 UART 设备节点
  cat /proc/tty/drivers/msm_serial*
  # 测试 UART 收发（外接串口调试工具）
  stty -F /dev/ttyMSM3 115200
  echo "SE3 UART test" > /dev/ttyMSM3
  ```
- **预期结果**：
  - `/dev/ttyMSM*` 中可以看到 SE3 对应的串口设备节点
  - UART 收发功能正常，外部设备可正常通信
  - TrustZone 不会阻止非安全世界对 SE3 QUP 通道的访问
- **实际结果**：三个提交合入后，SE3 UART 功能正常，外部 UART 默认使能。

## 补丁内容

详见 Gerrit Change：
- #195829：https://[内部系统]/c/meigla/platform/vendor/qcom-opensource/devicetree/+/195829
- #195831：https://[内部系统]/c/meigla/platform/vendor/qcom-opensource/devicetree/+/195831
- #195834：https://[内部系统]/c/qcm6490-la-6-0_amss_standard_oem/+/195834

## 源码归档

- 源码未归档（远程源码树不可达，文件位于 Gerrit 仓库 `meigla/platform/vendor/qcom-opensource/devicetree` 和 `qcm6490-la-6-0_amss_standard_oem`）

## 引用文件索引

- `qcom/yupik-pinctrl.dtsi`：QCM6490 引脚复用配置，增加 SE3 UART 引脚（远程 `meigla/platform/vendor/qcom-opensource/devicetree`）
- `qcom/yupik-qupv3.dtsi`：QUPv3 控制器配置，增加 SE3 UART 节点（同上）
- `qcom/yupik-qrd.dtsi`：QRD 板级配置，引用 SE3 UART（同上）
- `qcom/yupik.dtsi`：SoC 级 DTS，注册 UART 总线别名（同上）
- `qcom/yupikp-iot-qrd-overlay-*.dtsi`：5 个产品 overlay，使能外部 UART（同上）
- `TZ.XF.5.35/trustzone_images/core/settings/buses/qup_accesscontrol/qupv3/config/kodiak/QUPAC_Access_MT912.c`：TZ QUP 访问控制配置（远程 `qcm6490-la-6-0_amss_standard_oem`）

_Author: 艾达_