# Muhsin Fatih Yorulmaz, tuj54686 - Intro Systems Prog & Oper Sys Fall 2018 Project 2

CFLAGS = -Wall -g -std=c++17 -w
CC = g++
LIB = -lreadline -lboost_system -lboost_filesystem

output: mufash.o diag.o built_in.o
	$(CC) $(CFLAGS) $(LIB) mufash.cpp misc/diag.cpp built_in.cpp misc/misc.cpp -o myshell

mufash.o: mufash.cpp
	$(CC) $(CFLAGS) -c mufash.cpp

diag.o: misc/diag.cpp
	$(CC) $(CFLAGS) -c misc/diag.cpp
	
misc.o: misc/misc.cpp
	$(CC) $(CFLAGS) -c misc/misc.cpp

built_in.o: built_in.cpp
	$(CC) $(CFLAGS) -c built_in.cpp

clean:
	@rm *.o *.out