#include <stdio.h>
#include <stdlib.h>
#define SIZE 1000

void initrandom(unsigned seed) {
    // call before generating random numbers
    #define RANDSIZE 256
    static char randstate[RANDSIZE];
    initstate(seed, randstate, RANDSIZE);
}

int myrandom(int low, int high) {
    return(low + random() % (high-low+1));
}

double rand01(void) {
    return(random()/(double)RAND_MAX);
}


int main() {
    int i;
    double table[SIZE];
    double sum = 0;

    initrandom(1357);
    for (i=0; i<SIZE; i++) {
        table[i] = rand01();
        sum += table[i];
        printf("%f\n", table[i]);
    }
    printf("the average is %f\n", sum/(float)SIZE);
}