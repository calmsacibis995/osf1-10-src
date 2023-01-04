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
 * @(#)$RCSfile: issplsched.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 16:34:36 $
 */

#include <machine/cpu.h>
#include <machine/asm.h>		/* Includes mach_assert.h */
#include <machine/regdef.h>
#include <hal/kn02ba.h>
#include <rt_sched.h>


#if RT_SCHED && MACH_ASSERT
/*
 * issplsched: test if SPL is at splsched.  Used for assertions in
 * scheduler routines.
 */
LEAF(issplsched)
	.set	noreorder
	mfc0	v0,C0_SR
	li	v1,SR_IMASK8
	andi	v0,v0,SR_IMASK
	bne	v0,v1,1f
	nop
	j	ra
	li	v0,1
1:	j	ra
	li	v0,0
	.set	reorder
	END(issplsched)
#endif /* RT_SCHED && MACH_ASSERT */
