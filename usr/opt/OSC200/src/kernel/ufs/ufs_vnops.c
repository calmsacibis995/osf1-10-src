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
static char	sccsid[] = "@(#)$RCSfile: ufs_vnops.c,v $ $Revision: 4.2.37.18 $ (DEC) $Date: 1994/01/18 22:57:14 $";
#endif 
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *	@(#)ufs_vnops.c	7.27 (Berkeley) 1/13/90
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <sys/secpolicy.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/buf.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/conf.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/flock.h>
#include <ufs/quota.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/fs_proto.h>
#if	MACH
#include <sys/syslimits.h>
#include <kern/assert.h>
#include <kern/parallel.h>
#include <kern/zalloc.h>
#include <kern/kalloc.h>
#endif
#include <mach/mach_types.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <sys/vfs_ubc.h>
#include <vm/vm_mmap.h>
#include <vm/vm_vp.h>
#include <vm/vm_debug.h>

#include <dcedfs.h>

#if defined(DCEDFS) && DCEDFS
enum vnuio_rw { VNUIO_READ, VNUIO_WRITE, VNUIO_READNESTED, VNUIO_WRITENESTED };
#endif /* DCEDFS */

int chown_restricted = 0;
extern int sys_v_mode;

/*
 * Global vfs data structures for ufs
 */

int	ufs_lookup(),
	ufs_create(),
	ufs_mknod(),
	ufs_open(),
	ufs_close(),
	ufs_access(),
	ufs_getattr(),
	ufs_setattr(),
	ufs_read(),
	ufs_write(),
	ufs_ioctl(),
	seltrue(),
	ufs_mmap(),
	ufs_fsync(),
	ufs_seek(),
	ufs_remove(),
	ufs_link(),
	ufs_rename(),
	ufs_mkdir(),
	ufs_rmdir(),
	ufs_symlink(),
	ufs_readdir(),
	ufs_readlink(),
	ufs_abortop(),
	ufs_inactive(),
	ufs_reclaim(),
	ufs_bmap(),
	ufs_strategy(),
	ufs_print(),
	ufs_page_read(),
	ufs_page_write(),
	ufs_getpage(),
	ufs_putpage(),
	ufs_swap(),
	ufs_bread(),
	ufs_brelse(),
	ufs_lockctl(),
	ufs_setvlocks(),
	ufs_syncdata();

struct vnodeops ufs_vnodeops = {
	ufs_lookup,		/* lookup */
	ufs_create,		/* create */
	ufs_mknod,		/* mknod */
	ufs_open,		/* open */
	ufs_close,		/* close */
	ufs_access,		/* access */
	ufs_getattr,		/* getattr */
	ufs_setattr,		/* setattr */
	ufs_read,		/* read */
	ufs_write,		/* write */
	ufs_ioctl,		/* ioctl */
	seltrue,		/* select */
	ufs_mmap,		/* mmap */
	ufs_fsync,		/* fsync */
	ufs_seek,		/* seek */
	ufs_remove,		/* remove */
	ufs_link,		/* link */
	ufs_rename,		/* rename */
	ufs_mkdir,		/* mkdir */
	ufs_rmdir,		/* rmdir */
	ufs_symlink,		/* symlink */
	ufs_readdir,		/* readdir */
	ufs_readlink,		/* readlink */
	ufs_abortop,		/* abortop */
	ufs_inactive,		/* inactive */
	ufs_reclaim,		/* reclaim */
	ufs_bmap,		/* bmap */
	ufs_strategy,		/* strategy */
	ufs_print,		/* print */
	ufs_page_read,		/* page_read */
	ufs_page_write,		/* page_write */
	ufs_getpage,		/* get page */
	ufs_putpage,		/* put page */
	ufs_swap,		/* swap handler */
	ufs_bread,		/* buffer read */
	ufs_brelse,		/* buffer release */
	ufs_lockctl,		/* file locking */
	ufs_syncdata,		/* fsync byte range */
};

int	spec_lookup(),
	spec_open(),
	ufsspec_read(),
	ufsspec_write(),
	spec_strategy(),
	spec_bmap(),
	spec_ioctl(),
	spec_select(),
	spec_seek(),
	ufsspec_close(),
	ufsspec_reclaim(),
	spec_badop(),
	spec_nullop(),
	spec_mmap(),
	spec_swap(),
	spec_lockctl();

extern int spec_bread(), spec_brelse();

struct vnodeops spec_inodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	spec_open,		/* open */
	ufsspec_close,		/* close */
	ufs_access,		/* access */
	ufs_getattr,		/* getattr */
	ufs_setattr,		/* setattr */
	ufsspec_read,		/* read */
	ufsspec_write,		/* write */
	spec_ioctl,		/* ioctl */
	spec_select,		/* select */
	spec_mmap,		/* mmap */
	spec_nullop,		/* fsync */
	spec_seek,		/* seek */
	spec_badop,		/* remove */
	spec_badop,		/* link */
	spec_badop,		/* rename */
	spec_badop,		/* mkdir */
	spec_badop,		/* rmdir */
	spec_badop,		/* symlink */
	spec_badop,		/* readdir */
	spec_badop,		/* readlink */
	spec_badop,		/* abortop */
	ufs_inactive,		/* inactive */
	ufsspec_reclaim,	/* reclaim */
	spec_bmap,		/* bmap */
	spec_strategy,		/* strategy */
	ufs_print,		/* print */
	spec_badop,		/* page_read */
	spec_badop,		/* page_write */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	spec_swap,		/* swap */
	spec_bread,		/* buffer read */
	spec_brelse,		/* buffer release */
	spec_lockctl,           /* file locking */
	spec_nullop,		/* fsync byte range */
};

int	fifo_open(),
	ufsfifo_close(),
	ufsfifo_read(),
	ufsfifo_write(),
	fifo_ioctl(),
	ufsfifo_getattr(),
	fifo_select();

extern int fifo_bread(), fifo_brelse();

struct vnodeops fifo_inodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	fifo_open,		/* open */
	ufsfifo_close,		/* close */
	ufs_access,		/* access */
	ufsfifo_getattr,	/* getattr */
	ufs_setattr,		/* setattr */
	ufsfifo_read,		/* read */
	ufsfifo_write,		/* write */
	fifo_ioctl,		/* ioctl */
	fifo_select,		/* select */
	spec_badop,		/* mmap */
	spec_nullop,		/* fsync */
	spec_seek,		/* seek */
	spec_badop,		/* remove */
	spec_badop,		/* link */
	spec_badop,		/* rename */
	spec_badop,		/* mkdir */
	spec_badop,		/* rmdir */
	spec_badop,		/* symlink */
	spec_badop,		/* readdir */
	spec_badop,		/* readlink */
	spec_badop,		/* abortop */
	ufs_inactive,		/* inactive */
	ufs_reclaim,		/* reclaim */
	spec_bmap,		/* bmap */
	spec_badop,		/* strategy */
	ufs_print,		/* print */
	spec_badop,		/* page_read */
	spec_badop,		/* page_write */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	spec_swap,		/* swap */
	fifo_bread,		/* buffer read */
	fifo_brelse,		/* buffer release */
	spec_lockctl,           /* file locking */
	spec_nullop,		/* fsync byte range */
};

enum vtype iftovt_tab[16] = {
	VNON, VFIFO, VCHR, VNON, VDIR, VNON, VBLK, VNON,
	VREG, VNON, VLNK, VNON, VSOCK, VNON, VNON, VBAD,
};
int	vttoif_tab[9] = {
	0, IFREG, IFDIR, IFBLK, IFCHR, IFLNK, IFSOCK, IFIFO, IFMT,
};

/*
 * Create a regular file
 */
ufs_create(ndp, vap)
	struct nameidata *ndp;
	struct vattr *vap;
{
	struct inode *ip;
	int error;

	if (error = maknode(vap, ndp, &ip))
		return (error);
	ndp->ni_vp = ITOV(ip);
	return (0);
}

/*
 * Mknod vnode call
 */
/* ARGSUSED */
ufs_mknod(ndp, vap, cred)
	struct nameidata *ndp;
	struct ucred *cred;
	register struct vattr *vap;
{
	struct inode *ip;
	int error;

	if (vap->va_rdev != (dev_t)VNOVAL) {
		if (vap->va_type != VBLK && vap->va_type != VCHR)
			return (EINVAL);
	}
	if (error = maknode(vap, ndp, &ip))
		return (error);
	iput(ip);
	return (0);
}

/*
 * Open called.
 *
 * Nothing to do.
 */
/* ARGSUSED */
ufs_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{

	return (0);
}

/*
 * Close called
 *
 * Update the times on the inode.
 */
/* ARGSUSED */
ufs_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);

	BM(VN_LOCK(vp));
	if (vp->v_usecount > 1) {
		u_int	flag;

		BM(VN_UNLOCK(vp));
		IN_LOCK(ip);
		flag = ip->i_flag & IREADERROR;
		IN_UNLOCK(ip);
		/*
		 * If there was an error initializing the inode, we don't
		 * want to do anything.  We will follow this path
		 * because vclean will bump the reference count.
		 */
		if (!flag)
			ITIMES(ip, &time, &time);
	} else
		BM(VN_UNLOCK(vp));
	return (0);
}

ufs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	struct inode *ip = VTOI(vp);
#if	QUOTA
	if (mode & VWRITE) {
		int error;
		switch (vp->v_type) {
		case VREG: case VDIR: case VLNK:
			if (error = getinoquota(ip))
				return (error);
		}
	}
#endif
	return (iaccess(ip, mode, cred));
}

/* ARGSUSED */
ufs_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);

	enum vtype type;

	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	ITIMES(ip, &time, &time);
	/*
	 * Copy from inode table
	 */
	IN_LOCK(ip);
	vap->va_fsid = ip->i_dev;
#if SEC_MAC
	if ((ip->i_type_flags & SEC_I_MLDCHILD) &&
	    (check_privileges && !privileged(SEC_MULTILEVELDIR, 0)))
	    vap->va_fileid = ip->i_parent;
	else
		vap->va_fileid = ip->i_number;
#else
	vap->va_fileid = ip->i_number;
#endif
	vap->va_mode = ip->i_mode & ~IFMT;
	vap->va_nlink = ip->i_nlink;
	vap->va_uid = ip->i_uid;
	vap->va_gid = ip->i_gid;
	vap->va_rdev = (dev_t)ip->i_rdev;
#ifdef __alpha
	vap->va_qsize = ip->i_din.di_qsize;
#else
	vap->va_qsize.val[0] = ip->i_din.di_qsize.val[0];
	vap->va_qsize.val[1] = ip->i_din.di_qsize.val[1];
#endif
	vap->va_atime.tv_sec = ip->i_atime;
	vap->va_atime.tv_usec = ip->i_uatime;
	vap->va_mtime.tv_sec = ip->i_mtime;
	vap->va_mtime.tv_usec = ip->i_umtime;
	vap->va_ctime.tv_sec = ip->i_ctime;
	vap->va_ctime.tv_usec = ip->i_uctime;
	vap->va_flags = ip->i_flags;
	vap->va_gen = ip->i_gen;
	/* this doesn't belong here */
	if (type == VBLK)
		vap->va_blocksize = BLKDEV_IOSIZE;
	else if (type == VCHR)
		vap->va_blocksize = MAXBSIZE;
	else
		vap->va_blocksize = ip->i_fs->fs_bsize;
	vap->va_bytes = dbtob(ip->i_blocks);
	IN_UNLOCK(ip);
#ifndef __alpha
	vap->va_bytes_rsv = -1;
#endif
	vap->va_type = type;
	return (0);
}

/*
 * Set attribute vnode op. called from several syscalls
 */
ufs_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	register struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);
	int error = 0;
	uid_t iuid;
	gid_t igid;

	/*
	 * Check for unsetable attributes.
	 */
	if ((vap->va_type != VNON) || (vap->va_nlink != (short)VNOVAL) ||
	    (vap->va_fsid != (int)VNOVAL) || (vap->va_fileid != (int)VNOVAL) ||
	    (vap->va_blocksize != (int)VNOVAL) || 
	    (vap->va_rdev != (dev_t)VNOVAL) ||
#if __alpha
	    (vap->va_bytes != VNOVAL) || 
#else
	    ((int)vap->va_bytes != VNOVAL) || 
#endif
	    (vap->va_gen != (u_int)VNOVAL)) {
		return (EINVAL);
	}
	/*
	 * Go through the fields and update if not VNOVAL.
	 *
	 * Since NFS VOP_SETATTR() calls are followed by VOP_FSYNC(),
	 * these ops don't have to be synchronous with the exception
	 * of itrunc(). Since itrunc() always calls iupdat() waiting for
	 * the inode to go to disk, we just need to set ICHGMETA which
	 * indicate the on-disk inode needs to be updated.
	 */
	if (vap->va_uid != (uid_t) VNOVAL || vap->va_gid != (gid_t) VNOVAL) {

		if (error = chown1(ip, vap->va_uid, vap->va_gid, cred))
			return (error);
		IN_LOCK(ip);
		ip->i_flag |= ICHGMETA;
		IN_UNLOCK(ip);
	}
	if (vap->va_size != (u_long)VNOVAL) {
		u_short mask = ISUID;
		BM(VN_LOCK(vp));
		if (vp->v_type == VDIR) {
			BM(VN_UNLOCK(vp));
			return (EISDIR);
		}
		BM(VN_UNLOCK(vp));
		/*
		 * itrunc() does the iupdat()
		 */
		if (error = itrunc(ip, vap->va_size, 0))
			return (error);
		IN_LOCK(ip);
		/*
		 * Don't clear enforcement mode lock bits, 
		 * indicated by setgid bit, but no group execute.
		 */
		if (!(ip->i_mode & ISGID) || (ip->i_mode & S_IXGRP))
			if (sys_v_mode == 0) {
				mask |= ISGID;
			}
		ip->i_mode &= ~mask;
		IN_UNLOCK(ip);
	}
	BM(IN_LOCK(ip));
	iuid = ip->i_uid;
	igid = ip->i_gid;
	BM(IN_UNLOCK(ip));
	if (vap->va_atime.tv_sec != (int)VNOVAL || 
	    vap->va_mtime.tv_sec != (int)VNOVAL) {
#if SEC_BASE
		if (security_is_on) {
#if SEC_ARCH
		   if (error = iaccess(ip, SP_SETATTRACC, cred))
			return error;
#endif
		   if(cred->cr_uid != iuid && !privileged(SEC_OWNER, EPERM))
			return EPERM;
		} else
#endif
		if (cred->cr_uid != iuid && 
		   (error = iaccess(ip, IWRITE, cred))) {
		        if (error == EACCES) error = EPERM;
			return(error);
	        }

		IN_LOCK(ip);
		if (vap->va_atime.tv_sec != (int)VNOVAL)
			ip->i_flag |= IACC;
		if (vap->va_mtime.tv_sec != (int)VNOVAL)
			ip->i_flag |= IUPD;
		ip->i_flag |= (ICHG|ICHGMETA);
		IN_UNLOCK(ip);
		if (error = iupdat(ip, &vap->va_atime, &vap->va_mtime, 0))
			return (error);
	}
	if (vap->va_mode != (u_short) VNOVAL) {
		error = chmod1(ip, (int)vap->va_mode, cred);
		IN_LOCK(ip);
		ip->i_flag |= ICHGMETA;
		IN_UNLOCK(ip);
	}
	if (vap->va_flags != (u_int)VNOVAL) {
#if SEC_ARCH
		if(security_is_on && (error = iaccess(ip, SP_SETATTRACC, cred)))
			return error;
#endif
#if SEC_BASE
		if (cred->cr_uid != iuid && !privileged(SEC_OWNER, EPERM))
			return EPERM;
#else 
		if (cred->cr_uid != iuid &&
		    (error = suser(cred, &u.u_acflag))) 	/* XXX */
			return (error);
#endif	/* SEC_BASE */

		IN_LOCK(ip);
#if SEC_BASE
		if (security_is_on) {
		if (iuid = privileged(SEC_FILESYS, 0))
			ip->i_flags = vap->va_flags;
		} else
#endif	/* SEC_BASE */
		if (iuid = (cred->cr_uid == 0))
			ip->i_flags = vap->va_flags;

		if (iuid == 0) {
			ip->i_flags &= 0xffff0000;
			ip->i_flags |= (vap->va_flags & 0xffff);
		}

		ip->i_flag |= (ICHG|ICHGMETA);
		IN_UNLOCK(ip);
	}
	return (error);
}


/*
 * Change the mode on a file.
 */
chmod1(ip, mode, cred)
	register struct inode *ip;
	register int mode;
	struct ucred *cred;
{
	int error = 0;
#if SEC_ARCH
	int ret;
	dac_t dac;
#endif

	/*
	 * We need to synchronize with other threads doing chmods and
	 * chowns so we keep the inode locked for a while.
	 */
	IN_LOCK(ip);
#if SEC_BASE
	if (cred->cr_uid != ip->i_uid && !privileged(SEC_OWNER, EPERM) ||
		!sec_mode_change_permitted(mode)) {
		IN_UNLOCK(ip);
		return EPERM;
	}
#else
	if (cred->cr_uid != ip->i_uid && (error = suser(cred, &u.u_acflag))) {
		IN_UNLOCK(ip);
		return (error);
	}
#endif
#if SEC_ARCH
	/* 
	 * Must unlock inode across this call; otherwise deadlock!
	 */

	if (security_is_on) {
		IN_UNLOCK(ip);
		VOP_ACCESS(ITOV(ip), SP_SETATTRACC, cred, error);
		if (error)
			return (error);
		IN_LOCK(ip);
	}
#endif

	ip->i_mode &= ~07777;

#if	SEC_BASE
/* XXX -- the inode is locked!!! check it out */
     if(check_privileges) {
	if ((ip->i_mode & IFMT) != IFDIR && (mode & ISVTX) &&
	    !privileged(SEC_LOCK, 0))
		mode &= ~ISVTX;
	if (!groupmember(ip->i_gid, cred) && (mode & ISGID) &&
	    !privileged(SEC_SETPROCIDENT, 0))
		mode &= ~ISGID;
#if SEC_PRIV
	/*
	 * If we are setting the SUID bit of a file owned by root, and
	 * the file resides on a secure filesystem, and we have the
	 * supropagate privilege, add the sucompat privilege to the
	 * file's potential and granted privilege sets.
	 * MP note: inode lock depended upon to make ADDBIT atomic
	 */
	if ((mode & ISUID) && ip->i_uid == 0 && VSECURE(ITOV(ip)) &&
	    privileged(SEC_SUPROPAGATE, 0)) {
		ADDBIT(ip->i_ppriv, SEC_SUCOMPAT);
		ADDBIT(ip->i_gpriv, SEC_SUCOMPAT);
	}
#endif
     } else
#endif /* SEC_BASE */
	if (cred->cr_uid) {
		if ((ip->i_mode & IFMT) != IFDIR)
			mode &= ~ISVTX;
		if (!groupmember(ip->i_gid, cred))
			mode &= ~ISGID;
	}
	ip->i_mode |= mode & 07777;
	ip->i_flag |= ICHG;
	IN_UNLOCK(ip);
#if	SEC_ARCH
	if (security_is_on) {
	  dac.uid = ip->i_uid;
	  dac.gid = ip->i_gid;
	  dac.mode = ip->i_mode & 0777;
	  IN_UNLOCK(ip);
	  ret = SP_CHANGE_OBJECT(ip->i_tag, &dac, SEC_NEW_MODE);
	  IN_LOCK(ip);
	  if (ret) {
		IN_LOCK(ip);
		if (ret & SEC_NEW_UID)
			ip->i_uid = dac.uid;
		if (ret & SEC_NEW_GID)
			ip->i_gid = dac.gid;
		if (ret & SEC_NEW_MODE)
			ip->i_mode = (ip->i_mode & ~0777) | (dac.mode & 0777);
		IN_UNLOCK(ip);
	  }
	}
#endif /* SEC_ARCH */
#if	MACH
	/* Mach VM system pays no attention to ISVTX bit. */
#else
	if ((vp->v_flag & VTEXT) && (ip->i_mode & ISVTX) == 0)
		xrele(vp);
#endif
	return (0);
}

/*
 * Perform chown operation on inode ip.
 *
 * Take the inode I/O lock for writing to
 * break chown1 races.  While ugly, doing so
 * considerably simplifies chown1, simplifies
 * other code (such as quotas) that depends
 * heavily on uids, and has little impact
 * on performance.  (To be honest, quotas
 * provided the strongest motivation; dealing
 * with all the possible chown1/chown1,
 * chown1/getinoquota, etc. races was just
 * too hairy.)
 *
 * The inode uid and gid are still altered under
 * inode incore lock, preserving locking assumptions
 * for other users of the inode uid and gid.
 */
chown1(ip, uid, gid, cred)
	register struct inode *ip;
	uid_t uid;
	gid_t gid;
	struct ucred *cred;
{
	uid_t ouid;
	gid_t ogid;
	int error;
#if SEC_ARCH
	int ret;
	dac_t dac;
#endif

	/*
	 * We need to synchronize with other threads racing to do
	 * chowns and chmods.  So we keep the inode locked for a while.
	 * We cheat by using the inode I/O lock for an extended period.
	 * We don't also need to take the incore lock when *examining*
	 * the inode's uid and gid because once the inode has been
	 * created this is the only code that modifies those fields.
	 * However, we must take the incore lock when *modifying* those
	 * fields, for the benefit of other code examining the fields
	 * without holding the inode I/O lock.
	 */
	IN_WRITE_LOCK(ip);
	if (uid == (uid_t) VNOVAL)
		uid = ip->i_uid;
	if (gid == (gid_t) VNOVAL)
		gid = ip->i_gid;

	/*
	 * If uid or gid is bigger than 2^16 - 1, return EINVAL,
	 * because i_uid and i_gid (the on-disk inode entries)
	 * are currently of type u_short, while uid and gid are
	 * of type u_int (uid_t and gid_t are of type u_int)..
	 */
	if ((uid & 0xffff0000) || (gid & 0xffff0000)) {
	    IN_WRITE_UNLOCK(ip);
	    return(EINVAL);
	}

	/*
	 * If we don't own the file, are trying to change the owner
	 * of the file, or are not a member of the target group,
	 * the caller must be superuser or the call fails.
	 */
	if (chown_restricted != -1) {
		/* As per POSIX */
		if ((cred->cr_uid != ip->i_uid || uid != ip->i_uid ||
		    !groupmember((gid_t)gid, cred)) &&
#if SEC_BASE
		/* XXX privileged returns the oppsite value that suser does */
		   (error = (privileged(SEC_CHOWN, EPERM) ? 0 : 1))) {
#else
		   (error = suser(cred, &u.u_acflag))) {
#endif 
			IN_WRITE_UNLOCK(ip);
			return (error);
		}
	} else {
		/* As per SVID-3 */
		if ( ( cred->cr_uid != ip->i_uid ) &&
#if SEC_BASE
		/* XXX privileged returns the oppsite value that suser does */
			(error = privileged(SEC_CHOWN, EPERM) ? 0 : 1)) {
#else
			(error = suser(cred, &u.u_acflag))) {
#endif
				IN_WRITE_UNLOCK(ip);
				return (error);
		}	
	}

#if	SEC_ARCH
	if(security_is_on && (error = iaccess(ip, SP_SETATTRACC, cred))) {
		IN_WRITE_UNLOCK(ip);
		return error;
	}
#endif
	ouid = ip->i_uid;
	ogid = ip->i_gid;
#if	QUOTA
	if ((error = getinoquota(ip)) ||
	    (error = quota_chown(ip, uid, gid, 0, cred))) {
		IN_WRITE_UNLOCK(ip);
		return (error);
	}
#endif
	IN_LOCK(ip);
	ip->i_uid = uid;
	ip->i_gid = gid;
	ip->i_flag |= ICHG;

#if SEC_BASE
	if (security_is_on) {
	ip->i_mode &= ~ISUID;
	ip->i_mode &= ~ISGID;
#if SEC_ARCH
	/* XXX inode locking !!! */
	/*
	 * If the user chowns the security policy daemon, and
	 * the daemon must fault a page from its binary, deadlock
	 * results on the daemon's inode's I/O lock.
	 *
	 * If the policy daemon indicates that the inode receives
	 * a new uid or gid, we blindly force the quota system to
	 * give the file's blocks to the new uid/gid.  Doing otherwise
	 * allows the quota system to reject our change, which means
	 * informing the policy daemon, which may then mean calling
	 * the quota system again.
	 */
	dac.uid = ip->i_uid;
	dac.gid = ip->i_gid;
	dac.mode = ip->i_mode & 0777;
	IN_UNLOCK(ip);
	ret = SP_CHANGE_OBJECT(ip->i_tag, &dac, SEC_NEW_UID|SEC_NEW_GID);
	IN_LOCK(ip);
	if (ret) {
#if	QUOTA
		if (ret & (SEC_NEW_UID|SEC_NEW_GID)) {
			IN_UNLOCK(ip);
			if (quota_chown(ip, dac.uid, dac.gid, FORCE, cred))
				panic("chown1:  security/quota botch");
			IN_LOCK(ip);
		}
#endif
		if (ret & SEC_NEW_UID)
			ip->i_uid = dac.uid;
		if (ret & SEC_NEW_GID)
			ip->i_gid = dac.gid;
		if (ret & SEC_NEW_MODE)
			ip->i_mode = (ip->i_mode & ~0777) | (dac.mode & 0777);
	}
#endif /* SEC_ARCH */
	} else
#endif /* SEC_BASE */
	if (cred->cr_uid != 0) {
		ip->i_mode &= ~ISUID;
		ip->i_mode &= ~ISGID;
	}

	IN_UNLOCK(ip);
	IN_WRITE_UNLOCK(ip);
	return (0);
}

/*
 * Vnode op for reading.
 */
/* ARGSUSED */
ufs_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);
	register struct fs *fs;
	struct buf *bp;
	daddr_t lbn, bn, rablock;
	int size, error = 0;
	int n, on, type;
	off_t isize;
	long diff;	/* needs to be signed off_t */

	if (uio->uio_rw != UIO_READ)
		panic("ufs_read mode");
	if (uio->uio_resid == 0)
		return (0);
	/* offset is an off_t; can't be negative (it's unsigned) */
	IN_READ_LOCK(ip);
	IN_LOCK(ip);
	type = ip->i_mode & IFMT;
	if (type != IFDIR && type != IFREG && type != IFLNK)
		panic("ufs_read type");
	ip->i_flag |= IACC;
	fs = ip->i_fs;
	isize = ip->i_size;
	IN_UNLOCK(ip);
	if (type == IFREG) return ufs_rwip(vp, uio, ioflag, cred);
	else do {
		lbn = lblkno(fs, uio->uio_offset);
		on = blkoff(fs, uio->uio_offset);
		n = MIN((unsigned)(fs->fs_bsize - on), uio->uio_resid);
		diff = isize - uio->uio_offset;
		if (diff <= 0) {
			IN_READ_UNLOCK(ip);
			return (0);
		}
		if (diff < n)
			n = diff;
		size = blksize(fs, ip, lbn);
		rablock = lbn + 1;
		VN_LOCK(vp);
 		if (vp->v_lastr + 1 == lbn &&
 		    lblktosize(fs, rablock) < isize) {
			VN_UNLOCK(vp);
 			error = breada(ITOV(ip), lbn, size, rablock,
 				blksize(fs, ip, rablock), NOCRED, &bp);
		} else {
			VN_UNLOCK(vp);
			error = bread(ITOV(ip), lbn, size, NOCRED, &bp);
		}
		LASSERT(BUF_LOCK_HOLDER(bp));
		ASSERT(bp->b_resid >= 0);
		VN_LOCK(vp);
		vp->v_lastr = lbn;
		VN_UNLOCK(vp);
		n = MIN(n, size - bp->b_resid);
		if (error) {
			brelse(bp);
			IN_READ_UNLOCK(ip);
			return (error);
		}
		error = uiomove(bp->b_un.b_addr + on, (int)n, uio);
		if (n + on == fs->fs_bsize || uio->uio_offset == isize)
			bp->b_flags |= B_AGE;
		brelse(bp);
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
	IN_READ_UNLOCK(ip);
	return (error);
}

/*
 * Vnode op for writing.
 */
ufs_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);
	register struct fs *fs;
	struct buf *bp;
	daddr_t lbn, bn;
	off_t osize, isize;
	int i, n, on, flags;
	unsigned efbig = 0;
#if	MACH
	int size, resid, error = 0;
#else
	int count, size, resid, error = 0;
#endif
	enum vtype	type;

	if (uio->uio_rw != UIO_WRITE)
		panic("ufs_write mode");
	if (uio->uio_resid == 0)
		return (0);
	/* offset is an off_t; can't be negative (it's unsigned) */
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	IN_WRITE_LOCK(ip);
	BM(IN_LOCK(ip));
	osize = isize = ip->i_size;
	BM(IN_UNLOCK(ip));
	switch (type) {
	case VREG:
		if (ioflag & IO_APPEND)
			uio->uio_offset = isize;
		/* fall through */
	case VLNK:
		break;

	case VDIR:
		if ((ioflag & IO_SYNC) == 0)
			panic("ufs_write nonsync dir write");
		break;

	default:
		panic("ufs_write type");
	}
	if (type == VREG) {
		register unsigned long file_limit;

		file_limit =  u.u_rlimit[RLIMIT_FSIZE].rlim_cur;
		if (uio->uio_offset >= file_limit) {
			IN_WRITE_UNLOCK(ip);
			unix_master();
			psignal(u.u_procp, SIGXFSZ);
			unix_release();
			return (EFBIG);
		}
		efbig = uio->uio_offset + uio->uio_resid;
		if (efbig > file_limit) {
			efbig -= file_limit;
			uio->uio_resid -= efbig;
		}
		else
			efbig = 0;
		error =  ufs_rwip(vp,  uio, ioflag, cred);
		goto done;
	}
	resid = uio->uio_resid;
	osize = isize;
	fs = ip->i_fs;
	flags = 0;
	if (ioflag & IO_SYNC)
		flags = B_SYNC;
	do {
		lbn = lblkno(fs, uio->uio_offset);
		on = blkoff(fs, uio->uio_offset);
		n = MIN((unsigned)(fs->fs_bsize - on), uio->uio_resid);
		if (n < fs->fs_bsize)
			flags |= B_CLRBUF;
		else
			flags &= ~B_CLRBUF;
		if (error = balloc(ip, lbn, (int)(on + n), &bp, flags))
			break;
		LASSERT(BUF_LOCK_HOLDER(bp));
		bn = bp->b_blkno;
		if (uio->uio_offset + n > isize) {
			IN_LOCK(ip);
			ip->i_size = uio->uio_offset + n;
			IN_UNLOCK(ip);
		}
		size = blksize(fs, ip, lbn);
		n = MIN(n, size - bp->b_resid);
		error = uiomove(bp->b_un.b_addr + on, n, uio);
		if (error) {
			bp->b_flags |= B_INVAL;
			brelse(bp);
			break;
		}	
		if (ioflag & IO_SYNC) {
			error = bwrite(bp);
			if (error)
				break;
		} else if (n + on == fs->fs_bsize) {
			bp->b_flags |= B_AGE;
			bawrite(bp);
		} else
			bdwrite(bp, bp->b_vp);
		IN_LOCK(ip);
		ip->i_flag |= IUPD|ICHG;
		IN_UNLOCK(ip);
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
	IN_WRITE_UNLOCK(ip);
done:
	IN_LOCK(ip);
#if SEC_PRIV
	bzero(ip->i_gpriv, sizeof ip->i_gpriv);
	bzero(ip->i_ppriv, sizeof ip->i_ppriv);
	ip->i_flag |= ICHGMETA;
#endif
	if (cred->cr_uid != 0) {
		u_short mask = ISUID;
		/*
		 * Don't clear enforcement mode lock bits, 
		 * indicated by setgid bit, but no group execute.
		 */
		if (!(ip->i_mode & ISGID) || (ip->i_mode & S_IXGRP))
			if (sys_v_mode == 0) {
				mask |= ISGID;
			}
		if (ip->i_mode & mask) {
			ip->i_mode &= ~mask;
			ip->i_flag |= ICHGMETA;
		}
	}
	IN_UNLOCK(ip);
	if (efbig > 0)
		uio->uio_resid += efbig;
	if (error && (ioflag & IO_UNIT)) {
		(void) itrunc(ip, osize, ioflag & IO_SYNC);
		uio->uio_offset -= resid - uio->uio_resid;
		uio->uio_resid = resid;
	}
	if (!error) {
		if ((ioflag & IO_SYNC) && !(ioflag & IO_DATAONLY)) {
			/*
			 * Regular files that do not increase in size
			 * and have not changed the on-disk inode do not
			 * need to be synchronously written. This is
			 * equivelent to update_flag in ULTRIX. Any
			 * on-disk inode changes will set the ICHGMETA flag.
			 * Note:
			 * 	Since ICHGMETA is set during block
			 *	[re]allocations, meta data changes done on
			 *	behalf of other processes will cause us to
			 *	synchronously write out the inode.
			 *
			 * Comment from ULTRIX:
			 *
			 * update_flag is a strange beast. If called to do a
			 * synchronous write (ioflag == IO_SYNC), then we must
			 * flush the gnode to disk after doing the write
			 * when any on-disk structures have changed. For
			 * performance reasons, we want to avoid this
			 * expensive operation if they haven't  changed. Since
			 * ufs_bmap() can change these  structures, we must be
			 * able to tell when it did. This is done by sending
			 * ufs_bmap() the address of update_flag as the sync
			 * flag when we are doing a synchronous write, and 0
			 * otherwise. ufs_bmap() will set update_flag to 1
			 * when we must flush the gnode, and will leave it
			 * unchanged otherwise. We set update_flag ourselves
			 * whenever the file is extended, since the new size
			 * must be put out to disk.
			 */
			if ((ip->i_mode & IFMT) == IFREG &&
			    (ip->i_flag & ICHGMETA) == 0 &&
			    ip->i_size <= osize)
				error = iupdat(ip, &time, &time, 0);
			else
				error = iupdat(ip, &time, &time, 1);
		}
	}
	return (error);
}

/* ARGSUSED */
ufs_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{

	return (ENOTTY);
}

/*
 * Mmap a file
 *
 */

ufs_mmap(register struct vnode *vp, 
	vm_offset_t offset,
	vm_map_t map,
	vm_offset_t *addrp,
	vm_size_t len,
	vm_prot_t prot,
	vm_prot_t maxprot,
	int flags,
	struct ucred *cred)
{
	struct vp_mmap_args args;
	register struct vp_mmap_args *ap = &args;
	extern kern_return_t u_vp_create();

	ap->a_offset = offset;
	ap->a_vaddr = addrp;
	ap->a_size = len;
	ap->a_prot = prot,
	ap->a_maxprot = maxprot;
	ap->a_flags = flags;
	return u_vp_create(map, vp->v_object, (vm_offset_t) ap);
}

ufs_fsync_int(vp, fflags, start, length, cred, waitfor)
	struct vnode *vp;
	int fflags;
	vm_offset_t start;
	vm_size_t length;
	struct ucred *cred;
	int waitfor;
{
	struct inode *ip = VTOI(vp);
	int error = 0;

	if ((fflags&(FWRITE_DATA | FWRITE_METADATA)) ==
	    (FWRITE_DATA | FWRITE_METADATA))
		fflags &= ~(FWRITE_DATA | FWRITE_METADATA);
	               /* cant have both */

	if (fflags&FWRITE) {
		BM(IN_LOCK(ip));
		ip->i_flag |= ICHG;
		BM(IN_UNLOCK(ip));
	}

	if (vp->v_type == VREG) {
		if (!(fflags & FWRITE_METADATA)) {
			/*
			 * Write out UBC cached file data. Take the inode
			 * read lock to guard against fsync/trucate races.
			 */
			IN_READ_LOCK(ip);
			error = ufs_flushdata(ip, start, length,
					      (waitfor == MNT_WAIT ? 1 : 0),
					      0);
			IN_READ_UNLOCK(ip);
		} 
		if (!(fflags & FWRITE_DATA)) {
			/*
			 * Write out metadata cache
			 *
			 * Note: vflushbuf() will block for all currently
			 * outstanding I/O only when called with B_SYNC.
			 */
			vflushbuf(vp, waitfor == MNT_WAIT ? B_SYNC : 0);

			/*
			 * If called with FWRITE_METADATA, only commit
			 * to disk if in-core inode contains changed
			 * metadata.
			 */
			if (fflags & FWRITE_METADATA &&
			    (ip->i_flag & ICHGMETA) == 0)
				waitfor = 0;
			return (iupdat(ip, &time, &time, waitfor == MNT_WAIT));
		}
		return (error);
	}

	vflushbuf(vp, waitfor == MNT_WAIT ? B_SYNC : 0);
	return (iupdat(ip, &time, &time, waitfor == MNT_WAIT));

}

/*
 * Synch an open file.
 */
/* ARGSUSED */
ufs_fsync(vp, fflags, cred, waitfor)
	struct vnode *vp;
	int fflags;
	struct ucred *cred;
	int waitfor;
{
	struct inode *ip = VTOI(vp);
	int error;
	vm_offset_t start;
	vm_size_t length;

	/*
	 * Write remaining dirty pages
	 */
	ubc_count_dirty(vp, &start, &length);
	error = ufs_fsync_int(vp, fflags, start, length, cred, waitfor);
	return (error);
}

/*
 * Synch a range of an open file.
 */
/* ARGSUSED */
ufs_syncdata(vp, fflags, start, length, cred)
	struct vnode *vp;
	int fflags;
	vm_offset_t start;
	vm_size_t length;
	struct ucred *cred;
{
	return(ufs_fsync_int(vp, FWRITE|FWRITE_DATA, start, length, cred,
			     MNT_WAIT));
}

/*
 * Seek on a file
 *
 * Negative offsets are invalid.
 */
/* ARGSUSED */
ufs_seek(vp, oldoff, newoff, cred)
	struct vnode *vp;
	off_t oldoff, newoff;
	struct ucred *cred;
{
	if ((long) newoff < 0)
		return(EINVAL);
	else
		return(0);
}

/*
 * ufs remove
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
ufs_remove(ndp)
	struct nameidata *ndp;
{
	register struct inode *ip, *dp;
	register struct vnode *vp;
	int error;

	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	ASSERT(vp->v_type != VDIR);
	BM(VN_UNLOCK(vp));
	ip = VTOI(ndp->ni_vp);
	dp = VTOI(ndp->ni_dvp);
	if ((ip->i_mode&IFMT) == IFDIR)
		return(EISDIR);
	/*
	 * We must ensure that the vnode describing this file is
	 * inaccessible after we remove the file from the directory
	 * so we purge its name cache entry.  If the link count on
	 * the inode drops to 0, we must also prevent NFS clients
	 * from using almost stale file handles for this inode so
	 * we increment the generation number.  The inode will be
	 * inacessible from both UFS and NFS file systems when the
	 * link count is 0.
	 */
	IN_WRITE_LOCK(dp);
	cache_purge(ITOV(ip));
	error = dirremove(ndp);
	if (!error) {
		IN_LOCK(ip);
		if (--ip->i_nlink == 0) {
			ip->i_gen = get_nextgen();
			ip->i_flag |= ICHG;
			IN_UNLOCK(ip);
		} else {
			ip->i_flag |= ICHG;
			IN_UNLOCK(ip);
		}
	}
	cache_purge(ITOV(ip));		/* Just in case... - XXX */
	IN_WRITE_UNLOCK(dp);
	iput(ip);
	iput(dp);
	return (error);
}

/*
 * link vnode call
 */
ufs_link(vp, ndp)
	register struct vnode *vp;
	register struct nameidata *ndp;
{
	register struct inode *ip = VTOI(vp);
	register struct inode *dp = VTOI(ndp->ni_dvp);
	int error;
	int decr_link = 0;

#if SEC_ARCH
	/* XXX inode locking !!! */
	if (security_is_on) {
	if (SP_ACCESS(ip->i_tag, VTOI(ndp->ni_dvp)->i_tag, SP_LINKACC, NULL)) {
		error = EACCES;
		goto out;
	}
	}
#endif
	/* make sure it's not a DIR */
	if (vp->v_type == VDIR) {
		error = EPERM;
		goto out;
	}

	IN_LOCK(ip);
	if ((ushort_t) ip->i_nlink >= LINK_MAX) {
		IN_UNLOCK(ip);
		error = EMLINK;
		goto out;
	}
	ip->i_nlink++;
	ip->i_flag |= ICHG;
	IN_UNLOCK(ip);
	decr_link = 1;
	error = iupdat(ip, &time, &time, 1);
	if (!error) {
		IN_WRITE_LOCK(dp);
		error = direnter(ip, ndp);
		IN_WRITE_UNLOCK(dp);
	}
out:
	iput(dp);
	if (error) {
		IN_LOCK(ip);
		if (decr_link)
			ip->i_nlink--;
		if (ip->i_nlink == 0)
			ip->i_gen = get_nextgen();
		ip->i_flag |= ICHG;
		IN_UNLOCK(ip);
	}
	return (error);
}

/*
 * Rename system call.
 * 	rename("foo", "bar");
 * is essentially
 *	unlink("bar");
 *	link("foo", "bar");
 *	unlink("foo");
 * but ``atomically''.  Can't do full commit without saving state in the
 * inode on disk which isn't feasible at this time.  Best we can do is
 * always guarantee the target exists.
 *
 * Basic algorithm is:
 *
 * 1) Bump link count on source while we're linking it to the
 *    target.  This also ensure the inode won't be deleted out
 *    from underneath us while we work (it may be truncated by
 *    a concurrent `trunc' or `open' for creation).
 * 2) Link source to destination.  If destination already exists,
 *    delete it first.
 * 3) Unlink source reference to inode if still around. If a
 *    directory was moved and the parent of the destination
 *    is different from the source, patch the ".." entry in the
 *    directory.
 */
ufs_rename(fndp, tndp)
	register struct nameidata *fndp, *tndp;
{
	register struct inode *ip, *xp, *dp;
	struct dirtemplate dirbuf;
	int doingdirectory = 0, oldparent = 0, newparent = 0;
	int stripslash = 0;
	int error = 0;
	int dummy;
#if defined(DCEDFS) && DCEDFS
	int flookup = fndp->ni_nameiop & SPECLOOKUP;
	int tlookup = tndp->ni_nameiop & SPECLOOKUP;
#else
        int flookup = 0;
        int tlookup = 0;
#endif /* DCEDFS */

	dp = VTOI(fndp->ni_dvp);
	ip = VTOI(fndp->ni_vp);
#if SEC_ARCH
	/*
	 * Check link access between existing file and target directory.
	 * If target file already exists, check process's delete access.
	 */
	if (security_is_on) {
	if (SP_ACCESS(ip->i_tag, VTOI(tndp->ni_dvp)->i_tag, SP_LINKACC, NULL)) {
		error = EACCES;
	} else if (tndp->ni_vp)
		error = iaccess(VTOI(tndp->ni_vp), SP_DELETEACC, tndp->ni_cred);
	if (error)
		goto abort;
	}
#endif /* SEC_ARCH */
	IN_WRITE_LOCK(dp);
	IN_LOCK(ip);
	if (ip->i_flag & IRENAME) {
		error = EINVAL;
		goto abort2;
	}
	ip->i_flag |= IRENAME;
	if ((ip->i_mode&IFMT) == IFDIR) {
		register struct dirent *d = &fndp->ni_dent;

		/*
		 * Avoid ".", "..", and aliases of "." for obvious reasons.
		 */
		if ((d->d_namlen == 1 && d->d_name[0] == '.') || dp == ip ||
#if SEC_MAC
		    /* Prevent renaming of mld subdirectories */
		    (ip->i_type_flags & SEC_I_MLDCHILD) ||
#endif
		    fndp->ni_isdotdot) {
			ip->i_flag &= ~IRENAME;
			error = EINVAL;
			goto abort2;
		}
		IN_UNLOCK(ip);
		oldparent = dp->i_number;
		doingdirectory++;
		stripslash = STRIPSLASH;
		cache_purge(ITOV(ip));
	} else
		IN_UNLOCK(ip);
	/*
	 * Check that the source has not been removed.  
	 */
	if (error = checkdir(fndp, DEL)) {
		IN_LOCK(ip);
		ip->i_flag &= ~IRENAME;
		goto abort2;
	}
	/*
	 * 1) Bump link count while we're moving stuff
	 *    around.  If we crash somewhere before
	 *    completing our work, the link count
	 *    may be wrong, but correctable.
	 */
	IN_LOCK(ip);
	ip->i_nlink++;
	ip->i_flag |= ICHG;
	IN_UNLOCK(ip);
	IN_WRITE_UNLOCK(dp);
	error = iupdat(ip, &time, &time, 1);
again:
	dp = VTOI(tndp->ni_dvp);
	xp = NULL;
	if (tndp->ni_vp)
		xp = VTOI(tndp->ni_vp);
	/*
	 * If ".." must be changed (ie the directory gets a new
	 * parent) then the source directory must not be in the
	 * directory heirarchy above the target, as this would
	 * orphan everything below the source directory. Also
	 * the user must have write permission in the source so
	 * as to be able to change "..". We must repeat the call
	 * to namei, as the parent directory is iput by the call
	 * to checkpath().
	 */
	if (oldparent != dp->i_number)
		newparent = dp->i_number;
	if (doingdirectory && newparent) {
		if (error = iaccess(ip, IWRITE, tndp->ni_cred))
			goto bad;
#if defined(DCEDFS) && DCEDFS
		tndp->ni_nameiop = RENAME | WANTPARENT | stripslash |
				   tlookup;
#else
		tndp->ni_nameiop = RENAME | WANTPARENT | stripslash;
#endif /* DCEDFS */
		do {
			dp = VTOI(tndp->ni_dvp);
			if (xp != NULL)
				iput(xp);
			if (error = checkpath(ip, dp, tndp->ni_cred))
				goto out;
			if (error = namei(tndp))
				goto out;
			xp = NULL;
			if (tndp->ni_vp)
				xp = VTOI(tndp->ni_vp);
		} while (dp != VTOI(tndp->ni_dvp));
	}
	IN_WRITE_LOCK(dp);
	if (xp == NULL) {
		/*
		 * If the target didn't exist, check that it hasn't
		 * been created.
		 */
		if (error = checkdir(tndp, ADD)) {
			if (error == EEXIST) {
				IN_WRITE_UNLOCK(dp);
				iput(dp);
				tndp->ni_nameiop = 
#if defined(DCEDFS) && DCEDFS
				    RENAME | WANTPARENT | stripslash |
				    tlookup;
#else
				    RENAME | WANTPARENT | stripslash;
#endif /* DCEDFS */
				if (error = namei(tndp))
					goto out;
				goto again;
			} else
				goto bad2;
		}
	} else {
		/*
		 * Check that the target hasn't been removed.
		 */
		 if (error = checkdir(tndp, DEL|RNM)) {
			if (error == EEXIST) {
				IN_WRITE_UNLOCK(dp);
#if	SEC_ARCH
				if (tndp->ni_vp)
					error = iaccess(VTOI(tndp->ni_vp),
							SP_SETATTRACC, cred));
				if (error) {
					/*
					 * The target vnode has changed, 
					 * release the new one
					 */
					vrele(tndp->ni_vp);
					xp = NULL;
					goto bad;
				}
#endif	/* SEC_ARCH */
				goto again;
			}
			if (error == ENOENT) {
				iput(xp);
				xp = NULL;
			} else
				goto bad2;
		}
	}
	if (xp && doingdirectory)
		IN_READ_LOCK(xp);
	/*
	 * 2) If target doesn't exist, link the target
	 *    to the source and unlink the source.
	 *    Otherwise, rewrite the target directory
	 *    entry to reference the source inode and
	 *    expunge the original entry's existence.
	 */
	if (xp == NULL) {
		if (dp->i_dev != ip->i_dev)
			panic("rename: EXDEV");
		/*
		 * Account for ".." in new directory.
		 * When source and destination have the same
		 * parent we don't fool with the link count.
		 */
		if (doingdirectory && newparent) {
			IN_LOCK(dp);
			if ((ushort_t) dp->i_nlink >= LINK_MAX) {
			        IN_UNLOCK(dp);
			        error = EMLINK;
			        goto bad2;
			}
			dp->i_nlink++;
			dp->i_flag |= ICHG;
			IN_UNLOCK(dp);
			error = iupdat(dp, &time, &time, 1);
		}
		if (error = direnter(ip, tndp)) {
			if (doingdirectory && newparent) {
				IN_LOCK(dp);
				dp->i_nlink--;
				dp->i_flag |= ICHG;
				IN_UNLOCK(dp);
			}
			goto bad2;
		}
	} else {
		struct vnode *vp;

		if (xp->i_dev != dp->i_dev || xp->i_dev != ip->i_dev)
			panic("rename: EXDEV");
		/*
		 * Short circuit rename(foo, foo).
		 */
		if (xp->i_number == ip->i_number)
			panic("rename: same file");
		/*
		 * If the parent directory is "sticky", then the user must
		 * own the parent directory, or the destination of the rename,
		 * otherwise the destination may not be changed (except by
		 * root). This implements append-only directories.
		 */
		BM(IN_LOCK(dp));
#if SEC_BASE
	     if (check_privileges)
		if ((dp->i_mode & ISVTX) && 
		    tndp->ni_cred->cr_uid != dp->i_uid) {
			BM(IN_UNLOCK(dp));
			BM(IN_LOCK(xp));
			if (xp->i_uid != tndp->ni_cred->cr_uid) {
				BM(IN_UNLOCK(xp));
				if (!privileged(SEC_OWNER, EPERM)) {
					error = EPERM;
					goto bad3;
				}
			} else
				BM(IN_UNLOCK(xp));
		} else
			BM(IN_UNLOCK(dp));
	     else
#endif
		if ((dp->i_mode & ISVTX) && tndp->ni_cred->cr_uid != 0 &&
		    tndp->ni_cred->cr_uid != dp->i_uid) {
			BM(IN_UNLOCK(dp));
			BM(IN_LOCK(xp));
			if (xp->i_uid != tndp->ni_cred->cr_uid) {
				BM(IN_UNLOCK(xp));
				error = EPERM;
				goto bad3;
			} else
				BM(IN_UNLOCK(xp));
		} else
			BM(IN_UNLOCK(dp));
		/*
		 * Target must be empty if a directory
		 * and have no links to it.
		 * Also, insure source and target are
		 * compatible (both directories, or both
		 * not directories).
		 */
		if ((xp->i_mode&IFMT) == IFDIR) {
			if (xp->i_nlink != 2 ||
			    !dirempty(xp, dp->i_number, tndp->ni_cred)) {
				error = EEXIST;
				goto bad3;
			} 
			if (!doingdirectory) {
				error = ENOTDIR;
				goto bad3;
			}
			cache_purge(ITOV(dp));
			cache_purge(ITOV(xp));
		} else if (doingdirectory) {
			error = EISDIR;
			goto bad3;
		}
		/*
                 * If we are renaming directories and we will be
                 * rewriting the source starting directory, fail
                 * the operation because we will need the source
                 * starting directory in tact to unlink the source.
                 */
                if (doingdirectory && xp == VTOI(fndp->ni_cdir)) {
                        error = EINVAL;
                        goto bad3;
                }
		if (error = dirrewrite(dp, ip, tndp))
			goto bad3;
		/*
		 * Adjust the link count of the target to
		 * reflect the dirrewrite above.  If this
		 * is a directory, it is empty, so we can
		 * squash the inode and any space associated
		 * with it. We disallowed renaming over top
		 * of a directory with links to it above, as
		 * the remaining link would point to a directory
		 * without "." or ".." entries.
		 */
		IN_LOCK(xp);
		xp->i_nlink--;
		if (doingdirectory) {
			if (--xp->i_nlink != 0)
				panic("rename: linked directory");
			xp->i_gen = get_nextgen();
			IN_UNLOCK(xp);
			cache_purge(ITOV(xp));
			/*
			* If the target directory is in the same
			* directory as the source directory,
			* decrement the link count on the parent
			* of the target directory.
			*/
			if (!newparent) {
				IN_LOCK(dp);
				dp->i_nlink--;
				dp->i_flag |= ICHG;
				IN_UNLOCK(dp);
			}
			IN_READ_UNLOCK(xp);
			error = itrunc(xp, 0, IO_SYNC);
			IN_LOCK(xp);
		} else if (xp->i_nlink == 0) {
			xp->i_gen = get_nextgen();
			IN_UNLOCK(xp);
                        cache_purge(ITOV(xp));
			IN_LOCK(xp);
		}
		xp->i_flag |= ICHG;
		IN_UNLOCK(xp);
		iput(xp);
		xp = NULL;
	}
	IN_WRITE_UNLOCK(VTOI(tndp->ni_dvp));

	/*
	 * 3) Unlink the source.
	 */
	iput(VTOI(fndp->ni_dvp));
#if defined(DCEDFS) && DCEDFS
	fndp->ni_nameiop = DELETE | WANTPARENT | stripslash | flookup;
#else
	fndp->ni_nameiop = DELETE | WANTPARENT | stripslash;
#endif /* DCEDFS */
	(void)namei(fndp);
	if (fndp->ni_vp != NULL) {
		xp = VTOI(fndp->ni_vp);
		dp = VTOI(fndp->ni_dvp);
	} else {
		/*
		 * If ni_vp is NULL, then source is already gone, and
		 * ni_dvp is undefined, and not referenced.
		 * Must fail check below (xp != ip) to clean up properly.
		 */
		ASSERT(ip != (struct inode *)NULL);
		ASSERT(error == 0);
		xp = NULL;
		dp = NULL;
	}
	/*
	 * Ensure that the directory entry still exists and has not
	 * changed while the new name has been entered. If the source is
	 * a file then the entry may have been unlinked or renamed. In
	 * either case there is no further work to be done. If the source
	 * is a directory then it cannot have been rmdir'ed; its link
	 * count of three would cause a rmdir to fail with EEXIST.
	 * The IRENAME flag ensures that it cannot be moved by another
	 * rename.
	 */
	if (xp != ip) {
		if (doingdirectory)
			panic("rename: lost dir entry");
	} else {
		/*
		 * If the source is a directory with a
		 * new parent, the link count of the old
		 * parent directory must be decremented
		 * and ".." set to point to the new parent.
		 */
		if (doingdirectory && newparent) {
			IN_LOCK(dp);
			dp->i_nlink--;
			dp->i_flag |= ICHG;
			IN_UNLOCK(dp);
#if defined(DCEDFS) && DCEDFS
                        error = vn_rdwr(VNUIO_READNESTED, ITOV(xp),
			       (caddr_t)&dirbuf,
#else
			error = vn_rdwr(UIO_READ, ITOV(xp), (caddr_t)&dirbuf,
#endif /* DCEDFS */
				sizeof (struct dirtemplate), (off_t)0,
				UIO_SYSSPACE, 0, tndp->ni_cred, (int *)0);
			if (error == 0) {
				if (dirbuf.dotdot_namlen != 2 ||
				    dirbuf.dotdot_name[0] != '.' ||
				    dirbuf.dotdot_name[1] != '.') {
					dirbad(xp, 12, "rename: mangled dir");
				} else {
					dirbuf.dotdot_ino = newparent;
#if defined(DCEDFS) && DCEDFS
					(void) vn_rdwr(VNUIO_WRITENESTED, 
					    ITOV(xp),
#else
					(void) vn_rdwr(UIO_WRITE, ITOV(xp),
#endif /* DCEDFS */
					    (caddr_t)&dirbuf,
					    sizeof (struct dirtemplate),
					    (off_t)0, UIO_SYSSPACE,
					    IO_SYNC, tndp->ni_cred, (int *)0);
					cache_purge(ITOV(dp));
				}
			}
		}
		IN_WRITE_LOCK(dp);
		error = dirremove(fndp);
		IN_WRITE_UNLOCK(dp);
		IN_LOCK(xp);
		if (!error) {
			if (--xp->i_nlink == 0)
				xp->i_gen = get_nextgen();
			xp->i_flag = (xp->i_flag | ICHG) & ~IRENAME;
		} else
			xp->i_flag &= ~IRENAME;
		IN_UNLOCK(xp);
	}
	iput(VTOI(tndp->ni_dvp)); 	/* parent of target */
	if (dp)
		iput(dp);	/* parent of source */
	if (xp)
		iput(xp);	/* source */
	iput(ip);		/* source */
	return (error);

bad3:
	if (xp && doingdirectory)	/* target */
		IN_READ_UNLOCK(xp);
bad2:
	IN_WRITE_UNLOCK(VTOI(tndp->ni_dvp));
bad:
	if (xp)				/* target */
		iput(xp);
	iput(dp);			/* parent of target */
out:
	iput(VTOI(fndp->ni_dvp));	/* parent of source */
	IN_LOCK(ip);
	if (--ip->i_nlink == 0)
		ip->i_gen = get_nextgen();
	ip->i_flag = (ip->i_flag | ICHG) & ~IRENAME;
	IN_UNLOCK(ip);
	iput(ip);			/* source */
	return (error);
abort2:
	IN_UNLOCK(ip);
	IN_WRITE_UNLOCK(dp);
abort:
	VOP_ABORTOP(tndp, dummy);
	vrele(tndp->ni_dvp);
	if (tndp->ni_vp)
		vrele(tndp->ni_vp);
	VOP_ABORTOP(fndp, dummy);
	vrele(fndp->ni_dvp);
	/* could have been modified by checkdir() */
	if (fndp->ni_vp)
		vrele(fndp->ni_vp);
	return (error);
}

/*
 * A virgin directory (no blushing please).
 */
struct dirtemplate mastertemplate = {
	0, 12, 1, ".",
	0, DIRBLKSIZ - 12, 2, ".."
};

#define DIRTEMPLATESIZ (sizeof(struct dirtemplate))

/*
 * Mkdir system call
 */
ufs_mkdir(ndp, vap)
	struct nameidata *ndp;
	struct vattr *vap;
{
	register struct inode *ip, *dp;
	struct inode *tip;
	struct vnode *dvp;
	struct dirtemplate *dirtemplate;
	int error;
	int dmode;
	struct vnode *vp;
	struct inode *tdp;
	uid_t uid;
	gid_t gid;
	caddr_t dirblksiz_buf;

	dvp = ndp->ni_dvp;
	dp = VTOI(dvp);
#if SEC_MAC
	/*
	 * When making a mld subdirectory we are called with NULL vap.
	 */
	if (vap == NULL) {
		/*
		 * Ensure that the process dominates the mld.
		 */
		if (!mld_dominate(dp->i_tag)) {
			iput(dp);
			return EACCES;
		}
		BM(IN_LOCK(dp));
    		dmode = dp->i_mode & 07777;
		BM(IN_UNLOCK(dp));
	} else
#endif /* SEC_MAC */
	dmode = vap->va_mode&0777;
	dmode |= IFDIR;
	/*
	 * Must simulate part of maknode here
	 * in order to acquire the inode, but
	 * not have it entered in the parent
	 * directory.  The entry is made later
	 * after writing "." and ".." entries out.
	 */
	error = ialloc(dp, dirpref(dp->i_fs), dmode, &tip);
	if (error) {
		iput(dp);
		return (error);
	}
	ip = tip;
	IN_LOCK(ip);
	uid = ndp->ni_cred->cr_uid;
	if (sys_v_mode == 0) {
		gid = dp->i_gid;
	} else {
		if (dp->i_mode & ISGID) {
			gid = dp->i_gid;
			dmode |= ISGID;
		} else {
			gid = ndp->ni_cred->cr_gid; /* egid of process */
			dmode &= ~ISGID;
		}
	}
	/*
	 * If uid or gid is bigger than 2^16 - 1, return EINVAL,
	 * because i_uid and i_gid (the on-disk inode entries)
	 * are currently of type u_short, while uid and gid are
	 * of type u_int (uid_t and gid_t are of type u_int)..
	 */
	if ((uid & 0xffff0000) || (gid & 0xffff0000)) {
		IN_UNLOCK(ip);
		ifree(ip, ip->i_number, dmode);
		iput(ip);
		iput(dp);
	    	return(EINVAL);
	}
	ip->i_uid = uid;
	ip->i_gid = gid;
#if SEC_MAC
	/*
	 * If making a mld subdirectory, inherit owner and group
	 * from the mld.
	 */
	if (vap == NULL) {
		ip->i_uid = dp->i_uid;
		ip->i_gid = dp->i_gid;
		ip->i_parent = dp->i_number;
		ip->i_type_flags = SEC_I_MLDCHILD;
	}
#endif /* SEC_MAC */
	IN_UNLOCK(ip);
#if	QUOTA
	/*
	 * No need to hold inode I/O write lock across
	 * this chkiq call because no one else knows
	 * about this inode.
	 */
	if ((error = getinoquota(ip)) ||
	    (error = chkiq(ip, 1, ndp->ni_cred, 0))) {
		ifree(ip, ip->i_number, dmode);
		iput(ip);
		iput(dp);
		return (error);
	}
#endif
	IN_LOCK(ip);
	ip->i_nlink = 2;
	ip->i_flag |= IACC|IUPD|ICHG;
	ip->i_mode = dmode;
	IN_UNLOCK(ip);
	vp = ITOV(ip);
	BM(VN_LOCK(vp));	/* Vnode should be unknown now anyway */
	vp->v_type = VDIR;	/* Rest init'd in iget() */
	BM(VN_UNLOCK(vp));

	error = iupdat(ip, &time, &time, 1);
	/*
	 * Bump link count in parent directory
	 * to reflect work done below.  Should
	 * be done before reference is created
	 * so reparation is possible if we crash.
	 */
	IN_LOCK(dp);
	if ((ushort_t) dp->i_nlink >= LINK_MAX) {
		IN_UNLOCK(dp);
		error = EMLINK;
		goto out;
	}
	dp->i_nlink++;
	dp->i_flag |= ICHG;
	IN_UNLOCK(dp);
	error = iupdat(dp, &time, &time, 1);

	/*
	 * Allocate space to contain a directory memory block,
	 * zero currently unused portion, point to proto area. 
	 */
	if ((dirblksiz_buf =  kalloc(DIRBLKSIZ)) == (caddr_t) 0)
	{
		error = ENOMEM;
		goto out;
	}
	bzero(dirblksiz_buf+DIRTEMPLATESIZ, DIRBLKSIZ-DIRTEMPLATESIZ);
	dirtemplate = (struct dirtemplate *) dirblksiz_buf;

	/*
	 * Initialize directory with "."
	 * and ".." from static template.
	 */
	*dirtemplate = mastertemplate;
	dirtemplate->dot_ino = ip->i_number;
	dirtemplate->dotdot_ino = dp->i_number;

#if defined(DCEDFS) && DCEDFS
	error = vn_rdwr(VNUIO_WRITENESTED, ITOV(ip), dirblksiz_buf,
#else
	error = vn_rdwr(UIO_WRITE, ITOV(ip), dirblksiz_buf,
#endif /* DCEDFS */
		DIRBLKSIZ, (off_t)0, UIO_SYSSPACE,
		IO_SYNC, ndp->ni_cred, (int *)0);
	kfree(dirblksiz_buf, DIRBLKSIZ);
	if (error)
		goto out;

	if (DIRBLKSIZ > dp->i_fs->fs_fsize)
		panic("mkdir: blksize");     /* XXX - should grow w/balloc() */

	/*
	 * Directory all set up, now
	 * install the entry for it in
	 * the parent directory.
	 */
	IN_WRITE_LOCK(dp);
	error = direnter(ip, ndp);
	IN_WRITE_UNLOCK(dp);
out:
	if (error) {
		if (error != EMLINK) {
			IN_LOCK(dp);
			dp->i_nlink--;
			dp->i_flag |= ICHG;
			IN_UNLOCK(dp);
		}
		IN_LOCK(ip);
		ip->i_nlink = 0;
		ip->i_gen = get_nextgen();
		ip->i_flag |= ICHG;
		IN_UNLOCK(ip);
		/*
		 * No need to do an explicit itrunc here,
		 * iput will do this for us because we set
		 * the link count to 0.
		 */
		iput(ip);
	} else
		ndp->ni_vp = ITOV(ip);
	iput(dp);
	return(error);
}

/*
 * Rmdir system call.
 */
ufs_rmdir(ndp)
	register struct nameidata *ndp;
{
	register struct inode *ip, *dp;
	int error = 0;

	ip = VTOI(ndp->ni_vp);
	dp = VTOI(ndp->ni_dvp);
	/*
	 * No rmdir "." please.
	 */
	if (dp == ip) {
		iput(dp);
		iput(ip);
		return (EINVAL);
	}
	/*
	 * We hold the directory read locked until we remove it
	 * in dirremove.  This prevents any files or directories
	 * from being created in the directory throughout this
	 * process.
	 * 
	 * We hold the parent directory write locked until after
	 * we remove the directory from it.  We need it to remove
	 * the directory from the parent.  We must take the write
	 * lock on the parent directory before we can take the read
	 * lock on the child directory.  This forces us to hold the
	 * write lock until after dirremove has done its work.
	 */
	IN_WRITE_LOCK(dp);
	IN_READ_LOCK(ip);
	/*
	 * Verify the directory is empty (and valid).
	 * (Rmdir ".." won't be valid since
	 *  ".." will contain a reference to
	 *  the current directory and thus be
	 *  non-empty.)
	 */
	if (ip->i_nlink != 2 || !dirempty(ip, dp->i_number, ndp->ni_cred)) {
		error = EEXIST;
		goto bad;
	}
	/*
	 * We remove the directory from the cache here because we
	 * don't want anyone to find it in the cache.  If dirremove
	 * fails, which it rarely does, the entry may have to be
	 * entered into the cache again later. 
	 */
	cache_purge(ITOV(ip));
	if (error = dirremove(ndp))
		goto bad;
	cache_purge(ITOV(dp));
	IN_LOCK(dp);
	dp->i_nlink--;
	dp->i_flag |= ICHG;
	IN_UNLOCK(dp);
	IN_WRITE_UNLOCK(dp);
	iput(dp);
	ndp->ni_dvp = NULL;
	/*
	 * Truncate inode.  The only stuff left
	 * in the directory is "." and "..".  The
	 * "." reference is inconsequential since
	 * we're squashing it.  The ".." reference
	 * has already been adjusted above.
	 * If we have a hard link to this directory,
	 * we only want to decrement by one.
	 */
	IN_LOCK(ip);
	ip->i_nlink -= 2;
	ASSERT(ip->i_nlink == 0);
	ip->i_gen = get_nextgen();
	IN_UNLOCK(ip);
	IN_READ_UNLOCK(ip);
	error = itrunc(ip, 0, IO_SYNC);
	cache_purge(ITOV(ip));		/* Just in case... - XXX */
	iput(ip);
	return (error);
bad:
	IN_READ_UNLOCK(ip);
	IN_WRITE_UNLOCK(dp);
	iput(dp);
	iput(ip);
	return (error);
}

/*
 * symlink -- make a symbolic link
 */
ufs_symlink(ndp, vap, target)
	struct nameidata *ndp;
	struct vattr *vap;
	char *target;
{
	struct inode *ip;
	int error;

	register int	len;

	extern create_fastlinks;

	/*
	 * Since the new inode must be consistent when made visible,
	 * maknode does the work to write the symlink to disk.
	 */
	vap->va_symlink = target;
	if (error = maknode(vap, ndp, &ip))
		return (error);
	iput(ip);
	return (error);
}

/*
 * Vnode op for read and write
 */
ufs_readdir(vp, uio, cred, eofflagp)
	struct vnode *vp;
	register struct uio *uio;
	struct ucred *cred;
	int *eofflagp;
{
	int count, lost, error;
	register struct inode *ip = VTOI(vp);

	count = uio->uio_resid;
	count &= ~(DIRBLKSIZ - 1);
	lost = uio->uio_resid - count;
	if (count < DIRBLKSIZ || (uio->uio_offset & (DIRBLKSIZ -1)))
		return (EINVAL);
	uio->uio_resid = count;
	uio->uio_iov->iov_len = count;
	error = ufs_read(vp, uio, 0, cred);
	uio->uio_resid += lost;
	BM(IN_LOCK(ip));
	if ((ip->i_size - uio->uio_offset) <= 0)
		*eofflagp = 1;
	else
		*eofflagp = 0;
	BM(IN_UNLOCK(ip));
	return (error);
}

/*
 * Return target name of a symbolic link
 */
ufs_readlink(vp, uiop, cred)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{

	register struct inode *ip = VTOI(vp);
	int error;
	u_long isize;

	/*
	 *	Encore fast symbolic link support
	 */
	BM(IN_LOCK(ip));
	if (ip->i_size > uiop->uio_resid)
	        return(ERANGE);
	if (ip->i_flags & IC_FASTLINK) {
		isize = ip->i_size;
		BM(IN_UNLOCK(ip));
		error = uiomove(ip->i_symlink, isize, uiop);
		IN_LOCK(ip);
		ip->i_flag |= IACC;
		IN_UNLOCK(ip);
		return(error);
	}
	BM(IN_UNLOCK(ip));
	return (ufs_read(vp, uiop, 0, cred));
}

/*
 * Ufs abort op, called after namei() when a CREATE/DELETE isn't actually
 * done. Iff ni_vp/ni_dvp not null and locked, unlock.
 */
ufs_abortop(ndp)
	register struct nameidata *ndp;
{
	return 0;
}

/*
 * Get access to bmap
 */
ufs_bmap(vp, bn, vpp, bnp)
	struct vnode *vp;
	daddr_t bn;
	struct vnode **vpp;
	daddr_t *bnp;
{
	struct inode *ip = VTOI(vp);

	if (vpp != NULL)
		*vpp = ip->i_devvp;
	if (bnp == NULL)
		return (0);
	return (bmap(ip, bn, bnp, 0));
}

/*
 * Just call the device strategy routine
 */
ufs_strategy(bp)
	register struct buf *bp;
{
	register struct inode *ip = VTOI(bp->b_vp);
	register struct buf *ep;
	struct vnode *vp;
	struct buf *ebp;
	daddr_t start, last;
	int error;

#if MACH_LDEBUG
	if (bp->b_flags & B_ASYNC) {
		LASSERT(BUF_IS_LOCKED(bp));
	} else {
		LASSERT(BUF_LOCK_HOLDER(bp));
	}
#endif
#ifdef	KTRACE
	kern_trace(1001,bp->b_dev, bp->b_blkno, bp->b_vp->v_op->vn_strategy);
#endif  /* KTRACE */
	if (bp->b_vp->v_type == VBLK || bp->b_vp->v_type == VCHR)
		panic("ufs_strategy: spec");
	if (bp->b_blkno == bp->b_lblkno) {
		if (error = bmap(ip, bp->b_lblkno, &bp->b_blkno, 0))
			return (error);
 		if (bp->b_blkno == -1)
  			clrbuf(bp);
  	}
 	if (bp->b_blkno == -1) {
 		biodone(bp);
  		return (0);
 	}
	vp = ip->i_devvp;
	bp->b_dev = vp->v_rdev;
	(*(vp->v_op->vn_strategy))(bp);
	return (0);
}

/*
 * Print out the contents of an inode.
 */
ufs_print(vp)
	struct vnode *vp;
{
	register struct inode *ip = VTOI(vp);

	printf("tag VT_UFS, ino %d, on dev %d, %d\n", ip->i_number,
		major(ip->i_dev), minor(ip->i_dev));
}

/*
 * Read wrapper for special devices.
 */
ufsspec_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	register struct inode *ip = VTOI(vp);;
	/*
	 * Set access flag.
	 */
	IN_LOCK(ip);
	ip->i_flag |= IACC;
	IN_UNLOCK(ip);
	return (spec_read(vp, uio, ioflag, cred));
}

/*
 * Write wrapper for special devices.
 */
ufsspec_write(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);

	/*
	 * Set update and change flags.
	 */
	IN_LOCK(ip);
	ip->i_flag |= IUPD|ICHG;
	IN_UNLOCK(ip);
	return (spec_write(vp, uio, ioflag, cred));
}

/*
 * Close wrapper for special devices.
 *
 * Update the times on the inode then do device close.
 */
ufsspec_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);

	BM(VN_LOCK(vp));
	if (vp->v_usecount > 1) {
		u_int	flag;

		BM(VN_UNLOCK(vp));
		IN_LOCK(ip);
		flag = ip->i_flag & IREADERROR;
		IN_UNLOCK(ip);
		/*
		 * If there was an error initializing the inode, we don't
		 * want to do anything.  We will follow this path
		 * because vclean will bump the reference count.
		 */
		if (flag)
			return(0);
		ITIMES(ip, &time, &time);
	} else
		BM(VN_UNLOCK(vp));
	return (spec_close(vp, fflag, cred));
}

/*
 * Read wrapper for fifos.
 */
ufsfifo_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);
	/*
	 * Set access flag.
	 */
	IN_LOCK(ip);
	ip->i_flag |= IACC;
	IN_UNLOCK(ip);
	return (fifo_read(vp, uio, ioflag, cred));
}

/*
 * Write wrapper for fifos.
 */
ufsfifo_write(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);
	/*
	 * Set update and change flags.
	 */
	IN_LOCK(ip);
	ip->i_flag |= IUPD|ICHG;
	IN_UNLOCK(ip);
	return (fifo_write(vp, uio, ioflag, cred));
}

/*
 * Close wrapper for fifos.
 *
 * Update the times on the inode then do device close.
 */
ufsfifo_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);

	VN_LOCK(vp);
	if (vp->v_usecount > 1) {
		VN_UNLOCK(vp);
		ITIMES(ip, &time, &time);
	} else
		VN_UNLOCK(vp);
	return (fifo_close(vp, fflag, cred));
}

/*
 * getattr wrapper for fifos.
 */
ufsfifo_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int error;

	/*
	 * Get most attributes from the inode, rest
	 * from the fifo.
	 */
	if (error = ufs_getattr(vp, vap, cred))
		return (error);
	return (fifo_getattr(vp, vap, cred));
}

/*
 * Make a new file.
 */
maknode(vap, ndp, ipp)
	register struct vattr *vap;
	register struct nameidata *ndp;
	struct inode **ipp;
{
	register struct inode *ip;
	struct inode *tip;
	register struct inode *pdir = VTOI(ndp->ni_dvp);
	ino_t ipref;
	register struct vnode *vp;
	enum vtype type;
	int mode;
	int error, updated = 0;
	uid_t uid;
	gid_t gid;

	*ipp = 0;
	mode = MAKEIMODE(vap->va_type, vap->va_mode);
	if ((mode & IFMT) == IFDIR)
		ipref = dirpref(pdir->i_fs);
	else
		ipref = pdir->i_number;
	error = ialloc(pdir, ipref, mode, &tip);
	if (error) {
		iput(pdir);
		return (error);
	}
	ip = tip;
	IN_LOCK(ip);
	uid = ndp->ni_cred->cr_uid;
	if (sys_v_mode == 0) {
		gid = pdir->i_gid;
#if	SEC_BASE
	/* XXX inode locked !!! */
	if ((ip->i_mode & ISGID) && !groupmember(gid, ndp->ni_cred) &&
	    !privileged(SEC_SETPROCIDENT, 0))
#else
	if ((ip->i_mode & ISGID) && !groupmember(gid, ndp->ni_cred) &&
	    suser(ndp->ni_cred, NULL))
#endif
		ip->i_mode &= ~ISGID;
	} else {
                /*
                 * Do it the way SVID III wants it
                 */
                if (pdir->i_mode & ISGID) {
                        /*
                         * Parent directory has the ISGID bit set.  Therefore
                         * new nodes' group id becomes the group id of the
                         * parent directory.
                         */
                        gid = pdir->i_gid;
                        if ((mode & IFMT) == IFDIR) {
                                /*
                                 * If some yo-yo tries to make a directory
                                 * using open we have to let him do it
                                 * therfore new directory gets the ISGID bit
                                 * set.
                                 */
                                mode |= ISGID;
                        }
                } else {
                        /*
                         * Parent directory does NOT have the ISGID bit set.
                         * Therefore new nodes' group id becomes the group id
                         * of the calling process.
                         */
                        gid = ndp->ni_cred->cr_gid; /* egid of process */
                        if ( !(vap->va_mode & ISGID)) {
                                /*
                                 * If we call open & forcibly set the ISGID bit
                                 * in the mode field then we WILL NOT fall
                                 * here.  If we do fall here, it is because
                                 * someone did a 'normal' open call, so we
                                 * must turn off the ISGID bit.
                                 * - It is not normal to set ISGID with open.
                                 */
                                mode &= ~ISGID;
                        }
                }
                if ( !groupmember(gid, ndp->ni_cred) ) {
                        mode &= ~ISGID;
                }
	}
	/*
	 * If uid or gid is bigger than 2^16 - 1, return EINVAL,
	 * because i_uid and i_gid (the on-disk inode entries)
	 * are currently of type u_short, while uid and gid are
	 * of type u_int (uid_t and gid_t are of type u_int)..
	 */
	if ((uid & 0xffff0000) || (gid & 0xffff0000)) {
		IN_UNLOCK(ip);
		ifree(ip, ip->i_number, mode);
		iput(ip);
		iput(pdir);
	    	return(EINVAL);
	}
	ip->i_uid = uid;
	ip->i_gid = gid;
	IN_UNLOCK(ip);
#if	QUOTA
	/*
	 * No need to hold inode I/O write lock across
	 * this chkiq call because no one else knows
	 * about this inode.
	 */
	if ((error = getinoquota(ip)) ||
	    (error = chkiq(ip, 1, ndp->ni_cred, 0))) {
		ifree(ip, ip->i_number, mode);
		iput(ip);
		iput(pdir);
		return (error);
	}
#endif
	IN_LOCK(ip);
	ip->i_flag |= IACC|IUPD|ICHG;
	if ((mode & IFMT) == 0)
		mode |= IFREG;
	ip->i_mode = mode;
	ip->i_nlink = 1;
	IN_UNLOCK(ip);
	vp = ITOV(ip);
	VN_LOCK(vp);
	vp->v_type = IFTOVT(mode);	/* Rest init'd in iget() */
	type = vp->v_type;
	/*
	 * If the inode was recycled from another type, we need to
	 * reset its v_ops.  Start out assuming regular file; it will
	 * be changed as required below.
	 */
	VN_UNLOCK(vp);

	ubc_object_allocate(vp);

	/*
	 * Handle atomic special cases:
	 *	-- special files
	 *	-- symlinks
	 *	-- sockets
	 * Some of this could possibly be avoided by vgone'ing the
	 * newly created vnodes, so they get set up properly in iget,
	 * but there are some races involved.
	 */
	if (vap->va_rdev != (dev_t)VNOVAL) {
		ASSERT(type == VBLK || type == VCHR);
		if (error = specalloc(vp, vap->va_rdev)) {
			VN_LOCK(vp);
			vp->v_type = VNON;
			VN_UNLOCK(vp);
			goto bad;
		}
		IN_LOCK(ip);
		ip->i_rdev = vap->va_rdev;
		IN_UNLOCK(ip);
		VN_LOCK(vp);
		vp->v_op = &spec_inodeops;
		VN_UNLOCK(vp);
	} else if (type == VLNK) {
		/*
	 	 * Handle symlinks, making a fast link if name fits.
	 	 */
		register int len = strlen(vap->va_symlink);
		if (len < MAX_FASTLINK_SIZE && create_fastlinks) {
			bcopy(vap->va_symlink, ip->i_symlink, len);
			IN_WRITE_LOCK(ip);
			IN_LOCK(ip);
			ip->i_size = len;
			/*
			 * Strictly speaking, these three statements
			 * need not be placed under inode I/O lock
			 * but doing so saves releasing, reacquiring
			 * and re-releasing the incore lock... on a
			 * file that won't have any other users yet!
			 */
			ip->i_symlink[len] = '\0';
			ip->i_flags |= IC_FASTLINK;
			ip->i_flag |= IACC|IUPD|ICHG;
			IN_UNLOCK(ip);
			IN_WRITE_UNLOCK(ip);
		} else {
			error = iupdat(ip, &time, &time, 1);
			updated = 1;
			if (error)
				goto bad;
#if defined(DCEDFS) && DCEDFS
			error = vn_rdwr(VNUIO_WRITENESTED, ITOV(ip), 
					vap->va_symlink, 
#else
			error = vn_rdwr(UIO_WRITE, ITOV(ip), vap->va_symlink, 
#endif /* DCEDFS */
					len, (off_t)0, UIO_SYSSPACE,
					0, ndp->ni_cred, (int *)0);
			if (error)
				goto bad;
		}
	} else if (type == VSOCK)
		vp->v_socket = (struct socket *) vap->va_socket;
	else if (type == VFIFO)
		vp->v_op = &fifo_inodeops;
	/*
	 * Make sure inode goes to disk before directory entry.
	 */
	if (!updated)
		error = iupdat(ip, &time, &time, 1);
	if (!error) {
		IN_WRITE_LOCK(pdir);
		error = direnter(ip, ndp);
		IN_WRITE_UNLOCK(pdir);
	}
	if (!error) {
		iput(pdir);
		*ipp = ip;
		return (0);
	}
bad:
	iput(pdir);
	/*
	 * Write error occurred trying to update the inode
	 * or the directory so must deallocate the inode.
	 */
	IN_LOCK(ip);
	ip->i_nlink = 0;
	ip->i_gen = get_nextgen();
	ip->i_flag |= ICHG;
	IN_UNLOCK(ip);
	iput(ip);
	return (error);
}

int	inode_read_aheads = 0;
int	inode_read_individuals = 0;

ufs_page_read(vp, uio, cred)
	struct vnode *vp;
	struct uio *uio;
	struct ucred *cred;
{
	register struct inode *ip = VTOI(vp);
	struct fs *fs;		/* filesystem */
	int bsize;		/* size of logical blocks */
	int amount;		/* bytes left to read */
	int sofar;		/* bytes read so far */
	int error;		/* our error return */
	int size;
	int phys;
	long limit;		/* can't read more bytes than this */
	off_t isize;

	error = EINVAL;		/* initially, no data read */

	IN_LOCK(ip);
	ip->i_flag |= IACC;
	IN_UNLOCK(ip);

	/*
	 *	Check that we are trying to read data that
	 *	lies within the file.
	 */

	fs = ip->i_fs;
	bsize = fs->fs_bsize;
	sofar = 0;
	size = uio->uio_resid;
	phys = 0;
	if (uio->uio_segflg == UIO_PHYSSPACE)
		phys++;

	IN_READ_LOCK(ip);
	BM(IN_LOCK(ip));
	isize = ip->i_size;
	limit = isize - uio->uio_offset;
	BM(IN_UNLOCK(ip));
	if ((limit <= 0) || (size <= 0)) {
		IN_READ_UNLOCK(ip);
		return(error);
	}
	amount = (size <= limit) ? size : limit;
	if (phys && limit < size)
		pmap_zero_page(uio->uio_iov->iov_base);

	do {
		daddr_t lbn;	/* logical block where our data lies */
		int on;		/* byte offset within that logical block */
		daddr_t bn;	/* disk block where our data lies */
		int n;		/* the number of bytes read this time */
		daddr_t rablock;
		int rasize;
		int fsize;	/* size of fragment/block */
		struct buf *bp;	/* our buffer with the data */

		/*
		 *	Find block and offset within it for our data.
		 */

		lbn = lblkno(fs, uio->uio_offset);
		on  = blkoff(fs, uio->uio_offset);

		/*
		 *	Don't read beyond the end of a logical block.
		 *	We handle logical blocks one at a time.
		 */

		n = MIN(bsize - on, amount);
		rablock = lbn + 1;

		/*
		 *	We might be reading from a fragment
		 *	instead of a full block.
		 */

		fsize = blksize(fs, ip, lbn);
		n = MIN(n, fsize);

		/*
		 *	If we're doing sequential IO, try read-ahead.
		 */

		VN_LOCK(vp);
		if (vp->v_lastr + 1 == lbn &&
		    lblktosize(fs, rablock) < isize) {
			VN_UNLOCK(vp);
			inode_read_aheads++;
				error = breada(vp, lbn, fsize, rablock,
					blksize(fs, ip, rablock), NOCRED, &bp);
		} else {
			VN_UNLOCK(vp);
			inode_read_individuals++;
			error = bread(vp, lbn, fsize, NOCRED, &bp);
		}
		VN_LOCK(vp);
		vp->v_lastr = lbn;
		VN_UNLOCK(vp);

		LASSERT(BUF_LOCK_HOLDER(bp));
		if (error) {
			VN_LOCK(vp);
			VN_UNLOCK(vp);
			brelse(bp);
#ifdef DEBUG
			printf("error %d on pagein (bread)\n", error);
#endif
			error = EIO;
			IN_READ_UNLOCK(ip);
			return(error);
		}
		ASSERT(bp->b_resid == 0);
		ASSERT(bp->b_bcount == fsize);
		if (phys)
			copy_to_phys(bp->b_un.b_addr+on, uio->uio_iov->iov_base + sofar, n);
		else
			bcopy(bp->b_un.b_addr+on, uio->uio_iov->iov_base + sofar, n);
		bp->b_flags |= B_USELESS;
		brelse(bp);

		/*
		 *	If we finally put good data
		 *	into the buffer, we have to zero
		 *	the initial portion that we skipped.
		 */
		if (error != 0) {
/* NEED TO ZERO BEGINNING OF PAGE IF phys */
/* THIS CODE IS NEVER REACHED!! */
			if (!phys && sofar > 0)
				bzero(uio->uio_iov->iov_base, sofar);
			error = 0;
		}

		sofar += n;
		uio->uio_offset += n;
		uio->uio_resid -= n;
		amount -= n;
	} while (amount > 0);
	IN_READ_UNLOCK(ip);
	/*
	 *	If we are returning real data in the buffer,
	 *	and we ran up against the file size,
	 *	we must zero the remainder of the buffer.
	 */
	if (!phys && (error == 0) && (limit < size))
		bzero(uio->uio_iov->iov_base + limit, size - limit);

	return(error);
}

ufs_page_write(vp, uio, cred, pager, offset)
	struct vnode	*vp;
	struct uio 	*uio;
	struct ucred	*cred;
	memory_object_t pager;
	vm_offset_t	offset;
{
	register struct inode *ip = VTOI(vp);
	int		error;

	error = ufs_write(vp, uio, 0, cred);
#ifdef DEBUG
	if (error)
		printf("error %d on pageout (ufs_rdwr)\n", error);
#endif
	return error;
}

/*
 * UFS Cluster I/O declarations
 */

/* Not being used for alpha - phh */
#define CLUSTER_DEBUG

#ifdef CLUSTER_DEBUG
/*
 * General cluster debug flag
 */
int cluster_debug = 0;
int cluster_write_debug = 0;
int cluster_read_debug = 0;
dev_t cluster_write_debug_dev = 0;

#endif
/*
 * Initial number of MAXBSIZE buffers per cluster buffer
 */
#define CLUSTER_MAXCONTIG 8

long cluster_maxcontig = CLUSTER_MAXCONTIG;
/*
 * Cluster tuning parameters
 */
int cluster_write_one = 1;		/*
					 * Only schedule one asynchronous
					 * write in ufs_delay_cluster().
					 */
int cluster_read_all = 1;		/*
					 * Cluster read all requested pages
					 * in ufs_getapage().
					 */
int cluster_max_read_ahead = 8;		/*
					 * Max number of clusters to stay ahead
					 * of sequential reader.
					 */
int cluster_consec_init = 2;		/*
					 * Initial value of i_consecreads.
					 */
int cluster_consec_incr = 1;		/*
					 * Add this amount to i_consecreads.
					 */
int cluster_lastr_init = -1;		/*
					 * Initial value of v_lastr.
					 */
struct ufs_clstats {
	long max_blocks_ahead;
	long max_clusters_ahead;
	long max_clusterlbn;
	long max_clustersz;
	long kalloc_failed;
};

struct ufs_clstats ufs_clstats = { 0L, 1L, 0L, 0L, 0L };

/*
 * Maximum number of blocks read-ahead policy preceeded applications.
 */
#define cluster_max_ahead ufs_clstats.max_blocks_ahead

/*
 * Maximum number of clusters read-ahead policy preceeded applications.
 */
#define cluster_max_clusters ufs_clstats.max_clusters_ahead

/*
 * Number of times kalloc for > CLUSTER_MAXCONTIG read-ahead failed
 */
#define kalloc_failed ufs_clstats.kalloc_failed

/*
 * Cluster I/O accounting
 */
struct ufs_clusterstats {
	long full_cluster_transfers;
	long part_cluster_transfers;
	long non_cluster_transfers;
	long sum_cluster_transfers[CLUSTER_MAXCONTIG + 2];
} ufs_clusterstats;

struct ufs_clusterstats ufs_clusterstats_read;
struct ufs_clusterstats ufs_clusterstats_write;

#define CLUSTER_READ_STATS(num_blks) { \
   if ((num_blks) <= 1) { \
	   ufs_clusterstats_read.non_cluster_transfers++; \
	   ufs_clusterstats.non_cluster_transfers++; \
   } else if ((num_blks) == fs->fs_maxcontig) { \
	   ufs_clusterstats_read.full_cluster_transfers++; \
	   ufs_clusterstats.full_cluster_transfers++; \
   } else { \
	   ufs_clusterstats_read.part_cluster_transfers++; \
	   ufs_clusterstats.part_cluster_transfers++; \
   } \
   if (num_blks <= CLUSTER_MAXCONTIG) { \
	   ufs_clusterstats_read.sum_cluster_transfers[(num_blks)] += 1; \
           ufs_clusterstats.sum_cluster_transfers[(num_blks)] += 1; \
   } else { \
	   ufs_clusterstats_read.sum_cluster_transfers[CLUSTER_MAXCONTIG+1] += 1; \
           ufs_clusterstats.sum_cluster_transfers[CLUSTER_MAXCONTIG+1] += 1; \
   } \
}

#define CLUSTER_WRITE_STATS(num_blks) { \
   if ((num_blks) <= 1) { \
	   ufs_clusterstats_write.non_cluster_transfers++; \
	   ufs_clusterstats.non_cluster_transfers++; \
   } else if ((num_blks) == cluster_maxcontig) { \
	   ufs_clusterstats_write.full_cluster_transfers++; \
	   ufs_clusterstats.full_cluster_transfers++; \
   } else { \
	   ufs_clusterstats_write.part_cluster_transfers++; \
	   ufs_clusterstats.part_cluster_transfers++; \
   } \
   if (num_blks <= CLUSTER_MAXCONTIG) { \
	   ufs_clusterstats_write.sum_cluster_transfers[(num_blks)] += 1; \
           ufs_clusterstats.sum_cluster_transfers[(num_blks)] += 1; \
   } else { \
	   ufs_clusterstats_write.sum_cluster_transfers[CLUSTER_MAXCONTIG+1] += 1; \
           ufs_clusterstats.sum_cluster_transfers[CLUSTER_MAXCONTIG+1] += 1; \
   } \
}

struct ufs_writes {
	dev_t dev;
	u_long	sync_notblockalligned;
	u_long	async_notblockalligned;
	u_long	sync_page_boundary;
	u_long	async_page_boundary;
	u_long	page_append_tries;
	u_long	page_append_success;
	u_long	sync_blocks;
	u_long	async_blocks;
	u_long	sync_multi_pages;
	u_long	async_multi_pages;
	u_long	sync_single_pages;
	u_long	async_single_pages;
	u_long	sync_frags;
	u_long	async_frags;
	u_long	sync_others;
	u_long	async_others;
	u_long	dir_iupdats;
	u_long	file_iupdats;
	u_long	sync_dirs;
	u_long	async_dirs;
	u_long 	sync_meta_blocks;
	u_long 	async_meta_blocks;
	u_long	sync_meta_cgs;
	u_long	async_meta_cgs;
	u_long	sync_meta_others;
	u_long	async_meta_others;
} ufs_writes;

ufs_rwblk(register struct vnode *vp, 
	register vm_page_t pp, 
	int pcnt,
	int flags,
	daddr_t rablkno)
{
	register struct inode *ip;
	register struct fs *fs;
	struct buf *rbp;
	register struct buf *bp;
	register vm_page_t lp;
	register u_int size;
	vm_page_t pn;
	int blocks;

	ip = VTOI(vp);
	fs = ip->i_fs;
	if (pcnt > 1) lp = pp->pg_pprev;
	else lp = pp;
	size = blksize(fs, ip, lblkno(fs, lp->pg_offset));
	if (size != fs->fs_bsize) {
		size = fragroundup(fs, ip->i_size);
		size = MIN(PAGE_SIZE, (size - lp->pg_offset));
		if (flags & B_READ && size < PAGE_SIZE)
			ubc_page_zero(lp, size, PAGE_SIZE - size);
		size += ptoa((pcnt - 1));
	}
	else size = ptoa(pcnt);

	if (ufs_writes.dev == ip->i_devvp->v_rdev && (flags & B_READ) == 0) {
		if (pp->pg_offset % 8192 && pp->pg_offset % 4096 == 0) {
			if (flags & B_ASYNC) {
				ufs_writes.async_page_boundary++;
				if (size > 4096)
					ufs_writes.async_notblockalligned++;
			} else {
				ufs_writes.sync_page_boundary++;
				if (size > 4096)
					ufs_writes.sync_notblockalligned;
			}
		}
		if (size % 8192 == 0) {
			if (flags & B_ASYNC)
				ufs_writes.async_blocks++;
			else
				ufs_writes.sync_blocks++;
		} else if (size > 8192 && size % 4096 == 0) {
			if (flags & B_ASYNC)
				ufs_writes.async_multi_pages++;
			else
				ufs_writes.sync_multi_pages++;
		} else if (size == 4096) {
			if (flags & B_ASYNC)
				ufs_writes.async_single_pages++;
			else
				ufs_writes.sync_single_pages++;
		} else if (size == 1024) {
			if (flags & B_ASYNC)
				ufs_writes.async_frags++;
			else
				ufs_writes.sync_frags++;
		} else {
			if (flags & B_ASYNC)
				ufs_writes.async_others++;
			else
				ufs_writes.sync_others++;
		}
	}

#if	UFS_BLK_CHECK
	{
		/*
		 * This code can fail while under a load.
		 * It is possible the UBC memory manager is pushing a
		 * page while a UNIX writer is extending the file.
		 * If the file system user sleeps before updating the
		 * inode state (allocating page which extend the file), 
		 * then ufs_ubcbmap will return a disk block
		 * which doesn't match what's in the physical page.  This
		 * isn't a bug but rather an inconsistency in the debug code.
		 * This could be corrected by recoding ufs_realloccg and
		 * not releasing pages until all pages have been updated.
		 */

		int alloc, error, cnt;
		daddr_t dblkno, bn;
		register struct inode *ip = VTOI(vp);
		register struct fs *fs = ip->i_fs;
		extern int waittime;

		for (pn = pp, cnt = 0; cnt < pcnt; pn = pn->pg_pnext, cnt++) {
			bn = lblkno(fs, pn->pg_offset);
			error = ufs_ubcbmap(ip, bn, fs->fs_bsize, 
				&dblkno, 1, &alloc);
			if (error || ((daddr_t) (pn->pg_pfs) != 
				(dblkno + 
				btodb(trunc_page(
				blkoff(fs, pn->pg_offset)))))) {
				waittime = 0;
				panic("ufs_rwblk: block invalid",pn);
			}
		}
	}
#endif	/* UFS_BLK_CHECK */

	ubc_bufalloc(pp, pcnt, &rbp, (vm_size_t)size, 1, flags);
	bp = rbp;
	bp->b_vp = vp;
	bp->b_dev = ip->i_devvp->v_rdev;
	bp->b_blkno = (daddr_t) (pp->pg_pfs);

#if	VM_IOCHECK
	vm_iocheck(bp, ip->i_fs->fs_bsize * 2);
#endif	/* VM_IOCHECK */

	(*(ip->i_devvp->v_op->vn_strategy))(bp);

	blocks = (pcnt * PAGE_SIZE) / fs->fs_bsize;
	if (flags & B_READ) {
		CLUSTER_READ_STATS(blocks);
	} else {
		CLUSTER_WRITE_STATS(blocks);
	}

	/*
	 * Schedule read-ahead
	 */
	if ((flags & B_READ) && rablkno)
		ufs_rablk(vp, ip, fs, rablkno);
	/*
	 * Wait for synchronous I/O
	 */
	if ((flags & B_ASYNC) == 0)
		return ubc_sync_iodone(bp);
	return 0;
}

struct cluster_read_aheads {
	daddr_t lbn;
	daddr_t	bn;
	u_int	sz;
	u_int	clsz;
};

ufs_rablk(register struct vnode *vp,
	register struct inode *ip,
	register struct fs *fs,
	register daddr_t lbn)
{
	register vm_page_t pp, lp;
	register vm_size_t size;
	int error, kpcnt;
	boolean_t alloc;
	daddr_t dblkno;
	daddr_t rablkno;
	int ra_clsz;
	int ra_sz;
	struct cluster_read_aheads cluster_ra[CLUSTER_MAXCONTIG];
	struct cluster_read_aheads *clra = cluster_ra;
	struct cluster_read_aheads *clra_start = cluster_ra;
	int ind, num_clusters, consecreads, maxcontig;
	daddr_t save_lbn;
	int save_sz, kasize, maxahead;
#ifdef CLUSTER_DEBUG
	int cldebug = 0;
#endif

	kasize = 0;
	maxcontig = MIN(fs->fs_maxcontig, cluster_maxcontig);
	num_clusters = 1;
	lbn--;
	BM(IN_LOCK(ip));
	consecreads = ip->i_consecreads;
	if (consecreads == cluster_consec_init)
		maxahead = 0;
	else if (consecreads > maxcontig)
		maxahead = maxcontig * (consecreads - maxcontig);
	else
		maxahead = consecreads;
	maxahead++;
	
	/*
	 * Calculate the read-ahead logical block number.
	 */
	if (lbn == ip->i_clusterlbn && ip->i_clustersz) {
		/*
		 * We are the start of the previously read
		 * cluster, start I/O on the next.
		 */
		rablkno = ip->i_clusterlbn + ip->i_clustersz;
		if (maxcontig > 1 && consecreads > maxcontig) {
			num_clusters = (consecreads - maxcontig) + 1;
			num_clusters = MIN(num_clusters,
					   cluster_max_read_ahead);
			if (num_clusters > CLUSTER_MAXCONTIG) {
				/*
				 * The stack variable `cluster_ra' has space for
				 * CLUSTER_MAXCONTIG read-ahead clusters. If
				 * we going to need more, allocate the entire
				 * amount from the kernel memory allocator.
				 */
				kasize = num_clusters *
					sizeof(struct cluster_read_aheads);
				clra_start =
					(struct cluster_read_aheads *)kalloc(kasize);
				if (clra_start == 0) {
					num_clusters = CLUSTER_MAXCONTIG;
					clra_start = cluster_ra;
					kasize = 0;
					kalloc_failed++;
				} else {
					clra = clra_start;
				}
			}
			if (num_clusters > cluster_max_clusters)
				cluster_max_clusters = num_clusters;
		}
	} else if (ip->i_clustersz &&
		   lbn + maxahead >= ip->i_clusterlbn &&
		   lbn < ip->i_clusterlbn) {
		/*
		 * We are still within the bounds of the
		 * last cluster we read.
		 */
		BM(IN_UNLOCK(ip));
		return (0);
	} else {
		/*
		 * Start a new cluster at the next block.
		 */
		rablkno = lbn + 1;
	}
	BM(IN_UNLOCK(ip));
	if (lblktosize(fs, rablkno) >= ip->i_size)
		return (0);

	save_lbn = rablkno;
	save_sz = 0;
	for (ind = 0; ind < num_clusters; ind++) {
		/*
		 * Call bmap() which returns the size of the
		 * cluster to read in the variable 'ra_clsz'.
		 * 'ra_clsz' will never exceed maxcontig.
		 */
		if (bmap(ip, rablkno, &dblkno, &ra_clsz))
			goto out;
		if (ra_clsz <= 0 || dblkno == (daddr_t)-1) {
			num_clusters = ind;
			break;
		}
		clra->lbn = rablkno;
		clra->bn = dblkno;
		clra->clsz = ra_clsz;
		clra->sz = (ra_clsz - 1) * fs->fs_bsize;
		clra->sz += blksize(fs, ip, rablkno + (ra_clsz - 1));

		save_sz += ra_clsz;
		rablkno += ra_clsz;
		clra++;
	}
	if (num_clusters == 0)
		goto out;

	if (num_clusters > 1) {
		BM(IN_LOCK(ip));
		ip->i_clusterlbn = save_lbn;
		ip->i_clustersz = save_sz;
		BM(IN_UNLOCK(ip));
	}

	if (maxcontig > 1 && ip->i_clusterlbn > ufs_clstats.max_clusterlbn)
		ufs_clstats.max_clusterlbn = ip->i_clusterlbn;
	if (maxcontig > 1 && ip->i_clustersz > ufs_clstats.max_clustersz)
		ufs_clstats.max_clustersz = ip->i_clustersz;
	if (maxcontig > 1 && lbn <= ip->i_clusterlbn &&
	    (ip->i_clusterlbn - lbn) + ip->i_clustersz > cluster_max_ahead) {
		cluster_max_ahead = (ip->i_clusterlbn - lbn) + ip->i_clustersz;
	}

	if (maxcontig > 1 && consecreads >= maxcontig &&
	    consecreads < maxcontig + cluster_max_read_ahead) {
		if (consecreads + cluster_consec_incr >
		    maxcontig + cluster_max_read_ahead)
			consecreads = maxcontig + cluster_max_read_ahead;
		else
			consecreads += cluster_consec_incr;
		BM(IN_LOCK(ip));
		ip->i_consecreads = consecreads;
		BM(IN_UNLOCK(ip));
	}

	clra = clra_start;
loop:
#ifdef CLUSTER_DEBUG
  if ((cluster_read_debug && maxcontig > 1) || cluster_debug) {
     cldebug = 1;
     printf("ufs_rablk: num_clusters %d lbn %d ", num_clusters, lbn);
     printf("rablkno %d dblkno %d ra_clsz %d ra_sz %d\n",
	    clra->lbn, clra->bn, clra->clsz, clra->sz);
  }
#endif
	/*
	 * Calling ubc_kluster() with B_CACHE will stop the
	 * search when a page is cached.
	 */
	pp = ubc_kluster(vp, (vm_page_t) 0, lblktosize(fs, clra->lbn), 
			 round_page(clra->sz), B_CACHE|B_WANTFREE|B_BUSY,
			 UBC_HNONE, &kpcnt);
	if (pp) {
		size = 0;
		dblkno = clra->bn + btodb(blkoff(fs, pp->pg_offset));
		lp = pp;
		do {
			size += PAGE_SIZE;
			(daddr_t) (lp->pg_pfs) = dblkno;
			dblkno += btodb(PAGE_SIZE);
		} while (lp = lp->pg_pnext);
#ifdef CLUSTER_DEBUG
  if (cldebug)
     printf("ufs_rablk: Calling ufs_rwblk rablkno %d count %d\n",
	    clra->lbn, size);
#endif
		ufs_rwblk(vp, pp, kpcnt, B_READ|B_ASYNC, 0);
	}
	if (--num_clusters > 0) {
		clra++;
		goto loop;
	}
out:
	if (clra_start != cluster_ra && kasize)
		kfree(clra_start, kasize);
	return 0;
}

struct ufs_getapage_stats {
	long read_looks;
	long read_hits;
	long read_miss;
} ufs_getapage_stats;

ufs_getapage(struct vnode *vp,
	register vm_offset_t offset,
	vm_size_t len,
	vm_prot_t *protp,
	vm_page_t *pl,
        vm_map_entry_t ep,
	int rwflg,
	int async,
	struct ucred *cred)
{
	register struct inode *ip;
	register struct fs *fs;
	register vm_page_t pp, kpp;
	int uflags, error;
	daddr_t dblkno, rablkno, tmp;
	unsigned long bn;
	register u_int iosize;
	vm_page_t app;
	register u_int newsize;
	off_t isize;
	boolean_t alloc;
	vm_page_t kpl;
	int khold, kplcnt, clsz, maxcontig;
	register vm_offset_t kstart;
	register vm_size_t ksize;
	register long pgoff;
	int max_blocks, save_consecreads;

	ip = VTOI(vp);
	fs = ip->i_fs;
	maxcontig = MIN(fs->fs_maxcontig, cluster_maxcontig);

	bn = (unsigned long)lblkno(fs, offset);
	/*
	 * daddr_t is a signed int for now...
	 */
	if (bn > 0x7ffffffe) {
		*pl = VM_PAGE_NULL;
		return (EFBIG);
	}

	BM(IN_LOCK(ip));
	isize = ip->i_size;
	BM(IN_UNLOCK(ip));

	if (!protp && (rwflg & B_READ) == 0 && ((offset + len) > isize))
		newsize = blkoff(fs, offset) + len;
	else
		newsize = 0;

	rablkno = 0;
	if (rwflg & B_READ) {
		++ufs_getapage_stats.read_looks;
		/*
		 * Schedule read-ahead before possibly blocking on
		 * a busy page.
		 */
		if (ubc_incore(vp, trunc_page(offset), PAGE_SIZE)) {
			VN_LOCK(vp);
			rablkno = bn + 1;
 			if (vp->v_lastr + 1 == bn &&
			    lblktosize(fs, rablkno) < isize) {
				VN_UNLOCK(vp);
				ufs_rablk(vp, ip, fs, rablkno);
				VN_LOCK(vp);
			} else if (vp->v_lastr != bn) {
				BM(IN_LOCK(ip));
				ip->i_consecreads = cluster_consec_init;
				BM(IN_UNLOCK(ip));
			}
			vp->v_lastr = bn;
			VN_UNLOCK(vp);
		}
		uflags = 0;
	} else {
		uflags = B_CACHE;
	}

	error = ubc_lookup(vp, trunc_page(offset), fs->fs_bsize, 
			   len + offset, &app, &uflags);

	/*
	 * If B_NOCACHE is NOT set, and a read or write:
	 *	Page was found and returned
	 *	Hold count incremented
	 *
	 * If B_NOCACHE is set, and a write:
	 *	No page is returned
	 *
	 * If B_NOCACHE is set and a read:
	 *	A page was allocated and returned
	 *	Page is marked as busy
	 *	Hold count incremented
	 */

	if (error) {
		*pl = VM_PAGE_NULL;
		return error;
	}
	pp = app;

	/*
	 * Found a page.
	 */

	if (!newsize && (uflags & B_NOCACHE) == 0) {
		if ((rwflg & B_READ) == 0) {
			if ((daddr_t) (pp->pg_pfs) == (daddr_t) -1) { 
				error = ufs_ubcbmap(ip, bn, fs->fs_bsize, 
						&dblkno, rwflg, &alloc);
				if (error)
					goto failed;
				if (protp)
					ubc_page_dirty(pp);
				goto holealloc;
			} else if ((uflags & B_DIRTY) == 0)
				ubc_page_dirty(pp);
			if (protp)
				*protp = 0;
		} else {
			++ufs_getapage_stats.read_hits;
			if(ep && current_thread())
				 current_thread()->thread_events.pageins++;
			if (protp) {
				if ((daddr_t) (pp->pg_pfs) == (daddr_t) -1 || 
				    (uflags & B_DIRTY) == 0)
					*protp = VM_PROT_WRITE;
				else
					*protp = 0;
			}
		}
	}

	/*
	 * Page not found
	 */

	else {
		error = ufs_ubcbmap(ip, bn, newsize, &dblkno, rwflg, &alloc);

		if (error) {
			if ((uflags & B_NOCACHE) == 0) { 
				ubc_page_release(pp, 0);
				pp = VM_PAGE_NULL;
			} else if (rwflg & B_READ) {
				ubc_page_release(pp, B_DONE|B_BUSY|B_FREE);
				pp = VM_PAGE_NULL;
			}
			goto failed;
		}

		/*
		 * Not a hole and not a newly allocated disk block
		 */

		if (dblkno != (daddr_t) -1 && !alloc) {
			if ((rwflg & B_READ) == 0) {
				if (newsize) {
					IN_LOCK(ip);
					ip->i_size = offset + len;
					ip->i_flag |= IUPD|ICHG|ICHGMETA;
					IN_UNLOCK(ip);
				}
				if (uflags & B_NOCACHE) {
					uflags = 0;
					error = ubc_lookup(vp, 
						trunc_page(offset), 
						fs->fs_bsize, len + offset, 
						&app, &uflags);
					pp = app;	
					if (error)
						goto failed;
					if ((uflags & B_NOCACHE) == 0)
						goto done;
					/*
					 * If were rewriting the entire page,
					 * we are done.
					 */
					if ((offset & page_mask) == 0 &&
					    len >= PAGE_SIZE) {
						ubc_page_zero(pp, 0, PAGE_SIZE);
						/*
						 * If this page is not block
						 * alligned, increment the
						 * logical disk block number
						 * accordingly.
						 */
						dblkno += btodb(blkoff(fs,
							        pp->pg_offset));
						(daddr_t)(pp->pg_pfs) = dblkno;
						ubc_page_release(pp, B_DONE);
						goto done;
					}
				} else
					goto done;
			}
			max_blocks = 0;

			if (rwflg & B_READ) {
				++ufs_getapage_stats.read_miss;
				if (cluster_read_all) {
					pgoff = offset & page_mask;
					max_blocks = howmany(round_page(len+pgoff),
							     fs->fs_bsize);
					if (max_blocks == 1)
						max_blocks = 0;
					else
						max_blocks = MIN(max_blocks,
								 maxcontig);
				}
				VN_LOCK(vp);
 				if (vp->v_lastr + 1 == bn &&
				    lblktosize(fs, (bn + 1)) < isize)
					rablkno = bn + 1;
				else {
					rablkno = (daddr_t) 0;
					if (vp->v_lastr != bn) {
						BM(IN_LOCK(ip));
						ip->i_consecreads =
							cluster_consec_init;
						BM(IN_UNLOCK(ip));
					}
				}
				vp->v_lastr = bn;
				VN_UNLOCK(vp);
			} else
				rablkno = 0;

			kstart = pp->pg_offset & fs->fs_bmask;
			if (max_blocks > 1) {
				BM(IN_LOCK(ip));
				save_consecreads = ip->i_consecreads;
				ip->i_consecreads = max_blocks;
				BM(IN_UNLOCK(ip));
				bmap(ip, bn, &tmp, &clsz);
				BM(IN_LOCK(ip));
				ip->i_consecreads = save_consecreads;
				BM(IN_UNLOCK(ip));
				ksize = (clsz - 1) * fs->fs_bsize;
				ksize += blksize(fs, ip, bn + (clsz - 1));
			} else {
				ksize = blksize(fs, ip, bn);
			}
			if (ksize % fs->fs_bsize) {
				ksize -= (ksize % fs->fs_bsize);
				ksize += round_page(fragroundup(fs, blkoff(fs,ip->i_size)));
			}
			if ((kstart + ksize) <= round_page(offset+len))
				khold = UBC_HACP;
			else
				khold = UBC_HNONE;
				
			kpl = ubc_kluster(vp, pp, kstart, ksize,
					  B_CACHE|B_WANTFREE|B_BUSY,
					  khold, &kplcnt);
			kpp = kpl;
			dblkno += btodb(blkoff(fs, kpp->pg_offset));
					
			do {
				(daddr_t) (kpp->pg_pfs) = dblkno;
				dblkno += btodb(PAGE_SIZE);
			} while (kpp = kpp->pg_pnext);

			error = ufs_rwblk(vp, kpl, kplcnt, 
					  async|B_READ, rablkno); 
			if (error) {
				if (khold & UBC_HACP) {
					for (kpp = pp->pg_pnext; kpp;
					     kpp = app) {
						app = kpp->pg_pnext;
						ubc_page_release(kpp,
								 0);
					}
				}
				goto failed;
			}

			if (khold & UBC_HACP) {
				 for (kpp = pp; kpp; kpp = kpp->pg_pnext) {
					if (protp) {
						if (rwflg & B_READ || async)
							*protp++ =
								VM_PROT_WRITE;
						else {
							*protp++ = 0;
							ubc_page_dirty(kpp);
						}
					}
					*pl++ = kpp;
				}
				*pl = VM_PAGE_NULL;
				return 0;
			}
			if (protp) {
				if (rwflg & B_READ || async)
					*protp = VM_PROT_WRITE;
				else {
					*protp = 0;
					ubc_page_dirty(pp);
				}
			}
		} else if (dblkno == (daddr_t) -1) {
			/*
			 * Must be read case because write 
			 * would cause allocation.
			 */

			if (protp)
				*protp = VM_PROT_WRITE;
			ubc_page_zero(pp, 0, PAGE_SIZE);
			(daddr_t) (pp->pg_pfs) = (daddr_t) -1;
			ubc_page_release(pp, B_DONE);
		} else if (alloc) {
			static char ufsalloc_panic[] =
				"ufs_getapage: allocation failed";

			if (newsize) {
				BM(IN_LOCK(ip));
				ip->i_size = offset + len;
				ip->i_flag |= IUPD|ICHG|ICHGMETA;
				BM(IN_UNLOCK(ip));
			}

			/*
			 * Allocate a page if ubc_lookup() failed
			 * to find a resident page.
			 * If ubc_lookup() returned a page, we are
			 * allocating a block in a sparse file.
			 */
			if (pp == VM_PAGE_NULL) {
				uflags = 0;
				error = ubc_page_alloc(vp, trunc_page(offset), 
						       fs->fs_bsize,
						       len + offset,
						       &app, &uflags);
				pp = app;
				if (error || (uflags & B_NOCACHE) == 0) 
					panic(ufsalloc_panic);
			}
holealloc:
			if ((iosize = blksize(fs, ip, bn)) > PAGE_SIZE) {
				register vm_offset_t start;
				vm_page_t npp;

				for(start = offset & fs->fs_bmask; iosize;
				    start += PAGE_SIZE, 
				    dblkno += btodb(PAGE_SIZE)) {
					if (start != trunc_page(offset)) {
#ifdef notdef
						uflags = 0;
						error = ubc_page_alloc(vp, 
							start, 
							fs->fs_bsize, 
							len + offset, &npp, 
							&uflags);
						if (error || 
						(uflags & B_NOCACHE) == 0) 
							panic(ufsalloc_panic);
#endif
						/*
						 * Only allocate a new page
						 * if page not resident.
						 */
						uflags = 0;
						error = ubc_lookup(vp, 
								   start, 
								   fs->fs_bsize, 
								   len + offset,
								   &npp,
								   &uflags);
						if (error)
							panic(ufsalloc_panic);
						if (uflags & B_NOCACHE)
							ubc_page_release(npp,
									 B_DONE);
						ubc_page_zero(npp, 0,
							      PAGE_SIZE);
						(daddr_t) (npp->pg_pfs) = 
							dblkno;
						ubc_page_release(npp, B_DIRTY);
					} else
						(daddr_t) (pp->pg_pfs) =
							dblkno;
					iosize -= MIN(PAGE_SIZE,iosize);
				}
			}
			else
				(daddr_t)(pp->pg_pfs) = dblkno;

			if (protp)
				*protp = 0;
			ubc_page_zero(pp, 0, PAGE_SIZE);
		}
	}
done:
	*pl = pp;
	*(pl + 1) = VM_PAGE_NULL;
	return 0;

failed:
	*pl = VM_PAGE_NULL;
	if (pp)
		ubc_page_release(pp, 0);
	return error;

}

ufs_getpage(struct vnode *vp,
	register vm_offset_t offset,
	vm_size_t len,
	vm_prot_t *protp,
	register vm_page_t *pl,
	register int plsz,
	vm_map_entry_t ep,
	vm_offset_t addr,
	int rwflg,
	struct ucred *cred)
{
	register struct inode *ip = VTOI(vp);
	off_t isize;
	int error;
	register vm_offset_t end;
	struct fs *fs;

	if (ep) {
		if (rwflg & B_READ)
			IN_READ_LOCK(ip);
		else
			IN_WRITE_LOCK(ip);
	}
	fs = ip->i_fs;
	if (fs->fs_bsize < PAGE_SIZE)
		panic("ufs_getpage: fs_bsize < PAGE_SIZE");

	BM(IN_LOCK(ip));
	isize = ip->i_size;
	BM(IN_UNLOCK(ip));

	if ((offset + len) > round_page(isize) && ep) {
		error =  KERN_INVALID_ADDRESS;
		goto done;
	}

	/*
	 * Fetch the first page syncrhonously
	 */

	error = ufs_getapage(vp, offset, len, protp, pl, ep,  rwflg, 0, cred);
	if (error) {
		*pl = VM_PAGE_NULL;
		goto done;
	}
	else if ((((offset & page_mask) + len) <= PAGE_SIZE) && plsz == 1) 
		goto done;
	else pl++;

	if (ep) {
		vm_offset_t back, forward;
		register vm_offset_t noffset;

		if ((*ep->vme_kluster)(ep, addr, plsz, &back, &forward))
			goto done;
		protp++;

		/*
		 * klustering causes only read condition faults
		 */

			
		if (forward > offset) for (end = forward, 
				noffset = offset + PAGE_SIZE,
				len = forward - offset; 
				noffset <= end; noffset += PAGE_SIZE, 
				len -= PAGE_SIZE, pl++, protp++)  {
			if (*pl != VM_PAGE_NULL) continue;
			else if (error = ufs_getapage(vp, noffset, 
				len, protp, pl, ep, 1, B_ASYNC, cred)) {
				*pl = VM_PAGE_NULL;
					goto done;
			}
		}
		if (back < offset) for (noffset = back,
				len = offset - back; noffset < offset; 
				noffset += PAGE_SIZE, len -= PAGE_SIZE, 
				pl++, protp++) {
			if (*pl != VM_PAGE_NULL) continue;
			else if (error = ufs_getapage(vp, noffset, 
				len, protp, pl, ep, 1, B_ASYNC, cred)) {
				*pl = VM_PAGE_NULL;
				goto done;
			}
		}
	}
	else {
		len += (offset & page_mask);
		offset = trunc_page(offset);
		for (end = offset + len, offset += PAGE_SIZE, len -= PAGE_SIZE;
			offset < end; 
			offset += PAGE_SIZE, pl++, len -= PAGE_SIZE) {
			if (*pl != VM_PAGE_NULL) continue;
			else if (error = ufs_getapage(vp, offset, len,
				protp, pl, ep, rwflg, B_ASYNC, cred)) {
				*pl = VM_PAGE_NULL;
				goto done;
			}
		}
	}
done:
	if (ep) {
		if (rwflg & B_READ)
			IN_READ_UNLOCK(ip);
		else
			IN_WRITE_UNLOCK(ip);
	}
	return error;
	
}

ufs_putpage(register struct vnode *vp,
	register vm_page_t *pl,
	register int pcnt,
	int flags,
	struct ucred *cred)
{
	register struct inode *ip;
	register vm_page_t pp, plp;
	register struct fs *fs;
	int error, plcnt, maxcontig;
	vm_offset_t start;
	vm_size_t length;

	ip = VTOI(vp);

	if (flags & B_UBC) {
		VN_LOCK(vp);
		if (vp->v_flag & VXLOCK) {
			VN_UNLOCK(vp);
			while (pcnt--) {
				ubc_page_release(*pl, B_DONE|B_DIRTY);
				*pl++ = VM_PAGE_NULL;
			}
			return 0;
		}
		else VN_UNLOCK(vp);
	}
	else if (flags & B_MSYNC) {
		IN_LOCK(ip);
		ip->i_flag |= ICHG;
		IN_UNLOCK(ip);
		iupdat(ip, &time, &time, 1);
	}

	fs = ip->i_fs;
	maxcontig = cluster_maxcontig;

	/*
	 * Push each page and mark a successful push as a null page.
	 */

	while (pcnt--) {
		pp = *pl;
		/*
		 * Call ubc_dirty_kluster() looking for a full
		 * extent of dirty data. ufs_writepages() will
		 * write out the page list employing cluster logic
		 * for logically contiguous pages.
		 *
		 * Unfortunaltely, we can't tell ufs_writepages()
		 * to write just one logically contiguous cluster
		 * because ubc_dirty_kluster() does nasty things
		 * to pages retrieved with flags (B_FREE|B_INVAL).
		 */
		plp = ubc_dirty_kluster(vp, pp, pp->pg_offset & fs->fs_bmask,
					fs->fs_bsize * maxcontig, flags,
					FALSE, &plcnt);
		error = ufs_writepages(plp, ip, flags, 0, 0);
		if (error)
			return error;
		*pl++ = VM_PAGE_NULL;
	}
	return 0;
}

ufs_bread(register struct vnode *vp,
	  off_t lbn,
	  struct buf **bpp,
	  struct ucred *cred)
{
	register struct inode *ip;
	register struct fs *fs;
	vm_page_t pl[VP_PAGELIST+1];
	register vm_page_t *pp, lp, fp;
	register struct buf *bp;
	int error;

	ip = VTOI(vp);
	fs = ip->i_fs;

	IN_READ_LOCK(ip);

	/*
	 * If this read will go past end of file,
	 * force caller to use ufs_rwip().
	 */
	if ((lbn * fs->fs_bsize) + fs->fs_bsize > ip->i_size) {
		IN_READ_UNLOCK(ip);
		return(EOPNOTSUPP);
	}

	error = ufs_getpage(vp, lbn * fs->fs_bsize, fs->fs_bsize,
			    (vm_prot_t *) 0, pl,
			    atop(round_page(fs->fs_bsize)),
			    (vm_map_entry_t) 0, (vm_offset_t) 0,
			    B_READ, cred);

	if (error) {
		IN_READ_UNLOCK(ip);
#ifdef DEBUG
		printf("ufs_bread: ufs_getpage returned error %d\n", error);
#endif
		return (error);
	}
	bp = (struct buf *)ubc_bufget();
	bp->b_flags = B_READ|B_DONE;
	bp->b_bcount = 0;
	bp->b_resid = 0;
	/*
	 * Link pages together
	 */
	for (lp = 0, pp = pl; (*pp != VM_PAGE_NULL); pp++) {
		vm_page_wait(*pp);
		if (lp)
			lp->pg_pnext = *pp;
		else
			fp = *pp;
		lp = *pp;
		bp->b_bcount += PAGE_SIZE;
	}
	lp->pg_pnext = VM_PAGE_NULL;
	fp->pg_pprev = lp;
	/*
	 * Call bp_mapin() to allocate virtually contiguous
	 * space for buffer to point at.
	 */
	bp->b_pagelist = fp;
	bp->b_un.b_addr = (caddr_t) (page_to_phys(fp));
	if(error = bp_mapin(bp))
		ufs_brelse(vp,bp);
	else
		*bpp = bp;
	return (error);
}

ufs_brelse(register struct vnode *vp,
	   register struct buf *bp)
{
	register vm_page_t pp, cp;
	register struct inode *ip;

	ip = VTOI(vp);
	pp = bp->b_pagelist;
	ubc_buffree(bp);
	do {
		cp = pp;
		pp = pp->pg_pnext;
		ubc_page_release(cp, 0);
	} while (pp != VM_PAGE_NULL);
	IN_READ_UNLOCK(ip);
	return (0);
}

ufs_swap(register struct vnode *vp,
	vp_swap_op_t swop,
	vm_offset_t args)
{
	panic("ufs_swap: swapping currently not supported in UFS");
}

ufs_writepages(register vm_page_t fp,
	       struct inode *ip,
	       int ioflags,
	       int write_one,
	       int *pcnt) /* != 0, return number of pages written */
{
	register vm_page_t lp, pp, tp;
	struct fs *fs;
	struct vnode *vp = ITOV(ip);
	int maxcontig, just_release, pages, error, ret, tot_pages;
#ifdef CLUSTER_DEBUG
	int cldebug = 0;
#endif

	fs = ip->i_fs;
	maxcontig = cluster_maxcontig;
	maxcontig = (maxcontig * fs->fs_bsize) / PAGE_SIZE;
	ret = 0;
#ifdef CLUSTER_DEBUG
  if ((cluster_write_debug && maxcontig > 1) || cluster_debug || (ip->i_devvp)->v_rdev == cluster_write_debug_dev) {
     cldebug = 1;
     printf("ufs_writepages: fp 0x%lx maxcontig pages = %d (ioflags = 0x%x)\n",
	    fp, maxcontig, ioflags);
  }
#endif
	/*
	 * Count up logically consecutive dirty pages and call
	 * ufs_rwblk() to schedule I/O.
	 *
	 * fp = first page in entire dirty list
	 * lp = beginning page of contiguous list
	 * pp = current page being looked at
	 * tp = page before pp in page list
	 */
	tot_pages = pages = 0;
	just_release = 0;
	for (pp = lp = tp = fp; pp != VM_PAGE_NULL; pp = pp->pg_pnext) {
		/*
		 * For all the pages in the dirty list,
		 * check for logical contiguous pages.
		 */
		if (just_release) {
			ubc_page_release(tp, B_DONE|B_DIRTY);
		} else if ((lp->pg_pfs + (pages * btodb(PAGE_SIZE)) !=
			   pp->pg_pfs) || pages >= maxcontig) {
			tp->pg_pnext = VM_PAGE_NULL;
			/*
			 * Current page isn't contiguous or a cluster
			 * has been built up. The current page becomes
			 * head of a new list. Put last page of
			 * contiguous list in newly created page list.
			 */
			pp->pg_pprev = lp->pg_pprev;
			/*
			 * The contiguous list has a new last page.
			 */
			lp->pg_pprev = tp;
			error = ufs_rwblk(vp, lp, pages, ioflags, 0);
			if (error && !ret)
				ret = error;
			tot_pages += pages;
			/*
			 * Current page is now the beginning of
			 * contiguous page list.
			 */
			lp = pp;
			pages = 1;
			if (write_one)
				just_release = 1;
		} else
			pages++;
		tp = pp;
	}
	if (just_release) {
		ubc_page_release(tp, B_DONE|B_DIRTY);
	} else {
		/*
		 * No more pages in dirty list, write out contiguous
		 * list of pages
		 */
		tp->pg_pnext = VM_PAGE_NULL;
		lp->pg_pprev = tp;
		error = ufs_rwblk(vp, lp, pages, ioflags, 0);
		if (error && !ret)
			ret = error;
		tot_pages += pages;
	}
#ifdef CLUSTER_DEBUG
  if (cldebug)
     printf("ufs_writepages: Wrote %d pages (ioflags = 0x%x)\n",
	    tot_pages, ioflags);
#endif
	if (pcnt)
		*pcnt = tot_pages;
	return (ret);
}


ufs_flushdata(register struct inode *ip,
	      off_t ostart,
	      off_t olength,
	      int sync,
	      int write_one)
{
	off_t start, length;
	vm_page_t fp;
	struct vnode *vp;
	struct fs *fs;
	int pages, error, flags, ret, elbn;
	off_t flushoff, flushlen, flushend, delayend;
#ifdef CLUSTER_DEBUG
	int cldebug = 0;
#endif

	/*
	 * Return if nothing to flush
	 */
	if (olength == 0L)
		return (0);

	vp = ITOV(ip);
	fs = ip->i_fs;
	error = 0;
	flags = 0;
	ret = 0;

	/*
	 * UFS should always flush data blocks in fs_bsize and
	 * fs_fsize increments.
	 */
	start = lblkno(fs, ostart) * (off_t)fs->fs_bsize;
	elbn = lblkno(fs, ostart + olength - 1);
	length = blksize(fs, ip, elbn);
	length += elbn * (off_t)fs->fs_bsize;
	length -= start;

	if (sync) {
		flags = B_WANTED;	/* Wait for busy/wired pages */
		write_one = 0;
	}
#ifdef CLUSTER_DEBUG
  if ((cluster_write_debug && cluster_maxcontig > 1) || cluster_debug || (ip->i_devvp)->v_rdev == cluster_write_debug_dev) {
     cldebug = 1;
     printf("ufs_flushdata: ostart %ld olength %d start %ld length %d\n",
	    ostart, olength, start, length);
  }
#endif
	flushoff = start;
	flushlen = 0;
	do {
		/*
		 * Find all dirty pages between start and start + length
		 * that are not busy for I/O.
		 */
		fp = ubc_dirty_kluster(vp, (vm_page_t) 0, start, length,
				       flags, FALSE, &pages);
		if (fp == VM_PAGE_NULL) {
			flushlen += length;
			goto out;
		}
#ifdef CLUSTER_DEBUG
  if (cldebug)
     printf("ufs_flushdata: ip 0x%x found %d dirty pages\n",
	    ip, pages);
#endif
		if (start < fp->pg_offset) {
			flushlen += (fp->pg_offset - start);
			length -= (fp->pg_offset - start);
			start = fp->pg_offset;
		}
		/*
		 * Call ufs_writepages() to actually do the I/O.
		 * This routine will use cluster logic if applicable.
		 */
		error = ufs_writepages(fp, ip, (sync ? 0 : B_ASYNC),
				       write_one, &pages);
		if (error && !ret)
			ret = error;
		start += (pages * PAGE_SIZE);
		length -= (pages * PAGE_SIZE);
		flushlen += (pages * PAGE_SIZE);
	} while (length > 0 && !write_one);
out:
	IN_LOCK(ip);
	delayend = ip->i_delayoff + ip->i_delaylen - 1;
	flushend = flushoff + flushlen - 1;
	/*
	 * If there is an overlap between the data we wrote, and
	 * the delayed dirty data, reset the inode cluster counters.
	 */
	if (ip->i_delaylen &&
	    flushoff < delayend && flushend > ip->i_delayoff) {
		if (flushoff <= ip->i_delayoff && flushend >= delayend) {
			/*
			 * Flushed region completely overlaps delayed data
			 */
			ip->i_delayoff = 0;
			ip->i_delaylen = 0;
		} else if (flushoff <= ip->i_delayoff && flushend < delayend) {
			/*
			 * Flushed region overlaps beginning of delayed data.
			 */
			ip->i_delaylen -= (flushend - ip->i_delayoff);
			ip->i_delayoff = flushend;
			ip->i_delayoff++;
			ip->i_delaylen--;
		} else if (flushoff > ip->i_delayoff && flushoff < delayend &&
			   flushend >= delayend) {
			/*
			 * Flushed region overlaps end of delayed data.
			 */
			ip->i_delaylen -= (delayend - flushoff);
			ip->i_delaylen--;
		}
	}
	IN_UNLOCK(ip);
#ifdef CLUSTER_DEBUG
  if (cldebug)
     printf("ufs_flushdata: returning ip 0x%lx i_delayoff %ld i_delaylen %d\n",
	    ip, ip->i_delayoff, ip->i_delaylen);
#endif
	return (ret);
}

ufs_delay_cluster(register struct inode *ip,
		  off_t ooff,
		  int olen)
{
	int maxcontig;
	struct fs *fs;
	off_t off, maxcontigb;
#ifdef CLUSTER_DEBUG
	int cldebug = 0;
#endif

	/*
	 * Initialize variables.
	 */
	fs = ip->i_fs;
	maxcontig = cluster_maxcontig;
	maxcontigb = maxcontig * fs->fs_bsize;
	off = ooff - (ooff % PAGE_SIZE);

#ifdef CLUSTER_DEBUG
  if ((cluster_write_debug && cluster_maxcontig > 1) || cluster_debug || (ip->i_devvp)->v_rdev == cluster_write_debug_dev) {
     cldebug = 1;
     printf("ufs_delay_cluster: ip 0x%lx uoffset %ld poffset %ld len %d ",
	    ip, ooff, off, olen);
     printf("i_delayoff %d i_delaylen %d\n", ip->i_delayoff, ip->i_delaylen);
  }
#endif
	/*
	 * McVoy / Kleiman
	 *
	 * This page can be delayed if the current length of the
	 * delayed data is less than the maximum cluster size, and
	 * the offset for this request immediately follows the end
	 * of the previously delayed data.
	 *
	 * Note: we do not know if the data is logically
	 * contiguous until we write it in ufs_flushdata().
	 */
	IN_LOCK(ip);
	if (ip->i_delayoff + ip->i_delaylen == off &&
	    ip->i_delaylen + PAGE_SIZE <= maxcontigb) {
		if ((ooff + olen) % fs->fs_bsize == 0)
			ip->i_delaylen += PAGE_SIZE;
		IN_UNLOCK(ip);
		return (0);
	}
	/*
	 * Check if all inclusive
	 */
	if (off >= ip->i_delayoff && off < ip->i_delayoff + ip->i_delaylen) {
		IN_UNLOCK(ip);
		return (0);
	}
	IN_UNLOCK(ip);

#ifdef CLUSTER_DEBUG
  if (cldebug) {
     printf("ufs_delay_cluster: ip 0x%x offset %d calling ufs_flushdata ",
	    ip, off);
     printf("with i_delayoff %d i_delaylen %d\n",
	    ip->i_delayoff, ip->i_delaylen);
  }
#endif

	/*
	 * Find all pages from delayoff to
	 * delayoff + delaylen and write them
	 * out asynchronously.
	 */
	ufs_flushdata(ip, ip->i_delayoff, ip->i_delaylen, 0,
		      cluster_write_one);

	IN_LOCK(ip);
	/*
	 * Check if this page can be appended to the
	 * current delayed data.
	 */
	if (cluster_write_one &&
	    ip->i_delayoff + ip->i_delaylen == off &&
	    ip->i_delaylen + PAGE_SIZE <= maxcontigb) {
		/*
		 * Append page to current cluster
		 * if write dirties last byte in block
		 */
		if ((ooff + olen) % fs->fs_bsize == 0)
			ip->i_delaylen += PAGE_SIZE;
	} else {
		/*
		 * Begin delayed cluster with this page
		 * if write dirties last byte in block
		 */
		if ((ooff + olen) % fs->fs_bsize == 0) {
			ip->i_delayoff = off;
			ip->i_delaylen = PAGE_SIZE;
		} else {
			ip->i_delayoff = 0;
			ip->i_delaylen = 0;
		}
	}
	IN_UNLOCK(ip);
	return (0);
}

int delay_wbuffers = 0;
int delay_wbuffers_percent = 70;
struct delay_stats {
	int checks;
	int delays;
	int too_dirty;
} delay_stats;

ufs_rwip(register struct vnode *vp, 
	register struct uio *uio, 
	int ioflag, 
	struct ucred *cred)
{
	register struct vm_vp_object *vop;
	vm_page_t pl[VP_PAGELIST+1];
	register vm_page_t *pp;
	register struct fs *fs;
	struct inode *ip;
	register long pgoff;
	vm_offset_t addr;
	register vm_size_t len;
	vm_size_t n, diff;
	int rwflg, extend_write, error;
	off_t isize, bsize;
	vm_offset_t sync_off, uoff;
	vm_size_t sync_len;

	ip = VTOI(vp);
	fs = ip->i_fs;

	if (uio->uio_rw == UIO_READ)
		rwflg = B_READ; /* == B_CLRBUF */
	else if (ioflag & IO_SYNC)
		rwflg = (B_SYNC | (ioflag & IO_DATAONLY));
	else
		rwflg = B_WRITE;
	IN_LOCK(ip);
	isize = ip->i_size;
	if (rwflg & B_READ)
		ip->i_flag |= IACC;
	IN_UNLOCK(ip);
	error = 0;

	do {
		pp = pl; 
		pgoff = uio->uio_offset & page_mask;
                if ((uio->uio_resid + pgoff) > VP_PAGELISTSIZE)
                        len = VP_PAGELISTSIZE - pgoff;
                else
			len = uio->uio_resid;
		extend_write = 0;
		if ((uio->uio_offset + len) >= isize) { 
			if (rwflg & B_READ) {
				if (uio->uio_offset >= isize) {
					IN_READ_UNLOCK(ip);
					return 0;
				} else
					len = isize - uio->uio_offset;
			} else { 
                                len = MIN((unsigned)(fs->fs_bsize -
                                        blkoff(fs, uio->uio_offset)), len);
				isize = uio->uio_offset + len;
				++extend_write;
				bsize = ip->i_size;
			}
		}
		error = ufs_getpage(vp, uio->uio_offset, len, (vm_prot_t *) 0,
				    pl, atop(round_page(len + pgoff)),
				    (vm_map_entry_t) 0, (vm_offset_t) 0,
				    rwflg, cred);
		if (error)
			break;
		sync_off = sync_len = 0;
		do {
			vm_page_wait(*pp);
			uoff = uio->uio_offset;
			n = MIN((PAGE_SIZE - pgoff), uio->uio_resid);
			if (rwflg & B_READ) {
				diff = isize - uio->uio_offset;
				if (diff < n)
					n = diff;
			}
			addr = ubc_load(*pp, pgoff, n); 
			error = uiomove(addr + pgoff, n, uio);
			ubc_unload(*pp, pgoff, n);
			if (error) {
				/*
				 * Release held pages in case we
				 * call itrunc(), marking the first
				 * one as dirty.
				 */
				ubc_page_release(*pp, B_DIRTY);
				*pp = VM_PAGE_NULL;
				pp++;
				while (*pp != VM_PAGE_NULL) {
					vm_page_wait(*pp);
					ubc_page_release(*pp, 0);
					*pp = VM_PAGE_NULL;
					pp++;
				}
				/*
				 * If the uiomove of a file extending write
				 * failed, normalize the inode size
				 */
				if (extend_write) {
					if (uio->uio_offset > bsize)
						bsize = uio->uio_offset;
					IN_SET_RECURSIVE(ip);
					itrunc(ip, bsize, (ioflag & IO_SYNC));
					IN_CLEAR_RECURSIVE(ip);
				}
				break;
			}
			if ((rwflg & B_READ) == 0) {
				ubc_page_release(*pp, B_DIRTY);
				/*
				 * Synchronous writes will be delayed
				 * and written after parsing all the pages
				 * in the list.
				 */
				if (ioflag & IO_SYNC) {
					if (sync_len == 0) {
						sync_off = (*pp)->pg_offset;
						sync_len = PAGE_SIZE;
					} else
						sync_len += PAGE_SIZE;
				} else {
					extern int ubc_dirtypages, ubc_pages;
					/*
					 * ufs_delay_cluster() will either
					 * add this page to the current
					 * cluster, or write out a cluster
					 * and start another with this
					 * page.
					 */
					delay_stats.checks++;
					if (delay_wbuffers) {
						if (100*ubc_dirtypages/ubc_pages > delay_wbuffers_percent) {
							delay_stats.too_dirty++;
							ufs_delay_cluster(ip,
									  uoff,
									  n);
						} else
							delay_stats.delays++;

					} else
						ufs_delay_cluster(ip,
								  uoff,
								  n);
				}
				IN_LOCK(ip);
				ip->i_flag |= IUPD|ICHG;
				IN_UNLOCK(ip);
			} else {
				ubc_page_release(*pp, 0);
			}
			*pp = VM_PAGE_NULL;
			pp++;
			pgoff = 0;
		} while (error == 0 && (*pp != VM_PAGE_NULL) && uio->uio_resid);

		if (ioflag & IO_SYNC && sync_len) {
			error = ufs_flushdata(ip, sync_off, sync_len, 1, 0);
		}

	} while (error == 0 && uio->uio_resid);

	if (error || *pp != VM_PAGE_NULL) 
		while (*pp != VM_PAGE_NULL) {
			vm_page_wait(*pp);
			ubc_page_release(*pp, 0);
			pp++;
		}

	if (rwflg & B_READ)
		IN_READ_UNLOCK(ip);
	else
		IN_WRITE_UNLOCK(ip);
	return error;
}

ufs_lockctl(vp, eld, flag, cred, clid, offset)
        struct vnode *vp;
        struct eflock *eld;
        int flag;
        struct ucred *cred;
        pid_t clid;
        off_t offset;
{
        int error;

	/* 
	 * From nfssrc4.1: removed to pass PCTS
	 *
		if ((flag & RGETFLCK == 0) && (flag & RSETFLCK == 0)) {
			if (vp->v_type == VBLK || vp->v_type == VCHR ||
			    vp->v_type == VFIFO)
				return (EINVAL);
		} else {
	*/

	if (vp->v_type == VBLK || vp->v_type == VCHR || vp->v_type == VFIFO)
		return (EINVAL);

	/*
		}
	*/
		
        if (flag & CLNFLCK)
                return(cleanlocks(vp));
        if (flag & VNOFLCK)
                return(locked(vp, eld, flag));

        if (flag & GETFLCK) {
                return(getflck(vp, eld, offset, clid, FILE_LOCK));
        }
	if (flag & RGETFLCK) {
		return(getflck(vp, eld, offset, clid, LOCKMGR));
	}

        if (flag & SETFLCK) {
                return(setflck(vp, eld, flag&SLPFLCK, offset, clid, FILE_LOCK));
        }
        if (flag & RSETFLCK) {
		if (eld->l_type == F_UNLKSYS) {
			kill_proc_locks(eld);
			return(0);
		}
                return(setflck(vp, eld, flag&SLPFLCK, offset, clid, LOCKMGR));
        }

        if (flag & ENFFLCK)
                return(ufs_setvlocks(vp));
        return(EINVAL);
}

static int
ufs_setvlocks(vp)
	struct vnode *vp;
{
        struct inode *ip = VTOI(vp);

        IN_LOCK(ip);
        VN_LOCK(vp);
        if ((ip->i_mode & S_ISGID) && (!(ip->i_mode & S_IXGRP)))
                vp->v_flag |= VENF_LOCK;
        vp->v_flag |= VLOCKS;
        VN_UNLOCK(vp);
        IN_UNLOCK(ip);
        return(0);
}
