/*
    Built-in commands for mufash

*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <string>

#include <vector>
#include <map>
#include <algorithm>

#define REP(size) for(size_t i=0, length=size; i<length; ++i)
#define REPW(size)  size_t w,length; length=size; while(w<length)
#define vi  vector<int>
#define vs  vector<string>
#define st  size_t
#include <boost/algorithm/string.hpp>

#include "built_in.hpp"

using namespace std;

std::map<string, void*> commands;

void init() {
    commands = {
        {"cd", &cd}
    };
}


void cd(int argc, char** argv) {
    if(chdir(argv[1]) != 0) {
        fprintf(stderr, "cd: %s: %s\n", strerror(errno), path);
    }
}