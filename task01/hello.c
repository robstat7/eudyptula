#include <linux/module.h>     /* Needed by all modules */ 
#include <linux/kernel.h>     /* Needed for KERN_DEBUG */
#include <linux/init.h>       /* Needed for the macros */ 
  
MODULE_LICENSE("GPL"); 
  
static int __init hello_start(void) 
{ 
    printk(KERN_DEBUG "Hello World!\n");
    return 0; 
} 
  
static void __exit hello_end(void) 
{ 
} 
  
module_init(hello_start); 
module_exit(hello_end);
