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
static char	*sccsid = "@(#)$RCSfile: mfs_vnops.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/04/01 17:19:39 $";
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

 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <sys/vmmac.h>
#include <sys/errno.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <ufs/mfsnode.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>

/*
 * mfs vnode operations.
 */
int	mfs_open(),
	mfs_strategy(),
	mfs_bmap(),
	mfs_ioctl(),
	mfs_close(),
	mfs_inactive(),
	mfs_print(),
	mfs_badop(),
	mfs_nullop(),
	mfs_bread(),
	mfs_brelse();

/*
 * use spec_reclaim to clean up aliases
 */
extern int	spec_reclaim();
extern int	ufs_lockctl();

struct vnodeops mfs_vnodeops = {
	mfs_badop,		/* lookup */
	mfs_badop,		/* create */
	mfs_badop,		/* mknod */
	mfs_open,		/* open */
	mfs_close,		/* close */
	mfs_badop,		/* access */
	mfs_badop,		/* getattr */
	mfs_badop,		/* setattr */
	mfs_badop,		/* read */
	mfs_badop,		/* write */
	mfs_ioctl,		/* ioctl */
	mfs_badop,		/* select */
	mfs_badop,		/* mmap */
	mfs_badop,		/* fsync */
	mfs_badop,		/* seek */
	mfs_badop,		/* remove */
	mfs_badop,		/* link */
	mfs_badop,		/* rename */
	mfs_badop,		/* mkdir */
	mfs_badop,		/* rmdir */
	mfs_badop,		/* symlink */
	mfs_badop,		/* readdir */
	mfs_badop,		/* readlink */
	mfs_badop,		/* abortop */
	mfs_inactive,		/* inactive */
	spec_reclaim,		/* reclaim */
	mfs_bmap,		/* bmap */
	mfs_strategy,		/* strategy */
	mfs_print,		/* print */
	mfs_badop,		/* page read */
	mfs_badop,		/* page write */
	mfs_badop,		/* getpage */
	mfs_badop,		/* putpage */
	mfs_badop,		/* swap */
	mfs_bread,		/* buffer read */
	mfs_brelse,		/* buffer release */
	ufs_lockctl,		/* file locking */
	mfs_badop,		/* fsync byte range */
};

/*
 * Vnode Operations.
 *
 * Open called to allow memory filesystem to initialize and
 * validate before actual IO. Record our process identifier
 * so we can tell when we are doing I/O to ourself.
 */
/* ARGSUSED */
mfs_open(vpp, mode, cred)
	register struct vnode **vpp;
	int mode;
	struct ucred *cred;
{
	int error;
	register struct vnode *vp = *vpp;

	BM(VN_LOCK(vp));
	if (vp->v_type != VBLK) {
		BM(VN_UNLOCK(vp));
		panic("mfs_open not VBLK");
		/* NOTREACHED */
	}
	BM(VN_UNLOCK(vp));
	error = makealias(vp);
	return(error);
}

/*
 * Ioctl operation.
 */
/* ARGSUSED */
mfs_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{

	return (-1);
}

/*
 * Pass I/O requests to the memory filesystem process.
 */
mfs_strategy(bp)
	register struct buf *bp;
{
	register struct mfsnode *mfsp;
	struct vnode *vp;

	if (vfinddev(bp->b_dev, VBLK, &vp))
		panic("mfs_strategy: bad dev");
	/*
	 * No locking here; these fields are read-only.
	 */
	mfsp = VTOMFS(vp);
	if (mfsp->mfs_pid == u.u_procp->p_pid) {
		mfs_doio(bp, mfsp->mfs_baseoff);
	} else {
		VN_LOCK(vp);
		bp->av_forw = mfsp->mfs_buflist;
		mfsp->mfs_buflist = bp;
		VN_UNLOCK(vp);
		if (mfsp->mfs_pid)
			thread_wakeup_one((vm_offset_t)vp);
		else
			mfs_error_state(vp);
	}
	return (0);
}

/*
 * Memory file system I/O.
 *
 * Essentially play ubasetup() and disk interrupt service routine by
 * doing the copies to or from the memfs process. If doing physio
 * (i.e. pagein), we must map the I/O through the kernel virtual
 * address space.
 */
mfs_doio(bp, base)
	register struct buf *bp;
	caddr_t base;
{
	register caddr_t vaddr;
	int off, npf, npf2, reg;
	caddr_t kernaddr, offset;

	/*
	 * For phys I/O, map the b_addr into kernel virtual space
	 */
	if ((bp->b_flags & B_PHYS) == 0) {
		kernaddr = bp->b_un.b_addr;
	} else {
		panic("mfs_doio: B_PHYS\n");
	}
	offset = base + (bp->b_blkno << DEV_BSHIFT);
	if (bp->b_flags & B_READ)
		bp->b_error = copyin(offset, kernaddr, bp->b_bcount);
	else
		bp->b_error = copyout(kernaddr, offset, bp->b_bcount);
	if (bp->b_error)
		bp->b_flags |= B_ERROR;
	biodone(bp);
}

/*
 * The pid associated with the virtual address space has been killed.
 * Mark any pending I/O buffers with an error, and hopefully none will
 * be metadata reads...
 */
mfs_error_state(vp)
	struct vnode *vp;
{
	register struct mfsnode *mfsp = VTOMFS(vp);
	struct buf *bp;

	VN_LOCK(vp);
	while (bp = mfsp->mfs_buflist) {
		mfsp->mfs_buflist = bp->av_forw;
		VN_UNLOCK(vp);
		bp->b_error = EIO;
		bp->b_flags |= B_ERROR;
		biodone(bp);
		VN_LOCK(vp);
	}
	VN_UNLOCK(vp);
}

/*
 * This is a noop, simply returning what one has been given.
 */
mfs_bmap(vp, bn, vpp, bnp)
	struct vnode *vp;
	daddr_t bn;
	struct vnode **vpp;
	daddr_t *bnp;
{

	if (vpp != NULL)
		*vpp = vp;
	if (bnp != NULL)
		*bnp = bn;
	return (0);
}

/*
 * Memory filesystem close routine
 */
/* ARGSUSED */
mfs_close(vp, flag, cred)
	register struct vnode *vp;
	int flag;
	struct ucred *cred;
{
	register struct mfsnode *mfsp = VTOMFS(vp);
	register struct buf *bp;
	struct specalias *sa;

	/*
	 * Finish any pending I/O requests.
	 */
	VN_LOCK(vp);
	while (bp = mfsp->mfs_buflist) {
		mfsp->mfs_buflist = bp->av_forw;
		VN_UNLOCK(vp);
		mfs_doio(bp, mfsp->mfs_baseoff);
		VN_LOCK(vp);
	}
	VN_UNLOCK(vp);
	/*
	 * On last close of a memory filesystem
	 * we must invalidate any in core blocks, so that
	 * we can, free up its vnode.
	 */
	vflushbuf(vp, 0);
	if (vinvalbuf(vp, 1))
		return (0);
	/*
	 * There should be no way to have any more uses of this
	 * vnode, so if we find any other uses, it is a panic.
	 */
	BM(VN_LOCK(vp));
	if (vp->v_usecount > 1 || mfsp->mfs_buflist) {
		vprint("mfs_close", vp);
		panic("mfs_close");
	}
	BM(VN_UNLOCK(vp));
	/*
	 * Send a request to the filesystem server to exit.
	 */
	thread_wakeup_with_result((vm_offset_t)vp, THREAD_INTERRUPTED);
	/*
	 * Wake up initial thread, so it can exit.
	 */
	thread_wakeup((vm_offset_t)mfsp);
	/*
	 * Must decrement usecount on specalias structure.  Cannot go
	 * through normal mechanism (spec_close) since it assumes a
	 * normal device.  All we need to do is decrement the usecount.
	 * No locking required -- nobody knows about these vnodes.
	 */
	sa = vp->v_specinfo->si_alias;
	ASSERT((sa != (struct specalias *) 0) && 
	       (sa->sa_usecount == 1));
	sa->sa_usecount--;
	return (0);
}

/*
 * Memory filesystem inactive routine
 */
/* ARGSUSED */
mfs_inactive(vp)
	struct vnode *vp;
{

	return (0);
}

/*
 * Print out the contents of an mfsnode.
 */
mfs_print(vp)
	struct vnode *vp;
{
	register struct mfsnode *mfsp = VTOMFS(vp);

	printf("tag VT_MFS, pid %d, base %d, size %d\n", mfsp->mfs_pid,
		mfsp->mfs_baseoff, mfsp->mfs_size);
}

/*
 * Block device bad operation
 */
mfs_badop()
{

	panic("mfs_badop called\n");
	/* NOTREACHED */
}

/*
 * Block device null operation
 */
mfs_nullop()
{

	return (0);
}

mfs_bread(vp, lbn, bpp, cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
	struct ucred *cred;
{
	return (EOPNOTSUPP);
}

mfs_brelse(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	return (EOPNOTSUPP);
}
