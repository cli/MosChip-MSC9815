#include <linux/module.h>
#include <linux/parport.h>
#include "mcs9815.h"

extern struct mcs9815_port* port0;
extern struct mcs9815_port* port1;

#define PORT(p) \
	(p == port0->port ? port0 : port1)

/* Writes the given byte value in the data register */
void write_data(struct parport* port, unsigned char value)
{
	printk(KERN_DEBUG "%s: write_data\n", port->name);
	outb(value, REG_EPPDATA(PORT(port)));
}

/* Writes up to len bytes of the given buffer in the specified port */
size_t compat_write_data(struct parport* port, const void* buf, size_t len, int flags)
{
	size_t n;
	printk(KERN_DEBUG "%s: compat_write_data\n", port->name);
	
	for(n = 0; n < len; n++)
	{
		write_data(port, ((unsigned char*)buf)[n]);
	}
	return n;
}

/* Reads a byte off the data register of the given port */
unsigned char read_data(struct parport* port)
{
	printk(KERN_DEBUG "%s: read_data\n", port->name);
	return inb(REG_DPR(PORT(port)));
}

/* 
 * Writes the bitmask control into the control register of the specified
 * port. The control value is buffered in the mcs9815_port struct for
 * read access (see read_control, frob_control).
 */
void write_control(struct parport* port, unsigned char control)
{
	struct mcs9815_port* p = PORT(port);
	printk(KERN_DEBUG "%s: write_control\n", port->name);
	outb(control, REG_DCR(p));
	p->ctrl = control;
}

/* 
 * Returns the bitmask representing the contents of the port's control
 * register. As the value is buffered in the mcs9815_struct to actual
 * hardware access is performed.
 */
unsigned char read_control(struct parport* port)
{
	printk(KERN_DEBUG "%s: read_control\n", port->name);
	return PORT(port)->ctrl;
}

/*
 * This function takes the contents of the port's control register,
 * masks out the bits using (negated) mask and xor'ing the result with
 * val. The result of this operation is written to the port's control
 * register.
 * The function can be used to easily toggle bits in the control 
 * register of the port.
 */
unsigned char frob_control(struct parport* port, unsigned char mask,
							unsigned char val)
{
	struct mcs9815_port* p = PORT(port);
	printk(KERN_DEBUG "%s: frob_control\n", port->name);
	
	// Masking out the bits, xor'ing with val ...
	p->ctrl = (p->ctrl & ~mask) ^ val;
	
	// ... and write the result to control register
	write_control(port, p->ctrl);
	
	return p->ctrl;
}

/* Returns the value of the status register */
unsigned char read_status(struct parport* port)
{
	printk(KERN_DEBUG "%s: read_status\n", port->name);
	return inb(REG_DSR(PORT(port)));
}

/*
 * Reads up to len bytes into the given buffer using parport nibble 
 * mode (reading status register).
 */
size_t nibble_read_data(struct parport* port, void* buf, size_t len, int flags)
{
	int n;
	printk(KERN_DEBUG "%s: nibble_read_data\n", port->name);	
	for(n = 0; n < len; n++)
	{
		((unsigned char*)buf)[n] = read_status(port);
	}
	return n;
}

/*
 * Reads up to len bytes into the given buffer using byte mode.
 */
size_t byte_read_data(struct parport* port, void* buf, size_t len, int flags)
{
	size_t n;
	struct mcs9815_port* p = PORT(port);
	printk(KERN_DEBUG "%s: byte_read_data\n");
	for(n = 0; n < len; n++)
	{
		((unsigned char*)buf)[n] = inb(REG_DPR(p));
	}
	return n;
}

/*
 * Sets bit 4 of control register to enable the generation of interrupts.
 */
void enable_irq(struct parport* port)
{
	printk(KERN_DEBUG "%s: enable_irq\n", port->name);
	
	// Set bit 4 of control register
	frob_control(port, (1 << 4), (1 << 4));
}

/*
 * Clears bit 4 of the control register to disable interrupts on the
 * port.
 */
void disable_irq(struct parport* port)
{
	printk(KERN_DEBUG "%s: disable_irq\n", port->name);
	
	// Clear bit 4 of control register
	frob_control(port, (1 << 4), 0);
}

/*
 * This sets the data direction to forward (which is default for SPP)
 */
void data_forward(struct parport* port)
{
	printk(KERN_DEBUG "%s: data_forward\n", port->name);
	
	// Clear bit 5 of control register
	frob_control(port, (1 << 5), 0);
}

/*
 * Reverses the data direction (required for byte transfer mode)
 */
void data_reverse(struct parport* port)
{
	printk(KERN_DEBUG "%s: data_reverse\n", port->name);
	
	// Set bit 5 of control register
	frob_control(port, (1 << 5), (1 << 5));
}

/*
 * Initializes the chip.
 */
void init_state(struct pardevice* pardev, struct parport_state* parstate)
{
	printk(KERN_INFO "mcs9815: init_state\n");
	
	// Initialize chip (see Master Reset Conditions, p. 12)
	parstate->u.pc.ctr = 0xc;
	write_control(PORT(pardev->port)->port, parstate->u.pc.ctr);

	// Write 0x35 in ECR register for EPP mode
}

/*
 * Saves the state of the port. Why is this called on driver removal?
 */
void save_state(struct parport* parport, struct parport_state* parstate)
{
	printk(KERN_DEBUG "%s: save_state\n", parport->name);
	//const struct parport_pc_private* priv = parport->physport->private_data;
	
}

/*
 * Restores the state of the port.
 */
void restore_state(struct parport* parport, struct parport_state* parstate)
{
	printk(KERN_DEBUG "%s: restore_state\n", parport->name);
}

/*
 * Writes the up to len bytes of the given buffer using EPP data registers.
 */
size_t epp_write_data(struct parport* port, const void* buf, size_t len, int flags)
{
	int n;
	printk(KERN_DEBUG "%s: epp_write_data\n", port->name);
	for(n = 0; n < len; n++)
	{
		outb(REG_EPPDATA(PORT(port)), ((unsigned char*)buf)[n]);
	}
	return len;
}

/*
 * Writes up to len bytes to the specified port using EPP address registers.
 */
size_t epp_write_addr(struct parport* port, const void* buf, size_t len, int flags)
{
	int n;
	printk(KERN_DEBUG "%s: epp_write_addr\n", port->name);
	for(n = 0; n < len; n++)
	{
		outb(REG_EPPADDR(PORT(port)), ((unsigned char*)buf)[n]);
	}
	return len;
}

/*
 * Reads up to len bytes into the given buffer from the EPP data registers.
 */
size_t epp_read_data(struct parport* port, void* buf, size_t len, int flags)
{
	int n;
	printk(KERN_DEBUG "%s: epp_read_data\n", port->name);
	for(n = 0; n < len; n++)
	{
		((unsigned char*)buf)[n] = inb(REG_EPPDATA(PORT(port)));
	}
	return len;
}

/*
 * Reads up to len bytes into the given
 */
size_t epp_read_addr(struct parport* port, void* buf, size_t len, int flags)
{
	int n;
	printk(KERN_DEBUG "%s: epp_read_addr\n", port->name);
	for(n = 0; n < len; n++)
	{
		((unsigned char*)buf)[n] = inb(REG_EPPADDR(PORT(port)));
	}
	return len;
}

size_t ecp_write_data(struct parport* port, const void* buf, size_t len, int flags)
{
	size_t n;
	struct mcs9815_port* p = PORT(port);
	n = 0;
	
	// While FIFO is not full write data to it
	while((inb(REG_ECR(p)) & MASK_FIFO_FULL) == 0 && n < len)
	{
		outb(REG_CFIFO(p), buf[n]);
		n++;
	}
	
	return n;
}

size_t ecp_read_data(struct parport* port, void* buf, size_t len, int flags)
{
	size_t n;
	struct mcs9815_port* p = PORT(port);
	
	// Read from FIFO until it is empty
	while((inb(REG_ECR(p)) & MASK_FIFO_EMPTY) == 0 && n < len)
	{
		((unsigned char*)buf)[n] = inb(REG_CFIFO(p));
		n++;
	}
	return n;
}

size_t ecp_write_addr(struct parport* port, const void* buf, size_t len, int flags)
{
}

struct parport_operations ops =
{
	.write_data        = write_data,
	.compat_write_data = compat_write_data,
	.read_data         = read_data,
	.nibble_read_data  = nibble_read_data,
	.byte_read_data    = byte_read_data,
	
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
	
	.epp_write_data = epp_write_data,
	.epp_write_addr = epp_write_addr,
	.epp_read_data  = epp_read_data,
	.epp_read_addr  = epp_read_addr,
	
	.ecp_write_data = ecp_write_data,
	.ecp_read_data  = ecp_read_data,
	.ecp_write_addr = ecp_write_addr,

	.owner = THIS_MODULE,
};
