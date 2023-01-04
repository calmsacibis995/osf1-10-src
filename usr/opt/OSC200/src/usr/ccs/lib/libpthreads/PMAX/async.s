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
 *	@(#)$RCSfile: async.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:18 $
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

#include <mips/asm.h>
#include <mips/regdef.h>
#include <mips/inst.h>

	.text
	.align	2
	.set	noreorder

/*
 * Glue to deliver an asynchronous function call. vp_call_setup has saved
 * our old state on the stack and put all the information we need in registers.
 *
 *	a0 - the function call parameter
 *	s0 - the function address
 *
 * We are prepared for the function to return although it never will as this
 * is use only for cancellation which does a pthread_exit after cleanup.
 */

	.globl	vp_call_deliver
	.ent	vp_call_deliver
vp_call_deliver:
	jal	s0			# jump to the function
	nop				# shadow not needed
	lw	ra, 0(sp)		# restore the old return address
	lw	s0, 4(sp)		# restore s0 from stack
	lw	a0, 8(sp)		# restore a0 from stack
	addu	sp, 12			# adjust sp back to where is was before
					# this call was forced on us

	j	ra			# back we go to what we were doing
	nop				# shadow not needed

.end	vp_call_deliver

	.set	reorder

