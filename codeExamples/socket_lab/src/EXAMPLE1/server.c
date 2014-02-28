#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* --------------------------------------------------------------------- */
/* This is a sample server which opens a stream socket and then awaits   */
/* requests coming from client processes. In response for a request, the */
/* server sends an integer number such that different processes get      */
/* distinct numbers. We assume that the server and the clients run on    */
/* the same machine.                                                     */
/* --------------------------------------------------------------------- */

int main(void) {
    int sock, snew, fromlength, number;
    struct sockaddr_un master, from;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) {
        perror ("Server: cannot open master socket");
        exit (1);
    }

    strcpy(master.sun_path, "the_number_server");
    master.sun_family = AF_UNIX;

    if (bind(sock, (struct sockaddr*) &master, sizeof(struct sockaddr_un))) {
        perror ("Server: cannot bind master socket");
        exit (1);
    }

    number = 0;
    listen(sock, 5);

    while (1) {
        fromlength = sizeof(from);
        snew = accept(sock, (struct sockaddr*) &from, &fromlength);
        if (snew < 0) {
            perror ("Server: accept failed");
            exit (1);
        }

        write(snew, &number, sizeof(number));
        close(snew);
        number++;
    }
    return 0;
}
