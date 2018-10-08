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

using namespace std;

namespace built_in {
    void init();
    void cd(char* path);
    std::map<string, void*> commands;
}