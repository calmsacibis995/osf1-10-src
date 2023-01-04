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
 *	@(#)$RCSfile: fp_intr.s,v $ $Revision: 1.2.3.5 $ (DEC) $Date: 1992/06/24 15:48:05 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from fp_intr.s	2.1	(ULTRIX/OSF)	12/3/90
 */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * fp_intr.s -- floating pointer interrupt handler
 */

#include <machine/cpu.h>
#include <machine/fpu.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <machine/asm.h>
#include <machine/pcb.h>
#include <machine/thread.h>
#include <assym.s>

#include <machine/softfp.h>
#include <sys/signal.h>
#include <cpus.h>

IMPORT(splm, SPLMSIZE*4)

/*
 * Register setup before calling an interrupt handler (from intr() in 
 * mips/trap.c through the table of routines registered with the interrupt
 * dispatcher (see handler_dispatch()).
 *	a0 -- exception frame pointer
 *
 * This routine is called from a 'C' routine thus the registers are
 * handled accordingly.
 *
 * To get to here there must be some form of floating-point coprocessor
 * hardware to generate the interrupt.
 *
 * The pointer to the thread structure which currently owns the floating point
 * unit is in fpowner.  And the current values of the floating-point registers
 * are in the floating-point registers in the coprocessor.
 *
 * To exit this routine any modified value of a floating-point register
 * is just left in that register and then a return to the caller is done.
 */

/*
 * Floating-point coprocessor interrupt handler
 */
#define	FRAME_SIZE	24
#define	LOCAL_SIZE	8
#define	A0_OFFSET	FRAME_SIZE+4*0
#define	A1_OFFSET	FRAME_SIZE+4*1
#define	A2_OFFSET	FRAME_SIZE+4*2
#define	A3_OFFSET	FRAME_SIZE+4*3
#define	RA_OFFSET	FRAME_SIZE-LOCAL_SIZE+4*0
#define	V0_OFFSET	FRAME_SIZE-LOCAL_SIZE+4*1

NESTED(fp_intr, FRAME_SIZE, ra)
	subu	sp,FRAME_SIZE
	sw	ra,RA_OFFSET(sp)
	sw	a0,A0_OFFSET(sp)	# save exception frame pointer
	.mask	0x80000002, -LOCAL_SIZE

#ifdef	DEBUG
	PRINTF("fp_intr\n");
	lw	a0,A0_OFFSET(sp)
#endif
#if     NCPUS > 1
        li      a3,PCB_WIRED_ADDRESS
        lw      a3,PCB_CPU_NUMBER(a3)
        la      a1,fpowner_array
        sll     a3,2                    # *(sizeof caddr)
        addu    a3,a3,a1
        lw      a3,0(a3)
#else
        lw      a3,fpowner_array        # this thread now owns the fp
#endif
	beq	a3,zero,strayfp_intr	# coproc 1 not currently owned

#ifdef	notdef
	/* This is Ultrix stuff, which I don't believe I'll need.
	 * In case some user program depends on it, I'll put it in.
	 */
	/*
	 * If the p_fp is P_FP_SIGINTR1 then a SIGFPE is generated on every
	 * floating-point interrupt before each instruction and is then set to
	 * P_FP_SIGINTR2.  If p_fp is P_FP_SIGINTR2 then no SIGFPE is generated
	 * but p_fp is set to P_FP_SIGINTR1.
	 */
	lw	a1,P_FP(a3)
	beq	a1,zero,2f
	bne	a1,P_FP_SIGINTR1,1f
	# We are in state 1, so change to state2 and generate a SIGFPE
	li	a1,P_FP_SIGINTR2
	sw	a1,P_FP(a3)
	b	12f
1:	# We are in state 2, so change to state1 and don't generate a SIGFPE
 	li	a1,P_FP_SIGINTR1
	sw	a1,P_FP(a3)
2:
#endif	/* notdef */

	.set	noreorder
	nop
	mfc0	a1,C0_SR	# enable coproc 1 for the kernel
	nop
	nop
	or	a1,SR_CU1		
	nop
	mtc0	a1,C0_SR
	nop
	nop
	.set	reorder

	lw	a3,EF_EPC*4(a0)		# load the epc into a3

	# Check the fp implementation so to know were to get the instruction
	# and then load it into register a1 where softfp() expects it.
	lw	a2,fptype_word
	bne	a2,IRR_IMP_R2360,2f

	# For board implementations the instruction is in the fpc_eir
	# floating-point control register.
	sw	a3,V0_OFFSET(sp)	# save the resulting pc (board case)
	cfc1	a1,fpc_eir
	b	4f
2:
	# For chip implementations the floating-point instruction that caused
	# the interrupt is at the address of the epc as modified by the branch
	# delay bit in the cause register.

	lw	a1,0(a3)		# load the instr at the epc into a1
	lw	v0,EF_CAUSE*4(a0)	# load the cause register into v0
	bltz	v0,3f			# check the branch delay bit
	
	# This is not in a branch delay slot (branch delay bit not set) so
	# calculate the resulting pc (epc+4) into v0 and continue to softfp().
	addu	v0,a3,4
	sw	v0,V0_OFFSET(sp)	# save the resulting pc
	b	4f
3:
	# This is in a branch delay slot so the branch will have to be emulated
	# to get the resulting pc (done by calling emulate_branch() ).
	# The arguments to emulate_branch are:
	#	a0 -- ef (exception frame)
	#	a1 -- the branch instruction
	#	a2 -- the floating-point control and status register
	#
	cfc1	a2,fpc_csr		# get value of fpc_csr
	jal	emulate_branch		# emulate the branch
	sw	v0,V0_OFFSET(sp)	# save the resulting pc
	lw	a0,A0_OFFSET(sp)	# restore exception frame pointer
	lw	a2,fptype_word		# restore a2 with fptype_word value
	lw	a3,EF_EPC*4(a0)		# load the epc into a3

	# Now load the floating-point instruction in the branch delay slot
	# to be emulated by softfp().
	lw	a1,4(a3)
4:
	/*
	 * Check to see if the instruction to be emulated is a floating-point
	 * instruction.  If it is not then this interrupt must have been caused
	 * by writing to the fpc_csr a value which caused the interrupt.
	 * It is possible however that when writing to the fpc_csr the
	 * instruction that is to be "emulated" when the interrupt is handled
	 * looks like a floating-point instruction and will incorrectly be
	 * emulated and a SIGFPE will not be sent.  This is the user's problem
	 * because he shouldn't write a value into the fpc_csr which should
	 * cause an interrupt.
	 */
	srl	a3,a1,OPCODE_SHIFT
	beq	a3,OPCODE_C1,10f

	/*
	 * Not a floating point instruction.
	 * Setup and call psignal() to send a SIGFPE to the current thread.
	 * If the owner of the floating-point unit is the current thread then
	 * give it an AST to force entry into trap so the signal
	 * will be posted before returning to user mode.
	 */
12:
#if     NCPUS > 1
        li      a0,PCB_WIRED_ADDRESS
        lw      a0,PCB_CPU_NUMBER(a0)
        la      a1,active_threads
        sll     a0,2                    # *(sizeof caddr)
        addu    a1,a1,a0
        lw      a1,0(a1)                # >>active_threads<<
	la	a3,fpowner_array
	addu	a3,a3,a0
	lw	a0,0(a3)
#else
        lw      a1,active_threads       # only 1 cpu....
	lw      a0,fpowner_array
#endif



	bne	a0,a1,11f		# not current thread
	li	a1,PCB_WIRED_ADDRESS
	sw	sp,need_ast		# force resched
11:
	lw	a0,UTASK(a0)
	lw	a0,U_PROCP(a0)
	/*
	 * if this is a pure thread, no signals 
	 * XXX needs to generate an exception for them too.
	 */
	beq	zero,a0,nosig
	li	a1,SIGFPE
	jal	psignal
nosig:
	# We must clear the coprocessor interrupt without losing fp
	# state. We do this by calling checkfp which will unload
	# the fp to the pcb and clear the fp csr.  A signal is
	# pending, so sendsig will clear the csr in the pcb after
	# saving the fp state from the pcb into the sigcontext and
	# before calling the signal handler
#if     NCPUS > 1
        li      a0,PCB_WIRED_ADDRESS
        lw      a0,PCB_CPU_NUMBER(a0)
        la      a1,fpowner_array
        sll     a0,2                    # *(sizeof caddr)
        addu    a0,a0,a1
        lw      a0,0(a0)
#else
        lw      a0,fpowner_array        # this thread now owns the fp
#endif
	move	a1,zero
	jal	checkfp

	b	8f

10:
	# For now all instructions that cause an interrupt are just handed
	# off to softfp() to emulate it and come up with correct result.
	# The arguments to softfp() are:
	#	a0 -- ef (exception frame)
	#	a1 -- floating-point instruction
	#	a2 -- fptype_word
	#
	# What might be done for all exceptions for which the trapped
	# result is the same as the untrapped result is: turn off the enables,
	# re-excute the instruction, restore the enables and then post a SIGFPE.

	jal	softfp
	beq	v0,zero,5f	# no signal posted

	# We must clear the coprocessor interrupt without losing fp
	# state, we do this by calling checkfp which will unload
	# the fp to the pcb and clear the fp csr.  A signal is
	# pending, sendsig will clear the csr in the pcb after
	# saving the fp state from the pcb into the sigcontext and
	# before calling the signal handler
#if     NCPUS > 1
        li      a0,PCB_WIRED_ADDRESS
        lw      a0,PCB_CPU_NUMBER(a0)
        la      a1,fpowner_array
        sll     a0,2                    # *(sizeof caddr)
        addu    a0,a0,a1
        lw      a0,0(a0)
#else
        lw      a0,fpowner_array        # this thread now owns the fp
#endif
	# Softfp sometimes initiates it's own checkfp
	beq	a0,zero,8f
	move	a1,zero
	jal	checkfp
	b	8f

5:
#ifdef ASSERTIONS
	/*
	 * If going back to user code without posting a signal there must
	 * not be any exceptions which could cause an interrupt.
	 */
	cfc1	a0,fpc_csr
	and	a1,a0,CSR_EXCEPT	# isolate the exception bits
	and	a0,CSR_ENABLE 		# isolate the enable bits 
	or	a0,(UNIMP_EXC >> 5)	# fake an enable for unimplemented
	sll	a0,5			# align both bit sets
	and	a0,a1			# check for coresponding bits
	beq	a0,zero,7f		# if not then ok
	PANIC("fp_intr csr exceptions")
7:
#endif /* ASSERTIONS */

	# Enable fp and hardclock interrupts to allow time to be charged
	# to kernel instead of user. This is done since fp_intr is
	# higher priority than hardclock. Ok because kernel never does
	# floating point.

	# *DGD*  IN ULTRIX: (removed by gmm)
	# Removed the code to lower the priority to enable clock interrupt.
	# Other than the unknown side effects of lowering the ipl from an
	# interrupt routine, hardclock gets timeout lock which could give
	# trouble if run queue lock is already held from swtch(). run queue
	# lock is got at splclock(). This change means that at the next clock
	# tick, time will be chared to the user.
	# *DGD*  KEPT OSF IPL DROP

	.set	noreorder
#ifdef	MSERIES
	li	v1,SR_IEC|SR_IMASK4
#endif
#ifdef	mips	
	li	v1,SR_IEC|SR_IMASK5
#endif
	mtc0	v1,C0_SR
	nop
	.set	reorder

	# The instruction was emulated by softfp() without a signal being
	# posted so now change the epc to the target pc.  This is a nop if
	# this is the board because we stored the epc in V0_OFFSET when we
	# started.
	lw	a0,A0_OFFSET(sp)	# restore the exception frame pointer
	lw	v0,V0_OFFSET(sp)	# get the resulting pc
	sw	v0,EF_EPC*4(a0)		# store the resulting pc in the epc

8:
	lw	ra,RA_OFFSET(sp)
	addu	sp,FRAME_SIZE
	j	ra

/*
 * Nobody was using the floating point coprocessor, but
 * we got an interrupt.
 */
strayfp_intr:
	.set	noreorder
	mfc0	a2,C0_SR	# enable coproc 1 for the kernel
	nop
	or	a3,a2,SR_CU1		
	mtc0	a3,C0_SR
	nop			# before we can really use cp1
	nop			# before we can really use cp1
	ctc1	zero,fpc_csr	# shut it off
	mtc0	a2,C0_SR
	nop
	.set	reorder

#ifdef	notdef
	lw	a0,fpu_inited
	beq	a0,zero,7f		# if not inited then don't complain
#endif
	PRINTF("stray fp interrupt\012")
7:	lw	ra,RA_OFFSET(sp)
	addu	sp,FRAME_SIZE
	j	ra

	.end	fp_intr
