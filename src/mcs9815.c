#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nerdbuero Staff");

static int __init mcs9815_init(void)
{
	printk(KERN_DEBUG "MCS9815 module loading...\n");

	printk(KERN_DEBUG "MCS9815 module loaded.\n");
	return 0;
}

static void __exit mcs9815_exit(void)
{
	printk(KERN_DEBUG "MCS9815 module unloading...\n");
	
	printk(KERN_DEBUG "MCS9815 module unloaded.\n");
}

module_init(mcs9815_init);
module_exit(mcs9815_exit);
