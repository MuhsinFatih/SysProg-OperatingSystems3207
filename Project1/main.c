#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "misc.h"

struct Conf conf;

size_t simTime = 0;


void simulate() {
	while(simTime < conf.FIN_TIME) {
		
	}
}



int main(int argc, char const *argv[])
{
	printf("hello!\n");

	char *buffer = NULL;
	conf = readConf("conf.txt");
	

	return 0;
}
