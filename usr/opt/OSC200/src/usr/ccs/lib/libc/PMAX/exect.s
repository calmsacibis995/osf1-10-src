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
 *	@(#)$RCSfile: exect.s,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/03/03 12:59:39 $
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
/* $Header: /usr/sde/alpha/rcs/alpha/src/./usr/ccs/lib/libc/PMAX/exect.s,v 4.2.3.4 1992/03/03 12:59:39 Al_Delorey Exp $ */

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/*
 * Don Anderson  11/4/91  Make code "piciefiable"
 *
 */
#include <mips/regdef.h>
#include <mips/asm.h>
#include <syscall.h>

.globl exect
.ent   exect
exect:
	.frame	sp, 16, ra		/* pseudo frame for picie */
	subu	sp, 16
	li	a0,2
	la	a1,errmsg
	li	a2,22
	li	v0,SYS_write
	syscall
	jal	abort
	addu	sp, 16
.end exect

.rdata
errmsg:
	.ascii	"exect not implemented\n"
