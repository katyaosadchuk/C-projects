Kateryna Osadchuk
Lab 2 - Basic Shell

Part A: Function and File Descriptions
In my submission, you will find 5 files: makefile, helpdoc.txt, myshell.c, utility.c, and utility.h. The makefile contains instructions for the compiler which allows you to easily build the executable file. The helpdoc.txt serves as the source for the “help” command. That is, when the user types “help” while running myshell, the help function reads from helpdoc.txt to the standard output (screen unless there is a redirect or pipe). The helpdoc contains some basic information about some commands as well as the syntax for redirects and pipes. Finally, all of the code for functionality of the shell is in myshell.c and utility.c.

First, I will describe the contents of myshell.c. This file has components for three tasks: setting the initial environment variables such as shell and path, processing batch files, and the main loop that runs the shell. In lines 21-40, I use putenv to set the path and shell variables. I also created my own environment variable called “file location”. This saves the pathname of the helpdoc.txt file (which should be in same directory as the myshell executable) in order to open it for reading when the help function is called. I did this because I found that if I used cd to change directories, I needed to open helpdoc.txt using its full pathname and the simplest way to find it was by setting and accessing environment variables. 

Next, in lines 43 to 63, I handle batch file processing. I do this by attempting to open argv[1] (which is where the filename would be if one is passed in) and if the file pointer isn’t NULL, then this indicates a file name was, in fact, passed in. Then, I loop through each line in the file, format the line by getting rid of newline character (this prevents certain bugs), then call a function to parse the line, call another function to execute the command, and free memory. After I reach the end of the file, I exit myshell. 

On the other hand, if argv[1] returns a NULL file pointer, then no batch file name was passed in. Thus here, I run a loop that ends only when the user types “quit”, “exit”, or the EOF key. Inside the loop, I display a prompt for the user, get input from the user, and then handle the input almost identically to how lines were handled in batch file processing. That is, I replace the \n character with \0 to prevent bugs, check if the user typed the command to quit (and if so, call exit()), parse the line, execute the command, and free the memory. Note that when I display the prompt for the user to enter a command, the prompt contains the current path, which I get using the getcwd function. 

Thus, the myshell.c file contains the basic backbone of the shell. Meanwhile, all of the functionality is in the utility.c file. That is, I mentioned I call a function to parse the line and execute the command, and these functions are found in utility.c. In fact, here are the following functions found in utility.c and a brief summary of each:
	1.	Void check_for_pipe (char* s1): this helper function parses the string s1 and checks to see if there is a “|” character present. If there is, it sets the global variable “is_pipe” to true.  
	2.	Void check_for_background(char* s1): this helper function parses the string s1 and checks to see if there is a “&” character, which would indicate a background task. If this character is found, it updates the “is_background” global boolean to true. 
	3.	Void check_redirects (char** tokens): this helper function iterates over all words in tokens (which is essentially an array of strings) and checks if any of them are >, < , or >>. If it finds a redirect character, if updates a global variables to store the name of the new stdin or stdout file. Also, it updates global booleans “redirect_std_in”, “redirect_std_out”, and/or “no_truncate_redirect_stdout” as necessary.  
	4.	char **make_array_of_words(char *string): This function takes in a string and uses the strtok function to create an array of strings (in heap), which it then returns. This is used exclusively to convert the string entered by the user into an array or words, which will later be used to call built in functions, or exec a new process.  
	5.	commands* parse_string(char *s1): This function takes in a string (user’s entry), parses it in one of two ways, and returns a struct called “commands”. This may be the most complicated function so let me begin by explaining the struct “commands”. “Commands” has two members, first_command and second_command, which are both of type char** (i.e. an array of words). If the user uses a pipe to connect two commands (such as: cat file.txt | wc -l), the first_command member will store the array of words made from the string “cat file.txt” and the second_command member will store the array made from the string “wc -l”. On the other hand, if the user passes in a line with no pipes, then first_command contains the array made from the whole line and second_command is set to NULL. This allows me to handle multiple commands at the same time. As for the function itself, it begins by checking for pipes and backgrounds using the helper functions mentioned previously. If there are no pipes in the user’s entry, it creates an array of words using make_array_of_words from the string s1 that is passed in, checks for redirects (using the helper function), sets the struct’s first_command member to store the array, and sets the second_command member to hold NULL. If, however, a pipe is found, the function first splits the string using the pipe character as a delimiter. It stores both halves of the original string in an array. For instance, if s1 is “cmd1 arg1 | cmd2”, the array of two strings will store “cmd1 arg1” at index 0 and “cmd2” at index 1. Then, we pass both entries of the array into the function make_array_of_words. At this point, we have two arrays of words, one for each part of the pipe. Thus, we set first_command to store the first command array (shocking!) and second_command to store the second command array. We return this struct from the function.  
	6.	void restore_input_output(void): this is a helper function that restores stdin/stdout if there were any redirects. 
	7.	int num_builtins(void): this is a helper function that counts the number of builtin functions 
	8.	void cd(char **argv): builtin function to change directory 
	9.	void path(char **argv): builtin function to update path environment variable 
	10.	void clr(char **argv): builtin function to clear screen 
	11.	void echo1(char **argv): builtin function to print what user typed to screen  
	12.	void dir(char **argv): builtin function to show contents of directory 
	13.	void env(char **argv): built in function to print all environment variables 
	14.	void help(char **argv): builtin function that reads from helpdoc.txt to display basic information about the shell. It uses a makeshift more filter to display the output, meaning it displays 20 lines at a time, then waits for the user to press enter before displaying the next twenty lines. When it reaches the end of the file, it returns to the loop and takes in the next input.  
	15.	void execute_pipe(char **argv1, char **argv2): This function executes the commands when the user pipes two commands, which are saved as arrays of words argv1 and argv2. I execute the commands by first creating a pipe using the pipe() function, then I fork a child process. In this child, I close the reading end of the pipe and redirect stdout to the write end of the pipe using dup2 function. Then, I call the command in argv1 using by exec-ing a new process. Back in the parent process, I fork another child process. In this child process, I close the writing end of the pipe and redirect the stdin to be the read end of the pipe. Then, call the second command by exec-ing a new process. Finally, I close both ends of the pipe and wait for the child processes to terminate.  
	16.	int execute(char **argv1, char **argv2): This function is responsible for executing all commands and is called after each time user entry is parsed. First, I check if the user entered an empty line and return if that’s the case. Then, if there is a pipe, I call execute_pipe() to handle executing the commands. After the pipe is executed, I restore the input/output and return. If there is not a pipe, I continue in the function by removing the redirect arguments and the &, if they are there. That is, the command “env > test.txt” becomes “env” and the command “pwd &” becomes simply “pwd”. This prevents buggy behavior but still does redirects and background tasks as needed. Then, I use the global redirect booleans to check if there are any redirects and update the stdin/stdout files as necessary. Next, I check if the user is calling a builtin commands by comparing the first word of the user’s entry to each entry in the array of builtin command names. If a match is found, I call the corresponding builtin function from an array of function pointers and then return to the main loop. This approach is concise and clean, which is why I use it rather than employing multiple if/else if statements. Finally, if a builtin function isn’t called, I fork/exec a process to execute the command. I also use the global boolean is_background to determine if the parent process waits for the child or not. Before returning to the main loop, I call restore_input_output to clean up any redirects that happened.  
Through these 16 functions, I achieve the basic functionality of myshell. 

Part B: Testing myshell
To test myshell, I ran the following commands, all of which executed correctly:
	1.	Pwd 
	2.	Dir 
	3.	Env > test.txt 
	4.	Ls 
	5.	Cat test.txt | wc -l 
	6.	Echo1 hello there 
	7.	Ls & 
	8.	Help 
	9.	Cd .. 
	10.	Cd .. 
	11.	Ls 
	12.	Cd Lab2 
	13.	Clr 
	14.	Path /bin 
	15.	Mkdir hello 
	16.	Cd hello 
	17.	Touch hi 
	18.	Rm hi 
	19.	Cd .. 
	20.	Rmdir hello 
	21.	Quit 

To test batchfiles, I also passed in a file called batchfile.txt by typing “./myshell batchfile.txt” . Batchfile.txt contained the following:
pwd
cd ..
cd ..
dir
mkdir new
cd new
env > test.txt
ls
rm test.txt
cd ..
ls
rmdir new
dir
All of these commands were executed correctly and then the shell exited. 

Also, I tested background tasks by calling “sleep(5)” in the child process before exec-ing the process. That way, when I typed “pwd”, the shell has a delay before it prints the working directory and displays the prompt. On the other hand, when I typed “pwd &”, the prompt was immediately displayed and prompted user input. After a few seconds, the pwd command would display the working directory. 

Part C: Performance and Next Steps
Based on my understanding of the project, everything seemed to work fairly correctly with almost no errors. If I had more time, I would construct functionality to allow for multiple pipes. Also, my current pipes don’t work for builtin functions, so I would fix the code to allow for builtin functions to be piped. Also, I noticed that if there is a directory with a space in the name (for example, something like “Lab 2”), cd-ing into that directory won’t work because of how I parse and execute the commands. That is, calling “cd Lab\ 2” won’t work because my code parses on whitespace and will interpret that command as “cd Lab\”, which is probably not a directory that exists. If I had more time, I would fix this bug. For now, however, it suffices to change my directory names to not have any spaces. 
