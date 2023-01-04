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
static char	*sccsid = "@(#)$RCSfile: nfs_config.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/06/09 12:43:42 $";
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
 * Routines for dynamically loading and unloading
 * NFS functionality and system calls from the kernel.
 */

#include <sys/sysconfig.h>
#include <sys/mount.h>
#include <sys/errno.h>
#include <nfs/nfssys.h>

extern int     (*nfssvc_func)();
extern int     (*async_daemon_func)();
extern int     (*exportfs_func)();

extern struct vfsops nfs_vfsops;
extern nfs_svc(), nfs_async_daemon(), nfs_exportfs();

int
nfs_configure(op, indata, indatalen, outdata, outdatalen)
	sysconfig_op_t		op;
	filesys_config_t *	indata;
	size_t	 		indatalen;
        filesys_config_t * 	outdata;
        size_t          	outdatalen;
{
	int	ret;

	switch (op) {
		case SYSCONFIG_CONFIGURE:
			if ((ret = vfssw_add(MOUNT_NFS, &nfs_vfsops)) != 0)
				return(ret);
			break;
		case SYSCONFIG_UNCONFIGURE:
			if ((ret = nfs_unconfig()) != 0)
				return(ret);
			break;
		default:
			return EINVAL;
			break;
	}
        if (outdata != NULL && outdatalen == sizeof(filesys_config_t)) {
                outdata->fc_version = OSF_FILESYS_CONFIG_10;
                outdata->fc_type = MOUNT_NFS;
        }
	return 0;
}


nfs_syscall_config()
{
	/*
	 * Configure system calls
	 */
	NFS_SYS_LOCK();
	nfssvc_func = nfs_svc;
	async_daemon_func = nfs_async_daemon;
	exportfs_func = nfs_exportfs;
	NFS_SYS_UNLOCK();
	return(0);
}


nfs_unconfig()
{
	int error;
	extern int nfs_nfsds, nfs_asyncdaemons;

	NFSD_LOCK();
	if (nfs_nfsds != 0 || nfs_asyncdaemons != 0) {
		NFSD_UNLOCK();
		return(EBUSY);
	}
	if (error = vfssw_del(MOUNT_NFS)) {
		NFSD_UNLOCK();
		return(error);
	}
	NFS_SYS_LOCK();
	nfssvc_func = NULL_FUNC;
	async_daemon_func = NULL_FUNC;
	exportfs_func = NULL_FUNC;
	NFS_SYS_UNLOCK();
	NFSD_UNLOCK();
	return 0;
}
