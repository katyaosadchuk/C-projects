// Kateryna Osadchuk
// Lab 2

#ifndef utility_h
#define utility_h

#include <stdio.h>

//stores commands called by user
extern char **environ;
typedef struct commands{
    char **first_command;
    char **second_command;
}commands;


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

#endif
