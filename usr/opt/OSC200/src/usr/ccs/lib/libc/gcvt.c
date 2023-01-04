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
static char	*sccsid = "@(#)$RCSfile: gcvt.c,v $ $Revision: 4.2.9.3 $ (DEC) $Date: 1993/06/08 13:21:22 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
/*
 * FUNCTIONS: gcvt
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * gcvt.c	1.9  com/lib/c/cnv,3.1,8943 10/27/89 16:14:03
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#pragma weak gcvt = __gcvt
#endif
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <fp.h>
#include <nl_types.h>
#include <langinfo.h>
#include "print.h"

/* TRIMZERO causes trailing zeros to be removed.  This is
 * required for bsd compatability.  The #ifdef around this
 * code has been around since sccs level 1.1, so it came
 * with the code
 */

#define TRIMZERO

/*
 * NAME: gcvt
 *                                                                    
 * FUNCTION: convert double to readable ascii string
 *                                                                    
 * NOTES:
 *
 * RETURNS: a character string
 *
 */

char *
gcvt(double number, int ndigit, char *buf)
{
	int sign, decpt;
	register char *p1, *p2 = buf;
	register int i;
	char	*radix;
	int	radixlen;
#ifdef _THREAD_SAFE
	char	lbuf[NDIG];

	ecvt_r(number, ndigit, &decpt, &sign, lbuf, NDIG);
	p1 = lbuf;
#else
	p1 = ecvt(number, ndigit, &decpt, &sign);
#endif	/* _THREAD_SAFE */
	if (sign)
		*p2++ = '-';
	/* If string returned from ecvt is not numeric then it should	*/
	/* 		be "NaNQ", "NaNS", or "INF"			*/
	if( !FINITE(number) ) {
		strcpy(p2,p1);
		return(buf);
	}

	radix = nl_langinfo(RADIXCHAR);
	if ( !radix || !(*radix))
	  	radix = ".";	/* Use "." if no other radix-point preferred */
	radixlen = strlen(radix);

	if (decpt > ndigit || decpt <= -4) {	/* E-style */
		decpt--;
		*p2++ = *p1++;

		strcpy(p2,radix);		/* insert "decimal point" */
		p2 += radixlen;

		for (i = 1; i < ndigit; i++)
			*p2++ = *p1++;
#ifdef TRIMZERO
		while (p2[-1] == '0')
			p2--;
		if (!strcmp( &p2[-radixlen], radix ))
		    	p2 -= radixlen;

#endif
		*p2++ = 'e';
		if (decpt < 0) {
			decpt = -decpt;
			*p2++ = '-';
		} else
			*p2++ = '+';
		for (i = 1000; i != 0; i /= 10) /* 3B or CRAY, for example */
			if (i <= decpt || i <= 10) /* force 2 digits */
				*p2++ = (decpt / i) % 10 + '0';
	} else {
		if (decpt <= 0) {
			*p2++ = '0';
			strcpy(p2,radix);
			p2 += radixlen;
			while (decpt < 0) {
				decpt++;
				*p2++ = '0';
			}
		}
		for (i = 1; i <= ndigit; i++) {
			*p2++ = *p1++;
			if (i == decpt) {
			  	strcpy(p2, radix);
				p2 += radixlen;
			}
		}
		if (ndigit < decpt) {
			while (ndigit++ < decpt)
				*p2++ = '0';
			strcpy(p2, radix);
			p2 += radixlen;
		}
#ifdef TRIMZERO
		while (*--p2 == '0' && p2 > buf) /* back over trailing zeros */
			;			 /*  til anything else */
		p2++;				 /* But don't back too far */

		if (!strncmp( &p2[-radixlen], radix, radixlen)) {
		    /*
		     * If it was the "radix-point string", back up
		     * to just after the last digit.
		     */
		    p2 -= radixlen;
		}
#endif
	}
	*p2 = '\0';
	return (buf);
}

