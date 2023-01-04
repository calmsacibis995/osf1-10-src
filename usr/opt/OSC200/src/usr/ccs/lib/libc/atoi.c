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
static char	*sccsid = "@(#)$RCSfile: atoi.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/06/08 01:25:57 $";
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
 * COMPONENT_NAME: LIBCCNV atoi
 *
 * FUNCTIONS: atoi
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * atoi.c    1.11  com/lib/c/cnv,3.1,9021 5/1/90 09:43:19
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <math.h>

/*
 * NAME: atoi
 *                                                                    
 * FUNCTION: Converts a string to an integer
 *                                                                    
 * NOTES:
 *
 * RETURNS: returns an integer formed from *nptr
 *	    INT_MIN or INT_MAX if overflow occurs on conversion
 *
 */

int 	
atoi(const char *nptr)
{
	int n, sign;

	n = 0;
	sign = 0;
	while (isspace(*nptr)) nptr++;	/* skip leading whitespace */
	switch (*nptr) {
	case '-':
		sign++;
		/* Fall Through */
	case '+':
		nptr++;
	}

	while (isdigit(*nptr)) {
 	      /*
		Check for overflow.  If the current value (before
 	   	adding current digit) is already greater than
           	INT_MAX / 10, we know that another digit will
 	   	not fit.  Also if after the current digit is added,
           	if the new value is less than the old value, we 
 	   	know that overflow will occur.
              */
 		if (((n * 10 + *nptr - '0') < n) || (n > (INT_MAX /10))) {
			n = (sign == 1) ? INT_MIN : INT_MAX;		
			_Seterrno(ERANGE);
			return (n);
		}
		n = n * 10 + *nptr++ - '0';
	}
	return(sign? -n: n);
}
