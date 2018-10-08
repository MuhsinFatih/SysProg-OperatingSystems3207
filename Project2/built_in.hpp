/*
    Built-in commands for mufash

*/
#pragma once

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
    void cd(int argc, char** argv);
    typedef void(*funPtr)(int argc, char** argv);
    extern std::map<string, funPtr> commands;
    extern std::map<string, int> asdfdfsaga;
    void run(int argc, char** argv);
}