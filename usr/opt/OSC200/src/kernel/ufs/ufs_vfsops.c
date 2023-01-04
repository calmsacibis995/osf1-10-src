
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
static char	*sccsid = "@(#)$RCSfile: ufs_vfsops.c,v $ $Revision: 4.2.43.5 $ (DEC) $Date: 1994/01/14 16:35:49 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
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
 * ufs_vfsops.c
 *
 *	Revision History:
 *
 * 08-Aug-91	dws
 *	Allow forced unmounts.
 *
 *  8-Aug_91 	prs
 *	Fixed read-only mounts to not write on umount.
 *
 * 17-Jun-91	Mitch Condylis
 *	Prevent non-root mounts in ufs_mount now and initialize new
 *	m_uid mount structure field to zero in ufs_mountroot().
 *
 */

#if	MACH
#include <quota.h>
#include <bufcache_stats.h>
#ifdef	i386
#include <cputypes.h>
#endif
#endif

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

#include <rt_preempt.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/mount.h>
#include <sys/buf.h>
#include <sys/biostats.h>
#include <sys/ucred.h>
#include <sys/file.h>
#include <sys/disklabel.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#if	MACH
#include <kern/zalloc.h>
#include <kern/kalloc.h>
#else
#include <sys/malloc.h>
#endif
#include <ufs/quota.h>
#if	QUOTA
#include <sys/proc.h>
#endif
#include <ufs/fs.h>
#include <ufs/fs_proto.h>
#include <ufs/ufsmount.h>
#include <ufs/inode.h>
#include <sys/lock_types.h>
#if	MACH
#include <sys/user.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#endif
#if	EXL
#include <machine/open.h>
#endif
#include <sys/open.h>

#if	MACH
zone_t	superblock_zone;
#endif

/*
 * ufs vfs operations.
 */
int ufs_mount();
int ufs_start();
int ufs_unmount();
int ufs_root();
int ufs_quotactl();
int ufs_statfs();
int ufs_sync();
int ufs_fhtovp();
int ufs_vptofh();
int ufs_init();
int ufs_mountroot();
int ufs_noop();

struct vfsops ufs_vfsops = {
	ufs_mount,
	ufs_start,
	ufs_unmount,
	ufs_root,
	ufs_quotactl,
	ufs_statfs,
	ufs_sync,
	ufs_fhtovp,
	ufs_vptofh,
	ufs_init,
	ufs_mountroot,
	ufs_noop,
};

void irefresh();

/*
 * ufs mount table.
 */
struct ufsmount *mounttab;

ufs_noop()
{
	return(0);
}

/*
 * Called by vfs_mountroot when ufs is going to be mounted as root.
 * Assume that nothing else can be happening in the system, so there's
 * no locking done.
 *
 * Name is updated by mount(8) after booting.
 */
#define ROOTNAME	"root_device"

ufs_mountroot(mp, vpp)
	register struct mount *mp;
	struct vnode **vpp;
{
	extern dev_t rootdev;
	struct ufsmount *ump;
	register struct fs *fs;
	u_int size;
	int error;

	mp->m_op = &ufs_vfsops;
#if	SEC_FSCHANGE
	mp->m_flag = M_RDONLY | M_SECURE;
#else
	mp->m_flag = M_RDONLY;
#endif
	mp->m_exroot = 0;
	mp->m_uid = 0;
	mp->m_mounth = (struct vnode *)0;
#if	SER_COMPAT || RT_PREEMPT
	mp->m_funnel = FUNNEL_NULL;
#endif
	if (bdevvp(rootdev, vpp))
		panic("can't setup bdevvp of rootdev");
	MOUNT_LOOKUP_LOCK_INIT(mp);
	MOUNT_VLIST_LOCK_INIT(mp);
	MOUNT_LOCK_INIT(mp);
	error = mountfs(rootvp, mp, 0);
	if (error) {
		return (error);
	}
	mp->m_next = mp;
	mp->m_prev = mp;
	mp->m_vnodecovered = (struct vnode *)0;
#if	SEC_FSCHANGE
	if (security_is_on) {
		if (error = sec_ufsmountcheck(mp)) {
			(void) ufs_unmount(mp, 0);
			rootfs = NULL;
			return (error);
		}
	}
#endif
	ump = VFSTOUFS(mp);
	fs = ump->um_fs;
	bzero(fs->fs_fsmnt, sizeof(fs->fs_fsmnt));
	fs->fs_fsmnt[0] = '/';
	bcopy((caddr_t)fs->fs_fsmnt, (caddr_t)mp->m_stat.f_mntonname, MNAMELEN);
	(void) copystr(ROOTNAME, mp->m_stat.f_mntfromname, MNAMELEN - 1, &size);
	bzero(mp->m_stat.f_mntfromname + size, MNAMELEN - size);
	/*
	 * prime the struct statfs in the mount structure
	 */
	(void) ufs_statfs(mp);
	rootfs = mp;
	inittodr(fs->fs_time);
	return (0);
}

/*
 * VFS Operations.
 *
 * mount system call
 *
 * Synchronization assumptions:
 *	-- Mount structure could be on global list (if M_UPDATE), so
 *	   we need to lock (sigh).
 *	-- Other attempted mounts on this directory are locked out by
 *	   mount().
 * We are responsible for detecting races on mounting the device passed in.
 * N.B.  Expect ndp to contain a pointer to the vnode to be covered in ni_vp.
 */
int updaterefresh = 1;

ufs_mount(mp, path, data, ndp)
	register struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
{
	struct vnode *devvp;
	struct ufs_args args;
	struct ufsmount *ump;
	register struct fs *fs;
	u_int size;
	int error;

	/* Disable user mounts for ufs */
	if (mp->m_uid)
		return (EPERM);

	if (error = copyin(data, (caddr_t)&args, sizeof (struct ufs_args)))
		return (error);
	/*
	 * Process export requests.
	 */
	MOUNT_LOCK(mp);
	if ((args.exflags & M_EXPORTED) || (mp->m_flag & M_EXPORTED)) {
		if (args.exflags & M_EXPORTED)
			mp->m_flag |= M_EXPORTED;
		else
			mp->m_flag &= ~M_EXPORTED;
		if (args.exflags & M_EXRDONLY)
			mp->m_flag |= M_EXRDONLY;
		else
			mp->m_flag &= ~M_EXRDONLY;
		mp->m_exroot = args.exroot;
	}
	if ((mp->m_flag & M_UPDATE) == 0) {
#if	SER_COMPAT || RT_PREEMPT
		mp->m_funnel = FUNNEL_NULL;
#endif
		MOUNT_UNLOCK(mp);
		/*
		 * getmdev expects the vnode to be covered in ni_bypassvp
		 * to handle certain pathname combinations that could
		 * otherwise deadlock.  ni_bypassvp is an alias for
		 * ni_vp, so there's no extra work to be done here.
		 */
		if ((error = getmdev(&devvp, args.fspec, ndp)) != 0)
			return (error);
		if ((error = mountfs(devvp, mp, 0)) != 0) {
			vrele(devvp);
			return(error);
		}
#if	SEC_FSCHANGE
		if (error = sec_ufsmountcheck(mp)) {
			(void) ufs_unmount(mp, 0);
			return error;
		}
#endif
		fs = (VFSTOUFS(mp))->um_fs;
	} else {
		int flag = mp->m_flag;

		ump = VFSTOUFS(mp);
		MOUNT_UNLOCK(mp);
		if (!ump)
			return (ENODEV);
		fs = ump->um_fs;
		FS_LOCK(fs);
		if (fs->fs_ronly) {
			struct buf *bp;
			int s;

			/* make it switchable behavior */
			if (!updaterefresh)
				goto norefresh;
			else
				FS_UNLOCK(fs);
			/*
			 * Things to do to update the mount:
			 * 1) invalidate all cached file data.
			 * 2) invalidate all cached meta-data (devvp).
			 *    (done in mountfs()).
			 * 3) re-read superblock and cylinder group
			 *    information from disk (mountfs).
			 * 4) invalidate all inactive vnodes.
			 * 5) re-read inode data for all active vnodes.
			 *
			 * If changing from read-only to read-write:
			 * 6) change struct fs and update to disk.
			 *
			 * Synchronization note:
			 * Since the M_RDONLY flag may have already been
			 * cleared, there could be opens happening for
			 * write while we work.  This is OK, because
			 * no data will be flushed until the fs_ronly
			 * flag is cleared (see iupdat and ufs_sync), and 
			 * all in-core data and meta-data is about to
			 * be flushed or refreshed.
			 */
			devvp = ump->um_devvp;
			if (mntinvalbuf(mp))
				printf("ufs_mount: mntinvalbuf: dirty fs");
			/*
			 * Use mountfs, with update flag, to re-read
			 * meta-data into superblock.
			 */
			if (error = mountfs(devvp, mp, ump))
				return (error);
/*
			if (mountfs(devvp, mp, ump))
				panic("ufs_mount: cannot re-mount fs");
*/
			/*
			 * Flush inactive vnodes and 
			 * re-read inode data for active vnodes.
			 * Must happen before clearing fs_ronly flag.
			 */
			irefresh(mp);
			/*
	 	 	 * Save clean flag; unconditionally restored
			 * unmount so save it here.  
			 * Set fs_clean to 0 if mounting writable.
		 	 */
			FS_LOCK(fs);
norefresh:
			if ((flag & M_RDONLY) == 0) {
				fs->fs_ronly = 0;
				if (fs->fs_fmod != 0)
					panic("ufs_mount; rofs mod");
				fs->fs_flags = fs->fs_clean;
				fs->fs_clean = 0; /* mnted writable ==> dirty */
				/*
		 	 	 * Get the modified superblock written to disk.
		 	 	 */
				s = splhigh();
				TIME_READ_LOCK();
				fs->fs_time = time.tv_sec;
				TIME_READ_UNLOCK();
				splx(s);
				FS_UNLOCK(fs);
				/*
				 * check for sbupdate failure case,
				 * i.e. fs is readonly (i.e. cdrom).
				 */ 
				if (error = sbupdate(ump, MNT_WAIT)) {
					MOUNT_LOCK(mp);
					mp->m_flag |= M_RDONLY;
					MOUNT_UNLOCK(mp);
					FS_LOCK(fs);
					fs->fs_ronly = 1;
					fs->fs_clean = fs->fs_flags;
					fs->fs_fmod = 0;   /* just in case */
					FS_UNLOCK(fs);
					return(error);
				}
				FS_LOCK(fs);
			}
		}
		FS_UNLOCK(fs);
		/*
		 * Verify that the specified device is the one that
		 * is really being used for the root file system.
		 * It's possible for an alias to be in use, so we
		 * compare the rdev fields, not the vnodes.
		 */
		if (args.fspec == 0)
			return (0);
		/*
		 * getmdev expects the vnode to be covered in ni_bypassvp
		 * to handle certain pathname combinations that could
		 * otherwise deadlock.  ni_bypassvp is an alias for
		 * ni_vp, so there's no extra work to be done here.
		 */
		if ((error = getmdev(&devvp, args.fspec, ndp)) == 0) {
			if (devvp->v_rdev != ump->um_devvp->v_rdev) {
				error = EINVAL;	/* needs translation */
			}
			vrele(devvp);
		}
		if (error)
			return (error);
	}
	/*
	 * fs is set correctly
	 */
	(void) copyinstr(path, fs->fs_fsmnt, sizeof(fs->fs_fsmnt) - 1, &size);
	bzero(fs->fs_fsmnt + size, sizeof(fs->fs_fsmnt) - size);
	bcopy((caddr_t)fs->fs_fsmnt, (caddr_t)mp->m_stat.f_mntonname, MNAMELEN);
	(void) copyinstr(args.fspec, mp->m_stat.f_mntfromname, MNAMELEN - 1, 
		&size);
	bzero(mp->m_stat.f_mntfromname + size, MNAMELEN - size);
	(void) ufs_statfs(mp);
	return (0);
}

/*
 * Common code for mount and mountroot
 *
 * Synchronization assumptions:
 *	-- mp is NOT on global list (mount update doesn't get here), so
 *	   no mount structure locking required.
 *	-- Other attempted mounts, including forcible ones on this 
 *	   directory are locked out by mount().
 *	-- No lock needed on fs->*, since it's a new mount, except for
 *	   update case.
 */
mountfs(devvp, mp, update)
	struct vnode *devvp;
	struct mount *mp;
	struct ufsmount *update;
{
	register struct ufsmount *ump;
	struct buf *bp = NULL;
	register struct fs *fs;
	struct vnode *vp;
	struct partinfo dpart;
	struct devget dget;		/* DEVIOCGET ioctl */
	int ro_device = 0;
	caddr_t base, space;
	int havepart = 0, blks;
	int error, i, size;
	int retval;
	int needclose = 0;
	int setmounted = 0;
	int ronly = (mp->m_flag & M_RDONLY) != 0;

	/*
	 * Disallow multiple mounts of the same device.
	 * Disallow mounting of an open block device.
	 * Allow mounting of an open character device.
	 * Flush out any old buffers remaining from previous use.
	 */
	if (vinvalbuf(devvp, 1) && update)
		panic("mountfs: modified meta-data");
	if (update) {
		/*
		 * Leave FS read-only until we're ready to change it,
		 * which is after meta-data in inode cache has been
		 * refreshed.
		 */
		ronly = 1;
		ump = update;
		goto updateskip;
	}
	MOUNTTAB_LOCK();
	for (ump = &mounttab[0]; ump < &mounttab[nmount]; ump++) {
		if (ump->um_fs == NULL) 
			break;
	}
	if (ump >= &mounttab[nmount]) {
		MOUNTTAB_UNLOCK();
		error = EMFILE;			/* needs translation */
		goto out1;
	}
	ump->um_fs = (struct fs *)1;		/* just to reserve this slot */
	MOUNTTAB_UNLOCK();

/* In SVR4, driver open and close routines expect to get the OTYP_MNT flag
 * if the open/close was done as the result of a mount/unmount.  While OSF/1 
 * can pass a flag parameter to device open/close routines, it is only 
 * supported in spec_open() and spec_close(). These are the functions invoked
 * via VOP_OPEN and VOP_CLOSE for device special files. Therefore, we need 
 * to inform spec_open()/spec_close() that we are doing a mount/unmount.
 */
	VOP_OPEN(&devvp, (ronly ? FREAD : FREAD|FWRITE)|OTYP_MNT, NOCRED, error);

	if (error)
		goto out;
	needclose = 1;
	/*
	 * To determine if the device is already mounted or open, we call
	 * setmount(), which knows the magic.  We set the mounted flag
	 * at the same time to prevent races on mounting this device.
	 */
	if (setmount(devvp, SM_OPEN|SM_MOUNTED|SM_SETMOUNT)) {
		error = EBUSY;
		goto out;
	}
	setmounted = 1;
updateskip:
	/*
	 * determine readwrite characteristics of drive:
	 * 1. write-protected:  disallow mount read-write.
	 * 2. readonly-device:  non write capable, skip dirty check;
	 *	e.g. ultrix cdrom filesystems.
	 */
	VOP_IOCTL(devvp, DEVIOCGET, (caddr_t)&dget, FREAD, NOCRED, i, &retval);
	if (i == 0) {
		if ((mp->m_flag & M_RDONLY) == 0 &&
		    (dget.stat & (DEV_WRTLCK|DEV_RDONLY))) {
			error = EROFS;
			goto out;
		}
		ro_device = (dget.stat & DEV_RDONLY);
		if (ro_device && update)
			return (0);
	} else {
		/*
		 * couldn't get device status, we cannot determine
		 * a readonly-device, which is non-fatal.
		 */
	}
	VOP_IOCTL(devvp, DIOCGPART, (caddr_t)&dpart, FREAD, NOCRED, i, &retval);
	if (i != 0)
		size = DEV_BSIZE;
	else {
		havepart = 1;
		size = dpart.disklab->d_secsize;
	}
	/*
	 * read in super block, validate it.
	 */
	if (error = bread(devvp, SBLOCK, SBSIZE, NOCRED, &bp)) {
		goto out;
	}
	fs = bp->b_un.b_fs;
#if	SEC_FSCHANGE
        if (fs->fs_magic != FS_MAGIC && fs->fs_magic != FS_SEC_MAGIC ||
            fs->fs_bsize > MAXBSIZE || 
#ifdef __alpha
			fs->fs_bsize < (sizeof(struct fs) - ALPHA_EXT))
#else
			fs->fs_bsize < sizeof(struct fs))
#endif
#else
	if (fs->fs_magic != FS_MAGIC || fs->fs_bsize > MAXBSIZE ||
#ifdef __alpha
	    fs->fs_bsize < (sizeof(struct fs) - ALPHA_EXT))
#else
	    fs->fs_bsize < sizeof(struct fs))
#endif
#endif
	{
		error = EINVAL;		/* XXX also needs translation */
		goto out;
	}
	/*
	 * Only 8K file systems can be mounted
	 */
	if (fs->fs_bsize != 8192) {
		error = EOPNOTSUPP;	/* PRS - XXX 4K lockout */
		goto out;
	}
        /*
	 * Try to read last fragment
	 *
	 * Since the disk drivers do not mark reads past EOM with
	 * errors (only zero transfer counts), we must verify we
	 * can read last fragment.
	 */
	{
		struct buf *ebp;

		error = bread(devvp, fsbtodb(fs, fs->fs_size - 1),
		      	fs->fs_fsize, NOCRED, &ebp);
        	ebp->b_flags |= B_INVAL;
		if (error || ebp->b_resid) {
			brelse(ebp);
			error = EINVAL;
			goto out;
		}
		brelse(ebp);
	}

	/*
	 * Check for dirty file system.
	 */
	if (devvp != rootvp && (fs->fs_clean != FS_CLEAN) && 
	    !(mp->m_flag & M_FMOUNT) && !update) {
		if (!ro_device) {
			error = EDIRTY;
			goto out;
		}
	}

	/*
	 * Allocate space for cylinder group inforation.
 	 */
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
#if	MACH
	base = space = (caddr_t)kalloc(fs->fs_cssize);
	if (base == 0) {
		error = ENOMEM;
		goto out;
	}
#else
	base = space = (caddr_t)malloc((u_long)fs->fs_cssize, 
						M_SUPERBLK, M_WAITOK);
#endif

	/*
	 * Read in cylinder group inforation.
 	 */
	for (i = 0; i < blks; i += fs->fs_frag) {
		int size;
		struct buf *cbp;

		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks - i) * fs->fs_fsize;
		error = bread(devvp, fsbtodb(fs, fs->fs_csaddr + i), size,
			NOCRED, &cbp);
		if (error) {
			brelse(cbp);
#if	MACH
			kfree(base, fs->fs_cssize);
#else
			free((caddr_t)base, M_SUPERBLK);
#endif
			goto out;
		}
		bcopy((caddr_t)cbp->b_un.b_addr, space, (u_int)size);
		space += size;
		brelse(cbp);
	}

	/*
	 * THIS IS THE POINT OF NO RETURN FOR update.
	 *
	 * The super block for update has been carefully
	 * presered up to this point in case an error
	 * occurred. The old cylinder summary information
	 * is freed and the super block pointers to this
	 * information are build. No errors should occur
	 * beyond this point.
	 */
	if (update) {
#if	MACH
		kfree(ump->um_fs->fs_csp[0], fs->fs_cssize);
#else
		free((caddr_t)ump->um_fs->fs_csp[0], M_SUPERBLK);
#endif
	}

	/*
	 * Allocate superblock if not update.
	 */
	if (!update) {
#if	MACH
		/*
	 	* superblock zone has memory of size, MAXBSIZE.
	 	*/
		ZALLOC(superblock_zone, ump->um_fs, struct fs *);
#else
		ump->um_fs = (struct fs *)malloc(
#ifdef __alpha
					(u_long)(fs->fs_sbsize+ALPHA_EXT),
#else
					(u_long)fs->fs_sbsize, 
#endif /* __alpha */
					M_SUPERBLK, M_WAITOK);
#endif
		FS_LOCK_INIT(fs);
	}

	/*
	 * Synchronization issues:
	 * In the update case, is it possible that someone is
	 * trying to get to the struct fs, and will be actively using
	 * the lock.  We copy the new struct fs in parts, around the
	 * lock, to deal with current users.  It also eliminates the
	 * need to change all of the i_fs fields of open inodes
	 * to a new pointer.
	 * This only happens if UNIX_LOCKS is on (we have a lock
	 * in the struct fs).
	 *
	 * There is an assumption that while a filesystem is
	 * read-only, that the fs->fs_csp array WILL NOT be
	 * touched.  This is evidenced by the fact that we
	 * just freed its memory (above), and it will not
	 * be valid until after we fill in the cylinder
	 * group information.
	 */
#if	UNIX_LOCKS
	if (update) {
		struct fs *tfs;
		u_int copysize;
		/*
		 * Copy the new information, leaving the lock in place.
		 */
		fs = ump->um_fs;
		tfs = bp->b_un.b_fs;
		FS_LOCK(fs);
		copysize = (u_int)&fs->fs_un - (u_int)&fs->fs_link;
		bcopy((caddr_t)tfs, (caddr_t)fs, copysize);
		copysize = (u_int)fs->fs_sbsize - copysize - sizeof(fs->fs_un);
		bcopy((caddr_t)&tfs->fs_qbmask, (caddr_t)&fs->fs_qbmask, 
		      copysize);
		/* Build the cylinder group table */
		for (space = base, i = 0; i < blks; i += fs->fs_frag) {
			fs->fs_csp[fragstoblks(fs, i)] = (struct csum *)space;
			space += fs->fs_bsize;
		}
		fs->fs_ronly = ronly;
		FS_UNLOCK(fs);
	} else {
#endif
		bcopy((caddr_t)bp->b_un.b_addr, (caddr_t)ump->um_fs,
		      (u_int)fs->fs_sbsize);
		fs = ump->um_fs;
		/* Build the cylinder group table */
		for (space = base, i = 0; i < blks; i += fs->fs_frag) {
			fs->fs_csp[fragstoblks(fs, i)] = (struct csum *)space;
			space += fs->fs_bsize;
		}
		fs->fs_ronly = ronly;
#if	UNIX_LOCKS
	}
#endif
	fs->fs_fmod = 0;	/* just in case */
	if (fs->fs_sbsize < SBSIZE)
		bp->b_flags |= B_INVAL;
	brelse(bp);
	bp = NULL;

	/*
	 * Save clean flag.  We unconditionally restore fs_clean on unmount
	 * so save it here unconditionally.  Only set fs_clean to 0 if we
	 * are mounting writable.
	 */
	FS_LOCK(fs);
	fs->fs_flags = fs->fs_clean;
	if (ronly == 0) {
		fs->fs_fmod = 1;
		fs->fs_clean = 0;	/* mounted writable ==> dirty */
	}
	FS_UNLOCK(fs);
	if (havepart) {
		dpart.part->p_fstype = FS_BSDFFS;
		dpart.part->p_fsize = fs->fs_fsize;
		dpart.part->p_frag = fs->fs_frag;
		dpart.part->p_cpg = fs->fs_cpg;
	}
	/* Sanity checks for old file systems.			   XXX */
	FS_LOCK(fs);
	fs->fs_npsect = MAX(fs->fs_npsect, fs->fs_nsect);	/* XXX */
	fs->fs_interleave = MAX(fs->fs_interleave, 1);		/* XXX */
	if (fs->fs_postblformat == FS_42POSTBLFMT)		/* XXX */
		fs->fs_nrpos = 8;				/* XXX */
	FS_UNLOCK(fs);

	if (update) /* we're done */
		return (0);

	mp->m_data = (qaddr_t)ump;
	mp->m_stat.f_fsid.val[0] = devvp->v_rdev;
	mp->m_stat.f_fsid.val[1] = MOUNT_UFS;
	mp->m_flag |= M_LOCAL;
	ump->um_mountp = mp;
	ump->um_dev = devvp->v_rdev;
	ump->um_devvp = devvp;
#if	QUOTA
	for (i = 0; i < MAXQUOTAS; i++) {
		ump->um_quotas[i] = NULLVP;
		ump->um_cred[i] = NOCRED;
	}
	ump->um_qsync = 0;
	UMPQ_LOCK_INIT(ump);
	UMPQ_SYNC_LOCK_INIT(ump);
#endif
	/*
	 * Get the superblock written to disk
	 */
	if (error = ufs_sync(mp, 1)) {
		/*
		 * sync/sbupdate failed (not update case).
		 */
		FS_LOCK(fs);
		fs->fs_clean = fs->fs_flags;	/* revert       */
		fs->fs_fmod = 0;		/* just in case */
		FS_UNLOCK(fs);
		goto out;	/* cleanup devvp */
	}
	return (0);
out:
	if (bp)
		brelse(bp);
	if (needclose) {
		if (setmounted)
			(void) setmount(devvp, SM_CLEARMOUNT);
		VOP_CLOSE(devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED, i);
	}
	if (ump->um_fs == (struct fs *)1)
		ump->um_fs = NULL;
	else if (ump->um_fs) {
		if (!update) {
#if	MACH
			ZFREE(superblock_zone, ump->um_fs);
#else
			free((caddr_t)ump->um_fs, M_SUPERBLK);
#endif
			ump->um_fs = NULL;
		}
	}
out1:
	return (error);
}

/*
 * Make a filesystem operational.
 * Nothing to do at the moment.
 */
/* ARGSUSED */
ufs_start(mp, flags)
	struct mount *mp;
	int flags;
{

	return (0);
}

/*
 * unmount system call
 *
 * Synchronization assumptions:
 *	-- other unmounts are locked out.
 *	-- mp is still on global list, and is accessible.
 */
ufs_unmount(mp, mntflags)
	struct mount *mp;
	int mntflags;
{
	register struct ufsmount *ump;
	register struct fs *fs;
	int error, i, ronly, flags = 0;
	struct buf *bp;
#if	QUOTA
	int mflag;
#endif

#ifdef notdef
	if (mntflags & MNT_FORCE)
		flags |= FORCECLOSE;
#endif
	mntflushbuf(mp, 0);
	if (mntinvalbuf(mp))
		return (EBUSY);
	BM(MOUNT_LOCK(mp));
	ump = VFSTOUFS(mp);
#if	QUOTA
	mflag = mp->m_flag;
#endif	
	BM(MOUNT_UNLOCK(mp));
	if (!ump)
		return(ENODEV);
	vflushbuf(ump->um_devvp, B_SYNC);
	if (vinvalbuf(ump->um_devvp, 1))
		return(EBUSY);
	if ((ump->um_devvp)->v_flag & VIOERROR) {
		mp->m_flag |= M_IOERROR;
		(ump->um_devvp)->v_flag &= ~VIOERROR;
	}
#if	QUOTA
	/*
	 * It is not possible for any other processor to
	 * be setting these flags while we are unmounting.
	 *
	 * Because vfs_quotactl does a namei to find the
	 * filesystem it will operate on, we can be sure
	 * that any quotactl requests arriving after the
	 * unmount operation begins will pend at our
	 * filesystem's root until this unmount request
	 * terminates.  However, we must wait for any
	 * quotactl requests in progress to terminate.
	 */
	if (mflag & M_QUOTA) {
		if (error = vflush(mp, NULLVP, SKIPSYSTEM|flags))
			return (error);
		for (i = 0; i < MAXQUOTAS; i++)
			quotaoff(mp, i);
		/*
		 * Here we fall through to vflush again to ensure
		 * that we have gotten rid of all the system vnodes.
		 */
	}
#endif
	if (error = vflush(mp, NULLVP, flags))
		return (error);
	fs = ump->um_fs;
	ronly = !fs->fs_ronly;

	if (!fs->fs_ronly) {
		/*
	 	 * Restore the file system clean flag.
	 	 * This does "half" of an sbupdate()
	 	 */
		bp = getblk(ump->um_devvp, SBLOCK, (int)fs->fs_bsize);
		bcopy((caddr_t)fs, bp->b_un.b_addr, (u_int)fs->fs_sbsize);
		if (mp->m_flag & M_IOERROR) {
			bp->b_un.b_fs->fs_clean = 0;
			printf("Warning: %s: Write failures were detected\n",
			       mp->m_stat.f_mntfromname);
			printf("Warning: %s : Marking file system dirty\n",
			       mp->m_stat.f_mntfromname);
			printf("Warning: %s : Run fsck before mounting\n",
			       mp->m_stat.f_mntfromname);
		} else
			bp->b_un.b_fs->fs_clean = fs->fs_flags;
		bp->b_un.b_fs->fs_flags = 0;
		bwrite(bp);
	}

#if	MACH
	/*
	 * use kfree for the first one
	 */
	kfree(fs->fs_csp[0], fs->fs_cssize);
	ZFREE(superblock_zone, fs);
#else
	free((caddr_t)fs->fs_csp[0], M_SUPERBLK);
	free((caddr_t)fs, M_SUPERBLK);
#endif
	MOUNTTAB_LOCK();
	ump->um_fs = NULL;
	ump->um_dev = NODEV;
	MOUNTTAB_UNLOCK();
	/*
	 * Clear mounted flag from this vnode
	 */
	(void) setmount(ump->um_devvp, SM_CLEARMOUNT);

/* In SVR4, driver open and close routines expect to get the OTYP_MNT flag
 * if the open/close was done as the result of a mount/unmount.  While OSF/1 
 * can pass a flag parameter to device open/close routines, it is only 
 * supported in spec_open() and spec_close(). These are the functions invoked
 * via VOP_OPEN and VOP_CLOSE for device special files. Therefore, we need 
 * to inform spec_open()/spec_close() that we are doing a mount/unmount.
 */
	VOP_CLOSE(ump->um_devvp, (ronly ? FREAD : FREAD|FWRITE)|OTYP_MNT, NOCRED, error);

	(ump->um_devvp)->v_flag &= ~VIOERROR;
	vrele(ump->um_devvp);
	ump->um_devvp = (struct vnode *)0;
	MOUNT_LOCK(mp);
	mp->m_flag &= ~(M_LOCAL|M_IOERROR);
	MOUNT_UNLOCK(mp);
	return (error);
}


/*
 * Return root of a filesystem
 * Synchronization assumptions:
 *	-- it's safe, and file system isn't going anywhere.
 */
ufs_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	char fake_vnode[FAKE_INODE_SIZE];
	register struct inode *ip;
	struct inode *nip;
	struct vnode *tvp;
	int error;

	tvp = (struct vnode *) fake_vnode;
	fake_inode_init(tvp, mp);
	error = iget(VTOI(tvp), (ino_t)ROOTINO, &nip, 0);
	if (error)
		return (error);
	*vpp = ITOV(nip);
	return (0);
}

/*
 * Do operations associated with quotas
 */
ufs_quotactl(mp, cmds, uid, arg)
	struct mount *mp;
	int cmds;
	uid_t uid;
	caddr_t arg;
{
	register struct nameidata *ndp = &u.u_nd;
	struct ufsmount *ump = VFSTOUFS(mp);
	struct proc *p = u.u_procp;	/* XXX */
	int cmd, type, error;
	uid_t ruid;

#if	!QUOTA
	return (EOPNOTSUPP);
#else
	type = cmds & SUBCMDMASK;
	cmd = cmds >> SUBCMDSHIFT;

	switch(type) {
	case USRQUOTA:
		BM(PROC_LOCK(p));
		ruid = p->p_ruid;
		BM(PROC_UNLOCK(p));
		if (uid == NOUID)
			uid = ruid;
		break;
	case GRPQUOTA:
		if (uid == NOUID) {
			BM(PROC_LOCK(p));
			uid = (uid_t)p->p_rgid;
			BM(PROC_UNLOCK(p));
		}
		break;
	default:	
		return(EINVAL);
	}

	switch (cmd) {
	case Q_GETQUOTA:
	case Q_SYNC:
		if (type == USRQUOTA) {
			if (uid == ruid)
				break;
		} else {		/* type == GRPQUOTA */
			if (groupmember((gid_t)uid, ndp->ni_cred))
				break;
		}
		/* fall through */
	default:
#if	SEC_BASE
		if (!privileged(SEC_ACCT, EPERM))
			return(EPERM);
#else
		if (error = suser(ndp->ni_cred, &u.u_acflag))
			return (error);
#endif
	}

	if ((u_int)type >= MAXQUOTAS)
		return (EINVAL);

	switch (cmd) {

	case Q_QUOTAON:
		return (quotaon(ndp, mp, type, arg));

	case Q_QUOTAOFF:
		return (quotaoff(mp, type));

	case Q_SETQUOTA:
		return (setquota(mp, uid, type, arg));

	case Q_SETUSE:
		return (setuse(mp, uid, type, arg));

	case Q_GETQUOTA:
		return (getquota(mp, uid, type, arg));

	case Q_SYNC:
		return (qsync(mp));

	default:
		return (EINVAL);
	}
	/* NOTREACHED */
#endif
}

/*
 * Get file system statistics.
 *
 * Synchronization assumptions:
 *	-- File system isn't going anywhere.
 *	-- Lock order: mount structure, then struct fs.
 */
ufs_statfs(mp)
	struct mount *mp;
{
	register struct statfs *sbp;
	register struct fs *fs;

	MOUNT_LOCK(mp);
	sbp = &mp->m_stat;
	fs = (VFSTOUFS(mp))->um_fs;
#if	SEC_FSCHANGE
	if (fs->fs_magic != FS_MAGIC && fs->fs_magic != FS_SEC_MAGIC)
#else
	if (fs->fs_magic != FS_MAGIC)
#endif
		panic("ufs_statfs");
	sbp->f_type = MOUNT_UFS;
	sbp->f_fsize = fs->fs_fsize;
	sbp->f_bsize = fs->fs_bsize;
	sbp->f_blocks = fs->fs_dsize;
	FS_LOCK(fs);
	sbp->f_bfree = fs->fs_cstotal.cs_nbfree * fs->fs_frag +
		fs->fs_cstotal.cs_nffree;
	sbp->f_bavail = ((long)fs->fs_dsize * (100 - fs->fs_minfree) / 100) -
		(fs->fs_dsize - sbp->f_bfree);
	sbp->f_files =  fs->fs_ncg * fs->fs_ipg - ROOTINO;
	sbp->f_ffree = fs->fs_cstotal.cs_nifree;
	FS_UNLOCK(fs);
	MOUNT_UNLOCK(mp);
	return (0);
}

int	syncprt = 0;

/*
 * Go through the disk queues to initiate sandbagged IO;
 * go through the inodes to write those that have been modified;
 * initiate the writing of the super block if it has been modified.
 */
ufs_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{
	register struct vnode *vp;
	register struct ufsmount *ump = VFSTOUFS(mp);
	register struct fs *fs;
	register struct vnode *nvp;
	int error, allerror = 0;
	extern void bufstats();

	if (syncprt)
		bufstats();
	fs = ump->um_fs;
	/*
	 * Write back modified superblock.
	 * Consistency check that the superblock
	 * is still in the buffer cache.
	 */
	FS_LOCK(fs);
	if (fs->fs_fmod) {
		int s;

		if (fs->fs_ronly) {		/* XXX */
			printf("fs = %s\n", fs->fs_fsmnt);
			panic("update: rofs mod");
		}
		fs->fs_fmod = 0;
		s = splhigh();
		TIME_READ_LOCK();
		fs->fs_time = time.tv_sec;
		TIME_READ_UNLOCK();
		splx(s);
		FS_UNLOCK(fs);
		allerror = error = sbupdate(ump, waitfor);
	} else if (fs->fs_ronly) {
		/* Read-only */
		FS_UNLOCK(fs);
		return(0);
	} else {
		/* Super block not modified */
		FS_UNLOCK(fs);
	}
	/*
	 * Write back each (modified) inode.
	 */
	MOUNT_VLIST_LOCK(mp);
	for (vp = mp->m_mounth; vp; vp = nvp) {
		register struct inode *ip;

		/*
 		 * nvp will hold the vnode pointer to the vnode we're 
		 * flushing, which could be different from vp, the one
		 * that's on the mount vnode list, in the case of VBLK.
		 */
		VN_LOCK(vp);
		if (vp->v_type == VBLK) {
			if ((nvp = shadowvnode(vp)) == NULLVP) {
			    VN_UNLOCK(vp);
			    nvp = vp->v_mountf;
			    continue;
			}
		} else
			nvp = vp;
		VN_UNLOCK(vp);
		ip = VTOI(vp);
		IN_LOCK(ip);
		if ((ip->i_flag & (IMOD|IACC|IUPD|ICHG|ICHGMETA)) == 0) {
			IN_UNLOCK(ip);
			VN_LOCK(nvp);
			if (nvp->v_type != VREG && nvp->v_dirtyblkhd == NULL) {
				VN_UNLOCK(nvp);
				nvp = vp->v_mountf;
				continue;
			}
		} else {
			IN_UNLOCK(ip);
			VN_LOCK(nvp);
		}
		if (vget_nowait(nvp)) {
			VN_UNLOCK(nvp);
			nvp = vp->v_mountf;
			continue;
		}
		MOUNT_VLIST_UNLOCK(mp);
		if (nvp->v_type == VREG) {
			VN_UNLOCK(nvp);
			if (nvp->v_dirtyblkhd) vflushbuf(nvp, 0);
			/*
			 * Write out UBC cached file data. Take the inode
			 * read lock to guard against sync/trucate races.
			 */
			IN_READ_LOCK(ip);
			ubc_flush_dirty(nvp, B_ASYNC);
			IN_READ_UNLOCK(ip);
		}
		else if (nvp->v_dirtyblkhd) {
			VN_UNLOCK(nvp);
			vflushbuf(nvp, 0);
		} else
			VN_UNLOCK(nvp);
		IN_LOCK(ip);
		if (ip->i_flag & (IMOD|IACC|IUPD|ICHG|ICHGMETA)) {
			IN_UNLOCK(ip);
			if (error = iupdat(ip, &time, &time, 
					   waitfor == MNT_WAIT ? 1 : 0))
				allerror = error;
		} else
			IN_UNLOCK(ip);
		vrele(nvp);
		MOUNT_VLIST_LOCK(mp);
		if (vp->v_mount == mp)
			nvp = vp->v_mountf;
		else  {
			BUF_STATS(bio_stats.ufssync_misses++);
			nvp = mp->m_mounth;
		}
	}
	MOUNT_VLIST_UNLOCK(mp);
	/*
	 * Force stale file system control information to be flushed.
	 */
	vflushbuf(ump->um_devvp, waitfor == MNT_WAIT ? B_SYNC : 0);
#if	QUOTA
	qsync(mp);
#endif
	return (allerror);
}

/*
 * Write a superblock and associated information back to disk.
 */
sbupdate(mp, waitfor)
	struct ufsmount *mp;
	int waitfor;
{
	register struct fs *fs = mp->um_fs;
	register struct buf *bp;
	int blks;
	caddr_t space;
	int i, size, error = 0;

	if (fs->fs_ronly)
		return 0;
	bp = getblk(mp->um_devvp, SBLOCK, (int)fs->fs_sbsize);
	FS_LOCK(fs);
	bcopy((caddr_t)fs, bp->b_un.b_addr, (u_int)fs->fs_sbsize);
	FS_UNLOCK(fs);
	/* Restore compatibility to old file systems.		   XXX */
	if (fs->fs_postblformat == FS_42POSTBLFMT)		/* XXX */
		bp->b_un.b_fs->fs_nrpos = -1;			/* XXX */
	if (waitfor == MNT_WAIT)
		error = bwrite(bp);
	else
		bawrite(bp);
	blks = howmany(fs->fs_cssize, fs->fs_fsize);
	space = (caddr_t)fs->fs_csp[0];
	for (i = 0; i < blks; i += fs->fs_frag) {
		size = fs->fs_bsize;
		if (i + fs->fs_frag > blks)
			size = (blks - i) * fs->fs_fsize;
		bp = getblk(mp->um_devvp, fsbtodb(fs, fs->fs_csaddr + i), size);
		FS_LOCK(fs);
		bcopy(space, bp->b_un.b_addr, (u_int)size);
		FS_UNLOCK(fs);
		space += size;
		if (waitfor == MNT_WAIT)
			error = bwrite(bp);
		else
			bawrite(bp);
	}
	return (error);
}

/*
 * File handle to vnode
 *
 * Have to be really careful about stale file handles:
 * - check that the inode number is in range
 * - call iget() to get the locked inode
 * - check for an unallocated inode (i_mode == 0)
 * - check that the generation number matches
 *
 * Synchronization assumptions:
 *	-- File system isn't going anywhere.
 */
ufs_fhtovp(mp, fhp, vpp)
	register struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{
	char fake_vnode[FAKE_INODE_SIZE];
	register struct ufid *ufhp;
	register struct fs *fs;
	register struct inode *ip;
	struct inode *nip;
	struct vnode *tvp;
	int error;

	ufhp = (struct ufid *)fhp;
	fs = VFSTOUFS(mp)->um_fs;
	if (ufhp->ufid_ino < ROOTINO ||
	    ufhp->ufid_ino >= fs->fs_ncg * fs->fs_ipg) {
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	tvp = (struct vnode *) fake_vnode;
	fake_inode_init(tvp, mp);
	if (error = iget(VTOI(tvp), ufhp->ufid_ino, &nip, 0)) {
		*vpp = (struct vnode *)0;
		return (error);
	}
	ip = nip;
	IN_LOCK(ip);
	/*
	 * If the file has been unlinked or the inode has been reclaimed,
	 * we return EINVAL.
	 */
	if ((ip->i_mode == 0) ||
	    (ip->i_gen != ufhp->ufid_gen)) {
		IN_UNLOCK(ip);
		iput(ip);
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	*vpp = ITOV(ip);
	IN_UNLOCK(ip);
	return (0);
}

/*
 * Vnode pointer to File handle
 * Synchronization assumptions:
 *	-- i_gen and i_number are read-only
 */
/* ARGSUSED */
ufs_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{
	register struct inode *ip = VTOI(vp);
	register struct ufid *ufhp;

	ufhp = (struct ufid *)fhp;
	ufhp->ufid_len = sizeof(struct ufid);
	ufhp->ufid_ino = ip->i_number;
	ufhp->ufid_gen = ip->i_gen;
	return (0);
}

/*
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device vnode if so.
 *
 * The user may specify nasty combinations, e.g.:
 *	mount -u /
 *	mount /usr/devxxx /usr
 * in which case the proposed mount point already has VMOUNTING set,
 * a mount structure attached and lookups disabled.  We set BYPASSVP
 * in nameiop; the caller must supply the covered vnode to bypass in
 * ndp->ni_bypassvp.
 */
getmdev(devvpp, fname, ndp)
	struct vnode **devvpp;
	caddr_t fname;
	register struct nameidata *ndp;
{
	struct vnode *vp;
	int error, type;
	extern struct vnode *rootvp;

	ndp->ni_nameiop = LOOKUP | FOLLOW | BYPASSVP;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = fname;
	if (error = namei(ndp)) {
		/* ENOENT needs translation */
		return (error == ENOENT ? ENODEV : error);
	}
	vp = ndp->ni_vp;
	VN_LOCK(vp);
	type = vp->v_type;
	VN_UNLOCK(vp);
	if (type == VBLK) {
		if (major(vp->v_rdev) < nblkdev)
			*devvpp = vp;
		else
			error = ENXIO;
	} else
		error = ENOTBLK;
	if (error)
		vrele(vp);
	return (error);
}

/*
 * Loop through all of the vnodes associated with a mount point, and 
 * vgone those with v_usecount of 0, and refresh the in-core inode
 * data for those which are in use.  This function may only be called
 * for a filesystem that is mounted read-only.
 * This function looks a lot like vflush, with some changes.
 *
 * Synchronization:
 * Since the filesystem is read-only, the only way that vnodes will be
 * removed from the mount list is through recycling, when the system is
 * short on vnodes, so there's little risk of the list changing.  However,
 * to be safe, if it looks like it's changed, we start over at the
 * beginning, and run through the loop until it gets through clean.
 *
 * NOTE:
 * Some of what this function does implies internal knowledge of vfs_subr.c,
 * which we don't really want to do, but...
 */

void
irefresh(mp)
	struct mount *mp;
{
	register struct vnode *vp, *nvp;
	struct inode *nip;
	
	MOUNT_VLIST_LOCK(mp);
	for (vp = mp->m_mounth; vp; vp = nvp) {
		nvp = vp->v_mountf;
		MOUNT_VLIST_UNLOCK(mp);
		VN_LOCK(vp);
		if (vp->v_usecount == 0) {
			(void) vgone(vp, VX_SLEEP, (struct vnodeops *) 0);
			VN_UNLOCK(vp);
		} else {
			register struct inode *ip = VTOI(vp);
			VN_UNLOCK(vp);
			/*
			 * Setting the update parameter to iget will
			 * cause it to re-read the inode data from the
			 * disk, and release the vnode reference before
			 * returning.
			 */
			(void)iget(ip, ip->i_number, &nip, 1);
			if (nip != ip) {
				struct vnodeops *ops;
				extern struct vnodeops  dead_vnodeops,
							spec_vnodeops;
				printf("irefresh: failed iget, vp = %X\n", vp);
				VN_LOCK(vp);
				if (!wait_for_vxlock(vp, 1)) {
					if (vp->v_type == VCHR ||
					    vp->v_type == VBLK)
						ops = &spec_vnodeops;
					else
						ops = &dead_vnodeops;
					VN_UNLOCK(vp);
					(void)vclean(vp, 0, ops);
					(void)clear_vxlock(vp);
				} else
					VN_UNLOCK(vp);

			}
		}
		MOUNT_VLIST_LOCK(mp);
		/*
		 * If nvp is still non-null and on the list, use it;
		 * otherwise, restart the loop.
		 */
		if (nvp && (nvp->v_mount != mp))
			nvp = mp->m_mounth;
	}
	MOUNT_VLIST_UNLOCK(mp);
}
