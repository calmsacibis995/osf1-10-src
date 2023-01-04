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
 *	@(#)$RCSfile: mlmacros.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:52 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>
#if	MMAX_XPC || MMAX_APC
#include <mmax/icu.h>
#endif	MMAX_XPC || MMAX_APC

/*
 * mlmacros.h
 *
 * The following file has the macros that are necessary for .s
 * files.
 */


#if	MMAX_XPC || MMAX_APC || MMAX_DPC
/*
 * Macros used to do "reti" and "rett" instructions because of chip bugs
 *
 * N.B.:  use RETT_ENB when returning to user-mode and and needing interrupts
 * to be enabled.  On the DPC, this will happen automatically but on the APC
 * and XPC the macro must do some extra work.
 */

#ifdef	RETIBUG

#define RETI(label) \
	br	label ;\
	.align	4 ;\
label:	reti

#define RETT(label) \
	br	label ;\
	.align	4 ;\
label:	rett	$0

#if	MMAX_XPC || MMAX_APC
#define	RETT_ENB(label) \
	br	label ;\
	.align	4 ;\
label:	ENBINT ;\
	rett	$0
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
#define	RETT_ENB(label)		RETT(label)
#endif	MMAX_DPC

#else	RETIBUG

#define RETI(label) label:	reti
#define RETT(label) label:	rett	$0

#if	MMAX_XPC || MMAX_APC
#define	RETT_ENB(label) \
label:	ENBINT; \
	rett	$0
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
#define	RETT_ENB(label)		RETT(label)
#endif	MMAX_DPC

#endif	RETIBUG
#endif	MMAX_XPC || MMAX_APC || MMAX_DPC

/* Macros to switch into and out of user stack */

#if	MMAX_XPC
/*
 * User stack pointer is visible in system mode.
 */
#endif	MMAX_XPC

#if	MMAX_APC || MMAX_DPC
#define USRSP	bispsrw	$PSR_S

#define SYSSP	bicpsrw	$PSR_S
#endif	MMAX_APC || MMAX_DPC

/* Macros to disable and enable interrupts. */

/* The following macros are used on entry/exit from traps.
 * The ENTER/EXIT_DISINT_TRAP macro is used for traps where
 * the hardware automatically disables interrupts in the PSR upon
 * entry to the routine (abt, ber). ENTER/EXIT_NODISINT_TRAP is
 * used for traps where the hardware does nothing to the PSR for
 * the trap.
 */

#if	MMAX_XPC || MMAX_APC
#define ENTER_DISINT_TRAP \
	sbitb	$ICU_NORMAL_BIT-8,@M_ICU_BASE+ISRV+RBIAS ; \
        sfsd r3 ; \
	bispsrw $PSR_I

#define EXIT_DISINT_TRAP(label) \
	cmpqd	$0,r3 ; \
	bne	label ; \
	cbitb	$ICU_NORMAL_BIT-8,@M_ICU_BASE+ISRV+RBIAS ; \
label:

#define ENTER_NODISINT_TRAP \
	bicpsrw	$PSR_I ; \
	sbitb	$ICU_NORMAL_BIT-8,@M_ICU_BASE+ISRV+RBIAS ; \
	bispsrw	$PSR_I ; \
        sfsd    r3

#define EXIT_NODISINT_TRAP(label) \
	cmpqd	$0,r3 ; \
	bne	label ; \
	bicpsrw	$PSR_I ; \
	cbitb	$ICU_NORMAL_BIT-8,@M_ICU_BASE+ISRV+RBIAS ; \
	bispsrw	$PSR_I ; \
label:

#define DISINT sbitb $ICU_NORMAL_BIT-8,@M_ICU_BASE+ISRV+RBIAS
#define ENBINT cbitb $ICU_NORMAL_BIT-8,@M_ICU_BASE+ISRV+RBIAS
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
#define ENTER_DISINT_TRAP

#define EXIT_DISINT_TRAP(label)

#define ENTER_NODISINT_TRAP \
	bicpsrw	$PSR_I

#define EXIT_NODISINT_TRAP(label)

#define DISINT	bicpsrw	$PSR_I
#define ENBINT	bispsrw	$PSR_I

#endif	MMAX_DPC

/* Macros to obtain the current CPU number */

#if	MMAX_XPC
#define	GETCPUID(dest) \
	movzbd	@XPCREG_CSR, dest; \
	andd	$(XPCCSR_CPUID | XPCCSR_SLOTID), dest
#endif	MMAX_XPC

#if	MMAX_APC
#define GETCPUID(dest) \
        movzbd  @APCREG_CSR, dest
#endif	MMAX_APC

#if	MMAX_DPC
#define GETCPUID(dest) \
	movd	@DPCREG_STS, dest ; \
	andd	$(DPCSTS_CPUID+DPCSTS_SLOTID), dest

/* Macros to obtain the current DPC slot number */
#define GETDPCSLOTA(dest) \
	GETCPUID(dest) ; \
	lshd	$-2,dest
#endif	MMAX_DPC


/*
 * Macros to enable and disable NMI's
 */

#if	MMAX_XPC || MMAX_APC
#define DISNMI(label,reg0,reg1) \
        sbitb   $ICU_POWERFAIL_BIT,@M_ICU_BASE+ISRV
#define ENBNMI(label,reg0,reg1) \
        cbitb   $ICU_POWERFAIL_BIT,@M_ICU_BASE+ISRV
#endif	MMAX_XPC || MMAX_APC

#if	MMAX_DPC
#define DISNMI(label,reg0,reg1) \
	GETCPUID(reg0) ; \
	ord	$DPCCTL_NMI_DISABLE,_Dpc_ctl[reg0:d] ; \
	movd	_Dpc_ctl[reg0:d],r1 ; \
	xord	$DPCCTL_FIX,r1 ; \
	movd	r1,@DPCREG_CTL ; \
	addqd	$1,@_Dpc_nmidiscnt[reg0:d]

#define ENBNMI(label,reg0,reg1) \
	GETCPUID(reg0) ; \
	addqd	$-1,@_Dpc_nmidiscnt[reg0:d] ; \
	cmpqd	$0,@_Dpc_nmidiscnt[reg0:d] ; \
	bne	label ; \
	comd	$DPCCTL_NMI_DISABLE,reg1 ; \
	andd	reg1,@_Dpc_ctl[reg0:d] ; \
	movd	_Dpc_ctl[reg0:d],reg1 ; \
	xord	$DPCCTL_FIX,reg1 ; \
	movd	reg1,@DPCREG_CTL ; \
label:

#endif MMAX_DPC

/* Build and restore the stack frame used by all traps/interrupts.
 * The stack frame consists of the fp, the registers, and the user sp.
 *
 * These macros should be the first and last things executed by the
 * trap/interrupt routines.
 */

#if	MMAX_XPC
#define BUILD_FRAME \
	enter	[r0,r1,r2,r3,r4,r5,r6,r7],$0 ; \
	sprd	usp,tos	;
#define RESTORE_FRAME \
	lprd	usp,tos	; \
	exit	[r0,r1,r2,r3,r4,r5,r6,r7]
#endif	MMAX_XPC

#if	MMAX_APC || MMAX_DPC
#define BUILD_FRAME \
	enter	[r0,r1,r2,r3,r4,r5,r6,r7],$0 ; \
	USRSP		; \
	sprd	sp,r4	; \
	SYSSP		; \
	movd	r4,tos	;

#define RESTORE_FRAME \
	movd	tos,r4	; \
	USRSP		; \
	lprd	sp,r4	; \
	SYSSP		; \
	exit	[r0,r1,r2,r3,r4,r5,r6,r7]
#endif	MMAX_APC || MMAX_DPC

/*
 * Macro to define entry points for assembly-language routines called
 * from C.
 */
#ifdef GPROF
#define	ENTRY(name) .globl _/**/name; _/**/name: jsr mcount
#else
#define	ENTRY(name) .globl _/**/name; _/**/name:
#endif GPROF

/*
 * Macro to call the routine panic with three parameters.
 */

#define PANIC(param1,param2,param3) \
	movd	param3,tos			; \
	movd	param2,r1			; \
	addr	param1,r0			; \
	jsr	@_panic				; \
	cmpqd	$0,tos

#define	PRINTF(fmt, arg)	\
	save	[r0,r1,r2]	; \
	movd	arg, r1		; \
	addr	fmt, r0		; \
	jsr	@_printf	; \
	restore	[r0,r1,r2]

#define	PDEBUG	0
#if	PDEBUG
/*
 * For times when life is hard, the following macros can be (carefully)
 * used to follow the action...  Data declarations are in lodebug_ns32k.s.
 */

/*
 * Emit a character onto the console with a minimum of fuss.  Should be
 * rewritten to use defines for SCC locations.  Assumes uniprocessor.
 * IMPORTANT NOTE:  use as PDC(label,x) where x is the character to be
 * displayed and there is no space between the comma and the x.  Otherwise
 * the macro may not assemble as expected.
 */
#define	PDC(l,c) \
l/**/1:	cmpqb	$0, @-262144; \
	bne	l/**/1 ; \
	movb	$'c,@-262143 ; \
	movqb	$1,@-262144

/*
 * Put two characters on the console, preceded by a "%".
 * IMPORTANT NOTE:  see caveats for PDC().
 */
#define	PDCC(l,c,d)	\
	PDC(l,%) ; \
l/**/3:	cmpqb	$0, @-262144 ; \
	bne	l/**/3 ; \
	movb	$'c, @-262143 ; \
	movqb	$1, @-262144 ;	\
l/**/5:	cmpqb	$0, @-262144 ; \
	bne	l/**/5 ; \
	movb	$'d, @-262143 ; \
	movqb	$1, @-262144

/*
 * Display contents of master ICU's in-service, pending and mask registers.
 * r1 should contain the address of a message identifying the location of
 * the code doing the trace.
 */
#define	ICU_TRACE(label) \
	save	[r0,r1,r2] ; \
	addr	@icu_trace_mes, r0 ; \
	jsr	@_printf ; \
	movd	4(sp), r0 ; \
	jsr	@_printf ; \
	addr	@icu_colon, r0 ; \
	jsr	@_printf ; \
	movqd	$0, r1 ; \
	movzbd	@M_ICU_BASE+ISRV,r1 ; \
	addr	@icu_isrv_mes, r0 ; \
	jsr	@_printf ; \
	movzbd	@M_ICU_BASE+ISRV+RBIAS, r1 ; \
	addr	@icu_isrvrb_mes, r0 ; \
	jsr	@_printf ; \
	movzbd	@M_ICU_BASE+IPND,r1 ; \
	addr	@icu_ipnd_mes, r0 ; \
	jsr	@_printf ; \
	movzbd	@M_ICU_BASE+IPND+RBIAS, r1 ; \
	addr	@icu_ipndrb_mes, r0 ; \
	jsr	@_printf ; \
	movzbd	@M_ICU_BASE+IMASK, r1 ; \
	addr	@icu_imsk_mes, r0 ; \
	jsr	@_printf ; \
	movzbd	@M_ICU_BASE+IMASK+RBIAS, r1 ; \
	addr	@icu_imskrb_mes, r0 ; \
	jsr	@_printf ; \
	restore	[r0, r1, r2]

#define	SYSNMI_TEST(label, mes)	\
	movqd	$0, r0	; \
	tbitd	$ICU_SYSNMI_BIT,@M_ICU_BASE+ISRV ; \
	bfc	label/**/A ; \
	addqd	$1, r0 ; \
label/**/A: \
	tbitd	$ICU_SYSNMI_BIT,@M_ICU_BASE+IMASK ; \
	bfc	label/**/B ; \
	addqd	$2, r0 ; \
label/**/B: \
	tbitd	$ICU_SYSNMI_BIT,@M_ICU_BASE+IPND ; \
	bfc	label/**/C ; \
	addqd	$4, r0 ; \
label/**/C: \
	cmpqd	$0, r0 ; \
	beq	label/**/D ; \
	PRINTF(nmi_exists_msg,$0); \
	PRINTF(mes,r0) ; \
label/**/D:

#else	PDEBUG

#define	PDC(l,c)
#define PDCC(l,c,d)
#define	ICU_TRACE(label)
#define	SYSNMI_TEST(l,m)

#endif	PDEBUG
