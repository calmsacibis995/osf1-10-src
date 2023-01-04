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
static char	sccsid[] = "@(#)$RCSfile: cdfs_vnops.c,v $ $Revision: 4.3.22.4 $ (DEC) $Date: 1993/09/22 18:28:17 $";
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
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#if SEC_ARCH
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
#include <sys/mode.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/specdev.h>
#include <sys/ioctl.h>
#include <cdfs/cdfs.h>
#include <cdfs/cdfsnode.h>
#include <cdfs/cdfsmount.h>
#include <dirent.h>
#if	MACH
#include <sys/syslimits.h>
#include <mach/memory_object.h>
#include <kern/mfs.h>
#include <kern/assert.h>
#include <kern/parallel.h>
#endif
#include <mach/mach_types.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <sys/vfs_ubc.h>
#include <vm/vm_mmap.h>
#include <vm/vm_debug.h>
#include <kern/kalloc.h>

/*
 * Global vfs data structures for cdfs
 */

int	cdfs_lookup(),
	cdfs_create(),
	cdfs_mknod(),
	cdfs_open(),
	cdfs_close(),
	cdfs_access(),
	cdfs_getattr(),
	cdfs_setattr(),
	cdfs_read(),
	cdfs_write(),
	cdfs_ioctl(),
	seltrue(),
	cdfs_mmap(),
	cdfs_fsync(),
	cdfs_seek(),
	cdfs_remove(),
	cdfs_link(),
	cdfs_rename(),
	cdfs_mkdir(),
	cdfs_rmdir(),
	cdfs_symlink(),
	cdfs_readdir(),
	cdfs_readlink(),
	cdfs_abortop(),
	cdfs_inactive(),
	cdfs_reclaim(),
	cdfs_bmap(),
	cdfs_strategy(),
	cdfs_print(),
	cdfs_page_read(),
	cdfs_page_write(),
        cdfs_getpage(),
        cdfs_putpage(),
        cdfs_swap(),
	cdfs_bread(),
	cdfs_brelse(),
	cdfs_lockctl(),
	cdfs_syncdata();

struct vnodeops cdfs_vnodeops = {
	cdfs_lookup,		/* lookup */
	cdfs_create,		/* create */
	cdfs_mknod,		/* mknod */
	cdfs_open,		/* open */
	cdfs_close,		/* close */
	cdfs_access,		/* access */
	cdfs_getattr,		/* getattr */
	cdfs_setattr,		/* setattr */
	cdfs_read,		/* read */
	cdfs_write,		/* write */
	cdfs_ioctl,		/* ioctl */
	seltrue,		/* select */
	cdfs_mmap,		/* mmap */
	cdfs_fsync,		/* fsync */
	cdfs_seek,		/* seek */
	cdfs_remove,		/* remove */
	cdfs_link,		/* link */
	cdfs_rename,		/* rename */
	cdfs_mkdir,		/* mkdir */
	cdfs_rmdir,		/* rmdir */
	cdfs_symlink,		/* symlink */
	cdfs_readdir,		/* readdir */
	cdfs_readlink,		/* readlink */
	cdfs_abortop,		/* abortop */
	cdfs_inactive,		/* inactive */
	cdfs_reclaim,		/* reclaim */
	cdfs_bmap,		/* bmap */
	cdfs_strategy,		/* strategy */
	cdfs_print,		/* print */
	cdfs_page_read,		/* page_read */
	cdfs_page_write,	/* page_write */
	cdfs_getpage,		/* get page */
	cdfs_putpage,		/* put page */
	cdfs_swap,		/* swap handler */
	cdfs_bread,		/* buffer read */
	cdfs_brelse,		/* buffer release */
	cdfs_lockctl,		/* Need for KLM support */
	cdfs_syncdata,		/* fsync byte range */
};

extern int	spec_lookup(),
	spec_open(),
	spec_close(),
	spec_read(),
	spec_write(),
	spec_ioctl(),
	spec_select(),
	spec_mmap(),
	spec_seek(),
	cdfsspec_reclaim(),
	spec_strategy(),
	spec_bmap(),
	spec_badop(),
	spec_nullop(),
	spec_swap(),
	spec_lockctl();

extern int spec_bread(), spec_brelse();

struct vnodeops spec_cdnodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	spec_open,		/* open */
	spec_close,		/* close */
	cdfs_access,		/* access */
	cdfs_getattr,		/* getattr */
	cdfs_setattr,		/* setattr */
	spec_read,		/* read */
	spec_write,		/* write */
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
	cdfs_inactive,		/* inactive */
	cdfsspec_reclaim,	/* reclaim */
	spec_bmap,		/* bmap */
	spec_strategy,		/* strategy */
	cdfs_print,		/* print */
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
	fifo_close(),
	fifo_read(),
	fifo_write(),
	fifo_ioctl(),
	cdfsfifo_getattr(),
	fifo_select();

extern int fifo_bread(), fifo_brelse();

struct vnodeops fifo_cdnodeops = {
	spec_lookup,		/* lookup */
	spec_badop,		/* create */
	spec_badop,		/* mknod */
	fifo_open,		/* open */
	fifo_close,		/* close */
	cdfs_access,		/* access */
	cdfsfifo_getattr,	/* getattr */
	cdfs_setattr,		/* setattr */
	fifo_read,		/* read */
	fifo_write,		/* write */
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
	cdfs_inactive,		/* inactive */
	cdfs_reclaim,		/* reclaim */
	spec_bmap,		/* bmap */
	spec_badop,		/* strategy */
	cdfs_print,		/* print */
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

enum vtype cdftovt_tab[16] = {
	VNON, VFIFO, VCHR, VNON, VDIR, VNON, VBLK, VNON,
	VREG, VNON, VLNK, VNON, VSOCK, VNON, VNON, VBAD,
};

/*
 * Create a regular file
 */
cdfs_create(ndp, vap)
	struct nameidata *ndp;
	struct vattr *vap;
{
	return(EROFS);
}
/*
 * Mknod vnode call
 */
/* ARGSUSED */
cdfs_mknod(ndp, vap, cred)
	struct nameidata *ndp;
	struct ucred *cred;
	register struct vattr *vap;
{
	return (EROFS);
}

/* ARGSUSED */
cdfs_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{
	struct cdnode *cdp = VTOCD(*vpp);
	if (cdp->cd_flag &  CDNODE_BADVOL)
	    return ENODEV;		/* per XCDR 3.6 */
	return (0);
}

/* ARGSUSED */
cdfs_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	return (0);
}

cdfs_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	struct cdnode *cdp = VTOCD(vp);
	return cdnodeaccess(cdp, mode, cred);
}

/* ARGSUSED */
cdfs_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	register struct cdnode *cdp = VTOCD(vp);
	register struct fs *fs;

	enum vtype type;

	if (cdp->cd_flag &  CDNODE_BADVOL)
	    return ENODEV;		/* per XCDR 3.6 */

	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	/*
	 * Copy from cdnode table
	 */
	IN_LOCK(cdp);
	fs = cdp->cd_fs;
	vap->va_fsid = cdp->cd_dev;
	vap->va_fileid = cdp->cd_number;
	vap->va_mode = cdp->cd_mode & ~CDFMT;
	vap->va_nlink = cdp->cd_nlink;
	vap->va_uid = cdp->cd_uid;
	vap->va_gid = cdp->cd_gid;
	vap->va_rdev = cdp->cd_rdev;
#if __alpha
	vap->va_qsize = cdp->cd_size;
#else
	vap->va_qsize.val[0] = cdp->cd_size;
#endif
	vap->va_atime.tv_sec = cdp->cd_atime;
	vap->va_atime.tv_usec = 0;
	vap->va_mtime.tv_sec = cdp->cd_mtime;
	vap->va_mtime.tv_usec = 0;
	vap->va_ctime.tv_sec = cdp->cd_ctime;
	vap->va_ctime.tv_usec = 0;
	vap->va_flags = 0;
	vap->va_gen = cdp->cd_gen;
	/* this doesn't belong here */
	if (type == VBLK)
		vap->va_blocksize = BLKDEV_IOSIZE;
	else if (type == VCHR)
		vap->va_blocksize = MAXBSIZE;
	else
	    vap->va_blocksize = fs->fs_ibsize;
	vap->va_bytes = cdp->cd_size;
	IN_UNLOCK(cdp);
#if !__alpha
	vap->va_bytes_rsv = -1;
#endif
	vap->va_type = type;
	return (0);
}
/*
 * getattr wrapper for fifos.
 */
cdfsfifo_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int error;

	/*
	 * Get most attributes from the inode, rest
	 * from the fifo.
	 */
	if (error = cdfs_getattr(vp, vap, cred))
		return (error);
	return (fifo_getattr(vp, vap, cred));
}

/*
 * Set attribute vnode op. called from several syscalls
 */
cdfs_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	register struct ucred *cred;
{
	register struct cdnode *cdp = VTOCD(vp);

	/*
	 * Check for unsettable attributes.
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
	if (vap->va_uid != (uid_t) VNOVAL ||
	    vap->va_gid != (gid_t) VNOVAL ||
	    vap->va_atime.tv_sec != (int)VNOVAL ||
	    vap->va_mtime.tv_sec != (int)VNOVAL ||
	    vap->va_mode != (u_short) VNOVAL ||
	    vap->va_flags != (u_int)VNOVAL)
	    return EROFS;

	/* allow vap->va_size for char, block, or fifo */
	if (vp->v_type == VCHR || vp->v_type == VBLK ||
	    vp->v_type == VFIFO)
	    return 0;
	return (EROFS);
}

/*
 * Vnode op for reading.
 */
/* ARGSUSED */
cdfs_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	register struct cdnode *cdp = VTOCD(vp);
	register struct fs *fs;
	int error = 0;
	int type;
	vm_size_t len;
	vm_page_t pl[VP_PAGELIST+1];
	register vm_page_t *pp;
	register vm_offset_t off;
	vm_size_t n, diff;
	vm_offset_t addr;
	extern cdfs_getpage();

	if (uio->uio_rw != UIO_READ)
		panic("cdfs_read mode");
	if (uio->uio_resid == 0)
		return (0);
	/* no need to check uio_offset < 0 since it's an off_t,
	   which is an unsigned quantity */
	/*
	 * Zero first page in list
	 */
	pp = pl;
	*pp = VM_PAGE_NULL;
	IN_READ_LOCK(cdp);
	IN_LOCK(cdp);
	type = cdp->cd_mode & CDFMT;
	if (type != CDFDIR && type != CDFREG)
		panic("cdfs_read: type");
	fs = cdp->cd_fs;
	IN_UNLOCK(cdp);

	if (type == CDFDIR) {
		/* underlying directory does not have a VM object associated
		   with it, so we must use bread & friends to get the
		   directory. */
		/* cf. ufs_read() */
		daddr_t lbn, bn;
		int lbs, stackxfercount;
		register int xfercount;
		unsigned int offinbuf;
		long dirsiz, dif; /* big,  to hold off_t */
		struct buf *bp;

		lbs = ISOFS_LBS(fs);
		dirsiz = cdp->cd_size;

		do {
			lbn = uio->uio_offset / lbs;

			/*
			 * xfercount will be set to # bytes from this block to
			 * copy out.
			 *
			 * figure out which fs->fs_ibsize block contains
			 * the desired file block, and at what offset that
			 * file block lays.
			 */

			cdfs_ibmap(cdp, lbn, &bn, &stackxfercount, &offinbuf);
			/* copy to register variable: */
			xfercount = stackxfercount;

			/*
			 * adjust offset, xfercount to reflect the user's
			 * offset
			 */
			offinbuf += (uio->uio_offset % lbs);

			xfercount -= (uio->uio_offset % lbs);

			dif = dirsiz - uio->uio_offset;
			if (dif <= 0) {
				IN_READ_UNLOCK(cdp);
				return (0);
			}

			/* clamp transfer by what's left & what's asked for */
			xfercount = MIN(xfercount, dif);
			xfercount = MIN(xfercount, uio->uio_resid);

			/*
			 * XXX
			 * should figure out if readahead is warranted.
			 * can't use cdfs_rablk because it uses UBC, which
			 * we can't use here.
			 */
			error = bread(vp, bn, fs->fs_ibsize, NOCRED, &bp);
			if (error) {
				brelse(bp);
				IN_READ_UNLOCK(cdp);
				return (error);
			}

			LASSERT(BUF_LOCK_HOLDER(bp));
			ASSERT(bp->b_resid >= 0);
			/* XXX should set v_lastr for readahead detection */
			if (bp->b_resid) {
				/*
				 * partial failure to read: figure out how
				 * much is available.  b_resid == # bytes not
				 * transferred
				 */
				xfercount += offinbuf;
				xfercount = MIN(xfercount,
						fs->fs_ibsize - bp->b_resid);
				xfercount -= offinbuf;
				if (xfercount < 0)
				    xfercount = 0;
			} 

			error = uiomove(bp->b_un.b_addr + offinbuf,
					(int)xfercount, uio);
			if (xfercount + offinbuf == fs->fs_ibsize
			    || uio->uio_offset == dirsiz)
			    bp->b_flags |= B_AGE;
			brelse(bp);
		} while (error == 0 && uio->uio_resid > 0 && xfercount != 0);
		IN_READ_UNLOCK(cdp);
		return (error);
	}
	/* not directory, use UBC: */
	do {
		pp = pl;
		off = uio->uio_offset & page_mask;
		if ((uio->uio_resid + off) > VP_PAGELISTSIZE)
                        len = VP_PAGELISTSIZE - off;
                else
			len = uio->uio_resid;
		if ((uio->uio_offset + len) >= cdp->cd_size) { 
			if (uio->uio_offset >= cdp->cd_size) {
				break;
			}
			len = cdp->cd_size - uio->uio_offset;
		}
		error = cdfs_getpage(vp, uio->uio_offset, 
				     len, (vm_prot_t *) 0, pl, 0,
				     (vm_map_entry_t) 0, (vm_offset_t) 0,
				     1, cred);
		if (error || *pp == VM_PAGE_NULL) {
			printf("cdfs_read: cdfs_getpage error %d\n", error);
			break;
		}
		do {
			vm_page_wait(*pp);
			n = MIN((PAGE_SIZE - off), uio->uio_resid);
			diff = cdp->cd_size - uio->uio_offset;
			if (diff < n)
				n = diff;
			addr = ubc_load(*pp, off, n); 
			error = uiomove(addr + off, n, uio);
			ubc_unload(*pp, off, n);
			if (error) {
				printf("cdfs_read: uiomove error %d\n", error);
				break;
			}
			ubc_page_release(*pp, 0);
			*pp = VM_PAGE_NULL;
			pp++;
			off = 0;
		} while (error == 0 && (*pp != VM_PAGE_NULL) &&
			 uio->uio_resid);
	} while (error == 0 && uio->uio_resid > 0);

	if (error || *pp != VM_PAGE_NULL) {
		while (*pp != VM_PAGE_NULL) {
			vm_page_wait(*pp);
			ubc_page_release(*pp, 0);
			pp++;
		}
	}

	IN_READ_UNLOCK(cdp);
	return (error);
}

/*
 * Vnode op for writing.
 */
cdfs_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	return(EROFS);
}

static int getdirbuf(struct fs *, struct cdnode *, struct vnode *,
		     unsigned int *, int *, unsigned int, struct buf **);

/* ARGSUSED */
cdfs_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{
    register int rc = 0;
    struct rrip_map_arg *maparg;
    struct rrip_suf_arg *sufarg;
    struct rrip_map_idx_arg *idxarg;
    struct rrip_devmap *dmap;
    struct nameidata *ndp = &u.u_nd;
    struct vnode *nvp = 0;
    struct buf *bp;
    register struct cdnode *cdp;
    struct fs *fs;
    dev_t convert;
    int len;
    unsigned char *kpath;
    int type;

    cdp = VTOCD(vp);
    type = cdp->cd_mode & CDFMT;
    if (type != CDFDIR && type != CDFREG)
	return ENOTTY;

    switch(com) {
    case CDIOCSETDMAP:
    case CDIOCUNSETDMAP:
#if	SEC_BASE
	if (security_is_on) {
	    if (!privileged(SEC_FILESYS, EPERM))
		return (EPERM);
	} else
#endif	/* SEC_BASE */
	    if (rc = suser(cred, NULL))
		return (rc);
	break;
    default:;
	/* CDIOCGETSUF, CDIOCGETDMAP, CDIOCGETDMAPIDX are not privileged.
	   CDIOCGETSUF requires VREAD on the requested vnode */
    }
    /* arg validation here, if appropriate (e.g. null ptr?) */
    switch (com) {
    case CDIOCGETSUF:			/* get System Use field */
    case CDIOCSETDMAP:			/* set device mapping */
    case CDIOCUNSETDMAP:		/* unset device mapping */
    case CDIOCGETDMAP:			/* get device mapping */
    case CDIOCGETDMAPIDX:		/* get nth device mapping */
	ndp->ni_segflg = UIO_USERSPACE;
	switch(com) {
	case CDIOCGETSUF:
	    sufarg = (struct rrip_suf_arg *)data;
	    ndp->ni_dirp = sufarg->path;
	    ndp->ni_nameiop = LOOKUP|NOFOLLOW;
	    break;
	case CDIOCGETDMAPIDX:
	    idxarg = (struct rrip_map_idx_arg *)data;
	    ndp->ni_dirp = idxarg->path;
	    ndp->ni_nameiop = LOOKUP|FOLLOW;
	    break;
	default:
	    maparg = (struct rrip_map_arg *)data;
	    ndp->ni_dirp = maparg->path;
	    ndp->ni_nameiop = LOOKUP|FOLLOW;
	    break;
	}
	/* XXX swap creds? */
	if (rc = namei(ndp))
	    return rc;
	nvp = ndp->ni_vp;
	if (nvp == NULL)
	    return ENOENT;
	if (nvp->v_op != &cdfs_vnodeops &&
	    nvp->v_op != &spec_cdnodeops &&
	    nvp->v_op != &fifo_cdnodeops) {
	    /* only CDFS nodes. */
	    vrele(nvp);
	    return EINVAL;
	}
	switch (com) {
	case CDIOCGETDMAP:
	case CDIOCGETSUF:
	case CDIOCGETDMAPIDX:
	    /* verify access */
	    VOP_ACCESS(nvp, VREAD, ndp->ni_cred, rc);
	    if (rc) {
		vrele(nvp);
		return rc;
	    }
	    /* XXX this code needs the cast.  why? */
	    if (com != (int) CDIOCGETDMAP)
		break;
	    /* else fall thru: */
	default:
	    if (nvp->v_type != VCHR && nvp->v_type != VBLK) {
		vrele(nvp);
		return EINVAL;
	    }
	    break;
	}
	if (nvp->v_mount != vp->v_mount) {
	    vrele(nvp);
	    return EINVAL;		/* XXX ? */
	}
	cdp = VTOCD(nvp);
	fs = cdp->cd_fs;
	if (fs->fs_format != ISO_RRIP) {
	    vrele(nvp);
	    return ENOTTY;
	}
    }
    switch (com) {
    case CDIOCSETDMAP:
	convert = makedev(maparg->major, maparg->minor);
	PN_ALLOCATE(kpath);
	rc = copyinstr(maparg->path, kpath, MAXPATHLEN, &len);
	if (!rc) {
	    char *kpath2;
	    kpath[MAXPATHLEN-1] = '\0'; /* XXX necessary? */
	    kpath2 = kalloc(strlen(kpath)+1);
	    if (!kpath2)
		rc = ENOMEM;
	    else {
		bcopy(kpath, kpath2, strlen(kpath)+1); /* include NUL */
		rc = rrip_mapdev(fs, cdp, convert, kpath2,
				 strlen(kpath)+1);
		if (!rc) {
		    /* We cannot change cd->cd_rdev until the node is
		       inactive, otherwise it will be unchained from the
		       alias vnodes incorrectly.  Therefore we vgone()
		       the old vnode so that we can reset the device number. */
		    /* The alternative is for the node to retain its old
		       identity until it is reclaimed, which might be never. */
		    VN_LOCK(nvp);
		    (void) vgone(nvp, VX_NOSLEEP, 0);
		    VN_UNLOCK(nvp);
		    /* will be reinitialized with new rdev on next ref */
		}
	    }
	}
	PN_DEALLOCATE(kpath);
	break;
    case CDIOCUNSETDMAP:
	rc = rrip_unmapdev(fs, cdp);
	if (!rc) {
	    maparg->major = major(cdp->cd_ondiskrdev);
	    maparg->minor = minor(cdp->cd_ondiskrdev);
	    /* convert back to old mapping */
	    /* see above comments about changing the device mapping */
	    VN_LOCK(nvp);
	    (void) vgone(nvp, VX_NOSLEEP, 0);
	    VN_UNLOCK(nvp);
	    /* will be reinitialized with new rdev on next ref */
	}
	break;
    case CDIOCGETDMAP:
	convert = rrip_getnodedevmap(fs, cdp);
	if (convert == NODEV)
	    rc = ESRCH;			/* ENOENT? XXX */
	else {
	    maparg->major = major(convert);
	    maparg->minor = minor(convert);
	}		
	break;
    case CDIOCGETDMAPIDX:
	dmap = rrip_getnthdevmap(fs, idxarg->index);
	if (dmap) {
	    idxarg->major = major(dmap->newdev);
	    idxarg->minor = minor(dmap->newdev);
	    rc = copyoutstr(dmap->path,
			    idxarg->path, idxarg->pathlen, &len);
	    idxarg->pathlen = MIN(dmap->pathlen,idxarg->pathlen);
	} else
	    rc = ESRCH;			/* no such mapping */
	break;
    case CDIOCGETSUF:
    {
	int count;
	unsigned char *inbuf;
	struct cd_suf_header *hdr;
	unsigned int offinbuf;
	int lbs;
	int lbn;
	int error;
	struct cd_suf_ce cont_holder;

	if (sufarg->fsec == -1)
	    sufarg->fsec = 1;		/* XXX */

	/* sufarg->sig_index is unsigned; no need for range check */

	if (sufarg->fsec != 1) {
	    rc = EINVAL;
	    break;
	}
	/*
	 * get the buffer with the directory entry.
	 */
	/* XXX multiple caching?  getting block from device
	   node rather than directory node. */
	lbs = ISOFS_LBS(fs);
	/* cdp->cd_number is byte offset of file's dirent */
	lbn = cdp->cd_number / lbs;	/* get lbn of directory entry */
	offinbuf = cdp->cd_number % lbs; /* offset in that block */
	rc = bread(cdp->cd_devvp, lbn * (lbs / DEV_BSIZE),
		   fs->fs_ibsize, NOCRED, &bp);
	if (rc)
	    break;
	    
	susp_compute_diroff((struct iso_dir *)(bp->b_un.b_addr + offinbuf),
			    fs->rrip_susp_offset, &count, &inbuf);
	bzero(&cont_holder, sizeof(cont_holder));
	do {
	    /* do NOT cache things based on this lookup (pass cdp == 0) */
	    hdr = susp_search(fs, cdp->cd_devvp, &inbuf, &count,
			      sufarg->sigok, sufarg->sig, &bp, 0,
			      &cont_holder);
	} while (hdr && --sufarg->sig_index);
	if (!hdr)
	    rc = ESRCH;			/* flag for user-level cd_suf(3) */
	else {
	    /* hdr points to it */
	    sufarg->buflen = MIN(sufarg->buflen, hdr->suf_length);
	    rc = copyout(hdr, sufarg->buf, sufarg->buflen);
	    brelse(bp);
	}	
	break;
    }
	
    default:
	rc = ENOTTY;
    }					/* switch(com) */
    if (nvp)
	vrele(nvp);
    return (rc);
}

/*
 * Mmap a file
 *
 */

cdfs_mmap(register struct vnode *vp, 
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

/*
 * Synch an open file.
 */
/* ARGSUSED */
cdfs_fsync(vp, fflags, cred, waitfor)
	struct vnode *vp;
	int fflags;
	struct ucred *cred;
	int waitfor;
{
	return(0);
}

/*
 * Synch a range of an open file.
 */
/* ARGSUSED */
cdfs_syncdata(vp, fflags, start, length, cred)
	struct vnode *vp;
	int fflags;
	vm_offset_t start;
	vm_size_t length;
	struct ucred *cred;
{
	return(cdfs_fsync(vp, fflags, cred, MNT_WAIT));
}

/*
 * Seek on a file
 *
 * Negative offsets are invalid.
 */
/* ARGSUSED */
cdfs_seek(vp, oldoff, newoff, cred)
	struct vnode *vp;
	off_t oldoff, newoff;
	struct ucred *cred;
{
    /* off_t is unsigned, don't need to check for < zero. */
    return(0);
}

/*
 * cdfs remove
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
cdfs_remove(ndp)
	struct nameidata *ndp;
{
	return(EROFS);
}

/*
 * link vnode call
 */
cdfs_link(vp, ndp)
	register struct vnode *vp;
	register struct nameidata *ndp;
{
	return(EROFS);
}

/*
 * Rename system call.
 * 	rename("foo", "bar");
 */
cdfs_rename(fndp, tndp)
	register struct nameidata *fndp, *tndp;
{
	return(EROFS);
}

/*
 * Mkdir system call
 */
cdfs_mkdir(ndp, vap)
	struct nameidata *ndp;
	struct vattr *vap;
{
	return(EROFS);
}

/*
 * Rmdir system call.
 */
cdfs_rmdir(ndp)
	register struct nameidata *ndp;
{
	return(EROFS);
}

/*
 * symlink -- make a symbolic link
 */
cdfs_symlink(ndp, vap, target)
	struct nameidata *ndp;
	struct vattr *vap;
	char *target;
{
	return(EROFS);
}



char
cdfs_tolower(c)
	int c;
{
	int result;

	if (c >= 'A' && c <= 'Z')
		result = c - 'A' + 'a';
	else
		result = c;
	return((char)result);
}

/*
 * helper routine to bread the block containing the directory entry
 * in question.
 */
static int 
getdirbuf(struct fs *fs,
	  struct cdnode *cdp,
	  struct vnode *vp,
	  unsigned int *offinbuf,
	  int *datainbuf,
	  unsigned int dir_offset,
	  struct buf **bpp)
{
    int lbn;				/* lbn of active on-disk directory
					   entry */
    daddr_t bn;
    int lbs = ISOFS_LBS(fs);
    int error;

    lbn = dir_offset / lbs;	/* get lbn of next directory entry */
    /*
     * Since directories cannot be interleaved, datainbuf
     * and tsize will both equal fs_ibsize, We are only
     * interested in bn and offinbuf.
     */
    cdfs_ibmap(cdp, lbn, &bn, datainbuf, offinbuf);
    /*
     * At this point, datainbuf counts the # of bytes left in the
     * buffer.  offinbuf is the offset from the buffer's start.
     *
     * adjust them more to get to the next directory entry, clamped by
     * the amount of data recorded in the diretory.
     * 
     */
    *offinbuf += (dir_offset % lbs);
    *datainbuf -= (dir_offset % lbs);
    *datainbuf = MIN(*datainbuf, cdp->cd_size - dir_offset);
    /* XXX todo: add read-ahead */
    if (error = bread(vp, bn, fs->fs_ibsize, NOCRED, bpp)) {
	if (*bpp)
	    brelse(*bpp);
	*bpp = (struct buf *)0;
    }
    return error;
}

/*
 * Vnode op for reading a directory
 */

cdfs_readdir(vp, uio, cred, eofflagp)
	struct vnode *vp;
	register struct uio *uio;
	struct ucred *cred;
	int *eofflagp;
{
    register struct cdnode *cdp = VTOCD(vp);
    register struct cdfsmount *cdmntp = VFSTOCDFS(vp->v_mount);
    struct buf *bp;			/* useful buf ptr */
    struct fs *fs;			/* ptr to CDFS superblock */
    int error = 0;
    int dirblkstotransfer;
    int lbs;				/* CDFS logical block size */
    struct iso_dir *tmp_iso_dir;
    struct hsg_dir *tmp_hsg_dir;
    struct dirent *gen_dir;
    union {
	unsigned char incoming[4];
	unsigned int  outgoing;
    } iso_convert_int;
    unsigned int ubuf_size;
    unsigned int isodir_reclen;
    unsigned int isodir_offset;		/* working copy of offset
					   into the real on-disk directory */
    unsigned int offinbuf;
    int gendir_resid;			/* #bytes left in the output
					   generic directory block */
    int gen_dir_count;			/* count of bytes of fabricated
					   dirents so far */
    int rablock, rasize;
    int wasdot;
    unsigned int diskaddr;
    int datainbuf, tsize;
    int isiso = 0, isrrip = 0;
    int skip_file;
    int next_dir_len;
    int length;

    CDDEBUG1(READDIRDEBUG,
	     printf("cdfs_readdir: cdp 0x%x uio 0x%x cred 0x%x count = %d\n",
		    cdp, uio, cred, uio->uio_resid));
    /*
     * If nothing to read......
     */
    if (uio->uio_resid == 0) {
	return (0);
    }

    bp = (struct buf *)0;

    /*
     * Basic strategy (for now):
     * start at the beginning of the real directory
     * scan until the fabricated dirents get beyond the
     * offset pointer the user requests.
     * Then start copying entries.
     *
     * Unfortunately, it requires a change in the readdir() interface
     * to do this more efficiently.
     *
     * XXX once the interface changes, read-ahead code should be added
     * at the bread()
     */

    /* get a block of memory to hold the dirents we create */
    ZALLOC(cdfsreaddir_zone, gen_dir, struct dirent *);

    fs = cdp->cd_fs;
    if (fs->fs_format == ISO_RRIP)
	isrrip = 1;
    else if (fs->fs_format == ISO_9660)
	isiso = 1;

    lbs = ISOFS_LBS(fs);

    ubuf_size = howmany(uio->uio_resid, DIRBLKSIZ) * DIRBLKSIZ;

    isodir_offset = 0;
    gen_dir_count = 0;

    /*
     * Set dirblkstotransfer to number of directory blocks which will
     * fit in the users buffer.
     */
    dirblkstotransfer = howmany(ubuf_size, DIRBLKSIZ);
    if (dirblkstotransfer <= 0) {
	error = EINVAL;
	CDDEBUG1(READDIRDEBUG,
		 printf("cdfs_readdir: dirblkstotransfer <= 0 = %d\n",
			dirblkstotransfer));
	goto out;
    }
    /*
     * Set dirblkstotransfer to the minimum of the number of directory
     * blocks that would fit into the user buffer, and the number of
     * directory blocks in the directory.
     */
    dirblkstotransfer = MIN(dirblkstotransfer, 
			    (roundup(cdp->cd_size, ISO_SECSIZE)
			     / DIRBLKSIZ));
    /*
     * If there is nothing left to transfer, goto out.
     */
    if (dirblkstotransfer == 0) {
	goto out;
    }
#ifdef CDFSDEBUG
    if (READDIRDEBUG) {
	printf("cdfs_readdir: dirblkstotransfer = %d\n", 
	       dirblkstotransfer);

	if (cdp->cd_number % ISO_SECSIZE) {
	    printf("cdfs_readdir: cdnode number %d (dir) does not start on a sector boundary\n", cdp->cd_number);
	    goto out;
	}
    }
#endif /* CDFSDEBUG */
    /* compute address of the beginning of the directory */
    diskaddr = ((unsigned int)cdp->iso_dir_extent +
		(unsigned int)cdp->iso_dir_xar) * lbs;
    do {
	CDDEBUG1(READDIRDEBUG,
		 printf("cdfs_readdir: isodir_offset = %d\n",
			isodir_offset));
	if (isodir_offset >= cdp->cd_size)
	    /* nothing left in the on-disk directory. */
	    break;
	/* get the directory block in-core */
	error = getdirbuf(fs, cdp, vp, &offinbuf, &datainbuf,
			  isodir_offset, &bp);
	if (error)
	    goto out;
	/*
	 * If the current directory record length is zero, skip
	 * to the start of the next record which will reside in
	 * the next sector.
	 * ISO9660 forbids directory entries from spanning logical sectors,
	 * so no worries about running beyond the buffer.
	 */
	if (isiso || isrrip) {
	    tmp_iso_dir = (struct iso_dir *)(bp->b_un.b_addr + offinbuf);
	    length = tmp_iso_dir->dir_len;
	} else {
	    tmp_hsg_dir = (struct hsg_dir *)(bp->b_un.b_addr + offinbuf);
	    length = tmp_hsg_dir->dir_len;
	}
	if (length == 0) {
	    brelse(bp);
	    bp = (struct buf *) 0;
	    isodir_offset += (ISO_SECSIZE - (isodir_offset % ISO_SECSIZE));
	    continue;
	}
		
	wasdot = 0;			/* last entry we saw was not `.' */
	tsize = datainbuf;		/* tsize counts down #bytes left
					   in the on-disk sector */
	gendir_resid = DIRBLKSIZ;	/* it's empty */
	do {
	    /* ino 0 is out of band since ISO 9660 disks
	       leave the first 16k of the disk unspecified,
	       so it's not a block address that appears anywhere.
	       thus, we can use it to flag alternate inums for
	       things. */
	    int hold_ino = 0;
	    unsigned char *alt_name = 0;
	    skip_file = 0;
	    switch(fs->fs_format) {
	    case ISO_RRIP:
	    {
		struct buf *nbp = 0;
		int dealloc;
		struct rrip_re *reptr;
		/* See similar code in cdfs_lookup() */
		if (tmp_iso_dir->dir_file_flags&ISO_FLG_DIR) {
		    if (rrip_skipdir(fs, cdp->cd_devvp, tmp_iso_dir, 0)) {
			/* relocated directory.  It doesn't live here. */
			skip_file = 1;
			length = tmp_iso_dir->dir_namelen;
			/* this entry is unused. set ino == 0 so that
			   this entry is ignored by user-level code. */
			gen_dir->d_ino = 0;
			gen_dir->d_namlen = length;
			bcopy(tmp_iso_dir->dir_name, gen_dir->d_name, length);
			gen_dir->d_name[length] = '\0';
			/* XXX should we just not copy the name at all? */

			isodir_reclen = (unsigned int)tmp_iso_dir->dir_len;
			tmp_iso_dir = (struct iso_dir *)
			    ((unsigned long)tmp_iso_dir + isodir_reclen);
			if (tsize - (int)isodir_reclen > 0)
			    /* room for more stuff... dir_len is first byte,
			       so it's guaranteed OK to ref */
			    next_dir_len = tmp_iso_dir->dir_len;
			else next_dir_len = 0;
			break;		/* from the switch statement */
		    }
		    if (tmp_iso_dir->dir_namelen == 1) {
			if (tmp_iso_dir->dir_name[0] == '\0') {
			    wasdot = 1;
			} else if (wasdot) {
			    hold_ino = rrip_parent_num(fs, cdp->cd_devvp,
						       tmp_iso_dir, 0);
			}
		    }
		} else {
		    /* not a directory...maybe */
		    /* look for relocated child, and get its block
		       offset/inode #.  returns zero if not relocated */
		    hold_ino = rrip_child_num(fs, cdp->cd_devvp,
					      tmp_iso_dir, 0);
		    /* we don't care if it's a directory or file, since
		       we're just returning name/inode pairs in dirent
		       structures. */
		}
		if (wasdot)
		    alt_name = 0;	/* force jump to code below */
		else
		    alt_name = rrip_compose_altname(fs,
						    tmp_iso_dir,
						    &nbp,
						    cdp->cd_devvp,
						    &length,
						    &dealloc);
		if (alt_name) {
		    if (hold_ino)
			gen_dir->d_ino = hold_ino;
		    else if (tmp_iso_dir->dir_file_flags&ISO_FLG_DIR) {
			CDFS_COPYINT(tmp_iso_dir->dir_extent_lsb,
				     iso_convert_int.incoming);
			gen_dir->d_ino = (iso_convert_int.outgoing +
					  tmp_iso_dir->dir_xar) * lbs;
		    } else {
			/*
			 * If associated file, skip over file.
			 */
			if (tmp_iso_dir->dir_file_flags & ISO_FLG_ASSOC) {
			    skip_file = 1;
			    gen_dir->d_ino = 0;
			} else
			    gen_dir->d_ino = diskaddr + isodir_offset;
		    }
		    gen_dir->d_namlen = length;
		    if (DIRSIZ(gen_dir) > gendir_resid) {
			/* name will not fit in this block.
			   put in a zapped entry, and then find a way
			   to restart */
			gen_dir->d_ino = 0;
			gen_dir->d_namlen = 0;
			gen_dir->d_name[0] = '\0';

			next_dir_len = 0;
			isodir_reclen = 0; /* cheat, and come back here */
			/* don't touch tmp_iso_dir */
		    } else {
			bcopy(alt_name, gen_dir->d_name, length);
			gen_dir->d_name[length] = '\0';
			isodir_reclen = 
			    (unsigned int)tmp_iso_dir->dir_len;
			tmp_iso_dir = (struct iso_dir *)
			    ((unsigned long)tmp_iso_dir + 
			     isodir_reclen);
			if (tsize - (int)isodir_reclen > 0)
			    /* room for more stuff... dir_len is first byte,
			       so it's guaranteed OK to ref */
			    next_dir_len = tmp_iso_dir->dir_len;
			else next_dir_len = 0;
		    }
		    if (nbp)
			brelse(nbp);
		    if (dealloc)
			PN_DEALLOCATE(alt_name);
		    break;
		} else {
		    CDDEBUG1(RRIPDIRDEBUG, printf("no alt name\n"));
		    /* if no alt name, fall through to
		       ISO_9660 default case: */
		}
	    }
	    case ISO_9660:
		if (hold_ino ||
		    (tmp_iso_dir->dir_file_flags&ISO_FLG_DIR)) {
		    CDFS_COPYINT(tmp_iso_dir->dir_extent_lsb,
				 iso_convert_int.incoming);

		    gen_dir->d_ino = hold_ino ? hold_ino :
			(iso_convert_int.outgoing +
			 tmp_iso_dir->dir_xar) * lbs;
		    if (tmp_iso_dir->dir_name[0] == '\0') {
			gen_dir->d_namlen = 1;
			gen_dir->d_name[0] = '.';
			gen_dir->d_name[1] = '\0';
			wasdot = 1;
		    } else if (wasdot) {
			gen_dir->d_namlen = 2;
			gen_dir->d_name[0] = '.';
			gen_dir->d_name[1] = '.';
			gen_dir->d_name[2] = '\0';
			wasdot = 0;
		    } else {
			gen_dir->d_namlen = 
			    tmp_iso_dir->dir_namelen;
			bcopy(tmp_iso_dir->dir_name,
			      gen_dir->d_name, 
			      tmp_iso_dir->dir_namelen);
			gen_dir->d_name[tmp_iso_dir->dir_namelen] = '\0';
		    }

		} else {
		    /*
		     * If associated file, skip over file.
		     */
		    if (tmp_iso_dir->dir_file_flags & ISO_FLG_ASSOC) {
			skip_file = 1;
			gen_dir->d_ino = 0;
		    } else
			gen_dir->d_ino = diskaddr + isodir_offset;
		    length = tmp_iso_dir->dir_namelen;
		    gen_dir->d_namlen = length;
		    bcopy(tmp_iso_dir->dir_name, gen_dir->d_name, length);
		    gen_dir->d_name[length] = '\0';
		}
		isodir_reclen = 
		    (unsigned int)tmp_iso_dir->dir_len;
		tmp_iso_dir = (struct iso_dir *)
		    ((unsigned long)tmp_iso_dir + 
		     isodir_reclen);
		if (tsize - (int)isodir_reclen > 0)
		    /* room for more stuff... dir_len is first byte,
		       so it's guaranteed OK to ref */
		    next_dir_len = tmp_iso_dir->dir_len;
		else next_dir_len = 0;
		break;
	    default:			/* HSG */
		if (tmp_hsg_dir->dir_file_flags&ISO_FLG_DIR) {
		    bcopy(tmp_hsg_dir->dir_extent_lsb, 
			  iso_convert_int.incoming,
			  sizeof(int));
		    gen_dir->d_ino = 
			(iso_convert_int.outgoing +
			 tmp_hsg_dir->dir_xar) * lbs;
		    if (tmp_hsg_dir->dir_name[0] == '\0') {
			gen_dir->d_namlen = 1;
			gen_dir->d_name[0] = '.';
			gen_dir->d_name[1] = '\0';
			wasdot = 1;
		    } else if (wasdot) {
			gen_dir->d_namlen = 2;
			gen_dir->d_name[0] = '.';
			gen_dir->d_name[1] = '.';
			gen_dir->d_name[2] = '\0';
			wasdot = 0;
		    } else {
			gen_dir->d_namlen = 
			    tmp_hsg_dir->dir_namelen;
			bcopy(tmp_hsg_dir->dir_name,
			      gen_dir->d_name, 
			      tmp_hsg_dir->dir_namelen);
			gen_dir->d_name[tmp_hsg_dir->dir_namelen] = '\0';
		    }

		} else {
		    /*
		     * If associated file, or volume seq
		     * number does not match file primary
		     * volume descriptor volume sequence
		     * number and multivolume set, skip
		     * over file.
		     */
		    if ((tmp_hsg_dir->dir_file_flags & ISO_FLG_ASSOC) ||
			(ISOFS_SETSIZE(fs) > 1 &&
			 tmp_hsg_dir->dir_vol_seq_no_lsb !=
			 ISOFS_VOLSEQNUM(fs))) {
			skip_file = 1;
			gen_dir->d_ino = 0;
		    } else
			gen_dir->d_ino = diskaddr + isodir_offset;
		    length = tmp_hsg_dir->dir_namelen;
		    gen_dir->d_namlen = length;
		    bcopy(tmp_hsg_dir->dir_name,
			  gen_dir->d_name, length);
		    gen_dir->d_name[length] = '\0';
		}

		isodir_reclen = 
		    (unsigned int)tmp_hsg_dir->dir_len;
		tmp_hsg_dir = (struct hsg_dir *)
		    ((unsigned long)tmp_hsg_dir + 
		     isodir_reclen);
		if (tsize - (int)isodir_reclen > 0)
		    /* room for more stuff... dir_len is first byte,
		       so it's guaranteed OK to ref */
		    next_dir_len = tmp_hsg_dir->dir_len;
		else next_dir_len = 0;
	    }				/* switch */

	    isodir_offset += isodir_reclen;
	    tsize -= isodir_reclen;

	    /*
	     * Strip off version number and ending "."
	     * if appropriate. Convert entry name to
	     * lower case.
	     */
	    if (!isrrip && !skip_file)
		cdfs_adjust_dirent_name(cdmntp, gen_dir->d_name,
					&gen_dir->d_namlen);
	    gen_dir->d_reclen = DIRSIZ(gen_dir);

	    if (gendir_resid < gen_dir->d_reclen)
		    panic("secsize_resid < d_reclen");

	    gendir_resid -= gen_dir->d_reclen;

	    if (
		/*
		 * If the there are only empty 
		 * directory records left in this 
		 * sector, pad out to  DIRBLKSIZ bytes.
		 */
		next_dir_len == 0

		/*
		 * If nothing left in this buffer,
		 * pad out to DIRBLKSIZ bytes.
		 */
		|| (tsize <= 0 || (tsize - next_dir_len <= 0))
		/*
		 * If this gen_dir buffer plus the
		 * next directory record will extend
		 * past the DIRBLKSIZ boundary,
		 * pad current gen_dir out to 
		 * DIRBLKSIZ bytes.
		 *
		 * Note - if RRIP names are not in use,
		 * next_dir_len will be bigger than the corresponding
		 * dirent, and this check will be sufficient.
		 * if RRIP names are in use, the NM may be continued in
		 * another block, and the next_dir_len will *NOT* be
		 * big enough.  RRIP handles that in the case above,
		 * by arranging for next_dir_len to be zero,
		 * providing a zero-inode block, and
		 * restarting the directory read with the same entry.
		 */
		 || (((gen_dir_count + gen_dir->d_reclen) % DIRBLKSIZ) +
		     next_dir_len > DIRBLKSIZ)) {
		/*
		 * if any of those conditions hold, do the padding:
		 */
	    flushblock:
		gen_dir->d_reclen += gendir_resid;
	    }

	    CDDEBUG1(READDIRDEBUG,
		     printf("ino %d reclen %d namelen %d\nname %s\n",
			    gen_dir->d_ino, gen_dir->d_reclen,
			    gen_dir->d_namlen, gen_dir->d_name));

	    if (gen_dir_count >= uio->uio_offset) {
		/* if we've gotten into the directory area the user requested,
		   copy this record out */
		error = uiomove(gen_dir, gen_dir->d_reclen, 
				uio);
		gen_dir_count += gen_dir->d_reclen;
		ubuf_size -= gen_dir->d_reclen;
		if (gen_dir_count % DIRBLKSIZ == 0) {
		    dirblkstotransfer--;
		    CDDEBUG1(READDIRDEBUG,
			     printf("dirblkstotransfer = %d tsize = %d ubuf_size = %d\n",
				    dirblkstotransfer, 
				    tsize, ubuf_size));

		}
	    } else
		gen_dir_count += gen_dir->d_reclen;

	} while (error == 0 && dirblkstotransfer > 0 && tsize > 0 &&
		 isodir_reclen && (gen_dir_count % DIRBLKSIZ != 0));
	
	brelse(bp);
	bp = (struct buf *)0;
    } while (error == 0 && ubuf_size && dirblkstotransfer > 0);

 out:

    if (bp)
	brelse(bp);
    ZFREE(cdfsreaddir_zone, gen_dir);
    BM(IN_LOCK(cdp));
    if (isodir_offset >= cdp->cd_size)
	*eofflagp = 1;
    else
	*eofflagp = 0;
    BM(IN_UNLOCK(cdp));
    return (error);
}


/*
 * Return target name of a symbolic link.
 */
cdfs_readlink(vp, uiop, cred)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
{
    register struct cdnode *cdp = VTOCD(vp);
    struct fs *fs = cdp->cd_fs;
    int error;

    if (fs->fs_format != ISO_RRIP)
	return EINVAL;
    /*
     * for RRIP, the symlink is pulled up at vnode activation time,
     * so we just copy it from there.
     */


    IN_LOCK(cdp);
    if (!isdone(cdp, SL)) {
	CDDEBUG1(WEIRDDEBUG,
		 printf("cdfs_readlink: node %d has no link!\n",
			cdp->cd_number));
	IN_UNLOCK(cdp);
	return (EINVAL);
    }
    if (cdp->cd_size > uiop->uio_resid) {
	IN_UNLOCK(cdp);
	return(ERANGE);
    }

    if (cdp->cd_linktarg) {
	error = uiomove(cdp->cd_linktarg, cdp->cd_size, uiop);
    } else {
	error = EIO;			/* link target missing for some
					   error case reason */
    }
    IN_UNLOCK(cdp);
    return error;
}

/*
 * cdfs abort op, called after namei() when a CREATE/DELETE isn't actually
 * done. Iff ni_vp/ni_dvp not null and locked, unlock.
 *
 * not useful in a read-only filesystem such as CDFS :-)
 */
cdfs_abortop(ndp)
	register struct nameidata *ndp;
{
	return 0;
}

/*
 * Get access to bmap
 */
cdfs_bmap(vp, bn, vpp, bnp)
	struct vnode *vp;
	daddr_t bn;
	struct vnode **vpp;
	daddr_t *bnp;
{
	struct cdnode *cdp = VTOCD(vp);
	int datainbuf, offinbuf;
	daddr_t dblkno;

	if (vpp != NULL)
		*vpp = cdp->cd_devvp;
	if (bnp == NULL) {
		*bnp = -1;
		return (0);
	}

	cdfs_ibmap(cdp, bn, &dblkno, &datainbuf, &offinbuf);
	if (offinbuf)
		dblkno += btodb(offinbuf);
	*bnp = dblkno;
	return (0);
}

/*
 * Just call the device strategy routine
 */
cdfs_strategy(bp)
	register struct buf *bp;
{
	register struct cdnode *cdp = VTOCD(bp->b_vp);
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
	if (bp->b_vp->v_type == VBLK || bp->b_vp->v_type == VCHR)
		panic("cdfs_strategy: spec");
	/*
	 * in CDFS, upper levels always pass true block numbers to
	 * bread(), since bmap needs to happen earlier for interleaved
	 * files.  So we just use bp->b_blkno as-is.
	 */
/**** PRS
	if (bp->b_blkno == bp->b_lblkno) {
		if (error = bmap(cdp, bp->b_lblkno, &bp->b_blkno))
			return (error);
 		if (bp->b_blkno == -1)
  			clrbuf(bp);
  	}
*****/
 	if (bp->b_blkno == -1) {
 		biodone(bp);
  		return (0);
 	}
	vp = cdp->cd_devvp;
	bp->b_dev = vp->v_rdev;
	(*(vp->v_op->vn_strategy))(bp);
	return (0);
}

/*
 * Print out the contents of a cdnode.
 */
cdfs_print(vp)
	struct vnode *vp;
{
	register struct cdnode *cdp = VTOCD(vp);

	printf("tag VT_CDFS, ino %d, on dev %d, %d\n", cdp->cd_number,
		major(cdp->cd_dev), minor(cdp->cd_dev));
}


cdfs_page_read(vp, uio, cred)
	struct vnode *vp;
	struct uio *uio;
	struct ucred *cred;
{
	int error;

	error = cdfs_read(vp, uio, 0, cred);
}

cdfs_page_write(vp, uio, cred, pager, offset)
	struct vnode	*vp;
	struct uio 	*uio;
	struct ucred	*cred;
	memory_object_t pager;
	vm_offset_t	offset;
{
	return(EROFS);
}

int cdfs_read_hit = 0;
int cdfs_read_miss = 0;
int cdfs_reada_hit = 0;
int cdfs_reada_miss = 0;

cdfs_rablk(vp, off, len)
	struct vnode *vp;
	vm_offset_t off;
	vm_size_t len;
{
	vm_page_t pp, tpp;
	struct buf *asyncbp;
	struct cdnode *cdp = VTOCD(vp);
	struct fs *fs;
	daddr_t lbn, dblkno;
	int kpcnt, transfers, error;
	struct buf *fbp, *pbp;
	vm_offset_t toff;

	fs = cdp->cd_fs;
	off = trunc_page(off);
	if (len + off >= cdp->cd_size)
		return (0);
	len = MIN((cdp->cd_size - off), len);

	if (CDNODE_LBS(cdp) == PAGE_SIZE) {
		/*
		 * One file block per VM page
		 *
		 * Calling ubc_kluster() with B_CACHE will stop the
		 * search when a page is cached (dirty).
		 */
		pp = ubc_kluster(vp, (vm_page_t) 0, off, 
				 MIN(len, fs->fs_ibsize),
				 B_CACHE|B_WANTFREE|B_BUSY,
				 UBC_HNONE, &kpcnt);
		if (pp) {
			cdfs_reada_miss += kpcnt;
			lbn = off / ISOFS_LBS(fs);
			cdfs_bmap(vp, lbn, 0, &dblkno);
			tpp = pp;
			do {
				(daddr_t) (tpp->pg_pfs) = dblkno;
				dblkno += btodb(PAGE_SIZE);
			} while (tpp = tpp->pg_pnext);
			ubc_bufalloc(pp, kpcnt, &asyncbp,
				     ptoa(kpcnt), 1, B_ASYNC|B_READ);

			asyncbp->b_vp = vp;
			asyncbp->b_dev = (cdp->cd_devvp)->v_rdev;
			asyncbp->b_blkno = (daddr_t) (pp->pg_pfs);

			(*((cdp->cd_devvp)->v_op->vn_strategy))(asyncbp);
		} else {
			cdfs_reada_hit++;
		}
	} else if (CDNODE_LBS(cdp) < PAGE_SIZE) {
		/*
		 * Interleaved file
		 *
		 * Calling ubc_kluster() with B_CACHE will stop the
		 * search when a page is cached (dirty).
		 */
		pp = ubc_kluster(vp, (vm_page_t) 0, off, 
				 PAGE_SIZE, B_CACHE|B_WANTFREE|B_BUSY,
				 UBC_HNONE, &kpcnt);
		if (pp) {
			cdfs_reada_miss++;
			transfers = PAGE_SIZE / CDNODE_LBS(cdp);
			if (transfers <= 0)
				transfers = 1;
			ubc_bufalloc(pp, 1, &asyncbp, CDNODE_LBS(cdp),
				     transfers, B_READ|B_ASYNC);
			fbp = asyncbp;
			toff = off;
			do {
				lbn = toff / ISOFS_LBS(fs);
				cdfs_bmap(vp, lbn, 0, &dblkno);
				toff += CDNODE_LBS(cdp);
				asyncbp->b_vp = vp;
				asyncbp->b_dev = (cdp->cd_devvp)->v_rdev;
				asyncbp->b_blkno = (daddr_t)dblkno;
				pbp = asyncbp;
				asyncbp = asyncbp->b_forw;
				(daddr_t) pp->pg_pfs = fbp->b_blkno;

				(*((cdp->cd_devvp)->v_op->vn_strategy))(pbp);

			} while (--transfers > 0);
		} else {
			cdfs_reada_hit++;
		}
	} else {
		panic("cdfs_rablk: File block size > PAGE_SIZE");
	}
	return (0);
}

cdfs_getpage(vp, offset, len, protp, pl, plsz, ep, addr, ro, cred)
	struct vnode *vp;
	register vm_offset_t offset;
	vm_size_t len;
	vm_prot_t *protp;
	register vm_page_t *pl;
	register int plsz;
	vm_map_entry_t ep;
	vm_offset_t addr;
	int ro;
	struct ucred *cred;
{
	register struct cdnode *cdp = VTOCD(vp);
	struct fs *fs;
	register vm_page_t pp, kpp;
	vm_offset_t off;
	int uflags, error;
	daddr_t lbn, bn, dblkno;
	vm_page_t app, kpl;
	struct buf *bp;
	int kplcnt, transfers, length;
	struct buf *pbp;
	vm_offset_t toff;

	fs = cdp->cd_fs;
	/*
	 * Don't read past the end
	 */
	if (offset + len > cdp->cd_size) {
		if (offset >= cdp->cd_size) {
			*pl = VM_PAGE_NULL;
			if (ep)
				return (KERN_INVALID_ADDRESS);
			return (0);
		}
		len = cdp->cd_size - offset;
	}
	length = len + (offset & page_mask);
	off = trunc_page(offset);

	error = 0;
	for (; length > 0; off += PAGE_SIZE, pl++, length -= PAGE_SIZE) {
		if (off >= cdp->cd_size) {
			*pl = VM_PAGE_NULL;
			break;
		}
		/*
		 * Schedule read-ahead before possibly blocking on
		 * a busy page.
		 */
		if (ubc_incore(vp, off, fs->fs_ibsize)) {
			if (CDNODE_LBS(cdp) == PAGE_SIZE) {
				cdfs_rablk(vp, off + fs->fs_ibsize,
					   fs->fs_ibsize);
			} else {
				cdfs_rablk(vp, off + PAGE_SIZE,
					   PAGE_SIZE);
			}
		}

		uflags = 0;
		error = ubc_lookup(vp, off, PAGE_SIZE, length + off, &app,
				   &uflags);
		if (error) {
			printf("cdfs_getpage: error %d\n", error);
			*pl = VM_PAGE_NULL;
			break;
		}
		pp = app;
		if (uflags & B_NOCACHE) {
			/*
			 * Page not found
			 */
			cdfs_read_miss++;
                        if(ep && current_thread())
                                 current_thread()->thread_events.pageins++;
			lbn = off / ISOFS_LBS(fs);
			cdfs_bmap(vp, lbn, 0, &dblkno);
			(daddr_t) pp->pg_pfs = dblkno;
			/*
			 * Do a kluster read of the remaining
			 * pages if file is not interleaved.
			 */
			if (CDNODE_LBS(cdp) == PAGE_SIZE) {
				kpl = ubc_kluster(vp, pp, off,
						  MIN(length, fs->fs_ibsize),
						  B_CACHE|B_WANTFREE|B_BUSY,
						  UBC_HACP, &kplcnt);
				kpp = kpl;
				do {
					(daddr_t) (kpp->pg_pfs) = dblkno;
					dblkno += btodb(PAGE_SIZE);
				} while (kpp = kpp->pg_pnext);

				ubc_bufalloc(kpl, kplcnt, &bp, ptoa(kplcnt),
					     1, B_READ);
				bp->b_vp = vp;
				bp->b_dev = (cdp->cd_devvp)->v_rdev;
				bp->b_blkno = (daddr_t) (kpl->pg_pfs);

				(*((cdp->cd_devvp)->v_op->vn_strategy))(bp);

				cdfs_rablk(vp, off + fs->fs_ibsize,
					   fs->fs_ibsize);

				ubc_sync_iodone(bp);

				for (kpp = kpl; kpp; kpp = kpp->pg_pnext) {
					*pl++ = kpp;
					if (protp)
						*protp++ = VM_PROT_WRITE;
				}
				*pl = VM_PAGE_NULL;
				return (0);
			}
			/*
			 * The file is interleaved. Several I/O's per
			 * page may have to be performed.
			 */
			transfers = PAGE_SIZE / CDNODE_LBS(cdp);
			if (transfers <= 0)
				transfers = 1;

			ubc_bufalloc(pp, 1, &bp, CDNODE_LBS(cdp),
				     transfers, B_READ);
			toff = off;
			do {
				bp->b_vp = vp;
				bp->b_dev = (cdp->cd_devvp)->v_rdev;
				bp->b_blkno = (daddr_t) dblkno;
				pbp = bp;
				bp = bp->b_forw;

				(*((cdp->cd_devvp)->v_op->vn_strategy))(pbp);

				if ((toff % PAGE_SIZE) == 0) {
					cdfs_rablk(vp, toff + PAGE_SIZE,
						   PAGE_SIZE);
				}
				ubc_sync_iodone(pbp);

				toff += CDNODE_LBS(cdp);
				lbn = toff / ISOFS_LBS(fs);
				cdfs_bmap(vp, lbn, 0, &dblkno);
			} while (--transfers > 0);
		} else {
			/*
			 * Page found
			 */
			cdfs_read_hit++;
		}
		*pl = pp;
		*(pl + 1) = VM_PAGE_NULL;
		if (protp)
			*protp++ = VM_PROT_WRITE;
	}
	return (error);
}

cdfs_putpage(vp, pl, pcnt, flags, cred)
	register struct vnode *vp;
	register vm_page_t *pl;
	register int pcnt;
	int flags;
	struct ucred *cred;
{
	return(EROFS);
}

cdfs_swap(vp, swop, args)
	register struct vnode *vp;
	vp_swap_op_t swop;
	vm_offset_t args;
{
	panic("cdfs_swap: Swapping not supported in CDFS");
}

cdfs_bread(register struct vnode *vp,
	  off_t lbn,
	  struct buf **bpp,
	  struct ucred *cred)
{
	register struct cdnode *cdp;
	register struct fs *fs;
	vm_page_t pl[VP_PAGELIST+1];
	register vm_page_t *pp, lp, fp;
	register struct buf *bp;
	int error;

	cdp = VTOCD(vp);
	fs = cdp->cd_fs;

	error = cdfs_getpage(vp, lbn * fs->fs_ibsize, fs->fs_ibsize,
			    (vm_prot_t *) 0, pl,
			    atop(round_page(fs->fs_ibsize)),
			    (vm_map_entry_t) 0, (vm_offset_t) 0,
			    B_READ, cred);

	if (error) {
		printf("cdfs_bread: cdfs_getpage returned error %d\n", error);
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
	bp_mapin(bp);
	*bpp = bp;
	return (0);
}

cdfs_brelse(register struct vnode *vp,
	   register struct buf *bp)
{
	register vm_page_t pp, cp;

	pp = bp->b_pagelist;
	ubc_buffree(bp);
	do {
		cp = pp;
		pp = pp->pg_pnext;
		ubc_page_release(cp, 0);
	} while (pp != VM_PAGE_NULL);
	return (0);
}

int
cdfs_lockctl(vp, eld, flag, cred, clid, offset)
	struct vnode *vp;
	struct eflock *eld;
	int flag;
	struct ucred *cred;
	int clid;
	off_t offset;
{
	return (EOPNOTSUPP);
}
/*
 * Local Variables:
 * c-indent-level:	8
 * End:
 */
