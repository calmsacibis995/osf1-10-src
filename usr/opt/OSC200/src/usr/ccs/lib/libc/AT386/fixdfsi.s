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
 *	@(#)$RCSfile: fixdfsi.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:47:51 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright(c) 1989 Robert V. Baron
 */

#include <machine/asm.h>

ENTRY(__fixdfsi)		/ df at 04(%esp) for 8 bytes
	subl	$4, %esp	/ df at 08(%esp)
	 subl	$4, %esp	/ df at 0c(%esp); ret at 8(%esp); rval at 4(%esp); cw at 0(%esp)
	 fstcw	(%esp)		
	  pushl	(%esp)		/ make copy
	  orl	$0xc00, (%esp)
	  fldcw	(%esp)		/ truncate
	  addl	$4, %esp	/ flush change
	fldl	0x0c(%esp)
	fistpl	4(%esp)
	 fldcw	(%esp)		/ restore
	 addl	$4, %esp
	popl	%eax
	ret
