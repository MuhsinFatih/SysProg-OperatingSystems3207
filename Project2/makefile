CFLAGS = -Wall -g -std=c++17 -w
CC = g++

output: mufash.o diag.o built_in.o
	$(CC) $(CFLAGS) mufash.o diag.o built_in.o -o program.out

mufash.o: mufash.cpp
	$(CC) $(CFLAGS) -c mufash.cpp

diag.o: misc/diag.cpp
	$(CC) $(CFLAGS) -c misc/diag.cpp

built_in.o: built_in.cpp
	$(CC) $(CFLAGS) -c built_in.cpp

clean:
	@rm *.o *.out