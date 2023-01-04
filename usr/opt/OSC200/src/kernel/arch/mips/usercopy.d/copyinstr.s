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
 * @(#)$RCSfile: copyinstr.s,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/06/03 11:23:51 $
 */

#include <machine/machparam.h>
#include <machine/cpu.h>
#include <machine/asm.h>
#include <machine/reg.h>
#include <machine/regdef.h>
#include <sys/errno.h>
#include <assym.s>





/*
 * Copy a null terminated string from the user address space into
 * the kernel address space.
 *
 * copyinstr(user_src, kernel_dest, maxlength, &lencopied)
 *	returns:
 *		0	- success
 *		EFAULT	- user_src not accessable
 *		ENOENT	- string exceeded maxlength
 */
LEAF(copyinstr)
/*	bltz	a0,cstrerror		# user_src must be in kuseg */

	bgez	a0,3f
	j	cstrerror
3:
	addiu	sp,sp,-4
	sw	ra,0(sp)
	li	v1,PCB_WIRED_ADDRESS
#ifdef ASSERTIONS
	lw	v0,PCB_NOFAULT(v1)
	beq	v0,zero,2f
	PANIC("recursive nofault")
2:
#endif

	.set	noreorder
	li	v0,NF_COPYSTR
	sw	v0,PCB_NOFAULT(v1)	# prepare for the worst
	.set	reorder
	jal	copystr
	li	v1,PCB_WIRED_ADDRESS
	sw	zero,PCB_NOFAULT(v1)

	lw	ra,0(sp)
	addiu	sp,sp,4
1:
	j	ra
	END(copyinstr)

