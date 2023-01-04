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
 * @(#)$RCSfile: kn_cache_ops.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:11:37 $
 */

#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/cpu.h>
#include <machine/machparam.h>

/*
 * kn_flush_cache()
 * flush entire cache
 */
LEAF(kn_flush_cache)
	.set	noreorder
	lw	t1,icache_size		# must load before isolating
	lw	t2,dcache_size		# must load before isolating
	mfc0	t3,C0_SR		# save SR
	mtc0	zero,C0_SR		# interrupts off
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop

	/*
	 * flush text cache
	 */
1:	li	v0,SR_ISC|SR_SWC	# disable intr, isolate and swap
	mtc0	v0,C0_SR
	li	t0,K1BASE
	subu	t0,t1
	li	t1,K1BASE
	la	v0,1f			# run cached
	j	v0
	nop
	.set	reorder

1:	sb	zero,0(t0)
	sb	zero,4(t0)
	sb	zero,8(t0)
	sb	zero,12(t0)
	sb	zero,16(t0)
	sb	zero,20(t0)
	sb	zero,24(t0)
	addu	t0,32
	sb	zero,-4(t0)
	bne	t0,t1,1b

	.set	noreorder
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop

	/*
	 * flush data cache
	 */
1:	li	v0,SR_ISC		# isolate and swap back caches
	mtc0	v0,C0_SR
	li	t0,K1BASE
	subu	t0,t2
	la	v0,1f
	j	v0			# back to cached mode
	nop
	.set	reorder

1:	sb	zero,0(t0)
	sb	zero,4(t0)
	sb	zero,8(t0)
	sb	zero,12(t0)
	sb	zero,16(t0)
	sb	zero,20(t0)
	sb	zero,24(t0)
	addu	t0,32
	sb	zero,-4(t0)
	bne	t0,t1,1b

	.set	noreorder
	nop				# insure isolated stores out of pipe
	nop
	nop
	mtc0	t3,C0_SR		# un-isolate, enable interrupts
	nop				# insure cache unisolate
	nop
	nop
	nop
	.set	reorder

	j	ra
	END(kn_flush_cache)

/*
 * kn_clean_icache(addr, len)
 * flush i cache for range of addr to addr+len-1
 * MUST NOT DESTROY a0 AND a1, SEE clean_cache ABOVE
 */
LEAF(kn_clean_icache)
	.set noreorder
	mfc0	t3,C0_SR		# save sr
	mtc0	zero,C0_SR		# interrupts off
	lw	t1,icache_size
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop

1:	li	v0,SR_ISC|SR_SWC	# disable intr, isolate and swap
	mtc0	v0,C0_SR
	bltu	t1,a1,1f		# cache is smaller than region
	nop
	move	t1,a1
1:	addu	t1,a0			# ending address + 1
	move	t0,a0
	la	v0,1f			# run cached
	j	v0
	nop
	.set	reorder

1:	sb	zero,0(t0)
	sb	zero,4(t0)
	sb	zero,8(t0)
	sb	zero,12(t0)
	sb	zero,16(t0)
	sb	zero,20(t0)
	sb	zero,24(t0)
	addu	t0,32
	sb	zero,-4(t0)
	bltu	t0,t1,1b

	.set	noreorder
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop

1:	nop				# insure isolated stores out of pipe
	mtc0	zero,C0_SR		# unisolate, unswap
	nop
	nop
	nop
	mtc0	t3,C0_SR		# enable interrupts
	j	ra			# return and run cached
	nop
	.set	reorder
	END(kn_clean_icache)



LEAF(kn_clean_dcache)
	.set	noreorder
	lw	t2,dcache_size
	mfc0	t3,C0_SR		# save sr
	li	v0,SR_ISC		# disable interrupts, isolate caches
	mtc0	v0,C0_SR
	bltu	t2,a1,1f		# cache is smaller than region
	nop
	move	t2,a1
1:	addu	t2,a0			# ending address + 1
	move	t0,a0
	nop
	nop				# cache must be isolated by now
	.set	reorder

1:	sb	zero,0(t0)
	sb	zero,4(t0)
	sb	zero,8(t0)
	sb	zero,12(t0)
	sb	zero,16(t0)
	sb	zero,20(t0)
	sb	zero,24(t0)
	addu	t0,32
	sb	zero,-4(t0)
	bltu	t0,t2,1b

	.set	noreorder
	nop				# insure isolated stores out of pipe
	nop
	nop
	mtc0	t3,C0_SR		# un-isolate, enable interrupts
	nop				# insure cache unisolated
	nop
	nop
	j	ra
	nop
	.set	reorder
	END(kn_clean_dcache)


/*
 * kn_page_iflush(addr)
 * flush one page of i cache, addr is assumed to be in K0SEG
 */
LEAF(kn_page_iflush)
	.set	noreorder
	lw	t1,icache_size
	mfc0	t3,C0_SR		# save sr
	mtc0	zero,C0_SR		# interrupts off
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop				# BDSLOT

	/*
	 * flush text cache
	 */
1:	li	v0,SR_ISC|SR_SWC	# disable intr, isolate and swap
	mtc0	v0,C0_SR
	bltu	t1,NBPG,1f		# cache is smaller than region
	nop				# BDSLOT
	li	t1,NBPG
1:	addu	t1,a0			# ending address + 1
	move	t0,a0
	la	v0,1f			# run cached
	j	v0
	nop				# cache must be isolated by now
	.set	reorder

1:	sb	zero,0(t0)
	sb	zero,4(t0)
	sb	zero,8(t0)
	sb	zero,12(t0)
	sb	zero,16(t0)
	sb	zero,20(t0)
	sb	zero,24(t0)
	addu	t0,32
	sb	zero,-4(t0)
	bltu	t0,t1,1b

	.set	noreorder
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop

1:	nop				# insure isolated stores out of pipe
	mtc0	zero,C0_SR		# unisolate, unswap
	nop
	nop
	nop
	nop
	mtc0	t3,C0_SR		# enable interrupts
	la	v0,1f			# run cached
	j	v0
	nop

	.set	reorder
1:
	j	ra
	END(kn_page_iflush)



/*
 * kn_page_dflush(addr)
 * flush one page of i cache, addr is assumed to be in K0SEG
 */
LEAF(kn_page_dflush)
	.set	noreorder
	lw	t2,dcache_size
	mfc0	t3,C0_SR		# save sr
	li	v0,SR_ISC		# interrupts off, isolate caches
	mtc0	v0,C0_SR
	bltu	t2,NBPG,1f		# cache is smaller than region
	nop
	li	t2,NBPG
1:	addu	t2,a0			# ending address + 1
	move	t0,a0			# cache must be isolated by now
	.set	reorder

1:	sb	zero,0(t0)
	sb	zero,4(t0)
	sb	zero,8(t0)
	sb	zero,12(t0)
	sb	zero,16(t0)
	sb	zero,20(t0)
	sb	zero,24(t0)
	addu	t0,32
	sb	zero,-4(t0)
	bltu	t0,t2,1b

	.set	noreorder
	nop				# insure isolated stores out of pipe
	nop
	nop
	mtc0	t3,C0_SR		# un-isolate, enable interrupts
	nop				# insure cache unisolated
	nop
	nop
	.set	reorder
	j	ra
	END(kn_page_dflush)

