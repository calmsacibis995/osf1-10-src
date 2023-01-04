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
 *	@(#)$RCSfile: SYS.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 02:57:15 $
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
/* SYS.h 4.1 83/05/10 */

#include <syscall.h>

/*
 * Macros to implement system calls
 */

#ifdef PROF
#define	ENTRY(x) \
				.data ;\
				.bss .P ## x,4,4 ;\
				.text ;\
				.globl _ ## x ;\
			_ ## x:	enter [],$0 ;\
				addr	.P ## x,r0 ;\
				jsr mcount ;
#else
#define	ENTRY(x) \
				.globl _ ## x ;\
			_ ## x: ;
#endif PROF

#ifdef PROF
#define EXIT \
				exit[] ;
#else
#define EXIT
#endif

#ifdef PROF
#define SP(x)	x+4(sp)
#else
#define SP(x)	x(sp)
#endif

#define SVC(x) \
				addr	@SYS_ ## x,r0 ;\
				addr	SP(4),r1 ;\
				svc ;


#define	SYSCALL(x) \
				PSEUDO(x,x) \
				EXIT;

#define	PSEUDO(x,y) \
				.globl cerror ;\
			.L ## x:	jump cerror ;\
			ENTRY(x) ;\
				SVC(y) ;\
				bcs .L ## x ;

#define PSEUDO1(x)	PSEUDO(x,x)
