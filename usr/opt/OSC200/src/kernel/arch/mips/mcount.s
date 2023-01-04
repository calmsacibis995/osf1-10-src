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
 *	@(#)$RCSfile: mcount.s,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:16:14 $
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

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * mcount.s -- profiling routines for invocation and basic block counting
 *
 * WARNING: THIS MODULE MUST BE COMPILED -p0
 *
 *	PROFTYPE == 1	=>	only pc sampling, no counting
 *	PROFTYPE == 2	=>	invocation counting
 *	PROFTYPE == 3	=>	basic block counting
 *	PROFTYPE == 4	=>	gprof style counting
 *
 * Calls to these routines are automatically generated by the mips assembler
 * and appear as the following code sequence:
 *		.set	noreorder
 *		.set	noat
 *		move	AT,ra		# save current return address
 *		jal	_mcount		# call profiling counting routine
 *		subu	sp,8		# BDSLOT: alloc stack space for 2 words
 *		# stack space must be free'd by _mcount
 *		.set	reorder
 *		.set	at
 */

#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/cpu.h>
#include <assym.s>

#ifdef PROFILING

#if PROFTYPE == 1

/*
 * Dummy _mcount when no counting is being performed
 */
LEAF(_mcount)
	.set	noreorder
	.set	noat
	addu	sp,8
	j	ra
	move	ra,AT
	.set	at
	.set	reorder
	END(_mcount)

#endif /* PROFTYPE == 1 */

#if PROFTYPE == 2 || PROFTYPE == 3

LEAF(_mcount)
	.set	noreorder
	sw	AT,4(sp)	# save caller's return address
	lw	AT,_mcountoff	# offset from start of text to data
	sw	fp,0(sp)	# save temp register
	.set	noat
	beqz	AT,$bailout	# no memory for counters
	li	fp, 0x1fffffff
	and	fp, ra, fp
	li	AT, 0x20000
	sub	fp,AT		# counter is at _mcountoff + (return
	#sub	fp, ra, 0x80020000
	lw	AT, _mcountoff
	srl	fp,3		# address) / 2; aligned to word
	sll	fp,2
	addu	AT,fp
	lw	fp,0(AT)	# get counter
	nop
	addu	fp,1
	sw	fp,0(AT)
$bailout:
	lw	AT,4(sp)	# reload callers return address
	lw	fp,0(sp)	# reload temp register
	addu	sp,8		# fixup stack
	j	ra		# return
	move	ra,AT		# BDSLOT: restore caller's return address
	.set	reorder
	.set	at
	END(_mcount)

#endif /* PROFTYPE == 2 || PROFTYPE == 3 */

#if PROFTYPE == 4
/*
 * _mcount for GPROF style profiling
 * maintains information for invocation counts by each calling site to
 * a called routine
 *
 * Assumes all caller-saved registers are free at _mcount call time
 */

/*
 * register usage
 */

#define Spl	t0
#define Selfpc	ra
#define Fromcp	t2
#define Top	t3
#define Prevtop	t4
#define Toindex	t5
#define Atsave	t6
#define Tmp1	t8
#define Tmp2	t9

LEAF(_mcount)
	.set	noat
	lw	Tmp1,profiling
	move	Atsave,AT		# needed in out
	bne	Tmp1,zero,$out		# profiling off
	.set	at
	mfc0	Spl,C0_SR
	mtc0	zero,C0_SR		# interrupts off
	lw	Tmp2,s_lowpc
	subu	Fromcp,Atsave,Tmp2	# offset into text
	lw	Tmp1,s_textsize
	bgeu	Fromcp,Tmp1,$done	# outside known text region
#if HASHFRACTION != 0
	divu	Fromcp,HASHFRACTION	# convert to short index
#endif
	lw	Tmp2,froms
	addu	Fromcp,Tmp2		# froms pointer
	lhu	Toindex,0(Fromcp)
	bne	Toindex,zero,$notfirst	# this arc traversed before
	lw	Tmp2,tos
	lhu	Toindex,TOS_LINK(Tmp2)	# allocate new tos entry
	addu	Toindex,1
	lw	Tmp1,tolimit		# max tos
	bgeu	Toindex,Tmp1,$overflow	# out of tos
	sh	Toindex,TOS_LINK(Tmp2)
	sh	Toindex,0(Fromcp)	# link to new entry
	mul	Top,Toindex,TOS_SIZE	# tos offset
	addu	Top,Tmp2		# pointer to tos entry
	sw	Selfpc,TOS_SELFPC(Top)	# fill in new entry
	li	Tmp1,1
	sw	Tmp1,TOS_COUNT(Top)
	sh	zero,TOS_LINK(Top)
	b	$done

$trvlink:
	lw	Tmp1,TOS_LINK(Top)	# next entry index
	bne	Tmp1,zero,$chklink	# check next entry
	/*
	 * end of link list and not found, allocate new tos entry
	 */
	lw	Tmp2,tos
	lhu	Toindex,TOS_LINK(Tmp2)	# allocate new tos entry
	addu	Toindex,1
	lw	Tmp1,tolimit		# max tos
	bgeu	Toindex,Tmp1,$overflow	# out of tos
	sh	Toindex,TOS_LINK(Tmp2)
	mul	Top,Toindex,TOS_SIZE	# tos offset
	addu	Top,Tmp2		# pointer to tos entry
	sw	Selfpc,TOS_SELFPC(Top)	# fill in new entry
	li	Tmp1,1
	sw	Tmp1,TOS_COUNT(Top)
	lhu	Tmp2,0(Fromcp)		# new entry pts to previous head
	sh	Tmp2,TOS_LINK(Top)
	sh	Toindex,0(Fromcp)	# head points to new entry
	b	$done

$chklink:
	move	Prevtop,Top
	mul	Top,Tmp1,TOS_SIZE	# Top = Top->link
	lw	Tmp2,tos
	addu	Top,Tmp2
	lw	Tmp1,TOS_SELFPC(Top)
	bne	Tmp1,Selfpc,$trvlink	# not this one, try next
	lw	Tmp2,TOS_COUNT(Top)
	addu	Tmp2,1
	sw	Tmp2,TOS_COUNT(Top)
	lhu	Toindex,TOS_LINK(Prevtop)
	lhu	Tmp1,TOS_LINK(Top)
	sh	Tmp1,TOS_LINK(Prevtop)
	lhu	Tmp2,0(Fromcp)		# new entry pts to previous head
	sh	Tmp2,TOS_LINK(Top)
	sh	Toindex,0(Fromcp)	# head points to new entry
	b	$done

$notfirst:
	mul	Top,Toindex,TOS_SIZE	# tos offset
	lw	Tmp1,tos
	addu	Top,Tmp1		# pointer to tos entry
	lw	Tmp2,TOS_SELFPC(Top)	# selfpc for this node
	bne	Tmp2,Selfpc,$trvlink	# traverse the link
	lw	Tmp1,TOS_COUNT(Top)	# bump count
	addu	Tmp1,1
	sw	Tmp1,TOS_COUNT(Top)

$done:
	mtc0	Spl,C0_SR
$out:
	.set	noreorder
	addu	sp,8
	j	ra
	move	ra,Atsave
	.set	reorder

$overflow:
	li	Tmp1,3
	sw	Tmp1,profiling
	PANIC	("TOS OVERFLOW")
	END(_mcount)

#endif /* PROFTYPE == 4 */

#endif /* PROFILING */
