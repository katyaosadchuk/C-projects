//
//  OpenFile.c
//  Lab 4
//
//  Created by Kat Osadchuk on 4/17/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#include "OpenFile.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//keep all open files in a linked list in memory
//function to add a node to the list
open_file_node* add_ll(int file_number, int offset, open_file_node** head){
    open_file_node *new = (open_file_node*) malloc(sizeof(open_file_node));
    new->fileNumber = file_number;
    new->offset = offset;
    new->nextNode = NULL;
    
    open_file_node *current = *head;
    if(*head == NULL){
        *head = new;
        return new;
    }
    
    while(current->nextNode != NULL){
        current = current->nextNode;
    }
    current->nextNode = new;
    
    return new;
}


//function to remove node from list
int remove_ll(open_file_node** head, int file_number){
    open_file_node* temp = *head, *prev = NULL;
    if (temp != NULL && temp->fileNumber == file_number){
        *head = temp->nextNode;
        free(temp);
        return 0;
    }
   
    while (temp != NULL && temp->fileNumber != file_number){
        prev = temp;
        temp = temp->nextNode;
    }
    
    if (temp == NULL){
        return -1;
    }
    
    prev->nextNode = temp->nextNode;
    free(temp);
    return 0;
}

//helper function to print list
void printList(open_file_node *node){
    while (node != NULL){
        printf(" %d ", node->fileNumber);
        node = node->nextNode;
    }
} 
