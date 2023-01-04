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
 * @(#)$RCSfile: kn5800_cache_ops.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 17:05:14 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>

#if	MIPSEB		/* Unaligned acceses */
#    define LWS lwl
#    define LWB lwr
#    define SWS swl
#    define SWB swr
#else	/* MIPSEL */
#    define LWS lwr
#    define LWB lwl
#    define SWS swr
#    define SWB swl
#endif



/*
 * kn5800_cln_icache(addr, len, wbfladdr)
 * flush i cache for range of addr to addr+len-1
 * MUST NOT DESTROY a0 AND a1, SEE clean_cache ABOVE
 * wbfladdr is the address to read to cause a write buffer flush
 */
LEAF(kn5800_cln_icache)
	lw	t1,icache_size
	.set noreorder
	nop
	mfc0	t3,C0_SR		# save sr
	nop
	mtc0	zero,C0_SR		# interrupts off
	nop
	.set reorder

	.set	noreorder
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop
1:	lw	v0,0(a2)		# isis hack to flush write buffer
	nop
	li	v0,SR_ISC|SR_SWC	# disable intr, isolate and swap
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
	nop				# keep pipeline clean
	nop				# keep pipeline clean
	nop				# keep pipeline clean
	mtc0	t3,C0_SR		# enable interrupts
	j	ra			# return and run cached
	nop
	.set	reorder
	END(kn5800_cln_icache)


LEAF(kn5800_cln_dcache)
	lw	t2,dcache_size
	.set noreorder
	nop
	mfc0	t3,C0_SR		# save sr
	nop
	.set reorder

	.set	noreorder
	lw	v0,0(a2)		# isis hack - to flush write buffer
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
	nop				# insure cache unisolated
	nop				# insure cache unisolated
	j	ra
	nop
	.set	reorder
	END(kn5800_cln_dcache)


/*
 * kn5800_flsh_cache()
 * flush entire cache
 */
LEAF(kn5800_flsh_cache)
	lw	t1,icache_size		# must load before isolating
	lw	t2,dcache_size		# must load before isolating
	.set noreorder
	nop
	mfc0	t3,C0_SR		# save SR
	nop
	mtc0	zero,C0_SR		# interrupts off
	nop
	.set reorder

	.set	noreorder
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop

	/*
	 * flush text cache - but for isis we must muck with the rinval
	 * bit in csr1 to stop invalidates.
	 */
	/* code added for isis (except for the label "1:" begins here. */
1:	lui	v1, 0x1			# put RINVAL bit in v1
	lw	t7, 0(a0)		# get csr1 contents
	nop
	or	t8, t7, v1		# set rinval bit in t8
	sw	t8, 0(a0)		# set RINVAL in csr1
	nop
2:	lw	t7, 0(a0)		# read csr1
	nop
	and	t8, t7, v1		# check rinval
	beq	t8,zero,2b		# loop till set
	nop				# branch delay slot
	/* end code added for isis */
	li	v0,SR_ISC|SR_SWC	# disable intr, isolate and swap
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
#ifdef CACHETRICKS
	lw	v0,icachemask		# index of last entry in icachecnt
	li	v1,0
	sll	v0,1			# offset to last entry
1:	lhu	t0,icachecnt(v1)
	addu	t0,1
	sh	t0,icachecnt(v1)
	addu	v1,2
	ble	v1,v0,1b		# more cachecnt's to bump

	lw	v0,dcachemask		# index of last entry in dcachecnt
	li	v1,0
	sll	v0,1			# offset to last entry
1:	lhu	t0,dcachecnt(v1)
	addu	t0,1
	sh	t0,dcachecnt(v1)
	addu	v1,2
	ble	v1,v0,1b		# more cachecnt's to bump
#endif /* CACHETRICKS */

	j	ra
	END(kn5800_flsh_cache)


/*
 * kn5800_page_iflush(addr, wbfladdr)
 * flush one page of i cache, addr is assumed to be in K0SEG
 * wbfladdr is the address to read to cause a write buffer flush
 */
LEAF(kn5800_pg_iflush)
	lw	t1,icache_size
	.set noreorder
	nop
	mfc0	t3,C0_SR		# save sr
	nop
	mtc0	zero,C0_SR		# interrupts off
	nop
	.set reorder
	.set	noreorder
	la	v0,1f
	or	v0,K1BASE
	j	v0			# run uncached
	nop				# BDSLOT

	/*
	 * flush text cache
	 */
1:	lw	v0,0(a1)		# isis hack to flush write buffer
	li	v0,SR_ISC|SR_SWC	# disable intr, isolate and swap
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
#ifdef CACHETRICKS
	lw	v0,icachemask
	srl	t1,a0,PGSHIFT
	and	t1,v0
	sll	t1,1			# cachecnt index
	lhu	t0,icachecnt(t1)
	addu	t0,1
	sh	t0,icachecnt(t1)
#endif /* CACHETRICKS */
	j	ra
	END(kn5800_pg_iflush)



/*
 * kn5800_page_dflush(addr, wbfladdr)
 * flush one page of i cache, addr is assumed to be in K0SEG
 * wbfladdr is the address to read to cause a write buffer flush
 */
LEAF(kn5800_pg_dflush)
	lw	t2,dcache_size
	.set noreorder
	nop
	mfc0	t3,C0_SR		# save sr
	nop
	.set reorder

	.set	noreorder
	lw	v0,0(a1)		# isis hack to flush write buffer
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
#ifdef CACHETRICKS
	lw	v0,dcachemask
	srl	t1,a0,PGSHIFT
	and	t1,v0
	sll	t1,1			# cachecnt index
	lhu	t0,dcachecnt(t1)
	addu	t0,1
	sh	t0,dcachecnt(t1)
#endif /* CACHETRICKS */
	j	ra
	END(kkn5800_pg_dflush)
