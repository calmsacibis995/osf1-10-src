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
 *	@(#)$RCSfile: strcmp.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:21:43 $
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

/* This function is an assembly-code replacement for
   the libc function "strcmp."  */
/* Libc currently has a mips-specific C version that uses 7 instructions/byte.
   (It claims to use 6 cycles/byte, but is wrong!)
   This function uses an unrolled loop, which uses 5 instructions per byte.

   Under some circumstances more characters are read than are
   required for determining the collating order, but it
   never reads beyond the end of either string.

   There are one caveat to consider: this function is written
   in assembler code, and as such, cannot be merged
   using the U-code loader. */

/* Craig Hansen - 6-June-86 */

#include <mips/asm.h>
#include <mips/regdef.h>

	.text	

LEAF(strcmp)

	.set	noreorder
	lbu	t0,0(a0)
1:	lbu	t1,0(a1)
	beq	t0,0,2f
	addi	a0,2
	bne	t0,t1,3f
	lbu	t2,-1(a0)	# ok to load since -2(a0)!=0
	lbu	t1,1(a1)
	beq	t2,0,2f
	addi	a1,2
	beq	t2,t1,1b
	lbu	t0,0(a0)	# ok to load since -1(a0) != 0
	j	ra
	subu	v0,t2,t1	
2:	j	ra
	subu	v0,zero,t1
3:	j	ra
	subu	v0,t0,t1
	.end	strcmp
