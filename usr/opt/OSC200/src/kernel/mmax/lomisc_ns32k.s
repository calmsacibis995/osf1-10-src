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
 *	@(#)$RCSfile: lomisc_ns32k.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:28 $
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
 * Copyright (c) 1989 Encore Computer Corporation
 */
#include "assym.s"

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax_idebug.h>
#include <fast_csw.h>
#include <cpus.h>

#include <mmax/psl.h>
#include <mmax/cpudefs.h>
#include <mmax/mlmacros.h>


#ifdef	GPROF
	/*
	 *	mcount is a trampoline routine that saves the arguments
	 *	to the current procedure, and calls _mcount [subr_mcount.c]
	 *	with the pc it was called from and the pc above it.
	 */

	.globl	mcount
	.globl	_mmax_mcount
mcount:
	save	[r0,r1]			/* Save parameters in scratch regs. */
	movd	8(sp),r0		/* pc of caller: 8 skips [r0,r1] */
	movd	4(fp),r1		/* pc of caller's caller */
	jsr	_mmax_mcount		/* Go do the dirty work */
	restore	[r0,r1]			/* Get parameters back */
	ret	$0			/* And return to caller */
#endif	GPROF

/*
 # addupc (pc, u_prof, ticks)
 #	int	*pc#		User PC on the stack.
 #	uprof	*u_prof#	address of the user profile area.
 #	int	ticks#		value to add to u_prof[pc-pc_off]

 #	This takes the PC at the time of a trap (or syscall), the user
 #	profile data area, and the clock ticks since the last call.
 #	The an offset (u_prof->pr_offset) is subtracted from the user PC,
 #	the PC is then scaled by u_prof->pr_scale.  Finally the the adjusted
 #	scaled PC is used as an index into the u_prof->pr_base buffer.

 #	See UNIX Manual profil(2) for a more concise decription.
 */

	.globl	_fuword
	.globl	_suword
ENTRY(addupc)
				# User PC in r0
				# Address of the profile region in r1

	movd	r3,tos		# save r3
	movd	r1,r3		# Save pointer for later use
	cmpd	r0,PR_OFFSET(r3)  #If the offset is greater than the PC...
	bge	add1
	movd	tos,r3
	ret	$0		#...return
add1:
	subd	PR_OFFSET(r3),r0	#APC = PC - u_prof->pr_offset;
	lshd	$-1,r0		#Logical right shift APC (clear sign-bit)
	movd	PR_SCALE(r3),r2	#Get the scale
	lshd	$-1,r2		#Logical right shift scale (clear sign-bit)
	meid	r2,r0		#APC = scale * APC
	lshd	$-14,r0		#Supposed to be  lshq -14,r0
	movd	r1,r2		#  copy off the high double of the quad...
	lshd	$-14,r1		#  shift the high double...
	cmpqd	$0,r1		#  if any-thing left in r1 - return
	beq	add2
	movd	tos,r3
	ret	$0
add2:
	lshd	$18,r2		#  now shift r2 the other way...
	ord	r2,r0		#  finally done with the lshq -14,r0
	bicd	$1,r0		#Use word addresses
	cmpd	r0,PR_SIZE(r3)	#Check against length
	bls	add3		#If greater than...
	movd	tos,r3
	ret	$0		#	...return

add3:	addd	PR_BASE(r3),r0	#Add in the base...
	adjspb	$4		# stack cell for movsu/movus
	GETCPUID(r2)
	movd	_active_threads[r2:d],r2
	addr	addupc_fail,THREAD_RECOVER(r2)	# set up recover entry
	movusw	0(r0),0(sp)	# get current count (on stack).
	movzwd	12(sp),r1	# get ticks [12 = cell + saved r3 + return pc]
	indexb	r1,$9,0(sp)	# add 10*ticks to count
	movd	r1,0(sp)	# put count back on stack
	movsuw	0(sp),0(r0)	# and write new count to user
	movqd	$0,THREAD_RECOVER(r2)	# clear recover entry
	adjspb	$-4		# remove extra stack cell.
	movd	tos,r3		# restore r3
	ret	$0

addupc_fail:
	movqd	$0,PR_SCALE(r3)	# turn off profiling (clear pr_scale)
	movqd	$0,THREAD_RECOVER(r2)	# clear recover entry
	adjspb	$-4		# remove extra stack cell
	movd	tos,r3		# restore r3
	ret	$0

#if	FAST_CSW
/*
 *	Fast context switch code changes:
 *
 * Don't save/restore user sp -- it's in trap frame.
 * Don't save/restore mod -- it's a constant in OS, and hw will handle.
 * Don't save/restore psw on APC.  Interrupt state isn't there any more.
 * Got rid of extra GETCPUID in save_context().
 * Leave pc's on stack.  Code before start.thread and in pcb.c modified.
 * Ignore interrupt state.  Caller must disable interrupts and is
 *	responsible for restoring interrupt state.  Kernel thread
 *	trampoline used to get kernel threads right.
 * Must maintain _active_threads array here instead of sched_prim.c.
 */

/*
 * load_context(thread) - load context for the specified context
 * thread	*thread;	- in r0
 */
	.globl	_active_uareas
	.globl	_active_threads

	.globl	_load_context_count
ENTRY(load_context)
	addqd	$1, @_load_context_count
	/*
	 *	Save uarea pointers in a global data structure, and set
	 *	active_threads.
	 */
	GETCPUID(r1)
	movd	r0,_active_threads[r1:d]
	movd	THREAD_UTASK(r0),_active_uareas+UADDR_UTASK[r1:q]
	movd	THREAD_UTHREAD(r0),_active_uareas+UADDR_UTHREAD[r1:q]

	movd	THREAD_PCB(r0),r0
	lprd	sp,PCB_SSP(r0)
	lprd	fp,PCB_FP(r0)
	movd	PCB_R7(r0),r7
	movd	PCB_R6(r0),r6
	movd	PCB_R5(r0),r5
	movd	PCB_R4(r0),r4
	movd	PCB_R3(r0),r3
	lmr	ptb1,PCB_PTBR(r0)
	movl	PCB_F0(r0),f0
	movl	PCB_F2(r0),f2
	movl	PCB_F4(r0),f4
	movl	PCB_F6(r0),f6
#if	MMAX_XPC
	movl	PCB_F1(r0),f1
	movl	PCB_F3(r0),f3
	movl	PCB_F5(r0),f5
	movl	PCB_F7(r0),f7
#endif	MMAX_XPC
	lfsr	PCB_FSR(r0)
	movqd	$1,r0		# indicate load context return
	ret	$0
/*
 * save_context saves the context of the current thread into that thread's
 * pcb and switches to a per-processor stack.  The calling routine
 * must have ALL of its local variables in registers.
 */

	.globl	_save_context_count
ENTRY(save_context)
	addqd	$1, @_save_context_count
	GETCPUID(r2)
	movd	_active_threads[r2:d],r0	/* This CPU's thread */
	movd	THREAD_PCB(r0),r0	/* The PCB ptr */

	movd	0(sp),r1
	sprd	sp,PCB_SSP(r0)
	sprd	fp,PCB_FP(r0)
	movd	r7,PCB_R7(r0)
	movd	r6,PCB_R6(r0)
	movd	r5,PCB_R5(r0)
	movd	r4,PCB_R4(r0)
	movd	r3,PCB_R3(r0)
	sfsr	PCB_FSR(r0)
	movl	f0,PCB_F0(r0)
	movl	f2,PCB_F2(r0)
	movl	f4,PCB_F4(r0)
	movl	f6,PCB_F6(r0)
#if	MMAX_XPC
	movl	f1,PCB_F1(r0)
	movl	f3,PCB_F3(r0)
	movl	f5,PCB_F5(r0)
	movl	f7,PCB_F7(r0)
#endif	MMAX_XPC

	/* switch to interrupt stack */
	movd	_interrupt_stack[r2:d],r0	# get its stack base
	addd	$INTSTACK_SIZE,r0	# go to end of stack
	lprd	sp,r0			# load stack
	lprd	fp,r0			#   registers

	movqd	$0,r0			# indicate save_context return
	jump	r1			# return to the caller

/*
 *	switch_task_context context switches between threads in two different
 *	tasks.  Caller has disabled interrupts.
 *
 *	register usage
 *	r0 - old thread
 *	r1 - new thread
 *	r2 - pcb/cpu number
 *
 *	Changes from separate save/load context
 * Don't have to switch to interrupt stack.
 */
	.globl	_thread_continue
	.globl	_switch_task_context
	.globl	_swtch_tsk_ctxt_cnt
_switch_task_context:
	addqd	$1, @_swtch_tsk_ctxt_cnt
	movd	THREAD_PCB(r0),r2	/* The PCB ptr */
	sprd	sp,PCB_SSP(r2)
	sprd	fp,PCB_FP(r2)
	movd	r7,PCB_R7(r2)
	movd	r6,PCB_R6(r2)
	movd	r5,PCB_R5(r2)
	movd	r4,PCB_R4(r2)
	movd	r3,PCB_R3(r2)
	sfsr	PCB_FSR(r2)
	movl	f0,PCB_F0(r2)
	movl	f2,PCB_F2(r2)
	movl	f4,PCB_F4(r2)
	movl	f6,PCB_F6(r2)
#if	MMAX_XPC
	movl	f1,PCB_F1(r2)
	movl	f3,PCB_F3(r2)
	movl	f5,PCB_F5(r2)
	movl	f7,PCB_F7(r2)
#endif	MMAX_XPC

	/*
	 *	Save uarea pointers in a global data structure and set
	 *	active_threads.
	 */
	GETCPUID(r2)
	movd	r1,_active_threads[r2:d]
	movd	THREAD_UTASK(r1),_active_uareas+UADDR_UTASK[r2:q]
	movd	THREAD_UTHREAD(r1),_active_uareas+UADDR_UTHREAD[r2:q]

	movd	THREAD_PCB(r1),r2
	lprd	sp,PCB_SSP(r2)
	lprd	fp,PCB_FP(r2)
	movd	PCB_R7(r2),r7
	movd	PCB_R6(r2),r6
	movd	PCB_R5(r2),r5
	movd	PCB_R4(r2),r4
	movd	PCB_R3(r2),r3
	lmr	ptb1,PCB_PTBR(r2)
	movl	PCB_F0(r2),f0
	movl	PCB_F2(r2),f2
	movl	PCB_F4(r2),f4
	movl	PCB_F6(r2),f6
#if	MMAX_XPC
	movl	PCB_F1(r2),f1
	movl	PCB_F3(r2),f3
	movl	PCB_F5(r2),f5
	movl	PCB_F7(r2),f7
#endif	MMAX_XPC
	lfsr	PCB_FSR(r2)

	/*
	 *	thread_continue(old_thread)
	 */
	jsr	_thread_continue
	movqd	$1,r0		# indicate load context return
	ret	$0

/*
 *	switch_thread_context context switches between threads in the same
 *	task.  Caller has disabled interrupts.
 *
 *	register usage
 *	r0 - old thread
 *	r1 - new thread
 *	r2 - cpu number/pcb
 *
 *	Changes from separate save/load context
 * Don't have to switch to interrupt stack.
 * Must set active_threads here.
 *
 *	Changes from switch_task_context
 * Don't need to load UTASK or ptbr.
 */
	.globl	_switch_thread_context
	.globl	_swtch_thrd_ctxt_cnt
_switch_thread_context:
	addqd	$1, @_swtch_thrd_ctxt_cnt
	movd	THREAD_PCB(r0),r2	/* The PCB ptr */
	sprd	sp,PCB_SSP(r2)
	sprd	fp,PCB_FP(r2)
	movd	r7,PCB_R7(r2)
	movd	r6,PCB_R6(r2)
	movd	r5,PCB_R5(r2)
	movd	r4,PCB_R4(r2)
	movd	r3,PCB_R3(r2)
	sfsr	PCB_FSR(r2)
	movl	f0,PCB_F0(r2)
	movl	f2,PCB_F2(r2)
	movl	f4,PCB_F4(r2)
	movl	f6,PCB_F6(r2)
#if	MMAX_XPC
	movl	f1,PCB_F1(r2)
	movl	f3,PCB_F3(r2)
	movl	f5,PCB_F5(r2)
	movl	f7,PCB_F7(r2)
#endif	MMAX_XPC

	/*
	 *	Save uarea pointers in a global data structure and set
	 *	active_threads.
	 */
	GETCPUID(r2)
	movd	r1,_active_threads[r2:d]
	movd	THREAD_UTHREAD(r1),_active_uareas+UADDR_UTHREAD[r2:q]

	movd	THREAD_PCB(r1),r2
	lprd	sp,PCB_SSP(r2)
	lprd	fp,PCB_FP(r2)
	movd	PCB_R7(r2),r7
	movd	PCB_R6(r2),r6
	movd	PCB_R5(r2),r5
	movd	PCB_R4(r2),r4
	movd	PCB_R3(r2),r3
	movl	PCB_F0(r2),f0
	movl	PCB_F2(r2),f2
	movl	PCB_F4(r2),f4
	movl	PCB_F6(r2),f6
#if	MMAX_XPC
	movl	PCB_F1(r2),f1
	movl	PCB_F3(r2),f3
	movl	PCB_F5(r2),f5
	movl	PCB_F7(r2),f7
#endif	MMAX_XPC
	lfsr	PCB_FSR(r2)

	/*
	 *	thread_continue(old_thread)
	 */
	jsr	_thread_continue
	movqd	$1,r0		# indicate load context return
	ret	$0

/*
 *	thread_begin is entry point for new threads.
 */
	.globl _thread_begin
_thread_begin:
	ENBINT			# must do here
	br	?rett_ast
	
/*
 *	kernel_thread_begin is a trampoline for kernel threads to make
 *	sure interrupts get enabled.  thread_start pushes thread start
 *	and kernel_thread_begin onto stack of new thread.
 */
	.globl _kernel_thread_begin
_kernel_thread_begin:
	ENBINT			# turn on interrupts
	ret	$0		# return to real thread.

#else	FAST_CSW

/*
 * load_context(thread) - load context for the specified context
 * thread	*thread;	- in r0
 */
	.globl	_active_uareas
	.globl	_load_context_count

ENTRY(load_context)
	addqd	$1, @_load_context_count
	/*
	 *	Save uarea pointers in a global data structure.
	 */
	GETCPUID(r1)
	movd	THREAD_UTASK(r0),_active_uareas+UADDR_UTASK[r1:q]
	movd	THREAD_UTHREAD(r0),_active_uareas+UADDR_UTHREAD[r1:q]

	movd	THREAD_PCB(r0),r0
#if	MMAX_XPC
	lprd	usp,PCB_USP(r0)
#endif	MMAX_XPC
#if	MMAX_APC || MMAX_DPC
	USRSP
	lprd	sp,PCB_USP(r0)
	SYSSP
#endif	MMAX_APC || MMAX_DPC
	lprd	sp,PCB_SSP(r0)
	lprd	fp,PCB_FP(r0)
	movd	PCB_R7(r0),r7
	movd	PCB_R6(r0),r6
	movd	PCB_R5(r0),r5
	movd	PCB_R4(r0),r4
	movd	PCB_R3(r0),r3
	lmr	ptb1,PCB_PTBR(r0)
	movl	PCB_F0(r0),f0
	movl	PCB_F2(r0),f2
	movl	PCB_F4(r0),f4
	movl	PCB_F6(r0),f6
#if	MMAX_XPC
	movl	PCB_F1(r0),f1
	movl	PCB_F3(r0),f3
	movl	PCB_F5(r0),f5
	movl	PCB_F7(r0),f7
#endif	MMAX_XPC
	lfsr	PCB_FSR(r0)
	lprw	mod,PCB_MODPSR(r0)		/* Restore this turkey */
	movd	PCB_PC(r0),tos			/* Save return address */

#if	MMAX_XPC
	/*
	 * On the XPC we must invalidate the instruction cache because the
	 * XPC does not propagate I-stream invalidates up to the 532.  It is
	 * possible (although unlikely) that we have bogus instructions cached
	 * when the page in main memory has been changed (say, as the result of
	 * resolving a page fault).  There is no need to flush the data cache.
	 *
	 * Flushing the I-cache here also takes care of the case where the user
	 * has requested a ptrace write to the child's text space; if we are
	 * the child about to execute we have made sure that we don't have the
	 * old instruction in our cache.
	 */
	cinv	[a,i], r0
#endif	MMAX_XPC
#if	MMAX_XPC || MMAX_APC
        movb    PCB_ISRV(r0),@(M_ICU_BASE+ISRV)
        movb    (PCB_ISRV+1)(r0),@(M_ICU_BASE+ISRV+RBIAS)
#endif	MMAX_XPC || MMAX_APC
#if	MMAX_DPC
	tbitw	$PSR_I_BIT,PCB_MODPSR+2(r0)	/* Were interrupts enabled? */
	bfs	.ldintenable
	DISINT
	br	.ldint
.ldintenable:
	ENBINT
.ldint:
#endif	MMAX_DPC
	movqd	$1,r0		# indicate load context return
	ret	$0
/*
 * save_context saves the context of the current thread into that thread's
 * pcb and switches to a per-processor stack.  The calling routine
 * must have ALL of its local variables in registers.
 */

	.globl	_active_threads
	.globl	_save_context_count
ENTRY(save_context)
	addqd	$1, @_save_context_count
	GETCPUID(r0)
	movd	_active_threads[r0:d],r0	/* This CPU's thread */
	movd	THREAD_PCB(r0),r0	/* The PCB ptr */
#if	MMAX_XPC || MMAX_APC
        movb    @(M_ICU_BASE+ISRV),PCB_ISRV(r0)
        movb    @(M_ICU_BASE+ISRV+RBIAS),(PCB_ISRV+1)(r0)
#endif	MMAX_XPC || MMAX_APC
	movd	tos,r1			/* Save the return PC */
	movd	r1,PCB_PC(r0)
	sprw	mod,PCB_MODPSR(r0)	/* Save this turkey */
	sprw	psr,PCB_MODPSR+2(r0)	/* Save the current interrupt state */
	DISINT
#if	MMAX_XPC
	sprd	usp,PCB_USP(r0)
#endif	MMAX_XPC
#if	MMAX_APC || MMAX_DPC
	USRSP
	sprd	sp,PCB_USP(r0)
	SYSSP
#endif	MMAX_APC || MMAX_DPC
	sprd	sp,PCB_SSP(r0)
	sprd	fp,PCB_FP(r0)
	movd	r7,PCB_R7(r0)
	movd	r6,PCB_R6(r0)
	movd	r5,PCB_R5(r0)
	movd	r4,PCB_R4(r0)
	movd	r3,PCB_R3(r0)
	sfsr	PCB_FSR(r0)
	movl	f0,PCB_F0(r0)
	movl	f2,PCB_F2(r0)
	movl	f4,PCB_F4(r0)
	movl	f6,PCB_F6(r0)
#if	MMAX_XPC
	movl	f1,PCB_F1(r0)
	movl	f3,PCB_F3(r0)
	movl	f5,PCB_F5(r0)
	movl	f7,PCB_F7(r0)
#endif	MMAX_XPC

	/* switch to interrupt stack */
	GETCPUID(r0)			# Get the processor number of this CPU
	movd	_interrupt_stack[r0:d],r0	# get its stack base
	addd	$INTSTACK_SIZE,r0	# go to end of stack
	lprd	sp,r0			# load stack
	lprd	fp,r0			#   registers

	movqd	$0,r0			# indicate save_context return
	jump	r1			# return to the caller

/*
 *	thread_begin is entry point for new threads; must check for immediate
 *	termination.
 */
	.globl _thread_begin
_thread_begin:
	br	?rett_ast
	
#endif	FAST_CSW


/*
.* gen_sysnmi - generate a system NMI
 *
.* ARGUMENTS: None
 *
.* USAGE:
 *	This routine is used to generate an NMI interrupt to all
 *	other CPUs in the system (including this CPU). On the MULTIMAX
 *	it generates the NMI on IO processors also, but they
 *	ignore it. This routine is used by the debugger to generate
 *	NMIs when all processors are to be stopped (at breakpoints
 *	and trace traps), and is used by panic() to stop all
 *	processors in the system.
 *
 */
ENTRY(gen_sysnmi)
#if	MMAX_XPC
	cbitw	$(XPCCSR_SYSNMI_BIT), @XPCREG_CSR # make it happen
#endif	MMAX_XPC
#if	MMAX_APC
        movqd   $APCNMI_VALUE,@APCREG_NMI /* Write to register */
#endif	MMAX_APC
#if	MMAX_DPC
	/*
	 * Turn on the system NMI bit in the in the
	 * DPCCTL register. The bit is auto-cleared.
	 */
	GETCPUID(r0)			/* Get the CPU number */
	movd	@_Dpc_ctl[r0:d],r0	/* Get the current value of register */
	ord	$DPCCTL_GEN_SYSNMI,r0	/* Turn on the system NMI bit */
	xord	$DPCCTL_FIX,r0		/* Flip around appropriate bits */
	movd	r0,@DPCREG_CTL		/* Load the register */
#endif	MMAX_DPC
	ret	$0

/*
 #
 # stop_cpus -- halt all cpus in their tracks
 #
 # Note that the calling CPU will also receive an NMI; what happens to
 # it is dependent on whether we're panicing, whether the debugger is
 # present, and so on.
 #
 */
	.set	nmi_lock, NMI_LOCK + NMI_STATE	# Also in SCC shared memory
	.set	nmi_owner, NMI_LOCK + NMI_OWNER	# Also in SCC shared memory
	.set	nmi_flags, NMI_LOCK + NMI_FLAGS	# Also in SCC shared memory

#if	MMAX_XPC
/*
 * The DPC and APC are constructed so that an NMI freezes the bus interface,
 * preventing the CPUs from continuing until the SCC has had a chance to
 * process the NMI, examine the state of the nmi_flags array, decide whether
 * the NMI was deliberately induced by a CPU, print the appropriate message,
 * and then unfreeze the bus.  The XPC, on the other hand, does not freeze
 * its bus interface when SYSNMI is asserted.  (Other bus transactions may
 * still be in progress and with the current writeback cache protocol some
 * of those transactions may require responses from the XPC.  Note that the
 * issue of freezing the bus interface has nothing to do with the CPU's
 * notion of ENBNMI/DISNMI.)  Thus the XPC continues immediately after
 * generating the SYSNMI and, were it not for the delay loop, would probably
 * clear the nmi_lock before the SCC had a chance to act on the NMI and
 * examine the lock.
 */
#define	XPC_NMI_SCC_DELAY	1000000
#endif	MMAX_XPC

	.globl	_stop_cpus
_stop_cpus:
	DISNMI(scpu.001, r0, r1)
	GETCPUID(r0)			# Find out who we are
scpu.nmilock:
	sbitib	$0, @nmi_lock		# Try lock guarding SCC nmi struct
	bfs	scpu.nmilock		# bfs = couldn't get lock

	addr	@NCPUS, r1		# Clear nmi_flags[0..MAXCPUS-1]
scpu.clear:
	movqb	$0, @nmi_flags-1[r1:b]
	acbd	$-1, r1, scpu.clear

	movqb	$-1, @nmi_flags[r0:b]	# Pretend the SCC is sending us an nmi
	movb	r0, @nmi_owner		# Take ownership of the lock

	jsr	@_gen_sysnmi		# Do the nmi

#if	MMAX_XPC
	movd	$XPC_NMI_SCC_DELAY, r0	# Give the SCC a chance to act...
nmi_delay_loop:
	acbd	$-1, r0, nmi_delay_loop
#endif	MMAX_XPC
	movqb	$0, @nmi_lock		# Release the SCC nmi lock
	ENBNMI(scpu.002, r0, r1)	# Re-enable NMIs
	ret	$0			# All Done

/*
 * Read one entry from the CPU's vector FIFO register.  This clocks down
 * the vectorbus fifo.
 */
    .globl  _read_fifo
_read_fifo:
#if	MMAX_XPC
	movzwd	@VB_FIFO, r0
#endif	MMAX_XPC
#if	MMAX_APC || MMAX_DPC
	movd    @VB_FIFO, r0
#endif	MMAX_APC || MMAX_DPC
	ret	$0

/*
 * bit_find_set(index, max, array) - return position of first bit set
 *	in a bit array of size max starting at (0-based) index.
 *	Optimized assuming array is double-word (32 bits) aligned.
 */
ENTRY(bit_find_set)
	movd	r1,tos		# save max on stack
	subd	r0,r1		# count = max - index
	movd	r0,r2		# take index
	ashd	$-3,r2		#   convert to byte offset
	bicd	$3,r2		#     align to double boundary
	addd	8(sp),r2	#       and add array address to get base
			# 8 = jsr address (4) + saved max (4)
	andd	$0x1f,r0	# convert index to offset within double
	addd	r0,r1		# add to count - this is a fudge because
			# the first time through the loop we will only compare
			# (32 - offset) bits, but subtract 32 from count.
bitfs.loop:
	ffsd	0(r2),r0	# Check here
	bfc	bitfs.found	# branch if found.
			# NOTE: failure to find clears r0, so next offset is 0
	addr	-32(r1),r1	# Subtract checked bits from count to check
	cmpqd	$0,r1		# Any left ??
	bge	bitfs.fail	# If not, failed to find a set bit.
	addqd	$4,r2		# advance base to next longword
	br	bitfs.loop	# continue with next check

bitfs.found:
	cmpd	r0,r1		# check where bit was found.
	bge	bitfs.fail	# return failure if out of range.
	subd	r1,tos		# convert max to index of first bit in the
				#   double where ffsd found a set bit.
	addd	tos,r0		# index of bit in array = index of first bit
				#   in double + offset within double.
	ret	$0		# done

bitfs.fail:
	movd	tos,r0		# get max (error code)
	ret	$0		#   and return

/*
 #
 #	cksum_mbuf -- Add to Internet checksum for one mbuf of data
 #
 #	This routine takes the data at the address specified with the length
 #	specified and adds each word into the checksum whose address is
 #	provided.
 #
 #
 */
ENTRY(cksum_mbuf)
	movd	r3, tos		# Save register
	movd	0(8(sp)),r3	# Get current checksum value

/*
 #	See if data buffer is aligned on a 32-bit boundary.  If not, add
 #	the first word to the checksum and adjust the starting address
 #	so an optimized loop can be used to add to the checksum 32 bits at
 #	a time.
 */

	movqd	$2, r2		#
	andd	r0, r2		#
	cmpqd	$0, r2		# Is this on a longword boundary?
	beq	cksum_32	# Yes, no need to align
	cmpqd	$2, r1		# Is the length at least 2 bytes?
	bgt	cksum_32	# No, so don't bother to align
	movzwd	0(r0), r2	# Get copy of first word
	addd	r2, r3		# Add first word to checksum
	addr	2(r0), r0	# Skip first word
	addqd	$-2, r1		# Subtract first word from length

/*
 #	As long as there is enough data to handle 32 bits at a time, add
 #	32 bits to the checksum with carry added.  Do as much checksum as
 #	possible 32 bits at a time.  In fact, this loop is unrolled to
 #	make overhead from branches small.
 #
 #	We can do a 16 bit ones complement sum 32 bits at a time because the
 #	32 bit register is acting as two 16 bit registers for adding, with
 #	carries from the low added into the high (by normal carry-chaining)
 #	and carries from the high carried into the low on the next word
 #	by use of the addcd instruction.  This lets us run this loop at
 #	almost memory speed.
 #
 #	Here there is the danger of high order carry out, and we carefully
 #	use addcd.
 */

cksum_32:
	addd	$-32, r1	# Subtract next 32 bytes from length
	cmpqd	$0, r1		# Are there 32 bytes left?
	bgt	cksum_add32	# No, see if there are 8 bytes
	bicpsrb	$(PSR_C)	# Clear carry bit in psr
	addcd	0(r0), r3	# Add next 32 bytes 4 bytes at a time
	addcd	4(r0), r3	#
	addcd	8(r0), r3	#
	addcd	12(r0), r3	#
	addcd	16(r0), r3	#
	addcd	20(r0), r3	#
	addcd	24(r0), r3	#
	addcd	28(r0), r3	#
	addcd	$0, r3		# Add in remaining carry to sum
	addr	32(r0), r0	# Skip 32 bytes just handled
	br	cksum_32	# Go back to handle next 32 bytes

/*
 #	There aren't 32 bytes left, but there may be enough to handle the
 #	remainder in multiples of 8 bytes.
 */

cksum_add32:
	addd	$32, r1		# Compensate for last 32 taken from len
cksum_8:
	addqd	$-8, r1		# Subtract next 8 bytes from len
	cmpqd	$0, r1		# Are there 8 bytes left?
	bgt	cksum_comb	# No, go combine 16 bit numbers
	bicpsrb	$(PSR_C)	# Clear carry bit in psr
	addcd	0(r0), r3	# Add next 8 bytes 4 bytes at a time
	addcd	4(r0), r3	#
	addcd	$0, r3		# Add in remaining carry to sum
	addr	8(r0), r0	# Skip 8 bytes just handled
	br	cksum_8		# Go back to handle next 8 bytes

/*
 #	Now eliminate the possibility of carry-out's by folding back to a
 #	16 bit number (adding high and low parts together.)  Then mop up
 #	trailing words and maybe an odd byte.
 */

cksum_comb:
	addd	$8, r1		# Compensate for last 8 taken from len
	movd	r3, r2		# Get copy of checksum
	ashd	$-16, r2	# Shift high part to low part of reg.
	addw	r2, r3		# Add high and low part together
	addcd	$0, r3		# Add carry to checksum
	movzwd	r3, r3		# Zero out high word of result

/*
 #	Handle remaining bytes a word at a time
 */

cksum_2:
	addqd	$-2, r1		# Subtract next 2 bytes to handle
	cmpqd	$0, r1		# Are there 2 bytes left?
	bgt	cksum_1		# No, go check for odd byte
	movzwd	0(r0), r2	# Yes, get next word
	addr	2(r0), r0	# Skip word just copied
	addd	r2, r3		# Add word to checksum
	br	cksum_2		# Go back and handle next word

/*
 #	See if there is an odd byte.  If not, then just put the checksum
 #	back at the address specified and return 0 to the caller.  If so,
 #	add this byte to the checksum, return it to the address specified,
 # 	and return 1 to indicate there is a word split across an mbuf.
 */

cksum_1:
	movd	r0, r2		# Save arg so we can use r0
	movqd	$0, r0		# Assume there is no byte left
	cmpqd	$-1, r1		# Is there a byte left?
	bne	cksum_exit	# No, exit with return of 0
	movzbd	0(r2), r2	# Get copy of last byte
	addd	r2, r3		# Add in extra byte
	movqd	$1, r0		# Set return value to 1 for extra byte
cksum_exit:
	movd	r3, 0(8(sp))	# Save checksum
	movd	tos, r3		# Restore register
	ret	$0
/*
 #
 #	cksum_fold -- Fold high and low parts of Internet checksum
 #
 #	Fold high 16-bits and low 16-bits of checksum into one 16-bit
 #	checksum with carry added.
 #
 */
ENTRY(cksum_fold)
	movd	r0, r1		# Get copy of checksum
	ashd	$-16, r1	# Shift high 16-bits to low 16 of r1
	addw	r1, r0		# Add high 16-bits to low 16
	addcd	$0, r0		# Add carry bit
	comd	r0, r0		# One's complement of checksum
	movzwd	r0, r0		# Zero high 16 bits
	ret	$0		# Return 16-bit checksum


/*
 * setjmp (address) - char	*address
 *	Stores information at address to enable a "return" to the current
 *	user PC.
 */

ENTRY(setjmp)
				# r0 has the address of the data storage area.

	movd    r3,0(r0)	#"Push" general registers.
	movd    r4,4(r0)
	movd    r5,8(r0)
	movd    r6,12(r0)
	movd    r7,16(r0)
	sprd    fp,20(r0)	#"Push" FP
	sprd    sp,24(r0)	#"Push" user SP
	sprw    upsr,30(r0)	#"Push" user PSR (condition codes)
	movd    0(sp),32(r0)	#"Push" PC
	movqd   $0,r0		#Return 0 to 'parent'.
	ret     $0


/*
 # longjmp (address, val) - char	*address
 #			   int	val
 #	Reads the information from address and simulates a return to the
 #	stored PC.  Uses rett to restore the PSR on return.  Returns val
 #	if non-zero, else returns 1.
 */

ENTRY(longjmp)
				# R0 has the addr of the stored return values.
				# R1 has the return value

	movd    0(r0),r3	#Restore the general registers.
	movd    4(r0),r4
	movd    8(r0),r5
	movd    12(r0),r6
	movd    16(r0),r7
	lprd    fp,20(r0)	# Restore FP
	lprw    upsr,30(r0)	# Restore user PSR (condition codes)
	movd    32(r0),r2	# Restore PC
	lprd    sp,24(r0)	# Restore user SP
	movd	tos, r0		# (skip over where return address would go)
	movd	r1,r0		# Setup return value
	cmpqd	$0,r0		# Guarantee that r0 != 0
	bne	longret
	movqd	$1,r0
longret:
	jump	0(r2)		# Return to 'child' (r0 = (val) ? val : 1).



/*
 * float_sync(pcb)
 * struct pcb *pcb;
 *
 * Copy floating point state from the chip to memory.
 */
ENTRY(float_sync)
	sfsr	PCB_FSR(r0)
	movl	f0, PCB_F0(r0)
	movl	f2, PCB_F2(r0)
	movl	f4, PCB_F4(r0)
	movl	f6, PCB_F6(r0)
#if	MMAX_XPC
	movl	f1, PCB_F1(r0)
	movl	f3, PCB_F3(r0)
	movl	f5, PCB_F5(r0)
	movl	f7, PCB_F7(r0)
#endif	MMAX_XPC
	lfsr	PCB_FSR(r0)		# paranoid:  to preserve TT (XXX)
	ret	$0

#if	!MMAX_IDEBUG
/*
 * Disable NMIs (basically all possible interrupt sources) and return
 * the old ICU value.  Used for special cases where the system can't
 * take any interrupts at all; for example, the console_printf_lock can
 * be required at any interrupt level, so while holding that lock is is
 * necessary to screen out all interrupts.
 */
ENTRY(splnmi)
	movb	@M_ICU_BASE+ISRV, r0
	sbitb	$ICU_POWERFAIL_BIT, @M_ICU_BASE+ISRV
	ret	$0
ENTRY(splnmix)
	movb	r0, @M_ICU_BASE+ISRV
	ret	$0
#endif	!MMAX_IDEBUG

/*
 * Miscellaneous locore-related data definitions.
 */
	.text
/*
 *  Arrange for the include file version number to appear directly in
 *  the name list for ease of use by vmversion(3).
 */
#include <sys/version.h>
	.globl	_INCLUDE_VERSION
	.set	_INCLUDE_VERSION,INCLUDE_VERSION


	.data
	.globl	_save_context_count
_save_context_count:	.double	0
	.globl	_load_context_count
_load_context_count:	.double 0
	.globl	_swtch_tsk_ctxt_cnt	/* Extern task context switch count */
	.globl	_swtch_thrd_ctxt_cnt	/* Extern thread context switch count */
	.globl	_nmicnt			/* interrupt counts */
_nmicnt:.double	0
	.globl	_abtcnt
_abtcnt:.double	0
	.globl	_fpucnt
_fpucnt:.double	0
	.globl	_illcnt
_illcnt:.double	0
	.globl	_svccnt
_svccnt:.double	0
	.globl	_dvzcnt
_dvzcnt:.double	0
	.globl	_flgcnt
_flgcnt:.double	0
	.globl	_bptcnt
_bptcnt:.double	0
	.globl	_trccnt
_trccnt:.double	0
	.globl	_undcnt
_undcnt:.double	0
	.globl	_astcnt
_astcnt:.double	0

#if	MMAX_XPC
	.globl _ovfcnt
_ovfcnt:.double 0
	.globl _dbgcnt
_dbgcnt:.double 0
#endif	MMAX_XPC

#if	MMAX_XPC || MMAX_APC
	.globl _tsecnt
_tsecnt:.double 0
	.globl _sticnt
_sticnt:.double 0
	.globl _bogcnt
_bogcnt:.double 0
	.globl	_vec2cnt
_vec2cnt:	.double	0
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_APC
	.globl _bercnt
_bercnt:.double 0
#endif	MMAX_APC

	.globl	_psa			/* panic save areas for all cpus */
_psa:	.space	TOT_PSASIZ

	.globl	_intstack		/* temporary interrupt stack */
_intstack:
	.space	INTSTACK_SIZE
eintstack:				/* top of interrupt stack */
	.globl	_eintstack
_eintstack:

	/*
	 *	The following variables are in the data segment so
	 *	that we don't lose their values when we clear
	 *	the bss segment later.
	 */
	.globl	_boothowto
	.globl	_bootdev
	.globl	_OSmodpsr /* valid modpsr pair for operating system */
	.data
_boothowto:	.double	0
_bootdev:	.double	0
_OSmodpsr:	.double	0
