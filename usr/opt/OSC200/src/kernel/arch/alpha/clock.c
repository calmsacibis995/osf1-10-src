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
static char *rcsid = "@(#)$RCSfile: clock.c,v $ $Revision: 1.2.2.14 $ (DEC) $Date: 1992/12/14 15:54:34 $";
#endif
/*
 * Modification History: alpha/clock.c
 *
 * 28-oct-91 -- prm
 *      Add routines to support convertion from TM structure to todr format
 *      - toywrite_convert; toyread_convert.
 *
 * 26-Apr-91 -- afd
 *	Created this file for Alpha support.
 */

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <hal/cpuconf.h>
#include <kern/assert.h>
#include <mach/boolean.h>

#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/scb.h>

#include <io/dec/uba/ubareg.h>
#include <mach/vm_prot.h>
/*
 * Machine-dependent clock routines.
 *
 * Startrtclock restarts the real-time clock, which provides
 * hardclock interrupts to kern_clock.c.
 *
 * Inittodr initializes the time of day hardware which provides
 * date functions.  Its primary function is to use some file
 * system information in case the hardware clock lost state.
 *
 * Resettodr restores the time of day hardware after a time change.
 */

/*
 * Setup the real-time clock.
 * The clock is always running on Alpha, so we don't need to start it.  
 * Instead, we need to enable the interrupt to be taken by modifying the 
 * instruction at the location clock_enable. (see locore.s).
 */

extern int clock_enable;
startrtclock()
{
        write_instruction(&clock_enable,0x420035a1);/* cmpeq r16,0x1,r1 */
}

/*
 * Initialize the time of day register, based on the time base which is, e.g.
 * from a filesystem.  Base provides the time to within six months,
 * and the time of year clock provides the rest.
 */

/* forward reference declaration */
int cvt_sec_to_tm(/* seconds, *tm */);
long cvt_tm_to_sec(/* *tm */);

inittodr(base)
	time_t base;
{
	long todr;
	long deltat;
	long tmp_tv_sec;
	int year = YRREF;
	u_int secyr;
	time_t min_time;
	struct tm conversion_tm;

#ifdef	notdef
	extern int savetime;	/* used to save the boot time for errlog */
#endif

	/*
	 * Get the hardware's idea of how much time has gone by since
	 * the begining of the year as reflected in 'base'.
	 *
	 * 'base' is initialized from the root file system's time stamp.
	 *
	 * todr is in 100ns increments. todr/UNITSPERSEC, should
	 * reflect elapsed seconds.
	 */
	todr = read_todclk();

	/* 
	 * time can't be before... today, Oct 29 1992.
	 * the epoch (1970) + 22yrs (1992) + 43wks (last wk Oct)
	 * So... make it noon on someday Monday of last week.
	 */
	min_time = 22*SECYR + (42*7)*SECDAY + SECDAY*5/2;

	/* check for time before the update of this code */
	if (base < min_time) {
		printf("WARNING: preposterous time in file system");

		time.tv_sec = min_time;		/* time.tv_sec will */
						/* overflow in 2037, */
						/* with the epoch */
						/* being 1970 */   
		resettodr();
#ifdef	notdef
		savetime = time.tv_sec;
#endif
		printf(" -- CHECK AND RESET THE DATE!\n");
		return;
	}

	if (todr < TODRZERO) {
		printf("WARNING: lost battery backup clock");
		time.tv_sec = base;
		/*
		 * Believe the time in the file system for lack of
		 * anything better, resetting the TODR.
		 */
		resettodr();
#ifdef	notdef
		savetime = time.tv_sec;
#endif
		printf(" -- CHECK AND RESET THE DATE!\n");
		return;
	}


	/*
	 * Get the seconds since the epoch for Jan 1st of the base
	 * year. Use the same conversion routines called by the CPU
	 * specific readtodr routines.
	 */
	cvt_sec_to_tm(base,&conversion_tm);
	conversion_tm.tm_sec = 0;
	conversion_tm.tm_min = 0;
	conversion_tm.tm_hour = 0;
	conversion_tm.tm_mday = 1;
	conversion_tm.tm_mon = 0;
	tmp_tv_sec = cvt_tm_to_sec(&conversion_tm);

	/*
	 * Add the time since the begining of the base year, as
	 * reflected by (todr-TODRZERO/UNITSPERSEC), back into
	 * tmp_tv_sec.
	 */
	todr = ((todr-TODRZERO)/UNITSPERSEC);
	tmp_tv_sec += todr;

	/*
	 * Check that result will be in positive (int) range.
	 */
	if ((tmp_tv_sec != (int)tmp_tv_sec) ||
	    (tmp_tv_sec < 0))
	{
		printf("WARNING: lost battery backup clock");
		time.tv_sec = base;
		/*
		 * Believe the time in the file system for lack of
		 * anything better, resetting the TODR.
		 */
		resettodr();
#ifdef	notdef
		savetime = time.tv_sec;
#endif
		printf(" -- CHECK AND RESET THE DATE!\n");
		return;
	}

	/*
	 * Set the system time. Seconds since the epoch.
	 */
	time.tv_sec = (int)tmp_tv_sec;

	/*
	 * See if we gained/lost two or more days;
	 * if so, assume something is amiss.
	 */
	deltat = time.tv_sec - base;
	if (deltat < 0)
		deltat = -deltat;
	if (deltat >= 2*SECDAY) {
		printf("WARNING: clock %s %d days\n",
		    time.tv_sec < base ? "lost" : "gained", deltat / SECDAY);
	}
	resettodr();
#ifdef	notdef
	savetime = time.tv_sec;
#endif
}

/*
 * Reset the TODR based on the time value. This is used when
 * the TODR has a preposterous value or when the time is reset
 * by the stime system call.  Also called when the TODR goes past
 * TODRZERO + 10,000,000*(SECYEAR+2*SECDAY) (e.g. on Jan 2 just
 * after midnight) to wrap the TODR around.  The number 10,000,000 comes
 * from the number of clock ticks per second (at 100ns per tick).
 */
resettodr()
{
	int year = YRREF;
	u_long secyr;
	u_long yrtime;
	int s;
	struct tm conversion_tm;

	s = splclock();
	yrtime = time.tv_sec;
	splx(s);

	/*
	 * convert time.tv_sec to tm structure to set offset into year
	 * without regard to year. Do this using same convertion
	 * routines as current platform's BB_WATCH support routines.
	 */
	cvt_sec_to_tm(yrtime,&conversion_tm);
	year = conversion_tm.tm_year;
	conversion_tm.tm_year = 70;
	yrtime = cvt_tm_to_sec(&conversion_tm);

	/* need to account for changing a leap year to a non-leap year */
	if (LEAPYEAR(year) && (conversion_tm.tm_mon >= 3))
		yrtime += SECDAY;

	write_todclk(yrtime);
}

/*
 * Return the best possible estimate of the time in the timeval
 * to which tvp points.
 * We must compensate for wraparound which is not yet reflected in the time
 * (which happens when the counter hits 0 and wraps after the splhigh(),
 * but before the counter latch command).  Also check that this time is
 * no less than any previously-reported time, which could happen around
 * the time of a clock adjustment.  Just for fun, we guarantee that
 * the time will be greater than the value obtained by a previous call.
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

microtime(tvp)
	register struct timeval *tvp;
{
	static struct timeval lasttime;
	int s;

	s = splclock();
	tvp -> tv_sec = time.tv_sec;
	tvp -> tv_usec = time.tv_usec + (1000000/hz);
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

/*
 * cvt_tm_to_sec
 *
 * This routine converts the values in a tm structure to the number of seconds
 * since the epoch (1-JAN-1970 00:00:00).  It is the inverse of the gmtime
 * functionality.  No sanity checks are performed (GIGO).
 *
 * Only some of the tm structure fields are used to calculate the time:
 *
 *  tm->tm_year		The number of years since 1900 (for example 1991 would
 *			be represented by tm->tm_year == 91, and 2001 would be
 *			represented by tm->tm_year == 101).
 *  tm->tm_mon		The index of the month within the year, base 0.  For
 *			example January is 0, June 5, and December 11.
 *  tm->tm_mday		Day of the month (1-31).
 *  tm->tm_hour		Hour (0-23)
 *  tm->tm_min		Minute (0-59)
 *  tm->tm_sec		Second (0-59)
 *
 * This routine returns the time since the epoch in seconds.  The caller should
 * convert to appropriate units.
 *
 * This is a kernel routine, and expects the values in the tm structure to be
 * valid.
 *
 * The caller is responsible for mapping the month and year value to the
 * values expected in a tm structure.  For example, the MC146818 (Dallas 1287)
 * watch chip stores months base 1 and years in the range 00-99.  To use this
 * routine to get seconds since the epoch, the caller needs to subtract 1 from
 * the month value returned by the watch chip, and map the year value
 * appropriately (for instance if (year < 70) year += 100).  Also, the watch
 * must be in 24 hour mode, or the caller must convert the hours from 12-hour
 * format to binary format.  (The same goes for binary vs. BCD mode).
 *
 * The tables defined below are used to calculate the number of days in whole
 * months in the current year, minus 1 (to save subtracting 1 from the current
 * day number).  Months are base 0.
 */

static int normal_days[13] = {
	/* JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC */
	    -1,  30,  58,  89, 119, 150, 180, 211, 242, 272, 303, 333 };
static int leap_days[13] = {
	/* JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC */
            -1,  30,  59,  90, 120, 151, 181, 212, 243, 273, 304, 334 };

long cvt_tm_to_sec(tm)
	struct tm *tm;
{
	register long full_years, months, days, hours, minutes, seconds;

	/* The year calculation is good until 2100, because 2000 is a leap
	 * year.  This lets us use the divisible-by-four rule for determining
	 * number of leap years, without concern for the divisible-by-100 and
	 * divisible-by-400 exceptions.  When we're all dead someone will have
	 * to soup this up.
	 *
	 * The year computation was simplified, taking into the relationship
	 * of the epoch to the current year, and the epoch to the first leap
	 * year after the epoch.  Note that YRREF - 1900 = 70, and 
	 * ((YRREF - 1) % 4) = 1.  These values make days come out correct --
	 * I'm not sure whether they'll work right if YRREF changes (I think
	 * they will).
	 */
	full_years = tm->tm_year - (YRREF - 1900);
	days = full_years * 365 + (full_years + ((YRREF - 1) % 4)) / 4;

	/*
	 * Compute the number of days within the current year.  This is table
	 * driven.  To reduce computation, different tables are used for leap-
	 * and non-leap- years.  The tables hold the sum of all days in
	 * preceding months, with one day subtracted.  This eliminates the
	 * need to loop through the month table or subtract 1 from
	 * tm->tm_mday.
	 *
	 * This is the only case where an invalid reference could cause us
	 * grief.  * Out-of-range references get mapped here to avoid really
	 * strange errors.  * An assertion is also provided, but won't be
	 * there in the production * machine.  As long as this routine's
	 * complement is used to compute the * date, there should be no
	 * problem with out-of-range dates.
	 */
	months = tm->tm_mon;
#if MACH_ASSERT
	ASSERT((months >= 0) && (months <= 11));
#else /* MACH_ASSERT */
	if (months < 0) months = 0;
	if (months > 11) months = 11;
#endif /* MACH_ASSERT */
	days += ((tm->tm_year % 4 == 0) ? 
		 leap_days[months] : normal_days[months]);

	/*
	 * Adding days in month, hours, minutes, and seconds 
	 * is straightforward.
	 */
	days += tm->tm_mday;
	hours = days * 24 + tm->tm_hour;
	minutes = hours * 60 + tm->tm_min;
	seconds = minutes * 60 + tm->tm_sec;

	return (seconds);
}

/*
 * cvt_sec_to_tm
 *
 * This routine does the same thing as gmtime, without using a static buffer.
 *
 * The caller must adjust the tm values for the device which is being used.
 * For example, to use the MC146818 or Dallas 1287, it is necessary to add 1
 * to the month and map years > 99 to 00 (works until 2100).  Note that the
 * current Dallas 1287 chip maps year 00 to a leap year, so it works for the
 * year 2000.
 *
 * This routine and cvt_tm_to_sec have been tested for dates from the
 * epoch (00:00 Jan 1 1970) through 2010.  Used together, they provide a
 * valid 1:1 mapping between calendar-like dates and seconds since the epoch.
 * The dates generated are also consistent with those generated by the ULTRIX
 * 4.2 gtime() routine, so there is a good chance that all dates generated are
 * valid, as well as calendar-like.
 *
 * The tables below represent the number of seconds in cumulative preceding
 * months, for normal years and leap years.  They are used to determine the
 * month number to generate.  The month number ranges from 0 (JAN) through
 * 11 (DEC), NOT 1-12.
 */

static int normal_day_secs[13] = {
	0L * SECJAN + 0L * SECJUN,			/* JAN */
	1L * SECJAN + 0L * SECJUN,			/* FEB */
	1L * SECJAN + 0L * SECJUN + SECFEB, 		/* MAR */
	2L * SECJAN + 0L * SECJUN + SECFEB,		/* APR */
	2L * SECJAN + 1L * SECJUN + SECFEB,		/* MAY */
	3L * SECJAN + 1L * SECJUN + SECFEB,		/* JUN */
	3L * SECJAN + 2L * SECJUN + SECFEB,		/* JUL */
	4L * SECJAN + 2L * SECJUN + SECFEB,		/* AUG */
	5L * SECJAN + 2L * SECJUN + SECFEB,		/* SEP */
	5L * SECJAN + 3L * SECJUN + SECFEB,		/* OCT */
	6L * SECJAN + 3L * SECJUN + SECFEB,		/* NOV */
	6L * SECJAN + 4L * SECJUN + SECFEB,		/* DEC */
	INT_MAX };

static int leap_day_secs[13] = {
	0L * SECJAN + 0L * SECJUN,			/* JAN */
	1L * SECJAN + 0L * SECJUN,			/* FEB */
	1L * SECJAN + 0L * SECJUN + SECLFEB,		/* MAR */
	2L * SECJAN + 0L * SECJUN + SECLFEB,		/* APR */
	2L * SECJAN + 1L * SECJUN + SECLFEB,		/* MAY */
	3L * SECJAN + 1L * SECJUN + SECLFEB,		/* JUN */
	3L * SECJAN + 2L * SECJUN + SECLFEB,		/* JUL */
	4L * SECJAN + 2L * SECJUN + SECLFEB,		/* AUG */
	5L * SECJAN + 2L * SECJUN + SECLFEB,		/* SEP */
	5L * SECJAN + 3L * SECJUN + SECLFEB,		/* OCT */
	6L * SECJAN + 3L * SECJUN + SECLFEB,		/* NOV */
	6L * SECJAN + 4L * SECJUN + SECLFEB,		/* DEC */
	INT_MAX };

int cvt_sec_to_tm(time,tm)
	long time;
	struct tm *tm;
{
	/*
	 * This is designed for 64-bit longs.  If longs are only 32 bits, this
	 * code will break around 2038.  Changing to unsigned long would help,
	 * but makes testing harder since invalid negative values can't be
	 * tested by the DEBUG code.
	 */
	register long seconds = time;
	register long quads, years, months, days, hours, mins;
	register int *mp;
#if DEBUG
	static int last_time_seconds = 0;
	static int last_time_months = 0;
	static int last_months = 0;
#endif /* DEBUG */

	/*
	 * There's no such thing as a leap week, so calculate this up front.
	 * The epoch was on a Thursday.  Jan 1, 1900 was on a Monday.  This
	 * will not be correct if YRREF changes!
	 */
	ASSERT(YRREF == 1970);
	tm->tm_wday = (seconds / SECDAY + 4) % 7;
  
	/*
	 * Bias the time so that leapyears occur at every fourth year.  Count
	 * number of full four year periods, and subtract them out.  Then
	 * count number of full years left.  Dates near the end of a leap year
	 * need special treatment, so that an extra year is not added.  Once
	 * years are calculated, need to subtract off the initial bias.
	 * Establish the pointer to the correct month seconds table while
	 * we're at it.
	 */
	seconds += SECYR * ((YRREF - 1) % 4);
	quads = seconds / SECQYR;
	seconds -= quads * SECQYR;
	years = seconds / SECYR;
	if (years == 4) years = 3;		/* Dec. 31 of leap year */
	mp = (years == 3) ? leap_day_secs : normal_day_secs;
	seconds -= years * SECYR;
	years += quads * 4 - ((YRREF - 1) % 4) + (YRREF - 1900);
	tm->tm_year = years;
#if DEBUG
	if (seconds < 0) {
		if (time != last_time_seconds + 1) {
			printf("error: seconds < 0 (%d)\n", time);
		}
		last_time_seconds = time;
	}
	else {
		if (last_time_seconds) {
			printf("ended: seconds < 0 (%d)\n", time);
			last_time_seconds = 0;
		}
	}
#endif /* DEBUG */

	/*
	 * Now that full years have been subtracted, can get day of year.
	 */
	tm->tm_yday = seconds / SECDAY;

	/*
	 * Guess the month by dividing by number of seconds in a long month.
	 * Then verify against the actual month seconds table.  The table has
	 * a large positive value at the end, to prevent running past the last
	 * entry.
	 */
	months = seconds / SECJAN;
	mp += months;
	while (*mp <= seconds) { mp++; months++; };
	mp--; months--;
#if DEBUG
	if ((months < 0) || (months > 11)) {
		if ((time != last_time_months + 1) || (months != last_months)) {
			printf("error: months = %d (%d)\n", months, time);
			last_months = months;
		}
		last_time_months = time;
	}
	else {
		if (last_time_months) {
			printf("ended: months = %d (%d)\n", last_months, time);
		}
	}
#endif /* DEBUG */
	seconds -= *mp;
	tm->tm_mon = months;

	/*
	 * The rest is straightforward.
	 */
	days = seconds / SECDAY;
	seconds -= days * SECDAY;
	tm->tm_mday = days + 1;
	hours = seconds / SECHOUR;
	seconds -= hours * SECHOUR;
	tm->tm_hour = hours;
	mins = seconds / SECMIN;
	seconds -= mins * SECMIN;
	tm->tm_min = mins;
	tm->tm_sec = seconds;

	return 0;
}

/*
 * tm_invalid
 *
 * Simple sanity check for times in the tm struct.  This does not check for
 * things like the wrong day of the week or day in year, or days that don't
 * exist (like April 31 or Feb 29 of a non-leap year).  It is meant to catch
 * obvious garbage only.
 *
 * Note that a year greater than 199 (2099) is treated as invalid, since the
 * time conversion routines above will break in 2100.
 */
boolean_t tm_invalid(tm)
	struct tm *tm;
{
	return ((tm->tm_sec > 59)	|| (tm->tm_sec < 0)
		|| (tm->tm_min > 59)    || (tm->tm_min < 0)
		|| (tm->tm_hour > 23)   || (tm->tm_hour < 0)
		|| (tm->tm_mday > 31)   || (tm->tm_mday < 0)
		|| (tm->tm_mon > 11)    || (tm->tm_mon < 0)
		|| (tm->tm_year > 199)  || (tm->tm_year < 0));
}
