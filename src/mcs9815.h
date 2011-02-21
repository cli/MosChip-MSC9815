#ifndef _MCS9815_H_
#define _MCS9815_H_

#include <linux/parport.h>
#include <linux/pci.h>

#define BAR_LEN 8

// Chip register ports
#define REG_DPR(port)		(port->bar0 + 0)
#define REG_DSR(port)		(port->bar0 + 1)
#define REG_DCR(port)		(port->bar0 + 2)
#define REG_EPPADDR(port)	(port->bar0 + 3)
#define REG_EPPDATA(port)	(port->bar0 + 4)
#define REG_CFIFO(port)		(port->bar1 + 0)
#define REG_CONFB(port)		(port->bar1 + 1)
#define REG_ECR(port)		(port->bar1 + 2)

#define MASK_FIFO_EMPTY	0	// Bit 0 of REG_ECR
#define MASK_FIFO_FULL	2	// Bit 1 of REG_ECR

struct mcs9815_port
{
	struct parport* port;
	
	// Every port uses two BARs as I/O ports
	unsigned long bar0;	// Standard parallel port register
	unsigned long bar1;	// Conf register / ECR register
	unsigned int bar0_alloc : 1;
	unsigned int bar1_alloc : 1;
	
	// Cached control registers 
	unsigned int ctrl;
};

#endif
