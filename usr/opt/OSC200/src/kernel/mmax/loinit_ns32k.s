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
 *	@(#)$RCSfile: loinit_ns32k.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:24 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include "assym.s"

#include "mmax_xpc.h"
#include "mmax_apc.h"
#include "mmax_dpc.h"
#include "fast_csw.h"
#include "cpus.h"

#include "mmax/cpudefs.h"
#include "mmax/mlmacros.h"
#include "mmax/psl.h"
#include "mmax/mmu.h"
#include "mmax/fpu.h"
#if	MMAX_XPC
#include "mmax/cfg.h"
#endif	MMAX_XPC

 #
 # CPU Initialization parameters
 #
/*
 * CPU initialization and manipulation; hide trivial changes in names.
 */
#if	MMAX_XPC
#define	PSR_INIT	PSR_S+PSR_U+PSR_I+PSR_V	/* Sys sp/mode, no ints/ovf */
#define	MMU_ON(label)	lmr	mcr, $MCR_TU+MCR_TS+MCR_DS
#define	MMU_OFF(label)	lmr	mcr, $MCR_MAGIC
#define	FPU_PRES_BIT	XPCCSR_32381_PRES_BIT
#define	CPU_REG_CSR	XPCREG_CSR
#define	FPA_ON_BITS	CFG_I|CFG_M|CFG_IC|CFG_DC|CFG_F|CFG_PF
#define	FPU_ON_BITS	CFG_I|CFG_M|CFG_IC|CFG_DC|CFG_F
#define	FPX_OFF_BITS	CFG_I|CFG_M|CFG_IC|CFG_DC
#define	FPA_ON(label)	lprd	cfg, $(FPA_ON_BITS)
#define	FPU_ON(label)	lprd	cfg, $(FPU_ON_BITS)
#define	FPX_OFF(label)	lprd	cfg, $(FPX_OFF_BITS)
#define	PSA_BOARD_TYPE	PSA_XPC_TYPE
#if	PDEBUG && notyet
#define	OS_START_LED	LED(0x80)	/* started up -- single green led */
#else	PDEBUG && notyet
#define	OS_START_LED
#endif	PDEBUG && notyet
#endif	MMAX_XPC

#if	MMAX_APC
#define	PSR_INIT	PSR_S+PSR_U+PSR_I	/* Sys sp/mode, no ints */
#define	MMU_ON(label)	lmr	few, $FEW_TU+FEW_TS+FEW_DS
#define	MMU_OFF(label)	lmr	few, $FEW_MAGIC
#define	FPU_PRES_BIT	APCCSR_32081_PRES_BIT
#define	CPU_REG_CSR	APCREG_CSR
/* icu, fpu, mmu, custom, ff, 4K pages, fast custom, fast mmu protocol */
#define	FPA_ON_BITS	[i, f, m, c, ff, fm, fc, p]
#define	FPU_ON_BITS	[i, f, m,        fm,     p]
#define	FPX_OFF_BITS	[i,    m,        fm,     p]
#define	FPA_ON(label)	setcfg	FPA_ON_BITS
#define	FPU_ON(label)	setcfg	FPU_ON_BITS
#define FPX_OFF(label)	setcfg	FPX_OFF_BITS
#define	PSA_BOARD_TYPE	PSA_APC_TYPE
#if	PDEBUG && notyet
#define	OS_START_LED	LED(0x1)	/* started up -- single green led */
#else	PDEBUG && notyet
#define	OS_START_LED
#endif	PDEBUG && notyet
#endif	MMAX_APC

#if	MMAX_DPC
#define	PSR_INIT	PSR_S+PSR_U+PSR_I	/* Sys sp/mode, no ints */
#define	MMU_ON(label)	lmr	msr, $MSR_TRANSUSER+MSR_TRANSSUP+MSR_USEPTB1
#define	MMU_OFF(label)	lmr	msr, $MSR_MAGIC
#define	FPU_ON_BITS	[m, f]				# DPC [ MMU + FPU ]
#define	FPX_OFF_BITS	[m   ]
#define	FPU_ON(label)	setcfg	FPU_ON_BITS
#define	FPX_OFF(label)	setcfg	FPX_OFF_BITS
#define	PSA_BOARD_TYPE	PSA_DPC_TYPE
#if	PDEBUG && notyet
#define	OS_START_LED	LED(0x0)	/* start up worked -- clear leds */
#else	PDEBUG && notyet
#define	OS_START_LED
#endif	PDEBUG && notyet
#endif	MMAX_DPC

 #
 #	This entry point receives control from the embedded OS debugger,
 #	dbmon, via the system bootstrap program, Sysboot. The boot
 #	processor enters the OS here, the non-boot processors are started at
 #	'start.notboot', so there's no locking code.
 #
 #	r7		- Pointer to boot structure

/*
 * Some multimax cpu boards support a fast floating point option.
 * We avoid the issue of binding floating point binaries to boards
 * with compatible options by asserting that all active boards must
 * have the same floating point options.
 */
	.data
	.globl	fpapresent
fpapresent:	.int	0
	.text

	.globl	os_start
os_start:
	bicpsrw	$(PSR_INIT)		/* initialize psr to known state */
#if	MMAX_XPC || MMAX_APC
	tbitd	$FPU_PRES_BIT, @CPU_REG_CSR	/* check for accelerator */
	bfs	nofpa
	addqd	$1, fpapresent		/* found accelerator */
	FPA_ON(os_start)		/* turn it on */
	br	fp_x
nofpa:
#endif	MMAX_XPC || MMAX_APC
	FPU_ON(os_start)		/* standard fpu, turn it on */
fp_x:
	lfsr	$FSR_RM_NEAREST		/* magic fpu init */
	MMU_OFF(os_start)		/* make sure mmu translation is off */
#if	MMAX_XPC
	lprd	dcr, $0			/* clear debug control register */
	movd	$(XPCCSR_NO_SYSNMI | XPCCSR_BURST_ENABLE), r0
	movd	r0, @XPCREG_CSR	/* initialize XPC board state */
	movd	$XPCCTL_NORMAL_BITS, @XPCREG_CTL
	movd	@XPCREG_ERR, r0	/* clear error status register */
#endif	MMAX_XPC
	addr	_eintstack, r0		# Base of processor stack
	lprd	sp, r0			# Load stack pointer
	lprd	fp, r0			# Load frame pointer
	OS_START_LED

 # Save entry modpsr for non-boot processors

	addr	_modbegin, r0		# Address of a 'mod' table
	lprw	mod, r0			# Load mod register
	movw	r0, _OSmodpsr	# Save for non-boot cpus and thread_start
	movw	$(PSR_I), _OSmodpsr + 2	# Set this up for thread_start

	addr	_Vectors, r2		# Get our interrupt base
	lprd	intbase, r2		# Load new interrupt base

 #	Call the Multimax initialization routine
 #	mmax_init(bootp)
 #	struct	boot	*bootp
 #	which clears memory, sets up various important variables, massages
 #	the Boot data structure, causes board-state to be initialized, etc.
 #	Upon return the maps are setup and ptb0 is loaded
 #

	movd	r7,r0			# Address of Boot data structure
	jsr	_mmax_init

	/*
	 * Turn on the memory management and the extended translation 
	 * buffer for the master.  The hardware requires the ETLB to
	 * be enabled after virtual memory is turned on.  This code
	 * only works if kernel virtual = physical.
	 */
	lmr	ptb0,_ptbr		# load ptb0
	MMU_ON(os_start)		# into virtual mode
#if	MMAX_DPC && MMAX_ETLB
	/*
	 * Additional help is required for the DPC.
	 */
	GETCPUID(r0)			# Get the CPU number 
	xord	$DPCCTL_ETLB_OFF,@_Dpc_ctl[r0:d] # Turn on ETLB. It's off now.
	movd	@_Dpc_ctl[r0:d],r0	# Get the new value
	xord	$DPCCTL_FIX,r0		# Flip around appropriate bits 
	movd	r0,@DPCREG_CTL		# Load the register 
#endif	MMAX_DPC && MMAX_ETLB

	addr	_eintstack, r0
	lprd	sp, r0			# Load stack pointer
	lprd	fp, r0			# Load frame pointer

	jsr	_setup_main		# returns initial thread in r0

	/*
	 *	Set up initial PCB and change to kernel stack in context
	 *	of first thread.  Most pcb inits were done by pcb_init
	 *	(called from thread_create); all that's missing is the pc.
	 */

	movd	THREAD_PCB(r0),r1		# get virtual address of PCB
#if	FAST_CSW
	movd	PCB_SSP(r1),r2			# push
	addqd	$-4,r2				#   start address
	addr	start.thread,0(r2)		#     on to
	movd	r2,PCB_SSP(r1)			#       stack
#else	FAST_CSW
	addr	start.thread,PCB_PC(r1)		# initial pc
#endif	FAST_CSW
	
	jsr	_load_context

	/* _load_context doesn't return, we wind up at start.thread
		in a thread context.  */

 # Call main of process - to complete kernel initialization.
start.thread:

#if	FAST_CSW
	ENBINT				# no longer in load_context
#endif	FAST_CSW
 # Call main of process - to complete kernel initialization.

	jsr	_main			# Call OS initialization code

	.globl	rett_ast
	.globl	_start_init
_start_init:

/* proc[1] == /etc/init now running here; start other cpus and run icode */

	tbitd	$(RB_B_MULTICPU), _boothowto	# Multiprocessor boot ?
	bfc	notmulti		# If fc, no
	addr	start.notboot, r0	# Start entry for non-boot cpus.
	jsr	_start_cpus		# go start them.
notmulti:

	/*
	 *	About to exec init, but first clean up stack.
	 */
	GETCPUID(r0)				# Cpu number.
	movd	_active_threads[r0:d],r0	# This cpu's thread.
	movd	THREAD_KERNEL_STACK(r0),r0	# Thread's kernel stack.
	addd	$KERNEL_STACK_SIZE, r0		# Go to end of stack.
	lprd	sp,r0				# and load
	lprd	fp,r0				# registers

	/*
	 *	Now set up stack as if a trap had occurred.
	 */
	movqd	$0,tos			# Make room for modpsr
	movw	$PSR_I+PSR_S+PSR_U,2(sp) # Psr => interrupts, usp, user mode
	movw	$0x20,0(sp)		#  legal mod value
	movqd	$0,tos			# Save location for pc to return to
	BUILD_FRAME			# Make this look like a system call
	jsr	_MMAX_load_init_program	# load_init_program()
	jump	@rett_ast		# going directly to user.
					# rett_ast is in lotrap_ns32k.s

/*
 *	start.notboot is the entry point for the slaves.  
 *	start_cpus(start.notboot) causes them to start up directly here.
 *	As each CPU starts, it checks its fpu type to ensure that all
 * 	configured processors have the same kind of fpu.
 */

	.globl	_ptbr
	.globl	_slave_main

fpafpumis:
	PANIC(@fpafpu_msg, $0, $0)	# Panic
fpafpu_msg:
	.asciz	"FPA/FPU Mismatch"

start.notboot:				# Non-boot entry point

 # Load processor mod register with value left behind by boot processor.

	lprw	mod, _OSmodpsr		# Load mod NOW!!!

 # Initialize processor

	bicpsrw	$(PSR_INIT)
#if	MMAX_XPC || MMAX_APC
	tbitd	$FPU_PRES_BIT, @CPU_REG_CSR
	bfs	notboot.nofpa		# if fpu present, handle below
	FPA_ON(start.notboot)		# we have fpa, start it up
	cmpqd	$0, fpapresent		# make sure master cpu had fpa, too
	beq	fpafpumis		# otherwise, panic
	br	notboot.fp_x
notboot.nofpa:
#endif	MMAX_XPC || MMAX_APC
	FPU_ON(start.notboot)		# we have fpu, start it up
#if	MMAX_XPC || MMAX_APC
	cmpqd	$1, fpapresent		# make sure master cpu had fpu, too
	beq	fpafpumis		# otherwise, panic
#endif	MMAX_XPC || MMAX_APC
notboot.fp_x:
	lfsr	$FSR_RM_NEAREST		# magic fpu init
	MMU_OFF(start.notboot)
#if	MMAX_XPC
	lprd	dcr, $0			/* clear debug control register */
	movd	$(XPCCSR_NO_SYSNMI | XPCCSR_BURST_ENABLE), r0
	movd	r0, @XPCREG_CSR
	movd	$XPCCTL_NORMAL_BITS, @XPCREG_CTL
	movd	@XPCREG_ERR, r0	/* clear error status register */
#endif	MMAX_XPC

 # Get a valid stack we can live with

	GETCPUID(r0)			# Get the processor number of this CPU
	movd	_interrupt_stack[r0:d],r0	# get its stack base
	addd	$INTSTACK_SIZE,r0	# go to end of stack
	lprd	sp,r0			# load stack
	lprd	fp,r0			#   registers

	addr	_Vectors, r2		# Get our interrupt base
	lprd	intbase, r2		# Load new interrupt base

	GETCPUID(r0)			# processor number of this CPU
	jsr	_startCPU		# start it up.

	/* go into mapped mode - this assumes kernel virtual=physical */
	lmr	ptb0,_ptbr		# load ptb0
	MMU_ON(start.notboot)		# into virtual mode
#if	MMAX_DPC && MMAX_ETLB
	/*
	 * Extra help needed for the DPC.  Turn on the ETLB.
	 */
	GETCPUID(r0)			# Get the CPU number 
	xord	$DPCCTL_ETLB_OFF,@_Dpc_ctl[r0:d] # Turn on ETLB. It's off now.
	movd	@_Dpc_ctl[r0:d],r0	# Get the new value
	xord	$DPCCTL_FIX,r0		# Flip around appropriate bits 
	movd	r0,@DPCREG_CTL		# Load the register 
#endif	MMAX_DPC && MMAX_ETLB

	/* acknowledge that this processor is started */
	jsr	_ack_cpustart

	/* Call slave_main to finish the
		initialization */

	ENBINT				# turn on interrupts
	bispsrw	$(PSR_I)
	jsr	_slave_main
	wait


#if	MMAX_XPC || MMAX_APC
/*
 * CPUicuenable: initialize the ICU and enable all the correct interrupts 
 *      for this CPU
 *
 * ARGUMENTS: None
 *
 * USAGE: This routine is called once for each CPU in the system to setup
 *	the ICU, including the time-slice end counters.
 */

CPUicu_start:	.asciz	"at CPUicuenable start"
CPUicu_midway:	.asciz	"right after clearing IPND"
CPUicu_end:	.asciz	"at CPUicuenable end"
        .globl  _CPUicuenable
_CPUicuenable:

	/*addr	@CPUicu_start, r1*/
	/*ICU_TRACE(CPUlab1)*/
/*
 * Initialize Master ICU
 */
        movd    $M_ICU_BASE, r0             /* Get ICU base address */
                                            /* Freeze counters, ints */
        movb    $CFRZ + COUTD + FRZ + NTAR, IMCTL(r0)
        movb    $CCON, ICCTL(r0)            /* init counters, and halt them */
        movb    $CVECT_M, ICIPTR(r0)        /* ICU counter vector pin */
        movb    $MICUBIAS_M, ISVCT(r0)      /* Vector bias */
/*
 * Initialize LCSV and HCSV counter starting values. Counters are frozen
 * and halted.
 */
        movb    $CBEGINHH, (IHCSV + RBIAS)(r0)
        movb    $C_HUGE_HL, IHCSV(r0)
        movb    $CBEGINLH, (ILCSV + RBIAS)(r0)
        movb    $CBEGINLL, ILCSV(r0)
/*
 * Write starting values into LCCV AND HCCV. Use current values so
 * there is a correct start of the timers.
 */
        movb    $CBEGINHH, (IHCCV + RBIAS)(r0)
        movb    $C_HUGE_HL, IHCCV(r0)
        movb    $CBEGINLH, (ILCCV + RBIAS)(r0)
        movb    $CBEGINLL, ILCCV(r0)
/*
 * Initialize CICTL.
 */
        movb    $WENH + WENL,ICICTL(r0)    /* Clear count cntl, enable write */
        movb    $WENH + CIEH,ICICTL(r0)    /* Enable H/L interrupts */
/*
 * Initialize IPS, PDIR, OCASN, and PDAT.
 */
        movb    $IOCONFIG_M, IPS(r0)        /* select interrupt-I/O config. */
        movb    $PORTDIR_M, IPDIR(r0)       /* set port direction to input */
        movb    $CWAVE, IOCASN(r0)          /* set counter wave assignment */
        movb    $PDATAI, IPDAT(r0)          /* initialize port data */
/*
 * CASCADE defines which pins are cascade type sources.
 */
        movb    $(CASCADE_M & 0xff), ICSRC(r0)
        movb    $((CASCADE_M > 8) & 0xff), (ICSRC + RBIAS)(r0)
/*
 * Initialize the pending register.
 */
        movb    $0xff, IELTG(r0)            /* Insure level triggering to... */
        movb    $0xff, (IELTG + RBIAS)(r0)
        movb    $0x40, IPND(r0)             /* dismiss pending interrupts and */
        movb    $0x40, (IPND + RBIAS)(r0)
        movb    $0x00, ISRV(r0)             /* dismiss interrupts in service. */
        movb    $0x00, (ISRV + RBIAS)(r0)
/*
 * Set trigger polarity and mode.
 */
        movb    $(TPL_M & 0xff), ITPL(r0)
        movb    $((TPL_M > 8) & 0xff), (ITPL + RBIAS)(r0)
        movb    $(ELTG_M & 0xff), IELTG(r0)
        movb    $((ELTG_M > 8) & 0xff), (IELTG + RBIAS)(r0)
/*
 * Set up first priority, normally 0 for nested mode.
 */
        movb    $PRIOR, IFPRT(r0)
/*
 * Enable all interrupts in mask (use ISRV to mask them).
 */
        movb    $((~ICU_ENABLE_ALL) & 0xff), IMASK(r0) /* set interrupt mask */
        movb    $(((~ICU_ENABLE_ALL) > 8) & 0xff), (IMASK + RBIAS)(r0)
        movb    $NTAR, IMCTL(r0)            /* enable interrupt sampling */
#if	MMAX_APC
/*
 * Initialize Slave ICU
 */
        movd    $S_ICU_BASE, r0             /* Get ICU base address */
                                            /* Freeze counters, ints */
        movb    $CFRZ + COUTD + FRZ + NTAR, IMCTL(r0)
        movb    $CCON, ICCTL(r0)            /* init counters, and halt them */
        movb    $CVECT_S, ICIPTR(r0)        /* ICU counter vector pin */
        movb    $MICUBIAS_S, ISVCT(r0)      /* Vector bias */
/*
 * Initialize LCSV and HCSV counter starting values. Counters are frozen
 * and halted.
 */
        movb    $CBEGINHH, (IHCSV + RBIAS)(r0)
        movb    $C_HUGE_HL, IHCSV(r0)
        movb    $CBEGINLH, (ILCSV + RBIAS)(r0)
        movb    $CBEGINLL, ILCSV(r0)
/*
 * Write starting values into LCCV AND HCCV. Use current values so
 * there is a correct start of the timers.
 */
        movb    $CBEGINHH, (IHCCV + RBIAS)(r0)
        movb    $C_HUGE_HL, IHCCV(r0)
        movb    $CBEGINLH, (ILCCV + RBIAS)(r0)
        movb    $CBEGINLL, ILCCV(r0)
/*
 * Initialize CICTL.
 */
        movb    $WENH + WENL, ICICTL(r0)    /* Clear count cntl, enable write */
        movb    $WENH + CIEH, ICICTL(r0)    /* Enable H/L interrupts */
/*
 * Initialize IPS, PDIR, OCASN, and PDAT.
 */
        movb    $IOCONFIG_S, IPS(r0)        /* select interrupt-I/O config. */
        movb    $PORTDIR_S, IPDIR(r0)       /* set port direction to input */
        movb    $CWAVE, IOCASN(r0)          /* set counter wave assignment */
        movb    $PDATAI, IPDAT(r0)          /* initialize port data */
/*
 * CASCADE defines which pins are cascade type sources.
 */
        movb    $(CASCADE_S & 0xff), ICSRC(r0)
        movb    $((CASCADE_S > 8) & 0xff), (ICSRC + RBIAS)(r0)
/*
 * Initialize the pending register.
 */
        movb    $0xff, IELTG(r0)            /* Insure level triggering to... */
        movb    $0xff, (IELTG + RBIAS)(r0)
        movb    $0x40, IPND(r0)             /* dismiss pending interrupts and */
        movb    $0x40, (IPND + RBIAS)(r0)
        movb    $0x00, ISRV(r0)             /* dismiss interrupts in service. */
        movb    $0x00, (ISRV + RBIAS)(r0)
/*
 * Set trigger polarity and mode.
 */
        movb    $(TPL_S & 0xff), ITPL(r0)
        movb    $((TPL_S > 8) & 0xff), (ITPL + RBIAS)(r0)
        movb    $(ELTG_S & 0xff), IELTG(r0)
        movb    $((ELTG_S > 8) & 0xff), (IELTG + RBIAS)(r0)
/*
 * Set up first priority, normally 0 for nested mode.
 */
        movb    $PRIOR, IFPRT(r0)
/*
 * Enable all interrupts in mask (use ISRV to mask them).
 */
        movb    $(((~ICU_ENABLE_ALL) > 16) & 0xff), IMASK(r0)
        movb    $(((~ICU_ENABLE_ALL) > 24) & 0xff), (IMASK + RBIAS)(r0)
        movb    $NTAR, IMCTL(r0)            /* enable interrupt sampling */
#endif	MMAX_APC
/*
 *  Done, return
 */
	DISINT
	/*addr	@CPUicu_end, r1*/
	/*ICU_TRACE(CPUlab1)*/
	ret	$0
#endif	MMAX_XPC || MMAX_APC



/*  Routine to load ICU Counter with TSE value at CPU Startup
 *
 *  called from startrtclock at boot time
 *
 */

#if	MMAX_XPC || MMAX_APC
		.globl _upc_tse
		.globl _start_tseval
		.data
_upc_tse:	.double 1
_start_tseval:	.int	0		# will be set to 92160 from param file
		.text

ENTRY(icu_tseinit)
	movd	$M_ICU_BASE,r0		/* TSE timer's in master (32 bits) */
	andb	$TIMERSTOPH,ICCTL(r0)	/* Stop the clock !		   */
	movd	@_start_tseval, @_upc_tse
	movd	@_upc_tse,tos		/*   probably divide by hz here)   */
	movb	0(sp),ILCSV(r0)		/* Must load a byte at a time...   */
	movb	1(sp),ILCSV+RBIAS(r0)
	movb	2(sp),IHCSV(r0)
	movb	3(sp),IHCSV+RBIAS(r0)
	adjspb	$-4			/* Clean up			   */
	movb	$CRUHLSEL,ICCTL(r0)	/* Kick ICU off now...		   */
	ret	$0
#endif	MMAX_XPC || MMAX_APC
