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
 * Some of the code runs in only one mode, some runs in either!!
 *
 */

#include <machine/regdef.h>
#include <machine/reg.h>
#include <machine/asm.h>

/*
 * Note: Following is really based on pagesize
 */
#define KSEGBASE	0xfffffc0000000000
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


/*
 * halt entry point
 */
LEAF(stop)
XLEAF(halt)
1:	call_pal PAL_halt
	br	ra,1b
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
	lda	sp,-EF_SIZE(sp)
	stq     ra,EF_RA*8(sp)
	.set noat
	stq     AT,EF_AT*8(sp)
	.set at
	stq	gp,EF_GP*8(sp)
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
	ldq	gp,EF_GP*8(sp)
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
	ret	zero,(ra)
	END(reg_restore)

LEAF(imb)
	call_pal PAL_imb
	ret	zero,(ra)
END(imb)

#ifdef SECONDARY

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

	.data
	.globl palmode
palmode: .quad 0
	.text
/*
 * Store quadword physical stqp(paddr,value)
 */
LEAF(stqp)
	ldq	t0,palmode
	beq	t0,1f		# if 0 vms else osf
	ldiq	t0,KSEGBASE
	addq	t0,a0,t1
	stq	a1,0(t1)
	ret     zero,(ra)

1:	call_pal PAL_stqp
	ret     zero,(ra)
END(stqp)

/*
 * Load quadword physical ldqp(paddr)
 */
LEAF(ldqp)
	ldq	t0,palmode
	beq	t0,1f		# if 0 vms else osf
	ldiq	t0,KSEGBASE
	addq	t0,a0,t1
	ldq	v0,0(t1)
	ret     zero,(ra)

1:	call_pal PAL_ldqp
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

/* Support for the host-to-network and network-to-host conversions, 
 * needed by netload.
 * These should be identical to the nuxi_16 and nuxi_32 routines 
 * in arch/alpha/byte_swap.s
 */

/*
 * nuxi_s(word)	Swap bytes in a word
 */
LEAF(nuxi_s)				# a0 = ??ab
	  extbl   a0,1,t0		# t0 =    a
	  extbl   a0,0,t1		# t1 =    b
	  sll     t1,8,t1		# t1 =   b0
	  or      t1,t0,v0		# v0 =   ba
	  ret     zero,(ra)
END(nuxi_s)

/*
 * nuxi_32(longword)	Swap bytes in a word
 */
LEAF(nuxi_l)				# a0 = abcd
	  extbl   a0,1,t0		# t0 =    c
	  extbl   a0,0,t1		# t1 =    d
	  sll     t1,24,t1		# t1 = d000
	  sll	  t0,16,t0		# t0 =  c00
	  or      t1,t0,v0		# v0 = dc00
	  extbl	  a0,2,t0		# t0 =    b
	  sll     t0,8,t0		# t0 =   b0
	  or	  t0,v0,v0		# v0 = dcb0
	  extbl   a0,3,t0		# t0 =    a
	  addl	  t0,v0,v0		# v0 = dcba (canonicalize too)
	  ret     zero,(ra)
END(nuxi_l)
#endif /* SECONDARY */
