#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>

#define LWHPS2FPGA_BASE 0xff200000
#define BLINKER_OFFSET 0x0
#define BLINKER_BASE (LWHPS2FPGA_BASE + BLINKER_OFFSET)

static struct device_driver blinker_driver = {
	.name = "blinker",
	.bus = &platform_bus_type,
};

ssize_t blinker_show(struct device_driver *drv, char *buf)
{
	return 0;
}

ssize_t blinker_store(struct device_driver *drv, const char *buf, size_t count)
{
	u8 delay;
	int ret;

	if (buf == NULL) {
		pr_err("Error, string must not be NULL\n");
		return -EINVAL;
	}

	if ((ret = kstrtou8(buf, 10, &delay)) < 0) {
		pr_err("Could not convert string to integer\n");
		return ret;
	}

	if (delay < 1 || delay > 15) {
		pr_err("Invalid delay %d\n", delay);
		return -EINVAL;
	}

	outb(delay, BLINKER_BASE);

	return count;
}

static DRIVER_ATTR(blinker, S_IWUSR, blinker_show, blinker_store);

MODULE_LICENSE("Dual BSD/GPL");

static int __init blinker_init(void)
{
	int ret;
	struct resource *res;

	if ((ret = driver_register(&blinker_driver)) < 0)
		return ret;

	ret = driver_create_file(&blinker_driver, &driver_attr_blinker);
	if (ret < 0) {
		driver_unregister(&blinker_driver);
		return ret;
	}

	res = request_region(BLINKER_BASE, 1, "blinker");
	if (res == NULL) {
		driver_remove_file(&blinker_driver, &driver_attr_blinker);
		driver_unregister(&blinker_driver);
		return -EBUSY;
	}

	return 0;
}

static void __exit blinker_exit(void)
{
	driver_remove_file(&blinker_driver, &driver_attr_blinker);
	driver_unregister(&blinker_driver);
	release_region(BLINKER_BASE, 1);
}

module_init(blinker_init);
module_exit(blinker_exit);
