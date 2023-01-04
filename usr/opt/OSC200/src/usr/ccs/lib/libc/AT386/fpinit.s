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
 *	@(#)$RCSfile: fpinit.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:48:07 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 *  File:         fpinit.s
 *  Description:  Reset floating point coprocessor.
 * 
 *  Copyright Ing. C. Olivetti & C. S.p.A. 1989
 *  All rights reserved.
 */                                                                       

/*
 * OSF/1 Release 1.0
 */

#include <machine/asm.h>
#include <machine/fpreg.h>


/* 
 * Reset the floating point coprocessor to the same state that the 
 * kernel initializes it to.  This code should track the 
 * initialization code in i386/fpsup.c.  
 * Notice that we use a local variable instead of a global for setting
 * the control word.  This is so that we don't have to rewrite this 
 * proc for use in a multi-threaded world.
 */

ENTRY(_fpinit)
	FRAME
	subl	$4,%esp			/ diddle with control word here

	/*
	 * must allow invalid operation, zero divide, and
	 * overflow interrupt conditions and change to use
	 * long real precision
	 */
	fninit
	fstcw 	-4(%ebp)
#ifdef	wheeze
	andl	$-1![FPINV|FPZDIV|FPOVR|FPPC],-4(%ebp)
#else
	andl	$-1^(FPINV|FPZDIV|FPOVR|FPPC),-4(%ebp)
#endif
	orl	$(FPSIG53|FPIC),-4(%ebp)
	fldcw	-4(%ebp)

	leave
	ret
