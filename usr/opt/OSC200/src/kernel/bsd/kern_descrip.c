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
static char	*rcsid = "@(#)$RCSfile: kern_descrip.c,v $ $Revision: 4.4.20.11 $ (DEC) $Date: 1993/12/10 00:27:30 $";
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
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Modification history
 *
 * 23-Sep-1991	Fred Canter
 *	Fix a bug (introduced with > 64 open FD support) which caused
 *	select to hang instead of return an error if the file descriptor
 *	had been closed. This causes the X server to sometimes not remove
 *	the window when the client terminated. The fix is to not decrement
 *	uf_lastfile in the in close.
 *
 * 19-Aug-1991	Philip Cameron
 *	Modified fcntl F_SETLK and F_SETLKW commnads to handle the
 *	Ultrix 4.2 F_UNLCK sub-command. (The F_GETLK is still broken)
 *
 * 19-Jun-1991  Ajay Kachrani
 * 	Fix fcntl (SETFL- FNDELAY) bug for socket by calling fset/ioctl
 *	for only flags that users have requested.
 *
 * 3-Jun-1991	lebel
 *	Added support for > 64 open file descriptors per process.
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/vnode.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <kern/parallel.h>
#include <sys/lock_types.h>
#include <kern/assert.h>

/*
 * Descriptor management.
 */

/*
 * The free file lock controls allocation and deallocation of elements
 * within the file structure list.
 */
udecl_simple_lock_data(,free_file_lock)
#define	FREE_FILE_LOCK()	usimple_lock(&free_file_lock)
#define	FREE_FILE_UNLOCK()	usimple_unlock(&free_file_lock)
#define	FREE_FILE_LOCK_INIT() 	usimple_lock_init(&free_file_lock)

/*
 * Private routine forward declarations.
 */
int      alloc_fd_slots();

/*
 * Ensure that descriptor slot n has been allocated, returning ENOMEM
 * if unable to do so and 0 otherwise.  Expressed as a macro to make it
 * cheap enough to call within loops iterating over all descriptors.
 * The predicate part ensures that the function to allocate descriptor
 * slots in the range [NOFILE_IN_U .. u.u_rlimit.rlim_cur] is called only 
 * when necessary.  Assumes FDTABLE is locked.
 */
#define enable_fd_slot(n) \
	(((n) < (NOFILE_IN_U + u.u_file_state.uf_of_count)) ? \
		0 : alloc_fd_slots((n)))

/*
 * System calls on descriptors.
 */
/* ARGSUSED */
getdtablesize(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	*retval = u.u_rlimit[RLIMIT_NOFILE].rlim_cur;
	return (0);
}

/* ARGSUSED */
getdopt(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (EOPNOTSUPP);
}

/* ARGSUSED */
setdopt(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (EOPNOTSUPP);
}

/* ARGSUSED */
dup(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	i;
	} *uap = (struct args *) args;
	struct file *fp;
	int j;
	int error = 0;
	char poflags;
	register struct ufile_state *ufp = &u.u_file_state;

	if (error = getf(&fp, uap->i, &poflags, ufp))
		return (error);
	if (error = ufalloc(0, &j, ufp))
		goto out;
	*retval = j;
	dupit(j, fp, poflags &~ UF_EXCLOSE, ufp);
out:
	FP_UNREF(fp);
	return (error);
}

/* ARGSUSED */
dup2(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	i;
		long	j;
	} *uap = (struct args *)args;
	struct file *fp[2];
	int error = 0;
	char poflags;
	register struct ufile_state *ufp = &u.u_file_state;
	register struct vnode *vp;
	struct stat sb;

	if (error = getf(&fp[0], uap->i, &poflags, ufp))
		goto done;
	if (uap->j < 0 || uap->j >= u.u_rlimit[RLIMIT_NOFILE].rlim_cur) {
		error = EBADF;
		goto out;
	}
	*retval = uap->j;
	if (uap->i == uap->j) {
		error = 0;
		goto out;
	}

	U_FDTABLE_LOCK(ufp);
	if (error = enable_fd_slot(uap->j)) {
		U_FDTABLE_UNLOCK(ufp);
		goto out;
	}
	fp[1] = U_OFILE(uap->j, ufp);
	U_OFILE_SET(uap->j, U_FD_RESERVED, ufp);
	U_FDTABLE_UNLOCK(ufp);

	/* audit info */
	if ( DO_AUDIT(SYS_dup2,0) ) {
		/* 2 <= AUD_VNOMAX (# entries in u_vno_dev, u_vno_num, u_vno_mode) */
		for ( ; u.u_vno_indx < 2 && fp[u.u_vno_indx]; u.u_vno_indx++ )
			if ( (vp = (struct vnode *)fp[u.u_vno_indx]->f_data)
			&& fp[u.u_vno_indx]->f_type == DTYPE_VNODE ) {
				if ( vn_stat ( vp, &sb ) ) break;
				u.u_vno_dev[u.u_vno_indx]  = S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) ?
                                    sb.st_rdev : sb.st_dev;
				u.u_vno_num[u.u_vno_indx]  = sb.st_ino;
				u.u_vno_mode[u.u_vno_indx] = sb.st_mode;
			}
	}

	if (fp[1])
		error = closef(fp[1]);
	dupit(uap->j, fp[0], poflags &~ UF_EXCLOSE, ufp);
	/* 
	 * dup2() must succeed even though the close had an error.
	 */
	error = 0;		/* XXX */
out:
	FP_UNREF(fp[0]);
done:
	AUDIT_CALL ( SYS_dup2, error, args, *retval, AUD_HPR, (char *)0 );
	return (error);
}

dupit(fd, fp, flags, ufp)
	long fd;
	register struct file *fp;
	register long flags;
	register struct ufile_state *ufp;
{

	/*
	 * Assume referenced file structure and reserved file descriptor.
	 */
	ASSERT(U_OFILE(fd, ufp) == U_FD_RESERVED);
	U_FDTABLE_LOCK(ufp);
	FP_REF(fp);			/* account for new reference */
	U_OFILE_SET(fd, fp, ufp);
	U_POFILE_SET(fd, flags, ufp);
	if (fd > ufp->uf_lastfile)
		ufp->uf_lastfile = fd;
	U_FDTABLE_UNLOCK(ufp);
}

/*
 * The file control system call.
 */
/* ARGSUSED */
fcntl(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct file *fp, *nfp;
	register struct args {
		long	fdes;
		long	cmd;
		long	arg;
	} *uap = (struct args *)args;
	int i, error = 0;
	char poflags;
	int lckflag = 0;
	off_t offset;
	register struct ufile_state *ufp = &u.u_file_state;
	int fflags;

	/* remote locks use an extended ld structure. */
	union {
		struct flock fl;
		struct eflock efl;
	} xld;
#define rld (xld.fl)    /* regular lock descriptor */
#define eld (xld.efl)   /* extended lock descriptor */

        /* for F_CNVT */
        struct f_cnvt_arg {
                nfsv2fh_t *fh;
                int filemode;
		int fd;
        };
        struct f_cnvt_arg a;
        register struct f_cnvt_arg *ap;
        nfsv2fh_t tfh;
        register int filemode;
        int indx, mode;
        struct vnode *vp;
        struct vnode *myfhtovp();
	void fdealloc();
        extern struct fileops vnops;
	enum vtype type;
	u_long v_flag;
	struct flock inflock;
	long arg[2];

	if (error = getf(&fp, uap->fdes, &poflags, ufp))
		return (error);
	BM(FP_IO_LOCK(fp));
	offset = fp->f_offset;
	BM(FP_IO_UNLOCK(fp));

	switch(uap->cmd) {
	case F_DUPFD:
		i = uap->arg;
		if (i >= 0 && i < u.u_rlimit[RLIMIT_NOFILE].rlim_cur) {
			if (!(error = ufalloc(i, &i, ufp))) {
				*retval = i;
				dupit(i, fp, poflags &~ UF_EXCLOSE, ufp);
			}
		} else error = EINVAL;
		arg[0] = (long)"F_DUPFD";
		arg[1] = uap->fdes;
		AUDIT_CALL ( SYS_fcntl, error, arg, *retval, AUD_VHPR, "rc0000000" );
		break;

	case F_GETFD:
		*retval = poflags & 1;
		break;

	case F_SETFD:
		U_FDTABLE_LOCK(ufp);
		/*
		 * Only perform the requested operation if the file
		 * descriptor was not closed or re-allocated between
		 * the first lock during getf and this lock.  Ignore
		 * poflags, u.u_pofile[uap->fdes] might have changed.
		 */
		if (U_OFILE(uap->fdes, ufp) == fp) {
			U_POFILE_SET(uap->fdes, 
				(U_POFILE(uap->fdes, ufp) &~ FD_CLOEXEC) |
				(uap->arg & FD_CLOEXEC), ufp);
		}
		U_FDTABLE_UNLOCK(ufp);
		break;

	case F_GETFL:
		BM(FP_LOCK(fp));
		*retval = fp->f_flag+FOPEN;
		BM(FP_UNLOCK(fp));
		break;

	case F_SETFL:
		/*
		 * Guarantee that flags will match the request to
		 * the ioctl routine by holding a lock across both
		 * operations.  We can't hold f_incore_lock for a
		 * long time so we cheat by using the f_io_lock.
		 * This is only a problem for a multiprocessor or a
		 * pre-emptible kernel.
		 */
		MP_ONLY(FP_IO_LOCK(fp));
		FP_LOCK(fp);
		fflags = fp->f_flag & FASYNC;
		fp->f_flag &= FCNTLCANT;
		fp->f_flag |= (uap->arg-FOPEN) &~ FCNTLCANT;
                if ((fflags ^ fp->f_flag) & FASYNC) {
                        /* Call down only if FASYNC changes */
                        FP_UNLOCK(fp);
                        fflags ^= FASYNC;
                        if (error = fioctl(fp, FIOASYNC, (caddr_t)&fflags)) {
                                /* Reset FASYNC to previous on failure */
                                FP_LOCK(fp);
                                fp->f_flag ^= FASYNC;
                                FP_UNLOCK(fp);
                        }
                } else
                        FP_UNLOCK(fp);
		MP_ONLY(FP_IO_UNLOCK(fp));
		break;

	case F_GETOWN:
		error = fgetown(fp, retval);
		break;

	case F_SETOWN:
		error = fsetown(fp, uap->arg);
		break;

	case F_GETLK:
	case F_RGETLK:
		/* get record lock */
		if (uap->cmd == F_GETLK) {
			if (copyin(uap->arg, &eld, sizeof(rld))) {
				error = EFAULT;
				break;
			}
			eld.l_rpid = 0;
			eld.l_rsys = 0;
			lckflag |= GETFLCK;
			bcopy(&eld, &inflock, sizeof(struct flock));
		} 
		else {
#if     SEC_BASE
			if (!privileged(SEC_REMOTE, 0)) {
				error = EPERM;
				break;
			}
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				break;
#endif
			if (copyin(uap->arg, &eld, sizeof(eld))) {
				error = EFAULT;
				break;
			}
			lckflag |= RGETFLCK;
		}
		if (fp->f_type == DTYPE_VNODE) {
			/* 
			 * Make l_start relative to beginning of file
			 */
			if (error = convoff(fp, &eld))
				break;
			/* Convert negative length to postive */
			if ((long)eld.l_len < 0) {
				(long)eld.l_start += (long)eld.l_len;
				(long)eld.l_len = -(long)eld.l_len;
			}
			if ((long)eld.l_start < 0) {
				error = EINVAL;
				break;
			}
			VOP_LOCKCTL((struct vnode *)fp->f_data, &eld,
				lckflag, fp->f_cred, u.u_procp->p_pid,
				offset, error);
			if ((!error) && (uap->cmd == F_GETLK)) {
				/*
			 	 * per SVID, change only 'l_type' field if 
				 * unlocked
			 	 */
				if (eld.l_type == F_UNLCK) {
				   inflock.l_type = F_UNLCK;
				   if (copyout(&inflock,uap->arg,sizeof(rld)))
						error = EFAULT;
				} else {
				   if (copyout(&eld, uap->arg, sizeof(rld)))
						error = EFAULT;
				}
			} 
			else if ((!error) && (uap->cmd == F_RGETLK)) {
				if (copyout(&eld, uap->arg, sizeof(eld)))
					error = EFAULT;
			}
		}
		break;

	case F_SETLKW:
	case F_RSETLKW:
		/* set record lock and wait if blocked */
		lckflag |= SLPFLCK;
		/*
		 * Fall through
		 */
	case F_SETLK:
	case F_RSETLK:
		/* set record lock and return if blocked */
		if ((uap->cmd == F_SETLK) || (uap->cmd == F_SETLKW)) {
			if (copyin(uap->arg, &eld, sizeof(rld))) {
				error = EFAULT;
				break;
			}
			eld.l_rpid = 0;
			eld.l_rsys = 0;
			lckflag |= SETFLCK;
		} 
		else {
#if     SEC_BASE
			if (!privileged(SEC_REMOTE, 0)) {
				error = EPERM;
				break;
			}
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				break;
#endif
			if (copyin(uap->arg, &eld, sizeof(eld))) {
				error = EFAULT;
				break;
			}
			lckflag |= RSETFLCK;
		}
		if (fp->f_type == DTYPE_VNODE) {
			if ((eld.l_type==F_RDLCK && (fp->f_flag & FREAD)==0) ||
			    (eld.l_type==F_WRLCK && (fp->f_flag & FWRITE)==0)) {
				error = EBADF;
			} else {
				/* 
			 	 * Make l_start relative to beginning of file
			 	 */
				if (error = convoff(fp, &eld))
					break;
				/* Convert negative length to postive */
				if ((long)eld.l_len < 0) {
					(long)eld.l_start += (long)eld.l_len;
					(long)eld.l_len = -(long)eld.l_len;
				}
				if ((long)eld.l_start < 0) {
					error = EINVAL;
					break;
				}
				/*
			 	 *  XXX skipped memory_mapped stuff from 
			 	 *  OSF1.1 for now due to vm changes
			 	 */
				VOP_LOCKCTL((struct vnode *)fp->f_data,
					&eld, lckflag, fp->f_cred, 
					u.u_procp->p_pid, offset, error);

				if ((!error) && ((uap->cmd == F_RSETLKW) || 
				                 (uap->cmd == F_RSETLK))) {
				     if (copyout(&eld, uap->arg, sizeof(eld)))
					   error = EFAULT;
				}
			}
		}
		break;

	case F_PURGEFS:		/* Purge lock manager locks on fs */
#if     SEC_BASE
		if (!privileged(SEC_REMOTE, 0)) {
			error = EPERM;
			break;
		}
#else
		if (error = suser(u.u_cred, &u.u_acflag))
			break;
#endif
		purge_fs_locks((struct vnode *)fp->f_data);
		break;

	case F_CNVT:
		/*
	 	 * F_CNVT:  given a pointer to a fhandle_t and a mode, open
	 	 * the file corresponding to the fhandle_t with the given mode
	 	 * and return a file descriptor.  Note:  uap->fdes is unused.
	 	 */
#if     SEC_BASE
		if (!privileged(SEC_REMOTE, 0)) {
			error = EPERM;
			break;
		}
#else
		if (error = suser(u.u_cred, &u.u_acflag))
			break;
#endif
		if (copyin((caddr_t) uap->arg, (caddr_t) &a, sizeof (a))) {
			error = EFAULT;
			break;
		} else
			ap = &a;

		if (copyin((caddr_t) ap->fh, (caddr_t) &tfh, sizeof (tfh))) {
			error = EFAULT;
			break;
		}

		filemode = ap->filemode - FOPEN;
		if (filemode & FCREAT) {
			error = EINVAL;
			break;
		}
		mode = 0;
		if (filemode & FREAD)
			mode |= VREAD;
		if (filemode & (FWRITE | FTRUNC))
			mode |= VWRITE;
		/*
		 * Adapted from copen
		 */
		if (error = falloc(&nfp, &indx))
			break;
		ap->fd = indx; /* result for lockmgr */

		/*
		 * This takes the place of lookupname in copen.  We
		 * can't use fhtovp because we want this to work on
		 * files that may not have been exported.
		 */
		if ((vp = myfhtovp(&tfh)) == (struct vnode *) NULL) {
			error = ESTALE;
			goto bad;
		}
		/*
		 * Adapted from vn_open
		 */
		VN_LOCK(vp);
		type = vp->v_type;
		v_flag = vp->v_flag;
		VN_UNLOCK(vp);
		if (type == VSOCK) {
			error = EOPNOTSUPP;
			goto bad;
		}
		if (filemode & (FWRITE | FTRUNC)) {
			if (type == VDIR) {
				error = EISDIR;
				goto bad;
			}
			if (error = vn_writechk(vp)) {
				error = EROFS;
				goto bad;
			}
		}
		VOP_ACCESS(vp, mode, u.u_cred, error);
		if (error)
			goto bad;

		VOP_OPEN(&vp, filemode, u.u_cred, error);
		if ((error == 0) && (filemode & FTRUNC) && (type != VFIFO)) {
			struct vattr vattr;

			filemode &= ~FTRUNC;
			vattr_null(&vattr);
			vattr.va_size = 0;
			VOP_SETATTR(vp, &vattr, u.u_cred, error);
		}
		if (error)
			goto bad;

		/*
		 * Adapted from copen 
		 */
		FP_LOCK(nfp);
		nfp->f_flag = filemode & FMASK;
		nfp->f_type = DTYPE_VNODE;
		nfp->f_data = (caddr_t) vp;
		nfp->f_ops = &vnops;
		if (vp->v_type == VFIFO)
			nfp->f_flag |= (filemode & FNDELAY);
		FP_UNLOCK(nfp);
		U_FD_SET(indx, nfp, &u.u_file_state);
		if (copyout(ap, (caddr_t)uap->arg, sizeof(a))) {
			error = EFAULT;     /* fall through bad */
		} else {
			VN_LOCK(vp);
			if (type != VFIFO) {
				if (filemode & FREAD)
					vp->v_rdcnt++;
				if (filemode & FWRITE)
					vp->v_wrcnt++;
			}
			VN_UNLOCK(vp);
			break;  /* skip bad */
		}
bad:
		U_FD_SET(indx, NULL, &u.u_file_state);
		fdealloc(nfp);
		if (vp)
			vrele(vp);
		break;

	default:
		error = EINVAL;
	}
out:
	FP_UNREF(fp);
	return (error);
}

convoff(fp, efl)
	struct file *fp;
	struct eflock *efl;
{
	short type, whence;
	struct vattr va;
	int error=0;

	type = efl->l_type;
	whence = efl->l_whence;

	/* check l_type is valid */
	if ((type != F_RDLCK) && (type != F_WRLCK) &&
	    (type != F_UNLCK) && (type != F_UNLKSYS))
		return(EINVAL);

	/* if reference to end-of-file, must get current attrs */
	if (whence == L_XTND) {
		VOP_GETATTR((struct vnode *)fp->f_data, &va, u.u_cred, error);
		if (error) return(error);
	} 
	if (whence == L_INCR)
		(long)efl->l_start += fp->f_offset;
	else if (whence == L_XTND)
		(long)efl->l_start += va.va_size;
	else if (whence != L_SET)
		return(EINVAL);
	efl->l_whence = L_SET;
	return(0);
}


/*
 * We require a version of fhtovp that simply converts an fhandle_t to
 * a vnode without any ancillary checking (e.g., whether it's exported).
 */
static struct vnode *
myfhtovp(fh)
	nfsv2fh_t *fh;
{
	int error;
	struct mount *mp;
	struct vnode *vp;

	/*
	 * Check that the file system is mounted on and
	 * if so get a ref on it.
	 */
	mp = getvfs(&fh->fh_generic.fh_fsid);
	if (!mp) {
		return ((struct vnode *) NULL);
	}
	VFS_FHTOVP(mp, &fh->fh_generic.fh_fid, &vp, error);
	MOUNT_LOOKUP_DONE(mp);
	if (error || vp == (struct vnode *)NULL)  {
		return ((struct vnode *) NULL);
	}
	return(vp);
}

fset(fp, bit, value)
	struct file *fp;
	long bit, value;
{

	FP_LOCK(fp);
	if (value)
		fp->f_flag |= bit;
	else
		fp->f_flag &= ~bit;
	FP_UNLOCK(fp);
	/* was (bit == FNDELAY) */
	return (fioctl(fp, 
		(int)((bit & (FNDELAY|FNONBLOCK)) ? FIONBIO : FIOASYNC),
		(caddr_t)&value));
}

fgetown(fp, valuep)
	struct file *fp;
	int *valuep;
{
	int error;

	switch (fp->f_type) {

	case DTYPE_SOCKET:
		return soo_ioctl(fp, (int)SIOCGPGRP, (caddr_t)valuep);

	default:
		error = fioctl(fp, (int)TIOCGPGRP, (caddr_t)valuep);
		*valuep = -*valuep;
		return (error);
	}
}

fsetown(fp, value)
	struct file *fp;
	long value;
{

	if (fp->f_type == DTYPE_SOCKET)
		return soo_ioctl(fp, (int)SIOCSPGRP, (caddr_t)&value);
	if (value > 0) {
		struct proc *p;
		unix_master();
		p = pfind(value);
		if (p == 0) {
			unix_release();
			return (ESRCH);
		}
		value = p->p_pgrp->pg_id;
		unix_release();
	} else
		value = -value;
	return (fioctl(fp, (int)TIOCSPGRP, (caddr_t)&value));
}

fioctl(fp, cmd, value)
	struct file *fp;
	unsigned int cmd;
	caddr_t value;
{
	int	error, ret_val;

	FOP_IOCTL(fp, cmd, value, &ret_val,error);
	return (error);
}

/* ARGSUSED */
close(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct args {
		long	i;
	} *uap = (struct args *)args;
	register int i = uap->i;
	register struct file *fp;
	register struct ufile_state *ufp = &u.u_file_state;
	register struct vnode *vp;
	struct stat sb;
	int error;

	U_FDTABLE_LOCK(ufp);
	if (i < 0 || i > ufp->uf_lastfile) {
		U_FDTABLE_UNLOCK(ufp);
		return (EBADF);
	}
	/*
	 * Can't use getf because the file descriptor table lock
	 * must be held across all these operations.
	 */
	if ((fp = U_OFILE(i, ufp)) == NULL || fp == U_FD_RESERVED) {
		U_FDTABLE_UNLOCK(ufp);
		return (EBADF);
	}
	U_POFILE_SET(i, 0, ufp);
	U_OFILE_SET(i, NULL, ufp);
/*
 * Fred Canter -- 9/24/91
 *	Fix for select hanging instead of returning an error
 *	if the file descriptor has been closed.
 */
#ifdef	notdef
	while (ufp->uf_lastfile >= 0 && (U_OFILE(ufp->uf_lastfile, ufp))== NULL)
		ufp->uf_lastfile--;
#endif
	U_FDTABLE_UNLOCK(ufp);

	/* audit info */
	if ( DO_AUDIT(SYS_close,0) ) {
		if ( (vp = (struct vnode *)fp->f_data) && fp->f_type == DTYPE_VNODE
		&& vn_stat ( vp, &sb ) == 0 ) {
			u.u_vno_dev[0]  = S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode) ? 
                            sb.st_rdev : sb.st_dev;
			u.u_vno_num[0]  = sb.st_ino;
			u.u_vno_mode[0] = sb.st_mode;
			u.u_vno_indx = 1;
		}
	}

	/*
	 * Use closef instead of FP_UNREF to get closef's return value.
	 */
	error = closef(fp);

	AUDIT_CALL ( SYS_close, error, args, *retval, AUD_HPR, (char *)0 );
	return (error);
}

/* ARGSUSED */
fstat(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	struct file *fp;
	register struct args {
		long	fdes;
		struct	stat *sb;
       	} *uap = (struct args *)args;
	struct stat ub;
	int error = 0, rval = 0;

	if (error = getf(&fp, uap->fdes, FILE_FLAGS_NULL, &u.u_file_state))
		return (error);
	switch (fp->f_type) {
	case DTYPE_VNODE:
		error = vn_stat((struct vnode *)fp->f_data, &ub);
		if(!error && S_ISCHR(ub.st_mode)) {
			FOP_IOCTL(fp,I_ISASTREAM,0,&rval,error);
			if(!error && (rval == I_FIFO || rval == I_PIPE)) {
				ub.st_mode &= ~S_IFMT;
				ub.st_mode |= S_IFIFO;
				ub.st_size = 0;
			}
			error = 0;
		}
		break;

	case DTYPE_SOCKET:
		error = soo_stat((struct socket *)fp->f_data, &ub);
		break;

	default:
		panic("fstat");
		/*NOTREACHED*/
	}
	if (error == 0)
		error = copyout((caddr_t)&ub, (caddr_t)uap->sb,
		    sizeof (ub));
	FP_UNREF(fp);
	return (error);
}

/*
 * Allocate a user file descriptor.
 * ufalloc, when successful, returns with
 * the file descriptor slot marked as reserved.
 */
ufalloc(i, result, ufp)
	register long i;
	int *result;
	register struct ufile_state *ufp;
{
	int error;

	U_FDTABLE_LOCK(ufp);
	for (; i < u.u_rlimit[RLIMIT_NOFILE].rlim_cur; i++) {
		if (error = enable_fd_slot(i)) {
			U_FDTABLE_UNLOCK(ufp);
			return(error);
		}
		if (U_OFILE(i, ufp) == U_FD_RESERVED)
			continue;
		if (U_OFILE(i, ufp) == NULL) {
			U_POFILE_SET(i, 0, ufp);
			U_OFILE_SET(i, U_FD_RESERVED, ufp);
			if (i > ufp->uf_lastfile)
				ufp->uf_lastfile = i;
			U_FDTABLE_UNLOCK(ufp);
			*result = i;
			return (0);
		}
	}
	U_FDTABLE_UNLOCK(ufp);
	return (EMFILE);
}

/*
 * Number of available user file descriptors.
 * Even if we lock the entries as we
 * count them, the caller can't be sure
 * that there are really "avail" available.
 */
ufavail(ufp)
register struct ufile_state *ufp;
{
	register int i;
	register int maxfd;
	register int avail;

	U_FDTABLE_LOCK(ufp);
	/*
         * If the process hasn't yet had occasion to allocate its
         * overflow descriptor array out to the maximum extent, confine the
         * search to the preallocated slots and credit the descriptors
         * obtainable by allocating to the maximum extent.
         */
	if (ufp->uf_lastfile < 0)
		return(u.u_rlimit[RLIMIT_NOFILE].rlim_cur);
	else
		maxfd = ufp->uf_lastfile;
	avail = (u.u_rlimit[RLIMIT_NOFILE].rlim_cur - maxfd) - 1;
	for (i = 0; i < maxfd; i++)
		if (U_OFILE(i, ufp) == NULL)
			avail++;
	U_FDTABLE_UNLOCK(ufp);
	return (avail);
}

struct file free_file_head;	/* Head of free file list */

/*
 * Allocate a user file descriptor and a file structure.   The u.u_ofile slot
 * for the allocated file descriptor will be set to U_FD_RESERVED to prevent
 * other threads within the task from using that slot until the file
 * structure has been completely initialized.  The caller is responsible
 * for attaching the file structure to the file descriptor or for freeing
 * both.
 */

falloc(resultfp, resultfd)
	struct file **resultfp;
	int *resultfd;
{
	register struct file *fp;
	int error, i;

	if (error = ufalloc(0, &i, &u.u_file_state))
		return (error);

	FREE_FILE_LOCK();
	if (free_file_head.f_freef == &free_file_head) {
		FREE_FILE_UNLOCK();
		U_FD_SET(i, NULL, &u.u_file_state);
		tablefull("file");
		return(ENFILE);
	}
	fp = free_file_head.f_freef;
	ASSERT(fp->f_count == 0 && fp->f_type == DTYPE_FREE);
	free_file_head.f_freef = fp->f_freef;
	fp->f_freef = NULL;
	/* Keep this for now - XXX */
	fp->f_type = DTYPE_RESERVED;
	FREE_FILE_UNLOCK();
	/* Until we get rid of file table walkers - XXX */
	FP_LOCK(fp);
	fp->f_count = 1;
	fp->f_data = 0;
	fp->f_offset = 0;
#if	SER_COMPAT
	fp->f_funnel = 1;
#endif
	/* Maybe we should lock here - XXX */
	crhold(u.u_cred);
	fp->f_cred = u.u_cred;
	/* Until we get rid of file table walkers - XXX */
	FP_UNLOCK(fp);
	ASSERT(U_OFILE(i, &u.u_file_state) == U_FD_RESERVED);
	ASSERT(fp->f_count == 1 && fp->f_type == DTYPE_RESERVED);
	if (resultfp)
		*resultfp = fp;
	if (resultfd)
		*resultfd = i;
	return(0);
}

/*
 * Deallocate a file structure and return it to the free file list
 */

void
fdealloc(fp)
struct file *fp;
{
	
	/* This may change - XXX */
	ASSERT(fp->f_count == 1 && fp->f_type != DTYPE_FREE && fp->f_cred);
	crfree(fp->f_cred);
	FP_LOCK(fp);
	fp->f_count = 0;
	fp->f_type = DTYPE_FREE;
	fp->f_cred = NULL;
	FP_UNLOCK(fp);
	FREE_FILE_LOCK();
	fp->f_freef = free_file_head.f_freef;
	free_file_head.f_freef = fp;
	FREE_FILE_UNLOCK();
}


/*
 * Convert a user supplied file descriptor into a pointer
 * to a file structure.  Because of MP locking considerations, this is a
 * function rather than a macro.
 */
getf(fpp, fdes, flagsp, ufp)
	struct file **fpp;
	long fdes;
	char *flagsp;
	register struct ufile_state *ufp;
{
	register struct file *fp;

	U_FDTABLE_LOCK(ufp);
	if (fdes < 0 || fdes > ufp->uf_lastfile) {
		U_FDTABLE_UNLOCK(ufp);
		return (EBADF);
	}
	if ((fp = U_OFILE(fdes, ufp)) == NULL || fp == U_FD_RESERVED) {
		U_FDTABLE_UNLOCK(ufp);
		return (EBADF);
	}
	FP_REF(fp);
	if (flagsp != FILE_FLAGS_NULL)
		*flagsp = U_POFILE(fdes, ufp);
	U_FDTABLE_UNLOCK(ufp);
	*fpp = fp;
	return (0);
}

/*
 * Internal form of close.
 * Decrement reference count on file structure.
 */
closef(fp)
	register struct file *fp;
{
	int error;

	if (fp == NULL)
		return (0);
	/*
	 * SV file locking: clean locks, if present
	 */
	if (fp->f_type == DTYPE_VNODE) {
		int flag, error;
		struct vnode *vp = (struct vnode *)fp->f_data;

		BM(VN_LOCK(vp));
		flag = vp->v_flag;
		BM(VN_UNLOCK(vp));
		if (flag & VLOCKS)
			VOP_LOCKCTL(vp, NULL, CLNFLCK, fp->f_cred,
				u.u_procp->p_pid, 0, error);
	}
	FP_LOCK(fp);
	if (fp->f_count > 1) {
		fp->f_count--;
		FP_UNLOCK(fp);
		return (0);
	}
	FP_UNLOCK(fp);
	if (fp->f_count != 1)
		panic("closef: f_count not 1");
	FOP_CLOSE(fp, error);
	fdealloc(fp);
	return (error);
}


/*
 * Apply an advisory lock on a file descriptor.
 */
/* ARGSUSED */
flock(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	fd;
		long	how;
	} *uap = (struct args *)args;
	struct file *fp;
	int error = 0;

	if (error = getf(&fp, uap->fd, FILE_FLAGS_NULL, &u.u_file_state))
		return (error);
	if (fp->f_type != DTYPE_VNODE) {
		error = EOPNOTSUPP;
		goto out;
	}
	if (uap->how & LOCK_UN) {
		vn_funlock(fp, FSHLOCK|FEXLOCK);
		goto out;
	}
	if ((uap->how & (LOCK_SH | LOCK_EX)) == 0) {
		goto out;			/* error? */
	}
	if (uap->how & LOCK_EX)
		uap->how &= ~LOCK_SH;
	/* avoid work... */
	FP_LOCK(fp);
	if ((fp->f_flag & FEXLOCK) && (uap->how & LOCK_EX) ||
	    (fp->f_flag & FSHLOCK) && (uap->how & LOCK_SH)) {
		FP_UNLOCK(fp);
		goto out;
	}
	FP_UNLOCK(fp);
	error = vn_flock(fp, uap->how);
out:
	FP_UNREF(fp);
	return (error);
}

void
file_table_init()
{
	struct file	*fp;

	for (fp = file; fp < fileNFILE; fp++) {
		fp->f_count = 0;
		fp->f_type = DTYPE_FREE;
		fp->f_freef = fp+1;
		FP_LOCK_INIT(fp);
		FP_IO_LOCK_INIT(fp);
	}
	(fp-1)->f_freef = &free_file_head;
	free_file_head.f_freef = file;
	FREE_FILE_LOCK_INIT();
}

/*
 * Allocate descriptor slots in the range [0 .. u.u_rlimit.rlim_cur], 
 * assuming that the range [0 .. NOFILE_IN_U] is currently allocated.  
 * Intended to be called only from the enable_fd_slot macro.
 */
int
alloc_fd_slots(fdno)
	int fdno;  /* fd of interest */
{
	struct file **ofilep = NULL;
	char *pofilep = NULL;
	int count;
	register struct ufile_state *ufp = &u.u_file_state;

	/*
         * The first overflow buffer will contain 64 file descriptors.
         * Subsequent allocations will double buffer size.
         */
	if (ufp->uf_of_count == 0)
		count = NOFILE_IN_U;
	else
		count = 2 * ufp->uf_of_count;
	/*
	 * If the buffer size calculated above is not big enough to be
	 * able to reference the fd of interest, we figure out how big
	 * the buffer needs to be (making it a multiple of NOFILE_IN_U).
	 * This case can happen as a result of a dup2.
	 */
	if (fdno >= (count + NOFILE_IN_U))
		count = (fdno / NOFILE_IN_U) * NOFILE_IN_U;

	/*
	 * Allocate new overflow buffers.  
	 * Make sure the buffer entries are zeroed.
	 */
	ofilep = (struct file **) kalloc(count * sizeof(struct file *));
	if ((caddr_t)ofilep == NULL)
		return(ENOMEM);
	if ((pofilep = (char *)kalloc(count)) == NULL) {
		kfree(ofilep, count * sizeof(struct file *));
		return(ENOMEM);
	}
	bzero((caddr_t)ofilep, count * sizeof(struct file *));
	bzero((caddr_t)pofilep, count);
	
	/*
	 * Copy contents of old overflow buffers into the new buffers
	 */
	if (ufp->uf_of_count) {
		bcopy(ufp->uf_ofile_of, ofilep, 
			ufp->uf_of_count * sizeof(struct file *));
		bcopy(ufp->uf_pofile_of, pofilep, ufp->uf_of_count);
		/*
	 	 * Release previous memory back to the pool.
	 	 */
		kfree(ufp->uf_ofile_of,
			ufp->uf_of_count * sizeof(struct file *));
		kfree(ufp->uf_pofile_of, ufp->uf_of_count);
	}
	/*
	 * Update pointers and current count.
	 */
	ufp->uf_ofile_of = ofilep;
	ufp->uf_pofile_of = pofilep;
	ufp->uf_of_count = count;
	return(0);
}
