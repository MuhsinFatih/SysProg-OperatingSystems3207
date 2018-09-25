#pragma once
#include <sys/types.h>
#include "pqueue.h"

struct Conf
{
    uint SEED;
    uint INIT_TIME;
    uint FIN_TIME;

    // jobs enter the system with an interarrival time that is uniformly distributed
    uint ARRIVE_MIN;
    uint ARRIVE_MAX;
    uint QUIT_PROB;
    
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
    uint CPU_MIN;
    uint CPU_MAX;
    uint DISK1_MIN;
    uint DISK1_MAX;
    uint DISK2_MIN;
    uint DISK2_MAX;
    

    // FIFO QUEUES:
    /*
        CPU
        disk1
        disk2
        priority queue (to store events)
    */
    //an event might be something like "a new job entered the system", "a disk read finished", "a job finished at the CPU", etc. 
};
struct Conf readConf(char *relativePath);

struct Context {
    int eip;
    int esp;
    int ebx;
    int ecx;
    int edx;
    int esi;
    int edi;
    int ebp;
};

typedef struct job
{
    // struct Context context;
    int id;
    int arrivalTime;
    int burstTime;
} Job;

typedef struct cpu {
    pNode* queue;
    int queueSize;
    Job currentJob;
} CPU;
typedef struct disk {
    pNode* queue;
    int queueSize;
    Job currentJob;
} Disk;


void initrandom(unsigned seed);
int myrandom(int low, int high);
double rand01(void);


void log_event(size_t time, const char* event);