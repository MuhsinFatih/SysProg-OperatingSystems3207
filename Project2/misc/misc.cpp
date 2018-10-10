#include <vector>
#include <string>
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

#include "misc.hpp"


vector<string> str_split(const string& str, const string& delim)
// source: https://stackoverflow.com/a/37454181/2770195
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

char** vs_to_ch(vector<string> tokens) {
    size_t c = tokens.size();
    char** ch = (char**)malloc(c * sizeof(char*));
    
    for(size_t i=0; i<c; ++i) {
        ch[i] = new char[tokens[i].size()+1];
        strcpy(ch[i], tokens[i].c_str());
    }
    
    return ch;
}

string concat_path (string p1, string p2) {
    char delim = '/';
    if(p1[p1.length()-1] == '/') p1.erase(p1.length() -1, 1);
    if(p2[0] == '/') p2.erase(0,1);
    return p1 + '/' + p2;
}

string getcwd_string() {
   char buff[PATH_MAX];
   getcwd(buff, PATH_MAX);
   std::string cwd(buff);
   return cwd;
}

bool isvalid(char buf) {
    return (buf >= '0' && buf <= '9') || (buf >= 'A' && buf <= 'Z') || (buf >= 'a' && buf <= 'z') || buf == '/' || buf == '-' || buf == '_';
}