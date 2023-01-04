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
static char *rcsid = "@(#)$RCSfile: kern_fuser.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1993/05/27 21:36:39 $";
#endif

#include <sys/file.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/fuser.h>


enum match_type { MATCH_VNODE, MATCH_MOUNT, MATCH_FSID };


#define FUSER_MATCH(ptr) \
	fuser_match(match_method, (ptr), path_vnode, path_mount, &path_fsid)


int
fuser_match(match_method, ptr, vp, mp, fp)
enum match_type	match_method;
register struct vnode	*ptr;
register struct vnode	*vp;
struct mount		*mp;
fsid_t			*fp;
{
	/*
	 *  No associated vnode for this check, so return 0.
	 */
	if (ptr == NULL)	return(0);

	switch(match_method) {

	    case MATCH_VNODE:
		return(ptr == vp);

	    case MATCH_MOUNT:
		return(ptr->v_mount == mp);

	    case MATCH_FSID:
		if ((ptr->v_mount == NULL) || (fp == NULL)) {
			return(0);
		}
		return(fsid_equal(&ptr->v_mount->m_stat.f_fsid, fp));
	}
	return(0);
}


int
fuser(p, args, retval)
struct proc *p;
void *args;
int *retval;
{
/* For alpha porting, redeclare flags and bufcount from int to long */
	struct args {
		char		*path;
#if	defined(__alpha)
		long		flags;
#else	/* !__alpha */
		int		flags;
#endif	/* !__alpha */
		f_user_t	*buffer;
#if	defined(__alpha)
		long		bufcount;
#else	/* !__alpha */
		int 		bufcount;
#endif	/* !__alpha */
	};
	struct args *uap = (struct args *)args;

	struct nameidata	*ndp = &u.u_nd;
	enum match_type		match_method;
	struct vnode		*path_vnode;
	struct mount		*path_mount;
	fsid_t			path_fsid;
	struct proc		*target_process;
	struct file		*file_ptr;
	struct ufile_state	*file_state_ptr;
	struct uio		uio;
	struct iovec		iov;
	struct vnode		*old_cdir, *old_rdir;
	f_user_t fuser;
	int	match_list;
	int	match_count;
	int	error;
	int	fd;
	
	/*
	 * Find the vnode associated with 'path'.  If successful, namei()
	 * returns the vnode locked.
	 */
	ndp->ni_dirp = uap->path;
	ndp->ni_segflg = UIO_USERSPACE;
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	if (error = namei(ndp)) {
		return(error);
	}
	path_vnode = ndp->ni_vp;
	old_cdir = ndp->ni_cdir;
	old_rdir = ndp->ni_rdir;

	/*
	 * Determine the matching algorithm.
	 */
	if ((path_vnode->v_type == VDIR) && (path_vnode->v_flag & VROOT) &&
	    (uap->flags & F_CONTAINED)) {
		/*
		 * We need to compare mount points.
		 */
		match_method = MATCH_MOUNT;
		path_mount = path_vnode->v_mount;
	}
	else if ((path_vnode->v_type == VBLK) && !(uap->flags & F_FILE_ONLY)) {
		/*
		 * We need to compare file system ids.
		 */
		match_method = MATCH_FSID;
		fsid_copy(&path_vnode->v_mount->m_stat.f_fsid,
			  &path_fsid);
	}
	else {
		/*
		 * Compare vnodes.
		 */
		match_method = MATCH_VNODE;
	}
	
	/*
	 * Initialization for our nameidata structure.
	 */
	ndp->ni_dirp = "..";
	ndp->ni_segflg = UIO_SYSSPACE;
	ndp->ni_nameiop = LOOKUP | FOLLOW;
	
	match_list = 0;
	match_count = 0;

	/*
	 * Initialization for uiomove(), in case we actually
	 * find something.
	 */
	uio.uio_rw = UIO_READ;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = 0;
	uio.uio_segflg = UIO_USERSPACE;
	uio.uio_resid = uap->bufcount * sizeof(f_user_t);
	iov.iov_base = (caddr_t)uap->buffer;

	for (target_process = allproc; target_process != NULL;
	     target_process = target_process->p_nxt) {
		/*
		 * If this process has no utask structure, skip it.  It won't
		 * be using the file.
		 */
		if (target_process->utask == NULL) {
			continue;
		}

		/*
		 * Check the current directory.
		 */
		if (FUSER_MATCH(target_process->u_cdir)) {
			match_list |= F_CDIR;
		}

		/*
		 * Check the parent of the current directory.
		 */
		ndp->ni_cdir = target_process->u_cdir;
		ndp->ni_rdir = target_process->u_rdir;
		if (!namei(ndp)) {
			if (FUSER_MATCH(ndp->ni_vp)) {
				match_list |= F_PDIR;
			}
			vrele(ndp->ni_vp);
		}
		ndp->ni_cdir = old_cdir;
		ndp->ni_rdir = old_rdir;

		/*
		 * Check the current root, if a chroot() was previously done.
		 */
		if (FUSER_MATCH(target_process->u_rdir)) {
			match_list |= F_RDIR;
		}

		/*
		 * Check the controlling TTY.
		 */
		if ((target_process->p_pgrp != NULL) &&
		    (target_process->p_session != NULL) &&
		    FUSER_MATCH(target_process->p_session->s_ttyvp)) {
			match_list |= F_TTY;
		}
		/*
		 * Check the vnode tracing to, if this is currently being done.
		 */
		if (FUSER_MATCH(target_process->p_tracep)) {
			match_list |= F_TRACE;
		}

		/*
		 * Check each individual open file.  If even one matches,
		 * don't bother with the rest because we don't count the
		 * number of open files matching, just whether or not any do.
		 */
		file_state_ptr = &(target_process->u_file_state);
		U_FDTABLE_LOCK(file_state_ptr);
		for (fd = 0; fd <= u.u_lastfile; fd++) {
			file_ptr = U_OFILE(fd, file_state_ptr);
			U_POFILE_SET(fd, 0, file_state_ptr);
			if ((file_ptr != NULL) &&
			    (file_ptr != U_FD_RESERVED) &&
			    (file_ptr->f_type == DTYPE_VNODE) &&
			    (FUSER_MATCH(((struct vnode *)file_ptr->f_data)))){
				match_list |= F_OPEN;
				break;
			}
		}
		U_FDTABLE_UNLOCK(file_state_ptr);

		ndp->ni_cdir = old_cdir;
		ndp->ni_rdir = old_rdir;

		/*
		 * If even one of the checks matched, write out a record if
		 * possible.  If there isn't room for this record, however,
		 * don't bother checking any more since we won't be able to
		 * add the next either.
		 */
		if (match_list) {
			if (uio.uio_resid >= sizeof(fuser)) {
				fuser.fu_pid = target_process->p_pid;
				fuser.fu_flags = match_list;
				fuser.fu_uid = target_process->p_ruid;
	
				iov.iov_len = sizeof(f_user_t);
				if (error = uiomove((caddr_t) &fuser,
						    sizeof(fuser), &uio)) {
					vrele(path_vnode);
					return(error);
				}
				match_count++;
			}
			else {
				/**
					Not sure what error to return here.
				**/
				error = 0;
				break;
			}
	
			/*
			 * Initialize for the next trip through.	
			 */
			match_list = 0;
		}
	}

	/*
	 * Release the path vnode.
	 */
	vrele(path_vnode);
	*retval = match_count;
	return(error);
}
