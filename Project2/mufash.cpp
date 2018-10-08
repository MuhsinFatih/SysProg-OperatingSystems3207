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
#define REPW(size)  size_t w,length; length=size \
                    while(w<length)
#define vi  vector<int>
#define vs  vector<string>
#define st  size_t


#include "misc/diag.cpp"


using namespace std;

void prompt() {

}


extern char** environ; // the last item in the array is a NULL c-string
int main(int argc, char** argv) {
    print_c_arr(argc, argv);
    char *s = *environ;

    for(int i=0;s;++i) {
        cout << s << endl;
        s = *(environ+i);
    }
}