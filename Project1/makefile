CFLAGS = -Wall -g
CC = gcc

output: main.o misc.o pqueue.o
	$(CC) $(CFLAGS) main.o misc.o pqueue.o -o program.out

main.o: main.c
	gcc -c main.c

misc.o: misc.c
	gcc -c misc.c

pqueue.o: pqueue.c
	gcc -c pqueue.c

clean:
	@rm *.o *.out