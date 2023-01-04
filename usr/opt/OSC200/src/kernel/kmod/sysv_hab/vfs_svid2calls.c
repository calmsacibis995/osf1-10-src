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
static char     *sccsid = "@(#)$RCSfile: vfs_svid2calls.c,v $ $Revision: 1.1.3.8 $ (DEC) $Date: 1992/11/23 15:48:48 $";
#endif
/*
 */


/*
 * vfs_svid2calls.c
 *
 *      Revision History:
 *
 * 12-Mar-91
 *      Initial Implementation.
 *
 */

#if 	MACH
#include <mach_nbc.h>
#endif
#include <mach_ldebug.h>

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
				       (u_long)sizeof(struct mount),	\
				       M_MOUNT, M_WAITOK)
#define	MOUNT_DEALLOCATE(mp)	FREE((caddr_t)mp, M_MOUNT)
#define	PATH_ALLOCATE(path)	MALLOC((path), char *, MAXPATHLEN,	\
				       M_NAMEI,M_WAITOK)
#define	PATH_DEALLOCATE(path)	FREE((path), M_NAMEI)

#endif	/* MACH */


/*
 * Unlink system call.
 * Hard to avoid races here, especially
 * in unlinking directories.
 */
sysv_unlink(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		char	*fname;
	} *uap = (struct args *) args;
	register struct nameidata *ndp = &u.u_nd;
	register struct vnode *vp;
	int flag, error, ret_val;
	enum vtype type;


	ndp->ni_nameiop = DELETE | WANTPARENT;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_dirp = uap->fname;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	type = vp->v_type;
	flag = vp->v_flag;
	BM(VN_UNLOCK(vp));
	if (type == VDIR) {
		error = sysv_rmdir(vp);
		return(error);
	}

	/*
	 * Don't unlink a mounted file.
	 */
	if (flag & VROOT) {
		error = EBUSY;
		goto out;
	}


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
 * s5 Rmdir subroutine.
 * namei() was called by calling routine, s5unlink() syscall.
 */
sysv_rmdir(vp)
register struct vnode *vp;
{
	register struct nameidata *ndp = &u.u_nd;
	int error, flag, ret_val;

#if SEC_BASE
	/*
	 * Must have the link privilege.
	 */
	if (!privileged(SEC_LINKDIR, EPERM))
		return (EPERM);
#else

	/*
	 * Must be super user
	 */
											if (error = suser(u.u_cred, &u.u_acflag))
		goto out;
#endif


	vp = ndp->ni_vp;
	BM(VN_LOCK(vp));
	flag = vp->v_flag;
	BM(VN_UNLOCK(vp));

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
 * System V habitat version of open
 *
 * This is needed so that a '1' can be passed in for the
 * compat flag.  This allows controlling terminals to be
 * enabled the way SVID III likes it.  See copen call
 * in vfs/vfs_syscalls and ttyopen in bsd/tty.c for
 * more info.
 */
sysv_open(p, args, retval)
        struct proc *p;
        void *args;
        long *retval;
{
        register struct args {
                char    *fname;
                long    mode;
                long    crtmode;
        } *uap = (struct args *)  args;
        int error;

        error = copen(p, args, retval, 1);
        return (error);
}
