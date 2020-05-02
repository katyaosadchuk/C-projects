//
//  OpenFile.h
//  Lab 4
//
//  Created by Kat Osadchuk on 4/17/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#ifndef OpenFile_h
#define OpenFile_h

//struct that saves info when a file is opened
typedef struct node
{
    int fileNumber;
    int offset;
    struct node *nextNode;
} open_file_node;


open_file_node* add_ll(int, int, open_file_node**);
int remove_ll(open_file_node**, int);
void printList(open_file_node *node);

#endif /* OpenFile_h */
