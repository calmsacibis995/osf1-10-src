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
 *	@(#)$RCSfile: tlb.s,v $ $Revision: 1.2.4.3 $ (DEC) $Date: 1992/07/08 08:49:21 $
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
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 *
 * File: tlb.s
 *	
 *	Assembly functions to manipulate the mips TLB.
 *
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>

#include <assym.s>

	.set	noreorder

/* MACRO - internal
 *
 * tlb_probe_entry(pid,addr,oldpid,entry,index)
 *
 * Purpose
 *	Common code to probe for an entry in the tlb.
 *
 * Description
 *	Preserves the current pid, returns the index
 *	of the mapping for (pid,addr). If none is
 *	found returns a negative value.
 *	As a byproduct, leaves the appropriate value
 *	for the tlbhi register in entry.
 *	Note that the macro needs a delay cycle to
 *	complete and make 'index' valid.
 */
#define tlb_probe_entry(pid,addr,oldpid,entry,index)	\
	mfc0	oldpid,C0_TLBHI;	/* save current tlb pid */	\
	and	addr,TLBHI_VPNMASK;	/* drop offset from address */	\
	sll	pid,TLBHI_PIDSHIFT;	/* shift pid in place */	\
	or	entry,addr,pid;		/* tlbhi value to look for */	\
	mtc0	entry,C0_TLBHI;		/* put args into tlbhi */	\
	nop;				/* let tlbhi get through pipe */\
	c0	C0_PROBE;		/* probe for entry */		\
	nop;				/* pipeline */			\
	nop;				/* pipeline */			\
	mfc0	index,C0_INX;		/* see what happened */


/*
 * tlb_probe(pid, vaddr)
 *
 * Purpose
 *	Probe the tlb for a valid mapping of (pid,vaddr)
 *
 * Description
 *	Returns 1 if the tuple is mapped, else 0
 */
LEAF(tlb_probe)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR

	tlb_probe_entry(a0,a1,t1,a0,a0)

	nop
	bltz	a0,1f			# found any ?
	move	v0,zero			# assume not
	li	v0,1
1:	mtc0	t1,C0_TLBHI		# restore old tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore sr and return
	END(tlb_probe)


/*
 * tlb_zero(i)
 *
 * Purpose
 *	Invalidate the i-th TLB entry.
 *
 * Description
 *	Puts some special neutral mapping at entry i.
 *	Uses a virtual address in the k1seg, which will
 *	not be looked for translation. Uses zero for
 *	the physical page, for cosmetic reasons.
 */
LEAF(tlb_zero)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	li	t2,K1BASE&TLBHI_VPNMASK	# neutral vaddr
	mfc0	t1,C0_TLBHI		# save current tlbpid
	sll	a0,TLBINX_INXSHIFT	# shift pid in place
	mtc0	t2,C0_TLBHI		# set VPN and TLBPID
	mtc0	zero,C0_TLBLO		# set PPN and protection
	mtc0	a0,C0_INX		# set INDEX
	nop
	c0	C0_WRITEI		# drop it in
	nop
	mtc0	t1,C0_TLBHI		# restore old tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(tlb_zero)

/*
 * tlb_map(line, pid, vaddr, pte)
 *
 * Purpose
 *	Insert a mapping in a tlb line
 *
 * Description
 *	Builds a tlb entry for (pid,vaddr,pte) and inserts
 *	it at the given tlb line. Valid lines are indexed
 *	from 0 to NTLBENTRIES, but validity is not checked for.
 *	Address does not need to be rounded.
 *	Note that no check is made against conflicting mappings.
 */
LEAF(tlb_map)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	sll	a0,TLBINX_INXSHIFT	# shift line index in place
	mfc0	t1,C0_TLBHI		# save current tlbpid
	sll	a1,TLBHI_PIDSHIFT	# line up pid bits
	and	a2,TLBHI_VPNMASK	# chop offset bits
	or	a1,a2
	mtc0	a1,C0_TLBHI		# set VPN and TLBPID
	mtc0	a3,C0_TLBLO		# set PPN and access bits
	mtc0	a0,C0_INX		# set INDEX to wired entry
	nop
	c0	C0_WRITEI		# drop it in
	nop
	mtc0	t1,C0_TLBHI		# restore old tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(tlbwired)

/*
 * tlb_map_random(pid, vaddr, pte)
 *
 * Purpose
 *	Insert a mapping someplace in the tlb
 *
 * Description
 *	Like tlb_map(), but uses one of the lines indexed by
 *	the random register (randomly :-).
 *	This one also checks against existing mapping
 *	that would cause conflict and overrides them.
 *	Returns whether this happened (1) or not (0).
 */

LEAF(tlb_map_random)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR

	tlb_probe_entry(a0,a1,t1,t2,t3)

	move	v0,zero			# assume no conflicts
	bltz	t3,1f			# not found
	mtc0	a2,C0_TLBLO		# pte for new entry (BDSLOT)
	nop
	c0	C0_WRITEI		# re-use line
	b	2f
	li	v0,1			# overridden
1:
	nop
	c0	C0_WRITER		# use random slot
	nop
2:	mtc0	t1,C0_TLBHI		# restore tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(tlb_map_random)

/*
 * tlb_modify(pid,vaddr,writeable)
 *
 * Purpose
 *	Set or clear the writeable bit of an entry
 *
 * Description
 *	Probes first for a mapping, if found changes it
 *	to be writeable or not as requested.
 *	[Protection is independent of user/kernel mode.]
 */

LEAF(tlb_modify)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR

	tlb_probe_entry(a0,a1,t1,t2,t3)

	nop
	bltz	t3,9f
	move	v0,zero			# assume probe failed (BDSLOT)
	c0	C0_READI		# load entry in TLBLO/TLBHI
	nop
	nop
	mfc0	t2,C0_TLBLO
	bne	a2,zero,1f		# make it writeable ?
	or	t2,TLBLO_D		# set writeable bit (BDSLOT)
	and	t2,~TLBLO_D		# clear writeable bit
1:	mtc0	t2,C0_TLBLO		# change entry
	nop
	c0	C0_WRITEI
	nop
9:	mtc0	t1,C0_TLBHI		# restore old tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(tlb_modify)

/*
 * tlb_unmap(pid, vaddr)
 *
 * Purpose
 *	Remove the mapping for address vaddr with id pid.
 *
 * Description
 *	Probes to see if there is any.  Note that
 *	interrupts must be disabled in this and the other
 *	tlb operations since an interrupt could cause
 *	a miss which would alter the content of the 
 *	tlb coprocessor registers.
 */

LEAF(tlb_unmap)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR

	tlb_probe_entry(a0,a1,t1,t2,t3)

	move	v0,zero			# assume not there
	bltz	t3,1f			# probe failed
	li	t2,K1BASE&TLBHI_VPNMASK	# invalid vaddr (not 0!)
	mtc0	t2,C0_TLBHI		# invalidate entry
	mtc0	zero,C0_TLBLO		# cosmetic
	nop
	c0	C0_WRITEI
	nop
1:	mtc0	t1,C0_TLBHI		# restore old tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(tlb_unmap)


/*
 * tlb_flush(from_line,to_line)
 *
 * Purpose
 *	Flush all tlb lines in the given range (inclusive)
 *
 * Description
 *	Zeroes all entries in the given range of lines.
 *	Could be done with tlb_zero, but this is slightly
 *	faster. Besides, one would probably expect this
 *	function anyways.
 */
LEAF(tlb_flush)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	li	t2,K1BASE&TLBHI_VPNMASK	# set up to invalidate entries
	mfc0	t1,C0_TLBHI		# save current tlbpid
	mtc0	zero,C0_TLBLO		# setup neutral mapping
	mtc0	t2,C0_TLBHI
	sll	t2,a0,TLBINX_INXSHIFT
1:
	mtc0	t2,C0_INX		# set index
	addu	a0,1			# bump to next entry
	c0	C0_WRITEI		# invalidate
	bne	a0,a1,1b		# more to do
	sll	t2,a0,TLBINX_INXSHIFT	# prepare index (BDSLOT)

	mtc0	t1,C0_TLBHI		# restore tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(flush_tlb)
/*
 * tlb_flush_range(pid, vpnl, vpnh)
 * Flush VPN belonging to pid from pvnl to pvnh
 * This routine doesn't scan the wired entries.
 */

LEAF(tlb_flush_range)
	mfc0	t0,C0_SR		# save SR 
	mtc0	zero,C0_SR		# disable interrupts
	li	t2,K1BASE&TLBHI_VPNMASK	# set up to invalidate entries
	mfc0	t1,C0_TLBHI		# save current tlbpid
	li	t3,NWIREDENTRIES	# start at wired base
	li	t7,NTLBENTRIES		# number entries
	sll	a0,TLBHI_PIDSHIFT	# line up pid
	sll	a1,TLBHI_VPNSHIFT	# line vpnl up
	sll	a2,TLBHI_VPNSHIFT	# line vpnh up
	or	a1,a1,a0		# vpnl!pid
	or	a2,a2,a0		# vpnh!pid
	sll	v0,t3,TLBINX_INXSHIFT	# shift into position
1:	mtc0	v0,C0_INX		# set index
	nop
	c0	C0_READI		# read the entry
	mfc0	t4,C0_TLBHI		# fetch high part of entry
	addu	t3,1			# bump index
	mtc0	zero,C0_TLBLO		# assume a match
	and	t5,t4,TLBHI_PIDMASK	# does the pid match ?
	bne	t5,a0,2f		# no
	sltu	t6,t4,a1		# compare low bound
	bne	t6,zero,2f		# failed
	sltu	t6,a2,t4		# compare high bound
	bne	t6,zero,2f		# failed;
	nop
	mtc0	t2,C0_TLBHI		# flush it
	nop
	c0	C0_WRITEI		# invalidate
2:	bne	t3,t7,1b		# more to do
	sll	v0,t3,TLBINX_INXSHIFT	# shift into position

	mtc0	t1,C0_TLBHI		# restore tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(tlb_flush_range)

/*
 * tlb_flush_pid(pid,from_line,to_line)
 *
 * Purpose
 *	Flush all the entries with the given pid
 *
 * Description
 *	Makes sure no entries for the given pid are in the tlb.
 *	Walks through all entries in the given range (inclusive).
 *	
 */
LEAF(tlb_flush_pid)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	li	t2,K1BASE&TLBHI_VPNMASK	# set up to invalidate entries
	mfc0	t1,C0_TLBHI		# save current tlbpid
	sll	t3,a1,TLBINX_INXSHIFT	# starting index
	sll	a0,TLBHI_PIDSHIFT	# lined up pid to check against
1:
	mtc0	t3,C0_INX		# set index
	addu	a1,1			# bump to next entry
	c0	C0_READI		# read the entry
	nop
	nop
	mfc0	t4,C0_TLBHI
	mtc0	zero,C0_TLBLO		# assume a match
	and	t4,TLBHI_PIDMASK	# does the pid match ?
	bne	t4,a0,2f
	mtc0	t2,C0_TLBHI		# yep, flush it
	nop
	c0	C0_WRITEI		# invalidate
2:	bne	a1,a2,1b		# more to do
	sll	t3,a1,TLBINX_INXSHIFT	# prepare next index (BDSLOT)

	mtc0	t1,C0_TLBHI		# restore tlbpid
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(flush_tlbpid)

/*
 * tlb_set_context(pmap)
 *
 * Purpose
 *	Setup the tlb to use the given pmap
 *
 * Description
 *	Changes the current TLBPID, sets the CONTEXT register
 *	for use by the utlbmiss handler.
 *	Used at initialization and context switching.
 */

LEAF(tlb_set_context)
	mfc0	t0,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
#if NCPUS > 1
	li      t3,PCB_WIRED_ADDRESS
	lw      t3,PCB_CPU_NUMBER(t3)
	lw	t2,PMAP_PTEBASE(a0)	# get page table base
	sll     t3,2
	addu	t1,t3,a0
	lw	t1,PMAP_PID(t1)		# get new tlbpid
	mtc0	t2,C0_CTXT		# utlbmiss is happy now
	sll	t1,TLBHI_PIDSHIFT	# line up pid bits
	and	t1,TLBHI_PIDMASK	# sanity
	mtc0	t1,C0_TLBHI		# assert new pid
	la	t1,active_pmap
	addu	t1,t1,t3
	sw	a0,0(t1)		# assert new pmap
#else /* NCPUS = 1 */
	lw	t2,PMAP_PTEBASE(a0)	# get page table base
	lw	t1,PMAP_PID(a0)		# get new tlbpid
	mtc0	t2,C0_CTXT		# utlbmiss is happy now
	sll	t1,TLBHI_PIDSHIFT	# line up pid bits
	and	t1,TLBHI_PIDMASK	# sanity
	mtc0	t1,C0_TLBHI		# assert new pid
	sw	a0,active_pmap		# assert new pmap

#endif /* NCPUS > 1 */
	j	ra
	mtc0	t0,C0_SR		# restore SR and return
	END(set_tlb_context)

/*
 * tlb_probe_and_wire(xxx,entry,pte,index)
 *
 * Purpose
 *	Wire an (existing) tlb entry
 *
 * Description
 *	Probe for 'entry' (a formatted TLBHI value) in the TLB.
 *	If a line exists for it, remove it.
 *	Insert the entry at line index.
 *
 * Calling sequence
 *		a0 -- 		preserved
 *		a1 -- entry	clobbered
 *		a2 -- pte	clobbered
 *		a3 -- index	clobbered
 *		v0 -- 		clobbered
 *
 * Side effects
 *	Changes the TLBPID.
 */

LEAF(tlb_probe_and_wire)
	mtc0	a2,C0_TLBHI		# probe (vaddr,0)
	nop
	c0	C0_PROBE
	nop
	nop
	mfc0	v0,C0_INX		# get index
	nop
	bltz	v0,1f			# nope, missing
	nop
	beq	v0,a3,1f		# already in place ?
	li	v0,K1BASE&TLBHI_VPNMASK

	mtc0	v0,C0_TLBHI		# invalidate old one
	mtc0	zero,C0_TLBLO
	nop
	c0	C0_WRITEI
	nop
	nop				# fall through

1:	/* Entry not (anymore) in tlb, wire it */
	mtc0	a3,C0_INX
	mtc0	a1,C0_TLBLO
	mtc0	a2,C0_TLBHI
	nop
	c0	C0_WRITEI
	j	ra
	nop

	END(tlb_probe_and_wire)
/*
 * save_tlb(save_array) -- snapshots the tlb
 */

LEAF(save_tlb)
	.set	noreorder
	li	v0,TLBWIREDBASE		# first entry to save
	li	v1,NTLBENTRIES		# last entry plus one
	mfc0	t1,C0_TLBHI		# save pid
	mfc0	t2,C0_SR
	mtc0	zero,C0_SR		# interrupts off
1:
	sll	t0,v0,TLBINX_INXSHIFT	# set tlb index and read
	mtc0	t0,C0_INX
	nop
	c0	C0_READI
	mfc0	t5,C0_TLBHI		# Copy entry
	nop
	sw	t5,0(a0)
	addu	a0,4
	mfc0	t5,C0_TLBLO
	nop
	sw	t5,0(a0)
	addu	a0,4
	addu	v0,1			# bump to next tlb slot
	bne	v0,v1,1b		# done?
	sll	t0,v0,TLBINX_INXSHIFT	# BDSLOT
	mtc0	t1,C0_TLBHI
	nop
	mtc0	t2,C0_SR
	.set	reorder
	j	ra
	END(save_tlb)
