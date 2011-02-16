#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/parport.h>
#include <linux/pci.h>
#include "mcs9815.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nerdbuero Staff");

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

extern struct parport_operations ops;

static struct mcs9815_port* port0 = NULL;
static struct mcs9815_port* port1 = NULL;

// Probes the Base Address Register (BAR) of the given PCI device
static int probe_bar(struct pci_dev* dev, unsigned long* start, int bar)
{
	unsigned long end;
	*start = pci_resource_start(dev, bar);
	end   = pci_resource_end(dev, bar);
	return end - *start;	
} 

static void init_parport(struct mcs9815_port* port, const char* name)
{
	port->port->name = name;
	port->port->irq  = -1; // -1 disables interrupt
}

// Registers the given MCS9815 port at the parport subsystem
static int register_parport(struct mcs9815_port* port, struct parport_operations* ops)
{
	port->port = parport_register_port(0, 0, 0, ops);
	if(unlikely(port->port == NULL))
	{
		return -1;
	}
	return 0;
}

// Unregisters the port at the parport subsystem and the frees the
// resources of the port
static void free_parport(struct mcs9815_port* port)
{
	if(port != NULL)
	{
		if(port->port != NULL)
		{
			parport_remove_port(port->port); // Does function free port0->port?
		}
		kfree(port);
	}
}

// PCI probe function that is called by the kernel if it detects the
// hardware specified by the above id_table entry
static int pci_probe(struct pci_dev* dev, const struct pci_device_id* id)
{
	struct parport_operations *ops;

	if(unlikely(pci_enable_device(dev) < 0)) // Error codes < 0?
	{
		printk("pci_enable_device failed!");
		return -1;
	}

	// Now it's time to register this module as parport driver, isn't it?
	ops = kmalloc(sizeof(struct parport_operations), GFP_KERNEL);
	if(ops == NULL)
	{
		goto err0;
	}
	
	// Allocate memory for port structures
	port0 = kmalloc(sizeof(struct mcs9815_port), GFP_KERNEL);
	if(unlikely(port0 == NULL))
	{
		printk("Cannot allocate structure for port 0!\n");
		goto err1; // Free ops, disable PCI device and exit
	}
	
	port1 = kmalloc(sizeof(struct mcs9815_port), GFP_KERNEL);
	if(unlikely(port1 == NULL))
	{
		printk("Cannot allocate structure for port 1\n");
		goto err2; // Free ports, free ops, disable PCI device and exit
	}

	// Probe I/O areas; MCS9815 uses two bars per port
	probe_bar(dev, &(port0->bar0), 0);
	probe_bar(dev, &(port0->bar1), 1);
	probe_bar(dev, &(port1->bar0), 2);
	probe_bar(dev, &(port1->bar1), 3);	
	
	// We can adjust the base, irq and dma parameter later in the
	// parport struct
	printk("parport_register_port\n");
	if(register_parport(port0, ops) != 0)
	{
		goto err2;
	}
	
	if(register_parport(port1, ops) != 0)
	{
		goto err2;
	}

	// Adjust parameter
	init_parport(port0, "mcs9815-port0");
	init_parport(port1, "mcs9815-port1");
	
	// We have successfully registered our parport, now it's time to
	// announce it to the system and device drivers
	parport_announce_port(port0->port);
	parport_announce_port(port1->port);

	printk("PCI probe finished.\n");
	return 0;

// Error handling below
err2:
	free_parport(port0);
	free_parport(port1);
	port0 = NULL;
	port1 = NULL;

err1:
	kfree(ops);

err0:
	pci_disable_device(dev);
	return -1;
}

// Called when the module's PCI driver is removed from the kernel.
// This functions unregisters the ports, frees its resources and
// disables the PCI device.
static void pci_remove(struct pci_dev* dev)
{
	// Remove parallelport from the parport subsystem and 
	// free the allocated resources
	free_parport(port0);
	free_parport(port1);
	
	// Disable PCI device
	pci_disable_device(dev);
}

// Module init function
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

// Module exit function
static void __exit mcs9815_exit(void)
{
	printk(KERN_DEBUG "MCS9815 module unloading...\n");
	pci_unregister_driver(&mcs9815_pci_driver);
	printk(KERN_DEBUG "MCS9815 module unloaded.\n");
}

module_init(mcs9815_init);
module_exit(mcs9815_exit);
