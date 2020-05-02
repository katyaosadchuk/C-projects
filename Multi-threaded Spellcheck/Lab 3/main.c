// Kateryna Osadchuk
// Lab 3

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "queue.h"
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include "extract_dict.h"

// define constants to be used later
#define DEFAULT_DICTIONARY "words.txt"
#define DEFAULT_PORT "11111"
#define MAX_LINE 64
#define EXIT_USAGE_ERROR 1
#define EXIT_GETADDRINFO_ERROR 2
#define EXIT_BIND_FAILURE 3
#define EXIT_LISTEN_FAILURE 4
#define BACKLOG 10
//max number of elements in work queue is 20
#define MAX_IN_SOCKET_Q 20
//5 worker threads
#define NUM_WORKERS 5

// dictionary will be stored in the global linkedlist called "list"
word_linkedlist* list;

// struct that stores a pointer to the queue holding work and synchronization primitives
typedef struct {
    socket_queue* work;
    pthread_cond_t empty_slot;
    pthread_cond_t full_slots;
    pthread_mutex_t mutex;
} protect;
//create global semaphore struct
protect* sems;


//function prototypes
bool check(char*, word_node*);
void process(int);
void* worker_function(void*);
int getlistenfd(const char*);
ssize_t readLine(int fd, void *buffer, size_t n);

int main(int argc, const char * argv[]) {
    const char* dictionary_used;
    const char* port_used;
    int listenfd, connectedfd;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size;
    char client_name[MAX_LINE];
    char client_port[MAX_LINE];
    pthread_t threadPool[NUM_WORKERS];
    
    //initialize the struct and semaphores
    sems = (protect*) malloc(sizeof(protect));
    pthread_mutex_init(&sems->mutex, NULL);
    pthread_cond_init(&sems->empty_slot, NULL);
    pthread_cond_init(&sems->full_slots, NULL);
    
    // if dictionary and port aren't passed in, use default
    if (argc<2) {
        port_used = DEFAULT_PORT;
        dictionary_used = DEFAULT_DICTIONARY;
    } else if(argc == 2) {
        //if you pass in 1 argument with .txt, assume it is the dictionary file
        if (strstr(argv[1], ".txt") == NULL){
            port_used = argv[1];
            dictionary_used = DEFAULT_DICTIONARY;
        }
        //if you pass in 1 argument without .txt, assume it is port number
        if (strstr(argv[1], ".txt") != NULL) {
            port_used = DEFAULT_PORT;
            dictionary_used = argv[1];
        }
    } else if(argc == 3){
        // otherwise, select argument with .txt to be dictionary file and argument without .txt to be port number
        if ((strstr(argv[1], ".txt") == NULL) && (strstr(argv[2], ".txt") != NULL)) {
            port_used = argv[1];
            dictionary_used = argv[2];
        } else if ((strstr(argv[1], ".txt") != NULL) && (strstr(argv[2], ".txt") == NULL)) {
            dictionary_used = argv[1];
            port_used = argv[2];
        }
    } else {
        puts("Please only pass in two arguments along with the executable");
        exit(0);
    }
    // extract words from dictionary file into linkedlist of words
    list = extract(dictionary_used);
    //initialize queue that will hold work
    sems->work = (socket_queue*) malloc(sizeof(socket_queue));
    initialize_socket_queue(sems->work);
    
    //create worker threads
    for (int i = 0; i < NUM_WORKERS; i++) {
        if(pthread_create(&threadPool[i], NULL, worker_function, NULL) == 0){
            printf("Worker thread %d created\n", i);
        }
    }

    //get connection
    listenfd=getlistenfd(port_used);
    
    while(1) {
        //get connection
        client_addr_size = sizeof(client_addr);
        connectedfd=accept(listenfd, (struct sockaddr*)&client_addr, &client_addr_size);
        if (connectedfd == -1) {
            fprintf(stderr, "accept error\n");
            continue;
        }
        
        if (getnameinfo((struct sockaddr*)&client_addr, client_addr_size,
                        client_name, MAX_LINE, client_port, MAX_LINE, 0)!=0) {
            fprintf(stderr, "error getting name information about client\n");
        } else {
            printf("accepted connection from %s:%s\n", client_name, client_port);
        }
     
        //lock mutex
        pthread_mutex_lock(&sems->mutex);
        //if work queue is full, go to sleep and give up mutex
        while (sems->work->count == MAX_IN_SOCKET_Q) {
            pthread_cond_wait(&sems->empty_slot, &sems->mutex);
        }
        //if work queue is not full, add connected socket to work queue
        enqueue_socket(sems->work, connectedfd);
        //signal that there is a job in the queue
        pthread_cond_signal(&sems->full_slots);
        // give up mutex
        pthread_mutex_unlock(&sems->mutex);
       
    }
    
    return 0;
}

//function that worker threads will execute
void* worker_function(void* args){
    while(1){
        int socket_num;
        //lock mutex
        pthread_mutex_lock(&sems->mutex);
        //if there is no jobs in the queue, go to sleep and give up mutex
        while(sems->work->count == 0){
            pthread_cond_wait(&sems->full_slots, &sems->mutex);
        }
        //if there is a job, remove it from the queue
        socket_num = dequeue_socket(sems->work);
        //signal there is space in the queue
        pthread_cond_signal(&sems->empty_slot);
        //unlock mutex
        pthread_mutex_unlock(&sems->mutex);
        
        //process the job
        process(socket_num);
        
        //close socket
        if(close(socket_num) != -1){
            printf("Connection %d closed\n", socket_num);
        }
    }
}

//function to process a job on a socket
void process(int connectedfd){
    char line[MAX_LINE];
    memset(line, 0, sizeof(line));
    ssize_t bytes_read;
    // read a line until client terminates connection
    while ((bytes_read=readLine(connectedfd, line, MAX_LINE-1))>0) {
        //remove newline characters from client's entry
        char* ptr1;
        if( (ptr1 = strchr(line, '\n')) != NULL){
            *ptr1 = '\0';
        }
        char* ptr2;
        if( (ptr2 = strchr(line, '\r')) != NULL){
            *ptr2 = '\0';
        }
        
        //check if the word is in the dictionary (ie the linked list)
        bool val = check(line, list->front);
        if(val > 0){
            //if word is in dictionary, write OK
            strcat(line, " OK\n");
        } else {
            //if word is not in dict, write misspelled
            strcat(line, " MISSPELLED\n");
        }
        
        //write result to socket
        write(connectedfd, line, sizeof(line));
        //clear memory - prevents bugs
        memset(line, 0, sizeof(line));
    }
}

//function to check if a word is in the dictionary
bool check(char* word, word_node* list_of_words){
    word_node* current = list_of_words;
   //iterate through all words in the linkedlist of words
    while (current->next != NULL) {
        //select a word from the list
        char* entry = current->word;
        //if it matches the word passed in, then word is spelled correctly so return true
        if(strcmp(entry, word) == 0){
            return true;
        }
        //if not a match yet, keep going to next entry in dictionary
        current = current->next;
    }
    //if no matches are found in dictionary, then word is misspelled
    return false;
}



// all code below was given to us by Dr. Fiore

/* given a port number or service as string, returns a
 descriptor that we can pass to accept() */
int getlistenfd(const char *port) {
    int listenfd, status;
    struct addrinfo hints, *res, *p;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM; /* TCP */
    hints.ai_family = AF_INET;       /* IPv4 */
    
    if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo error %s\n", gai_strerror(status));
        exit(EXIT_GETADDRINFO_ERROR);
    }
    
    /* try to bind to the first available address/port in the list.
     if we fail, try the next one. */
    for(p = res;p != NULL; p = p->ai_next) {
        if ((listenfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol))<0) {
            continue;
        }
        
        if (bind(listenfd, p->ai_addr, p->ai_addrlen)==0) {
            break;
        }
    }
    freeaddrinfo(res);
    if (p==NULL) {
        exit(EXIT_BIND_FAILURE);
    }
    
    if (listen(listenfd, BACKLOG)<0) {
        close(listenfd);
        exit(EXIT_LISTEN_FAILURE);
    }
    return listenfd;
}

ssize_t readLine(int fd, void *buffer, size_t n) {
    ssize_t numRead;                    /* # of bytes fetched by last read() */
    size_t totRead;                     /* Total bytes read so far */
    char *buf;
    char ch;
    
    if (n <= 0 || buffer == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    buf = buffer;                       /* No pointer arithmetic on "void *" */
    
    totRead = 0;
    while(1) {
        numRead = read(fd, &ch, 1);
        
        if (numRead == -1) {
            if (errno == EINTR)         /* Interrupted --> restart read() */
                continue;
            else
                return -1;              /* Some other error */
            
        } else if (numRead == 0) {      /* EOF */
            if (totRead == 0)           /* No bytes read; return 0 */
                return 0;
            else                        /* Some bytes read; add '\0' */
                break;
            
        } else {                        /* 'numRead' must be 1 if we get here */
            if (totRead < n - 1) {      /* Discard > (n - 1) bytes */
                totRead++;
                *buf++ = ch;
            }
            
            if (ch == '\n')
                break;
        }
    }
    
    *buf = '\0';
    return totRead;
}

