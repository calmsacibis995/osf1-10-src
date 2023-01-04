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
static char	*sccsid = "@(#)$RCSfile: DStemp.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:24:39 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * COMPONENT_NAME: (LIBCIO) Standard C Library I/O Functions 
 *
 * FUNCTIONS: __ito6, __pidnid, __lucky
 *
 * ORIGINS: 3, 27 
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
 * DStemp.c	1.7  com/lib/c/io,3.1,8943 9/9/89 12:41:06
 */

#include <stdio.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/times.h>

/*
 * These routines support the Distributed Services modifications
 * to tmpnam(3), tempnam(3), and mktemp(3).
 */

/*
 * __ito6 --	Converts integers to strings of characters in a 6-bit
 *		character set (in which all characters are printable).
 *
 *		Called by __pidnid() and __lucky().
 */
char *
__ito6(num, len)
register unsigned long num;
int len;	/* length of string desired */
{
#define	ABC "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_%"
	register int i;
	static char buf[7];

	buf[6] = '\0';
	for (i = 5; i >= (6 - len); i--)
	{
		buf[i] = ABC[num & 0x3f];
		num >>= 6;
	}
	return(&buf[i+1]);
}

/*
 * __pidnid --	Encodes the process' pid, nid, and an incrementing
 *		selector as a 10-character, null-terminated string of
 *		printable characters.
 *
 *		Called by tmpnam(3) and tempnam(3).
 */
char *
__pidnid()
{
	static char buf[11];
	static unsigned int selector = 0;

	/*
	 * The characters in the encoded string are designated
	 * as follows:
	 *	pid (16 bits) =>	3 6-bit chars (18 bits)
	 *	nid (low 12 bits) =>	2 6-bit chars (12 bits)
	 *	selector (arbitrary) =>	5 6-bit chars (30 bits)
	 */

	strcpy(buf, __ito6(getpid(), 3));
	strcat(buf, __ito6(gethostid() & 0xfff, 2));
	strcat(buf, __ito6(selector++, 5));
	buf[10] = '\0';
	selector &= 0x3ffffff;
	return(buf);
}

/*
 * __lucky --	Encodes the process' pid, a pseudo-random number,
 *		and an incrementing selector as a 6-character, null-
 *		terminated string of printable characters.
 *
 *		Called by mktemp(3).
 */
char *
__lucky()
{
	unsigned long random;
	unsigned short seed[3];
	unsigned long mung;
	struct tms tms;
	static char buf[7];
	static unsigned int selector = 0;

	/*
	 * 36 bits are used to create a 6-character suffix, according
	 * to the definitions below.
	 */
#define PIDBITS		16	/* from user's pid */
#define	SELBITS		6	/* from static selector */
#define	RANDBITS	14	/* from pseudo-random number */

#define	FRONTBITS	30
#define	BACKBITS	(36 - FRONTBITS)
#define	PIDSHIFT	(FRONTBITS - PIDBITS)
#define	SELSHIFT	(PIDSHIFT - SELBITS)
#define	RANDSHIFT	(RANDBITS - SELSHIFT)
#define	SELMASK		((1 << SELBITS) - 1)
#define RANDMASK	((1 << RANDBITS) - 1)
#define	BACKMASK	((1 << BACKBITS) - 1)

	/*
	 * Seed the random number generator with this process' user
	 * and system times, and use the top of the result for the
	 * random bit field.
	 */
	times(&tms);
	seed[0] = tms.tms_utime >> 16;
	seed[1] = tms.tms_utime;
	seed[2] = tms.tms_stime;
	random = jrand48(seed);
	random = (random >> (32 - RANDBITS)) & RANDMASK;

	/*
	 * Since we can't call __ito6() with a 36-bit number, we
	 * call it twice, once with FRONTBITS bits and once with 
	 * BACKBITS.
	 */
	mung = getpid() << PIDSHIFT;
	mung |= (selector++ & SELMASK) << SELSHIFT;
	mung |= random >> RANDSHIFT;
	strcpy(buf, __ito6(mung, FRONTBITS / 6));
	strcat(buf, __ito6(random & BACKMASK, BACKBITS / 6));

	buf[6] = '\0';
	selector &= SELMASK;

	return(buf);
}
