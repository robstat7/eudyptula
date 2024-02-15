// SPDX-License-Identifier: GPL-2.0-or-later
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <uapi/linux/hid.h>

MODULE_LICENSE("GPL");

/* table of devices that work with this driver */
static struct usb_device_id hello_usb_keyboard_table[] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
			     USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ }			 /* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, hello_usb_keyboard_table);

static int hello_usb_keyboard_probe(struct usb_interface *interface,
				    const struct usb_device_id *id)
{
	return 0;
}

static void hello_usb_keyboard_disconnect(struct usb_interface *interface)
{
}

static struct usb_driver hello_usb_keyboard_driver = {
	.name        = "hello_usbkbd",
	.probe       = hello_usb_keyboard_probe,
	.disconnect  = hello_usb_keyboard_disconnect,
	.id_table    = hello_usb_keyboard_table,
	.supports_autosuspend = 1,
};

static int __init usb_hello_usb_keyboard_init(void)
{
	int result;

	/* register this driver with the USB subsystem */
	result = usb_register(&hello_usb_keyboard_driver);
	if (result < 0) {
		pr_err("usb_register failed for the %s driver. Error number %d\n",
			hello_usb_keyboard_driver.name, result);
		return -1;
	}

	return 0;
}

static void __exit usb_hello_usb_keyboard_exit(void)
{
	/* deregister this driver with the USB subsystem */
	usb_deregister(&hello_usb_keyboard_driver);
}

module_init(usb_hello_usb_keyboard_init);
module_exit(usb_hello_usb_keyboard_exit);
