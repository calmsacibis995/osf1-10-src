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
 *	@(#)$RCSfile: context.s,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/03/18 15:01:51 $
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
 *	File: context.s
 *
 *	Context switching and context save/restore primitives
 *
 *	Revision History:
 *
 * 8-Apr-91	Ron Widyono
 *	Do not allow inclusion of sys/preempt.h from machine/pmap.h.
 *
 */
/* #include <fast_csw.h>       /* ALWAYS fast; why would you want slow? */
#include <mach_kdb.h>
#include <pmap_pcache.h>
#include <rt_preempt.h>

#if	RT_PREEMPT
#define _SKIP_PREEMPT_H_
#endif

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <machine/pcb.h>
#include <machine/pmap.h>
#include <machine/thread.h>
#include <assym.s>
#include <machine/machparam.h>
#include <mach/mips/vm_param.h>
#include <cpus.h>

#define DEBUG	0

	BSS(t0_temp,4)			# squirrel away t0 for save_context_all


	.set	noreorder		# unless overridden

/*
 * save_context()
 *
 * 	Save thread's context in the pcb
 *
 * Description
 *	Saves all registers that have not already been saved
 *	on the stack (by C calling conventions) in the pcb.
 *	Saves status register and stack pointer in pcb as well.
 *	Returns 0 at splhigh()
 */

LEAF(save_context)
	mfc0	t1,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	li	t0,PCB_WIRED_ADDRESS		# get the thread's PCB
	/*
	 * 	We only need to save those registers that are callee-saved
	 *	in C, everything else is already on the stack.
	 */
	sw	sp,PCB_SP*4(t0)
	sw	ra,PCB_PC*4(t0)
	sw	t1,PCB_SR*4(t0)

	sw	fp,PCB_FP*4(t0)
	sw	s0,PCB_S0*4(t0)
	sw	s1,PCB_S1*4(t0)
	sw	s2,PCB_S2*4(t0)
	sw	s3,PCB_S3*4(t0)
	sw	s4,PCB_S4*4(t0)
	sw	s5,PCB_S5*4(t0)
	sw	s6,PCB_S6*4(t0)
	sw	s7,PCB_S7*4(t0)
	j	ra
	move	v0,zero			# return 0
	END(save_context)

/*
 * save_context_all()
 *
 * 	Save all registers in the pcb, for debugging
 *
 * Description
 *	Saves all registers in the pcb. Called during panic/reboot so that
 *	all register contents can be easily located when debugging.
 *	Saves status register and stack pointer in pcb as well.
 *	Returns 0 at splhigh()
 */
LEAF(save_context_all)
	.set	noat
	mfc0	t1,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	sw	t0,t0_temp
	li	t0,PCB_WIRED_ADDRESS		# get the thread's PCB
	sw	$at,PCB_AT*4(t0)
	sw	v0,PCB_V0*4(t0)
	sw	v1,PCB_V1*4(t0)
	sw	a0,PCB_A0*4(t0)
	sw	a1,PCB_A1*4(t0)
	sw	a2,PCB_A2*4(t0)
	sw	a3,PCB_A3*4(t0)
	sw	t1,PCB_T1*4(t0)
	sw	t2,PCB_T2*4(t0)
	sw	t3,PCB_T3*4(t0)
	sw	t4,PCB_T4*4(t0)
	sw	t5,PCB_T5*4(t0)
	sw	t6,PCB_T6*4(t0)
	sw	t7,PCB_T7*4(t0)
	sw	sp,PCB_SP*4(t0)
	sw	ra,PCB_PC*4(t0)
	sw	fp,PCB_FP*4(t0)
	sw	s0,PCB_S0*4(t0)
	sw	s1,PCB_S1*4(t0)
	sw	s2,PCB_S2*4(t0)
	sw	s3,PCB_S3*4(t0)
	sw	s4,PCB_S4*4(t0)
	sw	s5,PCB_S5*4(t0)
	sw	s6,PCB_S6*4(t0)
	sw	s7,PCB_S7*4(t0)
	sw	t8,PCB_T8*4(t0)
	sw	t9,PCB_T9*4(t0)
	sw	k0,PCB_K0*4(t0)
	sw	k1,PCB_K1*4(t0)
	sw	gp,PCB_GP*4(t0)

/*	oh, and get the original value of t0 and save it as well */

	lw	t1,t0_temp
	nop
	sw	t1,PCB_T0*4(t0)

/*	save some special registers */

	mflo	t1
	sw	t1,PCB_LO*4(t0)
	mfhi	t1
	sw	t1,PCB_HI*4(t0)

/*	maybe we should save the floating point registers as well, but not
 *	today. --RSC
 */

	.set	at
	j	ra
	move	v0,zero			# return 0
	END(save_context_all)

/*
 * load_context(thread)
 *
 * Purpose
 * 	Restore state of the given thread, from its pcb
 *
 * Description
 *	Wires the mapping(s) for the thread's kernel stack.
 *	Installs this as the current thread on all the various
 *	globals: active_threads .
 *	Enables use of the FPU (when back to user) if nobody
 *	has changed its content, but only if we ever used it.
 *	Restores the state that save_context() has saved in the PCB
 *	and returns control to the thread [which will then return
 *	from save_context()]
 * 	Returns 1
 */
LEAF(load_context)
	mtc0	zero,C0_SR		# disable interrupts
	mfc0	s7,C0_TLBHI		# save current pid
	/*
	 * There are various possible accidents we must prevent here.
	 * Basically we might screw up by taking a tlbmiss when we
	 * are not ready, or by not setting things up right so that
	 * going into user mode and back fails.
	 *
	 * The first is only really a problem when inside this function.
	 * It is avoided by collectiong all the informations we
	 * need in registers when still in a state that allows
	 * tlb misses, and then entering a small critical section where
	 * misses are NOT allowed. CAVEAT MAINTAINER!
	 *
	 * The second requires that the thread's kernel stack be
	 * mapped at all times, as we need to restore the user's
	 * registers on it before we clobber them.
	 * Also, the code that exits to user mode checks for ASTs
	 * in the PCB while it cannot tlbmiss, so that information
	 * is kept in a global variable.
	 * A further optimization is to keep the kernel stack pointer
	 * of the current thread in a global variable so that we can
	 * get it quickly when we switch back to kernel mode.
	 */

	/*
	 *	Part 1: kernel stack
	 */
	lw	s0,THREAD_KERNEL_STACK(a0)	# get ksp0, might miss
	li	a1,KPTEADDR		# kernel_pmap->pte_base
	move	s1,s0			# preserve ksp0
	srl	s0,MIPS_PGSHIFT		# pte=pmap_pte(addr)
	sll	s0,2
	addu	s0,a1
	lw	s0,0(s0)		# get pte, might miss
	nop

					# page must be writable!!
	and	s4,s0,PG_M
	bne	s4,zero,1f
	or	s0,s0,PG_M		# bdslot
					# note page "will be" dirty
	move	s4,a0	
	jal	pmap_set_modify
	move	a0,s0
	li	a1,KPTEADDR		# kernel_pmap->pte_base
	move	a0,s4
1:

	/*
	 *	Do it again for second page
	 */
	li	s5,MIPS_PGBYTES
	addu	s5,s1,s5
	srl	s5,MIPS_PGSHIFT		# pte=pmap_pte(addr)
	sll	s5,2
	addu	s5,a1
	lw	s5,0(s5)		# get pte, might miss
	nop
	and	s4,s5,PG_M
	bne	s4,zero,1f
	or	s5,s5,PG_M		# bdslot
					# note page "will be" dirty
	move	s4,a0	
	jal	pmap_set_modify
	move	a0,s5
	move	a0,s4
1:

	/*
	 *	Part 3: Critical section
	 *
	 *	Setup the wired entries in the TLB and
	 *	other important global variables.
	 *	Relevant globals are bracketed in comments
	 *
	 *	>>>	TLB MISSES NOT ALLOWED HERE	<<<
	 */
	li	a2,VM_MIN_KERNEL_ADDRESS-(2*NBPG)
	li	a3,TLBWIRED_KSTACK<<TLBINX_INXSHIFT
	mtc0	a3,C0_INX
	mtc0	s0,C0_TLBLO
	mtc0	a2,C0_TLBHI

	li	s2,PCB_WIRED_ADDRESS
	c0	C0_WRITEI

	li      a2,VM_MIN_KERNEL_ADDRESS-(1*NBPG)
	li	a3,TLBWIRED_KSTACK1<<TLBINX_INXSHIFT

	mtc0	a3,C0_INX
	mtc0	s5,C0_TLBLO
	mtc0	a2,C0_TLBHI
	nop
	c0	C0_WRITEI
	nop
	mtc0	s7,C0_TLBHI		# restore tlbpid
	lw	sp,PCB_SP*4(s2)		# get new sp value
#if	NCPUS > 1
	lw      s6,PCB_CPU_NUMBER(s2)   #NOTE....load current cpu no
	la	a1,active_threads
	sll	s6,2			# *(sizeof caddr)
	add	a1,a1,s6
	sw	a0,0(a1)		# >>active_threads<<
#else
        sw      a0,active_threads       # only 1 cpu....
#endif	/* NCPUS */



	/*
	 *	>> END OF CRITICAL SECTION <<
	 *
	 *	We can take misses again now, but we're not done yet.
	 */

	/*
	 *	Part 4: Coprocessor(s) usability.
	 *
	 *	Only enable if nobody stole it in the meantime.
	 *	The write to EF_SR is to the execption from create when
	 * 	going from user to kernel mode.
	 */
	.set	reorder
	li      s1,PCB_WIRED_ADDRESS-EF_SIZE
#if NCPUS > 1
	la	a1,fpowner_array
	addu	a1,a1,s6
	lw	a2,0(a1)
#else
	lw	a2,fpowner_array	# fpowner == me ?
#endif	/* NCPUS */
	lw	s0,EF_SR*4(s1)		# from user's exception frame
	bne	a0,a2,1f
	or	s0,SR_CU1		# yep, enable
	b	2f
1:	and	s0,~SR_CU1
2:	sw	s0,EF_SR*4(s1)

	.set	noreorder

#if	PMAP_PCACHE
#ifdef  PMAP_PCACHE_DEBUG
	jal	pmap_pcache_debug		
#endif
	/*
	 *	Part 4.bis: Reload wired ptes from cache
	 */
	lw	s0,active_pmap
	li	s1,2
	li	s4,7
	addu	s0,PMAP_PCACHE_DATA
loop:	lw	a2,0(s0)		/* address */
	addu	s0,8
	beq	a2,zero,test
	move	a0,s1
	lw	a3,-4(s0)		/* pte */
	lw	a1,active_pmap
	jal	tlb_map
	lw	a1,PMAP_PID(a1)		/* pid */
test:	bne	s1,s4,loop
	addiu	s1,1
#endif	/* PMAP_PCACHE */

	/*
	 *	Part 5: Reload thread's registers and return
	 */
	lw	ra,PCB_PC*4(s2)
	lw	fp,PCB_FP*4(s2)
	lw	s0,PCB_S0*4(s2)
	lw	s1,PCB_S1*4(s2)
	lw	s3,PCB_S3*4(s2)
	lw	s4,PCB_S4*4(s2)
	lw	s5,PCB_S5*4(s2)
	lw	s6,PCB_S6*4(s2)
	lw	v0,PCB_SR*4(s2)
	lw	s7,PCB_S7*4(s2)
	mtc0	v0,C0_SR
	lw	s2,PCB_S2*4(s2)
	j	ra
	li	v0,1			# return non-zero
	END(load_context)

	.set	reorder

#if	MACH_KDB
/*
 * kdbsetjmp(jmp_buf)
 *
 * Purpose
 *	Save current context for non-local goto's
 *
 * Description
 *	Saves all registers that are callee-saved in the
 *	given longjmp buffer.  Same as user level _setjmp
 * 	Return 0
 */
LEAF(kdbsetjmp)
	sw	ra,JB_PC*4(a0)
	sw	sp,JB_SP*4(a0)
	sw	fp,JB_FP*4(a0)
	sw	s0,JB_S0*4(a0)
	sw	s1,JB_S1*4(a0)
	sw	s2,JB_S2*4(a0)
	sw	s3,JB_S3*4(a0)
	sw	s4,JB_S4*4(a0)
	sw	s5,JB_S5*4(a0)
	sw	s6,JB_S6*4(a0)
	.set noreorder
	mfc0	v0,C0_SR
	sw	s7,JB_S7*4(a0)
	.set reorder
	sw	v0,JB_SR*4(a0)
	move	v0,zero
	j	ra
	END(kdbsetjmp)


/*
 * kdblongjmp(jmp_buf)
 *
 * Purpose
 *	Perform a non-local goto
 *
 * Description
 *	Restores all registers that are callee-saved from the
 *	given longjmp buffer.  Same as user level _longjmp
 * 	Return a non-zero value.
 */
LEAF(kdblongjmp)
	lw	ra,JB_PC*4(a0)
	lw	sp,JB_SP*4(a0)
	lw	fp,JB_FP*4(a0)
	lw	s0,JB_S0*4(a0)
	lw	s1,JB_S1*4(a0)
	lw	s2,JB_S2*4(a0)
	lw	s3,JB_S3*4(a0)
	lw	s4,JB_S4*4(a0)
	lw	s5,JB_S5*4(a0)
	lw	s6,JB_S6*4(a0)
	lw	v0,JB_SR*4(a0)
	.set noreorder
	lw	s7,JB_S7*4(a0)
	mtc0	v0,C0_SR
	.set reorder
	li	v0,1		/* return non-zero */
	j	ra
	END(kdblongjmp)
#endif	/* MACH_KDB */

/*
 * OSF/1.0 had this here.
 * Kept for historical;
 * Context switch doesn't work without this code, given
 * the new kernel stack changes.
 *
 */
/*
#if	FAST_CSW
 */
	.set noreorder
/*
 * switch_task_context(old_thread,new_thread)
 *
 * Purpose
 *	Switch context among two threads in different tasks
 *
 * Description
 *	Bump switch task count and disable interrupts, then merge into the 
 *	flow for switch_thread_context.
*/
LEAF(switch_task_context)
	la	t2,swtch_tsk_ctxt_cnt
	lw	t3,0(t2)
	mfc0	t1,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	addiu	t3,1
	j	swtch_task_merge
	sw	t3,0(t2)
	/* NOTREACHED */
	END(switch_task_context)
/*
 * switch_thread_context(old_thread,new_thread)
 *
 * Purpose
 *	Switch context among two threads in the same task.
 *
 * Description
 *	This routine is functionally equivalent to:
 *	save_context(th1)
 *	load_context(th2)
 *	thread_continue(th1) 
 *
 * 	The ordering is important to SMP system because we must fully 
 *	get rid of the last thread and into the context of the next,
 *	BEFORE releasing it with a "thread_continue".
 */
LEAF(switch_thread_context)
	lw	t2,swtch_thrd_ctxt_cnt
	mfc0	t1,C0_SR		# save SR and disable interrupts
	mtc0	zero,C0_SR
	addiu	t2,1
	sw 	t2,swtch_thrd_ctxt_cnt	
	
swtch_task_merge:
	li	t0,PCB_WIRED_ADDRESS		# get the thread's PCB
	/*
	 * 	We only need to save those registers that are callee-saved
	 *	in C, everything else is already on the stack.
	 */
	sw	sp,PCB_SP*4(t0)
	sw	ra,PCB_PC*4(t0)
	sw	t1,PCB_SR*4(t0)

	sw	fp,PCB_FP*4(t0)
	sw	s0,PCB_S0*4(t0)
	sw	s1,PCB_S1*4(t0)
	sw	s2,PCB_S2*4(t0)
	sw	s3,PCB_S3*4(t0)
	sw	s4,PCB_S4*4(t0)
	sw	s5,PCB_S5*4(t0)
	sw	s6,PCB_S6*4(t0)
	sw	s7,PCB_S7*4(t0)


	move	s3,a0
	move	a0,a1

	mfc0	s7,C0_TLBHI		# save current pid
#if NCPUS > 1
	lw      s6,PCB_CPU_NUMBER(t0)   # load current cpu no
#endif
	/*
	 * There are various possible accidents we must prevent here.
	 * Basically we might screw up by taking a tlbmiss when we
	 * are not ready, or by not setting things up right so that
	 * going into user mode and back fails.
	 *
	 * The first is only really a problem when inside this function.
	 * It is avoided by collectiong all the informations we
	 * need in registers when still in a state that allows
	 * tlb misses, and then entering a small critical section where
	 * misses are NOT allowed. CAVEAT MAINTAINER!
	 *
	 * The second requires that the thread's kernel stack be
	 * mapped at all times, as we need to restore the user's
	 * registers on it before we clobber them.
	 * Also, the code that exits to user mode checks for ASTs
	 * in the PCB while it cannot tlbmiss, so that information
	 * is kept in a global variable.
	 * A further optimization is to keep the kernel stack pointer
	 * of the current thread in a global variable so that we can
	 * get it quickly when we switch back to kernel mode.
	 */

	/*
	 *	Part 1: kernel stack
	 */
	lw	s0,THREAD_KERNEL_STACK(a0)	# get ksp0, might miss
	li	a1,KPTEADDR		# kernel_pmap->pte_base
	move	s1,s0			# preserve ksp0
	srl	s0,MIPS_PGSHIFT		# pte=pmap_pte(addr)
	sll	s0,2
	addu	s0,a1
	lw	s0,0(s0)		# get pte, might miss
	nop

					# page must be writable!!
	and	s4,s0,PG_M
	bne	s4,zero,1f
	or	s0,s0,PG_M		# bdslot
					# note page "will be" dirty
	move	s4,a0	
	jal	pmap_set_modify
	move	a0,s0
	li	a1,KPTEADDR		# kernel_pmap->pte_base
	move	a0,s4
1:

	/*
	 *	Do it again for second page
	 */
	li	s5,MIPS_PGBYTES
	addu	s5,s1,s5
	srl	s5,MIPS_PGSHIFT		# pte=pmap_pte(addr)
	sll	s5,2
	addu	s5,a1
	lw	s5,0(s5)		# get pte, might miss
	nop
	and	s4,s5,PG_M
	bne	s4,zero,1f
	or	s5,s5,PG_M		# bdslot
					# note page "will be" dirty
	move	s4,a0	
	jal	pmap_set_modify
	move	a0,s5
	move	a0,s4
1:

	/*
	 *	Part 3: Critical section
	 *
	 *	Setup the wired entries in the TLB and
	 *	other important global variables.
	 *	Relevant globals are bracketed in comments
	 *
	 *	>>>	TLB MISSES NOT ALLOWED HERE	<<<
	 */	
	li      a2,VM_MIN_KERNEL_ADDRESS-(2*NBPG)
	li	a3,TLBWIRED_KSTACK<<TLBINX_INXSHIFT
	mtc0	a3,C0_INX
	mtc0	s0,C0_TLBLO
	mtc0	a2,C0_TLBHI
	li      s2,PCB_WIRED_ADDRESS
	c0	C0_WRITEI

	li      a2,VM_MIN_KERNEL_ADDRESS-(1*NBPG)
	li	a3,TLBWIRED_KSTACK1<<TLBINX_INXSHIFT

	mtc0	a3,C0_INX
	mtc0	s5,C0_TLBLO
	mtc0	a2,C0_TLBHI
	nop
	c0	C0_WRITEI
	nop
	mtc0	s7,C0_TLBHI		# restore tlbpid
	lw	sp,PCB_SP*4(s2)		# get new sp value
#if NCPUS > 1
	sw      s6,PCB_CPU_NUMBER(s2)   # store current cpu no
        la      a1,active_threads
        sll     s6,2                    # *(sizeof caddr)
        add     a1,a1,s6
        sw      a0,0(a1)                # >>active_threads<<
#else
	sw      a0,active_threads	# only 1 cpu....
#endif	/* NCPUS */

	/*
	 *	>> END OF CRITICAL SECTION <<
	 *
	 *	We can take misses again now, but we're not done yet.
	 */

	/*
	 *	Part 4: Coprocessor(s) usability.
	 *
	 *	Only enable if nobody stole it in the meantime.
	 *	The write to EF_SR is to the execption from create when
	 * 	going from user to kernel mode.
	 */
	.set	reorder
	li      s1,PCB_WIRED_ADDRESS-EF_SIZE
#if NCPUS > 1
        la      a1,fpowner_array
        addu    a1,a1,s6
        lw      a2,0(a1)
#else
        lw      a2,fpowner_array        # fpowner == me ?
#endif	/* NCPUS */
	lw	s0,EF_SR*4(s1)		# from user's exception frame
	bne	a0,a2,1f
	or	s0,SR_CU1		# yep, enable
	b	2f
1:	and	s0,~SR_CU1
2:	sw	s0,EF_SR*4(s1)

	.set	noreorder

#if	PMAP_PCACHE
#ifdef  PMAP_PCACHE_DEBUG
	jal	pmap_pcache_debug		
#endif
	/*
	 *	Part 4.bis: Reload wired ptes from cache
	 */
	lw	s0,active_pmap
	li	s1,2
	li	s4,7
	addu	s0,PMAP_PCACHE_DATA
xxloop:	lw	a2,0(s0)		/* address */
	addu	s0,8
	beq	a2,zero,xxtest
	move	a0,s1
	lw	a3,-4(s0)		/* pte */
	lw	a1,active_pmap
	jal	tlb_map
	lw	a1,PMAP_PID(a1)		/* pid */
xxtest:	bne	s1,s4,xxloop
	addiu	s1,1
#endif	/* PMAP_PCACHE */

	/*
	 *	Part 5: Reload thread's registers and return
	 */
	jal	thread_continue
	move 	a0,s3			#BD-slot ... move old thread to
					# argument 1 register.
	lw	ra,PCB_PC*4(s2)
	lw	fp,PCB_FP*4(s2)
	lw	s0,PCB_S0*4(s2)
	lw	s1,PCB_S1*4(s2)
	lw	s3,PCB_S3*4(s2)
	lw	s4,PCB_S4*4(s2)
	lw	s5,PCB_S5*4(s2)
	lw	s6,PCB_S6*4(s2)
	lw	v0,PCB_SR*4(s2)
	lw	s7,PCB_S7*4(s2)
	mtc0	v0,C0_SR
	lw	s2,PCB_S2*4(s2)
	j	ra
	li	v0,1			# return non-zero
	.set	reorder
	/* NOTREACHED */
	END(switch_thread_context)

/* matching #endif to #if FAST_CSW above, which is out w/new kstack */
/* #endif	/* FAST_CSW */
