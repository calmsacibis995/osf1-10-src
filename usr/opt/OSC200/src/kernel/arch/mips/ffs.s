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
 * @(#)$RCSfile: ffs.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 15:18:59 $
 */

#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>

/*
 * ffs(word)
 * BEWARE: that C version of this routine that is distributed with 4.2
 * is incorrect!
 *
 * find first bit set in word (a la VAX instruction)
 * looks at low order bits first, lowest order bit is 1, highest bit is 32
 * no bits returns 0
 */
LEAF(ffs)
	.set	noreorder
	move	v0,zero
	beq	a0,zero,2f		# no bits set, return zero
1:	and	v1,a0,1
	addu	v0,1
	beq	v1,zero,1b
	srl	a0,1			# BDSLOT: shift right to next bit
2:	j	ra
	nop
	.set	reorder
	END(ffs)

#ifdef notdef
LEAF(ffs)
	move	v1,zero			# initial table offset
	and	v0,a0,0xffff		# check lower halfword
	bne	v0,zero,1f		# bits in lower halfword
	addu	v1,64			# table offset for halfword
	srl	a0,16			# check upper halfword
1:	and	v0,a0,0xff		# check lower byte of halfword
	bne	v0,zero,2f		# bits in lower byte
	addu	v1,32			# table offset for byte
	srl	a0,8			# check upper byte of halfword
2:	and	v0,a0,0xf		# check lower nibble
	bne	v0,zero,3f		# bits in lower nibble
	addu	v1,16			# table offset for nibble
	srl	v0,a0,4			# check upper nibble
	and	v0,0xf			# isolate lower nibble
3:	addu	v1,v0			# total table offset
	lbu	v0,ffstbl(v1)		# load bit number from table
	j	ra
	END(ffs)

	.data
#define NIBBLE(x) \
	.byte	0,       1+(x)*4, 2+(x)*4, 1+(x)*4; \
	.byte	3+(x)*4, 1+(x)*4, 2+(x)*4, 1+(x)*4; \
	.byte	4+(x)*4, 1+(x)*4, 2+(x)*4, 1+(x)*4; \
	.byte	3+(x)*4, 1+(x)*4, 2+(x)*4, 1+(x)*4
ffstbl:
	NIBBLE(0)
	NIBBLE(1)
	NIBBLE(2)
	NIBBLE(3)
	NIBBLE(4)
	NIBBLE(5)
	NIBBLE(6)
	NIBBLE(7)

	.text
#endif /* notdef */

