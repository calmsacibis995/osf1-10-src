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
static char	*sccsid = "@(#)$RCSfile: vfs_syscalls.c,v $ $Revision: 4.3.25.12 $ (DEC) $Date: 1993/08/27 14:49:37 $";
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
 * vfs_syscalls.c
 *
 *	Revision History:
 *
 *  8-Aug-91	prs
 *	Added a check for root vnode to mount1() to ensure the directory to
 *	be mounted on is not the root of another mount point; since namei()
 *	will traverse the covered vnode to the root.
 *
 * 17-Jun-91	Mitch Condylis
 *	Moved checking of uid during mount into subordinate 
 *	filesystems.  This was to allow user mounts with nfs.
 *	Also added check that vnode to be covered is owned by
 *	uid doing mount.
 *
 * 10-Jun-91    Larry Cohen
 *	getfh now needs the exporting granting file descriptor.
 *
 * 30-May-91	prs
 *	Merged in 1.0.1 bug fixes from OSF.
 *
 * 5-May-91	Ron Widyono
 *	Incorporate run-time options for kernel preemption (rt_preempt_enabled).
 *
 * 6-Apr-91	Ron Widyono
 *	Enable MOUNT_ENABLE_LOOKUPS calls for the RT_PREEMPT case so that
 *	lock counts stay consistent.
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */

#if 	MACH
#include <mach_nbc.h>
#endif
#include <mach_ldebug.h>
#include <rt_preempt.h>

#include <sys/secdefines.h>
#if	SEC_BASE
#include <sys/security.h>
#endif
#if	SEC_ARCH
#include <sys/secpolicy.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/vnode.h>
#include <ufs/inode.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <kern/macro_help.h>
#include <ufs/ufsmount.h>
#include <sys/lock_types.h>
#if	MACH
#include <kern/zalloc.h>
#include <kern/mfs.h>
#include <kern/assert.h>
#else
#include <sys/malloc.h>
#endif

#if     UNIX_LOCKS
#include <sys/ucred.h>
#endif


#if	MACH

#define	MOUNT_ALLOCATE(mp)	ZALLOC(mount_zone, mp, struct mount *);
#define	MOUNT_DEALLOCATE(mp)	ZFREE(mount_zone, (mp));
#define	PATH_ALLOCATE(path)	ZALLOC(pathname_zone, (path), char *);
#define	PATH_DEALLOCATE(path)	ZFREE(pathname_zone, (path))

#else	/* MACH */

#define	MOUNT_ALLOCATE(mp)	MALLOC(mp, (struct mount *),		\
				       sizeof(struct mount),		\
				       M_MOUNT, M_WAITOK)
#define	MOUNT_DEALLOCATE(mp)	FREE((caddr_t)mp, M_MOUNT)
#define	PATH_ALLOCATE(path)	MALLOC((path), char *, MAXPATHLEN,	\
				       M_NAMEI,M_WAITOK)
#define	PATH_DEALLOCATE(path)	FREE((path), M_NAMEI)

#endif	/* MACH */

void update_venf_lock(), vmountset(), vmountclear();
static struct vnode *mount_lookupname(char *);

#if	RT_PREEMPT
extern int	rt_preempt_enabled;
#endif

/*
 * Virtual File System System Calls
 */

/*
 * mount system call
 *
 * Synchronization:  mount must wait for VMOUNTING to clear on the
 * (possibly covered) vnode.  Mount then sets VMOUNTING, causing
 * new pathname and file handle translations to sleep.  Next, mount
 * checks whether there's already a filesystem mounted.  This is
 * legal for remount, which must then acquire the filesystem's
 * m_lookup_lock (causing mount to wait for active translations to
 * finish).  When mount is all done, it must clear VMOUNTING from
 * the (possibly covered) vnode and wake up waiting translators.
 */
#if	SEC_ARCH
mount(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return mount1(p, args, retval, (tag_t *) 0);
}

mount1(p, args, retval, tags)
	struct proc *p;
	void *args;
	long *retval;
	tag_t *tags;

#else	/* !SEC_ARCH */

mount(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
#endif	/* !SEC_ARCH */
{
	register struct args {
		long	type;
		char	*dir;
		long	flags;
		caddr_t	data;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp, *cvp;
	register struct mount *mp;
	int error, flag, mount_update, clear_mount;
	struct vattr vattr;
	extern int mount_maxtype;

#if SEC_BASE
	/*
	 * Must have the mount privilege.
	 */
	if (check_privileges && !privileged(SEC_MOUNT, EPERM)) {
		error = EPERM;
		goto done;
	}
#endif
	/*
	 * Get vnode to be covered
	 */
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->dir;
	if (error = namei(ndp))
		goto done;
	vp = ndp->ni_vp;
	mount_update = 0;
	clear_mount = 0;

	if (uap->flags & M_UPDATE) {
		mount_update++;
		BM(VN_LOCK(vp));
		if ((vp->v_flag & VROOT) == 0) {
			BM(VN_UNLOCK(vp));
			vrele(vp);
			error = EINVAL;
			goto done;
		}
		mp = vp->v_mount;
		BM(VN_UNLOCK(vp));
		/*
		 * Forcible unmount could remove the filesystem even 
		 * though we hold a referenced vnode.  v_vnodecovered 
		 * might go away.
		 */
		if (mp != DEADMOUNT) {
			if (cvp = mp->m_vnodecovered) {
				VREF(cvp);
				/*
				 * could have raced with unmount
				 */
				if (!cvp->v_mountedhere) {
					vrele(vp);
					goto out1;
				}
			} else	/* must be root filesystem */
				cvp = (struct vnode *) 0;
		} else {
			vrele(vp);
			error = ENODEV;
			goto done;
		}
		vrele(vp);

		/* Only root or user that did mount can do remount */
		if ((u.u_uid) && (u.u_uid != mp->m_uid)) {
			error = EPERM;
			goto out1;
		}

		/*
		 * We allow going from read-only to read-write,
		 * but not from read-write to read-only.
		 */
		MOUNT_LOCK(mp);
		if ((mp->m_flag & M_RDONLY) == 0 &&
		    (uap->flags & M_RDONLY) != 0) {
			MOUNT_UNLOCK(mp);
			error = EOPNOTSUPP;	/* Needs translation */
			goto out1;
		}
		flag = mp->m_flag;
		mp->m_flag |= M_UPDATE;
		goto update;
	}

	cvp = vp;
	VN_LOCK(cvp);
	/* SVID says that fattach must be able to cover an in-use file */
	if (cvp->v_usecount != 1 && (unsigned long)uap->type != MOUNT_FFM) {
		error = EBUSY;
		goto out2;
	}
	if (cvp->v_mountedhere != NULLMOUNT || cvp->v_flag & VROOT) {
		error = EBUSY;
		goto out2;
	}
	if (cvp->v_type != VDIR && (unsigned long)uap->type != MOUNT_FFM) {
		error = ENOTDIR;
		goto out2;
	}
	VN_UNLOCK(cvp);

	/* Put the check for ownership here so that the check for VROOT is
	 * done first.  This is required to meet SVID since EBUSY should
	 * be returned if a stream is already fattached.  Putting this
	 * check before the above checks causes mount (and thus fattach())
	 * to return EPERM since an fattached stream will always have
	 * root ownership and this test would fail before the VROOT
	 * test (causing the wrong errno to get returned).
	 */
	/* Check that user owns vnode to be covered */
	if (u.u_uid) {
		vattr_null(&vattr);
		VOP_GETATTR(vp, &vattr, u.u_cred, error);
		if (error)
			goto out1;
		if (u.u_uid != vattr.va_uid) {
			error = EPERM;
			goto out1;
		}
	}

	if ((unsigned long)uap->type > mount_maxtype ||
	    vfssw[uap->type] == (struct vfsops *)0) {
		error = ENODEV;
		goto out1;
	}
	vmountset(cvp);
	vinvalbuf(cvp, 1);
	clear_mount++;

	/*
	 * Allocate and initialize the file system.
	 * The mount initialization can be done without lock because
	 * no one else knows about the mount structure yet; the vnode
	 * initialization can be done without lock because no one can
	 * translate through the covered vnode now.
	 */
	MOUNT_ALLOCATE(mp);
	mp->m_op = vfssw[uap->type];
	mp->m_flag = 0;
	mp->m_exroot = 0;
	mp->m_uid = u.u_uid;
	mp->m_mounth = (struct vnode *)0;
#if SEC_FSCHANGE
	/*
	 * If this is an lmount, copy the global tag pool into the mount
	 * structure, else mark it as a labeled filesystem.  The M_SECURE
	 * flag is checked later in the sec_ufsmountcheck hook function
	 * to be sure it is consistent with whether or not the mounted
	 * volume is actually labeled.
	 */
	if (security_is_on) {
		if (tags)
			bcopy(tags, mp->m_tag, sizeof mp->m_tag);
		else
			mp->m_flag |= M_SECURE;
	} else
		bzero(mp->m_tag, sizeof mp->m_tag);

#endif /* SEC_FSCHANGE */
	mp->m_vnodecovered = cvp;
#if	SER_COMPAT || RT_PREEMPT
	mp->m_funnel = !FUNNEL_NULL;	/* XXX */
#endif
	MOUNT_LOOKUP_LOCK_INIT(mp);
	MOUNT_VLIST_LOCK_INIT(mp);
	MOUNT_LOCK_INIT(mp);
	MOUNT_DISABLE_LOOKUPS(mp);
	MOUNT_LOCK(mp);
	cvp->v_mountedhere = mp;

update:
	/*
	 * Set the mount level flags.
	 */
	LASSERT(MOUNT_LOCK_HOLDER(mp));
	LASSERT(mount_update || MOUNT_LOOKUPS_DISABLED(mp));
	if (uap->flags & M_RDONLY)
		mp->m_flag |= M_RDONLY;
	else
		mp->m_flag &= ~M_RDONLY;
	if (uap->flags & M_NOSUID)
		mp->m_flag |= M_NOSUID;
	else
		mp->m_flag &= ~M_NOSUID;
	if (uap->flags & M_NOEXEC)
		mp->m_flag |= M_NOEXEC;
	else
		mp->m_flag &= ~M_NOEXEC;
	if (uap->flags & M_NODEV)
		mp->m_flag |= M_NODEV;
	else
		mp->m_flag &= ~M_NODEV;
	if (uap->flags & M_SYNCHRONOUS)
		mp->m_flag |= M_SYNCHRONOUS;
	else
		mp->m_flag &= ~M_SYNCHRONOUS;
	if (uap->flags & M_FMOUNT)
		mp->m_flag |= M_FMOUNT;
	else
		mp->m_flag &= ~M_FMOUNT;
/*
 * If it's a user-level mount, don't allow setuid
 * programs or device special file access.
 */
	if (u.u_uid) 
		mp->m_flag |= M_NOSUID|M_NODEV;

	MOUNT_UNLOCK(mp);

	/*
	 * Mount the filesystem.
	 */
	VFS_MOUNT(mp, uap->dir, uap->data, ndp, error);
	if (mount_update) {
		MOUNT_LOCK(mp);
		if (error)
			mp->m_flag = flag;
		else
			mp->m_flag &= ~M_UPDATE;
		MOUNT_UNLOCK(mp);
		goto out1;
	}
	if (error) {
		cvp->v_mountedhere = NULLMOUNT;
#if	MACH_LDEBUG
		MOUNT_ENABLE_LOOKUPS(mp);
#elif	RT_PREEMPT
		if (rt_preempt_enabled)
			MOUNT_ENABLE_LOOKUPS(mp);
#endif
		MOUNT_DEALLOCATE(mp);
		goto out1;
	}

	/*
	 * Put the new filesystem on the mount list after root.
	 */
	MOUNTLIST_LOCK();
	mp->m_next = rootfs->m_next;
	mp->m_prev = rootfs;
	rootfs->m_next = mp;
	mp->m_next->m_prev = mp;
	MOUNTLIST_UNLOCK();

	cache_purge(cvp);
	MOUNT_ENABLE_LOOKUPS(mp);
	vmountclear(cvp);
	VFS_START(mp, 0, error);
	goto done;

out2:
	VN_UNLOCK(cvp);
out1:
	if (cvp) {
		if (clear_mount)
			vmountclear(cvp);
		vrele(cvp);
	}
done:
	/*
	 * audit the mount operations
	 */
        if ( DO_AUDIT ( SYS_mount, error ) ) {

	union mnt_arg {		/* mount options */
		struct ufs_args ufs_args;
		struct nfs_args nfs_args;
		struct mfs_args mfs_args;
		struct cdfs_args cdfs_args;
		procfs_args procfs_args;
	} mnt_arg;
	long arg[4];

	/* need to get the object name off the stack */
	arg[1] = 0;
	switch ( uap->type ) {

	case MOUNT_UFS:
	case MOUNT_MSFS:		/* ADVfs (nee MegaSafe)
					   uses ufs_args */
		if ( copyin ( uap->data, &mnt_arg.ufs_args, sizeof(struct ufs_args) ) == 0 )
			arg[1] = (long)mnt_arg.ufs_args.fspec;
		break;

	case MOUNT_NFS:
		if ( copyin ( uap->data, &mnt_arg.nfs_args, sizeof(struct nfs_args) ) == 0 )
			if ( mnt_arg.nfs_args.flags & NFSMNT_HOSTNAME )
				arg[1] = (long)mnt_arg.nfs_args.hostname;
		break;

	case MOUNT_MFS:
		if ( copyin ( uap->data, &mnt_arg.mfs_args, sizeof(struct mfs_args) ) == 0 )
			arg[1] = (long)mnt_arg.mfs_args.name;
		break;

	case MOUNT_CDFS:
		if ( copyin ( uap->data, &mnt_arg.cdfs_args, sizeof(struct cdfs_args) ) == 0 )
			arg[1] = (long)mnt_arg.cdfs_args.fspec;
		break;

	case MOUNT_PROCFS:
		if ( copyin ( uap->data, &mnt_arg.procfs_args, sizeof(procfs_args) ) == 0 )
			arg[1] = (long)mnt_arg.procfs_args.fspec;
		break;
        }

	arg[0] = (long)uap->dir;
	arg[2] = uap->type;
	arg[3] = uap->flags;

	if ( arg[1] ) {
		AUDIT_CALL(SYS_mount, error, arg, *retval, AUD_VHPR, "BBaa00000" );
	}
	else {
		AUDIT_CALL(SYS_mount, error, arg, *retval, AUD_VHPR, "B0aa00000" );
	}

	} /* end audit */

	return (error);
}

/*
 * Unmount system call.  Takes a path to the vnode mounted
 * on as argument.
 *
 * Most of the work is done by dounmount().
 */
unmount(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*pathp;
		long	flags;
	} *uap = (struct args *) args;
	register struct vnode *vp;
	register struct nameidata *ndp = &u.u_nd;
	struct mount *mp;
	int error;

#if SEC_BASE
	/*
	 * Must have the mount privilege.
	 */
	if (check_privileges && !privileged(SEC_MOUNT, EPERM))
		return (EPERM);
#endif

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->pathp;
	/*
	 * The following call to namei could hang in the case where 
	 * a nested NFS filesystem is being unmounted.  This is because
	 * namei will have to contact the parent directory via NFS to resolve
	 * the pathname of the directory which is being unmounted.
	 * Normally this unusual circumstance is not a problem because the
	 * unmount command could be interrupted.  However in the context
	 * of a system shutdown there is no context in which to interrupt
	 * the unmount process which was forked  from the halt/shutdown 
	 * command; thereby causing the shutdown to hang.
 	 *
 	 * So to work around that problem, if the unmount is a forced
	 * unmount (which is what the halt command issues via the "-f"
	 * flag to umount) then skip this call to namei to obtain the vp.
	 * Instead call a routine which walks through the mount table to
	 * find a match on the pathname.
	 */
	if (uap->flags & MNT_FORCE) {
		vp = mount_lookupname(uap->pathp);
		if (vp == NULLVP) {
			/*
			 * A name match wasn't found from searching the mount
			 * table.  In this case revert to the original 	
			 * approach of calling namei to get the vp.
			 */
			if (error = namei(ndp))
				return (error);
			vp = ndp->ni_vp;
		}
	}
	else {
		if (error = namei(ndp))
			return (error);
		vp = ndp->ni_vp;
	}
	/*
	 * Must be the root of the filesystem
	 */
	BM(VN_LOCK(vp));
	if ((vp->v_flag & VROOT) == 0) {
		BM(VN_UNLOCK(vp));
		vrele(vp);
		return (EINVAL);
	}
	mp = vp->v_mount;
	BM(VN_UNLOCK(vp));

	if (mp == DEADMOUNT) {
		vrele(vp);
		return(ENODEV);
	}

	/* unmount must be done by root or user that did mount */
	if ((u.u_uid) && (u.u_uid != mp->m_uid)) {
		vrele(vp);
		return (EPERM);
	}

	return (dounmount(vp, mp, uap->flags));
}

/*
 * dounmount
 * Do most of the work of an unmount.  This function is shared by
 * ufs and mfs unmounts.
 *
 * First must disable new pathname translations (VMOUNTING)
 * and wait for old ones to complete (MOUNT_DISABLE_LOOKUPS).
 * Then it is safe to try to unmount the filesystem.
 *
 * We need the vp passed in because it's not safe to do the
 * vrele in unmount, before doing vmountset; it could create races. 
 * So we vrele vp here (if it's non-null).
 *
 * N.B.  Waiting for old translations to complete probably
 * breaks forcible unmount.
 */
dounmount(vp, mp, flags)
	struct vnode *vp;
	register struct mount *mp;
	int flags;
{
	register struct vnode  *cvp, *rvp, *nvp;
	int error;

	ASSERT(mp != DEADMOUNT && mp != NULLMOUNT);
	if (cvp = mp->m_vnodecovered) {
		VREF(cvp);
		vmountset(cvp);
		/*
		 * could have raced with another unmount
		 */
		if (!cvp->v_mountedhere) {
			error = ENODEV;
			goto out;
		}
	} else {
		/*
		 * it's the root!
		 */
		error = EBUSY;
		goto out;
	}
	MOUNT_DISABLE_LOOKUPS(mp);
	/*
	 * vp, if set, came in referenced (the root vnode of the FS).
	 */
	if (vp != (struct vnode *) 0)
		vrele(vp);
	vrele(cvp);

	/*
	 * Uncache segmented vnodes that might be cached from this mp
	 */

	u_seg_cache_clear(mp);

	cache_purgevfs(mp);	/* remove cache entries for this file sys */
	VFS_SYNC(mp, MNT_WAIT, error);

	VFS_UNMOUNT(mp, flags, error);
	if (error) {
		MOUNT_ENABLE_LOOKUPS(mp);
		vmountclear(cvp);
		return (error);
	}

	/*
	 * Remove all ways to find the mountpoint:  covered vnode
	 * and mount list.
	 */
	VN_LOCK(cvp);
	cvp->v_mountedhere = NULLMOUNT;
	VN_UNLOCK(cvp);

	MOUNTLIST_LOCK();
	if (mp == rootfs)
		panic("dounmount: unmounting root");
	mp->m_prev->m_next = mp->m_next;
	mp->m_next->m_prev = mp->m_prev;
	MOUNTLIST_UNLOCK();

	/*
	 * Set v_mount to null to prevent references to memory
	 * we are going to free. Ideally, this would be done
	 * in the sfs unmount routine, but why propagate mount
	 * vnode list logic there....
	 */
	MOUNT_VLIST_LOCK(mp);
	for (rvp = mp->m_mounth; rvp; rvp = nvp) {
		VN_LOCK(rvp);
		nvp = rvp->v_mountf;
		if (rvp->v_mount != mp) {
			VN_UNLOCK(rvp);
			break;
		}
		rvp->v_mount = DEADMOUNT;
		rvp->v_mountf = NULL;
		rvp->v_mountb = NULL;
		VN_UNLOCK(rvp);
	}
	MOUNT_VLIST_UNLOCK(mp);

	vmountclear(cvp);
	vrele(cvp);
#if	MACH_LDEBUG
	MOUNT_ENABLE_LOOKUPS(mp);
#elif	RT_PREEMPT
	if (rt_preempt_enabled)
		MOUNT_ENABLE_LOOKUPS(mp);
#endif
	MOUNT_DEALLOCATE(mp);
	return (error);
out:
	if (cvp) {
		vmountclear(cvp);
		vrele(cvp);
	}
	return (error);
}


/*
 * Get file handle system call
 */
getfh(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
		fhandle_t *fhp;
		long	xfd;  /* export granting directory */
	} *uap = (struct args *) args;

	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp, *xvp;
	char buff[sizeof(fsid_t) + 2 * sizeof(struct fid)];
	fhandle_t *fh = (fhandle_t *)buff;
	int error;
	struct file *fp1, *xfp;

#if SEC_BASE
	/*
	 * Must have the remote privilege.
	 */
	if (!privileged(SEC_REMOTE, EPERM))
		return (EPERM);
#else
	/*
	 * Must be super user
	 */
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
	/*
	 * check to see if we are using the old technique of passing
	 * in file descriptors
	 */
	if ((u_int)uap->fname < OPEN_MAX_SYSTEM) {
		/* new */
		if (error = getf(&fp1, (int)uap->fname, FILE_FLAGS_NULL,
				 			&u.u_file_state))
			return (error);
		if (error = getf(&xfp, (int)uap->xfd, FILE_FLAGS_NULL,
				 			&u.u_file_state)) {
			FP_UNREF(fp1);
			return (error);
		}
        	vp = (struct vnode *)fp1->f_data;
        	xvp = (struct vnode *)xfp->f_data;
	
	} else {
	
		ndp->ni_nameiop = LOOKUP | FOLLOW;
		ndp->ni_segflg = UIO_USERSPACE;
		ndp->ni_dirp = uap->fname;
		if (error = namei(ndp))
			return (error);
		vp = ndp->ni_vp;
	
		xvp = 0;
	}

	/* Can't getfh an NFS file (VFS_VPTOFH panics) */
	if (vp->v_mount->m_stat.f_type == MOUNT_NFS) {
		if (xvp) {
			FP_UNREF(fp1);
			FP_UNREF(xfp);
		} else {
			vrele(vp);
		}
		return(EREMOTE);
	}

	bzero((caddr_t)fh, sizeof(fhandle_t));
	fh->fh_fsid = vp->v_mount->m_stat.f_fsid;
	VFS_VPTOFH(vp, &fh->fh_fid, error);

	if (xvp) {
		if (!error)
			VFS_VPTOFH(xvp, &fh->fh_efid, error);
		if (!error) {
			/*
	 	 	 * Check to see if we can stuff the export fid 
			 * into the fhandle.  If not then we can not support 
			 * this file system.
	 	 	 */
#ifndef NFS_FHSIZE
#define NFS_FHSIZE 32
#endif
			if (sizeof(fsid_t) + fh->fh_fid.fid_len + 
				fh->fh_efid.fid_len > NFS_FHSIZE ) 

				error = EREMOTE;

		}
		FP_UNREF(fp1);
		FP_UNREF(xfp);
	} else {
		vrele(vp);
	}

	if (!error)
		error = copyout((caddr_t)fh, (caddr_t)uap->fhp, 
			sizeof (fhandle_t));
	return (error);
}


/*
 * Sync system call.
 * Sync each mounted filesystem.
 */
/* ARGSUSED */
sync(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct mount *mp;
	int error = 0;

	MOUNTLIST_LOCK();
	mp = rootfs;
	do {
		/*
		 * Never want to take the mount lookup lock unconditionally
		 * while holding the mount list lock:
		 *	1.  Skip over filesystems being mounted/unmounted.
		 *	2.  Possible deadlock with unmount.
		 */
		if (!MOUNT_LOOKUP_TRY(mp))
			goto skip;

		MOUNT_LOCK(mp);
		if ((mp->m_flag & (M_RDONLY|M_SYNCING)) != 0)
			goto next;
		mp->m_flag |= M_SYNCING;
		MOUNT_UNLOCK(mp);
		MOUNTLIST_UNLOCK();

		VFS_SYNC(mp, MNT_NOWAIT, error);

		MOUNTLIST_LOCK();
		MOUNT_LOCK(mp);
		mp->m_flag &= ~M_SYNCING;
next:		MOUNT_UNLOCK(mp);
		MOUNT_LOOKUP_DONE(mp);

		/*
		 * Mount list may have changed but:
		 *	1.  This filesystem is still mounted;
		 *	2.  Currently hold mount list lock;
		 * so m_next is still valid.
		 */
skip:		mp = mp->m_next;
	} while (mp != rootfs);
	MOUNTLIST_UNLOCK();
	return (error);
}

/*
 * get filesystem statistics
 *
 * Holding the referenced vnode returned by namei prevents
 * the filesystem from becoming unmounted.
 */
statfs(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char *path;
		struct statfs *buf;
	} *uap = (struct args *) args;
	register struct mount *mp;
	struct statfs sf, *sp;
	register struct nameidata *ndp = &u.u_nd;
	int error;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->path;
	if (error = namei(ndp))
		return (error);
	mp = ndp->ni_vp->v_mount;
	VFS_STATFS(mp, error);
	if (error) {
		vrele(ndp->ni_vp);
		return (error);
	}
	MOUNT_LOCK(mp);
	mp->m_stat.f_flags = mp->m_flag & M_VISFLAGMASK;
	MP_ONLY(sf = mp->m_stat);
	MOUNT_UNLOCK(mp);
	MP_ONLY(sp = &sf);
	UNI_ONLY(sp = &mp->m_stat);
	error = copyout((caddr_t)sp, (caddr_t)uap->buf, sizeof(*sp));
	vrele(ndp->ni_vp);
	return (error);
}

/*
 * Holding the referenced vnode returned by getvnode prevents
 * the filesystem from becoming unmounted in the normal case.
 */
fstatfs(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long fd;
		struct statfs *buf;
	} *uap = (struct args *) args;
	struct file *fp;
	struct mount *mp;
	struct statfs sf, *sp;
	int error;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	mp = ((struct vnode *)fp->f_data)->v_mount;
	VFS_STATFS(mp, error);
	if (error)
		goto out;
	MOUNT_LOCK(mp);
	mp->m_stat.f_flags = mp->m_flag & M_VISFLAGMASK;
	MP_ONLY(sf = mp->m_stat);
	sf = mp->m_stat;
	MOUNT_UNLOCK(mp);
	MP_ONLY(sp = &sf);
	UNI_ONLY(sp = &mp->m_stat);
	error = copyout((caddr_t)sp, (caddr_t)uap->buf, sizeof(*sp));
out:
	FP_UNREF(fp);
	return(error);
}

/*
 * get statistics on all filesystems
 *
 * Mount lookup lock protects filesystem from being unmounted
 * while VFS_STATFS does its work.
 */
getfsstat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		struct statfs *buf;
		long bufsize;
		long flags;
	} *uap = (struct args *) args;
	register struct mount *mp;
	register struct statfs *sp;
	struct statfs sf;
	caddr_t sfsp;
	long count, maxcount, error;

	maxcount = uap->bufsize / sizeof(struct statfs);
	sfsp = (caddr_t)uap->buf;
	if (sfsp && maxcount == 0) {
		*retval = 0;
		return (0);
	}
	count = 0;
	MOUNTLIST_LOCK();
	mp = rootfs;
	if (!sfsp) {
		do {
			count++;
			mp = mp->m_prev;
		} while (mp != rootfs);
	} else {
		do {
			if (!MOUNT_LOOKUP_TRY(mp)) {
				mp = mp->m_prev;
				continue;
			}
			MOUNTLIST_UNLOCK();
			/*
			 * If MNT_NOWAIT is specified, do not refresh the
			 * fsstat cache. MNT_WAIT overrides MNT_NOWAIT.
			 */
			if ((uap->flags & MNT_WAIT) != 0) {
				VFS_STATFS(mp, error);
				if (error)
					goto next;
			}
			MOUNT_LOCK(mp);
			mp->m_stat.f_flags = mp->m_flag & M_VISFLAGMASK;
			MP_ONLY(sf = mp->m_stat);
			MOUNT_UNLOCK(mp);
			MP_ONLY(sp = &sf);
			UNI_ONLY(sp = &mp->m_stat);
			if (error = copyout((caddr_t)sp, sfsp, sizeof(*sp))) {
				MOUNT_LOOKUP_DONE(mp);
				return (error);
			}
			sfsp += sizeof(*sp);
			count++;
next:			MOUNTLIST_LOCK();
			MOUNT_LOOKUP_DONE(mp);
			mp = mp->m_prev;
		} while (mp != rootfs && count < maxcount);
	}
	MOUNTLIST_UNLOCK();
	*retval = count;
	return (0);
}


/*
 * Change current working directory to a given file descriptor.
 */
fchdir(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	fd;
	} *uap = (struct args *) args;
	register struct vnode *vp;
	struct file *fp;
	register int error = 0;
	enum vtype type;
	struct vnode *ocdir;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	vp = (struct vnode *)fp->f_data;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type != VDIR)
		error = ENOTDIR;
	else
		VOP_ACCESS(vp, VEXEC, u.u_cred, error);
	if (error)
		goto out;
	VREF(vp);
	U_HANDY_LOCK();
	ocdir = u.u_cdir;
	u.u_cdir = vp;
	U_HANDY_UNLOCK();
	vrele(ocdir);
out:
	FP_UNREF(fp);
	return (error);
}

/*
 * Change current working directory (``.'').
 */
chdir(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	int error;
	struct vnode *ocdir;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = chdirec(ndp))
		return (error);
	U_HANDY_LOCK();
	ocdir = u.u_cdir;
	u.u_cdir = ndp->ni_vp;
	U_HANDY_UNLOCK();
	vrele(ocdir);
	return (0);
}

/*
 * Change notion of root (``/'') directory.
 */
chroot(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	int error;
	struct vnode *ordir;

#if SEC_BASE
	if (!privileged(SEC_CHROOT, EPERM))
		return (EPERM);
#else
	if (error = suser(u.u_cred, &u.u_acflag))
		return (error);
#endif
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = chdirec(ndp))
		return (error);
	ordir = NULL;
	U_HANDY_LOCK();
	if (u.u_rdir != NULL)
		ordir = u.u_rdir;
	u.u_rdir = ndp->ni_vp;
	U_HANDY_UNLOCK();
	if (ordir != NULL)
		vrele(ordir);
	return (0);
}

/*
 * Common routine for chroot and chdir.
 */
chdirec(ndp)
	register struct nameidata *ndp;
{
	struct vnode *vp;
	int error;
	enum vtype type;

	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type != VDIR)
		error = ENOTDIR;
	else
		VOP_ACCESS(vp, VEXEC, ndp->ni_cred, error);
	if (error)
		vrele(vp);
	return (error);
}

/*
 * Open system call.
 *
 * Check permissions, allocate an open file structure,
 * and call the device open routine if any.
 * The open system call always passes O_NOCTTY flag, because controlling
 * tty must now be explicitly assigned.  Binary compatibility is maintained
 * with the oopen() call, which results in not passing the O_NOCTTY flag.
 *
 * The new creat() call is a library which calls open.
 */
#ifdef	COMPAT_43
/*
 * Creat system call.
 * Now a library routine that calls open.
 */
ocreat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		long	fmode;
	} *uap = (struct args *) args;

	struct args1 {
		char	*fname;
		long	mode;
		long	crtmode;
	} openuap;

	openuap.fname = uap->fname;
	openuap.crtmode = uap->fmode;
	openuap.mode = O_WRONLY | O_CREAT | O_TRUNC;
	return (copen(p, &openuap, retval, 1));
}

/*
 * Open system call.  Compatibility version.
 */
oopen(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return (copen(p, args, retval, 1));
}

/*
 * Open system call.  The real thing.
 */
open(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return (copen(p, args, retval, 0));
}

copen(p, args, retval, compat)
	int compat;
#else
open(p, args, retval)
#endif	/* COMPAT_43 */
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		long	mode;
		long	crtmode;
	} *uap = (struct args *)  args;
	struct nameidata *ndp = &u.u_nd;
	register struct file *fp;
	int fmode, cmode;
	struct file *nfp;
	int indx, error;
	extern struct fileops vnops;

	if (error = falloc(&nfp, &indx))
		return (error);
	ASSERT(U_OFILE(indx, &u.u_file_state) == U_FD_RESERVED);
	ASSERT(nfp->f_count == 1 && nfp->f_type == DTYPE_RESERVED);
	fp = nfp;
	fmode = uap->mode - FOPEN;
#ifdef	COMPAT_43
	if (!compat)
#endif
	fmode |= O_NOCTTY;	
	U_HANDY_LOCK();
	cmode = ((uap->crtmode &~ u.u_cmask) & 07777) &~ ISVTX;
	U_HANDY_UNLOCK();
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = vn_open(ndp, fmode, cmode)) {
		U_FD_SET(indx, NULL, &u.u_file_state);
		fdealloc(fp);
		if (error == ERESTART)
			error = EINTR;
		return (error);
	}
	/*
	 * If the returned vnode is for the file descriptor file system, we don't
	 * want the vnode we've got.  Instead, we want to duplicate the vnode
	 * referred to by the file descriptor found in the data field of the FDFS
	 * vnode.
	 */
	if (ndp->ni_vp->v_flag & VDUP)
	{
	  extern int fdfs_file_descriptor(struct vnode *vnode_ptr);
	  struct
	  {
#ifdef	__alpha
            long i;
#else  /* __alpha */
	    int i;
#endif /* __alpha */
	  } dupargs;
	  
	  /*
	   * The call to falloc() above allocated both a file descriptor AND a file
	   * structure.  Because we're duplicating an existing file descriptor, we'll
	   * share the file structure from the duplicated file.
	   */
	  U_FD_SET(indx, NULL, &u.u_file_state);
	  fdealloc(nfp);
	  dupargs.i = fdfs_file_descriptor(ndp->ni_vp);
	  vrele(ndp->ni_vp);
	  return (dup(p, &dupargs, retval));
	}else{
   	  FP_LOCK(fp);
          fp->f_flag = fmode & FMASK;
          fp->f_type = DTYPE_VNODE;
          fp->f_ops = &vnops;
          fp->f_data = (caddr_t)ndp->ni_vp;
#if     SER_COMPAT
          fp->f_funnel = FUNNEL_NULL;
#endif
          FP_UNLOCK(fp);
          U_FD_SET(indx, fp, &u.u_file_state);
	}
	/* */
	*retval = indx;
	return (0);
}

/*
 * Mknod system call
 */
mknod(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		long	fmode;
		long	dev;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct vattr vattr;
	int error, ret_val;
	short cmask;

#if SEC_BASE
	if ((uap->fmode & IFMT) != IFIFO &&
	    !privileged(SEC_MKNOD, EPERM))
		return (EPERM);
#else
	if ((uap->fmode & IFMT) != IFIFO &&
	    (error = suser(u.u_cred, &u.u_acflag)))
		return (error);
#endif
	ndp->ni_nameiop = CREATE | WANTPARENT;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (vp != NULL) {
		error = EEXIST;
		goto out;
	}
	vattr_null(&vattr);
	switch (uap->fmode & IFMT) {

	case IFMT:	/* used by badsect to flag bad sectors */
		vattr.va_type = VBAD;
		break;
	case IFCHR:
		vattr.va_type = VCHR;
		vattr.va_rdev = uap->dev;
		break;
	case IFBLK:
		vattr.va_type = VBLK;
		vattr.va_rdev = uap->dev;
		break;
	case IFIFO:
		vattr.va_type = VFIFO;
		break;
	default:
		error = EINVAL;
		goto out;
	}
	U_HANDY_LOCK();
	cmask = u.u_cmask;
	U_HANDY_UNLOCK();
	vattr.va_mode = (uap->fmode & 07777) &~ cmask;

	/*
	 * Range check passed dev_t entry.  Dev_t's can be up to
	 * VNOVAL-1 in size (major + minor) for character or block
	 * device special files...
	 */
	if (((vattr.va_type == VBLK) || (vattr.va_type == VCHR)) &&
		(!(vattr.va_rdev < VNOVAL)))
		error = EINVAL;

out:
	if (!error)
		VOP_MKNOD(ndp, &vattr, ndp->ni_cred, error);
	else {
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		if (vp)
			vrele(vp);
	}
	return (error);
}

/*
 * link system call
 */
link(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*target;
		char	*linkname;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp, *xp;
	int error, ret_val;
	enum vtype type;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->target;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type == VDIR) {
		error = EPERM;
		goto out1;
	}
	ndp->ni_nameiop = CREATE | WANTPARENT;
	ndp->ni_dirp = (caddr_t)uap->linkname;
	if (error = namei(ndp))
		goto out1;
	xp = ndp->ni_vp;
	if (xp != NULL) {
		error = EEXIST;
		goto out;
	}
	xp = ndp->ni_dvp;
	if (vp->v_mount != xp->v_mount)
		error = EXDEV;
out:
	if (!error)
		VOP_LINK(vp, ndp, error);
	else {
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		if (ndp->ni_vp)
			vrele(ndp->ni_vp);
	}
out1:
	vrele(vp);
	return (error);
}

/*
 * symlink -- make a symbolic link
 */
symlink(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*target;
		char	*linkname;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	struct vattr vattr;
	char *target;
	int error, ret_val;
	short cmask;

	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->linkname;
	PATH_ALLOCATE(target);
	if (error = copyinstr(uap->target, target, MAXPATHLEN, (u_int *)0)) {
		if (error == ENOENT) {
			error = ENAMETOOLONG;
		}
		goto out;
	}
	ndp->ni_nameiop = CREATE | WANTPARENT;
	if (error = namei(ndp))
		goto out;
	if (ndp->ni_vp) {
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		vrele(ndp->ni_vp);
		error = EEXIST;
		goto out;
	}
	vattr_null(&vattr);
	BM(U_HANDY_LOCK());
	cmask = u.u_cmask;
	BM(U_HANDY_UNLOCK());
	vattr.va_mode = 0777 &~ cmask;
	vattr.va_type = VLNK;
	VOP_SYMLINK(ndp, &vattr, target, error);
out:
	PATH_DEALLOCATE(target);
	return (error);
}

/*
 * Unlink system call.
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
unlink(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	int error, ret_val;
	enum vtype type;

	ndp->ni_nameiop = DELETE | WANTPARENT;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type == VDIR) {
		error = EPERM;
		goto out;
	}
	/*
	 * Don't unlink a mounted file.
	 */
	BM(VN_LOCK(vp));
	if (vp->v_flag & VROOT) {
		BM(VN_UNLOCK(vp));
		error = EBUSY;
		goto out;
	}
	BM(VN_UNLOCK(vp));

	/*
	 * Purge any possible segmentation caching of vnode (only can happen
	 * if it's code had been exec'ed or mmap'ed with execute permission).
	 */
	u_seg_uncache_vnode(vp);

out:
	if (!error)
		VOP_REMOVE(ndp, error);
	else {
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		vrele(vp);
	}
	return (error);
}

/*
 * Seek system call
 */
lseek(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct file *fp;
	register struct args {
		long	fdes;
		off_t	off;
		long	sbase;
	} *uap = (struct args *) args;
	struct vattr vattr;
	struct vnode *vp;
	enum vtype type;
	int error = 0;
	off_t newoffset;

	if (error = getf(&fp, uap->fdes, FILE_FLAGS_NULL, &u.u_file_state))
		return (error);
	if ((fp->f_type != DTYPE_VNODE)) {
		error = ESPIPE;
		goto out1;
	}
	vp = (struct vnode *) fp->f_data;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type == VFIFO) {
		error = ESPIPE;
		goto out1;
	}
	FP_IO_LOCK(fp);
	switch (uap->sbase) {

	case L_INCR:
		BM(FP_LOCK(fp));
		newoffset = fp->f_offset + uap->off;
		BM(FP_UNLOCK(fp));
		break;

	case L_XTND:
		VOP_GETATTR(vp, &vattr, u.u_cred, error);
		if (error)
			goto out;
		newoffset = uap->off + vattr.va_size;
		break;

	case L_SET:
		newoffset = uap->off;
		break;

	default:
		error = EINVAL;
		goto out;
	}

	VOP_SEEK(vp, fp->f_offset, newoffset, u.u_cred, error);

	if (error == 0) {
		FP_LOCK(fp);
		fp->f_offset = newoffset;
		FP_UNLOCK(fp);
		*retval = newoffset;
	}
out:
	FP_IO_UNLOCK(fp);
out1:
	FP_UNREF(fp);
	return(error);
}

/*
 * Access system call
 */
saccess(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		long	fmode;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct ucred *cred = ndp->ni_cred;
	register struct vnode *vp;
	uid_t	ruid;
	gid_t	rgid;
	int 	error, mode;
	int	swapped = 0;
#if	!MACH
	int svuid, svgid;
#endif

	short eff_only = 0;

	/*
	 * Check the validity of the uap->fmode argument.
	 */
	if (uap->fmode && (uap->fmode & ~(R_OK|W_OK|X_OK|EFF_ONLY_OK)))
		return(EINVAL);

	/*
	 * Duplicate credentials so this thread can override some of
	 * the values without disturbing other threads in the same
	 * task that may be using the credentials.
	 */
	PROC_LOCK(p);
	ruid = p->p_ruid;
	rgid = p->p_rgid;
	PROC_UNLOCK(p);
	if (cred->cr_uid != ruid || cred->cr_gid != rgid) {
		++swapped;
		if (uap->fmode == EFF_ONLY_OK)
			eff_only = 1;
#if	MACH
		ndp->ni_cred = crdup(cred);
		if (!eff_only) {
			ndp->ni_cred->cr_uid = ruid;
			ndp->ni_cred->cr_gid = rgid;
		}
#else
		if (!eff_only) {
			svuid = cred->cr_uid;
			svgid = cred->cr_gid;
			cred->cr_uid = ruid;
			cred->cr_gid = rgid;
		}
#endif
#if	SEC_ARCH
		SP_CHANGE_SUBJECT();
#endif
	}
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		goto out;
	vp = ndp->ni_vp;
	/*
	 * fmode == 0 means only check for exist
	 */
	if (uap->fmode) {
		mode = 0;
		if (uap->fmode & R_OK)
			VOP_ACCESS(vp, VREAD, ndp->ni_cred, error);
		if (!error && (uap->fmode & W_OK) &&
						(error = vn_writechk(vp)) == 0)
			VOP_ACCESS(vp, VWRITE, ndp->ni_cred, error);
		if (!error && (uap->fmode & X_OK))
			VOP_ACCESS(vp, VEXEC, ndp->ni_cred, error);
	}
	vrele(vp);
out:
	if (swapped) {
#if	MACH
		crfree(ndp->ni_cred);
		ndp->ni_cred = cred;
#else
		if (!eff_only) {
			u.u_uid = svuid;
			u.u_gid = svgid;
		}
#endif
#if	SEC_ARCH
		SP_CHANGE_SUBJECT();
#endif
	}
	return (error);
}

/*
 * Stat system call.  This version follows links.
 */
stat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return(stat1(p, args, retval, FOLLOW));
}

/*
 * Lstat system call.  This version does not follow links.
 */
lstat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{

	return (stat1(p, args, retval, NOFOLLOW));
}

stat1(p, args, retval, follow)
	struct proc *p;
	void *args;
	long *retval;
	int follow;
{
	register struct args {
		char	*fname;
		struct stat *ub;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	struct stat sb;
	int error, rval = 0;

	ndp->ni_nameiop = LOOKUP | follow;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	error = vn_stat(ndp->ni_vp, &sb);
	if(!error && S_ISCHR(sb.st_mode)) {
		VOP_IOCTL(ndp->ni_vp,I_ISASTREAM,0,0,u.u_cred,error,&rval);
		if(!error && (rval == I_FIFO || rval == I_PIPE)) {
			sb.st_mode &= ~S_IFMT;
			sb.st_mode |= S_IFIFO;
			sb.st_size = 0;
		}
		error = 0;
	}
	vrele(ndp->ni_vp);
	if (error)
		return (error);
	error = copyout((caddr_t)&sb, (caddr_t)uap->ub, sizeof (sb));
	return (error);
}

/*
 * Return target name of a symbolic link
 */
readlink(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*name;
		char	*buf;
		long	count;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct iovec aiov;
	struct uio auio;
	int error, count;
	enum vtype type;

	ndp->ni_nameiop = LOOKUP;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->name;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type != VLNK) {
		error = EINVAL;
		goto out;
	}
#if	SEC_ARCH
	if (security_is_on) {
	VOP_ACCESS(vp, SP_STATACC, ndp->ni_cred, error);
	if (error)
		goto out;
	}
#endif
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_offset = 0;
	auio.uio_rw = UIO_READ;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_resid = uap->count;
	VOP_READLINK(vp, &auio, ndp->ni_cred, error);
out:
	vrele(vp);
	count = uap->count - auio.uio_resid;
	*retval = count;
	if (!error && (auio.uio_resid > 0))
		subyte((caddr_t)uap->buf + count, '\0');
	return (error);
}

/*
 * XXX -- we are not currently supporting chflags and fchflags
 *	  due to a conflict in bits.  We are using the low bit of
 *	  the on-disk inode for fast symbolic links, which is
 *	  in direct conflict with the ability *chflags gives the
 *	  superuser to change the low 16 bits of that field.
 *
 * This conflict needs to be resolved before we can support these
 * calls or their equivalent.
 */
/*
 * Change flags of a file given path name.
 */
chflags(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
		long	flags;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct vattr vattr;
	int error;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	vattr_null(&vattr);
	vattr.va_flags = uap->flags;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (rofs(vp->v_mount)) {
		error = EROFS;
		goto out;
	}
	VOP_SETATTR(vp, &vattr, ndp->ni_cred, error);
out:
	vrele(vp);
	return (error);
}

/*
 * Change flags of a file given a file descriptor.
 */
fchflags(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	fd;
		long	flags;
	} *uap = (struct args *) args;
	struct vattr vattr;
	struct vnode *vp;
	struct file *fp;
	int error;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	vattr_null(&vattr);
	vattr.va_flags = uap->flags;
	vp = (struct vnode *)fp->f_data;
	if (rofs(vp->v_mount)) {
		error = EROFS;
		goto out;
	}
	VOP_SETATTR(vp, &vattr, fp->f_cred, error);
out:
	FP_UNREF(fp);
	return (error);
}

/*
 * Change mode of a file given path name.
 */
chmod(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
		long	fmode;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct vattr vattr;
	int error;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	vattr_null(&vattr);
	vattr.va_mode = uap->fmode & 07777;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (rofs(vp->v_mount)) {
		error = EROFS;
		goto out;
	}
	VOP_SETATTR(vp, &vattr, ndp->ni_cred, error);
	if ((! error) && (vp->v_type == VREG)) {
	      /* enf locking on regular file only */
	      update_venf_lock(uap->fmode & 07777, vp);
	}
out:
	vrele(vp);
	return (error);
}

/*
 * Change mode of a file given a file descriptor.
 */
fchmod(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	fd;
		long	fmode;
	} *uap = (struct args *) args;
	struct vattr vattr;
	struct vnode *vp;
	struct file *fp;
	int error;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	vattr_null(&vattr);
	vattr.va_mode = uap->fmode & 07777;
	vp = (struct vnode *)fp->f_data;
	if (rofs(vp->v_mount)) {
		error = EROFS;
		goto out;
	}
	VOP_SETATTR(vp, &vattr, fp->f_cred, error);
	if ((! error) && (vp->v_type == VREG)) {
	      /* enf locking on regular file only */
	      update_venf_lock(uap->fmode & 07777, vp);
	}
out:
	FP_UNREF(fp);
	return (error);
}

/*
 * Set ownership given a path name.
 */
chown(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return(chown2(p, args, retval, FOLLOW));
}

/*
 * Set ownership given a path name, do not follow last link.
 */
lchown(p, args, retval)
        struct proc *p;
        void *args;
        long *retval;
{
        return(chown2(p, args, retval, 0));
}

chown2(p, args, retval, flag)
        struct proc *p;
        void *args;
        long *retval;
	int flag;
{
	struct args {
		char	*fname;
		long	uid;
		long	gid;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct vattr vattr;
	int error;

	ndp->ni_nameiop = LOOKUP | flag;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	vattr_null(&vattr);
	vattr.va_uid = uap->uid;
	vattr.va_gid = uap->gid;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (rofs(vp->v_mount)) {
		error = EROFS;
		goto out;
	}
	VOP_SETATTR(vp, &vattr, ndp->ni_cred, error);
out:
	vrele(vp);
	return (error);
}

/*
 * Set ownership given a file descriptor.
 */
fchown(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	fd;
		long	uid;
		long	gid;
	} *uap = (struct args *) args;
	struct vattr vattr;
	struct vnode *vp;
	struct file *fp;
	int error;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	vattr_null(&vattr);
	vattr.va_uid = uap->uid;
	vattr.va_gid = uap->gid;
	vp = (struct vnode *)fp->f_data;
	if (rofs(vp->v_mount)) {
		error = EROFS;
		goto out;
	}
	VOP_SETATTR(vp, &vattr, fp->f_cred, error);
out:
	FP_UNREF(fp);
	return (error);
}

utimes(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		char	*fname;
		struct	timeval *tptr;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct timeval tv[2];
	struct vattr vattr;
	int error;
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (rofs(vp->v_mount)) {
		error = EROFS;
		goto out;
	}
	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error)
		goto out;
#if	SEC_BASE
	if (u.u_uid != vattr.va_uid && !privileged(SEC_OWNER, EPERM)) {
		if (uap->tptr != (struct timeval *)NULL) {
			error = EPERM;
			goto out;
		}
		VOP_ACCESS(vp, VWRITE, ndp->ni_cred, error);
		if (error)
			goto out;
	}
#else
	if (u.u_uid != vattr.va_uid &&
	    (error = suser(u.u_cred, &u.u_acflag))) {
		if (uap->tptr != (struct timeval *)NULL)
			goto out;
		VOP_ACCESS(vp, VWRITE, ndp->ni_cred, error);
		if (error)
			goto out;
	}
#endif
	if (uap->tptr == (struct timeval *)NULL) {
		int s;

		s = splhigh();
		TIME_READ_LOCK();
		tv[0] = tv[1] = time;
		TIME_READ_UNLOCK();
		splx(s);
	} else 
		error = copyin((caddr_t)uap->tptr, (caddr_t)tv, sizeof (tv));
		
	if (!error) {
		vattr_null(&vattr);
		vattr.va_atime = tv[0];
		vattr.va_mtime = tv[1];
		VOP_SETATTR(vp, &vattr, ndp->ni_cred, error);
	}
out:
	vrele(vp);
	return (error);
}

/*
 * Truncate a file given its path name.
 */
truncate(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
		off_t	length;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	int error;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	error = vtruncate(ndp->ni_vp, uap->length, ndp->ni_cred, 1);
	vrele(ndp->ni_vp);
	return (error);
}

/*
 * Truncate a file given a file descriptor.
 */
ftruncate(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	fd;
		off_t	length;
	} *uap = (struct args *) args;
	struct file *fp;
	int error, flag;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	if ((flag & FWRITE) == 0) {
#if SEC_BASE
		RECLASSIFY_ET(ET_ACCESS_DENIAL);
#endif
		error = EBADF;
	}
	else {
	        error = vtruncate((struct vnode *)fp->f_data, uap->length,
				  fp->f_cred, 0);
	}
	FP_UNREF(fp);
	return (error);
}

/*
 * Common code for truncate()/ftruncate().
 */
vtruncate(vp, length, cred, check_access)
	register struct vnode *vp;
	off_t length;
	register struct ucred *cred;
	int	check_access;
{
	struct vattr vattr;
	int error;
	enum vtype type;
	u_int vflag;

	VN_LOCK(vp);
	type = vp->v_type;
	vflag = vp->v_flag;
	VN_UNLOCK(vp);
	if (type != VREG)
	        return(EINVAL);

	if (vflag & VENF_LOCK)
	      /* enf locks on file */
	      return(EAGAIN);

	if (error = vn_writechk(vp))
		return(error);

	if (check_access) {
		VOP_ACCESS(vp, VWRITE, cred, error);
		if (error)
		        return(error);
	}

	/*
	 *	Get the length to figure out whether this truncate
	 *	is actually extending the file.
	 */
	VOP_GETATTR(vp, &vattr, cred, error);
	if (error)
		return(error);

	if (length <= vattr.va_size) {

		/*
		 *	Normal truncate.  Still have to do the setattr
		 *	even if the file size isn't changing.
		 */
	        vattr_null(&vattr);
		vattr.va_size = length;		
		VOP_SETATTR(vp, &vattr, cred, error);
	        return(error);
	}
	else {
                struct uio uio;
                struct iovec iov;
                int zero = 0;

		/*
		 *	Extending.  VOP_SETATTR won't extend on
		 *	most filesystems.  Write a zero byte into
		 *	the new end of file to force extension.
		 */
                iov.iov_base = (caddr_t) &zero;
                iov.iov_len = 1;
                uio.uio_iov = &iov;
                uio.uio_iovcnt = 1;
                uio.uio_offset = length - 1;
                uio.uio_segflg = UIO_SYSSPACE;
                uio.uio_rw = UIO_WRITE;
                uio.uio_resid = 1;
                VOP_WRITE(vp, &uio, 0, cred, error);
		return(error);
	}	
}

/*
 * Synch an open file.
 */
fsync(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	fd;
	} *uap = (struct args *) args;
	register struct vnode *vp;
	struct file *fp;
	int error, flag;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	vp = (struct vnode *)fp->f_data;
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	VOP_FSYNC(vp, flag, fp->f_cred, MNT_WAIT, error);
	FP_UNREF(fp);
	return (error);
}

/*
 * Rename system call.
 *
 * Source and destination must either both be directories, or both
 * not be directories.  If target is a directory, it must be empty.
 */
rename(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*from;
		char	*to;
	} *uap = (struct args *) args;
	register struct vnode *tvp, *fvp, *tdvp;
	register struct nameidata *ndp = &u.u_nd;
	struct nameidata tond;
	struct utask_nd tutnd;
	int error, ret_val;
	int stripslash = 0;
	enum vtype ftype, ttype;

	/*
	 * See if we're doing directories before we decide to 
	 * strip trailing slashes.  This requires an extra namei
	 * operation.  This could probably be optimized, but...
	 */
	ndp->ni_nameiop = LOOKUP;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->from;
	if (error = namei(ndp))
		return (error);
	tvp = ndp->ni_vp;
	BM(VN_LOCK(tvp));
	if (tvp->v_type == VDIR)
		stripslash = STRIPSLASH;
	BM(VN_UNLOCK(tvp));
	vrele(tvp);
	/*
	 * Do it again...
	 */
	ndp->ni_nameiop = DELETE | WANTPARENT | stripslash;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->from;
	if (error = namei(ndp))
		return (error);
	u.u_vno_indx--; /* don't record this namei data */
	fvp = ndp->ni_vp;
	tond.ni_utnd = &tutnd;
	nddup(ndp, &tond);
	tond.ni_nameiop = RENAME | WANTPARENT | stripslash;
	tond.ni_segflg = UIO_USERSPACE;
	tond.ni_dirp = uap->to;
	error = namei(&tond);
	if (error) {
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		vrele(fvp);
		goto out1;
	}
	tdvp = tond.ni_dvp;
	tvp = tond.ni_vp;
	if (tvp != NULL) {
		BM(VN_LOCK(fvp));
		ftype = fvp->v_type;
		BM(VN_UNLOCK(fvp));	
		BM(VN_LOCK(tvp));
		ttype = tvp->v_type;
		BM(VN_UNLOCK(tvp));	
		if (ftype == VDIR && ttype != VDIR) {
			error = ENOTDIR;
			goto out;
		} else if (ftype != VDIR && ttype == VDIR) {
			error = EISDIR;
			goto out;
		}
		if (fvp->v_mount != tvp->v_mount) {
			if (fvp->v_mount == tdvp->v_mount) /* XXX VSX tests */
				error = EBUSY;
			else
				error = EXDEV;
			goto out;
		}
	}
	if (fvp->v_mount != tdvp->v_mount) {
		error = EXDEV;
		goto out;
	}
	if (fvp == tdvp)
		error = EINVAL;
	/*
	 * If source is the same as the destination,
	 * then there is nothing to do.
	 */
	if (fvp == tvp)
		error = -1;
out:
	if (!error)
		VOP_RENAME(ndp, &tond, error);
	else {
		VOP_ABORTOP(&tond, ret_val);
		vrele(tdvp);
		if (tvp)
			vrele(tvp);
		ASSERT(fvp && ndp->ni_dvp);
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		vrele(fvp);
	}
out1:
	ndrele(&tond);
	if (error == -1)
		return (0);
	return (error);
}

/*
 * Mkdir system call
 */
mkdir(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*name;
		long	dmode;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct vattr vattr;
	int error, ret_val;
	short cmask;

	ndp->ni_nameiop = CREATE | WANTPARENT | STRIPSLASH;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->name;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (vp != NULL) {
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		vrele(vp);
		return (EEXIST);
	}
	vattr_null(&vattr);
	vattr.va_type = VDIR;
	BM(U_HANDY_LOCK());
	cmask = u.u_cmask;	
	BM(U_HANDY_UNLOCK());
	vattr.va_mode = (uap->dmode & 0777) &~ cmask;
	VOP_MKDIR(ndp, &vattr, error);
	if (!error)
		vrele(ndp->ni_vp);
	return (error);
}

/*
 * Rmdir system call.
 */
rmdir(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*name;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	int error, flag, ret_val;
	enum vtype type;

	ndp->ni_nameiop = DELETE | WANTPARENT | STRIPSLASH;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->name;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	flag = vp->v_flag;
	BM(VN_UNLOCK(vp));
	if (type != VDIR) {
		error = ENOTDIR;
		goto out;
	}
	/*
	 * No rmdir "." please.
	 */
	if (ndp->ni_dvp == vp) {
		error = EINVAL;
		goto out;
	}
	/*
	 * Don't unlink a mounted file.
	 */
	if (flag & VROOT)
		error = EBUSY;
out:
	if (!error)
		VOP_RMDIR(ndp, error);
	else {
		VOP_ABORTOP(ndp, ret_val);
		vrele(ndp->ni_dvp);
		vrele(vp);
	}
	return (error);
}

/*
 * Read a block of directory entries in a file system independent format
 */
getdirentries(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	fd;
		char	*buf;
		unsigned long count;
		long	*basep;
	} *uap = (struct args *) args;
	register struct vnode *vp;
	struct file *fp;
	struct uio auio;
	struct iovec aiov;
	off_t off;
	int error, eofflag, flag;
	enum vtype type;

	if (error = getvnode(uap->fd, &fp))
		return (error);
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));	
	if ((flag & FREAD) == 0) {
		error = EBADF;
		goto out;
	}
	vp = (struct vnode *)fp->f_data;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type != VDIR) {
		error = EINVAL;
		goto out;
	}
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_rw = UIO_READ;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_resid = uap->count;
	FP_IO_LOCK(fp);
	auio.uio_offset = off = fp->f_offset;
	VOP_READDIR(vp, &auio, fp->f_cred, &eofflag, error);
	fp->f_offset = auio.uio_offset;
	FP_IO_UNLOCK(fp);
	if (error)
		goto out;
	error = copyout((caddr_t)&off, (caddr_t)uap->basep, sizeof(long));
	*retval = uap->count - auio.uio_resid;
out:
	FP_UNREF(fp);
	return (error);
}

/*
 * mode mask for creation of files
 */
umask(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	mask;
	} *uap = (struct args *) args;

	U_HANDY_LOCK();
	*retval = u.u_cmask;
	u.u_cmask = uap->mask & 07777;
	U_HANDY_UNLOCK();
	return (0);
}

/*
 * Void all references to file by ripping underlying filesystem
 * away from vnode.
 */
revoke(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	struct vattr vattr;
	int error;
	enum vtype type;

	ndp->ni_nameiop = LOOKUP | FOLLOW;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	BM(VN_UNLOCK(vp));
	if (type != VCHR && type != VBLK) {
		error = EINVAL;
		goto out;
	}
	VOP_GETATTR(vp, &vattr, u.u_cred, error);
	if (error)
		goto out;
#if	SEC_BASE
	if (u.u_uid != vattr.va_uid && !privileged(SEC_OWNER, EPERM)) {
		error = EPERM;
		goto out;
	}
#else
	if (u.u_uid != vattr.va_uid &&
	    (error = suser(u.u_cred, &u.u_acflag)))
		goto out;
#endif
	/*
	 * N.B. The clearalias call used to be conditional upon an active
	 *	(i.e. > 1) v_usecount.  This will not work if the argument
	 *	to revoke() is an alias for an open device.
	 *	Clearalias can handle unopened devices.
	 */
	clearalias(vp, 0);
out:
	vrele(vp);
	return (error);
}

/*
 * Returns an error to the caller or a referenced
 * file structure which the caller must dispose of
 * when done with the file structure and/or vnode.
 */
getvnode(fdes, fpp)
	struct file **fpp;
	int fdes;
{
	struct file *fp;
	int error;

	if (error = getf(&fp, fdes, FILE_FLAGS_NULL, &u.u_file_state))
		return (error);
	if (fp->f_type != DTYPE_VNODE) {
		FP_UNREF(fp);
		return (EINVAL);
	}
	*fpp = fp;
	return (0);
}


vmountwait(vp)
struct vnode	*vp;
{
	LASSERT(VN_LOCK_HOLDER(vp));
	while (vp->v_flag & VMOUNTING) {
		vp->v_flag |= VMOUNTWAIT;
		assert_wait((vm_offset_t)&vp->v_mountedhere, FALSE);
		VN_UNLOCK(vp);
		thread_block();
		VN_LOCK(vp);
	}
	LASSERT(VN_LOCK_HOLDER(vp));
	ASSERT((vp->v_flag & VMOUNTING) == 0);
}

void
vmountset(vp)
struct vnode	*vp;
{
	VN_LOCK(vp);
	vmountwait(vp);
	vp->v_flag |= VMOUNTING;
	VN_UNLOCK(vp);
}

void
vmountclear(vp)
struct vnode	*vp;
{
	int wakeup;

	VN_LOCK(vp);
	ASSERT((vp->v_flag & VMOUNTING) == VMOUNTING);
	wakeup = (vp->v_flag & VMOUNTWAIT);
	vp->v_flag &= ~(VMOUNTING|VMOUNTWAIT);
	VN_UNLOCK(vp);
	if (wakeup)
		thread_wakeup((vm_offset_t)&vp->v_mountedhere);
}

/*
 * We could simply not lock the root file system for performance reasons,
 * since it cannot be unmounted, but the mount update function may want 
 * to do something "interesting" that would require turning off path 
 * translation.  So we synchronize lookups and mounts on the root.
 */
vmountread(vp)
struct vnode *vp;
{
	register struct vnode *cvp;

	cvp = vp->v_mount->m_vnodecovered;
	if (cvp) {
		VN_LOCK(cvp);
		vmountwait(cvp);
		MOUNT_LOOKUP(vp->v_mount);
		VN_UNLOCK(cvp);
	} else {
		ASSERT(vp->v_mount == rootfs);
		MOUNT_LOOKUP(rootfs);
	}
}

utnd_dup(src, dst)
struct utask_nd	*src, *dst;
{

	UTND_LOCK(src);
	dst->utnd_cdir = src->utnd_cdir;
	dst->utnd_rdir = src->utnd_rdir;
	UTND_UNLOCK(src);
}


rofs(mp)
register struct mount *mp;
{
	register int	flag;

	BM(MOUNT_LOCK(mp));	
	flag = mp->m_flag;
	BM(MOUNT_UNLOCK(mp));
	return((flag & M_RDONLY) != 0);
}


/*
 * Update the VENF_LOCK flag in the vnode to indicate if the file has 
 * enforcement mode file locking on based on the mode bits of the file.
 * If it's locked and the mode bits indicate enforcement mode, set
 * the flag; otherwise, clear it.
 */
static void
update_venf_lock(mode,vp)
int mode;
struct vnode *vp;
{
	VN_LOCK(vp);
	if ((mode & S_ISGID) && (!(mode & S_IXGRP))) {
		if (vp->v_flag & VLOCKS)
			vp->v_flag |= VENF_LOCK;
	} else
		vp->v_flag &= ~VENF_LOCK;
	VN_UNLOCK(vp);
}

/*
 * mount_lookupname():
 * Search the mount table for a name match with the user argument
 * passed in.  If one is found, reference the root vnode and return
 * it.
 */
static struct vnode *
mount_lookupname(char *pathp)
{
	caddr_t pathbuf;
	struct vnode *vp;
	struct mount *mp;
	int len, error;

	PATH_ALLOCATE(pathbuf);
	error = copyinstr(pathp, pathbuf, MAXPATHLEN, &len);
	if (error) {
		PATH_DEALLOCATE(pathbuf);
		return (NULLVP);
	}
	vp = NULLVP;
restart:
	MOUNTLIST_LOCK();
	mp = rootfs;
	ASSERT(mp);
	do {
		if (!bcmp(pathbuf, mp->m_stat.f_mntonname, (unsigned) len))
			break;
		mp = mp->m_next;
	} while (mp != rootfs);
	if (mp != rootfs) {
		/* got one; now try to lock the file system */
		if (!MOUNT_LOOKUP_TRY(mp)) {
			MOUNTLIST_UNLOCK();
			thread_block();
			goto restart;
		}
		MOUNTLIST_UNLOCK();
		VFS_ROOT(mp, &vp, error);
		if (error) {
			MOUNT_LOOKUP_DONE(mp);
			vp = NULLVP;	/* just to be sure */
			goto out;
		}
		ASSERT(vp->v_flag & VROOT);
		/* 
		 * If there is a nested mount point, just give up
		 * because the unmount wouldn't succeed anyways.
		 */
		if (vp->v_mountedhere) {
			vrele(vp);
			vp = NULLVP;
		} 
		MOUNT_LOOKUP_DONE(mp);
	} else
		MOUNTLIST_UNLOCK();
out:
	PATH_DEALLOCATE(pathbuf);
	return (vp);
}
