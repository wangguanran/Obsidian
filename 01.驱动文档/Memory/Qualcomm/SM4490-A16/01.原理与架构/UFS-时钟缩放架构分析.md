# UFS 时钟缩放架构分析（[项目代号] / SM4490）

> 平台：SM4490-A16（parrot）| 内核：msm-kernel | 模块：drivers/scsi/ufs

## 工作原理

UFS 主机控制器（ufshcd/ufs-qcom）通过 **devfreq 框架**做时钟缩放：根据 IO 忙闲度动态调整 `core_clk` 频率，平衡性能与功耗。

```
IO 请求 → ufshcd → devfreq governor (simple_ondemand)
                    ├─ busy% > upthreshold(70)          → 升频
                    ├─ busy% <= upthreshold-downdiff    → 降频
                    └─ 其余情况                          → 维持当前频率
```

### 关键参数（ufs_qcom_config_scaling_param）

| 参数 | 默认值 | 语义 |
|------|--------|------|
| upthreshold | 70 | busy% 高于此值升频 |
| downdifferential | 65（UFS 3.x）/<br>45（UFS 2.x，本修复后） | 降频阈值 = upthreshold - downdifferential（默认 5，修复后 25） |
| polling_ms | 60 | 采样周期 |

### 器件 quirk 机制（ufs_qcom_dev_fixups）

驱动用 `UFS_FIX(vendor, model, quirk)` 表按 JEDEC 厂商 ID 匹配器件，挂载行为修正。与时钟缩放相关的关键 quirk：

- `UFS_DEVICE_QUIRK_DELAY_BEFORE_LPM`：idle 后延迟进入低功耗模式，为 devfreq 保留降频窗口。

## 初始化流程

1. `ufs_qcom_probe()` → `ufshcd_alloc_host()`；
2. `ufs_qcom_setup_clocks()` / `ufs_qcom_init()`：注册 devfreq 设备；
3. `ufs_qcom_config_scaling_param()`：配置 devfreq governor 参数（upthreshold/downdifferential）；
4. 器件枚举后 `ufshcd_fixup_device()`：按厂商 ID 查 `ufs_qcom_dev_fixups[]` 应用 quirk。

## 数据通路

```
用户/文件系统 → SCSI 层 → ufshcd_queuecommand → ufs-qcom (variant ops)
                                                     ↓
                                              UFS 器件 (HS-G2/G4)
                                                     ↓
                                 devfreq 采样 busy% → simple_ondemand → clk 频率切换
```

## 关键函数

| 函数 | 作用 |
|------|------|
| `ufs_qcom_config_scaling_param()` | 设置 upthreshold/downdifferential（本修复点） |
| `ufs_qcom_dev_fixups[]` | 器件 quirk 表（本修复点：新增 HOSINGLOBAL） |
| `ufs_qcom_update_bus_bw_vote()` | devfreq 回调，频率切换 |
| `ufshcd_fixup_device()` | 应用 quirk |

## 与平台交互

- `limit_phy_submode`（ufs_qcom_host 私有字段）：标识 PHY 子模式，**0 = UFS 2.x**（HS-G2 速率族），非 0 = UFS 3.x（HS-G4），本修复据此区分器件代际；
- 时钟树：UFS core_clk 由 GCC/GPUCC 提供，devfreq 通过 `clk_set_rate` 切换。

## 参考

- `drivers/scsi/ufs/ufs-qcom.c`：本平台归档源码见 [[01.驱动文档/Memory/Qualcomm/SM4490-A16/91.源码与补丁索引/kernel_driver/drivers/scsi/ufs/ufs-qcom.c\|ufs-qcom.c]]

_Author: wangguanran_
