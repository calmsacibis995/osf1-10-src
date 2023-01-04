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
 *	@(#)$RCSfile: coproc_cntrl.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 15:09:46 $
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
 * derived from coproc_control.s	2.4      (ULTRIX)        10/13/89
 */
/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/*
 * This file contains routines to get and set the coprocessor point control
 * registers.
 */

/* Revision History
 *
 * 19-Nov-1990	dutile
 *	merge of snap5 & osc25; nofault access through
 * 	current_pcb structure
 *
 * 20-Jul-1990	burns
 *	first hack at moving to OSF/1 (snap3)
 *
 * 13-Oct-89 -- gmm
 *	smp changes. Access nofault through cpudata 
 */
 
#include <machine/regdef.h>
#include <machine/fpu.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <assym.s>

/*
 * get_fpc_csr returns the fpc_csr.
 */
LEAF(get_fpc_csr)
#ifndef SABLE
	.set	noreorder
	mfc0	v1,C0_SR
	li	a0,SR_CU1
	mtc0	a0,C0_SR
	nop				# before we can really use cp1
	nop				# before we can really use cp1
	cfc1	v0,fpc_csr
	nop				# from OSF
	mtc0	v1,C0_SR
	.set	reorder
#else /* SABLE */
	move	v0,zero
#endif /* !SABLE */
	j	ra
	END(get_fpc_csr)

/*
 * set_fpc_csr sets the fpc_csr and returns the old fpc_csr.
 */
LEAF(set_fpc_csr)
#ifndef SABLE
	.set	noreorder
	mfc0	v1,C0_SR
	li	a1,SR_CU1
	mtc0	a1,C0_SR
	nop				# before we can really use cp1
	nop				# before we can really use cp1
	cfc1	v0,fpc_csr
	nop				# from OSF
	ctc1	a0,fpc_csr
	nop				# from OSF
	mtc0	v1,C0_SR
	.set	reorder
#endif /* !SABLE */
	j	ra
	END(set_fpc_csr)

/*
 * get_fpc_eir returns the fpc_eir.
 */
LEAF(get_fpc_eir)
#ifndef SABLE
	.set	noreorder
	mfc0	v1,C0_SR
	li	a0,SR_CU1
	mtc0	a0,C0_SR
	nop				# before we can really use cp1
	nop				# before we can really use cp1
	cfc1	v0,fpc_eir
	nop				# from OSF
	mtc0	v1,C0_SR
	.set	reorder
#else /* SABLE */
	move	v0,zero
#endif /* !SABLE */
	j	ra
	END(get_fpc_eir)

/*
 * get_cpu_irr -- returns cpu revision id word
 * returns zero for cpu chips without a PRID register
 */
LEAF(get_cpu_irr)
#ifndef SABLE
	.set noreorder
	nop
	mfc0	a1,C0_SR		# save sr
	nop
	mtc0	zero,C0_SR		# interrupts off
	nop
	.set reorder


#ifdef ASSERTIONS
	li	a2,PCB_WIRED_ADDRESS
	lw	a0,PCB_NOFAULT(a2)
	beq	a0,zero,8f
	PANIC("recursive nofault")
8:
#else
	li	a2,PCB_WIRED_ADDRESS		# osc25 addition
#endif /* ASSERTIONS */
	li	a0,NF_REVID		# set-up nofault handling
	sw	a0,PCB_NOFAULT(a2)
	move	v0,zero			# return zero for chips w/o PRID reg
	.set	noreorder
	li	a0,SR_BEV		# chips w/o PRID don't have BEV
	mtc0	a0,C0_SR
	nop
	nop
	mfc0	a0,C0_SR
	nop
	and	a0,SR_BEV
	beq	a0,zero,1f		# no BEV, so no PRID
	nop
	mfc0	v0,C0_PRID
1:
	.set	reorder
	sw	zero,PCB_NOFAULT(a2)
	j	ra
#else /* SABLE */
	move	v0,zero
	j	ra
#endif /* !SABLE */
	END(get_cpu_irr)

/*
 * get_fpc_irr -- returns fp chip revision id
 * NOTE: should not be called if no fp chip is present
 */
LEAF(get_fpc_irr)
#ifndef SABLE
	.set	noreorder
	mfc0	a1,C0_SR		# save sr
	nop
	mtc0	zero,C0_SR
#ifdef ASSERTIONS
	li	a2,PCB_WIRED_ADDRESS
	lw	a0,PCB_NOFAULT(a2)
	nop
	beq	a0,zero,8f
	nop
	PANIC("recursive nofault")
8:
#else
	li	a2,PCB_WIRED_ADDRESS
#endif /* ASSERTIONS */
	li	a0,NF_REVID		# LDSLOT
	sw	a0,PCB_NOFAULT(a2)
	li	v0,SR_CU1
	mtc0	v0,C0_SR		# set fp usable
	nop				# before we can really use cp1
	nop				# before we can really use cp1
	cfc1	v0,fpc_irr		# get revision id
	sw	zero,PCB_NOFAULT(a2)
	mtc0	a1,C0_SR
	j	ra
	nop
	.set	reorder
#else /* SABLE */
	move	v0,zero
	j	ra
#endif /* !SABLE */
	END(get_fpc_irr)

/*
 * set_fpc_led -- set floating point board leds
 */
LEAF(set_fpc_led)
#ifndef SABLE
	.set	noreorder
	mfc0	v1,C0_SR
	li	a1,SR_CU1
	mtc0	a1,C0_SR
	nop				# before we can really use cp1
	nop				# before we can really use cp1
	ctc1	a0,fpc_led		# set leds
	mtc0	v1,C0_SR
	.set	reorder
#endif /* !SABLE */
	j	ra
	END(set_fpc_led)

LEAF(reviderror)
	.set noreorder
	nop
	mtc0	a1,C0_SR		# restore sr
	nop
	.set reorder
	move	v0,zero
	j	ra
	END(reviderror)
