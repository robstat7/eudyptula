// SPDX-License-Identifier: GPL-2.0-or-later

#define DEBUG

#include <linux/module.h>
#include <linux/printk.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");

static int __init hello_start(void)
{
	pr_debug("Hello World!\n");

	return 0;
}

static void __exit hello_end(void)
{
}

module_init(hello_start);
module_exit(hello_end);
