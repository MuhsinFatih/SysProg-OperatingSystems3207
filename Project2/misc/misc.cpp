#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
using namespace std;

#include "misc.hpp"


// source: https://stackoverflow.com/a/37454181/2770195
vector<string> str_split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

vector<char*> vs_to_ch(vector<string> tokens) {
    vector<char*> v;
    for(int i=0; i<tokens.size(); ++i) {
        v.push_back((char*)tokens[i].c_str());
    }
    v.push_back(NULL);
    return v;
}