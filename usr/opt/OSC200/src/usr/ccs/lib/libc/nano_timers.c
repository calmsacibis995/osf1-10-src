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
static char	*sccsid = "@(#)$RCSfile: nano_timers.c,v $ $Revision: 4.3.8.2 $ (DEC) $Date: 1993/06/07 23:30:01 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak abstimer = __abstimer
#pragma weak getclock = __getclock
#pragma weak gettimer = __gettimer
#pragma weak mktimer = __mktimer
#pragma weak nanosleep_d9 = __nanosleep_d9
#pragma weak reltimer = __reltimer
#pragma weak resabs = __resabs
#pragma weak resclock = __resclock
#pragma weak resrel = __resrel
#pragma weak ressleep = __ressleep
#pragma weak rmtimer = __rmtimer
#pragma weak setclock = __setclock
#endif
#define POSIX_4D9
#include <sys/time.h>
#include <errno.h>
#include <sys/timers.h>

#include "ts_supp.h"

#define NULL 0
#define NSEC_PER_USEC (1000)
#define NSEC_PER_SEC (1000000000)

#ifdef	_THREAD_SAFE
#include "rec_mutex.h"

extern struct rec_mutex _nanotimer_rmutex;
#define	RETURN(val)	return (TS_UNLOCK(&_nanotimer_rmutex), val)
#else
#define	RETURN(val)	return (val)
#endif	/* _THREAD_SAFE */

static pid_t timer_allocated = (pid_t)0;

int
getclock(int clock_type, struct timespec *tp)
{
	struct timeval berktv;

	if (clock_type != TIMEOFDAY) {
		_Seterrno(EINVAL);
		return (-1);
	}
	if (gettimeofday(&berktv, NULL) < 0)
		return (-1);

	tp->tv_sec = berktv.tv_sec;
	tp->tv_nsec = berktv.tv_usec * NSEC_PER_USEC;
	return (0);
}

int
setclock(int clock_type, struct timespec *tp)
{
	struct timeval berktv;

	if (clock_type != TIMEOFDAY) {
		_Seterrno(EINVAL);
		return (-1);
	}
	if (tp->tv_nsec < 0 || tp->tv_nsec >= NSEC_PER_SEC) {
		_Seterrno(EINVAL);
		return (-1);
	}

	berktv.tv_sec = tp->tv_sec;
	berktv.tv_usec = tp->tv_nsec / NSEC_PER_USEC;
	return (settimeofday(&berktv, NULL));
}

int
resclock(int clock_type, struct timespec *res, struct timespec *maxval)
{
	_Seterrno(ENOSYS);
	return (-1);
}

timer_t
mktimer(int clock_type, int notify_type, void *itimercbp)
{
	if (clock_type != TIMEOFDAY) {
		_Seterrno(EINVAL);
		return ((timer_t)-1);
	}
	if (notify_type != DELIVERY_SIGNALS) {
		_Seterrno(EINVAL);
		return ((timer_t)-1);
	}
	TS_LOCK(&_nanotimer_rmutex);
	if (timer_allocated == getpid()) {
		_Seterrno(EAGAIN);
		RETURN((timer_t)-1);
	}
	timer_allocated = getpid();
	RETURN((timer_t)0);
}

/*
 * ITIMER_REAL_COE HACK XXXXX	Undocumented extension
 * for nano_timers which will be Cleared On Exec.
 * Hacks are in common with bsd/kern_{time,exec}.c 
 * *** To Be Removed when nano_timer support moves into the kernel ***
 */ 
#define ITIMER_REAL_COE 0xDEADBEEF

int
rmtimer(timer_t timerid)
{
	struct itimerval btimer;

	TS_LOCK(&_nanotimer_rmutex);
	if (timer_allocated != getpid() || timerid != (timer_t)0) {
		_Seterrno(EINVAL);
		RETURN(-1);
	}
	timer_allocated = (pid_t)0;

	/* cancel pending timer, if any */

	btimer.it_value.tv_sec = 0;
	btimer.it_value.tv_usec = 0;
	btimer.it_interval.tv_sec = 0;
	btimer.it_interval.tv_usec = 0;
	if (setitimer( ITIMER_REAL_COE /*ITIMER_REAL*/, &btimer, NULL) < 0)
		RETURN(-1);

	RETURN(0);
}

int
gettimer(timer_t timerid, struct itimerspec *value)
{
	struct itimerval btimer;

	TS_LOCK(&_nanotimer_rmutex);
	if (timer_allocated != getpid() || timerid != (timer_t)0) {
		_Seterrno(EINVAL);
		RETURN(-1);
	}
	if (getitimer(ITIMER_REAL, &btimer) < 0)
		RETURN(-1);

	value->it_value.tv_sec = btimer.it_value.tv_sec;
	value->it_value.tv_nsec = btimer.it_value.tv_usec * NSEC_PER_USEC;
	value->it_interval.tv_sec = btimer.it_interval.tv_sec;
	value->it_interval.tv_nsec = btimer.it_interval.tv_usec * NSEC_PER_USEC;
	RETURN(0);
}

int
reltimer(timer_t timerid, struct itimerspec *value, struct itimerspec *ovalue)
{
	struct itimerval btimer;
	struct itimerval obtimer;
	struct itimerval *obp;

	TS_LOCK(&_nanotimer_rmutex);
	if (timer_allocated != getpid() || timerid != (timer_t)0) {
		_Seterrno(EINVAL);
		RETURN(-1);
	}
	if (value->it_value.tv_nsec >= NSEC_PER_SEC ||
	    value->it_interval.tv_nsec >= NSEC_PER_SEC ||
	    value->it_value.tv_nsec < 0 ||
	    value->it_interval.tv_nsec < 0) {
		_Seterrno(EINVAL);
		RETURN(-1);
	}
	btimer.it_value.tv_sec = value->it_value.tv_sec;
	btimer.it_value.tv_usec = value->it_value.tv_nsec / NSEC_PER_USEC;
	btimer.it_interval.tv_sec = value->it_interval.tv_sec;
	btimer.it_interval.tv_usec = value->it_interval.tv_nsec / NSEC_PER_USEC;
	if (setitimer( ITIMER_REAL_COE /*ITIMER_REAL*/,
					&btimer, ovalue ? &obtimer : NULL) < 0)
		RETURN(-1);
	if (ovalue != NULL) {
		ovalue->it_value.tv_sec = obtimer.it_value.tv_sec;
		ovalue->it_value.tv_nsec = obtimer.it_value.tv_usec * NSEC_PER_USEC;
		ovalue->it_interval.tv_sec = obtimer.it_interval.tv_sec;
		ovalue->it_interval.tv_nsec = obtimer.it_interval.tv_usec * NSEC_PER_USEC;
	}
	RETURN(0);
}

int
abstimer(timer_t timerid, struct itimerspec *value, struct itimerspec *ovalue)
{
	_Seterrno(ENOSYS);
	return (-1);
}

int
resrel(timer_t timerid, struct timespec *res, struct timespec *max)
{
	_Seterrno(ENOSYS);
	return (-1);
}

int
resabs(timer_t timerid, struct timespec *res, struct timespec *max)
{
	_Seterrno(ENOSYS);
	return (-1);
}

int
nanosleep(struct timespec *rqtp, struct timespec *rmtp)
{
	_Seterrno(ENOSYS);
	return (-1);
}

int
ressleep(struct timespec *res, struct timespec *max)
{
	_Seterrno(ENOSYS);
	return (-1);
}
