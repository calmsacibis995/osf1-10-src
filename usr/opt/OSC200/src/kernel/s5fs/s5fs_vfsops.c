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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif
#ifndef lint
static char	*sccsid = "@(#)s5fs_vfsops.c	9.2	(ULTRIX/OSF)	10/18/91";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * s5fs_vfsops.c
 *
 *      Revision History:
 *
 * 17-Jun-91    Mitch Condylis
 *      Prevent non-root mounts in s5fs_mount.
 *	rerutn EPERM.
 * 18-Aug-91	Vipul Patel
 *	OSF/1 Release 1.0.1
 *	handcrafted vnode and inode in s5fs_fhtovp(0 and s5fs_root().
 *	lock vnode before accessing shadowvnode.
 */
/*

 */
#ifdef	i386
#include <cputypes.h>
#endif

#include <rt_preempt.h>
#include <ser_compat.h>
#include <sysv_fs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/buf.h>
#include <sys/ucred.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <mach/vm_param.h>
#include <vm/vm_kern.h>
#include <vm/vm_object.h>
#include <kern/zalloc.h>
#include <s5fs/s5param.h>
#include <s5fs/filsys.h>
#include <s5fs/s5mount.h>
#include <s5fs/s5inode.h>
#if	EXL
#include <machine/open.h>
#endif	/* EXL */
#include <sys/open.h>

zone_t	s5_superblock_zone;

/*
 * s5fs vfs operations.
 */
int s5fs_mount();
int s5fs_start();
int s5fs_unmount();
int s5fs_root();
int s5fs_quotactl();
int s5fs_statfs();
int s5fs_sync();
int s5fs_fhtovp();		/* XXX - to work on */
int s5fs_vptofh();		/* XXX - to work on */
int s5fs_init();
int s5fs_badop();

struct vfsops s5fs_vfsops = {
	s5fs_mount,
	s5fs_start,
	s5fs_unmount,
	s5fs_root,
	s5fs_quotactl,
	s5fs_statfs,
	s5fs_sync,
	s5fs_fhtovp,
	s5fs_vptofh,
	s5fs_init,
	s5fs_badop,	/* mountroot */
	s5fs_badop	/* swapvp */
};

/*
 * s5fs mount table.
 */
struct s5fsmount s5fs_mounttab[NMOUNT];

s5fs_badop()
{
	return(EINVAL);
}

s5fs_root(mp, vpp) 
	struct mount *mp;
	struct vnode **vpp;
{
	char	fake_s5_vnode[FAKE_S5INODE_SIZE];
	register struct s5inode *ip;	
	struct s5inode *nip;
	struct vnode *tvp;
	int error;

	tvp = (struct vnode *) fake_s5_vnode;
	fake_s5inode_init(tvp, mp);

	error = s5iget(S5VTOI(tvp), (ino_t)s5ROOTINO, &nip);
	if (error)
		return (error);
	*vpp = S5ITOV(nip);
	return(0);
}

/* Initialize System V fake inode */

fake_s5inode_init(tvp, mp)
struct vnode *tvp;
struct mount *mp;
{
	struct s5inode	*ip;
	struct s5fsmount *s_mp;

	tvp->v_mount = mp;
        VN_LOCK_INIT(tvp);
	ip = S5VTOI(tvp);
	ip->i_vnode = tvp;
	s_mp = VFSTOS5FS(mp);
	ip->i_dev = s_mp->um_dev;
	ip->i_s5fs = s_mp->um_fs;
	ip->i_devvp = s_mp->um_devvp;

/*
	printf("fake_s5inode: tvp->v_mount %x, ip->i_vnode %x, mp-> %x\n",
		tvp->v_mount, ip->i_vnode, mp);
*/
	return(0);
}


/*
 * VFS Operations.
 *
 * mount system call
 */
s5fs_mount(mp, path, data, ndp)
	struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
{
	struct vnode *devvp;
	s5fs_args args;
	struct s5fsmount *ump;
	register struct filsys *fs;
	u_int size;
	int error;

	/* Disable user mounts for s5fs */
	if (mp->m_uid)
		return (EPERM);

	if (error = copyin(data, (caddr_t)&args, sizeof (s5fs_args)))
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
	        if ((error = s5getmdev(&devvp, args.fspec, ndp)) != 0)
		        return (error);
		if ((error = s5mountfs(devvp, mp)) != 0) {
			vrele(devvp);
			return(error);
 	        }
		fs = (VFSTOS5FS(mp))->um_fs;
	} else {
		ump = VFSTOS5FS(mp);
		MOUNT_UNLOCK(mp);
		if (!ump)
			return (ENODEV);
		fs = ump->um_fs;
		/*
		 * Verify that the specified device is the one that
		 * is really being used for the root file system.
		 * It's possible for an alias to be in use, so we
		 * compare the rdev fields, not the vnodes.
		 */
		if (args.fspec == 0)
			return (0);
		if ((error = s5getmdev(&devvp, args.fspec, ndp)) == 0) {
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
	bzero((caddr_t)mp->m_stat.f_mntonname, MNAMELEN);
        (void) copyinstr(path, mp->m_stat.f_mntonname, 
			 sizeof(mp->m_stat.f_mntonname) - 1, &size);
        bcopy((caddr_t)mp->m_stat.f_mntonname, fs->s_fname,
	        sizeof(fs->s_fname));
        (void) copyinstr(args.fspec, mp->m_stat.f_mntfromname, MNAMELEN - 1,
                &size);
        bzero(mp->m_stat.f_mntfromname + size, MNAMELEN - size);
        /*
         * prime the struct statfs in the mount structure
         */
        (void) s5fs_statfs(mp);
 
	return (0);
}

/*
 * Common code for s5fs_mount (and s5fs_mountroot --- not clear if V.2 fs
 *	will be mounted as root fs.  Thus s5fs_mountroot() is not written
 *	yet.
 */
s5mountfs(devvp, mp)
	struct vnode *devvp;
	struct mount *mp;
{
	register struct s5fsmount *ump;
	struct s5fsmount *fmp = NULL;
	struct buf *bp = NULL;
	register struct filsys *fs;
	dev_t dev = devvp->v_rdev;
	caddr_t base, space;
	int error, i, size;
	int needclose = 0;
	int setmounted = 0;
	int ronly = (mp->m_flag & M_RDONLY) != 0;

  	/* 
  	 * Flush out any old buffers from previous use.
  	 */
  	(void) vinvalbuf(devvp, 1);
	for (ump = &s5fs_mounttab[0]; ump < &s5fs_mounttab[NMOUNT]; ump++) {
		if (ump->um_fs == NULL) {
			if (fmp == NULL)
				fmp = ump;
		} else if (dev == ump->um_dev) {
			return (EBUSY);		/* needs translation */
		}
	}
	if ((ump = fmp) == NULL)
		return (EMFILE);		/* needs translation */
	ump->um_fs = (struct filsys *)1;	/* just to reserve this slot */

/* In SVR4, driver open and close routines expect to get the OTYP_MNT flag
 * if the open/close was done as the result of a mount/unmount.  While OSF/1 
 * can pass a flag parameter to device open/close routines, it is only 
 * supported in spec_open() and spec_close(). These are the functions invoked
 * via VOP_OPEN and VOP_CLOSE for device special files. Therefore, we need 
 * to inform spec_open()/spec_close() that we are doing a mount/unmount.
 */
	VOP_OPEN(&devvp, (ronly ? FREAD : FREAD|FWRITE)|OTYP_MNT, NOCRED, error);

	if (error) {
		ump->um_fs = NULL;
		return (error);
	}
	needclose = 1;
	/*
	 * To determine if the device is already mounted or open, we call
	 * setmount(), which knows the magic.  We set the mounted flag
	 * at the same time to prevent races on mounting this device.
	 */
	if (setmount(devvp, SM_OPEN|SM_MOUNTED|SM_SETMOUNT)) {
		error = EBUSY;
		ump->um_fs = NULL;
		goto out;
	}
	setmounted = 1;
	if (error = bread(devvp, SUPERB, MAX_S5BSIZE, NOCRED, &bp)) {
		ump->um_fs = NULL;
		goto out;
	}
	fs = bp->b_un.b_s5fs;
	/* verify that we are mounting a System V file system */
	if (fs->s_magic != FsMAGIC || 
	   (fs->s_type > Fs3b) || (fs->s_type < Fs1b)) {
		ump->um_fs = NULL;
		error = EINVAL;		/* XXX also needs translation */
		goto out;
	}
	fs->s_ilock = 0;
	fs->s_flock = 0;
	fs->s_ninode = 0;
	fs->s_inode[0] = 0;
	/*
	 * superblock zone has memory of size, MAXBSIZE.
	 */
	ZALLOC(s5_superblock_zone, ump->um_fs, struct filsys *);
	bcopy((caddr_t)bp->b_un.b_s5fs, (caddr_t)ump->um_fs,
	   (u_int)sizeof(struct filsys));
	brelse(bp);
	bp = NULL;
	fs = ump->um_fs;
	fs->s_ronly = ronly;
	lock_init(&(ump)->um_fsflock, TRUE);
	lock_init(&(ump)->um_fsilock, TRUE);

	if (ronly == 0) fs->s_fmod = 1;
	mp->m_data = (qaddr_t)ump;
	mp->m_stat.f_fsid.val[0] = (long)dev;
	mp->m_stat.f_fsid.val[1] = MOUNT_S5FS;
	ump->um_mountp = mp;
	ump->um_dev = dev;
	ump->um_devvp = devvp;

	return (0);
out:
	if (needclose) {
		int err;
		if (setmounted)
			(void) setmount(devvp, SM_CLEARMOUNT);
		VOP_CLOSE(devvp, ronly ? FREAD : FREAD|FWRITE, NOCRED, err);
	}
	if (ump->um_fs) {
		ZFREE(s5_superblock_zone, ump->um_fs);
		ump->um_fs = NULL;
	}
	return (error);
}

/*
 * Make a filesystem operational.
 * Nothing to do at the moment.
 */
/* ARGSUSED */
s5fs_start(mp, flags)
	struct mount *mp;
	int flags;
{

	return (0);
}

/*
 * unmount system call
 */
s5fs_unmount(mp, mntflags)
	struct mount *mp;
	int mntflags;
{
	register struct s5fsmount *ump;
	register struct filsys *fs;
	int error, ronly, flags = 0;

	if (mntflags & MNT_FORCE)
		return (EINVAL);
	if (mntflags & MNT_FORCE)
		flags |= FORCECLOSE;
	mntflushbuf(mp, 0);
	if (mntinvalbuf(mp))
		return (EBUSY);
	ump = VFSTOS5FS(mp);
	if (error = vflush(mp, NULLVP, flags))
		return (error);

#if	MACH_NBC
	mfs_cache_clear();		/* remove cached mapped files */
#endif

	fs = ump->um_fs;
	ronly = !fs->s_ronly;

	ZFREE(s5_superblock_zone, fs);
	ump->um_fs = NULL;
	ump->um_dev = NODEV;
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
	vrele(ump->um_devvp);
	ump->um_devvp = (struct vnode *)0;
	return (error);
}


/*
 * Quota controls -- not supported on s5fs.
 */
s5fs_quotactl(mp, cmds, uid, arg)
	struct mount *mp;
	int cmds;
	uid_t uid;
	caddr_t arg;
{
#ifdef	lint
	mp = mp; cmds = cmds; uid = uid; arg = arg;
#endif
	return (EOPNOTSUPP);
}


/*
 * Get file system statistics.
 */
s5fs_statfs(mp)
	struct mount *mp;
{
	register struct statfs *sbp;
	register struct s5fsmount *ump;
	register struct filsys *fs;

	MOUNT_LOCK(mp);
	sbp = &mp->m_stat;
	ump = VFSTOS5FS(mp);
	MOUNT_UNLOCK(mp);
	fs = ump->um_fs;
	if (fs->s_magic != FsMAGIC)
		panic("s5fs_statfs");
	sbp->f_type = MOUNT_S5FS;
	sbp->f_fsize = FsBSIZE(fs);	/* fundamental filesystem block size */
	sbp->f_bsize = FsBSIZE(fs);	/* optimal transfer block size */
	sbp->f_blocks = fs->s_fsize;	/* total data blocks in file system */
	sbp->f_bfree = fs->s_tfree;	/* free blocks in fs */
	sbp->f_bavail = fs->s_tfree;	/* free blocks avail to non-su */
	                                /* total file nodes in file system */
	sbp->f_files =  fs->s_isize * FsINOPB(FsBSIZE(fs)) - s5ROOTINO;
	sbp->f_ffree = fs->s_tinode; 	/* free file nodes in fs */
	return (0);
}

int	s5fs_syncprt = 0;

/*
 * Go through the disk queues to initiate sandbagged IO;
 * go through the inodes to write those that have been modified;
 * initiate the writing of the super block if it has been modified.
 */
s5fs_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{
	register struct vnode *vp;
	register struct s5fsmount *ump = VFSTOS5FS(mp);
	register struct filsys *fs;
	struct vnode *nvp;
	int error, allerror = 0;
	static int updlock = 0;

	if (s5fs_syncprt)
		s5bufstats(ump->um_fs);
	if (updlock)
		return (EBUSY);
	fs = ump->um_fs;
	if (fs == (struct filsys *)1)
		return (0);
	updlock++;
	/*
	 * Write back modified superblock.
	 * Consistency check that the superblock
	 * is still in the buffer cache.
	 */
	if (fs->s_fmod != 0) {
		int s;
		if (fs->s_ronly != 0) {		/* XXX */
			printf("s5fs = %s\n", fs->s_fname);
			panic("update: rofs mod");
		}
		fs->s_fmod = 0;
		s = splhigh();
		TIME_READ_LOCK();
		fs->s_time = time.tv_sec;
		TIME_READ_UNLOCK();
		splx(s);
		error = s5bupdate(ump, waitfor);
	}
	/*
	 * Write back each (modified) inode.
	 */
loop:
	for (vp = mp->m_mounth; vp; vp = nvp) {
		register struct s5inode *ip;

		/*
 		 * nvp will hold the vnode pointer to the vnode we're 
		 * flushing, which could be different from vp, the one
		 * that's on the mount vnode list, in the case of VBLK.
		 */
		VN_LOCK(vp);
		if (vp->v_type == VBLK) {
			if ((nvp = shadowvnode(vp)) == NULLVP) {
			    VN_UNLOCK(vp);
			    if (vp) nvp = vp->v_mountf;
			    continue;
			}
		} else
			nvp = vp;
		ip = S5VTOI(vp);
		if ((ip->i_flag & (S5IACC | S5IUPD | S5ICHG)) == 0 
		    && nvp->v_dirtyblkhd == NULL) {
			nvp = vp->v_mountf;
			continue;
		}
		if (vget(nvp))
			goto loop;
		if (nvp->v_dirtyblkhd)
			vflushbuf(nvp, 0);
		if ((ip->i_flag & (S5IACC | S5IUPD | S5ICHG)) &&
		    (error = s5iupdat(ip, &time, &time)))
			allerror = error;
		vrele(nvp);
		if (vp->v_mount == mp)
			nvp = vp->v_mountf;
		else
			nvp = mp->m_mounth;
	}
	updlock = 0;
	/*
	 * Force stale file system control information to be flushed.
	 */
	vflushbuf(ump->um_devvp, waitfor == MNT_WAIT ? B_SYNC : 0);
	return (allerror);
}

/*
 * Write a System V superblock back to disk.
 */
s5bupdate(mp, waitfor)
	struct s5fsmount *mp;
	int waitfor;
{
	register struct filsys *fs = mp->um_fs;
	register struct buf *bp;
	int blks;
	caddr_t space;
	int i, size, error = 0;

	bp = getblk(mp->um_devvp, SUPERB, (int)(sizeof (struct filsys)));
	bcopy((caddr_t)fs, bp->b_un.b_addr, (u_int)(sizeof (struct filsys)));
	if (waitfor == MNT_WAIT)
		error = bwrite(bp);
	else
		bawrite(bp);
	return (error);
}

/*
 * Print out statistics on the current allocation of the buffer pool.
 * Can be enabled to print out on every ``sync'' by setting "s5fs_syncprt"
 * above.
 *
 * Note: the array counts is sized by MAX_S5FSIZE to allow for the max
 *	 block size supported on System V fs.
 */
s5bufstats(fs)
	struct filsys *fs;
{
	int s, i, j, count;
	register struct buf *bp, *dp;
#ifdef	__alpha
	int counts[MAX_S5BSIZE/8192+1];		/* allow for the max size */
#else
	int counts[MAX_S5BSIZE/CLBYTES+1];	/* allow for the max size */
#endif
	static char *bname[BQUEUES] = { "LOCKED", "LRU", "AGE", "EMPTY" };

	for (bp = bfreelist, i = 0; bp < &bfreelist[BQUEUES]; bp++, i++) {
		count = 0;
		for (j = 0; j <= FsBSIZE(fs)/CLBYTES; j++)
			counts[j] = 0;
		s = splbio();
		for (dp = bp->av_forw; dp != bp; dp = dp->av_forw) {
			counts[dp->b_bufsize/CLBYTES]++;
			count++;
		}
		splx(s);
		printf("%s: total-%d", bname[i], count);
		for (j = 0; j <= FsBSIZE(fs)/CLBYTES; j++)
			if (counts[j] != 0)
				printf(", %d-%d", j * CLBYTES, counts[j]);
		printf("\n");
	}
}

/*
 * File handle to vnode
 *
 * Have to be really careful about stale file handles:
 * - check that the inode number is in range
 * - call iget() to get the locked inode
 * - check for an unallocated inode (i_mode == 0)
 * - check that the generation number matches
 */
s5fs_fhtovp(mp, fhp, vpp)			
	register struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{
	char	fake_s5_vnode[FAKE_S5INODE_SIZE];
        register struct ufid *ufhp;
	register struct filsys *fs;
	struct s5inode *ip;
	struct s5inode *nip;
	struct vnode *tvp;
	int error;

	ufhp = (struct ufid *)fhp;
	fs = VFSTOS5FS(mp)->um_fs;
	if (ufhp->ufid_ino < s5ROOTINO ||
	    ufhp->ufid_ino >= fs->s_isize * FsINOPB(FsBSIZE(fs))) {
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}

	tvp = (struct vnode *) fake_s5_vnode;
	fake_s5inode_init(tvp, mp);

	if (error = s5iget(S5VTOI(tvp), ufhp->ufid_ino, &nip)) {
		*vpp = (struct vnode *)0;
		return (error);
	}
	ip = nip;
	if (ip->i_mode == 0) {
		s5iput(ip);
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	if (ip->i_gen != ufhp->ufid_gen) {
		s5iput(ip);
		*vpp = (struct vnode *)0;
		return (EINVAL);
	}
	*vpp = S5ITOV(ip);
	return (0);
}

/*
 * Vnode pointer to File handle
 */
s5fs_vptofh(vp, fhp)		
	struct vnode *vp;
	struct fid *fhp;
{
	register struct s5inode *ip = S5VTOI(vp);
	register struct ufid *ufhp;

	ufhp = (struct ufid *)fhp;
	ufhp->ufid_len = sizeof(struct ufid);
	ufhp->ufid_ino = ip->i_number;
	ufhp->ufid_gen = ip->i_gen;
	return (0);
}

/*
 * Used by mount.
 * Check that the user's argument is a reasonable
 * thing on which to mount, and return the device number if so.
 */
s5getmdev(devvpp, fname, ndp)
	struct vnode **devvpp;
	caddr_t fname;
	register struct nameidata *ndp;
{
	register struct vnode *vp;
	int error;

/*	ndp->ni_nameiop = LOOKUP | LOCKLEAF | FOLLOW; */
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = fname;
	if (error = namei(ndp)) {
		if (error == ENOENT)
			return (ENODEV);	/* needs translation */
		return (error);
	}
	vp = ndp->ni_vp;
	if (vp->v_type != VBLK) {
		vrele(vp);
		return (ENOTBLK);
	}
	if (major(vp->v_rdev) >= nblkdev) {
		vrele(vp);
		return (ENXIO);
	}
	s5IUNLOCK(S5VTOI(vp));
	*devvpp = vp;
	return (0);
}
