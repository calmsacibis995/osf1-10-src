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
static char	*sccsid = "@(#)$RCSfile: s5fs_vnops.c,v $ $Revision: 4.2.5.7 $ (DEC) $Date: 1992/12/11 16:52:04 $";
#endif 
/*
 * (c) Copyright 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * History:
 *
 *	August 20, 1991: vipul patel
 *		OSF/1 Release 1.0.1 bug fixes.
 *		several fixes in s5fs_link(), s5fs_remove() and
 *		s5fs_rename(). inode lock fixes.
 *
 * 	ADK:
 *		original OSF/1 Release 1.0
 */

#include <sysv_fs.h>
#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#include <sys/secpolicy.h>
#endif
#include <sys/types.h>
#include <s5fs/s5param.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/fcntl.h>
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
#include <s5fs/filsys.h>
#include <s5fs/s5inode.h>
#include <s5fs/s5ino.h>
#include <s5fs/s5dir.h>
#include <sys/syslimits.h>
#include <vm/vm_page.h>

extern int chown_restricted;
/*
 * Global vfs data structures for s5fs
 */

int	s5fs_lookup(),
	s5fs_create(),
	s5fs_mknod(),
	s5fs_open(),
	s5fs_close(),
	s5fs_access(),
	s5fs_getattr(),
	s5fs_setattr(),
	s5fs_read(),
	s5fs_write(),
	s5fs_ioctl(),
	seltrue(),
	s5fs_mmap(),
	s5fs_fsync(),
	s5fs_seek(),
	s5fs_remove(),
	s5fs_link(),
	s5fs_rename(),
	s5fs_mkdir(),
	s5fs_rmdir(),
	s5fs_symlink(),
	s5fs_readdir(),
	s5fs_readlink(),
	s5fs_abortop(),
	s5fs_inactive(),
	s5fsspec_reclaim(),
	s5fs_reclaim(),
	s5fs_bmap(),
	s5fs_strategy(),
	s5fs_print(),
	s5fs_page_read(),
	s5fs_page_write(),
	s5fs_getpage(),
	s5fs_putpage(),
	s5fs_swap(),
	s5fs_bread(),
	s5fs_brelse(),
	s5fs_lockctl(),
	s5fs_setvlocks(),
	s5fs_syncdata();

struct vnodeops s5fs_vnodeops = {
	s5fs_lookup,		/* lookup */
	s5fs_create,		/* create */
	s5fs_mknod,		/* mknod */
	s5fs_open,		/* open */
	s5fs_close,		/* close */
	s5fs_access,		/* access */
	s5fs_getattr,		/* getattr */
	s5fs_setattr,		/* setattr */
	s5fs_read,		/* read */
	s5fs_write,		/* write */
	s5fs_ioctl,		/* ioctl */
	seltrue,		/* select */
	s5fs_mmap,		/* mmap */
	s5fs_fsync,		/* fsync */
	s5fs_seek,		/* seek */
	s5fs_remove,		/* remove */
	s5fs_link,		/* link */
	s5fs_rename,		/* rename */
	s5fs_mkdir,		/* mkdir */
	s5fs_rmdir,		/* rmdir */
	s5fs_symlink,		/* symlink */
	s5fs_readdir,		/* readdir */
	s5fs_readlink,		/* readlink */
	s5fs_abortop,		/* abortop */
	s5fs_inactive,		/* inactive */
	s5fs_reclaim,		/* reclaim */
	s5fs_bmap,		/* bmap */
	s5fs_strategy,		/* strategy */
	s5fs_print,		/* print */
	s5fs_page_read,		/* page_read */
	s5fs_page_write,		/* page_write */
	s5fs_getpage,		/* get page */
	s5fs_putpage,		/* put page */
	s5fs_swap,		/* swap */
	s5fs_bread,		/* buffer read */
	s5fs_brelse,		/* buffer release */
	s5fs_lockctl,           /* file locking */
	s5fs_syncdata,		/* fsync byte range */
};

int	spec_lookup(),
	spec_open(),
	s5fsspec_read(),
	s5fsspec_write(),
	spec_strategy(),
	spec_bmap(),
	spec_ioctl(),
	spec_select(),
	spec_seek(),
	s5fsspec_close(),
	spec_lockctl(),
	spec_badop(),
	spec_nullop();

struct vnodeops spec_s5inodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	spec_open,		/* open */
	s5fsspec_close,		/* close */
	s5fs_access,		/* access */
	s5fs_getattr,		/* getattr */
	s5fs_setattr,		/* setattr */
	s5fsspec_read,		/* read */
	s5fsspec_write,		/* write */
	spec_ioctl,		/* ioctl */
	spec_select,		/* select */
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
	s5fs_inactive,		/* inactive */
	s5fsspec_reclaim,	/* reclaim */
	spec_bmap,		/* bmap */
	spec_strategy,		/* strategy */
	s5fs_print,		/* print */
	spec_badop,		/* page_read */
	spec_badop,		/* page_write */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	spec_badop,		/* swap */
	s5fs_bread,		/* buffer read */
	s5fs_brelse,		/* buffer release */
	spec_lockctl,           /* file locking */
};

int	fifo_open(),
	s5fsfifo_close(),
	s5fsfifo_read(),
	s5fsfifo_write(),
	fifo_ioctl(),
	s5fsfifo_getattr(),
	fifo_select();

struct vnodeops s5fifo_inodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	fifo_open,		/* open */
	s5fsfifo_close,		/* close */
	s5fs_access,		/* access */
	s5fsfifo_getattr,	/* getattr */
	s5fs_setattr,		/* setattr */
	s5fsfifo_read,		/* read */
	s5fsfifo_write,		/* write */
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
	s5fs_inactive,		/* inactive */
	s5fs_reclaim,		/* reclaim */
	spec_bmap,		/* bmap */
	spec_badop,		/* strategy */
	s5fs_print,		/* print */
	spec_badop,		/* page_read */
	spec_badop,		/* page_write */
	spec_badop,		/* getpage */
	spec_badop,		/* putpage */
	spec_badop,		/* swap */
	s5fs_bread,		/* buffer read */
	s5fs_brelse,		/* buffer release */
	spec_lockctl,           /* file locking */
};

enum vtype s5iftovt_tab[16] = {
	VNON, VFIFO, VCHR, VNON, VDIR, VNON, VBLK, VNON,
	VREG, VNON, VLNK, VNON, VSOCK, VNON, VNON, VBAD
};
int	s5vttoif_tab[9] = {
	0, S5IFREG, S5IFDIR, S5IFBLK, S5IFCHR, S5IFLNK, S5IFSOCK, S5IFIFO,
	S5IFMT,
};

/*
 * Create a regular file and returns its associated vnode pointer vp. 
 */
s5fs_create(ndp, vap)
	register struct nameidata *ndp;
	register struct vattr *vap;
{
	struct s5inode *ip;
	int mode, error = 0;

	mode = S5VTTOIF(vap->va_type) | vap->va_mode;
	if (error = s5maknode(mode, ndp, &ip))
		return (error);
	ndp->ni_vp = S5ITOV(ip);
	if (ndp->ni_vp->v_type == VSOCK)
		ndp->ni_vp->v_socket = (struct socket *) vap->va_socket;
	return (0);
}

/*
 * Open called.
 *
 * Nothing to do.
 */
/* ARGSUSED */
s5fs_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{

	return (0);
}

/*
 * Close  called
 *
 * Update the times on the inode.
 */
/* ARGSUSED */
s5fs_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);

	if (vp->v_usecount > 1 && !(s5LOCK_LOCKED(ip)))
		s5iupdat(ip, &time, &time);
	 return (0);
 }

/*
 * Vnode op for reading.
 */

/* ARGSUSED */
s5fs_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);
	register  int bsize;
	struct buf *bp;
	daddr_t lbn, rablock;
	int  diff, error = 0;
	long n, on, type;

	if (uio->uio_rw != UIO_READ)
		panic("s5fs_read mode");
	type = ip->i_mode & S5IFMT;
	if (type != S5IFDIR && type != S5IFREG && type != S5IFLNK)
		panic("s5fs_read type");
	if (uio->uio_resid == 0)
		return (0);
	if (uio->uio_offset < 0)
		return (EINVAL);
	ip->i_flag |= S5IACC;
	/*
	 * Get the block size, bsize, of this file system.
	 * The buffer size should always be the same as bsize. 
	 */
	s5ILOCK(ip);
	bsize = FsBSIZE(ip->i_s5fs);
	do {
		lbn = FsBNO(bsize, uio->uio_offset);
		on =  FsBOFF(bsize, uio->uio_offset); 
		n = MIN((unsigned)(bsize - on), uio->uio_resid);
		diff = ip->i_size - uio->uio_offset;
		if (diff <= 0) {
			s5IUNLOCK(ip);
			return (0);
		}
		if (diff < n)
			n = diff;
		rablock = lbn + 1;
 		if (vp->v_lastr + 1 == lbn &&
  		    FsLTOP(bsize, rablock) < ip->i_size)
 			error = breada(vp, lbn, bsize, rablock, 
						bsize, NOCRED, &bp);
		else
			error = bread(vp, lbn, bsize, NOCRED, &bp);
		vp->v_lastr = lbn;
		n = MIN(n, bsize - bp->b_resid);
		if (error) {
			brelse(bp);
			s5IUNLOCK(ip);
			return (error);
		}
		error = uiomove(bp->b_un.b_addr + on, (int)n, uio);
		if (n + on == bsize || uio->uio_offset == ip->i_size)
			bp->b_flags |= B_AGE;
		brelse(bp);
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
        s5IUNLOCK(ip);
	return (error);
}


/*
 * Vnode op for writing.
 */
s5fs_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);
	struct buf *bp;
	daddr_t lbn, bn;
	int n, on, flags;
	unsigned efbig = 0;
	int bsize, resid, error = 0;

	if (uio->uio_rw != UIO_WRITE)
		panic("s5fs_write mode");
	switch (vp->v_type) {
	case VREG:
		if (ioflag & IO_APPEND)
			uio->uio_offset = ip->i_size;
		/* fall through */
	case VLNK:
		break;

	case VDIR:
		if ((ioflag & IO_SYNC) == 0)
			panic("s5fs_write nonsync dir write");
		break;

	default:
		panic("s5fs_write type");
	}
	if (uio->uio_offset < 0)
		return (EINVAL);
	if (uio->uio_resid == 0)
		return (0);
	if (vp->v_type == VREG) {
		register unsigned long file_limit;

		file_limit = u.u_rlimit[RLIMIT_FSIZE].rlim_cur;
		if (uio->uio_offset >= file_limit) {
			psignal(u.u_procp, SIGXFSZ);
			return (EFBIG);
		}
		efbig = uio->uio_offset + uio->uio_resid;
		if (efbig > file_limit) {
			efbig -= file_limit;
			uio->uio_resid -= efbig;
		}
		else
			efbig = 0;
	}
	resid = uio->uio_resid;
	flags = 0;
	if (ioflag & IO_SYNC)
		flags = B_SYNC;
	s5ILOCK(ip);
	bsize = FsBSIZE(ip->i_s5fs);
	do {
		lbn = FsBNO(bsize, uio->uio_offset);
		on =  FsBOFF(bsize, uio->uio_offset); 
		n = MIN((unsigned)(bsize - on), uio->uio_resid);

		if (n < bsize)
			flags |= B_CLRBUF;
		else
			flags &= ~B_CLRBUF;
		if (error = s5bmap(ip, lbn, &bn, flags, B_WRITE))
			break;
		if (n == bsize)
			bp = getblk(vp, lbn, bsize);
		else
			bread(vp, lbn, bsize, NOCRED, &bp);

		if (uio->uio_offset + n > ip->i_size)
			ip->i_size = uio->uio_offset + n;

/**	INODE_UNCACHE
		if ((vp->v_vm_info != VM_INFO_NULL) &&
		    (vp->v_vm_info->pager != MEMORY_OBJECT_NULL))
			inode_uncache(vp);
**/
		n = MIN(n, bsize - bp->b_resid);

		error = uiomove(bp->b_un.b_addr + on, n, uio);
		if (ioflag & IO_SYNC)
			(void) bwrite(bp);
		else if (n + on == bsize) {
			bp->b_flags |= B_AGE;
			bawrite(bp);
		} else
			bdwrite(bp, bp->b_vp);
		ip->i_flag |= S5IUPD|S5ICHG;
		if (cred->cr_uid != 0)
			ip->i_mode &= ~(S5ISUID|S5ISGID);
	} while (error == 0 && uio->uio_resid > 0 && n != 0);
	if (efbig > 0)
		 uio->uio_resid += efbig;
/*
 * This code is from UFS.  We cannot currently support this because
 * the s5fs does not currently support truncation of files to any
 * length other than 0.  This also means that it is not possible to
 * use truncate to grow a s5fs file.
 *
 *	if (error && (ioflag & IO_UNIT)) {
 *		u_long osize;
 *		(void) itrunc(ip, osize, ioflag & IO_SYNC);
 *		uio->uio_offset -= resid - uio->uio_resid;
 *		uio->uio_resid = resid;
 *	}
 */
	s5IUNLOCK(ip);
	return (error);
}
/*
 * Mknod vnode call
 */
/* ARGSUSED */
s5fs_mknod(ndp, vap, cred)
	struct nameidata *ndp;
	struct ucred *cred;
	struct vattr *vap;
{
	struct s5inode *ip;
	struct vnode *vp;
	int mode, error = 0;


	if (vap->va_rdev != VNOVAL) {
		if (vap->va_type != VBLK && vap->va_type != VCHR)
			return (EINVAL);
	}
	mode = S5VTTOIF(vap->va_type) | vap->va_mode;
	if (error = s5maknode(mode, ndp, &ip))
		return (error);
	ip->i_flag |= S5IACC|S5IUPD|S5ICHG;
	if (vap->va_rdev != VNOVAL) {
		ip->i_rdev = vap->va_rdev;
	}
	/*
	 * Eliminate the vnode, and let it get set up by s5iget()
	 * when next looked up.
	 */
	vp = S5ITOV(ip);
	vp->v_type = VNON;
	VN_LOCK(vp);
	vgone(vp, VX_NOSLEEP, 0);
	VN_UNLOCK(vp);
	vrele(vp);
	return (0);
}

s5fs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{

	return (s5iaccess(S5VTOI(vp), mode, cred));
}


/*
 * link vnode call
 */
s5fs_link(vp, ndp)
	register struct vnode *vp;
	register struct nameidata *ndp;
{
	register struct s5inode *ip = S5VTOI(vp);
	int error;

	if ((ushort_t) ip->i_nlink >= LINK_MAX) 
		return( EMLINK);
	ip->i_nlink++;
	ip->i_flag |= S5ICHG|S5ISYN;
	error = s5iupdat(ip, &time, &time);
	if (!error) {
	        s5ILOCK(S5VTOI(ndp->ni_dvp));
		error = s5wdir(ip, ndp, 0);
	}
	if (error) {
		if (--ip->i_nlink == 0)
			ip->i_gen = get_nextgen();
		ip->i_flag |= S5ICHG;
	}
	return (error);
}


/*
 * Mkdir system call
 */
s5fs_mkdir(ndp, vap)
	struct nameidata *ndp;
	struct vattr *vap;
{
	struct s5inode *ip, *dp;
	int dmode;
	int error = 0;

	dmode = vap->va_mode&0777;
	dmode |= S5IFDIR;
	error = s5maknode(dmode, ndp, &ip);
	return (error);

}

/*
 * Vnode op for read and write
 */
s5fs_readdir(vp, uio, cred, eofflagp)
	struct vnode *vp;
	register struct uio *uio;
	struct ucred *cred;
        int *eofflagp;
{
	register struct s5inode *ip = S5VTOI(vp);
	int i, bsize, count, entries, lost, error;
	struct s5direct *r;
	struct gpdirect *d;
	struct uio tuio;
	struct iovec tiov;

	/*
	 * Get the block size, bsize, of this file system.
	 */
	bsize = FsBSIZE(ip->i_s5fs);
	count = uio->uio_resid;
	count &= ~(bsize - 1);
	lost = uio->uio_resid - count;
	if (count < bsize || (uio->uio_offset & (s5DIRECTSIZE -1)))
		return (EINVAL);
/* 
 * Need to repackage sys V directory into a file system independent format:
 *   1) Setup struct tuio so that s5fs_read reads one bsize block into the
 *      readdirbuf buffer which was allocated in s5fs_init.
 *   2) Copy each entry in block to the buffer dirrenttemp which was also
 *      allocated in s5fs_init.  Transform data as we go along.  If we can
 *      not fit all of the new entries in the buffer, scale the number of
 *      entries back to the largest size that will fit.
 *   3) Call uiomove to copy file system independent entries out to user.
 *   4) Reset offset in struct uio so user's file pointer will be correct.
 */
	tiov.iov_base = (caddr_t)readdirbuf;
	tiov.iov_len = bsize;
	tuio.uio_iov = &tiov;
	tuio.uio_iovcnt = 1;
	tuio.uio_offset = uio->uio_offset;
	tuio.uio_resid = bsize;
	tuio.uio_segflg = UIO_SYSSPACE;
	tuio.uio_rw = UIO_READ;

	error = s5fs_read(vp, &tuio, 0, cred);
	entries = ((bsize - tuio.uio_resid) / s5DIRECTSIZE);
	if ((entries * gpDIRECTSIZE) > bsize) 
		entries = ((entries*s5DIRECTSIZE) / gpDIRECTSIZE);
	for (i = 0, r = readdirbuf, d = gpdirect; i < entries;
	         r++, d++, i++) {

		d->d_ino = (u_long)r->d_ino;
		d->d_reclen = gpDIRECTSIZE;
		d->d_namlen = s5DIRSIZ;
		d->d_padding = 0;
		bcopy(r->d_name, d->d_name, s5DIRSIZ);
	}

	error = uiomove(gpdirect, entries * gpDIRECTSIZE, uio);
	if (!error) 
		uio->uio_offset -= entries * (gpDIRECTSIZE - s5DIRECTSIZE);
	uio->uio_resid += lost;
	if ((ip->i_size - uio->uio_offset) <= 0)
		*eofflagp = 1;
	else
		*eofflagp = 0;
	return (error);
}


/*
 * s5fs remove
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
s5fs_remove(ndp)
	struct nameidata *ndp;
{
	register struct s5inode *ip, *dirip;
	register struct vnode *vp;
	int error;

	vp = ndp->ni_vp;
	ASSERT(vp->v_type != VDIR);
	ip = S5VTOI(ndp->ni_vp);
	dirip = S5VTOI(ndp->ni_dvp);
	if ((ip->i_mode&S5IFMT) == S5IFDIR)
		return(EISDIR);

	cache_purge(S5ITOV(ip));
	s5ILOCK(dirip);
	error = s5dirremove(ndp);
	s5IUNLOCK(dirip);
	if (!error) {
		if (--ip->i_nlink == 0) 
			ip->i_gen = get_nextgen();
		ip->i_flag |= S5ICHG;
	}
/**	INODE_UNCACHE
	if ((ip->i_nlink == 0) && (ndp->ni_vp->v_vm_info != VM_INFO_NULL) &&
	    (vp->v_vm_info->pager != MEMORY_OBJECT_NULL))
		inode_uncache(vp);
**/
	s5iput(ip);
	s5iput(dirip);
	return (error);
}

/*
 * Get access to bmap
 */

s5fs_bmap(vp, lbn, vpp, bnp)
	struct vnode *vp;
	daddr_t lbn;
	struct vnode **vpp;
	daddr_t *bnp;
{
	struct s5inode *ip = S5VTOI(vp);

	if (vpp != NULL)
		*vpp = ip->i_devvp;
	if (bnp == NULL)
		return (0);
	return (s5bmap(ip, lbn, bnp, 0, B_READ));
}

/*
 * Just call the device strategy routine
 */
s5fs_strategy(bp)
	register struct buf *bp;
{
	register struct s5inode *ip = S5VTOI(bp->b_vp);
	struct vnode *vp;
	int error;

	if (bp->b_vp->v_type == VBLK || bp->b_vp->v_type == VCHR)
		panic("s5fs_strategy: spec");
	if (bp->b_blkno == bp->b_lblkno) {
		if (error = s5bmap(ip, bp->b_lblkno, &bp->b_blkno, 0, B_READ))
			return (error);
 		if ((long)bp->b_blkno == -1)
  			clrbuf(bp);
  	}
 	if ((long)bp->b_blkno == -1) {
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
s5fs_print(vp)
	struct vnode *vp;
{
	register struct s5inode *ip = S5VTOI(vp);

	printf("S5FS: ino %d, mode %o, links %d, size %d, on dev %d, %d%s\n", 
	       ip->i_number, ip->i_mode, ip->i_nlink, ip->i_size,
	       major(ip->i_dev), minor(ip->i_dev),
	       (s5LOCK_LOCKED(ip)) ? " (LOCKED)" : "");
}

/*
 * Change the mode on a file.
 * Inode must be locked before calling.
 */
s5chmod1(vp, mode, cred)
	register struct vnode *vp;
	register int mode;
	struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);
	int error;

#if SEC_BASE
	if (!sec_owner(ip->i_uid, ip->i_uid))
		return EPERM;
#else
	if (cred->cr_uid != ip->i_uid &&
	    (error = suser(cred, &u.u_acflag)))
		return (error);
#endif
	ip->i_mode &= ~07777;
	if (cred->cr_uid) {
		if (vp->v_type != VDIR)
			mode &= ~S5ISVTX;
		if (!groupmember(ip->i_gid, cred))
			mode &= ~S5ISGID;
	}
	ip->i_mode |= mode & 07777;
	ip->i_flag |= S5ICHG;
	return (0);
}

/*
 * Perform chown operation on inode ip;
 * inode must be locked prior to call.
 */
s5chown1(vp, uid, gid, cred)
	register struct vnode *vp;
	uid_t uid;
	gid_t gid;
	struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);
	int error;

	if (uid == (uid_t) VNOVAL)
		uid = ip->i_uid;
	if (gid == (gid_t) VNOVAL)
		gid = ip->i_gid;
	/*
	 * If we don't own the file, are trying to change the owner
	 * of the file, or are not a member of the target group,
	 * the caller must be superuser or the call fails.
	 */
#if SEC_BASE
	if (!sec_owner(ip->i_uid, ip->i_uid) ||
	    !sec_owner_change_permitted(ip->i_uid, ip->i_gid, uid, gid))
		return EPERM;
#else
	if (chown_restricted != -1) {
		/* As per POSIX */
		if ((cred->cr_uid != ip->i_uid || uid != ip->i_uid ||
		    !groupmember((gid_t)gid, cred)) &&
		    (error = suser(cred, &u.u_acflag)))
		return (error);
	} else {
		/* As per SVID-3 */
		if ( ( cred->cr_uid != ip->i_uid ) &&
			(error = suser(cred, &u.u_acflag))) {
				return (error);
		}
	}
#endif
	if (ip->i_uid != uid && cred->cr_uid != 0)
		ip->i_mode &= ~S5ISUID;
	if (ip->i_gid != gid && cred->cr_uid != 0)
		ip->i_mode &= ~S5ISGID;
	ip->i_uid = uid;
	ip->i_gid = gid;
	ip->i_flag |= S5ICHG;
	return (0);
}


/* ARGSUSED */
s5fs_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{

	return (ENOTTY);
}


/*
 * Synch an open file.
 */
/* ARGSUSED */
s5fs_fsync(vp, fflags, cred, waitfor)
	struct vnode *vp;
	int fflags;
	struct ucred *cred;
	int waitfor;
{
	struct s5inode *ip = S5VTOI(vp);

	if (fflags&FWRITE)
		ip->i_flag |= S5ICHG;
	vflushbuf(vp, waitfor == MNT_WAIT ? B_SYNC : 0);
	if (waitfor)
		ip->i_flag |= S5ISYN;
	return (s5iupdat(ip, &time, &time));
}

/*
 * Synch a range of an open file.
 */
/* ARGSUSED */
s5fs_syncdata(vp, fflags, start, length, cred)
	struct vnode *vp;
	int fflags;
	vm_offset_t start;
	vm_size_t length;
	struct ucred *cred;
{
	return(s5fs_fsync(vp, fflags, cred, MNT_WAIT));
}

/*
 * Seek on a file
 *
 * Nothing to do, so just return.
 */
/* ARGSUSED */
s5fs_seek(vp, oldoff, newoff, cred)
	struct vnode *vp;
	off_t oldoff, newoff;
	struct ucred *cred;
{
	if ((int) newoff < 0)
		return(EINVAL);
	else
		return(0);
}



/*
 * This is the common code to make a new file, a new directory and a new 
 * node
 * NOTE: s5ialloc() would not synchronously write the new inode, it is the
 * 	 caller that does so (by calling s5iupdat) after updating  the 
 * 	 new inode attributes appropriately. 
 *
 */
s5maknode(mode, ndp, ipp)
register mode;
register struct nameidata *ndp;
struct s5inode **ipp;
{
	register struct s5inode *ip;
	register struct s5inode *dirip = S5VTOI(ndp->ni_dvp);
	struct s5inode *nip;
	struct uio uiop;
	struct iovec	iov;
	struct s5dirtemp  ndirent;
	int error = 0;
	
	ndirent.dot_ino =  (s5ino_t)0;
	ndirent.dotdot_ino = (s5ino_t)0;
	ndirent.dot_name[0] = ndirent.dotdot_name[0] = '.';
	ndirent.dotdot_name[1] = '.';
	ndirent.dot_name[1] = ndirent.dotdot_name[2] = '\0';

	*ipp = 0;
	if ((mode&S5IFMT) == 0)
		mode |= S5IFREG;
	if ((mode&S5IFMT == S5IFDIR) && ((ushort_t) dirip->i_nlink >= LINK_MAX))
		error = EMLINK;
	else
		error = s5ialloc(dirip, mode, &nip);

	if (error) {
		s5iput(dirip);
		return(error);
	}
	ip = nip;
	s5ILOCK(ip);
	s5ILOCK(dirip);
	ip->i_flag |= S5IACC|S5IUPD|S5ICHG;
	ip->i_mode = mode;
	S5ITOV(ip)->v_type = S5IFTOVT(mode);	/* Rest init'd in iget() */
	if ((mode&S5IFMT) == S5IFDIR)
		ip->i_nlink = 2;
	else
		ip->i_nlink = 1;
	ip->i_uid = ndp->ni_cred->cr_uid;
	ip->i_gid = dirip->i_gid;

#if SEC_BASE
	if ((ip->i_mode & S5ISGID) && !groupmember(ip->i_gid, ndp->ni_cred) &&
	    !privileged(SEC_SETPROCIDENT, 0))
#else
	if ((ip->i_mode & S5ISGID) && !groupmember(ip->i_gid, ndp->ni_cred) &&
	    suser(ndp->ni_cred, NULL))
#endif
		ip->i_mode &= ~S5ISGID;

	/* 
	 *  Adding a new entry to the current directory depending on ..
         */
	if ((mode&S5IFMT) == S5IFDIR) {
		/* 
		 * make a new directory    
		 * Write the "." and ".." to the new directory 
 		 */
		ndirent.dotdot_ino = dirip->i_number;
		ndirent.dot_ino = ip->i_number;
		iov.iov_base = (caddr_t)&ndirent;
		iov.iov_len = 2 * sizeof(struct s5direct); 
		uiop.uio_iov = &iov;
		uiop.uio_iovcnt = 1;
		uiop.uio_offset = 0;
		uiop.uio_resid = iov.iov_len;
		uiop.uio_rw = UIO_WRITE;
		uiop.uio_segflg = UIO_SYSSPACE;

		s5_SET_RECURSIVE(ip);
		error = s5fs_write(S5ITOV(ip), &uiop, IO_SYNC, ndp->ni_cred);
		s5_CLEAR_RECURSIVE(ip);
		if (error) {
			goto bad;
		}	

		dirip->i_nlink++;
		dirip->i_flag |= S5ICHG;
		if (s5iupdat(dirip, &time, &time)) {
		        goto bad;
		}

	}
	ip->i_flag |= S5ISYN;

	if (error = s5iupdat(ip, &time, &time)) {
		goto bad;
	}

	/* 
	 * Write the new inode number and new entry name to the directory.
         */
	error = s5wdir(ip, ndp, 0);
	dirip = (struct s5inode *) NULL;
bad:
	s5IUNLOCK(ip);
	if (dirip) {
		s5IUNLOCK(dirip);
		s5iput(dirip);
	}
        if (error) {
	        ip->i_nlink = 0;
		ip->i_flag |= S5ICHG;
         	s5iput(ip);
	}
	else
	        ndp->ni_vp = S5ITOV(ip);

	*ipp = nip;
	return(error);
}
	

/*
 * Write a directory entry after a call to namei, using the parameters
 * which it left in nameidata.  The argument ip is the s5inode which the
 * new directory entry will refer to.  The nameidata field ndp->ni_dvp
 * is a pointer to the directory to be written, which was left referenced 
 * by namei.  This routine calls releases the reference on the directory.  
 * Remaining parameters (ndp->ni_offset, ndp->ni_count) indicate how the 
 * space for the new entry is to be obtained.
 *
 * NOTE: Writing a new directory entry to the parent directory is now done
 *	 synchronously because otherwise we will panic in s5fs_write.  If 
 *       this proves to be a performance problem, it could be done async.
 */
s5wdir(ip, ndp, newparent)
struct s5inode *ip;
struct nameidata *ndp;
int newparent;
{
	struct s5direct tdirect;
	register struct s5inode *dirip = S5VTOI(ndp->ni_dvp);
	int i = 0;
        int error = 0;

	ASSERT(s5ILOCK_HOLDER(dirip));
	if (error = s5checkdir(ndp, ADD|DEL))
		goto out;
	tdirect.d_ino = ip->i_number;
	while ((tdirect.d_name[i] = ndp->ni_gpdent.d_name[i]) != '\0'
	        && ++i<s5DIRSIZ)
		; 
	ndp->ni_count = sizeof(struct s5direct);
	ndp->ni_base = (caddr_t)&tdirect;		
	ndp->ni_resid = sizeof(struct s5direct);
	ndp->ni_iovcnt = 1;
	ndp->ni_uioseg = UIO_SYSSPACE;
	ndp->ni_rw = UIO_WRITE;

	s5_SET_RECURSIVE(dirip);
	error = s5fs_write(ndp->ni_dvp, &ndp->ni_uio, IO_SYNC, ndp->ni_cred);
	s5_CLEAR_RECURSIVE(dirip);
out:
	if (error && newparent && (ip->i_mode == S5IFDIR)) {
		dirip->i_nlink--;
		dirip->i_flag |= S5ICHG;
	}
	dirip->i_dirstamp++;
	s5IUNLOCK(dirip);
	s5iput(dirip);
	return(error);
}


/* ARGSUSED */
s5fs_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	struct s5inode *ip = S5VTOI(vp);
	struct s5dinode *dp;
	struct buf *bp;
	int bsize;
	int error = 0;
	
	/* Read the disk inode */

	bsize = FsBSIZE(ip->i_s5fs);
	error = bread(ip->i_devvp, FsLTOP(bsize, FsITOD(bsize, ip->i_number)), 
                      bsize, NOCRED, &bp);
	if (error) {
		brelse(bp);
		s5iput(ip);
		return(error);
	}

	dp = bp->b_un.b_s5dino;
	dp +=  FsITOO(bsize, ip->i_number);

	if (ip->i_flag&(S5IUPD|S5IACC|S5ICHG)) {
		if (ip->i_flag&S5IACC)
			dp->di_atime = time.tv_sec;
		if (ip->i_flag&S5IUPD)
			dp->di_mtime = time.tv_sec;
		if (ip->i_flag&S5ICHG)
			dp->di_ctime = time.tv_sec;
		ip->i_flag &= ~(S5IACC|S5IUPD|S5ICHG);
	}
	/*
	 * Copy from inode table
	 */
	vap->va_fsid = ip->i_dev;
	vap->va_fileid = ip->i_number;
	vap->va_mode = ip->i_mode & ~S5IFMT;
	vap->va_nlink = ip->i_nlink;
	vap->va_uid = (uid_t)ip->i_uid;
	vap->va_gid = (gid_t)ip->i_gid;
	vap->va_rdev = (dev_t)ip->i_rdev;
	vap->va_size = ip->i_size;	
#if !__alpha
	vap->va_size_rsv = 0;
#endif
	vap->va_atime.tv_sec = dp->di_atime;
	vap->va_atime.tv_usec = 0;
	vap->va_mtime.tv_sec = dp->di_mtime;
	vap->va_mtime.tv_usec = 0;
	vap->va_ctime.tv_sec = dp->di_ctime;
	vap->va_ctime.tv_usec = 0;
	vap->va_flags = (u_long)ip->i_flag;

	brelse(bp);

	/* this doesn't belong here */
	if (vp->v_type == VBLK)
		vap->va_blocksize = BLKDEV_IOSIZE;
	else if (vp->v_type == VCHR)
		vap->va_blocksize = MAXBSIZE;
	else 	
		vap->va_blocksize = bsize;
	vap->va_bytes = ip->i_size;
#if !__alpha
	vap->va_bytes_rsv = -1;
#endif
	vap->va_type = vp->v_type;
	return (0);
}

/*
 * Set attribute vnode op. called from several syscalls
 */
s5fs_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	register struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);
	int error = 0;

	/*
	 * Check for unsettable attributes.
	 */
	if ((vap->va_type != VNON) || (vap->va_nlink != VNOVAL) ||
	    (vap->va_fsid != VNOVAL) || (vap->va_fileid != VNOVAL) ||
	    (vap->va_blocksize != VNOVAL) || (vap->va_rdev != VNOVAL) ||
	    ((int)vap->va_bytes != VNOVAL) || (vap->va_gen != VNOVAL)) {
		return (EINVAL);
	}
	/*
	 * Go through the fields and update iff not VNOVAL.
	 */
	if (vap->va_uid != (uid_t) VNOVAL || vap->va_gid != (gid_t) VNOVAL)
		if (error = s5chown1(vp, vap->va_uid, vap->va_gid, cred))
			return (error);
	if (vap->va_size != VNOVAL) {
		if (vp->v_type == VDIR)
			return (EISDIR);
		/*
		 * XXX
		 * For now, reject attempts to truncate to non-zero length.
		 * We would like to be able to truncate
		 * to any size smaller than the current size; truncation
		 * to larger sizes is done by VOP_WRITE.
		 */
		if (vap->va_size == 0) {
			s5ILOCK(ip);
		        s5itrunc(ip);
			s5IUNLOCK(ip);
		} else if (vap->va_size != ip->i_size) 
		        return (EINVAL);	
		ip->i_mode &= ~(S5ISUID|S5ISGID);
	}
	if (vap->va_atime.tv_sec != VNOVAL || vap->va_mtime.tv_sec != VNOVAL) {
#if SEC_BASE
		if (!sec_owner(ip->i_uid, ip->i_uid))
			return EPERM;
#else
		if (cred->cr_uid != ip->i_uid &&
		    (error = suser(cred, &u.u_acflag)))
			return (error);
#endif
		if (vap->va_atime.tv_sec != VNOVAL)
			ip->i_flag |= S5IACC;
		if (vap->va_mtime.tv_sec != VNOVAL)
			ip->i_flag |= S5IUPD;
		ip->i_flag |= S5ICHG|S5ISYN;
		if (error = s5iupdat(ip, &vap->va_atime, &vap->va_mtime))
			return (error);
	}
	if (vap->va_mode != (u_short)VNOVAL)
		error = s5chmod1(vp, (int)vap->va_mode, cred);
	if (vap->va_flags != VNOVAL) {
		/*
		 * Allow setting of only high-order 16 bits of flag
		 * by owner.  Superuser can change all of them.  The
		 * low 16 bits are reserved for system use.
		 * UFS uses bit 1 to indicate fast symbolic links.
		 */
#if SEC_BASE
		if (!sec_owner(ip->i_uid, ip->i_uid))
			return EPERM;
		if (privileged(SEC_FILESYS, 0))
#else
		if (cred->cr_uid != ip->i_uid &&
		    (error = suser(cred, &u.u_acflag)))
			return (error);
		if (cred->cr_uid == 0)
#endif
		{
			ip->i_flag = vap->va_flags;
		} else {
			ip->i_flag &= 0xffff;
			ip->i_flag |= (vap->va_flags & 0xffff0000);
		}
		ip->i_flag |= S5ICHG;
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
 *    target.  This also ensures the inode won't be deleted out
 *    from underneath us while we work (it may be truncated by
 *    a concurrent `trunc' or `open' for creation).
 * 2) Link source to destination.  If destination already exists,
 *    delete it first.
 * 3) Unlink source reference to inode if still around. If a
 *    directory was moved and the parent of the destination
 *    is different from the source, patch the ".." entry in the
 *    directory.
 */
s5fs_rename(fndp, tndp)
	register struct nameidata *fndp, *tndp;
{
	register struct s5inode *ip, *xp, *dp;
	int doingdirectory = 0, oldparent = 0, newparent = 0;
	int error = 0;
	int stripslash = 0;
	struct s5dirtemp dirbuf;

	dp = S5VTOI(fndp->ni_dvp);
	ip = S5VTOI(fndp->ni_vp);

	if ((ip->i_mode & S5IFMT) == S5IFDIR) {
	      register struct gpdirect *d = &fndp->ni_gpdent;

	      /*
	       * Avoid ".", ".." and aliases of "." for obvious reasons.
	       */

	      if ((d->d_namlen == 1 && d->d_name[0] == '.') || (dp == ip) ||
		  fndp->ni_isdotdot || (ip->i_flag & S5IRENAME)) {
			VOP_ABORTOP(tndp, error);
			vrele(tndp->ni_dvp);
			if (tndp->ni_vp)
				vrele(tndp->ni_vp);
			VOP_ABORTOP(fndp, error);
			vrele(fndp->ni_dvp);
			vrele(fndp->ni_vp);
			return (EINVAL);
	      }

	      ip->i_flag |= S5IRENAME;
	      oldparent = dp->i_number;
	      doingdirectory++;
	      stripslash = STRIPSLASH;
	}
	s5ILOCK(dp);
	s5ILOCK(ip);

	/*
	 * Check that the source has not been removed.
	 */
	if (error = s5checkdir(fndp, DEL)) {
		ip->i_flag &= ~S5IRENAME;
		s5IUNLOCK(dp);
		s5IUNLOCK(ip);
		VOP_ABORTOP(tndp, error);
		vrele(tndp->ni_dvp);
		if (tndp->ni_vp)
			vrele(tndp->ni_vp);
		VOP_ABORTOP(fndp, error);
		vrele(fndp->ni_dvp);
		vrele(fndp->ni_vp);
		return (error);
	}
	/*
	 * Don't need this anymore
	 */
	s5IUNLOCK(dp);
	s5iput(dp);
	/*
	 * 1) Bump link count while we're moving stuff around.  If we
	 *    crash somewhere before completing our work, the link count
	 *    may be wrong, but correctable.
	 */

	ip->i_nlink++;
	ip->i_flag |= S5ICHG|S5ISYN;
	error = s5iupdat(ip,&time, &time);
	s5IUNLOCK(ip);

	/* 
	 * When the target exists, both the directory 
	 * and target vnodes are returned referenced
	 * and need to be.
	 */
        
	dp = S5VTOI(tndp->ni_dvp);
	xp = NULL;
	if (tndp->ni_vp) {
	      xp = S5VTOI(tndp->ni_vp);
	      s5ILOCK(xp);
	}

	/*
	 * If ".." must be changed (i.e., the directory gets a new 
	 * parent) then the source directory must not be in the 
	 * directory heirarchy above the target, as this would 
	 * orphan everything below the source directory.  Also,
	 * the user must have write permission in the source
	 * so as to be able to change "..".  We must repeat the call to
	 * namei, as the parent directory is released by the call to
	 * s5checkpath().
	 * xp must be locked explicitly, since the lookup operation
	 * will not lock it, and s5iget() will return it referenced  
	 * unlocked.
	 * dp is not locked until after this loop.
	 */

	if (oldparent != dp->i_number)
	      newparent = dp->i_number;
	if (doingdirectory && newparent) {
	      if (error = s5iaccess(ip, S5IWRITE, tndp->ni_cred))
		    goto bad;
	      tndp->ni_nameiop = RENAME | WANTPARENT | NOCACHE | stripslash;
	      do {
		    dp = S5VTOI(tndp->ni_dvp);
		    if (xp != NULL) {
			  s5IUNLOCK(xp);
		          s5iput(xp);
		    }
		    if (error = s5checkpath(ip, dp, tndp->ni_cred))
		          goto out;
		    if (error = namei(tndp))
		          goto out;
		    xp = NULL;
			if (tndp->ni_vp) {
				xp = S5VTOI(tndp->ni_vp);
				s5ILOCK(xp);
			}
	      } while (dp != S5VTOI(tndp->ni_dvp));
	}
	s5ILOCK(dp);

	/* 2) If target doesn't exist, link the target to the source
	 *    and unlink the source.  Otherwise, rewrite the target
	 *    directory entry to reference the source inode and
	 *    expunge the original entry's existence.
	 */

	if (xp == NULL) {
	      if (dp->i_dev != ip->i_dev)
		    panic ("s5fs_rename: EXDEV");
	      /*
	       * Account for ".." in new directory.
	       * When source and destination have the same
	       * parent we don't fool with the link count.
	       */
	      if (doingdirectory && newparent) {
		    if ((ushort_t) dp->i_nlink >= LINK_MAX) {
			error = EMLINK;
			goto out;
		    }
		    dp->i_nlink++;
		    dp->i_flag |= S5ICHG;
		    error = s5iupdat(dp, &time, &time);
	      }
	      /*
	       * Note: s5wdir unlocks and s5iput's tndp->ni_dvp (dp).
	       *	It also will decrement the i_nlink count on
	       *	dp on error if the third parameter is true.
	       */
	      error = s5wdir(ip, tndp, ((doingdirectory && newparent) ? 1 : 0));
	      if (error)
	      	    goto out;
	} else {
	      if (xp->i_dev != dp->i_dev || xp->i_dev != ip->i_dev)
		    panic ("s5fs_rename: EXDEV 1");
              /*
               * Short circuit rename(foo,foo);
               */
              if (xp->i_number == ip->i_number)
		    panic ("s5fs_rename: same file");
              /*
	       * If the parent directory is "sticky", then the user must
	       * own the parent directory, or the destination of the rename,
	       * otherwise the destination may not be changed (except by
	       * root).  This implements append-only directories.
	       */
	       if ((dp->i_mode & S5ISVTX) && tndp->ni_cred->cr_uid != 0 &&
                   tndp->ni_cred->cr_uid != dp->i_uid &&
                   xp->i_uid != tndp->ni_cred->cr_uid) {
                          error = EPERM;
                          goto bad;
                }

                /* Target must be empty if a directory
                 * and have no links to it.
                 * Also, insure source and target are
                 * compatible (both directories, or both
                 * not directories).
                 */
                if ((xp->i_mode&S5IFMT) == S5IFDIR) {
                      if (xp->i_nlink > 2 ||
			  !s5dirempty(xp,dp->i_number, tndp->ni_cred)) {
                               error = ENOTEMPTY;
                               goto bad;
		       }
                      if (!doingdirectory) {
                           error = ENOTDIR;
                           goto bad;
                      }
                      cache_purge(S5ITOV(dp));
                } else if (doingdirectory) {
                      error = EISDIR;
                      goto bad;
                }
                if (error = s5dirrewrite(dp,ip,tndp))
                      goto bad;
		s5IUNLOCK(dp);
                vrele(S5ITOV(dp));
                /*
                 * Adjust the link count of the target to
		 * reflect the s5dirrewrite above.  If this is
		 * a directory it is empty and there are
		 * no links to it, so we can squash the inode and
		 * any space associated with it.  We disallowed
		 * renaming over top of a directory with links to
		 * it above, as the remaining link would point to
		 * a directory without "." or ".." entries.
		 */
		xp->i_nlink--;
	        if (doingdirectory) {
			if (--xp->i_nlink != 0)
			      panic("s5fs_rename: linked directory");
			error = s5itrunc(xp);
		}
		xp->i_flag |= S5ICHG;
		s5IUNLOCK(xp);
		s5iput(xp);
		xp = NULL;
	     }

	     /*
	      * 3) Unlink the source.
	      */

	      fndp->ni_nameiop = DELETE | WANTPARENT | stripslash;
	      (void)namei(fndp);
	      if (fndp->ni_vp != NULL) {
		      xp = S5VTOI(fndp->ni_vp);
		      dp = S5VTOI(fndp->ni_dvp);
		      s5ILOCK(xp);
		      s5ILOCK(dp);
	      } else {
		      /*
		       * If ni_vp is NULL, then source is already gone, and
		       * ni_dvp is undefined, and not referenced.
		       * Must fail check below (xp != ip) to clean up properly.
		       */
		      ASSERT(ip != (struct s5inode *)NULL);
		      ASSERT(error == 0);
		      xp = NULL;
		      dp = NULL;
	      }

	      /*
	       * Ensure that the directory entry still exists and has not
	       * changed while the new name has been entered.  If the source
	       * is a file then the entry may have been unlinked or renamed.
	       * In either case there is no further work to be done.  If
	       * the source is a directory then it cannot have been rmdir'ed;
	       * its link count of three would cause a rmdir to fail with
	       * ENOTEMPTY.  The S5IRENAME flag ensures that it cannot be
	       * moved by another rename.
	       */

	       if (xp != ip) {
		       if (doingdirectory)
			    panic ("s5fs_rename: lost dir entry");
	       } else {
		       /*
			* If the source is a directory with a
			* new parent, the link count of the old
			* parent directory must be decremented
			* and ".." set to point to the new parent.
			*/
		       if (doingdirectory && newparent) {
			       s5_SET_RECURSIVE(xp);
			       dp->i_nlink--;
			       dp->i_flag != S5ICHG;
			       error = vn_rdwr(UIO_READ, S5ITOV(xp),
				       (caddr_t)&dirbuf, sizeof(struct
				       s5dirtemp), (off_t)0, UIO_SYSSPACE,
				       IO_NODELOCKED, tndp->ni_cred, 
				       (int *)0);
			       if (error == 0) {
				       if (dirbuf.dotdot_name[0] != '.' ||
					   dirbuf.dotdot_name[1] != '.' ||
					   dirbuf.dotdot_name[2] != '\0') {
					       panic("s5fs_rename: mangled dir");
				       } else {
					       dirbuf.dotdot_ino = newparent;
					       (void)vn_rdwr(UIO_WRITE,
						  S5ITOV(xp), 
						  (caddr_t)&dirbuf,
						  sizeof(struct s5dirtemp),
						  (off_t) 0, UIO_SYSSPACE,
						  IO_NODELOCKED|IO_SYNC,
						  tndp->ni_cred, (int *)0);
					       cache_purge(S5ITOV(dp));
				       }
			       }
			       s5_CLEAR_RECURSIVE(xp);
		       }
		       error = s5dirremove (fndp);
		       if (! error) {
			       xp->i_nlink--;
			       xp->i_flag != S5ICHG;
		       }
		       xp->i_flag &= ~S5IRENAME;
	       }
	       if (dp) {
		   s5IUNLOCK(dp);
		   vrele(S5ITOV(dp));
		}
	       if (xp) {
		   s5IUNLOCK(xp);
		   vrele(S5ITOV(xp));
	       }
	       vrele(S5ITOV(ip));
	       return (error);
							     	     
bad: 
	       if (xp) {
		   s5IUNLOCK(xp);
		   s5iput(xp);
	       }
	       s5IUNLOCK(dp);
	       s5iput(dp);
out:
	       ip->i_nlink--;
	       ip->i_flag |= S5ICHG;
	       vrele(S5ITOV(ip));
	       return (error);
}

/*
 * Rmdir system call.
 */
s5fs_rmdir(ndp)
	register struct nameidata *ndp;
{
	register struct s5inode *ip, *dp;
	int error = 0;

        ip = S5VTOI(ndp->ni_vp);
	dp = S5VTOI(ndp->ni_dvp);
	/*
	 * No rmdir "." please.
	 */
	if (dp == ip) {
		s5iput(dp);
		s5iput(ip);
		return(EINVAL);
	}
	/* 
	 * Verify the directory is empty (and valid).
	 * (Rmdir ".." won't be valid since
	 * ".." will contain a reference to
	 * the current directory and thus be
	 * non-empty.)
	 */
	s5ILOCK(dp);
	s5ILOCK(ip);
	if (ip->i_nlink != 2 || !s5dirempty(ip, dp->i_number, ndp->ni_cred)){
		error = ENOTEMPTY;
		goto out;
	}
	/*
	 * We hold the parent directory locked until after we
	 * remove the directory from it.  We need it to remove
	 * the directory from the parent.
	 */
	error = s5dirremove(ndp);
	if (error) goto out;

	dp->i_nlink--;
	dp->i_flag |= S5ICHG;
	cache_purge(S5ITOV(dp));
	s5IUNLOCK(dp);
	s5iput(dp);
	ndp->ni_dvp = NULL;
	/*
	 * Truncate inode.  The only stuff left
	 * in the directory is "." and "..".  The
	 * "." reference is inconsequential since
	 * we're quashing it.  The ".." reference
	 * has already been adjusted above.  We've
	 * removed the "." refernece and the reference
	 * in the parent directory, but there may be
	 * other hard links so decrement by 2 and
	 * worry about them later.
	 */
	ip->i_nlink -=2;
	s5itrunc(ip);
	cache_purge(S5ITOV(ip));
	s5IUNLOCK(ip);
	s5iput(ip);
	return(error);
out:
	s5IUNLOCK(ip);
	s5IUNLOCK(dp);
	s5iput(dp);
	s5iput(ip);
	return(error);
}

/*
 * symlink -- make a symbolic link
 */
s5fs_symlink(ndp, vap, target)
	struct nameidata *ndp;
	struct vattr *vap;
	char *target;
{
        struct s5inode *ip;
	int error;

	error = s5maknode(S5IFLNK | vap->va_mode, ndp, &ip);
	if (! error) {
		error = vn_rdwr(UIO_WRITE, S5ITOV(ip), target, 
				strlen(target),	(off_t)0, UIO_SYSSPACE, 
				IO_NODELOCKED, ndp->ni_cred, (int*)0);
		s5iput(ip);
	}
	return (error);
}

/*
 * Return target name of a symbolic link
 */

s5fs_readlink(vp, uiop, cred)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
	if (S5VTOI(vp)->i_size > uiop->uio_resid)
	        return (ERANGE);
        return (s5fs_read(vp, uiop, 0, cred));
}

/*
 * Abort op, called after namei() when a CREATE/DELETE isn't actually
 * done.
 */
s5fs_abortop(ndp)
	register struct nameidata *ndp;
{
	return(0);
}

s5fs_page_read(vp, uio, cred)
	struct vnode *vp;
	struct uio *uio;
	struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);
	int bsize;		/* size of logical blocks */
	int limit;		/* can't read more bytes than this */
	int amount;		/* bytes left to read */
	int sofar;		/* bytes read so far */
	int error;		/* our error return */
	int size;
	int phys;

	error = EINVAL;		/* initially, no data read */

	s5ILOCK(ip);
	ip->i_flag |= S5IACC;

	/*
	 *	Check that we are trying to read data that
	 *	lies within the file.
	 */

	bsize = FsBSIZE(ip->i_s5fs);
	sofar = 0;
	size = uio->uio_resid;
	phys = 0;
	if (uio->uio_segflg == UIO_PHYSSPACE)
		phys++;

	limit = ip->i_size - uio->uio_offset;
	if ((limit <= 0) || (size <= 0))
		goto done;
	amount = (size <= limit) ? size : limit;
	if (phys && limit < size)
		pmap_zero_page(uio->uio_iov->iov_base);

	do {
		daddr_t lbn;	/* logical block where our data lies */
		int on;		/* byte offset within that logical block */
		int n;		/* the number of bytes read this time */
		daddr_t rablock;
		struct buf *bp;	/* our buffer with the data */

		/*
		 *	Find block and offset within it for our data.
		 */

		lbn = FsBNO(bsize, uio->uio_offset);
		on  = FsBOFF(bsize, uio->uio_offset);

		/*
		 *	Don't read beyond the end of a logical block.
		 *	We handle logical blocks one at a time.
		 */

		n = MIN((unsigned)(bsize - on), amount);
		rablock = lbn + 1;

		/*
		 *	If we're doing sequential IO, try read-ahead.
		 */

		if (vp->v_lastr + 1 == lbn &&
		    FsLTOP(bsize, rablock) < ip->i_size) 
			error = breada(vp, lbn, bsize, rablock,
					        bsize, NOCRED, &bp);
	        else
		        error = bread(vp, lbn, bsize, NOCRED, &bp);

		vp->v_lastr = lbn;

		if (error) {
			brelse(bp);
			printf("error %d on pagein (bread)\n", error);
			error = EIO;
			goto done;
		}

		if (phys)
			copy_to_phys(bp->b_un.b_addr+on, uio->uio_iov->iov_base+sofar, n);
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

	/*
	 *	If we are returning real data in the buffer,
	 *	and we ran up against the file size,
	 *	we must zero the remainder of the buffer.
	 */

	if ( !phys && (error == 0) && (limit < size))
		bzero(uio->uio_iov->iov_base + limit, size - limit);

    done:
	s5IUNLOCK(ip);
  	if (error)
		printf("error %d on pagein (s5fs_page_read)\n", error);
	return error;
}

s5fs_page_write(vp, uio, cred, pager, offset)
	struct vnode *vp;
	struct uio *uio;
	struct ucred *cred;
	memory_object_t pager;
	vm_offset_t offset;
{
	register struct s5inode *ip = S5VTOI(vp);
	int		error;

	error = s5fs_write(vp, uio, 0, cred);
	if (error)
		printf("error %d on pageout (s5fs_page_write)\n", error);
	return error;
}

/*
 * Read wrapper for special devices.
 */
s5fsspec_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	/*
	 * Set access flag.
	 */
	S5VTOI(vp)->i_flag |= S5IACC;
	return (spec_read(vp, uio, ioflag, cred));
}

/*
 * Write wrapper for special devices.
 */
s5fsspec_write(vp, uio, ioflag, cred)
	struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{

	/*
	 * Set update and change flags.
	 */
	S5VTOI(vp)->i_flag |= S5IUPD|S5ICHG;
	return (spec_write(vp, uio, ioflag, cred));
}

/*
 * Close wrapper for special devices.
 *
 * Update the times on the inode then do device close.
 */
s5fsspec_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	register struct s5inode *ip = S5VTOI(vp);

	if (vp->v_usecount > 1 && !(s5LOCK_LOCKED(ip)))
		s5iupdat(ip, &time, &time);
	return (spec_close(vp, fflag, cred));
}

/*
 * Read wrapper for fifos.
 */
s5fsfifo_read(vp, uio, ioflag, cred)
     struct vnode *vp;
     struct uio *uio;
     int ioflag;
     struct ucred *cred;
{
     /*
      * Set access flag.
      */
     S5VTOI(vp)->i_flag |= S5IACC;
     return (fifo_read(vp, uio, ioflag, cred));
}

/*
 * Write wrapper for fifos.
 */
s5fsfifo_write(vp, uio, ioflag, cred)
     struct vnode *vp;
     struct uio *uio;
     int ioflag;
     struct ucred *cred;
{
     /*
      * Set update and change flags.
      */
     S5VTOI(vp)->i_flag |= S5IUPD | S5ICHG;
     return (fifo_write(vp, uio, ioflag, cred));
}


/*
 * Close wrapper for fifos.
 *
 * Update the times on the inode then do device close.
 */
s5fsfifo_close(vp, fflag, cred)
     struct vnode *vp;
     int fflag;
     struct ucred *cred;
{
     register struct s5inode *ip = S5VTOI(vp);

     if (vp->v_usecount > 1 && !(s5LOCK_LOCKED(ip)))
		s5iupdat(ip, &time, &time);
     return (fifo_close(vp, fflag, cred));
}

/*
 * getattr wrapper for fifos.
 */
s5fsfifo_getattr(vp, vap, cred)
     struct vnode *vp;
     register struct vattr *vap;
     struct ucred *cred;
{
     int error;

     /*
      * Get most attributes from the inode, rest from the fifo.
      */

     if (error = s5fs_getattr(vp, vap, cred))
           return (error);
     return (fifo_getattr(vp, vap, cred));
}

s5fs_mmap(register struct vnode *vp, 
	vm_offset_t offset,
	vm_map_t map,
	vm_offset_t *addrp,
	vm_size_t len,
	vm_prot_t prot,
	vm_prot_t maxprot,
	int flags,
	struct ucred *cred)
{
	return EIO;
}
s5fs_getpage(struct vnode *vp,
	register vm_offset_t offset,
	vm_size_t len,
	vm_prot_t *protp,
	vm_page_t *pl,
	register int plsz,
	vm_map_entry_t ep,
	vm_offset_t addr,
	int rw,
	struct ucred *cred)
{
	return EIO;
}

s5fs_putpage(register struct vnode *vp,
	register vm_page_t *pl,
	register int pcnt,
	int flags,
	struct ucred *cred)
{
	return EIO;
}

s5fs_swap(register struct vnode *vp,
	vp_swap_op_t swop,
	vm_offset_t args)
{
	return EIO;
}

s5fs_bread(vp, lbn, bpp, cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
	struct ucred *cred;
{
	return (EOPNOTSUPP);
}

s5fs_brelse(vp, bp)
	register struct vnode *vp;
	register struct buf *bp;
{
	return (EOPNOTSUPP);
}

s5fs_lockctl(vp, eld, flag, cred, clid, offset)
	struct vnode *vp;
	struct eflock *eld;
	int flag;
	struct ucred *cred;
	pid_t clid;
	off_t offset;
{
	int error;

	if (flag & CLNFLCK)
		return(cleanlocks(vp));
	if (flag & VNOFLCK)
		return(locked(vp, eld, flag));
	if (flag & GETFLCK) {
		return(getflck(vp, eld, offset, clid, FILE_LOCK));
	}
	if (flag & SETFLCK) {
		return(setflck(vp, eld, flag&SLPFLCK, offset, clid,FILE_LOCK));
	}
	if (flag & ENFFLCK)
		return(s5fs_setvlocks(vp));
	return(EINVAL);
}

static int
s5fs_setvlocks(vp)
	struct vnode *vp;
{
	struct s5inode *ip = S5VTOI(vp);

	if ((ip->i_mode & S_ISGID) && (!(ip->i_mode & S_IXGRP)))
		vp->v_flag |= VENF_LOCK;
	vp->v_flag |= VLOCKS;
	return(0);
}
