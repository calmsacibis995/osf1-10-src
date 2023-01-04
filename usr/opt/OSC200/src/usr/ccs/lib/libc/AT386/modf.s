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
 *	@(#)$RCSfile: modf.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:51:05 $
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
 * Copyright (c) 1989 Robert Victor Baron
 */
/*
 * double modf (value, iptr)
 * double value, *iptr;
 *
 * Modf returns the fractional part of "value",
 * and stores the integer part indirectly through "iptr".
 */

#include <machine/asm.h>

ENTRY(modf)
	FRAME
	pushl	%eax
	movl	16(%ebp), %eax
	fldl	8(%ebp)
	fld	%st(0)
	 subl	 $4, %esp
	 fstcw	 (%esp)		/ requires non reg.  I think this is wrong /
	 pushl	 (%esp)		/ make copy
	 orl	 $0xc00, (%esp)
	 fldcw	 (%esp)		/ round down
	frndint
	 addl	$4, %esp	/ flush change
	 fldcw	 (%esp)		/ restore
	 addl	$4, %esp
	fsubr	%st, %st(1)
	fstpl	(%eax)
	popl	%eax
	EMARF
	ret
