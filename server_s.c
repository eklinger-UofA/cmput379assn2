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

/*  
 * Accepted command line args are as follows:
 *
 * ./server_s <TCP port number> </path/to/static/files/> </path/to/logfile.txt>
 *
 * More details in README
 */

/*
 * compile with cc:
 *
 * cc -c strlcpy.c
 * cc -o server_s server_s.c strlcpy.o
 *
 * Alternatively makefile is included:
 *
 * make makes 
 */

#include "server_s.h"

#define MAXCONN 256
struct con connections[MAXCONN];

/* states used in struct con. */
#define STATE_UNUSED 0
#define STATE_READING 1
#define STATE_WRITING 2

/*
 * get a free connection structure, to save a new connection in
 */
struct con * get_free_conn()
{
		int i;
		for (i = 0; i < MAXCONN; i++) {
		    	if (connections[i].state == STATE_UNUSED)
			        	return(&connections[i]);
		}
		return(NULL); /* we're all full - indicate this to our caller */
}

/*
 * close or initialize a connection - resets a connection to the default,
 * unused state.
 */
void closecon (struct con *cp, int initflag)
{
		if (!initflag) {
				if (cp->sd != -1)
					    close(cp->sd); /* close the socket */
				free(cp->buf); /* free up our buffer */
		}
		memset(cp, 0, sizeof(struct con)); /* zero out the con struct */
		cp->buf = NULL; /* unnecessary because of memset above, but put here
				 * to remind you NULL is 0.
				 */
		cp->sd = -1;
}


char dir_to_host[256];
char log_file[256];
char carriage_return[10] = "\r\n";

int main(int argc, char *argv[])
{
        struct sockaddr_in master, from;
        char *ep, ip[INET_ADDRSTRLEN];
        int i, port_number, sd, fromlength, max = -1, omax;
        u_short port;

        fd_set *readable = NULL, *writable = NULL;
        int rc;

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
        sd = socket(AF_INET, SOCK_STREAM, 0);
        if (sd < 0) {
                fprintf(stderr, "Server: cannot open master socket\n");
                exit(1);
        }

        /* Bind to the socket */
        if (bind(sd, (struct sockaddr*) &master, sizeof(master))){
                fprintf(stderr, "Server: cannot open master socket\n");
                exit(1);
        }
        
        /* We are now bound and listing to connections on sock
         * to a connected client
         */
        if (listen(sd, 5) == -1){
                fprintf(stderr, "listen failed\n");
        }

		/* initialize all our connection structures */
		for (i = 0; i < MAXCONN; i++)
			    closecon(&connections[i], 1);

        while(1){
                int maxfd = -1;
                int i = 0;
                
        		omax = max;
		        max = sd; /* the listen socket */
                
         		for (i = 0; i < MAXCONN; i++) {
			            if (connections[i].sd > max)
				                max = connections[i].sd;
		        }   

				if (max > omax) {
						/* we need bigger fd_sets allocated */
                    
						/* free the old ones - does nothing if they are NULL */
						free(readable);
						free(writable);
                    
						/*
						 * this is how to allocate fd_sets for select
						 */
						readable = (fd_set *)calloc(howmany(max + 1, NFDBITS),
						    sizeof(fd_mask));
						if (readable == NULL)
							err(1, "out of memory");
						writable = (fd_set *)calloc(howmany(max + 1, NFDBITS),
						    sizeof(fd_mask));
						if (writable == NULL)
							err(1, "out of memory");
						omax = max;
						/*
						 * note that calloc always returns 0'ed memory,
						 * (unlike malloc) so these sets are all set to 0
						 * and ready to go
						 */
				}
                else {
	 			        /*
	 			   		 * our allocated sets are big enough, just make
	 			   		 * sure they are cleared to 0. 
	 			   		 */
	 			   		memset(readable, 0, howmany(max+1, NFDBITS) *
	 			   		    sizeof(fd_mask));
	 			   		memset(writable, 0, howmany(max+1, NFDBITS) *
	 			   		    sizeof(fd_mask));
		        }

				/*
				 * we are always interesting in reading from the
				 * listening socket. so put it in the read set.
				 */
    
				FD_SET(sd, readable);
				if (maxfd < sd)
					    maxfd = sd;

				/*
				 * now go through the list of connections, and if we
				 * are interested in reading from, or writing to, the
				 * connection's socket, put it in the readable, or
				 * writable fd_set - in preparation to call select
				 * to tell us which ones we can read and write to.
				 */
				for (i = 0; i<MAXCONN; i++) {
						if (connections[i].state == STATE_READING) {
								FD_SET(connections[i].sd, readable);
								if (maxfd < connections[i].sd)
									    maxfd = connections[i].sd;
						}
						if (connections[i].state == STATE_WRITING) {
								FD_SET(connections[i].sd, writable);
								if (maxfd < connections[i].sd)
									    maxfd = connections[i].sd;
						}
				}

				/*
				 * finally, we can call select. we have filled in "readable"
				 * and "writable" with everything we are interested in, and
				 * when select returns, it will indicate in each fd_set
				 * which sockets are readable and writable
				 */
				i = select(maxfd + 1, readable, writable, NULL,NULL);
				if (i == -1  && errno != EINTR)
					    err(1, "select failed");

		        if (i > 0) {
				        if (FD_ISSET(sd, readable)) {
								struct con *cp;
								int newsd;
								socklen_t slen;
								struct sockaddr_in sa;
                                
								slen = sizeof(sa);
								newsd = accept(sd, (struct sockaddr *)&sa,
								    &slen);
								if (newsd == -1)
									err(1, "accept failed");
                                
								cp = get_free_conn();
								if (cp == NULL) {
									/*
									 * we have no connection structures
									 * so we close connection to our
									 * client to not leave him hanging
									 * because we are too busy to
									 * service his request
									 */
									close(newsd);
								} else {
									/*
									 * ok, if this worked, we now have a
									 * new connection. set him up to be
									 * READING so we do something with him
									 */
                                    inet_ntop(AF_INET, &(sa.sin_addr), ip,
                                        INET_ADDRSTRLEN);
                                    cp->ip = ip;
									cp->state = STATE_READING;
									cp->sd = newsd;
									cp->slen = slen;
									memcpy(&cp->sa, &sa, sizeof(sa));
								}
						}
                }
				/*
				 * now, iterate through all of our connections,
				 * check to see if they are readble or writable,
				 * and if so, do a read or write accordingly 
				 */
				for (i = 0; i<MAXCONN; i++) {
						if ((connections[i].state == STATE_READING) &&
						        FD_ISSET(connections[i].sd, readable))
						 	    select_read_request(&connections[i]);
						if ((connections[i].state == STATE_WRITING) &&
						        FD_ISSET(connections[i].sd, writable))
						 	    select_write_response(&connections[i]);
				}
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
                strlcpy(log_file, log_file_path, sizeof(log_file));
        }
        fclose(fp);
}

/* Reads in the request, saves it in the buffer in con struct */
void select_read_request(struct con *cp)
{
        char requestInfo[4096] = {0};
        char firstLine[80];
        int read = read_request(cp->sd, requestInfo);

        get_request_first_line(requestInfo, firstLine);
        
        cp->requestInfo = firstLine;
		cp->state = STATE_WRITING;
}

/* Write the response to the client */
void select_write_response(struct con *cp)
{
        char firstLine[80];
        char time_buffer[70];
        char s[256];
        char* http_method = NULL; 
        char* filePath = NULL; 
        char* httpProt = NULL; 
        char *source = NULL;
        char fullFilePath[100];
        char progress_string[8];
        struct stat sb;
        FILE *requestedfp;
        long bufsize;
        int progress;
        size_t newLen;
        
        get_current_time(time_buffer, 70);

        strlcpy(firstLine, cp->requestInfo, sizeof(firstLine));
        strcpy(s, firstLine);

        http_method = strtok(s, " "); 
        filePath = strtok(NULL, " "); 
        httpProt = strtok(NULL, " "); 

        if (check_http_method(http_method)){
        }
        else {
                fprintf(stderr, "Bad HTTP method\n");
                /* return a 400 response for a bad request */
                return_bad_request(cp->sd, time_buffer);
                write_logs(time_buffer, cp->ip, firstLine, "400 Bad Request", NULL);
		        cp->state = STATE_READING;
                return;
        }

        /* Concat the paths of the served dir and the requested file path */
        strlcpy(fullFilePath, dir_to_host, sizeof(fullFilePath));
        strcat(fullFilePath, filePath);


        /* check for existence of requested file */
        if (stat(fullFilePath, &sb) == -1){
                /* file doesn't exist */
                return_not_found(cp->sd, time_buffer);
                write_logs(time_buffer, cp->ip, firstLine, "404 Not Found", NULL);
		        cp->state = STATE_READING;
                return;
        }

        /* check for access/permission to the file */
        requestedfp = fopen(fullFilePath, "r");
        if (!requestedfp){
                fprintf(stderr, "requested file doesn't exist, should report "
                    "as error");
                return_forbidden(cp->sd, time_buffer);
                write_logs(time_buffer, cp->ip, firstLine, "403 Forbidden", NULL);
		        cp->state = STATE_READING;
                return;
        }

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

        progress = return_200_ok(cp->sd, time_buffer, source);
        sprintf(progress_string, "%d/%d", (int)strlen(source), progress);
        write_logs(time_buffer, cp->ip, firstLine, "200 OK", progress_string);
		cp->state = STATE_READING;
        
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
        FILE *fp; /* A file pointer */        
        int fc;

        fp = fopen(log_file, "a+");
        if (progress){
            fprintf(fp, "%s\t%s\t%s\t%s %s\n", time, ip, firstLine, response,
                progress);
        }
        else {
            fprintf(fp, "%s\t%s\t%s\t%s\n", time, ip, firstLine, response);
        }
        fc = fclose(fp);
        if (fc == 0){
        }
        else{
        }
}

/* gets the current time in the format suitable for http response */
void get_current_time(char* buff, int size)
{
        struct tm *timeinfo;
        time_t rawtime;

        time(&rawtime);
        timeinfo = gmtime(&rawtime);
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


        length = (int)strlen(responseBody);
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
