#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pqueue.h"



pNode* new_queue_node(void* val, int priority) {
    pNode* node = (pNode*)malloc(sizeof(pNode));
    node->val = val;
    node->next = NULL;
    node->priority = priority;

    return node;
}

// Remove the head and move head to next
// Mutates the head to point the next node
void pop_queue(pNode* head) {
    pNode* temp = head;
    *head = *(head->next); // now the head pointer points to head->next. Old head is destroyed
    free(temp);
}

// push to appropriate position
void push_queue(pNode** head, void* val, int priority) {
    pNode* start = *head;
    pNode* node = new_queue_node(val, priority);
    if(*head == NULL) {
        *head = node; // mutate head to point the new node. nice
        return;
    }
    
    // Special case: The head of the list has higher priority (smaller number)
    // Insert new node before the head
    if((*head)->priority > priority) {
        node->next = *head;
        *head = node;
    } else {
        // Find the position to insert new node
        while(start->next != NULL &&
              start->next->priority < priority){
            start = start->next;
        }
        node->next = start->next;
        start->next = node;
    }
}

bool is_queue_empty(pNode* head) {
    return head == NULL;
}

int size_queue(pNode* head) {
    int size = 0;
    pNode* start = head;
    while(start != NULL){
        start = start->next;
        ++size;
    }
    return size;
}
