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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: select.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:28:15 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "select.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:38:38"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   select
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"
#include <signal.h>

#define NAPINTERVAL 100

struct _timeval {
	long tv_sec;
	long tv_usec;
};

#ifdef NEEDSELECT

/*
 * NAME:        select
 *
 * FUNCTION:
 *
 *      Emulation of 4.2BSD select system call.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Code for various kinds of delays.  Most of this is nonportable and
 *      requires various enhancements to the operating system, so it won't
 *      work on all systems.  It is included in curses to provide a portable
 *      interface, and so curses itself can use it for function keys.
 *
 *      This is somewhat crude but
 *      better than nothing.  We do FIONREAD on each fd, and if we have to
 *      wait we use nap to avoid a busy wait.  The resolution of the nap
 *      will hurt response - so will the fact that we ignore the write fds.
 *      If we are simulating nap with a 1 second sleep, this will be very
 *      poor.
 *
 *      nfds is the number of fds to check - this is usually 20.
 *      prfds is a pointer to a bit vector of file descriptors - in the case
 *      where nfds < 32, prfds points to an integer, where bit 1<<fd
 *      is 1 if we are supposed to check file descriptor fd.
 *      pwfds is like prfds but for write checks instead of read checks.
 *      ms is the max number of milliseconds to wait before returning
 *      failure.
 *
 *      The value returned is the number of file descriptors ready for input.
 *      The bit vectors are updated in place.
 */

int
select(nfds, prfds, pwfds, pefds, timeout)
register int nfds;
int *prfds, *pwfds, *pefds;
struct _timeval *timeout;
{
	register int fd;
	register int rfds = *prfds;
	register int n;
	int nwaiting, rv = 0;
	long ms = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;

	for (;;) {
		/* check the fds */
		for (fd=0; fd<nfds; fd++)
			if (1<<fd & rfds) {
				ioctl(fd, FIONREAD, &nwaiting);
				if (nwaiting > 0) {
					rv++;
				} else
					*prfds &= ~(1<<fd);
			}
		if (rv)
			return rv;

		/* Nothing ready.  Should we give up? */
		if (ms <= 0)
			return 0;

		*prfds = rfds;	/* we clobbered it, so restore. */

		/* Wait a bit */
		n = NAPINTERVAL;
		if (ms < NAPINTERVAL)
			n = ms;
		ms -= n;
		napms(n);
	}
}
#endif /* NEEDSELECT */
