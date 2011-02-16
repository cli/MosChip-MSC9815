#include <linux/module.h>
#include <linux/parport.h>

void write_data(struct parport* parport, unsigned char c)
{
	printk("mcs9815: write_data\n");
}

unsigned char read_data(struct parport* parport)
{
	printk("mcs9815: read_data\n");
	return 0;
}

void write_control(struct parport* parport, unsigned char c)
{
	printk("mcs9815: write_control\n");
}

unsigned char read_control(struct parport* parport)
{
	printk("mcs9815: read_control\n");
	return 0;
}

unsigned char frob_control(struct parport* parport, unsigned char mask,
							unsigned char val)
{
	printk("mcs9815: frob_control\n");
	return 0;
}

unsigned char read_status(struct parport* parport)
{
	printk("mcs9815: read_status\n");
	return 0;
}


void enable_irq(struct parport* parport)
{
	printk("mcs9815: enable_irq\n");
}

void disable_irq(struct parport* parport)
{
	printk("mcs9815: disable_irq\n");
}

void data_forward(struct parport* parport)
{
	printk("mcs9815: data_forward\n");
}

void data_reverse(struct parport* parport)
{
	printk("mcs9815: data_reverse\n");
}

/* For core parport code. */
//void (*init_state)(struct pardevice *, struct parport_state *);
//void (*save_state)(struct parport *, struct parport_state *);
//void (*restore_state)(struct parport *, struct parport_state *);

struct parport_operations ops =
{
	.write_data = write_data,
	.read_data  = read_data,
	
	.write_control = write_control,
	.read_control  = read_control,
	.frob_control  = frob_control,
	.read_control  = read_control,
	
	.enable_irq  = enable_irq,
	.disable_irq = disable_irq,
	
	.data_forward = data_forward,
	.data_reverse = data_reverse,

	.owner = THIS_MODULE,
};
