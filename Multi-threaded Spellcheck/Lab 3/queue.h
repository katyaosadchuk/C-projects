//
//  queue.h
//  Lab 3
//
//  Created by Kat Osadchuk on 3/21/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#ifndef queue_h
#define queue_h

#include <stdio.h>
typedef struct word_node
{
    char* word;
    struct word_node *next;
} word_node;

typedef struct socket_node
{
    int socket;
    struct socket_node *next;
} socket_node;

typedef struct word_linkedlist
{
    int count;
    word_node *front;
    word_node *rear;
} word_linkedlist;

typedef struct socket_queue
{
    int count;
    socket_node *front;
    socket_node *rear;
} socket_queue;

void initialize_word_linkedlist(word_linkedlist *);
void initialize_socket_queue(socket_queue *);
void add_string(word_linkedlist *, char*);
void enqueue_socket(socket_queue *, int);
int dequeue_socket(socket_queue *);

#endif /* queue_h */
