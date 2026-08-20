#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#define DRIVER_NAME "cash_drawer"
#define DEVICE_NAME "cash_drawer"
#define PULSE_WIDTH_MS 250
struct cash_drawer_platform_data {
	int gpio_24v_en;
	int gpio_12v_en;
	int gpio_rj232_en;
	int gpio_cd1_en;
	int gpio_cd2_en;
	int gpio_cd_det;

	int pulse_width_ms;
};

struct cash_drawer_device {
	struct cdev cdev;
	dev_t devno;
	struct device *device;
	struct cash_drawer_platform_data *pdata;
};

static struct cash_drawer_device *cash_drawer_dev;
static int cd_det_irq;
static struct class *cash_drawer_class;
static struct input_dev *input_dev;

static struct cash_drawer_platform_data *cash_drawer_pdata(struct device *dev)
{
	return dev_get_drvdata(dev);
}

static ssize_t set_cd1_en(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	unsigned long value;
	int ret;

	if (!pdata || !gpio_is_valid(pdata->gpio_cd1_en))
		return -ENODEV;

	ret = kstrtoul(buf, 10, &value);
	if (ret)
		return ret;

	if (value) {
		gpio_direction_output(pdata->gpio_cd1_en, 1);
		msleep(pdata->pulse_width_ms);
		gpio_direction_output(pdata->gpio_cd1_en, 0);
		dev_info(dev, "CD1_EN high pulse sent (active high)\n");
	} else {
		gpio_direction_output(pdata->gpio_cd1_en, 0);
	}
	return count;
}

static DEVICE_ATTR(cd1_en, 0200, NULL, set_cd1_en);

static ssize_t set_cd2_en(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	unsigned long value;
	int ret;

	if (!pdata || !gpio_is_valid(pdata->gpio_cd2_en))
		return -ENODEV;

	ret = kstrtoul(buf, 10, &value);
	if (ret)
		return ret;

	/* Same active-high polarity as CD1: pulse high then low; 0 keeps low */
	if (value) {
		gpio_direction_output(pdata->gpio_cd2_en, 1);
		msleep(pdata->pulse_width_ms);
		gpio_direction_output(pdata->gpio_cd2_en, 0);
		dev_info(dev, "CD2_EN high pulse sent (active high)\n");
	} else {
		gpio_direction_output(pdata->gpio_cd2_en, 0);
	}

	return count;
}

static DEVICE_ATTR(cd2_en, 0200, NULL, set_cd2_en);

static ssize_t show_12v_en(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	int value = 0;

	if (pdata && gpio_is_valid(pdata->gpio_12v_en))
		value = gpio_get_value(pdata->gpio_12v_en);

	return sysfs_emit(buf, "%d\n", value);
}

static ssize_t set_12v_en(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	unsigned long value;
	int ret;

	if (!pdata || !gpio_is_valid(pdata->gpio_12v_en))
		return -ENODEV;

	ret = kstrtoul(buf, 10, &value);
	if (ret)
		return ret;

	gpio_direction_output(pdata->gpio_12v_en, value ? 1 : 0);
	dev_info(dev, "12V_EN set to %lu\n", value);
	return count;
}

static DEVICE_ATTR(12v_en, 0644, show_12v_en, set_12v_en);

static ssize_t show_24v_en(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	int value = 0;

	if (pdata && gpio_is_valid(pdata->gpio_24v_en))
		value = gpio_get_value(pdata->gpio_24v_en);

	return sysfs_emit(buf, "%d\n", value);
}

static ssize_t set_24v_en(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	unsigned long value;
	int ret;

	if (!pdata || !gpio_is_valid(pdata->gpio_24v_en))
		return -ENODEV;

	ret = kstrtoul(buf, 10, &value);
	if (ret)
		return ret;

	gpio_direction_output(pdata->gpio_24v_en, value ? 1 : 0);
	dev_info(dev, "24V_EN set to %lu\n", value);
	return count;
}

static DEVICE_ATTR(24v_en, 0644, show_24v_en, set_24v_en);

static ssize_t show_rj232_en(struct device *dev,
			     struct device_attribute *attr, char *buf)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	int value = 0;

	if (pdata && gpio_is_valid(pdata->gpio_rj232_en))
		value = gpio_get_value(pdata->gpio_rj232_en);

	return sysfs_emit(buf, "%d\n", value);
}

static ssize_t set_rj232_en(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf, size_t count)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	unsigned long value;
	int ret;

	if (!pdata || !gpio_is_valid(pdata->gpio_rj232_en))
		return -ENODEV;

	ret = kstrtoul(buf, 10, &value);
	if (ret)
		return ret;

	gpio_direction_output(pdata->gpio_rj232_en, value ? 1 : 0);
	dev_info(dev, "RJ232_EN set to %lu\n", value);
	return count;
}

static DEVICE_ATTR(rj232_en, 0644, show_rj232_en, set_rj232_en);

static ssize_t show_voltage_status(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	int voltage_12v = 0;
	int voltage_24v = 0;
	const char *voltage_str;

	if (pdata) {
		if (gpio_is_valid(pdata->gpio_12v_en))
			voltage_12v = gpio_get_value(pdata->gpio_12v_en);
		if (gpio_is_valid(pdata->gpio_24v_en))
			voltage_24v = gpio_get_value(pdata->gpio_24v_en);
	}

	if (voltage_24v && !voltage_12v)
		voltage_str = "24V";
	else if (voltage_12v && !voltage_24v)
		voltage_str = "12V";
	else if (voltage_12v && voltage_24v)
		voltage_str = "ERROR (both 12V and 24V enabled)";
	else
		voltage_str = "0V (both disabled)";

	return sysfs_emit(buf, "Current voltage: %s\n12V_EN: %d\n24V_EN: %d\n",
			  voltage_str, voltage_12v, voltage_24v);
}

static DEVICE_ATTR(voltage_status, 0444, show_voltage_status, NULL);

static ssize_t show_cd_det(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	int value = 0;

	if (pdata && gpio_is_valid(pdata->gpio_cd_det))
		value = gpio_get_value(pdata->gpio_cd_det);

	return sysfs_emit(buf, "%d\n", value);
}

static DEVICE_ATTR(cd_det, 0444, show_cd_det, NULL);

static int cash_drawer_open(struct inode *inode, struct file *filp)
{
	filp->private_data = cash_drawer_dev;
	return 0;
}

static int cash_drawer_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static const struct file_operations cash_drawer_fops = {
	.owner = THIS_MODULE,
	.open = cash_drawer_open,
	.release = cash_drawer_release,
};

static irqreturn_t cd_det_interrupt(int irq, void *dev_id)
{
	struct cash_drawer_platform_data *pdata = dev_id;
	int det_value;

	if (!pdata || !gpio_is_valid(pdata->gpio_cd_det))
		return IRQ_NONE;

	det_value = gpio_get_value(pdata->gpio_cd_det);

	if (input_dev) {
		input_report_key(input_dev, KEY_PROG1, det_value ? 0 : 1);
		input_sync(input_dev);
		if (input_dev->dev.parent)
			dev_dbg(input_dev->dev.parent,
				"Cash drawer %s\n",
				det_value ? "close" : "open");
	}

	return IRQ_HANDLED;
}

static void cash_drawer_free_gpios(struct cash_drawer_platform_data *pdata)
{
	if (!pdata)
		return;

	if (gpio_is_valid(pdata->gpio_cd_det))
		gpio_free(pdata->gpio_cd_det);
	if (gpio_is_valid(pdata->gpio_cd2_en))
		gpio_free(pdata->gpio_cd2_en);
	if (gpio_is_valid(pdata->gpio_cd1_en))
		gpio_free(pdata->gpio_cd1_en);
	if (gpio_is_valid(pdata->gpio_rj232_en))
		gpio_free(pdata->gpio_rj232_en);
	if (gpio_is_valid(pdata->gpio_12v_en))
		gpio_free(pdata->gpio_12v_en);
	if (gpio_is_valid(pdata->gpio_24v_en))
		gpio_free(pdata->gpio_24v_en);

	pdata->gpio_24v_en = -EINVAL;
	pdata->gpio_12v_en = -EINVAL;
	pdata->gpio_rj232_en = -EINVAL;
	pdata->gpio_cd1_en = -EINVAL;
	pdata->gpio_cd2_en = -EINVAL;
	pdata->gpio_cd_det = -EINVAL;
}

static int parse_device_tree(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct cash_drawer_platform_data *pdata;
	int ret;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	pdata->gpio_24v_en = -EINVAL;
	pdata->gpio_12v_en = -EINVAL;
	pdata->gpio_rj232_en = -EINVAL;
	pdata->gpio_cd1_en = -EINVAL;
	pdata->gpio_cd2_en = -EINVAL;
	pdata->gpio_cd_det = -EINVAL;

	pdata->gpio_24v_en = of_get_named_gpio(np, "gpio_24v_en", 0);
	if (!gpio_is_valid(pdata->gpio_24v_en)) {
		dev_err(dev, "Failed to get 24v_en GPIO\n");
		return -EINVAL;
	}
	ret = gpio_request(pdata->gpio_24v_en, "gpio_24v_en");
	if (ret < 0) {
		dev_err(dev, "Request gpio_24v_en GPIO failed, ret = %d\n", ret);
		pdata->gpio_24v_en = -EINVAL;
		goto err_free_gpios;
	}

	pdata->gpio_12v_en = of_get_named_gpio(np, "gpio_12v_en", 0);
	if (!gpio_is_valid(pdata->gpio_12v_en)) {
		dev_err(dev, "Failed to get 12v_en GPIO\n");
		ret = -EINVAL;
		goto err_free_gpios;
	}
	ret = gpio_request(pdata->gpio_12v_en, "gpio_12v_en");
	if (ret < 0) {
		dev_err(dev, "Request gpio_12v_en GPIO failed, ret = %d\n", ret);
		pdata->gpio_12v_en = -EINVAL;
		goto err_free_gpios;
	}

	pdata->gpio_rj232_en = of_get_named_gpio(np, "gpio_rj232_en", 0);
	if (!gpio_is_valid(pdata->gpio_rj232_en)) {
		dev_err(dev, "Failed to get gpio_rj232_en GPIO\n");
		ret = -EINVAL;
		goto err_free_gpios;
	}
	ret = gpio_request(pdata->gpio_rj232_en, "gpio_rj232_en");
	if (ret < 0) {
		dev_err(dev, "Request gpio_rj232_en GPIO failed, ret = %d\n", ret);
		pdata->gpio_rj232_en = -EINVAL;
		goto err_free_gpios;
	}

	pdata->gpio_cd1_en = of_get_named_gpio(np, "gpio_cd1_en", 0);
	if (!gpio_is_valid(pdata->gpio_cd1_en)) {
		dev_err(dev, "Failed to get gpio_cd1_en GPIO\n");
		ret = -EINVAL;
		goto err_free_gpios;
	}
	ret = gpio_request(pdata->gpio_cd1_en, "gpio_cd1_en");
	if (ret < 0) {
		dev_err(dev, "Request gpio_cd1_en GPIO failed, ret = %d\n", ret);
		pdata->gpio_cd1_en = -EINVAL;
		goto err_free_gpios;
	}

	pdata->gpio_cd2_en = of_get_named_gpio(np, "gpio_cd2_en", 0);
	if (!gpio_is_valid(pdata->gpio_cd2_en)) {
		dev_err(dev, "Failed to get gpio_cd2_en GPIO\n");
		ret = -EINVAL;
		goto err_free_gpios;
	}
	ret = gpio_request(pdata->gpio_cd2_en, "gpio_cd2_en");
	if (ret < 0) {
		dev_err(dev, "Request gpio_cd2_en GPIO failed, ret = %d\n", ret);
		pdata->gpio_cd2_en = -EINVAL;
		goto err_free_gpios;
	}

	pdata->gpio_cd_det = of_get_named_gpio(np, "gpio_cd_det", 0);
	if (!gpio_is_valid(pdata->gpio_cd_det)) {
		dev_err(dev, "Failed to get gpio_cd_det GPIO\n");
		ret = -EINVAL;
		goto err_free_gpios;
	}
	ret = gpio_request(pdata->gpio_cd_det, "gpio_cd_det");
	if (ret < 0) {
		dev_err(dev, "Request gpio_cd_det GPIO failed, ret = %d\n", ret);
		pdata->gpio_cd_det = -EINVAL;
		goto err_free_gpios;
	}
	gpio_direction_input(pdata->gpio_cd_det);

	ret = of_property_read_u32(dev->of_node, "pulse-width-ms",
				   &pdata->pulse_width_ms);
	if (ret) {
		dev_warn(dev, "No pulse-width-ms specified, using default %dms\n",
			 PULSE_WIDTH_MS);
		pdata->pulse_width_ms = PULSE_WIDTH_MS;
	}

	dev_set_drvdata(dev, pdata);
	dev_info(dev, "Cash drawer platform data parsed successfully\n");
	return 0;

err_free_gpios:
	cash_drawer_free_gpios(pdata);
	return ret;
}

static int create_cash_drawer_device(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cash_drawer_device *cdev;
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(dev);
	dev_t devno;
	int ret;

	cdev = devm_kzalloc(dev, sizeof(*cdev), GFP_KERNEL);
	if (!cdev)
		return -ENOMEM;

	ret = alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME);
	if (ret < 0) {
		dev_err(dev, "Failed to allocate device number\n");
		return ret;
	}

	cdev->devno = devno;
	cdev->pdata = pdata;

	cdev_init(&cdev->cdev, &cash_drawer_fops);
	cdev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&cdev->cdev, devno, 1);
	if (ret) {
		dev_err(dev, "Failed to add character device\n");
		unregister_chrdev_region(devno, 1);
		return ret;
	}

	cdev->device = device_create(cash_drawer_class, dev, devno, cdev,
				     DEVICE_NAME);
	if (IS_ERR(cdev->device)) {
		ret = PTR_ERR(cdev->device);
		dev_err(dev, "Failed to create device node\n");
		cdev_del(&cdev->cdev);
		unregister_chrdev_region(devno, 1);
		return ret;
	}

	cash_drawer_dev = cdev;
	dev_info(dev, "Cash drawer character device created\n");
	return 0;
}

static int create_control_nodes(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	ret = device_create_file(dev, &dev_attr_cd1_en);
	if (ret) {
		dev_err(dev, "Failed to create cd1_en attribute\n");
		return ret;
	}

	ret = device_create_file(dev, &dev_attr_cd2_en);
	if (ret) {
		dev_err(dev, "Failed to create cd2_en attribute\n");
		device_remove_file(dev, &dev_attr_cd1_en);
		return ret;
	}

	ret = device_create_file(dev, &dev_attr_12v_en);
	if (ret) {
		dev_err(dev, "Failed to create 12v_en attribute\n");
		device_remove_file(dev, &dev_attr_cd1_en);
		device_remove_file(dev, &dev_attr_cd2_en);
		return ret;
	}

	ret = device_create_file(dev, &dev_attr_24v_en);
	if (ret) {
		dev_err(dev, "Failed to create 24v_en attribute\n");
		device_remove_file(dev, &dev_attr_cd1_en);
		device_remove_file(dev, &dev_attr_cd2_en);
		device_remove_file(dev, &dev_attr_12v_en);
		return ret;
	}

	ret = device_create_file(dev, &dev_attr_rj232_en);
	if (ret) {
		dev_err(dev, "Failed to create rj232_en attribute\n");
		device_remove_file(dev, &dev_attr_cd1_en);
		device_remove_file(dev, &dev_attr_cd2_en);
		device_remove_file(dev, &dev_attr_12v_en);
		device_remove_file(dev, &dev_attr_24v_en);
		return ret;
	}

	ret = device_create_file(dev, &dev_attr_voltage_status);
	if (ret) {
		dev_err(dev, "Failed to create voltage_status attribute\n");
		device_remove_file(dev, &dev_attr_cd1_en);
		device_remove_file(dev, &dev_attr_cd2_en);
		device_remove_file(dev, &dev_attr_12v_en);
		device_remove_file(dev, &dev_attr_24v_en);
		device_remove_file(dev, &dev_attr_rj232_en);
		return ret;
	}

	ret = device_create_file(dev, &dev_attr_cd_det);
	if (ret) {
		dev_err(dev, "Failed to create cd_det attribute\n");
		device_remove_file(dev, &dev_attr_cd1_en);
		device_remove_file(dev, &dev_attr_cd2_en);
		device_remove_file(dev, &dev_attr_12v_en);
		device_remove_file(dev, &dev_attr_24v_en);
		device_remove_file(dev, &dev_attr_rj232_en);
		device_remove_file(dev, &dev_attr_voltage_status);
		return ret;
	}

	dev_info(dev, "All control nodes created successfully\n");
	return 0;
}

static void remove_control_nodes(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	device_remove_file(dev, &dev_attr_cd1_en);
	device_remove_file(dev, &dev_attr_cd2_en);
	device_remove_file(dev, &dev_attr_12v_en);
	device_remove_file(dev, &dev_attr_24v_en);
	device_remove_file(dev, &dev_attr_rj232_en);
	device_remove_file(dev, &dev_attr_voltage_status);
	device_remove_file(dev, &dev_attr_cd_det);
}

static int create_input_device(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	input_dev = input_allocate_device();
	if (!input_dev) {
		dev_err(dev, "Failed to allocate input device\n");
		return -ENOMEM;
	}

	input_dev->name = "Cash Drawer Detector";
	input_dev->phys = "cash_drawer/input0";
	input_dev->id.bustype = BUS_HOST;
	input_dev->dev.parent = &pdev->dev;

	set_bit(EV_KEY, input_dev->evbit);
	set_bit(KEY_PROG1, input_dev->keybit);

	ret = input_register_device(input_dev);
	if (ret) {
		dev_err(dev, "Failed to register input device\n");
		input_free_device(input_dev);
		input_dev = NULL;
		return ret;
	}

	dev_info(dev, "Cash drawer input device created\n");
	return 0;
}

static int cash_drawer_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cash_drawer_platform_data *pdata;
	int ret;

	dev_info(dev, "Cash drawer driver probing\n");

	ret = parse_device_tree(pdev);
	if (ret) {
		dev_err(dev, "Failed to parse device tree\n");
		return ret;
	}
	pdata = cash_drawer_pdata(dev);

	cash_drawer_class = class_create(DRIVER_NAME);
	if (IS_ERR(cash_drawer_class)) {
		dev_err(dev, "Failed to create device class\n");
		cash_drawer_free_gpios(pdata);
		return PTR_ERR(cash_drawer_class);
	}

	ret = create_control_nodes(pdev);
	if (ret) {
		class_destroy(cash_drawer_class);
		cash_drawer_free_gpios(pdata);
		return ret;
	}

	ret = create_input_device(pdev);
	if (ret) {
		remove_control_nodes(pdev);
		class_destroy(cash_drawer_class);
		cash_drawer_free_gpios(pdata);
		return ret;
	}

	cd_det_irq = gpio_to_irq(pdata->gpio_cd_det);
	if (cd_det_irq < 0) {
		dev_err(dev, "Failed to get IRQ for cd_det GPIO\n");
		ret = cd_det_irq;
		goto err_irq;
	}

	ret = request_irq(cd_det_irq, cd_det_interrupt,
			  IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			  "cash_drawer_det", pdata);
	if (ret) {
		dev_err(dev, "Failed to request interrupt for cd_det GPIO\n");
		goto err_irq;
	}

	dev_info(dev, "Registered interrupt %d for cash drawer detection\n",
		 cd_det_irq);

	ret = create_cash_drawer_device(pdev);
	if (ret) {
		dev_err(dev, "Failed to create cash drawer device\n");
		goto err_device;
	}

	dev_info(dev, "Cash drawer driver probed successfully\n");
	return 0;

err_device:
	free_irq(cd_det_irq, pdata);
	cd_det_irq = 0;
err_irq:
	remove_control_nodes(pdev);
	if (input_dev) {
		input_unregister_device(input_dev);
		input_dev = NULL;
	}
	class_destroy(cash_drawer_class);
	cash_drawer_class = NULL;
	cash_drawer_free_gpios(pdata);
	return ret;
}

static void cash_drawer_remove(struct platform_device *pdev)
{
	struct cash_drawer_platform_data *pdata = cash_drawer_pdata(&pdev->dev);

	dev_info(&pdev->dev, "Removing cash drawer driver\n");

	if (cash_drawer_dev) {
		device_destroy(cash_drawer_class, cash_drawer_dev->devno);
		cdev_del(&cash_drawer_dev->cdev);
		unregister_chrdev_region(cash_drawer_dev->devno, 1);
		cash_drawer_dev = NULL;
	}

	remove_control_nodes(pdev);

	if (input_dev) {
		input_unregister_device(input_dev);
		input_dev = NULL;
	}

	if (cash_drawer_class) {
		class_destroy(cash_drawer_class);
		cash_drawer_class = NULL;
	}

	if (cd_det_irq > 0) {
		free_irq(cd_det_irq, pdata);
		cd_det_irq = 0;
	}

	cash_drawer_free_gpios(pdata);
	dev_set_drvdata(&pdev->dev, NULL);

	dev_info(&pdev->dev, "Cash drawer driver removed\n");
}

static const struct of_device_id cash_drawer_of_match[] = {
	{ .compatible = "vendor,cash-drawer", },
	{ },
};
MODULE_DEVICE_TABLE(of, cash_drawer_of_match);

static struct platform_driver cash_drawer_driver = {
	.probe = cash_drawer_probe,
	.remove = cash_drawer_remove,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = cash_drawer_of_match,
		.owner = THIS_MODULE,
	},
};

static int __init cash_drawer_init(void)
{
	pr_info("%s: init\n", __func__);
	return platform_driver_register(&cash_drawer_driver);
}

static void __exit cash_drawer_exit(void)
{
	platform_driver_unregister(&cash_drawer_driver);
}

module_init(cash_drawer_init);
module_exit(cash_drawer_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("MeiG cash drawer GPIO driver");
MODULE_AUTHOR("MeiG");
