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
	.asciiz "@(#)$RCSfile: entry.s,v $ $Revision: 1.2.2.14 $ (DEC) $Date: 1993/01/22 17:27:42 $"
	.text

/*
 * Modification History: machine/alpha/entry.s
 *
 * 11-May-91 -- afd
 *	Ported to OSF.
 *
 * 10-Sep-90 -- rjl
 *	Created this file for Alpha support.
 */

#include <machine/asm.h>
#include <machine/regdef.h>
#include <machine/pal.h>
#include <machine/vmparam.h>
#include <mach/alpha/vm_param.h>

#define VPTBASE	0xfffffffc00000000

/************************************************************************
 *									*
 *	OSFpalcode version of the entry code				*
 *									*
 ************************************************************************/

/*
 * Kernel entry point table
 */
	.text
	.globl	start
	.ent	start
	.globl	eprol
	.frame	sp,0,ra
	.mask	0,0
/*
 * This is the real entry point for the kernel image.  The boot programs
 * enter here with a0 = pfn of the first available page and a1 = the ptbr.
 * We are running under OSFpal at this point.
 */
start:
eprol:
	br	zero,_realstart		# starting point of the kernel
	.end
/*
 * This is the restart entry point, which is used to generate a crash dump.
 */
	.globl	restart
	.ent	restart
restart:
	br	zero,doadump		# dump core to dump device	
	.end

/*
 * realstart
 *
 * This routine sets up the global ptr (gp) and calls several kernel
 * initialization routines.
 *
 * It passes along the argc, argv, & environ from the bootpath to alpha_init.
 */
	.ent	_realstart
	.frame	sp,8,ra
	.mask	M_RA,-8
	.globl	kgp			# make symbol available to dbx
	.align	3			# force quadword alignment
	.set	noreorder
_realstart:
	lda	sp,-8(sp)
	br	t0, 1f			# addr of pad bytes preceding .quad
kgp:	.quad	_gp
1:	ldq	gp,0(t0)		# load permanent value for gp
	stq	zero,0(sp)		# init start frame for debugger
	.set	reorder

	bis	a0,zero,s0		# a0 has the first usable pfn
	bis	a1,zero,s1		# a1 has the page table

/*
 * Save argc, argv, envp
 */
	bis     a2,zero,s2		# argc
	bis     a3,zero,s3		# argv
	bis     a4,zero,s4		# environ

	bis	gp,zero,a0		# tell palcode it's value
	call_pal PAL_wrkgp

/*
 * Setup the entry point addresses for the palcode
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
 * The first usable pfn (in s0) and the current page table base register
 * value (in s1) are passed into alpha_bootstrap, which passes them along
 * to pmap_bootstrap, both of which return the kernel startup ppcb.  This
 * value is used below to switch the kernel's virtual memory mapping.
 *
 * The argc (in s2) and argv (in s3) values passed to "start" are also
 * provided to alpha_bootstrap, which needs to copy the startup arguments
 * from their current location (at old addresses) to kernel storage which
 * will be accessible when the mapping is changed.
 *
 * Note that alpha_bootstrap is invoked under the old mapping, before the
 * kernel's bss is zeroed, and before kdebug can be used.  After it calls
 * kdebug_bootstrap, which copies the reserved-for-software field of the
 * rpb (rpb_software) into the global kernel variable kdebug_bkpt_vector,
 * breakpoints will work.
 */
	bis	s0,zero,a0		# 1st arg is pfn
	bis	s1,zero,a1		# 2nd arg is ptbr
	bis     s2,zero,a2		# 3rd arg is argc
	bis     s3,zero,a3		# 4th arg is argv
	jsr	ra,alpha_bootstrap	# bootstrap the kernel
	bis	v0,zero,s0		# save the proc 0 ppcb

/*
 * Setup the virtual page table pointer.  This must be done here just before
 * switching to the new PCB and the page table mapping to avoid taking
 * a tb hit with a new pointer and the old set of mappings.
 */
	ldiq	a0,VPTBASE
	call_pal PAL_wrvptptr

/*
 * Switch from boot environment to kernel startup ppcb.
 */
	bis	gp,zero,s1
	bis	s0,zero,a0
	call_pal PAL_swpctx
	bis	s1,zero,gp

/*
 * At this point we're running in the context set up by pmap_bootstrap.
 * Push a zero ra value onto the new stack to parallel what was done at
 * entry.  Then call alpha_init, which calls main(), which starts "init".
 */
	lda	sp,-8(sp)
	stq	zero,0(sp)
	bis     s2,zero,a0              # pass argc,argv,environ
	bis     s3,zero,a1
	bis     s4,zero,a2
	jsr	ra,alpha_init		# alpha_init(argc, argv, environ)
1:
	call_pal PAL_halt		# should never return here
	br	zero,1b			#  but if it does

	.end
