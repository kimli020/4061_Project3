Project 3: Multi-threaded Web Server

CSCI4061 Fall 2020 Project 3
Group 41 Group members: Minh Bui (bui00011), John Kimlinger (kimli020), Andrew Trudeau (trude135)
/* Test machine: Machine names: csel-kh4250-36.cselabs.umn.edu
* Name: Minh Bui, John Kimlinger, Andrew Trudeau
* X500: bui00011, kimli020, trude135 */

• The purpose of your program: The goal of the project is to construct a multi-threaded web server using POSIX threads in C and their synchronization methods (locks and conditional variables). The web server should be able to queue and process files of the types HTML, GIF, JPEG, and TXT, and of any arbitrary type (so long as available memory allows). For this process, the web server needs to handle a portion of the HTTP web protocol (the GET command to fetch web pages/files); several utility functions were already provided as an util.o file for handling the aspects of networking, and the focus of the project is to implement the multi-threaded operation.

• How to compile the program and run the web server:
    - Naviagate to the project directory(e.g. <path>/P3/p3/). This should have the following files: util.h, util.o, server.c, and Makefile.  
    - Run the make command: $make. This will produce the web_server executable and the web_server_log file. Running $make clean will remove these two files.
    - In the provided P3.zip, there is a zip file called testing.zip. Pick a directory in which to unzip this file (should by default unzip into a testing/ directory). The path to this testing/ directory will be important for running the server. In this submission, it was unzipped as [...]/P3/p3/testing/testing/[paths to testing sub directories (image/, text/) and urls, bigurls and how_to_test file].
    - To run the web_server executable, in the same directory, use a terminal command of the form:
    ./web_server <port> <path_to_testing>/testing <num_dispatch> <num_worker> <dynamic_flag> <queue_len> <cache_entries>
        - <port>: A random port number between 1024 to 65536 (other than 9000 since that collides with the example cases). This port number should match the port number in the urls. For this project, we chose 16703 at random. To use the urls and bigurls file in testing/, it is important to go into these files and change the port number in the urls to the port number chosen for the server
        - <path_to_testing>/testing: path to the unzipped directory for testing.zip content.
        - <num_dispatch>: Number of dispatcher threads to create, must not exceed 100.
        - <num_worker>: Number of worker threads to create, must not exceed 100.
        - <dynamic_flag>: set to 1 to activate dynamic management of the worker thread pool (add/delete worker threads as the server load, i.e. the number of queued requests, changes); set to 0 to keep a static number of workers in the pool as specified by <num_worker>.
        Note: The dynamic pool option was attempted for this project, but was not implemented correctly. The dynamic management thread is called using a conditional statement in main, and if dynamic_flag == 1 this will trigger a buggy implementation of the dynamic pool and disrupt server operations. Setting dynamic_flag == 0 will prevent this from ever being called and maintain stable server operations with a static thread pool.
        - <queue_len>: the capacity of the bounded buffer used, must not exceed 100.
        - <cache_entries>: number of entries in cache memory, not implemented, always set to 0.
      - After this, the web_server will run continuously until ^C is pressed on the terminal, at which points it prints out the number of pending requests in the queue (how many requests have not been processed) and terminates the server operation. We have found that the program is usually so fast that this number will always be 0 (printf statements confirmed that enQueue and deQueue operations do change the counter for pending requests), so if there are unprocessed requests at the time of the server's termination, those requests are not yet queued and there is no way for the server to determine that number.

• How the program works:
      The program will create multiple threads of the two types: dispatchers and workers. It will also implement a bounded buffer (in this case a ring buffer) to store a limited number of pending requests. A request is stored in a struct element with two fields: an int fd obtained from get_connection() and char* request to store the path to file/web page being fetched, and the ring buffer stores a limited number of such elements. The dispatcher threads will call accept_connection() and get_request() to populate a request structure and enqueue that to one end of the ring buffer. The worker threads will dequeue elements starting from the other end of the ring buffer, extract the data (e.g. threadID, filename/path to file or webpage, type of file, etc.), call return_error(), when the request cannot be processed normally, or return_result() as needed, and log the processed data on both the current server terminal and the web_server_log file. The ring buffer will be a FIFO, first in first out, buffer. Locks and conditional variables were implemented for the enQueue and deQueue operations used by the threads: enQueue will block if the buffer is full, deQueue will block if the buffer is empty, and each one will signal the other when their operations become valid (e.g. enQueue signals deQueue that is has placed an element in the queue, and if deQueue was previously blocking it could now go and pull that element from the now-not-empty buffer). A signal handler was implemented as discussed.

• How to test the server:
      We could use commands of the following formats to test the server:
      - Handle a single urls: wget http://127.0.0.1:16703<path to file/page being fetched>, such as wget http://127.0.0.1:16703/image/jpg/29.jpg.
        Note: the urls and bigurls in testing/ should both contain urls in this format, with the same port number.
      - Handle multiple urls requests as written in a file: wget -i <path-to-urls>/<url file>
        For this project, it's either: wget -i <path-to-urls>/urls (which contains urls for several file types) and wget -i <path-to-urls>/bigurls (which only contains urls to big.txt in testing/).
      - Test the handling of multiple concurrent url requests: cat <path _to_urls_file> | xargs -n 1 -P <how_many_instances> wget
        This will read from the file containing the urls as with the case above, but will call <how_many_instances> of wget concurrently.

• Extra credits:
  - Only extra credit A was attempted. The attempted code produces wrong results and interfere with the server operation. This could easily be excluded by setting dynamic_flag = 0 and run the server with a static thread pool.
  - Ideally, dynamic_pool_size_update should do the following: Using a 10 requests/1 worker ratio, it will continuously check the number of active workers against the number of queued requests in the buffer (i.e. the server load). If the number of active workers dipped below this ratio, it will generate more worker threads until the ratio is met. Conversely, if there are too many workers running, it will delete (using pthread_cancel) workers until either the ratio is met or there are 10 workers running, whichever yields the larger number of workers. The number of workers at any point should not dip below 10 workers.
  - In practice, the attempt will cause the server to block all incoming requests after a dynamic_pool_size_update operation, and the requests cannot be processed.

• Assumptions: Using the same simplifying assumptions as outlined in the assignment.

• Contribution by each member of the team:
  - Minh Bui: Revised the queue implementation into a ring buffer implementation. Updated the dispatcher and worker functions to reflect the new implementation. Responsible for the attempt at extra credit A. Debugged the request processing in the worker thread function.
  - John Kimlinger: Implemented the threading and synchronization mechanisms; wrote the earlier iteration of dispatcher and worker functions.  
  - Andrew Trudeau: Implemented request logging and graceful termination with ^C.
