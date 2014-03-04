/* server_p.h */

/* my includes */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>

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


void service_request(int, char*);
int read_request(int, char*);

void send_response(int, char*, char*, char*, unsigned int);
void return_200_ok(int, char*, char*);
void return_bad_request(int, char*);
void return_not_found(int, char*);
void return_found(int, char*);
void return_server_error(int, char*);
