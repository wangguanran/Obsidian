# Qualcomm QCM6490 (yupik) Camera / CCI 驱动架构分析

> **版本号：v1.0**

## 平台概述

QCM6490 (yupik) 的 Camera 子系统由 **CAMSS** 与 **CCI** 组成：

```
Sensor (I2C via CCI) ──┐
                       ▼
CCI 控制器 (qcom,cci0 / qcom,cci1)
   ├─ CCI0: CCI0/CCI1 引脚 (GPIO69-72)
   └─ CCI1: CCI2/CCI3 引脚 (GPIO73-76)
                       ▼
CSI PHY (csiphy0-4) ← 像素数据通道
   ├─ csiphy0/1/2/3/4
   └─ csi 时钟 (csi0-4phytimer)
                       ▼
CAMSS (qcom,camss, cam_cc_titan_top_gdsc)
   └─ VFE/ICP 等图像处理管线
```

## CCI 控制器（I2C 控制通道）

CCI (Camera Control Interface) 是 Qualcomm Camera 的专用 I2C 控制器，每路支持两组 I2C master（如 CCI1 控制器含 CCI2/CCI3 引脚组）。设备树中每个 `qcom,cci` 节点包含：

| 属性 | 作用 |
|:---|:---|
| `pinctrl-0/-1` | active/suspend 引脚下电配置（cciN_active / cciN_suspend） |
| `gpios` | CCI I2C 引脚（DATA/CLK 对） |
| `gpio-req-tbl-*` | 引脚请求表（num/flags/label），驱动据此 request 引脚 |
| `i2c_freq_*` | I2C 时序配置（standard/fast/fast_plus/custom） |
| `clocks` | CCI 参考时钟（cci_N_clk_src，37.5MHz） |

## MC937 的 CCI1 配置演进

- **修改前**：`cam_cci1` 同时挂 `cci2_active & cci3_active`，请求 GPIO73/74（CCI2）+ GPIO75/76（CCI3）4 个引脚；
- **修改后**：`cam_cci1` 只挂 `cci3_active`，仅请求 GPIO75/76（CCI3）2 个引脚，**释放 GPIO73/74**（该两脚被板级其它外设占用，Camera 与其它功能引脚冲突导致 Camera 不工作）。

```
cam_cci1 (qcom,cci1):
  修改前: pinctrl = cci2_active+cci3_active;  gpios = tlmm 73,74,75,76  ← 与其它外设冲突
  修改后: pinctrl = cci3_active;              gpios = tlmm 75,76       ← 释放 73/74
```

## CCI 引脚请求机制

CCI 驱动 probe 时按 `gpio-req-tbl-num` 逐项 `gpio_request()` 并设置 label；suspend 时按 `pinctrl-1` 释放引脚。因此 **pinctrl 引用与 gpios 列表必须一致** —— 只删 gpios 不够，必须同时把 pinctrl 中的 `cci2_active`/`cci2_suspend` 移除，否则仍会占用 GPIO73/74。

## 相关文件索引

- [[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-camera-937-pvt.dtsi|yupik-camera-937-pvt.dtsi]]（MC937 camera devicetree，Gerrit 恢复版）

---

_Author: wangguanran_