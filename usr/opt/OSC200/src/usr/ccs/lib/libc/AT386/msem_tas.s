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
 *	@(#)$RCSfile: msem_tas.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:51:23 $
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


/ Mutex implementation for i386

        .text

/	int
/	_msem_tas(lock)
/	int	*lock;
/ Returns the value of the lock variable.

	.globl	__msem_tas
__msem_tas:
	push	%ebp
	movl	%esp,%ebp			/ Establish a frame
	push	%edi
	xorl	%eax, %eax			/ clear return register
	movl	0x08(%ebp),%edi
        bts	$0,0(%edi)			/ Test and set lock bit
/ The correct instruction here is "setnc" but gas knows it as "setae"
	setae	%al				/ C=1 then didn't get it
	leave					/ Return value in eax
	ret

