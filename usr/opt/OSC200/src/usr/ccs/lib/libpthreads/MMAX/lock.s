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
 *	@(#)$RCSfile: lock.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:06 $
 */ 
/*
 */
#
# (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
# ALL RIGHTS RESERVED
#
# 
# Mach Operating System
# Copyright (c) 1989 Carnegie-Mellon University
# All rights reserved.  The CMU software License Agreement specifies
# the terms and conditions for use and redistribution.
#
#
# OSF/1 Release 1.0

# Mutex implementation for Encore Multimax.

.text

#	int
#	lock_try_set(lock)
#	int	*lock;

	.globl	_lock_try_set
_lock_try_set:
	sbitib	$0,0(4(sp))	# try to lock it
	sfcd	r0		# return 1 if successful,
	ret	$0		# 0 if it was already locked

#	void
#	lock_unset(lock)
#	int	*lock;

	.globl	_lock_unset
_lock_unset:
	movqd	$0,0(4(sp))
	ret	$0
