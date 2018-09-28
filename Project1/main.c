#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "misc.h"
#include "pqueue.h"

struct Conf conf;

size_t simTime = 0;
// #define numOfJobs 1000
// pNode* priorityQueue; // [4 /*(-1 (END event))*/]; // curent events
CPU cpu;
Disk disk1;
Disk disk2;
Disk* disks;
size_t diskCount;
/* Forward declerations */
	void killJob(Job* job, pNode *queuePos);
	void userActivity();
	Job* generateJob();
	bool jobNeedsDisk(Job* job);
	void arriveAtDisk(Job* job, Disk* disk);
	void diskEnter(pNode* jobPos, Disk* disk);
	void diskCompute();
	void cpuEnter(Job* job);
	bool cpuDetermineJob();
	void removeJobFromCPU();
	Disk* pickBestDisk();
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
		if(!cpuDetermineJob()) {continue; /* cpu is idle */}
		if(--cpu.currentJob->burstTime <= 0) {
			// job has finished compute time
			if(jobNeedsDisk(cpu.currentJob)) {
				Disk* bestDisk = pickBestDisk();
				if(bestDisk != NULL) {arriveAtDisk(cpu.currentJob, bestDisk);}
				removeJobFromCPU();
			} else {
				killJob(cpu.currentJob, cpu.queue);
				cpu.currentJob = NULL;
			}
		}
		diskCompute();
	}
	printf("simulation ended successfully\n");
}

Disk* pickBestDisk() {
	Disk* bestDisk = NULL;
	size_t qc = INT_MAX;
	for(size_t i = 0; i < diskCount; ++i) {
		Disk* disk = &disks[i];
		auto diskQueueCount = size_queue(disk->queue);
		if(disk->queueSize - diskQueueCount && diskQueueCount < qc) { // check if there is empty slot in the queue AND determine shortest current queue
			bestDisk = disk;
			qc = diskQueueCount;
		}
	}
	return bestDisk;
}

void removeJobFromCPU() {
	cpu.queue = cpu.queue->next;
	cpu.currentJob = NULL;

}

 /**
  * @brief If cpu is idle, switch to next available job
  * @retval returns false if CPU is idle (no job available), true otherwise
  */
bool cpuDetermineJob() {
	if(cpu.currentJob == NULL) { // cpu is idle
		
		// check disks for finished jobs
		for(size_t i = 0; i < diskCount; ++i) {
			Disk* disk = &disks[i];
			if(is_queue_empty(disk->queue)) {continue;}
			pNode* qp = disk->queue; // queue position
			size_t cpuQueueCount = size_queue(cpu.queue);
			// return all finished jobs to cpu as long as cpu queue has enough free slot
			Job* job = (Job*)qp->val;
			while(qp != NULL && job->burstTime == 0 && cpuQueueCount < cpu.queueSize) {
				cpuEnter(job);
				qp = qp->next;
				job = (Job*)qp->val;
			}
			disk->queue = qp; // point to the next job if one/several is removed from queue
		}
		


		if(!is_queue_empty(cpu.queue)) {
			cpuEnter(cpu.queue->val);
			return true;
		} else {
			return false;
		}
	}
	return true;
}

void diskCompute() {
	for(int i=0; i<diskCount; ++i) {
		Disk* disk = &disks[i];
		if(disk->currentJobPos == NULL) { // if disk is idle
			if(!is_queue_empty(disk->queue) && ((Job*)disk->queue->val)->burstTime > 0) { // if the disk queue contains a job AND the job is NOT finished
				diskEnter(disk->queue, disk);
			}
		}


		if(disk->currentJobPos != NULL) {
			if(--((Job*)disk->currentJobPos->val)->burstTime <= 0) {
				// job is completed on the disk
				((Job*)disk->currentJobPos->val)->burstTime = myrandom(conf.CPU_MIN, conf.CPU_MAX);
				disk->currentJobPos = disk->currentJobPos->next;
			}

		}
	}
}

void diskEnter(pNode* jobPos, Disk* disk) {
	disk->currentJobPos = jobPos;
	Job* job = (Job*)jobPos->val;
	char buffer[80];
	sprintf(buffer, "job %i entered disk%i (disk compute time:%i)", job->id, disk->id, job->burstTime);
	log_event(job->arrivalTime, buffer);
}

void arriveAtDisk(Job* job, Disk* disk) {
	job->burstTime = myrandom(disk->DISK_MIN, conf.DISK1_MIN);
	
	push_queue(&disk->queue, job, job->arrivalTime);

	char buffer[80];
	sprintf(buffer, "job %i arrived at disk%i_queue", job->id, disk->id);
	log_event(simTime, buffer);

}

void cpuEnter(Job* job) {
	cpu.currentJob = job;
	char buffer[80];
	sprintf(buffer, "job %i entered CPU (burst time:%i)", job->id, job->burstTime);
	log_event(job->arrivalTime, buffer);
}

// convenience function, returns true 80% of the time
bool jobNeedsDisk(Job* job) {
	return(myrandom(0,100) > 20);
}

void arriveAtCPU(Job* job) {
	push_queue(&cpu.queue, job, job->arrivalTime);
	char buffer[80];
	sprintf(buffer, "job %i arrived at CPU_queue", job->id);
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
		.id = 1,
		.queue = NULL,
		.queueSize = 4,
		.currentJobPos = NULL,
		.DISK_MIN = conf.DISK1_MIN,
		.DISK_MAX = conf.DISK1_MAX
	};
	disk2 = (Disk) {
		.id = 2,
		.queue = NULL,
		.queueSize = 4,
		.currentJobPos = NULL,
		.DISK_MIN = conf.DISK2_MIN,
		.DISK_MAX = conf.DISK2_MAX
	};

	disks = (Disk[]){disk1, disk2};
	diskCount = 2;
	upcomingJobsSize = cpu.queueSize;



	printf("there are %i items in the queue\n", size_queue(cpu.queue));
	printf("starting simulation\n");
	simulate();

	

	return 0;
}
