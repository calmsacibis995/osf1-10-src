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
static char	*sccsid = "@(#)$RCSfile: atol.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/06/08 01:26:02 $";
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
 * COMPONENT_NAME: LIBCCNV atol
 *
 * FUNCTIONS: atol
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *  "atol.c    1.11  com/lib/c/cnv,3.1,9021 5/1/90 09:43:33";
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <limits.h>
#include <errno.h>
#include <math.h>

/*
 * NAME: atol
 *                                                                    
 * FUNCTION: Converts a string to an long
 *                                                                    
 * NOTES:
 *
 * RETURNS: returns a long formed from *nptr
 *	    LONG_MIN or LONG_MAX of overflow occurs on conversion
 *
 */

long int
atol(const char *nptr)
{
	register long n;
	register int sign;

	n = 0;
	sign = 1;
	for(;;nptr++) {
		switch(*nptr) {
		case ' ':		/* check for whitespace */
		case '\t':
		case '\f':
		case '\n':
		case '\v':
		case '\r':
			continue;
		case '-':
			sign = -1;
		case '+':
			nptr++;
		}
		break;
	}
	while(*nptr >= '0' && *nptr <= '9') {
 	      /*
		Check for overflow.  If the current value (before
 	   	adding current digit) is already greater than
           	LONG_MAX / 10, we know that another digit will
 	   	not fit.  Also if after the current digit is added,
           	if the new value is less than the old value, we 
 	   	know that overflow will occur.
              */
 		if (((n * 10 + *nptr - '0') < n) || (n > (LONG_MAX/10))) {
			if (sign == 1)
				n = LONG_MAX;
			else
				n = LONG_MIN;
			_Seterrno(ERANGE);
			return(n);
		}
		n = n * 10 + *nptr++ - '0';
	}
	n *= sign;
	return (n);
}
