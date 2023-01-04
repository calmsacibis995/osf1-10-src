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
 *	@(#)$RCSfile: mode.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:25 $
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

/*	mode.h	4.2	81/05/14	*/

#include <i386/kdb/machine.h>
/*
 * sdb/adb - common definitions for old srb style code
 */

#define MAXCOM	64
#define MAXARG	32
#define LINSIZ	512

typedef	short	INT;
typedef	int		VOID;
typedef	char		BOOL;
typedef	char		*string_t;
typedef	char		msg[];
typedef	struct map	map;
typedef	map		*map_t;
typedef	struct bkpt	bkpt;
typedef	bkpt		*bkpt_t;


/* file address maps */
struct map {
	long	b1;
	long	e1;
	long	f1;
	long	b2;
	long	e2;
	long	f2;
	INT	ufd;
};

struct bkpt {
	long	loc;
	long	ins;
	INT	count;
	INT	initcnt;
	INT	flag;
	char	comm[MAXCOM];
	bkpt	*nxtbkpt;
};

typedef	struct reglist	REGLIST;
typedef	REGLIST		*REGPTR;
struct reglist {
	string_t	rname;
	INT	roffs;
	int	*rkern;
};
