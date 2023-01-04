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
static char *rcsid = "@(#)$RCSfile: kernargs.c,v $ $Revision: 1.2.6.3 $ (DEC) $Date: 1993/10/29 20:32:38 $";
#endif
#ifndef lint
static char	*sccsid = "@(#)kernargs.c	9.2	(ULTRIX/OSF)	10/30/91";
#endif	lint

#include <sys/param.h>
#include <machine/alpha_debug.h>
#include <kern/xpr.h>

#if XPR_DEBUG
extern unsigned long xprflags;
#endif /* XPR_DEBUG */

#if	MACH_KDB
extern int use_kdb, kdb_enable();
#endif	/* MACH_KDB */

extern long askme;
extern long boothowto;
extern long bufpages;
extern long do_virtual_tables;
extern long memlimit;
extern long nbuf;
extern long netblk_ldaddr;
extern long pmap_debug;
extern long syscalltrace;
extern long trap_debug;
extern long msgbuf_size;
#ifdef MEMLOG
long memlog;
#endif /* MEMLOG */

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
	{ "xprflags",	&xprflags,	0,	NULL },
#endif /* XPR_DEBUG */
	{ "askme",	&askme,			0,	NULL },
	{ "bufpages",	&bufpages,		1,	NULL },
	{ "nbuf",	&nbuf,			1,	NULL },
	{ "memlimit",	&memlimit,		1,	NULL },
	{ "pmap_debug", &pmap_debug,		0,	NULL },
	{ "syscalltrace", &syscalltrace,	0,	NULL },
	{ "boothowto",	&boothowto,		0,	NULL },
	{ "do_virtual_tables", &do_virtual_tables, 1,	NULL },
	{ "netblk",	&netblk_ldaddr,		1,	NULL },
	{ "trap_debug", &trap_debug,		0,	NULL },
	{ "msgbuf_size", &msgbuf_size,		0,	NULL },
#if	MACH_KDB
	{ "use_kdb",	&use_kdb,		0,	kdb_enable },
#endif	/* MACH_KDB */
#ifdef	MEMLOG
	{ "memlog",	&memlog,		0,	NULL },
#endif	/* MEMLOG */
	{ NULL,		NULL,			1,	NULL }
};
