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
static char	*sccsid = "@(#)$RCSfile: strtoul.c,v $ $Revision: 4.2.8.2 $ (DEC) $Date: 1993/06/07 19:45:16 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 */ 
/*
 * COMPONENT_NAME: LIBCCNV strtoul
 *
 * FUNCTIONS: strtoul
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * strtoul.c   1.10  com/lib/c/cnv,3.1,9021 12/7/89 20:35:12";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

#ifdef WCHAR
#	define ISSPACE  iswspace
#	define ISALNUM  iswalnum
#	define TYPE     wchar_t
#	define UTYPE	wchar_t
#	define NAME     wcstoul
#else
#	define ISSPACE  isspace
#	define ISALNUM  isalnum
#	define TYPE     char
#	define UTYPE	unsigned char
#	define NAME     strtoul
#endif

#define MBASE 36

extern int __ASCII_to_digit(int);

/*
 * NAME: strtoul
 *                                                                    
 * FUNCTION: Converts a string to an integer
 *                                                                    
 * RETURNS: returns an unsigned long integer formed from *nptr
 *	    returns 0 if an integer could not be formed
 *	    ULONG_MAX if overflow occurs on conversion
 *
 */


unsigned long int
NAME (const TYPE *nptr, TYPE * *endptr, int base)
{
	unsigned long val;
	int xx;
	int signchar = 0;      /* true if a "+" precedes the number */
	int digits = 0;        /* number of valid digits read */
	int overflag = 0;      /* overflow check */
	unsigned long maxval;

	UTYPE *ptr = (UTYPE *)nptr;

	val = 0L;
        if(base < 0 || base > MBASE) {
                _Seterrno(EINVAL);
	} else {
		while(ISSPACE(*ptr))
			++ptr;

		if(*ptr == '+') 
		  	++ptr;
		else if(*ptr == '-'){
		        ++ptr;
		  	signchar = 1;
		}

		if(base == 0) {
			if(*ptr == '0') {
				++ptr;

				if(*ptr == 'x' || *ptr == 'X') {
					++ptr;
					base = 16;
				} else {
					base = 8;
					++digits;
				       }
			} else
				base = 10;
		} else if(base == 16)
			if(ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X'))
				ptr += 2;
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
        	 */
          	 
		maxval = ULONG_MAX / (unsigned long)base;
		while(ISALNUM((int)*ptr) && (xx=__ASCII_to_digit(*ptr)) < base) {
			if ((val > maxval) || (base*val + xx < val)) {
				val = ULONG_MAX;
				_Seterrno(ERANGE);
				overflag++;
				break;
			}
			val = base*val + xx;
			++ptr;
			++digits;
		}
		/*
		 * If overflow occurred, keep scanning characters because
		 * the endptr is the first character that is not consistant
		 * with the base radix.
		 */
		if (overflag) {
		    while(ISALNUM(*ptr) && (__ASCII_to_digit(*ptr) < base))
		          ++ptr;
		}
    	}
	if(signchar == 1)
		if (val > (unsigned long) LONG_MIN)  /* warning! 2's compliment dependency! */
		{
		        val = ULONG_MAX;
			_Seterrno(ERANGE);
			overflag++;
		}
		else
		{
			val = (unsigned long) - (long) val;
		}

	if(endptr != NULL) {
	    if ( !digits )
	      *endptr = (TYPE *)nptr;
	    else
	      *endptr = (TYPE *)ptr;
	}

	return(val);
}
