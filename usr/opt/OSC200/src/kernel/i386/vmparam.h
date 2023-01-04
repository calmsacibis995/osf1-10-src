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
 *	@(#)$RCSfile: vmparam.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:20:33 $
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
 
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

/*
 * Machine dependent constants for I386
 */
/*
 * USRTEXT is the start of the user text/data space, while USRSTACK
 * is the top (end) of the user stack.  LOWPAGES and HIGHPAGES are
 * the number of pages from the beginning of the P0 region to the
 * beginning of the text and from the beginning of the P1 region to the
 * beginning of the stack respectively.
 */
#define USRTEXT		0
#define USRDATA		USRTEXT

#ifdef	KERNEL
#ifndef	ASSEMBLER
extern	int sigcode[5];/* must agree with sigcode declaration in kern_exec*/
#define SIGCODE_SIZE	(sizeof (sigcode))
#endif	ASSEMBLER
#endif	KERNEL

#define USRSTACK	(0xc0000000-sizeof(int [5]))	/* must agree with sigcode above */

/*
 * Virtual memory related constants, all in bytes
 */
#ifndef	MAXTSIZ
#define MAXTSIZ		(6*1024*1024)		/* max text size */
#endif
#ifndef	DFLDSIZ
#define DFLDSIZ		(6*1024*1024)		/* initial data size limit */
#endif
#ifndef	MAXDSIZ
#define MAXDSIZ		(16*1024*1024)		/* max data size */
#endif
#ifndef	DFLSSIZ
#define DFLSSIZ		(512*1024)		/* initial stack size limit */
#endif
#ifndef	MAXSSIZ
#define MAXSSIZ		MAXDSIZ			/* max stack size */
#endif
