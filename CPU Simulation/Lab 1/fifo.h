// Kateryna Osadchuk - Assignment 1


#ifndef fifo_h
#define fifo_h

#include <stdio.h>

// node for fifo queue
typedef struct Node1
{
    int jobNumber;
    int arrival_time;
    struct Node1 *nextNode;
} node_for_queue;

// functions
void addToQueue(int, int, node_for_queue**);
node_for_queue* removeFromQueue(node_for_queue**);

#endif /* fifo_h */
