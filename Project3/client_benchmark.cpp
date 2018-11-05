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
#include <chrono>
#include <thread>

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

void* client(void* arg) {
    while(true) {
        printf("thread_id:%i\n", std::this_thread::get_id());
        int sock;
        if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            // puts("error creating socket");
        }
        auto serv_addr = (struct sockaddr_in) {
            .sin_addr = AF_INET,
            .sin_port = htons(8888)
        };
        if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)  
        { 
            printf("\nInvalid address/ Address not supported \n"); 
        } 
    
        char* hello = "Abel";
        if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            puts("connection failed!");
        } else {
            write(sock, hello, strlen(hello));
            printf("said hello!\n");
            char buffer[1024];
            int valread = read(sock, buffer, 1024);
            printf("read the response!\n");
            printf("%s\n", buffer);
        }
        close(sock);
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


int main() {
    pthread_t* w1;
    for(size_t i=0; i<15; ++i) {
        pthread_t* worker = new pthread_t();
        w1 = worker;
        if(pthread_create(worker, NULL, client, NULL) < 0) {
            perror("could not create thread!\n");
        }
    }
    pthread_join(*w1, NULL);
}

