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
 * Open a file, write a string to it, and close the file.
 * Do it all using functions from the C standard: fopen/fprintf/fclose.
 */

/*
 * compile with "cc -o filewrite-C filewrite-C.c".
 *
 * Usage: ./filewrite-C test.C "this is a message"
 *
 */

#include <stdio.h>
#include <stdlib.h>

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
  FILE* f;
  int len, rc;

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
  if( (f = fopen(argv[1], "w")) == NULL )
    err(1, "could not open %s for writing.\n", argv[1]);
  
  // write to the file
  if( (rc = fprintf(f, "%s", argv[2])) != len)
  {
    fprintf(stderr, "write failed, wrote %d of %d chars.\n", rc, len);
    exit(-1);
  }

  // close the file
  if( fclose(f) != 0 )
    err(1, "failed to close file.\n");

  return(0);
}
