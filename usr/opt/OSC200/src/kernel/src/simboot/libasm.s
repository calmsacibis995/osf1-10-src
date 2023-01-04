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
 * @(#)libasm.s	9.3  (ULTRIX)        3/11/91
 */

/*
 *
 * This file contains asm machine and architecture dependant routines.
 *
 */

#include <machine/regdef.h>
#include <machine/reg.h>
#include <machine/asm.h>

/*
 * halt entry point
 */
LEAF(stop)
	call_pal PAL_halt
	.end	stop


/*
 * global location for storing the virtual address of the dispatch routine
 * (filled in by crt0)
 */
	.data
	.globl dispatch
dispatch:	.quad 0
	.text

/*
 * prom entry point dispatcher
 *
 * This routine forms a firewall between us and the console routines.
 * It saves all important registers except for v0,v1 and a0-a4.  It then
 * converts our calling sequence to the alpha standard call and dispatches 
 * to the prom.
 *
 */

#define TR_MASK M_T0|M_T1|M_T2|M_T3|M_T4|M_T5|M_T6|M_T7|M_T8|M_T9|M_T10|M_T11|M_T12
#define SR_MASK M_S0|M_S1|M_S2|M_S3|M_S4|M_S5|M_S6

#define EXCMASK TR_MASK|SR_MASK|M_GP|M_SP|M_RA

/*
 * Console callback procedure dispatch
 *
 * This routine is a jacket routine that converts our standard
 * C function call to the alpha calling standard call.
 */
NESTED(prom_dispatcher,EF_SIZE,ra)
	.mask   EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8))
	ldgp	gp,0(pv)
	lda	sp,-EF_SIZE(sp)
	stq     ra,EF_RA*8(sp)
	.set noat
	stq     AT,EF_AT*8(sp)
	.set at
	bsr	ra,reg_save

	lda	t1,dispatch		# get the address
	ldq	v0,(t1)
	bne	v0,1f			# if not filled in config
	jsr	ra,prom_init
	lda	t1,dispatch		# get the address
	stq	v0,(t1)
1:	lda	t1,disp_proc_va		# get the address
	ldq	pv,(t1)
	jsr	ra,(v0)			# go for it

	bsr	ra,reg_restore
	.set    noat
	ldq     AT,EF_AT*8(sp)
	.set    at
	ldq     ra,EF_RA*8(sp)
	lda	sp,EF_SIZE(sp)
	ret	zero,(ra)
END(prom_dispatcher)

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
 * VMS palcode calls
 */
#define PAL_mfpr_pcbb   0x12
#define PAL_mfpr_ptbr   0x15
#define PAL_mfpr_vptb   0x29
#define PAL_ldqp        0x03
#define PAL_stqp        0x04

/*
 * OSF palcode calls
 */
#define SWITCH		40		/* old, vms2osf call */
#define SWPPAL		10

LEAF(mfpr_pcbb)
	call_pal PAL_mfpr_pcbb
	ret	zero,(ra)
END(mfpr_pcbb)

LEAF(mfpr_ptbr)
	call_pal PAL_mfpr_ptbr
	ret	zero,(ra)
END(mfpr_ptbr)

LEAF(mfpr_vptb)
	call_pal PAL_mfpr_vptb
	ret	zero,(ra)
END(mfpr_vptb)


/*
 * Store quadword physical stqp(paddr,value)
 */
LEAF(stqp)
	call_pal PAL_stqp
	ret     zero,(ra)
END(stqp)

/*
 * Load quadword physical ldqp(paddr)
 */
LEAF(ldqp)
	call_pal PAL_ldqp
	ret     zero,(ra)
END(ldqp)

/*
 * Switch to the osfpalcode environment
 */
NESTED(switch_env,EF_SIZE,ra)
	.mask   EXCMASK|M_EXCFRM,-(EF_SIZE-(EF_RA*8))

	lda	sp,-EF_SIZE(sp)
	stq     ra,EF_RA*8(sp)
	.set noat
	stq     AT,EF_AT*8(sp)
	.set at
	stq	gp,EF_GP*8(sp)

	bsr	ra,reg_save

	/*
	 * Store KSP into new PCB
	 * An assumption is made about PCB layout here;
	 * specifically, KSP is first field in PCB.
	 */
	bis	a1,zero,s1	# save incoming a1
	bis	a0,zero,s0	# save copy of a0 (physical addr of new HWPCB)

	bis	sp,zero,a1
	call_pal PAL_stqp

	/*
	 * Set up parameters for SWPPAL call:
	 *
	 *   a0 = PALvar_OSF1
	 *   a1 = new PC
	 *   a2 = new PCBB
	 *   a3 = new VPTB
	 *   a4 = new KSP (per earlier version of ECO)
	 */
	bis	sp,zero,a4
	bis	s1,zero,a3	# VPTB
	bis	s0,zero,a2	# HWPCB
	br	a1,1f		# return PC is next instruction
	br	zero,2f

1:
	ldiq	a0,2		# OSF1 PAL variant

	ornot	zero,zero,v0	# -1, assume failure

	call_pal SWPPAL

2:
	bsr	ra,reg_restore

	ldq	gp,EF_GP*8(sp)
	.set    noat
	ldq     AT,EF_AT*8(sp)
	.set    at
	ldq     ra,EF_RA*8(sp)
	lda	sp,EF_SIZE(sp)
	ret	zero,(ra)
END(switch_env)


LEAF(tbis)
	bis     a0,zero,a1
	ldiq    a0,3
	call_pal PAL_tbi
	ret	zero,(ra)
END(tbis)
