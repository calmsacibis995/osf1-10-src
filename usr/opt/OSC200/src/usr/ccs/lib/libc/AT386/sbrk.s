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
 *	@(#)$RCSfile: sbrk.s,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:53:36 $
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


/*
 *
 * 90/02/22 10:55 jd at osf
 *	Replaced SYS.h with machine/asm.h and syscall.h.
 *
 */

/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(SYSLIBC_SCCS) && !defined(lint)

#endif /* SYSLIBC_SCCS and not lint */

#include	<machine/asm.h>
#include	<syscall.h>

#define	SYS_brk		17


	.globl	EXT(end)
	.globl	LCL(minbrk)
	.globl	LCL(curbrk)

	.data
LCL(minbrk):	.long	EXT(end)
LCL(curbrk):	.long	EXT(end)
	.text

ENTRY(sbrk)
	movl	LCL(curbrk),%eax
	movl	4(%esp),%edx 		/arg
	addl	%edx,%eax		/arg+curbrk
	pushl	%eax
	pushl	$0			/ get stack right (return address)
	movl	$17, %eax
	SVC
	jb	EXT(cerror)
	addl	$8,%esp
	movl	LCL(curbrk),%eax
	movl	4(%esp), %edx
	addl	%edx, LCL(curbrk)
	ret
