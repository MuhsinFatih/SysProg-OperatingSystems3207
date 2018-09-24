#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "misc.h"

#define INIT_TIME 0
#define FIN_TIME 100		// TODO


// jobs enter the system with an interarrival time that is uniformly distributed
#define ARRIVE_MIN 0
#define ARRIVE_MAX 100		// TODO

#define QUIT_PROB 0			// TODO

// use the least busy disk when I/O is requested, as if in RAID 1 drive configuration, all disks contain the same info
// if the disk queues are of equal length, choose one of the disk at random.

/*
When jobs reach some component (CPU, disk1, or disk2), if that component is free, 
the job begins service there immediately. If, on the other hand, that component is 
busy servicing someone else, the job must wait in that component's queue.
*/

// The queue for each system is FIFO

// determine burst time randomly

// a job is serviced by a component for an interval of time that is uniformly distributed
#define CPU_MIN 0			// TODO
#define CPU_MAX 100			// TODO
#define DISK1_MIN 0			// TODO
#define DISK1_MAX 100		// TODO

// FIFO QUEUES:
/*
	CPU
	disk1
	disk2
	priority queue (to store events)
*/
//an event might be something like "a new job entered the system", "a disk read finished", "a job finished at the CPU", etc. 

int main(int argc, char const *argv[])
{
	printf("hello!\n");

	char *buffer = NULL;
	struct Conf asdf;
	memset(&asdf, 0, sizeof(struct Conf));
	// asdf.FIN_TIME = 1;
	char *confText = readFile("conf.txt");
	printf("%s\n", confText);


	return 0;
}
