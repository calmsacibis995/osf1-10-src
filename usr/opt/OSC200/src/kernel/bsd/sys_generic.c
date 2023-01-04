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
static char *rcsid = "@(#)$RCSfile: sys_generic.c,v $ $Revision: 4.4.18.10 $ (DEC) $Date: 1993/08/19 12:32:56 $";
#endif 

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
 * sys_generic.c
 *
 * Modification History:
 *
 *  8-Oct-91    Philip Cameron
 *      Added support for Ultrix 4.2 't' ioctls. Not all of the 't' ioctls are 
 *	fixed here since there are some OSF ioctls that have the save encoding 
 *	of the name as a different Ultrix ioctl. These ioctls will be fixed
 *	when the code can detect which type of executable is making the request.
 *
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *
 */

#include <sys/unix_defs.h>
#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <procfs/procfs.h> /* for /proc fs ioctl info */

#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>
#include <kern/kalloc.h>
#include <sys/dk.h>

#include <dcedfs.h>

#include <builtin/ux_exception.h>
#include <mach/exception.h>

extern int procfs_ioctl();
extern struct vnodeops procfs_vnodeops;
extern int open_max_soft;

/*
 * Read system call.
 */
read(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	fdes;
		char	*cbuf;
		unsigned long  count;
	} *uap = (struct args *) args;
	struct uio auio;
	struct iovec aiov;

        ts_sysread++; /* global table() system call counter (see table.h) */

	aiov.iov_base = (caddr_t)uap->cbuf;
	aiov.iov_len = uap->count;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_rw = UIO_READ;
	return (rwuio(p, uap->fdes, &auio, UIO_READ, retval));
}

readv(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	fdes;
		struct	iovec *iovp;
		unsigned long iovcnt;
	} *uap = (struct args *) args;
	struct uio auio;
	register struct iovec *iov;
	int error;
	struct iovec aiov[UIO_SMALLIOV];

        ts_sysread++; /* global table() system call counter (see table.h) */

	if (uap->iovcnt > UIO_SMALLIOV) {
		if (uap->iovcnt > UIO_MAXIOV)
			return (EINVAL);	
		if ((iov = (struct iovec *)
			    kalloc(sizeof(struct iovec) * (uap->iovcnt))) == 0)
			return (ENOMEM);
	} else {
		if (uap->iovcnt == 0)
			return (EINVAL);
		iov = aiov;
	}
	auio.uio_iov = iov;
	auio.uio_iovcnt = uap->iovcnt;
	auio.uio_rw = UIO_READ;
	error = copyin((caddr_t)uap->iovp, (caddr_t)iov,
	    uap->iovcnt * sizeof (struct iovec));
	if (!error)
		error = rwuio(p, uap->fdes, &auio, UIO_READ, retval);
	if (uap->iovcnt > UIO_SMALLIOV)
		kfree(iov, sizeof(struct iovec)*uap->iovcnt);
	return (error);
}

/*
 * Write system call
 */
write(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	fdes;
		char	*cbuf;
		unsigned long count;
	} *uap = (struct args *) args;
	struct uio auio;
	struct iovec aiov;


        ts_syswrite++;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	auio.uio_rw = UIO_WRITE;
	aiov.iov_base = uap->cbuf;
	aiov.iov_len = uap->count;
	return (rwuio(p, uap->fdes, &auio, UIO_WRITE, retval));
}

writev(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	fdes;
		struct	iovec *iovp;
		unsigned long iovcnt;
	} *uap = (struct args *) args;
	struct uio auio;
	register struct iovec *iov;
	int error;
	struct iovec aiov[UIO_SMALLIOV];

	if (uap->iovcnt > UIO_SMALLIOV) {
		if (uap->iovcnt > UIO_MAXIOV)
			return (EINVAL);	
		if ((iov = (struct iovec *)
			    kalloc(sizeof(struct iovec) * (uap->iovcnt))) == 0)
			return (ENOMEM);
	} else {
		if (uap->iovcnt == 0)
			return (EINVAL);
		iov = aiov;
	}
	auio.uio_iov = iov;
	auio.uio_iovcnt = uap->iovcnt;
	auio.uio_rw = UIO_WRITE;
        ts_syswrite++;
	error = copyin((caddr_t)uap->iovp, (caddr_t)iov,
	    uap->iovcnt * sizeof (struct iovec));
	if (!error)
		error = rwuio(p, uap->fdes, &auio, UIO_WRITE, retval);
	if (uap->iovcnt > UIO_SMALLIOV)
		kfree(iov, sizeof(struct iovec)*uap->iovcnt);
	return (error);
}

rwuio(p, fdes, uio, rw, retval)
	struct proc *p;
	long fdes;
	register struct uio *uio;
	enum uio_rw rw;
	long *retval;
{
	struct file *fp;
	register struct iovec *iov;
	int i, count, flag, error;

	/* We could inline getf for performance */
	if (error = getf(&fp, fdes, FILE_FLAGS_NULL, &u.u_file_state))
		return (error);
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	if ((flag&(rw==UIO_READ ? FREAD : FWRITE)) == 0) {
		error = EBADF;
		goto out;
	}
	uio->uio_resid = 0;
	uio->uio_segflg = UIO_USERSPACE;
	iov = uio->uio_iov;
	for (i = 0; i < uio->uio_iovcnt; i++) {
		if (iov->iov_len < 0) {
			error = EINVAL;
			goto out;
		}
		uio->uio_resid += iov->iov_len;
		if (uio->uio_resid < 0) {
			error = EINVAL;
			goto out;
		}
		iov++;
	}
	count = uio->uio_resid;
	/*
	 * We don't need to crhold fp->f_cred because we already
	 * hold a reference on the file structure.  This prevents
	 * fp->f_cred from being freed.
	 */
	if (rw == UIO_READ)
		FOP_READ(fp, uio, fp->f_cred, error);
	else
		FOP_WRITE(fp, uio, fp->f_cred, error);
	if (error) {
		/*
		 * If some data has been moved, then we should
		 * report the movement count rather than the error.
		 */
		if (uio->uio_resid != count)
			error = 0;
		if ((error == EPIPE) && (rw == UIO_WRITE)) {
			thread_doexception(current_thread(), EXC_SOFTWARE,
			EXC_UNIX_BAD_PIPE, 0);
		}
	}
	*retval = count - uio->uio_resid;
	u.u_ioch += (unsigned long) *retval;

        /* global table() system call counter (see table.h) */
	if (rw == UIO_READ)
		ts_readch +=  (unsigned) *retval;
	else
		ts_writech +=  (unsigned) *retval;

out:
	FP_UNREF(fp);
	return (error);
}

/*
 * Ioctl system call
 */
ioctl(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	/*
	 * All ioctl calls now intercepted by DCE DFS
	 */

#if defined(DCEDFS) && DCEDFS
	return (dfs_xioctl (p, args, retval));
#else
	return (ioctl_base (p, args, retval));
#endif  /* DCEDFS */

}

/*
 * original, base Ioctl system call
 * now called from new ioctl entry point (above)
 */
ioctl_base (p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct vnode *vp;
	register struct args {
		long	fdes;
		u_long	cmd;
		caddr_t	cmarg;
	} *uap = (struct args *) args;
	struct file *fp;
	register int error, flag;
	register u_int com;
	register u_int size, real_size;
	caddr_t memp = 0;
	caddr_t procfs_addr = 0;
#define STK_PARMS	128
	char stkbuf[STK_PARMS];
	caddr_t data = stkbuf;
	struct ufile_state *ufp = &u.u_file_state;

	error = 0;
	if (error = getf(&fp, uap->fdes, FILE_FLAGS_NULL, ufp))
		return (error);
	BM(FP_LOCK(fp));
	flag = fp->f_flag;
	BM(FP_UNLOCK(fp));
	if ((flag & (FREAD|FWRITE)) == 0) {
		error = EBADF;
		goto out;
	}
	com = (u_int) uap->cmd;
	if (com == FIOCLEX) {
		U_FDTABLE_LOCK(ufp);
		U_POFILE_SET(uap->fdes, U_POFILE(uap->fdes, ufp) | UF_EXCLOSE,
			ufp);
		U_FDTABLE_UNLOCK(ufp);
		goto out;
	}
	if (com == FIONCLEX) {
		U_FDTABLE_LOCK(ufp);
		U_POFILE_SET(uap->fdes, U_POFILE(uap->fdes, ufp) & ~UF_EXCLOSE,
			ufp);
		U_FDTABLE_UNLOCK(ufp);
		goto out;
	}

	/*
	 * Interpret high order word to find
	 * amount of data to be copied to/from the
	 * user's address space.
	 */
	real_size = size = IOCPARM_LEN(com);
	/* /proc */
	vp = ((struct vnode *)fp->f_data);

	if (vp->v_op == &procfs_vnodeops) {
		register struct procnode *prcnd;
		/* Ensure that the file was opened for write, if any "control"
		 * ioctls are invoked.  If not, return EBADF.
		 */
		if((fp->f_flag & FWRITE) == NULL)
		{
		    switch(com) {
		    case PIOCSTOP:
		    case PIOCRUN:
		    case PIOCSTRACE:
		    case PIOCSSIG:
		    case PIOCKILL:
		    case PIOCUNKILL:
		    case PIOCSHOLD:
		    case PIOCSFAULT:
		    case PIOCCFAULT:
		    case PIOCSENTRY:
		    case PIOCSEXIT:
		    case PIOCSFORK:
		    case PIOCRFORK:
		    case PIOCSRLC:
		    case PIOCRRLC:
		    case PIOCSREG:
		    case PIOCSFPREG:
		    case PIOCNICE:
			error = EBADF;
			goto out;
		    default:
			break;
		    }
		}


		/* changes for /proc start here */
		prcnd = VNTOPN(vp);

		switch(com) {
		case PIOCRUN:	/* Allow a null pointer as the data argument */
		case PIOCSTOP:
		case PIOCWSTOP:
		case PIOCSSIG:
		case PIOCOPENM:
			if (uap->cmarg == 0) {
				real_size = size = 0;
				com &= ~IOC_INOUT;
				com |= IOC_VOID;
				com &= ~(0x1fff << 16);
			}
			break;
		case PIOCGROUPS:  /* Use kernel info to alloce full list size */
			real_size = size = size *
				proc[prcnd->prc_index].p_rcred->cr_ngroups;
			break;
		case PIOCACTION:  /* Use kernel info to alloc full list size */
			real_size = size = size * NSIG;
			break;
		case PIOCMAP:	/* Use kernel info to alloc full list size */
			real_size = size = size *
				proc[prcnd->prc_index].task->procfs.pr_nmap;
			break;
		case PIOCTLIST:	/* Use kernel info to alloce full list size */
			real_size = size = size *
				proc[prcnd->prc_index].task->thread_count;
			break;
		case PIOCNTHR:	/* Use kernel info to alloce full list size */
			real_size = size = size *
				proc[prcnd->prc_index].task->thread_count;
			break;

		case PIOCTRUN: 	    /* Special processing for thread ioctls */
		case PIOCTABRUN:
		case PIOCTGETTH:
		case PIOCTSTOP:
		case PIOCTSTATUS:
		case PIOCTGTRACE:
		case PIOCTSTRACE:
		case PIOCTSSIG:
		case PIOCTKILL:
		case PIOCTUNKILL:
		case PIOCTCFAULT:
		case PIOCTGFAULT:
		case PIOCTSFAULT:
		case PIOCTGFPREG:
		case PIOCTSFPREG:
		case PIOCTGREG:
		case PIOCTSREG:
			real_size = size =
				(size * ((prthreads_t *)(uap->cmarg))->pr_count)
				+ sizeof(prthreads_t);
			break;
		}
	}
	/* end changes for /proc */
	if (size > IOCPARM_MAX) {
		error = ENOTTY;
		goto out;
	}
	if (size > sizeof (stkbuf)) {
		if ((memp = (caddr_t)kalloc(size)) == 0) {
			error = ENOMEM;
			goto out;
		}
		data = memp;
	}
	if (com&IOC_IN) {
#if	mips || sun	/* Needed on Sun's to make Suntools happy */
		if (size == sizeof (int) && uap->cmarg == NULL)
			*(int *)data = 0;
		else
#endif
		if (size) {
			if (error = 
			    copyin(uap->cmarg, (caddr_t)data, (u_int)real_size))
				goto out;
		} else
			*(caddr_t *)data = uap->cmarg;
	} else if ((com&IOC_OUT) && size)
		/*
		 * Zero the buffer on the stack so the user
		 * always gets back something deterministic.
		 */
		bzero((caddr_t)data, real_size);
	else if (com&IOC_VOID)
		/* Modified for /proc was:
		 * *(caddr_t *)data = uap->cmarg;
		 * only */
		if(com != PIOCOPENM)
			*(caddr_t *)data = uap->cmarg;
		else {
			(caddr_t *)data = &procfs_addr;
		}
		/* End of /proc changes */

	switch (com) {
	case FIONBIO:
		/* was fset(fp, FNDELAY, *(int *)data); */
		error = fset(fp, FNONBLOCK, *(int *)data);
		break;

	case FIOASYNC:
		error = fset(fp, FASYNC, *(int *)data);
		break;

	case FIOSETOWN:
		error = fsetown(fp, *(int *)data);
		break;

	case FIOGETOWN:
		error = fgetown(fp, (int *)data);
		break;
	default:
		/*
		 * We expect the lower-level routine to lock and unlock
		 * the file structure as necessary.  We guarantee that the
		 * file structure won't disappear because we hold a reference
		 * on the structure courtesy of getf.
		 */


		FOP_IOCTL(fp, com, data, retval, error);

		/*
		 * Copy any data to user, real_size was
		 * already set and checked above.
		 */
		if (error == 0 && (com&IOC_OUT) && real_size) {
			error = copyout(data, uap->cmarg, (u_int)real_size);
		}
		/* New for /proc - OPENM needs to return a fd here */
		if((error == 0) && (uap->cmd == PIOCOPENM)) {
			*retval = *(int *)data;
		}
		/* End of /proc code */

		break;
	}
out:
	FP_UNREF(fp);

	/* audit security relevant ioctl's */
	if ( audswitch == 1 ) switch ( com ) {
	case TIOCSTI:
		{
		long arg[3];
		arg[0] = (long)"TIOCSTI";
		arg[1] = uap->fdes;
		arg[2] = (long)*data;
		AUDIT_CALL ( SYS_ioctl, error, arg, 0, AUD_VHPR, "rCa000000" );
		}
	}

	if (memp)
		kfree(memp, size);
	return (error);
}

/*
 * Select/Poll.
 */

/*ARGSUSED*/
seltrue(
	dev_t dev,
	short *events,
	short *revents,
	int scanning)
{
	if (scanning)
		*revents = *events;
	return (0);
}

int
selscan(
	register struct pollfd *fd_ptr,
	register struct file **fp_ptr,
	unsigned long *pnfds,
	long *retval)
{
	long number = 0;
	int error;
	register unsigned long nfds = *pnfds;
	struct file *fp;
	struct ufile_state *ufp = &u.u_file_state;

	while (nfds > 0) {
		fd_ptr->revents = 0;
		U_FDTABLE_LOCK(ufp);
		if (fd_ptr->fd > ufp->uf_lastfile ||
		     (fp = U_OFILE(fd_ptr->fd, ufp)) == NULL ||
		     fp == U_FD_RESERVED) {
			U_FDTABLE_UNLOCK(ufp);
			*pnfds -= nfds;
			return (EBADF);
		}
		FP_REF(fp);
		U_FDTABLE_UNLOCK(ufp);
		FOP_SELECT(fp, &fd_ptr->events, &fd_ptr->revents, 1, error);
		if (error || (fd_ptr->revents & POLLNVAL)) {
			FP_UNREF(fp);
			*pnfds -= nfds;
			if (error)
				return (error);
			return (EBADF);
		}
		/*
		 * For select, POLLHUP means readable, POLLERR means both.
		 * And always, be sure to ignore any extra bits.
		 */
		if (fd_ptr->revents & POLLERR)
			fd_ptr->revents |= (POLLNORM|POLLOUT);
		if (fd_ptr->revents & POLLHUP)
			fd_ptr->revents |= POLLNORM;
		if (fd_ptr->revents &= fd_ptr->events)
			number++;
		*fp_ptr++ = fp;
		fd_ptr++;
		nfds--;
	}
	*retval = number;
	return (0);
}

int
pollscan(
	register struct pollfd *fd_ptr,
	register struct file **fp_ptr,
	unsigned long *pnfds,
	long *retval)
{
	long number = 0;
	int error;
	register unsigned long nfds = *pnfds;
	struct file *fp;
	struct ufile_state *ufp = &u.u_file_state;

	while (nfds > 0) {
		fd_ptr->revents = 0;
		if (fd_ptr->fd >= 0) {
			U_FDTABLE_LOCK(ufp);
			if (fd_ptr->fd > ufp->uf_lastfile ||
			     (fp = U_OFILE(fd_ptr->fd, ufp)) == NULL ||
			     fp == U_FD_RESERVED) {
				U_FDTABLE_UNLOCK(ufp);
				fp = NULL;
				fd_ptr->revents |= POLLNVAL;
				number++; 
			} else {
				FP_REF(fp);
				U_FDTABLE_UNLOCK(ufp);
				FOP_SELECT(fp, &fd_ptr->events, &fd_ptr->revents, 1, error);
				if (error == 0) {
					if (fd_ptr->revents)
						number++;
				} else {
					FP_UNREF(fp);
					if (error != EBADF) {
						*pnfds -= nfds;
						return (error);
					}
					fp = NULL;
					fd_ptr->revents |= POLLNVAL;
					number++; 
				}
			}
		} else
			fp = NULL;
		*fp_ptr++ = fp;
		nfds--;
		fd_ptr++;
	}
	*retval = number;
	return (0);
}

static void
undo_scan(
	register struct pollfd *fd_ptr,
	register struct file **fp_ptr,
	register unsigned long nfds)
{
	int error;
	register struct file *fp;

	while (nfds-- > 0) {
		if (fp = *fp_ptr++) {
			FOP_SELECT(fp, &fd_ptr->events, (short*)NULL, 0, error);
			FP_UNREF(fp);
		}
		++fd_ptr;
	}
}

static int
do_scan(
	struct pollfd *fd_ptr,
	struct file **fp_ptr,
	unsigned long nfds,
	long *retval,
	int (*scan)(struct pollfd *, struct file **, unsigned long *, long *),
	struct timeval *atv)
{
	int timo = 0, error, s;
	event_t *eventp = &current_thread()->select_event;

	if (atv) {
		s = splhigh();
		TIME_READ_LOCK();
		timevaladd(atv, &time);
		TIME_READ_UNLOCK();
		splx(s);
	}
	for (;;) {
		event_clear(eventp);
		error = (*scan)(fd_ptr, fp_ptr, &nfds, retval);
		if (error || *retval)
			break;
		if (atv) {
			s = splhigh();
			TIME_READ_LOCK();
			/* this should be timercmp(&time, atv, >=) */
			if (time.tv_sec > atv->tv_sec ||
			    (time.tv_sec == atv->tv_sec &&
			     time.tv_usec >= atv->tv_usec)) {
				TIME_READ_UNLOCK();
				splx(s);
				break;
			}
			TIME_READ_UNLOCK();
			splx(s);
			timo = hzto(atv);
		}
		error = event_wait(eventp, TRUE, timo);
		if (error)
			break;
		undo_scan(fd_ptr, fp_ptr, nfds);
	}
	undo_scan(fd_ptr, fp_ptr, nfds);
	if (error == ERESTART)
		error = EINTR;	/* not restartable */
	else if (error == EWOULDBLOCK)
		error = 0;
	return (error);
}

#define LOCAL_FDS	32	/* This many fd's locally polled/selected */

/*
 * Select system call.
 */
select(
	struct proc *p,
	void *args,
	long *retval)
{
	struct args {
		long	nd;
		fd_set	*in, *ou, *ex;
		struct	timeval *tv;
	} *uap = (struct args *) args;
	register fd_mask *inbits, *oubits, *exbits;
	fd_mask rbits, wbits, ebits, tbit, bits;
	char *ip;
	int isize;
	register int i, j;
        long number;
	struct timeval atv;
	int nfd, error, tsize;
	register struct pollfd *fd_ptr;
	struct file **fp_ptr;
	register unsigned long f;
	struct pollfd fd_local[LOCAL_FDS];
	struct file *fp_local[LOCAL_FDS];
	fd_mask lbits[6 * howmany(LOCAL_FDS, NFDBITS)];
	struct thread *th = current_thread();

	/* BSD comment is "forgiving, if slightly wrong" */
        if ((nfd = uap->nd) > (th->u_address.utask->uu_file_state.uf_lastfile
	    + 1))
                nfd = uap->nd = 
                    th->u_address.utask->uu_file_state.uf_lastfile + 1;

	/*
	 * Copy in fd_sets and prepare pollfd array. Being pessimistic
	 * on sizes of bit arrays and pollfd's uses more memory but
	 * improves speed by simplifying loops. Also note cannot size
	 * pollfd array until copyins are complete.
	 */
	tsize = howmany(nfd, NFDBITS) * sizeof(fd_mask);
	i = 6 * tsize;
	if (nfd <= LOCAL_FDS) {
		ip = (char *)lbits;
		fd_ptr = fd_local;
		fp_ptr = fp_local;
	} else {
		isize = i + nfd * (sizeof (struct pollfd) 
			+ sizeof (struct file *));
		if ((ip = (char *) kalloc (isize)) == NULL) {
                        error = EAGAIN;
                        goto out;
                }
		fd_ptr = (struct pollfd *)&ip[i];
		fp_ptr = (struct file **)&ip[i + nfd * sizeof(struct pollfd)];
	}
	bzero(ip, i);
	inbits = (fd_mask *)(&ip[0]);
	oubits = (fd_mask *)(&ip[tsize * 2]);
	exbits = (fd_mask *)(&ip[tsize * 4]);
	if ((uap->in &&
	     (error = copyin((caddr_t)uap->in, (caddr_t)inbits, tsize))) ||
	    (uap->ou &&
	     (error = copyin((caddr_t)uap->ou, (caddr_t)oubits, tsize))) ||
	    (uap->ex &&
	     (error = copyin((caddr_t)uap->ex, (caddr_t)exbits, tsize))))
		goto out;

	/*
	 * Build pollfd array from select fd_sets.
	 */
	for (f = 0L, i = 0; i < nfd; i += NFDBITS) {
		rbits = *inbits++;
		wbits = *oubits++;
		ebits = *exbits++;
		bits = (rbits|wbits|ebits);
		if (j = ffs(bits)) {
			tbit = (1 << --j);
			while (j < NFDBITS && i + j < nfd) {
				if (bits & tbit) {
					fd_ptr[f].fd = i + j;
					fd_ptr[f].events = 0;
					if (rbits & tbit)
						fd_ptr[f].events |= POLLNORM;
					if (wbits & tbit)
						fd_ptr[f].events |= POLLOUT;
					if (ebits & tbit)
						fd_ptr[f].events |= POLLPRI;
					++f;
					if ((bits ^= tbit) == 0)
						break;
				}
				tbit <<= 1;
				++j;
			}
		}
	}

	if (uap->tv) {
		error = copyin((caddr_t)uap->tv, (caddr_t)&atv, sizeof (atv));
		if (error == 0 && (error = itimerfix(&atv)) == 0)
			error = do_scan(fd_ptr, fp_ptr, f, retval,
					selscan, &atv);
	} else
		error = do_scan(fd_ptr, fp_ptr, f, retval,
				selscan, (struct timeval*)0);
	if (error)
		goto out;

	/*
	 * Reconstitute fd_set bits from poll results.
	 */
	number = 0;
	while (f-- > 0) {
		if (fd_ptr[f].revents) {
			tbit = 1 << (fd_ptr[f].fd % NFDBITS);
			j = (fd_ptr[f].fd / NFDBITS);
			if (fd_ptr[f].revents & POLLNORM) {
				++number; inbits[j] |= tbit;
			}
			if (fd_ptr[f].revents & POLLOUT) {
				++number; oubits[j] |= tbit;
			}
			if (fd_ptr[f].revents & POLLPRI) {
				++number; exbits[j] |= tbit;
			}
		}
	}
	*retval = number;
	if (uap->in)
		error = copyout((caddr_t)inbits, (caddr_t)uap->in, tsize);
	if (error == 0 && uap->ou)
		error = copyout((caddr_t)oubits, (caddr_t)uap->ou, tsize);
	if (error == 0 && uap->ex)
		error = copyout((caddr_t)exbits, (caddr_t)uap->ex, tsize);
out:
	if (ip != (char *)lbits)
		kfree(ip, isize);
	return (error);
}

/*
 * Poll system call.
 */
poll(
	struct proc *p,
	void *args,
	long *retval)
{
	register struct args  {
		struct pollfd	*fds;
		unsigned long	nd;
		long 		timeout;
	} *uap = (struct args *) args;
	struct timeval atv;
	unsigned int allocsize;
	struct pollfd *fd_ptr, fd_local[LOCAL_FDS];
	struct file **fp_ptr, *fp_local[LOCAL_FDS];
	int error, timo, fd_size;
	struct thread *th = current_thread();

	if ((timo = uap->timeout) < -1)
		return (EINVAL);
	if (uap->nd == 0)
		return (0);
	BM(U_HANDY_LOCK());
        /* 
         * allow worst case: max number of possible open file desciptors
	 * this is per SVR4 description of poll()
         */
	if (uap->nd > open_max_soft) {
		BM(U_HANDY_UNLOCK());
		return (EINVAL);
	}
	BM(U_HANDY_UNLOCK());

	allocsize = uap->nd * sizeof(struct pollfd);
	if (allocsize <= sizeof fd_local) {
		fd_ptr = fd_local;
		fp_ptr = fp_local;
	} else {
		fd_size = allocsize + uap->nd * sizeof(struct file *);
		if ((fd_ptr = (struct pollfd *) kalloc (fd_size)) == NULL) {
                        error = EAGAIN;
                        goto out;
                }
		fp_ptr = (struct file **)((char *)fd_ptr + allocsize);
	}

	if (error = copyin((caddr_t)uap->fds, (caddr_t)fd_ptr, allocsize))
		goto out;

	if (timo >= 0) {
		/* timeout is in milliseconds */
		atv.tv_usec = 1000 * (timo % 1000);
		atv.tv_sec = timo / 1000;
		error = do_scan(fd_ptr, fp_ptr, (unsigned long)uap->nd, retval,
				pollscan, &atv);
	} else
		error = do_scan(fd_ptr, fp_ptr, (unsigned long)uap->nd, retval, 
				pollscan, (struct timeval *)0);

	if (error == 0)
		error = copyout((caddr_t)fd_ptr, (caddr_t)uap->fds, allocsize);
out:
	if (fd_ptr != fd_local)
		kfree(fd_ptr, fd_size);
	return (error);
}
