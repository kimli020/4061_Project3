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
int count = 0;

//thread
pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t queue_mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_added = PTHREAD_COND_INITIALIZER;
pthread_cond_t queue_free = PTHREAD_COND_INITIALIZER;

//  array to keep track of number of requests a worker thread has processed
int threadTrackNum[100];


/*  Circular queue implementation structures **********************************************/
//  Circular queue to implement bounded buffer
typedef struct circ_queue {
  int rear, front; // enQueue rear, deQueue front
  // enQueue and deQueue with capital Q's belong to this type of queue
  int size; // current number of elements present in the queue
  int capacity; // how many elements the circular queue could hold at maximum
  request_t *circArray;
} circ_queue_t;

circ_queue_t ringBufferInit (int cap) {  // capacity will be qlen
  circ_queue_t retQueue;
  retQueue.rear = retQueue.front = -1;
  retQueue.size = 0;
  retQueue.capacity = cap;
  retQueue.circArray = (request_t*) malloc((retQueue.capacity)*sizeof(request_t));

  return retQueue;
}

int enQueue(circ_queue_t *q, request_t req) {
  pthread_mutex_lock(&queue_mtx);           //lock and wait

  // Wait while queue is full
  while ((q->front == 0 && q->rear == q->capacity-1) ||(q->rear == (q->front-1)%(q->capacity-1))) { //capacity - 1 = max index
    pthread_cond_wait(&queue_free, &queue_mtx);
  }
  //  enQueue first element
  if (q->front == -1) {
    q->front = 0;
    q->rear = 0;
    q->circArray[q->rear] = req;
    q->size++;
  }
  else if (q->rear == q->capacity -1 && q->front != 0) {
    q->rear = 0;
    q->circArray[q->rear] = req;
    q->size++;
  }
  else {
    q->rear++;
    q->circArray[q->rear] = req;
    q->size++;
  }
  pthread_cond_signal(&queue_added);
  pthread_mutex_unlock(&queue_mtx);         //unlock;
  return 0;
}

int deQueue(circ_queue_t *q, request_t *result) {
  pthread_mutex_lock(&queue_mtx);           //lock and wait

  // Wait while queue is empty
  while (q->front == -1 || q->size == 0) {
    pthread_cond_wait(&queue_added, &queue_mtx);
  }

  *result = q->circArray[q->front];
  q->circArray[q->front].fd = -1; //signifies request has been processed
  q->size--;

  if(q->front == q->rear){
    q->front = -1;
    q->rear = -1;
  }
  else if (q->front == q->capacity -1)
    q->front = 0;
  else
    q->front++;
  pthread_cond_signal(&queue_free);
  pthread_mutex_unlock(&queue_mtx);         //unlock
  return 0;
}

/*****************************************************************************************/
//  struct to contain arguments for threads (thread id and current ring buffer)
typedef struct threadArg {
  int tid;  //thread id
  circ_queue_t *tQueue;
} threadArg_t;


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

/* ************************************ Utilities ********************************/
// Function to get the content type from the request
char* getContentType(char * mybuf) {
  // Should return the content type based on the file type in the request
  // (See Section 5 in Project description for more details)


  //Strings in C are hard...code was moved to worker() function. see comments there
    return 0;
}

// Function to open and read the file from the disk into the memory
// Add necessary arguments as needed
int readFromDisk(/*necessary arguments*/) {
    // Open and read the contents of file given the request
    return 0;
}

/**********************************************************************************/

// Function to receive the request from the client and add to the queue
void * dispatch(void *arg) {
  char temp[BUFF_SIZE];
  threadArg_t inArgs = *((threadArg_t *) arg);
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
    req.request = (char*) malloc(9);
    fflush(stdout);
    sprintf(req.request, ".%s", temp);
    enQueue(inArgs.tQueue, req);
  }
  return NULL;
}

// Function to retrieve the request from the queue, process it and then return a result to the client
void * worker(void *arg) {
  // int id = *((int *) arg);
  threadArg_t inArgs = *((threadArg_t *) arg);
  int id = inArgs.tid;
  request_t req;

  while (1) {

    // Get the request from the queue
    // dequeue(&req);
    deQueue(inArgs.tQueue, &req);

    char *buffer = malloc(BUFF_SIZE);
    strcpy(buffer,req.request);
    threadTrackNum[id]++;

    // Size of target file:
    struct stat st;
    stat(buffer, &st);
    int size = st.st_size;

    // Get the data from the disk or the cache (extra credit B)
    FILE *file = fopen(req.request, "r");
    if (file == NULL) {
      return_error(req.fd, "File not found.");

      printf("[%d][%d][%d][%s][%s]\n", id,threadTrackNum[id],req.fd,buffer,"File Not Found.");
      fprintf(logfile, "[%d][%d][%d][%s][%s]\n", id,threadTrackNum[id],req.fd,buffer,"File Not Found.");
      fflush(logfile);
      fflush(stdout);

      continue;
    }

 /*THE CODE BETWEEN THE ARROWS SHOULD BE IN getContentType, BUT I DON'T KNOW HOW TO PASS THINGS CORRECTLY*/
//-->
    char* temp = strrchr(req.request, '.');
    if (temp == NULL) {
      return_error(req.fd, "Can't determine file type/n");
      fclose(file);
      free(buffer);
      continue;
    }

    /*THIS VERSIONS WORKS*/
    if(strcmp(temp, ".html") == 0 ){
      return_result(req.fd, "text/html", buffer, size);
    } else if ( strcmp(temp, ".jpg") == 0 ){
      return_result(req.fd, "image/jpeg", buffer, size);
    } else if ( strcmp(temp, ".gif") == 0 ){
      return_result(req.fd, "image/gif", buffer, size);
    } else if( strcmp(temp, ".txt") == 0 ){
      return_result(req.fd, "text/plain", buffer, size);
    } else {
      printf("error getting file type \n");
      fclose(file);
      free(buffer);
      continue;

    }

    // Output: [threadId][reqNum][fd][Request string][bytes/error][Cache HIT/MISS] , Cache not implemented

    printf("[%d][%d][%d][%s][%d]\n", id,threadTrackNum[id],req.fd,buffer,size);
    fprintf(logfile, "[%d][%d][%d][%s][%d]\n", id,threadTrackNum[id],req.fd,buffer,size);
    fflush(logfile);
    fflush(stdout);
//-->

    fclose(file);
    free(buffer);
  }
  return NULL;
}


/**********************************************************************************/
void handler(int signal){
  printf("\nServer stopping...\n");
  printf("\nNumber of pending requests : %d \n", count);
  fclose(logfile);
  exit(0);
}

/**********************************************************************************/
int main(int argc, char **argv) {

 printf("Server Starting...\n");
 fflush(stdout);
    // Error check on number of arguments
    if(argc != 8){
        printf("usage: %s <port> <path> <num_dispatcher> <num_workers> <dynamic_flag> <queue_length> <cache_size>\n", argv[0]);
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
    logfile = fopen("./web_server_log", "w+");
    if (logfile == NULL){
      printf("ERROR: Unable to open web_server_log\n");
      return -1;
    }
    // Change the current working directory to server root directory
    chdir(path);

    // init the threadTrackNum array
    for(int i=0; i<MAX_QUEUE_LEN; i++)  {
      threadTrackNum[i] = 0;
    }

    // Initialize cache (extra credit B)

    // Start the server
    init(port);

    //  Init the ring buffer queue
    circ_queue_t ringQueue = ringBufferInit(qlen);
    circ_queue_t *ringQueuePtr = &ringQueue;
    //
    threadArg_t dispArg[qlen];
    threadArg_t workArg[qlen];

    // Create dispatcher and worker threads (all threads should be detachable)
    pthread_t dispatchers[num_dispatchers];
    pthread_t workers[num_workers];
    pthread_attr_t attr_detach;
    pthread_attr_init(&attr_detach);
    pthread_attr_setdetachstate(&attr_detach, PTHREAD_CREATE_DETACHED);

    int i;
    for(i=0; i < num_dispatchers; i++){
        dispArg[i].tid = i;
        dispArg[i].tQueue = ringQueuePtr;
        pthread_create(&dispatchers[i], &attr_detach, dispatch, &dispArg[i]);
    }

    for(i=0; i < num_workers; i++){
        // int *arg = malloc(sizeof(*arg)); // Pass ID as arg
        // *arg = i;
        workArg[i].tid = i;
        workArg[i].tQueue = ringQueuePtr;
        pthread_create(&workers[i], &attr_detach, worker, &workArg[i]);
        // free(arg);
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
