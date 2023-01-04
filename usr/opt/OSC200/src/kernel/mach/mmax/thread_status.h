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
 *	@(#)$RCSfile: thread_status.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:34:08 $
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
 * Copyright (c) 1987 Carnegie-Mellon University
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
 *	File:	{mmax,sqt}/thread_status.h
 *	Author:	David L. Black
 *
 *	This file contains the structure definitions for the user-visible
 *	thread state as applied to ns32000 processors.  NOTE: this file
 *	is shared between multiple ns32000 implementations (Balance and
 *	Multimax).
 *
 */

#ifndef	_MACH_MMAX_THREAD_STATUS_H_
#define _MACH_MMAX_THREAD_STATUS_H_

#include <sys/types.h>

/*
 *	Two structures are defined:
 *
 *	ns32000_thread_state	this is the structure that is exported
 *				to user threads for use in status/mutate
 *				calls.  This structure should never
 *				change.
 *
 *	ns32000_saved_state	this structure corresponds to the state
 *				of the user registers as saved on the
 *				stack upon kernel entry.  This structure
 *				is used internally only.  Since this
 *				structure may change from version to
 *				version, it is hidden from the user.
 */

#define NS32000_THREAD_STATE	(1)
				/* only one set of registers */

struct ns32000_thread_state {
	int	r0;
	int	r1;
	int	r2;
	int	r3;
	int	r4;
	int	r5;
	int	r6;
	int	r7;
	int	fp;
	int	sp;
	int	pc;
	short	mod;
	short	psr;
	int	fsr;
	int	f0;
	int	f1;
	int	f2;
	int	f3;
	int	f4;
	int	f5;
	int	f6;
	int	f7;
};

#define NS32000_THREAD_STATE_COUNT \
		(sizeof(struct ns32000_thread_state)/sizeof(int))


#define	NS32532_THREAD_STATE	(2)

struct ns32532_thread_state {
	int	r0;
	int	r1;
	int	r2;
	int	r3;
	int	r4;
	int	r5;
	int	r6;
	int	r7;
	int	fp;
	int	sp;
	int	pc;
	short	mod;
	short	psr;
	int	fsr;
	quad	f0;			/* l0 = (f1,f0) */
	quad	f1;
	quad	f2;			/* l2 = (f3,f2) */
	quad	f3;
	quad	f4;			/* l4 = (f5,f4) */
	quad	f5;
	quad	f6;			/* l6 = (f7,f6) */
	quad	f7;
};

#define	NS32532_THREAD_STATE_COUNT \
		(sizeof(struct ns32532_thread_state)/sizeof(int))

#ifdef	KERNEL
/*
 *	NOTE:	Floating point registers are never saved on the kernel
 *		stack.  They have to be altered by hand or in the pcb.
 */
struct ns32000_saved_state {
	int	sp;
	int	r7;
	int	r6;
	int	r5;
	int	r4;
	int	r3;
	int	r2;
	int	r1;
	int	r0;
#ifdef	balance
	int	ipl;
#endif	balance
	int	fp;
	int	pc;
	short	mod;
	short	psr;
};

#endif	KERNEL
#endif	_MACH_MMAX_THREAD_STATUS_H_
