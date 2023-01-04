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
 *	@(#)$RCSfile: locore.s,v $ $Revision: 1.2.3.6 $ (DEC) $Date: 1992/06/03 11:08:11 $	
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
 * Derived from locore.s	4.7        (ULTRIX)        3/6/91
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/************************************************************************
 *
 *	Modification History: locore.s
 *
 *   9-Nov-91	Darrell Dunnuck
 *	Added in global pointer and startup stack into doadump, and
 *	put the rex_console_init entry point back in.
 *
 *   6-Nov-91	Heather Gray
 *	New in_checksum().
 *
 *   17-Sep-91	Ron Widyono
 *	Fix code that generates warning assembler messages.  Code is in
 *	exception_exit and runq_update.
 *
 *   3-Sep-91	Ron Widyono
 *	Leave interrupts disabled when processing kernel-mode ASTs.
 *
 *   10-may-91 - Paul Grist
 *	3max+ support: kn03_wbflush, IBE/DBE work-around, splx PE fix.
 *
 *   7-May-91	Peter H. Smith
 *	Add some optimized scheduler support code.  This will go away if the
 *	MIPSCO compiler does a better job with the "c" versions of these
 *	routines.
 *	Added issplsched() function for debugging.
 *
 *   7-May-91	Ron Widyono
 *	Implement Kernel Mode ASTs.
 *	isipl0() function for preemption points.
 *
 *   26-apr-91 - burns
 *	3min support.
 *
 *   19-apr-91 - burns
 *	New spl code, OSF regleak fix.
 *
 *   21-Dec-90	-- Don Dutile
 *	Merge v4.ti to previously merged osc25/v4.pu (by Mr. Bill...oh nooooo)
 */

#define CNT_TLBMISS_HACK 0

#include <mach_emulation.h>
#include <mach_kdb.h>
#include <rt_preempt.h>
#include <rt_sched.h>
#include <rt_sched_opt.h>

#if	RT_PREEMPT
#define _SKIP_PREEMPT_H_
#endif

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/fpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <mach/machine/vm_param.h>
#include <machine/pmap.h>
#include <machine/thread.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <mach/kern_return.h>
#include <hal/entrypt.h>	/* prom entry point definitions */
#include <machine/inst.h>			/* tas emulation */
#ifdef doxpr
#include <kern/xpr.h>
#endif  /* doxpr */
#include "assym.s"
#include <kern/sched.h>
#include <cpus.h>

#ifdef DS5800
#include <hal/cpuconf.h>
#endif

#ifdef DS5000_100
#include <hal/kn02ba.h>
IMPORT(mips_spl_arch_type,4)
IMPORT(ipllevel,4)
IMPORT(splm,SPLMSIZE*4)
IMPORT(kn02ba_sim,SPLMSIZE*4)
#endif /* DS5000_100 */

/* TODO: begin stuff from OSF that is questionalble
 * at the very least SMP work is needed here and may be why
 * we do not have these items. */

#define	DEBUG		0



/*
 * 	Current owner of FP coprocessor, if any
 */
	BSS(fpowner_array, NCPUS*4)

/*
 *	Debugging aids
 */
#if	DEBUG
	ABS(crash_area, E_VEC-20)
	BSS(crash_saved_registers, 160)
#endif

/* End stuff from OSF that is questionable */

/*
 * Misc. kernel entry points
 */
EXPORT(_coredump)
	j	doadump

/* TODO: xprdump, xprtail and msgdump ifdef'd out */
#ifdef doxpr
EXPORT(_xprdump)
	jal	xprdump
	lw	a0,rex_magicid
	li	a1,0x30464354
	bne	a0,a1,1f
	li	a0,0x68
	j	rex_rex
1:	j	prom_restart

EXPORT(_xprtail)
	jal	xprtail
	lw	a0,rex_magicid
	li	a1,0x30464354
	bne	a0,a1,1f
	li	a0,0x68
	j	rex_rex
1:	j	prom_restart

EXPORT(_msgdump)
#ifndef ultrix
	jal	msgdump
#endif /* not ultrix */
	lw	a0,rex_magicid
	li	a1,0x30464354
	bne	a0,a1,1f
	li	a0,0x68
	j	rex_rex
1:	j	prom_restart
#endif /* doxpr */

/*
 * do a dump and then perform power-up sequence
 * called on warm starts
 */
EXPORT(doadump)
	la	gp,_gp

	.set noreorder
	nop
	mtc0	zero,C0_TLBHI
	nop
	li	v0,KPTEADDR
	nop
	mtc0	v0,C0_CTXT
	nop
	.set reorder

	li	a3,0x7ffff000
	la	a1,pcb_initial_space	# mips/entry.s
	addiu	a1,(NBPG-1)
	and	a1,a3,a1

	ori	a1,0x700
	li      a2,VM_MIN_KERNEL_ADDRESS-(2*NBPG)
	li	a3,TLBWIRED_KSTACK<<TLBINX_INXSHIFT
	.set 	noreorder
	mtc0	a3,C0_INX
	mtc0	a1,C0_TLBLO
	mtc0	a2,C0_TLBHI
	nop
	c0	C0_WRITEI
	nop
       	.set	reorder
	li      a3,0x7ffff000
        la      a1,pcb_initial_space    # mips/entry.s
        addiu   a1,((2*NBPG)-1)
        and     a1,a3,a1
        ori     a1,0x700
        li      a2,VM_MIN_KERNEL_ADDRESS-(1*NBPG)
        li      a3,TLBWIRED_KSTACK1<<TLBINX_INXSHIFT
        .set    noreorder
        mtc0    a3,C0_INX
        mtc0    a1,C0_TLBLO
        mtc0    a2,C0_TLBHI
        nop
        c0      C0_WRITEI
        nop
	.set	reorder
        li      sp,PCB_WIRED_ADDRESS-EF_SIZE # switch stack
	li	k0,PCB_WIRED_ADDRESS	# address of PCB
	sw      gp,PCB_KSTACK(k0)	# running on kernel stack now

	jal	dumpsetupvectors	# load exception vector code
	jal	dumpsys			# do the dump

	lw	a0,rex_magicid
	li	a1,REX_MAGIC
	bne	a0,a1,1f
	li	a0,0x62
	j	rex_rex
1:	j	prom_reboot


/*
 *	Exception vectors.
 *
 *	The machine really only knows to trap to either utlbmiss()
 *	or exception(). The latter dispatches to the appropriate
 *	vectors through the causevec[] table.
 *
 * The machine state (registers) must be managed carefully by this code
 * in order to prevent an exception (such as a hardware interrupt) from
 * disturbing the calculations in progress, or leaking information across
 * contexts.
 * utlbmiss only references k0 & k1, so does not save or restore any other
 * state.
 *
 * Register Number,			Saved By	Restored By
 * name and usage)			Routine		Routine
 * -------------------------------	--------	-----------
 * $0 (zero) hardware constant 0	N/A		N/A
 * $1 (at) assembler temporary		exception	exception_exit
 * $2,$3 (v0,v1) procedure results	save_frame	restore_frame
 * $4-$7 (a0-a3) procedure args		exception	exception_exit
 * $8-$15,$24,$25 (t0-t9) temp		save_frame	restore_frame
 * $16-$23 (s0-s7) callee saved		preserved by calling conventions
 * $26,$27 (k0,k1) kernel reserved	destroyed by all exceptions
 * $28 (gp) global data pointer		exception	exception_exit
 * $29 (sp) stack pointer		exception	exception_exit
 * $30 (s8 or fp) frame pointer		preserved by calling conventions
 * $31 (ra) return address		exception	exception_exit
 * hi,lo multiplier results		save_frame	restore_frame
 *
 * The registers marked as saved by 'exception' are used in the
 * general exception processing, so are saved and restored by the
 * common code. Registers whose values are preserved by the procedure
 * call convention and are not modified by the exception code do
 * not need to be treated specially here -- they will be handled by
 * the context switch code. Registers whose values are not preserved
 * across procedure calls must be saved by save_frame before dispatching to
 * the high-level exception handling code. The system call trap optimizes
 * the preservation of these registers since it is a form of procedure
 * call itself -- the contents are simply set to zero before returning
 * from the exception.
 *
 * After any exception, k0 will contain 0, and k1 will contain the
 * exception restart PC. (usually the exception location, but if 
 * instruction emulation or branch delay slot things occur, it may differ)
 */

/*
 * Deal with tlbmisses in KUSEG
 */
NESTED(utlbmiss, 0, k1)			# Copied down to 0x80000000
	.set	noreorder
 	.set	noat
	/*
	 * We cannot lose the information about what type of
	 * fault it is, in Mach.  BSD is simple minded and can.
	 * [Yes, we could play guessing games. But it would be much
	 *  more expensive than this]
	 */
	mfc0	k0,C0_CAUSE

        li      k1,PCB_WIRED_ADDRESS
	sw      k0,PCB_MIPS_USER_FAULT(k1)      #save mips_user_fault
	mfc0	k0,C0_CTXT
	mfc0	k1,C0_EPC
	lw	k0,0(k0)
	nop
	mtc0	k0,C0_TLBLO
	move	k0,zero
	tlbwr
	j	k1
	rfe
EXPORT(eutlbmiss)
 	.set	at
	.set	reorder
	END(utlbmiss)

/*
 * General exception entry point.
 */
#ifdef ASM_FIXED
#define	M_EXCEPT	+(M_SP|M_GP|M_AT|M_K1|M_A0|M_A1|M_A2|M_A3|M_S0|M_RA)
#define	M_TFISAVE	+(M_V0|M_V1|M_T0|M_T1|M_T2|M_T3|M_T4|M_T5|M_T6|\
			M_T7|M_T8|M_T9)
#else
#define	M_EXCEPT	0xb80100f3
#define	M_TFISAVE	0x4300ff0d
#define	M_EXCSAVE	0xfb01ffff
#define	M_TRAPSAVE	0xfbffffff
#define	M_SYSCALLSAVE	0xf80100ff
#endif

IMPORT(cpu,4)

VECTOR(exception, M_EXCEPT)	# Copied down to 0x80000080
	.set	noreorder
 	.set	noat
        li      k0,PCB_WIRED_ADDRESS    # address of PCB
        lw      k0,PCB_KSTACK(k0)       # get kstack flag
	nop
	beq	k0,zero,from_user_mode
	move	k0,AT			# BDSLOT: save at
#if	DEBUG
	li	AT,crash_area
	sw	k0,4(AT)
	lw	k0,0(AT)
	sw	k1,8(AT)
	addi	k0,1
	slt	k1,k0,32		# max nested exceptions
	bne	k1,zero,nested_ok	# still within limits ?
	sw	k0,0(AT)
					# nope
	lw	k1,8(AT)
	lw	k0,4(AT)
	sw	zero,0(AT)
	la	AT,crashit
	j	AT
	nop
nested_ok:
	lw	k0,4(AT)
	lw	k1,8(AT)
#endif
	.set	reorder
        /**** CHECK kernel stack *****/
        li      AT,PCB_WIRED_ADDRESS
        subu    AT,sp,AT
        blez    AT,exception_stack_high_ok
        j      	exception_saveoff 

exception_stack_high_ok:
        li      AT,VM_MIN_KERNEL_ADDRESS-KERNEL_STACK_SIZE+EF_SIZE
        subu    AT,sp,AT
        bgtz    AT,exception_stack_ok
	
	j	exception_saveoff
        /***** end  CHECK kernel stack *****/

 	.set	at

exception_stack_ok:
	sw	sp,EF_SP*4-EF_SIZE(sp)
	subu	sp,EF_SIZE
	b	2f

	/*
	 * Came from user mode or utlbmiss, initialize kernel stack
	 */
	/* OSF comment:
	 * Came from user mode, switch to kernel stack.
	 * load_context() takes care of setting up
	 * for us the right thing in the right place
	 */
	.set	noreorder
 	.set	noat
from_user_mode:
        li      AT,PCB_WIRED_ADDRESS-EF_SIZE
	/*
	 * If the kernel stack is not wired here we lose
	 */
	sw	sp,EF_SP*4(AT)		# save the user's one
	move	sp,AT			# get the kernel's one

	sw	gp,EF_GP*4(sp)

        li      AT,PCB_WIRED_ADDRESS    # address of PCB
        sw      sp,PCB_KSTACK(AT)       # now on kernel stack (sp != 0)
	
	.set	at
	la	gp,_gp
	/*
	 * Save registers on stack
	 */
2:	sw	a0,EF_A0*4(sp)
	sw	k0,EF_AT*4(sp)
	mfc0	a0,C0_EPC
	sw	k1,EF_K1*4(sp)		# in case we came from utlbmiss
	sw	a3,EF_A3*4(sp)
	sw	a1,EF_A1*4(sp)
	mfc0	a3,C0_CAUSE
#ifdef DS5000_100
/* For generic kernels, need the following (extra) code */
#ifdef MIPS_LIBRARY
	  /*
	   * If on 3MIN save ipllevel also
	   */
	lw	a1,mips_spl_arch_type
	nop
	beq	a1,zero,1f
	nop
#endif /* MIPS_LIBRARY */
	lw	a1,ipllevel
	nop
	sw	a1,EF_SYS1*4(sp)
	/* End 3MIN special */
#endif /* DS5000_100 */
1:
	sw	a2,EF_A2*4(sp)
	sw	s0,EF_S0*4(sp)
	sw	ra,EF_RA*4(sp)
#if	DEBUG
	lui	s0,0x0bad
	or	s0,a3,s0			# just a sentinel
	sw	s0,EF_CAUSE*4(sp)
	nop
#else
	sw	a3,EF_CAUSE*4(sp)
#endif
	mfc0	s0,C0_SR
	sw	a0,EF_EPC*4(sp)

#ifdef doxpr
#ifdef XPRBUG
	lw	a2,xpr_flags
	and	a2,XPR_INTR
	beq	a2,zero,1f
	jal	tfi_save
	subu	sp,10*4
	la	a0,9f
	lw	a1,EF_CAUSE*4(sp)
	la	a2,cause_desc
	move	a3,s0
	la	v0,sr_desc
	sw	v0,4*4(sp)
	jal	xprintf
	MSG("exception cause=%r sr=%r")
	la	a0,9f
	move	a1,sp
	lw	a2,EF_EPC*4(sp)
	jal	xprintf
	MSG("exception sp=0x%x pc=0x%x")
	addu	sp,10*4
	jal	tfi_restore
	lw	a3,EF_CAUSE*4(sp)
1:
#endif /* XPRBUG */
#endif /* doxpr */

	/*
	 * Dispatch to appropriate exception handler
	 * Register setup:
	 *	s0 -- SR register at time of exception
	 *	a0 -- exception frame pointer
	 *	a1 -- cause code
	 *	a3 -- cause register
	 */
	and	a1,a3,CAUSE_EXCMASK
	lw	a2,causevec(a1)
	move	a0,sp
	.set	noreorder
	j	a2
	sw	s0,EF_SR*4(sp)		# should clear PE in s0 after here
EXPORT(eexception)
	nop
	.set	reorder			# pretty up kdb
	END(exception)
	.data

/*
 * The following routine puts away enough information to allow a
 * kernel stack overflow to be debug.  We set the panicstr to make
 * this case easy to figure out...though this is not a "real" panic
 * because we never call it.
 *
 * To get a stack trace from a crash, use the pc to figure out the
 * frame size of the routine we were in.  Next assign $pc=overflow_ra
 * and $sp=overflow_sp-FRAME_SIZE. The frame pointer is saved off if 
 * all else fails.
 */ 
overflow_panic:
	.asciiz	"kernel stack overflow"
	.text

BSS(overflow_ra,4)
BSS(overflow_sp,4)
BSS(overflow_epc,4)
BSS(overflow_fp,4)
LEAF(exception_saveoff)
	la	t0,overflow_fp
	sw	fp,0(t0)
	la	t0,overflow_ra
	sw	ra,0(t0)
	la	t0,overflow_sp
	sw	sp,0(t0)
	la	t0,overflow_epc
	.set	noreorder
	mfc0	t1,C0_EPC
	nop
	.set	reorder
	sw	t1,0(t0)
	la	t0,overflow_panic
	la	t1,panicstr
	sw	t0,0(t1)

	j	doadump
	END(exception_saveoff)
/*
 * VEC_int -- interrupt handler
 */
#ifdef ASM_FIXED
VECTOR(VEC_int, M_EXCEPT|M_TFISAVE)
#else
VECTOR(VEC_int, M_EXCSAVE)
#endif
	jal	save_frame
	.set noreorder
	li	a2,SR_IEC|SR_IMASK8	# enable, but mask all interrupts
	mtc0	a2,C0_SR
#ifdef DS5800
	.set reorder
	/* For ISIS halt interrupt we want to go directly to the boot rom. */
	lw	v0, cpu			# get cpu
	bne	v0, DS_5800, 1f		# see if 5800
	andi	a2,a3,CAUSE_IP7 	# check cause for halt intr
	beq	a2,zero,1f		# no halt, skip to check for FP intr
	lw	v0,EF_V0*4(sp)		# restore v0 for isis halt path
	jal	prom_halt		# Call rom halt routine
	.set noreorder
	nop
	b	2f			# Let's get outta here.
	nop
#endif	/* DS5800 */
	.set reorder
	/*
	 * If this is a floating-point interrupt then we may need "all" the
	 * user's register values in case we need to emulate an branch
	 * instruction if we are in a branch delay slot.
	 */
1:
	andi	a2,a3,CAUSE_IP8		# check cause for possible fp intr
	beq	a2,zero,1f
	sw	s1,EF_S1*4(sp)
	sw	s2,EF_S2*4(sp)
	sw	s3,EF_S3*4(sp)
	sw	s4,EF_S4*4(sp)
	sw	s5,EF_S5*4(sp)
	sw	s6,EF_S6*4(sp)
	sw	s7,EF_S7*4(sp)
	sw	fp,EF_FP*4(sp)
1:
#ifdef	DS5000_100
/* For generic kernels, need the following (extra) code */
#ifdef	MIPS_LIBRARY
	/* 3MIN is a special case */
	lw	a2,mips_spl_arch_type
	beq	a2,zero,1f
#endif	/* MIPS_LIBRARY */
	move	a2,s0			# sr is arg3
        jal     kn02ba_intr
	b	2f
	/* End 3MIN special */
#endif	/* DS5000_100   */
1:
	move	a2,s0			# sr is arg3
	jal	kn01_intr

2:
	jal	restore_frame
	.set noreorder
	mtc0	s0,C0_SR		# disable interrupts
	.set reorder
	b	exception_exit
	END(VEC_int)

/*
 * TLB mod.
 * Could enable interrupts here if we were so inclined....
 */
#ifdef ASM_FIXED
VECTOR(VEC_tlbmod, M_EXCEPT|M_TFISAVE)
#else
VECTOR(VEC_tlbmod, M_EXCSAVE)
#endif
	.set noreorder
	mfc0	a2,C0_BADVADDR		# arg3 is bad vaddr
	nop
	.set reorder
	sw	a2,EF_BADVADDR*4(sp)	# save in case of trap (ugh!)
	jal	save_frame
	jal	tlbmod			# tlbmod(ef_ptr, code, vaddr, cause)

#ifdef DS5000_100
	.set noreorder
	mtc0	s0, C0_SR		# restore status register
	.set reorder
#endif	/* DS5000_100 */

	la	ra,exception_exit	# fake jal with less nops
					# _if we had reloc-reloc, 1 cycle
	beq	v0,zero,restore_frame	# zero if legal to modify
	or	a0,s0,SR_IEC		# enable interrupts
	.set noreorder
	mtc0	a0,C0_SR
	.set reorder
	move	a1,v0			# move software exception code
	move	a0,sp			# restore ep since tlbmod can trash
	lw	a2,EF_BADVADDR*4(sp)	# restore since tlbmod can trash - from OSF
	lw	a3,EF_CAUSE*4(sp)	# restore cause since tlbmod can trash
	b	soft_trap		# and handle as trap
	END(VEC_tlbmod)

/*
 * TLB miss. 
 * Handles TLBMiss Read and TLBMiss Write
 * Could enable interrupts here if we were so inclined....
 */
#ifdef ASM_FIXED
VECTOR(VEC_tlbmiss, M_EXCEPT|M_TFISAVE)
#else
VECTOR(VEC_tlbmiss, M_EXCSAVE)
#endif
	.set noreorder
	mfc0	a2,C0_BADVADDR		# arg3 is bad vaddr
	nop
	.set reorder
	sw	a2,EF_BADVADDR*4(sp)	# save in case of trap (ugh!)
	li      a3,PCB_WIRED_ADDRESS    # address of PCB
	lw      a3,PCB_MIPS_USER_FAULT(a3) # in case we came from utlbmiss
	jal	save_frame
	jal	tlbmiss			# tlbmiss(ef_ptr, code, vaddr, cause)
	lw	s0,EF_SR*4(sp)		# tlbmiss can alter return SR

#ifdef DS5000_100
	.set noreorder
	nop
	mtc0	s0, C0_SR		# restore status register
	.set reorder
#endif /* DS5000_100 */

	beq	v0,zero,1f		# zero if accessable
	or	a0,s0,SR_IEC		# enable interrupts
	.set noreorder
	mtc0	a0,C0_SR
	.set reorder
	move	a0,sp			# restore ep since tlbmiss can trash
	move	a1,v0			# software exception code
	lw	a3,EF_CAUSE*4(sp)	# restore cause since tlbmiss can trash
	b	soft_trap		# handle as trap

1:	la	ra,exception_exit	# 2 cycles, but 1 fills delay slot
	b	restore_frame
	END(VEC_tlbmiss)

/*
 * VEC_addrerr
 * Handles AdrErrRead, AdrErrWrite
 */
VECTOR(VEC_addrerr, M_EXCEPT)
	.set noreorder
	nop
	mfc0	a2,C0_BADVADDR
	nop
	.set reorder
	sw	a2,EF_BADVADDR*4(sp)
	b	VEC_trap
	END(VEC_addrerr)

/*
 * VEC_ibe -- Instruction fetch bus error.
 * Handles Instruction Bus Errors
 */
VECTOR(VEC_ibe, M_EXCEPT)
	.set noreorder
	nop
	mfc0	a2,C0_EPC
	nop
	mfc0	ra,C0_CAUSE
	nop
	.set reorder
	bgez	ra,1f		# BD bit not set
	addu	a2,4		# point at BD slot
1:	sw	a2,EF_BADVADDR*4(sp) # ibe's occur at pc
	jal	clr_be_intr	# Go clear any pending interrupt 
	b	VEC_trap
	END(VEC_ibe)

/*
 * VEC_dbe
 * Handles Data Bus Errors
 *
 * Trap will calculate appropriate badvaddr
 */
VECTOR(VEC_dbe, M_EXCEPT)
	jal     clr_be_intr	# Go clear any pending interrupt 
	b	VEC_trap
	END(VEC_dbe)

/*
 *	VEC_ill_instr
 */
VECTOR(VEC_ill_instr, M_EXCEPT)
	/*
	 * Check for a test-and-set pseudo instruction 
	 */
	lw	ra,EF_EPC*4(sp)		# use (a0,a2,ra) as scratch
	li	a2,tas_op
	lw	a0,0(ra)		# this cannot miss, right ?
	bne	a0,a2,truly_ill
	/*
	 * Hey, a TAS instruction!
	 *
	 * Emulate it, keeping interrupts disabled should
	 * do the trick.
	 */
	lw	a0,EF_A0*4(sp)		# get address of lock
	/* Protect against malicious uses  */
	bgez	a0,addr_ok		# sneaking into kernel space ?
	sw	a0,EF_BADVADDR*4(sp)	# fake an Ade exception
	li	a1,SEXC_ILL
	b	truly_ill		# proceed to trap()
addr_ok:
	/*
	 * Use the nofault trick.  If we fault on a bad
	 * user address we'll endup (by magic) in uaerror below
	 */
        li      a1,PCB_WIRED_ADDRESS
	li	a2,NF_USERACC
	sw	a2,PCB_NOFAULT(a1)
	/*
	 * Now it should be safe to do it.
	 *
	 * There is a subtle semantic question about a tas
	 * if it fails: does it actually write or not ?
	 * For instance, should a user that does a tas
	 * on e.g. his text segment get a bus error or not ?
	 * I think he should.
	 */
	lw	a2,0(a0)		# 	.. TEST ..
#ifdef	dont_do_the_write_cycle
	bne	a2,zero,1f		# 	.. AND ..
	sw	a0,0(a0)		# 	.. SET
#else
	sw	a0,0(a0)		# 	.. AND SET
#endif
1:	sw	a2,EF_A0*4(sp)		# return the result still in a0
	sw	zero,PCB_NOFAULT(a1)
	/*
	 * All done, increment PC and return
	 */
	addiu	ra,4			# increment PC
	sw	ra,EF_EPC*4(sp)
	b	exception_exit
truly_ill:
	move	a0,sp
	b	VEC_trap
	END(VEC_ill_instr)

LEAF(uaerror)
	/*
	 * User gave a bad address.  Give him a SEGV in trap()
	 * that stems from a write miss.
	 */
	li	a1,SEXC_SEGV
	li	a3,EXC_WMISS
	sw	a3,EF_CAUSE*4(sp)
	b	truly_ill
	END(uaerror)

/*
 * TRAP
 * Illegal Instruction, and Overflow.
 * Also handles software exceptions raised by tlbmod and tlbmiss,
 * NOTE: tlbmod and tlbmiss replace the original exception code with
 * an appropriate software exception code.
 */
#define	M_TRAP		+(M_S1|M_S2|M_S3|M_S4|M_S5|M_S6|M_S7)
#ifdef ASM_FIXED
VECTOR(VEC_trap, M_EXCEPT|M_TFISAVE|M_TRAP)
#else
VECTOR(VEC_trap, M_TRAPSAVE)
#endif
	.set noreorder
	jal	save_frame
	nop
	or	a2,s0,SR_IEC		# enable interrupts
	mtc0	a2,C0_SR
	.set reorder
soft_trap:				# (from tlbmod / tlbmiss)
	/*
	 * Save rest of state for debuggers
	 * ENTRY CONDITIONS: interrupts enabled, a1 contains software
	 * exception code
	 */
	sw	s1,EF_S1*4(sp)
	sw	s2,EF_S2*4(sp)
	sw	s3,EF_S3*4(sp)
	move	a2,s0
	sw	s4,EF_S4*4(sp)
	sw	s5,EF_S5*4(sp)
	sw	s6,EF_S6*4(sp)
	sw	s7,EF_S7*4(sp)
	sw	fp,EF_FP*4(sp)		# aka s8
	jal	trap			# trap(ef_ptr, code, sr, cause)
full_restore:
	lw	s1,EF_S1*4(sp)
	lw	s2,EF_S2*4(sp)
	lw	s3,EF_S3*4(sp)
	lw	s4,EF_S4*4(sp)
	lw	s5,EF_S5*4(sp)
	lw	s6,EF_S6*4(sp)
	lw	s7,EF_S7*4(sp)
	lw	fp,EF_FP*4(sp)
	jal	restore_frame
	.set noreorder
	mtc0	s0,C0_SR		# disable interrupts
	.set reorder
	b	exception_exit
	END(VEC_trap)

/*
 * VEC_nofault -- handles nofault exceptions early on in system initialization
 * before VEC_trap is usable.
 */
#ifdef ASM_FIXED
VECTOR(VEC_nofault, M_EXCEPT|M_TFISAVE)
#else
VECTOR(VEC_nofault, M_EXCSAVE)
#endif
	jal	save_frame
	move	a2,s0
	jal	trap_nofault		# trap_nofault(ef_ptr, code, sr, cause)
	jal	restore_frame

#ifdef	DS5000_100
	.set noreorder
	mtc0	s0, C0_SR		# restore status register
	.set reorder
#endif	/* DS5000_100 */

	b	exception_exit
	END(VEC_nofault)

/*
 * Syscall
 * NOTE: v0, and, v1 must get restored on exit from syscall!!
 */
#define	M_SYSCALL	+(M_V0|M_V1)
#ifdef ASM_FIXED
VECTOR(VEC_syscall, M_EXCEPT|M_SYSCALL)
#else
VECTOR(VEC_syscall, M_SYSCALLSAVE)
#endif
	blt	v0,zero,mach_trap	# a Mach trap ?
					#
#if	MACH_EMULATION
	/*
	 *	User space syscall emulation
	 */
#if     NCPUS > 1
        li      a2,PCB_WIRED_ADDRESS
        lw      a2,PCB_CPU_NUMBER(a2)
        la      a1,active_threads
        sll     a2,2                    # *(sizeof caddr)
        addu    a2,a1,a2
        lw      a2,0(a2)                # >>active_threads<<
#else
        lw      a2,active_threads       # only 1 cpu....
#endif
	lw	a2,THREAD_TASK(a2)	# ->task
	lw	a2,EML_DISPATCH(a2)	# ->eml_dispatch
	beq	a2,zero,normal_syscall	# no emulation
	lw	a1,DISP_COUNT(a2)
	bge	v0,a1,normal_syscall	# not emulated, or invalid
	sll	a1,v0,2			# offset into vector
	addu	a1,a2
	lw	a1,DISP_VECTOR(a1)
	beq	a1,zero,normal_syscall	# not this one
	/*
	 * Ok, we finally found out. Now it's easy: just fix 
	 * the PCs and return!
	 */
	lw	v0,EF_EPC*4(sp)		# PC to return to after emulation
	addu	v0,4
	sw	a1,EF_EPC*4(sp)		# PC to return to for emulation
	b	exception_exit
normal_syscall:
#endif
	or	a2,s0,SR_IEC		# enable interrupts
	.set noreorder
	nop
	mtc0	a2,C0_SR
	.set reorder
	sw	v0,EF_V0*4(sp)		# u_rval1
	sw	v1,EF_V1*4(sp)		# u_rval2
	/*
	 * U*x forking takes a special exit route and needs a full
	 * register save/restore.  Yes, this is a kludge.
	 */
	beq	v0,SYS_fork,1f
	bne	v0,SYS_vfork,2f
1:	jal	save_entire_frame

2:	move	a1,v0			# arg2 -- syscall number
	jal	syscall			# syscall(ef_ptr, sysnum, sr, cause)
	bne	v0,zero,full_restore	# doing a sigreturn
	move	t0,zero			# overwrite registers that are
	move	t1,zero			# not saved & restored by either
	move	t2,zero			# exception/exception_exit or the
	move	t3,zero			# C calling conventions. This
	move	t4,zero			# prevents information from leaking
	move	t5,zero			# from thread => thread and from the
	move	t6,zero			# kernel to user space.
	move	t7,zero
	move	t8,zero
	move	t9,zero
	mthi	zero
	mtlo	zero
	lw	v0,EF_V0*4(sp)		# u_rval1
	lw	v1,EF_V1*4(sp)		# u_rval2
	.set noreorder
	mtc0	s0,C0_SR		# disable interrupts
	.set reorder
	b	exception_exit

/*
 *	Mach traps are simpler, so we dispatch them here
 *
 *	Note: the following code lets the assembler do the necessary
 *	optimizations.  Before turning on .noreorder or such
 *	a good deal of restructuring is in order.
 */
	.extern	mach_trap_table
	IMPORT(mach_trap_count,4)

mach_trap:
	or	a1,s0,SR_IEC		# enable interrupts
	.set noreorder
	mtc0	a1,C0_SR
	.set reorder
	lw	a0,EF_EPC*4(sp)		# advance user's pc: no fancy
	addu	a0,4			#   branch delay syscalls please,
	sw	a0,EF_EPC*4(sp)		#   or you'll just lose.
	negu	v0			# get the id right
	lw	a1,mach_trap_count
	bleu	v0,a1,1f
					# invalid trap
	li	v0,KERN_FAILURE
	b	mach_trap_return
1:
	/*
	 *	Load trap descriptor, and check the number of args.
	 *	Entries in the table are defined by:
	 * 		typedef struct {
	 *			short		mach_trap_length;
	 *			short		mach_trap_flags;
	 *			int		(*mach_trap_function)();
	 * 		} mach_trap_t;
	 */
	sll	v0,3			# sizeof struct mach_trap_t
	la	s0,mach_trap_table
	addu	s0,v0			# get trap
	lh	a0,0(s0)		# trap->mach_trap_length
	lw	v0,4(s0)		# trap->mach_trap_function
	srl	a0,2			# nargs+1
	lw	a1,EF_A1*4(sp)
	lw	a2,EF_A2*4(sp)
	lw	a3,EF_A3*4(sp)
	.set	noat			# assembler not smart enough
	sltiu	AT,a0,6			# more than 4 args ?
	lw	a0,EF_A0*4(sp)
	bne	AT,zero,1f
	.set	at
	/*
	 * Warning: if we ever define traps with more than 6 args
	 * the following needs to be changed appropriately, just
	 * like the RT.
	 */
	subu	sp,8			# uhhmm, > 4 args
	sw	t0,16(sp)		# extra args in tN
	sw	t1,20(sp)
	jal	v0			# do the call
	addu	sp,8
	b	2f	
1:
	jal	v0			# do the call
2:
#if     NCPUS > 1
        li      a0,PCB_WIRED_ADDRESS
        lw      a0,PCB_CPU_NUMBER(a0)
        la      a1,active_threads
        sll     a0,2                    # *(sizeof caddr)
        addu    a0,a1,a0
        lw      a0,0(a0)                # >>active_threads<<
#else
        lw      a0,active_threads       # only 1 cpu....
#endif
	lw	s0,EF_SR*4(sp)		# restore sr

#define mach_traps_check_signals 1
#if	mach_traps_check_signals
/* This should be CHECK_SIGNALS, but isn't */
	lw	a1,UTASK(a0)
	lw	a1,U_PROCP(a1)
	beq	a1,zero,mach_trap_return
	lbu	a2,P_CURSIG(a1)
	lw	a1,P_SIG(a1)
	or	a1,a2
	beq	a1,zero,mach_trap_return
	sw	sp,need_ast		# sp always non zero

#endif

mach_trap_return:
	move	t0,zero			# overwrite registers that are
	move	t1,zero			# not saved & restored by either
	move	t2,zero			# exception/exception_exit or the
	move	t3,zero			# C calling conventions. This
	move	t4,zero			# prevents information from leaking
	move	t5,zero			# from thread => thread and from the
	move	t6,zero			# kernel to user space.
	move	t7,zero
	move	t8,zero
	move	t9,zero
	mthi	zero
	mtlo	zero
	move	v1,zero
	.set noreorder
	mtc0	s0,C0_SR		# disable interrupts
	.set reorder
	b	exception_exit

	END(VEC_syscall)

/*
 * Breakpoint -- determine if breakpoint is for prom monitor, else
 * call trap.
 */
VECTOR(VEC_breakpoint, M_EXCEPT)
	.set noreorder
	nop
	mfc0	k1,C0_CAUSE
	nop
	.set reorder
	lw	k0,EF_EPC*4(sp)
	and	k1,CAUSE_BD
	beq	k1,zero,1f
	addu	k0,4				# advance pc to bdslot
1:	lw	k0,0(k0)			# read faulting instruction
	and	k1,s0,SR_KUP			# if from use mode
	bne	k1,zero,2f			# kernel break not allowed
	lw	k1,kernelbp			# what a kernel bp looks like
	bne	k0,k1,2f			# not a kernel bp inst
	lw	k0,+RB_BPADDR			# address of breakpoint handler
	bne	k0,zero,4f
2:
	/*
	 * Check to see if there is a branch delay slot emulation taking place
	 * which is indicated by a non-zero value in PCB_BD_RA (left there by
	 * emulate_instr() ).  If this is the case go on to check for the two
	 * possible break instructions that emulate_instr() laid down.  If it
	 * is one of those two break instructions set the resulting pc and
	 * branch back to the caller of emulate_instr().  See emulate_instr()
	 * for the interface of how and where all this happens.
	 */
        li      a3,PCB_WIRED_ADDRESS
	lw	a2,PCB_BD_RA(a3)
	beq	a2,zero,VEC_trap	# handle as a trap
	lw	k1,bd_nottaken_bp	# check for the not taken branch bp
	bne	k0,k1,3f
	or	k1,s0,SR_IEC		# enable interrupts
	.set noreorder
	nop
	mtc0	k1,C0_SR
	nop
	.set reorder
	sw	zero,PCB_BD_RA(a3)	# clear the branch delay emulation
	lw	a3,PCB_BD_EPC(a3)	# the resulting pc in this case is just
	addu	a3,8			#  the pc of the next instruction after
					#  delay slot
	j	a2			# return to caller of emulate_instr()

bd_nottaken_bp:
	break	BRK_BD_NOTTAKEN

3:	lw	k1,bd_taken_bp		# check for the taken branch bp
	bne	k0,k1,VEC_trap		# handle as a trap
	or	k1,s0,SR_IEC		# enable interrupts
	.set noreorder
	nop
	mtc0	k1,C0_SR
	nop
	.set reorder
        li      a3,PCB_WIRED_ADDRESS
	sw	zero,PCB_BD_RA(a3)	# clear the branch delay emulation
	lw	a1,PCB_BD_INSTR(a3)	# the resulting pc in this case is
	lw	a3,PCB_BD_EPC(a3)	#  the target of the emulated branch
	sll	a1,16			#  so add the sign extended offset to
	sra	a1,16-2			#  branch's pc for the resulting pc
	addu	a3,a1
	addu	a3,4
	j	a2			# return to caller of emulate_instr()

bd_taken_bp:
	break	BRK_BD_TAKEN

4:	lw	a0,EF_A0*4(sp)
#if	DEBUG
	sw	zero,crash_area
#endif
	lw	a1,EF_A1*4(sp)
	lw	a2,EF_A2*4(sp)
	lw	a3,EF_A3*4(sp)
	lw	s0,EF_S0*4(sp)
	lw	ra,EF_RA*4(sp)
	lw	k1,EF_SR*4(sp)
	.set noreorder
	nop
	mtc0	k1,C0_SR
	nop
	.set reorder
	lw	k1,EF_K1*4(sp)
	lw	k0,EF_AT*4(sp)		# save AT in k0
	lw	sp,EF_SP*4(sp)

	.set	noat
	lw	AT,+RB_BPADDR		# address of breakpoint handler
	j	AT			# enter breakpoint handler
	.set	at

kernelbp:
	break	BRK_KERNELBP
	END(VEC_breakpoint)

	.globl 	start 
					/*
					 * Fetch the callee's return address
					 */
EXPORT(get_caller_ra)
	.set	noreorder
	move	t0,ra			/* return address */
	lui	t1,0xafbf		/* sw ra,xx(sp) */
	lui	t6,0xffff		/* mask for sw high word */
	la	t2,start	 	/* low address */
	addiu	t0,t0,-4		/* don't look at return address */
1:	
	lw	t3,0(t0)		/* fetch opcode at return address */
	addiu	t0,t0,-4		/* decrement pc */
	and	t4,t3,t6		/* and out low part of opcode */
	beq	t4,t1,2f		/* if sw hit then branch */
	sltu	t5,t0,t2		/* pc less than min */
	beq	zero,t5,1b		/* if not then continue */
	move	v0,zero			/* return bogus pc */
	j	ra			/* back to caller */
2:	andi	t3,t3,0xffff		/* fetch offset (assumed positive) */
	add	t3,sp,t3		/* address */
	lw	v0,0(t3)		/* our callers' return pc */
	j	ra			/* return */
	nop
	.set	reorder

EXPORT(sstepbp)
	break	BRK_SSTEPBP

#if	MACH_KDB
#include <machine/pcb.h>

VECTOR(kdb_breakpoint, M_EXCEPT)	# restart block points here
	.set	noat
	.set	noreorder
	lw	AT,kdbactive		# trap within KDB ?
	nop
	bne	AT,zero,1f
	nop
	la	AT,kdbpcb
	sw	k0,PCB_AT*4(AT)
	li	k0,0xbad00bad
	sw	k0,PCB_K0*4(AT)
	move	k0,AT
	.set	reorder
	.set	at
	sw	k1,PCB_K1*4(k0)
	sw	v0,PCB_V0*4(k0)
	sw	v1,PCB_V1*4(k0)
	sw	a0,PCB_A0*4(k0)
	sw	a1,PCB_A1*4(k0)
	sw	a2,PCB_A2*4(k0)
	sw	a3,PCB_A3*4(k0)
	sw	t0,PCB_T0*4(k0)
	sw	t1,PCB_T1*4(k0)
	sw	t2,PCB_T2*4(k0)
	sw	t3,PCB_T3*4(k0)
	sw	t4,PCB_T4*4(k0)
	sw	t5,PCB_T5*4(k0)
	sw	t6,PCB_T6*4(k0)
	sw	t7,PCB_T7*4(k0)
	sw	s0,PCB_S0*4(k0)
	sw	s1,PCB_S1*4(k0)
	sw	s2,PCB_S2*4(k0)
	sw	s3,PCB_S3*4(k0)
	sw	s4,PCB_S4*4(k0)
	sw	s5,PCB_S5*4(k0)
	sw	s6,PCB_S6*4(k0)
	sw	s7,PCB_S7*4(k0)
	sw	t8,PCB_T8*4(k0)
	sw	t9,PCB_T9*4(k0)
	sw	gp,PCB_GP*4(k0)
	sw	sp,PCB_SP*4(k0)
	sw	fp,PCB_FP*4(k0)
	sw	ra,PCB_RA*4(k0)
	mflo	v0
	sw	v0,PCB_LO*4(k0)
	mfhi	v1
	sw	v1,PCB_HI*4(k0)
	.set noreorder
	mfc0	v0,C0_EPC
	mfc0	v1,C0_SR
	sw	v0,PCB_PC*4(k0)
	mfc0	v0,C0_BADVADDR
	sw	v1,PCB_SR*4(k0)
	mfc0	v1,C0_CAUSE
	sw	v0,PCB_BAD*4(k0)
	mfc0	v0,C0_TLBLO
	sw	v1,PCB_CS*4(k0)
	mfc0	v1,C0_TLBHI
	sw	v0,PCB_TLO*4(k0)
	mfc0	v0,C0_INX
	sw	v1,PCB_THI*4(k0)
	mfc0	v1,C0_RAND
	sw	v0,PCB_INX*4(k0)
	mfc0	v0,C0_CTXT
	sw	v1,PCB_RAN*4(k0)
	sw	v0,PCB_CTX*4(k0)

1:	la	gp,_gp
	move	a0,sp
	bltz	sp,1f
	subu	sp,24			# unless on user stack, use thread's
					# kernel stack
        li      sp,PCB_WIRED_ADDRESS-EF_SIZE

1:	jal	kdb_trap		# kdb_trap(esp,0)
	move	a1,zero

	.set	reorder
	la	k0,kdbpcb
	lw	a0,PCB_A0*4(k0)
	lw	a1,PCB_A1*4(k0)
	lw	a2,PCB_A2*4(k0)
	lw	a3,PCB_A3*4(k0)
	lw	t0,PCB_T0*4(k0)
	lw	t1,PCB_T1*4(k0)
	lw	t2,PCB_T2*4(k0)
	lw	t3,PCB_T3*4(k0)
	lw	t4,PCB_T4*4(k0)
	lw	t5,PCB_T5*4(k0)
	lw	t6,PCB_T6*4(k0)
	lw	t7,PCB_T7*4(k0)
	lw	s0,PCB_S0*4(k0)
	lw	s1,PCB_S1*4(k0)
	lw	s2,PCB_S2*4(k0)
	lw	s3,PCB_S3*4(k0)
	lw	s4,PCB_S4*4(k0)
	lw	s5,PCB_S5*4(k0)
	lw	s6,PCB_S6*4(k0)
	lw	s7,PCB_S7*4(k0)
	lw	t8,PCB_T8*4(k0)
	lw	t9,PCB_T9*4(k0)
	lw	k1,PCB_K1*4(k0)
	lw	gp,PCB_GP*4(k0)
	lw	fp,PCB_FP*4(k0)
	lw	ra,PCB_RA*4(k0)
	lw	v0,PCB_LO*4(k0)
	mtlo	v0
	lw	v1,PCB_HI*4(k0)
	mthi	v1
	.set noreorder
	lw	v0,PCB_INX*4(k0)
	lw	v1,PCB_TLO*4(k0)
	mtc0	v0,C0_INX
	lw	v0,PCB_THI*4(k0)
	mtc0	v1,C0_TLBLO
	lw	v1,PCB_CTX*4(k0)
	mtc0	v0,C0_TLBHI
	lw	v0,PCB_CS*4(k0)
	mtc0	v1,C0_CTXT
	lw	v1,PCB_SR*4(k0)
	mtc0	v0,C0_CAUSE
	and	v1,~(SR_KUC|SR_IEC)	# sanity
	mtc0	v1,C0_SR
	lw	v0,PCB_V0*4(k0)
	lw	v1,PCB_V1*4(k0)
	lw	sp,PCB_SP*4(k0)
	.set	noat
	lw	AT,PCB_AT*4(k0)
	.set	at
	lw	k0,PCB_PC*4(k0)
	nop
	j	k0
	rfe
	.set	reorder
	END(kdb_breakpoint)
#endif


/*
 * Coprocessor unusable fault
 */
VECTOR(VEC_cpfault, M_EXCSAVE)
	or	a1,s0,SR_IEC		# enable interrupts
	.set noreorder
	mtc0	a1,C0_SR
	.set reorder

	and	a1,s0,SR_KUP
	beq	a1,zero,coproc_panic	# kernel tried to use coprocessor

	and	a1,a3,CAUSE_CEMASK
	srl	a1,CAUSE_CESHIFT
	bne	a1,1,coproc_not1	# not coproc 1

#ifdef ASSERTIONS
	and	a1,s0,SR_IBIT8
	bne	a1,zero,1f		# fp interrupts must be enabled!
	PANIC("VEC_cpfault")
1:
#endif

	/*
	 * This is the floating-point coprocessor (coprocessor 1) unusable
	 * fault handling code.  During auto configuration fptype_word
	 * is loaded from the floating-point coprocessor revision word or
	 * zeroed if there is no floating-point coprocessor.
	 */
	li      a1,PCB_WIRED_ADDRESS
	sw	sp,PCB_OWNEDFP(a1)	# mark that fp has been touched
	lw	a2,fptype_word		# check for what type of fp coproc
	bne	a2,zero,1f
	j	softfp_unusable		# no fp coproc (goto fp software)
1:
	or	a1,s0,SR_CU1		# enable coproc 1 for the user process
	sw	a1,EF_SR*4(sp)

#if     NCPUS > 1
        li      a2,PCB_WIRED_ADDRESS
        lw      a2,PCB_CPU_NUMBER(a2)
        la      a1,active_threads
        sll     a2,2                    # *(sizeof caddr)
        addu    a1,a1,a2
        lw      a1,0(a1)                # >>active_threads<<
        la      a3,fpowner_array
        addu    a3,a3,a2
        lw      a2,0(a3)                #  current coproc 1 (fp) owner?
#else
        lw      a1,active_threads       # only 1 cpu....
        lw      a2,fpowner_array
#endif
	beq	a2,a1,coproc_done	# owned by the current process

	or	a3,s0,SR_CU1|SR_IEC	# enable fp and interrupts
	.set noreorder
	nop
	mtc0	a3,C0_SR
	nop
	.set reorder
	beq	a2,zero,fp_notowned	# coproc 1 not currently owned

	/*
	 * Owned by someone other than the current process.
	 * Save state (into the fpowner_array) before taking possession.
	 */
#ifdef ASSERTIONS
	lw	a3,THREAD_STATE(a2)
	andi	a3,a3,TH_SWAPPED
	beq	a3,zero,1f
	PANIC("VEC_cpfault swapped out")
1:
#endif
	lw	a3,THREAD_PCB(a2)	# get owner's pcb

#ifdef	__STDC__
#define SAVECP1REG(reg) \
	swc1	$f##reg,PCB_FPREGS+reg*4(a3)
#else
#define SAVECP1REG(reg) \
	swc1	$f/**/reg,PCB_FPREGS+reg*4(a3)
#endif

	/*
	 * The floating-point control and status register must be
	 * read first to force all fp operations to complete and insure
	 * that all fp interrupts for this process have been delivered
	 */
	.set	noreorder
	cfc1	a2,fpc_csr
	nop
	sw	a2,PCB_FPC_CSR(a3)
	cfc1	a2,fpc_eir
	nop
	sw	a2,PCB_FPC_EIR(a3)
	SAVECP1REG(31); SAVECP1REG(30); SAVECP1REG(29); SAVECP1REG(28)
	SAVECP1REG(27); SAVECP1REG(26); SAVECP1REG(25); SAVECP1REG(24)
	SAVECP1REG(23); SAVECP1REG(22); SAVECP1REG(21); SAVECP1REG(20)
	SAVECP1REG(19); SAVECP1REG(18); SAVECP1REG(17); SAVECP1REG(16)
	SAVECP1REG(15); SAVECP1REG(14); SAVECP1REG(13); SAVECP1REG(12)
	SAVECP1REG(11); SAVECP1REG(10); SAVECP1REG(9);  SAVECP1REG(8)
	SAVECP1REG(7);  SAVECP1REG(6);  SAVECP1REG(5);  SAVECP1REG(4)
	SAVECP1REG(3);  SAVECP1REG(2);  SAVECP1REG(1);  SAVECP1REG(0)

	.set	reorder
fp_notowned:
	/*
	 * restore coprocessor state (from the current process)
	 */
	.set	noreorder
        li      a3,PCB_WIRED_ADDRESS

#ifdef	__STDC__
#define RESTCP1REG(reg) \
	lwc1	$f##reg,PCB_FPREGS+reg*4(a3)
#else
#define RESTCP1REG(reg) \
	lwc1	$f/**/reg,PCB_FPREGS+reg*4(a3)
#endif

	or	a2,s0,SR_CU1
	mtc0	a2,C0_SR		# disable interrupts, fp enabled
	nop				# before we can really use cp1
	nop				# before we can really use cp1
	RESTCP1REG(0);  RESTCP1REG(1);  RESTCP1REG(2);  RESTCP1REG(3)
	RESTCP1REG(4);  RESTCP1REG(5);  RESTCP1REG(6);  RESTCP1REG(7)
	RESTCP1REG(8);  RESTCP1REG(9);  RESTCP1REG(10); RESTCP1REG(11)
	RESTCP1REG(12); RESTCP1REG(13); RESTCP1REG(14); RESTCP1REG(15) 
	RESTCP1REG(16); RESTCP1REG(17); RESTCP1REG(18); RESTCP1REG(19)
	RESTCP1REG(20); RESTCP1REG(21); RESTCP1REG(22); RESTCP1REG(23)
	RESTCP1REG(24); RESTCP1REG(25); RESTCP1REG(26); RESTCP1REG(27)
	RESTCP1REG(28); RESTCP1REG(29); RESTCP1REG(30); RESTCP1REG(31)
	ctc1	zero,fpc_csr
	lw	a2,PCB_FPC_EIR(a3)
	nop
	ctc1	a2,fpc_eir
	lw	a2,PCB_FPC_CSR(a3)
	nop
	ctc1	a2,fpc_csr

#if     NCPUS > 1
        li      a3,PCB_WIRED_ADDRESS
        lw      a3,PCB_CPU_NUMBER(a3)
        la      a2,fpowner_array
        sll     a3,2                    # *(sizeof caddr)
        addu    a3,a3,a2
        sw      a1,0(a3)
#else
        sw      a1,fpowner_array        # this thread now owns the fp
#endif
	mtc0	s0,C0_SR		# disable interrupt and clear SR_CU1
	.set	reorder
	b	exception_exit

coproc_done:
	.set noreorder
	mtc0	s0,C0_SR		# disable interrupts
	.set reorder
	b	exception_exit

coproc_not1:
	li	a1,SEXC_CPU		# handle as software trap
	b	VEC_trap		# not soft_trap, must save regs yet

coproc_panic:
	PANIC("kernel used coprocessor")
	END(VEC_cpfault)

/*
 * checkfp(th, exiting)
 *	Called to release FP ownership.
 *	th = FP owner, true or presumed.
 *	exiting = 1 if thread is being terminated.
 */
LEAF(checkfp)
#if     NCPUS > 1
        li      a3,PCB_WIRED_ADDRESS
        lw      a3,PCB_CPU_NUMBER(a3)
        la      v0,fpowner_array
        sll     a3,2                    # *(sizeof caddr)
        addu    a3,a3,v0
        lw      v0,0(a3)
#else
        lw      v0,fpowner_array        # this thread now owns the fp
#endif
	bne	a0,v0,disable_fp	# not owner, but make sure.
	bne	a1,zero,1f		# exiting, don't save state
	lw	a3,fptype_word
	beq	a3,zero,1f		# no fp coprocessor
	lw	a3,THREAD_PCB(a0)	# get thread's pcb

	/*
	 * The floating-point control and status register must be
	 * read first so to stop the floating-point coprocessor.
	 */
	.set	noreorder
	mfc0	v1,C0_SR		# enable coproc 1 for the kernel
	nop
	or	v0,v1,SR_CU1		
	mtc0	v0,C0_SR		# PE BIT
	nop				# before we can really use cp1
	nop				# before we can really use cp1
	cfc1	v0,fpc_csr
	nop
	sw	v0,PCB_FPC_CSR(a3)
	cfc1	v0,fpc_eir
	nop
	sw	v0,PCB_FPC_EIR(a3)
	SAVECP1REG(31); SAVECP1REG(30); SAVECP1REG(29); SAVECP1REG(28)
	SAVECP1REG(27); SAVECP1REG(26); SAVECP1REG(25); SAVECP1REG(24)
	SAVECP1REG(23); SAVECP1REG(22); SAVECP1REG(21); SAVECP1REG(20)
	SAVECP1REG(19); SAVECP1REG(18); SAVECP1REG(17); SAVECP1REG(16)
	SAVECP1REG(15); SAVECP1REG(14); SAVECP1REG(13); SAVECP1REG(12)
	SAVECP1REG(11); SAVECP1REG(10); SAVECP1REG(9);  SAVECP1REG(8)
	SAVECP1REG(7);  SAVECP1REG(6);  SAVECP1REG(5);  SAVECP1REG(4)
	SAVECP1REG(3);  SAVECP1REG(2);  SAVECP1REG(1);  SAVECP1REG(0)
	ctc1	zero,fpc_csr		# clear any pending interrupts
	mtc0	v1,C0_SR		# disable kernel fp access
	nop
	.set	reorder

1:	
#if     NCPUS > 1
        li      a3,PCB_WIRED_ADDRESS
        lw      a3,PCB_CPU_NUMBER(a3)
        la      v0,fpowner_array
        sll     a3,2                    # *(sizeof caddr)
        addu    a3,a3,v0
        sw      zero,0(a3)
#else
        sw      zero,fpowner_array        # Mark FP as unowned
#endif
	/*
	 * Disable use of the coprocessor, so that state
	 * will be reloaded when/if necessary
	 */
XLEAF(disable_fp)
	/*
	 * Beware: this is used when forking U*x threads too 
	 */

	lw	a0,THREAD_KERNEL_STACK(a0)
	addu	a0,KERNEL_STACK_START_OFFSET-EF_SIZE	# adjust ksp
	lw	a1,EF_SR*4(a0)		# user's sr
	and	a1,~SR_CU1		# make it unusable
	sw	a1,EF_SR*4(a0)

2:	j	ra
	END(checkfp)

/*
 * save_frame -- save the machine state that is not normally preserved
 *		by the calling conventions, or by the general exception
 *		processing code - temp registers, hi, lo, v0, v1
 */
LEAF(save_frame)
	sw	v0,EF_V0*4(sp)
	sw	v1,EF_V1*4(sp)
	sw	t0,EF_T0*4(sp)
	mflo	t0
	sw	t1,EF_T1*4(sp)
	mfhi	t1
	sw	t2,EF_T2*4(sp)
	sw	t3,EF_T3*4(sp)
	sw	t4,EF_T4*4(sp)
	sw	t5,EF_T5*4(sp)
	sw	t6,EF_T6*4(sp)
	sw	t7,EF_T7*4(sp)
	sw	t8,EF_T8*4(sp)
	sw	t9,EF_T9*4(sp)
	sw	t0,EF_MDLO*4(sp)
	sw	t1,EF_MDHI*4(sp)
	j	ra
	END(save_frame)

/*
 * restore_frame -- restore state saved by save_frame
 */
LEAF(restore_frame)
	lw	v0,EF_MDLO*4(sp)
	lw	v1,EF_MDHI*4(sp)
	mtlo	v0
	mthi	v1
	lw	v0,EF_V0*4(sp)
	lw	v1,EF_V1*4(sp)
	lw	t0,EF_T0*4(sp)
	lw	t1,EF_T1*4(sp)
	lw	t2,EF_T2*4(sp)
	lw	t3,EF_T3*4(sp)
	lw	t4,EF_T4*4(sp)
	lw	t5,EF_T5*4(sp)
	lw	t6,EF_T6*4(sp)
	lw	t7,EF_T7*4(sp)
	lw	t8,EF_T8*4(sp)
	lw	t9,EF_T9*4(sp)
	j	ra
	END(restore_frame)

	IMPORT(lwc_interrupt_data,4)

#ifdef ASM_FIXED
VECTOR(VEC_lwc, M_EXCEPT|M_TFISAVE)
#else
VECTOR(VEC_lwc, M_EXCSAVE)
#endif
	.set noreorder
	jal	save_frame		# save temporaries
	nop
	or	a2,s0,SR_IEC		# set enable bit
	mtc0	a2,C0_SR		# enable interrupts
	.set reorder
	jal	lwc_schedule		# schedule the events
	jal	restore_frame		# restore temporaries
	.set noreorder
	mtc0	s0,C0_SR		# disable interrupts
	.set reorder
	b	exception_exit		# do exception exit
	END(VEC_lwc)

/*
 * End of exception processing.  Interrupts should be disabled.
 */
VECTOR(exception_exit, M_EXCEPT)
	/*
	 * ENTRY CONDITIONS:
	 *	Interrupts Disabled
	 *	s0 contains sr at time of exception, but sr will
	 *	  be reloaded from exception frame
	 *	- thread's stack MUST be wired
	 *
	 * If s0 says we are returning to user mode, check to see if
	 * an AST is pending.
	 * If so, get back to trap so that the necessary state will
	 * be saved for possible context switching.
	 */
#ifdef	ASSERTIONS
	and	a0,s0,SR_KUC|SR_IEC
	beq	a0,zero,8f
	PANIC("interrupts enabled in exception_exit!")
8:
#endif

#ifdef	DS5000_100
/* For generic kernels, need the following (extra) code */
#ifdef	MIPS_LIBRARY
	/* 3MIN requires special processing */
	lw	a0,mips_spl_arch_type
	beq	a0,zero,1f
#endif	/* MIPS_LIBRARY */

	.set noreorder
	lw	a0, EF_SYS1*4(sp)
	lw	a3, EF_SR*4(sp)		# get SR from exception frame
	sw	a0, ipllevel
	sll	a0, a0, 2		# multiply by 4
	lw	a1, kn02ba_sim(a0)	# get system interrupt mask value
	lw	a2, splm(a0)		# get status register mask value
	sw	a1, KN02BA_SIRM_K1ADDR	# load mask register with value
	lw	a1, KN02BA_SIRM_K1ADDR	# reread address to flush write buffer
	andi	a2, a2, 0xff00		# get interrupt mask bits only
	li	k0, 0xffff00ff
	and	a3, a3, k0		# turn off all mask bits
	or	a2, a2, a3		# or in new mask bits
	sw	a2, EF_SR*4(sp)		# restore SR to exception frame
	.set reorder
	/* End 3MIN Special */
#endif	/* DS5000_100       */
1:	

	.set noreorder
	and	a0,s0,SR_KUP
#if	!RT_PREEMPT
	beq	a0,zero,2f			# returning to kernel mode
	lw	a3,lwc_interrupt_data		# check for lwc interrupt
#else
	/*
	 * This code implements kernel mode ASTs.  A Kernel Mode AST occurs
	 * if:
	 *	o Kernel mode AST requested and
	 *	o Returning to Kernel mode  and
	 *	o Returning to spl0
	 */
        bne     a0,zero,3f                      # returning to user mode?
	lw	a0,rt_preempt_enabled		# kernel preemption enabled?
	nop
	beq	a0,zero,2f			# if not, nothing else to do
        /*
         * We are returning to kernel mode.  Check for AST.
         */
        lw      a0,need_ast                     # get need_ast, either way
        lw      a1,ast_mode                     # get AST type (user or kernel)
        beq     a0,zero,2f                      # no AST, kernel mode
        li      a0,SR_IMASK0                    # get IPL 0 mask
        beq     a1,zero,2f                      # if user mode, no AST allowed
        andi    a1,s0,SR_IMASK                  # kernel mode AST, test IPL
        bne     a0,a1,2f                        # if not IPL 0, no AST allowed
        move    a0,sp
        li      a1,SEXC_AST                     # software exception
	move	a2,s0
	lw	a3,EF_CAUSE*4(sp)
	jal	save_frame
	nop
	b	soft_trap
	nop
3:	lw	a3,lwc_interrupt_data		# check for lwc interrupt
#endif
	lw	a0,need_ast			# fetch ast
        beq	a3,zero,4f			# no lwc
        li      a1,SEXC_AST                     # software exception
	b	VEC_lwc				# have lwc
	nop
4:	beq     a0,zero,1f                      # no AST, user mode
        move    a0,sp
	lw	a3,EF_CAUSE*4(sp)
	b	VEC_trap
	nop					# from OSF
	.set reorder

1:
        li      k0,PCB_WIRED_ADDRESS            # address of PCB
        sw      zero,PCB_KSTACK(k0)         	# switching to user stack
	lw	gp,EF_GP*4(sp)
2:	lw	a0,EF_A0*4(sp)
	lw	a1,EF_A1*4(sp)
	lw	a2,EF_A2*4(sp)
	lw	a3,EF_A3*4(sp)
	lw	s0,EF_S0*4(sp)
	lw	ra,EF_RA*4(sp)
	lw	k0,EF_SR*4(sp)

#if	DEBUG
	sw	zero,crash_area
#endif

	.set noat
	lw	AT,EF_AT*4(sp)
	lw	k1,EF_EPC*4(sp)
	.set noreorder
	mtc0	k0,C0_SR			# PE BIT
	lw	sp,EF_SP*4(sp)
	move	k0,zero
	j	k1
	rfe
 	.set	at
	.set	reorder
	END(exception_exit)

VECTOR(VEC_unexp, M_EXCEPT)
	PANIC("unexpected exception")
	END(VEC_unexp)

/*
 * setsoft2(), aka acksoftnet - make software network interrupt request
 */
EXPORT(setsoft2)
EXPORT(setsoftnet)
	li	a0, CAUSE_SW2
	j	siron

/*
 * acksoft2(), aka acksoftnet - acknowledge software network interrupt
 */
EXPORT(acksoft2)
EXPORT(acksoftnet)
	li	a0, CAUSE_SW2
	j	siroff

/*
 * setsoftclock() - make software clock interrupt request
 */
EXPORT(setsoft1)
EXPORT(setsoftclock)
	li	a0, CAUSE_SW1
	j	siron

/*
 * acksoftclock() - acknowledge software clock interrupt
 */
EXPORT(acksoft1)
EXPORT(acksoftclock)
	li	a0, CAUSE_SW1
	j	siroff

/*
 * siron(level) -- make software interrupt request
 */
LEAF(siron)
	.set	noreorder
	mfc0	v0,C0_SR
	mtc0	zero,C0_SR		# disable all interrupts
	mfc0	v1,C0_CAUSE
	nop
	or	v1,a0
	mtc0	v1,C0_CAUSE
	mtc0	v0,C0_SR		# PE BIT
	j	ra	
	nop
	.set	reorder
	END(siron)

/*
 * siroff(level) -- acknowledge software interrupt request
 */
LEAF(siroff)
	.set	noreorder
	mfc0	v0,C0_SR
	mtc0	zero,C0_SR		# disable all interrupts
	mfc0	v1,C0_CAUSE
	not	a0
	and	v1,a0
	mtc0	v1,C0_CAUSE
	mtc0	v0,C0_SR		# PE BIT
	j	ra
	nop
	.set	reorder
	END(siroff)
/*
 *	Thread and system initialization
 */

/*
 * start_init()
 * Calls load_init_program() as if from syscall handler
 */
LEAF(start_init)		/* we need to save nothing */
        li      v0,PCB_WIRED_ADDRESS-EF_SIZE
	move	sp,v0
	jal load_init_program
	.set noreorder
	mtc0	zero,C0_SR		/* disable interrupts */
	.set reorder
	lw	s0,EF_SR*4(sp)		/* make sure sr is right */
	la	ra,exception_exit	/* full register reload */
	b	restore_frame
	END(start_init)

/*
 *	thread_bootstrap:
 *
 *	Bootstrap a new thread using the Mips thread state that has been
 *	placed on the stack.
 */
LEAF(thread_bootstrap)
	.set noreorder
	mtc0	zero,C0_SR		# no interrupts
	.set reorder
	li	s0,SR_KUP		# make belief sr for exception_exit
	b	full_restore		# full register reload
	END(thread_bootstrap)

/*
 *	Miscellaneous things
 */

/*
 * save_entire_frame - save the entire register state in anticipation
 * of a fork U*x call
 */
LEAF(save_entire_frame)
	sw	s1,EF_S1*4(sp)
	sw	s2,EF_S2*4(sp)
	sw	s3,EF_S3*4(sp)
	sw	s4,EF_S4*4(sp)
	sw	s5,EF_S5*4(sp)
	sw	s6,EF_S6*4(sp)
	sw	s7,EF_S7*4(sp)
	sw	t0,EF_T0*4(sp)
	mflo	t0
	sw	t1,EF_T1*4(sp)
	mfhi	t1
	sw	t2,EF_T2*4(sp)
	sw	t3,EF_T3*4(sp)
	sw	t4,EF_T4*4(sp)
	sw	t5,EF_T5*4(sp)
	sw	t6,EF_T6*4(sp)
	sw	t7,EF_T7*4(sp)
	sw	t8,EF_T8*4(sp)
	sw	t9,EF_T9*4(sp)
	sw	fp,EF_FP*4(sp)
	sw	t0,EF_MDLO*4(sp)
	sw	t1,EF_MDHI*4(sp)
	j	ra
	END(save_entire_frame)

#if	MACH_KDB
LEAF(getmemc)
	or	a0,K1BASE
	lb	v0,0(a0)
	j	ra
	END(getmemc)

LEAF(putmemc)
	or	a0,K1BASE
	sb	a1,0(a0)
	j	ra
	END(putmemc)
#endif

/* Can't use mips/endian.h in assembler files! */
#if	(defined(MIPSEL) + defined(MIPSEB)) != 1
One of MIPSEL or MIPSEB must be defined for the mips assembly files!
#endif

/*
 * gimmeabreak - executes a break instruction :-)
 */
LEAF(gimmeabreak)
	break	BRK_KERNELBP
	j	ra
	END(gimmeabreak)


#if	DEBUG
LEAF(crashit)
	.set	noat
	la	AT,crash_saved_registers
	sw	v0,0(AT)
	sw	v1,4(AT)
	sw	a0,8(AT)
	sw	a1,12(AT)
	sw	a2,16(AT)
	sw	a3,20(AT)
	sw	t0,24(AT)
	sw	t1,28(AT)
	sw	t2,32(AT)
	sw	t3,36(AT)
	sw	t4,40(AT)
	sw	t5,44(AT)
	sw	t6,48(AT)
	sw	t7,52(AT)
	sw	s0,56(AT)
	sw	s1,60(AT)
	sw	s2,64(AT)
	sw	s3,68(AT)
	sw	s4,72(AT)
	sw	s5,76(AT)
	sw	s6,80(AT)
	sw	s7,84(AT)
	sw	t8,88(AT)
	sw	t9,92(AT)
	sw	k0,96(AT)
	sw	k1,100(AT)
	sw	gp,104(AT)
	sw	sp,108(AT)
	la	sp,boot_stack
	subu	sp,EF_SIZE
	sw	fp,112(AT)
	sw	ra,116(AT)
	mflo	k0
	mfhi	k1
	sw	k0,120(AT)
	mfc0	k0,C0_TLBHI
	sw	k1,124(AT)
	mfc0	k1,C0_TLBLO
	sw	k0,128(AT)
	mfc0	k0,C0_INX
	sw	k1,132(AT)
	mfc0	k1,C0_CTXT
	sw	k0,136(AT)
	mfc0	k0,C0_SR
	sw	k1,140(AT)
	mfc0	k1,C0_BADVADDR
	sw	k0,144(AT)
	mfc0	k0,C0_CAUSE
	sw	k1,148(AT)
	mfc0	k1,C0_EPC
	sw	k0,152(AT)
	sw	k1,156(AT)
	mtc0	zero,C0_SR
	b	prom_restart
	.set	at
	END(crashit)
#endif
/* TODO: not yet */
#ifdef notdef
NESTED(smp_lock_retry,24,zero)
	.set	noreorder
	lw	t6,smp
	lw	t5,(a0)
	bne 	t6,zero,2f	
	lw	t1,u+PCB_CPUPTR	 /* get cpudata pointer */
	bltz	t5,3f
	lw	t2,CPU_HLOCK(t1) /* save off lock list head */
	sw	a0,CPU_HLOCK(t1) /* put new lock at head */
	sw	ra,L_PC(a0)	/* store off PC lock asserted at */	
	lui	t3,0x8000
	lw	t4,L_WON(a0)	/* get lock won */
	or	t6,t3,t5	/* set lock bit */
	addiu	t4,t4,1		/* increment lock won */
	sw	t6,(a0)
	sw	t4,L_WON(a0)
	j	ra
	sw	t2,L_PLOCK(a0)  /* chain old list off new lock */


3:
	addiu	sp,sp,-24	/* save stack space */
	b 	1f
	sw	ra,20(sp)	/* save off return */
			
2:	
	lw	t6,smp_debug	/* read up smp_debug*/
	addiu	sp,sp,-24	/* save stack space */
	bne	t6,zero,1f	/* branch if debug enabled */
	sw	ra,20(sp)	/* save off return */

	jal	setlock
	sw	a0,24(sp)	/* save off lock pointer */

	beq 	v0,zero,1f	/* branch if we failed to get lock */
	lw	a0,24(sp)	/* restore lock pointer */
	lw	t1,u+PCB_CPUPTR	 /* get cpudata pointer */
	lw	ra,20(sp)		/* restore return address */
	lw	t2,CPU_HLOCK(t1) /* save off lock list head */
	sw	a0,CPU_HLOCK(t1)	/* put new lock at head */
	lw	t4,L_WON(a0)	/* get lock won */
	sw	ra,L_PC(a0)		
	addiu	t4,t4,1		/* increment lock won */
	addiu	sp,sp,24		/* give back stack space */
	sw	t4,L_WON(a0)
	j	ra
	sw	t2,L_PLOCK(a0);		/* chain old list off new lock */

1:	li	a1,LK_RETRY
	jal	smp_lock_long		/*  smp_lock_long(lk,LK_RETRY,pc) */
	lw	a2,20(sp)

	lw	ra,20(sp)		/* restore return address */
	addiu	sp,sp,24		/* give back stack space */
	j 	ra
	li	v0,0			/* failed */

	.set reorder
        END(smp_lock_retry)

NESTED(smp_lock_once,24,zero)
	.set	noreorder
	lw	t6,smp
	lw	t5,(a0)
	bne 	t6,zero,2f	
	lw	t1,u+PCB_CPUPTR	 /* get cpudata pointer */
	bltz	t5,3f
	lw	t2,CPU_HLOCK(t1) /* save off lock list head */
	sw	a0,CPU_HLOCK(t1) /* put new lock at head */
	sw	ra,L_PC(a0)	/* store off PC lock asserted at */	
	lui	t3,0x8000
	lw	t4,L_WON(a0)	/* get lock won */
	or	t6,t3,t5	/* set lock bit */
	addiu	t4,t4,1		/* increment lock won */
	sw	t6,(a0)
	sw	t4,L_WON(a0)
	li	v0,1			/* success */
	j	ra
	sw	t2,L_PLOCK(a0)  /* chain old list off new lock */

3:				/* failed */
	j 	ra
	li	v0,0
			
2:	

	lw	t6,smp_debug	/* read up smp_debug*/
	addiu	sp,sp,-24	/* save stack space */
	bne	t6,zero,1f	/* branch if debug enabled */
	sw	ra,20(sp)	/* save off return */

	jal	setlock
	sw	a0,24(sp)	/* save off lock pointer */

	beq 	v0,zero,2f	/* branch if we failed to get lock */
	lw	a0,24(sp)	/* restore lock pointer */
	lw	t1,u+PCB_CPUPTR	 /* get cpudata pointer */
	lw	ra,20(sp)		/* restore return address */
	lw	t2,CPU_HLOCK(t1) /* save off lock list head */
	sw	a0,CPU_HLOCK(t1)	/* put new lock at head */


	lw	t4,L_WON(a0)	/* get lock won */
	sw	ra,L_PC(a0)		
	addiu	t4,t4,1		/* increment lock won */
	addiu	sp,sp,24		/* give back stack space */
	sw	t4,L_WON(a0)
	j	ra
	sw	t2,L_PLOCK(a0);		/* chain old list off new lock */

2:					/* failed */
	lw 	ra,20(sp)
	addiu	sp,sp,24		/* give back stack space */
	j 	ra
	li	v0,0

1:
	li	a1,LK_ONCE
	jal	smp_lock_long		/*  smp_lock_long(lk,LK_ONCE,pc) */
	lw	a2,20(sp)

	lw	ra,20(sp)		/* restore return address */
	addiu	sp,sp,24		/* give back stack space */
	j 	ra
	nop
	.set reorder
        END(smp_lock_once)

#endif

LEAF(rdnf_error)
        li      a1,PCB_WIRED_ADDRESS            #
 	sw	t1,PCB_NOFAULT(a1)
	.set noreorder
	mtc0	t0,C0_SR		# PE BIT
	.set reorder
	move	v0,zero
	j	ra
	END(rdnf_error)

/*
 * zone lock debugging function. -- used by kern/zalloc.c
 * 
 * Gets the caller's pc for tracing.
 * Beats the heck out of the old machdep routine
 * which returned 666!
 *
 */
LEAF(getpc)
	move	v0,ra
	j	ra
	END(getpc)


/*
 * unixtovms(srcaddr,dstaddr)
 *
 * a0 src address
 * a1 dst address
 * t0 tmp
 * t1 tmp
 * t2 tmp value ->0(a1)
 * t3 tmp value ->4(a1)
 * t4 carry
 * t5 <- constants - 0x989680   convert sec. to 100 nanosec. units.
 *	           - 0x4beb4000 difference between 00:00 Jan. 1 1970
 *	           - 0x7c9567   and 00:00 Nov. 17 1858 to form the
 *				 VMS time.
 */
LEAF(unixtovms)
	lw 	t0, 0(a0)
	lw 	t1, 4(a0)
	li 	t5, 0x989680
	multu 	t0, t5
	mflo	t2
	mfhi	t3
	addu	t0, t2, t1
	sltu	t4, t0, t2
	addu 	t3, t4
	li 	t5, 0x4beb4000
	addu 	t2, t0, t5
	sltu	t4, t2, t0
	addu 	t3, t4
	li 	t5, 0x7c9567
	addu	t3, t5
	sw	t2, 0(a1)
	sw	t3, 4(a1)
	j	ra
	END(unixtovms)

#ifdef notdef
/* TODO: wait_tick ifdef'd out for now */
/* Function used by LMF code to measure the speed of a PMAX type processor.
 * It returns the number of loop iterations before the clock
 * next ticks.   The arguments are the previous value of the clock
 * for comparison and a maximum for safety.
 *
 * This is in assembler to protect against compiler changes.
 * ***** If you change this code you must also change the line:
 * ***** #define THRESH xxx
 * ***** in kern_lmf.c after experimenting to find the new value.
 */

LEAF(wait_tick)
	.set	noreorder
	li	v0, 0		# Initialise counter
	la	t1, timepick
1:	lw	t0, (t1)	# Load timepick
	blez	a1, 2f		# Check for safety counter exhausted
	lw	t0, 4(t0)	# Load timepick->tv_usecs
	addi	a1, -1		# Decrement safety counter
	beq	t0, a0, 1b	# Branch if time has changed (clock ticked)
	addiu	v0, 1		# Count loop iterations in delay slot
2:	j	ra
	nop
	.set	reorder
	END(wait_change)
#endif
/*
 * This statement measures the length of the mfc0 code sequence to
 * see if the mfc0 assembler option has been invoked. See autoconf
 * for exact usage. (This is a hack).
 */
EXPORT(mfc0_start)
	.set noreorder
	nop
	mfc0	v0,C0_SR
	nop
	.set reorder

EXPORT(mfc0_end)
#ifdef notdef
/*
 * Bootstrap program executed in user mode
 * to bring up the system.
 */
EXPORT(icode)
	la	a0,icode_file
	la	v0,icode		# relocate for user space
	subu	a0,v0
	addu	a0,USRDATA
	la	a1,icode_argv
	la	v0,icode		# relocate for user space
	subu	a1,v0
	addu	a1,USRDATA
	sw	a0,0(a1)		# relocate vector in d space
	li	a2,0			# no environment please
	li	v0,SYS_execve
	syscall
	li	v0,SYS_exit
	syscall
1:	b	1b

	.align	4
EXPORT(icode_argv)
	.word	icode_file
	.space	10*4			# leave space for boot args

EXPORT(icode_file)
	.asciiz	"/etc/init"
	.space	32
argp:					# leave space for boot args
	.space	64

	.align	4

EXPORT(icode_args)
	.word	argp

EXPORT(icode_argc)
	.word	1

EXPORT(eicode)

#endif /* notdef */

	.sdata
EXPORT(Vaxmap)
	.word	1
EXPORT(Vaxmap_size)
	.word	1
EXPORT(VSysmap)
	.word	1
EXPORT(Dbptemap)
	.word	1
EXPORT(Vaxdbpte)
	.word	1
	.text

#ifndef ASM_FIXED
	.word	0
#endif /* !ASM_FIXED */

#if RT_SCHED_OPT
/*
 * runq_update_low
 *
 * This routine checks to see if the queue indexed by rq->low is empty.  If
 * it is not empty, the routine returns.  If it is empty, the bit in the run
 * queue bitmask corresponding to that queue is cleared and rq->low is updated.
 * The new rq->low value is returned.
 *
 * This routine should be called at splsched with the run queue locked.
 *
 * Calling sequence:
 *
 *    low = runq_update_low( &rq )
 *
 * Register usage:
 *	a0	Address of run queue.
 *	v0	Return value is new setting of rq->low.
 *	t9	Pointer to rq->low (&rq->low).
 *	t8	Holds return address.
 *	t2	Mask for clearing a bit in the run queue bitmask.
 *	t1	Scratch register.
 *	t0	Scratch register.
 *
 *  NOTE: This routine assumes that find_first_runq_bit_set doesn't trash
 *	  t8 or t9!
 */

/* Size of queue header, amount to shift for queue header. */
#define QHSZ 8
#define QHSH 3	

LEAF(runq_update_low)
	.set noreorder

	# Get rq->low.  It is after the run queue headers.
	lw	v0,QHSZ*NRQS(a0)	# Load rq->low.
	addu	t9,a0,QHSZ*NRQS		# BD: Pointer to rq->low.

	# Calculate address of run queue mask, and see if the queue is
	# empty.  Can just return if the queue is empty.  Otherwise, will
	# need to update the bitmask, so start fetching the bit to clear.
	sll	t0,v0,QHSH		# low * 8.
	addu	t1,t0,a0		# &rq->runq[rq->low]
	lw	t0,0(t1)		# Load rq->runq[rq->low].next.
	li	t2,1			# D:
	beq	t0,t1,1f		# Queue nonempty ? return : continue
	sll	t2,t2,v0		# BD: (1<<(low%32))
	j	ra			# Return.
	nop				# BD:               

	# Clear the bit in the run queue mask.  Note that this code does not
	# protect against the case (low >= NRQS).  If passed a bogus value, it
	# will just tweak the bit anyway.
1:	nor	t2,t2,zero		# ~(1<<(low%32))
	srl	t0,v0,5			# (low/32)
	sll	t0,2			# (&rq->mask.bits[low/32]-&rq->mask)
	addu	t1,t0,a0		# almost rq->mask.bits[low/32]
	lw	t0,QHSZ*NRQS+4+4(t1)	# rq->mask.bits[low/32]
	move	t8,ra			# D: Preserve return address for later.
	and	t0,t2,t0		# mask off the bit.
	sw	t0,QHSZ*NRQS+4+4(t1)	# store modified mask longword.

	# Calculate the value for rq->low.
	jal	find_first_runq_bit_set	# find rq->low value
	addu	a0,a0,QHSZ*NRQS+4+4	# BD: Arg is &rq->mask

	# Write the new value for rq->low and return it.
	sw	v0,0(t9)	# Store the new rq->low value.
	j	t8			# D: Use saved return address.
 	nop				# BD:
	.set reorder
END(runq_update_low)

/*
 * runq_update
 *
 * This routine checks to see the queue indexed by pri is empty.  If
 * it is not empty, the routine returns.  If it is empty, the bit in the run
 * queue bitmask corresponding to that queue is cleared.  If pri == rq->low,
 * rq->low is updated.
 *
 * This routine should be called at splsched with the run queue locked.
 *
 * Calling sequence:
 *
 *    runq_update( &rq, pri )
 *
 * Register usage:
 *	a0	Address of run queue.
 *	a1	Index of run queue to check.
 *	v0	Return value is new setting of rq->low.
 *	t9	Pointer to rq->low (&rq->low).
 *	t8	Holds return address.
 *	t2	Mask for clearing a bit in the run queue bitmask.
 *	t1	Scratch register.
 *	t0	Scratch register.
 *
 *  NOTE: This routine assumes that find_first_runq_bit_set doesn't trash
 *	  t8 or t9!
 */

/* Size of queue header, amount to shift for queue header. */
#define QHSZ 8
#define QHSH 3	

LEAF(runq_update)
	.set noreorder

	# Calculate address of run queue mask, and see if the queue is
	# empty.  Can just return if the queue is empty.  Otherwise, will
	# need to update the bitmask, so start fetching the bit to clear.
	sll	t0,a1,QHSH		# pri * 8.
	addu	t1,t0,a0		# &rq->runq[pri]
	lw	t0,0(t1)		# Load rq->runq[pri].next.
	li	t2,1			# D:
	beq	t0,t1,1f		# Queue nonempty ? return : continue
	sll	t2,t2,a1		# BD: (1<<(pri%32))
	j	ra			# Return.
	li	v0,-1			# BD: Never loaded rq->low.

	# Clear the bit in the run queue mask.  Note that this code does not
	# protect against the case (pri >= NRQS).  If passed a bogus value, it
	# will just tweak the bit anyway.
1:	nor	t2,t2,zero		# ~(1<<(pri%32))
	srl	t0,a1,5			# (pri/32)
	sll	t0,2			# (&rq->mask.bits[pri/32]-&rq->mask)
	addu	t1,t0,a0		# almost rq->mask.bits[pri/32]
	lw	t0,QHSZ*NRQS+4+4(t1)	# rq->mask.bits[pri/32]
	lw	v0,QHSZ*NRQS(a0)	# D: Load rq->low for later.
	and	t0,t2,t0		# mask off the bit.
	sw	t0,QHSZ*NRQS+4+4(t1)	# store modified mask longword.

	# If pri != rq->low, can return.  Otherwise, need to update rq->low.
	beq	a1,v0,2f		# Keep going if need more work.
	addu	t9,a0,QHSZ*NRQS		# BD: Pointer to rq->low.
	j	ra			# Can return.
	nop				# BD: v0 already contains rq->low.

	# Calculate the value for rq->low.
2:	move	t8,ra			# Preserve return address for later.
	jal	find_first_runq_bit_set	# find rq->low value.
	addu	a0,a0,QHSZ*NRQS+4+4	# BD: Arg is &rq->mask.

	# Write the new value for rq->low and return it.
	sw	v0,0(t9)		# Store the new rq->low value.
	j	t8			# D: Use saved return address.
 	nop				# BD: v0 already contains rq->low.
	.set reorder
END(runq_update)

/*
 * find_first_runq_bit_set
 *
 * Find first bit set in the run queue bit mask.  Used to identify first
 * nonempty run queue.
 *
 * Register usage:
 *	a0   Address of bit mask (&rq->mask).
 *	v0   Index of first bit.  During computation, holds count of bits
 *	     known to be clear.
 *	t1   One longword worth of bits from mask.
 *	t0   Temporary.
 *
 *  NOTE: There was nothing to do during the branch delay slots, so the
 *        target of the branch was propagated back to the branch delay slot,
 *	  and the branch target was moved up one instruction.  Improves
 *	  performance when lots of bits are set.
 *
 *  NOTE: This routine is called by runq_update_low, which stashes values in t8
 *	  and t9 to preserve them accross the call.  This routine must not
 *	  change those registers!
 */
LEAF(find_first_runq_bit_set)
	.set noreorder

	# Load first mask word.  If zero, load second and adjust index.	
	lw	t1,0(a0)		# Load first longword.
	li	v0,0			# D: Init v0 while waiting.
	bne	t1,zero,1f		# If nonzero, skip loading second.
	andi	t0,t1,0xffff		# BDO: start next computation early.

	lw	t1,4(a0)		# Load second longword.
	li	v0,32			# First 32 bits are zero.

	# See if first halfword is 0, and shift if it is.
	andi	t0,t1,0xffff		# Test one half of the longword.
1:	bne	t0,zero,2f		# See if low 16 bits are nonzero.
	andi	t0,t1,0xff		# BDO: start next computation early.
	srl	t1,t1,16		# 16 bits were 0.  Shift them off.
	addu	v0,16			# 16 more bits were zero.

	# See if first byte is 0, and shift if it is.
	andi	t0,t1,0xff		# Test first remaining byte.
2:	bne	t0,zero,3f		# If nonzero, skip.
	andi	t0,t1,0x7f		# BDO: start next computation early.
	srl	t1,t1,8			# 8 bits were 0.  Shift them off.
	addu	v0,8			# 8 more bits were zero.

	# Now have a byte.  If this is not the last byte of the mask,
	# we are guaranteed that one of the bits are set.  If the low seven
	# are clear, we know the eighth is set.  If this _is_ the last byte,
	# then all the bits might be 0, but we do not care since we are going
	# to return 63 in this case anyway.  This allows the use of a table
	# with 128 entries to provide the bit value.


	andi	t0,t1,0x7f
3:	lbu	t0,first_runq_bit_table(t0)
	j	ra				# D: t0 not loaded yet.
	addu	v0,t0				# BD: Add value from table.
	.set reorder
END(find_first_runq_bit_set)

	.data
first_runq_bit_table:
	.byte 7, 0, 1, 0, 2, 0, 1, 0		# entries 0 to 7.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 8 to 15.
	.byte 4, 0, 1, 0, 2, 0, 1, 0		# entries 16 to 23.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 24 to 31.
	.byte 5, 0, 1, 0, 2, 0, 1, 0		# entries 32 to 39.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 40 to 47.
	.byte 4, 0, 1, 0, 2, 0, 1, 0		# entries 48 to 55.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 56 to 63.
	.byte 6, 0, 1, 0, 2, 0, 1, 0		# entries 64 to 71.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 72 to 79.
	.byte 4, 0, 1, 0, 2, 0, 1, 0		# entries 80 to 87.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 88 to 95.
	.byte 5, 0, 1, 0, 2, 0, 1, 0		# entries 96 to 103.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 104 to 111.
	.byte 4, 0, 1, 0, 2, 0, 1, 0		# entries 112 to 119.
	.byte 3, 0, 1, 0, 2, 0, 1, 0		# entries 120 to 127.
	.text
#endif /* RT_SCHED_OPT */
