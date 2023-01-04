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
static char *rcsid = "@(#)$RCSfile: ctime.c,v $ $Revision: 4.4.14.7 $ (DEC) $Date: 1993/12/07 20:58:46 $";
#endif
/************************************************************************
 *			Modification History
 *
 * 004	Jon Reeves, 20-Nov-1989
 *	Added _last_tz_set logic: effectively do tzset every time, not
 *	just first, since environment may have changed.  Still leaves a
 *	little acceleration in place.  Believe user's tm_isdst for
 *	mktime.
 *
 * 003	Ken Lesniak, 31-Aug-1989
 *	Modified mktime to correctly handle all times not representable
 *
 * 002	Ken Lesniak, 17-Jul-1989
 *	Moved tz_is_set flag to tzset.c and actually set flag in tzset()
 *
 * 001	Ken Lesniak, 20-Mar-1989
 *	Added support for POSIX 1003.1 compliancy.
 *	Added mktime and difftime routines for ANSI X3J11.
 *	Moved tzset* routines to tzset.c.
 *
 *	Based on Berkeley ctime.c 1.2
 ************************************************************************/

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak asctime_r = __asctime_r
#pragma weak ctime_r = __ctime_r
#pragma weak gmtime_r = __gmtime_r
#pragma weak localtime_r = __localtime_r
#pragma weak offtime_r = __offtime_r
#else
#pragma weak qadd = __qadd
#pragma weak qmuladd = __qmuladd
#pragma weak offtime = __offtime
#endif
#endif
#include <sys/time.h>
#include <tzfile.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "tzs.h"
#include "ts_supp.h"
#ifdef _THREAD_SAFE  


/*  
*   The thread-safe version of the code has to be synchronized.
*
*   mutex _ctime_rmutex for synchronization
*/
#include "rec_mutex.h"

extern struct rec_mutex	_ctime_rmutex;
#define TZSET  _unlocked_tzset
extern int              _zdump;
#else /* _THREAD_SAFE  */
#define TZSET tzset
int                     _zdump = 0;
#endif

extern struct state	_tzs;


int offtime_r(struct tm *tmp, time_t *clock, long offset);
struct tm * offtime(time_t *clock, long offset);

/*
*	Thread-safe routines.
*	The thread-safe routines do not return pointers
*	to static data the information is now passed in.
*	Locks are used in localtime and tzset to avoid
*	corruption of global data.
*/
/* Thread-safe ctime */
#ifdef _THREAD_SAFE
int
ctime_r(const time_t *timer, char *cbuf, int len)
{
	struct tm 	tm;

	if ((cbuf == NULL) || (len < 1)) {
		TS_SETERR(EINVAL);
		return(-1);
	}
	if (localtime_r(timer, &tm) == 0) {
		asctime_r(&tm, cbuf, len);
		return(0);
	} else
		return (-1);
}
#else /* _THREAD_SAFE */

char *
ctime(const time_t *timer)
{
	return(asctime(localtime(timer)));
}
#endif /* _THREAD_SAFE */


/* Thread-safe localtime */

#ifdef  _THREAD_SAFE 
int
localtime_r(const time_t *timer, struct tm *ct)
#else /* _THREAD_SAFE */
struct tm *
localtime(const time_t *timer)
#endif /* _THREAD_SAFE */
{
	register struct ttinfo *	ttisp;
#ifndef _THREAD_SAFE 
	register struct tm *		ct;
#endif /* _THREAD_SAFE */
	register int			i;
	time_t				t;

#ifdef _THREAD_SAFE 
	if (!timer || ct == NULL) {
		TS_SETERR(EINVAL);
		return(-1);
	}
	TS_LOCK(&_ctime_rmutex);
#endif /* _THREAD_SAFE */ 

	/* rest of tzset() call avoidance logic moved to tzset() */
	if (!_zdump) (void) TZSET();

	t = *timer;
	if (_tzs.timecnt == 0 || t < _tzs.ats[0]) {
		i = 0;
		while (_tzs.ttis[i].tt_isdst)
			if (++i >= _tzs.timecnt) {
				i = 0;
				break;
			}
	} else {
		for (i = 1; i < _tzs.timecnt; ++i)
			if (t < _tzs.ats[i])
				break;
		i = _tzs.types[i - 1];
	}
	ttisp = &_tzs.ttis[i];
	/*
	** To get (wrong) behavior that's compatible with System V Release 2.0
	** you'd replace the statement below with
	**	ct = offtime((time_t) (t + ttisp->tt_gmtoff), 0L);
	*/
#ifndef _THREAD_SAFE
	ct = offtime(&t, ttisp->tt_gmtoff);
#else /* _THREAD_SAFE */
	offtime_r(ct, &t, ttisp->tt_gmtoff);
#endif /* _THREAD_SAFE */

	ct->tm_isdst = ttisp->tt_isdst;
	tzname[ct->tm_isdst] = &_tzs.chars[ttisp->tt_abbrind];
	ct->tm_zone = &_tzs.chars[ttisp->tt_abbrind];

#ifndef _THREAD_SAFE
	return ct;
#else /* _THREAD_SAFE */
	TS_UNLOCK(&_ctime_rmutex);
	return(0);
#endif /* _THREAD_SAFE */

}


static char *
itoa2(cp, n)
register char *cp;
int	n;
{
	if (n >= 10)
		*cp++ = (n / 10) % 10 + '0';
	else
		*cp++ = ' ';
	*cp++ = n % 10 + '0';
	return cp;
}


/* Thread-safe asctime */

#ifdef  _THREAD_SAFE
int
asctime_r(const struct tm *timeptr, char *cp, int len)
#else /* _THREAD_SAFE */
char *
asctime(const struct tm *timeptr)
#endif /* _THREAD_SAFE */
{
	static char	wday_name[DAYS_PER_WEEK][3] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char	mon_name[MONS_PER_YEAR][3] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
#ifndef _THREAD_SAFE
	static char	cbuf[26];
	register char	*cp;
#endif
	register char	*ncp;
	register int	year;

#ifdef _THREAD_SAFE
	if ((!timeptr) || (cp == NULL) || len < 1) {
		TS_SETERR(EINVAL);
		return(-1);
	}
#else
	cp = cbuf;
#endif /* _THREAD_SAFE */

	ncp = wday_name[timeptr->tm_wday];
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;

	*cp++ = ' ';
	ncp = mon_name[timeptr->tm_mon];
	*cp++ = *ncp++;
	*cp++ = *ncp++;
	*cp++ = *ncp++;

	*cp++ = ' ';
	cp = itoa2(cp, timeptr->tm_mday);

	*cp++ = ' ';
	cp = itoa2(cp, timeptr->tm_hour + 100);

	*cp++ = ':';
	cp = itoa2(cp, timeptr->tm_min + 100);

	*cp++ = ':';
	cp = itoa2(cp, timeptr->tm_sec + 100);

	*cp++ = ' ';
	year = TM_YEAR_BASE + timeptr->tm_year;
	cp = itoa2(cp, year / 100);
	cp = itoa2(cp, year);

	*cp++ = '\n';
	*cp++ = '\0';

#ifdef  _THREAD_SAFE 
	return(0);
#else /* _THREAD_SAFE  */
	return cbuf;
#endif /* _THREAD_SAFE */
}

#ifdef _THREAD_SAFE
int
gmtime_r(const time_t *timer, struct tm *xtime)
#else /* _THREAD_SAFE */
struct tm *
gmtime(const time_t *timer)
#endif /* _THREAD_SAFE */
{
#ifndef _THREAD_SAFE
	register struct tm *	xtime;

	xtime = offtime(timer, 0L);
#else /* _THREAD_SAFE */
	TS_EINVAL(!timer || !xtime);
	offtime_r(xtime, timer, 0L);
#endif /* _THREAD_SAFE */

	tzname[0] = "GMT";
	xtime->tm_zone = "GMT";		/* UCT ? */

#ifndef _THREAD_SAFE
	return xtime;
#else /* _THREAD_SAFE */
	return(0);
#endif /* _THREAD_SAFE */

}

static int	mon_lengths[2][MONS_PER_YEAR] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int	year_lengths[2] = {
	DAYS_PER_NYEAR, DAYS_PER_LYEAR
};

#ifdef _THREAD_SAFE
int
offtime_r(struct tm *tmp, time_t *clock, long offset)
#else /* _THREAD_SAFE */
struct tm *
offtime(time_t *clock, long offset)
#endif /* _THREAD_SAFE */
{
	register time_t		days;
	register time_t		rem;
	register int		y;
	register int		yleap;
	register int *		ip;
#ifndef _THREAD_SAFE
	register struct tm *	tmp;
	static struct tm	tm;

	tmp = &tm;
#else
	TS_EINVAL(!tmp || !clock);
#endif /* _THREAD_SAFE */

	days = *clock / SECS_PER_DAY;
	rem = *clock % SECS_PER_DAY;
	rem += (time_t)offset;
	while (rem < 0) {
		rem += SECS_PER_DAY;
		--days;
	}
	while (rem >= SECS_PER_DAY) {
		rem -= SECS_PER_DAY;
		++days;
	}
	tmp->tm_hour = (int) (rem / SECS_PER_HOUR);
	rem = rem % SECS_PER_HOUR;
	tmp->tm_min = (int) (rem / SECS_PER_MIN);
	tmp->tm_sec = (int) (rem % SECS_PER_MIN);
	tmp->tm_wday = (int) ((EPOCH_WDAY + days) % DAYS_PER_WEEK);
	if (tmp->tm_wday < 0)
		tmp->tm_wday += DAYS_PER_WEEK;
	y = EPOCH_YEAR;
	if (days >= 0)
		for ( ; ; ) {
			yleap = isleap(y);
			if (days < (long) year_lengths[yleap])
				break;
			++y;
			days -= (long) year_lengths[yleap];
		}
	else do {
		--y;
		yleap = isleap(y);
		days += (long) year_lengths[yleap];
	} while (days < 0);
	tmp->tm_year = y - TM_YEAR_BASE;
	tmp->tm_yday = (int) days;
	ip = mon_lengths[yleap];
	for (tmp->tm_mon = 0; days >= (long) ip[tmp->tm_mon]; ++(tmp->tm_mon))
		days -= (long) ip[tmp->tm_mon];
	tmp->tm_mday = (int) (days + 1);
	tmp->tm_isdst = 0;
	tmp->tm_zone = "";
	tmp->tm_gmtoff = offset;

#ifndef _THREAD_SAFE 
	return tmp;
#else
	return 0;
#endif /* _THREAD_SAFE */
}

#ifndef	_THREAD_SAFE
/*
 * The following types and routines are provided so that mktime() can
 * use quadword math as it combines the broken-down time into
 * a time_t value. mktime() does not impose any bounds on the broken-down
 * time components, without the quadword math, all sorts of overflows
 * are possible.
 *
 * The routines are not general purpose quadword routines. The types
 * of the operands are restricted, to allow shortcuts in the algorthims.
 *
 * I know this is ugly, I can't think of any other way to allow mktime
 * to handle all broken-down times that can represent a valid time_t.
 */

/* (U)WORD length is 16 bits */
/* (U)DWORD length is 32 bits */
/* doubleword length is 32 bits */
/* quadword length is 64 bits */

#define WORD   short
#define UWORD  unsigned short
#define DWORD  int
#define UDWORD unsigned int

typedef union {
	UDWORD	dw[2];
	UWORD	w[4];
    } quadword;

typedef union {
	UDWORD	dw;
	UWORD	w[2];
    } doubleword;

/* quadword(qw) = quadword(qw) + DWORD(op) */

void qadd(qw, op)
	register quadword *qw;
	DWORD		op;
    {
	doubleword	ac;
	doubleword	sext;
	doubleword	dw_op;

	sext.dw = 0;
	if (op < 0)
	    sext.dw = (UDWORD)-1;

	dw_op.dw = (UDWORD)op;

	qw->w[0] = (ac.dw = (UDWORD)qw->w[0] + dw_op.w[0]);
	qw->w[1] = (ac.dw = (UDWORD)qw->w[1] + dw_op.w[1] + (UDWORD)ac.w[1]);
	qw->w[2] = (ac.dw = (UDWORD)qw->w[2] + (UDWORD)sext.w[0] +
		            (UDWORD)ac.w[1]);
	qw->w[3] += (UDWORD)sext.w[1] + (UDWORD)ac.w[1];
    };

/* quadword(qw) = (quadword(qw) * UWORD(mop)) + DWORD(aop) */

void qmuladd(qw, mop, aop)
	register quadword *qw;
	UWORD		mop;
	DWORD		aop;
    {
	doubleword	ac;

	qw->w[0] = (ac.dw = (UDWORD)qw->w[0] * (UDWORD)mop);
	qw->w[1] = (ac.dw = ((UDWORD)qw->w[1] * (UDWORD)mop) + ac.w[1]);
	qw->w[2] = (ac.dw = ((UDWORD)qw->w[2] * (UDWORD)mop) + ac.w[1]);
	qw->w[3] = (qw->w[3] * mop) + ac.w[1];

	qadd(qw, aop);
    }

time_t mktime(tp)
	register struct tm *tp;
    {
	time_t		t;
	quadword	tq;
	int		y;
	int		m;
	int		ly;
	int		isdst;
	int		i;
	struct ttinfo	*ttis;
	int 		mktime_max_year,
			mktime_min_year,
			int_max,
			int_min;

	int_max=INT_MAX;
	int_min=INT_MIN;
 
	mktime_max_year = (int_max + 
			(int_min / MONS_PER_YEAR) + 
			(int_min / DAYS_PER_NYEAR) + 
			(int_min / (HOURS_PER_DAY * DAYS_PER_NYEAR)) + 
			(int_min / (MINS_PER_HOUR * HOURS_PER_DAY * 
				DAYS_PER_NYEAR)) + 
			(int_min / (SECS_PER_MIN * MINS_PER_HOUR * 
				HOURS_PER_DAY * DAYS_PER_NYEAR)) + 
			TM_YEAR_BASE);

	mktime_min_year = (int_min + 
			(int_max / MONS_PER_YEAR) + 
			(int_max / DAYS_PER_NYEAR) + 
			(int_max / (HOURS_PER_DAY * DAYS_PER_NYEAR)) + 
			(int_max / (MINS_PER_HOUR * HOURS_PER_DAY * 
				DAYS_PER_NYEAR)) + 
			(int_max / (SECS_PER_MIN * MINS_PER_HOUR * 
				HOURS_PER_DAY * DAYS_PER_NYEAR)) + 
			TM_YEAR_BASE);

	/* Per VSX4/XPG3 test suite: call tzset() to set tzname[] before
	 * doing any bounds checking so it will be set even if we
         * fail out.
         */
        TS_LOCK(&_ctime_rmutex);
	/* tzset() call avoidance logic moved to tzset() */
	(void)TZSET();
        TS_UNLOCK(&_ctime_rmutex);

	/* Make sure the year is in bounds to avoid overflowing the */
	/* month-year normalization */

	if (tp->tm_year < mktime_min_year || tp->tm_year > mktime_max_year)
	    return (time_t)-1;

	/* Get normalized month and year */

	y = tp->tm_year + TM_YEAR_BASE + (tp->tm_mon / MONS_PER_YEAR);
	if ((m = tp->tm_mon % MONS_PER_YEAR) < 0) {
	    m += MONS_PER_YEAR;
	    y--;
	};
	ly = isleap(y);

	/* Determine the number of days since the epoch */

	tq.dw[0] = tq.dw[1] = -1;
	qadd(&tq, tp->tm_mday);
	while (m != 0)
	    qadd(&tq, mon_lengths[ly][--m]);

	if (y >= EPOCH_YEAR) {
	    while (y != EPOCH_YEAR) {
		y--;
		qadd(&tq, year_lengths[isleap(y)]);
	    };
	} else {
	    do {
		qadd(&tq, -year_lengths[isleap(y)]);
		y++;
	    } while (y != EPOCH_YEAR);
	};
	
	/* Now convert everything to seconds */

	qmuladd(&tq, HOURS_PER_DAY, tp->tm_hour);
	qmuladd(&tq, MINS_PER_HOUR, tp->tm_min);
	qmuladd(&tq, SECS_PER_MIN, tp->tm_sec);

	/* Make sure the resulting time can be represented by a time_t */

	if (tq.dw[1] != 0 && tq.dw[1] != (UDWORD)-1)
	    return (time_t)-1;

	t = (time_t)tq.dw[0];

	/* Determine the GMT offset for the time */

	ttis = &_tzs.ttis[0];
	if (tp->tm_isdst < 0) {
	    if (_tzs.timecnt != 0 && t >= _tzs.ats[0] + ttis->tt_gmtoff) {
		for (i = 1; i < _tzs.timecnt; i++) {
		    ttis = &_tzs.ttis[_tzs.types[i - 1]];
		    if (t < _tzs.ats[i - 1] + ttis->tt_gmtoff) {
                        TS_UNLOCK(&_ctime_rmutex);
			return (time_t)-1;
                    }
		    if (t < _tzs.ats[i] + (&_tzs.ttis[_tzs.types[i]])->tt_gmtoff)
			break;
		};
	    };
	} else {
	    /* User specified DST, normalize to 0 or 1 (yes, '=' is right) */
	    if (isdst = tp->tm_isdst)
		isdst = 1;

	    /* Search for an appropriate DST setting; fall out if no DST */
	    for (i = 0; i < _tzs.timecnt; i++) {
		ttis = &_tzs.ttis[_tzs.types[i]];
		if (ttis->tt_isdst == isdst)
		    break;
	    }

	    /* Asked for DST (or standard), but none defined */

	    if (ttis->tt_isdst != isdst) {

	       /* Per VSX4/XPG3 test suite: if isdst was 1, try same search with
		* isdst = 0 even though the user asked for DST.  If this fails,
                * THEN return (time_t-1).
                */
	        if (isdst == 1) {
	           isdst = 0;
	           for (i = 0; i < _tzs.timecnt; i++) {
		      ttis = &_tzs.ttis[_tzs.types[i]];
		      if (ttis->tt_isdst == isdst)
		         break;
	           }
		}
	        if (ttis->tt_isdst != isdst) {
                   TS_UNLOCK(&_ctime_rmutex);
		   return (time_t)-1;
		}
            }
	}

	/* Adjust for time zone and daylight savings time */

	t -= ttis->tt_gmtoff;

	/* Normalize the broken down time and fill in the unspecified fields */

	(void)memcpy(tp, localtime(&t), sizeof(struct tm));

	/* Return the time */
        TS_UNLOCK(&_ctime_rmutex);

	return t;
    }
#endif	/*!_THREAD_SAFE*/
