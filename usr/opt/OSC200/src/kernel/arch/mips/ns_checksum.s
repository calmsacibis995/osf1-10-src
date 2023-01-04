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
 * @(#)$RCSfile: ns_checksum.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/15 08:12:53 $
 */

#include  <machine/regdef.h>
#include  <machine/asm.h>
#include  <machine/cpu.h>

/*
 *	The XNS checksummer does an add-and-cycle checksum.  Odd byte
 *	lengths are dealt with by postpending a garbage byte, which is
 *	carried with the packet forever, and is not assumed to be
 *	zero.  Thus, the algorithm is load a half-word, and add it to
 *	the current checksum.  Then, shift the whole mess left one bit,
 *	and iterate.  All math is ones-complement on  16 bits, so when
 *	we are done, we must  fold back all the carry bits that are in
 *	the high 16 bits of our register.  The caller is required to
 *	half-word align the packet, since we can't easily, and to
 *	postpend the garbage byte if necessary.
 */
LEAF(ns_checksum)
	move	v0,a2		# copy previous checksum
	move	t0, a1		# save count
	beq	a1,zero,3f	# count exhausted
	and	v1,a0,1
	beq	v1,zero,2f	# already on a halfword boundry
	li	v0, 0177777	# error code
	b	3f

1:	lhu	t8,0(a0)
	addu	a0,2
	addu	v0,t8
	subu	a1,2

2:	bge	a1,2,1b
	srl	v1,v0,16	# add in all previous wrap around carries
	and	v0,0xffff
	addu	v0,v1
	srl	v1,v0,16	# wrap-arounds could cause carry, also
	addu	v0,v1
	and	v0,0xffff
	sll	t0, 1		# divide count by two it - is round
	rol	v0, t0		# now do 32 bit rotate...
	and	v1, v0, 0xffff	# and fold it back down to 16 bits
	and	v0, 0xffff0000
	or	v0, v1		# done, now have 16 bit checksum

3:	j	ra
	END(ns_checksum)

