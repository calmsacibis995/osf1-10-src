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
static char *rcsid = "@(#)$RCSfile: uipc_usrreq.c,v $ $Revision: 4.3.12.5 $ (DEC) $Date: 1993/12/17 22:00:34 $";
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
 * Copyright (c) 1982, 1986, 1989 Regents of the University of California.
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
 *	Base:	uipc_usrreq.c	7.13 (Berkeley) 10/19/89
 * 	Merged: uipc_usrreq.c	7.20 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"
#include <sys/secdefines.h>

#include "sys/param.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/mount.h"
#include "sys/file.h"
#include "sys/stat.h"
#include "sys/time.h"

#include "sys/mbuf.h"	/* XXX must appear after mount.h */
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "sys/unpcb.h"
#include "sys/un.h"

#include "net/net_malloc.h"

#if 	SEC_BASE
#include <sys/security.h>
#include <sys/audit.h>
#endif
#if	MACH
#if	SEC_ARCH
#include <sys/secpolicy.h>
#include <sys/ioctl.h>

extern caddr_t findrights();
#endif
#endif

LOCK_ASSERTL_DECL

/*
 * Unix communications domain.
 *
 * TODO:
 *	SEQPACKET, RDM
 *	rethink name space problems
 *	need a proper out-of-band
 */
CONST struct	sockaddr sun_noname = { sizeof(sun_noname), AF_UNIX };
ino_t	unp_vno;			/* prototype for fake vnode numbers */
#if	NETSYNC_LOCK
simple_lock_data_t	global_unpconn_lock;
simple_lock_data_t	unp_misc_lock;
#endif


void
uipc_init()
{
	UNPCONN_LOCKINIT();
	UNPMISC_LOCKINIT();
}

/*ARGSUSED*/
uipc_usrreq(so, req, m, nam, control)
	struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	struct unpcb *unp = sotounpcb(so);
	register struct socket *so2;
	register int error = 0;

	if (req == PRU_CONTROL) {
#if	SEC_ARCH
		/* XXX m == scalar, nam == int * */
		if ((int) m == SIOCGPEERPRIV) {
			if (unp->unp_conn == (struct unpcb *) 0)
				return ENOTCONN;
			so2 = unp->unp_conn->unp_socket;
			/* Don't lock so2 for this */
			*(int *) nam = (so2->so_state & SS_PRIV) != 0;
			return 0;
		}
#endif
		return (EOPNOTSUPP);
	}
	if (req != PRU_SEND && control && control->m_len) {
		error = EOPNOTSUPP;
		goto release;
	}
	if (unp == 0 && req != PRU_ATTACH) {
		error = EINVAL;
		goto release;
	}

	LOCK_ASSERT("uipc_usrreq", SOCKET_ISLOCKED(so));

	switch (req) {

	case PRU_ATTACH:
		if (unp) {
			error = EISCONN;
			break;
		}
		error = unp_attach(so);
		break;

	case PRU_DETACH:
		unp_detach(unp);
		break;

	case PRU_BIND:
		SOCKET_UNLOCK(so);		/* namei may be interrupted */
		error = unp_bind(unp, nam);
		SOCKET_LOCK(so);
		break;

	case PRU_LISTEN:
		if (unp->unp_vnode == 0)
			error = EINVAL;
		break;

	case PRU_CONNECT:
		error = unp_connect(so, nam);
		break;

	case PRU_CONNECT2:
		error = unp_connect2(so, (struct socket *)nam);
		break;

	case PRU_DISCONNECT:
		unp_disconnect(unp);
		break;

	case PRU_ACCEPT:
		/*
		 * Pass back name of connected socket,
		 * if it was bound and we are still connected
		 * (our peer may have closed already!).
		 */
		if (unp->unp_conn && unp->unp_conn->unp_addr) {
			nam->m_len = unp->unp_conn->unp_addr->m_len;
			bcopy(mtod(unp->unp_conn->unp_addr, caddr_t),
			    mtod(nam, caddr_t), (unsigned)nam->m_len);
		} else {
			nam->m_len = sizeof(sun_noname);
			*(mtod(nam, struct sockaddr *)) = sun_noname;
		}
		break;

	case PRU_SHUTDOWN:
		socantsendmore(so);
		unp_usrclosed(unp);
		break;

	case PRU_RCVD:
		switch (so->so_type) {

		case SOCK_DGRAM:
			panic("uipc 1");
			/*NOTREACHED*/

		case SOCK_STREAM:
#define	rcv (&so->so_rcv)
#define snd (&so2->so_snd)
			if (unp->unp_conn == 0)
				break;
			so2 = unp->unp_conn->unp_socket;
			LOCK_ASSERT("uipc_usrreq PRU_RCVD STREAM so2notso", (so->so_lock == so2->so_lock));
			SOCKBUF_LOCK(rcv);
			SOCKBUF_LOCK(snd);
			/*
			 * Adjust backpressure on sender
			 * and wakeup any waiting to write.
			 */
			snd->sb_mbmax += unp->unp_mbcnt - rcv->sb_mbcnt;
			unp->unp_mbcnt = rcv->sb_mbcnt;
			snd->sb_hiwat += unp->unp_cc - rcv->sb_cc;
			unp->unp_cc = rcv->sb_cc;
			if (so->so_special & SP_PIPE) {	/* Posix/AES */
				struct timeval now;
				microtime(&now);
				unp->unp_atime = now.tv_sec;
			}
			SOCKBUF_UNLOCK(snd);
			sowwakeup(so2);
			SOCKBUF_UNLOCK(rcv);
#undef snd
#undef rcv
			break;

		default:
			panic("uipc 2");
		}
		break;

	case PRU_SEND:
		if (control && (error = unp_internalize(control)))
			break;
		switch (so->so_type) {

		case SOCK_DGRAM: {
			CONST struct sockaddr *from;

			if (nam) {
				if (unp->unp_conn) {
					error = EISCONN;
					break;
				}
				error = unp_connect(so, nam);
				LOCK_ASSERT("uipc_usrreq PRU_SEND DGRAM so", SOCKET_ISLOCKED(so));
				if (error)
					break;
			} else {
				if (unp->unp_conn == 0) {
					error = ENOTCONN;
					break;
				}
			}
			so2 = unp->unp_conn->unp_socket;
			LOCK_ASSERT("uipc_usrreq PRU_SEND DGRAM so2notso", (so2->so_lock == so->so_lock));
			SOCKBUF_LOCK(&so->so_snd);
			SOCKBUF_LOCK(&so2->so_rcv);
			if (unp->unp_addr)
				from = mtod(unp->unp_addr, CONST struct sockaddr *);
			else
				from = &sun_noname;
			if (sbappendaddr(&so2->so_rcv, from, m, control)) {
				SOCKBUF_UNLOCK(&so2->so_rcv);
				sorwakeup(so2);
				m = 0;
				control = 0;
			} else {
				SOCKBUF_UNLOCK(&so2->so_rcv);
				error = ENOBUFS;
			}
			SOCKBUF_UNLOCK(&so->so_snd);
			if (nam)
				unp_disconnect(unp);
			break;
		}

		case SOCK_STREAM:
#define	rcv (&so2->so_rcv)
#define	snd (&so->so_snd)
			if (so->so_state & SS_CANTSENDMORE) {
				error = EPIPE;
				break;
			}
			if (unp->unp_conn == 0)
				panic("uipc 3");
			so2 = unp->unp_conn->unp_socket;
			LOCK_ASSERT("uipc_usrreq PRU_SEND STREAM so2notso", (so2->so_lock == so->so_lock));
			SOCKBUF_LOCK(snd);
			SOCKBUF_LOCK(rcv);
			/*
			 * Send to paired receive port, and then reduce
			 * send buffer hiwater marks to maintain backpressure.
			 * Wake up readers.
			 */
			if (control) {
				(void)sbappendcontrol(rcv, m, control);
				control = 0;
			} else
				sbappend(rcv, m);
			snd->sb_mbmax -=
			    rcv->sb_mbcnt - unp->unp_conn->unp_mbcnt;
			unp->unp_conn->unp_mbcnt = rcv->sb_mbcnt;
			snd->sb_hiwat -= rcv->sb_cc - unp->unp_conn->unp_cc;
			unp->unp_conn->unp_cc = rcv->sb_cc;
			if (so->so_special & SP_PIPE) {	/* Posix/AES */
				struct timeval now;
				microtime(&now);
				unp->unp_ctime = unp->unp_mtime = now.tv_sec;
			}
			SOCKBUF_UNLOCK(rcv);
			sorwakeup(so2);
			SOCKBUF_UNLOCK(snd);
			m = 0;
#undef snd
#undef rcv
			break;

		default:
			panic("uipc 4");
		}
		break;

	case PRU_ABORT:
		unp_drop(unp, ECONNABORTED);
		break;

	case PRU_SENSE:
		if (so->so_type == SOCK_STREAM && unp->unp_conn != 0) {
			so2 = unp->unp_conn->unp_socket;
			/* Pipe stat's must behave per Posix/AES */
			if (so->so_special & SP_PIPE) {
				struct socket *rso, *wso;
				struct unpcb *runp, *wunp;
				/* Make socket pair appear as one entity */
				if (so->so_special & SP_WATOMIC) { /* write */
					wso = so; rso = so2;
					wunp = unp; runp = unp->unp_conn;
				} else {			   /* read */
					wso = so2; rso = so;
					wunp = unp->unp_conn; runp = unp;
				}
				((struct stat *) m)->st_atime = runp->unp_atime;
				((struct stat *) m)->st_mtime = wunp->unp_mtime;
				((struct stat *) m)->st_ctime = wunp->unp_ctime;
				((struct stat *) m)->st_blksize =
					wso->so_snd.sb_hiwat+rso->so_rcv.sb_cc;
				((struct stat *) m)->st_size =
					rso->so_rcv.sb_cc;
				if (unp->unp_vno == 0)
					unp->unp_vno = unp->unp_conn->unp_vno;
			/* Else traditional socket behavior */
			} else {
				((struct stat *) m)->st_blksize =
					so->so_snd.sb_hiwat+so2->so_rcv.sb_cc;
				((struct stat *) m)->st_size =
					so2->so_rcv.sb_cc;
			}
		} else
			((struct stat *) m)->st_blksize = so->so_snd.sb_hiwat;
		((struct stat *) m)->st_dev = NODEV;
		if (unp->unp_vno == 0) {
			UNPMISC_LOCK();
			unp->unp_vno = unp_vno++;
			UNPMISC_UNLOCK();
		}
		((struct stat *) m)->st_ino = unp->unp_vno;
		return (0);

	case PRU_RCVOOB:
		return (EOPNOTSUPP);

	case PRU_SENDOOB:
		error = EOPNOTSUPP;
		break;

	case PRU_SOCKADDR:
		if (unp->unp_addr) {
			nam->m_len = unp->unp_addr->m_len;
			bcopy(mtod(unp->unp_addr, caddr_t),
			    mtod(nam, caddr_t), (unsigned)nam->m_len);
		} else
			nam->m_len = 0;
		break;

	case PRU_PEERADDR:
		if (unp->unp_conn && unp->unp_conn->unp_addr) {
			nam->m_len = unp->unp_conn->unp_addr->m_len;
			bcopy(mtod(unp->unp_conn->unp_addr, caddr_t),
			    mtod(nam, caddr_t), (unsigned)nam->m_len);
		} else if (unp->unp_conn) {
			nam->m_len = sizeof(sun_noname);
			*(mtod(nam, struct sockaddr *)) = sun_noname;
		} else
			nam->m_len = 0;
		break;

	case PRU_SLOWTIMO:
		break;

	default:
		panic("piusrreq");
	}
release:
	if (control)
		m_freem(control);
	if (m)
		m_freem(m);
	return (error);
}

/*
 * Both send and receive buffers are allocated PIPSIZ bytes of buffering
 * for stream sockets, although the total for sender and receiver is
 * actually only PIPSIZ.
 * Datagram sockets really use the sendspace as the maximum datagram size,
 * and don't really want to reserve the sendspace.  Their recvspace should
 * be large enough for at least one max-size datagram plus address.
 */
u_long	unpst_sendspace = PIPSIZ;
u_long	unpst_recvspace = PIPSIZ;
u_long	unpdg_sendspace = 2*1024;	/* really max datagram size */
u_long	unpdg_recvspace = 4*1024;

int	unp_rights;			/* file descriptors in flight */

unp_attach(so)
	struct socket *so;
{
	register struct unpcb *unp;
	int error;
	
	if (so->so_snd.sb_hiwat == 0 || so->so_rcv.sb_hiwat == 0) {
		switch (so->so_type) {

		case SOCK_STREAM:
			error = soreserve(so, unpst_sendspace, unpst_recvspace);
			break;

		case SOCK_DGRAM:
			error = soreserve(so, unpdg_sendspace, unpdg_recvspace);
			break;
		}
		if (error)
			return (error);
	}
	NET_MALLOC(unp, struct unpcb *, sizeof *unp, M_PCB, M_NOWAIT);
	if (unp == NULL)
		return (ENOBUFS);
	bzero((caddr_t)unp, sizeof *unp);
	so->so_pcb = (caddr_t)unp;
	unp->unp_socket = so;
	return (0);
}

void
unp_detach(unp)
	register struct unpcb *unp;
{
	
	if (unp->unp_vnode) {
		VN_LOCK(unp->unp_vnode);
		unp->unp_vnode->v_socket = 0;
		VN_UNLOCK(unp->unp_vnode);
		vrele(unp->unp_vnode);
		unp->unp_vnode = 0;
	}
	if (unp->unp_conn)
		unp_disconnect(unp);
	while (unp->unp_refs)
		unp_drop(unp->unp_refs, ECONNRESET);
	soisdisconnected(unp->unp_socket);
	unp->unp_socket->so_pcb = 0;
	m_freem(unp->unp_addr);
	NET_FREE(unp, M_PCB);
	UNPMISC_LOCK();
	if (unp_rights)
		unp_gc();
	UNPMISC_UNLOCK();
}

unp_bind(unp, nam)
	struct unpcb *unp;
	struct mbuf *nam;
{
	struct sockaddr_un *soun = mtod(nam, struct sockaddr_un *);
	register struct vnode *vp;
	register struct nameidata *ndp = &u.u_nd;
	struct vattr vattr;
	int error;

	ndp->ni_dirp = soun->sun_path;
	if (unp->unp_vnode != NULL)
		return (EINVAL);
	if (soun->sun_family != AF_UNIX)
		return (EAFNOSUPPORT);
	if (nam->m_len == MLEN) {
		if (*(mtod(nam, caddr_t) + nam->m_len - 1) != 0)
			return (EINVAL);
	} else
		*(mtod(nam, caddr_t) + nam->m_len) = 0;
/* SHOULD BE ABLE TO ADOPT EXISTING AND wakeup() ALA FIFO's */
#if	MACH
	ndp->ni_nameiop = CREATE | FOLLOW | WANTPARENT;
#else
	ndp->ni_nameiop = CREATE | FOLLOW | LOCKPARENT;
#endif
	ndp->ni_segflg = UIO_SYSSPACE;
	if (error = namei(ndp))
		return (error);
	vp = ndp->ni_vp;
	if (vp != NULL) {
#if	MACH
		VOP_ABORTOP(ndp, error);
		vrele(ndp->ni_dvp);
#else
		VOP_ABORTOP(ndp);
		if (ndp->ni_dvp == vp)
			vrele(ndp->ni_dvp);
		else
			vput(ndp->ni_dvp);
#endif
		vrele(vp);
		return (EADDRINUSE);
	}
#if	MACH
	vattr_null(&vattr);
	vattr.va_type = VSOCK;
	vattr.va_mode = 0777;
	/*
	 * The creation of the socket must be "atomic," so the create
	 * operation needs additional information for the VSOCK case.
	 */
	vattr.va_socket = (char *) unp->unp_socket;
	VOP_CREATE(ndp, &vattr, error);
	if (error)
		return (error);
	vp = ndp->ni_vp;
	if (vp->v_socket != unp->unp_socket)	/* filesystem sanity check */
		panic("unp_bind");
	unp->unp_vnode = vp;
	unp->unp_addr = m_copym(nam, 0, (int)M_COPYALL, M_DONTWAIT);
#else
	VATTR_NULL(&vattr);
	vattr.va_type = VSOCK;
	vattr.va_mode = 0777;
	if (error = VOP_CREATE(ndp, &vattr))
		return (error);
	vp = ndp->ni_vp;
	vp->v_socket = unp->unp_socket;
	unp->unp_vnode = vp;
	unp->unp_addr = m_copym(nam, 0, (int)M_COPYALL, M_DONTWAIT);
	VOP_UNLOCK(vp);
#endif
	return (0);
}

unp_connect(so, nam)
	struct socket *so;
	struct mbuf *nam;
{
	register struct sockaddr_un *soun = mtod(nam, struct sockaddr_un *);
	register struct vnode *vp;
	register struct socket *so2, *so3;
	register struct nameidata *ndp = &u.u_nd;
	struct unpcb *unp2;
	int error;
#if	NETSYNC_LOCK
	int so2notso;
#endif

	LOCK_ASSERT("unp_connect", SOCKET_ISLOCKED(so));
	if (soun->sun_family != AF_UNIX)
		return (EAFNOSUPPORT);
	ndp->ni_dirp = soun->sun_path;
	if (nam->m_data + nam->m_len == &nam->m_dat[MLEN]) {	/* XXX */
		if (*(mtod(nam, caddr_t) + nam->m_len - 1) != 0)
			return (EMSGSIZE);
	} else
		*(mtod(nam, caddr_t) + nam->m_len) = 0;
	SOCKET_UNLOCK(so);			/* namei may be interrupted */
#if	MACH
	ndp->ni_nameiop = LOOKUP | FOLLOW;
#else
	ndp->ni_nameiop = LOOKUP | FOLLOW | LOCKLEAF;
#endif
	ndp->ni_segflg = UIO_SYSSPACE;
	if (error = namei(ndp)) {
		SOCKET_LOCK(so);
		return (error);
	}
	vp = ndp->ni_vp;
	VN_LOCK(vp);
	if (vp->v_type != VSOCK) {
		VN_UNLOCK(vp);
		error = ENOTSOCK;
		goto bad2;
	} else
		VN_UNLOCK(vp);
#if	MACH
	VOP_ACCESS(vp, VWRITE, ndp->ni_cred, error);
#else
	error = VOP_ACCESS(vp, VWRITE, ndp->ni_cred);
#endif
	if (error)
		goto bad2;
	so2 = vp->v_socket;
	if (so2 == 0) {
		error = ECONNREFUSED;
		goto bad2;
	}
	if (so->so_type != so2->so_type) {
		error = EPROTOTYPE;
		goto bad2;
	}
	SOCKET_LOCK2(so, so2);
#if	NETSYNC_LOCK
	so2notso = (so2->so_lock != so->so_lock);
#endif
	if (so->so_proto->pr_flags & PR_CONNREQUIRED) {
		if ((so2->so_options & SO_ACCEPTCONN) == 0) {
			error = ECONNREFUSED;
#if	NETSYNC_LOCK
			if (so2notso)
				SOCKET_UNLOCK(so2);
#endif
			goto bad;
		}
#if	SEC_ARCH
		bcopy(so->so_tag, so2->so_tag, SEC_NUM_TAGS * sizeof(tag_t));
#endif
		unp2 = sotounpcb(so2);	/* Get server name before unlock */
		if (unp2->unp_addr)
			nam = m_copym(unp2->unp_addr, 0, (int)M_COPYALL, M_DONTWAIT);
		else
			nam = 0;
		if ((so3 = sonewconn(so2, 0)) == 0) {
			error = ECONNREFUSED;
			if (nam) m_freem(nam);
#if	NETSYNC_LOCK
			/*
			 * sonewconn() unlocks so2. But if
			 * so and so2 shared the same lock...
			 */
			if (!so2notso)
				goto bad2;
#endif
			goto bad;
		}
#if	NETSYNC_LOCK
		/*
		 * Note that so3 is brand new and has no relation
		 * to so. But sonewconn unlocked the old so2, which
		 * may have shared a lock with so (or have been so
		 * itself!). We thus may need to relock so.
		 */
		if (!so2notso)
			SOCKET_LOCK(so);
#endif
		sotounpcb(so3)->unp_addr = nam;
		so2 = so3;
	}
	error = unp_connect2(so, so2);
#if	MACH
	vrele(vp);
#else
	vput(vp);
#endif
	return (error);

bad2:
	SOCKET_LOCK(so);
bad:
#if	MACH
	vrele(vp);
#else
	vput(vp);
#endif
	return (error);
}

unp_connect2(so, so2)
	register struct socket *so;
	register struct socket *so2;
{
	register struct unpcb *unp = sotounpcb(so);
	register struct unpcb *unp2;

	LOCK_ASSERT("unp_connect2 so", SOCKET_ISLOCKED(so));
	LOCK_ASSERT("unp_connect2 so2", SOCKET_ISLOCKED(so2));
	if (so2->so_type != so->so_type)
		return (EPROTOTYPE);
#if	NETSYNC_LOCK
	solockpair(so, so2);
#endif
	unp2 = sotounpcb(so2);
	unp->unp_conn = unp2;
	switch (so->so_type) {

	case SOCK_DGRAM:
		UNPCONN_LOCK();
		unp->unp_nextref = unp2->unp_refs;
		unp2->unp_refs = unp;
		UNPCONN_UNLOCK();
		soisconnected(so);
		break;

	case SOCK_STREAM:
		unp2->unp_conn = unp;
		soisconnected(so);
		soisconnected(so2);
		if (so->so_special & SP_PIPE) {	/* Posix/AES */
			struct timeval now;
			microtime(&now);
			unp->unp_ctime = unp->unp_atime =
				unp->unp_mtime = now.tv_sec;
			unp2->unp_ctime = unp2->unp_atime =
				unp2->unp_mtime = now.tv_sec;
		}
		break;

	default:
		panic("unp_connect2");
	}
	return (0);
}

void
unp_disconnect(unp)
	struct unpcb *unp;
{
	register struct unpcb *unp2 = unp->unp_conn;

	if (unp2 == 0)
		return;
#if	NETSYNC_LOCK
	/*
	 * unp->unp_socket is locked on entry... be sure unp2's is also.
	 */
	LOCK_ASSERT("unp_disconnect", SOCKET_ISLOCKED(unp->unp_socket));
	if (unp->unp_socket->so_lock != unp2->unp_socket->so_lock)
		SOCKET_LOCK(unp2->unp_socket);
#endif
	unp->unp_conn = 0;
	switch (unp->unp_socket->so_type) {

	case SOCK_DGRAM:
		UNPCONN_LOCK();
		if (unp2->unp_refs == unp)
			unp2->unp_refs = unp->unp_nextref;
		else {
			unp2 = unp2->unp_refs;
			for (;;) {
				if (unp2 == 0)
					panic("unp_disconnect");
				if (unp2->unp_nextref == unp)
					break;
				unp2 = unp2->unp_nextref;
			}
			unp2->unp_nextref = unp->unp_nextref;
		}
		unp->unp_nextref = 0;
		UNPCONN_UNLOCK();
		unp->unp_socket->so_state &= ~SS_ISCONNECTED;
		break;

	case SOCK_STREAM:
		soisdisconnected(unp->unp_socket);
		unp2->unp_conn = 0;
		soisdisconnected(unp2->unp_socket);
		break;
	}
#if	NETSYNC_LOCK
	if (unp->unp_socket->so_lock != unp2->unp_socket->so_lock)
		SOCKET_UNLOCK(unp2->unp_socket);
#endif
}

#ifdef notdef
void
unp_abort(unp)
	struct unpcb *unp;
{

	unp_detach(unp);
}
#endif

/*ARGSUSED*/
void
unp_usrclosed(unp)
	struct unpcb *unp;
{

}

void
unp_drop(unp, errno)
	struct unpcb *unp;
	int errno;
{
	struct socket *so = unp->unp_socket;

	LOCK_ASSERT("unp_drop", SOCKET_ISLOCKED(so));
	so->so_error = errno;
	unp_disconnect(unp);
	if (so->so_head) {
		so->so_pcb = (caddr_t) 0;
		m_freem(unp->unp_addr);
		NET_FREE(unp, M_PCB);
		sofree(so);
	}
}

#ifdef notdef
void
unp_drain()
{

}
#endif

#ifdef __alpha
/* bump to the next 8 byte boundary */
#define align8(x,t)	(t)(((unsigned long)(x)+7) & ((unsigned long)~7))
#else
#define align8(x,t)	(t) x
#endif
unp_externalize(rights)
	struct mbuf *rights;
{
	register int i;
	register struct cmsghdr *cm = mtod(rights, struct cmsghdr *);
	register struct file **rp = align8((cm+1),struct file **);
	register int	*ip = (int *)(cm+1);
	register struct file *fp;
	int newfds = (cm->cmsg_len - align8(sizeof(*cm),int)) / sizeof (struct file *);
	int f;

#if	SEC_ARCH
	if (security_is_on) {
	rp = (struct file **) findrights((caddr_t) rp,
			cm->cmsg_len - sizeof(*cm), SEC_RIGHTS_FDS, &newfds);
	if (rp)
		newfds /= sizeof(int);
	else
		newfds = 0;
	}
#endif	/* SEC_ARCH */
#if	MACH
	if (newfds > ufavail(&u.u_file_state)) {
#else
	if (newfds > ufavail()) {
#endif
		for (i = 0; i < newfds; i++) {
			fp = *rp;
			unp_discard(fp);
			*rp++ = 0;
		}
		return (EMSGSIZE);
	}
	for (i = 0; i < newfds; i++) {
#if	MACH
		if (ufalloc(0, &f, &u.u_file_state))
#else
		if (ufalloc(0, &f))
#endif
			panic("unp_externalize");
		fp = *rp++;
		U_FD_SET(f, fp, &u.u_file_state);
		FP_LOCK(fp);
		fp->f_msgcount--;
		FP_UNLOCK(fp);
		UNPMISC_LOCK();
		unp_rights--;
		UNPMISC_UNLOCK();
		*ip++ = f;
	}
#ifdef __alpha

	/* Adjust the sizes etc */
	cm->cmsg_len = sizeof(*cm) + (newfds * sizeof(int));
	rights->m_len = cm->cmsg_len;
#endif	/* ifdef __alpha */
	return (0);
}

unp_internalize(control)
	struct mbuf *control;
{
	register struct cmsghdr *cm = mtod(control, struct cmsghdr *);
	register struct file *fp;
	register int i, fd, error = 0;
	int oldfds;
	register struct file **fpp;
	register int *ip;

	if (cm->cmsg_type != SCM_RIGHTS || cm->cmsg_level != SOL_SOCKET ||
	    cm->cmsg_len != control->m_len)
		return (EINVAL);
#if	SEC_ARCH
	if (security_is_on) {
	ip = (int *) findrights((caddr_t)(cm + 1),
			cm->cmsg_len - sizeof(*cm), SEC_RIGHTS_FDS, &oldfds);
	if (ip)
		oldfds /= sizeof(int);
	else
		oldfds = 0;
	U_FDTABLE_LOCK(&u.u_file_state);
	for (i = 0; i < oldfds; i++) {
		fd = *(ip + i);
		if (fd < 0 || fd > u.u_file_state.uf_lastfile ||
		    ((fp) = U_OFILE(fd, &u.u_file_state)) == NULL ||
		    fp == U_FD_RESERVED) {
			error = EBADF;
			goto out;
		}
	}
	} else
#endif	/* !SEC_ARCH */
	{
	oldfds = (cm->cmsg_len - sizeof (*cm)) / sizeof (int);
#ifdef __alpha
	/* 
	 * Assert that we can fit the fp's within the extended MBUF
	 * For non-extended data mbuf, sockargs has made sure of this
	 */
	if ((control->m_flags & M_EXT) && 
	    ((oldfds*sizeof(struct file *)+align8(sizeof(*cm),int)) > control->m_ext.ext_size)) {
		error = E2BIG;	/* sorry, can't squeeze it */
		goto out;
	}
#endif
	ip = (int *)(cm + 1);
	U_FDTABLE_LOCK(&u.u_file_state);
	for (i = 0; i < oldfds; i++) {
		fd = *ip++;
		if (fd < 0 || fd > u.u_file_state.uf_lastfile ||
#if	MACH
		    ((fp) = U_OFILE(fd, &u.u_file_state)) == NULL ||
		    fp == U_FD_RESERVED) {
#else
		    u.u_ofile[fd] == NULL) {
#endif
			error = EBADF;
			goto out;
		}
	}
	/* 
	 * Start from the end to avoid overwriting when pointer size > int size
	 */
	ip  = (int *)(cm + 1) + oldfds - 1;
	fpp =  align8((cm+1), struct file **) + oldfds - 1;
	}
	for (i = 0; i < oldfds; i++) {
#if	MACH
		fp = U_OFILE(*ip, &u.u_file_state);
#else
		fp = u.u_ofile[*ip];
#endif
		*fpp-- = fp;
		ip--;
		FP_LOCK(fp);
		fp->f_count++;
		fp->f_msgcount++;
		FP_UNLOCK(fp);
		UNPMISC_LOCK();
		unp_rights++;
		UNPMISC_UNLOCK();
	}
#ifdef __alpha

	/* Adjust the sizes etc */
	cm->cmsg_len = align8(sizeof(*cm),int) + oldfds * sizeof(struct file *);
	control->m_len = cm->cmsg_len;
#endif	/* ifdef __alpha */
out:
	U_FDTABLE_UNLOCK(&u.u_file_state);
	return (error);
}

#if	NETSYNC_LOCK
/*
 * Unix connection rights garbage collection. Enough races in here
 * to choke Secretariat! RIP. However, the globals here are protected
 * by UNPMISC_LOCK(), I think.
 *
 * UNTESTED BECAUSE NOBODY PASSES RIGHTS AROUND AND/OR IF THEY DO
 * THE RIGHTS DON'T GET LEFT TO OTHERS TO GET GC'D. Yecch.
 */
#endif
int	unp_defer, unp_gcing;
extern	struct domain unixdomain;

void
unp_gc()
{
	register struct file *fp;
	register struct socket *so;

	if (unp_gcing)
		return;
	unp_gcing = 1;
	UNPMISC_UNLOCK();		/* until done */
restart:
	unp_defer = 0;
	for (fp = file; fp < fileNFILE; fp++)
		fp->f_flag &= ~(FMARK|FDEFER);
	do {
		for (fp = file; fp < fileNFILE; fp++) {
			if (fp->f_count == 0)
				continue;
			if (fp->f_flag & FDEFER) {
				fp->f_flag &= ~FDEFER;
				unp_defer--;
			} else {
				if (fp->f_flag & FMARK)
					continue;
				if (fp->f_count == fp->f_msgcount)
					continue;
				fp->f_flag |= FMARK;
			}
			if (fp->f_type != DTYPE_SOCKET ||
			    (so = (struct socket *)fp->f_data) == 0)
				continue;
			if (so->so_proto->pr_domain != &unixdomain ||
			    (so->so_proto->pr_flags&PR_RIGHTS) == 0)
				continue;
			SOCKET_LOCK(so);		/* ??? */
			SOCKBUF_LOCK(&so->so_rcv);	/* ??? */
			if (so->so_rcv.sb_flags & SB_LOCK) {
				sosbwait(&so->so_rcv, so);
				/* Locks are released by sosbwait */
				goto restart;
			}
			unp_scan(so->so_rcv.sb_mb, unp_mark);
			SOCKBUF_UNLOCK(&so->so_rcv);
			SOCKET_UNLOCK(so);
		}
	} while (unp_defer);
	for (fp = file; fp < fileNFILE; fp++) {
		if (fp->f_count == 0)
			continue;
		if (fp->f_count == fp->f_msgcount && (fp->f_flag & FMARK) == 0)
			while (fp->f_msgcount)
				unp_discard(fp);
	}
	UNPMISC_LOCK();		/* restore lock */
	unp_gcing = 0;
}

void
unp_dispose(m)
	struct mbuf *m;
{
	if (m)
		unp_scan(m, unp_discard);
}

void
unp_scan(m0, op)
	register struct mbuf *m0;
	void (*op)();
{
	register struct mbuf *m;
	register struct file **rp;
	register struct cmsghdr *cm;
	register int i;
	int qfds;

	while (m0) {
		for (m = m0; m; m = m->m_next)
			if (m->m_type == MT_CONTROL &&
			    m->m_len >= sizeof(*cm)) {
				cm = mtod(m, struct cmsghdr *);
				if (cm->cmsg_level != SOL_SOCKET ||
				    cm->cmsg_type != SCM_RIGHTS)
					continue;
#if	SEC_ARCH
				if (security_is_on) {
				rp = (struct file **)
					findrights((caddr_t)(cm + 1),
						cm->cmsg_len - sizeof(*cm),
							SEC_RIGHTS_FDS, &qfds);
				if (rp)
					qfds /= sizeof(struct file *);
				else
					qfds = 0;
				} else
#endif	/* !SEC_ARCH */
				{
				qfds = (cm->cmsg_len - sizeof *cm)
						/ sizeof (struct file *);
				rp = align8((cm + 1), struct file **);
				}
				for (i = 0; i < qfds; i++) {
					/* 
					 * This could be the "socket" whose
					 * close triggered this scan, in that
					 * case the msgcount would be 0. We
					 * don't want to recurse in that case.
					 */
					if ((*rp)->f_msgcount)
						(*op)(*rp++);
				}
				break;		/* XXX, but saves time */
			}
		m0 = m0->m_act;
	}
}

void
unp_mark(fp)
	struct file *fp;
{

	FP_LOCK(fp);
	if (fp->f_flag & FMARK) {
		FP_UNLOCK(fp);
		return;
	}
	unp_defer++;
	fp->f_flag |= (FMARK|FDEFER);
	FP_UNLOCK(fp);
}

void
unp_discard(fp)
	struct file *fp;
{

	FP_LOCK(fp);
	fp->f_msgcount--;
	FP_UNLOCK(fp);
	UNPMISC_LOCK();
	unp_rights--;
	UNPMISC_UNLOCK();
	FP_UNREF(fp);
}
