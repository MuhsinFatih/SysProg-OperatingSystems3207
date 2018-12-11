# Muhsin Fatih Yorulmaz
# tuj54686
# Intro Systems Prog & Oper Sys Fall 2018 Project 4

CFLAGS = -Wall -g -std=c++17 -w
CC = g++
LIB = -lreadline

all: main

main: main.o
	$(CC) $(CFLAGS) $(LIB) main.o -o main

main.o: main.cpp
	$(CC) $(CFLAGS) $(LIB) -c main.cpp


clean:
	@rm -rf main *.o *.out client client_benchmark spellchecker *.dSYM