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
static char	*sccsid = "@(#)$RCSfile: wsto2fp.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 20:47:01 $";
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
/*
 * RESTRICTED RIGHTS LEGEND
 * Use, Duplication or Disclosure by the Government is subject to
 * restrictions as set forth in paragraph (b)(3)(B) of the rights in
 * Technical Data and Computer Software clause in DAR 7-104.9(a).
 */ 

/*
 * COMPONENT_NAME: LIBCCNV dsto2fp
 *
 * FUNCTIONS: dsto2fp, checknf, checknan
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <float.h>
#include <ctype.h>
#include <math.h>
#include <machine/endian.h>

/*
 *      Macro to multiply the current pair of fp numbers by 10.0 and
 *      add the new character.
 */
#define ACCUM d = ch[c & 0xf];	\
	t = ten * h + d;	\
	h = (ten * h - t) + d;	\
	l = ten * l + h;	\
	h = t

static double ch[10] = {
	0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};

/*
 * NAME: dsto2fp
 *                                                                    
 * FUNCTION: convert decimal string to 2 fp numbers (hi part & low part)
 *                                                                    
 * NOTES:
 *
 * dsto2fp - convert decimal string to 2 fp numbers (hi part & low part)
 *
 *      Called with p -     pntr to ascii string
 *                  dbl -   pntr to array 2 doubles.
 *                  expon - pntr to the exponent returned.
 *                  class - pointer to result class
 *                          The class value is the same as returned by
 *                          the class() function except norms and denorms
 *                          both return FP_PLUS_NORM.
 *
 *      Returns pointer to character that terminates scan.
 *
 *      A valid decimal string has the following form:
 *
 *      ( {<digit>} [.{<digit>}] [(e|E) [+|-|<sp>] {<digit>} ] | "NaNS" |
 *      "NaNQ" | "INF" )
 *
 *      {} = 0 or more times
 *      [] = 0 or 1 times
 *
 *
 * RETURNS: 
 *      Returns pointer to character that terminates scan.
 */
#ifdef KJI

NLchar *wdsto2fp (p, dbl, expon, class)
NLchar   *p;
double dbl[2];
int    *expon;
int    *class;

{
	int    strncmp();
	NLchar   *s = p;        /* current character pointer                  */
	NLchar   *Eptr;         /* start of exponent string                   */
	NLchar   c;             /* current character holder                   */
	double d, h, l, t;    /* temps used to calculate the double results */
	double ten = 10.0;    /* constant 10.0                              */
	int    expsign = 1;   /* sign of exponent (+1 = '+'; -1 = '-')      */
	int    ct = 0;        /* number of significant digits collected     */
	int    expct = 0;     /* number of significant digits in exponent   */
	unsigned int exp = 0; /* holds the converted exponent               */
	int    nanret, ret = 0;

	*expon = 0;           /* start out with exponent = 0                */
	*class = FP_PLUS_NORM; /* assume number will be normalized fp       */

	h = 0.0;              /* init the high & low part of result to +0.0 */
	l = 0.0;

	c = *s++;             /* get the first character */
	while (isdigit(c = isascii(c)? c: _jistoa(c)))    /* while in front of the decimal point */
	{
		/* Multiply the previous value by 10 and add in the new digit */
		if (ct < 30) { 
			ACCUM; 
		} 
		else 
			(*expon)++;
		if ((c != '0') || ct ) 
			ct++;
		c = *s++;

	}
	/* Here on decimal point, e, E, Q, S, I, or end of string */
	/* If decimal point, get the stuff that's after it.       */
	if ( c == '.') {
		c = *s++;
		while ( isdigit(c = isascii(c)? c: _jistoa(c)) ) {
			/* Multiply the previous value by 10 and add 
			 * in the new digit  */
			if (ct < 30) {
				ACCUM;
				(*expon)--;    /* decrement the exponent   */
			}
			if ((c != '0') || ct ) 
				ct++;
			c = *s++;
		}  /* end while */
	}  /* end if */
	/* Get the exponent if there is one */
	if ( ((c == 'e') || (c == 'E')) && ( p != s-1)) {
		Eptr = s-1;      /* mark string start in case exp is invalid */
                c = *s++;
                c = isascii(c)? c: _jistoa(c);
		switch (c) {
		case '-': 
			expsign = -1;
		case '+': 
			c = *s++;
                        c = isascii(c)? c: _jistoa(c);
		}
		while ( isdigit(c = isascii(c)? c: _jistoa(c)) ) {
			exp = (exp*10) + (c & 0xf);
			c = *s++;
                        c = isascii(c)? c: _jistoa(c);
#ifdef JUNK
			if (exp > 0) {		/* Skip leading zeros */
				expct++;
				if (expct > 3)
					break;
			}
#endif
		}
	}

  /* If there are more than 3 significant digits in the exponent, an overflow
   * or an underflow will occur.  Therefore, we set exp to be 500
   * and return that value so that the routines that call this one 
   * can take the appropriate action.
   */

	if (expct > 3)
		exp = 500;

	*expon += exp*expsign;

	/* If we are still at 1st char, look for the IEEE special strings. */
	if ( (s-1) == p) {
		switch (c) {
		case 'n': 
		case 'N': 
			nanret = wchecknan(p);
			if (nanret == 1) {		/* NaNQ */
				h = DBL_QNAN;
				*class = FP_QNAN;
				s +=4; 
			}
			else if (nanret == 2) {		/* NaNS */
				h = DBL_SNAN;
				*class = FP_SNAN;
				s +=4; 
			}
			break;
		case 'i': 
		case 'I': 
			if ((ret = wchecknf(p)) != 0) {
				h = DBL_INFINITY;
				*class = FP_PLUS_INF;
				s += ret; 
			}
			break;
		}
	}

	if (h == 0.0) *class = FP_PLUS_ZERO;  /* if hi==0 then whole nbr == 0 */

#if BYTE_ORDER == LITTLE_ENDIAN
	dbl[0] = h;           /* Result = h and l   */
	dbl[1] = l;           /* Return in memory */
#else	/* BYTE_ORDER == LITTLE_ENDIAN */
	dbl[1] = h;           /* Result = h and l   */
	dbl[0] = l;           /* Return in memory */
#endif	/* BYTE_ORDER == LITTLE_ENDIAN */

	return (s-1);         /* Return value is ptr to char that ended scan */
}

/*
 * NAME: checknf
 *                                                                    
 * FUNCTION: check to see in string is infinity
 *                                                                    
 * NOTES:
 *	The check for infinity is case insensitive
 *
 * RETURNS: 1 for infinity string; 0 otherwise
 *
 */

int wchecknf(bp)
register NLchar *bp;
{
register NLchar c;
	*bp++;
        c = isascii(*bp)? *bp: _jistoa(*bp);
	if ((c == 'n') || (c == 'N'))
	{   *bp++;
            c = isascii(*bp)? *bp: _jistoa(*bp);
	    if ((c == 'f') || (c == 'F'))
	    {   
		return (1);
	    }
	}

	return (0);
}

/*
 * NAME: checknan
 *                                                                    
 * FUNCTION: check to see in string is NaNQ or NaNS
 *                                                                    
 * NOTES:
 *	The check for NaN is case insensitive
 *
 * RETURNS: 1 for NaNQ string; 2 for NaNS or NaN strings; 0 otherwise
 *
 */
int wchecknan(bp)
register NLchar *bp;
{
register NLchar c;
	*bp++;
        c = isascii(*bp)? *bp: _jistoa(*bp);
	if ((c == 'a') || (c == 'A'))
	{	*bp++;
                c = isascii(*bp)? *bp: _jistoa(*bp);
		if ((c == 'n') || (c == 'N'))
		{	*bp++;
                        c = isascii(*bp)? *bp: _jistoa(*bp);
			if ((c == 'q') || (c == 'Q')) {
				/* NanQ */
				return (1);
			}
			/* anything else is treated as a NaNS */
			return (2);
		}
	}

	return(0);
}

#endif /* end of KJI */
