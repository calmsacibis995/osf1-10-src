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
static char	*sccsid = "@(#)$RCSfile: mfs_vfsops.c,v $ $Revision: 4.2.11.3 $ (DEC) $Date: 1993/05/08 10:45:22 $";
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

/*
 * mfs_vfsops.c
 *
 *      Revision History:
 *
 * 17-Jun-91    Mitch Condylis
 *      Prevent non-root mounts in mfs_mount.
 *
 */

#include <sys/secdefines.h>

#include <rt_preempt.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/buf.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <ufs/ufsmount.h>
#include <ufs/mfsnode.h>
#include <ufs/fs.h>

#include <kern/thread.h>
#include <kern/sched_prim.h>

extern struct vnodeops mfs_vnodeops;

/*
 * mfs vfs operations.
 */
int mfs_mount();
int mfs_start();
int mfs_unmount();
extern int ufs_unmount();
extern int ufs_root();
extern int ufs_quotactl();
int mfs_statfs();
int mfs_sync();
extern int ufs_sync();
extern int ufs_fhtovp();
extern int ufs_vptofh();
int mfs_nullop();
int mfs_badoper();

struct vfsops mfs_vfsops = {
	mfs_mount,
	mfs_start,
	mfs_unmount,
	ufs_root,
	ufs_quotactl,
	mfs_statfs,
	mfs_sync,
	ufs_fhtovp,
	ufs_vptofh,
	mfs_nullop,	/* init */
	mfs_badoper,	/* mountroot */
	mfs_badoper,	/* swapvp */
};


/*
 * VFS Operations.
 */

mfs_badoper()
{
	return(EINVAL);
}

/*
 * mount system call
 */
/* ARGSUSED */
mfs_mount(mp, path, data, ndp)
	struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
{
	struct vnode *devvp;
	struct mfs_args args;
	struct ufsmount *ump;
	register struct fs *fs;
	register struct mfsnode *mfsp;
	static int mfs_minor;
	u_int size;
	int error;

	/* Disable user mounts for mfs */
	if (mp->m_uid)
		return (EPERM);

	MOUNT_LOCK(mp);
	if (mp->m_flag & M_UPDATE) {
		ump = VFSTOUFS(mp);
		fs = ump->um_fs;
		if (fs->fs_ronly && (mp->m_flag & M_RDONLY) == 0)
			fs->fs_ronly = 0;
		MOUNT_UNLOCK(mp);
		return (0);
	}
#if	SER_COMPAT || RT_PREEMPT
	mp->m_funnel = FUNNEL_NULL;
#endif
	mp->m_flag |= M_SWAP_NEVER;	/* cannot page to memory!!! */
	MOUNT_UNLOCK(mp);
	if (error = copyin(data, (caddr_t)&args, sizeof (struct mfs_args)))
		return (error);
	error = bdevvp(makedev(255, mfs_minor++), &devvp);
	if (error)
		return (error);
	devvp->v_tag = VT_MFS;
	devvp->v_op = &mfs_vnodeops;
	mfsp = VTOMFS(devvp);
	mfsp->mfs_baseoff = args.base;
	mfsp->mfs_size = args.size;
	mfsp->mfs_vnode = devvp;
	mfsp->mfs_pid = u.u_procp->p_pid;
	mfsp->mfs_task = current_task();
	mfsp->mfs_buflist = (struct buf *)0;
	if (error = mountfs(devvp, mp, 0)) {
		if (error == EBUSY)	/* cannot happen */
			panic("mfs_mount: duplicate mount");
		vrele(devvp);
		return (error);
	}
#if	SEC_FSCHANGE
	if (error = sec_ufsmountcheck(mp)) {
		(void) ufs_unmount(mp, 0);
		return error;
	}
#endif
	ump = VFSTOUFS(mp);
	fs = ump->um_fs;
	(void) copyinstr(path, fs->fs_fsmnt, sizeof(fs->fs_fsmnt) - 1, &size);
	bzero(fs->fs_fsmnt + size, sizeof(fs->fs_fsmnt) - size);
	bcopy((caddr_t)fs->fs_fsmnt, (caddr_t)mp->m_stat.f_mntonname, MNAMELEN);
	(void) copyinstr(args.name, mp->m_stat.f_mntfromname, MNAMELEN - 1, 
		&size);
	bzero(mp->m_stat.f_mntfromname + size, MNAMELEN - size);
	/*
	 * prime the struct statfs in the mount point
	 */
	(void) mfs_statfs(mp);
	/*
	 * Don't unmount mfs file systems during system reboot
	 * in boot()
	 */
	mp->m_flag &= ~M_LOCAL;
	return (0);
}

int mfs_lbolt(mp)
        struct mount **mp;
{
        wakeup(mp);
}

/*
 * Used to grab the process and keep it in the kernel to service
 * memory filesystem I/O requests.
 * Create a number of threads and start them off.
 */
mfs_start(mp, flags)
	struct mount *mp;
	int flags;
{
	register struct vnode 	*vp = VFSTOUFS(mp)->um_devvp;
	register struct mfsnode *mfsp = VTOMFS(vp);
	int 			i;
	thread_t		thread;
	int 			nmfsthreads;
	extern int 		numcpus;
	void			mfs_thread_start();
	int			mfs_lbolt();
	int			s, error;
	extern int		hz;


	/*
	 * Due to a problem in vm_map_pageable, we get a deadlock
	 * on the mmax when multiple threads are used, soooo we
	 * limit it to one for now.  Should be:
	 * 	nmfsthreads = numcpus + 1;
	 */
	nmfsthreads = 1;

	ASSERT(mfsp->mfs_task == current_task());
	/*
	 * Start nmfsthreads plus one (this one).
	 * A uniprocessor gets 2.
	 * Should we do boost priority?
	 */
	for (i = 0; i < nmfsthreads; i++) {
		if (thread_create(current_task(), &thread) ==
			KERN_SUCCESS) {
			thread_start(thread, mfs_thread_start, THREAD_USERMODE);
			(void) thread_resume(thread);
			mfsp->mfs_numthreads++;
		}
	}
	/*
	 * Wait until an unmount or signal happens, at which time, we
	 * see if it was an unmount or signal that caused us to wake up.
	 * If an unmount, then simply exit; if a signal, then attempt to
	 * unmount the file system, and then exit.
	 */
	thread = current_thread();
	assert_wait((vm_offset_t)mfsp, TRUE);
	thread_block();
	/*
	 * If we were signalled, then we need to try to do the unmount
	 * to clean up; otherwise, we were woken up via mfs_close, 
	 * which was called from unmount.
	 */
	if (thread->wait_result != THREAD_AWAKENED) {
		/*
		 * The mfs process has been signalled. Try the unmount.
		 * If the unmount fails, wait 5 seconds and try it again.
		 * If we are /tmp, waiting this interval allows other
		 * processes to die, and close files. However, if the second
		 * unmount attempt fails, put mfs into the error state. This
		 * marks all outstanding and future I/O requests as errors,
		 * and prevents unmounts and sync calls which could hang.
		 * There is not much we can do, since our virtual address space
		 * will be deallocated if this process dies.....
		 */
		error = dounmount((struct vnode *) 0, mp, MNT_NOFORCE);
		if (error) {
			s = splclock();
                        timeout(mfs_lbolt, (caddr_t)&mp, 5 * hz);
                        sleep((caddr_t)&mp, PRIBIO - 1);
                        /* ...zzz...zzz... */
                        splx(s);
                        error = dounmount((struct vnode *) 0, mp, MNT_NOFORCE);
			if (error) {
				mfsp->mfs_pid = (pid_t)0;
				mfs_error_state(vp);
			}
		}
	}
	return (0);
}

/*
 * mfs_thread_start:  the one that really does the work.
 *
 * Loop servicing I/O requests.
 * Copy the requested data into or out of the memory filesystem
 * address space.
 */
/* ARGSUSED */
void
mfs_thread_start()
{
	register struct vnode *vp;
	register struct mfsnode *mfsp;
	register struct mount *mp;
	register struct buf *bp;
	register caddr_t base;
	thread_t	th;

	/*
	 * Find our mount point, and proceed from there.
	 */
	for (mp = rootfs->m_next; mp != rootfs; mp = mp->m_next) {
		if (mp->m_stat.f_type == MOUNT_MFS) {
			vp = VFSTOUFS(mp)->um_devvp;
			mfsp = VTOMFS(vp);
			if (mfsp->mfs_task == current_task())
				break;
		}
	}
	if (mp == rootfs) {
		return;
	}
	base = mfsp->mfs_baseoff;
	th = current_thread();
	assert_wait((vm_offset_t)vp, TRUE);
	thread_block();
	VN_LOCK(vp);
	while (th->wait_result == THREAD_AWAKENED) {
		while (bp = mfsp->mfs_buflist) {
			mfsp->mfs_buflist = bp->av_forw;
			VN_UNLOCK(vp);
			mfs_doio(bp, base);
			VN_LOCK(vp);
		}
		assert_wait((vm_offset_t)vp, TRUE);
		VN_UNLOCK(vp);
		thread_block();
		VN_LOCK(vp);
	}
	VN_UNLOCK(vp);
	/*
	 * We've been interrupted or unmounted; cleanup happens in
	 * mfs_start, so we terminate.
	 */
	thread_terminate(th);
	thread_halt_self();
}

/*
 * Memory file system unmount call wrapper
 */
mfs_unmount(mp, mntflags)
        struct mount *mp;
        int mntflags;
{
	register struct ufsmount *ump;
	struct mfsnode *mfsp;

	BM(MOUNT_LOCK(mp));
        ump = VFSTOUFS(mp);
        BM(MOUNT_UNLOCK(mp));
	if (!ump)
                return(ENODEV);

	mfsp = VTOMFS(ump->um_devvp);
	if (mfsp->mfs_pid == (pid_t)0)
		return(EIO);

	return(ufs_unmount(mp, mntflags));
}
/*
 * Get file system statistics.
 */
mfs_statfs(mp)
	struct mount *mp;
{
	int error;

	error = ufs_statfs(mp);
	MOUNT_LOCK(mp);
	mp->m_stat.f_type = MOUNT_MFS;
	MOUNT_UNLOCK(mp);
	return (error);
}

/*
 * Memory file system sync call wrapper
 */
mfs_sync(mp, waitfor)
        struct mount *mp;
        int waitfor;
{
	register struct ufsmount *ump;
	struct mfsnode *mfsp;

	BM(MOUNT_LOCK(mp));
        ump = VFSTOUFS(mp);
        BM(MOUNT_UNLOCK(mp));
	if (!ump)
                return(ENODEV);

	mfsp = VTOMFS(ump->um_devvp);
	if (mfsp->mfs_pid == (pid_t)0)
		return(EIO);

	return(ufs_sync(mp, waitfor));
}
