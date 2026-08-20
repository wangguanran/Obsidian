# QCM6490 触摸手势唤醒驱动架构

## 架构分层

```
用户态 (input subsystem)
   ↑ KEY_WAKEUP 上报
触摸驱动 (egalax_i2c.c / sis_i2c.c)
   ↑ DRM_PANEL_EVENT_BLANK / UNBLANK 通知
DRM panel event notifier
   ↑ drm_panel 注册 (edp-drm-panel)
DP 显示驱动 (msm/dp/dp_panel.c, dp_parser.c)
   ↑ qcom,panel-notifier-support 解析
设备树 (DT overlay)
```

## 手势唤醒数据通路

1. **显示侧**：`dp_parser.c` 解析 `qcom,panel-notifier-support` 属性；`dp_panel.c` 在 eDP 场景下通过 `qcom,edp-default-panel` phandle 注册 `drm_panel`，并挂到 `sde_conn->panel`，使显示子系统能发出 `DRM_PANEL_EVENT_BLANK/UNBLANK` 事件
2. **触摸侧**：`sis_i2c.c` / `egalax_i2c.c` 注册 panel_event_notifier 回调，屏灭（BLANK early trigger）时：
   - 手势模式开启（`gesture_mode_enable`）：置 `g_tp_wakeup_flag=1`，`enable_irq_wake()` 保持中断唤醒能力
   - 手势模式关闭：`disable_irq()` 并记录 `sis_irq_suspended`
3. **唤醒上报**：触摸中断触发手势识别，`input_report_key(KEY_WAKEUP)` 上报系统唤醒
4. **恢复**：UNBLANK 时恢复 IRQ 状态（`enable_irq()` / `disable_irq_wake()`），复位 wakeup flag

## 关键函数

| 驱动 | 关键函数 | 作用 |
|------|---------|------|
| dp_panel.c | `dp_panel_edid_register()` | 注册 drm_panel 于 eDP 默认面板 |
| dp_parser.c | `dp_parser_parse()` | 解析 panel-notifier-support |
| sis_i2c.c | `sis_touch_panel_notifier_callback()` | 处理 BLANK/UNBLANK 事件 |
| egalax_i2c.c | `egalax_touch_panel_notifier_callback()` | 同上（eGalax 方案） |

## 注意事项

- `panel_event_notifier_register()` 返回 ERR_PTR 也可能表示失败，必须 `IS_ERR(cookie)` 一并判断（本平台修复点）
- 手势模式与普通模式的 IRQ 管理路径不同，suspend/resume 需对称处理

_Author: wangguanran_
