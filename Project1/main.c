#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>

#include "misc.h"
#include "pqueue.h"

struct Conf conf;

size_t simTime = 0;
// #define numOfJobs 1000
// pNode* priorityQueue; // [4 /*(-1 (END event))*/]; // curent events
CPU cpu;
Disk disk1;
Disk disk2;


void simulate() {
	while(simTime < conf.FIN_TIME && !is_queue_empty(cpu.queue)) {
		/*
			- if there is job in queue, check its status, process it.
				- if job is done, go to a disk or terminate (%80 - %20)
			- if there is finished job in disks, return it to cpu queue
			- if there is no finished job in disks, fetch the next job to cpu queue
		*/

	}
}


size_t jobCount = 0;
size_t lastArrivalTime = 0;
Job generateJob() {
	Job job = {
		.id = jobCount++,
		.arrivalTime = lastArrivalTime + myrandom(conf.ARRIVE_MIN, conf.ARRIVE_MAX),
		.burstTime = myrandom(conf.CPU_MIN, conf.CPU_MAX)
	};
	lastArrivalTime = job.arrivalTime;
	char buffer[80];
	sprintf(buffer,"job %i was created", job.id);
	log_event(job.arrivalTime, buffer);
	return job;
}

int main(int argc, char const *argv[])
{
	printf("hello!\n");

	char *buffer = NULL;
	conf = readConf("conf.txt");
	
	initrandom(conf.SEED);


	Job firstJob = generateJob();
	cpu = (CPU){
		.queue = new_queue_node(&firstJob, firstJob.arrivalTime),
		.queueSize = 3,
		.currentJob = NULL
	};
	disk1 = (Disk) {
		.queue = NULL,
		.queueSize = 1,
		.currentJob = NULL
	};
	disk2 = (Disk) {
		.queue = NULL,
		.queueSize = 1,
		.currentJob = NULL
	};

	// fill the cpu queue
	for(int i=0; i<cpu.queueSize-1; ++i) {
		Job newJob = generateJob();
		// priority of newJob is its arrival time
		push_queue(&cpu.queue, &newJob, newJob.arrivalTime);
	}

	printf("there are %i items in the queue\n", size_queue(cpu.queue));


	
	// for(size_t i = 0; i < numOfJobs; ++i) {
	// 	// jobs[i] = (struct Job) {
	// 	// 	.arrivalTime = myrandom(conf.ARRIVE_MIN, conf.ARRIVE_MAX),
	// 	// 	.burstTime = myrandom(conf.CPU_MIN, conf.CPU_MAX)
	// 	// };

		
	// }
	

	return 0;
}
