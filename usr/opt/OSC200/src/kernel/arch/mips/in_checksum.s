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
 * @(#)$RCSfile: in_checksum.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 18:02:38 $
 */


#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>

/*
 * Functional Description:
 *
 *	High speed replacement routine for the 16-bit one's complement
 *	IP checksum. Note that this code will only work on little-endian
 *	machines.
 *
 * Inputs:
 *
 *	unsigned short in_checksum( addr, len, prev )
 *
 *	addr	- pointer to buffer to be checksummed (any alignment)
 *	len	- # of bytes to checksum
 *	prev	- previous checksum value
 *
 * Outputs:
 *
 *	None
 *
 * Return Value:
 *
 *	The checksum value (16 low order bits).
 */
LEAF(in_checksum)

	.set	noreorder
	
	beqz	a1,9f			# if zero length, nothing to do
	move	v0,a2			# get previous checksum value
	and	t0,a0,1			# check for halfword alignment
	beqz	t0,1f			# if zero, already aligned
	lbu	t0,(a0)			# get unaligned byte
	addu	a0,1			# update pointer
	subu	a1,1			#   and length
	sll	t0,t0,8			# shift to correct location
	addu	v0,t0			# update checksum

/*
 * Here we do the real work - unroll the loop to compute the checksum
 * 16 bytes at a time.
 *
 *  Computation requirements:
 *		 4 cycles overhead +
 *		18 cycles to compute checksum per 16 bytes
 */
1:	and	t0,a1,~15		# 16 byte chunks
	beqz	t0,3f			# if zero, < 1 block to go
	subu	a1,t0			# update byte count
	addu	t0,a0			# compute data endpoint

2:	lhu	t1,(a0)			# get next halfword
	lhu	t2,2(a0)		#   2 at a time
	addu	v0,t1			# accumulate checksum
	addu	v0,t2
	lhu	t1,4(a0)		# get next halfword
	lhu	t2,6(a0)		#   2 at a time
	addu	v0,t1			# accumulate checksum
	addu	v0,t2
	lhu	t1,8(a0)		# get next halfword
	lhu	t2,10(a0)		#   2 at a time
	addu	v0,t1			# accumulate checksum
	addu	v0,t2
	lhu	t1,12(a0)		# get next halfword
	lhu	t2,14(a0)		#   2 at a time
	addiu	a0,16			# update address
	addu	v0,t1			# accumulate checksum
	bne	a0,t0,2b		# if NE, another block to compute
	addu	v0,t2			# accumulate checksum

/*
 * Run out the remainder of the data, computing the checksum over as many
 * halfwords as possible and then the possible unaligned byte at the end.
 *
 *  Computation requirements:
 *		 5 cycles overhead +
 *		 4 cycles to compute checksum per 2 bytes
 */

3:	beqz	a1,8f			# if zero all done
	and	t0,a1,~1		# 2 byte chunks
	beqz	t0,5f			# if zero, < 1 block to go
	subu	a1,t0			# update byte count
	addu	t0,a0			# compute data endpoint

4:	lhu	t1,(a0)			# get next half word
	addu	a0,2			# update address
	bne	a0,t0,4b		# if NE, another halfword to compute
	addu	v0,t1			# accumulate checksum

	beqz	a1,8f			# if zero, all done
	nop
5:	lbu	t0,(a0)			# get trailing byte
	nop
	addu	v0,t0			# merge into checksum

8:	srl	v1,v0,16		# add in pending
	andi	v0,0xFFFF		#   carries
	addu	v0,v1
	srl	v1,v0,16		# do it twice since
	andi	v0,0xFFFF		#   the original update
	addu	v0,v1			#      may cause a carry

9:	j	ra			# done
	andi	v0,0xFFFF		# mask to 16-bits

	.set	reorder

	END(in_checksum)

