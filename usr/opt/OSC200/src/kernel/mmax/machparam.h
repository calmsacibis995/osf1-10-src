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
 *	@(#)$RCSfile: machparam.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:39 $
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
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
 * Copyright (c) 1982 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */
 
#ifdef	KERNEL
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#else	KERNEL
#if	!defined(MMAX_XPC) && !defined(MMAX_APC) && !defined(MMAX_DPC)
#define	MMAX_XPC	1
#endif
#endif	KERNEL

#ifndef BYTE_ORDER
#include <machine/endian.h>
#endif
#include <machine/machlimits.h>

#include <mach/machine/vm_param.h>
#ifdef	KERNEL
#include <mach/vm_param.h>
#endif	KERNEL

/*
 * Machine dependent constants for Encore MULTIMAX.
 */
#define	MACHINE	"mmax"

#define NBPG	NS32K_PGBYTES	/* bytes/page */
#define PGOFSET	(NBPG-1)	/* byte offset into page */
#define PGSHIFT	NS32K_PGSHIFT	/* LOG2(NBPG) */

#if	MMAX_XPC || MMAX_APC
#define CLSIZE		1
#define CLSIZELOG2	0
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
#define CLSIZE		2
#define CLSIZELOG2	1
#endif	MMAX_DPC

#define SSIZE	(PAGE_SIZE/NBPG)	/* initial stack size/NBPG */
#define SINCR	(PAGE_SIZE/NBPG)	/* increment of stack/NBPG */

#define UPAGES	(8192/NBPG)		/* XXX pages of u-area */
					/* This presumes the U-area <= 8K bytes */

/*
 * Some macros for units conversion
 */
/* Core clicks (512 bytes) to segments and vice versa */
#define ctos(x)	(x)
#define stoc(x)	(x)

/* Core clicks (MMU Dependent!) to disk blocks */
#define ctod(x)	(x)
#define dtoc(x)	(x)
#define dtob(x)	((x)<<PGSHIFT)

/* clicks to bytes */
#define ctob(x)	((x)<<PGSHIFT)

/* bytes to clicks */
#define btoc(x)	((((unsigned)(x)+(NBPG-1))>>PGSHIFT))

/*
 * Macros to decode processor status word.
 */
#define USERMODE(psrmod)	(((psrmod >> 16) & PSR_U) == PSR_U)
#define BASEPRI(psrmod)		(((psrmod >> 16) & PSR_I) == PSR_I)


/*
 * Maximum string lengths
 */
#define PATH_LEN	256		/* Boot image and param file name */
#define VERS_LEN	256		/* System version */
#define INTR_LEN	8		/* Interrupt names */
#define TRAP_LEN	4		/* Trap names */

/*
 * Number of system interrupt and trap vectors
 */
#if	MMAX_XPC
#define NUMSYSVEC	32
#endif	MMAX_XPC
#if	MMAX_APC
#define NUMSYSVEC      256
#endif	MMAX_APC
#if	MMAX_DPC
#define NUMSYSVEC       16
#endif	MMAX_DPC
