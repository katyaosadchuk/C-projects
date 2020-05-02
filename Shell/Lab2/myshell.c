//
//  myshell.c
//  Lab2
//
//  Created by Kateryna Osadchuk on 2/8/19.
//
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>
#include "utility.h"



int main(int argc, const char * argv[]) {
    //create environment variable for where executable file and helpdoc.txt are
    //need this to open the helpdoc correctly
    char main_dir[MAXPATHLEN];
    getcwd(main_dir, MAXPATHLEN);
    
    char main_dir_env[MAXPATHLEN];
    strcpy(main_dir_env, "FILE_LOCATION=");
    strcat(main_dir_env, main_dir);
    putenv(main_dir_env);
    
    //update shell environment variable
    char current_path[MAXPATHLEN];
    getcwd(current_path, sizeof(current_path));
    strcat(current_path, argv[0]);
  
    char new_shell_env[sizeof(current_path) + sizeof("SHELL=")];
    strcpy(new_shell_env, "SHELL=");
    strcat(new_shell_env, current_path);

    //update path environment variable
    putenv(new_shell_env);
    putenv("PATH=/bin:/usr/bin:/usr/sbin");
    
    // process batch file if one is passed in
    FILE *batch_file = fopen(argv[1], "r");
    if(batch_file != NULL){
        char buff[1024];
        //get a line from the file
        while(fgets(buff, 1023, batch_file) != NULL)
        {
            //get rid of newline character
            size_t entry_length = strlen(buff);
            if(buff[entry_length - 1] == '\n'){
                buff[entry_length - 1] = '\0';
            }
            
            //parse line and execute command
            commands *args = parse_string(buff);
            char **argv1 = args->first_command;
            char **argv2 = args->second_command;
            execute(argv1, argv2);
            //free memory used
            free(args);
        }
        exit(0);
    }
    //if no batch file is passed
    else {
        while (1) {
            // get and print path in shell name
            char path_name[MAXPATHLEN];
            getcwd(path_name, sizeof(path_name));
            char entry[1024];
            printf("%s/myshell> ", path_name);
            //get user input
            fgets(entry, 1023, stdin);
            
            
            // get rid of new line marker
            size_t entry_length = strlen(entry);
            if(entry[entry_length - 1] == '\n'){
                entry[entry_length - 1] = '\0';
            }
            
            // check if user wants to quit
            if((strcmp(entry, "quit") == 0) || feof(stdin) || (strcmp(entry, "exit") == 0)){
                exit(0);
            }
            
            // parse and execute command
            commands *arguments = parse_string(entry);
            char** argv1 = arguments->first_command;
            char** argv2 = arguments->second_command;
    
            execute(argv1, argv2);
            
            //free memory
            free(arguments);
            memset(entry, 0, sizeof(entry));
            
            
        }
    }
}
