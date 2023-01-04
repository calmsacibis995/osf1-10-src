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
	.asciiz "@(#)$RCSfile: spl.s,v $ $Revision: 1.1.13.2 $ (DEC) $Date: 1993/06/07 20:25:52 $"
	.text

#include <rt_preempt.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/regdef.h>
#include "assym.s"

LEAF(swap_ipl)
	/* Always at SPLEXTREME until enable_spls() is called 
	 * and replaces the ldiq with call_pal PAL_swpipl
	 */
	ldiq	v0,SPLEXTREME	 /* replaced with call_pal PAL_swpipl */
	ret     zero,(ra)
	END(swap_ipl)

LEAF(kdebug_ipl)
	call_pal PAL_swpipl
	ret     zero,(ra)
	END(kdebug_ipl)

/*
 * getspl
 */
LEAF(getspl)
XLEAF(mfpr_ipl)
	call_pal PAL_rdps		/* in kernel mode ps is ipl!! */
	ret	zero,(ra)
	END(getspl)

#if     RT_PREEMPT
/*
 * iplis0: test if IPL is at lowest level (all interrupts enabled)
 */
	.align 3
LEAF(iplis0)
	nop			# try to dual-issue after return from PAL
	call_pal PAL_rdps
	cmpeq	v0,0,v0		# return v0 ? 0 : 1;
	ret	zero,(ra)
	END(iplis0)
#endif	/* RT_PREEMPT */

