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
#include <boost/filesystem.hpp>
#include <limits>

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


#include <readline/readline.h>
#include <readline/history.h>

using namespace std;
namespace fs = boost::filesystem;


char* USER;
char* HOSTNAME;
string PATH;
string CWD;
extern char** environ; // the last item in the array is a NULL c-string
std::map<string, string> executables;


string _user;
string _host;
string _promptArrow;

char promptBuffer[PATH_MAX];

char* prompt() {
    sprintf(promptBuffer, "%s@%s[%s] %s ", _user.c_str(), _host.c_str(), CWD.c_str(), _promptArrow.c_str());
    cout << promptBuffer;
    return "";
}

void fetchExecutables() {
    vs dirs = str_split(PATH, ":");
    for(string &d : dirs) {
        for (auto &p : fs::directory_iterator(d)) {
            try {
                if(!fs::is_directory(p))
                    executables.insert(std::pair<string,string>(p.path().filename().string(), p.path().string()));
            } catch(fs::filesystem_error &e) { 
                if(e.code() != boost::system::errc::permission_denied)
                    printf("%s: %s\n", p.path().c_str(), e.code().message().c_str());
            }
        }
    }
}

void fetchEnviron() {
    bool debug = false;
    if(debug) {
        CWD = "/Users/muhsinfatih/Documents/CourseMaterial/CIS 3207 - Systems Programming and Operating Systems/Assignments/Project2";
        USER = "muhsinfatih";
        HOSTNAME = "mufa-mbp";
        PATH = "/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin";
    } else {
        CWD = std::getenv("PWD");
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
        PATH = std::getenv("PATH");
    }
    fetchExecutables();
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
std::map<command, bool> is_parent = {
    {command::cd, 1},
    {command::clr, 0},
    {command::ls, 0},
    {command::environ, 0},
    {command::echo, 0},
    {command::help, 0},
    {command::pause, 1},
    {command::quit, 1},
};
std::map<string, command> built_in_commands;
void built_in_func(string executable_path, vs argv) {
    command cmd = built_in_commands[executable_path];
    
    switch (cmd) {
        case command::cd: {
            string path = argv[1];
            
            if(chdir(path.c_str()) != 0) {
                fprintf(stderr, "cd: %s: %s\n", strerror(errno), argv[1].c_str());
            } else {
                CWD = getcwd_string();
                setenv("CWD", CWD.c_str(), 1);
            }
            break;
        }
        case command::clr: {
            // struct winsize w;
            // ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
            // string ss = string(w.ws_row, '\n');
            // cout << ss << endl;
            cout << "\033[2J\033[1;1H";
            break;
        }
        case command::ls: {
            if(argv.size()-1 == 0) // no directory is specified: assume "."
                argv.push_back(".");
            if(argv.size()-1 /* minus the executable */  > 1) {
                for(size_t i=1; i<argv.size(); ++i) {
                    auto &d = argv[i];
                    printf("%s:\n", d.c_str());
                    for(auto &p : fs::directory_iterator(d)) {
                        printf("%s\n", p.path().filename().string().c_str());
                    }
                    cout << std::endl;
                }
            } else {
                for(size_t i=1; i<argv.size(); ++i) {
                    auto &d = argv[i];
                    for(auto &p : fs::directory_iterator(d)) {
                        printf("%s\n", p.path().filename().string().c_str());
                    }
                    cout << std::endl;
                }
            }
            break;
        }
        case command::environ: {
            for(char **cur = environ; *cur; ++cur) {
                puts(*cur);
            }
            break;
        }
        case command::echo: {
            for(size_t i=1;i<argv.size();++i) {
                cout << argv[i];
            }
            cout << std::endl;
            break;
        }
        case command::help: {

            break;
        }
        case command::pause: {
            while(cin.get() != '\n');
            break;
        }

        case command::quit: {
            exit(SIGTTIN);
        }
        default:
            break;
    }
}

enum class redirection
{none, pipe, redir_output, redir_input, append_output};

typedef struct exec
{
    pid_t pid;
    string executable_path;
    bool built_in;
    vector<string> args;
    redirection redir;
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
        {"quit", command::quit},
        {"exit", command::quit}
    };

    char* buf;
    std::string cmd_line; // string to store next command line
    while ((buf = readline(prompt())) != nullptr) {
        if (strlen(buf) > 0) {
            add_history(buf);
        }

        //
        cmd_line = std::string(buf);
        // std::getline(std::cin, cmd_line);           // get command line
        vs str_argv = str_split(cmd_line, " ");     // split command line to arguments
        if(!str_argv.size()) continue;
        char** cmd_argv = vs_to_ch(str_argv);
        
        string cmd = str_argv[0];

        
        // lexer ##########################3

        vector<exec> cmds;
        exec first = {
            .pid = NULL,
            .executable_path = "",
            .built_in = false,
            .args = (vs){""},
            .redir = redirection::none
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
                curCmd->args[currentArg] += cur;
            } else {
                if(cur == ' ') {
                    if(ignorespace) { continue;}
                    ignorespace = true;
                    curCmd->args.push_back("");

                    ++currentArg;
                } else if(cur == ';' || cur == '|' || cur == '>') {
                    ignorespace = true;
                    
                    if(curCmd->args[currentArg] == "") {
                        curCmd->args.pop_back();
                    }
                    if(cur == ';') curCmd->redir = redirection::none;
                    else if(cur == '|') curCmd->redir = redirection::pipe;
                    else if(cur == '>') {
                        if(i != c-1 && cmd_line[i+1] == '>') {
                            curCmd->redir = redirection::append_output;
                            ++i;
                        }
                        else curCmd->redir = redirection::redir_output;
                    } else if(cur == '<' && currentCMD != 0) {
                        cmds[currentCMD-1].redir = redirection::redir_input;
                    }
                    ++currentCMD;
                    currentArg = 0;
                    cmds.push_back((exec) {
                        .pid = NULL,
                        .executable_path = "",
                        .built_in = false,
                        .args = (vs){""},
                        .redir = redirection::none
                    });
                } else if(cur == '\\') {
                    // take next char no matter what
                    assert(cmd_line.size() >= i+1);
                    curCmd->args[currentArg] += cmd_line[++i];
                } else if(cur == '\'') {
                    bool escape = false;
                    
                    for(i++;i<c;++i) {
                        char cur2 = cmd_line[i];
                        if(cur2 == '\'' && !escape) {
                            break;
                        } else if(cur2 == '\\') {
                            escape = !escape;
                        } else {
                            escape = false;
                            curCmd->args[currentArg] += cur2;
                        }
                    }
                } else if(cur == '#') {
                    for(i++;i<c;++i) {
                        if(cmd_line[i] == '#') {
                            break;
                        }
                    }
                }
            }
        }
        struct exec *lastCMD = &cmds[cmds.size()-1];
        if(lastCMD->args.back() == "") {
            lastCMD->args.pop_back(); // parser creates extra arg if there is whitespace at the end, delete it
        }
        for(size_t i=0; i<cmds.size(); ++i) {
            cmds[i].executable_path = cmds[i].args[0];
        }

        vector<int*> prev_pipe = {};
        bool pipe_disconnected = false;
        bool skip_cmd = false;

        for(size_t i=0; i<cmds.size(); ++i) {
            cout << "-----------------" << endl;
            printf("exec_path:%s\nargs:\n", cmds[i].executable_path.c_str());
            for(size_t k=0; k<cmds[i].args.size();++k) {
                cout << cmds[i].args[k] << endl;
            }
            if(skip_cmd) {skip_cmd = false; continue;}
            exec cmd = cmds[i];
            if(map_contains(built_in_commands, cmd.executable_path)) {
                printf(GREEN "command (%s) is built-in!\n" RESET, cmd.executable_path.c_str());
                cmd.built_in = true;
            } else {
                char* rp = (char*) malloc(PATH_MAX * sizeof(char));
                if((rp = realpath(cmd.executable_path.c_str(), rp)) != NULL)
                    cmd.executable_path = rp;
                else {
                    cmd.executable_path = executables[cmd.executable_path];
                }
                free(rp);
                printf(CYAN "realpath: %s\n" RESET, cmd.executable_path.c_str());
            }
            int* pipefd;
            if(cmd.redir == redirection::pipe) {
                pipefd = (int*) malloc(2 * sizeof(int));
                pipe(pipefd);
            }
            if(i == 0 || !(cmds[i-1].redir == redirection::pipe))
                pipe_disconnected = true;
            else if(pipe_disconnected) pipe_disconnected = false;

            if(cmd.built_in && is_parent[built_in_commands[cmd.executable_path]] ){
                // special case for commands that change the parent process's environment
                built_in_func(cmd.executable_path, cmd.args);
                continue;
            }
            pid_t pid = fork(); if(pid < 0) {fprintf(stderr, "fork failed for cmd%i!\n", i); exit(1);}
            if(pid == 0) { // child
                { // pipe
                    if(!pipe_disconnected && prev_pipe.size() != 0) {
                        dup2(prev_pipe.back()[STDIN_FILENO], STDIN_FILENO);
                        {for(size_t i=prev_pipe.size();i-->0;)
                            close(prev_pipe[i][STDOUT_FILENO]);} // wait for previous write pipes to finish
                    }
                    if(cmd.redir == redirection::pipe && i != cmds.size()-1) {
                        dup2(pipefd[STDOUT_FILENO], STDOUT_FILENO);
                    }
                } { // redirection
                    if((cmd.redir == redirection::redir_output || cmd.redir == redirection::append_output) && i != cmds.size()-1) {
                        string ap = fs::absolute(cmds[i+1].executable_path).string();
                        int fd = open(ap.c_str(), O_CREAT|O_WRONLY| (cmd.redir == redirection::redir_output ? O_TRUNC : O_APPEND), S_IRUSR | S_IWUSR);
                        dup2(fd, STDOUT_FILENO);
                    }
                    else if(cmd.redir == redirection::redir_input && i != cmds.size()-1) {
                        string ap = fs::absolute(cmds[i+1].executable_path).string();
                        int fd = open(ap.c_str(), O_RDONLY);
                        dup2(fd, STDIN_FILENO);
                    }
                }
                
                if(cmd.built_in) {
                    built_in_func(cmd.executable_path, cmd.args);
                    exit(0);
                } else {
                    char** args = vs_to_ch(cmd.args);
                    args = (char**) realloc(args, (cmd.args.size()+1) * sizeof(char*));
                    args[cmd.args.size()] = NULL; // add null to the end
                    execv(cmd.executable_path.c_str(), args);
                    fprintf(stderr, "child %i failed!\n", i);
                    exit(1);
                }
            }

            cmd.pid = pid;
            if(cmd.redir == redirection::pipe)
                prev_pipe.push_back(pipefd);
            if(cmd.redir == redirection::redir_output || cmd.redir == redirection::append_output)
                skip_cmd = true;

            
            // ilk argümanı al
            // eğer path a benziyorsa tam yolu bul, // man realpath // RTFM :D
            // eğer benzemiyorsa path enviroment ile bbinary i bulmaya çalış
            // executable olduğundan emin ol
            // ???
            // profit

            // step = 0
            // cmds.size();
            // 

            printf("is_piped:%i\n", cmds[i].redir == redirection::pipe);
        }

        // wait for all pipes to close
        for(size_t i=0; i<prev_pipe.size(); ++i) {
            close(prev_pipe[i][STDIN_FILENO]);
            close(prev_pipe[i][STDOUT_FILENO]);
        }
        // wait for children to finish
        for(size_t i=0; i<cmds.size(); ++i) {
            int wstatus;
            waitpid(cmds[i].pid, &wstatus, NULL);
        }

    }
    return 0;
}
