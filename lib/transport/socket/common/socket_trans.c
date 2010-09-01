/*
 *  The OpenDiamond Platform for Interactive Search
 *  Version 4
 *
 *  Copyright (c) 2007-2008 Carnegie Mellon University
 *  All rights reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "socket_trans.h"

/*
 * Read "n" bytes from a descriptor reliably.
 */
ssize_t
readn(int fd, void *vptr, size_t n)
{
  size_t  nleft;
  ssize_t nread;
  char   *ptr;

  ptr = vptr;
  nleft = n;

  while (nleft > 0) {
    if ( (nread = read(fd, ptr, nleft)) < 0) {
      perror("read");
      if (errno == EINTR)
	  nread = 0;	/* and call read() again */
      else
	  return (-1);
    } else if (nread == 0)
      break;		/* EOF */

    nleft -= nread;
    ptr += nread;
  }
  return (n - nleft);		/* return >= 0 */
}


/*
 * Write "n" bytes to a descriptor reliably.
 */
ssize_t
writen(int fd, const void *vptr, size_t n)
{
  size_t nleft;
  ssize_t nwritten;
  const char *ptr;

  ptr = vptr;
  nleft = n;
  while (nleft > 0) {
    if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
      if (nwritten < 0 && errno == EINTR)
	  nwritten = 0;   /* and call write() again */
      else
	  return (-1);    /* error */
    }

    nleft -= nwritten;
    ptr += nwritten;
  }
  return (n);
}

const char *
diamond_error(int ret)
{
	switch(ret) {
	case DIAMOND_FAILURE:
		return "Generic failure.";
	case DIAMOND_FCACHEMISS:
		return "Signature not found in the cache";
	case DIAMOND_NOSTATSAVAIL:
		return "Statistics not available at the moment."
		       " (Is a search running?)";
	case DIAMOND_NOMEM:
		return "Failed to allocate enough memory.";
	case DIAMOND_COOKIE_EXPIRED:
		return "Scope cookie is no longer valid.";
	}
	return strerror(ret);
}