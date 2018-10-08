#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>

#include <vector>
#include <map>
#include <algorithm>

#define REP(size) for(size_t i=0, length=size; i<length; ++i)
#define REPW(size)  size_t w,length; length=size; while(w<length)
#define vi  vector<int>
#define vs  vector<string>
#define st  size_t
#include <boost/algorithm/string.hpp>

#include "misc/colors.h" // namespace color
#include "misc/diag.cpp"
#include "misc/misc.hpp"

using namespace std;

char* USER;
char* HOSTNAME;
char* PATH;
char* PWD;


string _user;
string _host;
string _promptArrow;
void prompt() {    
    printf("%s@%s[%s] %s\n", _user.c_str(), _host.c_str(), PWD, _promptArrow.c_str());
}

void fetchEnviron() {
    PWD = std::getenv("PWD");
    USER = std::getenv("USER");
    HOSTNAME = (char*)malloc(80 * sizeof(char));
    if(gethostname(HOSTNAME,80)) {
        fprintf(stderr, "could not fetch host name!\n");
    } else {
        // remove the domain from hostname
        size_t dotPos = 0;
        for(int i=0; HOSTNAME[i] != '\0'; ++i) {
            if(HOSTNAME[i] == '.') dotPos = i;
        }
        if(dotPos) {HOSTNAME[dotPos] = '\0'; free(*(&HOSTNAME + dotPos));}
    }
    _user = YELLOW; _user.append(USER); _user.append(RESET);
    _host = MAGENTA; _host.append(HOSTNAME); _host.append(RESET);
    _promptArrow = GREEN "➜" RESET;
}

extern char** environ; // the last item in the array is a NULL c-string
int main(int argc, char** argv) {
    print_c_arr(argc, argv);
    // printf("%s\n", std::getenv("PATH")); // get PATH variable to search for executables
    fetchEnviron();
    
    std::string cmd_line;
    while(true) {
        prompt();

        std::getline(std::cin, cmd_line);
        vs str_argv = split(cmd_line, " ");
        vector<char*> cmd_argv;
        for(auto& s : str_argv) {
            cmd_argv.push_back(&s.front());
        }
        print_c_arr(str_argv.size(), str_argv);
        
    }
    return 0;
}