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
static char *rcsid = "@(#)$RCSfile: nfs_client.c,v $ $Revision: 1.1.8.3 $ (DEC) $Date: 1993/09/22 13:12:28 $";
#endif
/*	@(#)nfs_client.c	2.7 90/07/06 NFSSRC4.1 from 1.13 90/01/22 SMI 	*/

/*
 * Copyright (c) 1987 by Sun Microsystems, Inc.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/ucred.h>
#include <sys/time.h>
#include <sys/buf.h>
#include <sys/stat.h>
#include <rpc/rpc.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <nfs/nfs.h>
#include <nfs/nfs_clnt.h>
#include <nfs/rnode.h>

/*
 * Attributes caching:
 *
 * Attributes are cached in the rnode in struct vattr form.
 * There is a time associated with the cached attributes (r_attrtime)
 * which tells whether the attributes are valid. The time is initialized
 * to the difference between current time and the modify time of the vnode
 * when new attributes are cached. This allows the attributes for
 * files that have changed recently to be timed out sooner than for files
 * that have not changed for a long time. There are minimum and maximum
 * timeout values that can be set per mount point.
 */

/*
 * Validate caches by checking cached attributes. If they have timed out
 * get the attributes from the server and compare mtimes. If mtimes are
 * different purge all caches for this vnode.
 */
nfs_validate_caches(vp, cred)
	struct vnode *vp;
	struct ucred *cred;
{
	struct vattr va;

	return (nfsgetattr(vp, &va, cred));
}

nfs_purge_caches(vp)
	struct vnode *vp;
{
	struct rnode *rp = vtor(vp);

        INVAL_ATTRCACHE(vp);
	cache_purge(vp);
	ubc_invalidate(vp, 0, 0, 0);
	vtor(vp)->r_flags &= ~REOF;
}

int
nfs_cache_check(vp, mtime, fsize)
	struct vnode *vp;
	struct timeval mtime;
	u_int	fsize;

{
	if (!CACHE_VALID(vtor(vp), mtime, fsize)) {
		nfs_purge_caches(vp);
	}
}

/*
 * Set attributes cache for given vnode using nfsattr.
 */
void
nfs_attrcache(vp, na)
	struct vnode *vp;
	struct nfsfattr *na;
{
	if (vtor(vp)->r_flags & RNOCACHE) {
		return;
	}
	nattr_to_vattr(vp, na, &vtor(vp)->r_attr);
	set_attrcache_time(vp);
}

/*
 * Set attributes cache for given vnode using vnode attributes.
 */
void
nfs_attrcache_va(vp, va)
	struct vnode *vp;
	struct vattr *va;
{
	struct rnode *rp = vtor(vp);

	if (rp->r_flags & RNOCACHE) {
		return;
	}
	/*
	 * If the attributes have changed, then invalidate our idea of
	 * the symbolic link target in case this is a symbolic link.
	 */
	if (bcmp(&rp->r_attr.va_mtime, &va->va_mtime, sizeof(va->va_mtime)))
		rtol(rp)[0] = 0;
	rp->r_attr = *va;
	vp->v_type = va->va_type;
	set_attrcache_time(vp);
}

set_attrcache_time(vp)
	struct vnode *vp;
{
	struct rnode *rp = vtor(vp);
	int delta;

	rp->r_attrtime = time;
	/*
	 * Delta is the number of seconds that we will cache
	 * attributes of the file.  It is based on the number of seconds
	 * since the last change (i.e. files that changed recently
	 * are likely to change soon), but there is a minimum and
	 * a maximum for regular files and for directories.
	 */
	delta = (time.tv_sec - rp->r_attr.va_mtime.tv_sec) >> 4;
	if (vp->v_type == VDIR) {
		if (delta < vtomi(vp)->mi_acdirmin) {
			delta = vtomi(vp)->mi_acdirmin;
		} else if (delta > vtomi(vp)->mi_acdirmax) {
			delta = vtomi(vp)->mi_acdirmax;
		}
	} else {
		if (delta < vtomi(vp)->mi_acregmin) {
			delta = vtomi(vp)->mi_acregmin;
		} else if (delta > vtomi(vp)->mi_acregmax) {
			delta = vtomi(vp)->mi_acregmax;
		}
	}
	rp->r_attrtime.tv_sec += delta;
}

/*
 * Fill in attribute from the cache. If valid return 1 otherwise 0;
 */
int
nfs_getattr_cache(vp, vap)
	struct vnode *vp;
	struct vattr *vap;
{
	struct rnode *rp = vtor(vp);

	if (time.tv_sec < rp->r_attrtime.tv_sec) {
		/*
		 * Cached attributes are valid
		 */
		*vap = rp->r_attr;
		return (1);
	}
	return (0);
}

/*
 * Get attributes over-the-wire.
 * Return 0 if successful, otherwise error.
 */
int
nfs_getattr_otw(vp, vap, cred)
	struct vnode *vp;
	struct vattr *vap;
	struct ucred *cred;
{
	int error;
	struct nfsattrstat *ns;

	ns = (struct nfsattrstat *)kmem_alloc(sizeof (*ns));

	error = rfscall(vtomi(vp), RFS_GETATTR, xdr_fhandle,
	    (caddr_t)vtofh(vp), xdr_attrstat, (caddr_t)ns, cred);

	if (error == 0) {
		error = geterrno(ns->ns_status);
		if (error == 0) {
			nattr_to_vattr(vp, &ns->ns_attr, vap);
		} else {
			PURGE_STALE_FH(error, vp);
		}
	}
	kmem_free((caddr_t)ns, sizeof (*ns));

	return (error);
}

/*
 * Return either cached ot remote attributes. If get remote attr
 * use them to check and invalidate caches, then cache the new attributes.
 */
int
nfsgetattr(vp, vap, cred)
	struct vnode *vp;
	struct vattr *vap;
	struct ucred *cred;
{
	int error;

	if (nfs_getattr_cache(vp, vap)) {
		/*
		 * got cached attributes, we're done.
		 */
		error = 0;
	} else {
		error = nfs_getattr_otw(vp, vap, cred);
		if (error == 0) {
			nfs_cache_check(vp, vap->va_mtime, (u_int)vap->va_size);
			nfs_attrcache_va(vp, vap);
		}
	}
	/* Return the client's view of file size */
	vap->va_size = vtor(vp)->r_size;

	return (error);
}

nattr_to_vattr(vp, na, vap)
	register struct vnode *vp;
	register struct nfsfattr *na;
	register struct vattr *vap;
{
	struct rnode *rp = vtor(vp);

	if (bcmp(&vap->va_mtime, &na->na_mtime, sizeof(na->na_mtime)))
		rtol(rp)[0] = 0; /* Junk old link info, if any */
	vap->va_type = (enum vtype)na->na_type;
	vap->va_mode = na->na_mode;
	vap->va_nlink = na->na_nlink;
	vap->va_uid = na->na_uid;
	vap->va_gid = na->na_gid;
	vap->va_size = na->na_size;   /* keep for cache validation	*/
	if (rp->r_size < na->na_size || ((rp->r_flags & RDIRTY) == 0))
		rp->r_size = na->na_size; /* Client's view of file size */
	switch(na->na_type) {
	case NFBLK:
		vap->va_blocksize = BLKDEV_IOSIZE;
		break;
	case NFCHR:
		vap->va_blocksize = MAXBSIZE;
		break;
	default:
		vap->va_blocksize = na->na_blocksize;
		break;
	}
	vap->va_rdev = na->na_rdev;
	vap->va_bytes = na->na_blocks * 512;
	vap->va_fsid = vp->v_mount->m_stat.f_fsid.val[0];
	vap->va_fileid = na->na_nodeid;
	vap->va_gen = na->na_ctime.tv_usec;
	vap->va_atime.tv_sec  = na->na_atime.tv_sec;
	vap->va_atime.tv_usec = na->na_atime.tv_usec;
	vap->va_mtime.tv_sec  = na->na_mtime.tv_sec;
	vap->va_mtime.tv_usec = na->na_mtime.tv_usec;
	vap->va_ctime.tv_sec  = na->na_ctime.tv_sec;
	vap->va_ctime.tv_usec = na->na_ctime.tv_usec;
	/*
	 * This bit of ugliness is a *TEMPORARY* hack to preserve the
	 * over-the-wire protocols for named-pipe vnodes.  It remaps the
	 * special over-the-wire type to the VFIFO type. (see note in nfs.h)
	 *
	 * BUYER BEWARE:
	 *  If you are porting the NFS to a non-SUN server, you probably
	 *  don't want to include the following block of code.  The
	 *  over-the-wire special file types will be changing with the
	 *  NFS Protocol Revision.
	 */
	if (NA_ISFIFO(na)) {
		vap->va_type = VFIFO;
		vap->va_mode = (vap->va_mode & ~S_IFMT) | S_IFIFO;
		vap->va_rdev = 0;
		vap->va_blocksize = na->na_blocksize;
	}
}
