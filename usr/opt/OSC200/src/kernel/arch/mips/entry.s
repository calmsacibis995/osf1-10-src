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
 *	@(#)$RCSfile: entry.s,v $ $Revision: 1.2.3.5 $ (DEC) $Date: 1992/07/08 08:35:56 $
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
 * derived from entry.s	2.2	(ULTRIX/OSF)	12/15/90
 */
/*
 * @(#)entry.s	4.3	(ULTRIX)	9/6/90
 */
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/*
 *		Modification History
 *
 * 17-Jan-91 -- Don Dutile
 *	Merged to v4.2.  Added save and restore of a3, the vector boot
 *	variable.
 *
 * 06-Sep-90 -- Randall Brown
 *	Cleanup of some code.  Removed code no longer being used. 
 *
 * 09-Aug-90 -- Randall Brown
 *	Added the variable sr_usermask, so that the mask that get put
 *	into the status register can vary from machine to machine .
 *
 * 20-Jul-1990	burns
 *	first hack at moving to OSF/1 (snap3)
 *
 * 16-Apr-90 -- jaw/gmm
 *	move kstackflg to cpudata structure.
 *
 * 01-Dec-89 -- bp
 *	fixed the SAS bss alignment problem for MDMAPSIZE
 *
 * 09-Nov-89 -- bp
 *	put mips page table entrys in bss
 *	be carefull adding entries because of an .lcomm problem
 *	which is commented on below
 *
 * 03-Oct-89 -- gmm
 *	SMP changes (remove kstackflag, secondary startup etc)
 *
 * 10-July-89 -- burns
 *	Support for DECsystem 58xx.
 *
 * 20 Feb 89 -- Kong
 *	Added to Sysmap 1026 PTEs to map the 4Mb memory space and 8Kb
 *	I/O space of the Qbus of the Mipsfair.
 *
 * 07 Dec 88 -- depp
 *	Moved the startup stack off the PROM stack (as MIPSco did it), to
 *	prevent PROM area corruption.  The PROM stack now exists elsewhere
 */


#include <confdep.h>

#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>

#include <machine/machparam.h>
#include <machine/vmparam.h>
#include <mach/mips/vm_param.h>
#include <machine/cpu.h>
#include <machine/pcb.h>
#include <hal/entrypt.h>	/* prom entry point definitions */

#include <kern/xpr.h>

#include <assym.s>
#include <mach/mips/vm_param.h>

/*
 * Save area for boot params
 */
	LBSS(_argc, 4)
	 BSS(_argv, 4)
	LBSS(_environ, 4)
	LBSS(_vector, 4)
/*
 * We better get a stack for ourselves as soon as possible.
 * Note: the "intstack" misnomer is because the machine-independent
 * code knows about this symbol, which must therefore be defined.
 */
	.text				# so that it goes upfront
	.align	2
	.globl	pcb_initial_space

pcb_initial_space:
	.space	2*NBPG
	.globl	intstack
intstack:
	.space	INTSTACK_SIZE-4
	.globl	boot_stack
boot_stack:
	.space	4

/*
 * Kernel entry point
 */
ENTRY_FRAME=	(4*4)+4+4		/* 4 argsaves, old fp, and old sp */
EXPORT(eprol)				/* for benefit of profiling */
NESTED(start, ENTRY_FRAME, zero)
        j       _realstart              /* kernel entry point */
        j       _coredump               /* dump core to config'ed dump dev */
#ifdef notdef
        j       _xprdump                /* dumpt  
        j       _xprtail                /* dump tail of trace buffer */
        j       _msgdump                /* dump msg buffer to console */
#endif /* notdef */

/*
 * Kernel initialization
 */
_realstart:
	/*
	 * Now on prom stack; a0, a1, a2, and a3 contain argc, argv, environ
	 * and vector from boot.  Stay on that stack, for the time being.
	 */

        /* jal  _dz_setup */
        /* above strickly for porting ... careful about trashing a0-a3  */

	la	gp,_gp
	/*
	 * Save invocation parameters
	 */
	sw	a0,_argc
	sw	a1,_argv
#ifdef	mips
	/* accomodate screwy Ultrix loader with similarly screwy fix */
	bne	a2,zero,1f
	la	a2,0xa000376c	/* XXX magic: prom environ */
1:
#endif	/* mips */
/* end from OSF */
	sw	a2,_environ
	sw	a3,_vector
	/*
	 * Check whether we should load kdebug
	 */
	jal	kdebug_init		# see if debugging requested
	la	gp,_gp			# this time, screwy prom_exec()
	/*
	 *  Reload arguments and get started
	 */
	lw	a0,_argc
	lw	a1,_argv		# kdebug_init may have to "change" argv
	lw	a2,_environ
	lw	a3,_vector
1:
/* set up virtual PCB */
	li	a3,0x7ffff000
	la	a1,pcb_initial_space
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
        la      a1,pcb_initial_space
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
        li      sp,PCB_WIRED_ADDRESS-ENTRY_FRAME          # switch stack
	li	k0,PCB_WIRED_ADDRESS	# address of PCB
	sw      gp,PCB_KSTACK(k0)	# running on kernel stack now
        sw      zero,ENTRY_FRAME-4(sp)  # zero old ra for debuggers

	lw	a0,_argc
	lw	a1,_argv		# kdebug_init may have to "change" argv
	lw	a2,_environ
	lw	a3,_vector

	.set noreorder
	nop
	mtc0	zero,C0_TLBHI		# clean a little
	nop
	li	v0,KPTEADDR		# kernel's pte base; was KPTEBASE in ULTRIX
	nop
	mtc0	v0,C0_CTXT
	nop
	.set reorder

	/*
	 * End of cold code.  mips_init(argc,argv,environ) will do the rest,
	 * including creating the first thread and making it start at main().
	 */
	jal	mips_init		# call mips_init(argc, argv, environ)

	/* NOTREACHED */
	END(start)
