// SPDX-License-Identifier: GPL-2.0-or-later
#include <linux/debugfs.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <asm-generic/errno-base.h>

MODULE_LICENSE("GPL");

#define MAX_SIZE          13	/* Maximum size of the data */

static DEFINE_MUTEX(hello_debugfs_mutex);

static const size_t page_size = 4096;
static char *data;
static size_t data_len;
static const char assigned_id[] = "7c1caf2f50aa\n";
static struct dentry *dir;

/*
 * This function will be called when we open the debugfs file.
 */
static int hello_debugfs_open(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * This function will be called when we close the debugfs file.
 */
static int hello_debugfs_close(struct inode *inode, struct file *filp)
{
	return 0;
}

/*
 * This function will be called when we write the id file.
 */
static ssize_t id_write(struct file *filp, const char __user *buf,
			size_t len, loff_t *f_pos)
{
	int status;
	char uid[MAX_SIZE];
	char kdata[MAX_SIZE];

	strscpy(uid, assigned_id, MAX_SIZE);

	if (len == MAX_SIZE) {
		/* copy data from user space to kernel space */
		status = copy_from_user(kdata, buf, len);
		if (status) {
			pr_err("hello_debugfs - error during copy_from_user\n");
			return -status;
		}

		/* make sure data is a null-terminated string */
		kdata[len - 1] = '\0';
		if (strcmp(kdata, uid) == 0)
			return len;
	}

	return -EINVAL;
}

/*
 * This function will be called when we read the id file.
 */
static ssize_t id_read(struct file *filp, char __user *buf,
		       size_t count, loff_t *f_pos)
{
	int status;
	size_t kdata_len;
	ssize_t bytes;

	kdata_len = strlen(assigned_id);
	bytes = count < (kdata_len - (*f_pos)) ? count : (kdata_len - (*f_pos));

	/* copy assigned ID from kernel space to user space */
	status = copy_to_user(buf, assigned_id, bytes);
	if (status) {
		pr_err("hello_debugfs - error during copy_to_user\n");
		return -status;
	}

	*f_pos += bytes;
	return bytes;
}

/*
 * This function will be called when the root user writes the foo file.
 */
static ssize_t foo_write(struct file *filp, const char __user *buf,
			 size_t len, loff_t *f_pos)
{
	int status;
	size_t bytes;
	ssize_t r;

	pr_info("putting write lock!\n");
	if (mutex_lock_interruptible(&hello_debugfs_mutex) < 0) {
		r = -EINTR;
		goto out;
	}

	bytes = len < page_size ? len : page_size;
	/* copy data from user space to kernel space */
	status = copy_from_user(data, buf, bytes);
	if (status) {
		pr_err("hello_debugfs - error during copy_from_user\n");
		return -status;
	}

	data_len = bytes;
	r = len;
	pr_info("releasing write lock!\n");
	mutex_unlock(&hello_debugfs_mutex);
out:
	/* No further write retry */
	return r;
}

/*
 * This function will be called when we read the foo file.
 */
static ssize_t foo_read(struct file *filp, char __user *buf,
			size_t count, loff_t *f_pos)
{
	int status;
	ssize_t bytes;
	ssize_t r;

	pr_info("putting read lock!\n");
	if (mutex_lock_interruptible(&hello_debugfs_mutex) < 0) {
		r = -EINTR;
		goto out;
	}

	bytes = count < (data_len - (*f_pos)) ? count : (data_len - (*f_pos));
	/* copy assigned ID from kernel space to user space */
	status = copy_to_user(buf, data, bytes);
	if (status) {
		pr_err("hello_debugfs - error during copy_to_user\n");
		return -status;
	}

	*f_pos += bytes;
	r = bytes;
	pr_info("releasing read lock!\n");
	mutex_unlock(&hello_debugfs_mutex);
out:
	return r;
}

/* file operations structures */
static const struct file_operations id_fops = {
	.owner          = THIS_MODULE,
	.write          = id_write,
	.read           = id_read,
	.open           = hello_debugfs_open,
	.release        = hello_debugfs_close,
	.llseek         = no_llseek,
};

static const struct file_operations foo_fops = {
	.owner          = THIS_MODULE,
	.write          = foo_write,
	.read           = foo_read,
	.open           = hello_debugfs_open,
	.release        = hello_debugfs_close,
	.llseek         = no_llseek,
};

static int __init debugfs_hello_debugfs_init(void)
{
	/* Create a subdirectory to hold a set of debugfs files */
	dir = debugfs_create_dir("eudyptula", NULL);
	if (dir < 0) {
		pr_err("debugfs_create_dir failed!\n");
		return -1;
	}

	if (debugfs_create_file("id", 0666, dir, NULL, &id_fops) < 0) {
		pr_err("debugfs_create_file failed!\n");
		return -1;
	}

	debugfs_create_ulong("jiffies", 0444, dir, (unsigned long *) &jiffies);

	if (debugfs_create_file("foo", 0644, dir, NULL, &foo_fops) < 0) {
		pr_err("debugfs_create_file failed!\n");
		return -1;
	}

	data = kzalloc(page_size, GFP_NOWAIT);

	pr_info("hello_debugfs initialized!\n");
	return 0;
}

static void __exit debugfs_hello_debugfs_exit(void)
{
	/* Remove all files under the subdirectory */
	debugfs_remove_recursive(dir);
	kfree(data);
	pr_info("hello_debugfs cleaned up!\n");
}

module_init(debugfs_hello_debugfs_init);
module_exit(debugfs_hello_debugfs_exit);
