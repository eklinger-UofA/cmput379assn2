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

/* server_p.c - base server code for the three implementations
 * of the server assignment 2 for CMPUT 379 Winter 2014
 *
 * Accepted command line args are as follows:
 *
 * ./server_p <TCP port number> </path/to/static/files/> </path/to/logfile.txt>
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

#include "server_p.h"


char dir_to_host[80];
char log_file[80];
char carriage_return[10] = "\r\n";
pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
        struct sockaddr_in master, from;
        char *ep, ip[INET_ADDRSTRLEN];
        int i, port_number, sock, fromlength;
        u_short port;

        /* for pthread stuff */
        pthread_t threads[NUM_THREADS];
        pthread_attr_t attr;
        struct request_info thread_data[NUM_THREADS];
        int rc, t;
        void *status;

        /* if the number of args aren't as expected, print usage info */
        check_number_of_args(argc);

        /* safe to do this */
        port = get_port_number(argv[1]);

        /* Check file and logs */
        check_file_directory(argv[2]);
        check_log_file(argv[3]);

    	/* don't daemonize if we compile with -DDEBUG */
	    if (daemon(1, 0) == -1)
		    err(1, "daemon() failed");

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

        /* init and set thread detached attributes */ 
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

        /* initialize mutex lock */
        if (pthread_mutex_init(&mutex, NULL) != 0){
                fprintf(stderr, "Init of mutex lock failed\n");
                exit(1);
        }

        t = 0;
	    printf("Server up and listening for connections on port %u\n", port);
        while(1){
                int fromsd;
                fromlength = sizeof(from);
                fromsd = accept(sock, (struct sockaddr *) &from, &fromlength);
                if (fromsd == -1){
                        fprintf(stderr, "accept failed\n");
                }
                /* get the ip of the request to be used in logging later */
                inet_ntop(AF_INET, &(from.sin_addr), ip, INET_ADDRSTRLEN);
                
                /* initialize thread data for this thread */
                thread_data[t].fromsd = fromsd;
                thread_data[t].ip = ip;

                if (t == NUM_THREADS){
                        t = 0;
                }
                /* Now ready to service the new request */
                rc = pthread_create(&threads[t], NULL, service_request, (void *)&thread_data[t]);
                t++;
                if (rc){
                        fprintf(stderr, "Error; return code from pthread_create is: %d\n", rc);
                        exit(1);
                }
        }
        pthread_exit(NULL);
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
        if ((errno == ERANGE && port_number == ULONG_MAX) ||
            (port_number > USHRT_MAX)) {
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
        /* time to test the directory we are given */
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
        fclose(fp);
}

/* Read the request and handle it */
//void service_request(int fromsd, char* ip)
void *service_request(void* td)
{
        char requestInfo[4096] = {0};
        char firstLine[80];
        char time_buffer[70];
        char s[256];
        char* http_method = NULL; 
        char* filePath = NULL; 
        char* httpProt = NULL; 
        char *source = NULL;
        char fullFilePath[100];
        char progress_string[8];
        char* ip = NULL;
        struct stat sb;
        FILE *requestedfp;
        long bufsize;
        int fromsd;
        int read;
        int progress;
        size_t newLen;

        struct request_info *thread_data;
        thread_data = td;
        ip = thread_data->ip;
        fromsd = thread_data->fromsd;
        read = read_request(fromsd, requestInfo);


        printf("This is whats contained in requestInfo char wise. \n");
        printf("buffer: %s\n", requestInfo);
        printf("And the value READ is as follows: %d\n", read);

        get_request_first_line(requestInfo, firstLine);
        printf("This is in firstLine: %s\n", firstLine);

        get_current_time(time_buffer, 70);

        strcpy(s, firstLine);

        http_method = strtok(s, " "); 
        printf("http_method: %s\n", http_method);
        filePath = strtok(NULL, " "); 
        printf("filePath: %s\n", filePath);
        httpProt = strtok(NULL, " "); 
        printf("httpProt: %s\n", httpProt);

        if (check_http_method(http_method)){
            printf("HTTP method is valid\n");
        }
        else {
            fprintf(stderr, "Bad HTTP method\n");
            /* return a 400 response for a bad request */
            return_bad_request(fromsd, time_buffer);
            write_logs(time_buffer, ip, firstLine, "400 Bad Request", NULL);
            return;
        }

        /* Concat the paths of the served dir and the requested file path */
        strlcpy(fullFilePath, dir_to_host, sizeof(fullFilePath));
        printf("fullFilePath: %s\n", fullFilePath);
        strcat(fullFilePath, filePath);
        printf("fullFilePath: %s\n", fullFilePath);


        /* check for existence of requested file */
        if (stat(fullFilePath, &sb) == -1){
                /* file doesn't exist */
                return_not_found(fromsd, time_buffer);
                write_logs(time_buffer, ip, firstLine, "404 Not Found", NULL);
                return;
        }

        /* check for access/permission to the file */
        requestedfp = fopen(fullFilePath, "r");
        if (!requestedfp){
                fprintf(stderr, "requested file doesn't exist, should report "
                    "as error");
                return_forbidden(fromsd, time_buffer);
                write_logs(time_buffer, ip, firstLine, "403 Forbidden", NULL);
                return;
        }

        // Read from the file, and get ints contents into a buffer
        // its in requestedfp
        /* Go to the end of the file */
        if (fseek(requestedfp, 0L, SEEK_END) == 0){
            /* Get the size of the file */

            bufsize = ftell(requestedfp);
            if (bufsize == -1){
                /* error */
                err(1, "Failed to get file size");
            }
            /* allocate our buffer to that size. */
            source = malloc(sizeof(char) * (bufsize + 1));

            /* go back to the start of the file */
            if(fseek(requestedfp, 0L, SEEK_SET) != 0){
                /* error */
                err(1, "Failed to seek back to beginning of file");
            }
            
            /* read the entire file into memory */
            newLen = fread(source, sizeof(char), bufsize, requestedfp);
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

        progress = return_200_ok(fromsd, time_buffer, source);
        printf("size of the file sent: %d\n", (int)strlen(source));
        printf("bytes written: %d\n", progress);
        sprintf(progress_string, "%d/%d", (int)strlen(source), progress);
        printf("progress string: %s\n", progress_string);
        write_logs(time_buffer, ip, firstLine, "200 OK", progress_string);
        
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

/* Gets the first line of the request from the client for future validation */
void get_request_first_line(char* requestInfo, char* firstLine)
{
        int i;

        for (i = 0; i < strlen(requestInfo); i++) {
            if (requestInfo[i] == '\n'){
                break;
            }
        }
        printf("Value of i: %d\n", i);
        // use strlcpy to copy first part of the request into new string
        strlcpy(firstLine, requestInfo, i);        
}

/* Checks the http request for validity of http method GET */
int check_http_method(char* method){
        if (strcmp(method, "GET")){
                /* method is not GET */
                return 0;
        }
        else {
                /* good http method */
                return 1;
        }
}

/* writes a suitable log record for the http reponse serviced */
void write_logs(char* time, char* ip, char* firstLine, char* response,
    char* progress)
{
        pthread_mutex_lock(&mutex);
        // practice opening, adding to and closing the log file
        FILE *fp; /* A file pointer */        
        int fc;

        // log file located at: /Users/Eric/git/379/assignment2/testLogFile.txt
        fp = fopen(log_file, "a+");
        printf("Opened log file successfully\n");
        if (progress){
            fprintf(fp, "%s\t%s\t%s\t%s %s\n", time, ip, firstLine, response,
                progress);
        }
        else {
            fprintf(fp, "%s\t%s\t%s\t%s\n", time, ip, firstLine, response);
        }
        fc = fclose(fp);
        if (fc == 0){
            printf("Closed file successfully\n");
        }
        else{
            printf("Failed to close file\n");
        }

        pthread_mutex_unlock(&mutex);
}

/* gets the current time in the format suitable for http response */
void get_current_time(char* buff, int size)
{
        time_t rawtime;
        struct tm *timeinfo;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buff, size, "%a %d %b %Y %T %Z", timeinfo);
}

/* writes the response to the client */
int send_response(int fromsd, char* time, char* firstLine,
                   char* responseBody, unsigned int bodySize)
{
        char returnBuffer[10000];
        char bodysize[5];
        int header_length, length;
        ssize_t written, w;

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
        header_length = strlen(returnBuffer) + strlen(carriage_return) * 4;
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, responseBody);
        strcat(returnBuffer, carriage_return);
        strcat(returnBuffer, carriage_return);


        printf("Just before writing file to the client\n");
        length = (int)strlen(responseBody);
        printf("The reported length is: %d\n", length);
        w = 0;
        written = 0;
        while (written < strlen(returnBuffer)){
                w = write(fromsd, returnBuffer + written, strlen(returnBuffer)
                    - written);
                if (w == -1){
                        if (errno != EINTR)
                                fprintf(stderr, "write failed");
                                return written;
                }
                else
                        written += w;
        }
        close(fromsd);
        return (written - header_length);
}

/* sends a 200 OK response */
int return_200_ok(int fromsd, char* time, char* responseBody)
{
        char firstLine[] = "HTTP/1.1 200 OK";

        return(send_response(fromsd, time, firstLine, responseBody,
            (unsigned int)strlen(responseBody)));
}

/* sends a 400 bad request response */
void return_bad_request(int fromsd, char* time)
{
        char firstLine[] = "HTTP/1.1 400 Bad Request";
        char responseBody[4096];

        strlcpy(responseBody, "<html><body><h2>Malformed Request</h2>Your "
                "browser sent a request I could not understand.</body> "
                "</html>", sizeof(responseBody));
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
