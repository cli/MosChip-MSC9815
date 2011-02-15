#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/parport.h>
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

struct parport_operations ops =
{
	
};

struct mcs9815_port
{
	struct parport* port;
	
	// Every port uses two BARs as I/O ports
	unsigned long bar0;	// Standard parallel port register
	unsigned long bar1;	// Conf register / ECR register
};

static struct mcs9815_port* port0 = NULL;
static struct mcs9815_port* port1 = NULL;

// We allow access to the parport via ioctl'ing the /dev/mcs9815 device.
// This is only for testing purposes as this might be dangerous if invalid 
// data is sent to the registers. Ye be warned!
static long mcs9815_ioctl(struct file* file, unsigned int cmd, unsigned long arg)
{
	return -ENOTTY;
}

static int probe_bar(struct pci_dev* dev, unsigned long* start, int bar)
{
	unsigned long end;
	start = pci_resource_start(dev, bar);
	end   = pci_resource_end(dev, bar);
	return end - start;	
} 

static int register_parport(struct mcs9815_port* port)
{
}

static int pci_probe(struct pci_dev* dev, const struct pci_device_id* id)
{
	unsigned long res_start, res_end, res_flags;
	unsigned int n;
	u32 val;
	struct parport_operations *ops;

	if(unlikely(pci_enable_device(dev) < 0)) // Error codes < 0?
	{
		printk("pci_enable_device failed!");
		return -1;
	}
	
	// Allocate memory for port structures
	port0 = kmalloc(sizeof(struct mcs9815_port), GFP_KERNEL);
	if(unlikely(port0 == NULL))
	{
		printk("Cannot allocate structure for port 0!\n");
		goto err0; // Disable PCI device and exit
	}
	port1 = kmalloc(sizeof(struct mcs9815_port), GFP_KERNEL);
	if(unlikely(port1 == NULL))
	{
		printk("Cannot allocate structure for port 1\n");
		kfree(port0); // Free structure of port 0
		port0 = NULL;
		goto err0;    // Disable PCI device and exit
	}

	// Probe I/O areas; MCS9815 uses two bars per port
	probe_bar(&(port0->bar0), 0);
	probe_bar(&(port0->bar1), 1);
	probe_bar(&(port1->bar0), 2);
	probe_bar(&(port1->bar1), 3);	

	// Read Subsystem ID and Subvendor ID (offset 0x2C) from PCI configuration 
	// space to determine the port configuration
	pci_read_config_dword(dev, 0x2C, &val);
	printk("Subsystem ID: %x\n", val >> 16);
	printk("Subvendor ID: %x\n", val & 0xFFFF);

	// Now it's time to register this module as parport driver, isn't it?
	ops = kmalloc(sizeof(struct parport_operations), GFP_KERNEL);
	if(ops == NULL)
	{
		goto err1;
	}
	
	// We can adjust the base, irq and dma parameter later in the
	// parport struct
	printk("parport_register_port\n");
	port0 = parport_register_port(0, 0, 0, ops);
	if(port0 == NULL)
	{
		printk("Not enough memory to allocate parport structure!\n");
		goto err2;
	}

	// TODO: Adjust parameter
	
	// We have successfully registered our parport, now it's time to
	// announce it to the system and device drivers
	//parport_announce_port(port0);

	printk("PCI probe finished.\n");
	return 0;

err2:
	kfree(ops);

err1:
	kfree(port0);
	kfree(port1);
	port0 = NULL;
	port1 = NULL;

err0:
	pci_disable_device(dev);
	return -1;
}

static void pci_remove(struct pci_dev* dev)
{
	// Remove parallelport from the parport subsystem
	parport_remove_port(port0);
	
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
