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
static char	*sccsid = "@(#)$RCSfile: access.c,v $ $Revision: 1.2 $ (DEC) $Date: 1992/01/15 01:11:04 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/* 
 * derived from access.c	2.1	(ULTRIX/OSF)	12/3/90";
 */

/*
 * Adb: access data in file/process address space.
 *
 * The routines in this file access referenced data using
 * the maps to access files, ptrace to access subprocesses,
 * or the system page tables when debugging the kernel,
 * to translate virtual to physical addresses.
 */


#include <hal/kdb/defs.h>


string_t	errflg;

/*
 * Primitives: put a value in a space, get a value from a space
 * and get a word or byte not returning if an error occurred.
 */
put(addr, space, value)
long	addr, value;
{
	(void) access(WT, addr, space, value);
}

u_int
get(addr, space)
long	addr;
{
	return (access(RD, addr, space, 0));
};

u_int
chkget(addr, space)
long	addr;
{
	u_int w = get(addr, space);
	chkerr();
	return(w);
}

u_int
bchkget(addr, space)
long	addr;
{
	return(chkget(addr, space) & 0377);
}

/*
 * Read/write according to mode at address addr in i/d space.
 * Value is quantity to be written, if write.
 *
 */
access(mode, addr, space, value)
int mode, space, value;
long	addr;
{
	int rd = mode == RD;
	int	w;

	if (space == NSP)
		return(0);
	w = 0;

	if (kdbreadwrite(curmap, addr, rd ? &w : &value, rd) < 0)
		rwerr(space);
	return (w);
}


rwerr(space)
int space;
{

	if (space & DSP)
		errflg = "data address not found";
	else
		errflg = "text address not found";
}


within(addr,lbd,ubd)
u_int addr, lbd, ubd;
{
	return(addr>=lbd && addr<ubd);
}


