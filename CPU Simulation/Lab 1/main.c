// Kateryna Osadchuk - Assignment 1


#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "fifo.h"
#include "priority_queue.h"

// declare global variables to hold information read from config.txt. Set to default values
int SEED = 98765;
int INIT_TIME = 0;
int FIN_TIME = 30000;
int ARRIVE_MIN = 10;
int ARRIVE_MAX = 20;
int CPU_MIN = 5;
int CPU_MAX = 25;
int DISK1_MIN = 20;
int DISK1_MAX = 120;
int DISK2_MIN = 10;
int DISK2_MAX = 100;
int QUIT_PROB = 0.2;

// enumerate the types of jobs
enum typesOfJobs{job_arrives, job_finishes_CPU, job_finishes_D1, job_finishes_D2};

// keeps track of the current length of queues
unsigned int number_Events_in_Priority = 0;
unsigned int number_jobs_in_CPU_Queue = 0;
unsigned int number_jobs_in_D1_Queue = 0;
unsigned int number_jobs_in_D2_Queue= 0;

// tracks current time
unsigned int currentTime = 0;

// keeps track of whether there is a job on CPU/D1/D2 or not
bool CPU_Busy = false;
bool D1_Busy = false;
bool D2_Busy = false;

// counters to help with gathering statistics
unsigned int number_total_jobs_CPU = 0;
unsigned int number_total_jobs_D1 = 0;
unsigned int number_total_jobs_D2 = 0;
unsigned int max_CPU_queue = 0;
unsigned int max_D1_queue = 0;
unsigned int max_D2_queue = 0;
unsigned int time_CPU_busy = 0;
unsigned int time_D1_busy = 0;
unsigned int time_D2_busy = 0;
unsigned int sum_CPU_queue_sizes = 0;
unsigned int sum_D1_queue_sizes = 0;
unsigned int sum_D2_queue_sizes = 0;
unsigned int num_CPU_queue_changes = 0;
unsigned int num_D1_queue_changes = 0;
unsigned int num_D2_queue_changes = 0;
unsigned int max_response_time_CPU = 0;
unsigned int max_response_time_D1 = 0;
unsigned int max_response_time_D2 = 0;
unsigned int sum_response_times_CPU = 0;
unsigned int num_CPU_response_changes = 0;
unsigned int sum_response_times_D1 = 0;
unsigned int num_D1_response_changes = 0;
unsigned int sum_response_times_D2 = 0;
unsigned int num_D2_response_changes = 0;

// make File pointer to log.txt global so that can be written to when job exits
FILE *log_file;


// function prototypes
void processJobArrives(int, node_for_priority**, node_for_queue**);
void job_finishes_at_CPU(int, node_for_priority**, node_for_queue**, node_for_queue**, node_for_queue**);
void job_finishes_at_D1(int,  node_for_priority**, node_for_queue**, node_for_queue**);
void job_finishes_at_D2(int,  node_for_priority**, node_for_queue**, node_for_queue**);
void get_config_file_info(void);
void get_statistics(void);


int main(int argc, const char * argv[]) {
    // open log.txt to write to
    log_file = fopen("log.txt", "w");
    if(log_file == NULL){
        puts("Error opening file for writing.");
        return 1;
    }
    
    // read config.txt file and update global vars defined on lines 9-20
    get_config_file_info();
    // seed random number generator and update current time
    srand(SEED);
    currentTime = INIT_TIME;
   
    // create FIFO and priority queues
    node_for_priority *priority_queue = (node_for_priority*) malloc(sizeof(node_for_priority));
    node_for_queue *CPU_queue = (node_for_queue*) malloc(sizeof(node_for_queue));
    node_for_queue *D1_queue = (node_for_queue*) malloc(sizeof(node_for_queue));
    node_for_queue *D2_queue = (node_for_queue*) malloc(sizeof(node_for_queue));
    
    // add first event to event (priority) queue - job 1 arrives
    int random = (rand() % (ARRIVE_MAX - ARRIVE_MIN + 1)) + ARRIVE_MIN;
    addToPriority(INIT_TIME + random, 1, job_arrives, &priority_queue);
    number_Events_in_Priority++;
    
    // until fin_time or event queue is empty, pop an event and handle it
    while (currentTime < FIN_TIME && number_Events_in_Priority > 0) {
        // remove event from priority queue, store job number and update current time
        node_for_priority *event = removeFromPriority(&priority_queue);
        int jobNum = event->jobNumber;
        currentTime = event->time;
        
        // make sure current time is still before FIN_TIME - otherwise it's time to end simulation
        if(currentTime < FIN_TIME){
            // handle event based on the event type and update log.txt
            if(event->type == job_arrives){
                processJobArrives(jobNum, &priority_queue, &CPU_queue);
                printf("Job %d arrived at time %d\n", jobNum, currentTime);
                fprintf(log_file, "Job %d arrived at time %d\n", jobNum, currentTime);
                
                
            } else if(event->type == job_finishes_CPU){
                number_total_jobs_CPU++;
                job_finishes_at_CPU(jobNum, &priority_queue, &CPU_queue, &D1_queue, &D2_queue);
                printf("Job %d finished on CPU at time %d\n", jobNum, currentTime);
                fprintf(log_file, "Job %d finished on CPU at time %d\n", jobNum, currentTime);
                
                
            } else if(event->type == job_finishes_D1){
                number_total_jobs_D1++;
                job_finishes_at_D1(jobNum, &priority_queue, &CPU_queue, &D1_queue);
                printf("Job %d finished on D1 at time %d\n", jobNum, currentTime);
                fprintf(log_file, "Job %d finished on D1 at time %d\n", jobNum, currentTime);
                
                
            } else if(event->type == job_finishes_D2){
                number_total_jobs_D2++;
                job_finishes_at_D2(jobNum, &priority_queue, &CPU_queue, &D2_queue);
                printf("Job %d finished on D2 at time %d\n", jobNum, currentTime);
                fprintf(log_file, "Job %d finished on D2 at time %d\n", jobNum, currentTime);
            }
        }
    }
    // add 'simulation finishes' to log.txt, close file, get statistics of run
    fprintf(log_file, "Simulation finishes at time %d\n", FIN_TIME);
    fclose(log_file);
    get_statistics();
    
}

// function to compute and print all statistics to a stats.txt file
void get_statistics(void){
    FILE *statistics = fopen("stats.txt", "w");
    float throughput_CPU = (float) number_total_jobs_CPU/(float) (FIN_TIME-INIT_TIME);
    float throughput_D1 = (float) number_total_jobs_D1/(float) (FIN_TIME-INIT_TIME);
    float throughput_D2 = (float) number_total_jobs_D2/(float) (FIN_TIME-INIT_TIME);
    fprintf(statistics, "Throughput of CPU: %f\n", throughput_CPU);
    fprintf(statistics, "Throughput of D1: %f\n", throughput_D1);
    fprintf(statistics, "Throughput of D2: %f\n\n", throughput_D2);
    
    fprintf(statistics, "Maximum number of jobs in CPU queue: %d\n", max_CPU_queue);
    fprintf(statistics, "Maximum number of jobs in D1 queue: %d\n", max_D1_queue);
    fprintf(statistics, "Maximum number of jobs in D2 queue: %d\n\n", max_D2_queue);
    
    float avg_CPU_queue = (float) sum_CPU_queue_sizes/(float)(num_CPU_queue_changes+1);
    float avg_D1_queue = (float) sum_D1_queue_sizes/(float)(num_D1_queue_changes+1);
    float avg_D2_queue = (float) sum_D2_queue_sizes/(float)(num_D2_queue_changes+1);
    fprintf(statistics, "Average number of jobs in CPU queue: %f\n", avg_CPU_queue);
    fprintf(statistics, "Average number of jobs in D1 queue: %f\n", avg_D1_queue);
    fprintf(statistics, "Average number of jobs in D2 queue: %f\n\n", avg_D2_queue);
 
    
    float utilization_CPU = (float) time_CPU_busy/(float)(FIN_TIME - INIT_TIME);
    float utilization_D1 = (float) time_D1_busy/(float)(FIN_TIME - INIT_TIME);
    float utilization_D2 = (float) time_D2_busy/(float)(FIN_TIME - INIT_TIME);
    fprintf(statistics, "Utilization of CPU: %f\n", utilization_CPU);
    fprintf(statistics, "Utilization of D1: %f\n", utilization_D1);
    fprintf(statistics, "Utilization of D2: %f\n\n", utilization_D2);
    
    fprintf(statistics, "Max response time of CPU: %d\n", max_response_time_CPU);
    fprintf(statistics, "Max response time of D1: %d\n", max_response_time_D1);
    fprintf(statistics, "Max response time of D2: %d\n\n", max_response_time_D2);
    
    float avg_resp_CPU = (float) sum_response_times_CPU / (float) num_CPU_response_changes;
    float avg_resp_D1 = (float) sum_response_times_D1 / (float) num_D1_response_changes;
    float avg_resp_D2 = (float) sum_response_times_D2 / (float) num_D2_response_changes;
    fprintf(statistics, "Avg response time CPU: %f\n", avg_resp_CPU);
    fprintf(statistics, "Avg response time D1: %f\n", avg_resp_D1);
    fprintf(statistics, "Avg response time D2: %f\n\n", avg_resp_D2);
    
    fclose(statistics);
    
}

// function to read config.txt data
void get_config_file_info(void){
    FILE *config = fopen("config.txt", "r");
    // if the config.txt file is empty or nonexistant, use default values as initialized
    if(config == NULL){
        puts("config.txt not found. Using default values.");
    } else {
        char line[256];
        int numbers[11];
        int counter = 0;
        //read file line by line and extract first number (integer) in line up until QUIT_PROB (which I put in the last line)
        while ((fgets(line, sizeof(line), config) != NULL) && counter < 11) {
            line[strlen(line) - 1] = '\0';
            sscanf(line, "%*[^0123456789]%d", &numbers[counter]);
            counter++;
            
        }
        // read in line holding QUIT_PROB info and extract QUIT_PROB as a float
        double probability;
        fgets(line, sizeof(line), config);
        line[strlen(line) - 1] = '\0';
        sscanf(line, "%*[^0123456789.0123456789]%lf", &probability);
        
        // update global config variables using extracted values (note that values are in order)
        SEED = numbers[0];
        INIT_TIME = numbers[1];
        FIN_TIME = numbers[2];
        ARRIVE_MIN = numbers[3];
        ARRIVE_MAX = numbers[4];
        CPU_MIN = numbers[5];
        CPU_MAX = numbers[6];
        DISK1_MIN = numbers[7];
        DISK1_MAX = numbers[8];
        DISK2_MIN = numbers[9];
        DISK2_MAX = numbers[10];
        QUIT_PROB = probability;
    
    }
}


// function to process a job arrival
void processJobArrives(int jobNum, node_for_priority** priority_queue, node_for_queue** CPU_queue){
    
    // generate a new job arrival event and update counter(so priority queue will never be empty)
    int random = (rand() % (ARRIVE_MAX - ARRIVE_MIN + 1)) + ARRIVE_MIN;
    addToPriority(currentTime + random, jobNum+1, job_arrives, priority_queue);
    number_Events_in_Priority++;
    
    // if CPU is busy, add job to CPU queue
    if(CPU_Busy == true){
        addToQueue(jobNum, currentTime, CPU_queue);
        
        //update statistic counters
        number_jobs_in_CPU_Queue++;
        sum_CPU_queue_sizes += number_jobs_in_CPU_Queue;
        num_CPU_queue_changes++;
        if(number_jobs_in_CPU_Queue > max_CPU_queue){
            max_CPU_queue = number_jobs_in_CPU_Queue;
        }
        
        // if CPU is free, generate a job finish on CPU event (i.e. send job to CPU)
    } else {
        int random2 = (rand() % (CPU_MAX - CPU_MIN + 1)) + CPU_MIN;
        addToPriority(currentTime + random2, jobNum, job_finishes_CPU, priority_queue);
        number_Events_in_Priority++;
        CPU_Busy = true;
        
        // update stat counters
        time_CPU_busy += random2;
        int job_response_CPU = random2;
        if(job_response_CPU > max_response_time_CPU){
            max_response_time_CPU = job_response_CPU;
        }
        sum_response_times_CPU += job_response_CPU;
        num_CPU_response_changes++;
    }
}


// function to process a job finishing at CPU
void job_finishes_at_CPU(int jobNum, node_for_priority** priority_queue, node_for_queue** CPU_queue, node_for_queue**  D1_queue, node_for_queue**  D2_queue){
    
    // determine if job exits or is sent to disk based on QUIT_PROB
    bool job_will_exit = rand() <  QUIT_PROB * ((double)RAND_MAX + 1.0);
    if(job_will_exit == true){
        fprintf(log_file, "Job %d exits at time %d\n", jobNum, currentTime);
    }
    
    // if job does not exit and D1 is free, send to D1 (generate finish on D1 event)
    else {
        if(D1_Busy == false){
            int random = (rand() % (DISK1_MAX - DISK1_MIN + 1)) + DISK1_MIN;
            addToPriority(currentTime + random, jobNum, job_finishes_D1, priority_queue);
            number_Events_in_Priority++;
            D1_Busy = true;
            
            // update statistics counters
            time_D1_busy += random;
            int job_response_time_D1 = random;
            if(job_response_time_D1 > max_response_time_D1){
                max_response_time_D1 = job_response_time_D1;
            }
            sum_response_times_D1+=job_response_time_D1;
            num_D1_response_changes++;
           
        }
        
        // if job does not exit and D2 is free, send to D1 (generate finish on D2 event)
        else if(D2_Busy == false){
            int random = (rand() % (DISK2_MAX - DISK2_MIN + 1)) + DISK2_MIN;
            addToPriority(currentTime + random, jobNum, job_finishes_D2, priority_queue);
            number_Events_in_Priority++;
            D2_Busy = true;
            
            //update counters
            time_D2_busy += random;
            int job_response_time_D2 = random;
            if(job_response_time_D2 > max_response_time_D2){
                max_response_time_D2 = job_response_time_D2;
            }
            sum_response_times_D2+=job_response_time_D2;
            num_D2_response_changes++;
            
        }
        // if both disks are busy, add job to shortest queue. If queues are same length, add to D2 queue by default
        else {
            if(number_jobs_in_D1_Queue < number_jobs_in_D2_Queue){
                addToQueue(jobNum, currentTime, D1_queue);
                
                // update stat counters
                number_jobs_in_D1_Queue++;
                sum_D1_queue_sizes += number_jobs_in_D1_Queue;
                num_D1_queue_changes++;
                if(number_jobs_in_D1_Queue > max_D1_queue){
                    max_D1_queue = number_jobs_in_D1_Queue;
                }
            } else {
                addToQueue(jobNum, currentTime, D2_queue);
                
                // update counters
                number_jobs_in_D2_Queue++;
                sum_D2_queue_sizes += number_jobs_in_D2_Queue;
                num_D2_queue_changes++;
                if(number_jobs_in_D2_Queue > max_D2_queue){
                    max_D2_queue = number_jobs_in_D2_Queue;
                }
            }
        }
    }
    
    // if there are items in CPU queue, remove the first one in line and process it (generate job finish on CPU event)
    if(number_jobs_in_CPU_Queue > 0){
        node_for_queue *next_job_on_CPU = removeFromQueue(CPU_queue);
        int arrival_time = next_job_on_CPU->arrival_time;
        int random2 = (rand() % (CPU_MAX - CPU_MIN + 1)) + CPU_MIN;
        addToPriority(currentTime + random2, next_job_on_CPU->jobNumber, job_finishes_CPU, priority_queue);
        number_Events_in_Priority++;
        CPU_Busy = true;
        
        //update stat counters
        time_CPU_busy += random2;
        number_jobs_in_CPU_Queue--;
        int job_response_time_CPU =  currentTime - arrival_time + random2;
        if(job_response_time_CPU > max_response_time_CPU){
            max_response_time_CPU = job_response_time_CPU;
        }
        sum_response_times_CPU += job_response_time_CPU;
        num_CPU_response_changes++;
    }
    // if CPU queue is empty, set CPU_Busy to false
    else {
        CPU_Busy = false;
    }
    
    
}

// function to handle job finishing on D1
void job_finishes_at_D1(int jobNum, node_for_priority** priority_queue, node_for_queue** CPU_queue, node_for_queue** D1_queue){
    
    // if CPU is not busy, send job to CPU
    if(CPU_Busy == false){
        int random = (rand() % (CPU_MAX - CPU_MIN + 1)) + CPU_MIN;
        addToPriority(currentTime + random, jobNum, job_finishes_CPU, priority_queue);
        number_Events_in_Priority++;
        CPU_Busy = true;
        
        //update stat counters
        time_CPU_busy += random;
        int job_response_time_CPU = random;
        if(job_response_time_CPU > max_response_time_CPU){
            max_response_time_CPU = job_response_time_CPU;
        }
        sum_response_times_CPU += job_response_time_CPU;
        num_CPU_response_changes++;
        
    }
    // if CPU is busy, add job to CPU queue
    else {
        addToQueue(jobNum, currentTime, CPU_queue);
        
        //update counters
        number_jobs_in_CPU_Queue++;
        sum_CPU_queue_sizes += number_jobs_in_CPU_Queue;
        num_CPU_queue_changes++;
        if(number_jobs_in_CPU_Queue > max_CPU_queue){
            max_CPU_queue = number_jobs_in_CPU_Queue;
        }
    }
    
    // if D1 has elements in its queue, pop first off and process it on D1 (generate job finish on D1 event)
    if(number_jobs_in_D1_Queue > 0){
        node_for_queue *next_job_on_disk = removeFromQueue(D1_queue);
        int arrival_time = next_job_on_disk->arrival_time;
        int random2 = (rand() % (DISK1_MAX - DISK1_MIN + 1)) + DISK1_MIN;
        addToPriority(currentTime + random2, next_job_on_disk->jobNumber, job_finishes_D1, priority_queue);
        number_Events_in_Priority++;
        D1_Busy = true;
        
        //update statistics counters
        time_D1_busy += random2;
        number_jobs_in_D1_Queue--;
        int job_response_time_D1 =  currentTime - arrival_time + random2;
        if(job_response_time_D1 > max_response_time_D1){
            max_response_time_D1 = job_response_time_D1;
        }
        sum_response_times_D1 += job_response_time_D1;
        num_D1_response_changes++;
    }
    // if D1 queue is empty, set D1_busy to false
    else {
        D1_Busy = false;
    }
}

// function to handle job finishing on D2
void job_finishes_at_D2(int jobNum, node_for_priority** priority_queue, node_for_queue** CPU_queue, node_for_queue** D2_queue){
    
    // if CPU is not busy, send job to CPU
    if(CPU_Busy == false){
        int random = (rand() % (CPU_MAX - CPU_MIN + 1)) + CPU_MIN;
        addToPriority(currentTime + random, jobNum, job_finishes_CPU, priority_queue);
        number_Events_in_Priority++;
        CPU_Busy = true;
        
        //update counters
        time_CPU_busy += random;
        int job_response_time_CPU = random;
        if(job_response_time_CPU > max_response_time_CPU){
            max_response_time_CPU = job_response_time_CPU;
        }
        sum_response_times_CPU += job_response_time_CPU;
        num_CPU_response_changes++;
    }
    // if CPU is busy, add job to CPU queue
    else {
        addToQueue(jobNum, currentTime, CPU_queue);
        
        //update counters
        number_jobs_in_CPU_Queue++;
        sum_CPU_queue_sizes += number_jobs_in_CPU_Queue;
        num_CPU_queue_changes++;
        if(number_jobs_in_CPU_Queue > max_CPU_queue){
            max_CPU_queue = number_jobs_in_CPU_Queue;
        }
    }
    
     // if D2 has elements in its queue, pop first off and process it on D1 (generate job finish on D2 event)
    if(number_jobs_in_D2_Queue > 0){
        node_for_queue *next_job_on_disk = removeFromQueue(D2_queue);
        int arrival_time = next_job_on_disk->arrival_time;
        int random2 = (rand() % (DISK2_MAX - DISK2_MIN + 1)) + DISK2_MIN;
        addToPriority(currentTime + random2, next_job_on_disk->jobNumber, job_finishes_D2, priority_queue);
        number_Events_in_Priority++;
        D2_Busy = true;
        
        //update counters
        time_D2_busy += random2;
        number_jobs_in_D2_Queue--;
        int job_response_time_D2 = currentTime - arrival_time + random2;
        if(job_response_time_D2 > max_response_time_D2){
            max_response_time_D2 = job_response_time_D2;
        }
        sum_response_times_D2+=job_response_time_D2;
        num_D2_response_changes++;
    }
    // if D2 queue is empty, set D2_busy to false
    else {
        D2_Busy = false;
    }
}




