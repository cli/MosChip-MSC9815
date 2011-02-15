#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/pci.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nerdbuero Staff");

// Function prototypes
static long mcs9815_ioctl(struct file*, unsigned int, unsigned long);

static const struct pci_device_id id_table[] =
{
	{
		.vendor = 0x9710,			/* MosChip Semiconductors */
		.device = 0x9815,			/* MCS9815 parport controller */
		.subvendor = 0x1000,
		.subdevice = 0x0020,
		.class = 0x078000,			/* See spec p. 9 */
		.class_mask = 0,			/* ? */
		.driver_data = 0
	},
	{0, }
};

static struct pci_driver mcs9815_pci_driver =
{
	.name = "mcs9815",
	.id_table = id_table
};

struct file_operations fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = mcs9815_ioctl
};

struct parport_operations ppops =
{
	
};

// We allow access to the parport via ioctl'ing the /dev/mcs9815 device.
// This is only for testing purposes as this might be dangerous if invalid 
// data is sent to the registers. Ye be warned!
static long mcs9815_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	return -ENOTTY;
}

static int pci_probe(struct pci_dev* dev, const struct pci_device_id* id)
{
	unsigned int n, res_start, res_end, res_flags;
	struct parport_operations *ops;

	if(unlikely(pci_enable_device(dev) < 0)) // Error codes < 0?
	{
		printk("pci_enable_device failed!");
		return -1;
	}
	
	// Probe I/O areas
	for(n = 0; n < 6; n++)
	{
		res_start = pci_resource_start(dev, n);
		res_end = pci_resource_end(dev, n);
		res_flags = pci_resource_flags(dev, n);
		printk("Bar %u: Start %x End %x Flags %x\n", n, res_start, res_end, res_flags);
	}

	// Now it's time to register this module as parport driver, isn't it?
	/*ops = kmalloc(sizeof(struct parport_operations), GFP_KERNEL);
	if(ops == NULL)
		goto err0;
	
	parport_register_port(PARPORT0_BASE, irq, dma, ops);*/

	printk("PCI probe finished.\n");
	return 0;
	
err0:
	pci_disable_device(dev);	
}

static void pci_remove(struct pci_dev* dev)
{	
	// Disable PCI device
	pci_disable_device(dev);
}

static int __init mcs9815_init(void)
{
	printk(KERN_DEBUG "MCS9815 module loading...\n");

	// Initialize PCI hardware
	mcs9815_pci_driver.probe = pci_probe;
	mcs9815_pci_driver.remove = pci_remove;
	
	if(pci_register_driver(&mcs9815_pci_driver) < 0) 
	{
		printk("Error registering driver!");
		return -1;
	}

	printk(KERN_DEBUG "MCS9815 module loaded.\n");
	return 0;
}

static void __exit mcs9815_exit(void)
{
	printk(KERN_DEBUG "MCS9815 module unloading...\n");
	pci_unregister_driver(&mcs9815_pci_driver);
	printk(KERN_DEBUG "MCS9815 module unloaded.\n");
}

module_init(mcs9815_init);
module_exit(mcs9815_exit);
