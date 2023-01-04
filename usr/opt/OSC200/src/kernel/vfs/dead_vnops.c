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
static char	*sccsid = "@(#)$RCSfile: dead_vnops.c,v $ $Revision: 4.2.4.4 $ (DEC) $Date: 1992/05/05 16:37:46 $";
#endif 
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
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)dead_vnops.c	7.7 (Berkeley) 1/2/90
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/mount.h>			/* funnelling through mnt struct */
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/namei.h>
#include <sys/buf.h>
#include <sys/poll.h>

int	dead_lookup(),
	dead_open(),
	dead_rdwr(),
	dead_strategy(),
	dead_ioctl(),
	dead_select(),
	dead_ebadf(),
	dead_badop(),
	dead_nullop(),
	dead_bread(),
	dead_brelse();

struct vnodeops dead_vnodeops = {
	dead_lookup,	/* lookup */
	dead_badop,	/* create */
	dead_badop,	/* mknod */
	dead_open,	/* open */
	dead_nullop,	/* close */
	dead_ebadf,	/* access */
	dead_ebadf,	/* getattr */
	dead_ebadf,	/* setattr */
	dead_rdwr,	/* read */
	dead_rdwr,	/* write */
	dead_ioctl,	/* ioctl */
	dead_select,	/* select */
	dead_badop,	/* mmap */
	dead_nullop,	/* fsync */
	dead_nullop,	/* seek */
	dead_badop,	/* remove */
	dead_badop,	/* link */
	dead_badop,	/* rename */
	dead_badop,	/* mkdir */
	dead_badop,	/* rmdir */
	dead_badop,	/* symlink */
	dead_ebadf,	/* readdir */
	dead_ebadf,	/* readlink */
	dead_badop,	/* abortop */
	dead_nullop,	/* inactive */
	dead_nullop,	/* reclaim */
	dead_rdwr,	/* bmap */
	dead_strategy,	/* strategy */
	dead_nullop,	/* print */
	dead_badop,	/* page_read */
	dead_badop,	/* page_write */
	dead_badop,	/* getpage */
	dead_badop,	/* putpage */
	dead_badop,	/* swap */
	dead_bread,	/* buffer read */
	dead_brelse,	/* buffer release */
	dead_nullop,	/* fsync byte range */
};

/*
 * TODO:
 * 	Add statistics for these functions.
 */

/*
 * Trivial lookup routine that always fails.
 */
dead_lookup(vp, ndp)
	struct vnode *vp;
	struct nameidata *ndp;
{

	ndp->ni_dvp = vp;
	ndp->ni_vp = NULL;
	return (ENOTDIR);
}

/*
 * Open always fails as if device did not exist.
 */
/* ARGSUSED */
dead_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{

	return (ENXIO);
}

/*
 * Vnode op for most dead vnop accesses via file descriptor
 */
/* ARGSUSED */
dead_rdwr(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	/*
	 * Always return EIO
	 */
	return (EIO);
}

/*
 * Always return EIO
 */
/* ARGSUSED */
dead_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	register int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{

	/*
	 * Always return EIO
	 */
	return (EIO);
}


/* ARGSUSED */
dead_select(vp, events, revents, scanning, cred)
	struct vnode *vp;
	short *events, *revents;
        int scanning;
	struct ucred *cred;
{

	if (scanning)
		*revents |= POLLNVAL;
	return (0);
}

/*
 * Always return EIO
 */
dead_strategy(bp)
	register struct buf *bp;
{

	return (EIO);
}

/*
 * Empty vnode failed operation
 */
dead_ebadf()
{

	return (EBADF);
}

/*
 * Empty vnode bad operation
 * Given parallelization and vnode semantics in OSF/1, under
 * some circumstances (e.g. forcible unmount), it might be possible to
 * get to this operation.  Therefore, a panic is undesirable, but it
 * is rare enough to warrant a printf.  It would be useful to know.
 */
dead_badop()
{

	printf("dead_badop called\n");
}

/*
 * Empty vnode null operation
 */
dead_nullop()
{

	return (0);
}

dead_bread(vp, lbn, bpp, cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
	struct ucred *cred;
{
	return (EOPNOTSUPP);
}

dead_brelse(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	return (EOPNOTSUPP);
}
