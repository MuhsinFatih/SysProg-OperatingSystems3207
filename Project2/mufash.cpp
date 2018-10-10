#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/ioctl.h>


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
string PATH;
char* PWD;
extern char** environ; // the last item in the array is a NULL c-string

string _user;
string _host;
string _promptArrow;
void prompt() {
    printf("%s@%s[%s] %s ", _user.c_str(), _host.c_str(), PWD, _promptArrow.c_str());
}

void fetchEnviron() {
    bool debug = true;
    if(debug) {
        PWD = "/Users/muhsinfatih/Documents/CourseMaterial/CIS 3207 - Systems Programming and Operating Systems/Assignments/Project2";
        USER = "muhsinfatih";
        HOSTNAME = "mufa-mbp";
    } else {
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
    }
    _user = YELLOW; _user.append(USER); _user.append(RESET);
    _host = MAGENTA; _host.append(HOSTNAME); _host.append(RESET);
    _promptArrow = GREEN "➜" RESET;
}


enum class command {
    cd = 0,
    clr = 1,
    ls = 2,
    environ = 3,
    echo = 4,
    help = 5,
    pause = 6,
    quit = 7,
};
std::map<string, command> built_in_commands;
void built_in_func(int argc, vs argv) {
    string cmd_str = argv[0];
    command cmd = built_in_commands[cmd_str];
    
    switch (cmd) {
        case command::cd: {
            string path = argv[1];
            
            if(chdir(path.c_str()) == 0) {
                fprintf(stderr, "cd: %s: %s\n", strerror(errno), argv[1].c_str());
            } else {
                PATH = getcwd_string();
            }
            break;
        }
        case command::clr: {
            // struct winsize w;
            // ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            // string ss = string(w.ws_row, '\n');
            // cout << ss << endl;
            cout << "\033[2J\033[1;1H";
        }
        case command::ls: {
            
        }
        case command::environ: {
            for(char **cur = environ; *cur; ++cur) {
                puts(*cur);
            }
        }
        default:
            break;
    }
}



typedef struct exec
{
    string executable_path;
    vector<string> args;
    bool is_piped;
};

int main(int argc, char** argv) {
    print_c_arr(argc, argv);
    // printf("%s\n", std::getenv("PATH")); // get PATH variable to search for executables
    fetchEnviron();
    
    // built_in::init();
    
    built_in_commands = (std::map<string, command>){
        {"cd", command::cd},
        {"clr", command::clr},
        {"dir", command::ls},
        {"ls" , command::ls},
        {"environ", command::environ},
        {"echo", command::echo},
        {"help", command::help},
        {"pause", command::pause},
        {"quit", command::quit}
    };


    std::string cmd_line; // string to store next command line
    while(true) {
        prompt();

        std::getline(std::cin, cmd_line);           // get command line
        vs str_argv = str_split(cmd_line, " ");     // split command line to arguments
        if(!str_argv.size()) continue;
        char** cmd_argv = vs_to_ch(str_argv);
        
/////////////////////
        string cmd = str_argv[0];
        
        vector<exec> cmds;
        exec first = {
            .executable_path = "",
            .args = (vs){""},
            .is_piped = false
        };
        int currentCMD = 0;
        int currentArg = 0;
        int currentStrOffset = 0;


        cmds.push_back(first);
        size_t exec_i;
        
        size_t c = cmd_line.length();

        bool ignorespace = false;

        for(int i=0; i<c; ++i) {
            char cur = cmd_line[i];
            struct exec *curCmd = &cmds[currentCMD];
            
            if(isvalid(cur)) {
                ignorespace = false;
                printf("Valid Char: %c\n", cur);
                curCmd->args[currentArg] += cur;
            } else {
                if(cur == ' ') {
                    if(ignorespace) { continue;}
                    ignorespace = true;
                    curCmd->args.push_back("");
                    printf("New Arg %s\n", curCmd->args[currentArg].c_str());
                    ++currentArg;
                } else if(cur == '|' || cur == ';') {
                    ignorespace = true;
                    
                    if(curCmd->args[currentArg] == "") {
                        curCmd->args.pop_back();
                    }
                    curCmd->is_piped = (cur == '|');

                    ++currentCMD;
                    currentArg = 0;
                    cmds.push_back((exec) {
                        .executable_path = "",
                        .args = (vs){""},
                        .is_piped =  false
                    });
                } else if(cur == '\'') {
                    /*size_t k = i;
                    while(k<c && cmd_line[k]) {

                    }*/
                    printf("Start quote");
                    bool escape = false;
                    for(;i<c;++i) {
                        char cur2 = cmd_line[i];
                        if(cur2 == '\'' && !escape) {
                            break;
                        } else if(cur2 == '\\') {
                            escape = !escape;
                        } else {
                            curCmd->args[currentArg] += cur2;
                        }
                        printf("Quotas %s\n", curCmd->args[currentArg].c_str());                                                        
                    }
                    
                }
            }
        }
        for(size_t i=0; i<cmds.size(); ++i) {
            cout << "-----------------" << endl;
            printf("exec_path:%s\nargs:\n", cmds[i].executable_path.c_str());
            for(size_t k=0; k<cmds[i].args.size();++k) {
                cout << cmds[i].args[k] << endl;
            }
            printf("is_piped:%i\n", cmds[i].is_piped);
        }
        continue;
        if(map_contains(built_in_commands, cmd)) {
            // command is a built-in function
            // built_in::run(str_argv.size(), str_argv);
            built_in_func(str_argv.size(), str_argv);
            continue;
        }


        int pid = fork(); // create new process for the program
        if(pid < 0) { // fork failed
            fprintf(stderr, "fork failed!\n");
        } else if(pid == 0) { // child: redirect standard output to a file
            // close(STDOUT_FILENO); // close pipe to stdout
            // char** a = &cmd_argv;
            int status = execvp(str_argv[0].c_str(), cmd_argv);
            if(status == -1) { // There was an error
                fprintf(stderr, "%s\n", strerror(errno));
                exit(errno);
            }
            
            return 0;
        } else {
            int status = 0;
            wait(&status);
            // status = WEXITSTATUS(status);
            // printf("child exited with code %d\n",status);
        }

    }
    return 0;
}
