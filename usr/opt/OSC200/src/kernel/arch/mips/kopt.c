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
static char	*sccsid = "@(#)$RCSfile: kopt.c,v $ $Revision: 1.2.4.2 $ (DEC) $Date: 1992/04/14 18:07:01 $";
#endif 
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
 * kopt.c
 *
 * Modification History:
 *
 *  8-Oct-91	Philip Cameron
 *	Removed ult_bin_isatty_fix since it is no longer needed.
 *
 *  2-Jul-91	Jim McGinness
 *	Change name "ult_bin" to "ult_bin_isatty_fix", which is now
 *	a configurable parameter.
 *
 * 21-Jan-91	Fred Canter
 *	Hack for ULTRIX isatty() binary compatibility.
 *
 */

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

#include "bin_compat.h"
#include "_lmf_.h"


#ifdef	notdef
#include <vme.h>
#endif
#include <mach_kdb.h>

#include <sys/param.h>
#include <kern/xpr.h>

#if XPR_DEBUG
extern unsigned int xprflags;
#endif /* XPR_DEBUG */
extern int askme;

extern unsigned zalloc_physical;

#if	MACH_KDB
extern int use_kdb, kdb_enable();
#endif	/* MACH_KDB */

extern int memlimit;
extern int bufpages, nbuf;

/* matching ifdef below for vme & MSERIES; fix conf/PMAX/files */
#ifdef	notdef
#if	NVME > 0
extern int causeecc, doecc();
extern int eccdelay;
#endif	/* NVME > 0 */
#ifdef	MSERIES
extern int scatgath;
extern int disk_cache;
extern int ovlapseeks;
#endif	/* MSERIES */
#endif	/* notdef */

int led_period;
extern int boothowto;
extern int pmap_debug, trap_debug;
extern int syscalltrace;
extern int tlbpid_recycle_fifo, tlbpid_flushes;
extern int do_virtual_tables;
extern int page_size;
#if BIN_COMPAT
extern int bin_compat_debug;
extern int bin_compat_trace;
#endif /*BIN_COMPAT*/
#if _LMF_
extern int lmf_debug;
extern int sysinfo_debug;
#endif /*_LMF_*/

/*
 * The following is a table of symbolic names and addresses of kernel
 * variables which can be tuned to alter the performance of the system.
 * They can be modified at boot time as a boot parameter or by the mipskopt
 * system call.  Variables marked as readonly can't be modifed after system
 * boot time (i.e. through the mipskopt call).  "func" is called after the
 * variable is set in case there is processing beyond storing the new value.
 */

struct kernargs kernargs[] = {
#if XPR_DEBUG
	{ "xprflags",	(int *)&xprflags,	0,	NULL },
#endif /* XPR_DEBUG */
	{ "askme",	&askme,			0,	NULL },
	{ "bufpages",	&bufpages,		1,	NULL },
	{ "nbuf",	&nbuf,			1,	NULL },
	{ "memlimit",	&memlimit,		1,	NULL },
	{ "pmap_debug", &pmap_debug,		0,	NULL },
	{ "trap_debug", &trap_debug,		0,	NULL },
	{ "syscalltrace", &syscalltrace,	0,	NULL },
	{ "led_period",	&led_period,		0,	NULL },
	{ "boothowto",	&boothowto,		0,	NULL },
	{ "tlbpid_recycle_fifo",&tlbpid_recycle_fifo,	0,	NULL },
	{ "tlbpid_flushes", &tlbpid_flushes,	0,	NULL },
	{ "do_virtual_tables", &do_virtual_tables, 1,	NULL },
	{ "page_size",	&page_size,		1,	NULL },
	{ "zalloc_physical", (int*)&zalloc_physical,	1,	NULL },
#if	MACH_KDB
	{ "use_kdb",	&use_kdb,		0,	kdb_enable },
#endif	/* MACH_KDB */
/* need to remove entries in conf/PMAX/files */
#ifdef	notdef
#if	NVME > 0
	{ "causeecc",	&causeecc,		0,	doecc },
	{ "eccdelay",	&eccdelay,		0,	NULL },
#endif	/* NVME > 0 */
#ifdef	MSERIES
	{ "scatgath",	&scatgath,		0,	NULL },
	{ "disk_cache",	&disk_cache,		0,	NULL },
	{ "ovlapseeks",	&ovlapseeks,		0,	NULL },
#endif	/* MSERIES */
#endif	/* notdef */
#if BIN_COMPAT
	{ "bin_compat_debug",	&bin_compat_debug,		0,	NULL },
	{ "bin_compat_trace",	&bin_compat_trace,		0,	NULL },
#endif /*BIN_COMPAT*/
#if _LMF_
	{ "lmf_debug",		&lmf_debug,	0,	NULL },
	{ "sysinfo_debug",	&sysinfo_debug,	0,	NULL },
#endif /*_LMF_*/
	{ NULL,		NULL,			1,	NULL }
};
