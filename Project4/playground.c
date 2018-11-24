#include <stdio.h>
#include <stdlib.h>

#pragma pack(push, 1)


typedef struct {
    unsigned char first_byte;
    unsigned char start_chs[3];
    unsigned char partition_type;
    unsigned char end_chs[3];
    unsigned long start_sector;
    unsigned long length_sectors;
} __attribute((packed)) PartitionTable;

typedef struct {
    int data7;
    unsigned char data8;
} __attribute((packed, aligned(2))) Sample1;

typedef struct {
    int data7;
    unsigned char data8;
} Sample2;


Sample1 s1 = (Sample1) {.data7 = 1+256+256, .data8 = 5};
Sample2 s2 = (Sample2) {.data7 = 1+256+256, .data8 = 5};

void dumpbytes(unsigned char* charPtr, size_t size) {
    for(int i=size-1; i>=0; --i) {
        printf("%02x ", charPtr[i]);
    }
    printf("\n");
}

int main(int argc, char const **argv)
{
    printf("size:%i\n", sizeof(Sample1));
    printf("s2: ");
    printf("%i %i\n", sizeof(char), sizeof(int));
    while(1) {
        // char c = getw(stdin);
        printf("> ");
        int c;
        scanf("%d", &c);
        s2.data7 = c;
        dumpbytes(&s2, sizeof(s2));

    }
    return 0;
}
