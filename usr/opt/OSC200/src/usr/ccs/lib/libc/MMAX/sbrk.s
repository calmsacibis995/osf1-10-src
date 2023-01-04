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
 *	@(#)$RCSfile: sbrk.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:06:15 $
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
/* sbrk.c 4.2 83/07/26 */

#include "SYS.h"

#define SYS_brk	17

	.globl	_end
	.text

ENTRY(sbrk)
	addd	curbrk,SP(4)
	addr	SP(4),r1
	addr	@SYS_brk,r0
	svc
	bcs 	.Lerr
	movd	curbrk,r0
	movd	SP(4),curbrk
	EXIT
	ret	$0

.Lerr:
	jump	cerror

	.data
	.globl	curbrk
curbrk:	.double	_end
