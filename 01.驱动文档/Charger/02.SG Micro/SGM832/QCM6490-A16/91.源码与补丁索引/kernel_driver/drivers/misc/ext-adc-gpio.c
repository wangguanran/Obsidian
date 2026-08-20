#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/leds.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/input/mt.h>
#include <linux/iio/consumer.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#define DRIVER_NAME "ext-adc-driver"
#define ADC_NUM_CHANNELS 2
#define ADC_POLL_INTERVAL_MS 500  // 1秒采样间隔

struct ext_adc_data {
    struct device *dev;
    struct iio_channel *adc_channels[ADC_NUM_CHANNELS];
    int num_channels;
    const char *channel_names[ADC_NUM_CHANNELS];
    struct delayed_work poll_work;
    struct kobject *kobj;
    struct mutex lock;
    int values[ADC_NUM_CHANNELS];
    bool enabled;
};

static int ext_adc_read_channel(struct ext_adc_data *adc_data, int channel)
{
    int ret = 0;
    int val = 0;
    
    if (channel >= adc_data->num_channels || !adc_data->adc_channels[channel]) {
        dev_err(adc_data->dev, "Invalid channel %d\n", channel);
        return -EINVAL;
    }

    dev_dbg(adc_data->dev, "Reading ADC channel %d\n", channel);

    ret = iio_read_channel_processed(adc_data->adc_channels[channel], &val);
    if (ret >= 0) {
        dev_dbg(adc_data->dev, "Channel %d processed value: %d uV\n", channel, val);
        return val;
    }

    dev_err(adc_data->dev, "Both processed and raw reads failed for channel %d: %d\n", 
            channel, ret);

    return ret;
}

// sysfs属性
static ssize_t adc_value_show(struct device *dev,
                             struct device_attribute *attr,
                             char *buf)
{
    struct ext_adc_data *adc_data = dev_get_drvdata(dev);
    const char *attr_name;
    int channel = 0;
    int val;

    if (!adc_data)
        return -ENODEV;

    attr_name = attr->attr.name;
    if (strstr(attr_name, "adc0"))
        channel = 0;
    else if (strstr(attr_name, "adc1"))
        channel = 1;
    
    if (channel >= adc_data->num_channels)
        return -EINVAL;
    
    mutex_lock(&adc_data->lock);
    val = ext_adc_read_channel(adc_data,channel);
    mutex_unlock(&adc_data->lock);
    
    if (val < 0) {
        return scnprintf(buf, PAGE_SIZE, "Error: %d\n", val);
    }

    return scnprintf(buf, PAGE_SIZE, "%d\n", val);
}

static ssize_t adc_enable_store(struct device *dev,
                               struct device_attribute *attr,
                               const char *buf, size_t count)
{
    struct ext_adc_data *adc_data = dev_get_drvdata(dev);
    bool enable;
    int ret;

    if (!adc_data)
        return -ENODEV;
    
    ret = kstrtobool(buf, &enable);
    if (ret)
        return ret;
    
    mutex_lock(&adc_data->lock);
    
    if (enable && !adc_data->enabled) {
        adc_data->enabled = true;
        schedule_delayed_work(&adc_data->poll_work,
                            msecs_to_jiffies(ADC_POLL_INTERVAL_MS));
        dev_info(dev, "ADC polling enabled\n");
    } else if (!enable && adc_data->enabled) {
        adc_data->enabled = false;
        cancel_delayed_work_sync(&adc_data->poll_work);
        dev_info(dev, "ADC polling disabled\n");
    }
    
    mutex_unlock(&adc_data->lock);
    
    return count;
}

static ssize_t adc_enable_show(struct device *dev,
                              struct device_attribute *attr,
                              char *buf)
{
    struct ext_adc_data *adc_data = dev_get_drvdata(dev);
    bool enabled;

    if (!adc_data)
        return -ENODEV;

    mutex_lock(&adc_data->lock);
    enabled = adc_data->enabled;
    mutex_unlock(&adc_data->lock);
    
    return scnprintf(buf, PAGE_SIZE, "%d\n", enabled);
}

static void adc_poll_work_func(struct work_struct *work)
{
    struct ext_adc_data *adc_data = container_of(work,
                                                struct ext_adc_data,
                                                poll_work.work);
    int i;
    
    mutex_lock(&adc_data->lock);
    
    if (!adc_data->enabled) {
        mutex_unlock(&adc_data->lock);
        return;
    }

    for (i = 0; i < adc_data->num_channels; i++) {
        adc_data->values[i] = ext_adc_read_channel(adc_data,i);
        if (adc_data->values[i] >= 0) {
            dev_info(adc_data->dev, "Channel %d: %d\n", i, adc_data->values[i]);
        } else {
            dev_err(adc_data->dev, "Channel %d read error: %d\n", 
                   i, adc_data->values[i]);
        }
    }
    
    mutex_unlock(&adc_data->lock);

    if (adc_data->enabled) {
        schedule_delayed_work(&adc_data->poll_work,
                            msecs_to_jiffies(ADC_POLL_INTERVAL_MS));
    }
}

static DEVICE_ATTR(adc0_value, 0444, adc_value_show, NULL);
static DEVICE_ATTR(adc1_value, 0444, adc_value_show, NULL);
static DEVICE_ATTR(enable, 0644, adc_enable_show, adc_enable_store);

static struct attribute *ext_adc_attrs[] = {
    &dev_attr_adc0_value.attr,
    &dev_attr_adc1_value.attr,
    &dev_attr_enable.attr,
    NULL,
};

static const struct attribute_group ext_adc_attr_group = {
    .attrs = ext_adc_attrs,
};

static int ext_adc_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct ext_adc_data *adc_data;
    int i, ret;
    
    dev_info(dev, "Probing external ADC driver\n");

    adc_data = devm_kzalloc(dev, sizeof(*adc_data), GFP_KERNEL);
    if (!adc_data)
        return -ENOMEM;
    
    adc_data->dev = dev;
    mutex_init(&adc_data->lock);

    adc_data->num_channels = of_property_count_strings(dev->of_node,
                                                      "io-channel-names");
    if (adc_data->num_channels <= 0) {
        dev_err(dev, "No ADC channels defined\n");
        return -ENODEV;
    }
    
    if (adc_data->num_channels > ADC_NUM_CHANNELS) {
        dev_warn(dev, "Too many channels, using first %d\n", ADC_NUM_CHANNELS);
        adc_data->num_channels = ADC_NUM_CHANNELS;
    }

    for (i = 0; i < adc_data->num_channels; i++) {
        ret = of_property_read_string_index(dev->of_node,
                                           "io-channel-names",
                                           i, &adc_data->channel_names[i]);
        if (ret) {
            dev_err(dev, "Failed to get channel name %d\n", i);
            return ret;
        }

        adc_data->adc_channels[i] = devm_iio_channel_get(dev,
                                                         adc_data->channel_names[i]);
        if (IS_ERR(adc_data->adc_channels[i])) {
            ret = PTR_ERR(adc_data->adc_channels[i]);
            dev_err(dev, "Failed to get ADC channel %s: %d\n",
                   adc_data->channel_names[i], ret);
            return ret;
        }
        
        dev_info(dev, "Got ADC channel: %s\n", adc_data->channel_names[i]);
    }

    INIT_DELAYED_WORK(&adc_data->poll_work, adc_poll_work_func);

    /* drvdata must be set before sysfs is visible */
    platform_set_drvdata(pdev, adc_data);
    adc_data->enabled = false;

    ret = sysfs_create_group(&dev->kobj, &ext_adc_attr_group);
    if (ret) {
        dev_err(dev, "Failed to create sysfs group: %d\n", ret);
        platform_set_drvdata(pdev, NULL);
        return ret;
    }

    schedule_delayed_work(&adc_data->poll_work,
                         msecs_to_jiffies(ADC_POLL_INTERVAL_MS));
    
    dev_info(dev, "External ADC driver probed successfully with %d channels\n",
            adc_data->num_channels);
    
    return 0;
}

static void ext_adc_remove(struct platform_device *pdev)
{
    struct ext_adc_data *adc_data = platform_get_drvdata(pdev);

    dev_info(&pdev->dev, "Removing external ADC driver\n");

    cancel_delayed_work_sync(&adc_data->poll_work);
    sysfs_remove_group(&pdev->dev.kobj, &ext_adc_attr_group);
    mutex_destroy(&adc_data->lock);
}

#ifdef CONFIG_PM_SLEEP
static int ext_adc_suspend(struct device *dev)
{
    struct ext_adc_data *adc_data = dev_get_drvdata(dev);
    
    dev_dbg(dev, "Suspending ADC driver\n");
    
    mutex_lock(&adc_data->lock);
    if (adc_data->enabled) {
        cancel_delayed_work_sync(&adc_data->poll_work);
    }
    mutex_unlock(&adc_data->lock);
    
    return 0;
}

static int ext_adc_resume(struct device *dev)
{
    struct ext_adc_data *adc_data = dev_get_drvdata(dev);
    
    dev_dbg(dev, "Resuming ADC driver\n");
    
    mutex_lock(&adc_data->lock);
    if (adc_data->enabled) {
        schedule_delayed_work(&adc_data->poll_work,
                            msecs_to_jiffies(ADC_POLL_INTERVAL_MS));
    }
    mutex_unlock(&adc_data->lock);
    
    return 0;
}

static SIMPLE_DEV_PM_OPS(ext_adc_pm_ops, ext_adc_suspend, ext_adc_resume);
#endif

static const struct of_device_id of_ext_adc_match[] = {
    { .compatible = "ext,adc-driver", },
    {},
};
MODULE_DEVICE_TABLE(of, of_ext_adc_match);

static struct platform_driver ext_adc_driver = {
    .probe      = ext_adc_probe,
    .remove     = ext_adc_remove,
    .driver     = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(of_ext_adc_match),
#ifdef CONFIG_PM_SLEEP
        .pm     = &ext_adc_pm_ops,
#endif
    },
};

static int __init ext_adc_init(void)
{
    pr_info("%s: External ADC driver init start\n", __func__);
    return platform_driver_register(&ext_adc_driver);
}

static void __exit ext_adc_exit(void)
{
    platform_driver_unregister(&ext_adc_driver);
    pr_info("%s: External ADC driver exit\n", __func__);
}

late_initcall(ext_adc_init);
module_exit(ext_adc_exit);

MODULE_DESCRIPTION("External ADC Driver for Multiple Channels");
MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:ext-adc-driver");