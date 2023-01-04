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
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: doscan.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:21:21 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "doscan.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:14:06";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _doscan, number, string, setup, __dsdummy__
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#ifdef NEEDDOSCAN
#include <stdio.h>
#include <ctype.h>
#include <varargs.h>
#include <values.h>

#define NCHARS	(1 << BITSPERBYTE)

extern double atof();
extern char *memset();
extern int ungetc();

/* 3 lines added for declare function type - nabe */
static int number();
static int string();
static unsigned char *setup();

/*
 * NAME:        _doscan
 *
 * EXECUTION ENVIRONMENT:
 *
 *      doscan is not documented, so people feel they have license to
 *      change the parameters.  The 3B implementation does not even look
 *      callable from C, and without documentation is not comprehensible.
 *      Since many systems have their own doprnt which is written in
 *      assembly language, and thus faster, but callable from C, we try
 *      to use the standard system version where possible.  The ifdefs
 *      below attempt to decide where it is possible - it may need to be
 *      updated later.
 */

int
_doscan(iop, fmt, va_alist)
register FILE *iop;
register unsigned char *fmt;
va_list va_alist;
{
/* comment out - nabe
	extern unsigned char *setup();
*/
	char tab[NCHARS];
	register int ch;
	int nmatch = 0, len, inchar, stow, size;

	/*******************************************************
	 * Main loop: reads format to determine a pattern,
	 *		and then goes to read input stream
	 *		in attempt to match the pattern.
	 *******************************************************/
	for( ; ; ) {
		if((ch = *fmt++) == '\0')
			return(nmatch); /* end of format */
		if(isspace(ch)) {
			while(isspace(inchar = getc(iop)))
				;
			if(ungetc(inchar, iop) != EOF)
				continue;
			break;
		}
		if(ch != '%' || (ch = *fmt++) == '%') {
			if((inchar = getc(iop)) == ch)
				continue;
			if(ungetc(inchar, iop) != EOF)
				return(nmatch); /* failed to match input */
			break;
		}
		if(ch == '*') {
			stow = 0;
			ch = *fmt++;
		} else
			stow = 1;

		for(len = 0; isdigit(ch); ch = *fmt++)
			len = len * 10 + ch - '0';
		if(len == 0)
			len = MAXINT;

		if((size = ch) == 'l' || size == 'h')
			ch = *fmt++;
		if(ch == '\0' ||
		    ch == '[' && (fmt = setup(fmt, tab)) == NULL)
			return(EOF); /* unexpected end of format */
		if(isupper(ch)) { /* no longer documented */
			size = 'l';
			ch = _tolower(ch);
		}
		if(ch != 'c' && ch != '[') {
			while(isspace(inchar = getc(iop)))
				;
			if(ungetc(inchar, iop) == EOF)
				break;
		}
		if((size = (ch == 'c' || ch == 's' || ch == '[') ?
		    string(stow, ch, len, tab, iop, &va_alist) :
		    number(stow, ch, len, size, iop, &va_alist)) != 0)
			nmatch += stow;
		if(va_alist == NULL) /* end of input */
			break;
		if(size == 0)
			return(nmatch); /* failed to match input */
	}
	return(nmatch != 0 ? nmatch : EOF); /* end of input */
}

/*
 * NAME:        number
 *
 * FUNCTION:
 *
 *      Functions to read the input stream in an attempt to match incoming
 *      data to the current pattern from the main loop of _doscan().
 */

static int
number(stow, type, len, size, iop, listp)
int stow, type, len, size;
register FILE *iop;
va_list *listp;
{
	char numbuf[64];
	register char *np = numbuf;
	register int c, base;
	int digitseen = 0, dotseen = 0, expseen = 0, floater = 0, negflg = 0;
	long lcval = 0;

	switch(type) {
	case 'e':
	case 'f':
	case 'g':
		floater++;
	case 'd':
	case 'u':
		base = 10;
		break;
	case 'o':
		base = 8;
		break;
	case 'x':
		base = 16;
		break;
	default:
		return(0); /* unrecognized conversion character */
	}
	switch(c = getc(iop)) {
	case '-':
		negflg++;
	case '+': /* fall-through */
		len--;
		c = getc(iop);
	}
	for( ; --len >= 0; *np++ = c, c = getc(iop)) {
		if(isdigit(c) || base == 16 && isxdigit(c)) {
			int digit = c - (isdigit(c) ? '0' :
			    isupper(c) ? 'A' - 10 : 'a' - 10);
			if(digit >= base)
				break;
			if(stow && !floater)
				lcval = base * lcval + digit;
			digitseen++;
			continue;
		}
		if(!floater)
			break;
		if(c == '.' && !dotseen++)
			continue;
		if((c == 'e' || c == 'E') && digitseen && !expseen++) {
			*np++ = c;
			c = getc(iop);
			if(isdigit(c) || c == '+' || c == '-')
				continue;
		}
		break;
	}
	if(stow && digitseen)
		if(floater) {
			register double dval;
	
			*np = '\0';
			dval = atof(numbuf);
			if(negflg)
				dval = -dval;
			if(size == 'l')
				*va_arg(*listp, double *) = dval;
			else
				*va_arg(*listp, float *) = (float)dval;
		} else {
			/* suppress possible overflow on 2's-comp negation */
			if(negflg && lcval != HIBITL)
				lcval = -lcval;
			if(size == 'l')
				*va_arg(*listp, long *) = lcval;
			else if(size == 'h')
				*va_arg(*listp, _SHORT *) = (_SHORT)lcval;
			else
				*va_arg(*listp, int *) = (int)lcval;
		}
	if(ungetc(c, iop) == EOF)
		*listp = NULL; /* end of input */
	return(digitseen); /* successful match if non-zero */
}

/*
 * NAME:        string
 */

static int
string(stow, type, len, tab, iop, listp)
register int stow, type, len;
register char *tab;
register FILE *iop;
va_list *listp;
{
	register int ch;
	register char *ptr;
	char *start;

	start = ptr = stow ? va_arg(*listp, char *) : NULL;
	if(type == 'c' && len == MAXINT)
		len = 1;
	while((ch = getc(iop)) != EOF &&
	    !(type == 's' && isspace(ch) || type == '[' && tab[ch])) {
		if(stow)
			*ptr = ch;
		ptr++;
		if(--len <= 0)
			break;
	}
	if(ch == EOF || len > 0 && ungetc(ch, iop) == EOF)
		*listp = NULL; /* end of input */
	if(ptr == start)
		return(0); /* no match */
	if(stow && type != 'c')
		*ptr = '\0';
	return(1); /* successful match */
}

/*
 * NAME:        setup
 */

static unsigned char *
setup(fmt, tab)
register unsigned char *fmt;
register char *tab;
{
	register int b, c, d, t = 0;

	if(*fmt == '^') {
		t++;
		fmt++;
	}
	(void)memset(tab, !t, NCHARS);
	if((c = *fmt) == ']' || c == '-') { /* first char is special */
		tab[c] = t;
		fmt++;
	}
	while((c = *fmt++) != ']') {
		if(c == '\0')
			return(NULL); /* unexpected end of format */
		if(c == '-' && (d = *fmt) != ']' && (b = fmt[-2]) < d) {
			(void)memset(&tab[b], t, d - b);
			fmt++;
		} else
			tab[c] = t;
	}
	return(fmt);
}
#else

/*
 * NAME:        __dsdummy__
 *
 * FUNCTION:
 *
 *      This is just to keep from getting a diagnostic from ranlib.
 */

__dsdummy__()
{
}
#endif /* NEEDDOSCAN */
