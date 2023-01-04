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
	.rdata
	.asciiz "@(#)$RCSfile: addupc.s,v $ $Revision: 1.1.10.2 $ (DEC) $Date: 1993/08/26 20:23:21 $"
	.text

#include <machine/cpu.h>	
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>

/* 
 * note that the stw and ldw instructions use at and t9 
 */

/*
 * addupc(pc, &u.u_prof, ticks)
 */
LEAF(addupc)
	ldq	t11,PR_OFF(a1)		# base of profile region
	subq	a0,t11,a0		# offset - pc in reg a0
	blt	a0,1f			# below of profile region
	ldq	t10,PR_SCALE(a1)	# fixed point scale factor
	subq	t10,2,v0
	bne	v0,6f
	bis	zero,zero,t10		# if scale is 2, put all ticks in
	br	zero,5f			# the first profile bucket
6:
	mulq	t10,a0,t10		# corrected pc times scale
	srl	t10,16,t10		# shift result right 16 (mapping)

	addq	t10,1,t10		# round-up to even
	and	t10,~1,t10
5:
	ldq	t11,PR_SIZE(a1)
	subq	t10, t11,v0
	ldq	a3,current_pcb
	bge	v0,1f			# above profile region

	ldq	t11,PR_BASE(a1)		# base of profile buckets
	addq	t10,t11,t10
	blt	t10,adderr		# jump to adderr now 
3:
#ifdef ASSERTIONS
	ldq	t11,PCB_NOFAULT(a3)
	beq	t11,2f
	PANIC("recursive nofault")
2:
#endif
	ldiq	t11,NF_ADDUPC
	stq	t11,PCB_NOFAULT(a3)
	ldw	t11,0(t10)		# add ticks to bucket
	addq	t11,a2,t11
	stw	t11,0(t10)
	stq	zero,PCB_NOFAULT(a3)

1:	RET
	END(addupc)

LEAF(adderr)
        stq      zero,PR_SCALE(a1)
        RET
	END(adderr)

