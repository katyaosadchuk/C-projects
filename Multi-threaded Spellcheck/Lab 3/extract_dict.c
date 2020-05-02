//
//  extract_dict.c
//  Lab 3
//
//  Created by Kat Osadchuk on 3/26/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#include "extract_dict.h"
#include "queue.h"
#include <string.h>
#include <stdlib.h>

//function to extract all words from dictionary file to linkedlist
word_linkedlist* extract(const char* dict){
    char line[128];
    //open dictionary file
    FILE* dictionary = fopen(dict, "r");
    if(!dictionary){
        puts("Dictionary not found!");
        exit(0);
        
    }
    //initialize linkedlist
    word_linkedlist* list = (word_linkedlist*) malloc(sizeof(word_linkedlist));
    initialize_word_linkedlist(list);
    
    //read in all lines of the file
    while(fgets(line, sizeof(line), dictionary) != NULL){
        size_t entry_length = strlen(line);
        //remove newline char
        if(line[entry_length - 1] == '\n'){
            line[entry_length - 1] = '\0';
        }
        //add word to linked list
        add_string(list, line);
    }
    return list;
}
