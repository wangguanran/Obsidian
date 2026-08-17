# 修改历史

## LA.UM.9.14.1 — HXB_SNM932_PinMingTong

### #195900 — remove useless gpiotest dts

| 项目 | 内容 |
|------|------|
| Change-Id | #195900 |
| Gerrit | 134 服务器，项目 LA.UM.9.14.1（源码不在本地） |
| 分支 | master_LA.4.0_HXB_SNM932_PinMingTong_7a07b9e |
| 作者 | kuangjincheng |
| 提交者 | weirong |
| 状态 | MERGED |
| 类型 | Bug（boot failure） |
| 提交标题 | [HXB_SNM932_PinMingTong][TaskID]120654[Description]device cannot boot[Solution]remove useless gpiotest dts |

**涉及文件统计：** 20 个文件（19 个删除 + 3 个修改），~15,006 行删除

**修改概要：** 彻底删除所有 gpiotest 相关的 DTS 文件及引用，修复因 gpiotest overlay 与产品硬件冲突导致的启动失败问题。

**补丁文件：** `patches/195900-remove-gpiotest-dts.patch`（393KB，15,146 行）