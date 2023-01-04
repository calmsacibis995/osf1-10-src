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
	.asciiz "@(#)$RCSfile: locore.s,v $ $Revision: 1.2.33.6 $ (DEC) $Date: 1993/10/27 22:09:33 $"
	.text

/*
 * This file contains asm machine and architecture dependant routines.
 *
 */

#ifdef notdef
#include <mach_emulation.h>
#include <mach_kdb.h>
#endif
#include <rt_preempt.h>
#include <rt_sched.h>

#include <machine/machparam.h>
#include <machine/vmparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <machine/psl.h>
#include <machine/trap.h>
#include <mach/machine/vm_param.h>
#include <machine/pmap.h>
#include <machine/thread.h>
#include <machine/rpb.h>
#include <machine/pcb.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <sys/reboot.h>
#include <sys/table.h>
#include <sys/proc.h>
#include <mach/kern_return.h>
#include <assym.s>


#define TR_MASK M_T0|M_T1|M_T2|M_T3|M_T4|M_T5|M_T6|\
		M_T7|M_T8|M_T9|M_T10|M_T11|M_T12
#define SR_MASK M_S0|M_S1|M_S2|M_S3|M_S4|M_S5|M_S6
#define AR_MASK M_A0|M_A1|M_A2|M_A3|M_A4|M_A5
#define EXCMASK TR_MASK|SR_MASK|AR_MASK|M_V0|M_RA|M_AT|M_GP|M_SP


/************************************************************************
 *									*
 *			Start	OSFpal flavor				*
 *									*
 ************************************************************************/

/*
 * Exception Vector routine entry macro.
 *
 * Save off enough state to start each of the exception routines
 *
 *	
 */
#define TMPSPACE	8

#define SWAPIPL(x,y)							\
LEAF(x);								\
	ldiq	a0,y;			/* get ipl into r16	*/	\
	call_pal PAL_swpipl;		/* set the ipl		*/	\
	ret	zero,(ra);						\
	END(x)


/*
 * Trap routine entry Macro
 * 
 *	registers a0..a2,gp,pc,ps have been saved on the stack
 *
 * Save off enough state to start each of the exception routines
 */
#define TRAP(routine,trap_code)						\
	.align	3;							\
	.globl	_X/**/routine;						\
	.ent	_X/**/routine,0;					\
	.frame	sp,EF_SIZE,zero;					\
	.mask	EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8));			\
_X/**/routine:;								\
	lda	sp,-EF_SSIZE(sp);	/* allocate space software */	\
	stq	ra,EF_RA*8(sp);		/* save return register	   */	\
	stq	v0,EF_V0*8(sp);		/* return value 0	   */	\
	stq	a3,EF_A3*8(sp);		/* argument register 3	   */	\
	stq	a4,EF_A4*8(sp);		/* argument register 4	   */	\
	stq	a5,EF_A5*8(sp);		/* argument register 5	   */	\
	.set	noat;							\
	stq	AT,EF_AT*8(sp);		/* save assembler temp     */	\
	.set	at;							\
	bsr	ra,reg_save;		/* save all registers	   */	\
	ldiq	a3,trap_code;		/* code in a3		   */	\
	bis	sp,zero,a4;		/* set up the frame pointer */	\
	jsr	ra,trap;		/* trap(a0,a1,a2,trap_code,ef) */\
	br	zero,exception_exit;				       	\
	END(routine)


/*
 * System Entry Instruction Fault
 */
	.align	3
	.globl	_XentIF
	.ent	_XentIF,0
	.frame	sp,EF_SIZE,zero
	.mask	EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
_XentIF:
	lda	sp,-EF_SSIZE(sp)	# allocate space software
	stq	ra,EF_RA*8(sp)		# save return register
	stq	v0,EF_V0*8(sp)		# return value 0
	stq	a3,EF_A3*8(sp)		# argument register 3
	stq	a4,EF_A4*8(sp)		# argument register 4
	stq	a5,EF_A5*8(sp)		# argument register 5
	.set	noat
	stq	AT,EF_AT*8(sp)		# save assembler temp
	.set	at
	bsr	ra,reg_save		# save all registers
	ldiq	a3,T_IFAULT		# code in a3
	cmpeq	a0,T_IFAULT_BPT,t0	# is this a breakpoint?
	blbs	t0,2f
1:	bis	sp,zero,a4		# set up the frame pointer
	jsr	ra,trap			# trap(a0,a1,a2,trap_code,ef)
	br	zero,exception_exit
2:	ldq	a4,EF_PS*8(sp)		# kernel mode?
	and	a4,PSL_CURMOD,t0
	bne	t0,1b
	bis	sp,zero,a4		# set up the frame pointer
	jsr	ra,kdebug_if_trap	# kdebug_if_trap(a0,a1,a2,trap_code,ef)
	br	zero,exception_exit
	END(_XentIF)


/*
 * Kdebug's memory fault handler
 */
	.align	3
	.globl	_XentKdebugMM
	.ent	_XentKdebugMM,0
	.frame	sp,EF_SIZE,zero
	.mask	EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
_XentKdebugMM:
	lda	sp,-EF_SSIZE(sp)	# allocate stack frame
	stq	ra,EF_RA*8(sp)		# save return register
	stq	v0,EF_V0*8(sp)		# return value 0
	stq	a3,EF_A3*8(sp)		# argument register 3
	stq	a4,EF_A4*8(sp)		# argument register 4
	stq	a5,EF_A5*8(sp)		# argument register 5
	.set	noat
	stq	AT,EF_AT*8(sp)		# save assembler temp
	.set	at
	bsr	ra,reg_save		# save all registers
	ldiq	a3,T_MMANG		# assign trap code parameter
	bis	sp,zero,a4		# assign frame pointer parameter
	jsr	ra,kdebug_mm_trap	# kdebug_mm_trap(a0,a1,a2,trap_code,ef)
	br	zero,exception_exit
	END(_XentKdebugMM)


/*
 * installs an exception handler, called with the first argument being
 * the address of the exception routine, and the second argument being
 * the index for the exception.
 */

LEAF(kdebug_install_handler)
	call_pal PAL_wrent
	ret     zero,(ra)
	END(kdebug_install_handler)


/*
 * alignment handler MACRO's, convert to regular leaf routines for uPAL
 */
#define SCBCMVEC(routine)						\
LEAF(routine);								\
	subq	sp, TMPSPACE, sp;					\

#define SCBCMEND(routine)						\
	addq	sp, TMPSPACE, sp;					\
	br	zero,_Xunaexit;						\
END(routine)

/*
 * Exception Exit
 *
 *	All exception and traps exit through here.  This code is responsible
 *	for emulating reschedule AST's and actual cleanup and exit.
 *	On entry s0 contains 0 for interrupts and 1 for traps.  This is used
 *	to determine if the user stack pointer needs to be updated.
 *
 *	Before restoring registers and returning, checks are made for pending
 *	lwc requests, pending user mode ast's, and possibly kernel mode ast's.
 *	The logic expressed in C is as follows:
 *
 *	#if RT_PREEMPT
 *		while (1) {
 *			if (exception_frame_ps & PSL_CURMOD) {
 *				/* here if returning to user mode * /
 *				if (slock_count[0])
 *					preemption_fixup();
 *				if (lwc_interrupt_data) {
 *					lwc_schedule();
 *					continue;
 *				}
 *				if (need_ast) {
 *					trap(junk1, junk2, junk3, T_AST, efp);
 *					continue;
 *				}
 *			} else {
 *				/* here if returning to kernel mode * /
 *				if (rt_preempt_enabled &&
 *				    need_ast &&
 *				    ast_mode != 0 &&
 *				    !(exception_frame_ps & PSL_IPL)) {
 *					splextreme();
 *					trap(junk1, junk2, junk3, T_AST, efp);
 *					continue;
 *				}
 *			}
 *			break;
 *		}
 *		<restore regs and return from exception or interrupt>
 *	#else
 *		while (exception_frame_ps & PSL_CURMOD) {
 *			/* here if returning to user mode * /
 *			if (lwc_interrupt_data) {
 *				lwc_schedule();
 *				continue;
 *			}
 *			if (need_ast) {
 *				trap(junk1, junk2, junk3, T_AST, efp);
 *				continue;
 *			}
 *			break;
 *		}
 *		<restore regs and return from exception or interrupt>
 *	#endif RT_PREEMPT
 */
	.globl	exception_exit
	.ent	exception_exit

exception_exit:
	ldq	a0,EF_PS*8(sp)
	and	a0,PSL_CURMOD,a1	# next mode
#if	RT_PREEMPT
	beq	a1,3f			# returning to kernel, check for KAST
	ldl	a0,slock_count		# if (slock_count[0])
	bne	a0,do_preemption_fixup	# 	preemption_fixup()
#else
	beq	a1,4f			# returning to kernel, ignore ast
#endif
1:
	ldq	a0,kernel_async_trap		# if (lwc_interrupt_data)
	bne	a0,exit_async_test

4:	ldq	v0,EF_V0*8(sp)		# restore value registers
	ldq	a3,EF_A3*8(sp)		# argument register 3
	ldq	a4,EF_A4*8(sp)		# argument register 4
	ldq	a5,EF_A5*8(sp)		# argument register 5

	.set	noat
5:	ldq	AT,EF_AT*8(sp)		# restore assembler temp
	.set	at
 	bsr	ra,reg_restore
	ldq	ra,EF_RA*8(sp)		# restore return addr reg
	lda	sp,EF_SSIZE(sp)		# unwind software portion of stack
	call_pal PAL_rti		# return from trap

#if RT_PREEMPT
	/*
	 * call preemption_fixup() to correct lock count
	 */
do_preemption_fixup:
	jsr	ra,preemption_fixup	# preemption_fixup()
	br	zero,1b
#endif

	/*
	 * call trap() to process ast
	 */

exit_async_test:
	ldl	a0,kernel_async_trap	# if (lwc_interrupt_data)
	beq	a0, exit_ast_handling

exit_lwc_handling:
	jsr	ra,lwc_schedule		#	lwc_schedule();
	br	zero,exception_exit

exit_ast_handling:
	ldiq	a3,T_AST		# code in a3; a0-a2 unused
	bis	sp,zero,a4		# pass exception frame
	jsr	ra,trap			# trap(a0,a1,a2,trap_code,ef);
	br	zero,exception_exit


#if	RT_PREEMPT
3:	ldl	a3,rt_preempt_enabled	# if (rt_preempt_enabled &&
	ldl	a2,kernel_async_trap+4	#     need_ast &&
	ldl	a1,ast_mode		#     ast_mode == KERNEL_AST
	cmoveq	a2,0,a3			#    )
	cmoveq	a1,0,a3			# 
	beq	a3,4b			# branch if no Kernel Mode AST

	and	a0,PSL_IPL		# extract return IPL
	bne	a0,4b			# if not IPL 0, no AST allowed
	/*
	 * Kernel Mode AST
	 */
	br	zero,exit_ast_handling	# go do AST
#endif
	.end 	exception_exit

/*
 * System Entry - Interrupts
 *
 *	All interrupts including machine checks come through this vector.
 *	This routine is responsible for saving machine state and little
 *	else.  The decode and dispatch is system dependent and is called
 *	through the processor switch.
 *
 *	Registers t7 has been saved on the stack
 *
 *	Entry is with s0(a0)  0 == IP interrupts
 *			      1 == clock
 *			      2 == machine check
 *			      3 == device interrupts
 *			      4 == performance monitor interrupts
 *			      5 == passive release
 *	NOTE.... value of 6 is currently used as a place holder for clock
 *		 until it is enabled!
 */
	.align	3
	.globl  clock_enable
	.globl	_XentInt
	.ent	_XentInt
	.frame	sp,EF_SIZE,zero
	.mask	EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
_XentInt:
	lda	sp,-EF_SSIZE(sp)	# allocate the stack frame
	stq	ra,EF_RA*8(sp)		# save return register
	stq	v0,EF_V0*8(sp)		# return value 0
	stq	a3,EF_A3*8(sp)		# argument register 3
	stq	a4,EF_A4*8(sp)		# argument register 4
	stq	a5,EF_A5*8(sp)		# argument register 5
	.set	noat
	stq	AT,EF_AT*8(sp)		# save assembler temp
	.set	at
	bsr	ra,reg_save		# save all registers
	lda	s0,atintr_level
	ldl	s1,0(s0)
	addl	s1,1,s1
	stl	s1,0(s0)	

/*
 * Clock interrupt -- Note that the instruction to test for the clock interrupt
 *                    below actually checks for value "6" undefined instead.  
 *		      This acts as a place holder until the startrt() is called.
 *		      The startrt() routine will modify the instruction at 
 *		      the label clock_enable to be a "cmpeq	a0,1,t0 ",
 *		      and thus allowing clock interrupts to be taken.	
 */
clock_enable:
	cmpeq	a0,6,t0			 
	blbc	t0,1f
/*
 * Hardclock interrupt
 */

	ldq	a0,EF_PC*8(sp)
	ldq	a1,EF_PS*8(sp)
	jsr	ra,hardclock		# hardclock(pc,ps)
	ldl	s1,0(s0)
	subl	s1,1,s1
	stl	s1,0(s0)	
	br	zero,exception_exit

1:
	cmpeq	a0,3,t0			# is this a device interrupt?
	blbc	t0,2f
/*
 * Device interrupts 
 */
	beq	a1,3f			# passive release, dismiss
	mb
	lda	t0,system_intr_cnts_type
	ldq	t1,INTR_TYPE_DEVICE*8(t0)
	addq	t1,1,t1
	stq	t1,INTR_TYPE_DEVICE*8(t0)	
	lda	t0,_scb			# get the address of the 
	addq	t0,a1,t0		# index to scbentry
	ldq	t1,(t0)			# get the address of the handler
	ldq	a0,8(t0)		# get the parameter
	jsr	ra,(t1)			# call the routine
1:
	ldl	s1,0(s0)
	subl	s1,1,s1
	stl	s1,0(s0)	
	br	zero,exception_exit

2:      cmpeq	a0,2,t0
	blbc	t0,1f
/*
 * Machine Checks
 */
	mb
	bis	a1,zero,a0		# move vector into a0
	bis	a2,zero,a1		# move logout_addr into k1
	bis	sp,zero,a2		# address of register save frame
	jsr	ra,mach_error		# mach_error(vector,ksegaddr_logout)
	ldl	s1,0(s0)
	subl	s1,1,s1
	stl	s1,0(s0)	
	br	zero,exception_exit

1:	bne	a0,1f			# ip interrupts
/*
 * Interprocessor interrupts
 */
	mb
/*
	jsr	ipintr			# ipintr()
*/
	ldl	s1,0(s0)
	subl	s1,1,s1
	stl	s1,0(s0)	
	br	zero,exception_exit

1:	cmpeq	a0,4,t0
	blbc	t0,1f

/*
 * Performance monitor
 */
	lda	t0,_scb			# get the address of the 
	addq	t0,a1,t0		# index to scbentry
	ldq	t1,(t0)			# get the address of the handler

	bis	a2,zero,a0		# counter# -> a0
	ldq	a1,EF_PC*8(sp)
	ldq	a2,EF_PS*8(sp)
	jsr	ra,(t1)			# perfmon(cntr#, pc, ps)
	ldl	s1,0(s0)
	subl	s1,1,s1
	stl	s1,0(s0)	
	br	zero,exception_exit

1:	cmpeq	a0,5,t0
	blbc	t0,1f
/*
 * Passive release
 */
3:
	lda	t0,system_intr_cnts_type
	ldq	t1,INTR_TYPE_PASSIVE_RELEASE*8(t0)
	addq	t1,1,t1
	stq	t1,INTR_TYPE_PASSIVE_RELEASE*8(t0)	
	ldl	s1,0(s0)
	subl	s1,1,s1
	stl	s1,0(s0)	
	br	zero,exception_exit

1:				/* catch case that clock is off */
	cmpeq 	a0,1,t0
	blbc 	t0,1f
	ldl	s1,0(s0)
	subl	s1,1,s1
	stl	s1,0(s0)	
	br      zero,exception_exit

1:
	PANIC("Illegal Interrupt Type")
	END(_XentInt)

/*
 * A system call, transform into syscall(a0,......)
 *
 * This is the only case where complete state doesn't have to be saved.
 * We know that we just came from usermode and therefore can assume that
 * we don't have to save the callee saved registers and the tempory registers.
 * Unfortunately we do have to get the user stack pointer, it hasn't been
 * saved anywhere.
 *
 * HOWEVER, UN*X forking takes a special exit route and needs a full
 * register save/restore.  Yes, this is a kludge.  (I stole it from
 * the mips version.)
 *
 */
	.align	3
	.globl	_Xsyscall
	.ent	_Xsyscall
	.frame	sp,EF_SIZE,zero
	.mask	AR_MASK|M_V0|M_RA|M_SP|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
_Xsyscall:
	lda	sp,-EF_SSIZE(sp)	# allocate stack space
	stq	ra,EF_RA*8(sp)		# save the return address
	/*
	 * Unfortunately the arguments must be homed into the stack
	 * so that signals work properly (sendsig needs the state)
	 */
	stq	v0,EF_V0*8(sp)		# return value 0
	stq	a0,EF_A0*8(sp)		# home the arguments
	stq	a1,EF_A1*8(sp)
	stq	a2,EF_A2*8(sp)
	stq	a3,EF_A3*8(sp)
	stq	a4,EF_A4*8(sp)
	stq	a5,EF_A5*8(sp)
	blt	v0,mach_trap		# mach traps have numbers < 0

	bis	v0,zero,a1		# set up to call syscall
	bis	sp,zero,a0

	subq	a1,SYS_fork,t0		# fork ?
	beq	t0,syscall_fork
	subq	a1,SYS_vfork,t0		# vfork ?
	beq	t0,syscall_fork

2:	jsr	ra,syscall		# syscall(ep, code)

	bne	v0,sigreturn_case	# branch for normal syscall return path
	/*
	 * Restore r27(t12,pv) here.  This is necessary since this
	 * thread could have changed state, through a thread_set_state()
	 * called on it by another thread.  When the thread is
	 * resumed the new state is restored here.  r27 is used in gp
	 * calculation and failing to restore it will generate segmentation
	 * faults in user code gp is evaluated and dereferenced.
	 */
7:	ldq	t12,EF_T12*8(sp)	# restore r27(t12,pv)

	/* check for pending lightweight context interrupts */
lwc_test:
	ldq	a0,kernel_async_trap	# check lwc & need_ast interrupt flags
	bne	a0,async_test		# branch if none to process

	.set	noat
	bis	zero,zero,AT		# clear out registers for security
	.set	at			# reasons.
	bis	zero,zero,t0
	bis	zero,zero,t1
	bis	zero,zero,t2
	bis	zero,zero,t3
	bis	zero,zero,t4
	bis	zero,zero,t5
	bis	zero,zero,t6
	bis	zero,zero,t7
	bis	zero,zero,t8
	bis	zero,zero,t9
	bis	zero,zero,t10
	bis	zero,zero,t11

	ldq	ra,EF_RA*8(sp)		# restore return address
	ldq	v0,EF_V0*8(sp)		# restore value registers
	/*
	 * These argument registers need to be restored.  a0..a2 are
	 * used to pass info to sigreturn. a3 is used as the flag for
	 * success/failure.
	 */
	ldq	a0,EF_A0*8(sp)		# restore argument registers
	ldq	a1,EF_A1*8(sp)
	ldq	a2,EF_A2*8(sp)
	ldq	a3,EF_A3*8(sp)
	ldq	a4,EF_A4*8(sp)
	ldq	a5,EF_A5*8(sp)
	lda	sp,EF_SSIZE(sp)		# restore stack
	call_pal PAL_rtsys		# return to user

	/*
	 * This is a special return path for the pseudo system call
	 * sigreturn(), which is only taken if a user process has
	 * caught a signal and is returning from its signal handler.
	 * In this case, we need a full restore of all the registers,
	 * which have been copied from the user's sigcontext structure
	 * into the kernel's exception frame.  Further, we need to take
	 * the regular exception exit return path.  This is crucial for
	 * preserving temporary registers, which would get trashed by
	 * the "rtsys" pal call but are preserved by the "rti" pal call.
	 */
sigreturn_case:
	br	zero,exception_exit	# branch to common exception return

syscall_fork:
	bsr	ra,reg_save		# save everything, if fork or vfork
	br	zero,2b

async_test:
	ldl	a0,kernel_async_trap 	# check lwc&need_ast interrupt flags
	beq	a0,ast_handling		# branch if none to process

lwc_handling:
	bsr	ra,reg_save		# save all registers
	jsr	ra,lwc_schedule		# lwc_schedule();
	bsr	ra,reg_restore		# restore registers
	br	zero,lwc_test		# loop back for lwc test

ast_handling:
	bsr	ra,reg_save		# save all registers
	ldiq	a3,T_AST		# code in a3; a0-a2 unused
	bis	sp,zero,a4		# set up the frame pointer
	jsr	ra,trap			# trap(a0,a1,a2,trap_code,ef)
	bsr	ra,reg_restore		# restore registers
	br	zero,lwc_test		# loop back for lwc test


	.extern	mach_trap_count
	.extern	mach_trap_table
mach_trap:
	lda	t2,mach_trap_table	# launch fetches early
	ldl	t1,mach_trap_count
	subq	zero,v0,t0		# "Mach trap number"
	lda	v0,KERN_FAILURE(zero)	# set return in case of invalid trap
	s8addq	t0,t2,t2		# &mach_trap_table["Mach trap number"/2]
	subq	t1,t0,t1		# (mach_trap_count - "Mach trap number")
	s8addq	t0,t2,t2		# &mach_trap_table["Mach trap number"]
	ble	t1,4f			# invalid trap

	/*
	 *	Load trap descriptor, and check the number of args.
	 *	Entries in the table are defined by:
	 * 		typedef struct {
	 *			short		mach_trap_length;
	 *			short		mach_trap_flags;
	 *			int		(*mach_trap_function)();
	 * 		} mach_trap_t;
	 */
/*
 * Warning: if we ever define traps with more than 6 args
 * we have to put them on the stack before doing the call
	ldq	t1,0(t2)		# trap->mach_trap_length (in bytes)
	extwl	t1,0,t1
	srl	t1,3,t1			# nargs+1
 */

	ldq	v0,8(t2)		# trap->mach_trap_function
	jsr	ra,(v0)			# do the call
4:	stq	v0,EF_V0*8(sp)		# pass return code to user

	/* This should be CHECK_SIGNALS, but isn't */
	ldq	a0,the_current_thread
	ldq	a1,UTASK(a0)
	ldq	a1,U_PROCP(a1)
	beq	a1,7b
	/*
	 * inline 'ldb a2,P_CURSIG(a1)'.  Otherwise assembler will use
	 * r27 as a temporary register which we didn't save.
	 */
	ldq_u	a2,P_CURSIG(a1)
	lda	a3,P_CURSIG(a1)
	extbl	a2,a3,a2
	ldq	a1,P_SIG(a1)		# really a sigset_t
	bis	a1,a2,a1
	beq	a1,7b			# no pending signals
	stl	a1,kernel_async_trap+4	# post ast (will be checked below)
	br	zero,7b
1:
	ldiq	v0,KERN_FAILURE
	br	zero,7b			# return from mach_trap
	.end	_Xsyscall

/*
 * System Entry Arithmetic Traps
 *
 */
TRAP(entArith,T_ARITH)

/*
 * System Entry Memory Management
 */
	.align	3
	.globl	_XentMM
	.ent	_XentMM,0
	.frame	sp,EF_SIZE,zero
	.mask	EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
_XentMM:
	lda	sp,-EF_SSIZE(sp)	# allocate stack frame
	stq	ra,EF_RA*8(sp)		# save return register
	stq	v0,EF_V0*8(sp)		# return value 0
	stq	a3,EF_A3*8(sp)		# argument register 3
	stq	a4,EF_A4*8(sp)		# argument register 4
	stq	a5,EF_A5*8(sp)		# argument register 5
	.set	noat
	stq	AT,EF_AT*8(sp)		# save assembler temp
	.set	at
	bsr	ra,reg_save		# save all registers
	cmplt	a1,2,t0			# is this a fault or tnv/acv?
	blbs	t0,3f			# tnv/acv are handled in trap
	bis	a0,zero,s0		# save arguments from palcode
	bis	a1,zero,s1
	bis	a2,zero,s2
	bis	a2,zero,a1		# put access type in 2nd arg
	jsr	ra,pmap_fault_on	# pmap_fault_on(va, mmcause)
	bne	v0,4f			# branch if successfully handled
	bis	s0,zero,a0		# else need to restore trap args
	bis	s1,zero,a1
	bis	s2,zero,a2
3:	ldiq	a3,T_MMANG		# assign trap code parameter
	bis	sp,zero,a4		# assign frame pointer parameter
	jsr	ra,trap			# trap(a0,a1,a2,trap_code,ef)
4:	
	br	zero,exception_exit
	END(_XentMM)

	.data
/*
 * alignment fault handler, note that we don't handle any of the
 * locking types (stx_c or ldx_l)
 */
align_handlers:
	.quad	ldf_fault, ldg_fault, lds_fault, ldt_fault
	.quad	stf_fault, stg_fault, sts_fault, stt_fault
	.quad	ldl_fault, ldq_fault, unaligned, unaligned
	.quad	stl_fault, stq_fault, unaligned, unaligned
	.text

/*
 * System Entry Unaligned Access
 * 
 *	user registers a0..a2,gp,pc,ps have been saved on the stack
 *
 *	a0 = va, a1 = instcode, a2 = src/dst regno
 *
 */
	.align	3
	.globl	_XentUna
	.ent	_XentUna,0
	.frame	sp,EF_SIZE,zero
	.mask	EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
_XentUna:
	lda	sp,-EF_SSIZE(sp)	# allocate space software
	stq	ra,EF_RA*8(sp)		# save return register
	stq	v0,EF_V0*8(sp)		# return value 0
	stq	a3,EF_A3*8(sp)		# argument register 3
	stq	a4,EF_A4*8(sp)		# argument register 4
	stq	a5,EF_A5*8(sp)		# argument register 5
	.set	noat
	stq	AT,EF_AT*8(sp)		# save assembler temp
	.set	at
	bsr	ra,reg_save		# save all registers

	/*
	 * protect against user mode access to kernel space
	 */
	blt	a0,_Xunacheck		# branch if kernel space access
					# (might continue at _XentUna1)

	/*
	 * fetch the unaligned access control flags for the process
	 */
	ldq	t0,the_current_thread	# this gets active_threads[0]
	ldq	t0,UTASK(t0)
	ldq	t0,U_PROCP(t0)
	ldl	s0,P_UAC(t0)		# this gets u.u_procp->p_uac
	ldl	s1,sys_uac		# this gets system-wide sys_uac
	or	s0,s1,s2		# this gets combination of both sets

	/*
	 * print a console/user-terminal fault message if appropriate
	 */
	and	s2,UAC_NOPRINT,t0	# isolate the print-suppress flags
	beq	t0,_Xunaprint		# branch if clear in both (default)
_XentUna1:				# will always return from _Xunaprint

	/*
	 * post a SIGBUS signal to the thread if requested for the process
	 */
	and	s0,UAC_SIGBUS,t0	# s0 still has u.u_procp->p_uac
	bne	t0,_Xunasignal		# branch if flag set (not default)
_XentUna2:				# will always return from _Xunasignal

	/*
	 * perform the unaligned access fixup if appropriate (the default)
	 */
	and	s0,UAC_NOFIX,t0		# s0 still has u.u_procp->p_uac
	bne	t0,_Xunaexit1		# branch if flag set (not default)
	ldq     s6,current_pcb		# set up nofault handler in pcb
	ldiq	t0,NF_USERACC		# get code for uaerror routine
	stq	t0,PCB_NOFAULT(s6)	# assign pcb_nofault for thread

	/*
	 * The memory reference instructions that can cause an
	 * alignment fault are coded 0x20..0x2f, therefore the
	 * significant bits that determine the function are in
	 * the lowest 4 bits.
	 */
	and	a1,0xf,t0		# isolate handler table index
	lda	t1,align_handlers	# get the base of the table
	s8addq	t0,t1,t1		# calculate entry address
	ldq	t2,(t1)			# get the handler address
	and	a2,0x1f,a1		# put register number in a1
	jmp	zero,(t2)		# dispatch to fixup routine
					# will normally return here
_Xunaexit:
	stq	zero,PCB_NOFAULT(s6)	# clear nofault handler code
_Xunaexit1:
	and	s0,UAC_SIGBUS,t0	# s0 still has u.u_procp->p_uac
	beq	t0,exception_exit	# branch if flag not set (default)
	jsr	ra,afault_psig		# psig() handles earlier SIGBUS post
	br	zero,exception_exit	# branch to common rti handling

_Xunacheck:
	bis	zero,zero,s0		# zero flags for kernel case
	bis	zero,zero,s1		# zero in case of future use
	bis	zero,zero,s2		# zero in case of future use
	ldq	t0,EF_PS*8(sp)		# get PS from exception frame
	and	t0,PSL_CURMOD,t0	# isolate user/kernel mode bit
	beq	t0,_Xunaprint		# branch ahead if in kernel mode

	/* here to handle invalid user mode access to kernel space */
	and	a1,4,t0			# check bit 2 of opcode
	ldiq	a1,T_MMANG_ACV		# fabricate mmcsr value
	ldiq	a2,MMF_READ		# assume read code for mmcause
	ldiq	t1,MMF_WRITE		# but we might switch to write
	cmovne	t0,t1,a2		# switch if opcode was a store
	ldiq	a3,T_MMANG		# set up trap code as 4th arg
	bis	sp,zero,a4		# set up exception frame arg
	jsr	ra,trap			# trap(a0,a1,a2,trap_code,ef)
	br	zero,exception_exit	# branch to common rti handling

_Xunaprint:
	bis	a0,zero,s3		# save a0/va, a1/opcode, a2/regno
	bis	a1,zero,s4
	bis	a2,zero,s5
	ldq	a3,EF_PC*8(sp)		# get the offending PC
	ldq	a4,EF_PS*8(sp)		# get the offending PS
	ldq	a5,EF_RA*8(sp)		# get the return address
	and	a1,0xf,a1		# isolate opcode type index
	subq	a3,4,a3			# used backed-up pc for printf
	jsr	ra,afault_print		# afault_print(va,type,pc,ps,ra)
	bis	s3,zero,a0		# restore a0/va, a1/opcode, a2/regno
	bis	s4,zero,a1
	bis	s5,zero,a2
	br	zero,_XentUna1		# branch back to fast path code

_Xunasignal:
	bis	a0,zero,s3		# save a0/va, a1/opcode, a2/regno
	bis	a1,zero,s4
	bis	a2,zero,s5
	jsr	ra,afault_signal	# afault_signal(va,opcode,regno)
	bis	s3,zero,a0		# restore a0/va, a1/opcode, a2/regno
	bis	s4,zero,a1
	bis	s5,zero,a2
	br	zero,_XentUna2		# branch back to fast path code

	END(_XentUna)

/*
 * start_init()
 * Calls load_init_program() as if from syscall handler
 */
LEAF(start_init)		/* we need to save nothing */
	ldq	v0,the_current_thread
	ldq	v0,THREAD_KERNEL_STACK(v0)	# reset the stack
	addq	v0,KERNEL_STACK_SIZE-EF_SIZE
	bis	v0,zero,sp

	jsr	ra,load_init_program
	
	ldiq	t0,8			# user mode, ipl 0
	stq	t0,EF_PS*8(sp)
	.set    noat;
	ldq     AT,EF_AT*8(sp)		# restore assembler temp
	.set    at
	ldq     ra,EF_RA*8(sp)		# retore return addr reg
	lda	sp,EF_SSIZE(sp)		# restore the stack
	ldq	a0,EF_A0*8(sp)		# restore argument registers
	ldq	a1,EF_A1*8(sp)
	ldq	a2,EF_A2*8(sp)
	call_pal PAL_rtsys		# return
	END(start_init)
/*
 *	thread_bootstrap:
 *
 *	Bootstrap a new thread using the Alpha thread state that has been
 *	placed on the stack.
 *
 *	This often occurs after a new thread results from a fork.
 */
LEAF(thread_bootstrap)
	bsr	ra,reg_restore		# full restore
	.set    noat
	ldq     AT,EF_AT*8(sp)         # restore assembler temp
	.set    at
	ldq     ra,EF_RA*8(sp)         # retore return addr reg
	ldq	v0,EF_V0*8(sp)
	ldq	a0,EF_A0*8(sp)
	ldq	a1,EF_A1*8(sp)
	ldq	a2,EF_A2*8(sp)
	ldq	a3,EF_A3*8(sp)
	ldq	a4,EF_A4*8(sp)		# restore argument registers
	ldq	a5,EF_A5*8(sp)
	lda	sp,EF_SSIZE(sp)        # restore the stack
	call_pal PAL_rtsys            # return
	END(thread_bootstrap)


/*
 * floating point disabled faults
 *
 * if the floating point unit has ever been used by this process
 * we restore registers from the pcb and let him continue making
 * sure we've set the ownedfp flag in the pcb
 *
 * called as enable_fen( current_pcb )
 */
#define ROUND_PLUS_INFINITY	0x0c00000000000000
#define DEFAULT_FPCR		ROUND_PLUS_INFINITY
LEAF(enable_fen)
	lda	sp,-8(sp)		# need space for "$f0" save
	bis	ra,zero,a4		# save the return address 
	bis	a0,zero,a1		# a0 holds PCB pointer
#if RT_PREEMPT
	call_pal PAL_rdps 		# check for ipl 0... can't allow preempt
	bne	v0,1f			# if ipl non-zero... leave it alone

	ldiq	a0,1			# set ipl to 1 (no preemption!!)
	call_pal PAL_swpipl

1:					# v0 value must be preserved in this
					# routine for check on exit.  If v0 is
					# zero, then must restore IPL to zero.
#endif

	ldiq	a0,1			# setup to set flag and turn on fen
	call_pal PAL_wrfen		# enable use of the floating point reg

	lda	a0,PCB_FPREGS(a1)	# get the address of the save area
	bsr	ra,freg_restore		# restore the floating point registers

	ldq	t4,PCB_OWNEDFP(a1)	# test if FP used before.  If not, 
	beq	t4,4f			# need to setup FPCR

2:
	stt	$f0,0(sp)		# save $f0 temporarily
	ldt	$f0,PCB_FPCR(a1)	# get the saved fpcr value from pcb
	trapb
	mt_fpcr	$f0,$f0,$f0		# set or restore fpcr
	trapb
	ldt	$f0,0(sp)		# restore $f0

	lda	sp,8(sp)		# pop off stack save area
#if RT_PREEMPT
	bne     v0,3f
	bis	a4,zero,ra		# restore the return address 
	bsr	zero,spl0		# NO RETURN!!!!!  Go to spl0 code for
					# preemption test.
3:
#endif
	ret	zero,(a4)
4:
	ldiq	t5,1
	stq 	t5,PCB_OWNEDFP(a1)
	ldiq	t4,DEFAULT_FPCR		# get default floating point control
	stq	t4,PCB_FPCR(a1)         # save off value in pcb 
	br	zero,2b

	END(enable_fen)

LEAF(disable_fen)
	ldiq    a0,0      
        call_pal PAL_wrfen
	RET
	END(disable_fen)

/*
 * Following are the handlers for unaligned access fixups.
 */

SCBCMVEC(stq_fault)
	bsr	ra,load_framereg	# get the register contents in t0
	bsr	ra,store_long		# store it in memory
SCBCMEND(stq_fault)

SCBCMVEC(stl_fault)
	bsr	ra,load_framereg	# get the register contents in t0
	bsr	ra,store_int		# store it in memory
SCBCMEND(stl_fault)

SCBCMVEC(stg_fault)
	bsr	ra,double_to_mem	# store the floating register in memory
	ldq	t0,(sp)		# get it back in a scalar register
	sll	t0,32,t1	# t1 = cd00
	srl	t0,32,t0	# t0 = 00ab
	or	t0,t1,t0	# t0 = cdab
	sll	t0,16,t1	# t1 = dab0
	srl	t0,16,t0	# t0 = 0cda
	zap	t1,48,t1	# t1 = d0b0
	zap	t0,12,t0	# t0 = 0c0a
	or	t0,t1,t0	# t0 = dcba
	bsr	ra,store_long	# store it in memory
SCBCMEND(stg_fault)

SCBCMVEC(stf_fault)
	bsr	ra,float_to_mem	# store the floating register in memory
	ldl	t0,0(sp)	# get it back in a scalar register
	sll	t0,16,t1	# t1 = 0ab0
	srl	t0,16,t0	# t0 = 000a
	or	t0,t1,t0	# t0 = 0aba
	bsr	ra,store_int	# store it in memory
SCBCMEND(stf_fault)

SCBCMVEC(sts_fault)
	bsr	ra,float_to_mem	# store the floating register in memory
	ldl	t0,(sp)		# get it
	bsr	ra,store_int	# store it in memory
SCBCMEND(sts_fault)

SCBCMVEC(stt_fault)
	bsr	ra,double_to_mem	# store the floating register in memory
	ldq	t0,(sp)		# get it
	bsr	ra,store_long	# store it in memory
SCBCMEND(stt_fault)

SCBCMVEC(ldq_fault)
	bsr	ra,load_long
	bsr	ra,store_framereg
SCBCMEND(ldq_fault)

SCBCMVEC(ldl_fault)
	bsr	ra,load_int
	bsr	ra,store_framereg
SCBCMEND(ldl_fault)

SCBCMVEC(ldg_fault)
	bsr	ra,load_long
	
	sll	t0,32,t1	# t1 = cd00
	srl	t0,32,t0	# t0 = 00ab
	or	t0,t1,t0	# t0 = cdab
	sll	t0,16,t1	# t1 = dab0
	srl	t0,16,t0	# t0 = 0cda
	zap	t1,48,t1	# t1 = d0b0
	zap	t0,12,t0	# t0 = 0c0a
	or	t0,t1,t0	# t0 = dcba
	stq	t0,(sp)
	bsr	ra,mem_to_double
SCBCMEND(ldg_fault)

SCBCMVEC(ldf_fault)
	bsr	ra,load_int	# put result in t0
	sll	t0,16,t1	# t1 = 0ab0
	srl	t0,16,t0	# t0 = 000a
	or	t0,t1,t0	# t0 = 0aba
	sll	t0,32,t0	# t0 = ba00
	extql	t0,4,t0
	stq	t0,(sp)
	bsr	ra,mem_to_float
SCBCMEND(ldf_fault)

SCBCMVEC(lds_fault)
	bsr	ra,load_int
	stl	t0,(sp)
	bsr	ra,mem_to_float
SCBCMEND(lds_fault)

SCBCMVEC(ldt_fault)
	bsr	ra,load_long
	stq	t0,(sp)
	bsr	ra,mem_to_double
SCBCMEND(ldt_fault)

/*
 * unaligned lock faults - ain't gona do em !!
 *
 * panic on kernel, log and kill on user
 */
SCBCMVEC(unaligned)
	ldq	a0,EF_PS*8+TMPSPACE(sp)	# get the offending PS
	jsr	ra,afault_error		# kill!
	call_pal	PAL_halt	# shouldn't get here
SCBCMEND(unaligned)

/*
 * Breakpoint instruction. Used by the kernel for single stepping.
 */
EXPORT(sstepbp)
	call_pal PAL_bpt

/*
 * Save and restore the floating point registers
 *
 *	freg_save(addr)
 *	freg_restore(addr)
 */
LEAF(freg_save)
	stt	$f0,0*8(a0)
	stt	$f1,1*8(a0)
	stt	$f2,2*8(a0)
	stt	$f3,3*8(a0)
	stt	$f4,4*8(a0)
	stt	$f5,5*8(a0)
	stt	$f6,6*8(a0)
	stt	$f7,7*8(a0)
	stt	$f8,8*8(a0)
	stt	$f9,9*8(a0)
	stt	$f10,10*8(a0)
	stt	$f11,11*8(a0)
	stt	$f12,12*8(a0)
	stt	$f13,13*8(a0)
	stt	$f14,14*8(a0)
	stt	$f15,15*8(a0)
	stt	$f16,16*8(a0)
	stt	$f17,17*8(a0)
	stt	$f18,18*8(a0)
	stt	$f19,19*8(a0)
	stt	$f20,20*8(a0)
	stt	$f21,21*8(a0)
	stt	$f22,22*8(a0)
	stt	$f23,23*8(a0)
	stt	$f24,24*8(a0)
	stt	$f25,25*8(a0)
	stt	$f26,26*8(a0)
	stt	$f27,27*8(a0)
	stt	$f28,28*8(a0)
	stt	$f29,29*8(a0)
	stt	$f30,30*8(a0)
	stt	$f31,31*8(a0)
	ret	zero,(ra)
END(freg_save)

LEAF(freg_restore)
	ldt	$f0,0*8(a0)
	ldt	$f1,1*8(a0)
	ldt	$f2,2*8(a0)
	ldt	$f3,3*8(a0)
	ldt	$f4,4*8(a0)
	ldt	$f5,5*8(a0)
	ldt	$f6,6*8(a0)
	ldt	$f7,7*8(a0)
	ldt	$f8,8*8(a0)
	ldt	$f9,9*8(a0)
	ldt	$f10,10*8(a0)
	ldt	$f11,11*8(a0)
	ldt	$f12,12*8(a0)
	ldt	$f13,13*8(a0)
	ldt	$f14,14*8(a0)
	ldt	$f15,15*8(a0)
	ldt	$f16,16*8(a0)
	ldt	$f17,17*8(a0)
	ldt	$f18,18*8(a0)
	ldt	$f19,19*8(a0)
	ldt	$f20,20*8(a0)
	ldt	$f21,21*8(a0)
	ldt	$f22,22*8(a0)
	ldt	$f23,23*8(a0)
	ldt	$f24,24*8(a0)
	ldt	$f25,25*8(a0)
	ldt	$f26,26*8(a0)
	ldt	$f27,27*8(a0)
	ldt	$f28,28*8(a0)
	ldt	$f29,29*8(a0)
	ldt	$f30,30*8(a0)
	ldt	$f31,31*8(a0)
	ret	zero,(ra)
END(freg_restore)


/*
 * halt entry point
 */
LEAF(halt)
1:	call_pal PAL_halt
	br	zero,1b		# can't ever come back from here !!!
	END(halt)

LEAF(kdebug_halt)
1:	call_pal PAL_halt
	br	zero,1b		# can't ever come back from here !!!
	END(kdebug_halt)

/*
 * Unaligned access support
 *
 * Much of the design follows an implimentation
 * done by Wayne Cardoza for VMS
 */


/*
 * Load an unaligned quadword
 *
 * input virtual address in a0
 *
 * return with long in t0
 */
LEAF(load_long)
	ldq_u	t0,(a0)
	ldq_u	t1,7(a0)
	extql	t0,a0,t0
	extqh	t1,a0,t1
	or	t0,t1,t0
	ret	zero,(ra)
	END(load_long)

/*
 * Load an unaligned longword
 *
 * input virtual address in a0
 *
 * return with int in t0
 */
LEAF(load_int)
	ldq_u	t0,(a0)
	ldq_u	t1,3(a0)
	extll	t0,a0,t0
	extlh	t1,a0,t1
	or	t0,t1,t0
	sll	t0,32,t0
	sra	t0,32,t0
	ret	zero,(ra)
	END(load_int)

/*
 * Store quadword at unaligned address
 * 
 * virtual address in a0
 *
 * quadword to store in t0
 */
LEAF(store_long)
	ldq_u	t1,(a0)			# get the words to be updated
	ldq_u	t2,7(a0)
	insql	t0,a0,t3		# insert the new bytes
	insqh	t0,a0,t4
	mskql	t1,a0,t1		# clear bytes so merge is correct
	mskqh	t2,a0,t2
	or	t3,t1,t1		# stuff in the new bytes
	or	t4,t2,t2
	stq_u	t1,(a0)			# put the new words back
	stq_u	t2,7(a0)
	ret	zero,(ra)
	END(store_long)

/*
 * Store longword at unaligned address
 * 
 * virtual address in a0
 *
 * longword to store in t0
 */
LEAF(store_int)
	ldq_u	t1,(a0)			# get the words to be updated
	ldq_u	t2,3(a0)
	insll	t0,a0,t3		# insert the new bytes
	inslh	t0,a0,t4
	mskll	t1,a0,t1		# clear bytes so merge is correct
	msklh	t2,a0,t2
	or	t3,t1,t1		# stuff in the new bytes
	or	t4,t2,t2
	stq_u	t2,3(a0)
	stq_u	t1,(a0)			# put the new words back
	ret	zero,(ra)
	END(store_int)

/*
 * load a floating point register from memory
 * 
 * ALPHA doesn't have conversion routines or a mechanism to move
 * a scalar register to a floating point register or visa versa.
 * This task is accomplished by loading and storing via a memory
 * location.
 *
 * This uses the saveargs part of the exception frame!!
 */

	.align	3
LEAF(double_to_mem)
	s8addq	a1,0,t1		# scale register number to array offset
	br	t0,xct_and_ret	# load t0 with the start of this array:
	stt  $f0,(sp);	 ret zero,(ra);	 stt  $f1,(sp);	 ret zero,(ra)
	stt  $f2,(sp);	 ret zero,(ra);	 stt  $f3,(sp);	 ret zero,(ra)
	stt  $f4,(sp);	 ret zero,(ra);	 stt  $f5,(sp);	 ret zero,(ra)
	stt  $f6,(sp);	 ret zero,(ra);	 stt  $f7,(sp);	 ret zero,(ra)
	stt  $f8,(sp);	 ret zero,(ra);	 stt  $f9,(sp);	 ret zero,(ra)
	stt $f10,(sp);	 ret zero,(ra);	 stt $f11,(sp);	 ret zero,(ra)
	stt $f12,(sp);	 ret zero,(ra);	 stt $f13,(sp);	 ret zero,(ra)
	stt $f14,(sp);	 ret zero,(ra);	 stt $f15,(sp);	 ret zero,(ra)
	stt $f16,(sp);	 ret zero,(ra);	 stt $f17,(sp);	 ret zero,(ra)
	stt $f18,(sp);	 ret zero,(ra);	 stt $f19,(sp);	 ret zero,(ra)
	stt $f20,(sp);	 ret zero,(ra);	 stt $f21,(sp);	 ret zero,(ra)
	stt $f22,(sp);	 ret zero,(ra);	 stt $f23,(sp);	 ret zero,(ra)
	stt $f24,(sp);	 ret zero,(ra);	 stt $f25,(sp);	 ret zero,(ra)
	stt $f26,(sp);	 ret zero,(ra);	 stt $f27,(sp);	 ret zero,(ra)
	stt $f28,(sp);	 ret zero,(ra);	 stt $f29,(sp);	 ret zero,(ra)
	stt $f30,(sp);	 ret zero,(ra);	 stt $f31,(sp);	 ret zero,(ra)
	END(double_to_mem)

	.align	3
xct_and_ret:
	addq	t1,t0,t0	# add the offset (t1) to the base (t0)
	jmp	zero,(t0)	# jump to instruction pair

	.align	3
LEAF(mem_to_double)
	s8addq	a1,0,t1		# scale register number to array offset
	br	t0,xct_and_ret	# load t0 with the start of this array:
	ldt  $f0,(sp);	 ret zero,(ra);	 ldt  $f1,(sp);	 ret zero,(ra)
	ldt  $f2,(sp);	 ret zero,(ra);	 ldt  $f3,(sp);	 ret zero,(ra)
	ldt  $f4,(sp);	 ret zero,(ra);	 ldt  $f5,(sp);	 ret zero,(ra)
	ldt  $f6,(sp);	 ret zero,(ra);	 ldt  $f7,(sp);	 ret zero,(ra)
	ldt  $f8,(sp);	 ret zero,(ra);	 ldt  $f9,(sp);	 ret zero,(ra)
	ldt $f10,(sp);	 ret zero,(ra);	 ldt $f11,(sp);	 ret zero,(ra)
	ldt $f12,(sp);	 ret zero,(ra);	 ldt $f13,(sp);	 ret zero,(ra)
	ldt $f14,(sp);	 ret zero,(ra);	 ldt $f15,(sp);	 ret zero,(ra)
	ldt $f16,(sp);	 ret zero,(ra);	 ldt $f17,(sp);	 ret zero,(ra)
	ldt $f18,(sp);	 ret zero,(ra);	 ldt $f19,(sp);	 ret zero,(ra)
	ldt $f20,(sp);	 ret zero,(ra);	 ldt $f21,(sp);	 ret zero,(ra)
	ldt $f22,(sp);	 ret zero,(ra);	 ldt $f23,(sp);	 ret zero,(ra)
	ldt $f24,(sp);	 ret zero,(ra);	 ldt $f25,(sp);	 ret zero,(ra)
	ldt $f26,(sp);	 ret zero,(ra);	 ldt $f27,(sp);	 ret zero,(ra)
	ldt $f28,(sp);	 ret zero,(ra);	 ldt $f29,(sp);	 ret zero,(ra)
	ldt $f30,(sp);	 ret zero,(ra);	 ldt $f31,(sp);	 ret zero,(ra)
	END(mem_to_double)

	.align	3
LEAF(float_to_mem)
	s8addq	a1,0,t1		# scale register number to array offset
	br	t0,xct_and_ret	# load t0 with the start of this array:
	sts  $f0,(sp);	 ret zero,(ra);	 sts  $f1,(sp);	 ret zero,(ra)
	sts  $f2,(sp);	 ret zero,(ra);	 sts  $f3,(sp);	 ret zero,(ra)
	sts  $f4,(sp);	 ret zero,(ra);	 sts  $f5,(sp);	 ret zero,(ra)
	sts  $f6,(sp);	 ret zero,(ra);	 sts  $f7,(sp);	 ret zero,(ra)
	sts  $f8,(sp);	 ret zero,(ra);	 sts  $f9,(sp);	 ret zero,(ra)
	sts $f10,(sp);	 ret zero,(ra);	 sts $f11,(sp);	 ret zero,(ra)
	sts $f12,(sp);	 ret zero,(ra);	 sts $f13,(sp);	 ret zero,(ra)
	sts $f14,(sp);	 ret zero,(ra);	 sts $f15,(sp);	 ret zero,(ra)
	sts $f16,(sp);	 ret zero,(ra);	 sts $f17,(sp);	 ret zero,(ra)
	sts $f18,(sp);	 ret zero,(ra);	 sts $f19,(sp);	 ret zero,(ra)
	sts $f20,(sp);	 ret zero,(ra);	 sts $f21,(sp);	 ret zero,(ra)
	sts $f22,(sp);	 ret zero,(ra);	 sts $f23,(sp);	 ret zero,(ra)
	sts $f24,(sp);	 ret zero,(ra);	 sts $f25,(sp);	 ret zero,(ra)
	sts $f26,(sp);	 ret zero,(ra);	 sts $f27,(sp);	 ret zero,(ra)
	sts $f28,(sp);	 ret zero,(ra);	 sts $f29,(sp);	 ret zero,(ra)
	sts $f30,(sp);	 ret zero,(ra);	 sts $f31,(sp);	 ret zero,(ra)
	END(float_to_mem)

	.align	3
LEAF(mem_to_float)
	s8addq	a1,0,t1		# scale register number to array offset
	br	t0,xct_and_ret	# load t0 with the start of this array:
	lds  $f0,(sp);	 ret zero,(ra);	 lds  $f1,(sp);	 ret zero,(ra)
	lds  $f2,(sp);	 ret zero,(ra);	 lds  $f3,(sp);	 ret zero,(ra)
	lds  $f4,(sp);	 ret zero,(ra);	 lds  $f5,(sp);	 ret zero,(ra)
	lds  $f6,(sp);	 ret zero,(ra);	 lds  $f7,(sp);	 ret zero,(ra)
	lds  $f8,(sp);	 ret zero,(ra);	 lds  $f9,(sp);	 ret zero,(ra)
	lds $f10,(sp);	 ret zero,(ra);	 lds $f11,(sp);	 ret zero,(ra)
	lds $f12,(sp);	 ret zero,(ra);	 lds $f13,(sp);	 ret zero,(ra)
	lds $f14,(sp);	 ret zero,(ra);	 lds $f15,(sp);	 ret zero,(ra)
	lds $f16,(sp);	 ret zero,(ra);	 lds $f17,(sp);	 ret zero,(ra)
	lds $f18,(sp);	 ret zero,(ra);	 lds $f19,(sp);	 ret zero,(ra)
	lds $f20,(sp);	 ret zero,(ra);	 lds $f21,(sp);	 ret zero,(ra)
	lds $f22,(sp);	 ret zero,(ra);	 lds $f23,(sp);	 ret zero,(ra)
	lds $f24,(sp);	 ret zero,(ra);	 lds $f25,(sp);	 ret zero,(ra)
	lds $f26,(sp);	 ret zero,(ra);	 lds $f27,(sp);	 ret zero,(ra)
	lds $f28,(sp);	 ret zero,(ra);	 lds $f29,(sp);	 ret zero,(ra)
	lds $f30,(sp);	 ret zero,(ra);	 lds $f31,(sp);	 ret zero,(ra)
	END(mem_to_float)
/*
 * get register from exception frame
 */
	.align 3
LEAF(load_framereg)
	s8addq	a1,0,t1		# scale register number to array offset
	br	t0,xct_and_ret	# load t0 with the start of this array:
	ldq	t0,EF_V0*8+TMPSPACE(sp);	ret zero,(ra)	#r0
	ldq	t0,EF_T0*8+TMPSPACE(sp);	ret zero,(ra)	#r1
	ldq	t0,EF_T1*8+TMPSPACE(sp);	ret zero,(ra)	#r2
	ldq	t0,EF_T2*8+TMPSPACE(sp);	ret zero,(ra)	#r3
	ldq	t0,EF_T3*8+TMPSPACE(sp);	ret zero,(ra)	#r4
	ldq	t0,EF_T4*8+TMPSPACE(sp);	ret zero,(ra)	#r5
	ldq	t0,EF_T5*8+TMPSPACE(sp);	ret zero,(ra)	#r6
	ldq	t0,EF_T6*8+TMPSPACE(sp);	ret zero,(ra)	#r7
	ldq	t0,EF_T7*8+TMPSPACE(sp);	ret zero,(ra)	#r8
	ldq	t0,EF_S0*8+TMPSPACE(sp);	ret zero,(ra)	#r9
	ldq	t0,EF_S1*8+TMPSPACE(sp);	ret zero,(ra)	#r10
	ldq	t0,EF_S2*8+TMPSPACE(sp);	ret zero,(ra)	#r11
	ldq	t0,EF_S3*8+TMPSPACE(sp);	ret zero,(ra)	#r12
	ldq	t0,EF_S4*8+TMPSPACE(sp);	ret zero,(ra)	#r13
	ldq	t0,EF_S5*8+TMPSPACE(sp);	ret zero,(ra)	#r14
	ldq	t0,EF_S6*8+TMPSPACE(sp);	ret zero,(ra)	#r15
	ldq	t0,EF_A0*8+TMPSPACE(sp);	ret zero,(ra)	#r16
	ldq	t0,EF_A1*8+TMPSPACE(sp);	ret zero,(ra)	#r17
	ldq	t0,EF_A2*8+TMPSPACE(sp);	ret zero,(ra)	#r18
	ldq	t0,EF_A3*8+TMPSPACE(sp);	ret zero,(ra)	#r19
	ldq	t0,EF_A4*8+TMPSPACE(sp);	ret zero,(ra)	#r20
	ldq	t0,EF_A5*8+TMPSPACE(sp);	ret zero,(ra)	#r21
	ldq	t0,EF_T8*8+TMPSPACE(sp);	ret zero,(ra)	#r22
	ldq	t0,EF_T9*8+TMPSPACE(sp);	ret zero,(ra)	#r23
	ldq	t0,EF_T10*8+TMPSPACE(sp);	ret zero,(ra)	#r24
	ldq	t0,EF_T11*8+TMPSPACE(sp);	ret zero,(ra)	#r25
	ldq	t0,EF_RA*8+TMPSPACE(sp);	ret zero,(ra)	#r26
	ldq	t0,EF_T12*8+TMPSPACE(sp);	ret zero,(ra)	#r27
	ldq	t0,EF_AT*8+TMPSPACE(sp);	ret zero,(ra)	#r28
	ldq	t0,EF_GP*8+TMPSPACE(sp);	ret zero,(ra)	#r29
	bis	v0,zero,t1; br zero,1f	# r30 - usp is in mtpr register
	bis	zero,zero,t0;			ret zero,(ra)	#r31
1:
	call_pal PAL_rdusp		
	bis	v0,zero,t0
	bis 	t1,zero,v0
	ret	zero,(ra)			
	END(load_framereg)
/*
 * store register into exception frame
 *
 * value in t0, register in a1.  Therefore, can't use xct_and_ret
 */
	.align	3
LEAF(store_framereg)
	s8addq	a1,0,t2		# scale register number to array offset
	br	t1,2f		# load t1 with the start of this array:
	stq	t0,EF_V0*8+TMPSPACE(sp);	ret zero,(ra)	#r0
	stq	t0,EF_T0*8+TMPSPACE(sp);	ret zero,(ra)	#r1
	stq	t0,EF_T1*8+TMPSPACE(sp);	ret zero,(ra)	#r2
	stq	t0,EF_T2*8+TMPSPACE(sp);	ret zero,(ra)	#r3
	stq	t0,EF_T3*8+TMPSPACE(sp);	ret zero,(ra)	#r4
	stq	t0,EF_T4*8+TMPSPACE(sp);	ret zero,(ra)	#r5
	stq	t0,EF_T5*8+TMPSPACE(sp);	ret zero,(ra)	#r6
	stq	t0,EF_T6*8+TMPSPACE(sp);	ret zero,(ra)	#r7
	stq	t0,EF_T7*8+TMPSPACE(sp);	ret zero,(ra)	#r8
	stq	t0,EF_S0*8+TMPSPACE(sp);	ret zero,(ra)	#r9
	stq	t0,EF_S1*8+TMPSPACE(sp);	ret zero,(ra)	#r10
	stq	t0,EF_S2*8+TMPSPACE(sp);	ret zero,(ra)	#r11
	stq	t0,EF_S3*8+TMPSPACE(sp);	ret zero,(ra)	#r12
	stq	t0,EF_S4*8+TMPSPACE(sp);	ret zero,(ra)	#r13
	stq	t0,EF_S5*8+TMPSPACE(sp);	ret zero,(ra)	#r14
	stq	t0,EF_S6*8+TMPSPACE(sp);	ret zero,(ra)	#r15
	stq	t0,EF_A0*8+TMPSPACE(sp);	ret zero,(ra)	#r16
	stq	t0,EF_A1*8+TMPSPACE(sp);	ret zero,(ra)	#r17
	stq	t0,EF_A2*8+TMPSPACE(sp);	ret zero,(ra)	#r18
	stq	t0,EF_A3*8+TMPSPACE(sp);	ret zero,(ra)	#r19
	stq	t0,EF_A4*8+TMPSPACE(sp);	ret zero,(ra)	#r20
	stq	t0,EF_A5*8+TMPSPACE(sp);	ret zero,(ra)	#r21
	stq	t0,EF_T8*8+TMPSPACE(sp);	ret zero,(ra)	#r22
	stq	t0,EF_T9*8+TMPSPACE(sp);	ret zero,(ra)	#r23
	stq	t0,EF_T10*8+TMPSPACE(sp);	ret zero,(ra)	#r24
	stq	t0,EF_T11*8+TMPSPACE(sp);	ret zero,(ra)	#r25
	stq	t0,EF_RA*8+TMPSPACE(sp);	ret zero,(ra)	#r26
	stq	t0,EF_T12*8+TMPSPACE(sp);	ret zero,(ra)	#r27
	stq	t0,EF_AT*8+TMPSPACE(sp);	ret zero,(ra)	#r28
	stq	t0,EF_GP*8+TMPSPACE(sp);	ret zero,(ra)	#r29
	bis	a0,zero,t1; br zero,1f	# r30 - usp is in mtpr register
	bis	zero,zero,zero;			ret zero,(ra)	#r31
1:
	bis	t0,zero,a0
	call_pal PAL_wrusp
	bis	t1,zero,a0
	ret	zero,(ra)			

2:	addq	t2,t1,t1	# add offset (t2) to base (t1)
	jmp	zero,(t1)	# jump to instruction pair
	END(store_framereg)

/*
 * Wrapper routines to get to per-platform routines;
 * effectively, fast jump table implementation for
 * critical path code.
 *
 * First pass implementation. Eventually, this will become
 * "self-modified" code by kernel (kn*_init funcitons will
 * load fixed address and jump).  Could use genassym-generated
 * defines for offsets, but since this is temporary, no need
 * to hit genassym twice (for end result of no change).
 *
 * Double-hexaword align to reduce I-cache miss penalty.
 * (D-HW fits 16 instructions)
 */
	.align 6
	.extern	dma_callsw
LEAF(dma_map_alloc)
	lda	t0,dma_callsw	# dma_map_alloc 1st entry in jmp table
	ldq	t0,(t0)		# get the address of the function
	jmp	(t0)
	END(dma_map_alloc)
LEAF(dma_map_load)
	lda	t0,dma_callsw	# dma_map_load 2nd entry in jmp table
	addq	t0,8,t0		# index to next entry
	ldq	t0,(t0)		# get the address of the function
	jmp	(t0)
	END(dma_map_load)
LEAF(dma_map_unload)
	lda	t0,dma_callsw	# dma_map_unload 3rd entry in jmp table
	addq	t0,16,t0	# index to next entry
	ldq	t0,(t0)		# get the address of the function
	jmp	(t0)
	END(dma_map_unload)
LEAF(dma_map_dealloc)
	lda	t0,dma_callsw	# dma_map_dealloc 4th entry in jmp table
	addq	t0,24,t0	# index to next entry
	ldq	t0,(t0)		# get the address of the function
	jmp	(t0)
	END(dma_map_dealloc)
	.align	4
LEAF(dma_min_boundary)
	lda	t0,dma_callsw	# dma_min_boundary 5th entry in jmp table
	addq	t0,32,t0	# index to next entry
	ldq	t0,(t0)		# get the address of the function
	jmp	(t0)
	END(dma_min_boundary)
/*
 * Console callback procedure dispatch
 *
 * This routine is a jacket routine that converts our standard
 * C function call to the alpha calling standard call.
 */
NESTED(prom_dispatcher,EF_SIZE,ra)
	.mask	TR_MASK|SR_MASK|M_RA|M_AT|M_GP|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
	lda	sp,-EF_SIZE(sp)
	stq     ra,EF_RA*8(sp)
	stq     gp,EF_GP*8(sp)
	.set noat
	stq     AT,EF_AT*8(sp)
	.set at
	bsr	ra,reg_save

	lda	t1,prom_stack_save_sp	# get address of sp save location
	stq	sp,(t1)			# save value of stack pointer
	lda	t2,prom_stack_va	# get address of prom stack va
	ldq	v0,(t2)			# get location of prom stack
	cmovne	v0,v0,sp		# if non-zero, switch stacks

	lda	t1,prom_dispatch_pv	# use variable assigned by prom_init()
	ldq	pv,(t1)			# load address of procedure descriptor
	ldq	v0,8(pv)		# load address of procedure entry
	jsr	ra,(v0)			# go for it

	lda	t1,prom_stack_save_sp	# restore value of stack pointer
	ldq	sp,(t1)

	bsr	ra,reg_restore
	.set    noat
	ldq     AT,EF_AT*8(sp)
	.set    at
	ldq     ra,EF_RA*8(sp)
	ldq     gp,EF_GP*8(sp)
	lda	sp,EF_SIZE(sp)
	ret	zero,(ra)
END(prom_dispatcher)

/*
 * Console callback procedure dispatch
 *
 * This routine is a jacket routine that converts our standard
 * C function call to the alpha calling standard call.
 */
NESTED(prom_fixup_dispatch,EF_SIZE,ra)
	.mask	TR_MASK|SR_MASK|M_RA|M_AT|M_GP|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
	lda	sp,-EF_SIZE(sp)
	stq     ra,EF_RA*8(sp)
	stq     gp,EF_GP*8(sp)
	.set noat
	stq     AT,EF_AT*8(sp)
	.set at
	bsr	ra,reg_save

	bis	a2,zero,pv		# load address of procedure descriptor
	ldq	v0,8(pv)		# load address of procedure entry
	jsr	ra,(v0)			# go for it

	bsr	ra,reg_restore
	.set    noat
	ldq     AT,EF_AT*8(sp)
	.set    at
	ldq     ra,EF_RA*8(sp)
	ldq     gp,EF_GP*8(sp)
	lda	sp,EF_SIZE(sp)
	ret	zero,(ra)
END(prom_fixup_dispatch)

/*
 * Save and restore the temps and callee saved registers
 * vPAL saves the temps in the exception frame, uPAL doesn't
 *
 */
LEAF(reg_save)
	stq	s0,EF_S0*8(sp)
	stq	s1,EF_S1*8(sp)
	stq	s2,EF_S2*8(sp)
	stq	s3,EF_S3*8(sp)
	stq	s4,EF_S4*8(sp)
	stq	s5,EF_S5*8(sp)
	stq	s6,EF_S6*8(sp)
	stq	t0,EF_T0*8(sp)
	stq	t1,EF_T1*8(sp)
	stq	t2,EF_T2*8(sp)
	stq	t3,EF_T3*8(sp)
	stq	t4,EF_T4*8(sp)
	stq	t5,EF_T5*8(sp)
	stq	t6,EF_T6*8(sp)
	stq	t7,EF_T7*8(sp)
	stq	t8,EF_T8*8(sp)
	stq	t9,EF_T9*8(sp)
	stq	t10,EF_T10*8(sp)
	stq	t11,EF_T11*8(sp)
	stq	t12,EF_T12*8(sp)
	ret	zero,(ra)
	END(reg_save)

LEAF(reg_restore)
	ldq	s0,EF_S0*8(sp)
	ldq	s1,EF_S1*8(sp)
	ldq	s2,EF_S2*8(sp)
	ldq	s3,EF_S3*8(sp)
	ldq	s4,EF_S4*8(sp)
	ldq	s5,EF_S5*8(sp)
	ldq	s6,EF_S6*8(sp)
	ldq	t0,EF_T0*8(sp)
	ldq	t1,EF_T1*8(sp)
	ldq	t2,EF_T2*8(sp)
	ldq	t3,EF_T3*8(sp)
	ldq	t4,EF_T4*8(sp)
	ldq	t5,EF_T5*8(sp)
	ldq	t6,EF_T6*8(sp)
	ldq	t7,EF_T7*8(sp)
	ldq	t8,EF_T8*8(sp)
	ldq	t9,EF_T9*8(sp)
	ldq	t10,EF_T10*8(sp)
	ldq	t11,EF_T11*8(sp)
	ldq	t12,EF_T12*8(sp)
	ret	zero,(ra)
	END(reg_restore)

/* 
 * get_caller_ra()
 *	purpose
 *		find the return address of the caller
 *	input
 *		none
 *	output
 *		none
 *	return
 *		caller return address
 */
#define OP_MASK         0x00000000ffff0000   /* mask for opcode and register */
#define OFFSET_MASK     0x000000000000ffff   /* mask for instruction offset */
#define STQ_TEMPLATE	0x00000000b75e0000   /* stq ra, 0(sp) instruction */

	.extern start
	.text

LEAF(get_caller_ra)
	ldgp	gp, 0(pv)
	bis	ra, 0, t0		/* copy current return address */
	lda	t1, start		/* water mark */
	ldiq	t2, STQ_TEMPLATE	/* template stq ra, 0(sp) */
1:
	subq	t0, 4			/* back up an instruction */ 
	cmpeq	t0, t1, v0
	bne	v0, 2f			/* pc sanity check */

	ldl	t3, 0(t0)		/* fetch instruction */
	and	t3, OP_MASK, v0
	cmpeq	v0, t2, v0		/* is the inst a stq ra ..... */
	beq	v0, 1b
					/* we found where the ra gets stuffed */
	and	t3, OFFSET_MASK, t3
	addq	t3, sp, v0 		/* add the offset to the sp value */
	ldq	v0, 0(v0) 		/* offset(sp) is the return address */
	RET				/* return the caller return address */
2:
	bis	zero, 0, v0
	RET				/* return error */

	END(get_caller_ra)

LEAF(enter_kdebug)
XLEAF(gimmeabreak)
	call_pal PAL_bpt
	RET
	END(enter_kdebug)

/*
 * Read the floating point control register.
 */
LEAF(_get_fpcr)
	lda     sp, -16(sp)             /* get some stack space */
	stt     $f0, 8(sp)              /* save register f0 */

	excb
	mf_fpcr	$f0, $f0, $f0
	trapb

	stt     $f0, 0(sp)              /* set up return of the fpcr */
	ldq     v0,  0(sp)
	
	ldt     $f0, 8(sp)              /* restore register f0 */
	lda     sp, 16(sp)              /* release stack space */
	RET
	END(_get_fpcr)

/*
 * Write the floating point control register.
 */
LEAF(_set_fpcr)
	lda     sp, -16(sp)             /* get some stack space */
	stt     $f0, 8(sp)              /* save register f0 */

	stq	a0, 0(sp)
	ldt	$f0, 0(sp)		/* argument to mt_fpcr */

	trapb
	mt_fpcr	$f0, $f0, $f0
	trapb

	ldt     $f0, 8(sp)              /* restore register f0 */
	lda     sp, 16(sp)              /* release stack space */
	RET
	END(_set_fpcr)

/*
 * doadump
 *
 * This is the handler for the restart entry point, which is used to
 * generate a crash dump after saving the appropriate state.  If the
 * call is from the console code (via "restart" or "restoreterm"),
 * then the ra register will be 0.  Otherwise, the ra register will
 * most likely be non-zero and we will make no assumptions about the
 * "halt" state in the rpb.
 */
	.globl	doadump
	.ent	doadump
	.frame	sp,EF_SIZE+8,zero
	.mask	EXCMASK|M_EXCFRM,-(EF_SIZE+8-(EF_RA*8))
	.align	3			# force quadword alignment
	.set	noreorder

doadump:
	lda	sp,-(EF_SIZE+8)(sp)	# allocate exception frame + 8
	stq	v0,EF_V0*8(sp)		# save every single register
	stq	gp,EF_GP*8(sp)		# including current gp value
	br	v0,1f			# load addr of embedded _gp
	.quad	_gp			# this must be quad aligned
1:	ldq	gp,0(v0)		# reload kernel gp for safety
	.set	noat			# save the rest of the registers
	stq	AT,EF_AT*8(sp)
	.set	at
	stq	ra,EF_RA*8(sp)
	stq	a0,EF_A0*8(sp)
	stq	a1,EF_A1*8(sp)
	stq	a2,EF_A2*8(sp)
	stq	a3,EF_A3*8(sp)
	stq	a4,EF_A4*8(sp)
	stq	a5,EF_A5*8(sp)
	bis	ra,zero,v0		# save ra (reg_save preserves v0)
	bsr	ra,reg_save		# reg_save saves t/s registers
	stq	zero,EF_SIZE(sp)	# clear loc for proc desc data
	lda	a0,EF_SIZE+8(sp)	# get sp value we entered with
	stq	a0,EF_SP*8(sp)		# temporarily assume this value
	stq	zero,EF_PS*8(sp)	# temporarily assume ipl zero
	stq	v0,EF_PC*8(sp)		# v0 has original value of ra
	bne	v0,2f			# check invocation method
	beq	pv,2f			# (pv ought to be valid)

	ldq	t0,0(pv)		# here if via console restart/rstrterm
	beq	t0,5f			# prevent re-entry via console vector
	stq	zero,0(pv)		# indicate we've been here already
	stq	t0,EF_SIZE(sp)		# save value from procedure desc
	bic	t0,7,t0			# clear out low-order flag bits
	ldq	t1,RPB_HALTPB(t0)	# fetch data from percpu slot
	ldq	t2,RPB_HALTPC(t0)
	ldq	t3,RPB_HALTPS(t0)
	ldq	t4,RPB_HALTAL(t0)
	ldq	t5,RPB_HALTRA(t0)
	ldq	t6,RPB_HALTPV(t0)
	zap	t0,0xf,t7		# generate unity_base from percpu
	addq	t7,t1,t1		# calculate kseg address of pcb
	ldq	t1,0(t1)		# fetch sp value at time of halt
	stq	t1,EF_SP*8(sp)		# update exception frame data
	stq	t2,EF_PC*8(sp)
	stq	t3,EF_PS*8(sp)
	stq	t4,EF_T11*8(sp)
	stq	t5,EF_RA*8(sp)
	stq	t6,EF_T12*8(sp)
	br	zero,3f
2:
	call_pal PAL_rdps		# here if via direct call or "start"
	stq	v0,EF_PS*8(sp)		# save current ipl level in frame
	ldq	t0,percpu		# use mapped percpu slot access
3:

	/*
	 * Clear the restart-capable bit in the percpu state flags of the rpb
	 * so this code won't get re-entered.  We want to preserve the state
	 * of the halt-requested bits so that the console code will perform
	 * the appropriate "auto-action" after we finish the memory dump.
	 */
	ldq	t1,RPB_STATE(t0)
	bic	t1,STATE_RC,t1
	stq	t1,RPB_STATE(t0)

	/*
	 * Reset the interrupt vectors to the original values in case they
	 * had been changed (by kdebug, for instance).
	 */
	lda	a0,_XentInt		# interrupts
	ldiq	a1,0
	call_pal PAL_wrent

	lda	a0,_XentArith		# arithmetic traps
	ldiq	a1,1
	call_pal PAL_wrent

	lda	a0,_XentMM		# memory management faults
	ldiq	a1,2
	call_pal PAL_wrent

	lda	a0,_XentIF		# instruction faults
	ldiq	a1,3
	call_pal PAL_wrent

	lda	a0,_XentUna		# unaligned accesses
	ldiq	a1,4
	call_pal PAL_wrent

	lda	a0,_Xsyscall		# system calls
	ldiq	a1,5
	call_pal PAL_wrent

	/*
	 * Save all of the register state again in the current pcb
	 * for clean tracebacks of the dump via dbx.
	 */
	ldq	a0,current_pcb		# check for a valid current pcb ptr
	beq	a0,4f

	lda	a0,PCB_REGS(a0)
	ldq	v0,EF_V0*8(sp)
	ldq	t0,EF_T0*8(sp)
	ldq	t1,EF_T1*8(sp)
	ldq	t2,EF_T2*8(sp)
	ldq	t3,EF_T3*8(sp)
	ldq	t4,EF_T4*8(sp)
	ldq	t5,EF_T5*8(sp)
	ldq	t6,EF_T6*8(sp)
	ldq	t7,EF_T7*8(sp)
	ldq	a3,EF_A3*8(sp)
	ldq	a4,EF_A4*8(sp)
	ldq	a5,EF_A5*8(sp)
	ldq	t8,EF_T8*8(sp)
	ldq	t9,EF_T9*8(sp)
	ldq	t10,EF_T10*8(sp)
	ldq	t11,EF_T11*8(sp)
	ldq	ra,EF_RA*8(sp)
	ldq	t12,EF_T12*8(sp)
	stq	v0,PCB_V0*8(a0)
	stq	t0,PCB_T0*8(a0)
	stq	t1,PCB_T1*8(a0)
	stq	t2,PCB_T2*8(a0)
	stq	t3,PCB_T3*8(a0)
	stq	t4,PCB_T4*8(a0)
	stq	t5,PCB_T5*8(a0)
	stq	t6,PCB_T6*8(a0)
	stq	t7,PCB_T7*8(a0)
	stq	s0,PCB_S0*8(a0)
	stq	s1,PCB_S1*8(a0)
	stq	s2,PCB_S2*8(a0)
	stq	s3,PCB_S3*8(a0)
	stq	s4,PCB_S4*8(a0)
	stq	s5,PCB_S5*8(a0)
	stq	s6,PCB_S6*8(a0)
	stq	a3,PCB_A3*8(a0)
	stq	a4,PCB_A4*8(a0)
	stq	a5,PCB_A5*8(a0)
	stq	t8,PCB_T8*8(a0)
	stq	t9,PCB_T9*8(a0)
	stq	t10,PCB_T10*8(a0)
	stq	t11,PCB_T11*8(a0)
	stq	ra,PCB_RA*8(a0)
	stq	t12,PCB_T12*8(a0)
	ldq	t0,EF_AT*8(sp)
	ldq	t1,EF_SP*8(sp)
	ldq	t2,EF_PS*8(sp)
	ldq	t3,EF_PC*8(sp)
	ldq	t4,EF_GP*8(sp)
	ldq	t5,EF_A0*8(sp)
	ldq	t6,EF_A1*8(sp)
	ldq	t7,EF_A2*8(sp)
	stq	t0,PCB_AT*8(a0)
	stq	t1,PCB_SP*8(a0)
	stq	t2,PCB_PS*8(a0)
	stq	t3,PCB_PC*8(a0)
	stq	t4,PCB_GP*8(a0)
	stq	t5,PCB_A0*8(a0)
	stq	t6,PCB_A1*8(a0)
	stq	t7,PCB_A2*8(a0)
4:

	/*
	 * Attempt a system memory dump.
	 */
	addl	zero,PROMPRINT,t0	# switch back to prom callbacks
	stl	t0,printstate		#   for kernel printf output
	addl	zero,1,t1		# also set global shutting down
	stl	t1,shutting_down	#   flag for device drivers
	jsr	ra,dumpsys 		# invoke system dump handling

	/*
	 * Halt the operating system.  Note that if doadump was invoked
	 * through the restoreterm vector, our exit path needs to disable
	 * the "continue" operation.
	 */
5:
	ldq	t0,EF_SIZE(sp)		# fetch saved proc desc data
	lda	sp,EF_SIZE+8(sp)	# deallocate our stack space
	beq	t0,6f			# just halt if not callback
	and	t0,1,t1			# isolate restoreterm flag
	beq	t1,6f			# just halt if not rstrterm
#if 0
	bic	t0,7,t0			# clear low-order flag bits
	ldq	t1,RPB_STATE(t0)	# fetch percpu state flags
	ldiq	t2,STATE_HALT_MASK	# get halt requested mask
	ldiq	t3,STATE_SVRS_TERM	# get save/rstr-term code
	bic	t1,t2,t1		#
	bis	t1,t3,t1		#
	stq	t1,RPB_STATE(t0)	# reassign halt request
#endif
	ornot	zero,zero,v0		# prevent o/s continue
6:
	call_pal PAL_halt		# halt and never return
	br	zero,6b			# (but just in case)

	.end	doadump

LEAF(getpc)
        bis     zero,ra,v0
        ret     zero,(ra)
END(getpc)

LEAF(getra)
        ldq     v0, 0(sp)
        ret     zero,(ra)
END(getra)

#if RT_SCHED
LEAF(find_first_runq_bit_set)
	ldq	a0, 0(a0)		# load the  mask
	bis	zero, zero, v0		# indexes are 0-based

	subq	zero, a0, t0		# complement all but first bit
	beq	a0, 20f			# quit now if no bits set

	and	a0, t0, t0		# clear all but first bit
	blbs	a0, 10f			# take an easy out

	extll	t0, 4, t1		# get high longword
	addq	v0, 32, t2		# adjust index assuming in high lw
	cmovne	t1, t1, t0		# if yes, replace bits
	cmovne	t1, t2, v0		# if yes, replace index

	extwl	t0, 2, t1		# get high word
	addq	v0, 16, t2		# adjust index assuming in high word
	cmovne	t1, t1, t0		# if yes, replace bits
	cmovne	t1, t2, v0		# if yes, replace index

	extbl	t0, 1, t1		# get high byte of low word
	addq	v0, 8, t2		# adjust index assuming in high byte
	cmovne	t1, t1, t0		# if yes, replace bits
	cmovne	t1, t2, v0		# if yes, replace index

	and	t0, 0x0f, t2		# null in bytes 0..3?
	addq	v0, 4, t1		# adjust index assuming no
	cmoveq	t2, t1, v0		# if no, replace index

	and	t0, 0x33, t3		# null in bytes 0..1, or 4..5?
	addq	v0, 2, t1		# adjust index assuming no
	cmoveq	t3, t1, v0		# if no, replace index

	and	t0, 0x55, t4		# null in bytes 0, 2, 4 of 6
	addq	v0, 1, t1		# adjust index assuming no
	cmoveq	t4, t1, v0		# if no, replace index

10:	RET

	.align	3
20:	addq	zero, 63, v0		# no bits set
	RET

END(find_first_runq_bit_set)

#endif /* RT_SCHED */
