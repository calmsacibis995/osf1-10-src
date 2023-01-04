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
static char	*sccsid = "@(#)$RCSfile: nfs_hooks.c,v $ $Revision: 4.3.4.4 $ (DEC) $Date: 1992/07/08 14:07:33 $";
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
 * Permanently resident hooks for dynamically loading NFS.
 * Until system calls can be dynamically allocated in the
 * sysent[], and until zones can be dynamically destroyed,
 * this file contains the necessary stubs for the NFS
 * system calls and zones.
 */

#include <sys/sysconfig.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <sys/lock_types.h>
#include <kern/zalloc.h>
#include <nfs/nfssys.h>
#include <sys/lock_types.h>
#include <sys/proc.h>

int     (*nfssvc_func)() = NULL_FUNC;
int     (*async_daemon_func)() = NULL_FUNC;
int     (*exportfs_func)() = NULL_FUNC;

#ifdef	DEBUG
int nfssvcdebug = 0;
#define DPRINTF(c, args)	if(c) printf args
#else
#define DPRINTF(c, args)
#endif

nfssvc(p, args, retval)
        struct proc *p;
        void *args;
        long *retval;
{
	int     (*func)();

	DPRINTF(nfssvcdebug, ("nfssvc: got here\n"));

	/*
	 * We take a blocking lock to ensure that NFS is not
	 * unloaded while we are starting a new daemon.
	 * nfs_svc will unlock NFSD_LOCK for us.
	 */
	DPRINTF(nfssvcdebug, ("nfssvc: do NFSD_LOCK\n"));
	NFSD_LOCK();

	DPRINTF(nfssvcdebug, ("nfssvc: do NFS_SYS_LOCK\n"));
	NFS_SYS_LOCK();
	func = nfssvc_func;

	DPRINTF(nfssvcdebug, ("nfssvc: do NFS_SYS_UNLOCK\n"));
	NFS_SYS_UNLOCK();

	if (func == NULL_FUNC) {
		NFSD_UNLOCK();
		DPRINTF(nfssvcdebug, ("nfssvc: nosys\n"));
		return nosys(p, args, retval);
	} else {
		DPRINTF(nfssvcdebug, ("nfssvc: doing func\n"));
		return (*func)(p, args, retval);
	}
}

async_daemon(p, args, retval)
        struct proc *p;
        void *args;
        long *retval;
{

	int     (*func)();

	/*
	 * We take a blocking lock to ensure that NFS is not
	 * unloaded while we are starting a new daemon.
	 * nfs_async_daemon will unlock it for us.
	 */
	NFSD_LOCK();
	NFS_SYS_LOCK();
	func = async_daemon_func;
	NFS_SYS_UNLOCK();
	if (func == NULL_FUNC) {
		NFSD_UNLOCK();
		return nosys(p, args, retval);
	} else
		return (*func)(p, args, retval);
}

exportfs(p, args, retval)
        struct proc *p;
        void *args;
        int *retval;
{

	int     (*func)();

	NFS_SYS_LOCK();
	func = exportfs_func;
	NFS_SYS_UNLOCK();
	if (func == NULL_FUNC) {
		return nosys(p, args, retval);
	} else
		return (*func)(p, args, retval);
}


nfs_hooks_init()
{
	NFS_SYS_LOCK_INIT();
	NFSD_LOCK_INIT();
	ASYNCD_LOCK_INIT();
}
