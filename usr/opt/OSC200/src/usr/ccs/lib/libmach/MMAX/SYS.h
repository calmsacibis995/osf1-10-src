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
 *	@(#)$RCSfile: SYS.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:10:09 $
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

/* SYS.h 4.1 83/05/10 */

#include <syscall.h>
/*
 * Macros to implement system calls
 */

#ifdef PROF
#define	ENTRY(x) \
				.data ;\
				.bss .P/**/x,4,4 ;\
				.text ;\
				.globl _/**/x ;\
			_/**/x:	addr	.P/**/x,r0 ;\
				jsr mcount ;
#else
#define	ENTRY(x) \
				.globl _/**/x ;\
			_/**/x: ;
#endif PROF

#define	SYSCALL(x) \
				.globl cerror ;\
			.L/**/x:	jump cerror ;\
			ENTRY(x) ;\
				addr	@SYS_/**/x,r0 ;\
				addr	4(sp),r1 ;\
				svc ;\
				bcs	.L/**/x ;

#define	PSEUDO(x,y) \
				.globl cerror ;\
			.L/**/x:	jump cerror ;\
			ENTRY(x) ;\
				addr @SYS_/**/y,r0 ;\
				addr 4(sp),r1 ;\
				svc ;\
				bcs .L/**/x ;
