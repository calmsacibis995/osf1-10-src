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
 * @(#)$RCSfile: fubyte.s,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 11:28:38 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>




LEAF(fubyte)
XLEAF(fuibyte)
	li	a1,PCB_WIRED_ADDRESS
#ifdef ASSERTIONS
	lw	v0,PCB_NOFAULT(a1)
	beq	v0,zero,2f
	PANIC("recursive nofault")
2:
#endif
	.set	noreorder
	bgez	a0,3f
	li	v0,NF_FSUMEM		# BDSLOT
	j	uerror
	nop
3:
	sw	v0,PCB_NOFAULT(a1)
	lbu	v0,0(a0)
	sw	zero,PCB_NOFAULT(a1)	# LDSLOT
	.set	reorder
	j	ra
	END(fubyte)

