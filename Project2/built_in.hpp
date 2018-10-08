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
    void cd(int argc, vs argv);
    typedef void(*funPtr)(int argc, std::vector<string> argv);
    extern std::map<string, funPtr> commands;
    void run(int argc, vs argv);
}