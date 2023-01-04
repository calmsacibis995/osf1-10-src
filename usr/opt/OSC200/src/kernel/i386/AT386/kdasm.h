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
 *	@(#)$RCSfile: kdasm.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:09:18 $
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
 * Some inline code to speed up major block copies to and from the
 * screen buffer.
 * 
 * Copyright Ing. C. Olivetti & C. S.p.A. 1988, 1989.
 *  All rights reserved.
 *
 * orc!eugene	28 Oct 1988
 *
 */


#define BPW	2			/* bytes per word */

/* The BSD lint can't seem to handle these asm sources */
#ifndef	lint

/*
 * Function:	kd_slmwd()
 *
 *	This function "slams" a word (char/attr) into the screen memory using
 *	a block fill operation on the 386.
 *
 */

asm	kd_slmwd(start, count, value)

{

%mem	start;	con	count;	con	value;

	pushl	%edi
	pushl	%ecx
	pushl	%eax
	pushl	start
	popl	%edi
	movl	count, %ecx
	movw	value, %ax
	rep
	stosw
	popl	%eax
	popl	%ecx
	popl	%edi

%mem	start;	mem	count;	mem	value;

	pushl	%edi
	pushl	%ecx
	pushl	%eax
	pushl	start
	popl	%edi
	movl	count, %ecx
	movw	value, %ax
	rep
	stosw
	popl	%eax
	popl	%ecx
	popl	%edi

}


/*
 * "slam up"
 */

asm	kd_slmscu(from, to, count)

{

%mem	from;	mem	to;	mem	count;

	pushl	%esi
	pushl	%edi
	pushl	%ecx
	pushl	from
	pushl	to
	popl	%edi
	popl	%esi
	movl	count, %ecx
	cmpl	%edi, %esi
	rep
	movsw
	popl	%ecx
	popl	%edi
	popl	%esi

}


/*
 * "slam down"
 */

asm	kd_slmscd(from, to, count)

{

%mem	from;	mem	to;	mem	count;

	pushl	%esi
	pushl	%edi
	pushl	%ecx
	pushl	from
	pushl	to
	popl	%edi
	popl	%esi
	movl	count, %ecx
	cmpl	%edi, %esi
	std
	rep
	movsw
	cld
	popl	%ecx
	popl	%edi
	popl	%esi

}

#endif	/* lint */
