// SPDX-License-Identifier: GPL-2.0-only
/*
 * SGM832 power monitor (ported from A14 msm-5.4)
 */
#include <linux/pinctrl/consumer.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/extcon.h>
#include <linux/notifier.h>
#include <linux/power_supply.h>

struct sgm832_data {
	struct i2c_client *client;
	struct delayed_work delay_work;
	struct kobject *bl_kobj;
	int irq_gpio;
	bool init_done;
	struct power_supply *usb_psy;
};

static int swapEndianness(int value)
{
	return ((value & 0xff) << 8) | ((value >> 8) & 0xff);
}

static void delaywork_func(struct work_struct *work)
{
	struct sgm832_data *data = container_of(to_delayed_work(work),
						struct sgm832_data, delay_work);
	int buf;

	buf = i2c_smbus_read_word_data(data->client, 0x01);
	if (buf < 0) {
		dev_err(&data->client->dev, "read reg 0x01 failed: %d\n", buf);
		return;
	}
	dev_dbg(&data->client->dev, "01 is 0x%x, 0x%x\n",
		buf, swapEndianness(buf));

	buf = i2c_smbus_read_word_data(data->client, 0xfe);
	if (buf < 0) {
		dev_err(&data->client->dev, "read reg 0xfe failed: %d\n", buf);
		return;
	}
	dev_dbg(&data->client->dev, "power 0xfe is 0x%x, 0x%x\n",
		buf, swapEndianness(buf));

	buf = i2c_smbus_read_word_data(data->client, 0xff);
	if (buf < 0) {
		dev_err(&data->client->dev, "read reg 0xff failed: %d\n", buf);
		return;
	}
	dev_dbg(&data->client->dev, "power 0xff is 0x%x, 0x%x\n",
		buf, swapEndianness(buf));

	buf = i2c_smbus_read_word_data(data->client, 0x05);
	if (buf < 0) {
		dev_err(&data->client->dev, "read reg 0x05 failed: %d\n", buf);
		return;
	}
	dev_dbg(&data->client->dev, "05 is 0x%x, 0x%x\n",
		buf, swapEndianness(buf));
}

static ssize_t sgm832_current_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct sgm832_data *data = dev_get_drvdata(dev);
	int buf_current;

	if (!data || !data->client)
		return -ENODEV;

	buf_current = i2c_smbus_read_word_data(data->client, 0x04);
	if (buf_current < 0) {
		dev_err(dev, "read current reg 0x04 failed: %d\n", buf_current);
		return buf_current;
	}

	buf_current = swapEndianness(buf_current);
	dev_dbg(dev, "current 0x04:0x%x\n", buf_current);
	buf_current = (buf_current * 4900) / 32768;

	return sysfs_emit(buf, "%d\n", buf_current);
}

static ssize_t sgm832_power_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct sgm832_data *data = dev_get_drvdata(dev);
	int buf_power;

	if (!data || !data->client)
		return -ENODEV;

	buf_power = i2c_smbus_read_word_data(data->client, 0x03);
	if (buf_power < 0) {
		dev_err(dev, "read power reg 0x03 failed: %d\n", buf_power);
		return buf_power;
	}

	buf_power = swapEndianness(buf_power);
	dev_dbg(dev, "power 0x03:0x%x\n", buf_power);
	/* 122500 * ~0xFFFF overflows 32-bit int; use 64-bit math */
	buf_power = (int)(((long long)buf_power * 122500) / 32768);

	return sysfs_emit(buf, "%d\n", buf_power);
}

static ssize_t sgm832_vbus_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	struct sgm832_data *data = dev_get_drvdata(dev);
	int buf_vbus;

	if (!data || !data->client)
		return -ENODEV;

	buf_vbus = i2c_smbus_read_word_data(data->client, 0x02);
	if (buf_vbus < 0) {
		dev_err(dev, "read vbus reg 0x02 failed: %d\n", buf_vbus);
		return buf_vbus;
	}

	buf_vbus = swapEndianness(buf_vbus);
	/* A14 used float *1.25; keep integer math for kernel */
	buf_vbus = (buf_vbus * 5) / 4;

	return sysfs_emit(buf, "%d\n", buf_vbus);
}

static struct device_attribute sgm832_dev_attr[] = {
	__ATTR(sgm_current, 0444, sgm832_current_show, NULL),
	__ATTR(sgm_power, 0444, sgm832_power_show, NULL),
	__ATTR(sgm_vbus, 0444, sgm832_vbus_show, NULL),
};

static int sgm832_create_sysfs(struct device *dev)
{
	int result = 0, num_attr, i;

	num_attr = ARRAY_SIZE(sgm832_dev_attr);
	for (i = 0; i < num_attr; i++) {
		result = device_create_file(dev, &sgm832_dev_attr[i]);
		if (result < 0) {
			while (--i >= 0)
				device_remove_file(dev, &sgm832_dev_attr[i]);
			return result;
		}
	}

	return 0;
}

static void sgm832_remove_sysfs(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(sgm832_dev_attr); i++)
		device_remove_file(dev, &sgm832_dev_attr[i]);
}

static irqreturn_t sgm832_irq_handler(int irq, void *dev_id)
{
	struct sgm832_data *data = dev_id;

	if (!data || !data->client)
		return IRQ_NONE;

	disable_irq_nosync(irq);
	dev_dbg(&data->client->dev, "sgm832_irq_handler\n");
	schedule_delayed_work(&data->delay_work, msecs_to_jiffies(500));
	enable_irq(irq);
	return IRQ_HANDLED;
}

static int sgm832_request_irq(struct sgm832_data *data)
{
	int ret = -EINVAL;

	if (gpio_is_valid(data->irq_gpio) || data->client->irq > 0) {
		if (gpio_is_valid(data->irq_gpio))
			data->client->irq = gpio_to_irq(data->irq_gpio);

		ret = request_threaded_irq(data->client->irq, NULL,
					   sgm832_irq_handler,
					   IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
					   data->client->name,
					   data);
		if (ret < 0) {
			dev_err(&data->client->dev,
				"Failed to request irq %d\n", data->client->irq);
			return ret;
		}
		return 0;
	}

	return ret;
}

static int sgm832_parse_dt(struct device *dev, struct sgm832_data *data)
{
	int ret;
	struct device_node *np = dev->of_node;

	data->irq_gpio = of_get_named_gpio(np, "sgm832,irq-gpio", 0);
	if (!gpio_is_valid(data->irq_gpio)) {
		dev_err(dev, "invalid irq-gpio\n");
		return -EINVAL;
	}

	ret = gpio_request(data->irq_gpio, "irq-gpio");
	if (ret < 0) {
		dev_err(dev, "request irq-gpio failed: %d\n", ret);
		return ret;
	}

	gpio_direction_input(data->irq_gpio);
	return 0;
}

static void sgm832_shutdown(struct i2c_client *client)
{
	struct sgm832_data *data = i2c_get_clientdata(client);

	if (!data || !data->init_done)
		return;

	cancel_delayed_work_sync(&data->delay_work);
}

static int sgm832_probe(struct i2c_client *client)
{
	struct sgm832_data *data;
	int ret;

	dev_info(&client->dev, "sgm832_probe start addr=0x%x\n", client->addr);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "Failed check I2C functionality");
		return -ENODEV;
	}

	data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->init_done = false;
	data->irq_gpio = -EINVAL;
	data->client = client;
	i2c_set_clientdata(client, data);

	if (client->dev.of_node) {
		ret = sgm832_parse_dt(&client->dev, data);
		if (ret) {
			dev_err(&client->dev, "Failed parse dts\n");
			goto err_clear_drvdata;
		}
	}

	ret = i2c_smbus_write_word_data(data->client, 0x05, 0x550d);
	if (ret < 0) {
		dev_err(&client->dev, "write reg 0x05 failed: %d\n", ret);
		goto err_free_gpio;
	}

	INIT_DELAYED_WORK(&data->delay_work, delaywork_func);

	ret = sgm832_request_irq(data);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to request irq\n");
		goto err_free_gpio;
	}

	ret = sgm832_create_sysfs(&client->dev);
	if (ret < 0) {
		dev_err(&client->dev, "Cannot create sysfs\n");
		goto err_free_irq;
	}

	/* Start delayed work only after all resources are ready */
	schedule_delayed_work(&data->delay_work, msecs_to_jiffies(5000));

	data->init_done = true;
	dev_info(&client->dev, "sgm832_probe success\n");
	return 0;

err_free_irq:
	if (data->client->irq > 0)
		free_irq(data->client->irq, data);
err_free_gpio:
	if (gpio_is_valid(data->irq_gpio))
		gpio_free(data->irq_gpio);
err_clear_drvdata:
	i2c_set_clientdata(client, NULL);
	return ret;
}

static void sgm832_remove(struct i2c_client *client)
{
	struct sgm832_data *data = i2c_get_clientdata(client);

	if (!data)
		return;

	cancel_delayed_work_sync(&data->delay_work);
	sgm832_remove_sysfs(&client->dev);

	if (data->client->irq > 0)
		free_irq(data->client->irq, data);

	if (gpio_is_valid(data->irq_gpio))
		gpio_free(data->irq_gpio);

	i2c_set_clientdata(client, NULL);
	dev_info(&client->dev, "sgm832 driver removed\n");
}

static const struct of_device_id sgm832_match_table[] = {
	{.compatible = "sgm832",},
	{ },
};
MODULE_DEVICE_TABLE(of, sgm832_match_table);

static struct i2c_driver sgm832_driver = {
	.probe		= sgm832_probe,
	.remove		= sgm832_remove,
	.shutdown	= sgm832_shutdown,
	.driver = {
		.name	  = "sgm832",
		.owner	  = THIS_MODULE,
		.of_match_table = sgm832_match_table,
	},
};

static int __init sgm832_init(void)
{
	pr_info("sgm832 driver installing..\n");
	return i2c_add_driver(&sgm832_driver);
}

static void __exit sgm832_exit(void)
{
	pr_info("sgm832 driver exited\n");
	i2c_del_driver(&sgm832_driver);
}

late_initcall(sgm832_init);
module_exit(sgm832_exit);

MODULE_DESCRIPTION("SGM832 power monitor");
MODULE_LICENSE("GPL");
