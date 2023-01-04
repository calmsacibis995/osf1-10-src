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
static char	sccsid[] = "@(#)$RCSfile: vfs_vnops.c,v $ $Revision: 4.3.28.13 $ (DEC) $Date: 1993/12/15 21:07:32 $";
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
 *	@(#)vfs_vnops.c	7.13 (Berkeley) 12/21/89
 */
/*
 * Revision History
 *
 *  8-Aug-91 -- prs
 *	if open is called with O_CREAT, retry open/create if VOP_CREAT fails with
 *	EEXIST due to races with other threads. The root of this problem is
 *	name() does not return the parent directory locked.
 */

#if	MACH
#include <mach_nbc.h>
#endif

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
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
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <ufs/inode.h>
#include <ufs/fs.h>
#include <ufs/quota.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/vfs_proto.h>
#if	MACH
#include <kern/assert.h>
#include <kern/parallel.h>
#endif

#include <dcedfs.h>

int	vn_read(), vn_write(), vn_ioctl(), vn_select(), vn_close();
struct 	fileops vnops =
	{ vn_read, vn_write, vn_ioctl, vn_select, vn_close };

struct vnode *consvp;
decl_simple_lock_data(,consvp_lock)

#define MAN_LOCK  1		/* for checking System V mandatory locking */
#ifdef i386
#define XENIX_LOCK 0
#endif
extern struct vnodeops procfs_vnodeops;
extern struct vnodeops cdfs_vnodeops;
extern struct vnodeops dead_vnodeops;

/* If we're hitting the open/eexist race, rather than looping
 * in the kernel indefinitely, we'll give ourselves an out by
 * only doing it some large/finite amount of times.
 * (Note that non-excl open 'should' never return EEXIST.)
 */
long max_open_eexist_race = 10000; /* so we can 'tune' this */


/*
 * Common code for vnode open operations.
 * Check permissions, and call the VOP_OPEN or VOP_CREATE routine.
 */
vn_open(ndp, fmode, cmode)
	register struct nameidata *ndp;
	int fmode, cmode;
{
	register struct vnode *vp;
	struct vnode *newvp;
	struct vattr vat;
	struct vattr *vap = &vat;
	int error;
	enum vtype	type;
	u_long v_flag;
	int trys = 0;
#ifdef COMPAT_43
	int oflag;
	extern void spec_setopen();
#endif

	if (fmode & FCREAT) {
again:
		ndp->ni_nameiop = CREATE | WANTPARENT;
		if ((fmode & FEXCL) == 0)
			ndp->ni_nameiop |= FOLLOW;
		if (error = namei(ndp))
			return (error);
		if (ndp->ni_vp == NULL) {
			vattr_null(vap);
			vap->va_type = VREG;
			vap->va_mode = cmode;
			VOP_CREATE(ndp, vap, error);
			if (error) {
				if ((error == EEXIST) &&
				    ((fmode & FEXCL) == 0) &&
				    (++trys < max_open_eexist_race))
					goto again;
				return (error);
			}
			fmode &= ~FTRUNC;
			vp = ndp->ni_vp;
		} else {
			if (ndp->ni_dvp == ndp->ni_vp)
				vrele(ndp->ni_dvp);
			else if (ndp->ni_dvp != NULL)
				vrele(ndp->ni_dvp);
			ndp->ni_dvp = NULL;
			vp = ndp->ni_vp;
			if (fmode & FEXCL) {
				error = EEXIST;
				goto bad;
			}
			fmode &= ~FCREAT;
		}
	} else {
		ndp->ni_nameiop = LOOKUP | FOLLOW;
		if (error = namei(ndp))
			return (error);
		vp = ndp->ni_vp;
	}
	VN_LOCK(vp);
	type = vp->v_type;
	v_flag = vp->v_flag;
	VN_UNLOCK(vp);
	if (type == VSOCK) {
		error = EOPNOTSUPP;
		goto bad;
	}

	if ((fmode & FCREAT) == 0) {
		if (fmode & FREAD) {
			VOP_ACCESS(vp, VREAD, ndp->ni_cred, error);
			if (error)
				goto bad;
		}
		if (fmode & (FWRITE|FTRUNC)) {
			/*
			 * This should probably be changed, since
			 * POSIX says TRUNC on directory has no effect
			 */
			if (type == VDIR) {
				error = EISDIR;
				goto bad;
			}
			if (error = vn_writechk(vp))
				goto bad;
			VOP_ACCESS(vp, VWRITE, ndp->ni_cred, error);
			if (error)
				goto bad;
		}
	}
	if ((fmode & FTRUNC) && type != VFIFO) {
	        /* truncating - check enforcement mode locks */
	        if (v_flag & VENF_LOCK) {
	                error = EAGAIN;
	                goto bad;
		}
#ifdef i386
		/* check Xenix locks */
	        if (v_flag & VXENIX) {
	                error = EAGAIN;
	                goto bad;
		}
#endif
		vattr_null(vap);
		vap->va_size = 0;
		VOP_SETATTR(vp, vap, ndp->ni_cred, error);
		if (error)
			goto bad;
	}
#ifdef COMPAT_43
	if (type == VCHR) {
		BM(unix_master());
		oflag = u.u_procp->p_flag;
		BM(unix_release());
	}
#endif
	newvp = vp;
	VOP_OPEN(&newvp, fmode, ndp->ni_cred, error);
	vp = newvp;
	ndp->ni_vp = vp;	/* in case it got cloned */
#ifdef COMPAT_43
	/*
	 * If a controlling tty was implicitly assigned,
	 * we must set the session controlling tty vnode
	 * pointer.  See code in tty.c:ttyopen().
	 */
	unix_master();
	if (!error && type == VCHR && !(fmode&O_NOCTTY) && !(oflag&SCTTY) &&
	    (u.u_procp->p_flag&SCTTY)) {
		u.u_procp->p_session->s_ttyvp = vp;
		unix_release();
		/*
		 * it's as if we're doing an open
		 */
		spec_setopen(vp);
	} else
		unix_release();
#endif /* COMPAT_43 */
	if (error)
		vrele(vp);
	else {
		VN_LOCK(vp);
		/*
		 * Don't increment reader and writer counts for fifos.
		 * fifo_open takes care of that.
		 */
		if (type != VFIFO) {
			if (fmode & FREAD)
				vp->v_rdcnt++;
			if (fmode & FWRITE)
				vp->v_wrcnt++;
		}
		VN_UNLOCK(vp);
	}
	return (error);
bad:
	vrele(vp);
	return(error);
}

/*
 * Check for write permissions on the specified vnode.
 * The read-only status of the file system is checked.
 * Also, prototype text segments cannot be written.
 */
vn_writechk(vp)
	register struct vnode *vp;
{
	enum vtype	type;

	/*
	 * Disallow write attempts on read-only file systems;
	 * unless the file is a socket or a block or character
	 * device resident on the file system.
	 */

	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));

	BM(MOUNT_LOCK(vp->v_mount));
	if (vp->v_mount->m_flag & M_RDONLY) {
		BM(MOUNT_UNLOCK(vp->v_mount));
		if (type != VCHR && type != VBLK &&
		    type != VSOCK && type != VFIFO)
			return (EROFS);
	} else
		BM(MOUNT_UNLOCK(vp->v_mount));

	return (0);
}

#if defined(DCEDFS) && DCEDFS
enum vnuio_rw { VNUIO_READ, VNUIO_WRITE, VNUIO_READNESTED, VNUIO_WRITENESTED };
#endif /* DCEDFS */

/*
 * Vnode version of rdwri() for calls on file systems.
 */
vn_rdwr(rw, vp, base, len, offset, segflg, ioflg, cred, aresid)
#if defined(DCEDFS) && DCEDFS
        enum vnuio_rw rw;
#else
	enum uio_rw rw;
#endif /* DCEDFS */
	struct vnode *vp;
	caddr_t base;
	int len;
	off_t offset;
	enum uio_seg segflg;
	int ioflg;
	struct ucred *cred;
	int *aresid;
{
	struct uio auio;
	struct iovec aiov;
	int error;

	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_base = base;
	aiov.iov_len = len;
	auio.uio_resid = len;
	auio.uio_offset = offset;
	auio.uio_segflg = segflg;

/*
 * Note: The introduction of the DCEDFS symbols makes 
 * reading the following code somewhat difficult, but 
 * the bottom line is: 
 *       if the DFS layered product is not installed, 
 *       then 
 *           the "NESTED" flags will not be set, and
 *           the end result is that either VOP_READ
 *           or VOP_WRITE is called.  
 */

#if defined(DCEDFS) && DCEDFS
	auio.uio_rw = (enum uio_rw) rw;
        if (rw == VNUIO_READNESTED)
          auio.uio_rw = UIO_READ;
        if (rw == VNUIO_WRITENESTED)
          auio.uio_rw = UIO_WRITE;
        if ((enum uio_rw)rw == UIO_READ) {
#else
	auio.uio_rw = rw;

	/*
	 * Two fields were added to the uio structure to support the
	 * SVR4 DDI/DKI interfaces.  Initialize them.
	 */
	ddi_init_uio(vp, ioflg, &auio, TRUE);

	if (rw == UIO_READ) {
#endif /* DCEDFS */
		VOP_READ(vp, &auio, ioflg, cred, error);
#if !(defined(DCEDFS) && DCEDFS)
	} else {
		VOP_WRITE(vp, &auio, ioflg, cred, error);
	}
#else
        } else if ((enum uio_rw)rw == UIO_WRITE) {
                VOP_WRITE(vp, &auio, ioflg, cred, error);
        } else if (rw == VNUIO_READNESTED) {
                if (vp->v_flag & V_CONVERTED)
                   error = dfs_readop(vp, &auio, ioflg, cred);
                else
                   VOP_READ(vp, &auio, ioflg, cred, error);
        } else if (rw == VNUIO_WRITENESTED) {
                if (vp->v_flag & V_CONVERTED)
                   error = dfs_writeop(vp, &auio, ioflg, cred);
                else
                   VOP_WRITE(vp, &auio, ioflg, cred, error);
        }
#endif /* DCEDFS */
	if (aresid)
		*aresid = auio.uio_resid;
	else
		if (auio.uio_resid && error == 0)
			error = EIO;
	return (error);
}

vn_read(fp, uio, cred)
	struct file *fp;
	struct uio *uio;
	struct ucred *cred;
{
	register struct vnode *vp = (struct vnode *)fp->f_data;
	int count, error, flag, iolocked = 0;
	u_long v_flag;
	enum vtype type;
	int lckflag = 0;
	int ioflag = 0;

	VN_LOCK(vp);
	type = vp->v_type;
	v_flag = vp->v_flag;
	VN_UNLOCK(vp);
	if (type == VREG || type == VDIR || type == VBLK) {
		FP_IO_LOCK(fp);
		iolocked++;
	} else if (type == VCHR && (v_flag & VENF_LOCK)) {
		/*
		 * Do not lock a character device unless you need to.
		 * Some files are used for both read and write and
		 * it can cause hangs if you lock it (eg rlogin).
		 */
		FP_IO_LOCK(fp);
		iolocked++;
	}
	uio->uio_offset = fp->f_offset;
	count = uio->uio_resid;
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	if ((v_flag & VENF_LOCK) && iolocked) {
		struct eflock lckdat;

		/* mandatory file locking enabled and locks exist on file */
		/* was flag&FNDELAY */

		lckdat.l_start = uio->uio_offset;
		if (count)
			lckdat.l_len = count + lckdat.l_start;
		else
			lckdat.l_len = MAXEND;
		lckflag |= (VNOFLCK | VMANFLCK | VRDFLCK);
		if (flag & (FNDELAY|FNONBLOCK))
			lckflag |= SLPFLCK;
		lckdat.l_rpid = 0;
		lckdat.l_rsys = 0;
		FP_IO_UNLOCK(fp);
		/*
		 * a no-op for NFS files since NFS does not support
		 * mandatory locking
		 */
		VOP_LOCKCTL(vp, &lckdat, lckflag, fp->f_cred, 
				u.u_procp->p_pid, 0, error);
		FP_IO_LOCK(fp);
		if (error)
			goto out;
		else {
			/*
			 * Since the f_offset could have changed on us when
			 * we released FP_IO_LOCK, and called the file 
			 * locking, we need to reset it so the calling code
			 * does not need to change.  This amounts to an
			 * implicit lseek, which only changes things if
			 * we did sleep on a lock.  Only reset if we
			 * woke up gracefully.
			 */
			fp->f_offset = uio->uio_offset;
		}
	}
#ifdef i386
	if (v_flag & VXENIX) {
                /* Xenix file locks exist on file */
		/* was flag&FNDELAY */
	        if (error = locked(fp, flag&(FNDELAY|FNONBLOCK), uio,
			       1, XENIX_LOCK))
		      goto out;
	}
#endif
	/*
	 * Two fields were added to the uio structure to support the
	 * SVR4 DDI/DKI interfaces.  Initialize them.
	 */

	ddi_init_uio(vp, flag, uio, FALSE);
	if(flag & FNDELAY)
		ioflag |= IO_NDELAY;
	if(flag & FNONBLOCK)
		ioflag |= IO_NONBLOCK;
	VOP_READ(vp, uio, ioflag, cred, error);
	FP_LOCK(fp);
	fp->f_offset += count - uio->uio_resid;
	FP_UNLOCK(fp);
out:
	if (iolocked)
		FP_IO_UNLOCK(fp);
	return (error);
}

vn_write(fp, uio, cred)
	struct file *fp;
	struct uio *uio;
	struct ucred *cred;
{
	register struct vnode *vp = (struct vnode *)fp->f_data;
	register struct mount *mountp; 
	int count, flag, error, ioflag = 0, iolocked = 0;
	u_long v_flag;
	enum vtype type;
	int lckflag = 0;

	VN_LOCK(vp);
	type = vp->v_type;
	v_flag = vp->v_flag;
	mountp = vp->v_mount;
	VN_UNLOCK(vp);
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	if (type == VREG && (flag & FAPPEND))
		ioflag |= IO_APPEND;

	if(flag & FNDELAY)
		ioflag |= IO_NDELAY;
	if(flag & FNONBLOCK)
		ioflag |= IO_NONBLOCK;
	if (flag & FSYNC)
		ioflag |= IO_SYNC;
	if (type == VREG || type == VDIR) {
		MOUNT_LOCK(mountp);	
		/*
		 * If the filesystem was mounted synchronously, turn all
		 * writes into synchronous writes.
		 */
		if (mountp->m_flag & M_SYNCHRONOUS)
			ioflag |= IO_SYNC;
		MOUNT_UNLOCK(mountp);
		FP_IO_LOCK(fp);
		iolocked++;
	}
	else if (type == VBLK) {
		FP_IO_LOCK(fp);
		iolocked++;
	} else if (type == VCHR && (v_flag & VENF_LOCK)) {
		/*
		 * Do not lock a character device unless you need to.
		 * Some files are used for both read and write and
		 * it can cause hangs if you lock it (eg rlogin).
		 */
		FP_IO_LOCK(fp);
		iolocked++;
	}
	uio->uio_offset = fp->f_offset;
	count = uio->uio_resid;

	if ((v_flag & VENF_LOCK) && iolocked) {
		struct eflock lckdat;

                /* mandatory file locking enabled and locks exist on file */
	        /* was flag&FNDELAY */
		lckdat.l_start = uio->uio_offset;
		if (count)
			lckdat.l_len = count + lckdat.l_start;
		else
			lckdat.l_len = MAXEND;
		lckflag |= (VNOFLCK | VMANFLCK);
		if (flag & (FNDELAY|FNONBLOCK))
			lckflag |= SLPFLCK;
		lckdat.l_rpid = 0;
		lckdat.l_rsys = 0;
		FP_IO_UNLOCK(fp);
		/*
		 * a no-op for NFS files since NFS does not support
		 * mandatory locking
		 */
		VOP_LOCKCTL(vp, &lckdat, lckflag, fp->f_cred, 
			u.u_procp->p_pid, 0, error);
		FP_IO_LOCK(fp);
		if (error)
			goto out;
		else {
			/*
			 * Since the f_offset could have changed on us when
			 * we released FP_IO_LOCK, and called the file 
			 * locking, we need to reset it so the calling code
			 * does not need to change.  This amounts to an
			 * implicit lseek, which only changes things if
			 * we did sleep on a lock.  Only reset if we
			 * woke up gracefully.
			 */
			fp->f_offset = uio->uio_offset;
		}
	}
#ifdef i386
	if (v_flag & VXENIX) {
                /* Xenix file locks exist on file */
	        if (error = locked(fp, flag&FNDELAY, uio, 0, XENIX_LOCK))
		      goto out;
	}
#endif
	/*
	 * Two fields were added to the uio structure to support the
	 * SVR4 DDI/DKI interfaces.  Initialize them.
	 */

	ddi_init_uio(vp, flag, uio, FALSE);

	VOP_WRITE(vp, uio, ioflag, cred, error);
	FP_LOCK(fp);
	if (ioflag & IO_APPEND)
		fp->f_offset = uio->uio_offset;
	else
		fp->f_offset += count - uio->uio_resid;
	FP_UNLOCK(fp);
out:
	if (iolocked)
		FP_IO_UNLOCK(fp);
	return (error);
}

/*
 * Get stat info for a vnode.
 */
vn_stat(vp, sb)
	struct vnode *vp;
	register struct stat *sb;
{
	struct vattr vattr;
	register struct vattr *vap;
	int error;
	u_short mode;
	enum vtype type;

#if	SEC_ARCH
	if (security_is_on) {
	VOP_ACCESS(vp, SP_STATACC, u.u_cred, error);
	if (error)
		return error;
	}
#endif
	vap = &vattr;
	VOP_GETATTR(vp, vap, u.u_cred, error);
	if (error)
		return (error);
	/*
	 * Copy from vattr table
	 */
	sb->st_dev = vap->va_fsid;
	sb->st_ino = vap->va_fileid;
	mode = vap->va_mode;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	switch (type) {
	case VREG:
		mode |= S_IFREG;
		break;
	case VDIR:
		mode |= S_IFDIR;
		break;
	case VBLK:
		mode |= S_IFBLK;
		break;
	case VCHR:
		mode |= S_IFCHR;
		break;
	case VLNK:
		mode |= S_IFLNK;
		break;
	case VSOCK:
		mode |= S_IFSOCK;
		break;
	case VFIFO:
		mode |= S_IFIFO;
		break;
	default:
		return (EBADF);
	};
	sb->st_mode = mode;
	sb->st_nlink = vap->va_nlink;
	sb->st_uid = vap->va_uid;
	sb->st_gid = vap->va_gid;
	sb->st_rdev = vap->va_rdev;
	sb->st_size = vap->va_size;
	sb->st_atime = vap->va_atime.tv_sec;
	sb->st_spare1 = vap->va_atime.tv_usec;
	sb->st_mtime = vap->va_mtime.tv_sec;
	sb->st_spare2 = vap->va_mtime.tv_usec;
	sb->st_ctime = vap->va_ctime.tv_sec;
	sb->st_spare3 = vap->va_ctime.tv_usec;
	sb->st_blksize = vap->va_blocksize;
	sb->st_flags = vap->va_flags;
	sb->st_gen = vap->va_gen;
	sb->st_blocks = vap->va_bytes / S_BLKSIZE;
	return (0);
}

/*
 * Vnode ioctl call
 */
vn_ioctl(fp, com, data,retval)
	struct file *fp;
	unsigned int com;
	caddr_t data;
	long *retval;
{
	register struct vnode *vp = ((struct vnode *)fp->f_data);
	struct vattr vattr;
	int error, flag;
	enum vtype type;
	extern void spec_setopen();

	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	/*
	 * cdfs needs to be VREG/VDIR/VBLK for ioctl to work.
	 */
	if (vp->v_op == &cdfs_vnodeops) {
		type = VBLK;
	} else 
	/*
	 * procfs needs to be VREG for most ops, but VCHR for ioctl to work
	 */
	if (vp->v_op == &procfs_vnodeops) {
		type = VCHR;
	}
	/* end /proc code */
	switch (type) {

	case VREG:
	case VDIR:
		if (com == FIONREAD) {
#if	SEC_ARCH
			/*
			 * If a read access check was not performed at open
			 * time, perform a SP_STATACC access check now.
			 */
			if (security_is_on) {
			BM(FP_LOCK(fp));
			flag = fp->f_flag;
			BM(FP_UNLOCK(fp));
			if ((flag & FREAD) == 0) {
				VOP_ACCESS(vp, SP_STATACC, u.u_cred, error);
				if (error)
					return error;
			}
			}
#endif
			FP_IO_LOCK(fp);
			VOP_GETATTR(vp, &vattr, u.u_cred, error);
			if (error) {
				FP_IO_UNLOCK(fp);
				return (error);
			}
			*(off_t *)data = vattr.va_size - fp->f_offset;
			FP_IO_UNLOCK(fp);
			return (0);
		}
		if (com == FIONBIO || com == FIOASYNC)	/* XXX */
			return (0);			/* XXX */
		/* fall into ... */

	default:
		return (ENOTTY);

	case VBAD:
		if (vp->v_op == &dead_vnodeops)
			return(EIO);
		else
			return (ENOTTY);
	case VFIFO:
	case VCHR:
	case VBLK:
#if	SEC_ARCH
		/*
		 * If read and write access checks were not performed at open
		 * time, perform a SP_IOCTLACC access check now.
		 */
		if (security_is_on) {
		BM(FP_LOCK(fp));	/* possibly combine with op below? */
		flag = fp->f_flag;
		BM(FP_UNLOCK(fp));
		if ((flag & (FREAD|FWRITE)) != (FREAD|FWRITE)) {
			VOP_ACCESS(vp, SP_IOCTLACC, u.u_cred, error);
			if (error)
				return error;
		}
		}
#endif
		BM(FP_LOCK(fp));
		flag = fp->f_flag;
		BM(FP_UNLOCK(fp));
		VOP_IOCTL(vp, com, data, flag, u.u_cred, error, retval);
		if ((com == TIOCSCTTY) && (error == EJUSTRETURN)) {
			/* This is a special case: the user sent TIOCSCTTY
			 * down /dev/tty (syioctl).  This needs to be a NOP.
			 */
			return(0);
		}
		if (error)
			return error;
		if (com == TIOCSCTTY) {
			struct session *sess;

			sess = u.u_procp->p_session;
			SESS_LOCK(sess);
			if ((sess->s_ttyvp != NULLVP) &&
			    (sess->s_ttyvp != vp)) {
				struct vnode *tvp;
				tvp = sess->s_ttyvp;
				sess->s_ttyvp = vp;
				SESS_UNLOCK(sess);
				(void)spec_close(tvp,
					 0, u.u_cred);
				vrele(tvp);
			} else {
				sess->s_ttyvp = vp;
				SESS_UNLOCK(sess);
			}
			/*
			 * it's as if we're doing an open
			 */
			spec_setopen(vp);
		} else if (com == TIOCCONS) {
#if	SEC_BASE
			if (!privileged(SEC_ALLOWDACACCESS, 0))
				return (EPERM);
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				return (error);
#endif
			VN_LOCK(vp);
			simple_lock(&consvp_lock);
			if (*(int *)data) {
				if (!consvp) {
					vp->v_usecount++;
					consvp = vp;
				} else {
					error = EBUSY;
				}
			} else {
				if (vp == consvp) {
					consvp = NULLVP;
					vp->v_usecount--;
				} else if (consvp) {
					error = EBUSY;
				}
			}
			simple_unlock(&consvp_lock);
			VN_UNLOCK(vp);
		}
		return(error);
	}
}

/*
 * Vnode select call
 */
vn_select(fp, events, revents, scanning)
	struct file *fp;
	short *events, *revents;
	int scanning;
{
	int error;

	VOP_SELECT(((struct vnode *)fp->f_data), events, revents, scanning, u.u_cred, error);
	return(error);
}

/*
 * Vnode close call
 */
vn_close(fp)
	register struct file *fp;
{
	struct vnode *vp = ((struct vnode *)fp->f_data);
	int error, flag;

#if	UNIX_LOCKS
	/*
	 * Last close on a file structure; no other outstanding
	 * references.  Normal locking considerations do not apply:
	 * flags won't change unexpectedly, etc.
	 */
	ASSERT(fp->f_count == 1);
#endif
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	if (flag & (FSHLOCK|FEXLOCK))
		vn_funlock(fp, FSHLOCK|FEXLOCK);
	/*
	 * Must delete vnode reference from this file entry
	 * before VOP_CLOSE, so that only other references
	 * will prevent close.
	 */
	/*
	 * Shouldn't have to do this at all.  Who else checks f_data?  XXX
	 */
	FP_LOCK(fp);
	fp->f_data = (caddr_t) 0;
	FP_UNLOCK(fp);
	VOP_CLOSE(vp, flag, u.u_cred, error);
	VN_LOCK(vp);
	/*
	 * Don't decrement reader and writer counts for fifos.
	 * fifo_close takes care of that.
	 */
	if (vp->v_type != VFIFO) {
		if (flag & FREAD)
			vp->v_rdcnt--;
		if (flag & FWRITE)
			vp->v_wrcnt--;
	}
	VN_UNLOCK(vp);
	/*
	 * Unmount can race close as follows:
	 *	- unmount flushes the buffer cache
	 *	+ active vnode writer writes into buf cache
	 *	+ active vnode writer closes vnode
	 *	- unmount verifies no active vnodes on fs
	 *	- unmount allows fs to be unmounted, leaving
	 *	bogus buffer in memory.
	 * By taking the associated mount structure's lookup
	 * lock, the close will wait for the unmount to find
	 * this active vnode.
	 * N.B.  Only do this for vnodes attached to filesystems.
	 */
	if (vp->v_mount != DEADMOUNT) {
		struct mount *mp = vp->v_mount;

		MOUNT_LOOKUP_START(mp);
		vrele(vp);
		MOUNT_LOOKUP_DONE(mp);
	} else
		vrele(vp);
	return (error);
}

/*
 * Place an advisory lock on a vnode.
 * !! THIS IMPLIES THAT ALL STATEFUL FILE SERVERS WILL USE file table entries
 */
vn_flock(fp, cmd)
	register struct file *fp;
	int cmd;
{
	int error;
	register struct vnode *vp = (struct vnode *)fp->f_data;

	FP_LOCK(fp);

	/*
	 * If there's a exclusive lock currently applied
	 * to the file, then we've gotta wait for the
	 * lock with everyone else.
	 */
again:
	VN_LOCK(vp);
	while (vp->v_flag & VEXLOCK) {
		/*
		 * If we're holding an exclusive
		 * lock, then release it.
		 */
		if (fp->f_flag & FEXLOCK) {
			VN_UNLOCK(vp);
			FP_UNLOCK(fp);
			vn_funlock(fp, FEXLOCK);
			FP_LOCK(fp);
			VN_LOCK(vp);
			continue;
		}
		if (cmd & LOCK_NB) {
			VN_UNLOCK(vp);
			FP_UNLOCK(fp);
			return (EWOULDBLOCK);
		}
		/*
		 * Since we have both fp and vp locked, and you must
		 * get the fp lock first (see vn_funlock).  It should
		 * be safe to unlock the vnode, and let mpsleep
		 * unlock the fp for us.  mpsleep leaves lock unlocked
		 * on errors.
		 */
		vp->v_flag |= VLWAIT;
		VN_UNLOCK(vp);
		if (error = mpsleep((caddr_t)&vp->v_exlockc, PZERO + 1 | PCATCH,
				   "vn_exlock", 0, 
				   (void *) simple_lock_addr(fp->f_incore_lock),
				   MS_LOCK_SIMPLE))
			return (error);
		LASSERT(FP_LOCK_HOLDER(fp));
		VN_LOCK(vp);
	}
	if ((cmd & LOCK_EX) && (vp->v_flag & VSHLOCK)) {
		/*
		 * Must wait for any shared locks to finish
		 * before we try to apply a exclusive lock.
		 *
		 * If we're holding a shared
		 * lock, then release it.
		 */
		if (fp->f_flag & FSHLOCK) {
			VN_UNLOCK(vp);
			FP_UNLOCK(fp);
			vn_funlock(fp, FSHLOCK);
			FP_LOCK(fp);
			goto again;
		}
		if (cmd & LOCK_NB) {
			VN_UNLOCK(vp);
			FP_UNLOCK(fp);
			return (EWOULDBLOCK);
		}
		/*
		 * See comment above about use of mpsleep().
		 */
		vp->v_flag |= VLWAIT;
		VN_UNLOCK(vp);
		if (error = mpsleep((caddr_t)&vp->v_shlockc, PZERO + 1 | PCATCH,
				   "vn_shlock", 0, 
				   (void *) simple_lock_addr(fp->f_incore_lock),
				   MS_LOCK_SIMPLE))
			return (error);
		LASSERT(FP_LOCK_HOLDER(fp));
		goto again;
	}
	if (fp->f_flag & FEXLOCK)
		panic("vn_flock");
	if (cmd & LOCK_EX) {
		cmd &= ~LOCK_SH;
		vp->v_exlockc++;
		vp->v_flag |= VEXLOCK;
		fp->f_flag |= FEXLOCK;
	}
	if ((cmd & LOCK_SH) && (fp->f_flag & FSHLOCK) == 0) {
		vp->v_shlockc++;
		vp->v_flag |= VSHLOCK;
		fp->f_flag |= FSHLOCK;
	}
	VN_UNLOCK(vp);
	FP_UNLOCK(fp);
	return (0);
}

/*
 * Unlock a file.
 */
void
vn_funlock(fp, kind)
	register struct file *fp;
	int kind;
{
	register struct vnode *vp = (struct vnode *)fp->f_data;
	int flags;

	FP_LOCK(fp);
	kind &= fp->f_flag;
	if (vp == NULL || kind == 0) {
		FP_UNLOCK(fp);
		return;
	}
	VN_LOCK(vp);
	flags = vp->v_flag;
	if (kind & FSHLOCK) {
		if ((flags & VSHLOCK) == 0)
			panic("vn_funlock: SHLOCK");
		if (--vp->v_shlockc == 0) {
			vp->v_flag &= ~VSHLOCK;
			if (flags & VLWAIT)
				thread_wakeup((vm_offset_t)&vp->v_shlockc);
		}
		fp->f_flag &= ~FSHLOCK;
	}
	if (kind & FEXLOCK) {
		if ((flags & VEXLOCK) == 0)
			panic("vn_funlock: EXLOCK");
		if (--vp->v_exlockc == 0) {
			vp->v_flag &= ~(VEXLOCK|VLWAIT);
			if (flags & VLWAIT)
				thread_wakeup((vm_offset_t)&vp->v_exlockc);
		} 
		fp->f_flag &= ~FEXLOCK;
	}
	VN_UNLOCK(vp);
	FP_UNLOCK(fp);
}

/*
 * vn_fhtovp() - convert a fh to a vnode ptr (optionally locked)
 * 	- look up fsid in mount list (if not found ret error)
 *	- get vp by calling VFS_FHTOVP() macro
 */
vn_fhtovp(fhp, lockflag, vpp)
	fhandle_t *fhp;
	int lockflag;
	struct vnode **vpp;
{
	register struct mount *mp;
	int error;

	if ((mp = getvfs(&fhp->fh_fsid)) == NULL)
		return (ESTALE);
	VFS_FHTOVP(mp, &fhp->fh_fid, vpp, error);
	/* getvfs returned the mount structure locked */
	MOUNT_LOOKUP_DONE(mp);
	if (error)
		return (ESTALE);
	return (0);
}

/*
 * Noop
 */
vfs_noop()
{

	return (ENXIO);
}

/*
 * Null op
 */
vfs_nullop()
{

	return (0);
}


/*
 * Kernel open routine
 *   take a vnode and create a file descriptor from it
 */
int
vn_kopen(vp, fmode, fd)
        struct vnode *vp;
        int fmode;
        int *fd;
{
        struct file *fp;
        int indx;
        int error;
        extern struct fileops vnops;

        if (error = falloc(&fp, &indx))
                return(error);
        ASSERT(u.u_ofile[indx] == U_FD_RESERVED);
        ASSERT(fp->f_count == 1 && fp->f_type == DTYPE_RESERVED);

        VOP_OPEN(&vp, fmode, u.u_cred, error);
        if (error)
                {
                U_FD_SET(indx, NULL, &u.u_file_state);
                fdealloc(fp);
                return (error);
                }

        VN_LOCK(vp);
        /*
         * Don't increment reader and writer counts for fifos.
         * fifo_open takes care of that.
         */
        if (vp->v_type != VFIFO)
                {
                if (fmode & FREAD)
                        vp->v_rdcnt++;
                if (fmode & FWRITE)
                        vp->v_wrcnt++;
                }
        VN_UNLOCK(vp);

        FP_LOCK(fp);
        fp->f_flag = fmode & FMASK;
        fp->f_type = DTYPE_VNODE;
        fp->f_ops = &vnops;
        fp->f_data = (caddr_t)vp;
#if     SER_COMPAT
        fp->f_funnel = FUNNEL_NULL;
#endif
        FP_UNLOCK(fp);
        U_FD_SET(indx, fp, &u.u_file_state);
        *fd = indx;
        return (0);
}


/*
 * Console output redirection routine to be called by driver for
 * /dev/console if consvp is non-NULL.
 */
vn_consvp_write(uio)
	struct uio uio;
{
	register struct vnode *vp;
	int error;

	if (!(vp = consvp))
		return (ENXIO);

	VN_LOCK(vp);
	simple_lock(&consvp_lock);
	if (vp == consvp) {
		vp->v_usecount++;
		simple_unlock(&consvp_lock);
		VN_UNLOCK(vp);
		VOP_WRITE(vp, uio, IO_APPEND, NOCRED, error);
		VUNREF(vp);
	} else {
		simple_unlock(&consvp_lock);
		VN_UNLOCK(vp);
		error = ENXIO;
	}
	return (error);
}
