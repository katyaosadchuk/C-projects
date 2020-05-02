//
//  main.c
//  Lab 4
//
//  Created by Kat Osadchuk on 4/10/19.
//  Copyright © 2019 Kat Osadchuk. All rights reserved.
//

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "OpenFile.h"
#include "FAT.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#define NUM_BLOCKS 4000
#define SIZE_DATA_BLOCK 512
#define SIZE_FAT_ENTRY 8
#define NUM_DIRS 10
#define SIZE_DIRS 696


enum mode{file, dir};
enum open_mode{read_mode, write_mode, append_mode};
int current_number_of_dirs = 0;

DATA* data;
FAT* FAT_table;
directory* dirs;
open_file_node** head;

int my_create( char*);
int my_delete(char*);
open_file_node* my_open(char* path, int mode);
void my_close(open_file_node* node);
void create_dir(char*);
void delete_dir(char*);
int my_read(open_file_node*, char buff[], int);
int my_write(open_file_node*, char buff[], int);
int init(void);
int parse_path(const char *, char *names[100]);
char* timestamp(void);


int parse_path(const char *path, char *names[100]){
    char *token;
    char *temp = (char*) malloc(sizeof(path));
    int count=0;
    
    strcpy(temp, path);
    while((token = strsep(&temp, "/")) != NULL){
        names[count] = token;
        count++;
    }
    names[count] = NULL;
    return count;
}

char* timestamp(){
    time_t timer;
    char* buffer =(char*) malloc(sizeof(char*));
    struct tm* tm_info;
    
    time(&timer);
    tm_info = localtime(&timer);
    
    strftime(buffer, 26, "%Y-%m-%d_%H:%M:%S", tm_info);
    
    return buffer;
}

int my_create(char* path){
    char *path_names[100];
    int count = parse_path(path, path_names);
    
    //add to FAT
    int index = find_next_free_node(FAT_table);
    int file_num = index;
    if(index < 0){
        puts("Error! FAT is full.");
        return -1;
    } else {
        mark_node_taken(file_num, index, index, FAT_table);
    }

    //find parent directory based on path name
    char* parent = path_names[count-2];
    int i;
    for(i = 0; i< NUM_DIRS; i++){
        directory* dir = &dirs[i];
        char* dir_name = dir->dirname;
        if(strcmp(dir_name, parent) == 0){
            break;
        }
    }
    
    //add entry and metadata to parent directory
    directory* parent_dir = &dirs[i];
    int num_current_entries = parent_dir->num_entries;
    parent_dir->entries[num_current_entries].file_num = file_num;
    parent_dir->entries[num_current_entries].size = 0;
    char* time = timestamp();
    strcpy(parent_dir->entries[num_current_entries].create_time, time);
    strcpy(parent_dir->entries[num_current_entries].modified_time, time);
    strcpy(parent_dir->entries[num_current_entries].file_name, path_names[count-1]);
    parent_dir->num_entries++;
    
    
    return file_num;
}

int my_delete(char* path){
    char *path_names[100];
    int count = parse_path(path, path_names);
    
    //find parent directory based on path name
    char* parent = path_names[count-2];
    int i;
    for(i = 0; i< NUM_DIRS; i++){
        directory* dir = &dirs[i];
        char* dir_name = dir->dirname;
        if(strcmp(dir_name, parent) == 0){
            break;
        }
    }
    
    //check if file exists
    directory* parent_dir = &dirs[i];
    int found = -1;
    int j;
    for(j = 0; j < 20; j++){
        char* entry_name = parent_dir->entries[j].file_name;
        found = strcmp(entry_name, path_names[count-1]);
        if(found == 0){
            break;
        }
    }
    if(found != 0){
        puts("Error! File doesn't exist.");
        return -1;
    }
    
    //get file num
    int file_num = parent_dir->entries[j].file_num;
   
    //clear fat nodes
    clear_nodes(file_num, FAT_table);
    
    //remove entry from parent dir
    parent_dir->entries[j].file_num = 0;
    parent_dir->entries[j].size = 0;
    strcpy(parent_dir->entries[j].create_time, " ");
    strcpy(parent_dir->entries[j].modified_time, " ");
    strcpy(parent_dir->entries[j].file_name, " ");
    
    parent_dir->num_entries--;
    
    puts("File deleted");
    return 0;
}

open_file_node* my_open(char* path, int mode){
    char* path_names[100];
    int count = parse_path(path, path_names);
    open_file_node *node = NULL;
    
    //go to parent directory
    char* parent = path_names[count-2];
    int i;
    for(i = 0; i< NUM_DIRS; i++){
        directory* dir = &dirs[i];
        char* dir_name = dir->dirname;
        if(strcmp(dir_name, parent) == 0){
            break;
        }
    }
    
    //check if file exists
    directory* parent_dir = &dirs[i];
    bool file_exists = false;
    int j;
    for(j = 0; j < 20; j++){
        char* entry_name = parent_dir->entries[j].file_name;
        if(strcmp(entry_name, path_names[count-1]) == 0){
            file_exists = true;
            break;
        }
    }
    
    //get file num
    int file_num = parent_dir->entries[j].file_num;
    
    //if mode is read, offset is 0
    if(mode == read_mode){
        if(file_exists){
            node = add_ll(file_num, 0, head);
        } else {
            return NULL;
        }
    }
    
    //if mode is write and file exists, offset is 0
    //if file doesn't exist, make file
    else if(mode == write_mode){
        if(file_exists){
            node = add_ll(file_num, 0, head);
        } else {
            int file_num = my_create(path);
            node = add_ll(file_num, 0, head);
        }
    }
    
    //if file exists, offset is end of file
    //if file doesn't exist, create file and set offset = 0
    else if(mode == append_mode){
        if(file_exists){
            int file_size = parent_dir->entries[j].size;
            node = add_ll(file_num, file_size, head);
        } else {
            int file_num = my_create(path);
            node = add_ll(file_num, 0, head);
        }
    }
    
    return node;
}

void my_close(open_file_node* node){
    int file_num = node->fileNumber;
    remove_ll(head, file_num);
}

//function to create a new directory
void create_dir(char* name){
    if(current_number_of_dirs == NUM_DIRS){
        puts("ERROR. Can't make more than 10 directories. Sorry");
        return;
    }
    
    directory* new = &dirs[current_number_of_dirs];
    new->num_entries = 0;
    strcpy(new->dirname, name);
    
    current_number_of_dirs++;
}

//function to delete a directory
void delete_dir(char* name){
    if(strcmp(name, "root") == 0){
        puts("ERROR. Can't delete root directory.");
        return;
    }
    
    int i;
    for(i = 1; i < current_number_of_dirs; i++){
        directory* current = &dirs[i];
        char* current_name = current->dirname;
        if(strcmp(current_name, name) == 0){
            break;
        }
    }
    
    directory* to_delete = &dirs[i];
    if(to_delete->num_entries != 0){
        puts("Error! Can't delete nonempty directory.");
        return;
    } else {
        strcpy(to_delete->dirname, "");
        to_delete->num_entries = -1;
    }
    current_number_of_dirs--;
}


int my_read(open_file_node* node, char buff[], int length){
    int file_num = node->fileNumber;
    char* string = data->blocks[file_num].data;
    int bytes_read = 0;
    int position = node->offset;
    int buff_pos = 0;
    int current_block = file_num;
    
    while(bytes_read < length){
        memcpy(&buff[buff_pos], &string[position], 1);
        bytes_read++;
        node->offset++;
        position++;
        buff_pos++;
        if(bytes_read % 511 == 0){
            int next_block = get_next_used_block(FAT_table, file_num, current_block);
            if(next_block == -1){
                return -1;
            }
            string = data->blocks[next_block].data;
            current_block = next_block;
            position = 0;
        }
    }
    return bytes_read;
}

int my_write(open_file_node* node, char buff[], int length){
    int file_num = node->fileNumber;
    char* string = data->blocks[file_num].data;
    int bytes_written = 0;
    int position = node->offset;
    int buff_pos = 0;
    int current_block = file_num;
    
    while(bytes_written < length){
        memcpy(&string[position], &buff[buff_pos], 1);
        bytes_written++;
        node->offset++;
        position++;
        buff_pos++;
        if(bytes_written % 511 == 0){
            int next_block = find_next_free_node(FAT_table);
            mark_node_taken(file_num, next_block, current_block, FAT_table);
            if(next_block == -1){
                return -1;
            }
            string = data->blocks[next_block].data;
            current_block = next_block;
            position = 0;
        }
    }
    
    
    return bytes_written;
    
    
}

int init(void){
    //make root directory
    directory* root = &dirs[0];
    strcpy(root->dirname, "root");
    root->num_entries = 0;
    current_number_of_dirs++;
    
    init_FAT(FAT_table);
    return 0;
}

int main(int argc, const char * argv[]) {
    head = malloc(sizeof(open_file_node));
    
    int fd = open("Drive2MB", O_RDWR|O_APPEND|O_CREAT, 0777);
    ftruncate(fd, sizeof(FAT) + sizeof(DATA) + NUM_DIRS*SIZE_DIRS);
    
    struct stat buf;
    fstat(fd, &buf);
    off_t size = buf.st_size;
    
    FAT_table = (FAT*) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    data = (DATA*) ((char*)FAT_table + sizeof(FAT));
    dirs = (directory*) ((char*)data + sizeof(DATA));
    
    
    //IF LAUNCHING FOR FIRST TIME
    init();
//
//
//
//
//    //TESTING
    my_create("root/hello.txt");
    my_create("root/test.c");
    my_delete("root/hello.txt");
   

    open_file_node* f = my_open("root/test.c", write_mode);
    char* buff1 = malloc(514);
    for (int i = 0; i < 514; i++) {
       strcat(buff1, "a");
    }
    my_write(f,buff1, 514);

    open_file_node* f2 = my_open("root/test.c", read_mode);
    char* buff2 = malloc(514);
    my_read(f2, buff2, 514);
    printf("%s\n", buff2);



    
    return 0;
    
 
}
