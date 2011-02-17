#include <linux/module.h>
#include <linux/parport.h>
#include "mcs9815.h"

extern struct mcs9815_port* port0;
extern struct mcs9815_port* port1;

#define PORT(p) \
	(p == port0->port ? port0 : port1)

void write_data(struct parport* parport, unsigned char value)
{
	printk("%s: write_data\n", parport->name);
	outb(value, PORT(parport)->bar0 + REG_EPPDATA);
}

unsigned char read_data(struct parport* parport)
{
	printk("%s: read_data\n", parport->name);
	return inb(PORT(parport)->bar0 + REG_DPR);
}

void write_control(struct parport* parport, unsigned char value)
{
	struct mcs9815_port* p = PORT(parport);
	printk("%s: write_control\n", parport->name);
	outb(value, p->bar0 + REG_DCR);
	p->control = value;
}

unsigned char read_control(struct parport* parport)
{
	printk("%s: read_control\n", parport->name);
	return PORT(parport)->control;
}

unsigned char frob_control(struct parport* parport, unsigned char mask,
							unsigned char val)
{
	struct mcs9815_port* port = PORT(parport);
	printk("%s: frob_control\n", parport->name);
	
	// Masking out the bits, xor'ing with val ...
	port->control = (port->control & mask) ^ val;
	
	// ... and write the result to control register
	write_control(parport, port->control);
	
	return port->control;
}

unsigned char read_status(struct parport* parport)
{
	printk("%s: read_status\n", parport->name);
	return inb(PORT(parport)->bar0 + REG_DSR);
}

void enable_irq(struct parport* parport)
{
	printk("%s: enable_irq\n", parport->name);
	
	// Set bit 4 of control register
	write_control(parport, read_control(parport) | (1 << 4));
}

void disable_irq(struct parport* parport)
{
	printk("%s: disable_irq\n", parport->name);
	
	// Unset bit 4 of control register
	write_control(parport, read_control(parport) & (~(1 << 4)));
}

void data_forward(struct parport* parport)
{
	printk("%s: data_forward\n", parport->name);
}

void data_reverse(struct parport* parport)
{
	printk("%s: data_reverse\n", parport->name);
}

void init_state(struct pardevice* pardev, struct parport_state* parstate)
{
	printk("mcs9815: init_state\n");
}

void save_state(struct parport* parport, struct parport_state* parstate)
{
	printk("%s: save_state\n", parport->name);
}

void restore_state(struct parport* parport, struct parport_state* parstate)
{
	printk("%s: restore_state\n", parport->name);
}

struct parport_operations ops =
{
	.write_data = write_data,
	.read_data  = read_data,
	
	.write_control = write_control,
	.read_control  = read_control,
	.frob_control  = frob_control,
	.read_control  = read_control,
	.read_status   = read_status,
	
	.enable_irq  = enable_irq,
	.disable_irq = disable_irq,
	
	.data_forward = data_forward,
	.data_reverse = data_reverse,

	.init_state    = init_state,
	.save_state    = save_state,
	.restore_state = restore_state,

	.owner = THIS_MODULE,
};
