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
static char rcsid[] = "@(#)$RCSfile: draino.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:22:57 $";
#endif
/*
 * HISTORY
 */
/***
 ***  "draino.c  1.7  com/lib/curses,3.1,9008 12/4/89 21:01:17";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   draino
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/*
 * Code for various kinds of delays.  Most of this is nonportable and
 * requires various enhancements to the operating system, so it won't
 * work on all systems.  It is included in curses to provide a portable
 * interface, and so curses itself can use it for function keys.
 */


#include "cursesext.h"
#include <signal.h>

#define NAPINTERVAL 100

struct _timeval {
	long tv_sec;
	long tv_usec;
};

/*
 * NAME:        draino
 *
 * FUNCTION:
 *
 *      Wait until the output has drained enough that it will only take
 *      ms more milliseconds to drain completely.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Needs Berkeley TIOCOUTQ ioctl.  Returns ERR if impossible.
 */

int
draino(ms)
int ms;
{
	int ncthere;	/* number of chars actually in output queue */
	int ncneeded;	/* number of chars = that many ms */
	int rv;		/* ioctl return value */

#ifdef TIOCOUTQ
# define _DRAINO
	/* 10 bits/char, 1000 ms/sec, baudrate in bits/sec */
	ncneeded = baudrate() * ms / (10 * 1000);
	for (;;) {
		ncthere = 0;
		rv = ioctl(cur_term->Filedes, TIOCOUTQ, &ncthere);
#ifdef DEBUG
		fprintf(outf, "draino: rv %d, ncneeded %d, ncthere %d\n",
			rv, ncneeded, ncthere);
#endif
		if (rv < 0)
			return ERR;	/* ioctl didn't work */
		if (ncthere <= ncneeded) {
			return (0);
		}
		napms(NAPINTERVAL);
	}
#endif

#ifdef TCSETAW
# define _DRAINO
	/*
	 * USG simulation - waits until the entire queue is empty,
	 * then sets the state to what it already is (e.g. no-op).
	 * This only works if ms is zero.
	 */
	if (ms <= 0) {
		ioctl(cur_term->Filedes, TCSETAW, cur_term->Nttyb);
		return (OK);
	}
#endif

#ifndef _DRAINO

	return ERR;
#endif
}
