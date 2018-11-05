#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <limits>

#include <vector>
#include <map>
#include <algorithm>

#define REP(size) for(size_t i=0, length=size; i<length; ++i)
#define REPW(size)  size_t w,length; length=size; while(w<length)
#define print_err() fprintf(stderr, "%s\n", strerror(errno));
#define map_contains(map, key) map.find(key) != map.end()
#define vi  vector<int>
#define vs  vector<string>
#define st  size_t
#include <boost/algorithm/string.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        puts("error creating socket");
        return -1;
    }
    auto serv_addr = (struct sockaddr_in) {
        .sin_addr = AF_INET,
        .sin_port = htons(8888)
    };
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1;
    } 
   
    char* hello = "Abel";
    if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        puts("connection failed!");
        return -1;
    }
    send(sock, hello, strlen(hello), 0);
    printf("said hello!\n");
    char buffer[1024];
    int valread = read(sock, buffer, 1024);
    printf("read the response!\n");
    printf("%s\n", buffer);
    close(sock);
    return 0;
}