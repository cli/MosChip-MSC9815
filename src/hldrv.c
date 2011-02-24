#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/parport.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nerdbuero Staff");

struct pardevice *dev = NULL;
static struct timer_list timer;
static atomic_t running = ATOMIC_INIT(0);
static int timer_interval = 500;
module_param(timer_interval, int, S_IRUGO);
static unsigned char countvalue = 0;

// Increments the counter and writes it to parallelport
static void timer_strobe(unsigned long data)
{
	printk(KERN_DEBUG "hldrv: Tick!\n");
	if (atomic_read(&running) > 0)
	{
		countvalue++;
	        // Writes the countvalue into our port
        	dev->port->ops->write_data(dev->port, countvalue);
        	// Alternative:
		// written = parport_write(dev->port, &countvalue, 1);
		// dev->port->ops->write_data is  probably faster as parport_write 
		// has an additional call to compat_write_data() in between. 
		// Additionally parport_write() only works with our mcs9815 
		// driver and freezes with the default parport_pc driver...

		timer.expires = jiffies + timer_interval;
		add_timer(&timer);
	}
}

static void nerdbuero_detach (struct parport *port)
{
	// This Function does literally nothing, but it's nice mentioning it!
	// Here be dragons!
}

static void nerdbuero_attach (struct parport *port)
{
	if(dev == NULL)
	{
		dev = parport_register_device(port, "nerdbuero_driver",
						NULL /*lp_preempt*/, NULL, NULL, 0,
						NULL /*(void *) &lp_table[nr]*/);
		if(dev == NULL)
		{
			printk(KERN_WARNING "hldrv: cannot register parport device!\n");
			return;
		}
		printk(KERN_DEBUG "hldrv: device registered\n");
		
		if(parport_claim(dev) != 0)
		{
			printk(KERN_WARNING "hldrv: cannot claim device!\n");
			return;
		}
		printk(KERN_DEBUG "hldrv: parport claimed\n");
		
		parport_negotiate(dev->port, IEEE1284_MODE_COMPAT); // COMPAT = COMAPTIBLE = SPP
		
		atomic_set(&running, 1);
		
		// Initialize timer
		init_timer(&timer);
		timer.expires = jiffies + timer_interval;
		timer.data = 0;
		timer.function = timer_strobe;
		add_timer(&timer);
	}
}

// Parport driver struct
static struct parport_driver nerdbuero_driver = {
	.name = "nerdbueroblinker",
	.attach = nerdbuero_attach,
	.detach = nerdbuero_detach,
};

static int __init parport_init(void)
{
	printk(KERN_INFO "hldrv module loading...\n");
    
	if (parport_register_driver (&nerdbuero_driver)) //returns 0 on success
	{
		printk (KERN_ERR "nerdbuero: unable to register with parport\n");
		return -EIO;
	}

	printk(KERN_INFO "hldrv module loaded.\n");
	return 0;
}

static void __exit parport_exit(void)
{
	printk(KERN_INFO "hldrv module unloading...\n");
	
	// Stop timer
	atomic_set(&running, 0);
	del_timer_sync(&timer);
	
	// Release parport and unregister hldrv as device driver
	if(dev != NULL)
		parport_release(dev);
	parport_unregister_driver(&nerdbuero_driver);

	printk(KERN_INFO "hldrv module unloaded.\n");
}

module_init(parport_init);
module_exit(parport_exit);
