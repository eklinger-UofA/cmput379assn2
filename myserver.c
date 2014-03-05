/*
 * Copyright (c) 2014 Eric Klinger <eklinger@ualberta.ca>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/* myserver.c - base server code for the three implementations
 * of the server assignment 2 for CMPUT 379 Winter 2014
 *
 * Accepted command line args are as follows:
 *
 * ./myserver <TCP port number> </path/to/static/files/> </path/to/logfile.txt>
 *
 * More details in README
 */

/*
 * compile with gcc:
 *
 * gcc -c strlcpy.c
 * gcc -o server server.c strlcpy.o
 *
 * Alternatively makefile is included:
 *
 * make 
 * make run
 *
 * Testing for failure conditions added, please see README
 * and makefile for more details
 *
 * make clean also provided to clean up generated object files
 */

#include "myserver.h"


char dir_to_host[80];
char log_file[80];
char carriage_return[10] = "\r\n";

int main(int argc, char *argv[])
{
        struct sockaddr_in master, from;
        char *ep;
        int i, port_number, sock, fromlength;
        u_short port;

        /* if the number of args aren't as expected, print usage info */
        check_number_of_args(argc);

        /* safe to do this */
        port = get_port_number(argv[1]);

        /* Check file and logs */
        check_file_directory(argv[2]);
        check_log_file(argv[3]);

        /* time to set up and listen on the socket */
        memset(&master, 0, sizeof(master));
        master.sin_family = AF_INET;
        master.sin_addr.s_addr = htonl(INADDR_ANY);
        master.sin_port = htons(port);
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
                fprintf(stderr, "Server: cannot open master socket\n");
                exit(1);
        }

        /* Bind to the socket */
        if (bind(sock, (struct sockaddr*) &master, sizeof(master))){
                fprintf(stderr, "Server: cannot open master socket\n");
                exit(1);
        }
        
        /* We are now bound and listing to connections on sock
         * to a connected client
         */
        if (listen(sock, 5) == -1){
                fprintf(stderr, "listen failed\n");
        }

	    printf("Server up and listening for connections on port %u\n", port);
        while(1){
                int fromsd;
                fromlength = sizeof(from);
                fromsd = accept(sock, (struct sockaddr *) &from, &fromlength);
                if (fromsd == -1){
                        fprintf(stderr, "accept failed\n");
                }
                /* get the ip of the request to be used in logging later */
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(from.sin_addr), ip, INET_ADDRSTRLEN);
                
                /* Now ready to service the new request */
                service_request(fromsd, ip);
        }
        return(0);
}

/* Print a usage message and exit */
static void usage()
{
        extern char * __progname;
        fprintf(stderr, "Usage: %s portnumber path/to/files/to/server \
                /path/to/log/file.ext", __progname);
        exit(1);
}

/* Check that the number of args provided is 4 */
void check_number_of_args(int argc)
{
        /* if the number of args aren't as expected, print usage info */
        if (argc != 4) {
                usage();
        }
}

/* Gets the port number from argv[1] */
int get_port_number(char* port_string)
{
        char *ep;
        int port_number;
        /* get and print the port_number from arv */
        port_number = strtoul(port_string, &ep, 10);
        printf("port_number %u\n", port_number);

        if (*port_string == '\0' || *ep != '\0') {
                /* parameter wasn't a number, or was empty */
                fprintf(stderr, "%s - not a number\n", port_string);
                usage();
        }
        if ((errno == ERANGE && port_number == ULONG_MAX) || (port_number > USHRT_MAX)) {
                /* It's a number, but it either can't fit in an unsigned long or
                 * is too big for an unsigned short
                 */
                fprintf(stderr, "%s - value out of range\n", port_string);
                usage();
        }
        return port_number;
}

/* Check if the directory exists and if we have access */
void check_file_directory(char* directory_path)
{
        DIR* dir = opendir(directory_path);

        errno = 0;
        /* time to test the directory we are given in the running of the server */
        if (dir) {
                printf("Successfully opened directory\n");
                strlcpy(dir_to_host, directory_path, sizeof(dir_to_host));
                closedir(dir);
        }
        else if (ENOENT == errno) {
                fprintf(stderr, "Couldn't open directory\n");
                exit(1);
        }
        else {
                fprintf(stderr, "opendir failed for some other reason\n");
                exit(1);
        }
}

/* Check whether the provided log file exists and can be opened */
void check_log_file(char* log_file_path)
{
        /* Time to test the log file, if we can't open it error */
        FILE *fp = fopen(log_file_path, "r");
        if (!fp){
            fprintf(stderr, "Log files doesn't exist");
            exit(1);
        }
        else {
            printf("Log file exists\n");
            strlcpy(log_file, log_file_path, sizeof(log_file));
        }

}

/* Everything above has been refactored to an acceptable level */
/* And the stuff below needs some serious love */

void service_request(int fromsd, char* ip){
        char requestInfo[4096] = {0};
        int read = read_request(fromsd, requestInfo);
        printf("This is whats contained in requestInfo char wise. \n");
        printf("buffer: %s\n", requestInfo);
        printf("And the value READ is as follows: %d\n", read);
        // OK, now we have the request data, tokenize the first line and get on with it
        char firstLine[80];
        int i;
        for (i = 0; i < strlen(requestInfo); i++) {
            if (requestInfo[i] == '\n'){
                break;
            }
        }
        printf("Value of i: %d\n", i);
        // use strlcpy to copy first part of the request into new string
        strlcpy(firstLine, requestInfo, i);        
        printf("This is in firstLine: %s\n", firstLine);


        char s[256];
        strcpy(s, firstLine);
        /*
        char* token = strtok(s, " ");
        printf("token: %s\n", token);
        
        while (token) {
                printf("token: %s\n", token);
                token = strtok(NULL, " ");
        }
        */
        char* httpMethod = strtok(s, " "); 
        printf("httpmethod: %s\n", httpMethod);
        char* filePath = strtok(NULL, " "); 
        printf("filePath: %s\n", filePath);
        char* httpProt = strtok(NULL, " "); 
        printf("httpProt: %s\n", httpProt);

        // practice opening, adding to and closing the log file
        FILE *fp; /* A file pointer */        
        // log file located at: /Users/Eric/git/379/assignment2/testLogFile.txt
        fp = fopen(log_file, "a+");
        printf("Opened log file successfully\n");

        /* Concat the paths of the served dir and the requested file path */
        char fullFilePath[100];
        strlcpy(fullFilePath, dir_to_host, sizeof(fullFilePath));
        printf("fullFilePath: %s\n", fullFilePath);
        strcat(fullFilePath, filePath);
        printf("fullFilePath: %s\n", fullFilePath);

        FILE *requestedfp;

        time_t rawtime;
        struct tm *timeinfo;
        char buff[70];

        // prints: Current local time and date: Thu 27 Feb 2014 23:23:19 MST
        // this is the format for the logs and the responses to requests
        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buff, sizeof buff, "%a %d %b %Y %T %Z", timeinfo);


        requestedfp = fopen(fullFilePath, "r");
        if (!requestedfp){
            fprintf(stderr, "requested file doesn't exist, should report as error");
            return_not_found(fromsd, buff);
            return;
        }
        else {
            printf("requested file exists, can proceed and serve it\n");
        }

        // Read from the file, and get ints contents into a buffer
        // its in requestedfp
        /* Go to the end of the file */
        char *source = NULL;
        if (fseek(requestedfp, 0L, SEEK_END) == 0){
            /* Get the size of the file */
            long bufsize = ftell(fp);
            if (bufsize == -1){
                /* error */
                err(1, "Failed to get file size");
            }
            /* allocate our biffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));

            /* go back to the start of the file */
            if(fseek(requestedfp, 0L, SEEK_SET) != 0){
                /* error */
                err(1, "Failed to seek back to beginning of file");
            }
            
            /* read the entire file into memory */
            size_t newLen = fread(source, sizeof(char), bufsize, requestedfp);
            if (newLen == 0){
                err(1, "Error reading file");
            }
            else {
                source[++newLen] = '\0';
            }
        }
        fclose(requestedfp);

        printf("Moment of truth, does this fread work\n");
        printf("%s", source);

        // sending the file to the client
        return_200_ok(fromsd, buff, source);
        // Logging stuff works
        // Make a log entry for the reqest
        // time is in buff
        
        // Tue 05 Nov 2013 19:18:34 GMT    127.0.0.1    GET /testDocument.txt HTTP/1.1    404 Not Found
        printf("Stuff i want to write to the logfile:\n");
        printf("%s\n", buff);
        printf("%s\n", ip);
        printf("%s\n", firstLine);
        char responseText[80] = "200 OK";
        printf("%s\n", responseText);
        char progress[10] = "80/80";
        fprintf(fp, "%s\t%s\t%s\t%s %s\n", buff, ip, firstLine, responseText, progress);

        int fc = fclose(fp);
        if (fc == 0){
            printf("Closed file successfully\n");
        }
        else{
            printf("Failed to close file\n");
        }
        // end of logging, can update the log file with static data
        free(source);
}


/* Read the request from the client, return it in a buffer */
int read_request(int fromsd, char * buffer)
{
        int r, rc, maxread, reading, success;
                
        success = -1;
        reading = 1;
        r = -1;
        rc = 0;
        maxread = 4096 - 1;
        while ((r != 0) && (rc < maxread) && reading) {
                int i;
                r = read(fromsd, buffer + rc, maxread - rc);
                for (i = 0; i < strlen(buffer); i++){
                        if (buffer[i] == '\n'){
                                reading = 0;
                                rc = i;
                                success=0;
                                break;
                        }
                }
                if (r == -1){
                        if (errno != EINTR){
                                err(1, "read failed");
                        }
                }
                else {
                        rc += r;
                }
        }
        buffer[rc] = '\0';
        return success;
} 

void write_logs()
{

}

void send_response(int fromsd, char* time, char* firstLine,
                   char* responseBody, unsigned int bodySize)
{
        char returnBuffer[10000];
        char bodysize[5];
        sprintf(bodysize, "%d", (unsigned int)strlen(responseBody));
        printf("bodysize: %s", bodysize);

        strlcpy(returnBuffer, firstLine, sizeof(returnBuffer));
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, "Date: ");
        strcat(returnBuffer, time);
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, "Content-Type: text/html");
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, "Content-Length: ");
        strcat(returnBuffer, bodysize);
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, responseBody);
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, carriage_return);

        printf("return buffer for the response: %s\n", returnBuffer);

        ssize_t written, w;
        w = 0;
        written = 0;
        while (written < strlen(returnBuffer)){
                w = write(fromsd, returnBuffer + written, strlen(returnBuffer) - written);
                if (w == -1){
                        if (errno != EINTR)
                                err(1, "write failed");
                }
                else
                        written += w;
        }
        close(fromsd);
}

/* sends a 200 OK response */
void return_200_ok(int fromsd, char* time, char* responseBody)
{
        char firstLine[] = "HTTP/1.1 200 OK";

        send_response(fromsd, time, firstLine, responseBody,
            (unsigned int)strlen(responseBody));
}

/* sends a 400 bad request response */
void return_bad_request(int fromsd, char* time)
{
        char firstLine[] = "HTTP/1.1 400 Bad Request";
        char responseBody[4096];

        strlcpy(responseBody, "<html><body><h2>Malformed Request</h2>Your \
                browser sent a request I could not understand.</body> \
                </html>", sizeof(responseBody));
        printf("size of responseBody in bytes is: %d\n",
                (unsigned int)strlen(responseBody));
        send_response(fromsd, time, firstLine, responseBody,
                (unsigned int)strlen(responseBody));
}

/* sends a 404 not found response */
void return_not_found(int fromsd, char* time)
{
        char firstLine[] = "HTTP/1.1 404 Not Found";
        char responseBody[] = "<html><body><h2>Document not found</h2>You "
            "asked for a document that doesn't exist. That is so sad.</body>"
            "</html>";

        send_response(fromsd, time, firstLine, responseBody,
                (unsigned int)strlen(responseBody));
}

/* sends a 403 forbidden response */
void return_forbidden(int fromsd, char* time)
{
        char firstLine[] = "HTTP/1.1 403 Forbidden";
        char responseBody[] = "<html><body><h2>Permission Denied</h2>You "
                "asked for a document you are not permitted to see. It sucks "
                "to be you.</body></html>";

        send_response(fromsd, time, firstLine, responseBody,
                (unsigned int)strlen(responseBody));
}

/* sends a 500 Internal Server Error response */
void return_server_error(int fromsd, char* time)
{
        char firstLine[] = "HTTP/1.1 500 Internal Server Error";
        char responseBody[] = "<html><body><h2>Oops. That Didn't work</h2>I "
                "had some sort of problem dealing with your request. Sorry, "
                "I'm lame.</body></html>";

        send_response(fromsd, time, firstLine, responseBody,
                (unsigned int)strlen(responseBody));
}
