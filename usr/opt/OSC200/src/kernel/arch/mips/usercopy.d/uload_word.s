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
 * @(#)$RCSfile: uload_word.s,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/03/18 16:02:03 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>




/*
 * The following routines uload_word(), uload_half(), uloaduhalf(),
 * ustore_word() and ustore_half() load and store unaligned items.
 * The "addr" parameter is the address at which the reference is to be
 * made.  For load routines the value is returned indirectly through
 * the "pword" parameter.  For store routines the "value" pramameter
 * is stored.  All routines indicate an error by returning a non-zero
 * value.  If no error occurs a zero is returned.
 */

/*
 * int uload_word(addr, pword)
 * u_int addr, *pword;
 */
LEAF(uload_word)
	li	a2,PCB_WIRED_ADDRESS
	lw	v0,PCB_NOFAULT(a2)
	beq	v0,zero,8f
	PANIC("recursive nofault")
8:
	.set	noreorder
	li	v0,NF_FIXADE
	sw	v0,PCB_NOFAULT(a2)
	ulw	v1,0(a0)
	sw	zero,PCB_NOFAULT(a2)
	.set	reorder
	sw	v1,0(a1)
	move	v0,zero
	j	ra
	END(uload_word)

