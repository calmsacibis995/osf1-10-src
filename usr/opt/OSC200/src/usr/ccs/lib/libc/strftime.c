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
static char	*sccsid = "@(#)$RCSfile: strftime.c,v $ $Revision: 4.2.10.5 $ (DEC) $Date: 1993/10/05 21:03:17 $";
#endif 
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS:  strftime
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1991
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 * 
 * 1.3  com/lib/c/fmt/strftime.c, libcfmt, 9130320 7/17/91 15:22:50
 * 1.7  com/lib/c/fmt/__strftime_std.c, libcfmt, 9140320 9/26/91 13:59:49
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak NLstrtime = __NLstrtime
#endif
#include <stddef.h>
#include <sys/localedef.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <langinfo.h>
#include <stdlib.h>

#include "ts_supp.h"

#ifdef _THREAD_SAFE
#include "rec_mutex.h"
extern struct rec_mutex	_ctime_rmutex;	/* guard tzname[] access */
#endif	/* _THREAD_SAFE */

#define BADFORMAT  do { format=fbad; \
			bufp = "%"; } while (0)

#define BUFSIZE		1000
#define WIDTH(a)	(wpflag ? 0  : (a))
#define PUT(c)		(strp < strend ? *strp++  = (c) : toolong++)
#define GETSTR(f)  do { t=f; 				 \
			while(*subera && *subera != ':') \
				*t++ = *subera++; 		 \
			*t = '\0';			 \
		   } while (0)

#define GETNUMBER(v) (sprintf(buffer, "%d", v), buffer)

#define STRTONUM(str,num)	do { num = 0; 		 \
				while (isdigit (*str)) { \
					num *= 10; 	 \
					num += *str++ - '0'; \
				} } while(0)

#define CHECKFMT(f,s) (((f) && *(f))? (f) : (s))	/* 'f' is non-null string */

/* codes used by doformat() */

#define NOYEAR	2
#define NOSECS	3
#define SKIP	for(strp=lastf; (i = *format) && i != '%'; format++ )

struct era_struct {
	char	dir;		/* direction of the current era */
	int	offset;		/* offset of the current era */
	char	st_date[100];   /* start date of the current era */
	char	end_date[100];  /* end date of the current era */
	char	name[100];	/* name of the current era */
	char	form[100];	/* format string of the current era */
};
typedef struct era_struct *era_ptr;


#ifdef _THREAD_SAFE

typedef struct strftime_data {
	char	*hg_bufp;
	char	hg_buffer[BUFSIZE];
	struct era_struct	hg_eras;
	era_ptr	hg_era;
	int	hg_era_name;
	int	hg_era_year;
	int	hg_altera;
	char	*hg_altnum;
} strftime_data_t;


#define bufp		(strftime_data->hg_bufp)
#define buffer		(strftime_data->hg_buffer)
#define eras		(strftime_data->hg_eras)
#define ERASTR		(strftime_data->hg_era)
#define era_name	(strftime_data->hg_era_name)
#define era_yr		(strftime_data->hg_era_year)
#define altera		(strftime_data->hg_altera)
#define altnum		(strftime_data->hg_altnum)

#define GETNUM(a,b)		getnum(a, b, strftime_data)
#define GETTIMEZONE(a)		gettimezone(a, strftime_data)
#define CONV_TIME(a,b,c,d)	conv_time(a, b, c, d, strftime_data)
#define DOFORMAT(a,b,c,d,e,f)	doformat(a, b, c, d, e, f, strftime_data)

#else

static char *bufp;
static char buffer[BUFSIZE];
static struct era_struct eras;	/* the structure for current era */
static era_ptr era = &eras;	/* pointer to the current era */
static int altera;		/* Non-zero iff era-based formatting */
static char *altnum;		/* Points to alternate numeric representation */

#define ERASTR		era

#define GETNUM		getnum
#define GETTIMEZONE	gettimezone
#define CONV_TIME	conv_time
#define DOFORMAT	doformat

#endif /* _THREAD_SAFE */

/*
 * FUNCTION: getwidth()
 *	     This function calculate the number of digits of a integer.
 *
 * PARAMETERS:
 *	     int i - an integer to be calculated.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     It returns the the number of digit of parameter i.
 */
static int getwidth(int i)
{
	int i_width = 1;

	i /= 10;
	for (;i > 0;i /= 10)
		i_width++;
	return(i_width);
}

/*
 * FUNCTION: getnum()
 *	     This function convert a integral numeric value i into
 *	     character string with a fixed field.
 *
 * PARAMETERS:
 *	     int i - an integral value.
 *	     int n - output field width.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     It returns the character string of the integral value.
 */
static char *
#ifdef _THREAD_SAFE
getnum(int i, int n, strftime_data_t *strftime_data)
#else
getnum(int i, int n)
#endif /* _THREAD_SAFE */
{
	char *s = buffer;

	s += (n ? n : 19);
	*s = 0;
	if (altnum) {
		char	*p;	/* Points to front of i-th string */
		char	*q;	/* Points to terminator of i-th string */

		p = altnum;
		/*
		 * Search thru semicolon-separated strings for i-th alternate
		 * value
		 */
		while (i) {
			q = strchr(p, ';');	/* Possible terminator */
			if (!q) {		/* Ran off end of string? */
				p = q = "";
				break;
			}
			p = q+1;
			i--;
		}

		q = strchr(p, ';');
		if (!q)
			q = p + strlen(p);
		while (q > p)
			*--s = *--q;

	} else {
		while (s > buffer) {
			if (i == 0 && n == 0) break;
			*--s = (i % 10) + '0';
			i /= 10;
		}
	}
	return s;
}


/*
 * FUNCTION: gettimezone()
 *	     This function returns the name of the current time zone.
 *
 * PARAMETERS:
 *	     struct tm *timeptr - time structure.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	     It returns the current time zone.
 */
static char *
#ifdef _THREAD_SAFE
gettimezone(struct tm *timeptr, strftime_data_t *strftime_data)
#else
gettimezone(struct tm *timeptr)
#endif /* _THREAD_SAFE */
{
	if (daylight && timeptr->tm_isdst)
		return tzname[1];
	return tzname[0];
}


/*
 * FUNCTION: conv_time()
 *	     This function converts the current Christian year into year
 * 	     of the appropriate era. The era chosen such that the current 
 *	     Chirstian year should fall between the start and end date of 
 * 	     the first matched era in the hdl->era string. All the era 
 *	     associated information of a matched era will be stored in the era 
 *	     structure and the era year will be stored in the year 
 *	     variable.
 *
 * PARAMETERS:
 *	   _LC_TIME_t *hdl - the handle of the pointer to the LC_TIME
 *			     catagory of the specific locale.
 *	   struct tm *timeptr - date to be printed
 *	     era_ptr era - pointer to the current era.
 *	     int *year - year of the current era.
 *
 * RETURN VALUE DESCRIPTIONS:
 *	   - returns 1 if the current Christian year fall into an
 *	       valid era of the locale.
 *	   - returns 0 if not.
 */
static int
#ifdef _THREAD_SAFE
conv_time(_LC_time_t *hdl, struct tm *tm, era_ptr era, int *year,
	  strftime_data_t *strftime_data)
#else
conv_time(_LC_time_t *hdl, struct tm *tm, era_ptr era, int *year)
#endif /* _THREAD_SAFE */
{
	char *subera;
	char erabuf[BUFSIZE];
	char *era_s;
	char *str;
	char *t;
	char dirstr[2];
	int start_year = 0;
	int start_month = 0;
	int start_day = 0;
	int end_year = 0;
	int end_month = 0;
	int end_day = 0;
	int cur_year = 0;
	int cur_month = 0;
	int cur_day = 0;
	int no_limit = 0;
	int found = 0;
	int extra = 0;		/* extra = 1 when current date is less than
				   the start date, otherwise 0. This is the 
				   adjustment for correct counting up to the
				   month and day of the start date */
	char **era_list = hdl->era;

	cur_year = tm->tm_year + 1900;
	cur_month = tm->tm_mon + 1;
	cur_day = tm->tm_mday;

	for( ; (era_s = *era_list) != NULL; era_list++) {
	    int	nmatch,i;

	    nmatch = sscanf(era_s,
			    "%1[-+]:%u:%99[^:]:%99[^:]:%99[^:]:%n",
			    dirstr,
			    &era->offset,
			     era->st_date,
			     era->end_date,
			     era->name,
			    &i); 	/* Remember end of matched string */

	    if (nmatch != 5)		/* Bad era string */
		continue;		/* Try again */

	    era->dir = dirstr[0];

	    /* If era_year is defined, use it */
	    if (strcmp(hdl->era_year, ""))
		strcpy(era->form, hdl->era_year);
	    else
		strcpy(era->form, &era_s[i]);

	    str = era->st_date;
	    if (*str == '-') {
		str++;
		STRTONUM(str,start_year);
		start_year = -start_year + 1;
	    } else
		STRTONUM(str,start_year);

	    str++;			/* skip the slash */
	    STRTONUM(str,start_month);
	    str++;			/* skip the slash */
	    STRTONUM(str,start_day);

	    str = era->end_date;
	    if ((*str=='+' && *(str+1)=='*' )||(*str=='-' && *(str+1)=='*'))
		no_limit = 1;
	    else {
		no_limit = 0;
		if (*str == '-') {
		    str++;
		    STRTONUM(str,end_year);
		    end_year = -end_year + 1;
		} else
		    STRTONUM(str,end_year);
		str++;		/* skip the slash */
		STRTONUM(str,end_month);
		str++;		/* skip the slash */
		STRTONUM(str,end_day);
	    }
	    if (no_limit && cur_year >= start_year) {
		found = 1;
	    } else if (((cur_year > start_year) ||
		      (cur_year == start_year && cur_month > start_month) ||
		      (cur_year == start_year && cur_month == start_month &&
		       cur_day >= start_day)) &&
		     ((cur_year < end_year) ||
		      (cur_year == end_year && cur_month < end_month) ||
		      (cur_year == end_year && cur_month == end_month &&
		       cur_day <= end_day))) {
		found = 1;
	    } else 
		continue; 

	    if ((cur_month < start_month) || 
		(cur_month == start_month && cur_day < start_day))
		extra = 1;
	    if (era->dir == '+') {
		*year = cur_year - start_year + era->offset - extra;
	    } else
		*year = end_year - cur_year - extra;

	    if (found)
		return(1);
	}
	return(0);		/* No match for era times */
}


/*
 * FUNCTION: This function performs the actual formatting and it may
 *	     be called recursively with different values of code.
 *
 * PARAMETERS:
 *	   _LC_TIME_t *hdl - the handle of the pointer to the LC_TIME
 *			     category of the specific locale.
 *	   char *s - location of returned string
 *	   size_t maxsize - maximum length of output string
 *	   char *format - format that date is to be printed out
 *	   struct tm *timeptr - date to be printed
 *	     int code - this special attribute controls the outupt of
 *		    	certain field (eg: twelve hour form, without
 *			year or second for time and date format).
 *
 * RETURN VALUE DESCRIPTIONS:
 *	   - returns the number of bytes that comprise the return string
 *	     excluding the terminating null character.
 *	   - returns 0 if s is longer than maxsize
 */
static size_t
#ifdef _THREAD_SAFE
doformat(_LC_time_t *hdl, char *s, size_t maxsize, char *format, 
	 struct tm *timeptr, int code, strftime_data_t *strftime_data)
#else
doformat(_LC_time_t *hdl, char *s, size_t maxsize, char *format, 
	 struct tm *timeptr, int code)
#endif /* _THREAD_SAFE */
{
	int i;
	int firstday;		/* first day of the year */
	int toolong = 0;
	int weekno, wmod;
	char *strp;		/* pointer into output buffer str */
	char *strend;		/* last available byte in output buffer str */
	char *lastf;		/* last byte of str made by a format */
	int width;		/* width in current format or 0 */
	int prec;		/* precision in current format or 0 */
	int wpflag;		/* true if width or precision specified */
	char locbuffer[BUFSIZE];/* local temporary buffer */
	int year;		/* %o value, year in current era */
	int found;		/* logical flag for a valid era */
	char *fbad;		/* points to where format start to be invalid */
#ifdef _THREAD_SAFE
	char tzbuf[TZNAME_MAX+1];	/* tzname[] copy */
#else
	static int era_name = 0;/* logical flag for detected era name */
	static int era_yr = 0;	/* logical flag for detected era year */
#endif	/* _THREAD_SAFE */
	char *era_s;		/* locale's emperor/era string */
	char *subera;		/* era string of a multiple era */
	char *f;		/* era format of subera */
	char erabuf[BUFSIZE];  	/* a work buffer for the era string */
	char fill_char;		/* filling char which may be blank or zero */
	char *p;		/* temp pointer */

	lastf = strp = s;
	strend = s+maxsize-1;
	altera = 0;		/* Recursive call should reset 'altera' */
	while (i = *format++) {
		if (i != '%')
			PUT(i);
		else {
			wpflag = width = prec = 0;
			bufp = "";	/* This should get set in loop */
			fbad = format;
			fill_char = ' ';	/* blank is default fill char */

			/*
			 * get field width & precision
			 */
			if (*format == '0')		/* Zero-fill instead */
				fill_char = '0';

			width = strtol((char *)format, &p, 10);
			if ( p!=format) {
				format = p;
				wpflag++;
			}

			if ( *format == '.' ) {
				prec = strtoul( (char *)++format, &p, 10);
				if ( p!=format ) {
					format = p;
					wpflag++;
				}
			}

			switch (*format) {
			case 'O':
				format++;
				if (!hdl->era || !strchr("deHImMSuUVwWy",*format))
					BADFORMAT;
				else {
					if (*format == 'y');
						altera = 1;
					altnum = hdl->alt_digits;
				}
				break;

			case 'E':
				format++;
				if (!hdl->era || !strchr("cCxXyY",*format))
					BADFORMAT;
				else
					altera = 1;
				break;
			}

			switch(*format++) {
			case '%':
				bufp = "%";	/* X/Open - percent sign */
				break;

			case 'n':	/* X/Open - newline character */
				bufp = "\n";
				break;

			case 't':	/* X/Open - tab character */
				bufp = "\t";
				break;

			case 'm':	/* X/Open - month in decimal number */
				bufp = GETNUM(timeptr->tm_mon+1,WIDTH(2));
				altnum = NULL;
				break;

			case 'd': 	/* X/Open - day of month in decimal */
				bufp = GETNUM(timeptr->tm_mday,WIDTH(2));
				altnum = NULL;
				break;

			case 'e':	/* day of month with leading space */
				bufp = GETNUM(timeptr->tm_mday,WIDTH(2));
				if (!altnum && *bufp == '0')
					*bufp = ' ';
				altnum = NULL;
				break;

			case 'y':	/* X/Open - year w/o century 00-99 */
				if (code==NOYEAR) 
					 SKIP;
				else if (altera) {
					if (CONV_TIME(hdl, timeptr, ERASTR, &year)) {
						DOFORMAT(hdl, locbuffer, BUFSIZE,
							  "%Jy", timeptr, 0);
						bufp = locbuffer;
						altera = 0;
					} else {
						/* if era_year or era->form is not specified, %Ey will display %y output. */
						bufp=GETNUM(timeptr->tm_year,WIDTH(2));
						altnum = NULL;
					}
				} else {	
					 bufp=GETNUM(timeptr->tm_year,WIDTH(2));
					 altnum = NULL;
				}
				break;

			case 'H':	/* X/Open - hour (0-23) in decimal */
				bufp = GETNUM(timeptr->tm_hour,WIDTH(2));
				altnum = NULL;
				break;

			case 'M':	/* X/Open - minute in decimal */ 
				bufp = GETNUM(timeptr->tm_min,WIDTH(2));
				altnum = NULL;
				break;

			case 'S':	/* X/Open - second in decimal */
				if (code==NOSECS)
					 SKIP;
				else {
					 bufp=GETNUM(timeptr->tm_sec,WIDTH(2));
					 altnum = NULL;
				}
				break;

			case 'j': 	/* X/Open - day of year in decimal */
				bufp = GETNUM(timeptr->tm_yday+1,WIDTH(3));
				altnum = NULL;
				break;

			case 'w': 	/* X/Open - weekday in decimal */
				bufp = GETNUM(timeptr->tm_wday,WIDTH(1));
				altnum = NULL;
				break;

			case 'r': 	/* X/Open - time in AM/PM notation */
				DOFORMAT(hdl, locbuffer, BUFSIZE,
					 CHECKFMT(hdl->t_fmt_ampm, "%I:%M:%S %p"),
					 timeptr,0);
				bufp = locbuffer;
				break;

			case 'R':	/* X/Open - time as %H:%M */
				DOFORMAT(hdl, locbuffer, BUFSIZE, "%H:%M", timeptr,0);
				bufp = locbuffer;
				break;

			case 'T': 	/* X/Open - time in %H:%M:%S notation */
				DOFORMAT(hdl, locbuffer, BUFSIZE,
					 "%H:%M:%S", timeptr, 0);
				bufp = locbuffer;
				break;

			case 'X': 	/* X/Open - the locale time notation */
				if (altera &&
				    hdl->core.hdr.size > offsetof(_LC_time_t,era_t_fmt))
				    /*
				     * locale object is recent enough to have era_t_fmt
				     * field.
				     */
				    p = CHECKFMT(hdl->era_t_fmt, hdl->t_fmt);
				else
				    p = hdl->t_fmt;
				altera = 0;

				DOFORMAT(hdl, locbuffer, BUFSIZE,
					 p,
					 timeptr, 0);

				bufp = locbuffer;
				break;

			case 'l': 	/* IBM-long day name, long month name,
					locale date representation*/
				switch (*format++) {
				case 'a':
					bufp = strcpy(locbuffer, hdl->day[timeptr->tm_wday]);
					break;

				case 'h':
					bufp = strcpy(locbuffer, hdl->mon[timeptr->tm_mon]);
					break;

				case 'D':
					DOFORMAT(hdl, locbuffer, BUFSIZE,
						 "%b %d %Y", timeptr, 0);
					bufp = locbuffer;
					break;
				default :
					BADFORMAT;
				}
				break;

			case 's': 	/* IBM-hour(12 hour clock), long date
					   w/o year, long time w/o secs */
				switch (*format++) {
				case 'H':
					i=timeptr->tm_hour;
					bufp = GETNUM(i>12?i-12:i,WIDTH(2));
					break;

				case 'D':
					DOFORMAT(hdl, locbuffer, BUFSIZE,
						 "%b %d %Y", timeptr, NOYEAR);
						bufp = locbuffer;
					break;

				case 'T':
					DOFORMAT(hdl, locbuffer, BUFSIZE,
						 hdl->t_fmt, timeptr, NOSECS);
					bufp = locbuffer;
					break;
				default :
					BADFORMAT;
				}
				break;

			case 'a': 	/* X/Open - locale's abv weekday name */
				bufp = strcpy(locbuffer, hdl->abday[timeptr->tm_wday]);
				break;
		
			case 'h':	/* X/Open - locale's abv month name */

			case 'b':
				bufp = strcpy(locbuffer, hdl->abmon[timeptr->tm_mon]);
				break;

			case 'p': 	/* X/Open - locale's equivalent AM/PM */
				if (timeptr->tm_hour<12)
					strcpy(locbuffer, hdl->am_pm[0]);
				else
					strcpy(locbuffer, hdl->am_pm[1]);
				bufp = locbuffer;
				break;

			case 'Y':	/* X/Open - year w/century in decimal */
				if (code==NOYEAR)
					SKIP;
				else if (altera) {/* POSIX.2 %EY full alternate yr */
				        if (CONV_TIME(hdl, timeptr, ERASTR, &year)) {
					    DOFORMAT(hdl, locbuffer, BUFSIZE,
						     ERASTR->form, timeptr, 0);
					    bufp = locbuffer;
					} else {
						/* if era_year or era->form is not specified, %EY will display %Y output. */
						bufp = GETNUM(timeptr->tm_year+1900,
							      WIDTH(4));
						altnum = NULL;
					}
					altera = 0;
				} else {
					bufp = GETNUM(timeptr->tm_year+1900,
						      WIDTH(4));
					altnum = NULL;
				}
				break;

			case 'z':	/* IBM - timezone name if it exists */

			case 'Z':	/* X/Open - timezone name if exists */
#ifdef _THREAD_SAFE
				TS_LOCK(&_ctime_rmutex);
				strcpy(tzbuf, GETTIMEZONE(timeptr));
				TS_UNLOCK(&_ctime_rmutex);
				bufp = tzbuf;
#else
				bufp = GETTIMEZONE(timeptr);
#endif	/* _THREAD_SAFE */
				break;

			case 'A': 	/* X/Open -locale's full weekday name */
				bufp = strcpy(locbuffer, hdl->day[timeptr->tm_wday]);
				break;

			case 'B':	/* X/Open - locale's full month name */
				bufp = strcpy(locbuffer, hdl->mon[timeptr->tm_mon]);
				break;

			case 'I': 	/* X/Open - hour (1-12) in decimal */
				i = timeptr->tm_hour;
				bufp = GETNUM(i>12?i-12:i?i:12, WIDTH(2));
				altnum = NULL;
				break;

			case 'D': 	/* X/Open - date in %m/%d/%y format */
				DOFORMAT(hdl, locbuffer, BUFSIZE,
					 "%m/%d/%y", timeptr, 0);
				bufp = locbuffer;
				break;

			case 'x': 	/* X/Open - locale's date */
				if (altera)
				    p = CHECKFMT(hdl->era_d_fmt, hdl->d_fmt);
				else
				    p = hdl->d_fmt;

				altera = 0;
				DOFORMAT(hdl, locbuffer, BUFSIZE,
					 p, timeptr, 0);
				bufp = locbuffer;
				break;

			case 'c': 	/* X/Open - locale's date and time */
				if (altera &&
				    hdl->core.hdr.size > offsetof(_LC_time_t,era_d_t_fmt))
				    /*
				     * %Ec and we've got the era_d_t_fmt field
				     */
				    p = CHECKFMT(hdl->era_d_t_fmt, hdl->d_t_fmt);
				else
				    p = hdl->d_t_fmt;

				p = CHECKFMT(p, "%a %b %e %H:%M:%S %Y");

				DOFORMAT(hdl, locbuffer, BUFSIZE, p, timeptr, 0);
				bufp = locbuffer;
				break;

			case 'u':
				/* X/Open - week day as a number [1-7]
				   (Monday as 1) */
				bufp = GETNUM((timeptr->tm_wday == 0 ? 7 :
						timeptr->tm_wday), WIDTH(1));
				altnum = NULL;
				break;

				
			case 'U':
				/* X/Open - week number of the year (0-53)
				   (Sunday is the first day of week 1) */

				weekno = (timeptr->tm_yday + 7 - timeptr->tm_wday) / 7;
				bufp = GETNUM(weekno, WIDTH(2));
				altnum = NULL;
				break;

			case 'V':
				/* X/Open - week number of the year, (Mon=1)
				 * as [01,53]
				 */
				firstday = (timeptr->tm_wday + 6) % 7;	/* Prev day */
				weekno = (timeptr->tm_yday + 7 - firstday)/7;

				if (firstday > 4 ) /* Adjust for year with fewer*/
				    weekno--; 	   /* than 4 days in first week */

				weekno %= 53; 	/* [0..52] */
				weekno ++;	/* [1..53] */

				bufp= GETNUM(weekno,WIDTH(2));
				altnum = NULL;
				break;


			case 'W':
				/* X/Open - week number of the year (0-53)
				   (Monday is the first day of week 1) */

				firstday = (timeptr->tm_wday + 6) % 7;	/* Prev day */
				weekno = (timeptr->tm_yday + 7 - firstday)/7;

				bufp= GETNUM(weekno,WIDTH(2));
				altnum = NULL;
				break;

			 /* This is the additional code to support non-Christian
			    eras. The formatter %Jy will display the relative
			    year from the relevant era entry in NLYEAR, %Js will
			    display the era name.
			  */
			case 'J': 	/* IBM - era and year of the Emperor */
				 switch(*format++) {
				 case 'y': 
					if (hdl->era == NULL) {
						BADFORMAT;
					}
					else if (CONV_TIME(hdl, timeptr,
							   ERASTR, &year)){
						if (altnum)
							year %= 100;
						if (year >= 100) {
							bufp = GETNUM(year, WIDTH(getwidth(year)));
						}
						else {
							bufp = GETNUM(year, WIDTH(2));
						}
						era_name = 1;
					}
					else {
						BADFORMAT;
					}
					break;

				 case 's':
					if (! *hdl->era){
						BADFORMAT;
					}
					else if (era_yr) {
						bufp = ERASTR->name;
						era_yr = 0;
						era_name = 0;
					}
					else if (CONV_TIME(hdl, timeptr,
							   ERASTR, &year)) {
						bufp = ERASTR->name;
						era_name = 1;
					}
					else {
						BADFORMAT;
					}
					break;
				 default:
					BADFORMAT;
					break;
				}
				break;

			case 'C':
				if (!altera) {	/* %C - Century [00-99] */
					bufp = GETNUM(((timeptr->tm_year+1900)/100), WIDTH(2));
					break;
				}
				/* %EC should be handled even when hdl->era is not specified */
				if (altera && !*hdl->era) {
					bufp = GETNUM(((timeptr->tm_year+1900)/100), WIDTH(2));
					break;
				}

			case 'N':	/* locale's era name */
				if (! *hdl->era){
					BADFORMAT;
				}
				else if (era_yr) {
					bufp = ERASTR->name;
					era_yr = 0;
					era_name = 0;
				}
				else if (CONV_TIME(hdl, timeptr, ERASTR, &year)) {
					bufp = ERASTR->name;
					era_name = 1;
				}
				else {
					BADFORMAT;
					}
				break;

			case 'o':			/* era year */
				if (hdl->era == NULL) {
					BADFORMAT;
				}
				else if (era_name) {
					bufp = GETNUMBER (year);
					era_yr = 0;
					era_name = 0;
				}
				else if (CONV_TIME(hdl, timeptr, ERASTR, &year)) {
					bufp = GETNUMBER(year);
					era_name = 1;
				}
				else {
					BADFORMAT;
				}
				break;

			default:			 /* badformat */
				BADFORMAT;
				break;
			} /* switch */

			/* output bufp with appropriate padding */

			i = strlen(bufp);

			if (prec && prec < i) {	 /* truncate on right */
				*(bufp + prec) = '\0' ;
				i = prec;
			}
			if (width > 0)
				while(!toolong && i++ < width)
					PUT(fill_char);

			while(!toolong && *bufp)
				PUT(*bufp++);

			if (width<0)
				while(!toolong && i++ < -width)
					PUT(fill_char);
			lastf = strp;
		} 	/* i == '%' */

		if (toolong)
			break;
	}
	*strp = 0;
	if (toolong)
		return (0);
	return (strp - s);
}


/*
 * FUNCTION: strfmon_std()
 *	     This is the standard method to format the date and ouput to 
 *	     the output buffer s. The values returned are affected by 
 *	     the setting of the locale category LC_TIME and the time 
 *  	     information in the tm time structure.
 *
 * PARAMETERS:
 *	     _LC_TIME_t *hdl - the handle of the pointer to the LC_TIME
 *			       catagory of the specific locale.
 *	   char *s - location of returned string
 *	   size_t maxsize - maximum length of output string
 *	   char *format - format that date is to be printed out
 *	   struct tm *timeptr - date to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 *	   - returns the number of bytes that comprise the return string
 *	       excluding the terminating null character.
 *	   - returns 0 if s is longer than maxsize
 */
size_t 
__strftime_std( char *s, size_t maxsize, 
	       const char *format, const struct tm *timeptr, _LC_time_t *hdl)
{
#ifdef _THREAD_SAFE
	strftime_data_t	real_strftime_data;
	strftime_data_t	*strftime_data = &real_strftime_data;

	ERASTR = &eras;
	era_yr = 0;
	era_name = 0;
#endif /* _THREAD_SAFE */

	altnum = NULL;		/* Indate no alternate numerics in use */
	altera = 0;		/* Not using era-based date formats */

	return (DOFORMAT(hdl, s, maxsize, (char *)format,
			 (struct tm *)timeptr, 0));
}


/*
 * FUNCTION: strftime() is a method driven function where the time formatting
 *           processes are done the method points by __lc_time->core.strftime.
 *           It formats the date and output to the output buffer s. The values
 *           returned are affected by the setting of the locale category
 *           LC_TIME and the time information in the tm time structure.
 *
 * PARAMETERS:
 *           char *s - location of returned string
 *           size_t maxsize - maximum length of output string
 *           char *format - format that date is to be printed out
 *           struct tm *timeptr - date to be printed
 *
 * RETURN VALUE DESCRIPTIONS:
 *           - returns the number of bytes that comprise the return string
 *             excluding the terminating null character.
 *           - returns 0 if s is longer than maxsize
 */

size_t 
strftime(char *s, size_t maxsize, const char *format,
		const struct tm *timeptr)
{
        char * tz;

	if (format == NULL)
	    format = "%c";	/* SVVS 4.0 */

	/* tzset() call avoidance logic moved to tzset() */
	tzset();	 

	if (METHOD(__lc_time,strftime))
		return METHOD(__lc_time,strftime)( s,
					   maxsize, format,
                                           timeptr, __lc_time);
	else
		return __strftime_std( s, maxsize, format, timeptr, __lc_time);
}

/*
 * The OSF/1 1.0 strftime.c contained NLstrtime(), which
 * is just like strftime() except for its return value. 
 *
 * In 1.0, NLstrtime() was used only by the ex and lock
 * commands.  In 1.2, it was dropped, and those commands
 * were changed to use strftime() instead.  Until we
 * integrate the 1.2 versions of those commands, we need
 * to keep NLstrtime() around.
 */

char *
NLstrtime(char *s, size_t maxsize, const char *format,
		const struct tm *timeptr)
{
	strftime(s, maxsize, format, timeptr);
	return s;
}
