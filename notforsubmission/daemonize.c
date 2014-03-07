#include <sys/types.h> // purpose?
#include <sys/stat.h> // prupose?
#include <stdio.h> // standard for c programs
#include <stdlib.h> // standard for c programs
#include <fcntl.h> // purpose?
#include <errno.h> // supplies default error numbers
#include <unistd.h> // purpose?
#include <syslog.h> // using the syslogger for logging
#include <string.h> // standard string functions

int main(void) {
    
        /* Our process ID and session ID */
        pid_t pid, sid;

        /* Fork off from the parent process */
        pid = fork();
        if (pid < 0) {
                exit(EXIT_FAILURE); /* fork fail, error and exit */
        }
        /* if we got a good PID, then we can exit the parent process */
        if (pid > 0) {
                exit(EXIT_SUCCESS); /* gracefully exit parent process*/
        }

        /* change the file most mask */
        umask(0);

        /* open any log files here */

        /* create a new SID for the child process */
        sid = setsid();
        if (sid < 0) {
                /* log the failures */
                exit(EXIT_SUCCESS);
        }

        /* change the current working directory */
        if ((chdir("/")) < 0) {
                /* log the failures */
                exit(EXIT_SUCCESS);
        }

        /* close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
                
        /* daemon specific initialization goes here */

        /* the big loop */
        while(1) {
                /* do some task here... */

                sleep(30); /* wait for 30 seconds */
        }
        exit(EXIT_SUCCESS);
}

