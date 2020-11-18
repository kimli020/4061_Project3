#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include "util.h"
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>

#define MAX_THREADS 100
#define MAX_QUEUE_LEN 100
#define MAX_CE 100
#define INVALID -1
#define BUFF_SIZE 1024


/*
  THE CODE STRUCTURE GIVEN BELOW IS JUST A SUGGESTION. FEEL FREE TO MODIFY AS NEEDED
*/

// structs:
typedef struct request_queue {
   int fd;
   char *request;
} request_t;

typedef struct cache_entry {
    int len;
    char *request;
    char *content;
} cache_entry_t;

FILE *logfile = NULL;
request_t queue[MAX_QUEUE_LEN + 1];
int count = 0;
int queue_front = 0;
int queue_back = 0;

//thread
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_added = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_free = PTHREAD_COND_INITIALIZER;




/* ******************** Dynamic Pool Code  [Extra Credit A] **********************/
// Extra Credit: This function implements the policy to change the worker thread pool dynamically
// depending on the number of requests
void * dynamic_pool_size_update(void *arg) {
  while(1) {
    // Run at regular intervals
    // Increase / decrease dynamically based on your policy
  }
}
/**********************************************************************************/

/* ************************ Cache Code [Extra Credit B] **************************/

// Function to check whether the given request is present in cache
int getCacheIndex(char *request){
  /// return the index if the request is present in the cache
}

// Function to add the request and its file content into the cache
void addIntoCache(char *mybuf, char *memory , int memory_size){
  // It should add the request at an index according to the cache replacement policy
  // Make sure to allocate/free memory when adding or replacing cache entries
}

// clear the memory allocated to the cache
void deleteCache(){
  // De-allocate/free the cache memory
}

// Function to initialize the cache
void initCache(){
  // Allocating memory and initializing the cache array
}

/**********************************************************************************/

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)
  
  
  //Strings in C are hard...code was moved to dispatch() function. see comments there
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
    // Open and read the contents of file given the request
}

/**********************************************************************************/

/**********************************************************************************/
/*Helper function to add to queue. Used in dispatch().*/
int enqueue(request_t result) {
  pthread_mutex_lock(&queue_mtx);           //lock and wait 
  while ((queue_back + 1) % (MAX_QUEUE_LEN+1) == queue_front) {
    pthread_cond_wait(&queue_free, &queue_mtx);
  }
  queue[queue_back] = result;
  queue_back++;
  queue_back = queue_back % (MAX_QUEUE_LEN+1);
  pthread_cond_signal(&queue_added);
  pthread_mutex_unlock(&queue_mtx);         //unlock;
  return 0;
}
/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
  char temp[BUFF_SIZE];
  while (1) {
  
    // Accept client connection
    int fd = accept_connection();
    if(fd < 0){
        continue;
    }
    
    // Get request from the client
    int request = get_request(fd, temp);
    if (request != 0) {
      return_error(fd, "Invalid request.");
      continue;
    }  
    
    // Add the request into the queue
    request_t req;
    req.fd = fd;
    sprintf(req.request, ".%s", temp);
    enqueue(req);       //call helper function              
  }
  return NULL;
}


/**********************************************************************************/
/*Helper function for removing from queue. Used in worker().*/
int dequeue(request_t *result) {
  pthread_mutex_lock(&queue_mtx);           //lock and wait
  while (queue_front == queue_back) {
    pthread_cond_wait(&queue_added, &queue_mtx);
  }
  *result = queue[queue_front];
  queue_front++;
  queue_front = queue_front % (MAX_QUEUE_LEN+1);
  pthread_cond_signal(&queue_free);
  pthread_mutex_unlock(&queue_mtx);         //unlock
  return 0;
}
/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  request_t req;
  while (1) {
  
    // Get the request from the queue
    dequeue(&req);

    // Get the data from the disk or the cache (extra credit B)
    FILE *file = fopen(req.request, "r");
    if (file == NULL) {
      return_error(req.fd, "File not found.");
      continue;
    }
    
    char *buffer = malloc(BUFF_SIZE);
    strcpy(buffer,req.request);  
  
  
 /*THE CODE BETWEEN THE ARROWS SHOULD BE IN getContentType, BUT I DON'T KNOW HOW TO PASS THINGS CORRECTLY*/
//-->     
    char* temp = strrchr(req.request, '.'); 
    if (temp == NULL) {
      return_error(req.fd, "Can't determine file type/n");
      fclose(file);
      free(buffer);
      continue;
    }


    /*THIS WAY OF GETTING THE CONTENT TYPE CAUSES A SEGMENTATION FAULT...why?*/
/*
    char content_type = "";
    if(strcmp(temp, ".html") == 0 ){
      strcpy(content_type, "text/html");
    }else if ( strcmp(temp, ".jpg") == 0 ){
      strcpy(content_type, "image/jpeg");      
    }else if ( strcmp(temp, ".gif") == 0 ){
      strcpy(content_type, "image/gif");   
    }else if( strcmp(temp, ".txt") == 0 ){
      strcpy(content_type, "text/plain");  
    }else{
      printf("error getting file type \n");
      fclose(file);
      free(buffer);
      continue;
    }
    return_result(req.fd, content_type, buffer, BUFF_SIZE);      //defined in utils.c  
*/  
    /*THIS VERSIONS WORKS*/
    if(strcmp(temp, ".html") == 0 ){
      return_result(req.fd, "text/html", buffer, BUFF_SIZE);
    }else if ( strcmp(temp, ".jpg") == 0 ){
      return_result(req.fd, "image/jpeg", buffer, BUFF_SIZE);    
    }else if ( strcmp(temp, ".gif") == 0 ){
      return_result(req.fd, "image/gif", buffer, BUFF_SIZE); 
    }else if( strcmp(temp, ".txt") == 0 ){
      return_result(req.fd, "text/plain", buffer, BUFF_SIZE); 
    }else{
      printf("error getting file type \n");
      fclose(file);
      free(buffer);
      continue;
    }
//-->       
    
    fclose(file);
    free(buffer);
  }
  return NULL;
}


/**********************************************************************************/
void handler(int signal){
  printf("Number of pending requests : %d \n", count);
  close(logfile);
  exit(0);
}

/**********************************************************************************/
int main(int argc, char **argv) {

    // Error check on number of arguments
    if(argc != 8){
        printf("usage: %s port path num_dispatcher num_workers dynamic_flag queue_length cache_size\n", argv[0]);
        return -1;
    }

    // Get the input args
    int port =  strtol(argv[1], NULL, 10);
    char *path = argv[2];
    int num_dispatchers = strtol(argv[3], NULL, 10);
    int num_workers =  strtol(argv[4], NULL, 10);
    int dynamic_flag = strtol(argv[5], NULL, 10);
    int qlen = strtol(argv[6], NULL, 10);
    int cache_entries = strtol(argv[7], NULL, 10);

    /*Perform error checks on the input arguments*/
    //Check for valid Port #'s
    if (port < 1025 || port > 65535){
        printf("ERROR: Port # must be between 1025 and 65535 \n");
        return -1;
    }
    
    //Check # of dispatchers
    if (num_dispatchers < 1 || num_dispatchers > MAX_THREADS) {
        printf("Number of dispatchers must be between 1 and %i\n", MAX_THREADS);
        return -1;
    }
    
    //Check # of workers
    if (num_workers < 1 || num_workers > MAX_THREADS) {
        printf("Number of workers must be between 1 and %i\n", MAX_THREADS);
        return -1;
    }
    
    //check dynamic flag
    if (dynamic_flag != 0 && dynamic_flag != 1) {
        printf("ERROR: Dynamic flag should be 0 or 1. (static or dynamic worker thread pool)\n");
        return -1;
    }
    
    //Check qlen
    if (qlen < 1 || qlen > MAX_QUEUE_LEN) {
        printf("Request queue length must be between 1 and %i\n", MAX_QUEUE_LEN);
        return -1;
    }
    
    //Checks cache_entries
    if (cache_entries < 0 || cache_entries > MAX_CE) {
        printf("Number of cache entries must be between 1 and %i\n", MAX_CE);
         return -1;
    }
  

    // Change SIGINT action for grace termination
    signal(SIGINT, handler);

    // Open log file
    logfile = fopen("./web_server_log", "w");
    if (logfile == NULL){
      printf("ERROR: Unable to open web_server_log\n");
      return -1;
    }

    // Change the current working directory to server root directory
    chdir(path);
    
    // Initialize cache (extra credit B)

    // Start the server
    init(port);

    // Create dispatcher and worker threads (all threads should be detachable)
    pthread_t dispatchers[num_dispatchers];
    pthread_t workers[num_workers];
    pthread_attr_t attr_detach;
    pthread_attr_init(&attr_detach);
    pthread_attr_setdetachstate(&attr_detach, PTHREAD_CREATE_DETACHED);
    
    int i;
    for(i=0; i < num_dispatchers; i++){
        pthread_create(&dispatchers[i], &attr_detach, dispatch, NULL);
    }
    
    for(i=0; i < num_workers; i++){
        pthread_create(&workers[i], &attr_detach, worker, NULL);
    }

    // Create dynamic pool manager thread (extra credit A)
    
    //to keep main from closing prematurely 
    while(1) {
        sleep(1);
    }
    
/*Mostly sure the following requirements should all should be done in handler()*/
    
    // Terminate server gracefully
    // Print the number of pending requests in the request queue
    // close log file
    // Remove cache (extra credit B)

    return 0;
}
