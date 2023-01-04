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
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: access.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:10:26 $";
#endif 
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
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */

/*
 * Adb: access data in file/process address space.
 *
 * The routines in this file access referenced data using
 * the maps to access files, ptrace to access subprocesses,
 * or the system page tables when debugging the kernel,
 * to translate virtual to physical addresses.
 */



#include <sys/types.h>
#include <vm/vm_map.h>

extern	char		*kdberrflg;
extern	vm_map_t	 kdbcurmap;
extern	int		 kdbchkerr();

/*
 * Primitives: put a value in a space, get a value from a space
 * and get a word or byte not returning if an error occurred.
 */

kdbput(addr, value)
long	addr, value;
{
	if (kdbreadwrite(kdbcurmap, addr, &value, 0) < 0)
		kdberrflg = "can not change data";
}

u_int
kdbget(addr)
long	addr;
{
	int	w = 0;

	if (kdbreadwrite(kdbcurmap, addr, &w, 1) < 0)
		kdberrflg = "can not find address";
	return (w);
};

u_int
kdbchkget(addr)
long	addr;
{
	u_int w = kdbget(addr);
	kdbchkerr();
	return(w);
}

u_int
kdbbchkget(addr)
long	addr;
{
	u_int w = kdbget(addr);
	kdbchkerr();
	return(w&0xff);
}
