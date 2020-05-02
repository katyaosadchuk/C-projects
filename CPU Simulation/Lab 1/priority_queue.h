// Kateryna Osadchuk - Assignment 1


#ifndef priority_queue_h
#define priority_queue_h

#include <stdio.h>

// node containing data for priority queue
typedef struct Node2
{
    int jobNumber;
    int time;
    int type;
    struct Node2 *nextNode;
} node_for_priority;

// function prototypes
void addToPriority(int, int, int, node_for_priority**);
node_for_priority* removeFromPriority(node_for_priority**);

#endif /* priority_queue_h */
