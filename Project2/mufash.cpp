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
#define print_err() fprintf(stderr, "%s\n", strerror(errno));
#define map_contains(map, key) map.find(key) != map.end()
#define vi  vector<int>
#define vs  vector<string>
#define st  size_t
#include <boost/algorithm/string.hpp>

#include "misc/colors.h" // namespace color
#include "misc/diag.cpp"
#include "misc/misc.hpp"
#include "built_in.hpp"


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
    
    
    built_in::init();

    if(0) {
        
    }


    std::string cmd_line; // string to store next command line
    while(true) {
        prompt();

        std::getline(std::cin, cmd_line);       // get command line
        vs cmd_argv = split(cmd_line, " ");     // split command line to arguments
        
        print_c_arr(cmd_argv.size(), cmd_argv);
        
        string cmd = cmd_argv[0];
        
        if(map_contains(built_in::commands, cmd)) {
        //     // (*built_in::commands.at(cmd))(0,NULL);
        //     // cout << built_in::run(cmd_argv.size(), (char**)cmd_argv.data()) << endl;
        }
        built_in::asdfdfsaga.size();

        int pid = fork(); // create new process for the program
        if(pid < 0) { // fork failed
            fprintf(stderr, "fork failed!\n");
        } else if(pid == 0) { // child: redirect standard output to a file
            // close(STDOUT_FILENO); // close pipe to stdout
            
            int status = execvp(cmd_argv[0].c_str(), (char**)cmd_argv.data());
            if(status == -1) { // There was an error
                fprintf(stderr, "%s\n", strerror(errno));
                exit(errno);
            }
            
            return 0;
        } else {
            int status = 0;
            wait(&status);
            status = WEXITSTATUS(status);
            // printf("child exited with = %d\n",status);
        }

    }
    return 0;
}