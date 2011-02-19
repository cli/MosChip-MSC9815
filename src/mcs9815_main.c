#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/parport.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include "mcs9815.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nerdbuero Staff");

static const struct pci_device_id id_table[] =
{
	{
		.vendor      = 0x9710,			/* MosChip Semiconductors */
		.device      = 0x9815,			/* MCS9815 parport controller */
		.subvendor   = 0x1000,
		.subdevice   = 0x0020,
		.class       = 0x078000,		/* See spec p. 9 */
		.class_mask  = 0,				/* ? */
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

struct mcs9815_port* port0 = NULL;
//struct mcs9815_port* port1 = NULL;

// Probes the Base Address Register (BAR) of the given PCI device
static int probe_bar(struct pci_dev* dev, unsigned long* start, int bar)
{
	unsigned long end;
	*start = pci_resource_start(dev, bar);
	end   = pci_resource_end(dev, bar);
	return end - *start;	
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
			release_region(port->bar0, BAR_LEN); // What happens if we release the port without owning it?
			release_region(port->bar1, BAR_LEN);
		}
		kfree(port);
	}
}

static int init_parport(struct pci_dev* dev, struct mcs9815_port* port, 
                          const char* name, int bar0, int bar1)
{
	struct platform_device *pdev = NULL;

	probe_bar(dev, &(port->bar0), bar0);
	probe_bar(dev, &(port->bar1), bar1);
	
	// Request I/O port regions
	if(!request_region(port->bar0, BAR_LEN, name))
	{
		return -1;
	}
	
	if(!request_region(port->bar1, BAR_LEN, name))
	{
		return -2;
	}

	// We can adjust the base, irq and dma parameter later in the
	// parport struct
	printk("parport_register_port\n");
	if(register_parport(port, &ops) != 0)
	{
		printk("mcs9815: register_parport failed!\n");
		return -3;
	}
	
	// Initialize parport structure
	port->port->name  = name;
	port->port->irq   = -1; // -1 disables interrupt
	port->port->modes = PARPORT_MODE_PCSPP | PARPORT_MODE_SAFEININT;
	port->port->dma   = PARPORT_DMA_NONE;

	pdev = platform_device_register_simple("parport_pc", -1, NULL, 0);
	port->port->dev = &(pdev->dev);
	
	// We have successfully registered our parport, now it's time to
	// announce it to the system and device drivers
	parport_announce_port(port->port);
	
	return 0;
}

// PCI probe function that is called by the kernel if it detects the
// hardware specified by the above id_table entry
static int pci_probe(struct pci_dev* dev, const struct pci_device_id* id)
{
	if(unlikely(pci_enable_device(dev) < 0)) // Error codes < 0?
	{
		printk("pci_enable_device failed!");
		return -1;
	}
	
	// Allocate memory for port structures
	port0 = kmalloc(sizeof(struct mcs9815_port), GFP_KERNEL);
	if(likely(port0 != NULL))
	{
		if(init_parport(dev, port0, "mcs9815-port0", 0, 1) != 0)
		{
			printk("Cannot initialize port 0!\n");
			free_parport(port0);
			port0 = NULL;
		}
	}
	
/*	port1 = kmalloc(sizeof(struct mcs9815_port), GFP_KERNEL);
	if(likely(port1 != NULL))
	{
		if(init_parport(dev, port1, "mcs9815-port1", 2, 3) != 0)
		{
			printk("Cannot initialize port 1!\n");
			free_parport(port1);
			port1 = NULL;
		}
	}*/

	if(port0 == NULL/* && port1 == NULL*/)
	{
		printk("Initialization of both ports failed -> disabling PCI device\n");
		pci_disable_device(dev);
		return -2;
	}

	printk("PCI probe finished.\n");
	return 0;
}

// Called when the module's PCI driver is removed from the kernel.
// This functions unregisters the ports, frees its resources and
// disables the PCI device.
static void pci_remove(struct pci_dev* dev)
{
	// Remove parallelport from the parport subsystem and 
	// free the allocated resources
	free_parport(port0);
//	free_parport(port1);
	
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
