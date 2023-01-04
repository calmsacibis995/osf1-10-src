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
static char	*sccsid = "@(#)$RCSfile: optab.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:11:32 $";
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



#define ADB
#include <i386/kdb/defs.h>

#undef INSTTAB

#include <i386/kdb/instrs.h>

struct insttab insttab[] = {
#include <i386/kdb/instrs.adb>
	0};
#include <i386/kdb/assizetab.c>
#undef ADB

#define SYSTAB struct systab
SYSTAB {
	int	argc;
	char	*sname;
}
systab[] = {
	1, "indir",
	0, "exit",
	0, "fork",
	2, "read",
	2, "write",
	2, "open",
	0, "close",
	0, "wait",
	2, "creat",
	2, "link",
	1, "unlink",
	2, "exec",
	1, "chdir",
	0, "time",
	3, "mknod",
	2, "chmod",
	2, "chown",
	1, "break",
	2, "stat",
	2, "seek",
	0, "getpid",
	3, "mount",
	1, "umount",
	0, "setuid",
	0, "getuid",
	0, "stime",
	3, "ptrace",
	0, "alarm",
	1, "fstat",
	0, "pause",
	1, "30",
	1, "stty",
	1, "gtty",
	0, "access",
	0, "nice",
	0, "sleep",
	0, "sync",
	1, "kill",
	0, "csw",
	0, "setpgrp",
	0, "tell",
	0, "dup",
	0, "pipe",
	1, "times",
	4, "profil",
	0, "tiu",
	0, "setgid",
	0, "getgid",
	2, "signal",
	0, "49",
	0, "50",
	0, "51",
	0, "52",
	0, "53",
	0, "54",
	0, "55",
	0, "56",
	0, "57",
	0, "58",
	0, "59",
	0, "60",
	0, "61",
	0, "62",
	0, "63",
};

string_t	regname[];
string_t	fltimm[] = {
	"0.5", "0.5625", "0.625", "0.6875", "0.75", "0.8125", "0.875", "0.9375",
	"1.0", "1.125", "1.25", "1.375", "1.5", "1.625", "1.75", "1.875",
	"2.0", "2.25", "2.5", "2.75", "3.0", "3.25", "3.5", "3.75",
	"4.0", "4.5", "5.0", "5.5", "6.0", "6.5", "7.0", "7.5",
	"8.0", "9.0", "10.0", "11.0", "12.0", "13.0", "14.0", "15.0",
	"16.0", "18.0", "20.0", "22.0", "24.0", "26.0", "28.0", "30.0",
	"32.0", "36.0", "40.0", "44.0", "48.0", "52.0", "56.0", "60.0",
	"64.0", "72.0", "80.0", "88.0", "96.0", "104.0", "112.0", "120.0"
};

#ifdef	SDB
REGLIST reglist [] = {
	"p1lr", P1LR,
	"p1br",P1BR,
	"p0lr", P0LR,
	"p0br",P0BR,
	"ksp",KSP,
	"esp",ESP,
	"ssp",SSP,
	"psl", PSL,
	"pc", PC,
	"usp",USP,
	"fp", FP,
	"ap", AP,
	"r11", R11,
	"r10", R10,
	"r9", R9,
	"r8", R8,
	"r7", R7,
	"r6", R6,
	"r5", R5,
	"r4", R4,
	"r3", R3,
	"r2", R2,
	"r1", R1,
	"r0", R0,
};
#endif	SDB
