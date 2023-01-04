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
static char *rcsid = "@(#)$RCSfile: uipc_syscalls.c,v $ $Revision: 4.3.15.5 $ (DEC) $Date: 1993/07/08 23:04:29 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
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
 * Copyright (c) 1982, 1986, 1989, 1990 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	Base:	@(#)uipc_syscalls.c	2.1 (Berkeley) 12/3/90
 *	Merged: uipc_syscalls.c	7.20 (Berkeley) 6/30/90
 *
 * Edit History:
 * -------------
 * 26-Jun-1991	Ajay Kachrani
 *	In sockargs, increase the mbuf data offset for MT_CONTROL to avoid
 *	forming mbuf chain so that unp_internlize can handle 4.3 msghdr format
 */

#include "net/net_globals.h"
#if	MACH
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/file.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#if	MACH
#include "kern/parallel.h"

#if	SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#endif
#endif

#include <builtin/ux_exception.h>
#include <mach/exception.h>

LOCK_ASSERTL_DECL

#if	defined(COMPAT_43) && !defined(BYTE_ORDER)
/*#error*/ BYTE_ORDER not defined - necessary for COMPAT_43 networking
#endif

/*
 * System call interface to the socket abstraction.
 */

extern	struct fileops socketops;

/* ARGSUSED */
socket(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	domain;		/* real type: 'int' */
		long	type;		/* real type: 'int' */
		long	protocol;	/* real type: 'int' */
	} *uap = (struct args *) args;
	struct socket *so;
	struct file *fp;
	int fd, error;

	if (error = falloc(&fp, &fd))
		return (error);
	FP_LOCK(fp);
	fp->f_flag = FREAD|FWRITE;
	fp->f_type = DTYPE_SOCKET;
	fp->f_ops = &socketops;
#if	UNI_COMPAT
	fp->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(fp);
	if (error = socreate((int)uap->domain, &so, (int)uap->type, (int)uap->protocol)) {
		U_FD_SET(fd, NULL, &u.u_file_state);
		fdealloc(fp);
	} else {
		FP_LOCK(fp);
		fp->f_data = (caddr_t)so;
		ASSERT(fp->f_count == 1);
		FP_UNLOCK(fp);
		U_FD_SET(fd, fp, &u.u_file_state);
		*retval = fd;
	}
	return (error);
}

/* ARGSUSED */
bind(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	name;		/* real type: 'struct sockaddr *' */
		long	namelen;	/* real type: 'int' */
	} *uap = (struct args *) args;
	register struct file *fp;
	struct mbuf *nam;
	int error;

	fp = getsock((int)uap->s, &error);
	if (fp == 0)
		return (error);
	error = sockargs(&nam, uap->name, (int)uap->namelen, MT_SONAME);
	if (error == 0) {
		error = sobind((struct socket *)fp->f_data, nam);
		m_freem(nam);
	}
	FP_UNREF(fp);
	return (error);
}

/* ARGSUSED */
listen(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		long	backlog;	/* real type: 'int' */
	} *uap = (struct args *) args;
	register struct file *fp;
	int error;

	fp = getsock((int)uap->s, &error);
	if (fp == 0)
		return (error);
	error = solisten((struct socket *)fp->f_data, (int)uap->backlog);
	FP_UNREF(fp);
	return (error);
}

#ifdef COMPAT_43
accept(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (accept1(p, args, retval, 0));
}

oaccept(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (accept1(p, args, retval, 1));
}
#else /* COMPAT_43 */

#define accept1(p, args, retval, compat_43)	accept(p, args, retval) 
#endif

/* ARGSUSED */
accept1(p, args, retval, compat_43)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	name;		/* real type: 'struct sockaddr *' */
		int	*anamelen;	/* real type: 'int *' */
	} *uap = (struct args *) args;
	register struct file *fp;
	struct file *nfp;
	struct mbuf *nam = 0;
	int namelen, error;
	register struct socket *so;
	struct socket *aso;
	int nonblock;
	int nindex;
	DOMAIN_FUNNEL_DECL(f)

	if (uap->name && (error = copyin((caddr_t)uap->anamelen,
	    (caddr_t)&namelen, sizeof (namelen))))
		return (error);
	fp = getsock((int)uap->s, &error);
	if (fp == 0)
		return (error);
	so = (struct socket *)fp->f_data;
again:
	BM(FP_LOCK(fp));
	nonblock = (fp->f_flag & (FNDELAY|FNONBLOCK));
	BM(FP_UNLOCK(fp));
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if ((so->so_options & SO_ACCEPTCONN) == 0) {
		error = EINVAL;
		goto out;
	}
	if (nonblock && so->so_qlen == 0) {
		error = EWOULDBLOCK;
		goto out;
	}
	while (so->so_qlen == 0 && so->so_error == 0) {
		if (so->so_state & SS_CANTRCVMORE) {
			so->so_error = ECONNABORTED;
			break;
		}
		if (error = sosleep(so, (caddr_t)&so->so_timeo,
		    (PZERO+1) | PCATCH, 0))
			goto out;
	}
	if (so->so_error) {
		error = so->so_error;
		so->so_error = 0;
		goto out;
	}
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);

	/*
	 * We used to do various things BEFORE dequeuing the new socket
	 * from the head, but it creates several race conditions. It is
	 * necessary to get hold of the new socket first. The only thing
	 * that will behave differently is a failure of falloc. Previously
	 * the socket would not be lost.
	 */
	if (error = sodequeue(so, &aso, &nam, (int)compat_43)) {
		/* We may have lost the connection when we unlocked "so". */
		if (error == ENOTCONN)
			goto again;
		goto out2;
	}

#if	SEC_ARCH
	/*
	 * Perform an access check between the prototype socket and
	 * the new one.  The completely obvious choice of SP_IOCTLACC is
	 * to force a MAC-only equality check.  We should probably
	 * define a new access type with equivalent semantics and
	 * more mnemonic value. The audit hook is used to store
	 * the new socket tags (assuming the new socket and the
	 * prototype will be at the same levels.
	 *
	 * It is not necessary to lock the sockets simply to peek at
	 * the (constant) so_tag's.
	 */
	if (security_is_on) {
	if (SP_ACCESS(so->so_tag, aso->so_tag, SP_IOCTLACC, NULL))
		error = EACCES;
	}

	if (error || (error = falloc(&nfp, &nindex))) {
#else
	if (error = falloc(&nfp, &nindex)) {
#endif
		(void) soabort(aso);
		(void) soclose(aso);
		*retval = nindex;
		goto out2;
	}
	*retval = nindex;
	FP_LOCK(nfp);
	ASSERT(nfp->f_count == 1 && nfp->f_type == DTYPE_RESERVED);
	nfp->f_type = DTYPE_SOCKET;
	nfp->f_flag = FREAD|FWRITE;
	nfp->f_ops = &socketops;
	nfp->f_data = (caddr_t)aso;
#if	UNI_COMPAT
	nfp->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(nfp);
	if (uap->name) {
		if (namelen > nam->m_len)
			namelen = nam->m_len;
		/* SHOULD COPY OUT A CHAIN HERE */
		if ((error = copyout(mtod(nam, caddr_t), (caddr_t)uap->name,
		    (u_int)namelen)) == 0)
			error = copyout((caddr_t)&namelen,
			    (caddr_t)uap->anamelen, sizeof (*uap->anamelen));
	}
	m_freem(nam);
	FP_UNREF(fp);
	U_FD_SET(nindex, nfp, &u.u_file_state);
	return (error);

out:
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
out2:
	FP_UNREF(fp);
	if (nam)
		m_freem(nam);
	return (error);
}

/* ARGSUSED */
connect(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	name;		/* real type: 'struct sockaddr *' */
		long	namelen;	/* real type: 'int' */
	} *uap = (struct args *) args;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *nam;
	int nonblock, error;
	DOMAIN_FUNNEL_DECL(f)

	fp = getsock((int)uap->s, &error);
	if (fp == 0)
		return (error);
	so = (struct socket *)fp->f_data;
	BM(FP_LOCK(fp));
	nonblock = (fp->f_flag & (FNDELAY|FNONBLOCK));
	BM(FP_UNLOCK(fp));
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (nonblock && (so->so_state & SS_ISCONNECTING)) {
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		FP_UNREF(fp);
		return (EALREADY);
	}
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);	/* XXX suboptimal? */
	error = sockargs(&nam, uap->name, (int)uap->namelen, MT_SONAME);
	if (error) {
		FP_UNREF(fp);
		return (error);
	}
	error = soconnect(so, nam);
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if (error)
		goto bad;
	if (nonblock && (so->so_state & SS_ISCONNECTING)) {
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		FP_UNREF(fp);
		m_freem(nam);
		return (EINPROGRESS);
	}
	while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
		if (error = sosleep(so, (caddr_t)&so->so_timeo,
		    (PZERO+1) | PCATCH, 0))
			goto bad;
	if (error == 0) {
		error = so->so_error;
		so->so_error = 0;
	}
bad:
	so->so_state &= ~SS_ISCONNECTING;
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	FP_UNREF(fp);
	m_freem(nam);
	if (error == ERESTART)
		error = EINTR;
	return (error);
}

/* ARGSUSED */
socketpair(p, args, retval)
	struct proc *p;
	void *args;	
	long retval[];
{
	register struct args {
		long	domain;		/* real type: 'int' */
		long	type;		/* real type: 'int' */
		long	protocol;	/* real type: 'int' */
		long	*rsv;		/* real type: 'int rsv[2]' */
	} *uap = (struct args *) args;
	struct file *fp1, *fp2;
	struct socket *so1, *so2;
	int fd, error, sv[2];

	if (error = socreate((int)uap->domain, &so1, (int)uap->type, (int)uap->protocol))
		return (error);
	if (error = socreate((int)uap->domain, &so2, (int)uap->type, (int)uap->protocol))
		goto free1;
	if (error = falloc(&fp1, &fd))
		goto free2;
	sv[0] = fd;
	FP_LOCK(fp1);
	fp1->f_flag = FREAD|FWRITE;
	fp1->f_type = DTYPE_SOCKET;
	fp1->f_ops = &socketops;
	fp1->f_data = (caddr_t)so1;
#if	UNI_COMPAT
	fp1->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(fp1);
	if (error = falloc(&fp2, &fd))
		goto free3;
	FP_LOCK(fp2);
	fp2->f_flag = FREAD|FWRITE;
	fp2->f_type = DTYPE_SOCKET;
	fp2->f_ops = &socketops;
	fp2->f_data = (caddr_t)so2;
#if	UNI_COMPAT
	fp2->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(fp2);
	sv[1] = fd;
	if (error = soconnect2(so1, so2))
		goto free4;
	if ((int)uap->type == SOCK_DGRAM) {
		/*
		 * Datagram socket connection is asymmetric.
		 */
		 if (error = soconnect2(so2, so1))
			goto free4;
	}
	/* Copy out the two file descriptors */
	error = copyout((caddr_t)sv, (caddr_t)uap->rsv, 2 * sizeof (int));

	/* retval should only be set if the C library wrapper for the
	 * system call takes V0 and A4 returned by the kernel and stuffs
	 * them into the array passed in by the calling routine.  Libc doesn't
	 * currently do this, which causes the system call to return
	 * sv[0] rather than 0 or -1.  In the long run I think the code
	 * here and the libc code should be changed so that this call
	 * and pipe(2) are handled in the same manner.
	 *
	retval[0] = sv[0];
	U_FD_SET(sv[0], fp1, &u.u_file_state);
	retval[1] = sv[1];
	 */
	U_FD_SET(sv[0], fp1, &u.u_file_state);
	U_FD_SET(sv[1], fp2, &u.u_file_state);
	return (error);
free4:
	U_FD_SET(sv[1], NULL, &u.u_file_state);
	fdealloc(fp2);
free3:
	U_FD_SET(sv[0], NULL, &u.u_file_state);
	fdealloc(fp1);
free2:
	(void)soclose(so2);
free1:
	(void)soclose(so1);
	return (error);
}

/* ARGSUSED */
sendto(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	buf;		/* real type: 'char *' */
		long	len;		/* real type: 'int' */
		long	flags;		/* real type: 'int' */
		caddr_t	to;		/* real type: 'struct sockaddr *' */
		long	tolen;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = uap->to;
	msg.msg_namelen = (int)uap->tolen;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	msg.msg_control = 0;
#ifdef COMPAT_43
	msg.msg_flags = 0;
#endif
	aiov.iov_base = uap->buf;
	aiov.iov_len = (int)uap->len;
	return (sendit((int)uap->s, &msg, (int)uap->flags, retval));
}

#ifdef COMPAT_43
/* ARGSUSED */
osend(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	buf;		/* real type: 'char *' */
		long	len;		/* real type: 'int' */
		long	flags;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = (int)uap->len;
	msg.msg_control = 0;
	msg.msg_flags = 0;
	return (sendit((int)uap->s, &msg, (int)uap->flags, retval));
}

/* ARGSUSED */
osendmsg(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	msg;		/* real type: 'struct msghdr *' */
		long	flags;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];	/* XXX use malloc */
	int error;

	if (error = copyin(uap->msg, (caddr_t)&msg, sizeof(struct omsghdr)))
		return (error);
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0]))
		return (EMSGSIZE);
	if (error = copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
	    (unsigned)(msg.msg_iovlen * sizeof (struct iovec))))
		return (error);
	msg.msg_flags = MSG_COMPAT;
	msg.msg_iov = aiov;
	return (sendit((int)uap->s, &msg, (int)uap->flags, retval));
}
#endif

/* ARGSUSED */
sendmsg(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	msg;		/* real type: 'struct msghdr *' */
		long	flags;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];	/* XXX use malloc */
	int error;

	if (error = copyin(uap->msg, (caddr_t)&msg, sizeof (msg)))
		return (error);
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0]))
		return (EMSGSIZE);
	if (msg.msg_iovlen &&
	    (error = copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
	    (unsigned)(msg.msg_iovlen * sizeof (struct iovec)))))
		return (error);
	msg.msg_iov = aiov;
#ifdef COMPAT_43
	msg.msg_flags = 0;
#endif
	return (sendit((int)uap->s, &msg, (int)uap->flags, retval));
}

sendit(s, mp, flags, retsize)
	int s;
	register struct msghdr *mp;
	int flags;
	long *retsize;
{
	register struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	struct mbuf *to, *control;
	int len, error;
	
	fp = getsock(s, &error);
	if (fp == 0)
		return (error);
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_WRITE;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	to = control = 0;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0 || (auio.uio_resid += iov->iov_len) < 0) {
			error = EINVAL;
			goto bad;
		}
	}
	if (mp->msg_name) {
		if (error = sockargs(&to, mp->msg_name, (int)mp->msg_namelen,
		    MT_SONAME))
			goto bad;
	}
	if (mp->msg_control) {
		if (mp->msg_controllen < sizeof(struct cmsghdr)
#ifdef COMPAT_43
		    && mp->msg_flags != MSG_COMPAT
#endif
		) {
			error = EINVAL;
			goto bad;
		}
		if (error = sockargs(&control, mp->msg_control,
		    (int)mp->msg_controllen, MT_CONTROL))
			goto bad;
#ifdef COMPAT_43
		if (mp->msg_flags == MSG_COMPAT) {
			register struct cmsghdr *cm;

			M_PREPEND(control, sizeof(*cm), M_WAIT);
			if (control == 0) {
				error = ENOBUFS;
				goto bad;
			} else {
				cm = mtod(control, struct cmsghdr *);
				cm->cmsg_len = control->m_len;
				cm->cmsg_level = SOL_SOCKET;
				cm->cmsg_type = SCM_RIGHTS;
			}
		}
#endif
	}
#if	SEC_ARCH
	if (error = sec_internalize_rights(&control))
		goto bad;
#endif
	BM(FP_LOCK(fp));
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		flags |= MSG_NONBLOCK;
	BM(FP_UNLOCK(fp));
	len = auio.uio_resid;
	if (error = sosend((struct socket *)fp->f_data, to, &auio,
	    (struct mbuf *)0, control, flags)) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at sosend() */
			if (auio.uio_resid != len)
				error = 0;
			break;
		case EPIPE:
                        thread_doexception(current_thread(), EXC_SOFTWARE,
                        EXC_UNIX_BAD_PIPE, 0);
			break;
		}
	}
	if (error == 0)
		*retsize = len - auio.uio_resid;
bad:
	FP_UNREF(fp);
	if (to)
		m_freem(to);
	return (error);
}

#ifdef COMPAT_43
orecvfrom(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	buf;		/* real type: 'char *' */
		long	len;		/* real type: 'int' */
		long	flags;		/* real type: 'int' */
		caddr_t	from;		/* real type: 'struct sockaddr *' */
		int	*fromlenaddr;	/* real type: 'int *' */
	} *uap = (struct args *) args;

	uap->flags |= MSG_COMPAT;
	return (recvfrom(p, args, retval));
}
#endif

/* ARGSUSED */
recvfrom(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	buf;		/* real type: 'char *' */
		long	len;		/* real type: 'int' */
		long	flags;		/* real type: 'int' */
		caddr_t	from;		/* real type: 'struct sockaddr *' */
		int	*fromlenaddr;	/* real type: 'int *' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov;
	int error;

	if (uap->fromlenaddr) {
		if (error = copyin((caddr_t)uap->fromlenaddr,
		    (caddr_t)&msg.msg_namelen, sizeof (msg.msg_namelen)))
			return (error);
	} else
		msg.msg_namelen = 0;
	msg.msg_name = uap->from;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = (int)uap->len;
	msg.msg_control = 0;
	msg.msg_flags = (int)uap->flags;
	return (recvit((int)uap->s, &msg, (caddr_t)uap->fromlenaddr, retval));
}

#ifdef COMPAT_43
/* ARGSUSED */
orecv(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		caddr_t	buf;		/* real type: 'char *' */
		long	len;		/* real type: 'int' */
		long	flags;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = (int)uap->len;
	msg.msg_control = 0;
	msg.msg_flags = (int)uap->flags;
	return (recvit((int)uap->s, &msg, (caddr_t)0, retval));
}

/*
 * Old recvmsg.  This code takes advantage of the fact that the old msghdr
 * overlays the new one, missing only the flags, and with the (old) access
 * rights where the control fields are now.
 */
/* ARGSUSED */
orecvmsg(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		struct	omsghdr *msg;	/* real type: 'struct omsghdr *' */
		long	flags;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN];	/* XXX use malloc */
	int error;

	if (error = copyin((caddr_t)uap->msg, (caddr_t)&msg,
	    sizeof (struct omsghdr)))
		return (error);
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0]))
		return (EMSGSIZE);
	msg.msg_flags = ((int)uap->flags) | MSG_COMPAT;
	if (error = copyin((caddr_t)msg.msg_iov, (caddr_t)aiov,
	    (unsigned)(msg.msg_iovlen * sizeof (struct iovec))))
		return (error);
	msg.msg_iov = aiov;
	error = recvit((int)uap->s, &msg, (caddr_t)&uap->msg->msg_namelen, retval);

	if (msg.msg_controllen && error == 0)
		error = copyout((caddr_t)&msg.msg_controllen,
		    (caddr_t)&uap->msg->msg_accrightslen, sizeof (int));
	return (error);
}
#endif

/* ARGSUSED */
recvmsg(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		struct	msghdr *msg;	/* real type: 'struct msghdr msg[]' */
		long	flags;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct msghdr msg;
	struct iovec aiov[MSG_MAXIOVLEN], *uiov;	/* XXX use malloc */
	register int error;

	if (error = copyin((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg)))
		return (error);
	if ((u_int)msg.msg_iovlen >= sizeof (aiov) / sizeof (aiov[0]))
		return (EMSGSIZE);
#ifdef COMPAT_43
	msg.msg_flags = ((int)uap->flags) &~ MSG_COMPAT;
#else
	msg.msg_flags = (int)uap->flags;
#endif
	uiov = msg.msg_iov;
	msg.msg_iov = aiov;
	if (error = copyin((caddr_t)uiov, (caddr_t)aiov,
	    (unsigned)(msg.msg_iovlen * sizeof (struct iovec))))
		return (error);
	if ((error = recvit((int)uap->s, &msg, (caddr_t)0, retval)) == 0) {
		msg.msg_iov = uiov;
		error = copyout((caddr_t)&msg, (caddr_t)uap->msg, sizeof(msg));
	}
	return (error);
}

recvit(s, mp, namelenp, retsize)
	int s;
	register struct msghdr *mp;
	caddr_t namelenp;
	long *retsize;
{
	register struct file *fp;
	struct uio auio;
	register struct iovec *iov;
	register int i;
	int len, error;
	struct mbuf *from = 0, *control = 0;
	
	fp = getsock(s, &error);
	if (fp == 0)
		return (error);
	auio.uio_iov = mp->msg_iov;
	auio.uio_iovcnt = mp->msg_iovlen;
	auio.uio_segflg = UIO_USERSPACE;
	auio.uio_rw = UIO_READ;
	auio.uio_offset = 0;			/* XXX */
	auio.uio_resid = 0;
	iov = mp->msg_iov;
	for (i = 0; i < mp->msg_iovlen; i++, iov++) {
		if (iov->iov_len < 0 || (auio.uio_resid += iov->iov_len) < 0) {
			FP_UNREF(fp);
			return (EINVAL);
		}
	}
	BM(FP_LOCK(fp));
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		mp->msg_flags |= MSG_NONBLOCK;
	BM(FP_UNLOCK(fp));
	len = auio.uio_resid;
	if (error = soreceive((struct socket *)fp->f_data, &from, &auio,
	    (struct mbuf **)0, (mp->msg_control ? &control : (struct mbuf **)0),
	    &mp->msg_flags)) {
		switch (error) {
		case ERESTART: case EWOULDBLOCK: case EINTR:
			/* See comment at soreceive() */
			if (auio.uio_resid != len)
				error = 0;
			break;
		}
	}
	if (error)
		goto out;
	*retsize = len - auio.uio_resid;
	if (mp->msg_name) {
		len = mp->msg_namelen;
		if (len <= 0 || from == 0)
			len = 0;
		else {
#ifdef COMPAT_43
			if (mp->msg_flags & MSG_COMPAT)
				mtod(from, struct osockaddr *)->sa_family =
				    mtod(from, struct sockaddr *)->sa_family;
#endif
			if (len > from->m_len)
				len = from->m_len;
			/* else if len < from->m_len ??? */
			if (error = copyout(mtod(from, caddr_t),
			    (caddr_t)mp->msg_name, (unsigned)len))
				goto out;
		}
		mp->msg_namelen = len;
		if (namelenp &&
		    (error = copyout((caddr_t)&len, namelenp, sizeof (int)))) {
#ifdef COMPAT_43
			if (mp->msg_flags & MSG_COMPAT)
				error = 0;	/* old recvfrom didn't check */
			else
#endif
			goto out;
		}
	}
	if (mp->msg_control) {
#if	SEC_ARCH
		sec_externalize_rights(&control, (struct socket *) fp->f_data);
#endif
#ifdef	COMPAT_43
		/*
		 * We assume that old recvmsg calls won't receive access
		 * rights and other control info, esp. as control info
		 * is always optional and those options didn't exist in 4.3.
		 * If we receive rights, trim the cmsghdr; anything else
		 * is tossed.
		 */
		if (control && mp->msg_flags & MSG_COMPAT) {
			if (mtod(control, struct cmsghdr *)->cmsg_level !=
			    SOL_SOCKET ||
			    mtod(control, struct cmsghdr *)->cmsg_type !=
			    SCM_RIGHTS) {
				mp->msg_controllen = 0;
				goto out;
			}
			control->m_len -= sizeof (struct cmsghdr);
			control->m_data += sizeof (struct cmsghdr);
		}
#endif
		len = mp->msg_controllen;
		if (len <= 0 || control == 0)
			len = 0;
		else {
			if (len >= control->m_len)
				len = control->m_len;
			else
				mp->msg_flags |= MSG_CTRUNC;
			error = copyout((caddr_t)mtod(control, caddr_t),
			    (caddr_t)mp->msg_control, (unsigned)len);
		}
		mp->msg_controllen = len;
	}
out:
	FP_UNREF(fp);
	if (from)
		m_freem(from);
	if (control)
		m_freem(control);
	return (error);
}

/* ARGSUSED */
shutdown(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		long	how;		/* real type: 'int' */
	} *uap = (struct args *) args;
	struct file *fp;
	int error;

	fp = getsock((int)uap->s, &error);
	if (fp == 0)
		return (error);
	/*
	 * Decrementing fp's reference count can result in a call to
	 * soclose when racing another thread calling close(uap->s).
	 * Defer possibility until after calling soshutdown.
	 */
	error = soshutdown((struct socket *)fp->f_data, (int)uap->how);
	FP_UNREF(fp);
	return (error);
}

/* ARGSUSED */
setsockopt(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		long	level;		/* real type: 'int' */
		long	name;		/* real type: 'int' */
		caddr_t	val;		/* real type: 'char *' */
		long	valsize;	/* real type: 'int' */
	} *uap = (struct args *) args;
	struct file *fp;
	struct mbuf *m = NULL;
	int error;

	fp = getsock((int)uap->s, &error);
	if (fp == 0)
		return (error);
	if ((int)uap->valsize > MCLBYTES) {
		FP_UNREF(fp);
		return (EINVAL);
	}
	if (uap->val) {
		m = m_get(M_WAIT, MT_SOOPTS);
		if (m == NULL) {
			FP_UNREF(fp);
			return (ENOBUFS);
		}
		if ((int)uap->valsize > MLEN) {
			MCLGET(m,M_WAIT);
			if ((m->m_flags & M_EXT) == NULL) {
				(void) m_free(m);
				FP_UNREF(fp);
				return(ENOBUFS);
			}
		}
		if (error = copyin(uap->val, mtod(m, caddr_t),
		    (int)uap->valsize)) {
			(void) m_free(m);
			FP_UNREF(fp);
			return (error);
		}
		m->m_len = (int)uap->valsize;
	}
	error = sosetopt((struct socket *)fp->f_data, (int)uap->level, (int)uap->name, m);
	FP_UNREF(fp);
	return (error);
}

/* ARGSUSED */
getsockopt(p, args, retval)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	s;		/* real type: 'int' */
		long	level;		/* real type: 'int' */
		long	name;		/* real type: 'int' */
		caddr_t	val;		/* real type: 'char *' */
		int	*avalsize;	/* real type: 'int *' */
	} *uap = (struct args *) args;
	struct file *fp;
	struct mbuf *m = NULL;
	int valsize, error;

	fp = getsock((int)uap->s, &error);
	if (fp == 0)
		return (error);
	if (uap->val) {
		if (error = copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
		    sizeof (valsize))) {
			FP_UNREF(fp);
			return (error);
		}
	} else
		valsize = 0;
	if ((error = sogetopt((struct socket *)fp->f_data, (int)uap->level,
	    (int)uap->name, &m)) == 0 && uap->val && valsize && m != NULL) {
		if (valsize > m->m_len)
			valsize = m->m_len;
		error = copyout(mtod(m, caddr_t), uap->val, (u_int)valsize);
		if (error == 0)
			error = copyout((caddr_t)&valsize,
			    (caddr_t)uap->avalsize, sizeof (valsize));
	}
	FP_UNREF(fp);
	if (m)
		m_freem(m);
	return (error);
}

/* ARGSUSED */
pipe(p, args, retval)
	struct proc *p;
	void *args;
	long retval[];
{
	struct file *rf, *wf;
	struct socket *rso, *wso;
	int fd, error;

	if (error = socreate(AF_UNIX, &rso, SOCK_STREAM, 0))
		goto done;
	if (error = socreate(AF_UNIX, &wso, SOCK_STREAM, 0))
		goto free1;
	/* Need write atomicity and pipe times behavior */
	wso->so_special |= (SP_WATOMIC|SP_PIPE);
	rso->so_special |= SP_PIPE;
	if (error = falloc(&rf, &fd))
		goto free2;
	retval[0] = fd;
	FP_LOCK(rf);
	rf->f_flag = FREAD;
	rf->f_type = DTYPE_SOCKET;
	rf->f_ops = &socketops;
	rf->f_data = (caddr_t)rso;
#if	UNI_COMPAT
	rf->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(rf);
	if (error = falloc(&wf, &fd))
		goto free3;
	FP_LOCK(wf);
	wf->f_flag = FWRITE;
	wf->f_type = DTYPE_SOCKET;
	wf->f_ops = &socketops;
	wf->f_data = (caddr_t)wso;
#if	UNI_COMPAT
	wf->f_funnel = FUNNEL_NULL;
#endif
	FP_UNLOCK(wf);
	retval[1] = fd;
	if (error = soconnect2(wso, rso))
		goto free4;
	wso->so_state |= SS_CANTRCVMORE;
	rso->so_state |= SS_CANTSENDMORE;
	U_FD_SET(retval[0], rf, &u.u_file_state);
	U_FD_SET(retval[1], wf, &u.u_file_state);
	goto done;
free4:
	U_FD_SET(retval[1], NULL, &u.u_file_state);
	fdealloc(wf);
free3:
	U_FD_SET(retval[0], NULL, &u.u_file_state);
	fdealloc(rf);
free2:
	(void)soclose(wso);
free1:
	(void)soclose(rso);
done:
	AUDIT_CALL ( SYS_pipe, error, retval, 0, AUD_VHPR, "CC000000" );
	return (error);
}

/*
 * Get socket name.
 */
#ifdef COMPAT_43
getsockname(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (getsockname1(p, args, retval, 0));
}

ogetsockname(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (getsockname1(p, args, retval, 1));
}
#else /* COMPAT_43 */

#define getsockname1(p, args, retval, compat_43) getsockname(p, args, retval)
#endif

/* ARGSUSED */
getsockname1(p, args, retval, compat_43)
	struct proc *p;
	void *args;	
	long *retval;
{
	register struct args {
		long	fdes;		/* real type: 'int' */
		caddr_t	asa;		/* real type: 'struct sockaddr *' */
		int	*alen;		/* real type: 'int *' */
	} *uap = (struct args *) args;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len, error;
	DOMAIN_FUNNEL_DECL(f)

	fp = getsock((int)uap->fdes, &error);
	if (fp == 0)
		return (error);
	if (error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len))) {
		FP_UNREF(fp);
		return (error);
	}
	so = (struct socket *)fp->f_data;
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		FP_UNREF(fp);
		return (ENOBUFS);
	}
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	error = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR,
				(struct mbuf *)0, m, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	if (error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifdef COMPAT_43
	if (compat_43)
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
#endif
	error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (error == 0)
		error = copyout((caddr_t)&len, (caddr_t)uap->alen,
		    sizeof (len));
bad:
	FP_UNREF(fp);
	m_freem(m);
	return (error);
}

/*
 * Get name of peer for connected socket.
 */
#ifdef COMPAT_43
getpeername(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (getpeername1(p, args, retval, 0));
}

ogetpeername(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	return (getpeername1(p, args, retval, 1));
}
#else /* COMPAT_43 */

#define getpeername1(p, args, retval, compat_43) getpeername(p, args, retval)
#endif

/* ARGSUSED */
getpeername1(p, args, retval, compat_43)
	struct proc *p;
	void *args;	
	long *retval;
{
	struct args {
		long	fdes;		/* real type: 'int' */
		caddr_t	asa;		/* real type: 'struct sockaddr *' */
		int	*alen;		/* real type: 'int *' */
	} *uap = (struct args *) args;
	register struct file *fp;
	register struct socket *so;
	struct mbuf *m;
	int len, error;
	DOMAIN_FUNNEL_DECL(f)

	fp = getsock((int)uap->fdes, &error);
	if (fp == 0)
		return (error);
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL) {
		FP_UNREF(fp);
		return (ENOBUFS);
	}
	if (error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len)))
		goto bad;
	so = (struct socket *)fp->f_data;
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	if ((so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0) {
		SOCKET_UNLOCK(so);
		DOMAIN_UNFUNNEL(f);
		error = ENOTCONN;
		goto bad;
	}
	error = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR,
				(struct mbuf *)0, m, (struct mbuf *)0);
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	if (error)
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
#ifdef COMPAT_43
	if (compat_43)
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
#endif
	if (error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len))
		goto bad;
	error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
bad:
	FP_UNREF(fp);
	m_freem(m);
	return (error);
}

void
sockaddr_new(m)
	struct mbuf *m;
{
	if (m->m_type == MT_SONAME) {
		register struct sockaddr *sa = mtod(m, struct sockaddr *);
#if	defined(COMPAT_43) && BYTE_ORDER != BIG_ENDIAN
		if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
			sa->sa_family = sa->sa_len;
#endif
		sa->sa_len = m->m_len;
	}
}

void
sockaddr_old(m)
	struct mbuf *m;
{
#ifdef	COMPAT_43
	if (m->m_type == MT_SONAME) {
		mtod(m, struct osockaddr *)->sa_family =
		    mtod(m, struct sockaddr *)->sa_family;
	}
#endif
}

sockargs(mp, buf, buflen, type)
	struct mbuf **mp;
	caddr_t buf;
	int buflen, type; 
{
	register struct mbuf *m;
	int error;

#ifdef COMPAT_43
	if (type == MT_SONAME && 
	    ((u_int )buflen > MLEN) && ((u_int)buflen <= 112))
			buflen = MLEN;		/* unix domain compat. hack */
#endif
	if ((u_int)buflen > MCLBYTES) {
		return (EINVAL);
	}
	/* use cluster mbuf if input buffer bigger than MLEN */
#ifdef COMPAT_43
#ifdef __alpha
#define align8(x,t)	(t)(((unsigned long)(x)+7) & ((unsigned long)~7))
	/* 
	 * On alpha, when we internalize the fd's to fp's,
	 * size requirement will double. so we leave room in the data mbuf.
	 *
	 */
        if ( ((type != MT_CONTROL) && (buflen <= MLEN)) ||
	     ((type == MT_CONTROL) && (buflen <= ((MLEN-align8(sizeof(struct msghdr), int))/2))) ) {
#else
        if ( ((type != MT_CONTROL) && (buflen <= MLEN)) ||
	     ((type == MT_CONTROL) && (buflen <= (MLEN-sizeof(struct msghdr)))) ){
#endif
#else	/* if COMPAT_43 */
#ifdef __alpha
	/* On alpha, when we internalize the fd's to fp's,
	 * size requirement will double. so we leave room in the data mbuf.
	 */
        if ( ((buflen <= MLEN) && (type != MT_CONTROL)) ||
	     ((type == MT_CONTROL) && (buflen <= (MLEN/2))) {
#else
        if (buflen <= MLEN) {
#endif
#endif	/* if COMPAT_43 */
                m = m_getclr(M_WAIT, type);
                if (m == NULL)
                        return (ENOBUFS);
        } else {
                m = m_get(M_WAIT, type);
                if (m == NULL)
                        return (ENOBUFS);
                MCLGET(m,M_WAIT);
                if ((m->m_flags & M_EXT) == NULL) {
                        (void) m_free(m);
                        return(ENOBUFS);
                }
                bzero (mtod(m,caddr_t), MCLBYTES);
        }
	m->m_len = buflen;
/*
 * Make enough room to avoid MT_CONTROL chain, so that unp_internalize can 
 * handle 4.3 compatibility.
 */
#ifdef COMPAT_43
	if (type == MT_CONTROL)
		m->m_data += sizeof (struct cmsghdr);
#endif	
	if (error = copyin(buf, mtod(m, caddr_t), (u_int)buflen))
		(void) m_free(m);
	else
		sockaddr_new(*mp = m);
	return (error);
}

/*
 * Get socket from file struct.
 * Return an unlocked, referenced file structure.  The calling
 * routine must delete the reference when it is done with the
 * file structure. Note doesn't use GETF because of return value.
 */
struct file *
getsock(fdes, errp)
	int fdes;
	int *errp;
{
	register struct file *fp;

	U_FDTABLE_LOCK(&u.u_file_state);
	if (fdes < 0 || fdes > u.u_file_state.uf_lastfile) {
                U_FDTABLE_UNLOCK(&u.u_file_state);
		*errp = EBADF;
		return (0);
	}
#if	MACH
	fp = U_OFILE(fdes, &u.u_file_state);
	if (fp == U_FD_RESERVED)
		fp = NULL;
#else
	fp = u.u_ofile[fdes];
#endif
	if (fp == NULL) {
		*errp = EBADF;
	} else if (fp->f_type != DTYPE_SOCKET) {
		fp = NULL;
		*errp = ENOTSOCK;
	} else
		FP_REF(fp);
	U_FDTABLE_UNLOCK(&u.u_file_state);
	return (fp);
}
