#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;

#ifndef print_c_arr
#define print_c_arr(argc, argv) for(size_t i = 0; i < argc; i++) { \
        cout << argv[i] << endl;}
#endif