#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>

#include "misc.h"

struct Conf conf;

size_t simTime = 0;
// #define numOfJobs 1000
struct Job priorityQueue[4 /*(-1 (END event)*/)]; // curent events


void simulate() {
	while(simTime < conf.FIN_TIME) {
		
	}
}

void JOB_ARRIVAL_EVENT() {



}

size_t jobCount = 0;
struct Job generateJob() {
	struct Job job = {
		.arrivalTime = myrandom(conf.ARRIVE_MIN, conf.ARRIVE_MAX),
		.burstTime = myrandom(conf.CPU_MIN, conf.CPU_MAX)
	};
	return job;
}

int main(int argc, char const *argv[])
{
	printf("hello!\n");

	char *buffer = NULL;
	conf = readConf("conf.txt");
	
	initrandom(conf.SEED);

	struct Job firstJob = generateJob();


	for(size_t i = 0; i < numOfJobs; ++i) {
		// jobs[i] = (struct Job) {
		// 	.arrivalTime = myrandom(conf.ARRIVE_MIN, conf.ARRIVE_MAX),
		// 	.burstTime = myrandom(conf.CPU_MIN, conf.CPU_MAX)
		// };

		
	}
	

	return 0;
}
