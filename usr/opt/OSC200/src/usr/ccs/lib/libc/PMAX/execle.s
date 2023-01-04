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
 *	@(#)$RCSfile: execle.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:12:01 $
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

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

#include <mips/regdef.h>
#include <mips/asm.h>
#include <syscall.h>

FRMSIZE	=	20

NESTED(execle, FRMSIZE, zero)
	subu	sp,FRMSIZE
	sw	ra,FRMSIZE-4(sp)
	sw	a1,FRMSIZE+4(sp)
	sw	a2,FRMSIZE+8(sp)
	sw	a3,FRMSIZE+12(sp)
	move	a1,sp
	addu	a1,FRMSIZE+4
	move	a2,a1
1:	lw	v0,0(a2)
	addu	a2,4
	bne	v0,zero,1b
	lw	a2,0(a2)
	jal	execve
	lw	ra,FRMSIZE-4(sp)
	addu	sp,FRMSIZE
	RET		# execle(file, arg1, arg2, ..., 0, env);
.end execle
