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
static char	*sccsid = "@(#)$RCSfile: usleep.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 23:49:58 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	usleep.c	5.3 (Berkeley) 9/30/87
 */

/*
 * Modification History:
 *
 * 23-Apr-1991  Lai-Wah Hui 
 *        For RT the sleep() and usleep() functions are re-written
 *        such that these functions nolonger depend on the bsd on
 *        ITIMER_REAL timer.  The side effects of this are that
 *        these functions will now work correctly for threads.
 *        Also the time-remaining on the sleep interval will always
 *        correct regradless if the system time is changed.
 *        Note: RT is defined on the compile line by default.
 */


/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak usleep = __usleep
#endif
#endif
#include <sys/time.h>
#include <signal.h>

#define USPS	1000000		/* number of microseconds in a second */
#define TICK	10000		/* system clock resolution in microseconds */

#if !(RT)

#define	setvec(vec, a) \
	vec.sv_handler = a; vec.sv_mask = vec.sv_onstack = 0

static int ringring;
static void sleepx();

usleep(n)
	unsigned n;
{
	long omask;
	struct itimerval itv, oitv;
	register struct itimerval *itp = &itv;
	struct sigvec vec, ovec;

	if (n == 0)
		return;
	timerclear(&itp->it_interval);
	timerclear(&itp->it_value);
	if (setitimer(ITIMER_REAL, itp, &oitv) < 0)
		return;
	itp->it_value.tv_sec = n / USPS;
	itp->it_value.tv_usec = n % USPS;
	if (timerisset(&oitv.it_value)) {
		if (timercmp(&oitv.it_value, &itp->it_value, >)) {
			oitv.it_value.tv_sec -= itp->it_value.tv_sec;
			oitv.it_value.tv_usec -= itp->it_value.tv_usec;
			if (oitv.it_value.tv_usec < 0) {
				oitv.it_value.tv_usec += USPS;
				oitv.it_value.tv_sec--;
			}
		} else {
			itp->it_value = oitv.it_value;
			oitv.it_value.tv_sec = 0;
			oitv.it_value.tv_usec = 2 * TICK;
		}
	}
	setvec(vec, (void *)sleepx);
	(void) sigvec(SIGALRM, &vec, &ovec);
	omask = sigblock(sigmask(SIGALRM));
	ringring = 0;
	(void) setitimer(ITIMER_REAL, itp, (struct itimerval *)0);
	while (!ringring)
		sigpause(omask &~ sigmask(SIGALRM));
	(void) sigvec(SIGALRM, &ovec, (struct sigvec *)0);
	(void) sigsetmask(omask);
	(void) setitimer(ITIMER_REAL, &oitv, (struct itimerval *)0);
}

static void
sleepx()
{

	ringring = 1;
}
#else  /* RT version of usleep */

/*++
 *
 * usleep()
 *
 * Functional description:
 *
 *      This routine performs the  usleep function.  It results in the
 *      calling process entering the wait state for the amount of time
 *      specified in n useconds.  The thread remains in the wait state until
 *      a signal is received, or until the time expires.
 *
 * Inputs:
 *
 *      n - the amount of time to sleep, expressed as useconds
 *
 *
 * Implicit inputs:
 *
 *      None
 *
 * Outputs:
 *
 *      None
 *
 *
 * Implicit outputs/side effects:
 *
 *      errno will be set on error exit
 *
 *--
 */

usleep(n)
	unsigned n;
{
        struct timeval rqtp;
	if(n <= 0)
	    return;
	rqtp.tv_sec = n/USPS;
	rqtp.tv_usec = n%USPS;
	usleep_thread(&rqtp, (struct timeval *)0);
	return;
}
#endif
