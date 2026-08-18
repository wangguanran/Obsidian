// SPDX-License-Identifier: GPL-2.0-only
/*
 * UIC GPIO pulse: /dev/uic_pulse and /sys/class/misc/uic_pulse
 *
 * MT5205: GPIO32 IN (detect) / GPIO33 OUT (emit).
 * Default: IN/OUT ACTIVE_HIGH (MT5205 direct wire); optocoupler boards use ACTIVE_LOW IN.
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio/consumer.h>
#include <linux/hrtimer.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/math64.h>
#include <linux/irq.h>
#include <linux/kfifo.h>
#include <linux/ktime.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <uapi/linux/uic_pulse.h>

#define UIC_PULSE_FIFO_LEN		32
#define UIC_PULSE_SIM_MAX		256
#define UIC_PULSE_BUSYWAIT_US		2000

/* Wait until monotonic deadline (compensates usleep overshoot). */
static void uic_pulse_wait_until(ktime_t deadline)
{
	while (ktime_before(ktime_get(), deadline)) {
		s64 left_us = ktime_us_delta(deadline, ktime_get());

		if (left_us <= 0)
			break;
		if (left_us < UIC_PULSE_BUSYWAIT_US)
			udelay((unsigned int)left_us);
		else
			usleep_range((unsigned int)(left_us / 2),
				     (unsigned int)left_us);
	}
}

struct uic_pulse_dev {
	struct device *dev;
	struct gpio_desc *in;
	struct gpio_desc *out;
	int irq;
	bool started;
	bool in_active;
	bool pulse_valid;
	bool stuck;
	ktime_t t_active;
	ktime_t t_last_edge;
	u32 batch_count;
	u32 last_width_ms;
	struct uic_pulse_config cfg;
	spinlock_t lock;
	struct mutex io_lock;
	wait_queue_head_t wq;
	struct hrtimer debounce_timer;
	struct hrtimer stuck_timer;
	struct hrtimer batch_timer;
	DECLARE_KFIFO(fifo, struct uic_pulse_event, UIC_PULSE_FIFO_LEN);
	struct miscdevice misc;
};

static unsigned long uic_pulse_irq_flags(u32 edge)
{
	unsigned long flags = 0;

	switch (edge) {
	case UIC_PULSE_EDGE_RISING:
		flags |= IRQF_TRIGGER_RISING;
		break;
	case UIC_PULSE_EDGE_FALLING:
		flags |= IRQF_TRIGGER_FALLING;
		break;
	case UIC_PULSE_EDGE_BOTH:
	default:
		flags |= IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;
		break;
	}
	return flags;
}

static unsigned int uic_pulse_irq_type(u32 edge)
{
	switch (edge) {
	case UIC_PULSE_EDGE_RISING:
		return IRQ_TYPE_EDGE_RISING;
	case UIC_PULSE_EDGE_FALLING:
		return IRQ_TYPE_EDGE_FALLING;
	case UIC_PULSE_EDGE_BOTH:
	default:
		return IRQ_TYPE_EDGE_BOTH;
	}
}

static int uic_pulse_in_raw(struct uic_pulse_dev *d)
{
	if (!d->in)
		return -ENODEV;
	return gpiod_get_raw_value(d->in);
}

static bool uic_pulse_raw_is_active(const struct uic_pulse_config *cfg, int raw)
{
	if (cfg->active_level == UIC_PULSE_ACTIVE_LOW)
		return raw == 0;
	return raw != 0;
}

static void uic_pulse_set_out_raw(struct uic_pulse_dev *d, int raw)
{
	if (!d->out)
		return;
	if (gpiod_cansleep(d->out))
		gpiod_set_raw_value_cansleep(d->out, raw);
	else
		gpiod_set_raw_value(d->out, raw);
}

static int uic_pulse_out_active_raw(u32 level)
{
	return level == UIC_PULSE_ACTIVE_LOW ? 0 : 1;
}

static int uic_pulse_out_idle_raw(u32 level)
{
	return uic_pulse_out_active_raw(level) ? 0 : 1;
}

static void uic_pulse_push_event(struct uic_pulse_dev *d, u32 count, u32 width_ms)
{
	struct uic_pulse_event ev = {
		.pulse_count = count,
		.last_width_ms = width_ms,
		.timestamp_ns = ktime_get_ns(),
	};

	if (!count)
		return;
	kfifo_in(&d->fifo, &ev, 1);
	wake_up_interruptible(&d->wq);
}

static void uic_pulse_reset_state_locked(struct uic_pulse_dev *d)
{
	d->in_active = false;
	d->pulse_valid = false;
	d->stuck = false;
	d->batch_count = 0;
	d->last_width_ms = 0;
}

static enum hrtimer_restart uic_pulse_debounce_fn(struct hrtimer *t)
{
	struct uic_pulse_dev *d = container_of(t, struct uic_pulse_dev,
					       debounce_timer);
	unsigned long flags;
	int raw;
	bool active;
	bool need_cancel_stuck = false;
	bool need_cancel_batch = false;
	bool need_start_batch = false;

	raw = uic_pulse_in_raw(d);
	if (raw < 0)
		return HRTIMER_NORESTART;

	spin_lock_irqsave(&d->lock, flags);
	if (!d->started) {
		spin_unlock_irqrestore(&d->lock, flags);
		return HRTIMER_NORESTART;
	}

	active = uic_pulse_raw_is_active(&d->cfg, raw);
	if (active == d->in_active) {
		spin_unlock_irqrestore(&d->lock, flags);
		return HRTIMER_NORESTART;
	}

	d->in_active = active;
	if (active) {
		d->t_active = ktime_get();
		d->pulse_valid = true;
		d->stuck = false;
		/* New pulse: idle gap not reached yet, postpone batch. */
		if (d->cfg.batch_gap_ms && d->batch_count)
			need_cancel_batch = true;
		if (d->cfg.stuck_timeout_ms)
			hrtimer_start(&d->stuck_timer,
				      ms_to_ktime(d->cfg.stuck_timeout_ms),
				      HRTIMER_MODE_REL);
	} else {
		u32 width_ms;
		u64 width_us;

		need_cancel_stuck = true;
		width_us = (u64)ktime_us_delta(ktime_get(), d->t_active);
		width_ms = (u32)div_u64(width_us + 500, 1000);
		if (d->stuck || !d->pulse_valid) {
			d->pulse_valid = false;
			d->stuck = false;
		} else if (width_ms < d->cfg.min_valid_ms ||
			   width_ms > d->cfg.max_valid_ms) {
			d->pulse_valid = false;
		} else {
			d->last_width_ms = width_ms;
			d->batch_count++;
			d->pulse_valid = false;
			if (!d->cfg.batch_gap_ms)
				uic_pulse_push_event(d, 1, width_ms);
			else
				need_start_batch = true;
		}
	}
	spin_unlock_irqrestore(&d->lock, flags);
	if (need_cancel_stuck)
		hrtimer_cancel(&d->stuck_timer);
	if (need_cancel_batch)
		hrtimer_cancel(&d->batch_timer);
	if (need_start_batch)
		hrtimer_start(&d->batch_timer,
			      ms_to_ktime(d->cfg.batch_gap_ms),
			      HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart uic_pulse_stuck_fn(struct hrtimer *t)
{
	struct uic_pulse_dev *d = container_of(t, struct uic_pulse_dev,
					       stuck_timer);
	unsigned long flags;

	spin_lock_irqsave(&d->lock, flags);
	if (d->started && d->in_active) {
		d->stuck = true;
		d->pulse_valid = false;
	}
	spin_unlock_irqrestore(&d->lock, flags);
	return HRTIMER_NORESTART;
}

static enum hrtimer_restart uic_pulse_batch_fn(struct hrtimer *t)
{
	struct uic_pulse_dev *d = container_of(t, struct uic_pulse_dev,
					       batch_timer);
	unsigned long flags;
	u32 count, width;

	spin_lock_irqsave(&d->lock, flags);
	count = d->batch_count;
	width = d->last_width_ms;
	d->batch_count = 0;
	if (count)
		uic_pulse_push_event(d, count, width);
	spin_unlock_irqrestore(&d->lock, flags);
	return HRTIMER_NORESTART;
}

static irqreturn_t uic_pulse_isr(int irq, void *data)
{
	struct uic_pulse_dev *d = data;
	unsigned long flags;
	ktime_t now;
	u64 delta_us;

	now = ktime_get();
	spin_lock_irqsave(&d->lock, flags);
	if (!d->started) {
		spin_unlock_irqrestore(&d->lock, flags);
		return IRQ_HANDLED;
	}

	delta_us = (u64)ktime_us_delta(now, d->t_last_edge);
	d->t_last_edge = now;
	if (d->cfg.debounce_ms &&
	    delta_us < (u64)d->cfg.debounce_ms * 1000) {
		spin_unlock_irqrestore(&d->lock, flags);
		return IRQ_HANDLED;
	}

	if (d->cfg.debounce_ms) {
		hrtimer_start(&d->debounce_timer,
			      ms_to_ktime(d->cfg.debounce_ms),
			      HRTIMER_MODE_REL);
		spin_unlock_irqrestore(&d->lock, flags);
		return IRQ_HANDLED;
	}

	spin_unlock_irqrestore(&d->lock, flags);
	hrtimer_start(&d->debounce_timer, 0, HRTIMER_MODE_REL);
	return IRQ_HANDLED;
}

static void uic_pulse_cancel_timers(struct uic_pulse_dev *d)
{
	hrtimer_cancel(&d->debounce_timer);
	hrtimer_cancel(&d->stuck_timer);
	hrtimer_cancel(&d->batch_timer);
}

static int uic_pulse_validate_config(struct uic_pulse_config *c)
{
	if (c->active_level != UIC_PULSE_ACTIVE_HIGH &&
	    c->active_level != UIC_PULSE_ACTIVE_LOW)
		return -EINVAL;
	if (c->out_active_level != UIC_PULSE_ACTIVE_HIGH &&
	    c->out_active_level != UIC_PULSE_ACTIVE_LOW)
		return -EINVAL;
	if (c->irq_edge != UIC_PULSE_EDGE_RISING &&
	    c->irq_edge != UIC_PULSE_EDGE_FALLING &&
	    c->irq_edge != UIC_PULSE_EDGE_BOTH)
		return -EINVAL;
	if (c->min_valid_ms > c->max_valid_ms)
		return -EINVAL;
	if (c->max_valid_ms > 10000 || c->batch_gap_ms > 10000 ||
	    c->stuck_timeout_ms > 60000 || c->debounce_ms > 100)
		return -EINVAL;
	if (c->pulse_width_ms < UIC_PULSE_WIDTH_MIN_MS ||
	    c->pulse_width_ms > UIC_PULSE_WIDTH_MAX_MS)
		return -EINVAL;
	if (c->interval_ms > 10000)
		return -EINVAL;
	return 0;
}

static void uic_pulse_default_config(struct uic_pulse_config *c)
{
	c->active_level = UIC_PULSE_ACTIVE_HIGH;
	c->min_valid_ms = 10;
	c->max_valid_ms = 500;
	c->batch_gap_ms = 200;
	c->stuck_timeout_ms = 1000;
	c->debounce_ms = UIC_PULSE_DEBOUNCE_DEF_MS;
	c->irq_edge = UIC_PULSE_EDGE_BOTH;
	c->out_active_level = UIC_PULSE_ACTIVE_HIGH;
	c->pulse_width_ms = 50;
	c->interval_ms = UIC_PULSE_INTERVAL_DEF_MS;
}

static int uic_pulse_emit(struct uic_pulse_dev *d, struct uic_pulse_simulate *s)
{
	u32 width_ms, interval_ms, level, i;
	int active, idle;

	if (!d->out)
		return -ENODEV;
	if (!s->count || s->count > UIC_PULSE_SIM_MAX)
		return -EINVAL;

	width_ms = s->pulse_width_ms ? s->pulse_width_ms : d->cfg.pulse_width_ms;
	interval_ms = s->interval_ms ? s->interval_ms : d->cfg.interval_ms;
	level = (s->out_active_level == UIC_PULSE_OUT_LEVEL_CFG) ?
		d->cfg.out_active_level : s->out_active_level;

	if (width_ms < UIC_PULSE_WIDTH_MIN_MS ||
	    width_ms > UIC_PULSE_WIDTH_MAX_MS)
		return -EINVAL;
	if (level != UIC_PULSE_ACTIVE_HIGH && level != UIC_PULSE_ACTIVE_LOW)
		return -EINVAL;
	if (interval_ms > 10000)
		return -EINVAL;

	active = uic_pulse_out_active_raw(level);
	idle = uic_pulse_out_idle_raw(level);

	mutex_lock(&d->io_lock);
	for (i = 0; i < s->count; i++) {
		ktime_t deadline;

		uic_pulse_set_out_raw(d, active);
		deadline = ktime_add_ns(ktime_get(),
					(u64)width_ms * NSEC_PER_MSEC);
		uic_pulse_wait_until(deadline);
		uic_pulse_set_out_raw(d, idle);
		if (i + 1 < s->count && interval_ms) {
			deadline = ktime_add_ns(ktime_get(),
						 (u64)interval_ms * NSEC_PER_MSEC);
			uic_pulse_wait_until(deadline);
		}
	}
	mutex_unlock(&d->io_lock);
	return 0;
}

static int uic_pulse_open(struct inode *inode, struct file *file)
{
	struct miscdevice *misc = file->private_data;
	struct uic_pulse_dev *d = container_of(misc, struct uic_pulse_dev, misc);

	file->private_data = d;
	return 0;
}

static ssize_t uic_pulse_read(struct file *file, char __user *buf,
			      size_t count, loff_t *ppos)
{
	struct uic_pulse_dev *d = file->private_data;
	struct uic_pulse_event ev;
	unsigned long flags;
	int n;

	if (count < sizeof(ev))
		return -EINVAL;

	if (file->f_flags & O_NONBLOCK) {
		spin_lock_irqsave(&d->lock, flags);
		n = kfifo_out(&d->fifo, &ev, 1);
		spin_unlock_irqrestore(&d->lock, flags);
		if (!n)
			return -EAGAIN;
	} else {
		n = wait_event_interruptible(d->wq, ({
			int ready;

			spin_lock_irqsave(&d->lock, flags);
			ready = kfifo_out(&d->fifo, &ev, 1);
			spin_unlock_irqrestore(&d->lock, flags);
			ready;
		}));
		if (n)
			return n;
	}

	if (copy_to_user(buf, &ev, sizeof(ev)))
		return -EFAULT;
	return sizeof(ev);
}

static __poll_t uic_pulse_poll(struct file *file, poll_table *wait)
{
	struct uic_pulse_dev *d = file->private_data;
	unsigned long flags;
	__poll_t mask = 0;

	poll_wait(file, &d->wq, wait);
	spin_lock_irqsave(&d->lock, flags);
	if (!kfifo_is_empty(&d->fifo))
		mask |= EPOLLIN | EPOLLRDNORM;
	spin_unlock_irqrestore(&d->lock, flags);
	if (d->out)
		mask |= EPOLLOUT | EPOLLWRNORM;
	return mask;
}

static int uic_pulse_apply_config(struct uic_pulse_dev *d,
				  struct uic_pulse_config *cfg)
{
	unsigned long flags;
	int ret;

	ret = uic_pulse_validate_config(cfg);
	if (ret)
		return ret;

	mutex_lock(&d->io_lock);
	if (d->irq > 0) {
		disable_irq(d->irq);
		ret = irq_set_irq_type(d->irq, uic_pulse_irq_type(cfg->irq_edge));
		if (ret) {
			enable_irq(d->irq);
			mutex_unlock(&d->io_lock);
			return ret;
		}
		d->cfg.irq_edge = cfg->irq_edge;
	}
	uic_pulse_cancel_timers(d);
	spin_lock_irqsave(&d->lock, flags);
	d->cfg = *cfg;
	uic_pulse_reset_state_locked(d);
	spin_unlock_irqrestore(&d->lock, flags);
	uic_pulse_set_out_raw(d, uic_pulse_out_idle_raw(cfg->out_active_level));
	if (d->irq > 0)
		enable_irq(d->irq);
	mutex_unlock(&d->io_lock);
	return 0;
}

static void uic_pulse_get_config(struct uic_pulse_dev *d,
				 struct uic_pulse_config *cfg)
{
	unsigned long flags;

	spin_lock_irqsave(&d->lock, flags);
	*cfg = d->cfg;
	spin_unlock_irqrestore(&d->lock, flags);
}

static int uic_pulse_start(struct uic_pulse_dev *d)
{
	unsigned long flags;
	int raw;

	if (!d->in)
		return -ENODEV;

	mutex_lock(&d->io_lock);
	spin_lock_irqsave(&d->lock, flags);
	d->started = true;
	uic_pulse_reset_state_locked(d);
	/* Allow the first edge right after start (output often follows immediately). */
	d->t_last_edge = ktime_set(0, 0);
	raw = uic_pulse_in_raw(d);
	if (raw >= 0)
		d->in_active = uic_pulse_raw_is_active(&d->cfg, raw);
	spin_unlock_irqrestore(&d->lock, flags);
	mutex_unlock(&d->io_lock);
	return 0;
}

static void uic_pulse_stop(struct uic_pulse_dev *d)
{
	unsigned long flags;

	mutex_lock(&d->io_lock);
	spin_lock_irqsave(&d->lock, flags);
	d->started = false;
	uic_pulse_reset_state_locked(d);
	spin_unlock_irqrestore(&d->lock, flags);
	uic_pulse_cancel_timers(d);
	mutex_unlock(&d->io_lock);
}

static void uic_pulse_reset(struct uic_pulse_dev *d)
{
	unsigned long flags;

	mutex_lock(&d->io_lock);
	uic_pulse_cancel_timers(d);
	spin_lock_irqsave(&d->lock, flags);
	uic_pulse_reset_state_locked(d);
	spin_unlock_irqrestore(&d->lock, flags);
	mutex_unlock(&d->io_lock);
}

static void uic_pulse_flush(struct uic_pulse_dev *d)
{
	unsigned long flags;

	spin_lock_irqsave(&d->lock, flags);
	kfifo_reset(&d->fifo);
	spin_unlock_irqrestore(&d->lock, flags);
}

static long uic_pulse_ioctl(struct file *file, unsigned int cmd,
			    unsigned long arg)
{
	struct uic_pulse_dev *d = file->private_data;
	struct uic_pulse_config cfg;
	struct uic_pulse_simulate sim;

	switch (cmd) {
	case UIC_PULSE_IOC_SET_CONFIG:
		if (copy_from_user(&cfg, (void __user *)arg, sizeof(cfg)))
			return -EFAULT;
		return uic_pulse_apply_config(d, &cfg);

	case UIC_PULSE_IOC_GET_CONFIG:
		uic_pulse_get_config(d, &cfg);
		if (copy_to_user((void __user *)arg, &cfg, sizeof(cfg)))
			return -EFAULT;
		return 0;

	case UIC_PULSE_IOC_START:
		return uic_pulse_start(d);

	case UIC_PULSE_IOC_STOP:
		uic_pulse_stop(d);
		return 0;

	case UIC_PULSE_IOC_RESET:
		uic_pulse_reset(d);
		return 0;

	case UIC_PULSE_IOC_FLUSH_EVENT:
		uic_pulse_flush(d);
		return 0;

	case UIC_PULSE_IOC_OUTPUT:
		if (copy_from_user(&sim, (void __user *)arg, sizeof(sim)))
			return -EFAULT;
		return uic_pulse_emit(d, &sim);

	default:
		return -ENOTTY;
	}
}

static const struct file_operations uic_pulse_fops = {
	.owner		= THIS_MODULE,
	.open		= uic_pulse_open,
	.read		= uic_pulse_read,
	.poll		= uic_pulse_poll,
	.unlocked_ioctl	= uic_pulse_ioctl,
	.compat_ioctl	= uic_pulse_ioctl,
	.llseek		= no_llseek,
};

static struct uic_pulse_dev *uic_pulse_from_misc_dev(struct device *dev)
{
	struct miscdevice *misc = dev_get_drvdata(dev);

	return container_of(misc, struct uic_pulse_dev, misc);
}

static ssize_t config_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev);
	struct uic_pulse_config cfg;

	uic_pulse_get_config(d, &cfg);
	return sysfs_emit(buf,
		"active_level=%u\nmin_valid_ms=%u\nmax_valid_ms=%u\n"
		"batch_gap_ms=%u\nstuck_timeout_ms=%u\ndebounce_ms=%u\n"
		"irq_edge=%u\nout_active_level=%u\npulse_width_ms=%u\n"
		"interval_ms=%u\n",
		cfg.active_level, cfg.min_valid_ms, cfg.max_valid_ms,
		cfg.batch_gap_ms, cfg.stuck_timeout_ms, cfg.debounce_ms,
		cfg.irq_edge, cfg.out_active_level, cfg.pulse_width_ms,
		cfg.interval_ms);
}

static ssize_t config_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev);
	struct uic_pulse_config cfg;
	int n, ret;

	n = sscanf(buf, "%u %u %u %u %u %u %u %u %u %u",
		   &cfg.active_level, &cfg.min_valid_ms, &cfg.max_valid_ms,
		   &cfg.batch_gap_ms, &cfg.stuck_timeout_ms, &cfg.debounce_ms,
		   &cfg.irq_edge, &cfg.out_active_level, &cfg.pulse_width_ms,
		   &cfg.interval_ms);
	if (n != 10)
		return -EINVAL;
	ret = uic_pulse_apply_config(d, &cfg);
	return ret ? ret : count;
}
static DEVICE_ATTR_RW(config);

#define UIC_PULSE_CFG_ATTR(_field) \
static ssize_t _field##_show(struct device *dev, \
			     struct device_attribute *attr, char *buf) \
{ \
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev); \
	struct uic_pulse_config cfg; \
	uic_pulse_get_config(d, &cfg); \
	return sysfs_emit(buf, "%u\n", cfg._field); \
} \
static ssize_t _field##_store(struct device *dev, \
			      struct device_attribute *attr, \
			      const char *buf, size_t count) \
{ \
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev); \
	struct uic_pulse_config cfg; \
	u32 val; \
	int ret; \
	ret = kstrtou32(buf, 0, &val); \
	if (ret) \
		return ret; \
	uic_pulse_get_config(d, &cfg); \
	cfg._field = val; \
	ret = uic_pulse_apply_config(d, &cfg); \
	return ret ? ret : count; \
} \
static DEVICE_ATTR_RW(_field)

UIC_PULSE_CFG_ATTR(active_level);
UIC_PULSE_CFG_ATTR(min_valid_ms);
UIC_PULSE_CFG_ATTR(max_valid_ms);
UIC_PULSE_CFG_ATTR(batch_gap_ms);
UIC_PULSE_CFG_ATTR(stuck_timeout_ms);
UIC_PULSE_CFG_ATTR(debounce_ms);
UIC_PULSE_CFG_ATTR(irq_edge);
UIC_PULSE_CFG_ATTR(out_active_level);
UIC_PULSE_CFG_ATTR(pulse_width_ms);
UIC_PULSE_CFG_ATTR(interval_ms);

static ssize_t start_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev);
	int ret = uic_pulse_start(d);

	return ret ? ret : count;
}
static DEVICE_ATTR_WO(start);

static ssize_t stop_store(struct device *dev, struct device_attribute *attr,
			  const char *buf, size_t count)
{
	uic_pulse_stop(uic_pulse_from_misc_dev(dev));
	return count;
}
static DEVICE_ATTR_WO(stop);

static ssize_t reset_store(struct device *dev, struct device_attribute *attr,
			   const char *buf, size_t count)
{
	uic_pulse_reset(uic_pulse_from_misc_dev(dev));
	return count;
}
static DEVICE_ATTR_WO(reset);

static ssize_t flush_event_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	uic_pulse_flush(uic_pulse_from_misc_dev(dev));
	return count;
}
static DEVICE_ATTR_WO(flush_event);

static ssize_t output_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev);
	struct uic_pulse_simulate sim = {
		.count = 1,
		.pulse_width_ms = 0,
		.interval_ms = 0,
		.out_active_level = UIC_PULSE_OUT_LEVEL_CFG,
	};
	unsigned int v[4];
	int n, ret;

	n = sscanf(buf, "%u %u %u %u", &v[0], &v[1], &v[2], &v[3]);
	if (n < 1)
		return -EINVAL;
	sim.count = v[0];
	if (n >= 2)
		sim.pulse_width_ms = v[1];
	if (n >= 3)
		sim.interval_ms = v[2];
	if (n >= 4)
		sim.out_active_level = v[3];
	ret = uic_pulse_emit(d, &sim);
	return ret ? ret : count;
}
static DEVICE_ATTR_WO(output);

static ssize_t event_show(struct device *dev, struct device_attribute *attr,
			  char *buf)
{
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev);
	struct uic_pulse_event ev;
	unsigned long flags;
	int n;

	spin_lock_irqsave(&d->lock, flags);
	n = kfifo_out(&d->fifo, &ev, 1);
	spin_unlock_irqrestore(&d->lock, flags);
	if (!n)
		return sysfs_emit(buf, "empty\n");
	return sysfs_emit(buf, "%u %u %llu\n",
			  ev.pulse_count, ev.last_width_ms, ev.timestamp_ns);
}
static DEVICE_ATTR_RO(event);

static ssize_t started_show(struct device *dev, struct device_attribute *attr,
			    char *buf)
{
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev);

	return sysfs_emit(buf, "%u\n", d->started ? 1 : 0);
}
static DEVICE_ATTR_RO(started);

static ssize_t in_raw_show(struct device *dev, struct device_attribute *attr,
			   char *buf)
{
	struct uic_pulse_dev *d = uic_pulse_from_misc_dev(dev);
	int raw;

	raw = uic_pulse_in_raw(d);
	if (raw < 0)
		return raw;
	return sysfs_emit(buf, "%d\n", raw);
}
static DEVICE_ATTR_RO(in_raw);

static struct attribute *uic_pulse_class_attrs[] = {
	&dev_attr_config.attr,
	&dev_attr_active_level.attr,
	&dev_attr_min_valid_ms.attr,
	&dev_attr_max_valid_ms.attr,
	&dev_attr_batch_gap_ms.attr,
	&dev_attr_stuck_timeout_ms.attr,
	&dev_attr_debounce_ms.attr,
	&dev_attr_irq_edge.attr,
	&dev_attr_out_active_level.attr,
	&dev_attr_pulse_width_ms.attr,
	&dev_attr_interval_ms.attr,
	&dev_attr_start.attr,
	&dev_attr_stop.attr,
	&dev_attr_reset.attr,
	&dev_attr_flush_event.attr,
	&dev_attr_output.attr,
	&dev_attr_event.attr,
	&dev_attr_started.attr,
	&dev_attr_in_raw.attr,
	NULL,
};

static const struct attribute_group uic_pulse_group = {
	.attrs = uic_pulse_class_attrs,
};

static const struct attribute_group *uic_pulse_groups[] = {
	&uic_pulse_group,
	NULL,
};

static void uic_pulse_misc_unregister(void *data)
{
	misc_deregister(data);
}

static int uic_pulse_parse_dt(struct device *dev, struct device_node *np,
			      struct uic_pulse_dev *d)
{
	u32 val;

	uic_pulse_default_config(&d->cfg);
	if (!of_property_read_u32(np, "min-width-us", &val))
		d->cfg.min_valid_ms = max(1u, val / 1000);
	if (!of_property_read_u32(np, "max-width-us", &val))
		d->cfg.max_valid_ms = max(1u, val / 1000);
	if (!of_property_read_u32(np, "debounce-us", &val))
		d->cfg.debounce_ms = val / 1000; /* 0 allowed; omit uses DEF 2ms */
	if (!of_property_read_u32(np, "default-emit-us", &val)) {
		d->cfg.pulse_width_ms = val / 1000;
		if (d->cfg.pulse_width_ms < UIC_PULSE_WIDTH_MIN_MS)
			d->cfg.pulse_width_ms = UIC_PULSE_WIDTH_MIN_MS;
		if (d->cfg.pulse_width_ms > UIC_PULSE_WIDTH_MAX_MS)
			d->cfg.pulse_width_ms = UIC_PULSE_WIDTH_MAX_MS;
	}
	if (of_property_read_bool(np, "meig,in-active-high"))
		d->cfg.active_level = UIC_PULSE_ACTIVE_HIGH;
	return 0;
}

static int uic_pulse_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct device_node *child;
	struct uic_pulse_dev *d;
	int ret;

	if (!np)
		return -ENODEV;

	child = of_get_next_available_child(np, NULL);
	if (child)
		np = child;

	d = devm_kzalloc(dev, sizeof(*d), GFP_KERNEL);
	if (!d) {
		of_node_put(child);
		return -ENOMEM;
	}
	d->dev = dev;
	d->irq = -1;
	spin_lock_init(&d->lock);
	mutex_init(&d->io_lock);
	init_waitqueue_head(&d->wq);
	INIT_KFIFO(d->fifo);
	hrtimer_init(&d->debounce_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_init(&d->stuck_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	hrtimer_init(&d->batch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	d->debounce_timer.function = uic_pulse_debounce_fn;
	d->stuck_timer.function = uic_pulse_stuck_fn;
	d->batch_timer.function = uic_pulse_batch_fn;
	uic_pulse_parse_dt(dev, np, d);

	d->in = devm_fwnode_gpiod_get_index(dev, of_fwnode_handle(np), "in",
					    0, GPIOD_IN, UIC_PULSE_DEV_NAME);
	if (IS_ERR(d->in)) {
		ret = PTR_ERR(d->in);
		if (ret != -ENOENT) {
			of_node_put(child);
			return ret;
		}
		d->in = NULL;
	}

	d->out = devm_fwnode_gpiod_get_index(dev, of_fwnode_handle(np), "out",
					     0, GPIOD_OUT_LOW,
					     UIC_PULSE_DEV_NAME);
	if (IS_ERR(d->out)) {
		ret = PTR_ERR(d->out);
		if (ret != -ENOENT) {
			of_node_put(child);
			return ret;
		}
		d->out = NULL;
	}
	of_node_put(child);

	if (!d->in && !d->out) {
		dev_err(dev, "need in-gpios and/or out-gpios\n");
		return -EINVAL;
	}

	if (d->in) {
		if (gpiod_cansleep(d->in)) {
			dev_err(dev, "input GPIO must be MMIO\n");
			return -EINVAL;
		}
			d->irq = gpiod_to_irq(d->in);
		if (d->irq < 0)
			return d->irq;
		ret = devm_request_irq(dev, d->irq, uic_pulse_isr,
					 uic_pulse_irq_flags(d->cfg.irq_edge),
					 UIC_PULSE_DEV_NAME, d);
		if (ret)
			return ret;
	}

	if (d->out)
		uic_pulse_set_out_raw(d,
				      uic_pulse_out_idle_raw(d->cfg.out_active_level));

	d->misc.minor = MISC_DYNAMIC_MINOR;
	d->misc.name = UIC_PULSE_DEV_NAME;
	d->misc.fops = &uic_pulse_fops;
	d->misc.parent = dev;
	d->misc.groups = uic_pulse_groups;
	ret = misc_register(&d->misc);
	if (ret)
		return ret;
	ret = devm_add_action_or_reset(dev, uic_pulse_misc_unregister, &d->misc);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, d);
	dev_info(dev,
		 "/dev/%s in=%d out=%d irq=%d in_al=%u out_al=%u debounce=%ums\n",
		 UIC_PULSE_DEV_NAME,
		 d->in ? desc_to_gpio(d->in) : -1,
		 d->out ? desc_to_gpio(d->out) : -1,
		 d->irq, d->cfg.active_level, d->cfg.out_active_level,
		 d->cfg.debounce_ms);
	return 0;
}

static int uic_pulse_remove(struct platform_device *pdev)
{
	struct uic_pulse_dev *d = platform_get_drvdata(pdev);

	d->started = false;
	uic_pulse_cancel_timers(d);
	return 0;
}

static const struct of_device_id uic_pulse_of_match[] = {
	{ .compatible = "meig,gpio-pulse" },
	{ }
};
MODULE_DEVICE_TABLE(of, uic_pulse_of_match);

static struct platform_driver uic_pulse_driver = {
	.probe = uic_pulse_probe,
	.remove = uic_pulse_remove,
	.driver = {
		.name = "meig_gpio_pulse",
		.of_match_table = uic_pulse_of_match,
	},
};
module_platform_driver(uic_pulse_driver);

MODULE_DESCRIPTION("UIC GPIO pulse character device");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("wangguanran");
