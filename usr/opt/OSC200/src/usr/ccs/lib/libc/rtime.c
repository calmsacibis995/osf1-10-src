/*
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * HISTORY
 */
#ifndef lint
static char     *sccsid = "@(#)$RCSfile: rtime.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:29:06 $";
#endif
/*
 */


/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 * 1.8 88/02/08 
 */

/*
 * rtime - get time from remote machine
 *
 * gets time, obtaining value from host
 * on the udp/time socket.  Since timeserver returns
 * with time of day in seconds since Jan 1, 1900,  must
 * subtract seconds before Jan 1, 1970 to get
 * what unix uses.
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak rtime = __rtime
#endif
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <netinet/in.h>
#include <stdio.h>

#define NYEARS	(1970 - 1900)
#define TOFFSET (60*60*24*(365*NYEARS + (NYEARS/4)))
extern errno;

static void do_close();

rtime(addrp, timep, timeout)
	struct sockaddr_in *addrp;
	struct timeval *timep;
	struct timeval *timeout;
{
	int s;
	fd_set readfds;
	int res;
	unsigned long thetime;
	struct sockaddr_in from;
	int fromlen;
	int type;

	if (timeout == NULL) {
		type = SOCK_STREAM;
	} else {
		type = SOCK_DGRAM;
	}
	s = socket(AF_INET, type, 0);
	if (s < 0) {
		return(-1);
	}
	addrp->sin_family = AF_INET;
	addrp->sin_port = htons(IPPORT_TIMESERVER);
	if (type == SOCK_DGRAM) {
		res = sendto(s, (char *)&thetime, sizeof(thetime), 0, 
			     (struct sockaddr *)addrp, sizeof(*addrp));
		if (res < 0) {
			do_close(s);
			return(-1);	
		}
		do {
			FD_ZERO(&readfds);
			FD_SET(s, &readfds);
			res = select(_rpc_dtablesize(), &readfds, (int *)NULL, 
				     (int *)NULL, timeout);
		} while (res < 0 && errno == EINTR);
		if (res <= 0) {
			if (res == 0) {
				errno = ETIMEDOUT;
			}
			do_close(s);
			return(-1);	
		}
		fromlen = sizeof(from);
		res = recvfrom(s, (char *)&thetime, sizeof(thetime), 0, 
			       (struct sockaddr *)&from, &fromlen);
		do_close(s);
		if (res < 0) {
			return(-1);	
		}
	} else {
		if (connect(s, (struct sockaddr *)addrp, sizeof(*addrp)) < 0) {
			do_close(s);
			return(-1);
		}
		res = read(s, (char *)&thetime, sizeof(thetime));
		do_close(s);
		if (res < 0) {
			return(-1);
		}
	}
	if (res != sizeof(thetime)) {
		errno = EIO;
		return(-1);	
	}
	thetime = ntohl(thetime);
	timep->tv_sec = thetime - TOFFSET;
	timep->tv_usec = 0;
	return(0);
}

static void
do_close(s)
	int s;
{
	int save;

	save = errno;
	(void) close(s);
	errno = save;
}
