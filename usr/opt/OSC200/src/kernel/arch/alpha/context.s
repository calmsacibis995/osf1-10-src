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
	.rdata
	.asciiz "@(#)$RCSfile: context.s,v $ $Revision: 1.2.10.2 $ (DEC) $Date: 1993/04/23 21:07:30 $"
	.text

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
 *	Modification History: alpha/context.s
 *
 *	Context switching and context save/restore primitives
 *
 * 09-Sep-91	rjl
 *
 *	Added alpha specific primitives
 *
 */
#include <fast_csw.h>
#include <mach_kdb.h>

#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <machine/pcb.h>
#include <mach/alpha/vm_param.h>
#include <machine/pmap.h>
#include <machine/thread.h>
#include <assym.s>

	BSS(current_kstack,8)		# for exception()

	BSS(t0_temp,8)			# squirrel away t0 for save_context_all
	BSS(t1_temp,8)
	BSS(t2_temp,8)
	BSS(t3_temp,8)
        BSS(t4_temp,8)


/*
 * Note: uPAL trashes t0..t5, use a-registers as temps
 */
	.set	noreorder		# unless overridden

/*
 * save_context()
 *
 * 	Save thread's context in the pcb
 *
 * Description
 *	Routine MUST be called at SPLHIGH. 
 *	Saves all registers that have not already been saved
 *	on the stack (by C calling conventions) in the pcb.
 *	Saves status register and stack pointer in pcb as well.
 */

LEAF(save_context)
	/*
	 * Save C linkage registers
	 */
	ldq	a5,current_pcb
	lda	a5,PCB_REGS(a5)

	/*
	 * Now we're at high IPL
	 */
	stq	ra,PCB_PC*8(a5)	
	stq	sp,PCB_SP*8(a5)		# only needed for dbx tracebacks
	/*
	 * Save callee saved registers, all other live registers should have
	 * been saved on call to save_context() via C calling conventions
	 */
	stq	s0,PCB_S0*8(a5)
	stq	s1,PCB_S1*8(a5)
	stq	s2,PCB_S2*8(a5)
	stq	s3,PCB_S3*8(a5)
	stq	s4,PCB_S4*8(a5)
	stq	s5,PCB_S5*8(a5)
	stq	s6,PCB_S6*8(a5)
	/*
	 * See if the floating point unit is enabled
	 */
	ldq	a0,PCB_FEN-PCB_REGS(a5) # offset to fen word of the PCB
	blbs	a0,1f			# if low bit clear, not used
	bis	zero,zero,v0		# return 0
	ret	zero,(ra)
1:
	lda	a0,PCB_FPREGS-PCB_REGS(a5) # get address of fpoint register save
	/*	
	 * freg_save writes no registers.
	 */
	bsr	ra,freg_save		# save them as well
	excb
	mf_fpcr $f0, $f0, $f0           # read fpcr
	trapb
	lda	a0, PCB_FPCR-PCB_REGS(a5)
	stt     $f0,0(a0)		# store fpcr contents in pcb
	bis	zero,zero,a0		# s0 (r16) = 0
	call_pal PAL_wrfen		# turn off fpu
	ldq	ra,PCB_PC*8(a5)		# restore the return register

	bis	zero,zero,v0		# return 0
	ret	zero,(ra)
	 
	END(save_context)
/*
 * load_context(thread)
 *
 * Purpose
 * 	Restore state of the given thread, from its pcb
 *
 * Description
 *	Installs this as the current thread on all the various
 *	globals: active_threads, current_kstack, current_pcb, U_ADDRESS.
 *	Enables use of the FPU (when back to user) if nobody
 *	has changed its content, but only if we ever used it.
 *	Restores the state that save_context() has saved in the PCB
 *	and returns control to the thread [which will then return
 *	from save_context()]
 *
 *	Note that fp context gets restored by enable_fen.
 */
LEAF(load_context)
	/*
	 * 	a0 contains a thread pointer
	 *
	 */
	bis	a0,zero,a5		# save the thread address
	ldq	a4,THREAD_PCB(a5)	# thread->pcb

	ldq	a0,PCB_PADDR(a4)	#            ->pcb_paddr
	call_pal PAL_swpctx		# swap pcb's
	stq	a5,the_current_thread	# active_thread
	stq	a4,current_pcb		# current process control block

	.globl	U_ADDRESS		# inlining of load_context_data()
	lda	s0,U_ADDRESS
	ldq	s1,UTHREAD(a5)		# thread data
	ldq	s2,UTASK(a5)		# task data
	stq	s1,0(s0)		# U_ADDRESS.uthread
	stq	s2,8(s0)		# U_ADDRESS.utask

	/*
	 * Reload callee saved registers, all other live registers
	 * should be reloaded from stack via C calling conventions.
	 */
	lda	a4,PCB_REGS(a4)		# get the address of the registers
	ldq	s0,PCB_S0*8(a4)
	ldq	s1,PCB_S1*8(a4)
	ldq	s2,PCB_S2*8(a4)
	ldq	s3,PCB_S3*8(a4)
	ldq	s4,PCB_S4*8(a4)
	ldq	s5,PCB_S5*8(a4)
	ldq	s6,PCB_S6*8(a4)
	ldq	ra,PCB_PC*8(a4)
	ldiq	v0,1			# return non-zero
	ret	zero,(ra)
	END(load_context)


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
	END(kdblongjmp)
#endif	/* MACH_KDB */

/*
 * switch_context(old_thread,new_thread)
 *
 * Purpose
 *	Switch context among two threads;
 *
 * Description
 */
LEAF(switch_context)
	/*
	 * Save C linkage registers
	 */
	ldq	a5,current_pcb
	lda	a5,PCB_REGS(a5)

	stq	ra,PCB_PC*8(a5)	
	stq	sp,PCB_SP*8(a5)		# only needed for dbx tracebacks
	/*
	 * Save callee saved registers, all other live registers should have
	 * been saved on call to save_context() via C calling conventions
	 */
	stq	s0,PCB_S0*8(a5)
	stq	s1,PCB_S1*8(a5)
	stq	s2,PCB_S2*8(a5)
	stq	s3,PCB_S3*8(a5)
	stq	s4,PCB_S4*8(a5)
	stq	s5,PCB_S5*8(a5)
	stq	s6,PCB_S6*8(a5)
	/*
	 * See if the floating point unit is enabled
	 */
	ldq	a2,PCB_FEN-PCB_REGS(a5) # offset to fen word of the PCB
	bis	a0,zero,a3		# save old thread address
	bis	a1,zero,s5		# save new thread address
	blbs	a2,1f			# if low bit clear, not used

	/*
	 * 	a3 contains OLD thread pointer
	 * 	s5 contains NEW thread pointer
	 *
	 */
2:
	ldq	s4,THREAD_PCB(s5)	# thread->pcb

	ldq	a0,PCB_PADDR(s4)	#            ->pcb_paddr
	call_pal PAL_swpctx		# swap pcb's

	stq	s5,the_current_thread	# active_thread
	stq	s4,current_pcb		# current process control block

	.globl	U_ADDRESS		# inlining of load_context_data()
	ldq	s1,UTHREAD(s5)		# thread data
	ldq	s2,UTASK(s5)		# task data
	lda	s0,U_ADDRESS
	stq	s1,0(s0)		# U_ADDRESS.uthread
	stq	s2,8(s0)		# U_ADDRESS.utask

	/*
	 * Reload callee saved registers, all other live registers
	 * should be reloaded from stack via C calling conventions.
	 */
	lda	a4,PCB_REGS(s4)		# get the address of the registers
	ldq	s0,PCB_S0*8(a4)
	ldq	s1,PCB_S1*8(a4)
	ldq	s2,PCB_S2*8(a4)
	ldq	s3,PCB_S3*8(a4)
	ldq	s4,PCB_S4*8(a4)
	ldq	s5,PCB_S5*8(a4)
	ldq	s6,PCB_S6*8(a4)
	ldq	ra,PCB_PC*8(a4)

	/* 
	 * We are completely done with the old thread.  Release it now
	 * by calling thread_continue. 
	 * NO RETURN!!! After thread_continue we pick up execution wherever
	 * restored RA pointes to 
	 */
	bis     a3,zero,a0              # pass in old thread pointer 
	bsr	zero,thread_continue

1:
	lda	a0,PCB_FPREGS-PCB_REGS(a5) # get address of fpoint register save
	/*	
	 * freg_save writes no registers.
	 */
	bsr	ra,freg_save		# save them as well
	excb
	mf_fpcr $f0, $f0, $f0           # read fpcr
	trapb
	lda	a0, PCB_FPCR-PCB_REGS(a5)
	stt     $f0,0(a0)		# store fpcr contents in pcb
	bis	zero,zero,a0		# s0 (r16) = 0
	call_pal PAL_wrfen		# turn off fpu
	br	zero,2b

	END(switch_context)



/*
 * save_context_all()
 *
 * 	Save all registers in the pcb, for debugging
 *
 * Description
 *	Saves all registers in the pcb. Called during panic/reboot so that
 *	all register contents can be easily located when debugging.
 *	Saves status register and stack pointer in pcb as well.
 *	Returns 0 at splhigh() ( with interrupts off )
 */
LEAF(save_context_all)
	/*
	 * This is only used for panic/reboot so it doesn't need to be
	 * MP safe (?).  In any case the problem here is that there aren't
	 * any registers that can be used, everything must be saved so we
	 * need to use global memory location.
	 *
	 * We save everything but at, by the time you get here at has already
	 * been trashed a couple of times so what's the use in trying thwart
	 * the assembler?
	 */
	lda	sp,-16(sp)		# get some scratch space on stack
	stq	a0,0(sp)
	stq	a1,8(sp)
	ldq	a0,current_pcb		# get pcb address of current proc
	lda	a1,PCB_REGS(a0)		# get address of reg save area

	stq	v0,PCB_V0*8(a1)
	stq	t0,PCB_T0*8(a1)
	stq	t1,PCB_T1*8(a1)
	stq	t2,PCB_T2*8(a1)
	stq	t3,PCB_T3*8(a1)
	stq	t4,PCB_T4*8(a1)
	stq	t5,PCB_T5*8(a1)
	stq	t6,PCB_T6*8(a1)
	stq	t7,PCB_T7*8(a1)
	stq	s0,PCB_S0*8(a1)
	stq	s1,PCB_S1*8(a1)
	stq	s2,PCB_S2*8(a1)
	stq	s3,PCB_S3*8(a1)
	stq	s4,PCB_S4*8(a1)
	stq	s5,PCB_S5*8(a1)
	stq	s6,PCB_S6*8(a1)
	stq	a3,PCB_A3*8(a1)
	stq	a4,PCB_A4*8(a1)
	stq	a5,PCB_A5*8(a1)
	stq	t8,PCB_T8*8(a1)
	stq	t9,PCB_T9*8(a1)
	stq	t10,PCB_T10*8(a1)
	stq	t11,PCB_T11*8(a1)
	stq	ra,PCB_RA*8(a1)
	stq	t12,PCB_T12*8(a1)
	ldq	t0,0(sp)
	ldq	t1,8(sp)
	lda	sp,16(sp)		# release stack scratch space now
	stq	sp,PCB_SP*8(a1)		# only needed for dbx tracebacks
	stq	ra,PCB_PC*8(a1)
	stq	gp,PCB_GP*8(a1)
	stq	t0,PCB_A0*8(a1)
	stq	t1,PCB_A1*8(a1)
	stq	a2,PCB_A2*8(a1)

	/*
	 * See if the floating point unit is enabled
	 */
	ldq	t2,PCB_FEN(a0)		# offset to fen word of the PCB
	blbc	t2,1f			# if low bit clear, not used

	lda	a0,PCB_FPREGS(a0)	# get address of fp reg save area
	bsr	ra,freg_save		# save all fp regs in the PCB
	ldq	ra,PCB_RA*8(a1)		# restore old return address
	bis	zero,zero,a0		# assign disable flag arg
	call_pal PAL_wrfen		# turn off fpu

1:	bis	zero,zero,v0		# return 0
	ret	zero,(ra)
	 
	END(save_context_all)
