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
 *	@(#)$RCSfile: time.h,v $ $Revision: 4.4.3.7 $ (DEC) $Date: 1992/07/08 10:40:07 $
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
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#ifndef	_SYS_TIME_H_
#define _SYS_TIME_H_

#ifdef	_KERNEL
#include <sys/unix_defs.h>
#endif

#include <sys/limits.h>
#include <sys/types.h>
#include <sys/signal.h>

#ifdef  _KERNEL
/*
 * Structure returned by gmtime and localtime calls (see ctime(3)).
 */
struct tm {
        int     tm_sec;
        int     tm_min;
        int     tm_hour;
        int     tm_mday;
        int     tm_mon;
        int     tm_year;
        int     tm_wday;
        int     tm_yday;
        int     tm_isdst;
	long    tm_gmtoff;
        char    *tm_zone;
};
#else   /* _KERNEL  */
#include <time.h>
#endif  /*  _KERNEL  */

/*
 * The rest of this file is the interface to the BSD timer services.
 * Most of these services are implemented as subroutines that convert
 * the interface to the corresponding POSIX timer service.
 */

/*
 * The following are the BSD labels for the timer types.
 */
#define	ITIMER_REAL		0	/* Real time */
#define	ITIMER_VIRTUAL		1	/* Per-process time */
#define	ITIMER_PROF		2	/* Per-process user time */

struct timeval {
	int	tv_sec;		/* seconds */
	int	tv_usec;	/* microseconds */
};

 /*
  * Make timer interval for BSD "ITIMER_REAL"
  * timers and sleep functions.
  */

#define TOD_MAX_SECONDS 100000000

/*
 * Operations on timevals.
 *
 * Note that timercmp only works for cmp values of !=, >, and <.
 */
#define	timerisset(tvp)		((tvp)->tv_sec || (tvp)->tv_usec)
#define	timerclear(tvp)		(tvp)->tv_sec = (tvp)->tv_usec = 0
#define	timercmp(tvp, fvp, cmp)						\
	((tvp)->tv_sec cmp (fvp)->tv_sec ||				\
	 (tvp)->tv_sec == (fvp)->tv_sec &&				\
	 (tvp)->tv_usec cmp (fvp)->tv_usec)

struct	itimerval {
	struct		timeval it_interval; /* timer interval */
	struct		timeval it_value; /* current value */
};

/*
 * The following provides a way to convert the current time in GMT
 * to a local time.
 */
struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};

#define DST_NONE	0	/* not on dst */
#define DST_USA		1	/* USA style dst */
#define DST_AUST	2	/* Australian style dst */
#define DST_WET		3	/* Western European dst */
#define DST_MET		4	/* Middle European dst */
#define DST_EET		5	/* Eastern European dst */
#define DST_CAN		6	/* Canada */

#ifndef _KERNEL

#ifdef _NO_PROTO
extern int adjtime();
extern int getitimer();
extern int setitimer();
extern int gettimeofday();
extern int settimeofday();
extern int utimes();
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern int adjtime(struct timeval *, struct timeval *);
extern int getitimer(int, struct itimerval *);
extern int setitimer(int, struct itimerval *, struct itimerval *);
extern int gettimeofday(struct timeval *, struct timezone *);
extern int settimeofday(struct timeval *, struct timezone *);
extern int utimes(const char *, struct timeval *);
#if defined(__cplusplus)
}
#endif
#endif
#endif  /* _NO_PROTO */
#endif  /* _KERNEL */

#ifdef	_KERNEL
udecl_simple_lock_data(,time_lock)
#define	TIME_LOCK_INIT()	usimple_lock_init(&time_lock)
#define	TIME_READ_LOCK()	usimple_lock(&time_lock)
#define	TIME_WRITE_LOCK()	usimple_lock(&time_lock)
#define	TIME_READ_UNLOCK()	usimple_unlock(&time_lock)
#define	TIME_WRITE_UNLOCK()	usimple_unlock(&time_lock)
#endif

#ifdef 	_KERNEL
#define MAX_SECONDS LONG_MAX
#define NSEC_PER_USEC (1000)
#define NSEC_PER_SEC (1000000000)
#ifndef TIMER_ABS
#define TIMER_ABS 1
#endif
#ifndef TIMER_ABSTIME
#define TIMER_ABSTIME 0x00000001
#endif
#ifndef CLOCK_REALTIME
#define CLOCK_REALTIME 1
#endif
#endif	/* _KERNEL */

#if	defined(_KERNEL) || defined(_POSIX_4SOURCE)
/* 
 * P1003.4 Timer structure.
 */
typedef struct psx4_timer_struct {
        long		psx4t_idx;      /* Index value */
	long		psx4t_tid;	/* Timer's own ID */
	struct itimerval psx4t_timeval;	/* timeout and interval values */
	unsigned int	psx4t_active:1;	/* timer active	*/
	unsigned int	psx4t_type:1;   /* type of timer, 0=rel, 1 =abs	*/
	unsigned short	psx4t_overrun; 	/* timeout overrun interval count */
	sigval_t     	psx4t_value;	/* value to return with sig */
	int		psx4t_signo;	/* signal to raise on timeout */
	void     	*psx4t_p_proc;	/* pointer to proc/thread sig later */
} psx4_timer_t;

typedef struct psx4_tblock_struct {
	long		psx4tb_free;	/* index of next free timer */
	psx4_timer_t 	psx4_timers[TIMER_MAX];
} psx4_tblock_t;
#endif 	/* defined(_KERNEL) || defined(_POSIX_4SOURCE) */

#endif	/* _SYS_TIME_H_ */
