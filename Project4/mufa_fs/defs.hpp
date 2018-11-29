#pragma once

#include <iostream>
#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include <map>
#include <random>
#include <algorithm>
#include <chrono>
#include <functional>
using namespace std;


#define REP(size) for(size_t i=0; i<size; ++i)
#define REPW(size) size_t w=0; while(w<size)
#define vi vector<int>
#define vs vector<string>
#define st size_t
#define vul vector<unsigned long>
#define ul unsigned long

template <typename T>
void printArray(T arr, const st size, const char* heading = "") {
    cout << heading;
    st c = size;

    REP(c) printf(i<c-1 ? "%.3g, " : "%.3g\n", (double)(arr[i]));
}

void dumpbytes_b(unsigned char* charPtr, size_t size) {
    for(int i=size-1; i>=0; --i) {
        printf("%02x ", charPtr[i]);
    }
    printf("\n");
}

void dumpbytes(unsigned char* charPtr, size_t size) {
	for(int i=0; i<size; ++i) {
		printf("%02x ", charPtr[i]);
	}
	printf("\n");
}