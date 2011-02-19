#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/parport.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nerdbuero Staff");

struct pardevice *dev;
static struct timer_list timer;
static atomic_t running = ATOMIC_INIT(0);
static int timer_interval = 500;
module_param(timer_interval, int, S_IRUGO);
static unsigned char countvalue = 0;

/**
 * Write value to Parallelport
 * */
static void writeValueToParallel(void)
{
	int written;

	parport_claim_or_block(dev);
	parport_negotiate(dev->port, IEEE1284_MODE_COMPAT); // COMPAT = COMAPTIBLE = SPP
	written = parport_write(dev->port, &countvalue, 1); // Last param is number of bytes to write
	printk("nerdbuero: Written byte to parport");
	parport_release(dev);
}

/**
 * Timer function. Increments an Integervalue and calls Output
 * to parallelport
 * */
static void timer_strobe(unsigned long data)
{
	if (dev != NULL)
	{
		countvalue++;
		writeValueToParallel();
		
		if(atomic_read(&running) > 0)
		{
			timer.expires = jiffies + timer_interval;
			add_timer(&timer);
		}
	}
}

static void nerdbuero_detach (struct parport *port)
{
	//This Function does literally nothing, but it's nice mentioning it!
}
/* HIER STIMMT NOCH WAS NICHT!!!!
 * */
static void nerdbuero_attach (struct parport *port)
{
	dev = parport_register_device(port, "nerdbuero_driver", 
						   NULL/*lp_preempt*/, NULL, NULL, 0,
							NULL /*(void *) &lp_table[nr]*/);
	printk("Nerdbueroname: %s\n", port->name);
}

/**
 * Properties of the parport_driver struct needed by the parportsystem
 * 
 * */
static struct parport_driver nerdbuero_driver = {
	.name = "nerdbueroprinter",
	.attach = nerdbuero_attach,
	.detach = nerdbuero_detach,
};

static int __init parport_init(void)
{
	printk("Parport module loading...\n");
    
	if (parport_register_driver (&nerdbuero_driver)) //returns 0 on success
	{
		printk (KERN_ERR "nerdbuero: unable to register with parport\n");
		return -EIO;
	}

	// Initialize timer
	init_timer(&timer);
	timer.expires = jiffies + timer_interval;
	timer.data = 0;
	timer.function = timer_strobe;
	add_timer(&timer);

	printk("Parport module loaded.\n");
	return 0;
}

static void __exit parport_exit(void)
{
	printk("Parport module unloading...\n");
        
	atomic_set(&running, 0);
	del_timer_sync(&timer);
	parport_release (dev);
	printk("Parport module unloaded.\n");
}

module_init(parport_init);
module_exit(parport_exit);
