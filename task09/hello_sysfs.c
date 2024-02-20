// SPDX-License-Identifier: GPL-2.0-or-later
#include <linux/sysfs.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <asm-generic/errno-base.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");

#define ID_LEN          12	/* Length of the ID */

static DEFINE_MUTEX(hello_sysfs_mutex);

static const size_t page_size = 4096;
static char *data;
static const char assigned_id[] = "7c1caf2f50aa";
static struct kobject *kobj;

/*
 * This function will be called when we read the "id" file.
 */
static ssize_t id_show(struct kobject *kobj, struct kobj_attribute *attr,
		       char *buf)
{
	return sysfs_emit(buf, "%s\n", assigned_id);
}

/*
 * This function will be called when we write the "id" file.
 */
static ssize_t id_store(struct kobject *kobj, struct kobj_attribute *attr,
			const char *buf, size_t count)
{
	if (count == ID_LEN) {
		if (strncmp(buf, assigned_id, ID_LEN) == 0)
			return count;
	}

	return -EINVAL;
}

/* We can set atmost 0664. Setting 0666 throws a compilation error. */
static struct kobj_attribute id_attribute =
	__ATTR(id, 0664, id_show, id_store);

/*
 * This function will be called when we read the "jiffies" file.
 */
static ssize_t jiffies_show(struct kobject *kobj, struct kobj_attribute *attr,
			    char *buf)
{
	return sysfs_emit(buf, "%lu\n", jiffies);
}

/*
 * This function will be called when we write the "jiffies" file.
 */
static ssize_t jiffies_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t count)
{
	return 0;
}

static struct kobj_attribute jiffies_attribute =
	__ATTR(jiffies, 0444, jiffies_show, jiffies_store);

/*
 * This function will be called when we read the "foo" file.
 */
static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
			char *buf)
{
	ssize_t r;

	pr_info("putting read lock!\n");
	if (mutex_lock_interruptible(&hello_sysfs_mutex) < 0) {
		r = -EINTR;
		goto out;
	}

	r = sysfs_emit(buf, "%s", data);

	pr_info("releasing read lock!\n");
	mutex_unlock(&hello_sysfs_mutex);
out:
	return r;
}

/*
 * This function will be called when we write the "foo" file.
 */
static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
			 const char *buf, size_t count)
{
	size_t bytes;
	ssize_t r;

	pr_info("putting write lock!\n");
	if (mutex_lock_interruptible(&hello_sysfs_mutex) < 0) {
		r = -EINTR;
		goto out;
	}

	bytes = count < (page_size - 1) ? count : (page_size - 1);
	r = snprintf(data, page_size, "%.*s", (int)bytes, buf);

	pr_info("releasing write lock!\n");
	mutex_unlock(&hello_sysfs_mutex);
out:
	return r;
}

static struct kobj_attribute foo_attribute =
	__ATTR(foo, 0644, foo_show, foo_store);

/*
 * Create a group of attributes so that we can create and destroy them all
 * at once.
 */
static struct attribute *attrs[] = {
	&id_attribute.attr,
	&jiffies_attribute.attr,
	&foo_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

static int __init sysfs_hello_sysfs_init(void)
{
	int retval;

	/* Create a simple kobject with the name of "eudyptula" */
	kobj = kobject_create_and_add("eudyptula", kernel_kobj);
	if (!kobj)
		return -ENOMEM;

	/* Create the files associated with this kobject */
	retval = sysfs_create_group(kobj, &attr_group);
	if (retval)
		kobject_put(kobj);

	data = kzalloc(page_size, GFP_NOWAIT);
	/* Set the initial empty data string */
	strscpy(data, "\0", 1);

	pr_info("hello_sysfs initialized!\n");
	return retval;
}

static void __exit sysfs_hello_sysfs_exit(void)
{
	kobject_put(kobj);
	kfree(data);
	pr_info("hello_sysfs cleaned up!\n");
}

module_init(sysfs_hello_sysfs_init);
module_exit(sysfs_hello_sysfs_exit);
