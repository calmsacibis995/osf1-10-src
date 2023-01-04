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
 *	@(#)$RCSfile: dbmon.s,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 93/02/01 10:30:25 $
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 #
 # **********************************************************************
 # *	 Copyright (C) 1984 Hydra Computer Systems, Inc.
 # * ALL RIGHTS RESERVED. Licensed Material - Property of Hydra Computer
 # * Systems, Inc. This software is made available solely pursuant to the
 # * terms of a software license agreement which governs its use.
 # * Unauthorized duplication, distribution or sale are strictly
 # * prohibited.
 # *
 # * Module Function:
 #	This module contains the multiprocessor debugger that is linked with
 #	the operating system and communicates with the host end via serial
 #	line.
 # *
 # * Original Author: Tony Anzelmo	Created on: 12/01/84
 # *
 # * Revision Control Information:
 # *
 # **********************************************************************
 #
 */
#include "assym.s"

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

#include <mmax/mlmacros.h>
#include <mmax/psl.h>
#include <mmax/mmu.h>
#include <mmax/icu.h>
#include <mmax/cpudefs.h>
#include <mmax/pte.h>
#if	MMAX_XPC || MMAX_APC
#include <mmax/duart.h>
#endif


 # Exports

	.globl		start		# dbmon entry point
	.globl		dnmi		# nmi interrupt handler
	.globl		dbpt		# bpt interrupt handler
	.globl		dtrc		# trc interrupt handler
	.globl		_in_dbmon	# flag indicating code in dbmon

 # Imports

	.globl		os_start	# OS entry point
	.globl		_gen_sysnmi	# Generate system nmi function
	.globl		_debug_vaddr	# Virtual address for memory manip.
	.globl		_debug_pte	# Virtual address of pte for above.
#if	MMAX_DPC
 	.globl		_Dpc_ctl	# Dpc control register
 	.globl		_Dpc_nmidiscnt	# Dpc nmi disable count
#endif

 # Local macros

#define NBPW	4
#define MAXCPUS	64
#define NBPG	1024
#define PGSHIFT 10

#if	MMAX_XPC || MMAX_APC
#define NBPP_HW NBPP
#define HW_BPPSHFT BPPSHFT
#endif

#define VECTOR(label) \
	.word	0	; \
	.word	((label)-_dbmodbegin)

#define	DEB_DEBUG	0

#if	DEB_DEBUG
#define	DPRINTF(fmt,arg)	PRINTF(fmt,arg)
#else
#define	DPRINTF(fmt, arg)
#endif
 #
 # This dsect defines the format of the stack as the monitor is entered
 # a 32-bit entry is arranged for each registers, and then any register
 # can be changed by looking its name up in a table and changing the stack
 # copy. The change takes effect when the user program is restarted.
 #
	.text				# (``CDW'' = ``changes don't work'')
	.set	e_echo,		0		# echo flag
	.set	e_fsr32,	e_echo+4	# 32-bit fake copy of the psr
	.set	e_psr32,	e_fsr32+4	# 32-bit copy of the psr
	.set	e_sb,		e_psr32+4	# copy of mod table entry	CDW
	.set	e_is,		e_sb+4		# calculated from stack level	CDW
	.set	e_mod32,	e_is+4
	.set	e_fp,		e_mod32+4
	.set	e_us,		e_fp+4
	.set	e_intbase, 	e_us+4
	.set	e_r7,		e_intbase+4
	.set	e_r6,		e_r7+4
	.set	e_r5,		e_r6+4
	.set	e_r4,		e_r5+4
	.set	e_r3,		e_r4+4
	.set	e_r2,		e_r3+4
	.set	e_r1,		e_r2+4
	.set	e_r0,		e_r1+4
	.set	e_traptype, 	e_r0+4		# Trap type
	.set	e_pc,		e_traptype+4
	.set	e_mod0,		e_pc+4		# its original place on the stack
	.set	e_psr0,		e_mod0+2	# ""
	.set	e_ssp,		e_psr0+2	# Saved system stack pointer
	.set	e_frame,	e_ssp+4
 #
 # Static data.

	.data

 # Debug messages

dbpt.msg:
	.asciz	"Dbpt entry\n"
dbgtrap.msg:
	.asciz	"Dbgtrap entry, type %d\n"
dbgtrapcpu.msg:
	.asciz	"Debugger entry on cpu %d\n"
dbgbid.msg:
	.asciz	"Dbg.bid entry\n"
sysnmi.msg:
	.asciz	"Sysnmi generated from cpu %d\n"
suspend.msg:
	.asciz	"Suspended\n"
dbgget0.msg:
	.asciz	"Dbg.get0\n"
dbgget.msg:
	.asciz	"Dbg.get\n"
mstart.msg:
	.asciz	"Mstart\n"
nostep.msg:			
	.asciz	"Nostep psr 0x%x\n"
step.msg:		
	.asciz	"Step psr 0x%x\n"

 # the input line buffer

#define ibufsize	64
#define ibuffer		-ibufsize(fp)

 # ascii fun

	.set	ASC_BS,	0x8		# code for backspace
	.set	ASC_LF,	0xa		# code for line feed
	.set	ASC_CR,	0xd		# code for carriage return
	.set	ASC_XON, 0x11		# code for control/q
	.set	ASC_XOFF, 0x13		# code for control/s
	.set	ASC_ESC, 0x1b		# code for escape
	.set	ASC_SP,	0x20		# code for space
	.set	ASC_AST, 0x2a		# code for an asterisk
	.set	ASC_0,	0x30		# code for an ascii zero
	.set	ASC_9,	0x39		# code for a 9
	.set	ASC_EQ,	0x3d		# code for an =
	.set	ASC_QM,	0x3f		# code for an ?
	.set	ASC_AT,	0x40		# code for an at sign
	.set	ASC_A,	0x41		# code for an uppercase a
	.set	ASC_L,	0x4c		# code for an uppercase L
	.set	ASC_Z,	0x5a		# code for an uppercase z
	.set	ASC_a,	0x61		# code for an lowercase a
	.set	ASC_h,	0x68		# code for an lowercase h
	.set	ASC_n,	0x6e		# code for an lowercase n	
	.set	ASC_p,	0x70		# code for an lowercase p

 # magic context specifiers - c_byte is known to be exactly zero

	.set	 c_byte,0		# byte context was specified (or none)
	.set	 c_word,1		# word context was specified
	.set	 c_dble,2		# double words needed

 # I/O configuration

#if	MMAX_XPC
	# XPC - ddt output to duart line B
	.set	sio, XPCDUART_ADDR		# serial I/O channel address
 	.set	o_data, (XPCDUART_BRCVDATA - XPCDUART_ADDR)
						# offset of data register
 	.set	o_status, (XPCDUART_BSTATUS - XPCDUART_ADDR)
						# offset of status register
	.set	txrdy, DUART_TXRDY_BIT		# transmitter ready bit number
 	.set	rxrdy, DUART_RXRDY_BIT		# receiver ready bit number
#endif
#if	MMAX_APC
        .set    sio,APCDUART_ADDR       # serial I/O channel address
        .set    o_data,(APCDUART_BRCVDATA - APCDUART_ADDR)
                                        # offset of data register
        .set    o_status,(APCDUART_BSTATUS - APCDUART_ADDR)
                                        # offset of status register
        .set    txrdy,DUART_TXRDY_BIT   # transmitter ready bit number
        .set    rxrdy,DUART_RXRDY_BIT   # receiver ready bit number
#endif

#if	MMAX_XPC || MMAX_APC
#define MODE2_STOP_BITS		0x07
#define MODE2_NORMAL_INIT	(MODE2_STOP_BITS)
#define XMIT_9600_BAUD		0x0b
#define RECEIVE_9600_BAUD	0xb0
#endif

#if	MMAX_DPC
	.set	sio,DPCREG_SBXCTL0	# serial I/O channel address
	.set	o_data,(DPCREG_SBXDATA0 - DPCREG_SBXCTL0)
					# offset of data register
	.set	o_status,0		# offset of status register
	.set	txrdy,DPCSBXCTL_TXRDY_BIT # transmitter ready bit number
	.set	rxrdy,DPCSBXCTL_RXRDY_BIT # receiver ready bit number
#endif

 # This table defines the register names with their offsets on the
 # interrupt stack.  This way, a single subroutine is able to change
 # any register. ``Sp'' is an alias for ``us''.
 #
regt:	.ascii	"ps"
	.byte	e_psr32
	.ascii	"sb"
	.byte	e_sb
	.ascii	"is"
	.byte	e_is
	.ascii	"mo"
	.byte	e_mod32
	.ascii	"fp"
	.byte	e_fp
	.ascii	"us"
	.byte	e_us
	.ascii	"sp"
	.byte	e_us
	.ascii	"in"
	.byte	e_intbase
	.ascii	"r7"
	.byte	e_r7
	.ascii	"r6"
	.byte	e_r6
	.ascii	"r5"
	.byte	e_r5
	.ascii	"r4"
	.byte	e_r4
	.ascii	"r3"
	.byte	e_r3
	.ascii	"r2"
	.byte	e_r2
	.ascii	"r1"
	.byte	e_r1
	.ascii	"r0"
	.byte	e_r0
	.ascii	"pc"
	.byte	e_pc
regt1:
	.ascii	"fs"
	.byte	e_fsr32
	.ascii	"mb"
	.byte	e_frame+0
	.ascii	"mw"
	.byte	e_frame+1
	.ascii	"md"
	.byte	e_frame+2
regt2:
ejregt:	.int	(regt1-regt)		# number of bytes of register data
eregt:	.int	(regt2-regt)		# number of bytes of reg/mem data

vmem:	.ascii	"vb"
	.byte	e_frame+0
	.ascii	"vw"
	.byte	e_frame+1
	.ascii	"vd"
	.byte	e_frame+2
evmem:	.int	(.-vmem)

 # The entry message strings

#if	MMAX_XPC
trapt:	.asciz  "NVI NMI ABT FPU ILL SVC DVZ FLG BPT TRC UND RES BER PAR OVF DBG"
             #----====----====----====----====----====----====----====----====
             #-7  -6  -5  -4  -3  -2  -1   0   1   2   3   4   5   6   7   8
#endif
#if	MMAX_APC
trapt:  .asciz  "NVI NMI ABT FPU ILL SVC DVZ FLG BPT TRC UND RES BER PAR "
 #               ----====----====----====----====----====----====----====
#endif
#if	MMAX_DPC
trapt:	.asciz	"NVI NMI ABT FPU ILL SVC DVZ FLG BPT TRC UND RES "
 #		 ----====----====----====----====----====----====
#endif
 #
 # Multiprocessor debugger data

 # Debugger stacks, one per cpu.

	.bss	dbg_stack,NBPG*MAXCPUS,NBPW	# One page for each cpu

 # User interrupt bit in psr

	.bss	usr_ibit,MAXCPUS,1	# One for each cpu

 # System-wide flag indicating system is in debugger (used by NMI trap
 # handler to ignore NMIs caused by bad address entries).

	.bss	_in_dbmon,NBPW,NBPW

 # Take debugger flag.  Each byte cooresponds to a cpu and records whether the
 # cpu is trying to enter the debugger due to:
 #	o pressing the nmi button.  On HYDRA, the "white button" is replaced
 #	  by an SCC interface that sets allows a user to set a bit in a per-cpu
 #	  byte array.  On the user's requesting an NMI be sent to a processor,
 #	  the SCC marks the processor's byte and sends out a global NMI to the
 #	  system.
 #	o taking a breakpoint trap
 #	o taking an mmu breakpoint (local NMI)
 #
 # The lock regulates access to the array.  Only one cpu at a time should have
 # this flag set.

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
#define	XPC_NMI_SCC_DELAY	10000
#endif

 # Debug entry flags. Each byte corresponds to a cpu and records whether
 # the cpu is waiting to gain entry into the debugger.

	.bss	entry_pend,MAXCPUS,1	# One byte for each cpu

 # Stack built flag. Each byte records whether the cpu has the debug stack
 # built.

	.bss	stack_built,MAXCPUS,1	# One byte for each cpu

 # Step flag. Each byte records whether the cpu is about to single step.

	.bss	step_flag,MAXCPUS,1

 # Last trap. Each byte records the code byte of the last trap taken by the cpu.

	.bss	last_trap,MAXCPUS,1

 # Wait flag used to pause all cpus except the debugger owner.

	.bss	wait_flag,1,1		# Wait flag for all cpus but current

 # The boot processor identifier.

	.globl	_boot_cpuid
_boot_cpuid:
	.int	0

 # I/O service variables used for i/o requests to the remote debugger via the
 # boot processor.

	.bss	in_req,1,1		# Input request to boot cpu
	.bss	in_buf,1,1		# Input buffer to boot cpu

	.bss	out_req,1,1		# Output request to boot cpu
	.bss	out_buf,1,1		# Output buffer to boot cpu


 # Previous mmu status register contents used to determine whether an nmi
 # occurred due to an mmu break.

#if	MMAX_XPC
	.bss	old_mcr,MAXCPUS*NBPW,NBPW	# old mcr contents, one per cpu
#endif
#if	MMAX_APC
        .bss    old_few,MAXCPUS*NBPW,NBPW       # One per cpu
#endif
#if	MMAX_DPC
	.bss	old_msr,MAXCPUS*NBPW,NBPW	# One per cpu
#endif


 #
 # Interrupt Table. This table is a temporary interrupt vector table. The real
 # one is the OS module locore.s

	.text
	.align	4
_dbmodbegin:
_dbmodtab:
	.int	0
	.int	0
	.int	_dbmodbegin
	.int	0

 # Interrupt Table. This table is a temporary interrupt vector table. The real
 # one is the OS module locore.s.

#if	MMAX_XPC
	.text
inttable:
	VECTOR(dres)			/* Unused (0) */
	VECTOR(dres)			/* Unused (0) */
	VECTOR(dabt)			/* MMU abort (2) */
	VECTOR(dfpu)			/* FPU fault (3) */
	VECTOR(dill)			/* Illegal operation (4) */
	VECTOR(dsvc)			/* Supervisor call (5) */
	VECTOR(ddvz)			/* Divide by zero (6) */
	VECTOR(dflg)			/* Flag trap (7) */
	VECTOR(dbpt)			/* Breakpoint trap (8) */
	VECTOR(dtrc)			/* Trace trap (9) */
	VECTOR(dund)			/* Undefined instruction (10) */
	VECTOR(dber)			/* Restartable Bus Error (11) */
	VECTOR(dber)			/* Non-Restartable Bus Error (12) */
	VECTOR(dovf)			/* Integer Overflow (13) */
	VECTOR(ddbg)			/* Debug trap (14) */
	VECTOR(dres)			/* Unused (15) */
					##### Vectored Interrupts #####
	VECTOR(dbad)			/* Power Failure (16) */
	VECTOR(dnmi)			/* Non-maskable interrupt (17) */
	VECTOR(dbad)			/* Hard Nanobus error (18) */
	VECTOR(dbad)			/* DESTSEL Parity error (19) */
	VECTOR(dbad)			/* BTAG State error (20) */
	VECTOR(dbad)			/* BTAG-Cache Inconsistency (21) */
	VECTOR(dbad)		    	/* Rcv Address Parity error (22) */
	VECTOR(dbad)			/* Cache Parity error (23) */
	VECTOR(dbad)		    	/* BTAG Tag Parity error (24) */
	VECTOR(dbad)			/* Soft Nanobus (25) */
	VECTOR(dbad)			/* Vector Bus error (26) */
	VECTOR(dignore)			/* Low Timer (27) */
	VECTOR(dundefintr)		/* Unused (28) */
	VECTOR(dignore)			/* High timer - TSE (29) */
	VECTOR(dnvi)			/* Vector Bus (30) */
	VECTOR(dignore)			/* ICU race (ignore) (31) */
#endif

#if	MMAX_APC
	.text
db_no_icu:
        .byte   49              /* Value used for illegal cascaded ICU */
        .align  4
dbcascades:
        .int    db_no_icu               /* Unused (0) */
        .int    db_no_icu               /* Unused (1) */
        .int    db_no_icu               /* Unused (2) */
        .int    db_no_icu               /* Unused (3) */
        .int    db_no_icu               /* Unused (4) */
        .int    db_no_icu               /* Unused (5) */
        .int    db_no_icu               /* Unused (6) */
        .int    db_no_icu               /* Unused (7) */
        .int    S_ICU_BASE              /* Slave ICU Base Address (8) */
        .int    db_no_icu               /* Unused (9) */
        .int    db_no_icu               /* Unused (10) */
        .int    db_no_icu               /* Unused (11) */
        .int    db_no_icu               /* Unused (12) */
        .int    db_no_icu               /* Unused (13) */
        .int    db_no_icu               /* Unused (14) */
        .int    db_no_icu               /* Unused (15) */

inttable:
        VECTOR(dres)                    /* Unused (0) */
        VECTOR(dres)                    /* Unused (0) */
        VECTOR(dabt)                    /* MMU abort (2) */
        VECTOR(dfpu)                    /* FPU fault (3) */
        VECTOR(dill)                    /* Illegal operation (4) */
        VECTOR(dsvc)                    /* Supervisor call (5) */
        VECTOR(ddvz)                    /* Divide by zero (6) */
        VECTOR(dflg)                    /* Flag trap (7) */
        VECTOR(dbpt)                    /* Breakpoint trap (8) */
        VECTOR(dtrc)                    /* Trace trap (9) */
        VECTOR(dund)                    /* Undefined instruction (10) */
        VECTOR(dres)                    /* Unused (11) */
        VECTOR(dber)                    /* Bus Error (12) */
        VECTOR(dres)                    /* Unused (13) */
        VECTOR(dres)                    /* Unused (14) */
        VECTOR(dres)                    /* Unused (15) */
        VECTOR(dbad)                    /* Power Failure (16) */
        VECTOR(dnmi)                    /* Non-maskable interrupt (17) */
        VECTOR(dbad)                    /* Hard Nanobus 0 (18) */
        VECTOR(dbad)                    /* Hard Nanobus 1 (19) */
        VECTOR(dbad)                    /* Hard Nanobus 2 (20) */
        VECTOR(dbad)                    /* Hard Nanobus 3 (21) */
        VECTOR(dbad)                    /* Destsel Parity (22) */
        VECTOR(dundefintr)              /* Unused (23) */
        VECTOR(dundefintr)              /* Unused (24) */
        VECTOR(dbad)                    /* Soft Nanobus 0 (25) */
        VECTOR(dundefintr)              /* Unused (26) */
        VECTOR(dignore)                 /* Master Timer - TSE (27) */
        VECTOR(dnvi)                    /* Vector Bus (28) */
        VECTOR(dundefintr)              /* Unused (29) */
        VECTOR(dundefintr)              /* Unused (30) */
        VECTOR(dignore)                 /* Unused (31) */
        VECTOR(dbad)                    /* Soft Nanobus 1 (32) */
        VECTOR(dbad)                    /* Soft Nanobus 1 (33) */
        VECTOR(dbad)                    /* Rcv Address Parity 4 (34) */
        VECTOR(dbad)                    /* Rcv Address Parity 0 (35) */
        VECTOR(dbad)                    /* Rcv Address Parity 1 (36) */
        VECTOR(dbad)                    /* Rcv Address Parity 2 (37) */
        VECTOR(dbad)                    /* Rcv Address Parity 3 (38) */
        VECTOR(dbad)                    /* Btag Parity 0 (39) */
        VECTOR(dbad)                    /* Btag Parity 1 (40) */
        VECTOR(dbad)                    /* Vector Bus NAK (41) */
        VECTOR(dbad)                    /* Vector Bus Parity (42) */
        VECTOR(dbad)                    /* Vector Bus Out of Sync (43) */
        VECTOR(dbad)                    /* Cache Parity (44) */
        VECTOR(dbad)                    /* DUART (45) */
        VECTOR(dignore)                 /* Slave Timer (46) */
        VECTOR(dignore)                 /* Unused (47) */
        VECTOR(dundefintr)              /* Unused (48) */
        VECTOR(d_ill_casc)              /* Illegal Cascade (49) */
        VECTOR(dundefintr)              /* Unused (50) */
        VECTOR(dundefintr)              /* Unused (51) */
        VECTOR(dundefintr)              /* Unused (52) */
        VECTOR(dundefintr)              /* Unused (53) */
        VECTOR(dundefintr)              /* Unused (54) */
        VECTOR(dundefintr)              /* Unused (55) */
        VECTOR(dundefintr)              /* Unused (56) */
        VECTOR(dundefintr)              /* Unused (57) */
        VECTOR(dundefintr)              /* Unused (58) */
        VECTOR(dundefintr)              /* Unused (59) */
        VECTOR(dundefintr)              /* Unused (50) */
        VECTOR(dundefintr)              /* Unused (61) */
        VECTOR(dundefintr)              /* Unused (62) */
        VECTOR(dundefintr)              /* Unused (63) */
        VECTOR(dundefintr)              /* Unused (64) */
        VECTOR(dundefintr)              /* Unused (65) */
        VECTOR(dundefintr)              /* Unused (66) */
        VECTOR(dundefintr)              /* Unused (67) */
        VECTOR(dundefintr)              /* Unused (68) */
        VECTOR(dundefintr)              /* Unused (69) */
        VECTOR(dundefintr)              /* Unused (70) */
        VECTOR(dundefintr)              /* Unused (71) */
        VECTOR(dundefintr)              /* Unused (72) */
        VECTOR(dundefintr)              /* Unused (73) */
        VECTOR(dundefintr)              /* Unused (74) */
        VECTOR(dundefintr)              /* Unused (75) */
        VECTOR(dundefintr)              /* Unused (76) */
        VECTOR(dundefintr)              /* Unused (77) */
        VECTOR(dundefintr)              /* Unused (78) */
        VECTOR(dundefintr)              /* Unused (79) */
        VECTOR(dundefintr)              /* Unused (80) */
        VECTOR(dundefintr)              /* Unused (81) */
        VECTOR(dundefintr)              /* Unused (82) */
        VECTOR(dundefintr)              /* Unused (83) */
        VECTOR(dundefintr)              /* Unused (84) */
        VECTOR(dundefintr)              /* Unused (85) */
        VECTOR(dundefintr)              /* Unused (86) */
        VECTOR(dundefintr)              /* Unused (87) */
        VECTOR(dundefintr)              /* Unused (88) */
        VECTOR(dundefintr)              /* Unused (89) */
        VECTOR(dundefintr)              /* Unused (90) */
        VECTOR(dundefintr)              /* Unused (91) */
        VECTOR(dundefintr)              /* Unused (92) */
        VECTOR(dundefintr)              /* Unused (93) */
        VECTOR(dundefintr)              /* Unused (94) */
        VECTOR(dundefintr)              /* Unused (95) */
        VECTOR(dundefintr)              /* Unused (96) */
        VECTOR(dundefintr)              /* Unused (97) */
        VECTOR(dundefintr)              /* Unused (98) */
        VECTOR(dundefintr)              /* Unused (99) */
        VECTOR(dundefintr)              /* Unused (100) */
        VECTOR(dundefintr)              /* Unused (101) */
        VECTOR(dundefintr)              /* Unused (102) */
        VECTOR(dundefintr)              /* Unused (103) */
        VECTOR(dundefintr)              /* Unused (104) */
        VECTOR(dundefintr)              /* Unused (105) */
        VECTOR(dundefintr)              /* Unused (106) */
        VECTOR(dundefintr)              /* Unused (107) */
        VECTOR(dundefintr)              /* Unused (108) */
        VECTOR(dundefintr)              /* Unused (109) */
        VECTOR(dundefintr)              /* Unused (110) */
        VECTOR(dundefintr)              /* Unused (111) */
        VECTOR(dundefintr)              /* Unused (112) */
        VECTOR(dundefintr)              /* Unused (113) */
        VECTOR(dundefintr)              /* Unused (114) */
        VECTOR(dundefintr)              /* Unused (115) */
        VECTOR(dundefintr)              /* Unused (116) */
        VECTOR(dundefintr)              /* Unused (117) */
        VECTOR(dundefintr)              /* Unused (118) */
        VECTOR(dundefintr)              /* Unused (119) */
        VECTOR(dundefintr)              /* Unused (120) */
        VECTOR(dundefintr)              /* Unused (121) */
        VECTOR(dundefintr)              /* Unused (122) */
        VECTOR(dundefintr)              /* Unused (123) */
        VECTOR(dundefintr)              /* Unused (124) */
        VECTOR(dundefintr)              /* Unused (125) */
        VECTOR(dundefintr)              /* Unused (126) */
        VECTOR(dundefintr)              /* Unused (127) */
        VECTOR(dundefintr)              /* Unused (128) */
        VECTOR(dundefintr)              /* Unused (129) */
        VECTOR(dundefintr)              /* Unused (130) */
        VECTOR(dundefintr)              /* Unused (131) */
        VECTOR(dundefintr)              /* Unused (132) */
        VECTOR(dundefintr)              /* Unused (133) */
        VECTOR(dundefintr)              /* Unused (134) */
        VECTOR(dundefintr)              /* Unused (135) */
        VECTOR(dundefintr)              /* Unused (136) */
        VECTOR(dundefintr)              /* Unused (137) */
        VECTOR(dundefintr)              /* Unused (138) */
        VECTOR(dundefintr)              /* Unused (139) */
        VECTOR(dundefintr)              /* Unused (140) */
        VECTOR(dundefintr)              /* Unused (141) */
        VECTOR(dundefintr)              /* Unused (142) */
        VECTOR(dundefintr)              /* Unused (143) */
        VECTOR(dundefintr)              /* Unused (144) */
        VECTOR(dundefintr)              /* Unused (145) */
        VECTOR(dundefintr)              /* Unused (146) */
        VECTOR(dundefintr)              /* Unused (147) */
        VECTOR(dundefintr)              /* Unused (148) */
        VECTOR(dundefintr)              /* Unused (149) */
        VECTOR(dundefintr)              /* Unused (150) */
        VECTOR(dundefintr)              /* Unused (151) */
        VECTOR(dundefintr)              /* Unused (152) */
        VECTOR(dundefintr)              /* Unused (153) */
        VECTOR(dundefintr)              /* Unused (154) */
        VECTOR(dundefintr)              /* Unused (155) */
        VECTOR(dundefintr)              /* Unused (156) */
        VECTOR(dundefintr)              /* Unused (157) */
        VECTOR(dundefintr)              /* Unused (158) */
        VECTOR(dundefintr)              /* Unused (159) */
        VECTOR(dundefintr)              /* Unused (160) */
        VECTOR(dundefintr)              /* Unused (161) */
        VECTOR(dundefintr)              /* Unused (162) */
        VECTOR(dundefintr)              /* Unused (163) */
        VECTOR(dundefintr)              /* Unused (164) */
        VECTOR(dundefintr)              /* Unused (165) */
        VECTOR(dundefintr)              /* Unused (166) */
        VECTOR(dundefintr)              /* Unused (167) */
        VECTOR(dundefintr)              /* Unused (168) */
        VECTOR(dundefintr)              /* Unused (169) */
        VECTOR(dundefintr)              /* Unused (170) */
        VECTOR(dundefintr)              /* Unused (171) */
        VECTOR(dundefintr)              /* Unused (172) */
        VECTOR(dundefintr)              /* Unused (173) */
        VECTOR(dundefintr)              /* Unused (174) */
        VECTOR(dundefintr)              /* Unused (175) */
        VECTOR(dundefintr)              /* Unused (176) */
        VECTOR(dundefintr)              /* Unused (177) */
        VECTOR(dundefintr)              /* Unused (178) */
        VECTOR(dundefintr)              /* Unused (179) */
        VECTOR(dundefintr)              /* Unused (180) */
        VECTOR(dundefintr)              /* Unused (181) */
        VECTOR(dundefintr)              /* Unused (182) */
        VECTOR(dundefintr)              /* Unused (183) */
        VECTOR(dundefintr)              /* Unused (184) */
        VECTOR(dundefintr)              /* Unused (185) */
        VECTOR(dundefintr)              /* Unused (186) */
        VECTOR(dundefintr)              /* Unused (187) */
        VECTOR(dundefintr)              /* Unused (188) */
        VECTOR(dundefintr)              /* Unused (189) */
        VECTOR(dundefintr)              /* Unused (190) */
        VECTOR(dundefintr)              /* Unused (191) */
        VECTOR(dundefintr)              /* Unused (192) */
        VECTOR(dundefintr)              /* Unused (193) */
        VECTOR(dundefintr)              /* Unused (194) */
        VECTOR(dundefintr)              /* Unused (195) */
        VECTOR(dundefintr)              /* Unused (196) */
        VECTOR(dundefintr)              /* Unused (197) */
        VECTOR(dundefintr)              /* Unused (198) */
        VECTOR(dundefintr)              /* Unused (199) */
        VECTOR(dundefintr)              /* Unused (200) */
        VECTOR(dundefintr)              /* Unused (201) */
        VECTOR(dundefintr)              /* Unused (202) */
        VECTOR(dundefintr)              /* Unused (203) */
        VECTOR(dundefintr)              /* Unused (204) */
        VECTOR(dundefintr)              /* Unused (205) */
        VECTOR(dundefintr)              /* Unused (206) */
        VECTOR(dundefintr)              /* Unused (207) */
        VECTOR(dundefintr)              /* Unused (208) */
        VECTOR(dundefintr)              /* Unused (209) */
        VECTOR(dundefintr)              /* Unused (210) */
        VECTOR(dundefintr)              /* Unused (211) */
        VECTOR(dundefintr)              /* Unused (212) */
        VECTOR(dundefintr)              /* Unused (213) */
        VECTOR(dundefintr)              /* Unused (214) */
        VECTOR(dundefintr)              /* Unused (215) */
        VECTOR(dundefintr)              /* Unused (216) */
        VECTOR(dundefintr)              /* Unused (217) */
        VECTOR(dundefintr)              /* Unused (218) */
        VECTOR(dundefintr)              /* Unused (219) */
        VECTOR(dundefintr)              /* Unused (220) */
        VECTOR(dundefintr)              /* Unused (221) */
        VECTOR(dundefintr)              /* Unused (222) */
        VECTOR(dundefintr)              /* Unused (223) */
        VECTOR(dundefintr)              /* Unused (224) */
        VECTOR(dundefintr)              /* Unused (225) */
        VECTOR(dundefintr)              /* Unused (226) */
        VECTOR(dundefintr)              /* Unused (227) */
        VECTOR(dundefintr)              /* Unused (228) */
        VECTOR(dundefintr)              /* Unused (229) */
        VECTOR(dundefintr)              /* Unused (230) */
        VECTOR(dundefintr)              /* Unused (231) */
        VECTOR(dundefintr)              /* Unused (232) */
        VECTOR(dundefintr)              /* Unused (233) */
        VECTOR(dundefintr)              /* Unused (234) */
        VECTOR(dundefintr)              /* Unused (235) */
        VECTOR(dundefintr)              /* Unused (236) */
        VECTOR(dundefintr)              /* Unused (237) */
        VECTOR(dundefintr)              /* Unused (238) */
        VECTOR(dundefintr)              /* Unused (239) */
        VECTOR(dundefintr)              /* Unused (240) */
        VECTOR(dundefintr)              /* Unused (241) */
        VECTOR(dundefintr)              /* Unused (242) */
        VECTOR(dundefintr)              /* Unused (243) */
        VECTOR(dundefintr)              /* Unused (244) */
        VECTOR(dundefintr)              /* Unused (245) */
        VECTOR(dundefintr)              /* Unused (246) */
        VECTOR(dundefintr)              /* Unused (247) */
        VECTOR(dundefintr)              /* Unused (248) */
        VECTOR(dundefintr)              /* Unused (249) */
        VECTOR(dundefintr)              /* Unused (250) */
        VECTOR(dundefintr)              /* Unused (251) */
        VECTOR(dundefintr)              /* Unused (252) */
        VECTOR(dundefintr)              /* Unused (253) */
        VECTOR(dundefintr)              /* Unused (254) */
        VECTOR(dundefintr)              /* Unused (255) */
#endif

#if	MMAX_DPC
	.text
	.align	4
inttable:
	VECTOR(dnvi)
	VECTOR(dnmi)
	VECTOR(dabt)
	VECTOR(dfpu)
	VECTOR(dill)
	VECTOR(dsvc)
	VECTOR(ddvz)
	VECTOR(dflg)
	VECTOR(dbpt)
	VECTOR(dtrc)
	VECTOR(dund)
	VECTOR(dres)
	VECTOR(dres)
	VECTOR(dres)
	VECTOR(dres)
	VECTOR(dres)
#endif

 #
 #***********************************
 #	DEBUGGER STARTUP
 #***********************************

start:					#
#if	MMAX_XPC
	bicpsrw	$(PSR_I+PSR_S+PSR_U+PSR_V)
					# No ints, sys mode & stack, no int ovf
#endif
#if	MMAX_APC || MMAX_DPC
	bicpsrw	$(PSR_I+PSR_S+PSR_U)
					# No interupts, system mode and stack
#endif

	# Setup the debugger stack (stacks are per cpu). Note, that only the
	# boot processor executes this initialization code. All other
	# processors enter the OS in mp_init.s.
	GETCPUID(r0)			# Get this processor's identifier
	movzbd	r0, _boot_cpuid		# Save boot cpu id
	ashd	$PGSHIFT, r0		# Page number
	addr	dbg_stack+NBPG[r0:b], r0# Set stack base in register
	lprd	sp, r0			# Load stack pointer
	lprd	fp, r0			# And frame pointer

 # Setup a temporary interrupt table. The real OS initialization code in
 # locore.s will setup the permanent interrupt table.

 	addr	inttable, r2		# Address of interrupt table
	addr	@NUMSYSVEC, r0		# Number of interrupt entries
	addr	_dbmodtab,r1
int.init:				#
	movw	r1, -4(r2)[r0:d]	# Add mod to assembled in offset
	acbd	$-1, r0, int.init	# Do all interrupts
	lprd	intbase, r2		# Reload intbase

	# Initialize multiprocessor debugging flags.
	addr	@MAXCPUS, r0		# Get number of cpus
	movqd	$0, r1			# Init index
mp.init:				#
	movqb	$0, entry_pend[r1:b]	# Clear entry pending
	movqb	$0, stack_built[r1:b]	# Clear the stack built mask
	movqb	$0, step_flag[r1:b]	# Clear single-step flag
#if	MMAX_XPC
	movqd	$0, old_mcr[r1:d]	# Clear old mmu control register
#endif
#if	MMAX_APC
        movqd   $0, old_few[r1:d]       # Clear old mmu status register
#endif
#if	MMAX_DPC
	movqd	$0, old_msr[r1:d]	# Clear old mmu status register
#endif
	movqb	$0, usr_ibit[r1:b]	# Clear user i-bit
	addqd	$1, r1			# Next index value
	acbd	$-1, r0, mp.init	# For all cpus

	movqb	$0, wait_flag		# Clear the wait flag
	movqb	$0, in_req		# Clear input request
	movqb	$0, out_req		#  and output request

#if	MMAX_XPC || MMAX_APC
#if	MMAX_XPC
#define	BOARD_DUART_BMODE	XPCDUART_BMODE
#define	BOARD_DUART_BCLKSEL	XPCDUART_BCLKSEL
#define	BOARD_DUART_BCMD	XPCDUART_BCMD
#endif
#if	MMAX_APC
#define	BOARD_DUART_BMODE	APCDUART_BMODE
#define	BOARD_DUART_BCLKSEL	APCDUART_BCLKSEL
#define	BOARD_DUART_BCMD	APCDUART_BCMD
#endif

 # Set up DUART to use 9600 baud for debug line

 #
 # Initialize Channel B
 #
	movb	$MODE2_NORMAL_INIT, @BOARD_DUART_BMODE
 #
 # Set the remote debugger UART port to 9600 baud
 #
 	movb	$(XMIT_9600_BAUD | RECEIVE_9600_BAUD), @BOARD_DUART_BCLKSEL
 #
 #
 # Signetics recommends this sequence to turn on the UART
 #
	movb	$DUART_RESET_RX, @BOARD_DUART_BCMD
	movb	$DUART_RESET_TX, @BOARD_DUART_BCMD
	movb	$(DUART_RESET_ERR|DUART_TX_ENABLE|DUART_RX_ENABLE), @BOARD_DUART_BCMD
#endif
	
	# Transfer control to the real OS entry point.
	jump	@os_start

 #
 #*****************************
 #	DEBUGGER ENTRANCE
 #*****************************
 #
 # Initially, entry is via real hardware traps. After OS initialization, traps
 # enter the OS module lotrap_ns32k.s. Some traps are forwarded here (nmi, bpt,
 # trc). Others are handled by the OS. Entry via the following also occurs if
 # the debugger somehow traps itself before OS initialization.

	# ** NON-VECTORED INTERRUPT **
dnvi:					#
							PDCC(dnvi,N,V)
	movqd	$-7,tos			#
	br	dbgtrap			# To default processing

	# ** NON-MASKABLE INTERRUPT **
dnmi:					#
							PDCC(dnmi,N,M)
	movqd	$-6, tos		# Set trap type
	br	dbgtrap			# To default processing

	# ** MMU ABORT **
dabt:					#
							PDCC(dabt,A,B)
	movqd	$-5,tos			#
	br	dbgtrap			# To default processing

	# ** FPU FAULT **
dfpu:					#
							PDCC(dfpu,F,P)
	movqd	$-4,tos			#
	br	dbgtrap			# To default processing

	# ** ILLEGAL INSTRUCTION **
dill:					#
							PDCC(dill,I,L)
	movqd	$-3,tos			#
	br	dbgtrap			# To default processing

	# ** SUPERVISOR CALL **
dsvc:					#
							PDCC(dsvc,S,V)
	movqd	$-2,tos			#
	br	dbgtrap			# To default processing

	# ** DIVIDE BY ZERO **
ddvz:					#
							PDCC(ddvz,D,V)
	movqd	$-1,tos			#
	br	dbgtrap			# To default processing

	# ** FLAG INSTRUCTION **
dflg:					#
							PDCC(flg,F,L)
	movqd	$0,tos			#
	br	dbgtrap			# To default processing

	# ** BREAKPOINT INSTRUCTION **
dbpt:					#
							PDCC(dbpt,B,P)
	movd	$1,tos			#
	br	dbgtrap			# To default processing

	# ** TRACE TRAP (SINGLE-STEP) **
dtrc:					#
							PDCC(dtrc,T,R)
	movd	$2,tos			#
	br	dbgtrap			# To default processing

	# ** UNDEFINED OPERATION **
dund:					#
							PDCC(dund,U,N)
	movd	$3,tos			#
	br	dbgtrap			# To default processing

	# ** RESERVED OPERATION **
dres:					#
							PDCC(dres,R,E)
	movd	$4, tos			#
	br	dbgtrap			# To default processing

#if	MMAX_XPC || MMAX_APC
        # ** BUS ERROR **
dber:                                   #
							PDCC(dber,B,E)
        movd    $5, tos                 #
        br      dbgtrap                 # To default processing

        # ** UNEXPECTED ERROR **
dundefintr:                             # e.g. unused location, or illegal
d_ill_casc:                             #     cascade intr. or parity
dbad:                                   #
							PDCC(dbad,B,A)
        movd    $6, tos                 #
        br      dbgtrap                 # To default processing

#if	MMAX_XPC
dovf:					#
							PDCC(dovf,O,V)
	movd	$7, tos			#
	br	dbgtrap			# To default processing

ddbg:					#
							PDCC(ddbg,D,B)
	movd	$8, tos			#
	br	dbgtrap			# To default processing
#endif

        # ** IGNORE INTERRUPT **
dignore:                                # Bogus IRI5 and timers
							PDCC(dignore,I,G)
        RETI(dign.1)                    #
#endif

 # All traps come here.

dbgtrap:				#

	# Disable interrupts and non-maskable interrupts.  Wait is the default
	# action.
#if	MMAX_XPC || MMAX_APC
	DISINT				# Interrupts off at ICU...
	cbitb	$ICU_SYSNMI_BIT,@M_ICU_BASE+ISRV # So it will work no matter
/*      bispsrw $PSR_I		        # ...how we get here*/
#endif
#if	MMAX_DPC
	DISINT				# Interrupts off
#endif
								PDCC(dtp6,T,6)
	save	[r0, r1, r2]		# Save registers
								PDCC(dtp7,T,7)
	DISNMI(dtptrap.001, r0, r1)	# Disable nmi's
								PDCC(dtpA,T,A)
        bispsrw $PSR_I		        # ...how we get here
								PDCC(dtp8,T,8)
	movqd	$1, @_in_dbmon		# Mark system now in dbmon
								PDCC(dtp9,T,9)
	DPRINTF(dbgtrap.msg, 24(sp))	# Log debug message
	GETCPUID(r0)	
	DPRINTF(dbgtrapcpu.msg, r0)
	restore	[r1, r2]		# Restore registers (we still need r0)
	movqb	$-1, wait_flag		# Prepare everybody to wait

 # Set this cpu's suspend enable bit that allows suspends to be sent.
 # If we got this far, we must have a valid int base.

	GETCPUID(r0)			# Get cpu identifier

	# The stack (user's) currently is:
	#
	#	ssp ->	saved r0
	#		trap type
	#		trap pc
	#		trap psr/mod
	#
	# Build a debug stack frame on the debugger stack. The assumption here
	# is that the user is healthy enough that his stack functional. The
	# processor trap sequence requires a stack implying that we wouldn't
	# be here without a stack. If we've already built the stack (suspend
	# request from a cpu while we're in the debugger or white button) this
	# step is skipped.

	movb	4(sp), last_trap[r0:b]	# Save trap code
	cmpqb	$0, stack_built[r0:b]	# Is my stack built?
	bne	dbgtrap.3		# If so, go directly to common code

dbgtrap.1:				#
	ashd	$PGSHIFT, r0		# Page number
	addr	dbg_stack+NBPG[r0:b], r0# Set stack address
	sprd	sp, -4(r0)		# Save user stack pointer
	movmd	0(sp), -20(r0), $4	# Save trap context and r0 on debugger
					#  stack
	addr	-20(r0), r0		# Adjust 'stack pointer'
	lprd	sp, r0			# Switch to debugger stack
	save	[r1, r2, r3, r4, r5, r6, r7]
					# Save remaining general registers
	sprd	intbase, tos		# Save interrupt base
#if	MMAX_XPC
	sprd	usp, tos
#endif
#if	MMAX_APC || MMAX_DPC
	USRSP				# Switch to user mode stack
	sprd	sp, r1			# Get user stack pointer
	SYSSP				# Back to system stack pointer
	movd	r1, tos			# Save user stack pointer
#endif
	sprd	fp, tos			# Save frame pointer
	addr	-e_fp(sp), r1		# Compute top of the stack frame
	lprd	sp, r1			# Set new top of stack
	lprd	fp, r1			# And top of frame
	movzwd	e_mod0(fp), e_mod32(fp)	# Make a 32-bit copy of the mod
	movd	e_ssp(fp), e_is(fp)	# Make a copy of the user sp
	addd	$16, e_is(fp)		# Account for trap context
	movqd	$0,e_sb(fp)		# SB must be zero!
	movzwd	e_psr0(fp),e_psr32(fp)	# Make a 32-bit copy
	movd	e_psr32(fp),e_fsr32(fp)	# make fake copy for address translate
	GETCPUID(r0)			# Get cpu id
	movqb	$-1, stack_built[r0:b]	# Set stack built

	# If we didn't just do a single step, get the saved interrupt enable
	# bit from the debuggee psr and save it. This is done to allow single
	# steps to be performed with interrupts disabled and to restore the I
	# bit when the user proceeds.

	cmpqb	$-1, step_flag[r0:b]	# Single step in progress?
	beq	dbgtrap.2		# If ne yes
	extsd	e_psr32+1(fp), r1, $(PSR_I_BIT-8), $1
					# Get saved I-bit value
	movb	r1, usr_ibit[r0:b]	# Save interrupt bit
	br	dbgtrap.3		# To common

	# Single step restores the saved I bit of the debuggee.
dbgtrap.2:				#
	inssd	usr_ibit[r0:b], e_psr32+1(fp), $(PSR_I_BIT-8), $1
					# Restore saved I-bit

	# Common code for to load up sp and fp to top of debug frame.
dbgtrap.3:				#
	ashd	$PGSHIFT, r0		# Page number
	addr	dbg_stack+NBPG[r0:b], r0# Base of stack
	addr	-e_frame(r0), r0
					# Top of stack
	lprd	sp, r0			# Load stack pointer
	lprd	fp, r0			# And frame pointer

	# Stack frame is built and has following state:
	#
	#	Debugger stack				User stack
	#
	#	ssp---->echo flag
	#		fake psr for mmu stuff
	#		psr for changing
	#		static base
	#		user stack pointer------+
	#		mod for changing	|
	#		frame pointer		|
	#		user mode stack pointer	|
	#		intbase for changing	|	saved r0<-----------+
	#		r7			|	trap type           |
	#		r6			|	trap pc             |
	#		r5			|	trap psr/mod        |
	#		r4                      +------>(prior to trap)     |
	#		r3                                                  |
	#		r2                                                  |
	#		r1                                                  |
	#		r0                                                  |
	#		trap type                                           |
	#		trap pc                                             |
	#		trap psr/mod                                        |
	#		user stack pointer ---------------------------------+
	#
	# Check out the reason for the trap.  For non-maskable interrupts
	# there are 3 cases:
	#
	#	o Suspend request from another cpu. In this case we will fail
	#	  the following tests and will do the default: wait.
	#	o An mmu breakpoint. The old and new msr break enable bits will
	#	  be different.
	#	o The white button (virtual or physical depending on system).
	# For other trap types, check explicitly for bpt or step, based on the
	# stored trap-type.  Any trap not covered by the above (should be
	# none) also defaults to wait.

	GETCPUID(r0)			# Get cpu identifier
	movxbd	last_trap[r0:b], r0	# Get trap code

	# First check for a breakpoint trap.  If it is, try to get the
	# debugger. Note that more than 1 CPU may get here at the same time.

	cmpqb	$1, r0			# BPT?
	beq	dbg.bid			# eq = yes

	# If it's a trace trap, just go for the debugger.  No one else should
	# be running.

	cmpqb	$2, r0			# TRC?
	beq	dbg.get			# eq = yes

	# Perhaps NMI....

	cmpqb	$-6, r0			# NMI?
	beq	button.chk		# eq = yes

	# Anything else is questionable.  If we were doing a single-step,
	# assume we ended up in an undesired state and treat it like a TRC.
	# Otherwise, put ourselves on hold.

	GETCPUID(r0)			# Get cpu identifier
	cmpqb	$0, step_flag[r0:b]	# Doing single-step?
	beq	suspend.trap		# eq = no
	br	dbg.get

	# Next, check for the white button.  The array is lock-protected since
	# only one entry can be active at a time.  Note that on the DB16, when
	# the white button is actually pressed, this test will fail.  We need
	# to examine the suspend mask in this case to see if the a testbed was
	# suspended or buttoned.  On the Hydra, the nmi_flag for this cpu
	# is set by the SCC if someone requests a cpu-specific nmi.

button.chk:
	GETCPUID(r0)			# Get processor id
button.loop:
	sbitib	$0, @nmi_lock		# Lock already taken?
	bfs	button.loop		# If fs, already taken
	cmpqb	$0, @nmi_flags[r0:b]	# Button pressed or self-nmi?
	bne	dbg.get0		# If ne, yes
	movqb	$0, @nmi_lock		# Release lock
	br	suspend.trap		# Go into wait loop

	# On taking a breakpoint, we need to signal the rest of the system to
	# stop.  Get the nmi_lock and clear everyone's nmi_flag.  Then
	# set our nmi_flag and send an NMI to everyone.  We effectively
	# white-button ourselves and never return in-line.

dbg.bid:
	DPRINTF(dbgbid.msg, $0)		# Log debug message
	GETCPUID(r0)			# Get cpu identifier
	movqb	$-1, entry_pend[r0:b]	# Stake our claim
dbg.bid1:
	sbitib	$0, @nmi_lock		# Lock already taken?
	bfs	dbg.bid1		# bfs = access fails
	addr	@MAXCPUS, r1		# Set up loop

clear.buttons:
	movqb	$0, @nmi_flags-1[r1:b]	#
	acbd	$-1, r1, clear.buttons	# Loop back
	movqb	$-1, @nmi_flags[r0:b]	# Set us up as though the SCC did it
	movb	r0, @nmi_owner		# Set the owner field to us
	jsr	@_gen_sysnmi		# Set the system nmi bit
	DPRINTF(sysnmi.msg, r0)		#
#if	MMAX_XPC
	movd	$XPC_NMI_SCC_DELAY, r0	# Give the SCC a chance to act...
nmi_delay_loop:
	acbd	$-1, r0, nmi_delay_loop
#endif
	movqb	$0, @nmi_lock		# Release lock
	ENBNMI(nmi.wait01,r0,r1)	# Allow NMIs

nmi.wait:				#
	nop				# Hang out until NMI happens
	br	nmi.wait		# Dead end
	
	# A suspend cpu trap causes us to spin waiting to be release. If we're
	# the boot cpu we also perform i/o services for the cpu that owns the
	# debugger. Note, we can get ripped out of this code by the white
	# button of some cpu including ourselves.  This is the default action.

suspend.trap:				#
	DPRINTF(suspend.msg, $0)		#
	ENBNMI(suspend.001,r0,r1)	# Enable nmi's
suspend.1:				#
	GETCPUID(r1)			# Get cpu identifier
	cmpd	r1, _boot_cpuid		# Boot cpu?
	bne	suspend.2		# Just hang out if not boot cpu
	bsr	io_services		# Perform i/o services
suspend.2:				#
	tbitb	$0, wait_flag		# Wait still on?
	bfs	suspend.1		# If fs yes

	# When we get out of the wait, see if we want the debugger.  We may
	# in the case of simultaneous BPTs, in which case the first cpu
	# gets jerked into the suspend code by the second's nmi.

	DISNMI(suspend.003, r0, r1)	# Turn nmi off
	GETCPUID(r0)			# Get cpu identifier
	cmpqb	$0, entry_pend[r0:b]	# This cpu waiting to get in?
	bne	dbg.bid			# If ne, yes, go retry the whole thing
	ENBNMI(suspend.004,r0,r1)	# Enable nmis for proper normal.ret
					#  code path merge.
	br	normal.ret		# Otherwise, return normally

	# In the case of a white-button hit, clear the nmi_flag and release
	# the lock.

dbg.get0:
	movqb	$0, @nmi_flags[r0:b]	# Clear our NMI flag
	DPRINTF(dbgget0.msg, $0)		#
	movqb	$-1, @nmi_owner		# Mark as not owned
	movqb	$0, @nmi_lock		# Release lock

	# Load up the trap type and enter the debugger. Again we must enable
	# nmi's for the white button.
dbg.get:				#
	DPRINTF(dbgget.msg, $0)		#
	GETCPUID(r0)			# Get processor identifier
	movqb	$0, step_flag[r0:b]	# Clear step in progress
	movqb	$0, entry_pend[r0:b]	# Clear entry pending
	ENBNMI(dbg.get001,r0,r1)	# Enable nmi's
	DPRINTF(mstart.msg, $0)		#
	movd	e_traptype(fp), r0	# Get original trap type
	br	mstart			# To debugger command processing

 #
 #********************************************
 #	DEBUGGER EXIT
 #********************************************

#if	MMAX_DPC
	# Non-sequential instruction proceed. The trap bit is set in the mmu
	# msr, the saved interrupt enable bit is restored, and the processor
	# trace bit is cleared. Nonsequential proceed is treated like proceed
	# with respect to the debugger lock and suspended cpus.
nonseq.ret:				#
	DISNMI(nonseq.001,r0,r1)	# Disable nmi's
	GETCPUID(r0)			# Get cpu identifier
	smr	msr, tos		# Stack the msr
	sbitb	$(MSR_NT_BIT), 0(sp)	# Enable non-sequential breakpoint
	movd	tos, r4			# Save new msr
	cbitb	$(MSR_BE_BIT), r4		# Clear breakpoint bit
	cbitb	$(PSR_T_BIT), e_psr32(fp)
					# Clear the processor trace bit
	movb	usr_ibit[r0:b], r1	# Get saved user I-bit
	inssd	r1, e_psr32+1(fp), $(PSR_I_BIT-8), $1
					# Put in user's psr
	br	allret			# To common return
#endif

	# Normal proceed. The mmu msr is not modified, the saved interrupt
	# enable bit is restored, and the processor trace bit is cleared.
normal.ret:				#
	DISNMI(normal.001,r0,r1)	# Disable nmi's
	GETCPUID(r0)			# Get processor id
#if	MMAX_XPC
	smr	mcr, tos		# Set new mcr - bpt bit is in the ASR
	movd	tos, r4
#endif
#if	MMAX_APC
	smr	few, tos		# Set new few
	movd	tos, r4
#endif
#if	MMAX_DPC
	smr	msr, tos		# Stack the msr
	movd	tos, r4			# Set new msr
	cbitb	$(MSR_BE_BIT), r4	# Clear breakpoint bit
#endif
	cbitb	$(PSR_T_BIT), e_psr32(fp)
					# Clear the processor trace bit
	movb	usr_ibit[r0:b], r1	# Get saved user I-bit
	inssd	r1, e_psr32+1(fp), $(PSR_I_BIT-8), $1
					# Put in user psr
	br	allret			# To common return

	# Single step. The mmu msr is not modified, the processor interrupt
	# enabled bit is cleared (disable interrupts), and the processor trace
	# bit is set to force trace trap after a single instruction.
step.ret:				#
	DISNMI(step.001,r0,r1)		# Disable nmi's
	GETCPUID(r0)			# Get processor id
#if	MMAX_XPC
	smr	mcr, r4			# Set the mcr - bpt bit is in the ASR
#endif
#if	MMAX_APC
	smr	few, r4			# Set the few
					# Breakpoint bit is in the ASR
#endif
#if	MMAX_DPC
	smr	msr, tos		# Get current msr
   	movd	tos, r4
	cbitb	$(MSR_BE_BIT), r4	# Clear breakpoint bit
#endif
	cbitw	$(PSR_I_BIT), e_psr32(fp)	
					# Disable interrupts during single step
	sbitw	$(PSR_T_BIT), e_psr32(fp)
					# Enable trace trap
	movqb	$-1, step_flag[r0:b]	# Set the step flag
	br	allret			# To common return

	# Common return for all cpus. Restore processor registers from the
	# debugger stack.
allret:					#
	movw	e_psr32(fp), e_psr0(fp)	# Set possible new psr/mod
	movw	e_mod32(fp), e_mod0(fp)	#
	addqd	$-8, e_is(fp)		# Make room for return pc/psr/mod
	movd	e_is(fp), e_ssp(fp)	# Set up new if changed by user
	addr	e_fp(fp), r1		# Set address of stack
	lprd	sp, r1			# Load stack pointer for restore
	lprd	fp, tos			# Restore frame pointer
#if	MMAX_XPC
 	lprd	usp, tos
#endif
#if	MMAX_APC || MMAX_DPC
	movd	tos, r3			# Get user mode stack pointer
	USRSP				# Switch to user mode stack
	lprd	sp, r3			# Set user mode stack pointer
	SYSSP				# Back to system mode stack
#endif
	lprd	intbase, tos		# Load interrupt base

 # Final cleanup and return to user

	ENBNMI (allret.000, r0, r1)	# Enable nmi's
	movqd	$0, @_in_dbmon		# No longer in dbmon
	GETCPUID(r0)			# Get cpu id
#if	MMAX_XPC
	lmr	mcr, r4			# Load new mcr value
	movd	r4, old_mcr[r0:d]	# Save mmu control register
#endif
#if	MMAX_APC
        lmr     few, r4                 # Load new msr value
        movd    r4, old_few[r0:d]       # Save mmu status register
#endif
#if	MMAX_DPC
	lmr	msr, r4			# Load new msr value
	movd	r4, old_msr[r0:d]	# Save mmu status register
#endif
	movqb	$0, stack_built[r0:b]	# Stack no longer built
	cmpqb	$0, step_flag[r0:b]	# About to do a single-step?
	bne	allret.1		# If ne, yes


	movqb	$0, wait_flag		# Resume suspended cpus
allret.1:
#if	MMAX_XPC
	/*
	 * There is the possibility that dbmon was asked to modify a
	 * portion of our text space that is already cached on-chip.
	 * The XPC does not propagate I-stream invalidates up to the
	 * 532 so the cache could become inconsistent with main memory
	 * in this case.  Because we execute these dbmon subroutines
	 * relatively infrequently (a few times a second at best?) I
	 * am lazy and will invalidate the chip's I-stream cache every
	 * time we are ready to return to executing main-line code.
	 * N.B.  Modified dbmon instructions won't be guaranteed to
	 * be consistent until this point.  (But you weren't going to
	 * do that anyway, were you?)
	 */
	cinv	[a,i], r0
#endif
	restore	[r0, r1, r2, r3, r4, r5, r6, r7]
					# Restore general registers except r0
	movmd	4(sp), 0(12(sp)), $2	# Restore return pc/psr/mod to user
					#  stack
	lprd	sp, 12(sp)		# Restore user's stack pointer
	RETT(allret.rett)		# Return to user

 #
 #********************************************
 #	DEBUGGER PRIMITIVES
 #********************************************


 # io_services
 #
 #	This function performed i/o services on the boot processor for non-boot
 #	cpus.

io_services:				#
	save	[r0]			# Save register
	tbitb	$0, out_req		# Output request present?
	bfc	io_serv.1		# If fc no
	movzbd	out_buf, tos		# Stack the byte to write
	bsr	writebyte		# Write a byte
	cbitb	$0, out_req		# Signal done
io_serv.1:				#
	tbitb	$rxrdy,@sio+o_status	# Is receiver ready?
	bfc	io_serv.2		# If fc no, don't wait for input yet
	tbitb	$0, in_req		# Is there an input request?
	bfc	io_serv.2		# If fc no
	bsr	ibyte			# Receive a byte
	movb	r0, in_buf		# Store byte
	cbitb	$0, in_req		# Signal done
io_serv.2:				#
	restore	[r0]			# Restore scratch register
	ret	$0			#

 #

 # peek - look at the byte that will be read next
 #
 # returns:
 #	r0
 #
peek:	movzbd	0(r7),r0		# what's next?
ret0:	ret	$0			#



 # readbyte - get the next byte from the line buffer. Won't read past <nul>.
 #
 # returns:
 #	r0:	zero-extended lower case input byte
 #
readbyte:
	movzbd	0(r7),r0			# just grab the byte
	cmpqb	$0,r0				# ...and if it's not
	beq	read1				# ...nil, then
	addqd	$1,r7				# ...increment the pointer
read1:	ret	$0



 # readline - read a line and convert to lower case
 #
 # the echo options are implemented here
 #
 # uses:	r0
 #
bytefun:	.byte	ASC_Z,ASC_A		# checkb instruction operand
erase1:		.byte	ASC_BS,ASC_SP,ASC_BS,0	# character erase string
input:		.byte	ASC_CR,ASC_BS,ASC_ESC,ASC_LF,ASC_AT
						# magic character list
inlen:		.byte	.-input
chardisp:	.byte	c_none-c_go
		.byte	c_at-c_go,rb1-c_go,c_esc-c_go,c_bs-c_go,c_cr-c_go
 #
readline:
	save	[r1,r4]
	movxbd	$-ibufsize,r7			# initialize loop counter
rb1:	bsr	ibyte				# get an 8-bit byte
	extsd	r0,r4,$0,$7			# put a 7-bit ascii # in r4
	movzbd	inlen,r0			# get length of magic list
	addr	input,r1			# get addr of magic list
	skpsb	[u]				# search magic char list
c_go:	caseb	chardisp[r0:b]			# branch on chars left in list
c_none:	cmpb	r4,$ASC_SP			# not magic, check for (other)
	blt	rb1				# ...control chars (2B ignored)
rb2:	movb	r4,ibufsize+ibuffer[r7:b]	# install it in the line buffer
rb3:	acbd	$1,r7,rb1			# at end of buffer?
	movqd	$-1,r7				# don't overwrite the stack
	movb	$ASC_CR,r4				# set up for cr echo
c_cr:	movqb	$0,ibufsize+ibuffer[r7:b]	# add a nul for cr or ovrfl
rl2:	addr	ibuffer,r7			# done, set input pointer
	restore	[r1,r4]
	ret	$0
c_esc:	br	restart				# an escape aborts the command
c_at:	movqw	$0,e_echo(fp)			# disable echo for @lines
	br	rb1
c_bs:	cmpb	$-ibufsize,r7			# check for empty buffer
	bge	rb1				# if empty, branch back to top
rb2a0:	addqd	$-1,r7				# correct the buffer pointer
	br	rb1				# ...and try for another

 # wstrnul - write a string up to a null byte
 #
 # uses:	r0,r1
 #
wstrnul:adjspb 	$4			# write string until nul
	movmd	4(sp),tos,$2		# ...make room for the len param
	movqd	$-1,8(sp)		# ...add a large length and fall thru



 # wstrlen - write a string up to a count or a null byte
 #
 # uses: r0,r1
 #
wstrlen:movd	8(sp),r0		# r0 = count
	movd	4(sp),r1		# r1 = addr
wstr1:	cmpqb	$0,0(r1)		# don't write past <nul>
	beq	wstr2			# just return if one is found
	movzbd	0(r1),tos		# get a byte
	addqd	$1,r1			# select the next one
	bsr	writebyte		# what fun, write it out
	acbd	$-1,r0,wstr1		# apply the count test
wstr2:	ret	$8



 # writebyte - write a byte
 #
 # If we're the boot processor, we can talk to ddt directly.  Otherwise,
 # we need the services of the boot CPU.

writebyte:
	save	[r0, r1]		# Save some registers for our use
	GETCPUID(r0)			# Get cpu identifier
	cmpd	_boot_cpuid, r0		# Compare to boot cpu's id
	bne	writebyte_nb		# If ne, we're not the boot
wbretry:
	tbitb	$rxrdy,@sio+o_status	# first check the receiver
	bfc	wb1			# receiver is clear, continue
	movb	@sio+o_data, r0		# not clear, grab the byte
	cmpb	$ASC_XOFF, r0		# control/s?
	bne	wb0			# no
	bsr	skipdc1			# yes, wait for a control/q
wb0:
	cmpb	$ASC_ESC,r0		# receiver ready!? check for <esc>
	beq	restart			# if <esc>, abort this command
wb1:
	tbitb	$txrdy,@sio+o_status	# otherwise, try the transmitter again
	bfc	wbretry			# If not ready, try again.
	movb	12(sp),@sio+o_data	# Write the byte
	restore	[r0, r1]		# Restore registers
	ret	$4			# ...and get on with it

	# If we're not the boot processor, we need the i/o services of the
	# boot cpu
writebyte_nb:
	tbitb	$0, out_req		# Wait for synchronization
	bfs	writebyte_nb		# If fs, still outputting previous
	movb	12(sp), out_buf		# Store byte to be output
	sbitb	$0, out_req		# Set output request
	restore	[r0, r1]		# Restore register
	ret	$4			# Return to caller




 # various randomness
 #
 # case dispatch for commands
 #
casefun:.word	quest-case		# r0=0, skpsbu `until' condition not met
	.word	exclaim-case		# '!' - comment
	.word	exclaim-case		# <0> - comment (from <cr>)
	.word	go-case			# 'g' - go
	.word	step-case		# 's' - single step
	.word	image-case		# 'i' - image (binary) load
	.word	xconfig-case		# 'x' - execute setcfg instruction
	.word	wmmu-case		# 'w' - write mmu register
	.word	rmmu-case		# 'r' - read mmu register
	.word	re1-case		# 'c' - change (memory or registers)
	.word	re1-case		# 'p' - print (memory or registers)
	.word	load-case		# 'l' - (hex ascii) load
	.word	dump-case		# 'd' - dump memory
	.word	fill-case		# 'f' - fill memory
	.word	all-case		# 'a' - print all registers
	.word	move-case		# 'm' - move
	.word	verify-case		# 'v' - verify load
	.word	nonseq-case		# 'q' - set nonseq trap mmu
 #
comtab:		.asciz	"qvmafdlpcrwxisg!"   # the command character table
comsize:	.int	.-comtab	# its size
 #
atsign:		.asciz	"E "		# entry message
questfun:	.byte	ASC_QM		# question mark complaint
crlf:		.byte	ASC_LF,ASC_CR,0		# ...also used for cr,lf
crlfs:		.byte	ASC_LF,ASC_CR,ASC_AST,ASC_LF,ASC_CR,0
					# prompt message
wcrlfs:	addr	crlfs,tos		# prompt subroutine
	bsr	wstrnul			#
	ret	$0
exclaim:br	restart			# null command for '!' and <cr>


 #
 #**********************************
 #	PROCESS COMMANDS
 #**********************************
 #
 # r7: input buffer byte pointer (ends at cr, changed to <nul>)
 #
 # on entry:
 #
 # r0: trap type index -7..+3
 #
 # here to process a command after an exception event
 #
mstart:
	movqd	$4,tos			# length of trap type message
	addr	trapt+7*4[r0:d],tos	# address of trap type message
	addr	atsign,tos		# do the E first (or lose r0)
	bsr	wstrnul			# write them both
	bsr	wstrlen

 # here to process a command after a previous command (or fall thru from above)
 #
 # when restarting, the stack level is reset from fp
 #
restart:
	addr	ibuffer,tos		# set stack level and allocate the...
	lprd	sp,tos			# ...input buffer
	bsr	wcrlfs			# write the prompt
	movqw	$0, e_echo(fp)		# Echo flags are no-ops now
	bsr	readline		# read a line
	bsr	readbyte		# obtain a command character
	checkb	r1,bytefun,r0		# is it in 'A'..'Z'??
	bfs	testn			# no, add back the 'A'
	addb	$ASC_a-ASC_A,r0		# yes, correct case
testn:  movd	r0,tos			# save the command character
	cmpb	r0,$ASC_n		# see if it the load gpib command
	beq	namecase		# name command don't fold
	cmpb	r0,$ASC_h			# see if it the home dir command
	beq	namecase		# name command don't fold
	movd	r7,r5			# get the readline string
nloop:	movzbd	0(r5),r3		# just grab the byte
	cmpqb	$0,r3			# ...and if it's not
	beq	namecase		# stop end of string
	checkb	r1,bytefun,r3		# fold it
	bfs	testn2			# no, add back the 'A'
	addb	$ASC_a-ASC_A,r3		# yes, correct case
testn2:	movb	r3,0(r5)		# install it in the line buffer
	addqd	$1,r5
	br	nloop
namecase:movd	r0,r4			# install the until condition
	movzbd	comsize,r0		# get the command table size
	addr	comtab,r1		# get the command table address
	skpsb	[u]			# search: r0 will be the command index
case:	casew	casefun[r0:w]		# branch to it (r0=0 if com. not found)

 #
 # re1 is the entry point for print and change register/memory commands
 #
re1:	bsr	readbyte		# read a register name or a memory
	movw	r0,tos			# ...context: rr or mb, mw, md
	bsr	readbyte		# these are two chars long
	movb	r0,1(sp)		# have the two-char string at tos now
	movzbd	eregt,r1		# get the register table ending address
print1:	cmpw	regt-3[r1:b],0(sp)	# search the reg table for an offset
	beq	print2			# the table will give an fp offset
	acbd	$-3,r1,print1		# try the next
	movzbd	evmem,r1		# get the virtual table ending address
vprt1:	cmpw	vmem-3[r1:b],0(sp)	# search the table for an offset
	beq	vprt2			# the table will give an fp offset
	acbd	$-3,r1,vprt1		# try the next
	br	quest			# **no name found**
vprt2:	cmpqw	$0,tos			# pop the code off the stack
	movzbd	vmem-1[r1:b],r0		# get the fp offset out of the table
	cmpb	r0,$e_frame		# if >= e_frame, then it's v[bwd]
	bge	vmemfun			# ...so go print/change virtual memory
print2:	cmpqw	$0,tos			# pop the code off the stack
	movzbd	regt-1[r1:b],r0		# get the fp offset out of the table
	cmpb	r0,$e_frame		# if >= e_frame, then it's m[bwd]
	bge	memfun			# ...so go print/change memory
	cmpb	$ASC_p,0(sp)		# print or change?
	bne	changer			# go change a register
	movd	0(fp)[r0:b],tos		# go print a register
	bsr	hexout
	br	next
changer:addr	0(fp)[r0:b],tos		# this is where the reg is saved
	bsr	eqhexin			# convert an =hex value
	movd	r0,0(0(sp))		# overwrite the saved register
	br	next			# restart will reset the stack level

memfun:	adjspb	$16			# allocate tmp/src/dst/context
	subb	$e_frame,r0		# extract the context
	movxbd	r0,0(sp)		# specify context 0/1/2
	bsr	hexin			# get the address to print or change
	movd	r0,8(sp)		# assume it's a src address
	movqd	$0,12(sp)		# clear tmp
	cmpb	$ASC_p,16(sp)		# printing?
	beq	mem2			# yes, do this a bit differently
	movd	8(sp),4(sp)		# (src assumption wrong) save dst
	addr	12(sp),8(sp)		# get the real source address
	bsr	eqhexin			# convert the new value
	movd	r0,12(sp)		# save it
	bsr	conmove			# move with context of 0(sp)
	cmpqd	$0,tos			# clear source copy of tos
	br	next
mem2:	addr	12(sp),4(sp)		# specify dst as addr(tmp)
	bsr	conmove			# fetch it with the right context
	bsr	hexout			# output!
	br	next

vmemfun:adjspb	$16			# allocate tmp/src/dst/context
	subb	$e_frame,r0		# extract the context
	movxbd	r0,0(sp)		# specify context 0/1/2
	bsr	hexin			# get the address to print or change
	movd	r0,8(sp)		# assume it's a src address
	movqd	$0,12(sp)		# clear tmp
	cmpb	$ASC_p,16(sp)		# printing?
	beq	vmem2			# yes, do this a bit differently
	movd	8(sp),4(sp)		# (src assumption wrong) save dst
	addr	12(sp),8(sp)		# get the real source address
	bsr	eqhexin			# convert the new value
	movd	r0,12(sp)		# save it
	bsr	wvonmove		# move with context of 0(sp)
	cmpqd	$0,tos			# clear source copy of tos
	br	next
vmem2:	addr	12(sp),4(sp)		# specify dst as addr(tmp)
	bsr	rvonmove		# fetch it with the right context
	bsr	hexout			# output!
	br	next


 #
 # conmove(context,dst,src) - move src to dst as per context
 #
 # uses: r0
 #
conmove:movd	4(sp),r0		# get context
mgo:	caseb	movec[r0:b]		# select the appropriate,
movec:	.byte	bytem-mgo
	.byte	halfm-mgo	
	.byte	fullm-mgo
bytem:	movb	0(12(sp)),0(8(sp))
	ret	$12
halfm:	movw	0(12(sp)),0(8(sp))	
	ret	$12
fullm:	movd	0(12(sp)),0(8(sp))
	ret	$12




 # rvonmove(context,dst,vsrc) - move virtual src to dst as per context
 #
 #
rvonmove:
#if	MMAX_XPC
	smr	mcr, tos
	movd	tos, r0
#endif
#if	MMAX_APC
	smr	few, tos
	movd	tos, r0
#endif
#if	MMAX_DPC
	smr	msr,tos		# These two instructions are a hack
	movd	tos,r0		# around a bug when referencing mmu
				# registers (the reference must be to memory)
#endif
	tbitb	$(PSR_U_BIT),e_fsr32(fp)
	bfc	rsupruser
#if	MMAX_XPC
	tbitb	$MCR_TU_BIT,r0
#endif
#if	MMAX_APC
	tbitb	$FEW_TU_BIT,r0
#endif
#if	MMAX_DPC
	tbitb	$(MSR_TU_BIT),r0
#endif MMAX_APC
	bfc	conmove
	br	rdu
rsupruser:
#if	MMAX_XPC
	tbitb	$MCR_TS_BIT,r0
#endif
#if	MMAX_APC
        tbitb   $FEW_TS_BIT,r0
#endif
#if	MMAX_DPC
	tbitb	$(MSR_TS_BIT),r0
#endif
	bfc	conmove
	br	rmptb0
rdu:
#if	MMAX_XPC
	tbitb	$MCR_DS_BIT,r0
#endif
#if	MMAX_APC
        tbitb   $FEW_DS_BIT,r0
#endif
#if	MMAX_DPC
	tbitb	$(MSR_DS_BIT),r0
#endif
	bfc	rmptb0
	smr	ptb1,tos
	br	rmptb
rmptb0:	smr	ptb0,tos
rmptb:	movd	tos,r5
 #
	movd	4(sp),r0
rgo:	caseb	mover[r0:b]
mover:	.byte	byter-rgo
	.byte	halfr-rgo
	.byte	fullr-rgo
byter:	
	movqd	$1,r3
bback:	movd	12(sp),r0
	bsr	mapbyte		#return mapped to supervisor virtual address
	cmpqd	$0,r0		#check if successful
	bgt	rbadad	
	movb	0(r0),0(8(sp))
	addqd	$1,8(sp)
	addqd	$1,12(sp)
	acbd	$-1,r3,bback
	ret	$12
halfr:	
	movqd	$2,r3
	br	bback
fullr:
	movqd	$4,r3
	br	bback
rbadad:	movqd	$-1,16(sp)
	ret	$12


 # wvonmove(context,vdst,src) - move src to virtual dst as per context
 #
wvonmove:
#if	MMAX_XPC
	smr	mcr, tos
#endif
#if	MMAX_APC
	smr	few,tos
#endif
#if	MMAX_DPC
	smr	msr,tos
#endif
	movd	tos,r0
	tbitb	$(PSR_U_BIT),e_fsr32(fp)
	bfc	wsupruser
#if	MMAX_XPC
	tbitb	$MCR_TU_BIT,r0
#endif
#if	MMAX_APC
	tbitb	$FEW_TU_BIT,r0
#endif
#if	MMAX_DPC
	tbitb	$(MSR_TU_BIT),r0
#endif
	bfc	conmove
	br	wdu
wsupruser:
#if	MMAX_XPC
	tbitb	$MCR_TS_BIT,r0
#endif
#if	MMAX_APC
	tbitb	$FEW_TS_BIT,r0
#endif
#if	MMAX_DPC
	tbitb	$(MSR_TS_BIT),r0
#endif
	bfc	conmove
	br	wmptb0
wdu:
#if	MMAX_XPC
	tbitb	$MCR_DS_BIT,r0
#endif
#if	MMAX_APC
	tbitb	$FEW_DS_BIT,r0
#endif
#if	MMAX_DPC
	tbitb	$(MSR_DS_BIT),r0
#endif
	bfc	wmptb0
	smr	ptb1,tos
	br	wmptb
wmptb0:	smr	ptb0,tos
wmptb:	movd	tos,r5

	movd	4(sp),r0
wgo:	caseb	movew[r0:b]
movew:	.byte	bytew-wgo
	.byte	halfw-wgo
	.byte	fullw-wgo
bytew:	
	movqd	$1,r3
bbackw:	movd	8(sp),r0
	bsr	mapbyte		#return mapped to supervisor virtual address
	cmpqd	$0,r0		#check if successful
	bgt	wbadad	
	movb	0(12(sp)),0(r0)
	addqd	$1,8(sp)
	addqd	$1,12(sp)
	acbd	$-1,r3,bbackw
wbadad:	ret	$12
halfw:	
	movqd	$2,r3
	br	bbackw
fullw:
	movqd	$4,r3
	br	bbackw

 # Map a byte into system space including verifying that source address is
 # mapped.  This code from mp_debug.s, modified to use _debug_vaddr and
 # _debug_pte set aside by pmap_bootstrap code.
 #
 # Inputs:
 #	r5	- Level 1 page table base register
 #	r0	- Virtual address to map
 #
 # Outputs:
 #	r0	- System virtual address to access byte or -1 if invalid
 #
mapbyte:				#
	enter	[r1, r2, r3, r4, r5, r6, r7], $0
					# Save registers
	movd	_debug_vaddr, r2	# System address to map at
	movd	_debug_pte, r1		# vaddr of pte that maps *_debug_vaddr

	movd	r0, r3			# Get desired virtual
	andd	$(VA_L1MASK), r3	# Mask level 1 index
	ashd	$-(VA_L1SHIFT), r3	# Shift into place
	tbitd	$(PG_V_BIT), 0(r5)[r3:d]# Level 1 entry valid?
	bfc	mapbyte.err		# If fc no

	movd	0(r5)[r3:d], 0(r1)	# Copy Level 1 entry to level 2
#if	MMAX_XPC || MMAX_APC
	lmr     ivar0, r2		# Invalidate system address
#endif
#if	MMAX_DPC
	lmr	eia, r2			# Invalidate system address
#endif
	movd	r0, r3			# Get desired virtual
	andd	$(VA_L2MASK), r3	# Mask level 2 index
	ashd	$-(VA_L2SHIFT), r3	# Shift into place
	tbitd	$(PG_V_BIT), 0(r2)[r3:d]# Is level 2 entry valid?
	bfc	mapbyte.err		# If fc no

	movd	0(r2)[r3:d], 0(r1)	# Copy Level 2 entry to level 2
	ord	$(PG_KW), 0(r1)		# Full kernel access

	andd	$(VA_OFFMASK), r0	# Mask offset of target address
	addd	r2, r0			# Compute system address
#if	MMAX_XPC || MMAX_APC
	lmr     ivar0, r0		# Flush translation buffer
#endif
#if	MMAX_DPC
	lmr	eia, r0			# Flush translation buffer
#endif
	br	mapbyte.ret		# Join common

mapbyte.err:				#
	movqd	$-1, r0			# Set error return

mapbyte.ret:				#
	exit	[r1, r2, r3, r4, r5, r6, r7]
					# Restore registers
	ret	$0			# To caller r0 -> address

 # next - skip to eol and go again
 #
next:	br	restart



 # eqhexin - convert =x...
 # hexin   - convert x...
 #
 # conversion is halted by (a second) '=', or by a byte < '0'
 #
 # uses:	r1
 # returns:
 #	r0
 #
eqhexin:bsr	skipblanks		# leading blanks allowed before numbers
	bsr	readbyte		# well, try a byte
	cmpb	$ASC_EQ,r0			# this entry wants an `='
	beq	hexin			# got it
	cmpqb	$0,r0			# might be missing
	beq	quest			# so complain
	br	eqhexin			#
hexin:	bsr	skipblanks		# do this `again', may have entered here
	movqd	$0,r1			# entry point for simple conversion
hi1:	bsr	peek			# what's the next byte(?) are we done?
	cmpb	$ASC_EQ,r0			# terminate on `='
	beq	hi9			#
	cmpb	r0,$ASC_0			# terminate on c < '0'
	blt	hi9
	bsr	readbyte		# otherwise, read the byte (again)
	bsr	hex_ins			# convert it and insert it in r1
	br	hi1			# continue
hi9:	movd	r1,r0
	ret	$0



 # hex_ins: convert the hex ascii digit in r0 and shift it into r1
 #
lesshex:	.byte	ASC_9,ASC_0	# the operand for the checkb instruction
 #
hex_ins:checkb	r0,lesshex,r0		# check for digits, subtract ord('0')
	bfc	hs2			# f clear, was a digit, is now binary
	subb	$ASC_a-ASC_0-0xa,r0	# was probably alpha, adjust for binary
hs2:	lshd	$4,r1			# make 4 bits of room
	inssb	r0,r1,$0,$4		# insert the new digit
	ret	$0



 # hexout - output ``=xxxxxxxx'' with no newline.
 # hextwo - almost the same, but with no leading `='
 # hexbyteout - convert only one byte, no leading `='
 #
 # uses:	r0,r1,r2
 #
hexbytes:
	.byte	0xf,0xa			# operand for checkb instruction
 #
hexout:	movzbd	$ASC_EQ,tos			# write an `='
	bsr	writebyte		#
hextwo:	movd	4(sp),r0		# get the parameter
	movxbd	$-28,r1			# r1 is the shift count and loop var
hex1:	bsr	hex2			# do one hex digit
	acbb	$4,r1,hex1		# repeat ... until (r1+=4)==0
	bsr	hex2			# do the least significant digit
	ret	$4
hexbyteout:				# do only a single byte
	movd	4(sp),r0		# set it up
	movxbd	$-4,r1			# need only one loop trip
	br	hex1			# head for the main loop
 #
hex2:	movd	r0,r2			# output one hex digit
	lshd	r1,r2			# get digit in r2
	andb	$0xf,r2			# interested in the low 4 bits
 #	checkb	r2,hexbytes,r2		# check for a-f
 #	bfs	hex3			# no, must be numeric
 #	addb	$ASC_A-0xa-ASC_0,r2	# yes, add difference between '0' & 'a'
 #
 # Simulate the desired function from the above three instructions.
 # The instruction is used here in violation of its definition.
 # It was done this way because it was known to work on the 032 and 332.
 #
	cmpb	r2, $0xf
	bgt	outtarange
	cmpb	r2, $0xa
	blt	outtarange
	br	inrange
outtarange:
	subb	$0xa, r2
	br	hex3
inrange:
	subb	$0xa, r2
	addb	$7,r2			# yes, add diff between '0' & 'a'

hex3:	addb	$0xa+ASC_0,r2		# make it ascii
	movzbd	r2,tos			# what fun, write it out
	bsr	writebyte
	ret	$0



 # skip to dc1 (control/q)
 #
skipdc1:
	movd	r0,tos			# save r0
sd2:	bsr	ibyte			# get the next transmitted byte
	cmpb	$ASC_ESC,r0		# we are interested in dc1 and esc
	beq	restart			# could halt it all right here, U know
	cmpb	$ASC_XON,r0		# dc1/control q: resume transmission
	bne	sd2			# not dc1, ignore it
	movd	tos,r0			# restore r0
	ret	$0



 # ibyte - read image (binary) byte
 #
ibyte:	

	# If we're the boot cpu, we can do input directly
	GETCPUID(tos)			# Get cpu identifier
	cmpd	_boot_cpuid, tos	# Compare to boot cpu's id
	bne	xibyte			# If ne, we're not the boot

	tbitb	$rxrdy,@sio+o_status	# busy wait for a character
	bfc	ibyte			# ibyte is called by readline
	movb	@sio+o_data,r0		# ...and the binary loader
	ret	$0

	# If we're not the boot cpu, we need the i/o services of the boot
	# boot cpu
xibyte:					#
	sbitb	$0, in_req		# Set input request
xibyte.1:				#
	tbitb	$0, in_req		# Input done?
	bfs	xibyte.1		# If fs, no
	movb	in_buf, r0		# Return character
	ret	$0			# Return to caller




 # write a question mark, skip to eol, branch back to the command loop
 #
quest:	addr	questfun,tos
	bsr	wstrnul
	br	next



 # process a load command:
 #
 # l addr 11223344cc	nn=hexa byte cc=checksum# first blank is optional
 #
 # r3: 	next address to load
 # r4:	accumulator for checksum byte
 #
load:	bsr	hexin			# get the address, with no '='
	movd	r0,r3			#
	movqd	$0,r4			#
	bsr	readbyte		# need a space after the address
	cmpb	$ASC_SP,r0		# insist
	bne	quest			# ...on it
load1:	bsr	hexbyte			# read and convert a hex byte
	movd	r1,r2			#
	bsr	skipblanks		# skip blanks and peek at the next char
	cmpb	r0,$ASC_0		# if not hex ascii, then we just read
	blt	load2			# ...the checksum, and we're done
	movb	r2,0(r3)		# otherwise, it's data, so store it
	addb	0(r3),r4		# get it back from mem for the checksum
	addqd	$1,r3			# select the next address
	br	load1			# continue
load2:	cmpb	r2,r4			# done, compare checksum bytes
	beq	next			# if equal, then exit
	movzbd	r4,tos			# otherwise, need to write a message
	bsr	crcbotch		# ...and the computed check value
	br	next



 # complain about a checksum
 #
 # crcbotch(badcheckvalue)...called from the hex and image loader
 #
 # calls hexout (r0,r1,r2)
 #
crcbotch:
	addr	e_crc,tos		# print the crc message
	bsr	wstrnul
	movd	4(sp),tos		# print the computed check value
	bsr	hexout
	ret	$4
 #
e_crc:	.asciz	"\012\015E CRC\015"	# the crc message


 # hexbyte - read and covert a hex ascii byte
 #
 # uses:	r0
 # returns:
 #	r1
 #
hexbyte:bsr	skipblanks		# skip any leading blanks
	movqd	$0,r1			# assemble the value in r1
	bsr	readbyte		# get a byte
	bsr	hex_ins			# shift into r1
	bsr	readbyte		# and do it again
	bsr	hex_ins
	ret	$0



 # skipblanks - read over blanks
 #
 # returns:  r0 - result from peek
 #
skipblanks:
	bsr	peek			# will the next byte read be
	cmpb	$ASC_SP,r0			# ...a blank?
	bne	sb1			# if not, we're done
	bsr	readbyte		# otherwise, read it
	br	skipblanks		# ...and try for another
sb1:	ret	$0



 # nonseq - execute until non-sequential breakpoint
nonseq:					#
#if	MMAX_DPC
	br	nonseq.ret		#
#endif

 # step - single step once
 #
step:					#
	br	step.ret		# begin execution


 # go - start execution
 #
go:	br	normal.ret		# restart with a clear t bit



verify:
	bsr	hexin			# get base address
	movd	r0,r4			# ...in r4
	bsr	hexin			# get length
	movd	r0,r5			# ...in r5
	movqd	$0,r1			# accumulate checksum in r1
ver0:	movzbd	0(r4),r0		# get one byte
	addd	r0,r1			# add it in
	addqd	$1,r4			# select the next byte
	acbd	$-1,r5,ver0		
	movd	r1,tos			# print the computed checksum
	bsr	hexout
	br	next



 # image - binary load
 #
image:	bsr	asm32			# get the load address
	movd	r1,r2			# ...(in r2)
	bsr	asm32			# get the length (r1)
	movqd	$0,r3			# initialize checksum
	addqd	$1,r1			# do any zero length loads correctly
	br	i3			# ...by adding one and skipping one xfer
i2:	bsr	ibyte			# get the next byte
	movb	r0,0(r2)		# store it in memory
	addqd	$1,r2			# select the next address
	addb	-1(r2),r3		# accumulate checksum from memory
i3:	acbd	$-1,r1,i2		# loop fun
	bsr	ibyte			# get remote checksum
	cmpb	r0,r3			# compare with local checksum
	beq	i4			# if equal, we're done
	movzbd	r3,tos			# otherwise, should prob. say something
	bsr	crcbotch		# note the problem and the value
i4:	br	restart



 # assemble a 32 bit word, least significant byte first
 #
 # uses:		r0
 # returns:	r1
 #
asm32:	movqd	$0,r1			# accumulate value in r1
	movqw	$4,tos			# it takes four bytes
asm32a:	bsr	ibyte			# get one of them
	movb	r0,r1			# put in in r1<7:0>
	rotd	$-8,r1			# adjust significance of bytes
	acbw	$-1,tos,asm32a		# loop fun
	cmpqw	$0,tos			# pop off the loop counter
	ret	$0			#



 # the fill command
 #
 # r1 - fill byte
 # r2 - beginning (next) address
 # r3 - ending address
 #
fill:	bsr	hexin			# get the start address
	movd	r0,r2
	bsr	hexin			# get the ending address
	movd	r0,r3
	bsr	hexin			# get the value to fill with
	movd	r0,r1			# ...now in r1
	bsr	setcontext		# determine context
	cmpqb	$c_byte,r0		#
	bne	fill2			#
fill1:	cmpd	r2,r3			#
	bhi	next			#
	movb	r1,0(r2)		# store the byte
	addqd	$1,r2			# select the next address
	br	fill1			#
fill2:	movd	r0,r4			#
	addr	0(r0)[r0:b],r5		# r5 <- context [i.e., 1/2] * 2
	movd	r1,tos			# tos = source
fill3:	cmpd	r2,r3			# at ending address?
	bhi	next
	addr	tos,tos			# fill w/d is src, at tos
	movd	r2,tos			# push dst
	movzbd	r4,tos			# push context
	bsr	conmove			# move
	addd	r5,r2			# select next item
	br	fill3



 # move command
 #
move:	
	bsr	hexin			# get source address
	movd	r0,tos			# save this for r1
	bsr	hexin			# get destination address
	movd	r0,r2			# in r2, for string2
	bsr	hexin			# get length
	movd	r0,r3
	movd	tos,r1			# src in r1
	bsr	setcontext		# allow b/w/d
	movd	r0,r4			# need r0 for length
	movd	r3,r0			# so put the length there
movego:	caseb	movelst[r4:b]		# select a move with the right context	
movelst:.byte	bm-movego,wm-movego,dm-movego
bm:	movsb				# move bytes
wm:	movsw				# move words
dm:	movsd				# move doubles
	br	next



 # setcontext - read a byte, and return...
 #
 # r0	- c_byte, c_word, or c_dble as per nothing/'b', 'w', 'd'
 #
 #	  c_byte is known to be zero
 #
bwdlist:	.ascii	"dwb"
bwdsize:	.byte	.-bwdlist
bwdspec:	.byte	3,1
setcontext:
	save	[r1,r4]
	bsr	skipblanks		# skip to last parameter
	bsr	readbyte		# read it
	movb	r0,r4			# must be in 0/b/w/d
	addr	bwdlist,r1		#
	movzbd	bwdsize,r0		#
	skpsb	[u]			# search for b,w,d
	checkb	r0,bwdspec,r0		# normalize 1..3 to 0..2
	bfc	setc1			# if f=0, done
	movqd	$0,r0			# end of line or error
setc1:	restore	[r1,r4]
	ret	$0



 # the dump command
 #
 # r3: starting (next) address
 # r4: byte count remaining
 # r5: checksum for the current line
 #
dump:	bsr	hexin			# get the start address
	movd	r0,r3
	bsr	hexin			# get the byte count (alas, in hex)
	movd	r0,r4			#
dump1:	movzbd	$ASC_L,tos
	bsr	writebyte
	movd	r3,tos			# write out the address
	bsr	hextwo			# (without an `=')
	movzbd	$ASC_SP,tos			# do a space
	bsr	writebyte
	movqd	$0,r5			# initialize the checksum byte
	addqd	$1,r4			# want a parameter of zero to work
	br	dump3a			# so add one and jump to the test
dump2:	movzbd	0(r3),tos		# output the next byte
	bsr	hexbyteout
	addb	0(r3),r5		# factor it in to the checksum
	addqd	$1,r3			# select the next address
dump2a:	extsb	r3,r0,$0,$2		# add a space if address<1:0>=0
	cmpqb	$0,r0			# this can't
	bne	dump3			# ...possibly
	movzbd	$ASC_SP,tos			# ...be worth
	bsr	writebyte		# ...the effort
dump3:	extsb	r3,r0,$0,$4		# get address<3:0>, check if zero
	cmpqb	$0,r0			# if so, start a new line
	bne	dump3a			# otherwise, do another byte (if nec.)
	bsr	endline
	br	dump4			# be sure the line isn't ended twice
dump3a:	acbd	$-1,r4,dump2		# here with no line termination
	bsr	endline
	br	next
dump4:	acbd	$-1,r4,dump1		# here after a line is terminated
	br	next
endline:movd	r5,tos			# output the check sum byte and crlf
	bsr	hexbyteout		#
	movqd	$0,r5			# reset the checksum byte
	addr	crlf,tos		# need a crlf
	bsr	wstrnul			# ...to terminate the line
	ret	$0



 # all - print all registers
 #
all:	movzbd	ejregt,r3		# get the register table ending address
	movqd	$0,r4			# keep display count in r4
	movd	$0xaabb20,tos		# put template for name string on tos
all1:	movw	regt-3[r3:b],1(sp)	# insert register name to template
	addr	tos,tos			# push the template address
	bsr	wstrnul			# write the name, preceded by a blank
	movzbd	regt-1[r3:b],r0		# get the fp offset out of the table
	movd	0(fp)[r0:b],tos		# get the register off the stack
	bsr	hexout			# go print it
	addqd	$1,r4			# update display count
	extsb	r4,r0,$0,$2		# get display count <1:0>
	cmpqb	$0,r0			# if zero, end of the line
	bne	all2			# no, continue
	addr	crlf,tos		# yes, do a crlf
	bsr	wstrnul
all2:	acbd	$-3,r3,all1		# display some more?
	br	restart



 # setcfg prototype
 #
xcfg0:	.byte	0x0e,0x0b,0
	jump	r0
	.set	xclen, 5
 #
 # x setcfg
 #
xconfig:
#if	MMAX_XPC
	bsr	hexin			# obtain the ``new'' configuration
	lprd	cfg, r0			# 	and set it
	br	restart
#endif
#if	MMAX_APC || MMAX_DPC
	adjspb	$xclen			# make room for setcfg sequence
	movmb	xcfg0,tos,$xclen	# move it to tos
	bsr	hexin			# obtain the ``new'' configuration
	inssd	r0,1(sp),$7,$4		# create the instruction
	addr	rmmu2,r0		# warm up the getaway car
	jump	tos			# configure
#endif


#if	MMAX_XPC
 #
 # wmmu - write mmu register XPC 532
 #
wmmcase:	.word	wnop-wcase	#  0 ---
		.word	wnop-wcase	#  1 ---
		.word	wnop-wcase	#  2 ---
		.word	wnop-wcase	#  3 ---
		.word	wnop-wcase	#  4 ---
		.word	wnop-wcase	#  5 ---
		.word	wnop-wcase	#  6 ---
		.word	wnop-wcase	#  7 ---
		.word	wnop-wcase	#  8 ---
		.word	wmcr-wcase	#  9 MCR
		.word	wmsr-wcase	# 10 MSR
		.word	wtear-wcase	# 11 TEAR
		.word	wptb0-wcase	# 12 PTB0
		.word	wptb1-wcase	# 13 PTB1
		.word	wivar0-wcase	# 14 IVAR0
		.word	wivar1-wcase	# 15 IVAR1
		.word	wfpu0-wcase	# 16 F0 - Floating point registers
		.word	wfpu1-wcase	# 17 F1
		.word	wfpu2-wcase	# 18 F2
		.word	wfpu3-wcase	# 19 F3
		.word	wfpu4-wcase	# 20 F4
		.word	wfpu5-wcase	# 21 F5
		.word	wfpu6-wcase	# 22 F6
		.word	wfpu7-wcase	# 23 F7
		.word	wfsr-wcase	# 24 FSR
wmmu:	
	bsr	hexin			# get mmu register number
	movd	r0,r6
	bsr	eqhexin			# get the new register value (r0)
wcase:	casew	wmmcase[r6:w]		# case to short store mmu inst
wmcr:	lmr	mcr,r0
	GETCPUID(r1)
	movd	r0, old_mcr[r1:d]
	br	wmmu1
wmsr:	lmr	msr,r0
	br	wmmu1
wtear:	lmr	tear,r0
	br	wmmu1
wptb0:	lmr	ptb0,r0
	br	wmmu1
wptb1:	lmr	ptb1,r0
	br	wmmu1
wivar0:	lmr	ivar0,r0
	br	wmmu1
wivar1:	lmr	ivar1,r0
	br	wmmu1
 #
 #	BEWARE - Floating point register writes do not work for all XPC 532
 #	registers.  Only 32-bit registers supported here.
 #
wfpu0:	movd	r0,tos		########## FPU registers ##########
	movf	tos,f0
	br	wmmu1
wfpu1:	movd	r0,tos
	movf	tos,f1
	br	wmmu1
wfpu2:	movd	r0,tos
	movf	tos,f2
	br	wmmu1
wfpu3:	movd	r0,tos
	movf	tos,f3
	br	wmmu1
wfpu4:	movd	r0,tos
	movf	tos,f4
	br	wmmu1
wfpu5:	movd	r0,tos
	movf	tos,f5
	br	wmmu1
wfpu6:	movd	r0,tos
	movf	tos,f6
	br	wmmu1
wfpu7:	movd	r0,tos
	movf	tos,f7
	br	wmmu1
wfsr:	lfsr	r0
	# br	wmmu1
wnop:					# No-op
wmmu1:	br	restart			# restart will restore the stack level

 #
 # rmmu - read mmu register XPC
 #
rmmcase:	.word	rnop-rcase	#  0 ---
		.word	rnop-rcase	#  1 ---
		.word	rnop-rcase	#  2 ---
		.word	rnop-rcase	#  3 ---
		.word	rnop-rcase	#  4 ---
		.word	rnop-rcase	#  5 ---
		.word	rnop-rcase	#  6 ---
		.word	rnop-rcase	#  7 ---
		.word	rnop-rcase	#  8 ---
		.word	rmcr-rcase	#  9 FEW
		.word	rmsr-rcase	# 10 ASR
		.word	rtear-rcase	# 11 TEAR
		.word	rptb0-rcase	# 12 PTB0
		.word	rptb1-rcase	# 13 PTB1
		.word	rivar0-rcase	# 14 IVAR0
		.word	rivar1-rcase	# 15 IVAR1
		.word	rfpu0-rcase	# 16 F0 - Floating Point Registers
		.word	rfpu1-rcase	# 17 F1
		.word	rfpu2-rcase	# 18 F2
		.word	rfpu3-rcase	# 19 F3
		.word	rfpu4-rcase	# 20 F4
		.word	rfpu5-rcase	# 21 F5
		.word	rfpu6-rcase	# 22 F6
		.word	rfpu7-rcase	# 23 F7
		.word	rfsr-rcase	# 24 FSR
rmmu:	
	bsr	hexin			# get the mmu register number
rcase:	casew	rmmcase[r0:w]		# case to short store mmu inst
rmcr:	smr	mcr,tos
	br	rmmu1
rmsr:	smr	msr,tos
	br	rmmu1
rtear:	smr	tear,tos
	br	rmmu1
rptb0:	smr	ptb0,tos
	br	rmmu1
rptb1:	smr	ptb1,tos
	br	rmmu1
rivar0:	smr	ivar0,tos
	br	rmmu1
rivar1:	smr	ivar1,tos
	br	rmmu1
 #
 #	BEWARE - Floating point register writes do not work for all XPC 532
 #	registers.  Only 32-bit registers supported here.
 #
rfpu0:	movf	f0,tos
	br	rmmu1
rfpu1:	movf	f1,tos
	br	rmmu1
rfpu2:	movf	f2,tos
	br	rmmu1
rfpu3:	movf	f3,tos
	br	rmmu1
rfpu4:	movf	f4,tos
	br	rmmu1
rfpu5:	movf	f5,tos
	br	rmmu1
rfpu6:	movf	f6,tos
	br	rmmu1
rfpu7:	movf	f7,tos
	br	rmmu1
rfsr:	sfsr	tos
	br	rmmu1
rnop:	movqd	$0,tos			# No-op
rmmu1:	bsr	hexout			# print it
rmmu2:	br	restart			# restart will restore the stack level
#endif


#if	MMAX_APC
 #
 # wmmu - write mmu register APC
 #
wmmcase:	.word	wbar-wcase	#  0 BAR
		.word	wnop-wcase	#  1 ---
		.word	wbmr-wcase	#  2 BMR
		.word	wbdr-wcase	#  3 BDR
		.word	wnop-wcase	#  4 ---
		.word	wnop-wcase	#  5 ---
		.word	wbear-wcase	#  6 BEAR
		.word	wnop-wcase	#  7 ---
		.word	wnop-wcase	#  8 ---
		.word	wfew-wcase	#  9 FEW
		.word	wasr-wcase	# 10 ASR
		.word	wtear-wcase	# 11 TEAR
		.word	wptb0-wcase	# 12 PTB0
		.word	wptb1-wcase	# 13 PTB1
		.word	wivar0-wcase	# 14 IVAR0
		.word	wivar1-wcase	# 15 IVAR1
		.word	wfpu0-wcase	# 16 F0 - Floating point registers
		.word	wfpu1-wcase	# 17 F1
		.word	wfpu2-wcase	# 18 F2
		.word	wfpu3-wcase	# 19 F3
		.word	wfpu4-wcase	# 20 F4
		.word	wfpu5-wcase	# 21 F5
		.word	wfpu6-wcase	# 22 F6
		.word	wfpu7-wcase	# 23 F7
		.word	wfsr-wcase	# 24 FSR
wmmu:	
	bsr	hexin			# get mmu register number
	movd	r0,r6
	bsr	eqhexin			# get the new register value (r0)
wcase:	casew	wmmcase[r6:w]		# case to short store mmu inst
wbar:	lmr	bar,r0
	br	wmmu1
wbmr:	lmr	bmr,r0
	br	wmmu1
wbdr:	lmr	bdr,r0
	br	wmmu1
wbear:	lmr	bear,r0
	br	wmmu1
wfew:	lmr	few,r0
	GETCPUID(r1)
	movd	r0, old_few[r1:d]
	br	wmmu1
wasr:	lmr	asr,r0
	br	wmmu1
wtear:	lmr	tear,r0
	br	wmmu1
wptb0:	lmr	ptb0,r0
	br	wmmu1
wptb1:	lmr	ptb1,r0
	br	wmmu1
wivar0:	lmr	ivar0,r0
	br	wmmu1
wivar1:	lmr	ivar1,r0
	br	wmmu1
wfpu0:	movd	r0,tos		########## FPU registers ##########
	movf	tos,f0
	br	wmmu1
wfpu1:	movd	r0,tos
	movf	tos,f1
	br	wmmu1
wfpu2:	movd	r0,tos
	movf	tos,f2
	br	wmmu1
wfpu3:	movd	r0,tos
	movf	tos,f3
	br	wmmu1
wfpu4:	movd	r0,tos
	movf	tos,f4
	br	wmmu1
wfpu5:	movd	r0,tos
	movf	tos,f5
	br	wmmu1
wfpu6:	movd	r0,tos
	movf	tos,f6
	br	wmmu1
wfpu7:	movd	r0,tos
	movf	tos,f7
	br	wmmu1
wfsr:	lfsr	r0
	# br	wmmu1
wnop:					# No-op
wmmu1:	br	restart			# restart will restore the stack level

 #
 # rmmu - read mmu register APC
 #
rmmcase:	.word	rbar-rcase	#  0 BAR
		.word	rnop-rcase	#  1 ---
		.word	rbmr-rcase	#  2 BMR
		.word	rbdr-rcase	#  3 BDR
		.word	rnop-rcase	#  4 ---
		.word	rnop-rcase	#  5 ---
		.word	rbear-rcase	#  6 BEAR
		.word	rnop-rcase	#  7 ---
		.word	rnop-rcase	#  8 ---
		.word	rfew-rcase	#  9 FEW
		.word	rasr-rcase	# 10 ASR
		.word	rtear-rcase	# 11 TEAR
		.word	rptb0-rcase	# 12 PTB0
		.word	rptb1-rcase	# 13 PTB1
		.word	rivar0-rcase	# 14 IVAR0
		.word	rivar1-rcase	# 15 IVAR1
		.word	rfpu0-rcase	# 16 F0 - Floating Point Registers
		.word	rfpu1-rcase	# 17 F1
		.word	rfpu2-rcase	# 18 F2
		.word	rfpu3-rcase	# 19 F3
		.word	rfpu4-rcase	# 20 F4
		.word	rfpu5-rcase	# 21 F5
		.word	rfpu6-rcase	# 22 F6
		.word	rfpu7-rcase	# 23 F7
		.word	rfsr-rcase	# 24 FSR
rmmu:	
	bsr	hexin			# get the mmu register number
rcase:	casew	rmmcase[r0:w]		# case to short store mmu inst
rbar:	smr	bar,tos
	br	rmmu1
rbmr:	smr	bmr,tos
	br	rmmu1
rbdr:	smr	bdr,tos
	br	rmmu1
rbear:	smr	bear,tos
	br	rmmu1
rfew:	smr	few,tos
	# no need to test ASR.bkpt & clear FEW.bx (?)
	br	rmmu1
rasr:	smr	asr,tos
	br	rmmu1
rtear:	smr	tear,tos
	br	rmmu1
rptb0:	smr	ptb0,tos
	br	rmmu1
rptb1:	smr	ptb1,tos
	br	rmmu1
rivar0:	smr	ivar0,tos
	br	rmmu1
rivar1:	smr	ivar1,tos
	br	rmmu1
rfpu0:	movf	f0,tos
	br	rmmu1
rfpu1:	movf	f1,tos
	br	rmmu1
rfpu2:	movf	f2,tos
	br	rmmu1
rfpu3:	movf	f3,tos
	br	rmmu1
rfpu4:	movf	f4,tos
	br	rmmu1
rfpu5:	movf	f5,tos
	br	rmmu1
rfpu6:	movf	f6,tos
	br	rmmu1
rfpu7:	movf	f7,tos
	br	rmmu1
rfsr:	sfsr	tos
	br	rmmu1
rnop:	movqd	$0,tos			# No-op
rmmu1:	bsr	hexout			# print it
rmmu2:	br	restart			# restart will restore the stack level
#endif

#if	MMAX_DPC
 # wmmu - write mmu register
 #
wmmcase:	.byte	wbpr0-wcase,wbpr1-wcase,wnop-wcase,wnop-wcase
		.byte	wpf0-wcase,wpf1-wcase,wnop-wcase,wnop-wcase,wsc-wcase
		.byte	wnop-wcase,wmsr-wcase,wbcnt-wcase,wptb0-wcase
		.byte	wptb1-wcase,wnop-wcase,weia-wcase
wmmu:	
	bsr	hexin			# get mmu register number
	movd	r0,r6
	bsr	eqhexin			# get the new register value (r0)
	cmpb	r6,$0xf			# check if in range
	bgt	wnop			# no-op if not
wcase:	caseb	wmmcase[r6:b]		# case to short store mmu inst
wbpr0:	lmr	bpr0,r0
	br	wmmu1
wbpr1:	lmr	bpr1,r0
	br	wmmu1
wpf0:	lmr	pf0,r0
	br	wmmu1
wpf1:	lmr	pf1,r0
	br	wmmu1
wsc:	lmr	sc,r0
	br	wmmu1
wmsr:	lmr	msr,r0
	GETCPUID(r1)
	movd	r0, old_msr[r1:d]
	br	wmmu1
wbcnt:	lmr	bcnt,r0
	br	wmmu1
wptb0:	lmr	ptb0,r0
	br	wmmu1
wptb1:	lmr	ptb1,r0
	br	wmmu1
weia:	lmr	eia,r0
	br	wmmu1
wnop:	
wmmu1:	br	restart			# restart will restore the stack level



 # rmmu - read mmu register
 #
rmmcase:	.byte	rbpr0-rcase,rbpr1-rcase,rnop-rcase,rnop-rcase
		.byte	rpf0-rcase,rpf1-rcase,rnop-rcase,rnop-rcase,rsc-rcase
		.byte	rnop-rcase,rmsr-rcase,rbcnt-rcase,rptb0-rcase
		.byte	rptb1-rcase,rnop-rcase,reia-rcase
rmmu:	
	bsr	hexin			# get the mmu register number
	cmpb	r0,$0xf			# check if in range
	bgt	rnop			# no-op if not
rcase:	caseb	rmmcase[r0:b]		# case to short store mmu inst
rbpr0:	smr	bpr0,tos
	br	rmmu1
rbpr1:	smr	bpr1,tos
	br	rmmu1
rpf0:	smr	pf0,tos
	br	rmmu1
rpf1:	smr	pf1,tos
	br	rmmu1
rsc:	smr	sc,tos
	br	rmmu1
rmsr:	smr	msr,tos
	GETCPUID(r1)
	tbitd	$(MSR_BE_BIT), old_msr[r1:d] # Took bpt in mmu?
	bfs	rmmu1			# If fs, yes
	cbitb	$(MSR_BE_BIT), 0(sp)	# Else make sure bpt bit clear
	br	rmmu1
rbcnt:	smr	bcnt,tos
	br	rmmu1
rptb0:	smr	ptb0,tos
	br	rmmu1
rptb1:	smr	ptb1,tos
	br	rmmu1
reia:	smr	eia,tos
	br	rmmu1
rnop:	movqd	$0,tos
rmmu1:	bsr	hexout			# print it
rmmu2:	br	restart			# restart will restore the stack level
#endif
