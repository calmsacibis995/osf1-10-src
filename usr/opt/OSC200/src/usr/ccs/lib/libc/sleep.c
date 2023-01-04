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
static char	*sccsid = "@(#)$RCSfile: sleep.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/07 22:44:37 $";
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
 * Copyright (c) 1980 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *   	sleep.c	5.3 (Berkeley) 9/30/87
 */

/*
 * Modification History:
 *
 * 23-Apr-1991 Lai-Wah Hui
 *        For RT the sleep() and usleep() functions are re-written
 *        such that these functions nolonger depend on the bsd on
 *        ITIMER_REAL timer.  The side effects of this are that
 *        these functions will now work correctly for threads.
 *        Also the time-remaining on the sleep interval will always
 *        correct regradless if the system time is changed.  Note:
 *        RT is defined by default on the compile file.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak sleep = __sleep
#endif
#endif
#include <sys/time.h>
#include <signal.h>

#if !(RT)    

#define	setvec(vec, a) \
	vec.sv_handler = a; vec.sv_mask = vec.sv_onstack = 0

static int ringring;
static void sleepx(); 

unsigned int
sleep(n)
	unsigned n;
{
	long omask;
	struct itimerval itv, oitv;
	register struct itimerval *itp = &itv;
	struct sigvec vec, ovec;
	int short_timer = 0, long_timer = 0;
	unsigned int retval;
	time_t start_time, end_time;
	int time_slept;

	if (n == 0)
		return(0);
	timerclear(&itp->it_interval);
	timerclear(&itp->it_value);
	if ((retval = setitimer(ITIMER_REAL, itp, &oitv)) < 0)
		return(retval);
	itp->it_value.tv_sec = n;
	if (timerisset(&oitv.it_value)) {
		if (timercmp(&oitv.it_value, &itp->it_value, >)) {

			/* sleep is shorter than previous timer; just
			 * remember so that we can restore it later
			 */
			long_timer = 1;
		}
		else {

			/* previous timer should go off before sleep ends;
			 * sleep for shorter time, and remember this fact
			 * so we can resignal SIGALRM
			 */
			itp->it_value = oitv.it_value;
			short_timer = 1;
		}
	}
	setvec(vec, (void *)sleepx);
	(void) sigvec(SIGALRM, &vec, &ovec);
	omask = sigblock(sigmask(SIGALRM));
	ringring = 0;
	(void) time(&start_time);
	(void) setitimer(ITIMER_REAL, itp, (struct itimerval *)0);
	sigpause(omask &~ sigmask(SIGALRM));

	/* some signal (either ours or another) occurred. Did we sleep
	 * the full requested time?
	 */
	(void) time(&end_time);
	time_slept = (int) end_time - start_time;
	if (time_slept < n)
		retval = n - time_slept;
	else retval = 0;

	(void) sigvec(SIGALRM, &ovec, (struct sigvec *)0);
	(void) sigsetmask(omask);
	if (short_timer) {
		if (ringring) {
		/* this was actually caller's original timer, so don't
		 * steal the signal from him; also, if he had set an interval,
		 * set his new timer for him
		 */
			kill(getpid(), SIGALRM);
			oitv.it_value = oitv.it_interval;
		}
	
		else
			/* some other signal; let his timer run */
			oitv.it_value.tv_sec -= time_slept;
	}
	else if (long_timer)
			/* adjust outstanding timer */
		oitv.it_value.tv_sec -= time_slept;

	(void) setitimer(ITIMER_REAL, &oitv, (struct itimerval *)0);
	return(retval);
}

static void
sleepx()
{

	ringring = 1;
}
#else   /* RT version of sleep() */
/*++
 *
 * sleep()
 *
 * Functional description:
 *
 *      This routine performs a sleep function.  It results in the
 *      calling process entering the wait state for the amount of time
 *      specified in n seconds.  The thread remains in the wait state until
 *      a signal is received, or until the time expires.
 *
 * Inputs:
 *
 *      n - the amount of time to sleep, expressed as seconds
 *
 *
 * Implicit inputs:
 *
 *      None
 *
 * Outputs:
 *
 *      The amount of unslept time, if the process is
 *      awoken from its sleep state by a signal
 *
 * Implicit outputs/side effects:
 *
 *
 *      None
 *--
 */

unsigned int
sleep(n)
	unsigned n;
{
        struct timeval rqtp, rmtp;
	if(n <= 0)
	    return(0);

	timerclear(&rmtp);
	rqtp.tv_sec = n;
	rqtp.tv_usec = 0;
	if(usleep_thread(&rqtp, &rmtp) == -1)
            return(-1);
        if (rmtp.tv_sec)
 	    return(rmtp.tv_sec);
        return(0);
}
#endif  /* RT */
