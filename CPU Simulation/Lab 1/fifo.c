// Kateryna Osadchuk - Assignment 1

#include "fifo.h"
#include <stdlib.h>
void addToQueue(int jobNum, int time, node_for_queue** head){
    // make new node with job number
    node_for_queue *new = (node_for_queue*) malloc(sizeof(node_for_queue));
    new->jobNumber = jobNum;
    new->arrival_time = time;
    new->nextNode = NULL;
    
    // if queue is empty, make the new node the head of the queue
    node_for_queue *current = *head;
    if(*head == NULL  || (*head)->jobNumber == 0){
        *head = new;
        return;
    }
    
    // if queue is not empty, go to end of queue and add new node there
    while(current->nextNode != NULL){
        current = current->nextNode;
    }
    current->nextNode = new;
}

// function to remove a job from FIFO queue
node_for_queue* removeFromQueue(node_for_queue** head){
    node_for_queue *returningNode = (node_for_queue*) malloc(sizeof(node_for_queue));
    node_for_queue *temp = *head;
    // update struct with data to return
    returningNode->jobNumber = (*head)->jobNumber;
    returningNode->arrival_time = (*head)->arrival_time;
    returningNode->nextNode = NULL;
    // set the new head of queue as the second element in queue and delete first element
    *head = temp->nextNode;
    free(temp);
    return returningNode;
}
