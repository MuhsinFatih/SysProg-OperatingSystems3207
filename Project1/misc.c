#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "misc.h"


long fileLength(FILE *handle) {
	fseek(handle, 0, SEEK_END);
	long fsize = ftell(handle);
	rewind(handle);
	return fsize;
}

char* readFile(char *relativePath) {
	FILE *handle = fopen(relativePath, "r");
	size_t fsize;
	if(!handle) {return NULL;}
	
	fsize = fileLength(handle);
	char *buffer = (char*) malloc(sizeof(char) * (fsize + 1));

	size_t readSize = fread(buffer, sizeof(char), fsize, handle);
	buffer[fsize] = '\0';

	if(fsize != readSize) {
		// Incorrect size, something went wrong. Return null
		free(buffer);
		buffer = NULL;
	}
	fclose(handle);
	return buffer;    
}


char** str_split(char* a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}


void fillConf(struct Conf* conf, char* key, char* val) {
	// sorry for the dirty code :)
	if(!strcmp(key, "SEED")) {conf->SEED = atoi(val);return;}
	if(!strcmp(key, "INIT_TIME")) {conf->INIT_TIME = atoi(val);return;}
	if(!strcmp(key, "FIN_TIME")) {conf->FIN_TIME = atoi(val);return;}
	if(!strcmp(key, "ARRIVE_MIN")) {conf->ARRIVE_MIN = atoi(val);return;}
	if(!strcmp(key, "ARRIVE_MAX")) {conf->ARRIVE_MAX = atoi(val);return;}
	if(!strcmp(key, "QUIT_PROB")) {conf->QUIT_PROB = atoi(val);return;}
	if(!strcmp(key, "CPU_MIN")) {conf->CPU_MIN = atoi(val);return;}
	if(!strcmp(key, "CPU_MAX")) {conf->CPU_MAX = atoi(val);return;}
	if(!strcmp(key, "DISK1_MIN")) {conf->DISK1_MIN = atoi(val);return;}
	if(!strcmp(key, "DISK1_MAX")) {conf->DISK1_MAX = atoi(val);return;}
	if(!strcmp(key, "DISK2_MIN")) {conf->DISK2_MIN = atoi(val);return;}
	if(!strcmp(key, "DISK2_MAX")) {conf->DISK2_MAX = atoi(val);return;}
}

struct Conf readConf(char *relativePath) {
	struct Conf conf;
	memset(&conf, 0, sizeof(struct Conf)); // initialize conf to 0
	
	char *confText = readFile(relativePath);
	
	char **lines = str_split(confText,'\n');
	if(lines) {
		for (int i = 0; *(lines + i); ++i) {
			char **kv = str_split(*(lines + i),' '); // split key-value
			if(kv[0] && kv[1]) {
				fillConf(&conf, kv[0], kv[1]);
			}

            free(*(lines + i));
        }
	}


	return conf;
}