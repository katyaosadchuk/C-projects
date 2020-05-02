//
//  FAT.c
//  Lab 4
//
//  Created by Kat Osadchuk on 4/17/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#include "FAT.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int find_next_free_node(FAT*);
void clear_nodes(int fileNumber, FAT* );
void init_FAT(FAT*);
void mark_node_taken(int, int, int, FAT*);
int get_next_used_block(FAT*, int, int);
void init_dir(directory*);

//initialize FAT to indicate all nodes are empty
void init_FAT(FAT* fat){
    for(int i = 0; i < 4000; i++){
        fat -> entries[i].file_number = empty;
        fat -> entries[i].next = empty;
    }
}

//find the index of next empty node in FAT
int find_next_free_node(FAT* table){
    for(int i = 0; i < 4000; i++){
        FAT_entry node = table -> entries[i];
        if(node.file_number == empty){
            return i;
        }
    }
    return -1;
}

//for a given file and node, get index of where the next part of the file is
int get_next_used_block(FAT* table, int file_number, int current_index){
    FAT_entry node = table->entries[current_index];
    int next = node.next;
    return next;
}

//Updates FAT when files are created or expanded. Sets new_node to store the file number and -1 and updates previous_node to store next=new_node.
void mark_node_taken(int file_number, int new_node, int previous_node, FAT* table){
    FAT_entry* current = &table->entries[new_node];
    current->file_number = file_number;
    current->next = end_of_file;
    
    //when creating a file for the first time and have no previous nodes, set new_node=previous_node
    if(previous_node != new_node){
        FAT_entry* previous = &table->entries[previous_node];
        if(previous->file_number == current->file_number){
            previous->next = new_node;
        }
    }
    
}

//Sets all nodes corresponding with a given file number to empty 
void clear_nodes(int fileNumber, FAT* table){
    int index = fileNumber;
    FAT_entry* node = NULL;
    
    do {
        node = &table->entries[index];
        index = node->next;
        node->file_number = empty;
        node->next = empty;
    }
    while (index != end_of_file);
}
