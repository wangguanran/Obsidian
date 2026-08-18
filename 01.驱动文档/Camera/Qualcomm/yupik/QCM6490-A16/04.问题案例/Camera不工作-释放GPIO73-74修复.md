# Camera 不工作修复：释放 GPIO73/74（CCI2）

> **模块**: Camera | **厂商**: Qualcomm | **芯片**: yupik (QCM6490)
> **平台**: QCM6490-A16 (meigla camera-devicetree) | **类型**: Bug
> **Change**: #196280 | **作者**: zhaoqian | **状态**: MERGED

---

## 基本信息

| 项目 | 内容 |
|------|------|
| Change | #196280 |
| 项目仓库 | meigla/platform/vendor/opensource/camera-devicetree |
| 分支 | Develop_QCM6490.LA.6.0_VENDOR_QCOM_Platform_Elo_Rigel |
| 禅道任务 | Task 120725 |
| 作者 | zhaoqian |
| 芯片 | Qualcomm QCM6490 (yupik) |
| 平台 | QCM6490-A16（Rigel 系列） |
| 模块 | Camera (CCI I2C) |

## 现象

摄像头不工作（camera not working）。排查发现 CCI1 控制器节点同时使能了 CCI2（GPIO73/74）与 CCI3（GPIO75/76）两组 pinctrl 和 gpio 请求，GPIO73/74 与其他功能冲突，导致 camera 初始化失败。

## 根因分析

`yupik-camera-937-pvt.dtsi` 中 CCI1 节点（`qcom,cci1`）错误地同时引用了 cci2 与 cci3 两组引脚：

- `pinctrl-0 = <&cci2_active &cci3_active>` — 同时拉起两组 CCI 引脚
- `gpios = <&tlmm 73 0>, <&tlmm 74 0>, <&tlmm 75 0>, <&tlmm 76 0>` — 请求 4 个 GPIO
- `gpio-req-tbl-*` 对应 4 个 GPIO（CCI_I2C_DATA2/CLK2/CCI_I2C_DATA3/CLK3）

GPIO73/74（CCI2 的 I2C 数据/时钟）被占用后与其他外设（或复用配置）冲突，导致 camera 探测失败。实际该产品摄像头挂接在 CCI3 上，CCI2 不应在此节点使能，应释放 GPIO73/74。

## 处理方案

从 CCI1 节点移除 CCI2 相关配置，只保留 CCI3：

- `pinctrl-0/pinctrl-1` 只保留 `cci3_active` / `cci3_suspend`
- `gpios` 只保留 `<&tlmm 75 0>, <&tlmm 76 0>`（CCI3 DATA/CLK）
- `gpio-req-tbl-num/flags/label` 缩减为 2 项（CCI_I2C_DATA3 / CCI_I2C_CLK3）
- 增加注释说明：`//pinctrl-0 = <&cci3_active>;pinctrl-1 = <&cci3_suspend>; cam_default is active.`

释放出的 GPIO73/74 可供其他功能使用。

## 修改文件清单

| # | 文件路径 | 改动 | 说明 |
|---|---------|------|------|
| 1 | [[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-camera-937-pvt.dtsi\|yupik-camera-937-pvt.dtsi]] | +16/-10 | CCI1 节点移除 CCI2（GPIO73/74），仅保留 CCI3（GPIO75/76） |

## 配置方式

### DTS 配置（修改后）

```dts
&cci1 {
    /* ... */
    src-clock-name = "cci_1_clk_src";
    clock-cntl-level = "lowsvs";
    clock-rates = <37500000 0>;
    //pinctrl-0 = <&cci3_active>;pinctrl-1 = <&cci3_suspend>; cam_default is active.
    pinctrl-names = "cam_default", "cam_suspend";
    pinctrl-0 = <&cci3_active>;
    pinctrl-1 = <&cci3_suspend>;
    gpios = <&tlmm 75 0>,
            <&tlmm 76 0>;
    gpio-req-tbl-num = <0 1>;
    gpio-req-tbl-flags = <1 1>;
    gpio-req-tbl-label =  "CCI_I2C_DATA3",
                    "CCI_I2C_CLK3";
    /* ... */
};
```

### GPIO 对照

| GPIO | 功能 | 修改前 | 修改后 |
|:-----|:-----|:-------|:-------|
| 73 | CCI_I2C_DATA2 | 被 CCI1 请求 | 释放 |
| 74 | CCI_I2C_CLK2 | 被 CCI1 请求 | 释放 |
| 75 | CCI_I2C_DATA3 | 被 CCI1 请求 | 保留 |
| 76 | CCI_I2C_CLK3 | 被 CCI1 请求 | 保留 |

## 验证方式

1. **DTS 编译**：编译 camera-devicetree 产物无报错
2. **设备验证**：
   ```bash
   adb shell "cat /sys/kernel/debug/gpio | grep -E 'gpio-7[3-6]'"
   # 预期：gpio73/74 无 camera 占用（已释放），gpio75/76 为 CCI3 功能
   ```
3. **Camera 功能**：打开相机预览、拍照正常，`dmesg | grep -i cci` 无 CCI 冲突报错

## 结论

通过释放 CCI1 节点上误占用的 GPIO73/74（CCI2），消除 GPIO 复用冲突，摄像头恢复工作。GPIO73/74 可分配给其他外设。

## 补丁内容

```diff
Subject: [PATCH] [MC937][TaskID]120725[Description]camera not working[solution]release gpio73\gpio74[owner]wangzhiwen

diff --git a/937/pvt/yupik-camera-937-pvt.dtsi b/937/pvt/yupik-camera-937-pvt.dtsi
index b6becc4..4369524 100644
--- a/937/pvt/yupik-camera-937-pvt.dtsi
+++ b/937/pvt/yupik-camera-937-pvt.dtsi
@@ -754,18 +754,15 @@
 		src-clock-name = "cci_1_clk_src";
 		clock-cntl-level = "lowsvs";
 		clock-rates = <37500000 0>;
+                //pinctrl-0 = <&cci3_active>;pinctrl-1 = <&cci3_suspend>; cam_default is active.
 		pinctrl-names = "cam_default", "cam_suspend";
-		pinctrl-0 = <&cci2_active &cci3_active>;
-		pinctrl-1 = <&cci2_suspend &cci3_suspend>;
-		gpios = <&tlmm 73 0>,
-			<&tlmm 74 0>,
-			<&tlmm 75 0>,
+		pinctrl-0 = <&cci3_active>;
+		pinctrl-1 = <&cci3_suspend>;
+		gpios = <&tlmm 75 0>,
 			<&tlmm 76 0>;
-		gpio-req-tbl-num = <0 1 2 3>;
-		gpio-req-tbl-flags = <1 1 1 1>;
-		gpio-req-tbl-label = "CCI_I2C_DATA2",
-					"CCI_I2C_CLK2",
-					"CCI_I2C_DATA3",
+		gpio-req-tbl-num = <0 1>;
+		gpio-req-tbl-flags = <1 1>;
+		gpio-req-tbl-label = 	"CCI_I2C_DATA3",
 					"CCI_I2C_CLK3";

 		i2c_freq_100Khz_cci1: qcom,i2c_standard_mode {
```

## 补丁验证

⚠️ **无法在源码树验证**：`meigla/platform/vendor/opensource/camera-devicetree` 仓库不在 134 服务器上（无源码树）。补丁已从 Gerrit REST API 获取，格式完整（1 个文件，+16/-10）。

## 源码归档

| 文件 | 归档位置 | 说明 |
|:-----|:---------|:-----|
| yupik-camera-937-pvt.dtsi | [[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-camera-937-pvt.dtsi\|yupik-camera-937-pvt.dtsi]] | 从 Gerrit REST 恢复的合并后版本（52KB） |
| 196280.patch | [[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/patches/196280.patch\|196280.patch]] | 补丁文件 |

> ⚠️ **源码文件不在本地源码树归档**：项目 `meigla/platform/vendor/opensource/camera-devicetree` 在 134 服务器上没有对应源码树，仅归档了补丁文件与从 Gerrit REST 恢复的文件版本。如需获取源码，可通过 Gerrit 拉取补丁。

## 引用文件索引

### yupik-camera-937-pvt.dtsi
- 直达链接：[[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/dt_config/yupik-camera-937-pvt.dtsi|yupik-camera-937-pvt.dtsi]]
- 完整路径：`91.源码与补丁索引/dt_config/yupik-camera-937-pvt.dtsi`
- 说明：MC937 产品 camera devicetree，CCI1 节点修改（合并后版本，从 Gerrit REST 恢复）

### 196280.patch
- 直达链接：[[01.驱动文档/Camera/Qualcomm/yupik/QCM6490-A16/91.源码与补丁索引/patches/196280.patch|196280.patch]]
- 完整路径：`91.源码与补丁索引/patches/196280.patch`
- 说明：Change #196280 补丁（已清理隐私，末尾已补换行符）

_Author: wangguanran_
