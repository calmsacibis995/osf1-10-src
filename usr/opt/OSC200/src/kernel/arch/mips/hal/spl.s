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
 * @(#)$RCSfile: spl.s,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/04/14 17:41:20 $
 */

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include <hal/kn02ba.h>
#include <rt_preempt.h>

/*
 * Spl levels are not hardwired. They vary from one system
 * to another. Values for the logical levels are taken
 * from the array 'splm'. This array is initialized early
 * on in processor configuration from a system dependent.
 * splm array. The logical spl levels are defined in cpu.h.
 * 
 */


BSS(splm,SPLMSIZE*4)

/*
 * Standard spl routines, used by almost all mips based systems.
 * CAUTION: Do not remove the extra nop's at the end for each
 * routine. They are there to reserve space for brain dead systems
 * that require more bogus spl routines so that they can bcopy
 * down their trash. This way the only price paid by smart systems
 * are a few bytes of extra space. The 11 nop size was determined
 * using disassembled output. Don't be fooled by the source.
 *
 * DOUBLE CAUTION: DO NOT move any of the spl code, the bcopy
 * size determination knows the current order. It will break
 * royally if mucked with.
 */

#if	!RT_PREEMPT
LEAF(spl0)
XLEAF(splnone)
#else	/* !RT_PREEMPT */
LEAF(spl0x)
#endif	/* !RT_PREEMPT */
	.set	noreorder
	lw	v1,splm+(SPLNONE)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
/*
 * Label end of function so that we can use it to
 * calculate the size of the function when we bcopy
 * the spl functions of brain dead machines.
 */
EXPORT(espl0)
	.set	reorder
	END(spl0)

LEAF(splsoftclock)
	.set	noreorder
	lw	v1,splm+(SPLSOFTC)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(splsoftclock)

LEAF(splnet)
XLEAF(splstr)
	.set	noreorder
	lw	v1,splm+(SPLNET)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(splnet)

LEAF(splbio)
	.set	noreorder
	lw	v1,splm+(SPLBIO)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(splbio)

LEAF(splimp)
	.set	noreorder
	lw	v1,splm+(SPLIMP)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(splimp)

LEAF(spltty)
XLEAF(splcons)
	.set	noreorder
	lw	v1,splm+(SPLTTY)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(spltty)

LEAF(splclock)
XLEAF(spl7)
	.set	noreorder
	lw	v1,splm+(SPLCLOCK)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(splclock)

LEAF(splvm)
XLEAF(splmem)
	.set	noreorder
	lw	v1,splm+(SPLVM)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(splvm)

LEAF(splfpu)
XLEAF(splhigh)
XLEAF(splsched)
	.set	noreorder
	lw	v1,splm+(SPLFPU)*4
	mfc0	v0,C0_SR
	mtc0	v1,C0_SR
	j	ra
	nop			/* Needed anyway */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	.set	reorder
	END(splfpu)

#if	!RT_PREEMPT
/*
 * splx(ipl) -- restore previously saved ipl
 */
LEAF(splx)
#else	/* !RT_PREEMPT */
/*
 * splx1(ipl) -- restore previously saved ipl
 */
LEAF(splx1)
#endif	/* !RT_PREEMPT */
	.set	noreorder
	mfc0	v0,C0_SR
/* Temporary fix to avoid caller setting BEV bit in status register */
	li	t0,0xffbfffff	# Get the BEV bit
	and	a0,t0		# Clear BEV bit
	nop
	mtc0	a0,C0_SR
	j	ra
	nop
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
	nop			/* space saver - see comment above */
/*
 * Label end of function so that we can use it to
 * calculate the size of the function when we bcopy
 * the spl functions of brain dead machines.
 */
EXPORT(esplx)
	.set	reorder
	END(splx)


