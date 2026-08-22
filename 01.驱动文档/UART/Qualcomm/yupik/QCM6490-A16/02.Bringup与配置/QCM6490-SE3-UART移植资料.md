# Qualcomm QCM6490 (yupik) SE3 UART 移植资料

> **版本号：v1.0**

## 芯片信息

- **芯片**：Qualcomm QCM6490（yupikp）
- **平台**：QCM6490-A16（[项目代号]_A16 / Elo_[项目代号]），Linux 6.0
- **功能**：SE3 UART 引脚配置、默认使能外部 UART、TZ QUP 访问控制

## 硬件接口

| 项目 | 说明 |
|:---|:---|
| SE3 UART | QUPv3 SE3 通道（2uart，TX/RX） |
| pinctrl | qupv3_se3_2uart_pins（active/sleep） |
| alias | yupik.dtsi 注册 SE3 串口 |

## 关键配置

### DTS pinctrl（yupik-pinctrl.dtsi）

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
};
```

### QUPv3 节点（yupik-qupv3.dtsi）

```dts
&qupv3_se3_2uart {
    status = "okay";
    pinctrl-0 = <&qupv3_se3_2uart_tx_active>;
    pinctrl-1 = <&qupv3_se3_2uart_sleep>;
    pinctrl-names = "default", "sleep";
};
```

### 产品 overlay 使能外部 UART

```dts
&qupv3_seX_4uart {
    status = "okay";
};
```

### TZ QUP 访问控制（QUPAC_Access_MT912.c）

为 SE3 UART 对应的 QUP 通道增加非安全世界访问权限（+10/−6）。

## 编译与验证

```bash
# 检查 SE3 UART 设备是否注册
ls -l /dev/ttyMSM*

# 查看 UART 驱动
cat /proc/tty/drivers/msm_serial*

# 测试 UART 收发（外接串口调试工具）
stty -F /dev/ttyMSM3 115200
echo "SE3 UART test" > /dev/ttyMSM3
```

预期：`/dev/ttyMSM*` 可见 SE3 串口节点，收发正常，TrustZone 不阻止访问。

## 移植注意事项

- 三个 Change（#195829 DTS + #195831 overlay + #195834 TZ）必须**一起合入**，缺 TZ 放行则串口不可用
- pinctrl 引脚号需按目标板硬件原理图确认

## 引用文件索引

- [[01.驱动文档/UART/Qualcomm/yupik/QCM6490-A16/03.需求实现/SE3-UART移植.md|SE3-UART移植]]（补丁内容）
- `qcom/yupik-pinctrl.dtsi`、`yupik-qupv3.dtsi`、`yupik-qrd.dtsi`、`yupik.dtsi`、`yupikp-iot-qrd-overlay-*.dtsi`（远程）
- `TZ.XF.5.35/.../QUPAC_Access_MT912.c`（远程）

---

_Author: wangguanran_
