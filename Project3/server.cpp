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

using namespace std;

int main() {
    // set up connection to client
    struct sockaddr_in server, client;
    char* message;

    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_desc == -1) puts("Error creating socket!\n");
    // Prepare the sockaddr_instructure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8080);
    // Bind (connect the server's socket address to the socket descriptor); print a message and exit the program if an error occured
    if(::bind(socket_desc, (struct sockaddr*)&server, sizeof(server)) < 0) {
        puts("error: bind failed");
        return 1;
    }
    puts("bind done");
    // Listen (converts the active socket to a LISTENING socket; can accept connections)
    listen(socket_desc, 3);
    puts("waiting for incoming connections...");
    int new_socket;
    int c = sizeof(struct sockaddr_in);
    while(new_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c)) {
        if(new_socket < 0) {
            perror("error: accept failed");
            return -1;
        }
        puts("connection accepted!");
        string buffer; buffer.reserve(1024);
        int val = read(new_socket, (char*)buffer.data(), 1024);
        printf("%s\n", buffer.c_str());
        close(new_socket);
    }
}