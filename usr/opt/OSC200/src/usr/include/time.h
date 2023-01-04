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
 *	@(#)$RCSfile: time.h,v $ $Revision: 4.3.10.4 $ (DEC) $Date: 1993/11/18 01:06:02 $
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
 * COMPONENT_NAME: time.h
 *                                                                    
 * Copyright International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the Rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 * 
 *	time.h   1.23  com/inc,3.1,9013 1/18/90 09:56:02 
 */                                                                   

#ifndef _TIME_H_
#define _TIME_H_

#include <standards.h>

/*
 *
 *      The ANSI standard requires that certain values be in time.h.
 *      It also requires that if _ANSI_C_SOURCE is defined then ONLY these
 *      values are present.
 *
 *      This header includes all the ANSI required entries.  In addition
 *      other entries for the XIX system are included.
 *
 */
#ifdef _ANSI_C_SOURCE

#include <machine/machtime.h>		/* for CLOCKS_PER_SEC */

/* The following definitions are required to be in time.h by ANSI */

#ifndef NULL
#define NULL 	0L
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long 	size_t;
#endif

#ifndef _CLOCK_T
#define _CLOCK_T
typedef int		clock_t;
#endif

#ifndef _TIME_T
#define _TIME_T
typedef int		time_t;
#endif

struct	tm {			/* see ctime(3) */
        int     tm_sec;         /* seconds after the minute [0-60] */
        int     tm_min;         /* minutes after the hour [0-59] */
        int     tm_hour;        /* hours since midnight [0-23] */
        int     tm_mday;        /* day of the month [1-31] */
        int     tm_mon;         /* months since January [0-11] */
        int     tm_year;        /* years since 1900 */
        int     tm_wday;        /* days since Sunday [0-6] */
        int     tm_yday;        /* days since January 1 [0-365] */
        int     tm_isdst;       /* Daylight Savings Time flag */
	long    __tm_gmtoff;
        char    *__tm_zone;
};

#ifdef _OSF_SOURCE
#define tm_gmtoff __tm_gmtoff
#define tm_zone   __tm_zone
#endif

#ifdef _NO_PROTO

extern clock_t 	clock();
extern double 	difftime();
extern time_t 	mktime();
extern time_t 	time();
extern char 	*asctime();
extern char 	*ctime();
extern struct tm *gmtime();
extern struct tm *localtime();
extern size_t 	strftime();

/* REENTRANT FUNCTIONS */
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
extern int 	asctime_r();
extern int	ctime_r();
extern int 	gmtime_r();
extern int 	localtime_r();
#endif

#else /* _NO_PROTO */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif

extern clock_t 	clock(void);
extern double 	difftime(time_t , time_t );
extern time_t 	mktime(struct tm *);
extern time_t 	time(time_t *);
extern char 	*asctime(const struct tm *);
extern char 	*ctime(const time_t *);
extern struct tm *gmtime(const time_t *);
extern struct tm *localtime(const time_t *);
extern size_t 	strftime(char *, size_t , const char *,const struct tm *);

/* REENTRANT FUNCTIONS */
#if defined(_THREAD_SAFE) || defined(_REENTRANT)
extern int 	asctime_r(const struct tm *, char *, int);
extern int	ctime_r(const time_t *, char *, int);
extern int	gmtime_r(const time_t *, struct tm *);
extern int 	localtime_r(const time_t *, struct tm *);
#endif

#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */

#endif /*_ANSI_C_SOURCE */
 
/*
 *   The following are values that have historically been in time.h.
 *
 *   They are NOT part of the ANSI defined time.h and therefore are
 *   not included when _ANSI_C_SOURCE is defined.
 */

#ifdef _POSIX_SOURCE

#include <sys/types.h>

#ifndef CLK_TCK
#define CLK_TCK   60       /* clock ticks/second, >= 10 */
#endif

/*
 * POSIX.4 Clocks and Timers constants are now defined in <time.h>,
 * not <sys/timers.h>. For compatibility with earlier drafts, they
 * will continue to live in <timers.h> and be included indirectly
 * here.
 */
#ifdef	_POSIX_4SOURCE
#include <sys/timers.h>
#include <sys/signal.h>
#define MAX_SECONDS LONG_MAX
#define NSEC_PER_USEC (1000)
#define NSEC_PER_SEC (1000000000)
#define TIMER_ABS 1
#define TIMER_ABSTIME 0x00000001
#define CLOCK_REALTIME 1
#endif	/* _POSIX_4SOURCE */
#endif  /* _POSIX_SOURCE */

extern char *tzname[];

#ifdef _NO_PROTO
extern void tzset();
#else
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern void tzset(void);
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */

#ifdef _XOPEN_SOURCE
#ifndef _BSD
#ifdef _SVID3_COMPAT
extern time_t timezone;
#else
extern long timezone;
#endif /* _SVID3_COMPAT */
#endif /* ! _BSD */
extern int daylight;

#if defined(__cplusplus)
extern "C"
{
#endif
extern char *strptime __((const char *, const char *, struct tm *));
#if defined(__cplusplus)
}
#endif

#endif /* _XOPEN_SOURCE */

#ifdef _OSF_SOURCE

#define TIMELEN 26
#define TMZNLEN 50

#define TM_GMTOFF       tm_gmtoff
#define TM_ZONE         tm_zone

#ifndef RTPC_NO_NLS
/*  Suggested default length of time/date buffer */
#define NLTBMAX	64
#ifdef _NO_PROTO
extern unsigned char *NLctime(), *NLasctime();
extern char *NLstrtime();
#else /* ~ _NO_PROTO */
#if defined(__STDC__) || defined(__cplusplus)
#if defined(__cplusplus)
extern "C"
{
#endif
extern unsigned char *NLctime(long *);
extern unsigned char *NLasctime(struct tm *);
extern char *NLstrtime(char *, size_t , const char *, const struct tm *);
#if defined(__cplusplus)
}
#endif
#endif
#endif /* _NO_PROTO */
#endif /* RTPC_NO_NLS */

#endif /* _OSF_SOURCE */

#endif /* _TIME_H_ */
