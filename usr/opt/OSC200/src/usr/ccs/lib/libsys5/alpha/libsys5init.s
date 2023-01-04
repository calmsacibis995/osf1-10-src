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
	.asciiz "@(#)$RCSfile: libsys5init.s,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/05/22 18:11:11 $"
	.text

#include <sys/habitat.h>
#include <sys/uswitch.h>
#include <alpha/regdef.h>
#include <alpha/asm.h>
#include <syscall.h>

/*
 * System V sigaltstack function.
 * NOTE: This stub passes a hidden third argument of 0 to the kernel
 * sigaltstack() entry point. This argument is used in libsys5 to
 * assist in implement get/setcontext().
 */

LEAF(libsys5init)
	ldgp	gp, 0(pv)
	ldiq	a0,USC_SET
	ldiq	a1,(USW_SVR4|USW_NULLP)
	ldiq	v0,SYS_uswitch
	CHMK()
	bne	a3,err
	RET
err:
	br	gp,1f
1:
	ldgp	gp,0(gp)
	jmp	zero,_cerror
END(libsys5init)
