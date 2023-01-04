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
 *	@(#)$RCSfile: machparam.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:19:03 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */
#include  <i386/endian.h>

#define MACHINE "i386"

/*
 * Machine dependent constants for 80386.
 */
#define NBPG	4096		/* bytes/page */
#define PGOFSET	(NBPG-1)	/* byte offset into page */
#define PGSHIFT	12		/* LOG2(NBPG) */

#define CLSIZE		1
#define CLSIZELOG2	0

#define SSIZE	1		/* initial stack size/NBPG */
#define SINCR	1		/* increment of stack/NBPG */

#define UPAGES	16		/* pages of u-area */

/*
 * Some macros for units conversion
 */
/* Core clicks (512 bytes) to segments and vice versa */
#define ctos(x)	(x)
#define stoc(x)	(x)

/* Core clicks (512 bytes) to disk blocks */
#define ctod(x)	(x)
#define dtoc(x)	(x)
#define dtob(x)	((x)<<9)

/* clicks to bytes */
#define ctob(x)	((x)<<9)

/* bytes to clicks */
#define btoc(x)	((((unsigned)(x)+511)>>9))

/*
 * Macros to decode processor status word.
 */
#define USMODE	3		/* current Xlevel == user */
#define SEL_RPL		0x03
#define USERMODE(cs)	(((cs) & SEL_RPL) == USMODE)
#define BASEPRI(x)  ((x) != 0)

#ifdef	KERNEL
#ifndef	ASSEMBLER
int	cpuspeed;
#endif  ASSEMBLER
#define DELAY(n)	{ register int N = cpuspeed * (n); while (--N > 0); }

#else	KERNEL
#define DELAY(n)	{ register int N = (n); while (--N > 0); }
#endif	KERNEL
