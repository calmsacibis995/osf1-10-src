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
 *      @(#)$RCSfile: cma_assem.s,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1992/09/29 14:26:30 $
 */
/*
 *  Copyright (c) 1989, 1992
 *  by Digital Equipment Corporation, Maynard, Mass.
 *
 *  This software is furnished under a license and may be used and  copied
 *  only  in  accordance  with  the  terms  of  such  license and with the
 *  inclusion of the above copyright notice.  This software or  any  other
 *  copies  thereof may not be provided or otherwise made available to any
 *  other person.  No title to and ownership of  the  software  is  hereby
 *  transferred.
 *
 *  The information in this software is subject to change  without  notice
 *  and  should  not  be  construed  as  a commitment by Digital Equipment
 *  Corporation.
 *
 *  Digital assumes no responsibility for the use or  reliability  of  its
 *  software on equipment which is not supplied by Digital.
 */

/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	These are non-portable subroutines for thread service routines.  They
 *	cannot be written in C because they manipulate specific registers, etc.
 *
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	11 October 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	23 October 1989
 *		Created cma__c_default_ps
 *	002	Dave Butenhof	31 October 1989
 *		Add cma__fetch_sp routine
 *	003	Webb Scales	31 October 1989
 *		Fix bugs
 *	004	Webb Scales and Dave Butenhof	All Saints Day 1989
 *		Remove calls to enter/exit kernel routines.
 *	005	Dave Butenhof and Webb Scales	All Saints Day 1989
 *		Fix local label added in last change.
 *	006	CMA Team	3 November 1989
 *		Call undefer inside kernel region.
 *	007	Webb Scales	7 November 1989
 *		Brought cma__force_dispatch up to spec.
 *	008	Webb Scales	17 November 1989
 *		Totally reworked the code as a result of looking at the
 *		MIPS/Ultrix kernel code:
 *		 o  Beautified the code using cpp directives
 *		 o  Only save non-volatile context for normal context switch
 *		 o  Save full context & restore with syscall for asynch switch
 *	009	Webb Scales and Paul Curtin	6 July 1990
 *		Changed addu to subu in call on stack routine
 *	010	Dave Butenhof	16 August 1990
 *		Implement "springboard" routine for asynchronous alert
 *		delivery.
 *	011	Webb Scales	 2 October 1990
 *		Removed copyright character from file.
 *	012	Dave Butenhof	12 December 1990
 *		Change #include for OSF build structure (cma_host.h rather
 *		than the old "cma_host_mips_ultrix.h").
 *	013	Dave Butenhof	02 April 1991
 *		Support generic OSF/1 build
 *	014	Dave Butenhof	04 April 1991
 *		Support parallel threads with VP launching pad.
 *	015	Paul Curtin	08 April 1991
 *		Added cma__transfer main abort, etc.
 *	017	Dave Butenhof	25 April 1991
 *		Remove experimental locking and rely on O/S calls (blech).
 *	018	Dave Butenhof	14 May 1991
 *		Remove trace calls.
 *	019	Dave Butenhof	11 June 1991
 *		Support kernel lock tracing on UNIPROCESSOR by calling the
 *		cma__{enter|exit}_kern_record functions.
 *	020	Dave Butenhof	23 July 1991
 *		Modify instruction sequences to satisfy OSF/ULTRIX ld.
 *	021	Dave Butenhof	23 August 1991
 *		Continue 020.
 *	022	Dave Butenhof	29 August 1991
 *		Investigation of a QAR (cma_printf wrapper doesn't work right
 *		in a thread) revealed an undocumented rule of the MIPS
 *		calling convention: the stack must remain double-word
 *		aligned. Previously, the cma__execute_thread() assembler
 *		function that begins a thread, and takes 3 arguments, was
 *		causing an initial allocation of 3 words on the stack,
 *		breaking the alignment rules and causing varargs/stdargs to
 *		incorrectly round addresses. This change increases the stack
 *		allocation for those 3 arguments (in cma__create_thread) to 4 
 *		words so that double word alignment is retained.
 *	023	Dave Butenhof	29 October 1991
 *		Remove an unused external symbol reference that's causing
 *		trouble at OSF.
 *	024	Dave Butenhof	05 November 1991
 *		Integrate Dave Weisman's changes for OSF/1. The GNU assembler
 *		doesn't properly handle the "ld" macro, so expand all
 *		instances of "ld" to two "lw" opcodes. For symmetry (even
 *		though apparently not necessary), also expand "sd" to "sw".
 *	025	Dave Butenhof	21 January 1992
 *		Integrate Ken Ouellette's changes to compensate for picie's
 *		modifications of the machine code during creation of an OSF/1
 *		shareable library. Some of these changes are "clean": like
 *		saving and restoring additional registers. Other changes are
 *		gross hacks, such as compensating for the fact that picie
 *		reacts to swapping the sp value by incrementing or
 *		decrementing the result. The changes are conditionalized
 *		under "_CMA_SHLIB_" because the hacks would break normal
 *		operation (the other changes should be safe without picie).
 *	026	Brian Keane 	4 March 1992
 *		Tweak code prior to syscall in cma__force_dispatch for picie
 *		quirk.  Defer loading v0 with the syscall to make until
 *              right before the syscall, as picie is observed to trash v0.
 *	027	Brian Keane 	13 March 1992
 *		Conditionalize out call_on_stack if not _CMA_SHLIB_.
 *	028	Dave Butenhof	13 March 1992
 *		Eliminate excess arguments to cma__thread_base, since they're
 *		not used anyway (and haven't been for some time). Save a few
 *		cycles during thread create!
 *	029	Brian Keane 	13 April 1992
 *		More picie fixes.  Copy sp to t2 in force_dispatch so that
 *		picie doesn't interfere with the offsets into context block.
 *		Save general registers before floating registers so that t2
 *		is available for use.   Only save t9 and gp once if doing
 *              a shared library.
 *	030	Brian Keane 	16 April 1992
 *		Still more picie fixes.  Use s0 rather than t2 in force_dispatch
 *              so the value is preserved across routine calls.  Fix similar
 *		sp related problems in transfer_thread_context.
 */

/*
 *	A thread context consists of the saved register set necessary for
 *	rescheduling the thread on another processor.
 *
 *	The context consists of the general CPU registers $1-$31 (the value 
 *	of $0 never changes and thus need not be saved), the integer divide
 *	registers HI & LO, the floating point coprocessor registers $f0-$f31,
 *	and the floating point coprocessor control/status register FCR31.
 *
 *	The context is pushed onto the stack (fetching FCR31 has the desirable
 *	side effect of synchronizing with the floating point coprocessor and
 *	thereby preventing an exception resulting from interrupting an
 *	concurrent floating point instruction execution), and the sp is
 *	stored in the static context block in the TCB.
 *
 *	The process is slightly more complicated in the case of an
 *	asynchronous context switch, such as a quantum end.  The interrupt
 *	routine saves the return PC and the floating point coprocessor status
 *	in the TCB, replaces these values with the address of
 *	cma__force_dispatch and a known state status word.  When the interrupt
 *	routine returns, execution resumes in cma__force_dispatch instead of
 *	at the point where the interruption occurred.  There, the volatile
 *	registers are saved in a context area on the stack.  The saved values
 *	of the PC and processor status at the point of interruption are
 *	retrieved from the TCB by a call to a C subroutine and stored in the
 *	context area on the stack. Then the usual dispatching routines are
 *	called, resulting in a second, full context being saved on the stack.
 *	When the thread context is restored as a result of being rescheduled,
 *	it returns to cma__force_dispatch where the volatile registers, PC,
 *	and PS are restored allowing the thread to resume execution at the
 *	point where the interruption occurred.
 */

#ifdef __LANGUAGE_C__
# undef __LANGUAGE_C__
#endif

#ifndef __LANGUAGE_ASSEMBLY__
# define __LANGUAGE_ASSEMBLY__
#endif

#include <machine/regdef.h>
#include <machine/asm.h>
#include <machine/fpu.h>
#include <machine/inst.h>
#include <sys/syscall.h>
#include <setjmp.h>
#include <cma_config.h>
#include <cma_host.h>
#include <cma_defs.h>

#define ROUTINE(name)								\
NESTED(name, FSIZE, ra)			; /* FSIZE is #def'd for in routine */	\
	subu	sp, FSIZE		; /* Create frame for routine */	\
	sw	ra, FSIZE-FOFFS(sp)	; /* Fetch the return address */

#define RETURN									\
	lw	ra, FSIZE-FOFFS(sp)	; /* Fetch the return address */	\
	addu	sp, FSIZE		; /* Pop the frame */			\
	j	ra			; /* And return */

#undef	CALL
#define	CALL	jal


/*
 * Convenient stack offsets for routines that make calls when loading or
 * storing argument values.
 */
#define AA0	0
#define AA1	4
#define AA2	4*2
#define AA3	4*3
#define AA4	4*4
#define AA5	4*5
#define AA6	4*6
#define AA7	4*7
#define AA8	4*8

/*
 * Routines which are referenced below:
 */
	.globl	cma__abort_process
	.globl	cma__async_delivery
	.globl	cma__bugcheck
#ifdef _CMA_TRACE_KERNEL_
	.globl	cma__enter_kern_record
	.globl	cma__exit_kern_record
#endif
	.globl	cma__get_async_info
	.globl	cma__get_self_tcb
	.globl	cma__start_thread
	.globl	cma__thread_base
	.globl	cma__undefer
#ifdef _CMA_TRACE_KERNEL_
	.globl	cma__unset_kern_record
#endif
	.globl	cma__yield_processor
#if !_CMA_UNIPROCESSOR_
	.globl	cma__vp_lock
	.globl	cma__vp_unlock
#endif

/*
 * Global data:
 */
	IMPORT(cma__g_abort_signal, 4)
	IMPORT(cma__g_def_ctx_buf, 4)
	IMPORT(cma__g_kernel_critical, 4)

	.rdata

/*
 * This is the default value of the FPA control/status register
 * (Actually, this is my best guess):  No pending or past exceptions,
 * cleared condition bit, all traps enabled, round-to-nearest mode
 */
	EXPORT(cma__c_default_ps)
	.word	0x00000f80

	.data
#ifdef _CMA_TRACE_KERNEL_
	.align	2
filename: .ascii "cma_assem.s\000"
#endif
	.align	2
async_alert_bug:	.ascii	"do_async_alert:1\000"
	.text	
	.align	2

#if !_CMA_THREAD_IS_VP_
/*
 * cma__create_thread
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	 This routine initializes a thread (ie, its stack and context) and
 *	 prepares it for the scheduler
 *
 * FORMAL PARAMETERS:
 * 
 *	 a0	- the address of the child context buffer
 *	 a1	- the address of the base of the child thread stack
 *	 a2	- the address of the child TCB
 * 
 * IMPLICIT INPUTS:
 * 
 *	 None
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	 The child thread stack is initialized with a simulated call frame for
 *	 the call to cma__execute_thread and a context block which will cause
 *	 the new thread to begin execution at the start of that routine.  We
 *	 don't bother to initialize any of the other registers.
 *
 * FUNCTION VALUE:
 * 
 *	 None 
 * 
 * SIDE EFFECTS:
 * 
 *	 None
 *
 *	 NOTE:  This routine assumes the stack is already longword aligned
 *
 *	 NOTE:  This routine assumes that any interruptions which occur while
 *		we are one the child's stack instead of this thread's own stack
 *		won't be concerned about it.
 *
 *	 NOTE: This routine currently assumes that this thread will not be
 *		preempted during the the call to cma__start_thread.  This will
 *		be a feature which is added later with priorities.  At that
 *		time we should be sure that the kernel is entered/exited
 *		correctly.
 *
 *	 NOTE: We don't copy the register-base parameters on to the stack
 *		because we don't use them after we call our subroutines.
 *
 * Frame contains:
 *	 two saved registers
 *	 no local variables
 */
#undef	FSIZE
#define	FSIZE 12*4		/* Room for 10 args + 2 saved regs */
#undef	FOFFS
#define	FOFFS 1*4

#define C_SP	a1

ROUTINE(cma__create_thread)
	sw	s0, FSIZE-FOFFS-4(sp)
	.mask	0x80010000, -FOFFS	/* M_RA+M_S0 */

/*
 * Prevent reschedule interruptions
 */
#ifdef _CMA_TRACE_KERNEL_
	sw	a0, FSIZE(sp)
	sw	a1, FSIZE+4(sp)
	sw	a2, FSIZE+8(sp)
	sw	a3, FSIZE+12(sp)
	li	a0, __LINE__
	la	a1, filename
	jal	cma__enter_kern_record
	lw	a0, FSIZE(sp)
	lw	a1, FSIZE+4(sp)
	lw	a2, FSIZE+8(sp)
	lw	a3, FSIZE+12(sp)
#else
# if _CMA_UNIPROCESSOR_
10:	lw	s0, cma__g_kernel_critical
	bnez	s0, 10b
	li	t0, 1
	sw	t0, cma__g_kernel_critical
# else
	sw	a0, FSIZE(sp)
	sw	a1, FSIZE+4(sp)
	sw	a2, FSIZE+8(sp)
	sw	a3, FSIZE+12(sp)
10:	la	a0, cma__g_kernel_critical
	CALL	cma__tas
	bnez	v0, 10b
	lw	a0, FSIZE(sp)
	lw	a1, FSIZE+4(sp)
	lw	a2, FSIZE+8(sp)
	lw	a3, FSIZE+12(sp)
# endif
#endif

/*
 * Initialize the new thread's stack for "call" to cma__execute_thread.
 * NOTE: it must be kept 64-bit aligned, so allocate two words even though
 * there's only one argument.
 */
	subu	C_SP, 2*4	/* This is space for the argument list. */

/*
 * Build the parameter list for cma__execute_thread on the stack.
 * Technically the first four argments should be passed in registers,
 * but cma__transfer_thread_ctx doesn't restore volatile registers, like
 * the arguments registers.  Since there is room reserved on the stack
 * for these arguments anyway, put them there, and we'll make sure that
 * cma__execute_thread looks for them there instead of in the registers.
 */

/*
 * Store the TCB address in "a0"
 */
	sw	a2, 0(C_SP)

/*
 * Set up a context area on the stack so that cma__restore_thread_ctx
 * will do the right thing.
 */
	subu	C_SP, _JBLEN*4	/* This is space for the context area. */
	sw	C_SP, 0(a0)	/* Store sp in static context buffer */

/*
 * The rest of the stack frame is built by cma__execute_thread
 */

/*
 * Set up "ra" in the context buffer so that the new thread "resumes"
 * execution at the beginning of cma__execute_thread.
 */
	la	t0, cma__execute_thread /* Starting PC */
	sw	t0, JB_RA*4(C_SP)	/* Store PC in ra in context area */

/*
 * The child thread is now ready to run, so inform the scheduler
 * NOTE:  The parent may or may not be preempted by the child here.
 */
	move	a0, a2
	CALL	cma__start_thread

/*
 * We can now be rescheduled
 */

#ifdef _CMA_TRACE_KERNEL_
	li	a0, __LINE__
	la	a1, filename
	jal	cma__exit_kern_record
#else
	CALL	cma__undefer
	sw	zero, cma__g_kernel_critical
#endif
	lw	s0, FSIZE-FOFFS-4(sp)	/* Restore saved register */

	RETURN
END(cma__create_thread)
#endif		/* if !_CMA_THREAD_IS_VP_ */

#ifndef _CMA_SHLIB_
/*
 * cma__do_call_on_stack
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	 This routine calls the specified routine on the specified stack,
 *	 passing the specified parameter.
 *	 
 * FORMAL PARAMETERS:
 * 
 *	 a0	-  of base of target stack
 *	 a1	-  address of a routine entry point
 *	 a2	-  of parameter to be passed to the routine
 * 
 * IMPLICIT INPUTS:
 * 
 *	 None
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	 None
 *
 * FUNCTION VALUE:
 * 
 *	 Value returned by the supplied routine, if any
 * 
 * SIDE EFFECTS:
 * 
 *	 None
 *
 * Frame contains:
 *	 two saved registers
 *	 no local variables
 *	 one subroutine parameter
 */
#undef	FSIZE
#define	FSIZE 4*4
#undef	FOFFS
#define	FOFFS 1*4

ROUTINE(cma__do_call_on_stack)
	sw	s0, FSIZE-FOFFS-4(sp)	/* Protocol requires saving this reg */
	.mask	0x80010000, -FOFFS	/* M_RA+M_S0 */

/*
 * Switch to specified stack
 */
	move	s0, sp		/* Store the current sp */
#ifdef  _CMA_SHLIB_
/*
 * picie shrinks the stack by 16 bytes
 * after the previous move from sp to s0. We don't want this done.
 * Though, gross, just manually grow it by 16 here to compensate.
 */
        addiu   s0, -16
#endif

	subu	a0, a0, 4	/* Compute new stack, with space for arg */
#ifdef  _CMA_SHLIB_
/*
 * picie shrinks the stack by 16 bytes
 * after the next move from a0 to sp. We don't want this done.
 * Though, gross, just manually grow it by 16 here to compensate.
 */
        addiu   a0, -16
#endif
	move	sp, a0		/* Set to new stack */

/*
 * Call the specified routine
 */
	move	a0, a2		/* Set up parameter list */
	CALL	a1		/* Call users routine */
#ifdef  _CMA_SHLIB_
/*
 * picie tries to restore $gp from the stack at this point.
 * Unfortunately it is using a different stack now than the one $gp
 * was store on since we switched them. We need to figure out a
 * work-around for this.
 */
#endif

/*
 * Switch back to old stack, pop the frame, and return
 */
#ifdef  _CMA_SHLIB_
/*
 * picie shrinks the stack by 16 bytes
 * after the next move from s0 to sp. We don't want this done.
 * Though, gross, just manually grow it by 16 here to compensate.
 */
        addiu   s0, -16
#endif
	move	sp, s0			/* Restore the sp */
	lw	s0, FSIZE-FOFFS-4(sp)	/* Restore saved register */
	RETURN
END(cma__do_call_on_stack)
#endif

/*
 * cma__execute_thread
 *
 * FUNCTIONAL DESCRIPTION:
 *
 * 	This is the routine which occupies the frame at the base of the stack.
 *	It calls the user's "start routine", it holds the catch-all handler
 *	(set up in cma__create_thread), and it calls the thread cleanup
 *	routines after the "start routine" returns.
 *
 *	In the event of an exception, the catch-all handler may unwind back to
 *	this routine, write the reason for termination in -4(fp) [the exit
 *	status], and resume this routine to run down the thread.
 *
 * FORMAL PARAMETERS:
 * 
 *	a0, TCB(sp)	   - the address of the thread tcb
 * 
 * IMPLICIT INPUTS:
 * 
 *	 None
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	 None
 *
 * FUNCTION VALUE:
 * 
 *	 None (This function never returns)
 * 
 * SIDE EFFECTS:
 *
 *	 Note:  This routine is never explicitly called:  A call frame for it
 *		and a thread context buffer pointing to it are built by
 *		cma__create_thread.  When the thread is scheduled, it begins
 *		executing at the first instruction.  The parameter is
 *		"passed" to this routine in the stack frame, placed there by
 *		cma__create_thread, not in a0
 *
 *	 NOTE:  This routine DOES NOT RETURN: the call to cma__thread_base
 *		results in thread termination (eventually).
 *
 * Frame contains:
 *	 one saved register
 *	 one local variable (EXIT_STAT)
 *	 three subroutine parameters
 */
#undef	FSIZE
#define	FSIZE 12*4	/* Room for 10 args */
#undef	FOFFS
#define	FOFFS 2*4

#define TCB	    (FSIZE+0*4)

#define EXIT_STAT   (FSIZE-1*4)

ROUTINE(cma__execute_thread)
	.mask	M_RA, -FOFFS		/* Just save ra */

	lw	a0, TCB(sp)		/* Load TCB pointer */
	CALL	cma__thread_base	/* Call thread base routine */
	break	2			/* Do not continue execution! */
END(cma__execute_thread)

#if !_CMA_THREAD_IS_VP_
/*
 * cma__restore_thread_ctx
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	 This routine restores all context associated with a thread which is
 *	 necessary for scheduling it on the current processor.
 *
 *	 This routine causes execution to resume in the target thread, and
 *	 therefore does not return to the calling routine.
 *	 
 * FORMAL PARAMETERS:
 * 
 *	 a0	-  address of the buffer which holds the static context
 * 
 * IMPLICIT INPUTS:
 * 
 *	 None
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	 The main CPU general registers ($1-$31), the HI and LO registers, 
 *	 the PC (sort of), the CP1 (FPA) registers ($f0-$f31), and the FPA
 *	 control/status register (FCR31).
 *
 * FUNCTION VALUE:
 * 
 *	 None  (This routine does not return to the caller.)
 * 
 * SIDE EFFECTS:
 * 
 *	 NOTE:  This routine DOES NOT RETURN in the typical sense.
 */
LEAF(cma__restore_thread_ctx)

/*
 * Now, transfer to the new thread (ie, switch to its stack).
 */
	lw	sp, 0(a0)		/* Load the new sp from the TCB */

/*
 * Restore the values in all the registers that we are required to
 * preserve across subroutine calls
 */
	lw	s0, JB_S0*4(sp)
	lw	s1, JB_S1*4(sp)
	lw	s2, JB_S2*4(sp)
	lw	s3, JB_S3*4(sp)
	lw	s4, JB_S4*4(sp)
	lw	s5, JB_S5*4(sp)
	lw	s6, JB_S6*4(sp)
	lw	s7, JB_S7*4(sp)
#ifndef fp
	lw	s8, JB_S8*4(sp)
#else
	lw	fp, JB_FP*4(sp)
#endif


/*
 * We shouldn't have to synchronize with the FPC.  That should be done
 * either by the save-context routine or by the terminate thread
 * routine.
 */
	l.d	$f20, JB_F20*4(sp)
	l.d	$f22, JB_F22*4(sp)
	l.d	$f24, JB_F24*4(sp)
	l.d	$f26, JB_F26*4(sp)
	l.d	$f28, JB_F28*4(sp)
	l.d	$f30, JB_F30*4(sp)

/*
 * Load the return address so we know where to go back to
 */
	lw	ra, JB_RA*4(sp)

/*
 * Remove context area from the stack
 */
	move	a0, sp
	addu	a0, _JBLEN*4
	move	sp, a0

/*
 * Now simply return.  There is no frame to pop, the return address is
 * already in "ra"
 */
#ifdef  _CMA_SHLIB_
/*
 * Whoops, picie is assuing $r25 (i.e. t9) is always pointing to the
 * routine being executed. Just set it up before jumping.
 */
        move    t9, ra
#endif
	j	ra

END(cma__restore_thread_ctx)

/*
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine restores the main thread context, and calls
 *	cma_abort_process.
 *
 * FORMAL PARAMETERS:
 * 
 *	None:  this routine is never actually called
 * 
 * IMPLICIT INPUTS:
 * 
 *	None
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	None
 *
 * FUNCTION VALUE:
 * 
 *	None
 * 
 * SIDE EFFECTS:
 * 
 *	None
 */
	.ent	cma__transfer_main_abort
EXPORT(cma__transfer_main_abort)

#ifdef  _CMA_SHLIB_
        subu    sp, 4
#endif

/*
 * Switch to the initial thread's stack
 */
	lw	t1, cma__g_def_ctx_buf
#ifdef  _CMA_SHLIB_
        lw      t0, 0(t1)
        move    sp, t0
#else
	lw	sp, 0(t1)		/* Load the new sp from global */
#endif

/*
 * Set up call to abort process
 */
	lw	a0, cma__g_abort_signal 
	CALL	cma__abort_process

/*
 * We should never return from the abort
 */	
	break	2			/* Do not continue execution! */
#ifdef  _CMA_SHLIB_
        addu    sp, 4
#endif
END(cma__transfer_main_abort)

/*
 * cma__transfer_thread_ctx 2
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	 This routine stores the context of one thread and restores the context
 *	 of another, thus performing an actual context switch.  This routine is
 *	 used only for synchronous context switches (but the context which it
 *	 restores may have been synchronously or asynchronously saved).
 *
 *	 The newly restored thread resumes processing with the instruction
 *	 following it's last call to "cma__save_thread_ctx" (really
 * 	 cma__create_thread) or cma__transfer_thread_ctx, ie, this routine
 *	 returns directly to that instruction.
 *
 * FORMAL PARAMETERS:
 * 
 *	 a0  -  address of current thread's static context buffer
 *	 a1  -  address of new thread's static context buffer
 * 
 * IMPLICIT INPUTS:
 * 
 *	 The main CPU general registers ($1-ra), the HI and LO registers, 
 *	 the PC (sort of), the CP1 (FPA) registers ($f0-$f31), and the FPA
 *	 control/status register (FCR31).
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	 The main CPU general registers ($1-ra), the HI and LO registers, 
 *	 the PC (sort of), the CP1 (FPA) registers ($f0-$f31), and the FPA
 *	 control/status register (FCR31).
 *
 * FUNCTION VALUE:
 * 
 *	 None
 * 
 * SIDE EFFECTS:
 * 
 *	 NOTE:  When this routine returns, it is in a different thread...
 */
LEAF(cma__transfer_thread_ctx)

/*
 * Make room on the stack and create a couple of scratch registers
 */
	subu	sp, _JBLEN*4	/* Make room on the stack for the context */
#ifdef  _CMA_SHLIB_
/*
 * The following harmless instruction will remove the extra 16 bytes
 * the stack was grown by above.
 */
        addu    sp,0
#endif

/*
 * Store the values in all the registers that we are required to
 * preserve across subroutine calls
 */
	sw	s0, JB_S0*4(sp)
	sw	s1, JB_S1*4(sp)
	sw	s2, JB_S2*4(sp)
	sw	s3, JB_S3*4(sp)
	sw	s4, JB_S4*4(sp)
	sw	s5, JB_S5*4(sp)
	sw	s6, JB_S6*4(sp)
	sw	s7, JB_S7*4(sp)
#ifndef fp
	sw	s8, JB_S8*4(sp)
#else
	sw	fp, JB_FP*4(sp)
#endif

/*
 * Set up to save floating registers.    This code previously 
 * did sp-relative addressing to store the registers in the context block.
 * Our pal picie was mucking (incorrectly) with the offsets into the
 * context block, so we use another register (t0) to do the accessing.
 */
	addu	t0, sp, 0

#ifdef  _CMA_SHLIB_
/*
 * picie will try to add 16 to t0 because it recognizes it has 
 * been copied from sp, which it has tinkered with.
 * We counter this brilliant move by adding the 16 back in.
 */
        addiu   t0, -16
#endif

/*
 * I assume we don't need to synchronize with the FPC since this is a
 * normal subroutine call
 */
	s.d	$f20, JB_F20*4(t0)
	s.d	$f22, JB_F22*4(t0)
	s.d	$f24, JB_F24*4(t0)
	s.d	$f26, JB_F26*4(t0)
	s.d	$f28, JB_F28*4(t0)
	s.d	$f30, JB_F30*4(t0)

/*
 * Save the restart PC
 */
	sw	ra, JB_RA*4(sp)

/*
 * Store the sp in the TCB 
 */
#ifdef  _CMA_SHLIB_
        move    s0, sp
/*
 * picie shrinks the stack by 16 bytes
 * after the previous move from sp to s0. We don't want this done.
 * Though, gross, just manually grow it by 16 here to compensate.
 */
        addiu   s0, -16
	sw	s0, 0(a0)
#else
	sw	sp, 0(a0)
#endif

/*
 * And go restore the new thread's context
 */
	move	a0, a1		/* Context to restore */
	j	cma__restore_thread_ctx

/*
 * And, cma_restore_thread_ctx takes care of the rest  (There is no frame
 * to pop, and the new return address is stored in the context buffer.)
 */
END(cma__transfer_thread_ctx)

/*
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine is responsible for initiating asynchronous alert
 *	delivery.  It calls a routine (written in C) to actually raise the
 *	alert exception, to ensure that the raise is always consistent with
 *	the CMA exception package implementation on the platform (the extra
 *	call is probably not a serious performance issue in the context of an
 *	alert which will usually terminate the thread anyway).
 *
 *	This routine is never actually CALLed, rather it is "jumped" into by
 *	placing the entry point pc, "cma__do_async_alert", into the
 *	resume pc in a signal context block.
 *
 *	NOTE:  Therefore, this routine runs in the same frame as the routine
 *	       which was interrupted by the signal, but not at signal level.
 *
 * FORMAL PARAMETERS:
 * 
 *	None:  this routine is never actually called
 * 
 * IMPLICIT INPUTS:
 * 
 *	None
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	None
 *
 * FUNCTION VALUE:
 * 
 *	None
 * 
 * SIDE EFFECTS:
 * 
 *	None
 */
#ifdef  _CMA_SHLIB_
/*
 * This routine won't run through picie unless it actually allocates
 * some stack space.
 */
#undef  FSIZE
#define FSIZE   4
#undef  FOFFS
#define FOFFS   0
ROUTINE(cma__do_async_alert)
#else
	.ent	cma__do_async_alert
EXPORT(cma__do_async_alert)
#endif
#ifdef _CMA_TRACE_KERNEL_
	li	a0, __LINE__
	la	a1, filename
	jal	cma__exit_kern_record
#else
	CALL	cma__undefer
	sw	zero, cma__g_kernel_critical
#endif

	CALL	cma__async_delivery	/* Deliver the alert */

/*
 * This point should never be reached, since the CMA exception package doesn't
 * support continuable exceptions.
 */
	la	a0, async_alert_bug
	CALL	cma__bugcheck		/* Handle the error */
#ifdef  _CMA_SHLIB_
        RETURN
#endif
END(cma__do_async_alert)	
#endif		/* if !_CMA_THREAD_IS_VP_ */

/*
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine is responsible for delivering an async call from the
 *	virtual processor interface cma__vp_interrupt. It's not actually
 *	called, although the stack has been set up so that it behaves as a
 *	call. For convenience, the registers are set up so that the address
 *	to be called is in s0 (callee-saved register), and the argument to
 *	that routine is in a0 (the first argument register), so this code
 *	doesn't need to move anything around before the call.
 *
 * FORMAL PARAMETERS:
 * 
 *	none
 * 
 * IMPLICIT INPUTS:
 * 
 *	a0	interrupt argument
 *	s0	interrupt handler address
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	None
 *
 * FUNCTION VALUE:
 * 
 *	None
 * 
 * SIDE EFFECTS:
 * 
 *	None
 */
	.ent	cma__do_interrupt
EXPORT(cma__do_interrupt)
#if _CMA_SHLIB_
/*
 * picie complains unless some stack allocation is done before
 * stack deallocation. So lets just fool it and allocate and
 * deallocate an unsed 4 bytes.
 */
	subu	sp, 4
	CALL	s0			/* Deliver the alert */
	lw	ra, 4(sp)		/* Restore the return address */
	lw	s0, 8(sp)		/* Restore involuntary caller's s0 */
	lw	a0, 12(sp)		/* Restore old a0, too */
	addu	sp, 4			/* Restore old sp */
#else
	CALL	s0			/* Deliver the alert */
	lw	ra, 0(sp)		/* Restore the return address */
	lw	s0, 4(sp)		/* Restore involuntary caller's s0 */
	lw	a0, 8(sp)		/* Restore old a0, too */
	addu	sp, 12			/* Restore old sp */
#endif
	j	ra			/* Return to involuntary caller */
END(cma__do_interrupt)	

#if !_CMA_THREAD_IS_VP_
/*
 * cma__force_dispatch 2
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine is responsible for initiating an asynchronous context
 *	switch.
 *
 *	First, we save all registers which might be altered by subroutine 
 *	calls in a context area on the stack.  Then, we call a routine
 *	(written in C) to retrieve from the TCB the values which the PC and PS
 *	(floating point coprocessor status register) had at the point of
 *	interruption.  These values are written into the appropriate places in
 *	the context area on the stack.  Then, we yield the processor to
 *	another thread.  This results in a full context save before the new
 *	thread is scheduled.  When we are once again scheduled, a full context
 *	restore is performed in the process.  When control returns to this
 *	routine, we restore the volatile registers which we saved, and return.
 *
 *	This routine is never actually called, rather it is "jumped" into by
 *	placing the entry point PC, "cma__call_forced_dispatch", into the
 *	resume PC in a signal context block.
 *
 *	NOTE:  Therefore, this routine runs in the same frame as the routine
 *		which was interrupted by the signal.
 *
 *	 	 
 * FORMAL PARAMETERS:
 * 
 *	 None:  this routine is never actually called
 * 
 * IMPLICIT INPUTS:
 * 
 *	 None
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	 None
 *
 * FUNCTION VALUE:
 * 
 *	 None:  this routine does not return, execution simply resumes at the
 *		point where it was interrupted by the timeslice.
 * 
 * SIDE EFFECTS:
 * 
 *	 None
 *
 *	 NOTE:  This routine is not actually called (it is jumped into)
 */
	.ent	cma__force_dispatch
EXPORT(cma__force_dispatch)

/*
 * I need to save the $at, so the assembler can't be using it until I 
 * can get its current value saved.
 *
 * NOTE: This directive should precede the first instruction in this
 *	routine!
 */
	.set	noat

/*
 * Make room on the stack and create a couple of scratch registers
 */
	subu	sp, _JBLEN*4	/* Make room on the stack for the context */

/*
 * Now save the value in AT so we can let the assembler use it again
 * Also zero the $0 slot in the JB, just in case someone looks.
 */
	sw	zero, JB_ZERO*4(sp)
	sw	$at, JB_AT*4(sp)
	.set	at

/*
 * Save the main CPU general registers.
 */
	sw	v0, JB_V0*4(sp)		/* Save v0 & v1 */
	sw	v1, JB_V1*4(sp)		/* Save v0 & v1 */
	sw	a0, JB_A0*4(sp)		/* Save a0 & a1 */
	sw	a1, JB_A1*4(sp)		/* Save a0 & a1 */
	sw	a2, JB_A2*4(sp)		/* Save a2 & a3 */
	sw	a3, JB_A3*4(sp)		/* Save a2 & a3 */
	sw	t0, JB_T0*4(sp)		/* Save t0 & t1 */
	sw	t1, JB_T1*4(sp)		/* Save t0 & t1 */
	sw	t2, JB_T2*4(sp)		/* Save t2 & t3 */
	sw	t3, JB_T3*4(sp)		/* Save t2 & t3 */
	sw	t4, JB_T4*4(sp)		/* Save t4 & t5 */
	sw	t5, JB_T5*4(sp)		/* Save t4 & t5 */
	sw	t6, JB_T6*4(sp)		/* Save t6 & t7 */
	sw	t7, JB_T7*4(sp)		/* Save t6 & t7 */
	sw	t8, JB_T8*4(sp)		/* Save t8 & t9 */
	sw	s0, JB_S0*4(sp)		/* Save s0 & s1 */
	sw	s1, JB_S1*4(sp)		/* Save s0 & s1 */
	sw	s2, JB_S2*4(sp)		/* Save s2 & s3 */
	sw	s3, JB_S3*4(sp)		/* Save s2 & s3 */
	sw	s4, JB_S4*4(sp)		/* Save s4 & s5 */
	sw	s5, JB_S5*4(sp)		/* Save s4 & s5 */
	sw	s6, JB_S6*4(sp)		/* Save s6 & s7 */
	sw	s7, JB_S7*4(sp)		/* Save s6 & s7 */
#ifndef _CMA_SHLIB_
	sw	t9, JB_T9*4(sp)		/* Save t8 & t9 */
	sw	gp, JB_GP*4(sp)		/* Save gp */
#endif
#ifndef fp
	sw	s8, JB_S8*4(sp)		/* Save s8 & ra */
	sw	ra, JB_RA*4(sp)		/* Save s8 & ra */
#else
	sw	fp, JB_FP*4(sp)		/* Save fp & ra */
	sw	ra, JB_RA*4(sp)		/* Save fp & ra */
#endif

/*
 * Don't bother saving k0 & k1 - let the Ultrix kernel fend for itself
 */

/*
 * Set up to save floating registers.    This code previously 
 * did sp-relative addressing to store the registers in the context block.
 * Our pal picie was mucking (incorrectly) with the offsets into the
 * context block, so we use another register (s0) to do the accessing.
 */
	addu	s0, sp, 0

#ifdef  _CMA_SHLIB_
/*
 * picie will try to add 16 to s0 because it recognizes it has 
 * been copied from sp, which it has tinkered with.
 * We counter this brilliant move by adding the 16 back in.
 */
        addiu   s0, -16
#endif

/*
 * We have to synchronize with the FPA before saving any of its registers
 * otherwise we risk interrupting a floating point instruction and 
 * incurring an exception.  We do this by requesting the FPA 
 * control/status register value.  Don't bother saving it, since it will
 * be overwritten by the call to cma__get_async_info
 */
	cfc1	zero, fpc_csr	/* Request the FPA control/status reg */

/*
 * Now we can save the FP registers without worrying about screwing up a 
 * concurrently executing floating instruction (and getting an exception)
 */
	s.d	$f0, JB_F0*4(s0)
	s.d	$f2, JB_F2*4(s0)
	s.d	$f4, JB_F4*4(s0)
	s.d	$f6, JB_F6*4(s0)
	s.d	$f8, JB_F8*4(s0)
	s.d	$f10, JB_F10*4(s0)
	s.d	$f12, JB_F12*4(s0)
	s.d	$f14, JB_F14*4(s0)
	s.d	$f16, JB_F16*4(s0)
	s.d	$f18, JB_F18*4(s0)
	s.d	$f20, JB_F20*4(s0)
	s.d	$f22, JB_F22*4(s0)
	s.d	$f24, JB_F24*4(s0)
	s.d	$f26, JB_F26*4(s0)
	s.d	$f28, JB_F28*4(s0)
	s.d	$f30, JB_F30*4(s0)

/*
 * Save the integer multiply/divide registers.  Observe that we saved
 * mdlo in a totally bogus location.  That's because picie trashes
 * the place we would normally store it.  We catch up with this
 * later.
 */
	mflo	t0			/* Get integer mult/div lower word */
	mfhi	t1			/* Get integer mult/div upper word */
	sw	t0, JB_ONSIGSTK*4(s0)	/* Save integer mult/div register values */
	sw	t1, SC_MDLO*4+4(s0)	/* Save integer mult/div register values */

/*
 * Set up the value of the sp saved in the context buffer so that when
 * the context is restored, the buffer will disappear from the stack.
 */
	addu	t0, sp, _JBLEN*4	/* "Pop" the buffer from the stack */
	sw	t0, JB_SP*4(sp)		/* Save "popped" sp value */

/*
 * Now that the immediate context is safely on the stack, we can safely
 * call a subroutine to get the rest of the context which was saved for
 * us by the signal handler
 *
 * Since cma__get_async_info takes no arguments, it should require no
 * staging area in our frame for them, so don't create one...
 */

	CALL	cma__get_async_info	/* Put address of asynch ctx block in v0 */

/*
 * Move the information from the async context block in the tcb to the
 * signal context block we're constructing on the stack
 */
	lw	t0, AC_SIG_MASK*4(v0)	/* Get signal mask */
	lw	t1, AC_SIG_MASK*4+4(v0)	/* Get restart PC */
	sw	t0, JB_SIGMASK*4(s0)
	sw	t1, JB_SIGMASK*4+4(s0)
	lw	t0, AC_USED_FPC*4(v0)	/* Get flag for used FPC */
	sw	t0, (JB_FREGS-1)*4(s0)
	lw	t0, AC_FPC_CSR*4(v0)	/* Get the FPC CSR & EIR */
	lw	t1, AC_FPC_CSR*4+4(v0)	/* Get the FPC CSR & EIR */
	sw	t0, JB_FPC_CSR*4(s0)
	sw	t1, JB_FPC_CSR*4+4(s0)
	lw	t0, AC_CP0_CAUSE*4(v0)	/* Get the CP0 cause & bad VA registers */
	lw	t1, AC_CP0_CAUSE*4+4(v0)/* Get the CP0 cause & bad VA registers */
	sw	t0, (JB_FPC_CSR+2)*4(s0)
	sw	t1, (JB_FPC_CSR+2)*4+4(s0)
	lw	t0, AC_CPU_BAD_PA*4(v0)	/* Get the CPU board bad PA register */
	sw	t0, (JB_FPC_CSR+4)*4(s0)
#ifdef _CMA_SHLIB_
        lw      t0, AC_T9*4(v0)         /* get gp */
        lw      t1, AC_GP*4(v0)         /* get gp */
	sw	t0, JB_T9*4(s0)		/* Save gp */
	sw	t1, JB_GP*4(s0)		/* Save gp */
#endif

/*
 * Run a different thread for a while
 */
	CALL	cma__yield_processor	/* Perform the dispatch */

/*
 * We previously stored mdlo in the wrong place to avoid a picie
 * pitfall.  Fix things up here, and establish that we are not
 * on the signal stack.
 */
	lw	t3, JB_ONSIGSTK*4(s0)
	sw	t3, SC_MDLO*4(s0)
	sw	zero, JB_ONSIGSTK*4(s0)

/*
 * Restore our context
 */

/*
 * Set up for a system call.  We must call the system in order to
 * completely restore the register set in a safe manner.  The address 
 * of a signal context structure is placed in "a0" and the number of 
 * the system call, "sigreturn", is placed in "v0".  The context will 
 * be popped from the stack as part of the restore (ie the "sp" is
 * restored).  Leave the CMA kernel last thing before restoring
 * context (it makes no sense to call cma__undefer() from here).
 */
	move	a0, sp			/* Arg for syscall: addr of sig ctx buf */
#ifdef  _CMA_SHLIB_
/*
 * picie subtracted 16 extra bytes from our stack pointer at
 * the start of this routine. It then inserts an instruction to add
 * the 16 before storing it. Though, gross, just manually remove the
 * 16 before storing it. 
 */
        addiu   a0, -16
#endif

#ifdef _CMA_TRACE_KERNEL_
	li	a0, __LINE__
	la	a1, filename
	jal	cma__unset_kern_record
#else
	sw	zero, cma__g_kernel_critical
#endif

/*
 * Load which syscall to make.  This is done right before
 * the syscall so that picie doesn't blow away v0.  For some 
 * reason picie overwrites v0 when loading the address for
 * cma__g_kernel_critical. 
 */
	li	v0, SYS_sigreturn	/* Which syscall: Return from signal */
	syscall

#ifdef _CMA_SHLIB_
/*
 * The syscall above causes execution to resume elsewhere.  The following code is an attempt to appease picie.
 */
	addu	sp, _JBLEN*4
	j	ra
#endif

END(cma__force_dispatch)
#endif

/*
 * cma__fetch_sp
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine returns the current value of sp (the stack pointer).
 *
 * FORMAL PARAMETERS:
 * 
 *	none
 * 
 * IMPLICIT INPUTS:
 * 
 *	none
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	none
 *
 * FUNCTION VALUE:
 * 
 *	the current value of sp
 * 
 * SIDE EFFECTS:
 * 
 *	none
 */
LEAF(cma__fetch_sp)
	move	v0, sp
	j	ra
END(cma__fetch_sp)

/*
 * cma__fetch_gp
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine returns the current value of gp (the global pointer).
 *
 * FORMAL PARAMETERS:
 * 
 *	none
 * 
 * IMPLICIT INPUTS:
 * 
 *	none
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	none
 *
 * FUNCTION VALUE:
 * 
 *	the current value of sp
 * 
 * SIDE EFFECTS:
 * 
 *	none
 */
LEAF(cma__fetch_gp)
	move	v0, gp
	j	ra
END(cma__fetch_gp)

#if _CMA_THREAD_IS_VP_
/*
 * cma__do_break
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine causes a breakpoint. It's used to implement "set visible"
 *	on a VP implementation (not used for uniprocessors). cma__vp_interrupt
 *	is used to cause the target thread to execute this function.
 *
 * FORMAL PARAMETERS:
 * 
 *	none
 * 
 * IMPLICIT INPUTS:
 * 
 *	none
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	none
 *
 * FUNCTION VALUE:
 * 
 *	none
 * 
 * SIDE EFFECTS:
 * 
 *	none
 */
LEAF(cma__do_break)
	break	2
	j	ra
END(cma__do_break)

	.set	noreorder

/*
 * cma__tas
 *
 * FUNCTIONAL DESCRIPTION:
 * 
 *	This routine performs an atomic test-and-set operation on the data
 *	at a specified address.  The value is set (to its own address), and
 *	the previous value is returned.
 *
 * FORMAL PARAMETERS:
 * 
 *	a0	Address of an int on which to perform test-and-set function
 * 
 * IMPLICIT INPUTS:
 * 
 *	none
 * 
 * IMPLICIT OUTPUTS:
 * 
 *	none
 *
 * FUNCTION VALUE:
 * 
 *	v0 == 0 if word was previously clear, non-zero if previously set.
 * 
 * SIDE EFFECTS:
 * 
 *	0(a0) is always set to the (initial) value of a0 (its own address)
 *	when this returns.
 */
LEAF(cma__tas)
	lw	v0, 0(a0)
	nop
	bnez	v0, 10f
	nop
	.word	tas_op		/* Returns previous 0(a0) in a0 */
10:	j	ra
	move	v0, a0
END(cma__tas)

	.set	reorder

#endif
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_ASSEM.S */
/*  *23   17-APR-1992 08:39:32 KEANE "Fix picie probs in transfer_thread_context" */
/*   18A1 16-APR-1992 14:53:03 KEANE "Propogate picie fixes to BL9" */
/*  *22   13-APR-1992 16:45:36 KEANE "Fix picie related problems" */
/*  *21   13-MAR-1992 14:12:23 BUTENHOF "Eliminate excess arguments to thread_base" */
/*  *20   13-MAR-1992 10:38:53 KEANE "Exclude call_on_stack if not _CMA_SHLIB_" */
/*  *19   13-MAR-1992 10:34:32 KEANE "Fix picie problem in cma__force_dispatch" */
/*  *18   23-JAN-1992 14:22:26 BUTENHOF "Integrate shared library support" */
/*  *17    5-NOV-1991 14:58:09 BUTENHOF "[-.mips]" */
/*  *16   29-OCT-1991 12:11:04 BUTENHOF "Remove spurious declaration" */
/*  *15   14-OCT-1991 13:34:06 BUTENHOF "Fix TIN compilation" */
/*   12A1  3-SEP-1991 16:57:00 BUTENHOF "Fix BL7 bug" */
/*  *14    3-SEP-1991 10:28:48 BUTENHOF "More changes for OSF shareable library" */
/*  *13   22-AUG-1991 14:10:26 BUTENHOF "Fix do_call_on_stack code for Tin ld" */
/*  *12   11-JUN-1991 16:33:22 BUTENHOF "Make appropriate trace kernel calls" */
/*  *11   10-JUN-1991 18:08:54 SCALES "Add sccs headers for Ultrix" */
/*  *10   16-MAY-1991 14:09:21 BUTENHOF "Restore s0 in cma__create_thread()" */
/*  *9    15-MAY-1991 13:22:07 BUTENHOF "Change TAS" */
/*  *8    14-MAY-1991 16:59:48 BUTENHOF "Remove traces" */
/*  *7    14-MAY-1991 13:57:29 BUTENHOF "Change test-and-set" */
/*  *6     3-MAY-1991 11:26:54 BUTENHOF "Change test-and-set" */
/*  *5    12-APR-1991 23:33:47 BUTENHOF "OSF/1 Mach thread support" */
/*  *4     8-APR-1991 20:33:20 CURTIN "added cma__transfer_main_abort" */
/*  *3     3-APR-1991 15:59:32 BUTENHOF "Modify to support generic OSF/1 build" */
/*  *2    12-DEC-1990 20:33:29 BUTENHOF "Fix assem include, and clean up CMS history" */
/*  *1    12-DEC-1990 18:57:46 BUTENHOF "MIPS specific assem file" */
/*  DEC/CMS REPLACEMENT HISTORY, Element CMA_ASSEM.S */
