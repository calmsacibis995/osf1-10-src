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
/*	
 *	@(#)$RCSfile: endian.h,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:08:25 $
 */ 
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
 * derived from endian.h	2.1	(ULTRIX/OSF)	12/3/90
 */
/*
 * Copyright (c) 1987 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

#ifndef	_MACHINE_ENDIAN_H_
#define _MACHINE_ENDIAN_H_

/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define	LITTLE_ENDIAN	1234	/* least-significant byte first (vax) */
#define	BIG_ENDIAN	4321	/* most-significant byte first (IBM, net) */
#define	PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

/* MIPS may use either endian, compiler tells us which. */
#ifdef	__MIPSEL__
#define	BYTE_ORDER	LITTLE_ENDIAN
#undef	BYTE_MSF	/* XXX mips code continues to use this */
#else	/* __MIPSEL__ */
#ifdef	__MIPSEB__
#define	BYTE_ORDER	BIG_ENDIAN
#define	BYTE_MSF	/* XXX mips code continues to use this */
#else   /* __MIPSEB__ */
/*#error*/ Undefined MIPS byte order!
#endif
#endif

/*
 * Macros for network/external number representation conversion.
 */
#if BYTE_ORDER == BIG_ENDIAN && !defined(lint)

#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)

#define NTOHL(x)
#define NTOHS(x)
#define HTONL(x)
#define HTONS(x)

#else

#if defined(lint) || !defined(__GNUC__)

unsigned short	ntohs(), htons();
unsigned long	ntohl(), htonl();

#else	/* !lint && LITTLE_ENDIAN && __GNUC__ */

/*
 * Use GNUC support to inline the byteswappers.
 */

static __inline__
unsigned long	/* XXX "long" (and &xff's) needed for optimality */
ntohs(w)
	register unsigned long w;
{
	return ((w << 8) | ((w >> 8) & 0xff)) & 0xffff;
}
#define	htons	ntohs

static __inline__
unsigned long
ntohl(l)
	register unsigned long l;
{
	return (l << 24) | (l >> 24) | ((l & 0xff00) << 8) | ((l >> 8) & 0xff00);
}
#define htonl	ntohl

#endif	/* __GNUC__ inlines */

#define NTOHL(x)	(x) = ntohl((u_long)x)
#define NTOHS(x)	(x) = ntohs((u_short)x)
#define HTONL(x)	(x) = htonl((u_long)x)
#define HTONS(x)	(x) = htons((u_short)x)

#endif
#endif
