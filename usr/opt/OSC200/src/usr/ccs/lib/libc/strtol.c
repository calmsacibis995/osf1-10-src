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
#if !defined(lint) && !defined(WCHAR)
static char	*sccsid = "@(#)$RCSfile: strtol.c,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/06/07 19:45:00 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * COMPONENT_NAME: LIBCCNV strtol
 *
 * FUNCTIONS: strtol
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * strtol.c	1.19  com/lib/c/cnv,3.1,9013 2/26/90 21:55:08
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#define MBASE 36

#ifdef WCHAR
#	define ISSPACE  iswspace
#	define ISALNUM  iswalnum
#	define TYPE     wchar_t
#	define UTYPE	wchar_t
#	define NAME     wcstol
#else
#	define ISSPACE  isspace
#	define ISALNUM  isalnum
#	define TYPE     char
#	define UTYPE	unsigned char
#	define NAME     strtol


int
__ASCII_to_digit(int x)
{
	if( (x >= '0') && (x <= '9'))
		return (x - '0');
	if ( (x >= 'a') && (x <= 'z'))
		return (10 + x - 'a');
	if ( (x >= 'A') && (x <= 'Z'))
		return (10 + x - 'A');
	return (MBASE+1);
}
#endif	/* WCHAR */


/*
 * NAME: strtol
 *                                                                    
 * FUNCTION: Converts a string to an integer
 *                                                                    
 * RETURNS: returns an long integer formed from *nptr
 *	    returns 0 if an integer could not be formed
 *	    ULONG_MAX if overflow occurs on conversion
 *
 */


long int
NAME (const TYPE *fnptr, TYPE * *endptr, int base)
{
        UTYPE *nptr = (UTYPE *)fnptr;
	TYPE *orig;
	unsigned long val;
	int xx, sign;
	int digits = 0;        /* number of valid digits read */
	int overflag = 0;
 	unsigned long maxval;

	val = 0L;
	sign = 1;
	if(base < 0 || base > MBASE) {
		_Seterrno(EINVAL);
	} else {
		/*
		 * If nptr is NULL, return 0, leave endptr alone.
		 */
		if (nptr)
			orig = (TYPE *)nptr;
		else
			return(0L);
		while (ISSPACE(*nptr))
			++nptr;
		if (*nptr == '-') {
			++nptr;
			sign = -1;
		} else if (*nptr == '+') {
		  	++nptr;
	       	}
		if (base == 0) {
			if (*nptr == '0') {
				++nptr;
				if (*nptr == 'x' || *nptr == 'X') {
					++nptr;
					base = 16;
				} else {
					base = 8;
					++digits;
				}
			} else
				base = 10;
		} else if (base == 16)
			if (nptr[0] == '0' && (nptr[1] == 'x'||nptr[1] == 'X'))
				nptr += 2;
		/*
	 	 * for any base > 10, the digits incrementally following
	 	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 	 */

 		/* Check for overflow.  If the current value (before
 	   	 * adding current xx digit) is already greater than
           	 * ULONG_MAX / base, we know that another digit will
 	   	 * not fit.  Also if after the current digit is added,
           	 * if the new value is less than the old value, we 
 	   	 * know that overflow will occur.
		 * --except for the special case of LONG_MIN, which will
		 *   overflow in our positive accumulation, but nevertheless
		 *   is a valid value, which we must cater to.
         	 */
 		maxval = ULONG_MAX / (long)base;
		while(ISALNUM(*nptr) && (xx=__ASCII_to_digit(*nptr)) < base) {
 			if ((val > maxval) || 
			    (((long)(base*val + xx) < (long)val) && 
			     ! (((long) (base*val + xx) == LONG_MIN) &&
			      (sign == -1)))) {
				overflag = 1;
				if (sign == 1)
					val = LONG_MAX;
				else
					val = LONG_MIN;
				sign = 1;
				_Seterrno(ERANGE);
				break;
			}
			val = base*val + xx;
			++nptr;
			++digits;
		}
		/* If overflow occurred, keep scanning characters because
		 * the endptr is the first character that is not consistant
		 * with the base.
		 */
		if (overflag) {
			while(ISALNUM(*nptr) && (__ASCII_to_digit(*nptr) < base))
				++nptr;
		}
	}
        if(endptr)
            {
            /*
             * If endptr is not NULL, and if nptr is not NULL, then
             * nptr has an invalid final string.  If there is no valid
             * subject sequence, return unsigned 0 and nptr in *endptr.
             * Else return the invalid final string in *endptr.
             */
            if(!digits)
                {
                *endptr = orig;
                sign = 1;
                }
            else
                {
                *endptr = (TYPE *)nptr;
                }
            }
        else
            {
            /*
             * If *endptr is NULL, and there is no valid subject
             * sequence, then return unsigned 0.
             */
            if(!digits)
              sign = 1;
            }
	return(sign*val);
}
