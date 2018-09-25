#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "pqueue.h"

typedef struct node {
    void* val; // any pointer
    int priority; // will not use this time, but it turned out to be easy to implement anyways
    struct node* next;
} Node;

Node* newNode(void* val, int priority) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->val = val;
    node->next = NULL;
    node->priority = priority;

    return node;
}
Node* newNode(void* val) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->val = val;
    node->next = NULL;
    node->priority = 0;

    return node;
}

// Remove the head, move head to next
void pop(Node** head) {
    Node* temp = *head;
    (*head) = (*head)->next;
    free(temp);
}

// push to appropriate position
void pushQueue(Node** head, void* val, int priority) {
    Node* start = (*head);
    Node* node = newNode(val, priority);
    
    // Special case: The head of the list has higher priority (smaller number)
    // Insert new node before the head
    if((*head)->priority > priority) {
        node->next = *head;
        (*head) = node;
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

bool isEmpty(Node** head) {
    return (*head) == NULL;
}

