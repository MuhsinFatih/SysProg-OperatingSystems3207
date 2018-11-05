#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
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
#include <mutex>
#include <condition_variable>

#include "utils.cpp"
#include "semaphore.cpp"
#include "colors.h"
#include "consumer_producer_queue.hpp"

using namespace std;
#define LOGFILE "spellchecker.log"
#define DEFAULT_DICTIONARY "dictionary.txt"
#define DEFAULT_PORT 8888
#define NUM_WORKERS 10

/* Forward declarations */
void* connection_handler(void*);
void* worker_handler(void* arg);
void respond(int socket);
/* Forward declerations end */


std::set<string> dict;
vector<pthread_t*> worker_threads;
ConsumerProducerQueue<int> sockets(0);
ConsumerProducerQueue<std::string> log_queue(0);
std::mutex log_mutex;
std::ofstream logFile;


void addsocket(int client_socket) {
    sockets.add(client_socket);
}

void* worker_handler(void* arg) {
    int socket = sockets.consume();
    respond(socket);
    close(socket);
    return 0;
}

void* logger_handler(void* arg) {
    while(true) {
        string logtxt = log_queue.consume();
        std::unique_lock<std::mutex> lock(log_mutex);
        logFile << logtxt;
        logFile.flush();
        printf("%s\n", logtxt.c_str());
    }
    return 0;
}

// test with: "cat <(echo "yey") - | nc 127.0.0.1 8888"
int main(int argc, char** argv) {

    std::ifstream infile(DEFAULT_DICTIONARY);
    logFile.open(LOGFILE, std::ofstream::app);

    string line;
    while(std::getline(infile, line)) {
        dict.insert(line);
    }
    printf("read dictionary: %i items\n", dict.size());
    semaphore sem(5);

    // create worker threads
    
    for(size_t i = 0; i < NUM_WORKERS; ++i)
    {
        pthread_t* worker = new pthread_t();
        if(pthread_create(worker, NULL, worker_handler, NULL) < 0) {
            perror("could not create thread!\n");
        } else {
            worker_threads.push_back(worker);
        }
    }
    printf(MAGENTA "created thread pool\n" RESET);

    // ========================= logger =========================
    {
        pthread_t* logger = new pthread_t();
        if(pthread_create(logger, NULL, logger_handler, NULL) < 0) {
            perror(RED "could not create logger!\n" RESET);
        }
    }


    // ========================= server =========================
    {
        int socket_desc;
        struct sockaddr_in server, client;
        char *message;

        socket_desc = socket(AF_INET, SOCK_STREAM, 0);
        if(socket_desc == -1) printf("could not create socket\n");
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(DEFAULT_PORT);

        // bind
        if(::bind(socket_desc, (struct sockaddr*)&server, sizeof(server))<0) {
            puts("bind failed!");
            return 1;
        }

        //listen
        listen(socket_desc, 3);

        // accept incoming connection
        puts(CYAN "waiting for incoming connections..." RESET);
        socklen_t c = sizeof(client);
        
        while(true) {
            int client_socket = accept(socket_desc, (struct sockaddr*)&client, (socklen_t*)&c);
            cout << client_socket << endl;
            if(client_socket < 0) {
                perror("accept failed!");
                continue;
            }
            puts("connection accepted");

            // produce
            addsocket(client_socket);
        }
    }
    
    return 0;
}

void respond(int socket) {
    string log_txt;
    string word; word.reserve(30);
    int read_size;
    while((read_size = recv(socket, &word[0], 30, 0)) > 0) {
        word = word.c_str(); // TODO: do this properly
        auto start = chrono::high_resolution_clock::now();
        bool found = dict.find(word.c_str()) != dict.end();
        auto end = chrono::high_resolution_clock::now();
        append_format(log_txt,"{\n\ttime passed finding: %i microseconds\n",chrono::duration_cast<chrono::microseconds>(end-start).count());
        string ok = (found ? "OK" : "MISSPELLED");
        string response = word + ok;
        append_format(log_txt, "\t%s %s\n", word.c_str(), ok.c_str());
        write(socket, response.c_str(), response.length());
    }
    if(read_size == 0) {
        log_txt += "\tclient disconnected\n";
        fflush(stdout);
    } else if(read_size == -1) {
        log_txt += "\trecv failed\n";
    }
    log_txt += "}\n";
    
    // cout << log_txt;
    log_queue.add(log_txt);
}