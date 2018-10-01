# Discrete Event Simulation


This program creates uniformly distributed random events in an attempt to imitate user activity, and simulates the use of resources in the system.

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
SEED        # seed number for random generator. Unless this number is changed, same output will be produced
INIT_TIME   # start time of the simulation. Typically this is 0
FIN_TIME    # finish time of the simulation. Total simulation time is FIN_TIME - INIT_TIME
ARRIVE_MIN  # minimum time difference between two jobs entering the server
ARRIVE_MAX  # maximum time difference between two jobs entering the server
QUIT_PROB   # quit probability of a job when it has completed its time in CPU. The job will either quit or enter a disk queue
CPU_MIN     # minimum amount of time a job spends on CPU
CPU_MAX     # maximum amount of time a job spends on CPU
DISK1_MIN   # minimum amount of time a job spends on Disk1
DISK1_MAX   # maximum amount of time a job spends on Disk1
DISK2_MIN   # minimum amount of time a job spends on Disk2
DISK2_MAX   # maximum amount of time a job spends on Disk2
```

