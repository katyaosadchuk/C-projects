// Kateryna Osadchuk - Assignment 1

#include "priority_queue.h"
#include <stdlib.h>

// function to add element to a priority queue
void addToPriority(int time, int jobNum, int type, node_for_priority** head){
    // create a new priority queue node with necessary info
    node_for_priority *new = (node_for_priority*) malloc(sizeof(node_for_priority));
    new->time = time;
    new->jobNumber = jobNum;
    new->type = type;
    new->nextNode = NULL;
    
    // if priority queue is empty, set the new node as the head of priority queue
    node_for_priority *current = *head;
    if( *head == NULL ||(*head)->jobNumber == 0){
        *head = new;
        return;
    }
    
    // if priority queue is not empty but new node has higher priority than first element, set new element as head of priority queue
    if (current->time > time) {
        new->nextNode = current;
        *head = new;
    } else {
        // otherwise, iterate to correct position in priority queue and add node there
        while ((current->nextNode != NULL) && (current->nextNode->time < time)) {
            current = current->nextNode;
        }
        new->nextNode = current->nextNode;
        current->nextNode = new;
    }
    
    
}

// function to remove node from priority queue
node_for_priority* removeFromPriority(node_for_priority** head){
    node_for_priority *temp = *head;
    
    // save node's data in a new struct to return
    node_for_priority* data = (node_for_priority*) malloc(sizeof(node_for_priority));
    data->jobNumber = temp->jobNumber;
    data->time = temp->time;
    data->type = temp->type;
    data->nextNode = NULL;
    
    // set the head as the second element in queue, delete first node, update number of events in priority queue
    *head = (*head)->nextNode;
    free(temp);
    return data;
    
}
