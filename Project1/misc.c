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

struct Conf readconf() {

}