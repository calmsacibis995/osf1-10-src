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
 *	@(#)$RCSfile: setjmp.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:38:51 $
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
/*		4.1	setjmp.h	*/

/*
 * Copyright 1985 by MIPS Computer Systems, Inc.
 */

/*
 * jmp_buf indices
 */
#define	JB_PC		0
#define	JB_SP		1
#define	JB_FP		2
#define	JB_S0		3
#define	JB_S1		4
#define	JB_S2		5
#define	JB_S3		6
#define	JB_S4		7
#define	JB_S5		8
#define	JB_S6		9
#define	JB_S7		10

#define	JB_SIZE		11

#ifndef LOCORE
typedef int jmp_buf[JB_SIZE];	/* caller saved regs, sp, pc */
#endif
