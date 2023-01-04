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
static char	*sccsid = "@(#)$RCSfile: wstrtol.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/08 20:47:23 $";
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
 * COMPONENT_NAME: LIBCCNV wstrtol
 *
 * FUNCTIONS: wstrtol
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
/* wstrtol.c	1.2  com/lib/c/cnv/KJI,3.1,9021 12/14/89 13:11:08 */

/*LINTLIBRARY*/
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak wstrtol = __wstrtol
#endif
#endif
#include <NLctype.h>

#ifdef KJI
#define ASCII(x) (isascii(x)? x: _jistoa(x))
#define DIGIT(x) (isdigit(ASCII(x))? ((ASCII(x))-'0'): (10+tolower(ASCII(x))-'a'))
#define MBASE 36

long
wstrtol(NLchar *str, NLchar **ptr, int base)
{
	long val;
	int xx, sign;
	int asc;
	int signchar = 0;      /* true if a "+" or "-" precedes the number */
	int digits = 0;        /* number of valid digits read */

	val = 0L;
	sign = 1;
	if(base < 0 || base > MBASE)
		goto OUT;
	while(NCisspace(*str))
		++str;
	if((asc = ASCII(*str)) == '-') {
		++str;
		sign = -1;
		signchar = 1;
	} else if(asc == '+') {
		  ++str;
		  signchar = 1;
	       }
	if(base == 0) {
		if(ASCII(*str) == '0') {
		  	++str;
			if((asc=ASCII(*str)) == 'x' || asc == 'X') {
		  		++str;
				base = 16;
			} else
				base = 8;
		} else
			base = 10;
	} else if(base == 16)
		if(ASCII(str[0]) == '0' && (asc=ASCII(str[1]) == 'x' || asc == 'X'))
			str += 2;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	while(NCisalnum(*str) && (xx=DIGIT(*str)) < base) {
		/* accumulate neg avoids surprises near maxint */
		val = base*val - xx;
		++str;
		++digits;
	}
OUT:
	if(ptr != (NLchar**)0) {
		/* If there's a "+" or "-" and no number, back up
		 * one character so the correct pointer is returned.
		 */
		if(signchar && !digits)
		     str--;
		*ptr = str;
	}
	return(sign*(-val));
}
#else	/* stub for non-KJI */
long
wstrtol(NLchar *str, NLchar **ptr, int base)
{
}
#endif /* KJI */
