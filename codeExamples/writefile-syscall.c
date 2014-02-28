/*
 * Copyright (c) 2013 Paul Berube <pberube@ualberta.ca>
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
 * Pull off the veil...
 * Open a file, write a string to it, and close the file, as in filewrite-C.
 * Do it all using raw calls to syscall.
 */

/*
 * compile with "cc -o filewrite-POSIX filewrite-POSIX.c".
 *
 * Usage: ./filewrite-POSIX test.POSIX "this is a message"
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/syscall.h>

//#include <sys/types.h>
//#include <sys/stat.h>
#include <fcntl.h>

#include <err.h>
#include <errno.h>

#define MAXARGLEN  80


void usage()
{
	extern char * __progname;
	fprintf(stderr, "usage: %s filename string\n", __progname);
	exit(1);
}	


int main(int argc, char* argv[])
{
  int fd;
  int len, rc, written;

  // check the args
  if(argc != 3)
    usage();

  len = strnlen(argv[2], MAXARGLEN+1);
  if(len > MAXARGLEN)
  {
    fprintf(stderr, "string too long (%d, max %d)\n", len, MAXARGLEN);
    usage();
  }

  // open the file
  if( (fd = syscall(5, argv[1], O_CREAT | O_WRONLY | O_TRUNC, 0666)) == -1)
    err(1, "could not open %s for writing", argv[1]);

  // write to the file, checking for interupted writes
  for(written=0; written < len; written+=rc)
  {
		if( (rc = syscall(4, fd, argv[2]+written, len-written)) == -1) 
    {
			if (errno != EINTR)
				err(1, "write failed, only wrote %d of %d chars", written, len); 
			else
				rc = 0;
		}
	}	
  
  // close the file
  if(syscall(6, fd) != 0)
    err(1, "failed to close file.\n");

  return(0);
}
