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
static char	*sccsid = "@(#)$RCSfile: display_lock.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:37:11 $";
#endif 
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
#include <sys/unix_defs.h>
#include <mmax_mp.h>
#include <mmax_mpdebug.h>
#include <lock_stats.h>

#include "mda.h"
#include <mmax_apc.h>
#include <kern/lock.h>
#include <sys/lock_types.h>

char	*truefalse();

display_lock(lp)
struct	lock *lp;
{
	char	*p, *type;
	char	buf[16], chkbuf[16];

	p = "Unknown";
	type = "Unknown";

#if	UNIX_LOCKS
	switch(lp->lock_type & 0xff) {
		case LTYPE_DEFAULT_UNI:
					type = "Default Uni";
					break;
		
		case LTYPE_FILE_IO:	type = "File IO";
		                        break;

#if 0
		case LTYPE_FILE_DESC:	type = "File Desc";
					break;

		case LTYPE_FILE_TABLE:	type = "File Table";
					break;

		case LTYPE_MACH_DIR:	type = "Mach Dir";
		                        break;

		case LTYPE_IFREELIST:	type = "Ifreelist";
		                        break;

		case LTYPE_IHASH:	type = "Ihash";
		                        break;

		case LTYPE_INODE:	type = "Inode";
					break;

		case LTYPE_MOUNT_TABLE:	type = "Mount";
					break;

		case LTYPE_BUFHASH:	type = "Bufhash";
					break;

		case LTYPE_BFREELIST:	type = "Bfreelist";
					break;

		case LTYPE_BUF:		type = "Buffer";
					break;

		case LTYPE_STRUCT_FS:	type = "Struct FS";
					break;

		case LTYPE_PTY:
		case LTYPE_SCC_TTY:	type = "Pty/SCC tty";
					break;

		case LTYPE_CFREELIST:	type = "Cfreelist";
					break;

		case LTYPE_HOSTNAME:	type = "Hostname";
					break;

		case LTYPE_CKU:		type = "CKU";
		                        break;

		case LTYPE_RENAME:	type = "Rename";
		                        break;

		case LTYPE_KLM_LOCK:	type = "Klm";
		                        break;

		case LTYPE_VFS_LIST:	type = "Vfs List";
		                        break;

		case LTYPE_INODE_IO:	type = "Inode I/O";
		                        break;

		case LTYPE_VNODE_LOOKUP:
		                        type = "Vnode Lkup";
		                        break;

		case LTYPE_ACCT:	type = "Acct";
		                        break;

		case LTYPE_SVC:		type = "Svc";
		                        break;

		case LTYPE_FS_LAST:	type = "FS Last";
		                        break;

		case LTYPE_RNODE_IO:	type = "Rnode I/O";
		                        break;


#ifdef	LTYPE_NET_START
		case LTYPE_TCP:		type = "TCP";
					break;

		case LTYPE_INPCB:	type = "Inpcb";
					break;

		case LTYPE_SOCKET:	type = "Socket";
					break;

		case LTYPE_UDP:		type = "UDP";
					break;

		case LTYPE_IP:		type = "IP";
					break;

		case LTYPE_RAW:		type = "RAW";
					break;

		case LTYPE_ROUTE:	type = "Route";
					break;

		case LTYPE_ARP:		type = "ARP";
					break;
#endif	LTYPE_NET_START

#ifdef	LTYPE_NRC_START
		case LTYPE_NRCUSE:	type = "Nrcuse";
					break;

		case LTYPE_NRCFREEUSE:	type = "Nrcfreeuse";
					break;

		case LTYPE_NRCMAP:	type = "Nrcmap";
					break;

		case LTYPE_NRCITBL:	type = "Nrcitbl";
					break;

		case LTYPE_VIG:		type = "Vig";
					break;

		case LTYPE_VIGIO:	type = "Vigio";
					break;

		case LTYPE_PERMTBL:	type = "Permtbl";
					break;

		case LTYPE_PHYSINTR :	type = "Physintr";
					break;
#endif	LTYPE_NRC_START
#endif	/* 0 */

		default:		sprintf(buf, "Unknown(%u)",
					    lp->lock_type & 0xff);
					type = buf;
					break;
	}
#endif	/* UNIX_LOCKS */

#if	UNIX_LOCKS
	printthree("sss", "Type", "Checking", "Interlock",
		type, "N/A", lp->l_un.l_s.l_s_byte ? "Locked" : "Unlocked");
#else
	printthree("sss", "Type", "Checking", "Interlock",
		type, "N/A", (lp->interlock.lock_data ? "Locked" : "Unlocked"));
#endif
	printthree("sss", "want_write", "want_upgrade", "waiting",
		truefalse(lp->want_write), truefalse(lp->want_upgrade),
		truefalse(lp->waiting));
	printthree("sxx", "can_sleep", "read_count", "thread",
		truefalse(lp->can_sleep), lp->read_count, lp->thread);
#if	UNIX_LOCKS
	printthree("xxx", "recursion", "lthread", "lck addr",
		lp->recursion_depth, lp->lthread, lp->lck_addr);
	printthree("x  ", "unlck_addr", "", "",
		lp->unlck_addr, 0, 0);
#else
	printthree("x  ", "recursion", "", "",
		lp->recursion_depth, 0, 0);
#endif

#if	MMAX_LSTATS
	if(lp->lock_check == LCHECK_STATS) {
		printthree("xxx", "tries", "fails", "sleeps",
			lp->lock_tries, lp->lock_fails, lp->lock_sleeps);
		printthree("xxx", "wait_min", "wait_max", "max_sum",
			lp->lock_wait_min, lp->lock_wait_max,
			lp->lock_wait_sum);
		printthree("x  ", "max_read", "", "",
			lp->lock_max_read, 0, 0);
	}
#endif	MMAX_LSTATS
}

char *
truefalse(val)
int	val;
{
	return( val ? "TRUE" : "FALSE" );
}


display_simple_lock(sl)
struct slock *sl;
{
#if	MMAX_MP
#if	MMAX_MPDEBUG
	printthree("sxx", "lock_data", "slck_addr", "sunlck_addr",
		   sl->lock_data ? "Locked" : "Unlocked",
		   sl->slck_addr, sl->sunlck_addr);
#else	MMAX_MPDEBUG
	printthree("s  ", "lock_data", "", "",
		   sl->lock_data ? "Locked" : "Unlocked", 0, 0);
#endif	MMAX_MPDEBUG
#if	MMAX_LSTATS && notyet
	printthree("xxx", "thread", "slock_tries", "slock_fails",
		   sl->slthread, sl->slock_tries, sl->slock_fails);
	printthree("xxx", "slock_min_time", "slock_max_time", "slock_avg_time",
		   sl->slock_min_time, sl->slock_max_time, sl->slock_avg_time);
#else	MMAX_LSTATS && notyet
#if	MMAX_MPDEBUG
	printthree("x  ", "thread", "","", sl->slthread);
#endif	MMAX_MPDEBUG
#endif	MMAX_LSTATS && notyet
#else	MMAX_MP
	printthree("s  ", "lock_data", "", "",
		   sl->lock_data ? "Locked" : "Unlocked", 0, 0);
#endif	MMAX_MP
}
