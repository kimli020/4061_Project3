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


int logFd;
int pending_requests = 0;
request_t queue[MAX_QUEUE_LEN];
int queue_start = 0;
int queue_end = 0;
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER; // initiliaze to be able to lock and unlock




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
  
  const char* temp = strrchr(mybuf, '.'); 
  temp += 1;

  char* content_type = (char*) malloc(BUFF_SIZE*sizeof(char));
  
  if(!strcmp(temp, "html") || !strcmp(temp, "htm")){
    strcpy(content_type, "text/html");
  } else if (!strcmp(temp, "jpg")){
    strcpy(content_type, "image/jpeg");
  } else if (!strcmp(temp, "gif")){
    strcpy(content_type, "image/gif");
  } else {
    strcpy(content_type, "text/plain");
  }
  return content_type;
  
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
    // Open and read the contents of file given the request
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
  while (1) {
    // Accept client connection
    int fd = accept_connection();
    
    // Get request from the client
    if(fd > -1) {
      char filename[BUFF_SIZE];
      
      // Add the request into the queue
      if(get_request(fd, filename) == 0) {
          request_t req;
          req.fd = fd;;
          strcpy(req.request, filename);
          queue[queue_start] = req;
          queue_start ++;
      }else{
        continue;
      }
    }else{
       continue;
    }

  }
   return NULL;
}

/**********************************************************************************/

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {

  int num_requests = 0;
  while (1) {
    sleep(1);
   
    //if we've handled all requests
    if(queue_end == queue_start) {
      continue;  
    }

    // Get the request from the queue
    request_t req = queue[queue_end];
    queue_end++; 
    num_requests++;
    char* content_type = getContentType(req.request);
    return_error(req.fd, content_type);    /*int return_result(int fd, char *buf)*/

    // Get the data from the disk or the cache (extra credit B)

    // Log the request into the file and terminal

    // return the result
//    char* content_type = getContentType(req.request);
//    return_result(req.fd, content_type, contents, numbytes);      //defined in utils.c
//    free(content_type);
  }
  return NULL;
}


/**********************************************************************************/
void handler(int signal){
  printf("Number of pending requests : %d \n", pending_requests);
  close(logFd);
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
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    if(sigemptyset(&act.sa_mask) == -1 || sigaction(SIGINT, &act, NULL) == -1){
        printf("Failed to set SIGINT handler.\n");
        return -1;
    } 

    // Open log file
    logFd = open("web_server_log", O_CREAT | O_WRONLY, 0777);
    if (logFd < 0){
      printf("ERROR: Unable to create web_server_log file \n");
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

    // Terminate server gracefully
    handler(SIGINT);
    
    // Print the number of pending requests in the request queue
    // close log file
    //fclose(log_file);
    // Remove cache (extra credit B)

    return 0;
}
