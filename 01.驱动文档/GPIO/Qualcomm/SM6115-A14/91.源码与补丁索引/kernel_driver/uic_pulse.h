/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
#ifndef _UAPI_LINUX_UIC_PULSE_H
#define _UAPI_LINUX_UIC_PULSE_H

#include <linux/ioctl.h>
#include <linux/types.h>

#define UIC_PULSE_DEV_NAME		"uic_pulse"

#define UIC_PULSE_ACTIVE_HIGH		0
#define UIC_PULSE_ACTIVE_LOW		1

#define UIC_PULSE_EDGE_RISING		1
#define UIC_PULSE_EDGE_FALLING		2
#define UIC_PULSE_EDGE_BOTH		3

#define UIC_PULSE_WIDTH_MIN_MS		25
#define UIC_PULSE_WIDTH_MAX_MS		500
#define UIC_PULSE_INTERVAL_DEF_MS	100
#define UIC_PULSE_DEBOUNCE_DEF_MS	2
#define UIC_PULSE_OUT_LEVEL_CFG		0xffffffffu

/**
 * struct uic_pulse_config - runtime pulse configuration
 * @active_level: IN pulse polarity (ACTIVE_HIGH / ACTIVE_LOW)
 * @min_valid_ms: min accepted IN pulse width
 * @max_valid_ms: max accepted IN pulse width
 * @batch_gap_ms: idle gap before reporting one batch (0 = each pulse)
 * @stuck_timeout_ms: IN active longer than this is invalid (0 = off)
 * @debounce_ms: edge debounce (default 2)
 * @irq_edge: rising / falling / both
 * @out_active_level: GPIO33 pulse polarity
 * @pulse_width_ms: default OUT pulse width (25..500)
 * @interval_ms: gap between two OUT actives (default 100)
 */
struct uic_pulse_config {
	__u32 active_level;
	__u32 min_valid_ms;
	__u32 max_valid_ms;
	__u32 batch_gap_ms;
	__u32 stuck_timeout_ms;
	__u32 debounce_ms;
	__u32 irq_edge;
	__u32 out_active_level;
	__u32 pulse_width_ms;
	__u32 interval_ms;
};

/**
 * struct uic_pulse_simulate - GPIO33 coin-pulse emit
 * @count: number of pulses
 * @pulse_width_ms: 0 = use config
 * @interval_ms: 0 = use config
 * @out_active_level: UIC_PULSE_OUT_LEVEL_CFG = use config
 */
struct uic_pulse_simulate {
	__u32 count;
	__u32 pulse_width_ms;
	__u32 interval_ms;
	__u32 out_active_level;
};

/**
 * struct uic_pulse_event - one reported IN batch
 * @pulse_count: pulses in this batch
 * @last_width_ms: width of the last pulse in the batch
 * @timestamp_ns: monotonic timestamp of report
 */
struct uic_pulse_event {
	__u32 pulse_count;
	__u32 last_width_ms;
	__u64 timestamp_ns;
};

#define UIC_PULSE_IOC_MAGIC		'U'

#define UIC_PULSE_IOC_SET_CONFIG	_IOW(UIC_PULSE_IOC_MAGIC, 0x01, \
					     struct uic_pulse_config)
#define UIC_PULSE_IOC_GET_CONFIG	_IOR(UIC_PULSE_IOC_MAGIC, 0x02, \
					     struct uic_pulse_config)
#define UIC_PULSE_IOC_START		_IO(UIC_PULSE_IOC_MAGIC, 0x03)
#define UIC_PULSE_IOC_STOP		_IO(UIC_PULSE_IOC_MAGIC, 0x04)
#define UIC_PULSE_IOC_RESET		_IO(UIC_PULSE_IOC_MAGIC, 0x05)
#define UIC_PULSE_IOC_FLUSH_EVENT	_IO(UIC_PULSE_IOC_MAGIC, 0x06)
#define UIC_PULSE_IOC_OUTPUT		_IOW(UIC_PULSE_IOC_MAGIC, 0x07, \
					     struct uic_pulse_simulate)

#endif /* _UAPI_LINUX_UIC_PULSE_H */
