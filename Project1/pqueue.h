#pragma once
#include <sys/types.h>
#include <stdbool.h>
typedef struct node {
    void* val; // any pointer
    int priority;
    struct node* next;
} pNode;

pNode* new_queue_node(void* val, int priority);
void pop_queue(pNode** head);
void push_queue(pNode** head, void* val, int priority);
void push_queue_end(pNode** head, void* val);
bool is_queue_empty(pNode* head);
size_t size_queue(pNode* head);