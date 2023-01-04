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
static char	*sccsid = "@(#)$RCSfile: l64a.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/07/20 21:27:46 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.2
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: LIBCCNV l64a
 *
 * FUNCTIONS: l64a
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1988, 1989
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * l64a.c	1.4  com/lib/c/cnv,3.1,8943 9/13/89 09:24:26
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#if !defined(_THREAD_SAFE)
#pragma weak l64a = __l64a
#else
#define	l64a_r	__l64a_r
#pragma	weak l64a_r = __l64a_r
#endif
#endif
#include <values.h>

#include "ts_supp.h"

#define BITSPERCHAR	6 /* to hold entire character set */
#define BITSPERLONG	(BITSPERBYTE * sizeof(long))
#define NMAX		((BITSPERLONG + BITSPERCHAR - 1)/BITSPERCHAR)
#define SIGN		(-(1L << (BITSPERLONG - BITSPERCHAR - 1)))
#define CHARMASK	((1 << BITSPERCHAR) - 1)
#define WORDMASK	((1L << ((NMAX - 1) * BITSPERCHAR)) - 1)

/*
 * NAME: l64a
 *                                                                    
 * FUNCTION: convert long int to base 64 ascii
 *                                                                    
 * NOTES:
 * convert long int to base 64 ascii
 * char set is [./0-9A-Za-z]
 * two's complement negatives are assumed,
 * but no assumptions are made about sign propagation on right shift
 *
 * RETURNS: a character string
 */
#ifdef _THREAD_SAFE
int
l64a_r(register long lg, char *buf, int len)
#else
char *
l64a(register long lg)
#endif	/* _THREAD_SAFE */
{
#ifndef _THREAD_SAFE
	static char	buf[NMAX + 1];
#endif	/* _THREAD_SAFE */
	register char	*s = buf;

	TS_EINVAL((buf == 0 || len <= NMAX));

	while (lg != 0) {

		register int c = ((int)lg & CHARMASK) + ('0' - 2);

		if (c > '9')
			c += 'A' - '9' - 1;
		if (c > 'Z')
			c += 'a' - 'Z' - 1;
		*s++ = c;
		/* fill high-order CHAR if negative */
		/* but suppress sign propagation */
		lg = ((lg < 0) ? (lg >> BITSPERCHAR) | SIGN :
			lg >> BITSPERCHAR) & WORDMASK;
	}
	*s = '\0';
	return (TS_FOUND(buf));
}
