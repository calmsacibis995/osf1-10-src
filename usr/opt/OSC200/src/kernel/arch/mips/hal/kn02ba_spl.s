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
 * @(#)$RCSfile: kn02ba_spl.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 16:49:15 $
 */

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include <hal/kn02ba.h>

IMPORT(splm,SPLMSIZE*4)
IMPORT(kn02ba_sim,SPLMSIZE*4)
IMPORT(ipllevel,4)

/*
 * kn02ba_ routines are size sensitive. They are bcopied over the
 * spl routines. Any changes must be disassembled and any
 * adjustments to the nop's in the spl routines made.
 */

/*
 * kn02ba_spl0: Don't block against anything.
 * This should work on all machines.
 */
LEAF(kn02ba_spl0)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLNONE
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLNONE*4# get system interrupt mask value
	lw	v1, splm+SPLNONE*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_spl0)

/*
 * kn02ba_splsoftclock: block against clock software interrupts (level 1 softint).
 */
LEAF(kn02ba_splsoftclock)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLSOFTC
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLSOFTC*4 # get system interrupt mask value
	lw	v1, splm+SPLSOFTC*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_splsoftclock)
/*
 * kn02ba_splnet: block against network software interrupts (level 2 softint).
 */
LEAF(kn02ba_splnet)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLNET
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLNET*4	# get system interrupt mask value
	lw	v1, splm+SPLNET*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_splnet)


/*
 * kn02ba_splbio: block against all I/O device interrupts. all are vme
 */
LEAF(kn02ba_splbio)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLIO
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLIO*4	# get system interrupt mask value
	lw	v1, splm+SPLIO*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_splbio)

/*
 * kn02ba_splimp: block against network device interrupts
 * NOTE: the vax version of this routine blocks hardclocks.
 */
LEAF(kn02ba_splimp)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLIO
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLIO*4	# get system interrupt mask value
	lw	v1, splm+SPLIO*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_splimp)


/*
 * kn02ba_spltty: block against tty device interrupts. console uart and vme
 */
LEAF(kn02ba_spltty)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLIO
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLIO*4	# get system interrupt mask value
	lw	v1, splm+SPLIO*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_spltty)

/*
 * kn02ba_splclock: block against sched clock interrupts.
 */
LEAF(kn02ba_splclock)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLCLOCK
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLCLOCK*4 # get system interrupt mask value
	lw	v1, splm+SPLCLOCK*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_splclock)

/*
 * kn02ba_splmem: block against memory error interrupts
 */
LEAF(kn02ba_splvm)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLMEM
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLMEM*4	# get system interrupt mask value
	lw	v1, splm+SPLMEM*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_splmem)


/*
 * kn02ba_splfpu: block against fpu interrupts
 */
LEAF(kn02ba_splfpu)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK	# disable interrupts
	mtc0	v1, C0_SR
	nop
	lw	v0, ipllevel		# load return value with current ipl
	li	v1, SPLEXTREME
	sw	v1, ipllevel		# store new ipl into ipllevel
	lw	a0, kn02ba_sim+SPLEXTREME*4# get system interrupt mask value
	lw	v1, splm+SPLEXTREME*4	# get status register mask value
	sw	a0, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a0, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	v1, C0_SR		# load status register with value
	j	ra
	nop
	.set	reorder
	END(kn02ba_splfpu)

/*
 * kn02ba_splx(ipl) -- restore previously saved ipl
 */
LEAF(kn02ba_splx)
	.set	noreorder
	li	v1, KN02BA_SPL_MASK
	mtc0	v1, C0_SR		# disable interrupts
	nop
	lw	v0, ipllevel		# load return value with current ipl
	sll	a1, a0, 2		# multiply by 4
	lw	v1, kn02ba_sim(a1)	# get system interrupt mask value
	sw	a0, ipllevel		# store new ipl into ipllevel
	lw	a2, splm(a1)		# get status register mask value
	sw	v1, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	v1, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	mtc0	a2, C0_SR		# load status register with new mask
	j	ra
	nop
	.set	reorder
	END(kn02ba_splx)

