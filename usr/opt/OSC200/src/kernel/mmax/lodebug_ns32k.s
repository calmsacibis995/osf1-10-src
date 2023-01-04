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
 *	@(#)$RCSfile: lodebug_ns32k.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:41:21 $
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
 * Copyright (c) 1989 Encore Computer Corporation.
 */
/*
 * lodebug_ns32k.s:  miscellaneous assembly-language support for various
 *	assertions and debug statements.
 */
#include "assym.s"

#include "mach_assert.h"
#include "mach_ldebug.h"
#include "mmax_xpc.h"
#include "mmax_apc.h"
#include "mmax_dpc.h"
#include "mmax_debug.h"
#include "mmax_idebug.h"
#include "lock_stats.h"
#include "cpus.h"

#include "mmax/mlmacros.h"
#include "mmax/psl.h"
#include "mmax/icu.h"
#include "mmax/cpudefs.h"

	.globl	_getpc		/* get current pc */
_getpc:	movd	0(sp),r0
	ret	$0

 #
 # same, put result in r1 for inline lock code
 # (note: used by code NOT under MMAX_MPDEBUG)
 #
	.globl	_getpcr1
_getpcr1:	movd	0(sp),r1
		ret	$0

       .globl  _getpc_fromld   /* get pc of caller of lock_done */
_getpc_fromld:                  /* changes if lock_done stack frame changes */
        movd    8(sp),r0        /* enter [r3,r4],$0 on entry */
        ret     $0

	.globl	_getfrompc
_getfrompc:
	movd	4(fp),r0
	ret	$0

#if	MACH_ASSERT
	.globl	_getpc_fromep	/* get pc of caller of event_post */
_getpc_fromep:			/* changes if event_post stack frame changes */
	movd	16(sp),r0	/* enter [r3,r4],$0 on entry */
	ret	$0

	.globl	_getpc_fromec	/* get pc of caller of event_clear */
_getpc_fromec:			/* changes if event_clear stack frame changes */
	movd	16(sp),r0	/* enter [r3,r4],$0 on entry */
	ret	$0
#endif

#if	MMAX_DEBUG || MACH_LDEBUG
 #
 # Print stack trace
 #
 # Register Usage:
 #	R4	- Pointer to data (originally at SP+4)
 #	R3	- Byte Count
 #	R2	- Clobbered by printf
 #	R1	- Arg to printf
 #	R0	- Arg to printf
 #

	.globl	_page_mask
	.globl	_prstack
_prstack:
	save	[r3,r4]
	sprd	sp, r4		# fetch stack pointer & skip over...
	addd	$8, r4		# ...the registers we just saved
	movd	@_page_mask, r3	# compute offset to stack base by...
	comd	r3, r3		# ...computing high-order mask to...
	andd	r4, r3		# ...rid ourselves of low-order page bits...
	ord	@_page_mask, r3	# ...(but inc by one page to find base of...
	addqd	$1, r3		# ...(next page, avoiding extra reg. use)...
	subd	r4, r3		# ...and subtract off current "sp".
prq:	cmpd	$16, r3		# are there 4 words left to print?
	bhi	prw		# hi means no, try printing single words
	addr	stfmt, r0
	movd	r4, r1		# R1 - has stack pointer
	movd	12(r4), tos	# Fourth stack word
	movd	8(r4), tos	# Third stack word
	movd	4(r4), tos	# Second stack word
	movd	0(r4), tos	# First stack word
	jsr	@_printf
	adjspb	$-16
	addd	$16, r4		# move stack pointer over 16 bytes just printed
	addd	$-16, r3	# decrement counter by same 16 bytes
	br	prq		# and try for some more...
				# shift to printing one word at a time
prw:	cmpd	$4, r3		# at least one word left to go?
	bhi	prb		# hi means no, go try bytes
	addr	stfmt1, r0	# printf control string
	movd	r4, r1		# current stack address
	movd	0(r4), tos	# word from stack
	jsr	@_printf	# do it
	adjspb	$-4		# pop off word from stack
	addd	$4, r4		# one more word printed
	addd	$-4, r3		# one less word to print
	br	prw
				# shift to printing one byte at a time
prb:	cmpqd	$0, r3		# are there any bytes left?
	bhs	prd		# hi = no, we're done
	addr	stfmt1, r0	# we can re-use control string for single words
	movd	r4, r1		# r1 = pointer into stack
	movzbd	0(r4), tos	# push byte from stack
	jsr	@_printf
	adjspb	$-4
	addqd	$1, r4
	addqd	$-1, r3
	br	prb
prd:
	restore	[r3,r4]
	ret	$0

	.data
stfmt:	.asciz	"%x:  0x%08x    0x%08x    0x%08x    0x%08x\n"
stfmt1:	.asciz	"%x:  0x%08x\n"
	.text
#endif	/* MMAX_DEBUG || MMAX_PDEBUG */

#if	PDEBUG
	.globl	nmi_exists_msg
	.globl	icu_trace_mes
	.globl	icu_colon
	.globl	icu_isrv_mes
	.globl	icu_isrvrb_mes
	.globl	icu_ipnd_mes
	.globl	icu_ipndrb_mes
	.globl	icu_imsk_mes
	.globl	icu_imskrb_mes
nmi_exists_msg:
	.asciz	"\nNMI Condition:  "
icu_trace_mes:	.asciz	"\t<ICU> "
icu_colon:	.asciz	": "
icu_isrv_mes:	.asciz	" isrv(0x%x)"
icu_isrvrb_mes:	.asciz	" isrv+rbias(0x%x)\n"
icu_ipnd_mes:	.asciz	"\t\tipnd(0x%x)"
icu_ipndrb_mes:	.asciz	" ipnd+rbias(0x%x)"
icu_imsk_mes:	.asciz	" imsk(0x%x)"
icu_imskrb_mes:	.asciz	" imsk+rbias(0x%x)\n"

ENTRY(sysnmi_check)
	SYSNMI_TEST(sc0, r0)
	ret	$0

ENTRY(icu_trace)
	movd	r0, r1
	ICU_TRACE(itlab)
	ret	$0
#endif	/* PDEBUG */

#if	MMAX_IDEBUG
	.data
/*
 * Last place interrupts were turned on, off, or restored
 *
 */

	.globl	_last_intth
_last_intth:		.space	NCPUS * 4	/* last interrupted thread */
	.globl	_last_intpc
_last_intpc:		.space	NCPUS * 4	/* pc interrupted from */
	.globl	_last_inttype
_last_inttype:		.space	NCPUS * 4	/* last interrupt type */
	.globl	_last_spl0
_last_spl0:		.space	NCPUS * 4	/* last pc enabling ints */
	.globl	_last_splx
_last_splx:		.space	NCPUS * 4	/* last pc restoring ints */
	.globl	_last_splany
_last_splany:		.space	NCPUS * 4	/* last pc disabling ints */
	.globl	_last_splnmi
_last_splnmi:		.space	NCPUS * 4	/* last pc disabling nmi */
	.globl	_last_splnmix
_last_splnmix:		.space	NCPUS * 4	/* last pc enabling nmi */
	.globl	_spl_bogon
_spl_bogon:		.int	0
	.globl	_spl_retbogon
_spl_retbogon:		.int	0
spl_fail_mes:		.asciz	"splfoo at pc 0x%x botched"
spl_fail_panic:		.asciz	"splfoo botched"
	.globl	_splx_bogon
_splx_bogon:		.int	0
	.globl	_splx_setbogon
_splx_setbogon:		.int	0
splx_fail_mes:		.asciz	"splx at pc 0x%x botched"
splx_fail_panic:	.asciz	"splx botched"
	.text

ENTRY(spl0)
	cbitb	$ICU_NORMAL_BIT-8, @M_ICU_BASE+ISRV+RBIAS
	bispsrw	$PSR_I
        save	[r0]
	GETCPUID(r0)
        movd	4(sp),@_last_spl0[r0:d]		# pc of caller
        restore [r0]
	ret	$0

ENTRY(spl1)
ENTRY(spl2)
ENTRY(spl3)
ENTRY(spl4)
ENTRY(spl5)
ENTRY(spl6)
ENTRY(spl7)
ENTRY(splnet)
ENTRY(spltty)
ENTRY(splimp)
ENTRY(splbio)
ENTRY(splvm)
ENTRY(splhi)
ENTRY(splclock)
ENTRY(splsoftclock)
ENTRY(splhigh)
ENTRY(splsched)
	movb	@M_ICU_BASE+ISRV+RBIAS, r0
	sbitb	$ICU_NORMAL_BIT-8, @M_ICU_BASE+ISRV+RBIAS
        save	[r1]
	GETCPUID(r1)
        movd	4(sp),@_last_splany[r1:d]
	restore	[r1]
	/*
	 * After having done an s = splfoo(), we must see all bits clear
	 * in the low isrv byte.  The value being returned to the caller
	 * may or may not have the NORMAL bit set but must have all other
	 * bits clear, and the value in the high isrv byte must have just
	 * one bit, the NORMAL bit, set.
	 */
	save	[r0]
	cmpqb	$0, @M_ICU_BASE+ISRV
	bne	spl_fail
	andb	$~(ICU_NORMAL > 8), r0
	cmpqb	$0, r0
	bne	spl_fail
	cmpb	$(ICU_NORMAL > 8), @M_ICU_BASE+ISRV+RBIAS
	bne	spl_fail
	restore	[r0]
	ret	$0
spl_fail:
	movb	@M_ICU_BASE+ISRV, _spl_bogon
	movb	@M_ICU_BASE+ISRV+RBIAS, _spl_bogon+1
	movb	0(sp), _spl_retbogon
	PRINTF(@spl_fail_mes, 16(sp))
	PANIC(@spl_fail_panic, $0, $0)

ENTRY(splx)
        save	[r1]
	GETCPUID(r1)
        movd	4(sp),@_last_splx[r1:d]
	restore	[r1]
	save	[r0]
	/*
	 * The value being put back into the ICU may have the NORMAL bit
	 * set or clear but all other bits must be clear.
	 */
	andb	$~(ICU_NORMAL > 8), r0
	cmpqb	$0, r0
	bne	splx_fail
	restore	[r0]
	movb	r0, @M_ICU_BASE+ISRV+RBIAS
	/*
	 * After having done an splx(s), we must see all bits clear
	 * in the low isrv byte.  The value in the high isrv byte may
	 * have the NORMAL bit set but no other bit may be set.
	 */
	save	[r0]
	cmpqb	$0, @M_ICU_BASE+ISRV
	bne	splx_fail
	movb	@M_ICU_BASE+ISRV+RBIAS, r0
	andb	$~(ICU_NORMAL > 8), r0
	bne	splx_fail
	restore	[r0]
	ret	$0
splx_fail:
	movb	@M_ICU_BASE+ISRV, _splx_bogon
	movb	@M_ICU_BASE+ISRV+RBIAS, _splx_bogon+1
	movb	0(sp), _splx_setbogon
	PRINTF(@splx_fail_mes, 16(sp))
	PANIC(@splx_fail_panic, $0, $0)

/*
 * Disable NMIs (basically all possible interrupt sources) and return
 * the old ICU value.  Note that there are not many statements we can
 * make about the old or new ICU value, as we could be in almost any
 * interrupt state.
 */
ENTRY(splnmi)
	movb	@M_ICU_BASE+ISRV, r0
	sbitb	$ICU_POWERFAIL_BIT, @M_ICU_BASE+ISRV
        save	[r1]
	GETCPUID(r1)
        movd	4(sp),@_last_splnmi[r1:d]
	restore	[r1]
	ret	$0
ENTRY(splnmix)
        save	[r1]
	GETCPUID(r1)
        movd	4(sp),@_last_splnmix[r1:d]
	restore	[r1]
	movb	r0, @M_ICU_BASE+ISRV
	ret	$0

	.data
	.globl	_cpu_track
_cpu_track:		.space	NCPUS * 4
	.text
ENTRY(tracker)
	save	[r1]
	GETCPUID(r1)
	movd	4(sp), _cpu_track[r1:d]
	restore	[r1]
	ret	$0

/*
 * When exiting from a trap back to user-mode or kernel-mode, check
 * stacks, interrupt state, whatever.
 */
	.data
_roic_isrv_bogon:	.space	NCPUS * 4
	.text
roic_sys_stack:		.asciz	"rett_out to system mode with bad psl 0x%x\n"
roic_sys_stack_panic:	.asciz	"rett_out to system mode botch"
roic_sys_icu_mes:	.asciz	"rett_out:  icu 0x%x botched (psl=0x%x)\n"
roic_sys_icu_panic:	.asciz	"rett_out:  icu botch"
roic_usr_stack:		.asciz	"rett_out to user mode with bad psl 0x%x\n"
roic_usr_stack_panic:	.asciz	"rett_out to user mode botch"

ENTRY(rett_out_intr_checks)
	save	[r0,r1]
	GETCPUID(r1)
	tbitw	$PSR_U_BIT,10(fp)	/* Separate user, system mode cases */
	bfs	roic_usr
roic_sys:
	tbitw	$PSR_S_BIT,10(fp)	/* better be using system stack */
	bfc	roic_sys1
roic_bogus_psl:
	movd	8(fp), r0
	PRINTF(@roic_sys_stack, r0)
	PANIC(@roic_sys_stack_panic, $0, $0)
roic_sys1:
	cmpw	@_OSmodpsr, 8(fp)		/* must use OS mod table */
	bne	roic_bogus_psl
	cmpqb	$0, @M_ICU_BASE+ISRV		/* no higher ints blocked */
	beq	roic_sys2
roic_bogus_icu:
	movb	@M_ICU_BASE+ISRV, @_roic_isrv_bogon[r1:d]
	movb	@M_ICU_BASE+ISRV+RBIAS, @_roic_isrv_bogon+1[r1:d]
	movd	8(fp), tos
	movd	@_roic_isrv_bogon[r1:d], r1
	addr	@roic_sys_icu_mes, r0
	jsr	@_printf
	PANIC(@roic_sys_icu_panic, $0, $0)
roic_sys2:
	movb	@M_ICU_BASE+ISRV+RBIAS, r0	/* NORMAL bit may be set */
	andb	$~(ICU_NORMAL > 8), r0
	cmpqb	$0, r0
	bne	roic_bogus_icu
	br	roic_out
roic_usr:
	tbitw	$PSR_S_BIT,10(fp)		/* user stack is a must */
	bfs	roic_usr1
	movd	10(fp), r0
	PRINTF(@roic_usr_stack, r0)
	PANIC(@roic_usr_stack_panic, $0, $0)
roic_usr1:
	cmpqb	$0, @M_ICU_BASE+ISRV		/* no ints may be blocked */
	bne	roic_bogus_icu
	cmpqb	$0, @M_ICU_BASE+ISRV+RBIAS	/* ... in either isrv byte */
	bne	roic_bogus_icu
roic_out:
	restore	[r0,r1]
	ret	$0

/*
 * Interrupts should be enabled so no bits should be visible in the isrv.
 */
	.data
ibc_isrv_bogon:		.space	NCPUS * 4
	.text
ibc_isrv_bogon_mes:	.asciz	"icu_bits_clear:  from pc 0x%x, icu 0x%x\n"
ibc_isrv_bogon_panic:	.asciz	"icu_bits_clear:  bogus icu value"

	.globl	_icu_bits_clear
	.globl	_ints_on
	.globl	_icu_ints_on

ENTRY(icu_bits_clear)
	cmpqb	$0, @M_ICU_BASE+ISRV
	beq	ibc_pass1
ibc_crash:
	save	[r0,r1]				/* just in case */
	GETCPUID(r1)
	movb	@M_ICU_BASE+ISRV, @ibc_isrv_bogon[r1:d]
	movb	@M_ICU_BASE+ISRV+RBIAS, @ibc_isrv_bogon+1[r1:d]
	movd	@ibc_isrv_bogon[r1:d], tos	/* bogus icu value */
	movd	12(sp), r1			/* pc of caller */
	addr	@ibc_isrv_bogon_mes, r0
	jsr	@_printf
	PANIC(@ibc_isrv_bogon_panic, $0, $0)
ibc_pass1:
	cmpqb	$0, @M_ICU_BASE+ISRV+RBIAS
	bne	ibc_crash
	ret	$0

ENTRY(ints_on)				/* make sure CPU permits interrupts */
	sprw	psr,r0
	tbitw	$PSR_I_BIT,r0
	sfsd	r0
	ret	$0
ENTRY(icu_ints_on)			/* make sure ICU permits interrupts */
        tbitb	$ICU_NORMAL_BIT-8,@M_ICU_BASE+ISRV+RBIAS
	sfcd	r0
	ret	$0
#endif	/* MMAX_IDEBUG */

#if	MMAX_DEBUG || MMAX_IDEBUG
ENTRY(nmi_off)				/* NMIs blocked by setting POWERFAIL */
#if	MMAX_IDEBUG
	tbitb	$ICU_POWERFAIL_BIT,@M_ICU_BASE+ISRV
	sfsd	r0
#endif
	ret	$0
#endif	/* MMAX_DEBUG || MMAX_IDEBUG */
