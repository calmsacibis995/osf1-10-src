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
static char	*sccsid = "@(#)$RCSfile: tty_tty.c,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/05/12 15:21:46 $";
#endif 
/*
 */
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

/*
 * Indirect driver for controlling tty.
 */
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/user.h>
#include <sys/mount.h>			/* funnelling through mnt struct */
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/uio.h>
#include <kern/parallel.h>


#define CTTYVP(vp, p)   cttyvp(&(vp), p)

cttyvp(vp, p)
struct vnode    **vp;
struct proc     *p;
{
        PROC_LOCK(p);
        SESS_LOCK(p->p_session);
        *vp = p->p_flag&SCTTY? p->p_session->s_ttyvp: NULL;
        if(*vp)
                VREF(*vp);
        SESS_UNLOCK(p->p_session);
        PROC_UNLOCK(p);
}

/*ARGSUSED*/
syopen(dev, flag)
	dev_t dev;
	long flag;
{
	struct vnode *ttyvp;
	int error;

	CTTYVP(ttyvp, u.u_procp);

	if (ttyvp == NULL) {
		return (ENXIO);
	}
	VOP_OPEN(&ttyvp, flag, NOCRED, error);
	VUNREF(ttyvp);
	return (error);
}

/*ARGSUSED*/
syread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	long flag;
{
	struct vnode *ttyvp;
	int error;

	CTTYVP(ttyvp, u.u_procp);
	if (ttyvp == NULL) {
		return (ENXIO);
	}
	VOP_READ(ttyvp, uio, flag, NOCRED, error);
	VUNREF(ttyvp);
	return (error);
}

/*ARGSUSED*/
sywrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	long flag;
{
	struct vnode *ttyvp;
	int error;

	CTTYVP(ttyvp, u.u_procp);

	if (ttyvp == NULL) {
		return (ENXIO);
	}
	VOP_WRITE(ttyvp, uio, flag, NOCRED, error);
	VUNREF(ttyvp);
	return (error);
}

/*ARGSUSED*/
syioctl(dev, cmd, addr, flag)
	dev_t dev;
	unsigned int cmd;
	caddr_t addr;
	long flag;
{
	int retval;
	struct vnode *ttyvp;
	int error;

	CTTYVP(ttyvp, u.u_procp);

	if (ttyvp == NULL) {
		return (ENXIO);
	}
        if (cmd == TIOCSCTTY) {
                VUNREF(ttyvp);
                return(EJUSTRETURN);
	}
	if (cmd == TIOCNOTTY) {
		PROC_LOCK(u.u_procp);
		if (!SESS_LEADER(u.u_procp)) {
			u.u_procp->p_flag &= ~SCTTY;
			PROC_UNLOCK(u.u_procp);
#ifdef	COMPAT_43
			pgmv(u.u_procp, 0, 0);
#endif
			VUNREF(ttyvp);
			return (0);
		} else {
			PROC_UNLOCK(u.u_procp);
			VUNREF(ttyvp);
			return (EINVAL);
		}
	}
	VOP_IOCTL(ttyvp, cmd, addr, flag, NOCRED, error,&retval);
	VUNREF(ttyvp);
	return (error);
}

/*ARGSUSED*/
syselect(dev, events, revents, scanning)
	dev_t dev;
	short *events, *revents;
	int scanning;
{
	struct vnode *ttyvp;
	int error;

	CTTYVP(ttyvp, u.u_procp);

	if (ttyvp == NULL) {
		return (ENXIO);
	}
	VOP_SELECT(ttyvp, events, revents, scanning, NOCRED, error);
	VUNREF(ttyvp);
	return (error);
}
