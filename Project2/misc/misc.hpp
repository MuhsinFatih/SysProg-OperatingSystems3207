#pragma once

#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
using namespace std;

vector<string> str_split(const string& str, const string& delim);
vector<char*> vs_to_ch(vector<string> tokens);