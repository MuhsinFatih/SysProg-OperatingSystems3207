#pragma once
#include <sys/types.h>
#include <stdbool.h>
typedef struct node {
    void* val; // any pointer
    int priority; // will not use this time, but it turned out to be easy to implement anyways
    struct node* next;
} pNode;

pNode* new_queue_node(void* val, int priority);
void pop_queue(pNode** head);
void push_queue(pNode** head, void* val, int priority);
bool is_queue_empty(pNode* head);
int size_queue(pNode* head);