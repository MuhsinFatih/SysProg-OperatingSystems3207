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

/* Forward declerations */
	void killJob(Job* job, pNode *queuePos);
	void userActivity();
	Job* generateJob();
	bool jobNeedsDisk(Job* job);
/* End forward declerations */

void simulate() {
	size_t computeTime = 0;

	while(++simTime < conf.FIN_TIME) {
		userActivity(); // adds processes to the queue at random times to simulate process
		/*
			- if there is job in queue, check its status, process it.
				- if job is done, go to a disk or terminate (%80 - %20)
			- if there is finished job in disks, return it to cpu queue
			- if there is no finished job in disks, fetch the next job to cpu queue
			- if there is no job in the queue, wait for one to show up. stay idle (do nothong)
		*/
		if(cpu.currentJob == NULL) { // cpu is idle
			if(!is_queue_empty(cpu.queue)) {
				cpu.currentJob = (Job*)cpu.queue->val;
			} else {
				continue;
			}
		}
		if(--cpu.currentJob->burstTime <= 0) {
			// job has finished compute time
			// if(jobNeedsDisk(cpu.currentJob)) {
			// 	int disk1QueueCount = size_queue(disk1.queue);
			// 	int disk2QueueCount = size_queue(disk2.queue);
			// 	Disk disk = disk1QueueCount <= disk2QueueCount ? disk1 : disk2;
			// } else {
				killJob(cpu.currentJob, cpu.queue);
				cpu.currentJob = NULL;
			// }
			if(!is_queue_empty(cpu.queue)) cpu.currentJob = cpu.queue->val; // pick the next job
		}
	}
	printf("simulation ended successfully\n");
}

void cpuEnter(Job* job) {
	
}

// convenience function, returns true 80% of the time
bool jobNeedsDisk(Job* job) {
	return(myrandom(0,100) > 20);
}

void arriveAtCPU(Job* job) {
	push_queue(&cpu.queue, job, job->arrivalTime);
	char buffer[80];
	sprintf(buffer, "job %i arrived at CPU queue (burst time:%i)", job->id, job->burstTime);
	log_event(job->arrivalTime, buffer);
}

pNode* upcomingJobs;
size_t upcomingJobsSize;
// This method doesn't represent anything about cpu's workings
void userActivity() {
	if(!is_queue_empty(upcomingJobs)) { // add more jobs only if current batch is empty
		while (((Job*)upcomingJobs->val)->arrivalTime <= simTime) {
			auto x = ((Job*)upcomingJobs->val)->arrivalTime;
			// if a job in the upcomingJobs queue is supposed to arrive, remove it from upcomingjobs and add it to cpu queue
			Job* job = upcomingJobs->val;
			upcomingJobs = upcomingJobs->next; // move the head to the next without freeing the head
			arriveAtCPU(job);
			if(is_queue_empty(upcomingJobs)) {break;}
		}
	} else {
		if(size_queue(cpu.queue) >= cpu.queueSize) {return;}
		// fill the upcoming job queue
		for(int i=0; i<upcomingJobsSize; ++i) {
			pNode* current = upcomingJobs;
			while(current != NULL) {
				current = current->next;
			}
			Job* newJob = generateJob();
			// priority of newJob is its arrival time
			push_queue(&upcomingJobs, newJob, newJob->arrivalTime);
		}
	}
}
size_t jobCount = 0;
size_t lastArrivalTime = 0;
// TODO: move this to a library file
Job* generateJob() {
	Job* job = malloc(sizeof(Job));
	job->id = jobCount++;
	job->arrivalTime = lastArrivalTime + myrandom(conf.ARRIVE_MIN, conf.ARRIVE_MAX);
	job->burstTime = myrandom(conf.CPU_MIN, conf.CPU_MAX);

	lastArrivalTime = job->arrivalTime;
	
	return job;
}

void killJob(Job* job, pNode *queuePos) {
	char buffer[80];
	sprintf(buffer, "job %i destroyed", job->id);
	log_event(simTime, buffer);
	pop_queue(&cpu.queue);
	cpu.currentJob = NULL;
}


int main(int argc, char const *argv[])
{	
	printf("hello!\n");

	char *buffer = NULL;
	conf = readConf("conf.txt");
	
	initrandom(conf.SEED);

	cpu = (CPU){
		.queue = NULL,
		.queueSize = 5,
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


	upcomingJobsSize = cpu.queueSize;



	printf("there are %i items in the queue\n", size_queue(cpu.queue));
	printf("starting simulation\n");
	simulate();

	

	return 0;
}
