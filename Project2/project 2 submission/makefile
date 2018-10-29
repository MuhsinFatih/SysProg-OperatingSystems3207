# Muhsin Fatih Yorulmaz, tuj54686 - Intro Systems Prog & Oper Sys Fall 2018 Project 2

CFLAGS = -Wall -g -std=c++17 -w
CC = g++
LIB = -lreadline -lboost_system -lboost_filesystem

output: mufash.o diag.o built_in.o
	$(CC) $(CFLAGS) mufash.cpp misc/diag.cpp built_in.cpp misc/misc.cpp $(LIB) -o myshell


clean:
	@rm *.o *.out