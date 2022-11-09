/*

*/
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/module.h>

extern void mx6_bee01_wifi_ctl(bool on);

static ssize_t wifi_reset_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	if (strstr(buf, "1") != NULL)
	{
		mx6_bee01_wifi_ctl(true);
	}
	else if (strstr(buf, "0") != NULL)
	{
		mx6_bee01_wifi_ctl(false);
	}

	return count;
}

static struct kobj_attribute wifi_reset_attribute = __ATTR(wifi_reset, 0200, NULL, wifi_reset_store);

static struct attribute *attrs[] = {
	&wifi_reset_attribute.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __init wifi_reset_probe(struct platform_device *pdev)
{
	int ret;

	ret = sysfs_create_group(&pdev->dev.kobj, &attr_group);
	if (ret)
		dev_err(&pdev->dev, "failed to create sysfs entries\n");

	return ret;
}

static int __exit wifi_reset_remove(struct platform_device *pdev)
{
	sysfs_remove_group(&pdev->dev.kobj, &attr_group);

	return 0;
}

static struct platform_driver wifi_reset_driver = {
	.driver		= {
		.name	= "wifi_reset",
		.owner	= THIS_MODULE,
	},
	.remove		= __exit_p(wifi_reset_remove),
};

static int __init wifi_reset_init(void)
{
	return platform_driver_probe(&wifi_reset_driver, wifi_reset_probe);
}

static void __exit wifi_reset_exit(void)
{
	platform_driver_unregister(&wifi_reset_driver);
}

module_init(wifi_reset_init);
module_exit(wifi_reset_exit);

MODULE_AUTHOR("STST");
MODULE_DESCRIPTION("Wifi Reset module");
MODULE_LICENSE("GPL v2");
