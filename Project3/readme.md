# Networked spellchecker

## Introduction
Spellchecker is a simple language server which listens on a network server for words and responds with a message indicating whether the word sent is spelled correctly. It can listen to multiple simultaneous clients at once  
It takes in two optional arguments: dictionary file and port. The default values for dictionary and port are:
```
dictionary.txt
port: 8888
```
Given that a word "word" is sent to the server, the server will respond to that port with:
```
# if the word is spelled correctly:
wordOK
# if the word is spelled incorrectly:
wordMISSPELLED
```

The program also logs all the events and time cost of every lookup in the dictionary in a log file called: `spellchecker.log`

## Program design
I placed library and utility functions in their respected files, and rather than manually implementing each concurrency queue, I created a class called `ConsumerProducerQueue` where I implemented a simple FIFO consumer-producer queue, and used two instances, one for sockets and log_queue. For the logger thread, since there was only need for one mutex lock (only one logger thread), I used a simple lock mechanism to ensure safe use of critical section after consuming a log from log_queue.  
There are 10 worker threads each waiting to consume incoming sockets. Incoming sockets are also stored in a `ConsumerProducerQueue`. Rest of the program is fairly standard, mostly written as in the examples used in the lab and BinaryTides' Socket Programming Tutorial


## Testing
Both netcat and telnet have been tested to work simultaneously. An example test can be run with netcat as follows:
```
$ cat <(echo "yey") - | nc 127.0.0.1 8888
# output: yeyMISSPELLED

$ cat <(echo "hello") - | nc 127.0.0.1 8888
# output: helloOK
```
Also included is a benchmarking client called `client_benchmark` which creates 15 threads which send a correctly spelled and misspelled word ın alternating order on an infinite loop. run 
```
./client_benchmark
```
to start the client for testing. (Note, I was never able to access cis-linux servers, these tests however produce consistent correct results)

## Building
Simply run `make`, and a file named `spellchecker`, and one named `client` will be built.