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
 *	@(#)$RCSfile: lock.s,v $ $Revision: 1.1.2.5 $ (DEC) $Date: 1992/06/12 10:44:21 $
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

#include	<alpha/regdef.h>
#include	<alpha/asm.h>

/*
 * int lock_try_set(lock)
 *
 * Try to set lock.  Return 1 if successful, else zero.
 *
 * Note: Alpha architecture allows for a range of physical addresses
 * to be associated with the lock, which is implementation dependant.
 * The minimum lock range is 8 bytes.  The side effect is that nearby
 * locks will cause unintentional contention.
 */
LEAF(lock_try_set)
	bis	zero, zero, v0
1:	ldl_l	t0, (a0)
	blbs	t0, 2f		# lock already set
	bis	zero, 1, v0
	stl_c	v0, (a0)	# try to set it
	beq	v0, 3f		# store failed (account for interrupts)
2:				# v0 <- 1 if successful, else 0
	RET
3:	br	zero,1b		# try again, shouldn't happen too often
END(lock_try_set)

/*
 * void lock_unset(lock)
 *
 * clear lock.
 */
LEAF(lock_unset)
	stl	zero, (a0)
	RET
END(lock_unset)
