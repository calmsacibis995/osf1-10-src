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
static char	*sccsid = "@(#)$RCSfile: NLgetenv.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:02 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * FUNCTIONS: NLgetenv
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * NLgetenv.c	1.21  com/lib/c/gen,3.1,9013 2/28/90 16:20:51
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if defined(_THREAD_SAFE)
#pragma weak NLgetenv_r = __NLgetenv_r
#endif
#if !defined(_THREAD_SAFE)
#pragma weak NLgetenv = __NLgetenv
#endif
#endif
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <langinfo.h>
#include <stdlib.h>

#ifdef _THREAD_SAFE
#include <errno.h>
#include "rec_mutex.h"

extern struct rec_mutex	_locale_rmutex;
#endif

/*
** The references to the OSF/1 1.0 locale structure in
** NLgetenv() can be replaced with calls to localeconv()
** and nl_langinfo() except for a few cases.  Those
** cases deal with date/time strings from 1.0 locales
** that are either not present in the 1.2 locales or are
** in a different format.
**
** For the strings that are not present in the 1.2
** locales, the workaround is to hard-code the strings
** from the 1.0 C locale.  That works because most of
** those strings have the same values in all of the 1.0
** locales.  NLLDATE is the exception.  We won't worry
** about it though because the only one who calls
** NLgetenv("NLLDATE") is libc's NLtmtime(), and no one
** in all of OSF/1 1.0 calls that.
**
** For the strings that are in a different format in
** the 1.2 locales, the workaround is to use 1.2 locale
** info to build strings in the proper format.  That is
** done in the new function build_string().
*/

static char *nlldate  = "%b %d %Y";
static char *nlyear   = "19890108,Heisei:19261225,Showa:";
static char *nltmisc  = "at:each:every:on:through:am:pm:zulu";
static char *nltstrs  =
    "now:yesterday:tomorrow:noon:midnight:next:weekdays:weekend:today";
static char *nltunits =
    "minute:minutes:hour:hours:day:days:week:weeks:month:months:\
year:years:min:mins";

static nl_item abmon_items[] = {
    ABMON_1, ABMON_2, ABMON_3, ABMON_4,  ABMON_5,  ABMON_6,
    ABMON_7, ABMON_8, ABMON_9, ABMON_10, ABMON_11, ABMON_12
};

static nl_item   mon_items[] = {
    MON_1, MON_2, MON_3, MON_4,  MON_5,  MON_6,
    MON_7, MON_8, MON_9, MON_10, MON_11, MON_12
};

static nl_item abday_items[] = {
    ABDAY_1, ABDAY_2, ABDAY_3, ABDAY_4, ABDAY_5, ABDAY_6, ABDAY_7
};

static nl_item   day_items[] = {
    DAY_1, DAY_2, DAY_3, DAY_4, DAY_5, DAY_6, DAY_7 };

static nl_item am_pm_items[] = {
    AM_STR, PM_STR
};

#ifdef _THREAD_SAFE
static char *build_string_r(int *items, int num_items, char *buf, int buflen)
{
#else
static char *build_string  (int *items, int num_items)
{
	static char *buf  = NULL;
	static int buflen = 0;
#endif /* _THREAD_SAFE */
	int i, j, n;
	char *s;
	int newlen;

	j = 0;	/* index into buf[] */

	for (i = 0; i < num_items; i++) {
		s = nl_langinfo(items[i]);
		n = strlen(s);

		/*
		 * Need room for a colon separator, the item, and the
		 * null terminator.
		 */
		newlen = j + 1 + n + 1;
		if (newlen > buflen) {
#ifdef _THREAD_SAFE
			int nbytes_left = buflen - j - 1;
			if ((i != 0) && (nbytes_left > 0)) {
				buf[j++] = ':';
				nbytes_left--;
			}

			(void) strncpy(&buf[j], s, nbytes_left);
			j += nbytes_left;

			break;
#else
			buf = realloc(buf, newlen);
			if (buf == NULL) {
				buflen = 0;
				return "";
			}

			buflen = newlen;
#endif /* _THREAD_SAFE */
		}

		if (i != 0)
			buf[j++] = ':';
		
		(void) strncpy(&buf[j], s, n);
		j += n;
	}

	buf[j] = '\0';

	return(buf);
}

/*
**  Get an NLS parameter from the locale information
**  setup by a call to setlocale().
**  This parameter should belong in one of the following categories:
**  LC_MONETARY, LC_NUMERIC, LC_TIME, LC_MESSAGES.
**  If the information solicited is not found in the tables setup
**  by setlocale(), an American English default table is searched
**  and the value in that default is returned.  
**  Return a NULL pointer if no data can be found.
*/

#ifdef _THREAD_SAFE
int
NLgetenv_r(char *name, char *buf, int buflen)
#else /* _THREAD_SAFE */
char *
NLgetenv(name)
char *name;
#endif /* _THREAD_SAFE */
{
	struct lconv *plconv;

#define NUM_ELEMENTS(a) (sizeof(a) / sizeof((a)[0]))

#ifdef _THREAD_SAFE
#define	RETURN(s) \
	{ strncpy(buf, s, buflen); \
	  rec_mutex_unlock(&_locale_rmutex); \
	  return(0); \
	}
#define	BUILD_AND_RETURN(items) \
	{ build_string_r(items, NUM_ELEMENTS(items), buf,  buflen); \
	  rec_mutex_unlock(&_locale_rmutex); \
	  return(0); \
	}
#else
#define	RETURN(s)	return(s)
#define	BUILD_AND_RETURN(items) \
	return(build_string(items, NUM_ELEMENTS(items)))
#endif

#ifdef _THREAD_SAFE
	if (buf == NULL || buflen < 1) {
		seterrno(EINVAL);
		return(-1);
	}
#endif
#ifdef _THREAD_SAFE
	rec_mutex_lock(&_locale_rmutex);
#endif

	plconv = localeconv();

	/* LC_MONETARY */	

		/* international currency symbol */
	if(strcmp(name, "INT_CUR_SYM")==0)
		RETURN(plconv->int_curr_symbol);	

		/* national currency symbol */
	if(strcmp(name, "CUR_SYM")==0)
		RETURN(plconv->currency_symbol);

		/* currency decimal point */
	if(strcmp(name, "MON_DEC_PNT")==0)
		RETURN(plconv->mon_decimal_point);
			
		/* currency thousands separator */
	if(strcmp(name, "MON_THOUS")==0)
		RETURN(plconv->mon_thousands_sep);

		/* currency digits grouping */
	if(strcmp(name, "MON_GRP")==0)
		RETURN(plconv->mon_grouping );

		/* currency plus sign */
	if(strcmp(name, "POS_SGN")==0)
		RETURN(plconv->positive_sign);

		/* currency minus sign */
	if(strcmp(name, "NEG_SGN")==0)
		RETURN(plconv->negative_sign );	

		/* internat currency fract digits */
	if(strcmp(name, "INT_FRAC")==0)
		RETURN((char *)&plconv->int_frac_digits);

		/*currency fractional digits */
	if(strcmp(name, "FRAC_DIG")==0)
		RETURN((char *)&plconv->frac_digits );

		/* currency plus location */
	if(strcmp(name, "P_CS_PRE")==0)
		RETURN((char *)&plconv->p_cs_precedes);	

		/* currency plus space ind. */
	if(strcmp(name, "P_SEP_SP")==0)
		RETURN((char *)&plconv->p_sep_by_space );	

		/* currency minus location */
	if(strcmp(name, "N_CS_PRE")==0)
		RETURN((char *)&plconv->n_cs_precedes );	

		/* currency minus space ind. */
	if(strcmp(name, "N_SEP_SP")==0)
		RETURN((char *)&plconv->n_sep_by_space );	

		/* currency plus position */
	if(strcmp(name, "P_SGN_POS")==0)
		RETURN((char *)&plconv->p_sign_posn );	

		/* currency minus position */
	if(strcmp(name, "N_SGN_POS")==0)
		RETURN((char *)&plconv->n_sign_posn );	

	/* LC_NUMERIC */

	if(strcmp(name, "DEC_PNT")==0)
		RETURN(plconv->decimal_point );

	if(strcmp(name, "THOUS_SEP")==0)
		RETURN(plconv->thousands_sep );

	if(strcmp(name, "GROUPING")==0)
		RETURN(plconv->grouping );
		
	/* LC_TIME */

		/* NLTIME; date %X descriptor */
	if(strcmp(name,"NLTIME")==0)
		RETURN(nl_langinfo(T_FMT));         
		
		/* NLDATE; date %x descriptor */
	if(strcmp(name,"NLDATE" ) ==0)
		RETURN(nl_langinfo(D_FMT));         

		/* NLLDATE  long form */
	if(strcmp(name,"NLLDATE")==0)
		RETURN(nlldate);
		
		/* NLDATIM, date %c descriptor */
	if(strcmp(name,"NLDATIM")==0)
		RETURN(nl_langinfo(D_T_FMT));         

		/* NLSDAY; date %a descriptor */
	if(strcmp(name,"NLSDAY")==0)
		BUILD_AND_RETURN(abday_items);

		/* NLLDAY; date %A descriptor */
	if(strcmp(name,"NLLDAY")==0)
		BUILD_AND_RETURN(day_items);

		/* NLSMONTH; date %b descriptor */
	if(strcmp(name,"NLSMONTH")==0)
		BUILD_AND_RETURN(abmon_items);

		/* NLLMONTH; date %B descriptor */
	if(strcmp(name,"NLLMONTH")==0)
		BUILD_AND_RETURN(mon_items);

		/* NLTMISC at;each;every;on;through */
	if(strcmp(name,"NLTMISC")==0)
		RETURN(nltmisc);

		/* am and pm */
	if(strcmp(name,"AMPMSTR" )==0)
		BUILD_AND_RETURN(am_pm_items);
		
        	/* NLTSTRS */
	if(strcmp(name,"NLTSTRS")==0)
		RETURN(nltstrs);

		/* NLTUNITS */
	if(strcmp(name,"NLTUNITS" )==0)
		RETURN(nltunits);

		/* Name of the year and the starting time */
	if(strcmp(name,"NLYEAR" )==0)
		RETURN(nlyear);

	/* LC_MESSAGES */

		/*
		 * NLgetenv("MESSAGES") always returned ""
		 * in OSF/1 1.0.
		 */
	
		/* Message Catalog name */
	if(strcmp(name,"MESSAGES")==0)
		RETURN("");
		
		/* Response string for affirmation */
	if(strcmp(name,"YESSTR")==0)
		RETURN(nl_langinfo(YESSTR));
		
		/* Response string for negation */
	if(strcmp(name, "NOSTR")==0)
		RETURN(nl_langinfo(NOSTR));	
	
#ifdef _THREAD_SAFE
	rec_mutex_unlock(&_locale_rmutex);
	seterrno(ESRCH);
	return(-1);
#else
	return((char *)NULL);
#endif
}
