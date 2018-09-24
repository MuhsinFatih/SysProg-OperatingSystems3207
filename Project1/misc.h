#pragma once
#include <sys/types.h>

struct Conf
{
    uint SEED;
    uint INIT_TIME;
    uint FIN_TIME;
    uint ARRIVE_MIN;
    uint ARRIVE_MAX;
    uint QUIT_PROB;
    uint CPU_MIN;
    uint CPU_MAX;
    uint DISK1_MIN;
    uint DISK1_MAX;
    uint DISK2_MIN;
    uint DISK2_MAX;
};

char* readFile(char *relativePath);

struct Conf readConf(char *relativePath);