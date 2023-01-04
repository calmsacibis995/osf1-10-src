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
 *    @(#)$RCSfile: execute_br.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:12:14 $
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

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/* $ Header: execute_br.s,v 1.1 87/04/05 15:43:09 dce Exp $ */

#include <mips/regdef.h>
#include <mips/asm.h>

/*
 * execute_branch is passed a branch instruction and returns a non-zero
 * value if the branch is taken and a zero value otherwise.  This routine
 * is only intended to be used for branch on co-processor condition
 * instructions that can't be emulated.
 *
 * The branch instruction is executed in data space with a modified offset.
 * The code executed in data space looks like this:
 *	branch --------
 *	nop	      |
 *	j	a0    |
 *	j	a1 <---
 * Where a0 gets loaded with is the address of this routine's text label to
 * handle non-taken branches and a1 with the label for taken branches.
 */

#define	ICACHE		0x1		/* flush i cache */
#define	FRAME_SIZE	8
#define	LOCAL_SIZE	0
#define	RA_OFFSET	FRAME_SIZE-LOCAL_SIZE-4*1

NESTED(execute_branch, FRAME_SIZE, ra)
	.mask	M_RA, -(LOCAL_SIZE+4)

	subu	sp,FRAME_SIZE
	sw	ra,RA_OFFSET(sp)

	srl	a0,16		# build the branch with a modified offset
	sll	a0,16
	or	a0,8>>2	
	
	la	a1,execute_area	# store the instructions in the execute_area
	sw	a0,0(a1)	# the "branch with modified offset"
	sw	zero,4(a1)	# the "nop"
	lw	a0,1f
	sw	a0,8(a1)	# the "jr a0"
	lw	a0,2f
	sw	a0,12(a1)	# the "jr a1"

	la	a0,execute_area	# flush the icache for the execute_area
	li	a1,32
	li	a2,ICACHE
	jal	cacheflush

	la	a0,taken	# prepare to jump to the execute area
	la	a1,nottaken
	la	a2,execute_area
	.set	nomove
	j	a2
	nop
	nop
	nop
	nop
1:	j	a0		# template instruction that isn't executed
2:	j	a1		# template instruction that isn't executed
	.set	move
taken:
	li	v0,1
	j	ret
nottaken:
	move	v0,zero
ret:	lw	ra,RA_OFFSET(sp)
	addu	sp,FRAME_SIZE
	j	ra
	END(execute_branch)

	.data
execute_area:
	.word	0
	.word	0
	.word	0
	.word	0

	.word	0
	.word	0
	.word	0
	.word	0
