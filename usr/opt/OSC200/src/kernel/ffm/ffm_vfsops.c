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
static char *rcsid = "@(#)$RCSfile: ffm_vfsops.c,v $ $Revision: 1.1.4.7 $ (DEC) $Date: 1994/01/11 18:22:23 $";
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
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/user.h>
#include <sys/specdev.h>
#include <vm/vm_kern.h>

#include <ffm/ffmmount.h>

/*
   this module provides support for the vfs operations
   of the file-on-file mount filesystem type.

   The most interesting operation is ffm_mount(), which
   establishes the relationships between the original
   vnode and the mounted-on vnode.  It takes in an ffm_args,
   which allows either a file descriptor or a pathname
   to be passed.  It also determines (based on the FFM_CLONE
   flag) whether a clone vnode should be created or whether
   a special pass-thru vnode should be created. 

*/
/*
 * ffm vfs operations.
 */
int ffm_mount();
int ffm_start();
int ffm_unmount();
int ffm_root();
int ffm_quotactl();
int ffm_statfs();
int ffm_sync();
int ffm_fhtovp();
int ffm_vptofh();
int ffm_init();
int ffm_mountroot();
int ffm_swapvp();

struct vfsops ffm_vfsops = {
	ffm_mount,
	ffm_start,
	ffm_unmount,
	ffm_root,
	ffm_quotactl,
	ffm_statfs,
	ffm_sync,
	ffm_fhtovp,
	ffm_vptofh,
	ffm_init,
	ffm_mountroot,
	ffm_swapvp
};

/*
 * ffm mounts are not kept on a list of their own.  The FFM_MOUNTTAB_LOCK
 * protects only the following two fields.  The only way to access
 * a struct ffmmount is through the global mount list itself.
 */
#define MAX_FFMMOUNTS	1000
udecl_simple_lock_data(,ffm_mounttab_lock)
#define FFM_MOUNTTAB_LOCK()	usimple_lock(&ffm_mounttab_lock);
#define FFM_MOUNTTAB_UNLOCK()	usimple_unlock(&ffm_mounttab_lock);
#define FFM_MOUNTTAB_LOCK_INIT() usimple_lock_init(&ffm_mounttab_lock);

long total_ffmmounts = 0;
long max_ffmmounts = MAX_FFMMOUNTS;

/*
  mount operation
*/
static char ffm_mntfromname[] = "file-on-file mount";

ffm_mount(mp, path, data, ndp)
	register struct mount *mp;
	char *path;
	caddr_t data;
	struct nameidata *ndp;
{
	struct ffm_args args;
	struct ffmmount *fmp = NULL;
	struct file *fp;
	struct vnode *vp, *cvp, *newvp = NULL;
	struct ffm_node *ffp;
	int error, ignore_error;
	int ronly = (mp->m_flag & M_RDONLY) != 0;
	u_int size;
	void * privatep;
	extern struct vnodeops ffm_vnodeops;
	struct vattr vattr;

	/*
	 * check to see if we're covering a root -- not allowed.
	 * It is, however, legal to cover referenced vnodes.
	 */
	cvp = mp->m_vnodecovered;
	ASSERT(cvp != NULLVP);
	if (cvp->v_flag & VROOT)
		return(EBUSY);

	if (mp->m_flag & M_UPDATE)
		return (0);

	/*
	 * only allow mounts onto regular files and directories
	 */
	if (cvp->v_type != VREG && cvp->v_type != VDIR)
	        return(EINVAL);

	/* 
	 * any user may mount this filesystem type,
	 * but it requires write access 
	 */
	VOP_ACCESS(mp->m_vnodecovered, VWRITE, u.u_cred, error);

	if (error)
		return(error);

	VOP_GETATTR(mp->m_vnodecovered, &vattr, u.u_cred, error);
	if (error)
		return(error);

	if (u.u_cred->cr_uid != vattr.va_uid) {
		error = suser(u.u_cred,&u.u_acflag);
		if (error)
			return(error);
	}

	if (error = copyin(data, (caddr_t)&args, sizeof (struct ffm_args)))
		return (error);

	/*
	 * If it is a mount on an existing open file (FFM_FD), get
	 * the vnode from the file descriptor passed in; otherwise,
	 * lookup up the pathname passed.
	 */
	if (args.ffm_flags & FFM_FD) {
		if (error = getf(&fp, args.ffm_filedesc, 
				 FILE_FLAGS_NULL, &u.u_file_state))
			return (error);
		if ((fp->f_type != DTYPE_VNODE)) {
			error = EINVAL;
			FP_UNREF(fp);
			return(error);
		}
		vp = (struct vnode *) fp->f_data;
		VREF(vp);
		FP_UNREF(fp);
	} else {
		/* lookup the name */
		ndp->ni_nameiop = LOOKUP | FOLLOW;
		ndp->ni_segflg = UIO_USERSPACE;
		ndp->ni_dirp = args.ffm_pathname;
		if (error = namei(ndp)) {
			/* ENOENT needs translation */
			return (error == ENOENT ? ENODEV : error);
		}
		vp = ndp->ni_vp;
	}
	/*
	 * Check for invalid arguments:
	 *	1) We don't allow mounting directories (yet).
	 *	2) FFM_CLONE is only allowed for character special files.
	 */
	if ((vp->v_type == VDIR) ||
	    (vp->v_type != VCHR) && (args.ffm_flags & FFM_CLONE)) {
		error = EINVAL;
		goto out3;
	}

	fmp = (struct ffmmount *)kalloc(sizeof(struct ffmmount));
	FFM_MOUNTTAB_LOCK();
	if ((total_ffmmounts++ >= max_ffmmounts) || 
	    (fmp == (struct ffmmount *) 0)) {
		FFM_MOUNTTAB_UNLOCK();
		error = EMFILE;		/* needs translation */
		goto out2;
	}
	FFM_MOUNTTAB_UNLOCK();
	bzero((caddr_t)fmp, sizeof(struct ffmmount));

	if (!(args.ffm_flags & FFM_FD)) {
		VOP_OPEN(&vp, ronly ? FREAD : FREAD|FWRITE, NOCRED, error);
		if (error)
			goto out2;
	}
	if (args.ffm_flags & FFM_CLONE) {
		newvp = vp;
		error = spec_clone(&newvp,  0, u.u_cred, 
				   &privatep, SPEC_DONTOPEN);
		if (error)
			goto out1;
		if (!(args.ffm_flags & FFM_FD)) {
                        /*
                         * we've now got one too many open sa_usecount
                         * references on this alias.  Get rid of one.
                         */
                        VOP_CLOSE(vp, ronly ? FREAD : FREAD|FWRITE,
                                  NOCRED, error);
		}
	} else {
		if (getnewvnode(VT_FFM, &ffm_vnodeops, &newvp)) {
			error = EMFILE;
			goto out1;
		}
		newvp->v_type = vp->v_type;
#if	SEC_FSCHANGE
		/*
		 * If the underlying vnode has secure ops, attach
		 * ours, otherwise, copy the mount point tags.
		 */
		if (VHASSECOPS(vp)) {
			newvp->v_secop = &ffm_secops;
			mp->m_flag |= M_SECURE;
		} else {
			MOUNT_LOCK(vp->v_mount);
			ASSERT(!(vp->v_mount->m_flag & M_SECURE));
			ASSERT(newvp->v_secop == 0);
			bcopy(vp->v_mount->m_tag, mp->m_tag,sizeof(mp->m_tag));
			MOUNT_UNLOCK(vp->v_mount);
		}
#endif /* SEC_FSCHANGE */
		ffp = VTOF(newvp);
		ffp->fn_shadow = vp;
		ffp->fn_vnode = newvp;
	}
	if (!(args.ffm_flags & FFM_CLONE)
	    && ((vp->v_type == VBLK) || (vp->v_type == VCHR)) ) {
		/*
                 * specinfo already allocated for CLONE case
                 */
                (void) specalloc(newvp, vp->v_rdev);
		/* now setup the alias */
		makealias(newvp);
#define v_alias v_specinfo->si_alias
		newvp->v_alias->sa_private = vp->v_alias->sa_private;
                if (args.ffm_flags & FFM_FD) {
                        spec_setopen(vp);
                        VUNREF(vp); /* account for extra increment */
                }
	}
	newvp->v_flag |= VROOT;
	newvp->v_mount = mp;
	fmp->fm_mountp = mp;
	fmp->fm_vp = newvp;
	mp->m_data = (qaddr_t)fmp;
	if (args.ffm_flags & FFM_CLONE) {
		mp->m_stat.f_fsid.val[0] = newvp->v_rdev;
		fmp->fm_flags = FFM_CLONE;
	}
	else {
		mp->m_stat.f_fsid.val[0] = (int)newvp;
		fmp->fm_flags = 0;
	}
	if (ronly)
		fmp->fm_flags |= FFM_RDONLY;
	mp->m_stat.f_fsid.val[1] = MOUNT_FFM;
	mp->m_flag |= M_LOCAL;
	insmntque(newvp, mp);

	/* fill in (dummy) statfs information */
	(void) ffm_statfs(mp);
	(void) copyinstr(path, mp->m_stat.f_mntonname, MNAMELEN-1, &size);
	mp->m_stat.f_mntonname[size] = 0;
	if (args.ffm_flags & FFM_FD) {
		size = sizeof(ffm_mntfromname);
		bcopy(ffm_mntfromname, mp->m_stat.f_mntfromname, size);
	} else {
		(void) copyinstr(args.ffm_pathname, mp->m_stat.f_mntfromname,
				 MNAMELEN-1, &size);
		mp->m_stat.f_mntfromname[size] = 0;
	}
	/* Tell Streams about this so it can do its SVVS thing. */
	if ((vp->v_type == VBLK) || (vp->v_type == VCHR)) {
		struct vnode *tvp = newvp;
		int err = spec_ioctl(vp, FIOFATTACH, (caddr_t)&tvp,
					FKERNEL, NOCRED, &ignore_error);
	}
	return (0);

out1:
	if (!(args.ffm_flags & FFM_FD))
		VOP_CLOSE(vp, 0, NOCRED, ignore_error);
out2:
	FFM_MOUNTTAB_LOCK();
	total_ffmmounts--;
	FFM_MOUNTTAB_UNLOCK();
	if (fmp)
		kfree(fmp, sizeof(struct ffmmount));
out3:
	vrele(vp);
	return(error);
}

ffm_start(mp, flags)
	struct mount *mp;
	int flags;
{

	return (0);
}

ffm_unmount(mp, mntflags)
	struct mount *mp;
	int mntflags;
{
	struct ffmmount *fmp;
	int error;
	int flags = 0;
	int ronly = 0;
	int retval;
	struct ffm_node *ffp;
	struct vattr vattr;

	if (mntflags & MNT_FORCE)
		return (EINVAL);
	if (mntflags & MNT_FORCE)
		flags |= FORCECLOSE;

	VOP_ACCESS(mp->m_vnodecovered, VWRITE, u.u_cred, error);

	if (error)
		return(error);

	VOP_GETATTR(mp->m_vnodecovered, &vattr, u.u_cred, error);
	if (error)
		return(error);

	/*
	 * Only the owner or superuser can unmount.
	 */
	if (u.u_cred->cr_uid != vattr.va_uid) {
		error = suser(u.u_cred,&u.u_acflag);
		if (error)
			return(error);
	}

	fmp = VFSTOFFM(mp);
	ronly = fmp->fm_flags & FFM_RDONLY;
	ffp = VTOF(fmp->fm_vp);
	/* Tell Streams about this so it can do its SVVS thing. */
	{
		struct vnode *vp, *tvp;
		int ignore_error;
		tvp = fmp->fm_vp;
		vp = (fmp->fm_flags & FFM_CLONE) ? fmp->fm_vp : ffp->fn_shadow;
		if ((vp->v_type == VBLK) || (vp->v_type == VCHR))
			(void) spec_ioctl(vp, FIOFDETACH, (caddr_t)&tvp,
					FKERNEL, NOCRED, &retval);
	}
	if (fmp->fm_flags & FFM_CLONE)
		VOP_CLOSE(fmp->fm_vp, ronly ? FREAD : FREAD|FWRITE, 
			  NOCRED, error);
	else
		VOP_CLOSE(ffp->fn_shadow, ronly ? FREAD : FREAD|FWRITE, 
			  NOCRED, error);
	vrele(fmp->fm_vp);

	insmntque(fmp->fm_vp, DEADMOUNT);

	/* free ffmmount */
	kfree(fmp, sizeof(struct ffmmount));
	FFM_MOUNTTAB_LOCK();

	total_ffmmounts--;
	FFM_MOUNTTAB_UNLOCK();
	MOUNT_LOCK(mp);
	mp->m_flag &= ~M_LOCAL;
	MOUNT_UNLOCK(mp);
	return(error);
}

ffm_root(mp, vpp)
	struct mount *mp;
	struct vnode **vpp;
{
	struct ffmmount *fmp;

	fmp = VFSTOFFM(mp);
	VREF(fmp->fm_vp);
	*vpp = fmp->fm_vp;
	return(0);
}

ffm_quotactl(mp, cmds, uid, arg)
	struct mount *mp;
	int cmds;
	uid_t uid;
	caddr_t arg;
{
	struct ffmmount *fmp;
	struct ffm_node *fsp;
	int error;
	
	fmp = VFSTOFFM(mp);
	fsp = VTOF(fmp->fm_vp);
	VFS_QUOTACTL(fsp->fn_shadow->v_mount, cmds, uid, arg, error);
	return(error);
}

ffm_statfs(mp)
	struct mount *mp;
{
	struct ffmmount *fmp;
	struct ffm_node *fsp;
	struct mount *mp2;
	int error;
	
	fmp = VFSTOFFM(mp);
	if (fmp->fm_flags & FFM_CLONE) {
		MOUNT_LOCK(mp);
		mp->m_stat.f_type = MOUNT_FFM;
		mp->m_stat.f_flags = 0;
		mp->m_stat.f_fsize = 8192;
		mp->m_stat.f_bsize = mp->m_stat.f_fsize;
		mp->m_stat.f_blocks = 0;
		mp->m_stat.f_bfree = 0;
		mp->m_stat.f_bavail = 0;
		mp->m_stat.f_files = 1;
		mp->m_stat.f_ffree = 0;
		MOUNT_UNLOCK(mp);
		return(EINVAL);
	}
	fsp = VTOF(fmp->fm_vp);
	mp2 = fsp->fn_shadow->v_mount;
	VFS_STATFS(mp2, error);
	if (!error) {
		MOUNT_LOCK(mp);
		mp->m_stat.f_type = MOUNT_FFM;
		mp->m_stat.f_fsize = mp2->m_stat.f_fsize;
		mp->m_stat.f_bsize = mp2->m_stat.f_bsize;
		mp->m_stat.f_blocks = mp2->m_stat.f_blocks;
		mp->m_stat.f_bfree = mp2->m_stat.f_bfree;
		mp->m_stat.f_bavail = mp2->m_stat.f_bavail;
		mp->m_stat.f_files = mp2->m_stat.f_files;
		mp->m_stat.f_ffree = mp2->m_stat.f_ffree;
		MOUNT_UNLOCK(mp);
	}
	return(error);
}

ffm_sync(mp, waitfor)
	struct mount *mp;
	int waitfor;
{
	return(0);
}

ffm_fhtovp(mp, fhp, vpp)
	register struct mount *mp;
	struct fid *fhp;
	struct vnode **vpp;
{
	return(EINVAL);
}

ffm_vptofh(vp, fhp)
	struct vnode *vp;
	struct fid *fhp;
{
	return(EINVAL);
}

ffm_init()
{
	FFM_MOUNTTAB_LOCK_INIT();
	return (0);
}

ffm_mountroot()
{
	return(EINVAL);
}

ffm_swapvp()
{
	return EINVAL;
}
