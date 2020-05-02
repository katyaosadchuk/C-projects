//
//  queue.c
//  Lab 3
//
//  Created by Kat Osadchuk on 3/21/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#include "queue.h"
#include <stdlib.h>
#include <string.h>

//function to initialize linked list
void initialize_word_linkedlist(word_linkedlist *q){
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}
//function to initialize work queue
void initialize_socket_queue(socket_queue *q){
    q->count = 0;
    q->front = NULL;
    q->rear = NULL;
}

//function to add string to linked list
void add_string(word_linkedlist *q, char* new_word){
    //initialize new entry
    word_node *tmp = (word_node*) malloc(sizeof(word_node));
    tmp->word = malloc(strlen(new_word) + 1);
    strcpy(tmp->word, new_word);
    tmp->next = NULL;
    
    //if linked list is empty
    if(q->rear == NULL){
        q->front = q->rear = tmp;
    } else{
        //if linked list is not empty, add to end and update what the "rear" of LL is
        q->rear->next = tmp;
        q->rear = tmp;
    }
    q->count++;
}

//function to add a socket number to queue
void enqueue_socket(socket_queue *q, int socket){
    //initialize new entry
    socket_node *tmp = (socket_node*) malloc(sizeof(socket_node));
    tmp->socket = socket;
    tmp->next = NULL;
    
    //if the queue is empty
    if(q->rear == NULL){
        q->front = q->rear = tmp;
    } else{
        //if the queue is not empty, add to end and update "rear"
        q->rear->next = tmp;
        q->rear = tmp;
    }
    q->count++;
}

//function to remove first element from queue
int dequeue_socket(socket_queue *q){
    //if queue is empty, return -1
    if (q->front == NULL)
        return -1;
    
    //store number to return
    int retval = q->front->socket;
    //new "front" is second element in list
    q->front = q->front->next;
    
    
    // if front becomes NULL, then change rear also as NULL bc then the queue is empty
    if (q->front == NULL)
        q->rear = NULL;
    
    q->count--;
    //return the number stored in element removed
    return retval;
}


