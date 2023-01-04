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
static char	*rcsid = "@(#)$RCSfile: sys_socket.c,v $ $Revision: 4.3.10.2 $ (DEC) $Date: 1993/08/02 20:40:43 $";
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
 * Copyright (c) 1982, 1986, 1990 Regents of the University of California.
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
 *	Base:	sys_socket.c	7.5 (Berkeley) 5/9/89
 *	Merged: sys_socket.c	7.8 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"
#if	MACH
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/ioctl.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/uio.h"
#include "sys/stat.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/domain.h"
#include "sys/protosw.h"
#include "sys/poll.h"

LOCK_ASSERTL_DECL

CONST struct	fileops socketops =
    { soo_read, soo_write, soo_ioctl, soo_select, soo_close };

/* ARGSUSED */
soo_read(fp, uio, cred)
	struct file *fp;
	struct uio *uio;
	struct ucred *cred;
{
	int flags = 0;

	BM(FP_LOCK(fp));
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		flags = MSG_NONBLOCK;
	BM(FP_UNLOCK(fp));

	return soreceive((struct socket *)fp->f_data, (struct mbuf **)0,
		uio, (struct mbuf **)0, (struct mbuf **)0, &flags);
}

/* ARGSUSED */
soo_write(fp, uio, cred)
	struct file *fp;
	struct uio *uio;
	struct ucred *cred;
{
#if	SEC_ARCH
	struct mbuf *control = 0;
	int error;
#endif
	int flags = 0;

	BM(FP_LOCK(fp));
	if (fp->f_flag & (FNDELAY|FNONBLOCK))
		flags = MSG_NONBLOCK;
	BM(FP_UNLOCK(fp));

#if	!SEC_ARCH
	return sosend((struct socket *)fp->f_data, (struct mbuf *)0,
		uio, (struct mbuf *)0, (struct mbuf *)0, flags);
#else	/* SEC_ARCH */
	if (error = sec_internalize_rights(&control))
		return error;
	return sosend((struct socket *)fp->f_data, (struct mbuf *)0,
		uio, (struct mbuf *)0, control, flags);
#endif
}

soo_ioctl(fp, cmd, data)
	struct file *fp;
	unsigned int cmd;
	register caddr_t data;
{
	register struct socket *so = (struct socket *)fp->f_data;
	int error = 0;
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
/* ULTRIX value for SIOCSPGRP */
if ((unsigned)cmd == 0x80047308) cmd = SIOCSPGRP;
/* ULTRIX value for SIOCGPGRP */
if ((unsigned)cmd == 0x80047309) cmd = SIOCGPGRP;
	switch (cmd) {

	case FIONBIO:
		FP_LOCK(fp);
		if (*(int *)data)
			fp->f_flag |= FNONBLOCK;
		else
			fp->f_flag &= ~FNONBLOCK;
		FP_UNLOCK(fp);
		error = ESUCCESS;
		break;

	case FIOASYNC:
		SOCKBUF_LOCK(&so->so_rcv);
		SOCKBUF_LOCK(&so->so_snd);
		if (*(int *)data) {
			so->so_state |= SS_ASYNC;
			so->so_rcv.sb_flags |= SB_ASYNC;
			so->so_snd.sb_flags |= SB_ASYNC;
		} else {
			so->so_state &= ~SS_ASYNC;
			so->so_rcv.sb_flags &= ~SB_ASYNC;
			so->so_snd.sb_flags &= ~SB_ASYNC;
		}
		SOCKBUF_UNLOCK(&so->so_snd);
		SOCKBUF_UNLOCK(&so->so_rcv);
		break;

	case FIONREAD:
#if	SEC_ARCH
		if (security_is_on) {
		*(int *)data = sec_sobufcount(&so->so_rcv, so);
		} else
#endif
		{
		SOCKBUF_LOCK(&so->so_rcv);
		*(int *)data = so->so_rcv.sb_cc;
		SOCKBUF_UNLOCK(&so->so_rcv);
		}
		break;

	case SIOCSPGRP:
		so->so_pgid = *(pid_t *)data;
		break;

	case SIOCGPGRP:
		*(pid_t *)data = so->so_pgid;
		break;

	case SIOCATMARK:
		*(int *)data = (so->so_state&SS_RCVATMARK) != 0;
		break;

	default:
		/*
		 * Interface/routing/protocol specific ioctls:
		 * interface and routing ioctls should have a
		 * different entry since a socket's unnecessary
		 * However, socket SS_PRIV bit serves as auth.
		 */
		sopriv(so);
		if (IOCGROUP(cmd) == 'i')
			error = ifioctl(so, cmd, data);
		else if (IOCGROUP(cmd) == 'r')
			error = rtioctl(so, cmd, data);
		else
			error = ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL, 
		    (struct mbuf *)cmd, (struct mbuf *)data, (struct mbuf *)0));
		break;
	}
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

soo_select(fp, events, revents, scanning)
	struct file *fp;
	short *events, *revents;
	int scanning;
{
	register struct socket *so = (struct socket *)fp->f_data;
	DOMAIN_FUNNEL_DECL(f)

	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);

	/*
	 * Note - only the so_rcv selqueue is used.
	 * It's protected by the socket lock.
	 */
	if (scanning) {
		if (so->so_error)
			*revents |= POLLERR;
		else if (so->so_state & SS_CANTRCVMORE)
			*revents |= POLLHUP;
		if (*events & (POLLNORM|POLLPRI)) {
			SOCKBUF_LOCK(&so->so_rcv);
			if (*events & POLLNORM) {
				if (soreadable(so))
					*revents |= POLLNORM;
				else if (*revents == 0)
					so->so_rcv.sb_flags |= SB_SEL;
			}
			if (*events & POLLPRI) {
				if (so->so_oobmark ||
				    (so->so_state & SS_RCVATMARK))
					*revents |= POLLPRI;
				else if (*revents == 0)
					so->so_rcv.sb_flags |= SB_SEL;
			}
			SOCKBUF_UNLOCK(&so->so_rcv);
		}
		if (*events & POLLOUT) {
			SOCKBUF_LOCK(&so->so_snd);
			if (sowriteable(so))
				*revents |= POLLOUT;
			else if (*revents == 0)
				so->so_snd.sb_flags |= SB_SEL;
			SOCKBUF_UNLOCK(&so->so_snd);
		}
		if (*revents == 0)
			select_enqueue(&so->so_rcv.sb_selq);
	} else
		select_dequeue(&so->so_rcv.sb_selq);

	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (0);
}

#include <sys/mode.h>

soo_stat(so, ub)
	register struct socket *so;
	register struct stat *ub;
{
	int error = 0;
	DOMAIN_FUNNEL_DECL(f)

	bzero((caddr_t)ub, sizeof (*ub));
	DOMAIN_FUNNEL(sodomain(so), f);
	SOCKET_LOCK(so);
	error = ((*so->so_proto->pr_usrreq)(so, PRU_SENSE,
	    (struct mbuf *)ub, (struct mbuf *)0, 
	    (struct mbuf *)0));
	if(so->so_special & SP_PIPE)
		ub->st_mode |= S_IFIFO;
	else
		ub->st_mode |= S_IFSOCK;
	SOCKET_UNLOCK(so);
	DOMAIN_UNFUNNEL(f);
	return (error);
}

soo_close(fp)
	struct file *fp;
{
	struct socket *so;
	int error = 0;

	/*
	 * Should pass the f_flag to soclose as for per-file nonblock,
	 * but use of NBIO avoids interface change. Socket is discarded
	 * so NBIO goes with it.
	 */
	FP_LOCK(fp);
	if (so = (struct socket *)fp->f_data) {
		if (fp->f_flag & (FNDELAY|FNONBLOCK))
			so->so_state |= SS_NBIO;
		FP_UNLOCK(fp);
		error = soclose(so);
		FP_LOCK(fp);
		fp->f_data = 0;
	}
	FP_UNLOCK(fp);
	return (error);
}


