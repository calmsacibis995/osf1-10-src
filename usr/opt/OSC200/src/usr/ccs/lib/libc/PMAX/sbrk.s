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
 *	@(#)$RCSfile: sbrk.s,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1992/03/03 13:00:06 $
 */ 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/ccs/lib/libc/PMAX/sbrk.s,v 4.2.3.2 1992/03/03 13:00:06 Al_Delorey Exp $ */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

#include <mips/regdef.h>
#include <mips/asm.h>
#include <syscall.h>

#define	SYS_brk		17

	.globl	_end
	.globl	_minbrk
	.globl	_curbrk

/*.sdata*/.data
_minbrk:.word	_end
_curbrk:.word	_end

.text

LEAF(sbrk)
	lw	v1,_curbrk
	addu	a0,v1
	li	v0,SYS_brk
	syscall
	bne	a3,zero,err
	move	v0,v1			# return previous curbrk
	sw	a0,_curbrk		# update to new curbrk
	RET

err:
	j	_cerror
.end sbrk
