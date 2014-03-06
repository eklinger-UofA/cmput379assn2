/* server_f.h */

/* my includes */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dirent.h>
#include <time.h>

/* EXIT_SUCCESS = 0
 * EXIT_FAILURE = 1
 */

/* All error handling and input checking function */
static void usage();
void check_number_of_args(int);
int get_port_number(char*);
void check_file_directory(char*);
void check_log_file(char*);
static void kidhandler(int signum); 

/* Functions to handle the request */
void service_request(int, char*);
int read_request(int, char*);
void get_request_first_line(char*, char*);
int check_http_method(char*);

/* Logging functions */
void write_logs(char*, char*, char*, char*, char*);
void get_current_time(char*, int);

/* Functions for writing responses back to the client */
int send_response(int, char*, char*, char*, unsigned int);
int return_200_ok(int, char*, char*);
void return_bad_request(int, char*);
void return_not_found(int, char*);
void return_forbidden(int, char*);
void return_server_error(int, char*);

