#ifndef _MCS9815_H_
#define _MCS9815_H_

#include <linux/parport.h>
#include <linux/pci.h>

#define BAR_LEN 8

// Chip register offsets
#define REG_DPR 0
#define REG_DSR 1
#define REG_DCR 2
#define REG_EPPADDR 3
#define REG_EPPDATA 4
#define REG_CFIFO 0
#define REG_CONFB 1
#define REG_ECR 2

struct mcs9815_port
{
	struct parport* port;
	
	// Every port uses two BARs as I/O ports
	unsigned long bar0;	// Standard parallel port register
	unsigned long bar1;	// Conf register / ECR register
};

#endif
