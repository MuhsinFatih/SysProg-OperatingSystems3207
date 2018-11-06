# Muhsin Fatih Yorulmaz, tuj54686 - Intro Systems Prog & Oper Sys Fall 2018 Project 3

CFLAGS = -Wall -g -std=c++17 -w
CC = g++
LIB = -lthread

output: spellchecker
	$(CC) $(CFLAGS) $(LIB) server.cpp -o spellchecker
	
output: client
	$(CC) $(CFLAGS) $(LIB) client_benchmark.cpp -o client

clean:
	@rm *.o *.out