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
 *	@(#)$RCSfile: net_globals.h,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/09/21 22:14:46 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
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
 *	File:	net/net_globals.h
 *
 *	Revision History:
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patch.
 *
 * 5-May-91	Ron Widyono
 *	Don't enable NETSYNC_LOCK whenever RT_PREEMPT is selected.  This is
 *	temporary, until we find out exactly what is wrong with LOCK
 *	synchronization in the network.
 *
 */
/*
 * Global #defines for OSF/1 networking.
 *
 * Ugly as this file is, it makes the code a lot cleaner!
 *
 * The following major #defines are supported:
 *	MACH		multi or uniprocessor mach
 *	NCPUS		number of processors
 *	NNETTHREADS	pseudo-device specifying threads
 *	UNIX_LOCKS	configure with locking
 *	VAGUE_STATS	no locking around stats
 *
 * These defines yield the following internal defines:
 *	NETNCPUS	number of processors doing network
 *	NETSYNC_SPL	do process synch with spl's (may co-exist w/locks)
 *	NETSYNC_LOCK	do process sync with locks (may co-exist w/spl)
 *	NETISR_THREAD	do isr's via thread (else via software interrupt)
 *	NETSYNC_LOCKTEST	turn on extra lock debugging (UNIX, too)
 *
 * Current prerequisites (not enforced!):
 *	One or both of NETSYNC_SPL && NETSYNC_LOCK
 *	If !MACH, then (implied) UNIX
 *	NETISR_THREAD requires MACH
 *	NETSYNC_LOCK requires NETISR_THREAD
 *	UNIX requires NETSYNC_SPL
 */

#ifndef	_NET_GLOBALS_H_
#define _NET_GLOBALS_H_

#if	MACH

#include <cpus.h>
#include <unix_locks.h>
#include <vague_stats.h>
#include <netthreads.h>
#include <rt_preempt.h>

/*
 * These are default settings. Either or both of locking and spl are valid
 * for 1 or more cpus. However, recommend locks for multis, non-locks for unis.
 * The thread decision is dependent on several things. The biggest problem
 * with using softnets is reliable memory allocation, see net/net_malloc.h.
 */
#define NETNCPUS	NCPUS
#define NETSYNC_LOCK	(UNIX_LOCKS && !RT_PREEMPT)		/* maybe locks for synch */
#define NETSYNC_SPL	!NETSYNC_LOCK	/* else spl for synch */
#define NETISR_THREAD	(NETSYNC_LOCK || (NETNCPUS > 1) || (NNETTHREADS > 0))

#define PARALLEL_SELECT	1		/* new-style select */

#else	/* UNIX */

/*
 * Builds under 4.3 Reno (4.4 alpha) are possible. To do so:
 *	Move bsd/uipc* and bsd/sys_socket.c to kern/
 *	Move all of {net,netinet,netns}/ to same
 *	Move sys/{socket,socketvar,un,unpcb,domain,protosw,mbuf}.h to sys/
 *	Add -D_KERNEL to conf/Makefile.<machine> and -I.. to includes
 *	Delete radix.c, rtsock.c, and af.c from conf/files, and add netisr.c
 *	Delete slattach, loattach, ifinit, domaininit in kern/init_main.c,
 *		replace with netinit()
 *	Delete all dispatch (ipintr, etc) in locore.s and call Netintr
 *	Important: add options UIPC to machine config
 *	Options NS will work, but options ISO or IMP will not. Someday...
 *	Add options DEBUG to test locking
 * Applications will work with the following:
 *	Netstat will require recompile, route -C will work if route doesn't.
 *	Existing binaries will work unmodified, but if recompiling, force
 *		_SOCKADDR_LEN on in sys/socket.h and disable the redefine
 *		of the send/recv libc functions there.
 */
#define NETNCPUS	1
#define NETISR_THREAD	0	/* UNIX uses software interrupts */
#define NETSYNC_LOCK	DEBUG	/* and tests locks in debug */
#define NETSYNC_SPL	1	/* but spl for actual synch */

#define PARALLEL_SELECT	0	/* old-style select */
#define VAGUE_STATS	0
#define assert_wait(a,b)
#define clear_wait(a,b,c)
#define unix_master()
#define unix_release()

/* OSF/1 Parallelization compat for UNIX builds */
#define ASSERT(x)
#define BM(x)
#define FP_LOCK(x)
#define FP_UNLOCK(x)
#define FP_REF(x)
#define FP_UNREF(x)
#define U_FDTABLE_LOCK(x)
#define U_FDTABLE_UNLOCK(x)
#define VN_LOCK(x)
#define VN_UNLOCK(x)

#define fdealloc(fp)		{ crfree((fp)->f_cred); (fp)->f_count = 0; }
#define U_FD_SET(fd, val, x)	u.u_ofile[fd] = (val)

#endif

#if	NETSYNC_LOCK	/* Configure locking */

#if	MACH

#include "kern/lock.h"
#include "kern/assert.h"
#include "sys/lock_types.h"
#include <mach_ldebug.h>
#define LOCK_ASSERTL_DECL
#define LOCK_ASSERTL(string, cond)	ASSERT(cond)
#define LOCK_ASSERT(string, cond)	ASSERT(cond)

#define NETSYNC_LOCKTEST (DEBUG || MACH_LDEBUG)
#define LOCK_NETSTATS	 (VAGUE_STATS == 0)

#else	/* UNIX */

#include "net/net_unixlock.h"
#define LOCK_ASSERTL_DECL		static char _file_[] = __FILE__; \
					extern char _net_lock_format_[];
#define LOCK_ASSERTL(string, cond)	{ if(!(cond)) printf(_net_lock_format_,\
						string, _file_, __LINE__); }
#define LOCK_ASSERT(string, cond)	{ if(!(cond)) printf("\t%s\n",string); }
#define NETSYNC_LOCKTEST DEBUG
#define LOCK_NETSTATS	 1

#endif

#else	/* !NETSYNC_LOCK */
#define LOCK_ASSERTL_DECL
#define LOCK_ASSERTL(string, cond)
#define LOCK_ASSERT(string, cond)
#define NETSYNC_LOCKTEST	0
#define LOCK_NETSTATS		0
#if	!MACH
typedef int	lock_data_t, *lock_t;
#endif

#endif

#if	NETSYNC_SPL	/* Do spl() synch */
#define NETSPL_DECL(s)		int s;
#ifdef	__STDC__
#define NETSPL(s,level)		s = spl##level()
#else
#define NETSPL(s,level)		s = spl/**/level()
#endif
#define NETSPLX(s)			splx(s)
#else			/* Don't spl() synch */
#define NETSPL_DECL(s)
#define NETSPL(s,level)
#define NETSPLX(s)
#endif

#if	LOCK_NETSTATS
#define NETSTAT_LOCKINIT(lockp)	simple_lock_init(lockp)
#define NETSTAT_LOCK(lockp)	simple_lock(lockp)
#define NETSTAT_UNLOCK(lockp)	simple_unlock(lockp)
#else
#define NETSTAT_LOCKINIT(lockp)
#define NETSTAT_LOCK(lockp)
#define NETSTAT_UNLOCK(lockp)
#endif

/* ANSI-C compatibility */
#ifdef	__STDC__
#ifndef CONST
#define CONST           const
#endif /* CONST */
#define VOLATILE	volatile
#else
#define CONST
#define VOLATILE
#endif

/* Global function prototypes */
#include "sys/types.h"
#include "net/proto_net.h"
#include "net/proto_uipc.h"

#endif	/* _NET_GLOBALS_H_ */
