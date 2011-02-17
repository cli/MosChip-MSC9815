#ifndef _MCS9815_H_
#define _MCS9815_H_

#include <linux/parport.h>
#include <linux/pci.h>

#define BAR_LEN 8

struct mcs9815_port
{
	struct parport* port;
	
	// Every port uses two BARs as I/O ports
	unsigned long bar0;	// Standard parallel port register
	unsigned long bar1;	// Conf register / ECR register
};

#endif
