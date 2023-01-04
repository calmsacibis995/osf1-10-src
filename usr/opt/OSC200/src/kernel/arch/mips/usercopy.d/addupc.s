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
 * @(#)$RCSfile: addupc.s,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/09/29 09:21:12 $
 */

#include <machine/cpu.h>	
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>




/*
 * addupc(pc, &u.u_prof, ticks)
 */
LEAF(addupc)
	lw	v1,PR_OFF(a1)		# base of profile region
	subu	a0,v1			# corrected pc
	bltz	a0,1f			# below of profile region
	lw	v0,PR_SCALE(a1)		# fixed point scale factor
	bne	v0,02,5f
	li	v0,0			# if scale is 2, put all ticks in
	b	4f			# the first profile bucket
5:
	multu	v0,a0
	mflo	v0			# shift 64 bit result right 16
	srl	v0,16
	mfhi	v1
	sll	v1,16
	or	v0,v1
	addu	v0,1			# round-up to even
	and	v0,~1
4:
	lw	v1,PR_SIZE(a1)
	bgeu	v0,v1,1f		# above profile region
	li	a3,PCB_WIRED_ADDRESS	# BDSLOT
	lw	v1,PR_BASE(a1)		# base of profile buckets
	addu	v0,v1

	bgez	v0,3f			# jump to adderr now 
	j 	adderr
3:
#ifdef ASSERTIONS
	lw	v1,PCB_NOFAULT(a3)
	beq	v1,zero,2f
	PANIC("recursive nofault")
2:
#endif
	.set	noreorder
	li	v1,NF_ADDUPC
	sw	v1,PCB_NOFAULT(a3)
	lh	v1,0(v0)		# add ticks to bucket
	nop
	addu	v1,a2
	sh	v1,0(v0)
	sw	zero,PCB_NOFAULT(a3)
	.set	reorder

1:	j	ra
	END(addupc)

