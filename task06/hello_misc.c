// SPDX-License-Identifier: GPL-2.0-or-later
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <asm-generic/errno-base.h>

MODULE_LICENSE("GPL");

#define ID_LEN          12	/* Length of the ID */

static const char assigned_id[] = "7c1caf2f50aa";

/*
 * This function will be called when we open the misc device file.
 */
static int hello_misc_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * This function will be called when we close the misc device file.
 */
static int hello_misc_close(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * This function will be called when we write the misc device file.
 */
static ssize_t hello_misc_write(struct file *filp, const char __user *buf,
				size_t len, loff_t *f_pos)
{
	int status;
	char data[ID_LEN];

	if (len == ID_LEN) {
		/* copy data from user space to kernel space */
		status = copy_from_user(data, buf, ID_LEN);
		if (status) {
			pr_err("misc device - error during copy_from_user\n");
			return -status;
		}

		if (strncmp(data, assigned_id, ID_LEN) == 0)
			return len;
	}

	return -EINVAL;
}

/*
 * This function will be called when we read the misc device file.
 */
static ssize_t hello_misc_read(struct file *filp, char __user *buf,
			       size_t count, loff_t *f_pos)
{
	int status;
	ssize_t bytes;

	bytes = count < (ID_LEN - (*f_pos)) ? count : (ID_LEN - (*f_pos));

	/* Copy the assigned ID from kernel space to user space */
	status = copy_to_user(buf, assigned_id, bytes);
	if (status) {
		pr_err("misc device - error during copy_to_user\n");
		return -status;
	}

	*f_pos += bytes;
	return bytes;
}

/* file operations structure */
static const struct file_operations fops = {
	.owner          = THIS_MODULE,
	.write          = hello_misc_write,
	.read           = hello_misc_read,
	.open           = hello_misc_open,
	.release        = hello_misc_close,
	.llseek         = no_llseek,
};

/* Misc device structure */
static struct miscdevice hello_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "eudyptula",
	.fops = &fops,
};

static int __init misc_hello_misc_init(void)
{
	int error;

	/* register the miscellaneous device with the kernel */
	error = misc_register(&hello_misc_device);
	if (error) {
		pr_err("misc_register failed for the %s misc device. Error number %d\n",
			hello_misc_device.name, error);
		return error;
	}

	pr_info("%s misc device registered!\n", hello_misc_device.name);
	return 0;
}

static void __exit misc_hello_misc_exit(void)
{
	/* deregister the miscellaneous device with the kernel */
	misc_deregister(&hello_misc_device);
	pr_info("%s misc device deregistered!\n", hello_misc_device.name);
}

module_init(misc_hello_misc_init);
module_exit(misc_hello_misc_exit);
