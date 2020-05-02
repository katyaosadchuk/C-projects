//
//  utility.c
//  Lab2
//
//  Created by Kateryna Osadchuk on 2/8/19.
//  
//

#include "utility.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>

// function prototypes
commands* parse_string(char*);
void check_for_pipe(char *);
void check_for_background(char *);
int num_builtins(void);
int execute(char**, char**);
void clr(char**);
void cd(char**);
void path(char**);
void dir(char**);
void env(char**);
void echo1(char**);
void help(char**);
void check_redirects(char**);
void restore_input_output(void);
char **make_array_of_words(char *);
void execute_pipe(char**, char**);

//global variables
char PWD_buff[MAXPATHLEN];
bool is_background = false;
bool redirect_std_in = false;
bool redirect_std_out = false;
bool no_truncate_redirect_stdout = false;
bool is_pipe = false;
//stores name of new input/output if there is a redirect
char* new_std_in;
char* new_std_out;
//file descriptors of new input/output
int new_in_file = 0;
int new_out_file = 0;
// saves file descriptors of original input/output
int old_in_file = 0;
int old_out_file =0;



//list of builtin functions
char *builtin_str[] = {
    "cd",
    "path",
    "clr",
    "dir",
    "env",
    "echo1",
    "help",
};

//array of pointers to built in functions - makes calling a builtin function cleaner
void (*builtin_func[]) (char **) = {
    &cd,
    &path,
    &clr,
    &dir,
    &env,
    &echo1,
    &help,
};

//helped function that checks if user is piping commands
void check_for_pipe(char *string){
    is_pipe = false;
    int i = 0;
    while(string[i] != '\0'){
        if(string[i] == '|'){
            is_pipe = true;
        }
        i++;
    }
}

//helped function that checks for '&' which indicates a background process
void check_for_background(char *string){
    is_background = false;
    int i = 0;
    while(string[i] != '\0'){
        if(string[i] == '&'){
            is_background = true;
        }
        i++;
    }
}

//helper function that checks if user indicated any redirects
void check_redirects(char** tokens){
    //initialize global vars to false
    redirect_std_in = false;
    redirect_std_out = false;
    no_truncate_redirect_stdout = false;
    
    int i = 0;
    //iterate over all words in array
    if(tokens[0] != NULL){
        while (tokens[i] != NULL) {
            //check if stdin redirect
            if(strcmp(tokens[i], "<") == 0){
                redirect_std_in = true;
                //update name of new input (which would be argument immediately following <)
                new_std_in = tokens[i+1];
            }
            //check if stdout overwrite redirect
            if(strcmp(tokens[i], ">") == 0){
                redirect_std_out = true;
                //update name of new output
                new_std_out = tokens[i+1];
            }
            //check if stdout append redirect
            if(strcmp(tokens[i], ">>") == 0){
                no_truncate_redirect_stdout = true;
                //update name of new output
                new_std_out = tokens[i+1];
            }
            i++;
        }
    }
}


//splits line entered by user into an array of words
char **make_array_of_words(char *string){
    char *token;
    int bufsize = 64, position = 0;
    char **tokens = (char **) malloc(bufsize * sizeof(char*));
    //split user entry into words using space and tab as delimiters
    token = strtok(string, " \t");
    while (token != NULL)
    {
        //store word in array
        tokens[position] = token;
        position++;
        //reallocate memory if ran out
        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*));
        }
        //get next word
        token = strtok (NULL, "     ");
    }
    // terminate array with NULL
    tokens[position] = NULL;
    // return pointer to array
    return tokens;
}

//parses user entry in order to execute commands
commands* parse_string(char *string){
    //check for pipes and background tasks
    check_for_pipe(string);
    check_for_background(string);
    commands *retval = malloc(2*sizeof(char**));
    
    //if user is not piping a command
    if(!is_pipe){
        //split line into array of words
        char **array = make_array_of_words(string);
        //check for redirects
        check_redirects(array);
        //set struct entries so that array of words will be returned
        retval->first_command = array;
        //since no piping, second command is NULL
        retval->second_command = NULL;
    }
    //if user is piping commands, there are two independent commands that must be returned
    else {
        //make array to hold each command (as strings for now)
        char *commands[2];
        char *token;
        //get first command and save in array
        token = strtok(string, "|");
        commands[0] = token;
        //get second command and save in array
        token = strtok(NULL, "|");
        commands[1] = token;
        //split each command into array of words
        char **first_array= make_array_of_words(commands[0]);
        char **second_array = make_array_of_words(commands[1]);
        //save each array in struct entries
        retval->first_command = first_array;
        retval->second_command = second_array;
        
    }

    return retval;
}

//helper function to restore original stdin/stdout if there were any redirects
void restore_input_output(void){
    if(redirect_std_in) {
        dup2(old_in_file, STDIN_FILENO);
        close(old_in_file);
    } if(redirect_std_out){
        dup2(old_out_file, STDOUT_FILENO);
        close(old_out_file);
    } if(no_truncate_redirect_stdout) {
        dup2(old_out_file, STDOUT_FILENO);
        close(old_out_file);
    }
}

// helper function to find number of builtins
int num_builtins(void) {
    return sizeof(builtin_str) / sizeof(char *);
}

//change directory
void cd(char **argv){
    char cdbuffer[MAXPATHLEN];
    
    // if don't indicate what to change to, print current directory
    if(argv[1] == NULL){
        getcwd(cdbuffer,sizeof(cdbuffer));
        fprintf(stdout, "current working directory: \n%s\n",cdbuffer);
        return;
    } else {
        //change to indicated directory
        int ret = chdir(argv[1]);
        if(ret){
            fprintf(stderr, "error: %s\n", strerror(errno));
        } else {
            //update PWD environment variable
            strcpy(PWD_buff, "PWD=");
            getcwd(cdbuffer, sizeof(cdbuffer));
            strcat(PWD_buff, cdbuffer);
            if (putenv(PWD_buff)) {
                fprintf(stdout, "putenv() error\n");
            }
        }
    }
}

//update path environment variable
void path(char **argv){
    int i = 1;
    //add user indicated paths to a string to be used to update environment
    char *path = malloc(MAXPATHLEN * sizeof(char));
    strcpy(path, "PATH=");
    while (argv[i] != NULL) {
        strcat(path, argv[i]);
        strcat(path, ":");
        i++;
    }
    
    //get rid of trailing :
    size_t len = strlen(path);
    if(strcmp(&path[len - 1], ":") == 0){
        path[len-1] = '\0';
    }
  
    //update environment
    putenv(path);

}

//clear screen
void clr(char **argv){
    for(int i =0; i < 30; i++){
        printf("\n");
    }
}

//echo function - had to change name due to an error
void echo1(char **argv){
    int i = 1;
    // print out everything the user passed in
    while(argv[i] != NULL){
        fprintf(stdout, "%s ", argv[i]);
        i++;
    }
    fprintf(stdout, "\n");
    //restore original stdin/stdout if redirects were used
    restore_input_output();
}

//list contents of directory
void dir(char **argv){
    DIR *directory;
    struct dirent *values;
    
    //if not specified a directory
    if(argv[1] == NULL){
        char cdbuffer[MAXPATHLEN];
        getcwd(cdbuffer,sizeof(cdbuffer));
        //open current directory
        if ((directory = opendir(cdbuffer)) == NULL){
            fprintf(stderr, "opendir %s %s\n", cdbuffer, strerror(errno));
            return;
        }
    }
    //otherwise, open indicated directory
    else {
        if ((directory = opendir(argv[1])) == NULL){
            fprintf(stderr, "opendir %s %s\n", argv[1], strerror(errno));
            return;
        }
    }
    
    //print names of contents
    while ((values = readdir(directory))!=NULL) {
        fprintf(stdout, "%s\t", values->d_name);
    }
    
    //close directory
    fprintf(stdout, "\n");
    closedir(directory);
    //restore original stdin/stdout if redirects were used
    restore_input_output();
    
}

//print all environment variables
void env(char **argv){
    for (char **string = environ; *string; string++) {
        fprintf(stdout, "%s\n", *string);
    }
    //restore original stdin/stdout if there were any redirects
    restore_input_output();
}

//print info about shell
void help(char **argv){
    //open helpdoc file using its full path name
    char *file_location = getenv("FILE_LOCATION");
    char file_name[MAXPATHLEN];
    strcpy(file_name, file_location);
    strcat(file_name, "/helpdoc.txt");
    
    FILE *helpdoc = fopen(file_name, "r");
    if(helpdoc == NULL){
      fprintf(stdout, "error opening help doc\n");
      return;
    }
    
    // if we redirected output to a file, simply print all of helpdoc into the new output
    if(redirect_std_out || no_truncate_redirect_stdout){
        char line[128];
        while ( fgets(line, sizeof(line), helpdoc) != NULL )
        {
            fputs(line, stdout);
        }
    }
    //if output is the screen, use makeshift more filter
    else {
        char line[128];
        int linecount = 1;
         //print 20 lines (a screen) and then wait for user to press enter to read next 20 lines (simulating a more filter)
        while ( fgets(line, sizeof(line), helpdoc) != NULL )
        {
            if(linecount%21 == 0){
                while (getchar() != '\n');
            }
            fputs(line, stdout);
            linecount++;
        }
    }
    
    //restore original stdin/stdout if there were any redirects
    restore_input_output();
    //close file
    fclose (helpdoc);
}

//function to execute commands using a pipe
void execute_pipe(char **argv1, char **argv2){
    pid_t p1, p2;
    //create a pipe, 0 for reading, 1 for writing
    int p[2];
    pipe(p);
    
    // pipe1
    p1 = fork();
    if(p1 < 0){
        perror("Error in fork child");
        return;
    }
    else if (p1 == 0) {
        //in child, close read end and make write end the stdout
        close(p[0]);
        dup2(p[1], STDOUT_FILENO);
        
        //execute first command
        if(execvp(argv1[0], argv1) < 0) {
            perror("Error child1");
            return;
        }
    }
    
    //pipe 2
    p2 = fork();
    if(p2 < 0){
        perror("Error in fork child ");
        return;
    }
    else if (p2 == 0) {
        //in child, close write end and make read end the stdin
        close(p[1]);
        dup2(p[0], STDIN_FILENO);
        
        //execute second command
        if(execvp(argv2[0], argv2) < 0) {
            perror("Error child2");
            return;
        }
    }
    //close pipe
    close(p[0]);
    close(p[1]);
    
    //wait for children to finish before returning
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    
}

//main function to execute commands
int execute(char **argv1, char **argv2){
    //if pass in empty line - return
    char *command = argv1[0];
    if(command == NULL){
        return 1;
    }
    
    //if there is a pipe, use helped function to execute commands
    if(is_pipe){
        execute_pipe(argv1, argv2);
        restore_input_output();
        return 0;
    }
   
    //if last character is &, remove it to prevent bugs
    if(is_background){
        int i = 0;
        while (strcmp(argv1[i], "&") != 0) {
            i++;
        }
        argv1[i] = NULL;
    }
    //remove redirect arguments to prevent buggy behavior
    if(redirect_std_in || redirect_std_out || no_truncate_redirect_stdout){
        int i = 0;
        while ((strcmp(argv1[i], "<") != 0) && (strcmp(argv1[i], ">") != 0) && (strcmp(argv1[i], ">>") != 0)) {
            i++;
        }
        argv1[i] = NULL;
    }

    
    //change stdin/stdout if there is a redirect
    if(redirect_std_in){
        new_in_file = open(new_std_in, O_RDONLY, S_IRWXU );
        old_in_file = dup(STDIN_FILENO);
        dup2(new_in_file, STDIN_FILENO);
        close(new_in_file);
    } if(redirect_std_out) {
        new_out_file = open(new_std_out, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
        old_out_file = dup(STDOUT_FILENO);
        dup2(new_out_file, STDOUT_FILENO);
        close(new_out_file);
    } if(no_truncate_redirect_stdout) {
        new_out_file = open(new_std_out, O_WRONLY | O_APPEND | O_CREAT, S_IRWXU);
        old_out_file= dup(STDOUT_FILENO);
        dup2(new_out_file, STDOUT_FILENO);
        close(new_out_file);
    }
    
    
    //check if command is builtin and call corresponding function if there is a match
    for(int i=0; i < num_builtins(); i++){
        if(strcmp(command, builtin_str[i]) == 0){
            (*builtin_func[i])(argv1);
            return 0;
        }
    }
    
    //if command isn't built in, fork and exec a process
    pid_t pid = fork();
    int status;

    if(pid == 0){
        //exec in child
        if (execvp(argv1[0], argv1) < 0) {
            fprintf(stdout, "%s: Command not found.\n", argv1[0]);
            return 0;
        }
    }
    //if not a background process, have parent wait for child
    else if(!is_background){
        waitpid(pid, &status, 0);
    }
    
    //return stdin/stdout to original settings
    restore_input_output();
    
    return 0;
}
