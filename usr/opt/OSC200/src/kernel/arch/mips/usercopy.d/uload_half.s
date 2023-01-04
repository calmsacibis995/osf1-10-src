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
 * @(#)$RCSfile: uload_half.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 16:01:31 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>




/*
 * int uload_half(addr, pword)
 * u_int addr, *pword;
 */
LEAF(uload_half)
	li	a2,PCB_WIRED_ADDRESS
#ifdef ASSERTIONS
	lw	v0,PCB_NOFAULT(a2)
	beq	v0,zero,8f
	PANIC("recursive nofault")
8:
#endif
	.set	noreorder
	li	v0,NF_FIXADE
	sw	v0,PCB_NOFAULT(a2)
	.set	reorder
	ulh	v1,0(a0)
	.set	noreorder
	sw	zero,PCB_NOFAULT(a2)
	.set	reorder
	sw	v1,0(a1)
	move	v0,zero
	j	ra
	END(uload_half)

