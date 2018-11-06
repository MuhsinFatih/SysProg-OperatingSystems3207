# Muhsin Fatih Yorulmaz, tuj54686 - Intro Systems Prog & Oper Sys Fall 2018 Project 3

CFLAGS = -Wall -g -std=c++17 -w
CC = g++
LIB = -lpthread

output: spellchecker.o
	$(CC) $(CFLAGS) $(LIB) server.cpp -o spellchecker
	$(CC) $(CFLAGS) $(LIB) client_benchmark.cpp -o client_benchmark


spellchecker.o: server.cpp
	$(CC) $(CFLAGS) -c server.cpp

client.o: client_benchmark.cpp
	$(CC) $(CFLAGS) -c client_benchmark.cpp

clean:
	@rm -rf *.o *.out client client_benchmark spellchecker *.dSYM