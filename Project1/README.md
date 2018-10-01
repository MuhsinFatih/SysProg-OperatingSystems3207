# Discrete Event Simulation


This program creates uniformly distributed random events in an attempt to imitate user activity, and simulates the use of resources in the system. This implementation uses a FIFO algorithm: Jobs do not have explicit priority, but they are rather processed in the order that they enter the queue.

## Workspace
Following files are in the workspace:

```bash
main.c # The file where the function main() and most program logic is at.

misc.c
misc.h # various supporting functionality and data structures are implemented in misc.c and misc.h

pqueue.c
pqueue.h # priority queue implementation for CPU and Disk queues

conf.txt # configuration file for the simulation. Program takes arguments ONLY from this file.
```

Output Files:

```bash
stats.log # Statistics for each resource and significant events will be logged in stats.log
```

## Compiling
Workspace contains a `makefile`:

```bash
make && ./program.out # to run
make clean # to clean the output files
```




# Code Documentation
The code takes in arguments from `conf.txt`, which has the following parameters:

```bash
SEED		# seed number for random generator. Unless this number is changed, same output will be produced
INIT_TIME	# start time of the simulation. Typically this is 0
FIN_TIME	# finish time of the simulation. Total simulation time is FIN_TIME - INIT_TIME
ARRIVE_MIN	# minimum time difference between two jobs entering the server
ARRIVE_MAX	# maximum time difference between two jobs entering the server
QUIT_PROB	# quit probability of a job when it has completed its time in CPU. The job will either quit or enter a disk queue
CPU_MIN		# minimum amount of time a job spends on CPU
CPU_MAX		# maximum amount of time a job spends on CPU
DISK1_MIN	# minimum amount of time a job spends on Disk1
DISK1_MAX	# maximum amount of time a job spends on Disk1
DISK2_MIN	# minimum amount of time a job spends on Disk2
DISK2_MAX	# maximum amount of time a job spends on Disk2
```

The header file `misc.h` contains data structures that are important to the implementation. (The details of these data structures are further described via comments):

```c
struct Conf;		// simulation arguments from conf.txt
struct Telemetry;	// For each device in the server, a Telemetry object is created. This object holds the data that will be used to obtain statistics for each device
struct Stats {
    size_t maxQueueSize;        // maximum queue size
    double averageQueueSize;    // average queue size
    double utilization;         // time_the_server_is_busy/total_time
    size_t maxResponseTime;     // max difference in time between the job arrival at a server and the completion of the job at the server
    double averageResponseTime; // average difference in time between the job arrival at a server and the completion of the job at the server
    double throughput;          // number of jobs completed per unit of time
}

struct Job {
    size_t id;          // process id 
    size_t arrivalTime; // time at which the job has entered the server
    size_t burstTime;   // computation time needed on a device at any given time. For example, if the job is in the CPU, burstTime is the time needed to exit CPU
}
struct cpu {
    pNode* queue;       // job queue (linked list, priority queue)
    size_t queueSize;   // job queue capacity
    Job* currentJob;    // job which has entered CPU
    Telemetry telemetry;// telemetry object for CPU
}
// Disks have a similar data structure to the CPU

```

Following functions are used to log events and statistics:

```c
void log_event(size_t time, const char* event); // log an event with timestamp
void log(const char* event);        // log an event
void writeLogToFile(const char* path);  // write the global log to given file
```


The CPU and disks each use a priority queue to store processes that are to be executed. A custom priority queue is implemented in `pqueue.c`. The properties of each node in the queue are:

```c
void* val;
int priority;
struct node* next;
```
* `val` is a generic pointer with no data type. This allows storing any data structure in the queue, which in this case is a `Job` struct.  
* `priority`: Since this is a FIFO implementation, priority is not used in a traditional manner. Arrival time of a job is assigned as its priority.  
* `next`: Next item in the queue

## Simulation walkthrough

in `main.c` -> `main()` function, configuration arguments from `conf.txt` are read and stored in the object `conf`, and following devices are initialized:
```
CPU
disk1
disk2
```
CPU queue capacity is set to 3, and disk queue capacities are set to 4.
In an attempt to make resources of the system dynamic, disks are stored in a table: More disks can be added at will.  
With the `initStats()` invocation, statistics for each device are generated and all set to default initial states.

The `simulate()` invocation starts the server.


### simulate()

Simulation runs until the simulation time (set in `conf.txt`) is over.  
The `userActivity()` invocation attempts to simulate user activity, where a new job is created and added to the `CPU queue` when the simulation time hits the job's `arrivalTime`. Until the arrival time is hit, no new job will be created. This method produces jobs with evenly distributed random arrival time.  

cpuDetermineJob() function is then invoked where:  
Disks are checked for finished jobs. If there are any, and if the `cpu queue` is not full, these jobs are returned to the `cpu queue`.  
If the CPU is idle, the next available job in the queue enters the CPU. If no job is available, CPU stays idle and the function returns `false`. Otherwise, the return value is `true`

If the CPU is not idle, current job is processed.  
If the job has finished its time on cpu, there is an 80% chance by default (can be changed via conf.txt) that the job will arrive at a disk, which is determined by `jobNeedsDisk()`.  

If the job needs to use a disk, the most suitable disk is chosen (in this case, the disk with least jobs in the queue), and the job arrives at this disk and leaves the cpu queue. If there is no suitable disk, CPU will stay idle until one of the disks is available. When a disk is available, there will be a **deadlock** because both devices (cpu and the disk) will be waiting for each other to empty a slot. This deadlock check is implemented in `cpuDetermineJob()` and will be elaborated on shortly.  
If the job doesn't need to use a disk, it will be terminated (`killJob()`)

After handling the leaving of the job from cpu, `discCompute()` will simulate disk activity. If no job is currently being processed, the next available job in the disk queue will enter the disk. Current job's remaining computation time will be decremented by one. If the job has finished, disk will stay idle until cpu fetches the finished job.

`recordCommonStats()` will iterate all devices and record telemetry data

### Deadlock resolution

As mentioned above, a deadlock will happen when the queues of both CPU and all disks are full. The CPU and one or both of the disks will be waiting for each other to free up space in their respective queues. The resolution for this is in the function cpuDetermineJob (the resolution is annotated with `<?deadlock>`) where:  
If both the cpu and one of the disks have finished a job and are full in queue, one extra job will be accepted into the disk temporarily and the job on cpu will arrive at the disk, releasing a slot in the cpu queue. The finished job on disk will then be returned to the cpu, returning to the normal maximum queue size.

The simulation goes on until `simTime` arrives at `conf.FIN_TIME`.

### Statistics
Line annotated by `<?telemetry>`, the telemetry objects of each device are passed into `logStats()`, and `finalizeStats()` function calculates the statistics based on the telemetry. The statistics are then added to the log file after the significant events