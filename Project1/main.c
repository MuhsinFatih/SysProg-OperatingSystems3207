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
Disk** disks;
size_t diskCount;
/* Forward declerations */
	void killJob(Job* job, pNode *queuePos);
	void userActivity();
	Job* generateJob();
	bool jobNeedsDisk(Job* job);
	void arriveAtDisk(Job* job, Disk* disk);
	void diskEnter(Job* job, Disk* disk);
	void diskCompute();
	void cpuEnter(Job* job);
	bool cpuDetermineJob();
	void removeJobFromCPU();
	Disk* pickBestDisk();
	void arriveAtCPU_end(Job* job);
	void returnFromDisk(Job* job, Disk* disk);
	void recordCommonStats();
	void dumpStats(Telemetry **t, size_t tSize);
	void logStats(const char* deviceName, Telemetry* stats);
/* End forward declerations */

void simulate() {
	size_t computeTime = 0;

	while(++simTime <= conf.FIN_TIME) {
		userActivity(); // adds processes to the queue at random times to simulate process
		/*
			- if there is job in queue, check its status, process it.
				- if job is done, go to a disk or terminate (%80 - %20)
			- if there is finished job in disks, return it to cpu queue
			- if there is no finished job in disks, fetch the next job to cpu queue
			- if there is no job in the queue, wait for one to show up. stay idle (do nothong)
		*/
		if(!cpuDetermineJob()) {/* cpu is idle */}
		else if(--cpu.currentJob->burstTime <= 0) {
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
		recordCommonStats();
	}
	printf("simulation ended successfully\n");
}


void initStats() {
	cpu.telemetry = (Telemetry) {0};
	for(int i=0; i<diskCount; ++i) {
		disks[i]->telemetry = (Telemetry) {0};
	}
}

void recordCommonStats() {
	++cpu.telemetry.totalTime;
	if(cpu.currentJob != NULL) {
		++cpu.telemetry.busyTime;
	}
	cpu.telemetry.queueSum += size_queue(cpu.queue);
	for(size_t i=0; i<diskCount; ++i) {
		Disk* disk = disks[i];
		++disk->telemetry.totalTime;
		if(disk->currentJob != NULL) {
			++disk->telemetry.busyTime;
		}
		disk->telemetry.queueSum += size_queue(disk->queue);
	}
}



Disk* pickBestDisk() {
	Disk* bestDisk = NULL;
	size_t qc = INT_MAX;
	for(size_t i = 0; i < diskCount; ++i) {
		Disk* disk = disks[i];
		size_t diskQueueCount = size_queue(disk->queue);
		if(disk->queueSize - diskQueueCount && diskQueueCount < qc) { // check if there is empty slot in the queue AND determine shortest current queue
			bestDisk = disk;
			qc = diskQueueCount;
		}
	}
	return bestDisk;
}


void removeJobFromDisk(Disk* disk) {
	// collect telemetry
	size_t responseTime = simTime - disk->currentJob->arrivalTime;
	updateResponseTime(responseTime, &disk->telemetry);

	// remove job from disk
	disk->queue = disk->queue->next; // increment disk queue pointer
	disk->currentJob = NULL;
}
void removeJobFromCPU() {
	// collect telemetry
	size_t responseTime = simTime - cpu.currentJob->arrivalTime;
	updateResponseTime(responseTime, &cpu.telemetry);

	// remove job from cpu
	cpu.queue = cpu.queue->next; // increment cpu queue pointer
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
			Disk* disk = disks[i];
			if(is_queue_empty(disk->queue)) {continue;}

			Job* job = (Job*)disk->queue->val;
			size_t cpuQueueCount = size_queue(cpu.queue);
			
			if(job->burstTime <= 0 && cpuQueueCount < cpu.queueSize) {
				job->burstTime = myrandom(conf.CPU_MIN, conf.CPU_MAX);
				returnFromDisk(job, disk);
			}
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
		Disk* disk = disks[i];
		if(disk->currentJob == NULL) {
			if(!is_queue_empty(disk->queue) && ((Job*)disk->queue->val)->burstTime > 0) { // if the disk queue contains a job AND the job is NOT finished
				diskEnter((Job*)disk->queue->val, disk);
			}
		}

		if(disk->currentJob != NULL && disk->currentJob->burstTime > 0) { // wait for cpu if job is finished
			--disk->currentJob->burstTime; // disk computation
		}
	
	}
}

void diskEnter(Job* job, Disk* disk) {
	disk->currentJob = job;
	char buffer[80];
	sprintf(buffer, "job %lu entered disk%lu (disk compute time:%lu)", job->id, disk->id, job->burstTime);
	log_event(simTime, buffer);
}

void arriveAtDisk(Job* job, Disk* disk) {
	job->arrivalTime = simTime;
	job->burstTime = myrandom(disk->DISK_MIN, conf.DISK1_MAX);
	
	push_queue(&disk->queue, job, job->arrivalTime);

	char buffer[80];
	sprintf(buffer, "job %lu arrived at disk%lu_queue", job->id, disk->id);
	log_event(simTime, buffer);

}

void cpuEnter(Job* job) {
	cpu.currentJob = job;
	char buffer[80];
	sprintf(buffer, "job %lu entered CPU (burst time:%lu)", job->id, job->burstTime);
	log_event(simTime, buffer);
}

// convenience function, returns true 80% of the time
bool jobNeedsDisk(Job* job) {
	return(myrandom(0,100) > 20);
}

void arriveAtCPU(Job* job) {
	job->arrivalTime = simTime;
	push_queue(&cpu.queue, job, job->arrivalTime);
	char buffer[80];
	sprintf(buffer, "job %lu arrived at CPU_queue", job->id);
	log_event(job->arrivalTime, buffer);
}

void returnFromDisk(Job* job, Disk* disk) {
	removeJobFromDisk(disk);
	push_queue_end(&cpu.queue, job);
	char buffer[80];
	sprintf(buffer, "job %lu returned from disk %lu to CPU_queue", job->id, disk->id);
	log_event(simTime, buffer);
}


void arriveAtCPU_end(Job* job) {
	job->arrivalTime = simTime;
	push_queue_end(&cpu.queue, job);
	char buffer[80];
	sprintf(buffer, "job %lu arrived at CPU_queue", job->id);
	log_event(job->arrivalTime, buffer);
}

pNode* upcomingJobs;
size_t upcomingJobsSize;
// This method doesn't represent anything about cpu's workings
void userActivity() {
	if(!is_queue_empty(upcomingJobs)) { // add more jobs only if current batch is empty
		while (((Job*)upcomingJobs->val)->arrivalTime <= simTime) {
			size_t x = ((Job*)upcomingJobs->val)->arrivalTime;
			// if a job in the upcomingJobs queue is supposed to arrive, remove it from upcomingjobs and add it to cpu queue
			Job* job = upcomingJobs->val;
			upcomingJobs = upcomingJobs->next; // move the head to the next without freeing the head
			arriveAtCPU(job);
			if(is_queue_empty(upcomingJobs)) {break;}
		}
	} else {
		if(size_queue(cpu.queue) >= cpu.queueSize) {return;}
		// fill the upcoming job queue
		for(int i=0; i<myrandom(1,upcomingJobsSize); ++i) {
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
	sprintf(buffer, "job %lu destroyed", job->id);
	log_event(simTime, buffer);
	pop_queue(&cpu.queue);
	cpu.currentJob = NULL;
}


int main(int argc, char const *argv[])
{
	

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
		.currentJob = NULL,
		.DISK_MIN = conf.DISK1_MIN,
		.DISK_MAX = conf.DISK1_MAX
	};
	disk2 = (Disk) {
		.id = 2,
		.queue = NULL,
		.queueSize = 4,
		.currentJob = NULL,
		.DISK_MIN = conf.DISK2_MIN,
		.DISK_MAX = conf.DISK2_MAX
	};

	disks = (Disk*[]){&disk1, &disk2};
	Disk* diskk = disks[0];
	diskCount = 2;
	upcomingJobsSize = cpu.queueSize;



	printf("starting simulation\n");
	initLog();
	initStats();
	log("-------------------------------------------------\n"	
		"|                  Events:                      |\n"
		"-------------------------------------------------\n"
		);
	simulate();

	Telemetry* t[3] = {&cpu.telemetry,&disk1.telemetry,&disk2.telemetry};
	size_t tSize = sizeof(t)/sizeof(t[0]);
	
	// dumpStats(t, tSize);
	
	log("\n\n\n\n");
	log("-------------------------------------------------\n"	
		"|                  Statistics:                  |\n"
		"-------------------------------------------------\n"
		);

	
	logStats("CPU", &cpu.telemetry);
	for(size_t i=0; i<diskCount; ++i) {
		Disk* disk = disks[i];
		char diskname[20];
		sprintf(diskname, "disk%i", disk->id);
		logStats(diskname, &disk->telemetry);
	}
	writeLogToFile("stats.log");

	return 0;
}


void logStats(const char* deviceName, Telemetry* t) {
	char buffer[500];
	Stats* stats = malloc(sizeof(Stats));
	finalizeStats(t, stats);
	sprintf(buffer,
			"Stats for %s:\n\n"
			"averageQueueSize: %f\n"
			"utilization: %f\n"
			"maxResponseTime: %lu\n"
			"averageResponseTime: %f\n"
			"throughput: %f\n"
			"--------------------\n",
			deviceName,
			stats->averageQueueSize,
			stats->utilization,
			stats->maxResponseTime,
			stats->averageResponseTime,
			stats->throughput);
	log(&buffer);
}









void dumpStats(Telemetry **t, size_t tSize) {

	if(false){ // for debugging
		for(size_t i=0; i<tSize; ++i) {
			printf(	"Telemetry:\n"
					"-----------------------------------------\n"
					"totalTime: %lu\n"
					"busyTime: %lu\n"
					"queueSum: %lu\n"
					"responseTimeSum: %lu\n"
					"responseCount: %lu\n",
					t[i]->totalTime,
					t[i]->busyTime,
					t[i]->queueSum,
					t[i]->responseTimeSum,
					t[i]->responseCount);
		}
	}
	Stats* stats = malloc(sizeof(Stats) * tSize);
	for(size_t i=0; i<tSize; ++i) {
		finalizeStats(t[i], &stats[i]);
		printf(	"-------------------------------------\n"
				"Stats:\n"
				"--------\n"
				"averageQueueSize: %f\n"
				"utilization: %f\n"
				"maxResponseTime: %lu\n"
				"averageResponseTime: %f\n"
				"throughput: %f\n",
				stats[i].averageQueueSize,
				stats[i].utilization,
				stats[i].maxResponseTime,
				stats[i].averageResponseTime,
				stats[i].throughput);
	}

}