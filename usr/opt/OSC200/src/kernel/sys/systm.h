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
 *	@(#)$RCSfile: systm.h,v $ $Revision: 4.3.11.5 $ (DEC) $Date: 1993/06/25 22:45:36 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#ifndef	_SYS_SYSTM_H_
#define _SYS_SYSTM_H_

/*
 * Structure of the system-entry table
 */
#define NUMSYSCALLARGS 8
struct sysent
{
	short	sy_narg;			/* total number of arguments */
	short	sy_parallel;			/* can execute in parallel */
	int	(*sy_call)();			/* handler */
	unsigned int aud_param[NUMSYSCALLARGS+1];
					/* audit encodings (see audit.h) */
};


/*
 * Compatability module control block
 */
#define MAXCOMPATNAMSZ  16
#define MAXSTANZANAMSZ  16
#define MAXREVSZ	16
struct compat_mod {
	struct compat_mod * cm_next;		/* next in chain	*/
	char		cm_name[MAXCOMPATNAMSZ];/* name of this module	*/
	char		cm_ld_name[MAXSTANZANAMSZ];/* name stanza for module */
	char		cm_rev[MAXREVSZ];	/* revision in ascii	*/
	int		cm_revision;		/* revision number	*/
	int		(* cm_configure)();	/* config entry		*/
	struct compat_mod *(* cm_recognizer)(); /* called by exec	*/
	struct sysent   *(* cm_syscall)();	/* returns needed sysent */
	char		**call_name;		/* list of func names	*/
	int		cm_habitat;		/* habitat number	*/
	int		cm_base;		/* first sysent number	*/
	int		cm_nsysent;		/* number of sys calls	*/
	int		cm_refcount;		/* reference count	*/
	int		cm_totalcount;		/* usage count		*/
	int		cm_skipcount;		/* skip count		*/
	int		cm_nsyscalls;		/* number of traced svcs*/
	int		cm_flags;		/* control flags	*/
	unsigned int	*cm_auditmask;		/* module auditmask	*/
	int		*cm_stats;		/* statistics vector	*/
	char		*cm_trace;		/* Trace flag vector	*/
};
/* valid module names */
#define ULTBIN		"Ultrix4.2"
/* valid revisions */
#define ULT42V11	0x0101	/* 1.1 */
#define ULT42V11A	"1.1"
/* cm_flags */
#define	CM_DEBUG	1
#define CM_TRACE	2
#define CM_STATIC	4	/* can't unload this module */
#define CM_CONFIG	8	/* module is configured */
#define CM_TRACE_THIS	0x100	/* trace this call */


/*
 * Habitats and static modules
 *	MAXHABITATS is the maximum habitat number that is currently supported.
 *	MAXSTATICMODS is the maximum number of statically linked modules.
 */
#define MAXHABITATS	16	
#define MAXSTATICMODS	16


/*
 * Validity checking struct
 *	Each valid name and revision is listed
 */
struct cm_valid {
	char * cm_name;		/* valid name ..... */
	int	cm_rev;		/* .... and revision */
};


/*
 * Compatability module data for cfgmgr query command.
 *	This passes back trace information for one syscall at a time
 */
struct compat_query {
	int	next;		/* next non-zero count */
	int	svc;		/* svc being queried */
	int	count;		/* number of calls */
	int	trace;		/* trace flag */
	char	name[32];	/* name of the system call */
};

/*
 * Compatability module data for cfgmgr operate command.
 *	This passes trace and debug instructions
 */
struct compat_operate {
	int	on_flgs;	/* flags that are being truned on */
	int	off_flgs;	/* flags that are being turned off */
	int	skip;		/* skip count */
	char	svc[32];	/* trace svc name */
};


#ifdef	_KERNEL
#include <mach/boolean.h>
#include <sys/types.h>

/*
 * Compatability module support
 */
extern int cm_setup();
extern int cm_trace_this();
extern int cm_add();
extern int cm_del();
extern struct compat_mod *habitats[MAXHABITATS];
extern struct cm_valid cm_valid[];	/* valid modules */
extern void (* cm_static[MAXSTATICMODS])();
extern int cm_query();
extern int cm_operate();

/*
 * Random set of variables
 * used by more than one
 * routine.
 */
extern char	version[];		/* system version */

/*
 * Nblkdev is the number of entries
 * (rows) in the block switch. It is
 * set in binit/bio.c by making
 * a pass over the switch.
 * Used in bounds checking on major
 * device numbers.
 */
extern int	nblkdev;

/*
 * Number of character switch entries.
 * Set by cinit/prim.c
 */
extern int	nchrdev;

extern int	mpid;			/* generic for unique process id's */
extern char	kmapwnt;		/* Make #if cleaner */

extern int	maxmem;			/* actual max memory per process */
extern int	physmem;		/* physical memory on this CPU */

extern int	updlock;		/* lock for sync */
extern int	intstack[];		/* stack for interrupts */
extern dev_t	rootdev;		/* device of the root */
extern struct vnode *rootvp;		/* vnode of root filesystem */
extern dev_t	dumpdev;		/* device to take dumps on */
extern daddr_t	dumplo;			/* offset into dumpdev */

extern caddr_t	calloc();
extern struct sysent sysent[];
extern int	nsysent;
extern char	*panicstr;
extern long	boothowto;		/* reboot flags, from console subsys */
extern int	show_space;
extern u_long	bootdev;		/* boot dev, from bootstrap subsys */
extern int	selwait;


#ifdef	lint
/* casts to keep lint happy */
#define insque(q,p)	_insque((caddr_t)q,(caddr_t)p)
#define remque(q)	_remque((caddr_t)q)
#endif	/* lint */

#endif	/* _KERNEL */
#define FDFS_MAJOR	12		/* major # of FDFS filesystem 
					 * used by io/common/conf.c
					 * must be visible outside kernel
					*/
#endif	/* _SYS_SYSTM_H_ */
