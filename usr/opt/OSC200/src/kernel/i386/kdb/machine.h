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
 *	@(#)$RCSfile: machine.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:18 $
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

/*	machine.h	4.1	81/05/14	*/

#include <sys/vm.h>

#define PAGSIZ		(NBPG*CLSIZE)

#define DBNAME "kdb"

#define LPRMODE "%R"
#define OFFMODE "+%R"
#define TXTRNDSIZ	PAGSIZ

#define MAXINT	0x7fffffff
#define MAXSTOR ((1L<<31) - ctob(UPAGES))
#define MAXFILE 0xffffffff

/*
 * INSTACK tells whether its argument is a stack address.
 * INUDOT tells whether its argument is in the (extended) u. area.
 * These are used for consistency checking and dont have to be exact.
 *
 * INKERNEL tells whether its argument is a kernel space address.
 * KVTOPH trims a kernel virtal address back to its offset
 * in the kernel address space.
 */
#define INSTACK(x)	(((x)&0xf0000000) == 0x70000000)
#define INUDOT(x)	(((x)&0xf0000000) == 0x70000000)
#define INKERNEL(x)	(((x)&0xf0000000) == 0x80000000)

#define KVTOPH(x)	((x)&~ 0x80000000)
#define KERNOFF		0x80000000
