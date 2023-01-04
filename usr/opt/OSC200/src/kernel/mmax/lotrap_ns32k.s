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
 *	@(#)$RCSfile: lotrap_ns32k.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:32 $
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

#include <stat_time.h>
#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#include <mmax_etlb.h>
#include <mmax_idebug.h>

#include <sys/errno.h>
#include <mach/kern_return.h>
#include <kern/syscall_sw.h>

#include <mmax/mlmacros.h>
#include <mmax/psl.h>
#include <mmax/mmu.h>
#include <mmax/fpu.h>
#include <mmax/trap.h>
#include <mmax/cpudefs.h>
#include <mach/mmax/exception.h>
#include <cpus.h>

#if	!STAT_TIME
#include <mmax/timer_macros.h>
#endif


	.data
	.globl	_need_ast
	/*
	 * Declarations for the following are found in lomisc_ns32k.s.
	 */
	.globl	_psa			/* panic save areas for all cpus */
	.globl	_intstack		/* temporary interrupt stack */
	.globl	_eintstack		/* top of interrupt stack */
	.globl	_boothowto
	.globl	_OSmodpsr		/* valid modpsr pair for kernel */

	.globl	_nmicnt
	.globl	_abtcnt
	.globl	_fpucnt
	.globl	_illcnt
	.globl	_svccnt
	.globl	_dvzcnt
	.globl	_flgcnt
	.globl	_bptcnt
	.globl	_trccnt
	.globl	_undcnt
	.globl	_astcnt
#if	MMAX_XPC
	.globl	_ovfcnt
	.globl	_dbgcnt
#endif
#if	MMAX_XPC || MMAX_APC
	.globl	_tsecnt
	.globl	_sticnt
	.globl	_bogcnt
#endif
#if	MMAX_APC
	.globl	_bercnt
#endif
	.text
	.globl	_halt_cpu
	.globl	_panic
	.globl	_syscall
	.globl	_itbl
	.globl	_trap
	.globl	dnmi
	.globl	dtrc
	.globl	dbpt

/*
 * Hide some simple name changes.
 */
#if	MMAX_XPC
#define	MMU_DDI_BIT	MSR_DDI_BIT
#define	MMU_USR_BIT	MSR_USF_BIT
#define	CPU_STATUS_MASK	MSR_STT_MASK
#define	CPU_REG_CSR	XPCREG_CSR
#define	CPU_ERR_CSR	XPCREG_ERR
#define	PSA_BOARD_TYPE	PSA_XPC_TYPE
#endif

#if	MMAX_APC
#define	MMU_DDI_BIT	ASR_DDI_T_BIT
#define	MMU_USR_BIT	ASR_USR_MODE_BIT
#define	CPU_STATUS_MASK	ASR_ST_TMASK
#define	CPU_REG_CSR	APCREG_CSR
#define	CPU_ERR_CSR	APCREG_ERR
#define	PSA_BOARD_TYPE	PSA_APC_TYPE
#endif

#if	MMAX_DPC
#define	PSA_BOARD_TYPE	PSA_DPC_TYPE
#endif

/* Commonly used macros. These macros are used by all trap handlers
 * so that the trap handling code is as efficient and streamlined
 * as is possible.
 */

/*
 * At trap time, the following values are available with respect
 * to the frame pointer.  Note that these values are stacked by
 * the chip and so are available before a BUILD_FRAME is done.
 */
#define	TRAP_PC		4
#define	TRAP_PSR	8
#define	TRAP_MOD	10

#define TRAP(param1,param2) \
	addr	param2,r1			; \
	addr	param1,r0			; \
	jsr	@_trap


#if	MMAX_IDEBUG
/*
 * The MMAX_IDEBUG option records the various interrupts/traps as they come in.
 * The saved information includes the identy of the interrupt/trap, the pc
 * from whence it came, and the thread that was interrupted/trapped.
 *
 * MMAX_IDEBUG also checks various ICU in-service register state conditions.
 *
 * N.B.  MMAX_IDEBUG hasn't been checked with MMAX_DPC and probably won't work.
 */
#define	ID_TSE		-1
#define	ID_ABT		-2
#define	ID_FPU		-3
#define	ID_OVF		-6
#define	ID_DBG		-7
#define	ID_BER		-8
#define	ID_BOGUS_M	-9
#define	ID_BOGUS_S	-10
#define	ID_DUART	-11
#define	ID_TIMER_S	-12

#define	IDBG(reg,ident,intfrom) \
	GETCPUID(reg); \
	movd	$(ident), _last_inttype[reg:d]; \
	movd	intfrom, _last_intpc[reg:d]; \
	movd	_active_threads[reg:d], _last_intth[reg:d]
#define	TRACK()		jsr	@_tracker
	.globl	_tracker
	.globl	_rett_out_intr_checks

#else
#define	IDBG(reg,ident,intfrom)
#define	TRACK()
#endif

#if	MMAX_XPC || MMAX_APC
/*
 * Define macros for handling ICU state during interrupt processing.
 * Refer to comment in icu.h for an explanation of how the ICU in-
 * service register is manipulated during interrupt service.
 */

#define	SET_NONFATAL	sbitb	$(ICU_NONFATAL_BIT), @M_ICU_BASE+ISRV

/*
 * The {ENTER,EXIT}_INTERRUPT_NORMAL macros are used for soft error or
 * slave ICU interrupts between the NORMAL and NONFATAL bits, hard
 * errors between the NONFATAL and POWERFAIL bits, and POWERFAIL.
 *
 * We don't make any MMAX_IDEBUG assertions about these macros because
 * lower-priority bits can be in any condition and there are possible
 * races with the NONFATAL and POWERFAIL bits.
 */
#define ENTER_INTERRUPT_NORMAL(label)	bispsrw	$PSR_I
#define	EXIT_INTERRUPT_NORMAL(label)

/*
 * The {ENTER,EXIT}_INTERRUPT_RACE macros are used exclusively for
 * the TSE, VECBUS and BOGUS interrupts that race with the NORMAL bit.
 * On entry to the interrupt service routine, the NORMAL bit is set and
 * its old status is saved; the bit in the in-service register for the
 * interrupt is cleared; and processor interrupts are re-enabled.  On exit
 * from the interrupt service routine, the old NORMAL bit status is checked
 * and, if set, the NONFATAL bit is set.  When the subsequent RETI is done,
 * the NONFATAL bit will be cleared and the NORMAL bit will be left set.
 */
#if	!MMAX_IDEBUG
#define ENTER_INTERRUPT_RACE(ICUBIT,label) \
	sbitb	$(ICU_NORMAL_BIT-8), @M_ICU_BASE+ISRV+RBIAS ; \
	sfsd	r6 ; \
	cbitb	$(ICUBIT-8), @M_ICU_BASE+ISRV+RBIAS ; \
	bispsrw	$PSR_I
#define	EXIT_INTERRUPT_RACE(label) \
	cmpqd	$0, r6 ; \
	beq	label ; \
	SET_NONFATAL ; \
label:
#else	!MMAX_IDEBUG
#define ENTER_INTERRUPT_RACE(ICUBIT,label) \
	sbitb	$(ICU_NORMAL_BIT-8), @M_ICU_BASE+ISRV+RBIAS ; \
	sfsd	r6 ; \
	cbitb	$(ICUBIT-8), @M_ICU_BASE+ISRV+RBIAS ; \
	bispsrw	$PSR_I ; \
	cmpb	$0, @M_ICU_BASE+ISRV ; \
	beq	label/**/_pass1 ; \
label/**/_bogon: \
	movb	@M_ICU_BASE+ISRV, @enter_int_bogon ; \
	movb	@M_ICU_BASE+ISRV+RBIAS, @enter_int_bogon+1 ; \
	PRINTF(@enter_int_bogon_mes, @enter_int_bogon) ; \
	PANIC(@enter_int_bogon_panic, $0, $0) ; \
label/**/_pass1: \
	cmpb	$(ICU_NORMAL > 8), @M_ICU_BASE+ISRV+RBIAS ; \
	bne	label/**/_bogon
	
/*
 * We know that only the NORMAL bit can be set in the in-service register
 * at this time.  ENTER_INTERRUPT_RACE transformed the interrupt bit in
 * the isrv to the NORMAL bit with processor interrupts disabled, so the
 * processor can never see a lower-priority bit set in the isrv.  Nor can
 * the processor see a higher-priority bit set because if the priority is
 * higher than the NORMAL bit than the interrupt will already have been
 * taken or if the priority is between our interrupt and the NORMAL bit
 * then either (a) the interrupt was already taken in a race with setting
 * the NORMAL bit or (b) the higher-priority interrupt bit is set in the
 * pending register but not in the in-service register.
 */
#define	EXIT_INTERRUPT_RACE(label) \
	cmpb	$0, @M_ICU_BASE+ISRV ; \
	beq	label/**/_pass1 ; \
label/**/_bogon: \
	movb	@M_ICU_BASE+ISRV, @exit_int_bogon ; \
	movb	@M_ICU_BASE+ISRV+RBIAS, @exit_int_bogon+1 ; \
	PRINTF(@exit_int_bogon_mes, @exit_int_bogon) ; \
	PANIC(@exit_int_bogon_panic, $0, $0); \
label/**/_pass1: \
	cmpb	$(ICU_NORMAL > 8), @M_ICU_BASE+ISRV+RBIAS ; \
	bne	label/**/_bogon ; \
	cmpqd	$0, r6 ; \
	beq	label ; \
	SET_NONFATAL ; \
label:
#endif	/* !MMAX_IDEBUG */
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_DPC
#define ENTER_INTERRUPT_NORMAL(label)
#define EXIT_INTERRUPT_NORMAL(label)
#endif


/*
 * The interrupt dispatch table for ns32xxx family processors and
 * miscellaneous declarations.
 */

	.text
.globl	_start_text
_start_text:

 #
 # DESCRIPTION:
 # 	Declare the interrupt dispatch table. The macro only sets up the
 # 	offsets. The mod field must be set at runtime. See initialization code
 #	in locore_ns32k.s.
 #
	.align	4
	.globl	_modbegin
_modbegin:
_module:				# Module Table used for Interrupts
	.double	0			# Static Base
	.double	0			# Link Base
	.double _modbegin		# Start of module
	.double 0			# Reserved

	.globl	_Vectors
	.align	4
#define	VECTOR(label, begin)	.word	_module; .word	(label - begin)

#if	MMAX_XPC || MMAX_APC
/*
 * cascades: Cascade Table for Cascaded ICUs
 *
 * USAGE:
 *      Used by NS32000 systems with cascaded ICUs to determine the
 *      addressing used to get to the cascaded ICUs.

 *
 * ASSUMPTION:
 *      The cascade table is located immediately before the vector
 *      table.
 *
 * NOTES:
 *	The XPC does not have any slave ICUs so the cascade table
 *	is useless; it is included only in the event that a really
 *	weird error happens that causes an attempt to use the cascade table.
 */

no_icu:
        .byte   49              /* Value used for illegal cascaded ICU */
        .align  4
_cascades:
        .int    no_icu          /* Unused (0) */
        .int    no_icu          /* Unused (1) */
        .int    no_icu          /* Unused (2) */
        .int    no_icu          /* Unused (3) */
        .int    no_icu          /* Unused (4) */
        .int    no_icu          /* Unused (5) */
        .int    no_icu          /* Unused (6) */
        .int    no_icu          /* Unused (7) */
#if	MMAX_XPC
	.int	no_icu		/* Unused (8) */
#endif
#if	MMAX_APC
        .int    S_ICU_BASE      /* Slave ICU Base Address (8) */
#endif
        .int    no_icu          /* Unused (9) */
        .int    no_icu          /* Unused (10) */
        .int    no_icu          /* Unused (11) */
        .int    no_icu          /* Unused (12) */
        .int    no_icu          /* Unused (13) */
        .int    no_icu          /* Unused (14) */
        .int    no_icu          /* Unused (15) */
#endif	/* MMAX_XPC || MMAX_APC */

_Vectors:				##### Interrupt base (traps) #####
#if	MMAX_XPC || MMAX_APC
	VECTOR(res, _modbegin)		#  0	(unused)
	VECTOR(res, _modbegin)		#  1	(unused)
	VECTOR(abt, _modbegin)		#  2	MMU abort
	VECTOR(fpu, _modbegin)		#  3	FPU fault
	VECTOR(ill, _modbegin)		#  4	illegal operation
	VECTOR(svc, _modbegin)		#  5	supervisor call
	VECTOR(dvz, _modbegin)		#  6	divide by zero
	VECTOR(flg, _modbegin)		#  7	flag
	VECTOR(bpt, _modbegin)		#  8	breakpoint
	VECTOR(trc, _modbegin)		#  9	trace
	VECTOR(und, _modbegin)		# 10	undefined instruction
#if	MMAX_XPC
	VECTOR(rbe, _modbegin)		# 11	restartable bus error
	VECTOR(nbe, _modbegin)		# 12	non-restartable bus error
	VECTOR(ovf, _modbegin)		# 13	integer overflow
	VECTOR(dbg, _modbegin)		# 14	debug
#endif
#if	MMAX_APC
	VECTOR(res, _modbegin)		# 11	(unused)
	VECTOR(ber, _modbegin)		# 12	bus error
	VECTOR(res, _modbegin)		# 13	(unused)
	VECTOR(res, _modbegin)		# 14	(unused)
#endif
	VECTOR(res, _modbegin)		# 15	(unused)
#endif	/* MMAX_XPC || MMAX_APC */
#if	MMAX_DPC
	VECTOR(nvi, _modbegin)	# Non-vectored interrupt (0)
	VECTOR(nmi, _modbegin)	# Non-maskable interrupt (1)
	VECTOR(abt, _modbegin)	# MMU abort (2)
	VECTOR(fpu, _modbegin)	# FPU fault (3)
	VECTOR(ill, _modbegin)	# Illegal operation (4)
	VECTOR(svc, _modbegin) # Supervisor call (5)
	VECTOR(dvz, _modbegin) # Divide by zero (6)
	VECTOR(flg, _modbegin) # Flag trap (7)
	VECTOR(bpt, _modbegin) # Breakpoint trap (8)
	VECTOR(trc, _modbegin) # Trace trap (9)
	VECTOR(und, _modbegin) # Undefined instruction (10)
	VECTOR(res, _modbegin) # Reserved (11)
	VECTOR(res, _modbegin) # Reserved (12)
	VECTOR(res, _modbegin) # Reserved (13)
	VECTOR(res, _modbegin) # Reserved (14)
	VECTOR(res, _modbegin) # Reserved (15)
#endif
#if	MMAX_XPC
					##### Vectored Interrupts #####
	VECTOR(powerfail, _modbegin)	# 16	powerfail (0)
	VECTOR(nmi, _modbegin)		# 17	system NMI (1)
	VECTOR(hard_nbi, _modbegin)	# 18	hard nanobus err (2)
	VECTOR(destsel_par, _modbegin)	# 19	DESTSEL parity err (3)
	VECTOR(btag_state, _modbegin)	# 20	BTAG state err (4)
	VECTOR(btag_cache, _modbegin)	# 21	BTAG cache inconsistency (5)
	VECTOR(rcv_addr_par, _modbegin)	# 22	rcvd address parity err (6)
	VECTOR(cache_par, _modbegin)	# 23	cache parity error (7)
	VECTOR(btag_tag_par, _modbegin)	# 24	BTAG TAG parity err (8)
	VECTOR(soft_nbi, _modbegin)	# 25	soft nanobus err (9)
	VECTOR(vb_err, _modbegin)	# 26	Vector Bus err (10)
	VECTOR(timer_s, _modbegin)	# 27	prof timer (11)
	VECTOR(undefintr, _modbegin)	# 28	(unused) (12)
	VECTOR(tse, _modbegin)		# 29	TSE (13)
	VECTOR(vecbus, _modbegin)	# 30	Vector Bus interrupt (14)
	VECTOR(bogus_m, _modbegin)	# 31	IGNORED - bogus (15)
	VECTOR(undefintr, _modbegin)	# 32	(unused)
	VECTOR(undefintr, _modbegin)	# 33	   ||
	VECTOR(undefintr, _modbegin)	# 34	   ||
	VECTOR(undefintr, _modbegin)	# 35	   ||
	VECTOR(undefintr, _modbegin)	# 36	   \/
	VECTOR(undefintr, _modbegin)	# 37	
	VECTOR(undefintr, _modbegin)	# 38	
	VECTOR(undefintr, _modbegin)	# 39	
	VECTOR(undefintr, _modbegin)	# 40	
	VECTOR(undefintr, _modbegin)	# 41	
	VECTOR(undefintr, _modbegin)	# 42	
	VECTOR(undefintr, _modbegin)	# 43	
	VECTOR(undefintr, _modbegin)	# 44	
	VECTOR(undefintr, _modbegin)	# 45	
	VECTOR(undefintr, _modbegin)	# 46	
	VECTOR(undefintr, _modbegin)	# 47	
	VECTOR(undefintr, _modbegin)	# 48	
	VECTOR(undefintr, _modbegin)	# 49	
#endif
#if	MMAX_APC
					##### Vectored Interrupts #####
	VECTOR(powerfail, _modbegin)	# 16	powerfail
	VECTOR(nmi, _modbegin)		# 17	system NMI
	VECTOR(hard_nbi0, _modbegin)	# 18	hard nanobus 0
	VECTOR(hard_nbi1, _modbegin)	# 19	hard nanobus 1
	VECTOR(hard_nbi2, _modbegin)	# 20	hard nanobus 2
	VECTOR(hard_nbi3, _modbegin)	# 21	hard nanobus 3
	VECTOR(destsel_par, _modbegin)	# 22	destsel parity
	VECTOR(undefintr, _modbegin)	# 23	(unused - above nonfatal)
	VECTOR(undefintr, _modbegin)	# 24	(unused - slave ICU input)
	VECTOR(soft_nbi0, _modbegin)	# 25	soft nanobus 0
	VECTOR(undefintr, _modbegin)	# 26	(unused)
	VECTOR(tse, _modbegin)		# 27	master timer - TSE
	VECTOR(vecbus, _modbegin)	# 28	vector bus
	VECTOR(undefintr, _modbegin)	# 29	(unused)
	VECTOR(undefintr, _modbegin)	# 30	(unused)
	VECTOR(bogus_m, _modbegin)	# 31	(icu race: ignore)
	VECTOR(soft_nbi1, _modbegin)	# 32	soft nanobus 1
	VECTOR(soft_nbi2, _modbegin)	# 33	soft nanobus 2
	VECTOR(rcv_addr_par4,_modbegin)	# 34	rcv address parity 4 (BYTE)
	VECTOR(rcv_addr_par0,_modbegin)	# 35	rcv address parity 0
	VECTOR(rcv_addr_par1,_modbegin)	# 36	rcv address parity 1
	VECTOR(rcv_addr_par2,_modbegin)	# 37	rcv address parity 2
	VECTOR(rcv_addr_par3,_modbegin)	# 38	rcv address parity 3
	VECTOR(btag_par0, _modbegin)	# 39	btag parity 0
	VECTOR(btag_par1, _modbegin)	# 40	btag parity 1
	VECTOR(vb_nak, _modbegin)	# 41	vector bus NAK
	VECTOR(vb_par, _modbegin)	# 42	vector bus parity
	VECTOR(vb_out_of_sync,_modbegin) # 43	vector bus out of synch
	VECTOR(cache_par, _modbegin)	# 44	cache parity
	VECTOR(duart, _modbegin)	# 45	DUART
	VECTOR(timer_s, _modbegin)	# 46	slave timer
	VECTOR(bogus_s, _modbegin)	# 47	(icu race: ignore)
	VECTOR(undefintr, _modbegin)	# 48	(unused)
	VECTOR(illegal_cascade, _modbegin) # 49	illegal cascade intr
#endif
#if	MMAX_XPC || MMAX_APC
	VECTOR(undefintr, _modbegin)	# 50	(unused)
	VECTOR(undefintr, _modbegin)	# 51	   ||
	VECTOR(undefintr, _modbegin)	# 52	   ||
	VECTOR(undefintr, _modbegin)	# 53	   ||
	VECTOR(undefintr, _modbegin)	# 54	   \/
	VECTOR(undefintr, _modbegin)	# 55
	VECTOR(undefintr, _modbegin)	# 56
	VECTOR(undefintr, _modbegin)	# 57
	VECTOR(undefintr, _modbegin)	# 58
	VECTOR(undefintr, _modbegin)	# 59
	VECTOR(undefintr, _modbegin)	# 60
	VECTOR(undefintr, _modbegin)	# 61
	VECTOR(undefintr, _modbegin)	# 62
	VECTOR(undefintr, _modbegin)	# 63
	VECTOR(undefintr, _modbegin)	# 64
	VECTOR(undefintr, _modbegin)	# 65
	VECTOR(undefintr, _modbegin)	# 66
	VECTOR(undefintr, _modbegin)	# 67
	VECTOR(undefintr, _modbegin)	# 68
	VECTOR(undefintr, _modbegin)	# 69
	VECTOR(undefintr, _modbegin)	# 70
	VECTOR(undefintr, _modbegin)	# 71
	VECTOR(undefintr, _modbegin)	# 72
	VECTOR(undefintr, _modbegin)	# 73
	VECTOR(undefintr, _modbegin)	# 74
	VECTOR(undefintr, _modbegin)	# 75
	VECTOR(undefintr, _modbegin)	# 76
	VECTOR(undefintr, _modbegin)	# 77
	VECTOR(undefintr, _modbegin)	# 78
	VECTOR(undefintr, _modbegin)	# 79
	VECTOR(undefintr, _modbegin)	# 80
	VECTOR(undefintr, _modbegin)	# 81
	VECTOR(undefintr, _modbegin)	# 82
	VECTOR(undefintr, _modbegin)	# 83
	VECTOR(undefintr, _modbegin)	# 84
	VECTOR(undefintr, _modbegin)	# 85
	VECTOR(undefintr, _modbegin)	# 86
	VECTOR(undefintr, _modbegin)	# 87
	VECTOR(undefintr, _modbegin)	# 88
	VECTOR(undefintr, _modbegin)	# 89
	VECTOR(undefintr, _modbegin)	# 90
	VECTOR(undefintr, _modbegin)	# 91
	VECTOR(undefintr, _modbegin)	# 92
	VECTOR(undefintr, _modbegin)	# 93
	VECTOR(undefintr, _modbegin)	# 94
	VECTOR(undefintr, _modbegin)	# 95
	VECTOR(undefintr, _modbegin)	# 96
	VECTOR(undefintr, _modbegin)	# 97
	VECTOR(undefintr, _modbegin)	# 98
	VECTOR(undefintr, _modbegin)	# 99
	VECTOR(undefintr, _modbegin)	# 100
	VECTOR(undefintr, _modbegin)	# 101
	VECTOR(undefintr, _modbegin)	# 102
	VECTOR(undefintr, _modbegin)	# 103
	VECTOR(undefintr, _modbegin)	# 104
	VECTOR(undefintr, _modbegin)	# 105
	VECTOR(undefintr, _modbegin)	# 106
	VECTOR(undefintr, _modbegin)	# 107
	VECTOR(undefintr, _modbegin)	# 108
	VECTOR(undefintr, _modbegin)	# 109
	VECTOR(undefintr, _modbegin)	# 110
	VECTOR(undefintr, _modbegin)	# 111
	VECTOR(undefintr, _modbegin)	# 112
	VECTOR(undefintr, _modbegin)	# 113
	VECTOR(undefintr, _modbegin)	# 114
	VECTOR(undefintr, _modbegin)	# 115
	VECTOR(undefintr, _modbegin)	# 116
	VECTOR(undefintr, _modbegin)	# 117
	VECTOR(undefintr, _modbegin)	# 118
	VECTOR(undefintr, _modbegin)	# 119
	VECTOR(undefintr, _modbegin)	# 120
	VECTOR(undefintr, _modbegin)	# 121
	VECTOR(undefintr, _modbegin)	# 122
	VECTOR(undefintr, _modbegin)	# 123
	VECTOR(undefintr, _modbegin)	# 124
	VECTOR(undefintr, _modbegin)	# 125
	VECTOR(undefintr, _modbegin)	# 126
	VECTOR(undefintr, _modbegin)	# 127
	VECTOR(undefintr, _modbegin)	# 128
	VECTOR(undefintr, _modbegin)	# 129
	VECTOR(undefintr, _modbegin)	# 130
	VECTOR(undefintr, _modbegin)	# 131
	VECTOR(undefintr, _modbegin)	# 132
	VECTOR(undefintr, _modbegin)	# 133
	VECTOR(undefintr, _modbegin)	# 134
	VECTOR(undefintr, _modbegin)	# 135
	VECTOR(undefintr, _modbegin)	# 136
	VECTOR(undefintr, _modbegin)	# 137
	VECTOR(undefintr, _modbegin)	# 138
	VECTOR(undefintr, _modbegin)	# 139
	VECTOR(undefintr, _modbegin)	# 140
	VECTOR(undefintr, _modbegin)	# 141
	VECTOR(undefintr, _modbegin)	# 142
	VECTOR(undefintr, _modbegin)	# 143
	VECTOR(undefintr, _modbegin)	# 144
	VECTOR(undefintr, _modbegin)	# 145
	VECTOR(undefintr, _modbegin)	# 146
	VECTOR(undefintr, _modbegin)	# 147
	VECTOR(undefintr, _modbegin)	# 148
	VECTOR(undefintr, _modbegin)	# 149
	VECTOR(undefintr, _modbegin)	# 150
	VECTOR(undefintr, _modbegin)	# 151
	VECTOR(undefintr, _modbegin)	# 152
	VECTOR(undefintr, _modbegin)	# 153
	VECTOR(undefintr, _modbegin)	# 154
	VECTOR(undefintr, _modbegin)	# 155
	VECTOR(undefintr, _modbegin)	# 156
	VECTOR(undefintr, _modbegin)	# 157
	VECTOR(undefintr, _modbegin)	# 158
	VECTOR(undefintr, _modbegin)	# 159
	VECTOR(undefintr, _modbegin)	# 160
	VECTOR(undefintr, _modbegin)	# 161
	VECTOR(undefintr, _modbegin)	# 162
	VECTOR(undefintr, _modbegin)	# 163
	VECTOR(undefintr, _modbegin)	# 164
	VECTOR(undefintr, _modbegin)	# 165
	VECTOR(undefintr, _modbegin)	# 166
	VECTOR(undefintr, _modbegin)	# 167
	VECTOR(undefintr, _modbegin)	# 168
	VECTOR(undefintr, _modbegin)	# 169
	VECTOR(undefintr, _modbegin)	# 170
	VECTOR(undefintr, _modbegin)	# 171
	VECTOR(undefintr, _modbegin)	# 172
	VECTOR(undefintr, _modbegin)	# 173
	VECTOR(undefintr, _modbegin)	# 174
	VECTOR(undefintr, _modbegin)	# 175
	VECTOR(undefintr, _modbegin)	# 176
	VECTOR(undefintr, _modbegin)	# 177
	VECTOR(undefintr, _modbegin)	# 178
	VECTOR(undefintr, _modbegin)	# 179
	VECTOR(undefintr, _modbegin)	# 180
	VECTOR(undefintr, _modbegin)	# 181
	VECTOR(undefintr, _modbegin)	# 182
	VECTOR(undefintr, _modbegin)	# 183
	VECTOR(undefintr, _modbegin)	# 184
	VECTOR(undefintr, _modbegin)	# 185
	VECTOR(undefintr, _modbegin)	# 186
	VECTOR(undefintr, _modbegin)	# 187
	VECTOR(undefintr, _modbegin)	# 188
	VECTOR(undefintr, _modbegin)	# 189
	VECTOR(undefintr, _modbegin)	# 190
	VECTOR(undefintr, _modbegin)	# 191
	VECTOR(undefintr, _modbegin)	# 192
	VECTOR(undefintr, _modbegin)	# 193
	VECTOR(undefintr, _modbegin)	# 194
	VECTOR(undefintr, _modbegin)	# 195
	VECTOR(undefintr, _modbegin)	# 196
	VECTOR(undefintr, _modbegin)	# 197
	VECTOR(undefintr, _modbegin)	# 198
	VECTOR(undefintr, _modbegin)	# 199
	VECTOR(undefintr, _modbegin)	# 200
	VECTOR(undefintr, _modbegin)	# 201
	VECTOR(undefintr, _modbegin)	# 202
	VECTOR(undefintr, _modbegin)	# 203
	VECTOR(undefintr, _modbegin)	# 204
	VECTOR(undefintr, _modbegin)	# 205
	VECTOR(undefintr, _modbegin)	# 206
	VECTOR(undefintr, _modbegin)	# 207
	VECTOR(undefintr, _modbegin)	# 208
	VECTOR(undefintr, _modbegin)	# 209
	VECTOR(undefintr, _modbegin)	# 210
	VECTOR(undefintr, _modbegin)	# 211
	VECTOR(undefintr, _modbegin)	# 212
	VECTOR(undefintr, _modbegin)	# 213
	VECTOR(undefintr, _modbegin)	# 214
	VECTOR(undefintr, _modbegin)	# 215
	VECTOR(undefintr, _modbegin)	# 216
	VECTOR(undefintr, _modbegin)	# 217
	VECTOR(undefintr, _modbegin)	# 218
	VECTOR(undefintr, _modbegin)	# 219
	VECTOR(undefintr, _modbegin)	# 220
	VECTOR(undefintr, _modbegin)	# 221
	VECTOR(undefintr, _modbegin)	# 222
	VECTOR(undefintr, _modbegin)	# 223
	VECTOR(undefintr, _modbegin)	# 224
	VECTOR(undefintr, _modbegin)	# 225
	VECTOR(undefintr, _modbegin)	# 226
	VECTOR(undefintr, _modbegin)	# 227
	VECTOR(undefintr, _modbegin)	# 228
	VECTOR(undefintr, _modbegin)	# 229
	VECTOR(undefintr, _modbegin)	# 230
	VECTOR(undefintr, _modbegin)	# 231
	VECTOR(undefintr, _modbegin)	# 232
	VECTOR(undefintr, _modbegin)	# 233
	VECTOR(undefintr, _modbegin)	# 234
	VECTOR(undefintr, _modbegin)	# 235
	VECTOR(undefintr, _modbegin)	# 236
	VECTOR(undefintr, _modbegin)	# 237
	VECTOR(undefintr, _modbegin)	# 238
	VECTOR(undefintr, _modbegin)	# 239
	VECTOR(undefintr, _modbegin)	# 240
	VECTOR(undefintr, _modbegin)	# 241
	VECTOR(undefintr, _modbegin)	# 242
	VECTOR(undefintr, _modbegin)	# 243
	VECTOR(undefintr, _modbegin)	# 244
	VECTOR(undefintr, _modbegin)	# 245
	VECTOR(undefintr, _modbegin)	# 246
	VECTOR(undefintr, _modbegin)	# 247
	VECTOR(undefintr, _modbegin)	# 248
	VECTOR(undefintr, _modbegin)	# 249
	VECTOR(undefintr, _modbegin)	# 250
	VECTOR(undefintr, _modbegin)	# 251
	VECTOR(undefintr, _modbegin)	# 252
	VECTOR(undefintr, _modbegin)	# 253
	VECTOR(undefintr, _modbegin)	# 254
	VECTOR(undefintr, _modbegin)	# 255
#endif

		.text
/*2
.* trap: System trap handlers
 *
.* ARGUMENTS:
 *	The vector entry for a particular trap is branched to
 *	by the NS32032/NS32332, which identifies the trap type.
 *
.* USAGE:
 *	Each trap calls trap() with a stack with the
 *	following layout:

r0:  	FAULT TYPE
r1:  	CODE

  	PC  (jsr trap)	<---------- ssp
  	USP
  	R7
  	R6
  	R5
  	R4
  	R3
  	R2
  	R1
  	R0
  	FP <------------------- fp
  	PC  (trap)
  	PSR,MOD		! PSR is high order 16 bits

/*0	SVC traps call syscall() with a stack that has the above layout:
 */

/*
 *	Common ast check code for returning from traps.
 */
	.globl	rett_ast
rett_ast:
	tbitw	$PSR_U_BIT,10(fp)	# Don't process AST unless returning
			# 10 = fp(4) + trap-pc(4) + mod(2)
	bfc	rett_out_sys		/* to user mode */
	GETCPUID(r0)
	cmpqd	$0,_need_ast[r0:d]	/* Is an AST needed */
	beq	rett_out
	addqd	$1,_astcnt
	TRAP(T_AST,0)		/* trap(AST,0) */

	/*
	 *	Final return code.
	 */
rett_out:
#if	!STAT_TIME
	TIME_TRAP_UEXIT			/* sys return skips this macro */
	ENBINT				/* TIME_TRAP_UEXIT leaves ints off */
#endif
rett_out_sys:
#if	MMAX_IDEBUG
	jsr	@_rett_out_intr_checks
#endif
	RESTORE_FRAME			/* Restore the stack frame */
	.globl	_master_rett
	RETT(_master_rett)		/* Really return from trap */



/*
 * NMI - Non-maskable interrupt.
 */

	/*
	 *	nmi_panic is set to tell boot() that this panic was
	 *	caused by an nmi, and therefore disks should not be sync'ed.
	 */

	.globl	_nmi_panic
	.globl	nmi
	.globl	_panicnmiflag

nmi:
	/*
	 * NMIs have multiple uses. For this reason it is deemed
	 * better to handle them via a higher level routine - nmi_trap().
	 * This routine returns assorted values telling this code
	 * exactly what action to take...
	 */

	/*
	 *	NMIs aren't timed because they usually indicate hardware
	 *	disasters.
	 */

#if	MMAX_XPC
#define	XPC_NMI_DELAY	20000
	save	[r0]
	movd	$XPC_NMI_DELAY, r0	/* Give the SCC a chance to act...*/
nmi_delay_loop:
	acbd	$-1, r0, nmi_delay_loop
	restore	[r0]
#endif
	BUILD_FRAME			/* Create the stack frame */
	DISNMI(nmi.1, r0, r1)		/* Block any further NMIs */
#if	MMAX_XPC || MMAX_APC
	cbitb	$ICU_SYSNMI_BIT, @M_ICU_BASE+ISRV	/* clear SYSNMI int */
#endif
	GETCPUID(r1)
	cmpd	$0,@_panicnmiflag[r1:d]	/* First time thru here ? */
	bne	nmi01			/* No, don't save again   */

	/*
	 * Drain all interesting information from the CPU, MMU, and
	 * various board registers.  This information can be analyzed
	 * later by the debugger if it turns out the kernel has been
	 * booted by the interactive debugger or by the crash dump
	 * analyzer.  We anticipate that NMIs will be infrequent; if
	 * NMIs are used for profiling, this code should probably be
	 * moved elsewhere.
	 *
	 * Step 1:  integer cpu state.
	 */
	muld	$PSA_STRUCT_SIZE,r1	/* Index into Panic Save Area */
	addr	_psa,r2			/* Get address of Save Area */
	addd	r1,r2			/* Find adr of PSA for this cpu */
	movd	r2,r3			/* Save adr of PSA for this cpu */
	sprd	sp,PSA_SSP(r2)		/* Save system stack pointer */
	addd	$PSA_USP, r2		/* offset into psa for integer regs */
	addr	0(sp),r1		/* Source adr */
					/* already saved sp, so -1 */
	movd	$(PSA_STKFRM_SIZE/4), r0
	movsd				/* usp, r7-r0, fp, pc, psr */
	sprd	sp, PSA_SSP(r3)		/* Save system stack pointer */
	# save sb value here
	sprd	intbase, PSA_INTBASE(r3)
#if	MMAX_XPC
	sprd	cfg, PSA_CFG(r3)	/* can't get this on 032 and 332 */
#endif

	/*
	 * Step 2:  mmu state.
	 */
	smr	ptb0, PSA_PTB0(r3)
	smr	ptb1, PSA_PTB1(r3)
#if	MMAX_DPC
	smr	msr, PSA_MSR(r3)
	smr	eia, PSA_EIA(r3)
	smr	bpr0, PSA_BPR0(r3)
	smr	bpr1, PSA_BPR1(r3)
	smr	bcnt, PSA_BCNT(r3)
#endif
#if	MMAX_APC
	smr	few, PSA_FEW(r3)
	smr	asr, PSA_ASR(r3)
	smr	tear, PSA_TEAR(r3)
	smr	bear, PSA_BEAR(r3)
	smr	bar, PSA_BAR(r3)
	smr	bmr, PSA_BMR(r3)
	smr	bdr, PSA_BDR(r3)
#endif
#if	MMAX_XPC
	smr	mcr, PSA_MCR(r3)
	smr	msr, PSA_MSR(r3)
	smr	tear, PSA_TEAR(r3)
	sprd	bpc, PSA_BPC(r3)
	sprd	car, PSA_CAR(r3)
	sprd	dcr, PSA_DCR(r3)
	sprd	dsr, PSA_DSR(r3)	
#endif

	/*
	 * Step 3:  fpu state.
	 */
	sfsr	r0
	movd	r0, PSA_FSR(r3)
#if	MMAX_XPC
	movl	f0, PSA_F0(r3)
	movl	f1, PSA_F1(r3)
	movl	f2, PSA_F2(r3)
	movl	f3, PSA_F3(r3)
	movl	f4, PSA_F4(r3)
	movl	f5, PSA_F5(r3)
	movl	f6, PSA_F6(r3)
	movl	f7, PSA_F7(r3)
#endif
#if	MMAX_APC || MMAX_DPC
	movf	f0, PSA_F0(r3)
	movf	f1, PSA_F1(r3)
	movf	f2, PSA_F2(r3)
	movf	f3, PSA_F3(r3)
	movf	f4, PSA_F4(r3)
	movf	f5, PSA_F5(r3)
	movf	f6, PSA_F6(r3)
	movf	f7, PSA_F7(r3)
#endif

#if	MMAX_XPC || MMAX_APC
	/*
	 * Step 4:  interrupt controller state.
	 */
        movb    @(M_ICU_BASE+IPND), PSA_ICU_IPND(r3)	/* master icu */
        movb    @(M_ICU_BASE+IPND+RBIAS), PSA_ICU_IPND+1(r3)
	movb	@(M_ICU_BASE+IMASK), PSA_ICU_IMSK(r3)
	movb	@(M_ICU_BASE+IMASK+RBIAS), PSA_ICU_IMSK+1(r3)
        movb    @(M_ICU_BASE+ISRV), PSA_ICU_ISRV(r3)
        movb    @(M_ICU_BASE+ISRV+RBIAS), PSA_ICU_ISRV+1(r3)
#if	MMAX_APC
	movb	@(S_ICU_BASE+IPND), PSA_SICU_IPND(r3)	/* slave icu */
	movb	@(S_ICU_BASE+IPND+RBIAS), PSA_SICU_IPND+1(r3)
	movb	@(S_ICU_BASE+IMASK), PSA_SICU_IMSK(r3)
	movb	@(S_ICU_BASE+IMASK+RBIAS), PSA_SICU_IMSK+1(r3)
	movb	@(S_ICU_BASE+ISRV), PSA_SICU_ISRV(r3)
	movb	@(S_ICU_BASE+ISRV+RBIAS), PSA_SICU_ISRV+1(r3)
#endif
#endif	/* MMAX_XPC || MMAX_APC */

#if	MMAX_XPC || MMAX_APC
	/*
	 * Step 5:  record pending vectors.
	 */
					# drain vectorbus fifo
	addr	PSA_VBFIFO(r3), r2	# address of fifo save area
	movd	$PSA_VBMAX, r1		# maximum number of vectors to save
nmi_drain_vbf:
	movqb	$-1, 0(r2)		# initialize saved vector to -1
#if	MMAX_XPC
	tbitd	$(XPCVB_STAT_FIFO_BIT), @XPCVB_STATUS	# vector present?
#endif
#if	MMAX_APC
	tbitd	$(APCCSR_VB_FIFO_BIT), @APCREG_CSR	# vector present?
#endif
	bfc	nmi_vbf_empty		# no, try again
	movb	@VB_FIFO, 0(r2)		# yes, fetch and save
nmi_vbf_empty:
	addqd	$1, r2			# try for another vector
	acbd	$-1, r1, nmi_drain_vbf	# until we run out of save area
#endif	/* MMAX_XPC || MMAX_APC */

	/*
	 * Step 6:  miscellaneous.
	 */
	movd	@CPU_REG_CSR, PSA_CPU_REG_CSR(r3)	# Might be useful
#if	MMAX_XPC || MMAX_APC
	movd	@CPU_ERR_CSR, PSA_CPU_REG_ERR(r3)	# Might be useful
#endif
	movd	$PSA_PANIC_VERSION, PSA_VERSION(r3)	# Version for mda
	movd	$PSA_STRUCT_SIZE, PSA_SIZE(r3)		# Struct size for mda
	movqd	$PSA_BOARD_TYPE, PSA_CPUTYPE(r3)	# XPC/APC/DPC
	movd	@FRCOUNTER, PSA_TIMESTAMP(r3)		# stamp cpu check-in

nmi01:
	addqd	$1,_nmicnt
	jsr	@_nmi_trap
	ENBNMI(nmi.4, r0, r1)		/* Enable new NMIs */

	/*
	 * First possibility - merely resume.
	 */
	cmpqd	$NMI_RESUME,r0		/* Are we to merely resume? */
	bne	nmi.chk1		/* No... Go to next check */
	TRACK()
	br	rett_ast		/* Resume */

	/*
	 * Second possibility - panic the CPU.
	 */
	.globl nmipanic_msg		/* mda looks for this symbol */

nmi.chk1:
	cmpqd	$NMI_PANIC,r0		/* Are we to panic the CPU? */
	bne	nmi.chk2		/* No... Go to the next check */
	RESTORE_FRAME			/* Restore the stack frame */
	movqd	$1,_nmi_panic		/* Don't try to sync disks */
	PANIC(@nmipanic_msg,$0,$0)	/* panic(nmi message) */
nmipanic_msg:
	.asciz	"Fatal NMI Received from Console\n"

	/*
	 * Third possibility - dump the registers and halt the CPU
	 */
nmi.chk2:
	cmpd	$NMI_WAIT,r0		/* Are we to make the CPU halt? */
	bne	nmi.chk3		/* No... Go to the next check */
	RESTORE_FRAME			/* Restore the stack frame */
	jsr	@_halt_cpu		/* NEVER COME BACK */
					
	/*
	 * Last possibility - go to dbmon.
	 *
	 */
nmi.chk3:
	RESTORE_FRAME				/* Restore the stack frame */
	tbitd	$(RB_B_DEBUG), _boothowto	/* Debugger present ?      */
	bfc	nmi.chk4			/* If fc, no               */
	jump	@dnmi

nmi.chk4:
	movqd	$1,_nmi_panic			/* Don't try to sync disks */
	PANIC(@nodbg_msg,$0,$0)			/* panic(no dbg message)   */
nodbg_msg:
	.asciz	"Fatal NMI - No Debugger"


/*
 * ABT - Memory management fault.
 */

#if	TRACE_XPC_MMU_BUG
	.data
	.globl	_last_abt_code
	.globl	_last_abt_msr
_last_abt_code:	.int	0
_last_abt_msr:	.int	0
_sawit:		.asciz	"abt:  tear 0x%x msr 0x%x\n"
	.text
#endif
	.globl	fpapresent		/* set in loinit_ns32k.s */
	.globl	abt
abt:
	BUILD_FRAME
	ENTER_DISINT_TRAP
#if	!STAT_TIME
	TIME_TRAP_ENTRY_I(abt_time)
#endif
	IDBG(r0, ID_ABT, 40(sp))

#if	MMAX_XPC
	smr	msr, r0
	smr	tear, r1
#endif
#if	MMAX_APC
	cmpqd	$0, fpapresent
	beq	no_cone
 #
 # Cone must be idle before accessing the MMU during an ABT.
 #  We must wait for > 500 CPU cycles (total).  If !STAT_TIME
 #  is present, the loop can be a little shorter.
 #
 # 145 cycles for entry + 45/loop * 8 loops => 505 cycles.
 # 145 cycles for entry + 45/loop * 6 loops => 415 cycles.
 #
 #	r0 = 8 (or 6); do { } while((CSR & (32081|BUSY)) == BUSY && --r0);
 #
#if	STAT_TIME
#define	CONE_WAIT_LOOPS	8
#else
#define	CONE_WAIT_LOOPS	6
#endif
	movd	$CONE_WAIT_LOOPS, r0	# Loop iteration count
wait_for_cone:
	movd	@APCREG_CSR, r1		# Get CSR
	andd	$(APCCSR_32081_PRES+APCCSR_CONE_BUSY),r1
					# Mask 32081 present & cone busy bits
	cmpd	$(APCCSR_CONE_BUSY),r1	# Are we busy?
	bne	cone_is_done		#  branch if not
	acbd	$-1,r0,wait_for_cone	# Decrement iteration count till done
no_cone:
cone_is_done:
	smr	asr, r0			# Read the
	smr	tear, r1		#   mmu registers
#endif
#if	MMAX_DPC
	smr	eia, r1			# Read the
	smr	msr, r0			#   mmu registers
#endif
#if	TRACE_XPC_MMU_BUG
	enter	[r1,r2]
	enter	[r0]		# make this guy easily accessable to printf
	andd	$0xf0, r0
	cmpd	$0xb0, r0
	bne	noproblem
	addr	@_sawit, r0
	jsr	@_printf	# r0=format, r1=tear, tos=msr
noproblem:
	exit	[r0]
	exit	[r1,r2]
#endif
	addqd	$1,_abtcnt
	ENBINT
#if	MMAX_IDEBUG
	cmpd	$0, r3
	beq	abt_check1
	PRINTF(@abt_ints_off, $0)
abt_check1:
	jsr	@_icu_bits_clear
#endif
	/*
	 * Call trap() with correct parameters. If the call returns
	 * that means the abort trap was handled correctly, and we
	 * can clear a count used to track how many bad faults occured
	 * for MMU bugs.
	 */
#if	MMAX_XPC || MMAX_APC
	tbitd	$(MMU_DDI_BIT), r0	/* Check read or write */
	bfs	abt.write		/* if set, it's a write */
	addr	T_ABT_READ, r2		/* it's a read */
	br	abt.got.cod
abt.write:
	addr	T_ABT_WRITE, r2		/* write */
abt.got.cod:
#if	TRACE_XPC_MMU_BUG
	movd	r0, _last_abt_msr
#endif
	tbitd	$(MMU_USR_BIT), r0	/* User mode? */
	bfc	abt_2			/* no... */
	sbitd	$(TFV_USER_BIT),r2	/* yes, flag for trap()	  */
abt_2:
	andd	$(CPU_STATUS_MASK), r0	/* keep cpu status bits */
	lshd	$16,r0			/* Position in TF_STATUS  */
	ord	r0,r2			/* Add cpu status	  */
	movd	r2,r0			/* copy code and u/s flag */
#if	TRACE_XPC_MMU_BUG
	movd	r0, _last_abt_code
#endif
	jsr	_trap			/* go handle write        */
	br	abt.8
#endif	/* MMAX_XPC || MMAX_APC */
#if	MMAX_DPC
	andd	$MSR_READERR,r0		/* read or write attempt */
	cmpqd	$0,r0
	beq	abt_write
	addr	T_ABT_READ,r0
	jsr	_trap
	br	abt.8
abt_write:
	addr	T_ABT_WRITE,r0
	jsr	_trap
#endif
abt.8:
	/*
	 * Go back to the user.
	 */
	EXIT_DISINT_TRAP(abt.out)
	TRACK()
	br	rett_ast			/* Return to invoker */

/*
 * FPU - Floating point fault.
 */
	.globl fpu
fpu:
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_UENTRY		/* kernel does not use fpu */
#endif
	IDBG(r0, ID_FPU, 40(sp))
	addqd	$1,_fpucnt

	/*
	 * Get the FPU status register.
	 */
	sfsr	r0			/* Get FPU status register */

	/*
	 * Check the trap type and set the appropriate exception subcode.
	 */
	andd	$FSR_TT,r0		/* Mask trap type */

f_case:
	caseb	f_table[r0:b]		/* Check trap code */

f_table:
	.byte	f_none-f_case,f_und-f_case,f_ovf-f_case,f_dvz-f_case
	.byte	f_ill-f_case,f_inv-f_case,f_inex-f_case,f_oprnd-f_case

f_none:
	br	fpureturn			/* ignore */
f_und:
	addr	@EXC_NS32K_FPU_UNDERFLOW,r1	/* Underflow */
	br	fputrap
f_ovf:
	addr	@EXC_NS32K_FPU_OVERFLOW,r1	/* Overflow */
	br	fputrap
f_dvz:
	addr	@EXC_NS32K_FPU_DVZ,r1		/* Divide-by-zero */
	br	fputrap
f_inv:
	addr	@EXC_NS32K_FPU_INVALID,r1	/* Invalid operation */
	br	fputrap
f_ill:
	addr	@EXC_NS32K_FPU_ILLEGAL,r1	/* Illegal instuction */
	br	fputrap
f_inex:
	addr	@EXC_NS32K_FPU_INEXACT,r1	/* Inexact result */
	br	fputrap
f_oprnd:
	addr	@EXC_NS32K_FPU_OPERAND,r1	/* Operand error  */

fputrap:
	addr	T_FPU,r0		/* trap(FPU,code) */
	jsr	@_trap			/* process the trap */

fpureturn:
	TRACK()
	br	rett_ast		/* Return to invoker */


/*
 * ILL - Illegal instruction
 */
	.globl ill
ill:
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_ENTRY(ill_time)
#endif
	addqd	$1,_illcnt
	TRAP(T_ILL,0)				/* trap(ILL,0) */
	TRACK()
	br	rett_ast			/* Return to invoker */


/*
 * SVC - Supervisor call
 */

	.globl	svc
	.globl	_nsysent
	.bss	_bogus_syscalls,4,4
	.text

svc:
	BUILD_FRAME			/* Create the stack frame */
#if	MMAX_IDEBUG
	jsr	_ckints
#endif

#if	!STAT_TIME
	TIME_TRAP_UENTRY		/* kernel does not make syscalls */
#endif
	addqd	$1,_svccnt
	save	[r0,r1]
	movd	-4(fp),r0
	cmpqd	r0, _nsysent
	bge	bogus_call
	movd	_mach_trap_count,r1
	negd	r1,r1
	cmpqd	r0, r1
	ble	bogus_call
	br	get_on_with_it
bogus_call:
	addqd	$1,_bogus_syscalls
get_on_with_it:
	restore	[r0,r1]
	cmpd	-4(fp),$-9		/* if code (in r0) is < -9 */
	blt	_alt_sys_call		/*   syscall uses ACALL handler */

	jsr	@_syscall		/* syscall() */
	TRACK()
	br	rett_ast			/* Return to invoker */

/* Following code is the ACALL handler extensively rewritten
	from VAX assembler.  */

	.globl	_mach_trap_table
	.globl	_mach_trap_count

/*
 *	MULTIMAX register usage: r0 - code, r1 - first arg, r2 - second arg,
 *	user sp - pointer to ret addr and entire arg list.
 */

/*
 * 	Macros for finding things in svc frame.
 */
#define	USER_PC 4(fp)
#define	USER_R0	-4(fp)
#define USER_R1	-8(fp)
#define USER_R2	-12(fp)
#define USER_SP	-36(fp)

/* Macro definitions of register usage here. */
#define rOLD_SP	r7		/* r7 saves sp before pushing args */
#define	rARGP	r6		/* r6 points to list of 3rd ... args */
#define rBCNT	r5		/* r5 holds length of this list in bytes */
#define rCPU	r4		/* r4 holds cpu number */
#define rTHREAD	r3		/* r3 points to current thread structure */

/* r0, r1, and r2 are scratch */

_alt_sys_call:	
	addqd	$1,USER_PC		# advance pc over svc instr.
	GETCPUID(rCPU)				# Get cpu number
	movd	_active_threads[rCPU:d],rTHREAD	# and thread pointer
	negd	USER_R0,r0		# get code and make positive
	cmpd	_mach_trap_count,r0	# check if in range
	bgt	acall.ok		#   branch if ok
	addr	KERN_FAILURE,r0		# Out of range !
	br	acall.ret

/*
 *	Bring up descriptor of N'th entry (N in r0).
 */

acall.ok:
	addr	_mach_trap_table[r0:q],r0	# address of entry
	movd	4(r0),r2		# Get routine address.
	movzwd	0(r0),rBCNT		# length in bytes + 4(for arg count)
	cmpw	$12,rBCNT		# Any args on stack
					# 12 = sizeof(arg count plus 2 args)
	bge	acall.regs		# branch if no

/*
 *	Call has more than two arguments; these must be copied in to the
 *	stack.  NOTE: The byte count is too big by 12 because the arg
 *	count doesn't exist on this machine and the first 2 args are in
 *	registers. Similarly, the first 3 locations on the user stack
 *	don't get copied in because they are a return pc and 2 args.
 *	Hence adding rBCNT to the sp makes it point just beyond the last arg.
 */	

	movd	USER_SP,rARGP		# Copy user stack pointer
	addd	rBCNT,rARGP		# point to after last arg.
	subd	$12,rBCNT		# subtract stuff not to be copied
	sprd	sp,rOLD_SP		# save sp in case access to args fails
	addr	acall.fail,THREAD_RECOVER(rTHREAD)	# Set recover field
acall.args:
	addqd	$-4,rARGP		# back up one arg
	adjspb	$4			# allocate space
	movusd	0(rARGP),0(sp)		# copy one argument
	acbd	$-4,rBCNT,acall.args	# Continue if not done
	movqd	$0,THREAD_RECOVER(rTHREAD)	# Done, clear recover field

	movd	USER_R2,r1		# get last two
	movd	USER_R1,r0		#   arguments
	jsr	0(r2)			# call routine (finally !)
	lprd	sp,rOLD_SP		# clear extra arguments.
	br	acall.ret

	/* Access to extra args failed */
acall.fail:
	movqd	$0,THREAD_RECOVER(rTHREAD)	# Clear recover field
	addr	KERN_INVALID_ADDRESS,r0	# report access failure
	lprd	sp,rOLD_SP		# clear extra arguments.
	br acall.ret

	/* No extra args */
acall.regs:
	movd	USER_R2,r1		# get last two
	movd	USER_R1,r0		#   arguments
	jsr	0(r2)			# call routine (finally !)

acall.ret:
	movd	r0,USER_R0		# pass return code back to user.
	movd	THREAD_UTASK(rTHREAD),r2	# get proc
	movd	U_PROCP(r2),r2			#   pointer
	movzbd	P_CURSIG(r2),r0 	# check
	ord	P_SIG(r2),r0		#   if a signal
	cmpqd	$0,r0			#     is pending.
	beq	rett_ast
	GETCPUID(rCPU)			# Get cpu number - may have changed.
	movqd	$1,_need_ast[rCPU:d] 	# request ast to handle signal
	br	rett_ast

/*
 * DVZ - Divide by zero
 */
	.globl	dvz
dvz:
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_ENTRY(dvz_time)
#endif
	addqd	$1,_dvzcnt
	TRAP(T_DVZ,0)				/* trap(ARITHTRAP,INTDIV) */
	TRACK()
	br	rett_ast			/* Return to invoker */


/*
 * FLG - Flag instruction
 */
	.globl	flg
flg:
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_UENTRY		/* Kernel never does flag instr. */
#endif
	addqd	$1,_flgcnt
	TRAP(T_FLG,0)			/* trap(FLGTRAP,0) */
	TRACK()
	br	rett_ast		/* Return to invoker */


/*
 * BPT - Breakpoint instruction.
 */
	.globl	bpt
bpt:
	/*
	 * If we're debugging the kernel and the breakpoint was in system mode,
	 * we transfer directly to the rom code. Otherwise, we a do
	 * default trap processing.
	 */
	addqd	$1,_bptcnt
	tbitb	$PSR_U_BIT, 6(sp)	/* Are we in user mode? */
	bfs	bptdef			/* If so, do normal BPT */
#if	MMAX_DPC
	DISINT				/* Disable interrupts */
#endif
	jump	@dbpt			/* Go directly to dbmon breakpoint */
bptdef:
	/*
	 * Default breakpoint handling. Treat it like any other trap.
	 */
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_UENTRY		/* sys trap goes directly to dbmon */
#endif
	/* Note:  interrupts must be on at this point */
	TRAP(T_BPT,0)			/* trap(BPT) */
	TRACK()
	br	rett_ast		/* Return to invoker */


/*
 * TRC - Trace trap
 */
	.globl	trc
trc:
	/*
	 * If we're debugging the kernel and the trace trap was in system mode,
	 * we transfer directly to the rom code. Otherwise, do default trap
	 * processing.
	 */
	addqd	$1,_trccnt
	tbitb	$PSR_U_BIT, 6(sp)	/* Are we in user mode? */
	bfs	trcdef			/* If so, do normal TRC */
#if	MMAX_DPC
	DISINT				/* Disable interrupts */
#endif
	jump	@dtrc
trcdef:
	/*
	 * Default trace processing. Treat it like any other trap.
	 */
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_UENTRY		/* sys trap goes directly to dbmon */
#endif
	TRAP(T_TRC,0)			/* trap(TRC,0) */
	TRACK()
	br	rett_ast		/* Return to invoker */


/*
 * UND - Undefined operation
 */
	.globl	und
und:
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_ENTRY(und_time)
#endif
	addqd	$1,_undcnt
	TRAP(T_UND,0)			/* trap(UND,0) */
	TRACK()
	br	rett_ast		/* Return to invoker */


/*
 * RES - Reserved trap - something is terribly wrong
 */

/*	Don't time machine disasters. */


resmsg:	.asciz	"Reserved trap taken at pc = 0x%x psr = 0x%x.\n"

	.globl	res
res:
	BUILD_FRAME			/* Create the stack frame */
	movzwd	TRAP_PSR(fp), r0	/* PSR + MOD */
	PANIC(@resmsg, TRAP_PC(fp), r0)
	TRACK()
	br	rett_ast		/* "should never get here" */


#if	MMAX_XPC
/*
 * "Recoverable" bus error.  On the XPC, this really means broken hardware,
 * so it's time to say goodbye...
 */
	.globl	rbe
rbe:
	BUILD_FRAME
	bispsrw	$PSR_I			/* rbe turned off interrupts */
	TRAP(T_RBE,0)			/* trap(RBEFLT,istkp) */
	TRACK()
	br	rett_ast

	.globl	nbe			/* non-recoverable bus error */
nbe:
	BUILD_FRAME
	bispsrw	$PSR_I
	TRAP(T_NBE,0)			/* system will panic */
	TRACK()
	br	rett_ast

	.globl	ovf			/* integer overflow */
ovf:
	BUILD_FRAME
	IDBG(r0, ID_OVF, 40(sp))
	addqd	$1, _ovfcnt
	TRAP(T_OVF,0)			/* task/thread will be notified */
	TRACK()
	br	rett_ast

	.globl	dbg			/* debug condition occurred */
dbg:
	BUILD_FRAME
	IDBG(r0, ID_DBG, 40(sp))
	addqd	$1, _dbgcnt
	TRAP(T_DBG,0)
	TRACK()
	br	rett_ast
#endif

#if	MMAX_APC
	.globl	ber
ber:
	BUILD_FRAME			/* Create the stack frame */
#if	!STAT_TIME
	TIME_TRAP_ENTRY(ber_time)
#endif
	bispsrw	$PSR_I			/* ber turned them off */
	IDBG(r0, ID_BER, 40(sp))
	addqd	$1,_bercnt		/* Increment statistics */
	TRAP(T_BUSERR,0)		/* trap(BERFLT,istkp) */
	TRACK()
	br	rett_ast		/* Return to invoker */
#endif



#if	MMAX_XPC || MMAX_APC
#if	MMAX_XPC
#define	BOGUS_BIT	ICU_BOGUS_BIT
#endif
#if	MMAX_APC
#define	BOGUS_BIT	ICU_BOGUS_M_BIT
#endif
/*
 * Bogus interrupt caused by ICU race conditions
 */
	.globl	bogus_m
bogus_m:
	BUILD_FRAME			/* Create the stack frame */
	ENTER_INTERRUPT_RACE(BOGUS_BIT,bog.1) /* Fix interrupt state */
	IDBG(r0, ID_BOGUS_M, 40(sp))
	addqd	$1,_bogcnt		/* Increment statistics */
	EXIT_INTERRUPT_RACE(bog.9)	/* Fix up interrupt state */
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(bogm_out)			/* Return to invoker */
#endif	/* MMAX_XPC || MMAX_APC */


#if	MMAX_APC
	.globl	bogus_s
bogus_s:
	BUILD_FRAME			/* Create the stack frame */
	ENTER_INTERRUPT_NORMAL(bogs.1)	/* Fix up interrupt state */
	IDBG(r0, ID_BOGUS_S, 40(sp))
	addqd	$1,_bogcnt		/* Increment statistics */
	EXIT_INTERRUPT_NORMAL(bog.10)	/* Fix up interrupt state */
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(bogs_out)			/* Return to invoker */
#endif


#if	MMAX_APC
/* DUART - Duart interrupt (unexpected)
 *
 *  Note: There's no case for this in trap.c, it falls into the
 *	  default case processing (i.e., panic).
 */
	.globl	duart
duart:
	BUILD_FRAME			/* Create the stack frame */
	ENTER_INTERRUPT_NORMAL(duart.1)	/* Fix up interrupt state */
	IDBG(r0, ID_DUART, 40(sp))
	TRAP(T_DUARTFLT,0)		/* trap(DUARTFLT,istkp) */
	EXIT_INTERRUPT_NORMAL(duart.5)	/* Fix up interrupt state */
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(duart_out)			/* Return to invoker */
#endif


#if	MMAX_APC
/*
 * ILLEGAL_CASCADE - Illegal cascade interrupt
 */
	.globl	illegal_cascade
illegal_cascade:
	BUILD_FRAME			/* Create the stack frame */
	ENTER_INTERRUPT_NORMAL(ilc.1)	/* Fix up interrupt state */
	TRAP(T_CASCADE,0)		/* trap(ILLCASCFLT,istkp) */
	EXIT_INTERRUPT_NORMAL(ilc.5)	/* Fix up interrupt state */
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(ill_casc_out)		/* Return to invoker */
#endif


#if	MMAX_XPC || MMAX_APC
/*
 * TSE - Time Slice End Interrupt handler
 */

#if	MMAX_XPC
#define	ICU_TSE_BIT	ICU_TIMER_H_BIT
#endif
#if	MMAX_APC
#define	ICU_TSE_BIT	ICU_TIMER_M_BIT
#endif

	.globl	tse
tse:					/* Looks a lot like intentry */
	BUILD_FRAME			/* Create the stack frame */
	ENTER_INTERRUPT_RACE(ICU_TSE_BIT,tse.1) /* Fix interrupt state */

#if	!STAT_TIME
	TIME_INT_ENTRY(_kernel_timer)
#endif
	IDBG(r0, ID_TSE, 44(sp))
	/*
	 * Execute the interrupt service routine.
	 */
	addqd	$1,_tsecnt
	movqd	$0, r0			/* Pass a nil ihandler_t ptr */
	sprd	sp, r1			/* Pass the save frame */
	jsr	@_tseintr		/* Jump to interrupt service routine */


#if	!STAT_TIME
	TIME_INT_EXIT
#endif

	tbitw	$PSR_U_BIT,10(fp)	/* Don't process AST unless */
	bfc	tse_out			/* returning to user mode */
	GETCPUID(r0)
	cmpqd	$0,_need_ast[r0:d]	/* Is an AST needed */
	bne	tse_ast			/* if yes, go do it */
tse_out:
	EXIT_INTERRUPT_RACE(tseout1)
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(tse_reti)			/* Really return from interrupt */

	/*
	 * To do an AST after an interrupt, we must first do a
	 * reti instruction to get the hardware to re-enable interrupts.
	 */

tse_ast:
#if	MMAX_XPC || MMAX_APC
#if	MMAX_IDEBUG
	cmpd	$0, r6			/* shouldn't be a NORMAL bit race */
	beq	tse_ast_pass1		/* from user mode */
	PANIC(@tse_ast_race_bogon, $0, $0)
tse_ast_pass1:
#endif
#if	0
	EXIT_INTERRUPT_RACE(tse_ast.1)
#endif
#if	MMAX_IDEBUG
	cmpqb	$0, @M_ICU_BASE+ISRV
	beq	tse_ast_pass2
tse_ast_crash:
	movb	@M_ICU_BASE+ISRV, @tse_ast_bogon
	movb	@M_ICU_BASE+ISRV+RBIAS, @tse_ast_bogon+1
	PRINTF(@tse_ast_bogon_mes, @tse_ast_bogon)
	PANIC(@tse_ast_bogon_panic, $0, $0)
tse_ast_pass2:
	cmpb	$(ICU_NORMAL > 8), @M_ICU_BASE+ISRV+RBIAS
	bne	tse_ast_crash
#endif

	movd	_OSmodpsr,tos		/* push valid mod/psr pair with
						interrupts enabled */
	addr	do_tse_ast,tos		/* push return address  */
	RETI(tseout_ast)		/* ret from int to next instruction */
do_tse_ast:
#endif	/* MMAX_XPC || MMAX_APC */
#if	MMAX_DPC
	movd	_OSmodpsr,tos		/* push valid mod/psr pair with
						interrupts enabled */
	addr	do_tse_ast,tos		/* push return address  */
	RETI(tseout2)			/* return from interrupt to
						next instruction */
do_tse_ast:
#endif
#if	!STAT_TIME
	TIME_TRAP_UENTRY		/* AST must be from user mode */
#endif
	addqd	$1,_astcnt
	TRAP(T_AST,0)			/* trap(AST,istkp) */
#if	MMAX_IDEBUG
	GETCPUID(r0)
	movd	r6,_last_tse_ast_r6[r0:b]
	movb	@M_ICU_BASE+ISRV,_last_tse_ast_isrv[r0:b]
	movb	@M_ICU_BASE+ISRV+RBIAS,_last_tse_ast_rbias[r0:b]
#endif
	TRACK()
	br 	rett_ast		/* back to user */
#endif	/* MMAX_XPC || MMAX_APC */


#if	MMAX_XPC || MMAX_APC
/*
 * Slave Timer Interrupt
 *
 * The APC has a slave ICU whose timer is used for kernel profiling.  However,
 * Mach doesn't currently understand kernel profiling.
 *
 * The XPC's ICU runs with both timers concatenated for time-slice-end and
 * there is no slave ICU.  The "ICU_TIMER_L" will vector here but should
 * never happen.  WARNING:  if this timer is needed, the entry code must
 * be fixed up to recognize races with setting the NONFATAL bit.
 */
	.globl	timer_s
timer_s:
	BUILD_FRAME                     /* Create the stack frame */
	ENTER_INTERRUPT_NORMAL(sti.1)	/* Fix up interrupt state */
	IDBG(r0, ID_TIMER_S, 40(sp))
	addqd	$1,_sticnt		/* Increment statistics */
	PANIC(@timer_s_msg,$0,$0)
	EXIT_INTERRUPT_NORMAL(sti.5)	/* Fix up interrupt state */
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(tmr_s_out)			/* Return to invoker */

timer_s_msg:
#if	MMAX_XPC
	.asciz	"Unexpected low timer interrupt."
#endif
#if	MMAX_APC
	.asciz	"Unexpected slave timer / kernel profiling timer interrupt."
#endif
#endif	/* MMAX_XPC || MMAX_APC */


#if	MMAX_XPC || MMAX_APC
	.globl	powerfail
powerfail:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_POWERFAIL_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */
#endif

#if	MMAX_XPC
hard_nbi:
	BUILD_FRAME
	addr	@ICU_HARD_NBI_BIT, r7
	br	unexpected
destsel_par:
	BUILD_FRAME
	addr	@ICU_DESTSEL_PAR_BIT, r7
	br	unexpected
soft_nbi:
	BUILD_FRAME
	# No race here:
	# If this intr comes in after setting ISRV[icu_nonfatal_bit]
	# reti will clear that bit, this one stays set, all is well.
	addr	@ICU_SOFT_NBI_BIT, r7
	br	unexpected
rcv_addr_par:
	BUILD_FRAME
	addr	@ICU_RCV_ADDR_PAR_BIT, r7
	br	unexpected
btag_tag_par:
	BUILD_FRAME
	addr	@ICU_BTAG_TAG_PAR_BIT, r7
	br	unexpected
btag_state:
	BUILD_FRAME
	addr	@ICU_BTAG_BIT, r7
	br	unexpected
btag_cache:
	BUILD_FRAME
	addr	@ICU_BTAG_CACHE_BIT, r7
	br	unexpected
vb_err:
	BUILD_FRAME
	addr	@ICU_VB_ERR_BIT, r7
	br	unexpected
cache_par:
	BUILD_FRAME
	addr	@ICU_CACHE_PAR_BIT, r7
	br	unexpected
#endif

#if	MMAX_APC
hard_nbi0:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_HARD_NBI0_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

hard_nbi1:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_HARD_NBI1_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

hard_nbi2:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_HARD_NBI2_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

hard_nbi3:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_HARD_NBI3_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

destsel_par:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_DESTSEL_PAR_BIT,r7 /* Failure type */
        br      unexpected              /* Common code */

soft_nbi0:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_SOFT_NBI0_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

soft_nbi1:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_SOFT_NBI1_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

soft_nbi2:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_SOFT_NBI2_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

rcv_addr_par4:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_RCV_ADDR_PAR4_BIT,r7 /* Failure type */
        br      unexpected              /* Common code */

rcv_addr_par0:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_RCV_ADDR_PAR0_BIT,r7 /* Failure type */
        br      unexpected              /* Common code */

rcv_addr_par1:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_RCV_ADDR_PAR1_BIT,r7 /* Failure type */
        br      unexpected              /* Common code */

rcv_addr_par2:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_RCV_ADDR_PAR2_BIT,r7 /* Failure type */
        br      unexpected              /* Common code */

rcv_addr_par3:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_RCV_ADDR_PAR3_BIT,r7 /* Failure type */
        br      unexpected              /* Common code */

btag_par0:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_BTAG_PAR0_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

btag_par1:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_BTAG_PAR1_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

vb_nak:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_VB_NAK_BIT,r7      /* Failure type */
        br      unexpected              /* Common code */

vb_par:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_VB_PAR_BIT,r7      /* Failure type */
        br      unexpected              /* Common code */

vb_out_of_sync:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_VB_OUT_OF_SYNC_BIT,r7 /* Failure type */
        br      unexpected              /* Common code */

cache_par:
        BUILD_FRAME                     /* Create the stack frame */
        addr    @ICU_CACHE_PAR_BIT,r7   /* Failure type */
        br      unexpected              /* Common code */

#endif

#if	MMAX_XPC || MMAX_APC
unexpected:                             /* Common code for unexpected stuff */
        ENTER_INTERRUPT_NORMAL(unx_1)	/* Fix up interrupt state */
        addr    0(sp),r1                /* hw_isr(type,istkp) */
        movd    r7,r0
        jsr     @_hw_isr
        EXIT_INTERRUPT_NORMAL(unx_5)	/* Fix up interrupt state */
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(unex_out)			/* Return to invoker */
#endif


#if	MMAX_XPC || MMAX_APC
/*
 * UNDEFINTR - Undefined Interrupt Handler
 */
	.globl	undefintr
undefintr:
	BUILD_FRAME			/* Create the stack frame */
	ENTER_INTERRUPT_NORMAL(udf.5)	/* Fix up interrupt state */
	TRAP(T_UNDEFLT,0)		/* handle in trap.c       */
	EXIT_INTERRUPT_NORMAL(udf.5)	/* Fix up interrupt state */
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(undf_out)			/* Return to invoker */
#endif

/*
 * THIS IS THE VECTORBUS/NVI PREAMBLE
 */
	.globl	vecbus
	.globl	nvi
	.globl	_vecbus_isr
	.globl	_master_reti
#if !STAT_TIME
	.globl	_kernel_timer
#endif	!STAT_TIME
vecbus:					/* ICU (APC/XPC) interrupts */
nvi:					/* non-ICU (DPC) interrupts */
	BUILD_FRAME			/* Create the stack frame */
#if	MMAX_XPC || MMAX_APC
	ENTER_INTERRUPT_RACE(ICU_VECBUS_BIT,int.1) /* Fix interrupt state */
#endif	MMAX_XPC || MMAX_APC
#if	!STAT_TIME
	TIME_INT_ENTRY(_kernel_timer)
#endif	!STAT_TIME
	sprd	sp, r0			/* Pass the save frame and ... */
#if	MMAX_XPC
	tbitd	$(XPCVB_STAT_FIFO_BIT), @XPCVB_STATUS
	bfc	vecbus.bad		/* No -> bad error */
	movzbd	@VB_FIFO,r1     	/* Interrupt number */
#endif	MMAX_XPC
#if	MMAX_APC
	tbitd	$(APCCSR_VB_FIFO_BIT), @APCREG_CSR
	bfc	vecbus.bad		/* No -> bad error */
	movzbd	@VB_FIFO,r1     	/* Interrupt number */
#endif	MMAX_APC
#if	MMAX_DPC
	movzbd	@DPCREG_VECTOR,r1	/* Interrupt number */
#endif MMAX_DPC
	jsr	@_vecbus_isr		/* do it in `C' */
	br	intexit

#if	MMAX_XPC || MMAX_APC
vecbus.bad:				/* Invalid interrupt */
	PRINTF(@vecbusmsg,0)
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(_masterbad_reti)		/* Really return from interrupt */
vecbusmsg:
	.asciz	"Vector bus interrupt with no vector"
#endif	MMAX_XPC || MMAX_APC

	/*
	 * We are about to exit back to the user. Do a normal exit.
	 */
/*
 * THIS IS THE VECTORBUS/NVI POSTAMBLE
 */
intexit:
#if	!STAT_TIME
	TIME_INT_EXIT
#endif	!STAT_TIME
	tbitw	$PSR_U_BIT,10(fp)	/* Don't process AST unless */
	bfc	int_out			/* returning to user mode */
	GETCPUID(r0)
	cmpqd	$0,_need_ast[r0:d]	/* Is an AST needed */
	bne	int_ast			/* if yes, go do it */
int_out:
	EXIT_INTERRUPT_RACE(ret.0)
#if	MMAX_IDEBUG
	GETCPUID(r0)
	movd	r6,_last_int_r6[r0:b]
	movb	@M_ICU_BASE+ISRV,_last_int_isrv[r0:b]
	movb	@M_ICU_BASE+ISRV+RBIAS,_last_int_rbias[r0:b]
#endif	MMAX_IDEBUG
	.globl	_master_reti
	RESTORE_FRAME			/* Restore the stack frame */
	RETI(_master_reti)		/* Really return from interrupt */

	/*
	 * To do an AST after an interrupt, we must first do a
	 * reti instruction to get the hardware to re-enable interrupts.
	 */

int_ast:
#if	MMAX_XPC || MMAX_APC
#if	MMAX_IDEBUG
	cmpqd	$0, r6
	beq	int_ast_pass1
	PANIC(@int_ast_race_bogon, $0, $0)
int_ast_pass1:
	cmpqb	$0, @M_ICU_BASE+ISRV
	beq	int_ast_pass2
int_ast_crash:
	movb	@M_ICU_BASE+ISRV, int_ast_bogon
	movb	@M_ICU_BASE+ISRV+RBIAS, int_ast_bogon+1
	PRINTF(@int_ast_bogon_mes, @int_ast_bogon)
	PANIC(@int_ast_bogon_panic, $0, $0)
int_ast_pass2:
	cmpb	$(ICU_NORMAL > 8), @M_ICU_BASE+ISRV+RBIAS
	bne	int_ast_crash
#endif
#if	0
	/*
	 * This won't work at all unless we also do the RETI.
	 * So we either do the e_i_r()/RETI *or* we can simply
	 * decode r6 and, if r6 is 0, clear the NORMAL bit,
	 * otherwise leave it set.
	 */
#if	0
	cmpqb	$0, r6
	bne	int_ast_no_fix
	cbitb	$ICU_NORMAL_BIT-8, @M_ICU_BASE+ISRV+RBIAS
int_ast_no_fix:
#endif
	EXIT_INTERRUPT_RACE(int_ast.1)
#endif
	movd	_OSmodpsr,tos		/* push valid mod/psr pair with
						interrupts enabled */
	addr	do_ast,tos		/* push return address  */
	RETI(ast.int)			/* return from interrupt to
						next instruction */
do_ast:
#endif	/* MMAX_XPC || MMAX_APC */
#if	MMAX_DPC
	movd	_OSmodpsr,tos		/* push valid mod/psr pair with
						interrupts enabled */
	addr	do_ast,tos		/* push return address  */
	PREP_FOR_SHORT_RETI(ICU_VECBUS_BIT,intast1)
	RETI(ast.int)			/* return from interrupt to
						next instruction */
do_ast:
#endif
#if	!STAT_TIME
	TIME_TRAP_UENTRY		/* AST must be from user mode */
#endif
	addqd	$1,_astcnt
	TRAP(T_AST,0)			/* trap(AST,istkp) */
#if	MMAX_IDEBUG
	GETCPUID(r0)
	movd	r6,_last_int_ast_r6[r0:b]
	movb	@M_ICU_BASE+ISRV,_last_int_ast_isrv[r0:b]
	movb	@M_ICU_BASE+ISRV+RBIAS,_last_int_ast_rbias[r0:b]
#endif
	TRACK()
	br 	rett_ast		/* back to user */

/*
 *	bpt and halt
 */
ENTRY(bpt)
	bpt
	ret	$0

ENTRY(halt)
	bpt

#if	MMAX_IDEBUG
#if	MMAX_XPC || MMAX_APC
	.data
exit_int_bogon:		.int	0
enter_int_bogon:	.int	0
rett_out_bogon:		.int	0
abt_bogon:		.int	0
tse_ast_bogon:		.int	0
int_ast_bogon:		.int	0
_last_tse_r6:		.space	NCPUS * 4
_last_tse_isrv:		.space	128
_last_tse_rbias:	.space	128
_last_tse_ast_r6:	.space	NCPUS * 4
_last_tse_ast_isrv:	.space	128
_last_tse_ast_rbias:	.space	128
_last_int_r6:		.space	NCPUS * 4
_last_int_isrv:		.space	128
_last_int_rbias:	.space	128
_last_int_ast_r6:	.space	NCPUS * 4
_last_int_ast_isrv:	.space	128
_last_int_ast_rbias:	.space	128
	.text
exit_int_bogon_mes:	.asciz	"bogus isrv 0x%x in exit interrupt race\n"
exit_int_bogon_panic:	.asciz	"bogus isrv in exit interrupt race"
enter_int_bogon_mes:	.asciz	"bogus isrv 0x%x in enter interrupt race\n"
enter_int_bogon_panic:	.asciz	"bogus isrv in enter interrupt race"
abt_bogon_mes:		.asciz	"bogus isrv 0x%x in abt\n"
abt_bogon_panic:	.asciz	"bogus isrv in abt\n"
abt_ints_off:		.asciz	"abt while ints off\n"
tse_ast_bogon_mes:	.asciz	"bogus isrv 0x%x in tse ast\n"
tse_ast_bogon_panic:	.asciz	"bogus isrv in tse ast"
int_ast_bogon_mes:	.asciz	"bogus isrv 0x%x in int ast\n"
int_ast_bogon_panic:	.asciz	"bogus isrv in int ast"
tse_ast_race_bogon:	.asciz	"tse race with NORMAL bit from user mode"
int_ast_race_bogon:	.asciz	"int race with NORMAL bit from user mode"
#endif	/* MMAX_XPC || MMAX_APC */
#endif	/* MMAX_IDEBUG */
