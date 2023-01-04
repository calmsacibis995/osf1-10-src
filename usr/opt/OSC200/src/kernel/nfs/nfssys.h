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
 *	@(#)$RCSfile: nfssys.h,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/18 17:55:36 $
 */ 
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
 * Common functions for loadable NFS system calls.  Temporary
 * until we have general loadable system calls.
 */

#ifndef	_NFS_NFSSYS_H_
#define	_NFS_NFSSYS_H_


#define NULL_FUNC		(int (*)(int))0

udecl_simple_lock_data(,nfs_sys_lock)	/* for dynamic loading of nfs */
#define	NFS_SYS_LOCK()		usimple_lock(&nfs_sys_lock)
#define	NFS_SYS_UNLOCK()	usimple_unlock(&nfs_sys_lock)
#define	NFS_SYS_LOCK_INIT()	usimple_lock_init(&nfs_sys_lock)

lock_data_t	nfs_daemon_lock;	/* for dynamic loading/unloading */
#define	NFSD_LOCK()		lock_write(&nfs_daemon_lock)
#define	NFSD_UNLOCK()		lock_write_done(&nfs_daemon_lock)
#define	NFSD_LOCK_INIT()	lock_init2(&nfs_daemon_lock, TRUE, LTYPE_NFSD)

udecl_simple_lock_data(,asyncdaemon_lock)
#define	ASYNCD_LOCK()		usimple_lock(&asyncdaemon_lock)
#define	ASYNCD_UNLOCK()		usimple_unlock(&asyncdaemon_lock)
#define	ASYNCD_LOCK_INIT()	usimple_lock_init(&asyncdaemon_lock)

#endif	/* _NFS_NFSSYS_H_ */
