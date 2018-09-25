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
			break;
		}
		if(--cpu.currentJob->burstTime <= 0) {
			// job has finished compute time, will either go to a disk or exit (80 - 20)
			// if(myrandom(0,100) > 20) {
			// 	// TODO: implement
			// } else {
				killJob(cpu.currentJob, cpu.queue);
				if(!is_queue_empty(cpu.queue)) cpu.currentJob = cpu.queue->val; // pick the next job
			// }
		}
	}
	printf("simulation ended successfully\n");
}

pNode* upcomingJobs;
size_t upcomingJobsSize;
// This method doesn't represent anything about cpu's workings
void userActivity() {
	if(!is_queue_empty(upcomingJobs)) { // add more jobs only if current batch is empty
		while (((Job*)upcomingJobs->val)->arrivalTime >= simTime) {
			// if a job in the upcomingJobs queue is supposed to arrive, remove it from upcomingjobs and add it to cpu queue
			Job* node = upcomingJobs->val;
			upcomingJobs = upcomingJobs->next; // move the head to the next without freeing the head
			push_queue(&cpu.queue, node, node->arrivalTime);
		}
	} else {
		// fill the upcoming job queue
		{ // first item must be added seperately as it is to be the head
			Job* newJob = generateJob();
			upcomingJobs = new_queue_node(newJob, newJob->arrivalTime);
		}
		for(int i=0; i<upcomingJobsSize-1; ++i) {
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
	char buffer[80];
	sprintf(buffer, "job %i created (burst time:%i)", job->id, job->burstTime);
	log_event(job->arrivalTime, buffer);
	return job;
}

void killJob(Job* job, pNode *queuePos) {
	char buffer[80];
	sprintf(buffer, "job %i destroyed", job->id);
	log_event(simTime, buffer);
	cpu.queue = pop_queue(&cpu.queue);
}

int main(int argc, char const *argv[])
{
	char cwd[150];
   if (getcwd(cwd, sizeof(cwd)) != NULL) {
       printf("Current working dir: %s\n", cwd);
   } else {
       perror("getcwd() error");
       return 1;
   }
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
