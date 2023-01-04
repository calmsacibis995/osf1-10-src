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
 *	@(#)$RCSfile: lock.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:11:55 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 *
 */ 
/*
 * OSF/1 Release 1.0

/* Mutex implementation for Encore Multimax. */

#include <machine/asm.h>

.text

/*	int
*	lock_try_set(lock)
*	int	*lock;
*/

ENTRY(lock_try_set)
	movl	4(%esp),%ecx
			/ lock	prefix possibly necessary for multiple processor
	btsl	$1,(%ecx)	/ try to lock it
	jc	_.locked
	movl	$1,%eax		/ Successful ret 1
	RET

_.locked:
	movl	$0,%eax		/ Already locked
	RET

/*	void
*	lock_unset(lock)
*	int	*lock;
*/

ENTRY(lock_unset)
	movl	4(%esp),%ecx
			/ lock	prefix possibly necessary for multiple processor
	btrl	$1,(%ecx)	/ unlock it
	RET
