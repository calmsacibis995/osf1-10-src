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
static char *rcsid = "@(#)$RCSfile: microtime.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/10/13 15:36:27 $";
#endif

/*
 * Old history carried from clock.c
 *
 * 25-Sep-92	Ray Glaser
 *	Nuk'd microtime() look-ahead round-up seconds logic entirely as
 *	did OSF in 1.1 !  It is all do-do and makes the clock appear to
 *	run backwards for a second on a 5500 !  The following test acted
 *	like a counter because of a missing = sign.  
 *	    (tvp->tv_usec = lasttime.tv_usec + 1) > 1000000) {
 *	All you need is a fast cpu and a lot of quick calls to microtime()...
 *		
 * 13-Feb-92	Fred Canter
 *	Removed the two tick time bump from microtime(). It causes
 *	microtime to return a time that is one second less than the
 *	previous time (sometimes). Why should microtime report time
 *	plus 2 ticks (2 * 3906 Usec)?
/**
#*/
#include <sys/kernel.h>

/*
 * Return the best possible estimate of the time in the timeval
 * to which tvp points.  We do this by reading the interval count
 * register to determine the time remaining to the next clock tick.
 * We must compensate for wraparound which is not yet reflected in the time
 * (which happens when the counter hits 0 and wraps after the splhigh(),
 * but before the counter latch command).  Also check that this time is
 * no less than any previously-reported time, which could happen around
 * the time of a clock adjustment.  Just for fun, we guarantee that
 * the time will be greater than the value obtained by a previous call.
 *
 * Note: 2/13/92 -- above comment does not match the code.
 *
 * Why add 2 ticks to the time returned by microtime()? It causes time
 * reported by microtime to go backwards one second (sometimes). I hate
 * this, but I retained the ability to turn the 2 tick bump back on,
 * because we are 10 days from SSB submit!
 */
#ifndef ORIG_CODE
microtime(tvp)
        register struct timeval *tvp;
{
        int     s = splclock(); /* splhigh to coordinate with hardclock() */
	TIME_READ_LOCK();
        *tvp = time;
	TIME_READ_UNLOCK();
        splx(s);
}

#else ORIG_CODE
int	microtime_add_two_ticks = 0;	/* default is no bump */

microtime(tvp)
	register struct timeval *tvp;
{
	static struct timeval lasttime;
	int	s = splclock();

	tvp -> tv_sec = time.tv_sec;

	if (microtime_add_two_ticks)
		tvp -> tv_usec = time.tv_usec + (1000000/hz) + tick;
	else
		tvp -> tv_usec = time.tv_usec;
	while (tvp -> tv_usec > 1000000) {
		tvp -> tv_sec++;
		tvp -> tv_usec -= 1000000;
	}
	if (tvp->tv_sec == lasttime.tv_sec &&
	    tvp->tv_usec <= lasttime.tv_usec &&
	    (tvp->tv_usec = lasttime.tv_usec + 1) > 1000000) {
		tvp->tv_sec++;
		tvp->tv_usec -= 1000000;
	}
	lasttime = *tvp;
	splx (s);
}
#endif ORIG_CODE

