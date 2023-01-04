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
 * @(#)$RCSfile: hwcp.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 16:00:25 $
 */

#ifdef not_used

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>

/*
 *	hwcpout is going from 32 bit bus to 16 bit bus, hwcpin is opposite.
 *	a0 is the source. a1 is the destination. a2 is the byte-count.
 *	a3 is (usually) the value a0 should have when the current move'
 *	loop is done.  v0,v1,t0,t1 scratch regs, used for alignment, and
 *	moves.  Does not save any regs.  No return value.
 *
 *	Basic Algorithm:
 *		First check if the count passed is < HWMINCOPY.  If yes,
 *		jump to byte copy routine, it isn't worth hassling.
 *		Then, try to align the 32 bit side on a word bit boundary.
 *		If you can't, just byte copy.  This happens very rarely, the
 *		typical case is both sides word aligned.  Then do as many
 *		16 byte copy loops as you can, then do as many 2 byte copy
 *		iterations as you can, then pick up the dregs with byte copies.
 *		This assumes typical case is full aligned, 20-112 bytes.
 */

/*
 *	HWMINCOPY is the minimum copy size on which we try to align and
 *	use half-word rather than byte copy loop.  Mash thought 8-12
 *	was a good number for bcopy(), seems to me that the outside of
 *	that range would suite here.  Why? We are assuming cache hit rate
 *	of 0%.  Our half-word loop is less efficient in its use of the
 *	WB and of cycles than his full word loop. MORE THOUGHT HERE PLEASE.
 */

#define	HWMINCOPY	12

LEAF(hwcpout)
/*
 *	first, check for alignment possibilities
 */
	.sdata
tmp:
	.word	0		# used for WB flush
	.text
	xor	v0, a0, a1	# bash src & dst for align chk
	blt	a2, HWMINCOPY, hbytecopy	# too short, just byte copy
	and	v0, 1		# low-order bit for align chk
	subu	v1, zero, a0	# -src; BDSLOT
	bne	v0, zero, hbytecopy	# src and dst not alignable

/*
 * src and dst can be simultaneously word aligned.
 */
	and	v1, 3		# number of bytes til aligned
	subu	a2, v1		# bcount -= alignment
	addu	a3,v1,a0	# end of align move
	beq	v1, zero, hblkcopy	# already aligned

/*
 * This is the easy way, could maybe be done better.  The problem
 * is that lwl/r and swl/r will not work on the 16 bit side.  Since
 * worst case is three times through, the math to do the possible
 * half-word copy does not seem worth it, nor does the shifting to
 * use the lwl/r from the 32 bit side.
 */
1:				# tight loop
	lb	v0, 0(a0)
	addu	a0, 1
	sb	v0, 0(a1)
	addu	a1, 1
	sw	zero, tmp	# ensure no WB gather
	bne	a0, a3, 1b
	
/*
 * 16 byte block, aligned copy loop (for big reads/writes)
 * We must out fox the WB on 16 bit stores, else the card will
 * punt the data.  This explains the somewhat esoteric ordering of
 * the stores.  If we write consecutive half-words to the same
 * word address, we lose.
 */
hblkcopy:
	and	a3, a2, ~15	# total space in 16 byte chunks
	subu	a2, a3		# count after by-16 byte loop done
	beq	a3, zero, hwordcopy	# less than 16 bytes to copy
	addu	a3, a0		# source endpoint
	.set noreorder
1:	lw	v0, 0(a0)
	addu	a0, 16		# src += 16 ; no other delay slot...
	lw	t0, -12(a0)
	sh	v0, 2(a1)
	srl	v0,  16
	lw	t1, -8(a0)
	sh	t0, 6(a1)
	sh	v0, 0(a1) 
	srl	t0,  16
	lw	t2, -4(a0)
	sh	t1,  10(a1)
	sh	t0,  4(a1)
	srl	t1,  16
	sh	t2,  14(a1)
	sh	t1,  8(a1)
	srl	t2,  16
	sh	t2,  12(a1)
	bne	a0, a3, 1b
	addu	a1, 16		# dst += 16
	.set reorder

/*
 * copy what ever is left,  but is aligned,  in half-words
 */
hwordcopy:
	addu	a3, a2, a0	# source endpoint;
	ble	a2, 1, hbytecopy
/*
 * This could maybe be done better?
 */
	and	t0, a3, ~1	# catch tail
	subu	a2, a3, t0
	move	a3, t0
1:				# tight loop
	.set 	noreorder
	lh	v0, 0(a0)
	addu	a0, 2		#LDSLOT
	sh	v0, 0(a1)
	addu	a1, 2
	bne	a0, a3, 1b
	sw	zero, tmp	# ensure no WB gather
	.set	reorder
	
/*
 * Brute force byte copy loop, pick up the dregs.  Also pick up copies
 * that are unalignable, doing the math to be smarter under the
 * 16 bit constraints turns out to lose.
 */

hbytecopy:
	addu	a3, a2, a0	# source endpoint; BDSLOT
	ble	a2, zero, hcopydone	# nothing left to copy,  or bad length
1:				# tight loop
	.set	noreorder
	lb	v0, 0(a0)
	addu	a0, 1		# incr src address
	sb	v0, 0(a1)
	addu	a1, 1		# incr dst address
	bne	a0, a3, 1b
	sw	zero, tmp	# ensure no WB gather
	.set	reorder
hcopydone:
	j	ra
	END(hwcpout)

LEAF(hwcpin)
/*
 *	first, check for alignment possibilities
 */
	xor	v0, a0, a1	# bash src & dst for align chk
	blt	a2, HWMINCOPY, hbytecopy	# too short, just byte copy
	and	v0, 1		# low-order bit for align chk
	subu	v1, zero, a1	# -src
	bne	v0, zero, hbytecopy	# src and dst not alignable
	addu	t5, a1, a2	# firewall panic on overrun

/*
 * src and dst can be simultaneously word aligned.
 */
	and	v1, 3		# number of bytes til aligned
	subu	a2, v1		# bcount -= alignment
	addu	a3,v1,a0	# end of align move
	beq	v1, zero, blkcpin	# already aligned

/*
 * This is the easy way, could maybe be done better.  The problem
 * is that lwl/r and swl/r will not work on the 16 bit side.  Since
 * worst case is three times through, the math to do the possible
 * half-word copy does not seem worth it, nor does the shifting to
 * use the lwl/r from the 32 bit side.
 */
1:
	lb	v0, 0(a0)
	addu	a0, 1
	sb	v0, 0(a1)
	addu	a1, 1		# should go in the BDSLOT
	bne	a0, a3, 1b
	
/* 16 byte block copy */

blkcpin:
	and	a3, a2, ~15	# total space in 16 byte chunks
	subu	a2, a3		# count after by-16 byte loop done
	beq	a3, zero, hwordcopy	# less than 16 bytes to copy
	addu	a3, a0		# source endpoint
	.set noreorder
1:	
	lhu	v0, 0(a0)
	addu	a0, 16
	lhu	v1, -14(a0)
	sll	v0, 16
	lhu	t0, -12(a0)
	or	v0, v1
	sw	v0, 0(a1)
	lhu	t1, -10(a0)
	sll	t0, 16
	or	t0, t1
	lhu	v0, -8(a0)
	sw	t0, 4(a1)
	lhu	v1, -6(a0)
	sll	v0, 16
	or	v0, v1
	lhu	t0, -4(a0)
	sw	v0, 8(a1)
	lhu	t1, -2(a0)
	sll	t0, 16
	or	t0, t1
	sw	t0, 12(a1)
	bne	a0, a3, 1b
	addu	a1, 16			# dst+= 16; fills BD slot
	.set reorder
	bgt	a1, t5, bad
	b	hwordcopy
panic_lab:
	.data
	.asciiz "hwcpin"
	.text
bad:
	lw	a0, panic_lab
	jal	panic
	
	END(hwcpin)

#endif /* not_used */
