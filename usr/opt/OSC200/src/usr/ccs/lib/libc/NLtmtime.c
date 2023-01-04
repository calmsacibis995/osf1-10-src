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
static char	*sccsid = "@(#)$RCSfile: NLtmtime.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/06/08 01:18:57 $";
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
 * COMPONENT_NAME: (LIBCNLS) Standard C Library National Language Support 
 *
 * FUNCTIONS: NLtmtime, doscan, number, string, getfield, setparam,
 *	      setgen, setampm, setzone, detype, l_getshowa
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * NLtmtime.c	1.12  com/lib/c/nls,3.1,9013 2/13/90 15:47:52
 */
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak NLtmtime = __NLtmtime
#pragma weak detype = __detype
#endif
#endif
#define NULL 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <NLctype.h>
#include <values.h>
#include <time.h>

#define GETC(c)		(c = *strp++)
#define UNGETC(c)	(c = *--strp)
#define IGETC(c)	(c = *(*strp)++)
#define UNIGETC(c)	(c = *--*strp)

#define ISSTRING	0
#define ISNUM		1
#define ISERROR 	2

#define NOSECONDS	1

static int Noyear = 0, Noseconds = 0, finish = 0;
#ifdef KJI
static int Yearfnd = 0;
#endif
static unsigned char **savestrp;

/* Enumerated type used for field descriptor values;  easier to code
   and debug than preprocessor definitions
*/

#ifdef KJI
static enum desc { m, d, y, H, M, S, j, w, D, T, a, h, b, r, Y, la, lh, B,
		   lD, sH, sD, sT, p, z, Js, Jy
} desc;
#else
static enum desc { m, d, y, H, M, S, j, w, D, T, a, h, b, r, Y, la, lh, B,
		   lD, sH, sD, sT, p, z
} desc;
#endif

static void setgen(), setampm(), setzone();
static int string(), number(), doscan();
char  *NLgetenv();
static int setparam();
static char *static_strtok();
#define strtok(s1, s2) static_strtok(s1, s2)

/*
 * NAME: NLtmtime
 *
 * FUNCTION: Sets a time structure from string data.
 *
 * NOTE: doscan() processes data and return value.
 * 
 * RETURN VALUE DESCRIPTION: The number of matches found between format
 * 	     specifications and data.
 */
int
NLtmtime(str, fmt, tm)
unsigned char *str, *fmt;
struct tm *tm;
{
	int num;
	num = doscan(str, fmt, tm);
	finish = 0;
	return (num);
}

/*
 * NAME: doscan
 *
 * FUNCTION: Scans data and converts them according to fmt.
 *
 * NOTE: detype() determines format type; number() processes number type;
 *       string() processes char type.
 *
 * RETURN VALUE DESCRIPTION: The number of matches found between data and
 *	 format specifications.
 */
static int
doscan(str, fmt, tm)
unsigned char *str;	/* a string to be ocnverted */
unsigned char *fmt;	/* format specifications */
struct tm *tm;	/* tm struct */
{
	unsigned char *strp;	/* the next character position */
#ifdef KJI
	register int ch;	/* format char */
#else
	register unsigned char ch;
#endif
	int nmatch = 0, len, inchar, stow, size;

	strp = str;
	savestrp = &strp;
	for( ; ; ) {
#ifdef KJI
		if((ch = NCdechr(fmt)) == '\0' || finish)
			return(nmatch); /* end of format */
		fmt+=NLchrlen(fmt);
		if(NCisspace(ch)) {
			while(NCisspace(inchar = NCdechr(strp)))
				strp+=NLchrlen(strp);
			if (inchar != NULL)
				continue;
			break;
		}
#else
		if((ch = *fmt++) == '\0' || finish)
			return(nmatch); /* end of format */
		if(isspace(ch)) {
			GETC(inchar);
			while(isspace(inchar))
				GETC(inchar);
			if (UNGETC(inchar) != NULL)
				continue;
			break;
		}
#endif
		if(ch != '%' || (ch = *fmt++) == '%') {
			if (GETC(inchar) == ch)
				continue;
			if (UNGETC(inchar) != NULL)
				return(nmatch); /* failed to match input */
			break;
		}

		/* If we reach here, we must have encountered a '%' */
		if(ch == '*') {
			stow = 0;
			ch = *fmt++;
		} else
			stow = 1;

		for(len = 0; isdigit(ch); ch = *fmt++)
			len = len * 10 + ch - '0';
		if(len == 0)
			len = MAXINT;

		if(ch == '\0' )
			return(NULL); /* unexpected end of format */

#ifdef KJI
		while(NCisspace(inchar = NCdechr(strp)))
			strp+=NLchrlen(strp);
		if(inchar == NULL)
			break;
#else
		while(isspace(GETC(inchar)))
			;
		if(UNGETC(inchar) == NULL)
			break;
#endif

		switch (detype(ch, fmt) ) {
		case ISSTRING: size = string(stow, len, tm, &strp, fmt); break;
		case ISNUM:
			size = number(stow, len, tm, &strp);
			break;
		case ISERROR:  size = 0; 
		}
		nmatch += size;
		if(size == 0)
			return(nmatch); /* failed to match input */
		switch (desc) {
			case la: case lh: case lD:
			case sH: case sD: case sT:
#ifdef KJI
			case Js: case Jy:
#endif
				fmt++ ;
		}
	}
	return(nmatch); /* end of input */
}

/*
 * NAME: number
 *
 * FUNCTION: Processes digit.
 *
 * RETURN VALUE DESCRIPTION: The number of digit processed; 0 for
 *           the unrecognized conversion char.
 */
static int
number(stow, len, tm, strp)
int stow, len;	/*----  set if format is '*'   ----*/
               	/*----  the field length  ----*/
struct tm *tm;
unsigned char **strp;	/*----  the input data string ----*/
{
	unsigned char numbuf[64];	/*----  input data buffer  ----*/
	register unsigned char *np = numbuf; /*---- next char position   ----*/
	register int c;
	int digitseen = 0;	/*----  the number of digit  ----*/
	long lcval = 0;	/*----  digit  ----*/

#ifdef KJI
	char *s1;

	for(c = NCdechr(*strp); --len >= 0 && NCisdigit(c);
	    *np++ = c, c = NCdechr(*strp)) {
		int digit;
		*strp+=NLchrlen(*strp);
		c = isascii(c)? c: _jistoa(c);
		digit = c - '0';
#else
	IGETC(c);
	for( ; --len >= 0 && isdigit(c); *np++ = c, IGETC(c)) {
		int digit = c - '0';
#endif
		if(stow)
			lcval = 10 * lcval + digit;
		digitseen=1;
	}
	if(stow && digitseen)
		switch(desc) {
		case m:
			tm->tm_mon  = lcval - 1; break;
		case d:
			tm->tm_mday = lcval; break;
		case y:
			if (!Noyear)
				tm->tm_year = lcval;
			else 
				Noyear = 0;
			break;
		case H:
			tm->tm_hour = lcval; break;
		case M:
			tm->tm_min  = lcval; break;
		case S:
			if (!Noseconds)
				tm->tm_sec  = lcval;
			else
				Noseconds = 0;
			break;
		case j:
			tm->tm_yday = lcval - 1; break;
		case w:
			tm->tm_wday = lcval - 1; break;
		case Y:
			if (!Noyear)
				tm->tm_year = lcval - 1900;
			else
				Noyear = 0;
			break;

		case sH:
			tm->tm_hour = lcval;  break;
#ifdef KJI
		case Jy:
			if (Yearfnd) {			/* was Js found? */
				tm->tm_year += lcval;
				tm->tm_year -= 1901;
				Yearfnd = 0;
				break;
			} else {	/* if not, and only one NLYEAR entry */
				if ((s1 = NLgetenv("NLYEAR")) == NULL)
					return (0);
				if (strchr(s1, ':'))
					return (0);
				else if (l_getshowa("NLYEAR","",&tm->tm_year)) {
					tm->tm_year += lcval;
					tm->tm_year -= 1901;
					break;
				}
				return (0);	/* do not know which era */
			}
#endif
		default:
			return(0); /* unrecognized conversion character */
		}
#ifdef KJI
	if(c == NULL)
#else
	if(UNIGETC(c) == NULL)
#endif
		strp = NULL; /* end of input */
	return(digitseen);
}

/*
 * NAME: string
 *
 * FUNCTION: Processes a string according to the format.
 *
 * RETURN VALUE DESCRIPTION: The number of matches found between the format
 * 	     and the input; 0 for the unrecognized conversion character.	
 */

static int
string(stow, len, tm, strp, fmt)
register int stow, len;
struct tm *tm;
unsigned char **strp;	/*----  the input string  ----*/
unsigned char *fmt;     	/*----  the time format  ----*/
{
	register int ch;
	register unsigned char *ptr;	/*----  the next char position  ----*/
	unsigned char *start;
	int ret = 1;	/*----  return value  ----*/

	start = ptr = stow ? *strp : NULL;

	/* While we haven't reached end of string 
	   and the characters we are reading are not spaces */

	while(IGETC(ch) != NULL && !isspace(ch)) {

		if(stow)
			*ptr = ch;
		ptr++;
		if(--len <= 0)
			break;
	}
	if (ch == NULL)
		UNIGETC(ch);

	switch (desc) {
	case D:
		ret = setparam(NLgetenv("NLDATE"), start, tm, fmt + 1); break;
	case T:
	case r:
		ret = setparam(NLgetenv("NLTIME"), start, tm, fmt + 1); break;
	case a:
		setgen("NLSDAY", start, &tm->tm_wday); break;
	/* added the format %b to support posix */
	case h: case b:
		setgen("NLSMONTH", start, &tm->tm_mon); break;
	case la:	
		setgen("NLLDAY", start, &tm->tm_wday); break;
	/* added the format %B to support posix */
	case lh: case B:
		setgen("NLLMONTH", start, &tm->tm_mon); break;
	case lD:
		ret = setparam(NLgetenv("NLLDATE"), start, tm, fmt + 2); break;
	case sD:
	 	Noyear = 1; setparam(NLgetenv("NLLDATE"), start, tm, fmt + 2); break;
	case sT:
		Noseconds = 1; setparam(NLgetenv("NLTIME"), start, tm, fmt + 2); break;
	case p:
		setampm(start, tm); break;
	case z:
		setzone(start, tm); break;
#ifdef KJI
	case Js:
		ret = Yearfnd = l_getshowa("NLYEAR", start, &tm->tm_year); break;
#endif
	default:
		return (0);  /* could not recognize descriptor */
	}
	if (ch == NULL || len > 0 && UNIGETC(ch) == NULL)
		*strp = NULL; /* end of input */
	if (ptr == start)
		return(0); /* no match */
	return(ret); /* successful match */
}

/*
 * NAME: detype
 *
 * FUNCTION: Determines the type of format specifiers.
 *
 * RETURN VALUE DESCRIPTION: 0 for string; 1 for number; 2 for error.
 */
/*
   return type of format specifier;  assumes ch is first char in format
   specifier and that next char is = to *rest.  set global variable desc
   (enumerated type) to proper specifier.
*/

int
detype(ch, rest)
unsigned char ch;	/*----  the first character in format specifier  ----*/
unsigned char *rest;	/*----  the remaining characters  ----*/
{
	int typestat;	/*----  number, or string or error  ----*/

	/* initialize default to ISNUM to avoid duplicating code */

	typestat = ISNUM;

	switch (ch) {

	case 'm': desc = m; break;
	case 'd': desc = d; break;
	case 'y': desc = y; break;
	case 'H': desc = H; break;
	case 'M': desc = M; break;
	case 'S': desc = S; break;
	case 'j': desc = j; break;
	case 'w': desc = w; break;
	case 'Y': desc = Y; break;
	case 's':
		switch (*rest) {
		case 'H': desc = sH; break;
		case 'D': desc = sD; typestat = ISSTRING; break;
		case 'T': desc = sT; typestat = ISSTRING; break;
		default:
			typestat = ISERROR;
		}

		break;

	case 'l':

		typestat = ISSTRING;

		switch (*rest) {
		case 'a': desc = la; break;
		case 'h': desc = lh; break;
		case 'D': desc = lD; break;
		default:
			typestat = ISERROR;
		}

		break;

	case 'D': desc = D; typestat = ISSTRING; break;
	case 'T': desc = T; typestat = ISSTRING; break;
	case 'a': desc = a; typestat = ISSTRING; break;
	case 'h': desc = h; typestat = ISSTRING; break;
	/* added the format %b and %B to support posix */
	case 'b': desc = b; typestat = ISSTRING; break;
	case 'B': desc = B; typestat = ISSTRING; break;
	case 'r': desc = r; typestat = ISSTRING; break;
	case 'p': desc = p; typestat = ISSTRING; break;
	case 'z': desc = z; typestat = ISSTRING; break;
#ifdef KJI
	case 'J':
		switch (*rest) {
		case 'y': desc = Jy; typestat = ISNUM; break;
		case 's': desc = Js; typestat = ISSTRING; break;
		default:
			typestat = ISERROR;
		}
		break;
#endif
	default:
		typestat = ISERROR;
	}

	return (typestat);
}

/*
 * NAME: getfield
 *
 * FUNCTION: Counts the delimiter ':' and get the field.
 *
 * RETURN VALUE DESCRIPTION: the number of occurrence of ':'.
 */
static int
getfield(source, pat)
unsigned char *source, *pat;	/*----  an environment variable  ----*/ 
                       	/*----  a string to be scanned  ----*/
{

	unsigned char *s1, *newsource;	/*----  the next field position  ----*/
                           	/*----  a temporary storage for source  ----*/
	register int count = 0;	/*----  a counter for ':' ----*/

	newsource = (unsigned char *)malloc((size_t)(strlen((const char *)source) + 1));
	(void) strcpy ((char *)newsource, (const char *)source);
	s1 = (unsigned char *)strtok((char *)newsource, ":");

	while (strncmp((char *)s1, (char *)pat, strlen((const char *)s1))) {
		s1 = (unsigned char *)strtok((char *)0, ":");
		count++;
	}

	free ((void *)newsource);

	if (!s1)
		return 0;
	else
		return count;
}


/*
 * NAME: setparam
 *
 * FUNCTION: Sets parameters recursively.
 *
 * NOTE:     doscan() does scan and set parameters.
 * 
 * RETURN VALUE DESCRIPTION: the number of matches found between 
 *           the format and the input string. 
 */
static int
setparam(env, start, tm, rest)
unsigned char *env, *start, *rest;	/*----  the environment variable  ----*/
               			/*----  the input string  ----*/
				/*----  the format specification  ----*/
struct tm *tm;
{
	extern unsigned char **savestrp;
	int num = 1;

	if (env[0] != '*')
		(void) doscan(start, env, tm);
	else
		num = doscan(start, env + 1, tm);

	num += doscan (*savestrp, rest, tm);
	if (rest)
		++finish;

	return(num);
}


/*
 * NAME: setgen
 *
 * FUNCTION: Set environment and get fields.
 *
 * RETURN VALUE DESCRIPTION: None
 */
static void
setgen(ep, start, field)
unsigned char *ep;	/*----  the environment string  ----*/
int *field;	/*----  the field  ----*/
unsigned char *start;	/*----  the input string  ----*/
{
	unsigned char *env;

	env = (unsigned char *)NLgetenv(ep);

	*field = getfield(env, start);
}

/*
 * NAME: setampm
 *
 * FUNCTION: Sets am or pm.
 *
 * RETURN VALUE DESCRIPTION: None
 */
static void
setampm(start, tm)
struct tm *tm;
unsigned char *start;
{
	unsigned char *miscenv, *amex, *pmex, *s1;
	register int i;

	miscenv = (unsigned char *)NLgetenv("NLTMISC");
	s1 = (unsigned char *)malloc(strlen((const char *)miscenv) + 1);
	if (s1 == (unsigned char *)0)
		return;
	strcpy((char *)s1, (char *)miscenv);
	miscenv = s1;
	(void) strtok((char *)miscenv, ":");
	for (i=1; i<=4; i++) 
		(void) strtok((char *)0, ":");
	amex = (unsigned char *)strtok((char *)0, ":");
	pmex = (unsigned char *)strtok((char *)0, ":");

	if (!strncmp((const char *)amex, (const char *)start, (size_t)2)) 
		tm->tm_hour %= 12;
	else if (!strncmp((const char *)pmex, (const char *)start, (size_t)2))
		if (tm->tm_hour != 12) 
			tm->tm_hour += 12;
	free(s1);
}

/*
 * NAME: setzone
 *
 * FUNCTION: Sets zone.
 *
 * RETURN VALUE DESCRIPTION: None
 */
static void
setzone(start, tm)
unsigned char *start;	/*----  the input string  ----*/
struct tm *tm;
{
#ifdef KJI
	tzset();

	if (!strcmp(start, tzname[0]))
		tm->tm_isdst = 0;
	else if (!strcmp(start, tzname[1]))
		tm->tm_isdst = 1;
#else
	unsigned char *env;
	int envlen;

	env = (unsigned char *)getenv("TZ");
	envlen = strlen((const char *)env);

	if (!strncmp((const char *)start, (const char *)env, (size_t)3))
		tm->tm_isdst = 0;
	else if (envlen > 3 && !strncmp((const char *)start, (const char *)env + envlen - 3, (size_t)3))
		tm->tm_isdst = 1;
#endif
}

#ifdef KJI 
l_getshowa(ep, start, field)
char *ep;
char *start;
int *field;

{
	/* This routine searches the NLYEAR string for an entry with
	   matching era name (give era's names unless there is only
	   one entry...); when found, set switch and move the starting
	   year of the era into tm_year, for later use by %Jy.
	 */
	register int n, n1, n2;
	char *s1, *s2, *s3, *s4;

	s1 = NLgetenv(ep);
	if (s1 == 0)
		return (0);
	s3 = malloc(strlen(s1) + 1);
	(void) strcpy(s3, s1);
	s1 = strtok(s3, ":");
	if (s1 == 0)
		s1 = s3;
	do {
	    s2 = s1;
	    s4 = strchr(s2, ',');
	    if (!s4) {
		free (s3);
		return (0);
	    }
            ++s4;
	    if (strncmp(s4, start,strlen(s4)) == 0) {
	    	if (strncmp(s2, "-", 1) == 0) {
			++s2;
			n2 = 1;
		} else  n2 = 0;
	    	for (n = 0, n1 = 0; n1 < 4; n1++)
			n = n*10 + *s2++ - '0';
	    	if (n2)	n = -n;	
		*field = n;
		free (s3);
		return (1);
	    }
	}
	while (s1 = strtok(0, ":"));
	free (s3);
	return(0);
}
#endif

static char *
#ifdef _NO_PROTO
static_strtok(s1, s2)
char *s1;
const char *s2;
#else /* _NO_PROTO */
static_strtok(char *s1, const char *s2)
#endif /* _NO_PROTO */
{
	char	*p, *q, *r;
	static char	*savept;

	/*first or subsequent call*/
	p = (s1 == NULL)? savept: s1;

	if(p == 0)		/* return if no tokens remaining */
		return(NULL);

	q = p + strspn(p, s2);	/* skip leading separators */

	if(*q == '\0')		/* return if no tokens remaining */
		return(NULL);

	if((r = strpbrk(q, s2)) == NULL)	/* move past token */
		savept = 0;	/* indicate this is last token */
	else {
		/* skip over escaped separators */
		while (*(r-1) == '\\') {
			strcpy(r-1, r);
			if((r = strpbrk(r, s2)) == NULL) {
				savept = 0;
				return (q);
			}
		}
		*r = '\0';
		savept = ++r;
	}
	return(q);
}

