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
 *	@(#)$RCSfile: vmparam.h,v $ $Revision: 1.2.3.3 $ (DEC) $Date: 1992/03/18 16:03:28 $
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 *	@(#)vmparam.h	7.1 (Berkeley) 6/5/86
 */
#ifndef	_MACHINE_VMPARAM_H_
#define	_MACHINE_VMPARAM_H_ 1


/*
 * Machine dependent constants for MIPS
 */

/*
 * USRTEXT is the start of the user text, USRDATA is the start of the
 * user data, and USRSTACK is the top (end) of the user stack.  
 * LOWPAGES is the number of pages from the beginning of the text region
 * to the beginning of text.
 * HIGHPAGES is the number of pages from the beginning of the stack region
 * to the beginning of the stack.
 */

#define	USRTEXT		0x400000	/* user text starts at 4 MB */
#define USRDATA		0x10000000	/* user data starts at 256 MB */
#define	EA_SIZE		32		/* EMULATE_AREA size */
#ifndef ASSEMBLER
#define	USRSTACK	(((vm_offset_t)0x80000000)-NBPG)/* Top of user stack */
#else
#define	USRSTACK	(0x80000000-NBPG)/* Top of user stack */
#endif	/* !ASSEMBLER */
#define EMULATE_AREA	USRSTACK-EA_SIZE/* area for bp emulation */

/*
 * Virtual memory related constants, all in bytes
 * Note: only used if not specified in the kernel
 *	 config file (maxdsiz, maxssiz, dfldsiz, & dflssiz).
 */
#ifndef DFLDSIZ
#define	DFLDSIZ		(32*1024*1024)		/* initial data size limit */
#endif
#ifndef MAXDSIZ
#define	MAXDSIZ		(32*1024*1024)		/* max data size */
#endif
#ifndef	DFLSSIZ
#define	DFLSSIZ		(1024*1024)		/* initial stack size limit */
#endif
#ifndef	MAXSSIZ
#define	MAXSSIZ		MAXDSIZ			/* max stack size */
#endif


#endif	/* _MACHINE_VMPARAM_H_ */
