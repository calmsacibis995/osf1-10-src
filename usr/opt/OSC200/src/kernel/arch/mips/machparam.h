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
 *	@(#)$RCSfile: machparam.h,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/04/14 18:14:42 $
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
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

#ifndef BYTE_ORDER
#ifndef ASSEMBLER
#include <machine/endian.h>
#endif
#endif
#include <machine/machlimits.h>

/*
 * Machine dependent constants for mips.
 */
#define	MACHINE		"mips"

#define	NBPG		4096		/* bytes/page */
#define	PGOFSET		(NBPG-1)	/* byte offset into page */
#define	PGSHIFT		12		/* LOG2(NBPG) */
#define	DBSHIFT		9		/* LOG2(Disk block size) */

#define	CLSIZE		1
#define	CLSIZELOG2	0

#define	SSIZE		1		/* initial stack size/NBPG */
#define	SINCR		1		/* increment of stack/NBPG */

#define	UPAGES		2		/* pages of u-area */
#define	KERNELSTACK	0xffffe000	/* Top of kernel stack */
#define	UADDR		0xffffc000	/* address of u */
#define	UVPN		(UADDR>>PGSHIFT)/* virtual page number of u */

/*
 * Some macros for units conversion
 */
/* Core clicks (4096 bytes) to segments and vice versa */
#define	ctos(x)	(x)
#define	stoc(x)	(x)

/* Core clicks (4096 bytes) to disk blocks */
#define	ctod(x)	((x)<<(PGSHIFT-DBSHIFT))
#define	dtoc(x)	((unsigned)(x)>>(PGSHIFT-DBSHIFT))
#define	dtob(x)	((x)<<DBSHIFT)

/* clicks to bytes */
#define	ctob(x)	((x)<<PGSHIFT)

/* bytes to clicks */
#define	btoc(x)	(((unsigned)(x)+PGOFSET)>>PGSHIFT)

/*
 * Macros to decode processor status word.
 */
#define	USERMODE(sr)	usermode(sr)
#define BASEPRI(sr)     basepri(sr)
