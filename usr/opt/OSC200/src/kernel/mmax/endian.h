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
 *	@(#)$RCSfile: endian.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:38 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
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

#define	BYTE_ORDER	LITTLE_ENDIAN	/* byte order on mmax */

/*
 * Macros for network/external number representation conversion.
 */

#if defined(lint) || !defined(__GNUC__)

unsigned short	ntohs(), htons();
unsigned long	ntohl(), htonl();

#else

extern __inline__
unsigned short
ntohs(w)
	register unsigned short w;
{
	asm("movzwd	%0, %0" : "=r" (w) : "0" (w));
	asm("rotw	$8, %0" : "=r" (w) : "0" (w));
	return w;
}
#define	htons	ntohs

extern __inline__
unsigned long
ntohl(l)
	register unsigned long l;
{
	asm("rotw	$8, %0" : "=r" (l) : "0" (l));
	asm("rotd	$16,%0" : "=r" (l) : "0" (l));
	asm("rotw	$8, %0" : "=r" (l) : "0" (l));
	return l;
}
#define htonl	ntohl

#endif	/* __GNUC__ inlines */

#define NTOHL(x)	(x) = ntohl((u_long)x)
#define NTOHS(x)	(x) = ntohs((u_short)x)
#define HTONL(x)	(x) = htonl((u_long)x)
#define HTONS(x)	(x) = htons((u_short)x)

#endif
