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
#include <pthread.h>

using namespace std;

void *connection_handler(void*);

int main() {
    int socket_desc, new_socket, c;
    struct sockaddr_in server, client;
    char *message;

    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc == -1) printf("could not create socket\n");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // bind
    if(::bind(socket_desc, (struct sockaddr*)&server, sizeof(server))<0) {
        puts("bind failed!");
        return 1;
    }
    puts("bind done!");

    //listen
    listen(socket_desc, 3);;

    // accept incoming connection
    puts("waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    while(new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c)) {
        puts("connection accepted");
        // respond the client
        message = "hello client, I have received your connection and now I will assign a handler for you\n";
        write(new_socket, message, strlen(message));
        
        // create thread
        pthread_t sniffer_thread;
        if(pthread_create(&sniffer_thread, NULL, connection_handler, (void*) &new_socket) < 0) {
            perror("could not create thread");
            return 1;
        }
        puts("handler assigned");
    }
    if(new_socket < 0) {
        perror("accept failed!");
        return 1;
    }
    return 0;
}

void *connection_handler(void *socket_desc) {
    int sock = *(int*)socket_desc;
    char* message;

    // send messages to client
    message = "greetings! I am your connection handler!\n";
    write(sock, message, strlen(message));

    message = "Its my duty to communicate with you";
    write(sock, message, strlen(message));

    return 0;
}