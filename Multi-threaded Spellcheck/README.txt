Lab 3 - Spellcheck
Kateryna Osadchuk


Part 1: Program Overview
In this lab, I use multiple threads and sockets in a spellcheck application. To ensure security, I use a pthread_mutex lock for the queue holding work requests and two condition variables to indicate when there is a slot available or when there is a job to be processed. This ensures that only one thread accesses the work queue at a time, no jobs are added to a full queue (limit 20 jobs by default), and no thread tries to remove a job from an empty queue. I store the lock, condition variables, and the work queue in a struct called “protect”. When the program first launches, I initialize the struct, mutex, and condition variables.


I then proceed by updating the dictionary file and port number to use. That is, if the user doesn’t pass in any arguments (i.e. they only type ./spellcheck), the program uses the default port 11111 and the default dictionary file words.txt. Note that for the default dictionary file to be opened, it must be in the same folder as the executable program. If the user passes in a single argument, the program checks if the argument contains a .txt extension or not. If it does, it assumes the argument to be the dictionary file. Otherwise, it assumes it to be the port number. If the user passes in two arguments, it similarly takes the argument containing .txt to be the dictionary file and the other argument to be the port number. Should the user try passing in more than two arguments, the program will print an error and quit.


Once the program extracts which dictionary file and port number to use, it then extracts the words in the dictionary file into a linked list. The program assumes the entries in the dictionary file are separated by a new line character. This linked list will be shared among the threads of the program to check if a word is spelled correctly or not. The function to extract the words into the linked list is found in the extract_dict.c file while the linked list data structure and functions are found in queue.c. Note that there is no function to remove an entry from the linked list; this is to ensure the linked list isn’t corrupted by removing valid entries. 


Once the linked list of words is created, the program then initializes a worker queue (the queue structure and functions can be found in queue.c) , stored in the “protect” struct, and creates NUM_WORKERS (set to 5) threads. Next, the main thread runs a while loop to accept connections from the specified port. When a new connection is accepted, it is added to the work queue to be processed by a worker thread. Note that to ensure security, I first lock the mutex that protects the work queue, then check if there is space in the queue to add a job using the pthread_cond variable. Once this is done, I add the job, signal that there is a job in the queue, and then unlock the mutex. The main thread runs this loop of accepting connections and adding them to the work queue indefinitely. 


Meanwhile, each worker thread processes the jobs in the queue by executing “worker_function”, found in main.c. In this function, the thread runs in an infinite loop and begins by locking the mutex lock, checking if there is a job in the queue, removing a socket number from the work queue, signaling there is an empty slot, and then unlocking the lock. Once the job (socket number) is safely removed, it is processed by calling the “process” function, also found in main.c. Once the job is processed, the worker thread closes the socket connection. 


The “process” function runs in a loop that terminates when the user enters the EOF key. The loop reads in user input, removes \r and \n characters (new line characters), then checks if the user input is spelled correctly by calling the “check” function. If the check function returns true, then the user entry is  concatenated with “OK” and written to the socket. Otherwise if the check function returns false, the user entry is concatenated with “MISSPELLED” and written to the socket. 


The “check” function checks if a specified word is in the dictionary (the linked list of words created earlier). We do this by iterating through each element in the linked list and returning true if a match is found, meaning the word is spelled correctly. If we iterate through the whole linked list and do not return true, then we assume the word isn’t in the dictionary and return false, meaning the word is misspelled. 


Since the main thread runs in an infinite loop, terminate it by entering ^C (control + C). 


Part 2: Testing
I tested this program by creating 6 connections and typing in random words. All of the output was as expected. I noticed that the first 5 connections were immediately serviced, but the 6th was only serviced after some other connection was closed. This makes sense since I have 5 worker threads and each thread services one connection at a time. Once a thread was free, it serviced the 6th connection. However, while waiting for a worker, the 6th connection still allowed the user to enter input and produced the result (i.e. printing if the word was OK or MISSPELLED) as soon as a free thread serviced it.