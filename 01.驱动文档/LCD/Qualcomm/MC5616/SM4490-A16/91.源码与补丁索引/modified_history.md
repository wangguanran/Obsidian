# MC5616 SM4490-A16 显示修改历史

## Change #196185 (2026-08-18 归档)

- 提交: [MC5616][96455][Display]Disable ESD check for r66451 AMOLED video panel [Owner][同事]
- 涉及文件:
  - `vendor/qcom/proprietary/display-devicetree/display/parrot-sde-display-common.dtsi`（+1/−1，注释 `&dsi_r66451_amoled_video` 节点 `qcom,esd-check-enabled`）
- 补丁验证: ✅ 可干净应用（134 源码树直接 git apply --check）

### 相关提交基线

| 文件 | 提交 | 基线版本 |
|:---|:---|:---|
| parrot-sde-display-common.dtsi | c2094387615 | Snapdragon_Premium_High_2021.SPF.2.0.1 \| N/A 0.0.017.0 \| LA.VENDOR.1.0.r1-26100-WAIPIO.QSSI14.0-1 |

_Author: wangguanran_