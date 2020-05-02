//
//  FAT.h
//  Lab 4
//
//  Created by Kat Osadchuk on 4/17/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#ifndef FAT_h
#define FAT_h

#include <stdio.h>

//struct for a single FAT entry
typedef struct {
    int file_number;
    int next;
} FAT_entry;

//entire FAT table
typedef struct {
    FAT_entry entries[4000];
} FAT;

//struct for a single data block
typedef struct {
    char data[512];
} DATA_entry;

//struct for all data
typedef struct {
    DATA_entry blocks[4000];
} DATA;

//struct for a single directory entry
typedef struct dir1{
    char file_name[20];
    int file_num;
    int size;
    char create_time[20];
    char modified_time[20];
} dir_entry;

//struct for a directory - contains 10 entries
typedef struct dir2{
    char dirname[10];
    int num_entries;
    dir_entry entries[10];
} directory;


enum FAT_codes{empty=-2, end_of_file=-1};

int find_next_free_node(FAT*);
void clear_nodes(int fileNumber, FAT* );
void init_FAT(FAT*);
void mark_node_taken(int, int, int, FAT*);
int get_next_used_block(FAT*, int, int);
void init_dir(directory*);

#endif /* FAT_h */
