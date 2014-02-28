#include "myserver.h"

#include <dirent.h>

/* EXIT_SUCCESS = 0
 * EXIT_FAILURE = 1
 */

static void usage()
{
        extern char * __progname;
        fprintf(stderr, "Usage: %s portnumber path/to/files/to/server /path/to/log/file.ext", __progname);
        exit(1);
}
        

int main(int argc, char *argv[])
{
        char *ep;
        int i, portNumber;

        printf("argc: %d\n", argc);
        for (i = 0; i < argc; i++) {
                printf("argv[%d]: %s\n", i, argv[i]);
        }
        portNumber = strtoul(argv[1], &ep, 10);
        printf("portNumber %u\n", portNumber);

        if (*argv[1] == '\0' || *ep != '\0') {
                fprintf(stderr, "%s - not a number\n", argv[1]);
                usage();
        }

        /* time to test the directory we are given in the running of the server */
        DIR* dir = opendir(argv[2]);
        if (dir) {
                printf("Successfully opened directory\n");
                closedir(dir);
        }
        else if (ENOENT == errno) {
                printf("Couldn't open directory\n");
        }
        else {
                printf("opendir failed for some other reason\n");
        }

        return(0);
}

