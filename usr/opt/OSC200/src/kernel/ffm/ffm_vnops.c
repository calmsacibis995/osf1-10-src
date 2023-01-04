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
static char *rcsid = "@(#)$RCSfile: ffm_vnops.c,v $ $Revision: 1.1.4.2 $ (DEC) $Date: 1993/05/12 19:28:11 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
 */
/*
 * OSF/1 1.1
 */

#include <sys/secdefines.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/buf.h>
#include <vm/vm_page.h>
#include <vm/vm_vppage.h>
#include <sys/vfs_ubc.h>
#include <vm/vm_mmap.h>
#include <vm/vm_vp.h>
#include <vm/vm_debug.h>

#include <ffm/ffmmount.h>

/*
  This source contains the implementation of the pass-through
  operations for file-on-file mounts.  These operations are not
  used for character devices, which are handled specially via
  clone operations (see spec_vnops.c).
 
  These operations primarily act by loading the vnode of the
  underlying filesystem and calling the same operation using that
  vnode.

  It is believed that many of the normal operations cannot be
  executed due to the way that this filesystem works.  For example,
  since lookup returns the vnode of the underlying filesystem, any
  operation which does a lookup first will not result in an operation
  happening with an ffm_xxx operation.  The only operations needed
  here are those which can happen directly to a file or directory,
  i.e. open(), readdir(), etc.

*/

/*
 * Global vfs data structures for ffm
 */

int	ffm_lookup(),
	ffm_open(),
	ffm_close(),
	ffm_access(),
	ffm_getattr(),
	ffm_setattr(),
	ffm_read(),
	ffm_write(),
	ffm_ioctl(),
	ffm_select(),
	ffm_mmap(),
	ffm_fsync(),
	ffm_seek(),
	ffm_readdir(),
	ffm_inactive(),
	ffm_reclaim(),
	ffm_print(),
	ffm_page_read(),
	ffm_page_write(),
	ffm_getpage(),
	ffm_putpage(),
	ffm_swap(),
	ffm_bread(),
	ffm_brelse(),
	ffm_lockctl(),
	ffm_syncdata(),
	ffm_warn();

struct vnodeops ffm_vnodeops = {
	ffm_lookup,		/* lookup */
	ffm_warn,		/* create */
	ffm_warn,		/* mknod */
	ffm_open,		/* open */
	ffm_close,		/* close */
	ffm_access,		/* access */
	ffm_getattr,		/* getattr */
	ffm_setattr,		/* setattr */
	ffm_read,		/* read */
	ffm_write,		/* write */
	ffm_ioctl,		/* ioctl */
	ffm_select,		/* select */
	ffm_mmap,		/* mmap */
	ffm_fsync,		/* fsync */
	ffm_seek,		/* seek */
	ffm_warn,		/* remove */
	ffm_warn,		/* link */
	ffm_warn,		/* rename */
	ffm_warn,		/* mkdir */
	ffm_warn,		/* rmdir */
	ffm_warn,		/* symlink */
	ffm_readdir,		/* readdir */
	ffm_warn,		/* readlink */
	ffm_warn,		/* abortop */
	ffm_inactive,		/* inactive */
	ffm_reclaim,		/* reclaim */
	ffm_warn,		/* bmap */
	ffm_warn,		/* strategy */
	ffm_print,		/* print */
	ffm_page_read,		/* page_read */
	ffm_page_write,		/* page_write */
	ffm_getpage,		/* getpage */
	ffm_putpage,		/* putpage */
	ffm_swap,		/* swap */
	ffm_bread,		/* bread */
	ffm_brelse,		/* brelse */
	ffm_lockctl,		/* lockctl */
	ffm_syncdata,		/* syncdata */
};

/*
 * For now, we don't allow FFS mounts of directories, so if we
 * get called for a lookup, we can be guaranteed that it's an
 * error.  The vnode better not be type VDIR.
 *
 * This will change in a later release when directory mounts are 
 * allowed.
 */
ffm_lookup(vp, ndp)
	struct vnode *vp;
	register struct nameidata *ndp;
{
	ASSERT(vp->v_type != VDIR);
	ndp->ni_dvp = vp;
	ndp->ni_vp = NULLVP;
	return (ENOTDIR);

#if	0
	int error, release;
	struct vnode *svp;

	svp = VTOF(vp)->fn_shadow;
	VREF(svp);
	if (!strcmp(ndp->ni_dent.d_name, ".") &&
	    ndp->ni_next < (ndp->ni_pnbuf + ndp->ni_pathlen)) {
		VUNREF(svp);
		ndp->ni_vp = vp;
		ndp->ni_dvp = vp;
		VREF(vp);	/* reference parent */
		return(0);
	}
	VOP_LOOKUP(svp, ndp, error);
	/*
	 * the vrele done on the parent directory in namei will be
	 * on the shadow, so we need to release the reference on the
	 * ffm root vnode gained in ffm_root here, but only if certain
	 * errors have not occured; namei will release it in some cases.
	 */
	release = 1;
	if (error) {
		int flag = ndp->ni_nameiop & OPFLAG;
		if (flag == LOOKUP || flag == DELETE ||
			error != ENOENT || *ndp->ni_next != 0)
			release = 0;
		BM(MOUNT_LOCK(svp->v_mount));
		if (svp->v_mount->m_flag & M_RDONLY)
			release = 0;
		BM(MOUNT_UNLOCK(svp->v_mount));
	}
	if (release)
		vrele(vp);
	/*
	 * We're effectively crossing a mount point.  Previously,
	 * we needed to release the mount lookup lock on the file
	 * system we're leaving and gain the lock on the new
	 * filesystem.  The reason for that was that namei() simply
	 * released the current vnode -> v_mount lock when crossing
	 * mount points and when returning.
	 *
	 * This is no longer necessary because namei now tracks the
	 * last mount structure it locked and unlocks that one when
	 * crossing a mount point.  The reason we don't get in trouble
	 * here is that our reference on the new filesystem prevents
	 * unmounts from succeeding (other than forcible, and that
	 * won't work ... yet).  So there's no need to take the lock.
	 * Namei() will release the correct one when it either completes
	 * or crosses yet another mount point.
	 */
	return(error);
#endif	/* 0 */
}

ffm_open(vpp, mode, cred)
	struct vnode **vpp;
	int mode;
	struct ucred *cred;
{
	return(0);
}

ffm_close(vp, fflag, cred)
	struct vnode *vp;
	int fflag;
	struct ucred *cred;
{
	return(0);
}

ffm_access(vp, mode, cred)
	struct vnode *vp;
	int mode;
	struct ucred *cred;
{
	int error;

	VOP_ACCESS(VTOF(vp)->fn_shadow, mode, cred, error);
	return(error);
}

ffm_getattr(vp, vap, cred)
	struct vnode *vp;
	register struct vattr *vap;
	struct ucred *cred;
{
	int error;

	VOP_GETATTR(VTOF(vp)->fn_shadow, vap, cred, error);
	return(error);
}

ffm_setattr(vp, vap, cred)
	register struct vnode *vp;
	register struct vattr *vap;
	register struct ucred *cred;
{
	int error;

	VOP_SETATTR(VTOF(vp)->fn_shadow, vap, cred, error);
	return(error);
}

ffm_read(vp, uio, ioflag, cred)
	struct vnode *vp;
	register struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	int error;

	VOP_READ(VTOF(vp)->fn_shadow, uio, ioflag, cred, error);
	return(error);
}

ffm_write(vp, uio, ioflag, cred)
	register struct vnode *vp;
	struct uio *uio;
	int ioflag;
	struct ucred *cred;
{
	int error;

	VOP_WRITE(VTOF(vp)->fn_shadow, uio, ioflag, cred, error);
	return(error);
}

ffm_ioctl(vp, com, data, fflag, cred)
	struct vnode *vp;
	int com;
	caddr_t data;
	int fflag;
	struct ucred *cred;
{
	int error;
	int retval;

	VOP_IOCTL(VTOF(vp)->fn_shadow, com, data, fflag, cred, error, &retval);
	return(error);
}

ffm_select(vp, events, revents, scanning, cred)
	struct vnode *vp;
	short *events, *revents;
	int scanning;
	struct ucred *cred;
{
	int error;

	VOP_SELECT(VTOF(vp)->fn_shadow, events, revents, scanning, cred, error);
	return(error);
}

ffm_mmap(vp,offmet,map,addrp,len,prot,maxprot,flags,cred)
	register struct vnode *vp;
	vm_offset_t offmet;
	vm_map_t map;
	vm_offset_t *addrp;
	vm_size_t len;
	vm_prot_t prot;
	vm_prot_t maxprot;
	int flags;
	struct ucred *cred;
{
	int error;

	VOP_MMAP(VTOF(vp)->fn_shadow, offmet, map, addrp, len, prot, maxprot, flags, cred, error);
	return(error);
}

ffm_fsync(vp, fflags, cred, waitfor)
	struct vnode *vp;
	int fflags;
	struct ucred *cred;
	int waitfor;
{
	int error;

	VOP_FSYNC(VTOF(vp)->fn_shadow, fflags, cred, waitfor, error);
	return(error);
}

ffm_seek(vp, oldoff, newoff, cred)
	struct vnode *vp;
	off_t oldoff, newoff;
	struct ucred *cred;
{
	int error;

	VOP_SEEK(VTOF(vp)->fn_shadow, oldoff, newoff, cred, error);
	return(error);
}

ffm_readdir(vp, uiop, cred, eofflagp)
	struct vnode *vp;
	struct uio *uiop;
	struct ucred *cred;
	int *eofflagp;
{
	int error;

	VOP_READDIR(VTOF(vp)->fn_shadow, uiop, cred, eofflagp, error);
	return(error);
}

ffm_print(vp)
	struct vnode *vp;
{
	int error;

	VOP_PRINT(VTOF(vp)->fn_shadow, error);
	return(error);
}

ffm_page_read(vp, uio, cred)
	struct vnode *vp;
	struct uio *uio;
	struct ucred *cred;
{
	int error;

	VOP_PGRD(VTOF(vp)->fn_shadow, uio, cred, error);
	return(error);
}

ffm_page_write(vp, uio, cred, async_args, async_func)
	struct vnode	*vp;
	struct uio 	*uio;
	struct ucred	*cred;
	void 		*async_args;
	void		(*async_func)();
{
	int error;

	VOP_PGWR(VTOF(vp)->fn_shadow, uio, cred, async_args, async_func, error);
	return(error);
}

ffm_inactive(vp)
	struct vnode *vp;
{
	struct ffmmount *fmp;
	struct ffm_node *fsp;

	fmp = VFSTOFFM(vp->v_mount);
	fsp = VTOF(fmp->fm_vp);
	vrele(fsp->fn_shadow);
	return(0);
}

ffm_reclaim(vp)
	register struct vnode *vp;
{
	return(0);
}

ffm_getpage(vp,offmet,len,protp,pl,plsz,ep,addr,rwflg,cred)
	register struct vnode *vp;
        vm_offset_t offmet;
	vm_size_t len;
	vm_prot_t *protp;
	vm_page_t *pl;
	int plsz;
	vm_map_entry_t ep;
	vm_offset_t addr;
	int rwflg;
	struct ucred *cred;
{
	int error;

	VOP_GETPAGE(VTOF(vp)->fn_shadow,offmet,len,protp,pl,plsz,ep,addr,rwflg,cred,error);
	return(error);
}

ffm_putpage(vp,pl,pcnt,flags,cred)
	register struct vnode *vp;
	register vm_page_t *pl;
	register int pcnt;
	int flags;
	struct ucred *cred;
{
	int error;

	VOP_PUTPAGE(VTOF(vp)->fn_shadow,pl,pcnt,flags,cred,error);
	return(error);
}

ffm_swap(vp,swop,args,cred)
	register struct vnode *vp;
	vp_swap_op_t swop;
	vm_offset_t args;
	struct ucred *cred;
{
	int error;

	VOP_SWAP(VTOF(vp)->fn_shadow,swop,args,cred,error);
	return(error);
}

ffm_bread(vp,lbn,bpp,cred)
	register struct vnode *vp;
	off_t lbn;
	struct buf **bpp;
        struct ucred *cred;
{
	int error;

	VOP_BREAD(VTOF(vp)->fn_shadow,lbn,bpp,cred,error);
	return(error);
}

ffm_brelse(vp,bp)
	register struct vnode *vp;
	struct buf *bp;
{
	int error;

	VOP_BRELSE(VTOF(vp)->fn_shadow,bp,error);
	return(error);
}

ffm_lockctl(vp, eld, flag, cred, clid, offmet)
        register struct vnode *vp;
	struct eflock *eld;
	int flag;
	struct ucred *cred;
	pid_t clid;
	off_t offmet;
{
	int error;

	VOP_LOCKCTL(VTOF(vp)->fn_shadow,eld, flag, cred, clid, offmet, error);
	return(error);
}

ffm_syncdata(vp, fflags, start, length, cred)
	struct vnode *vp;
	int fflags;
	vm_offset_t start;
	vm_size_t length;
	struct ucred *cred;
{
	int error;

	VOP_SYNCDATA(VTOF(vp)->fn_shadow,fflags, start, length, cred, error);
	return(error);
}



int
ffm_warn(vp)
	struct vnode *vp;
{
	printf("ffm: unimplemented operation called\n");
	return(0);
}

#if SEC_FSCHANGE

int	ffm_getsecattr(),
	ffm_setsecattr(),
	ffm_dirempty();

struct vnsecops ffm_secops = {
	ffm_getsecattr,	/* VOP_GETSECATTR function */
	ffm_setsecattr,	/* VOP_SETSECATTR function */
	ffm_dirempty	/* VOP_DIREMPTY   function */
};

ffm_getsecattr(vp, vsap, cred)
	register struct vnode		*vp;
	register struct vsecattr	*vsap;
	struct ucred			*cred;
{
	int error;

	VOP_GETSECATTR(VTOF(vp)->fn_shadow, vsap, cred, error);
	return(error);
}

ffm_setsecattr(vp, vsap, cred)
	register struct vnode		*vp;
	register struct vsecattr	*vsap;
	struct ucred			*cred;
{
	int error;

	VOP_SETSECATTR(VTOF(vp)->fn_shadow, vsap, cred, error);
	return(error);
}

ffm_dirempty(vp, dvp, cred)
	struct vnode	*vp;	/* directory to be tested */
	struct vnode	*dvp;	/* parent of directory to be tested */
	struct ucred	*cred;	/* caller's credentials */
{
	int error;

	VOP_DIREMPTY(VTOF(vp)->fn_shadow, dvp, cred, error);
	return(error);
}

#endif /* SEC_FSCHANGE */
