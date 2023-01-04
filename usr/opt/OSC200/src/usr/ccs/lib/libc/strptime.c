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
static char *rcsid = "@(#)$RCSfile: strptime.c,v $ $Revision: 1.1.6.4 $ (DEC) $Date: 1993/10/05 21:03:21 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: LIBCFMT
 *
 * FUNCTIONS:  strptime
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * 1.3  com/lib/c/fmt/strptime.c, libcfmt, 9130320 7/17/91 15:23:44
 * 1.8  com/lib/c/fmt/__strptime_std.c, libcfmt,9140320 9/26/91 14:00:15
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak strptime = __strptime
#endif
#include <sys/localedef.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <langinfo.h>
#include <stdio.h>
#include <stdlib.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex	_ctime_rmutex;	/* guard tzname[] access */
#endif	/* _THREAD_SAFE */

/*
 * FUNCTION: strptime() is a method driven functions where the time formatting
 *	     processes are done in the method points by 
 *	     __lc_time->core.strptime.
 *           It parse the input buffer according to the format string. If
 *           time related data are recgonized, updates the tm time structure
 *           accordingly.
 *
 * PARAMETERS:
 *           const char *buf - the input data buffer to be parsed for any
 *                             time related information.
 *           const char *fmt - the format string which specifies the expected
 *                             format to be input from the input buf.
 *           struct tm *tm   - the time structure to be filled when appropriate
 *                             time related information is found.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - if successful, it returns the pointer to the character after
 *             the last parsed character in the input buf string.
 *           - if fail for any reason, it returns a NULL pointer.
 */
char *__strptime_std(const char *, const char *, struct tm *, _LC_time_t *);

char *
strptime(const char *buf, const char *fmt, struct tm *tm)
{
	if (METHOD(__lc_time,strptime) == NULL)
		return __strptime_std( buf,fmt,tm, __lc_time);
	else
		return METHOD(__lc_time,strptime)(buf,fmt,tm, __lc_time);
}


#define MONTH		12
#define DAY_MON		31
#define HOUR_24		23
#define HOUR_12		11
#define DAY_YR		366
#define MINUTE		59
#define SECOND		61
#define WEEK_YR		53
#define DAY_WK		6
#define YEAR_99		99
#define YEAR_1900	1900
#define YEAR_9999	9999

#define BUF_SIZE	1000	/* the buffer size of the working buffer */

#define SKIP_TO_NWHITE(s)	while (*s && (isspace(*s))) s++

#define GETSTR(f)	t=f; \
			while(*subera && *subera != ':') \
				*t++ = *subera++; \
			*t = '\0'

#define FLDTONUM(str,size,num)	_zero_cnt = 0; \
	                        while(*str == '0') {\
					               *str++; \
						       _zero_cnt++; \
						    }\
                                num = -1;\
				if (isdigit(*str)) {\
					num = *str++ - '0'; _cnt = size-1;\
	  				while (_cnt-- && isdigit(*str)) {\
					        num *=10;\
						num += *str++ - '0';\
					}\
				}\
				else\
					if (_zero_cnt >= size) num = 0;

#define STRTONUM(str,num)	num = 0; \
				while (isdigit (*str)) { \
					num *= 10; \
					num += *str++ - '0'; \
				}
/*
 * dysize(A) -- calculates the number of days in a year.  The year must be
 *      the current year minus 1900 (i.e. 1990 - 1900 = 90, so 'A' should
 *      be 90).
 */
#define dysize(A)	((((1900+(A)) % 4 == 0 && (1900+(A)) % 100 != 0) \
			|| (1900+(A)) % 400 == 0) ? 366:365)

struct era_struct {
	char	dir;		/* dircetion of the current era */
	int	offset;		/* offset of the current era */ 
	char	st_date[100];	/* start date of the current era */
	char	end_date[100];	/* end date of the current era */	
	char	name[100];	/* name of the current era */
	char	form[100];	/* format string of the current era */
};
typedef struct era_struct *era_ptr;

static int   day_year[]  = {0,31,59,90,120,151,181,212,243,273,304,334,365};
static int   day_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
static char *cur_era[2]  = { NULL, NULL } ;

/*
 * Global data for internal routines.
 * Use structure and pass by reference to make code thread safe.
 */
typedef struct strptime_data {
	int set_yr;	/* logical flag to see if year is detected */
	int set_mon;	/* logical flag to see if month detected */
	int set_day;	/* logical flag to see if day is detected */
	int set_week;   /* logical flag to see if week of year is detected */
	int am;		/* logical flag to show if its AM */
	int pm;		/* logical flag to show if its PM */
	int set_hour;	/* logical flag for setting the hour of tm */
	int era_name;	/* logical flag for detected era name */
	int era_year;	/* logical flag for detected era year */
	int week_of_year; /* contains the week of the year %U */
} strptime_data_t;

/*
 * Retain simple names.
 */
#define set_yr		(strptime_data->set_yr)
#define set_mon		(strptime_data->set_mon)
#define set_day		(strptime_data->set_day)
#define set_week	(strptime_data->set_week)
#define am		(strptime_data->am)
#define pm		(strptime_data->pm)
#define set_hour	(strptime_data->set_hour)
#define era_name	(strptime_data->era_name)
#define era_year	(strptime_data->era_year)
#define week_of_year    (strptime_data->week_of_year)

static char *
strptime_recurse(const char *, const char *, struct tm *, _LC_time_t *,strptime_data_t *);

/*
 * FUNCTION: set_day_of_year (struct tm *tm)
 *	If the month, day, and year have been determine. It should be able
 * 	to calculate the day-of-year field tm->tm_yday of the tm structure.
 *	It calculates if its leap year by calling the dysize() which 
 *	returns 366 for leap year.
 *
 * PARAMETERS:
 *           struct tm *tm - a pointer to the time structure where the
 *			     tm->tm_yday field will be set.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	void.
 */
static void
set_day_of_year(struct tm *tm, strptime_data_t *strptime_data)
{
	if (set_day && set_mon && set_yr) {
		if ((dysize(tm->tm_year) == 366) && (tm->tm_mon >= 2) ) 
			tm->tm_yday = day_year[tm->tm_mon] + tm->tm_mday;
		else 
			tm->tm_yday = day_year[tm->tm_mon] + tm->tm_mday - 1;
		set_yr = 0;
	}
}

/*
 * FUNCTION: set_month_of_year (struct tm *tm)
 *	If the day, the week of the year, and year have been determined,
 *      we should calculate the month-of-year (field tm->tm_mon)
 *      of the tm structure.  We also need to fill in the tm_yday field.
 *
 *	Calculate its leap year by calling the dysize() which 
 *	returns 366 for leap year.
 *
 * PARAMETERS:
 *           struct tm *tm - a pointer to the time structure where the
 *			     tm->tm_mon field will be set.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	void.
 */
static void
set_month_of_year(struct tm *tm, strptime_data_t *strptime_data, int monday_first_flg)
{

	int i, delta;
	struct tm JAN_1_tm;
	time_t t;

	if (set_day && set_week && set_yr) {

	    /* first calculate the day of the year if necessary.
	       To do this, we need to know the day of the week of 
	       the first day of the year.
	     */
	    JAN_1_tm.tm_sec = 0;
	    JAN_1_tm.tm_min = 0;
	    JAN_1_tm.tm_hour = 0;
	    JAN_1_tm.tm_mday = 1;
	    JAN_1_tm.tm_mon = 0;
	    JAN_1_tm.tm_year = tm->tm_year;
	    JAN_1_tm.tm_wday = -1;
	    JAN_1_tm.tm_yday = -1;
	    JAN_1_tm.tm_isdst = -1;

	    t = mktime(&JAN_1_tm);

	    /* get the difference between the day of the week for Jan 1
	       and the day of the week specified.
	    */
	    delta = tm->tm_wday - JAN_1_tm.tm_wday;

	    tm->tm_yday = ((week_of_year)*7) + delta;

	    /* if we are dealing with Monday as the first day  of
	       the week (i.e. %W), this means that everything before
	       the first Monday is in week 0.
	       If the week of the day we are looking for is 0, then
	       we need to account for the fact that it will be the last
	       day of the week instead of the first (must add a week).
	     */
	    if (monday_first_flg) {
	        if (tm->tm_wday == 0)
		    tm->tm_yday = ((week_of_year+1)*7) + delta;
	    }

	    tm->tm_mon = 0;

	    i = 0;

	    /* if we are dealing with a leap year, we may need
	       to adjust the day of year if the day is past February */
	    if ((tm->tm_yday > day_year[2]) && 
		(dysize(tm->tm_year) == 366)) {
		while (tm->tm_yday > (day_year[i+1]+1))
		       i++;
	        tm->tm_mday = (tm->tm_yday - (day_year[i]+1)) + 1;
	        }
	    else {
	        while (tm->tm_yday > day_year[i+1])
			i++;
	        tm->tm_mday = (tm->tm_yday - day_year[i]) + 1;
	        }

	    tm->tm_mon = i;
	}
}


/*
 * FUNCTION: conv_time (era_ptr era, int year, struct tm *tm)
 *	     By supplying the current era and year of the era, the function
 *	     converts this era/year combination into Christian ear and 
 *	     set the tm->tm_year field of the tm time structure.
 *
 * PARAMETERS:
 *           era_ptr era - the era structure provides the related information
 *			   of the current era.
 *           int year - year of the specific era.
 *           struct tm *tm - a pointer to the time structure where the
 *			     tm->tm_year field will be set.
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns 1 if the conversion is valid and successful. 
 *           - returns 0 if fails.
 */
static int
conv_time(era_ptr era, int year, struct tm *tm, strptime_data_t *strptime_data)
{
	char *str;
	int start_year = 0;
	int end_year = 0;
	int no_limit = 0;
	int i;

	str = era->st_date;
	if (*str == '-') {
		str++;
		STRTONUM(str, start_year);
		start_year = -start_year + 1;
	}
	else
		STRTONUM(str, start_year);

	str = era->end_date;
	if ((*str=='+' && *(str+1)=='*' ) || (*str=='-' && *(str+1)=='*'))
		no_limit = 1;
	else if (*str == '-') {
		str++;
		STRTONUM(str, end_year);
		end_year = -end_year + 1;
	}
	else
		STRTONUM(str, end_year);

	if (era->dir == '+') {
		i = year - era->offset + start_year;
		if (no_limit){
			tm->tm_year = i - YEAR_1900;
			set_yr = 1;
			set_day_of_year(tm, strptime_data);
			return (1);
		}
		if (i <= end_year) {
			tm->tm_year = i - YEAR_1900;
			set_yr = 1;
			set_day_of_year(tm, strptime_data);
			return (1);
		}
		return (0);
	}
	else {
		if ((i = end_year - year) <= start_year) {
			tm->tm_year = i - YEAR_1900;
			set_yr = 1;
			set_day_of_year(tm, strptime_data);
			return (1);
		}
		return (0);
	}
}

static char *
parse_alternate(const char *buf, const char **fmt, struct tm *tm,
		_LC_time_t *hdl, strptime_data_t *strptime_data )
{
    char dir[8];
    int	syr,smon,sday;
    char etime[64];
    int	off,eoff;
    int _cnt;			/* Temp for FLDTONUM macro */
    int _zero_cnt;              /* Temp for FLDTONUM macro */
    char name[64];
    char *newbuf;
    char **era_list = hdl->era;
    char altfmt[64];
    char *efmt = altfmt;

    SKIP_TO_NWHITE(buf);

    if ( !hdl->era || !*hdl->era) /* Are there ERAs in this locale? */
	return (NULL);		/* Nope */

	
    era_list = (cur_era[0] != NULL) ? cur_era : hdl->era ;
    for( ; *era_list; era_list++) {
	int nmatch;
	int fmtoff;

	nmatch = sscanf(*era_list,
			"%1[-+]:%u:%d/%d/%d:%63[^:]:%63[^:]:%n",
			dir,
			&eoff,
			&syr, &smon, &sday,
			etime,		/* end-time */
			name,		/* era-name (%EC) */
			&fmtoff); 	/* Offset where era-format begins (%EY) */

	if (nmatch != 7)
	    continue;	/* Malformated era, ignore it */
	
	efmt = *era_list + fmtoff;
	if (syr < 0)
		syr++ ;	/* Adjust for negative starting year */

	switch (**fmt) {
	  case 'c':		/* Alternative date time */
	    efmt = nl_langinfo(ERA_D_T_FMT);
	    goto recurse;

	  case 'Y':
recurse:
	    if (era_list != cur_era)
		cur_era[0] = era_list[0] ;
	    newbuf = strptime_recurse(buf, efmt, tm, hdl, strptime_data);
	    if (era_list != cur_era)
		cur_era[0] = NULL ;	/* Reset cur_era */

	    if (!newbuf)
		continue;
	    buf = newbuf;
	    goto matched;
	    
	  case 'C':		/* Base year */
	    {
		size_t len = strlen(name);
		if (strncmp(buf,name,len))
		    continue;
		
		buf	   += len ;
		tm->tm_year = eoff;
		goto matched	  ;
	    }
	    
	  case 'x':		/* Alternative date representation */
	    efmt = nl_langinfo(ERA_D_FMT);
	    goto recurse;
	    
	  case 'X':		/* Alternative time format */
	    efmt = nl_langinfo(ERA_T_FMT);
	    goto recurse;
	    
	  case 'y':		/* offset from %EC(year only) */
	    off = 0;
	    FLDTONUM(buf,8,off);
	    tm->tm_year = off;
	    goto matched;

	  default:	return (NULL);
	}
	
    } /* end for */

    return (NULL);		/* Fell thru for-loop */

matched:
    /*
     * Here only when matched on appropriate era construct
     */


     switch (*(*fmt)++) {
       case 'c':	break;	/* recursion filled in struct tm */

       case 'C':	tm->tm_year += syr - eoff - 1900;
			set_yr = 1 ;
			set_day_of_year(tm, strptime_data);
			break;

       case 'x':	break;
       case 'X':	break;

       case 'Y':	break;

       case 'y':	tm->tm_year += syr - eoff - 1900;
			set_yr = 1 ;
			set_day_of_year(tm, strptime_data);
			break;
     }
    return ((char *)buf);
}

/*
 * FUNCTION: This the standard method for function strptime.
 *	     It parses the input buffer according to the format string. If
 *	     time related data are recgonized, updates the tm time structure
 * 	     accordingly. 
 *
 * PARAMETERS:
 *           _LC_time_t *hdl - pointer to the handle of the LC_TIME
 *			       catagory which contains all the time related
 *			       information of the specific locale.
 *	     const char *buf - the input data buffer to be parsed for any
 *			       time related information.
 *	     const char *fmt - the format string which specifies the expected
 *			       format to be input from the input buf.
 *	     struct tm *tm   - the time structure to be filled when appropriate
 *			       time related information is found.
 *			       The fields of tm structure are:
 *
 *			       int 	tm_sec		seconds [0,61]
 *			       int	tm_min		minutes [0,61]
 *			       int	tm_hour		hour [0,23]
 *			       int	tm_mday		day of month [1,31]
 *			       int	tm_mon		month of year [0,11]
 *			       int	tm_wday		day of week [0,6] Sun=0
 *			       int	tm_yday		day of year [0,365]
 *			       int 	tm_isdst	daylight saving flag
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - if successful, it returns the pointer to the character after
 *	       the last parsed character in the input buf string.
 *           - if fail for any reason, it returns a NULL pointer. 
 */
static char *
strptime_recurse(const char *buf, const char *fmt, struct tm *tm,
		 _LC_time_t *hdl, strptime_data_t *strptime_data)
{
	char	bufchr;		/* current char in buf string */
	char	fmtchr;		/* current char in fmt string */
	int	found;		/* boolean flag for a match of buf and fmt */
	int	width;		/* the field width of an locale entry */ 
	int 	lwidth;		/* the field width of an locale entry */
	char 	*era_s;		/* locale's empiror/era string */

	struct era_struct eras; /* a structure for current era */
	era_ptr era = &eras;	/* pointer to the current era struct */
	int	year=0;		/* %o value, year in current era */

	int     Monday_first=1;   /* default to Sunday as first day of week */
	int	i;
	int	_cnt, _zero_cnt;/* Temporary used by FLDTONUM */
	char	**era_list;	/* points to current candidate ERA */
	char    *tz;
	
	SKIP_TO_NWHITE(fmt);
	while ((fmtchr = *fmt++) && (bufchr = *buf)) {
						/* stop when buf or fmt ends */
		if (fmtchr != '%') {
			SKIP_TO_NWHITE(buf);
			bufchr = *buf;
			if (bufchr == fmtchr) {
				buf++;
				SKIP_TO_NWHITE(fmt);
				continue;	/* ordinary char, skip */
			}
			else
				return (NULL); 	/* error, ordinary char in fmt 
						   unmatch char in buf */
		}
		else {
			/*
			 * Perform white space skipping for all format code
			 * except %t and %n.
			 */
			if ((*fmt != 't') && (*fmt != 'n'))
				SKIP_TO_NWHITE(buf) ;

			switch (fmtchr = *fmt++) {
			case 'a':
			case 'A': 
			/* locale's full or abbreviate weekday name */
				found = 0;
				for (i=0; i < 7 && !found; i++) {
					width = strlen(hdl->abday[i]);
					lwidth = strlen(hdl->day[i]);
					if (!strncmp(buf, hdl->day[i], lwidth)){
						found = 1;
						buf += lwidth;
					}
					else
					if (!strncmp(buf, hdl->abday[i],width)){
						found = 1;
						buf += width;
					}
				}
				if (found)
					tm->tm_wday = i-1;
				else
					return (NULL);
				break;

			case 'b': 
			case 'B': 
			case 'h':
			/* locale's full or abbreviate month name */
				found = 0;
				for (i=0; i < 12 && !found; i++) {
					width = strlen(hdl->abmon[i]);
					lwidth = strlen(hdl->mon[i]);
					if (!strncmp(buf, hdl->mon[i], lwidth)){
						found = 1;
						buf += lwidth;
					}
					else
					if (!strncmp(buf, hdl->abmon[i],width)){
						found = 1;
						buf += width;
					}
				}
				if (found) {
					tm->tm_mon = i-1;
					set_mon = 1;
					set_day_of_year(tm, strptime_data);
				}
				else
					return (NULL);
				break;

			case 'c': 		/* locale's date and time */
				if ((buf = strptime_recurse(buf, hdl->d_t_fmt, 
							    tm, hdl,
							    strptime_data))
				    == NULL)
					return (NULL);
				break;

			case 'd':		/* day of month, 1-31 */
			case 'e':
				FLDTONUM(buf,2,i);
				if (i > 0 && i <= DAY_MON) {
					tm->tm_mday = i;
					set_day = 1;
					set_day_of_year(tm, strptime_data);
				}
				else
					return (NULL);
				break;

			case 'D':		/* %m/%d/%y */
				if ((buf = strptime_recurse(buf, "%m/%d/%y", tm,
							    hdl, strptime_data))
				    == NULL)
					return (NULL);
				break;

			case 'E':
				buf = parse_alternate(buf,&fmt,tm,hdl,strptime_data);
				if (buf == NULL)
				    return(NULL);
				break;

			case 'H':		/* hour 0-23 */
				FLDTONUM(buf,2,i);
				if (i >= 0 && i <= HOUR_24) {
					if (am) {
						if (i > HOUR_12)
							return (NULL);
						am = 0;
					}
					else if (pm) {
						if (i <= HOUR_12)
							i += 12;
						pm = 0;
					}
					else
						set_hour = 1;
					tm->tm_hour = i;
				}
				else
					return (NULL);
				break;

			case 'I':		/* hour 1-12 */
				FLDTONUM(buf,2,i);
				if (i > 0 && i <= HOUR_12 + 1) {
					if (am) 
						am = 0;
					else if (pm) {
						i += 12;
						pm = 0;
					}
					else
						set_hour = 1;
					tm->tm_hour = i;
				}
				else
					return (NULL);
				break;

			case 'j':		/* day of year, 1-366 */
				FLDTONUM(buf,3,i);
				if (i > 0 && i <= DAY_YR)
					tm->tm_yday = i - 1;
				else
					return (NULL);
				break;

			case 'm':		/* month of year, 1-12 */
				FLDTONUM(buf,2,i);
				if (i > 0 && i <= MONTH) {
					tm->tm_mon = i-1;
					set_mon = 1;
					set_day_of_year(tm, strptime_data);
				}
				else
					return (NULL);
				break;

			case 'M':		/* minute 0-59 */
				FLDTONUM(buf,2,i);
				if (i >= 0 && i <= MINUTE)
					tm->tm_min = i;
				else
					return (NULL);
				break;

			case 'N':
				if ( !hdl->era || !*hdl->era)
				    return (NULL);

				era_list = (cur_era[0] != NULL)
					 ? cur_era : hdl->era ;

				era_name = 0; 		/* No match yet  */
				
				for (;(era_s = *era_list) != NULL; era_list++) {
				    int nmatch;
				    char dirbuf[2];
				    int i;
				    
 				    nmatch = sscanf(era_s,
					    "%[+-]:%u:%99[^:]:%99[^:]:%99[^:]:%n",
					    dirbuf,
					    &era->offset,
					    era->st_date,
					    era->end_date,
					    era->name,
					    &i);
				    
				    if (nmatch != 5) 	/* Bad era string */
					continue; 	/* Try next one */

				    era->dir = dirbuf[0];
				    strcpy(era->form, &era_s[i]);

				    /*
				     * Match era name against contents of buffer.
				     */
				    i = strlen(era->name);
				    if (!strncmp(buf, era->name,i)){
					buf += i;
					era_name = 1; 	/* Found a match */
					break; 	  	/* Stop looking for others */
				    }
				    
				} /* end-for */
			
				if (era_name) {
				    /*
				     * If era_year is not defined, it is always
				     * assumed to be the offset year from 
				     * which the era starts. This can handles
				     * those era years that have no %Ey in
				     * the era form.
				     */
				    if (era_year) {
					era_name = 0;
					era_year = 0;
				    } else
					year = era->offset ;
				    if (!conv_time(era, year, tm,
						   strptime_data))
					    return (NULL);
				} else
				    return (NULL);
				break;

			case 'n':		/* new line character */
				while (*buf && (isspace(*buf)) && *buf != '\n')
					 buf++;	/* skip all white pior to \n */
				if (*buf == '\n')
					buf++;
				else
					return (NULL);
				break;

			case 'o':		/* year of era */
				STRTONUM(buf, year);
				if (year >= 0) {
					era_year = 1;
					if (era_name) {
						era_year = 0;
						era_name = 0;
						if (!conv_time(era, year, tm,
							       strptime_data))
							return (NULL);
					}
				}
				break;

			case 'O': /* Field using alternate numeric symbols */
				/* Compare field against alternate digits */
			    {
#ifdef _THREAD_SAFE
#define STRTOK(a,b)	strtok_r((a),(b),savestate)
				char **savestate;
#else
#define STRTOK(a,b)	strtok((a),(b))
#endif
				int num=0;
				char *digs = strdup(hdl->alt_digits);
				char *cand = STRTOK(digs, ";");

				while (cand) {
				    if (strncmp(buf,cand,strlen(cand))) {
					/*
					 * Not a match with alternate representation
					 */
					cand = STRTOK(NULL,";");
					num++;
				    } else
					break;
				}

				if (!cand) return (NULL);

				/* Store resulting number in the right place */
				switch (*fmt++) {
				  case 'e':
				  case 'd':	tm->tm_mday = num;	break;
				  case 'H':	tm->tm_hour = num;	break;
				  case 'I':	tm->tm_hour = num;	break;
				  case 'm':	tm->tm_mon = num;	break;
				  case 'M':	tm->tm_min = num;	break;
				  case 'S':	tm->tm_sec = num;	break;
				  case 'U':	break;
				  case 'w':	tm->tm_wday = num;	break;
				  case 'W':	tm->tm_wday = (num+1)%7; break;
				  case 'y':	break;
				}
			    }
				
			case 'p':		/* locale's AM or PM */
				width = strlen(hdl->am_pm[0]);
				lwidth = strlen(hdl->am_pm[1]);
				if (!strncmp(buf, hdl->am_pm[0], width)) {
					if (set_hour) {
						if(tm->tm_hour > 0 &&
						   tm->tm_hour < 12)
							set_hour = 0;
						else
							return (NULL);
					}
					else
						am = 1;
					buf += width;
				}
				else if (!strncmp(buf, hdl->am_pm[1], lwidth)){
					if (set_hour) {
						if (tm->tm_hour > 0
						    && tm->tm_hour < 12) {
							tm->tm_hour += 12;
							set_hour = 0;
						}	
					}
					else
						pm = 1;
					buf += lwidth;
				}
				else
					return (NULL);
				break;

			case 'R': 		/* %H:%M */
				buf = strptime_recurse(buf, "%H:%M", tm,
						       hdl, strptime_data);
				if (buf == NULL)
					return (NULL);
				break;
				
			case 'r':		/* %I:%M:%S [AM|PM] */
				if ((buf = strptime_recurse(buf, "%I:%M:%S ",
							    tm, hdl,
							    strptime_data))
				    == NULL)
					return (NULL);

				/*
				 * Recursively invoke to get optional AM|PM
				 * and ignore if it doesn't appear.
				 */
				{
				    const char *p;

				    p = strptime_recurse(buf, "%p", tm, hdl,
							 strptime_data);

				    buf = (p)?p:buf;
				}

				break;

			case 'S':		/* second 0-61 */
				FLDTONUM(buf,2,i);
				if (i >= 0 && i <= SECOND)
					tm->tm_sec = i;
				else
					return (NULL);
				break;

			case 't':		/* tab character */
				while (*buf && (isspace(*buf)) && *buf != '\t')
					buf++;	/* skip all white prior to \t */
				if (*buf == '\t')
					buf++;
				else
					return (NULL);
				break;

			case 'T':		/* %H:%M:%S */
				if ((buf = strptime_recurse(buf, "%H:%M:%S", tm,
							    hdl, strptime_data))
				    == NULL)
					return (NULL);
				break;

			case 'U': 
				Monday_first = 0;
			case 'W':		/* week of year, 0-53 */
				FLDTONUM(buf,2,i);
				if (i >= 0 && i <= WEEK_YR) {
					week_of_year = i;
					set_week = 1;
					set_month_of_year(tm, strptime_data, 
							  Monday_first);
					set_day_of_year(tm, strptime_data);
				}
				else 
					return (NULL);
				break;

			case 'w':		/* day of week, 0-6 */
				FLDTONUM(buf,1,i);
				if (i >= 0 && i <= DAY_WK) {
					set_day = 1;
					tm->tm_wday = i;
				}
				else
					return (NULL);
				break;

			case 'x':		/* locale's date format */
				if ((buf = strptime_recurse(buf, hdl->d_fmt, tm,
							    hdl, strptime_data))
				    == NULL)
					return (NULL);
				break;

			case 'X':		/* locale's time format */
				if ((buf = strptime_recurse(buf, hdl->t_fmt, tm,
							    hdl, strptime_data))
				    == NULL)
					return (NULL);
				break;

			case 'y':		/* year of century, 0-99 */
				FLDTONUM(buf,2,i);
				if (i >= 0 && i <= YEAR_99) {
					tm->tm_year = i;
					set_yr = 1;
					set_day_of_year(tm, strptime_data);
				}
				else 
					return (NULL);
				break;

			case 'Y':		/* year with century, dddd */
				FLDTONUM(buf,4,i);
				if (i <= YEAR_9999) {
					tm->tm_year = i-YEAR_1900;
					set_yr = 1;
					set_day_of_year(tm, strptime_data);
				}
				else
					return (NULL);
				break;
	
			case 'Z':		/* time zone name */
				/* tzset() call avoidance logic moved to tzset() */
				tzset();
				TS_LOCK(&_ctime_rmutex);
				width = strlen(tzname[0]);
				lwidth = strlen(tzname[1]);
				if (!strncmp(buf, tzname[0], width)) {
					tm->tm_isdst = 1;
					buf += width;
				}
				else if (!strncmp(buf, tzname[1], lwidth)) {
					tm->tm_isdst = 0;
					buf += lwidth;
				}
				else {
					TS_UNLOCK(&_ctime_rmutex);
					return (NULL);
				}
				TS_UNLOCK(&_ctime_rmutex);
				break;

			case '%' :		/* double % character */
				if (bufchr == '%')
					buf++;
				else
					return (NULL);
				break;

			default:
				return (NULL);
			} /* switch */
		} /* else */
		SKIP_TO_NWHITE(fmt);
	} /* while */
	if (fmtchr)
		return (NULL); 		/* buf string ends before fmt string */
	return ((char *)buf);		/* successful completion */
}


/*
 * This is a wrapper for the real function which is recursive.
 * The global data is encapsulated in a structure which is initialised
 * here and then passed by reference.
 */
char *
__strptime_std(const char *buf, const char *fmt, struct tm *tm, _LC_time_t *hdl)
{
	strptime_data_t real_strptime_data;
	strptime_data_t *strptime_data = &real_strptime_data;
	char		*rptr ;

	/* Initialiize "hidden" global/static var's */
	set_yr = 0;
	set_mon = 0;
	set_day = 0;
	set_week = 0;
	am = 0;
	pm = 0;
	set_hour = 0;
	era_name = 0;
	era_year = 0;
	week_of_year = 0;
	cur_era[0] = NULL ;

	/* Call the recursive, thread-safe routine */
	rptr = strptime_recurse( buf, fmt, tm, hdl, strptime_data);
	/*
	 * Check if day of month is set correctly
	 */
	if ((rptr != NULL) && ((unsigned)tm->tm_mon < MONTH) &&
	    (day_month[tm->tm_mon] < DAY_MON)) {
		if ((tm->tm_mon == 1) && (dysize(tm->tm_year) == 366) &&
		    (tm->tm_mday > day_month[tm->tm_mon] + 1))
		    rptr = NULL ;
		else if (tm->tm_mday > day_month[tm->tm_mon])
		    rptr = NULL ;
	}
	return(rptr) ;
}
