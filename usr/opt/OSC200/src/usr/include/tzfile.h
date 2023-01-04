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
 * @(#)$RCSfile: tzfile.h,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/12/09 15:06:56 $
 */
/************************************************************************
 *			Modification History				*
 ************************************************************************/
/*
 * 001	Jon Reeves, 1989-Dec-04
 *	Increased TZ_MAX_CHARS from 50 to 100 so X/Open tests would pass.
 */

#include <sys/syslimits.h>

/*
** Information about time zone files.
*/

#define	TZDIR		"/etc/zoneinfo" /* Time zone object file directory */
#define	TZDEFAULT	"localtime"

/*
** Each file begins with. . .
*/

struct tzhead {
	char	tzh_reserved[32];	/* reserved for future use */
	char	tzh_timecnt[4];		/* coded number of transition times */
	char	tzh_typecnt[4];		/* coded number of local time types */
	char	tzh_charcnt[4];		/* coded number of abbr. chars */
};

/*
** . . .followed by. . .
**
**	tzh_timecnt (char [4])s		coded transition times a la time(2)
**	tzh_timecnt (unsigned char)s	types of local time starting at above
**	tzh_typecnt repetitions of
**		one (char [4])		coded GMT offset in seconds
**		one (unsigned char)	used to set tm_isdt
**		one (unsigned char)	that's an abbreviation list index
**	tzh_charcnt (char)s		'\0'-terminated zone abbreviaton strings
*/

/*
** In the current implementation, "tzset()" refuses to deal with files that
** exceed any of the limits below.
*/

/*
** The TZ_MAX_TIMES value below is enough to handle a bit more than a
** year's worth of solar time (corrected daily to the nearest second) or
** 138 years of Pacific Presidential Election time
** (where there are three time zone transitions every fourth year).
*/
#define	TZ_MAX_TIMES	370

#define	NOSOLAR			/* We currently don't handle solar time */

#ifndef	NOSOLAR
#define	TZ_MAX_TYPES	256	/* Limited by what (unsigned char)'s can hold */
#else /* !NOSOLAR */
#define	TZ_MAX_TYPES	10	/* Maximum number of local time types */
#endif /* !NOSOLAR */

#define TZ_MAX_CHARS	TZNAME_MAX

#define	SECS_PER_MIN	60
#define	MINS_PER_HOUR	60
#define	HOURS_PER_DAY	24
#define	DAYS_PER_WEEK	7
#define	DAYS_PER_NYEAR	365
#define	DAYS_PER_LYEAR	366
#define	SECS_PER_HOUR	(SECS_PER_MIN * MINS_PER_HOUR)
#define	SECS_PER_DAY	((long) SECS_PER_HOUR * HOURS_PER_DAY)
#define	MONS_PER_YEAR	12

#define	TM_SUNDAY	0
#define	TM_MONDAY	1
#define	TM_TUESDAY	2
#define	TM_WEDNESDAY	3
#define	TM_THURSDAY	4
#define	TM_FRIDAY	5
#define	TM_SATURDAY	6

#define	TM_JANUARY	0
#define	TM_FEBRUARY	1
#define	TM_MARCH	2
#define	TM_APRIL	3
#define	TM_MAY		4
#define	TM_JUNE		5
#define	TM_JULY		6
#define	TM_AUGUST	7
#define	TM_SEPTEMBER	8
#define	TM_OCTOBER	9
#define	TM_NOVEMBER	10
#define	TM_DECEMBER	11
#define	TM_SUNDAY	0

#define	TM_YEAR_BASE	1900

#define	EPOCH_YEAR	1970
#define	EPOCH_WDAY	TM_THURSDAY

/*
** Accurate only for the past couple of centuries;
** that will probably do.
*/

#define	isleap(y) (((y) % 4) == 0 && ((y) % 100) != 0 || ((y) % 400) == 0)
