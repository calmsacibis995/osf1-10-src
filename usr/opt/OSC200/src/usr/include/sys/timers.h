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
/*	
 *	@(#)$RCSfile: timers.h,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/12/15 22:14:34 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef _TIMERS_H
#define _TIMERS_H

#ifndef _TIMESPEC
#define _TIMESPEC

#include <time.h>
typedef struct timespec {
        time_t  tv_sec;         /* seconds */
        long    tv_nsec;        /* microseconds */
} timespec_t;

#endif /* _TIMESPEC */

#ifdef POSIX_4D9
#include <sys/types.h>
struct itimerspec {
	struct timespec	it_interval;	/* timer period */
	struct timespec	it_value;	/* timer expiration */
};

struct itimercb {
#ifdef _POSIX_AYNCHRONOUS_EVENTS
	struct event	itcb_event;	/* timer event definition */
#endif
	int		itcb_count;	/* timer "overrun" count */
};


#define TIMEOFDAY	1	/* time of day clock type */

/*
 * Notification types
 */
#define DELIVERY_SIGNALS	1
#define DELIVERY_EVENTS		2

#ifdef nanosleep
#undef nanosleep
#endif
#define nanosleep(rqtp,rmtp)   nanosleep_d9(rqtp,rmtp)
/*
 * Functions
 */
_BEGIN_CPLUSPLUS
int getclock(int clock_type, struct timespec *tp);
int setclock(int clock_type, struct timespec *tp);
int resclock(int clock_type, struct timespec *res, struct timespec *maxval);
timer_t mktimer(int clock_type, int notify_type, void *reserved);
int rmtimer(timer_t timerid);
int gettimer(timer_t timerid, struct itimerspec *value);
int reltimer(timer_t timerid, struct itimerspec *value, struct itimerspec *ovalue);
int abstimer(timer_t timerid, struct itimerspec *value, struct itimerspec *ovalue);
int resrel(timer_t timerid, struct timespec *res, struct timespec *max);
int resabs(timer_t timerid, struct timespec *res, struct timespec *max);
int nanosleep_d9(struct timespec *rqtp, struct timespec *rmtp);
int ressleep(struct timespec *res, struct timespec *max);
_END_CPLUSPLUS
#elif defined(_POSIX_4SOURCE)                         /* _POSIX_4D9 */
#include <sys/types.h>
#include <sys/signal.h>

/* 
 * Constants and Macros
 */

/*
 * Data Structure Definitions
 */

/*
 * The maximum timeout value, in seconds, allowed for an P1003.4
 * interval timers.
 */

struct	itimerspec {
	struct		timespec it_interval; /* timer interval */
	struct		timespec it_value; /* current value */
};

/* 
 * useful macros,  note these are not defined by P1003.4
 */

#define	nsec_timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_nsec)
#define	nsec_timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_nsec = 0

/*
 * Compare two nano_second timers.  Not this only works for
 * <, > or = but not for any conbination of the two.
 */

#define	nsec_timercmp(tvp, fvp, cmp)			        	\
	((tvp)->tv_sec cmp (fvp)->tv_sec ||				\
	 (tvp)->tv_sec == (fvp)->tv_sec &&				\
	 (tvp)->tv_nsec cmp (fvp)->tv_nsec)

/*
 * Routines now made obsolete in D11
 */

#ifdef POSIX_4D10
#define clock_gettimedrift clock_getdrift
#define clock_settimedrift clock_setdrift
#define clock_getres 	   clock_getres_d10
#else
#undef clock_gettimedrift
#undef clock_settimedrift
#undef nanosleep_getres
#undef timer_getres
#define nanosleep_getres   obsolete_function_nanosleep_getres
#define timer_getres       obsolete_function_timer_getres
#define clock_settimedrift obsolete_function_clock_settimedrift
#define clock_gettimedrift obsolete_fucntion_clock_gettimedrift
#endif  /* POSIX_4D10 */

/* 
 * routine definitions
 */

#ifdef _NO_PROTO
int clock_gettime();
int clock_settime();
int clock_getdrift();
int clock_setdrift();
timer_t timer_create();
int timer_delete();
int timer_gettime();
int timer_settime();
int timer_getoverrun();
int nanosleep();
#ifndef POSIX_4D10
int clock_getres();
#else
int clock_getres_d10();
int nanosleep_getres();
int timer_getres();
#endif     /* POSIX_4D10            */
#else      /* function proto-typing */
_BEGIN_CPLUSPLUS
int clock_gettime(int clock_id, struct timespec *tp);
int clock_settime(int clock_id, struct timespec *tp);
int clock_getdrift(int clock_id, int *oppb);
int clock_setdrift(int clock_id, const int ppb, int *oppb);
timer_t timer_create(int clock_id, struct sigevent *evp);
int timer_delete(timer_t timerid);
int timer_gettime(timer_t timerid, struct itimerspec *value);  
int timer_settime(timer_t timerid, int flags, struct itimerspec *value, 
                   struct itimerspec *ovalue); 
int timer_getoverrun(timer_t timerid);
int nanosleep(struct timespec *rqtp, struct timespec *rmtp);

#ifndef POSIX_4D10
int clock_getres(int clock_id,struct timespec *res);
#else
int clock_getres_d10(int clock_id,struct timespec *res,struct timespec *maxval);
int nanosleep_getres(struct timespec *res, struct timespec *maxval); 
int timer_getres(timer_t timerid, int abstime, struct timespec *res,
                 struct timespec *maxval);
#endif     /* POSIX_4D10            */
_END_CPLUSPLUS

#endif  /* _NO_PROTO_ */
#endif  /* _POSIX_4SOURCE */
#endif	/* _TIMERS_H */
