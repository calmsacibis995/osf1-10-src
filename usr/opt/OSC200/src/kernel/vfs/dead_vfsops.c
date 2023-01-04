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
static char	*sccsid = "@(#)$RCSfile: dead_vfsops.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1992/03/18 18:54:24 $";
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

#include <sys/param.h>
#include <sys/user.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/errno.h>

/*
 * dead vfs operations.
 */
int dead_badvfsop();
int dead_root();
int dead_quotactl();
int dead_statfs();
int dead_sync();
int dead_fhtovp();
int dead_vptofh();

struct vfsops dead_vfsops = {
	dead_badvfsop,	/* vfs_mount */
	dead_badvfsop,	/* vfs_start */
	dead_badvfsop,	/* vfs_unmount */
	dead_root,
	dead_quotactl,
	dead_statfs,
	dead_sync,
	dead_fhtovp,
	dead_vptofh,
	dead_badvfsop,	/* vfs_init */
	dead_badvfsop,	/* vfs_mountroot */
	dead_badvfsop,	/* vfs_swapvp */
};

/*
 * The dead mount structure
 */
struct mount dead_mount;

/*
 * VFS Operations.
 */
/* ARGSUSED */
dead_badvfsop()
{
#if	MACH_ASSERT
	printf("Dead_badvfsop called\n");
#endif
	panic("dead_badvfsop called");
}

/*
 * Return root of a filesystem
 */
dead_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	
#if	MACH_ASSERT
	printf("Dead_root called\n");
#endif
	return (ENODEV);
}

/*
 * Quota controls
 */
dead_quotactl(mp, cmds, uid, arg)
	struct mount *mp;
	int cmds;
	uid_t uid;
	caddr_t arg;
{
#if	MACH_ASSERT
	printf("Dead_quotactl called\n");
#endif
	return (EOPNOTSUPP);
}

/*
 * File system status
 */
dead_statfs(mp)
	struct mount *mp;
{
	
#if	MACH_ASSERT
	printf("Dead_statfs called\n");
#endif
	return (ENODEV);
}

/*
 * Flush out the buffer cache
 */
/* ARGSUSED */
dead_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{

#if	MACH_ASSERT
	printf("Dead_sync called\n");
#endif
	return (ENODEV);
}

/*
 * File handle to vnode pointer translation.
 */
/* ARGSUSED */
dead_fhtovp(mp, fhp, vpp)
	struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{

#if	MACH_ASSERT
	printf("Dead_fhtovp called\n");
#endif
	return (ENODEV);
}

/*
 * Vnode pointer to File handle.
 */
/* ARGSUSED */
dead_vptofh(mp, fhp, vpp)
	struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{

#if	MACH_ASSERT
	printf("Dead_vptofh called\n");
#endif
	return (ENODEV);
}
