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
static char *rcsid = "@(#)$RCSfile: str_tty.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/07/13 12:52:08 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */

/*
 * FILE:	str_tty.c - special stream head activities for TTY support.
 *
 * These activities are all related to the keyword "controlling tty". The
 * System V.3 STREAMS specification exposes the related data structures
 * and their semantics to the module, and lets it deal with it. In OSF/1
 * (and supposedly in V.4, too) we find a slightly changed data structure,
 * in order to implement POSIX standards. Therefore, we implement the
 * V.4 definition instead, PLUS what is needed to support OSF/1 ioctl's.
 *
 * The code which does the actual work here is copied from the traditional
 * TTY driver, and should be kept in sync with it.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/fcntl.h>		
#include <sys/user.h>		
#include <sys/proc.h>
#include <kern/parallel.h>
#include <sys/stat.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>

/*
 * ROUTINE:
 *	sth_ttyopen	- stream head activities after opening a tty
 *
 * PARAMETERS:
 *	sth		- the stream head
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 *	This routine is called when the stream head realizes that the
 *	stream acts as a terminal. This is the result of the module
 *	sending an M_SETOPTS message containing the SO_ISTTY flag
 *	during the qi_qopen processing.
 *
 *	The list of special activities needed at this time has currently
 *	only one element: the allocation of this stream as a controlling
 *	TTY. This is basically the same work, as we do it for the explicit
 *	TIOCSCTTY ioctl (see below), but in addition we need to check some
 *	more conditions.
 */

void
sth_ttyopen(sth, flag)
	STHP		sth;
	int		flag;
{
#ifdef COMPAT_43
	struct proc * p = u.u_procp;
        PROC_LOCK(p);  /* for p->p_pgrp (and therefore for session too) */
	if (!(flag&O_NOCTTY) && (!p->p_flag&SCTTY) &&  
	   ( !sth->sth_session) ) {
		if (!SESS_LEADER(p) && p->p_pgrp->pg_id != p->p_pid) {
                        PROC_UNLOCK(p);
			pgmv(p, p->p_pid, 1);
                } else
                        PROC_UNLOCK(p);
		(void)sth_tiocsctty(sth, p);
        } else 
                PROC_UNLOCK(p);
#endif /* COMPAT_43 */
}

/*
 * ROUTINE:
 *	sth_tiocsctty	- try to make this stream a controlling tty
 *
 * PARAMETERS:
 *	sth		- the stream head
 *
 * RETURN VALUE:
 *	EPERM		- permission denied
 *	0		- controlling terminal assigned
 *
 * DESCRIPTION:
 *	This the interface for the explicit allocation of a controlling
 *	TTY. This is used by the stream head when it finds a TIOCSCTTY ioctl
 *	request on a terminal stream. Note that this is handled by the
 *	stream head directly, i.e. there is "some" knowledge about
 *	TTYs. But since the VFS also knows about this one...
 *
 *	This routine is also used as part of the implict allocation
 *	during open.
 */

int
sth_tiocsctty(sth, p)
	STHP		sth;
	struct proc	*p;
{
	struct session *session;
	int	s;

	if (!(sth->sth_flags & F_STH_ISATTY))
		return ENOTTY;

	PROC_LOCK(p);	/* for pgrp and session */
	ASSERT(p->p_session);

	session = p->p_session;

	SESS_LOCK(session);
	if ( !SESS_LEADER(p) || 
	    ((session->s_ttyvp || sth->sth_session)
	    && (session != sth->sth_session))  ) {
		SESS_UNLOCK(session);
		PROC_UNLOCK(p);
		return EPERM;
	}
	session->s_fpgrpp = &sth->sth_pgrp;
	sth->sth_session = session;
	s = splhigh();
	SESS_FPGRP_LOCK(session);
	sth->sth_pgrp = p->p_pgrp;
	SESS_FPGRP_UNLOCK(session);
	splx(s);
	SESS_UNLOCK(session);
	p->p_flag |= SCTTY;
	PROC_UNLOCK(p);
	return 0;
}

/*
 * ROUTINE:
 *	sth_ttyclose	- stream head activities after closing a tty
 *
 * PARAMETERS:
 *	q	- read queue pointer
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 *	Deallocation of a controlling TTY. Currently, this only happens
 *	during close. This way, we at least don't lose data structures.
 */

void
sth_ttyclose(sth)
	STHP		sth;
{
	sth->sth_session = nil(struct session *);
	sth->sth_pgrp = nil(struct pgrp *);
}

/*
 * ROUTINE:
 *	sth_pgsignal - send a signal to this stream's process group
 *
 * PARAMETERS:
 *	sth	- stream head pointer
 *	sig	- signal number to send
 *
 * RETURN VALUE:
 *	(none)
 *
 * DESCRIPTION:
 *	Sends the indicated signal to the process group which has this
 *	stream as a controlling tty. Since we don't necessarily
 *	get called when we are no longer a CTTY, we have to check our
 *	data first.
 *
 *	For use by stream head only - modules must send M_SIG message.
 *	Caller must already have acquired stream head.
 */

void
sth_pgsignal(sth, sig)
	STHP	sth;
	int	sig;
{
	struct pgrp *pg;
	struct session	*session;
	int	s;

	
	session = sth->sth_session;
	if (session) {
		s = splhigh();
		SESS_FPGRP_LOCK(session);
	}
	if (pg = sth->sth_pgrp) {
		pgsignal(pg, sig, 1);
	}
	if (session) {
		SESS_FPGRP_UNLOCK(session);
		splx(s);
	}
}

int
sth_isctty(p, sth)
	struct proc	*p;
	STHP		sth;
{
	int rc;

	PROC_LOCK(p);	/* for pgrp and session */
	rc = (p->p_session == sth->sth_session) && (p->p_flag & SCTTY);
	PROC_UNLOCK(p);

	return rc;
}

int
sth_isbackground(p, sth)
	struct proc	*p;
	STHP		sth;
{
	int	rc;

	PROC_LOCK(p);	/* for pgrp and session */
	
	rc = (p->p_session == sth->sth_session) &&
	     (p->p_flag & SCTTY) &&
	     (p->p_pgrp != sth->sth_pgrp);

	PROC_UNLOCK(p);

	return rc;
}

int
osr_tiocspgrp(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	struct proc	*p = (struct proc *) osr->osr_ioctl_arg2p;
	pid_t		pgid = (pid_t)osr->osr_ioctl_arg1;
	struct pgrp	*pgrp;
	struct session	*session;
	int		s;

	if (PID_INVALID(pgid) || (pgid == 0))
		return (EINVAL);

	if(!P_REF(p))
		return EINVAL;
	if (!sth_isctty(p, sth)) {
		P_UNREF(p);
		return (ENOTTY);
	}
	pgrp = pgfind(pgid);
#ifdef COMPAT_43
	/*
	 * C-shell tries to set the tty's pgrp before it
	 * makes itself a pgrp leader.  Therefore, we now
	 * create a new process group, in the expectation
	 * that the program is about to do a setpgrp(0, pid).
	 * The fact that the C shell uses such a sequence
	 * is wrong, but we're trying to maintain compatibility.
	 */
	if (pgrp == NULL && pgid == p->p_pid) {
		pgmv(p, p->p_pid, 0);
		PROC_LOCK(p);
		if(pgrp = p->p_pgrp)
			PG_REF(pgrp);
		PROC_UNLOCK(p);
		if (pgrp == NULL) {
			P_UNREF(p);
			return(EPERM);
		}
	} else
#endif /* COMPAT_43 */
	/*
	 * We have reference on p and pgrp and session is read only.
	 */
	if (pgrp == NULL || pgrp->pg_session != p->p_session) {
		if (pgrp)
			PG_UNREF(pgrp);
		P_UNREF(p);
		return(EPERM);
	}
	P_UNREF(p);
	session = sth->sth_session;
	SESS_LOCK(session);
	s = splhigh();
	SESS_FPGRP_LOCK(session);
	sth->sth_pgrp = pgrp;
	SESS_FPGRP_UNLOCK(session);
	splx(s);
	SESS_UNLOCK(session);
	PG_UNREF(pgrp);
	return (0);
}

int
osr_tiocgpgrp(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	struct proc	*p = (struct proc *) osr->osr_ioctl_arg2p;
	struct session	*session;

	if (!sth_isctty(p, sth))
		return (ENOTTY);
	osr->osr_ioctl_arg0_len = sizeof (int);
	session = sth->sth_session;
	SESS_LOCK(session);
	*(int *)osr->osr_ioctl_arg0p =
			sth->sth_pgrp? sth->sth_pgrp->pg_id: PID_RSRVD;
	SESS_UNLOCK(session);
	return (0);
}

int
osr_tiocgsid(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	struct proc	*p = (struct proc *) osr->osr_ioctl_arg2p;

	if (!sth_isctty(p, sth))
		return (ENOTTY);
	osr->osr_ioctl_arg0_len = sizeof (pid_t);
	*(pid_t *)osr->osr_ioctl_arg0p = sth->sth_session->s_id;
	return (0);
}

int
osr_tiocsctty(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	struct proc	*p = (struct proc *) osr->osr_ioctl_arg2p;

	if ((sth->sth_flags & F_STH_ISATTY) == 0)
		return(ENOTTY);
	return (sth_tiocsctty(sth, p));
}

int
osr_tioccons(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;

	if ((sth->sth_flags & F_STH_ISATTY) == 0)
		return(ENOTTY);
	return (0);
}

/*
 * osr_bgcheck: check for attempt by background process to access
 * its controlling terminal.
 *
 * Argument: 
 *
 *	osr that originated as a pse_read, pse_write, or pse_ioctl
 *
 * Returns:
 *
 *	EIO	
 *
 * 		- if process state prevents signalling that job control
 *			shell needs (I/O should be failed)
 *
 *	0 	[with F_OSR_BLOCK_TTY set in osr_flags]
 *
 *		- if process should block, but should proceed with I/O 
 *			operation once foregrounded
 *
 *	0 	[with F_OSR_BLOCK_TTY clear in osr_flags]
 *
 *		- if process should proceed with I/O operation immediately
 *
 * Side effects:
 *
 *	The caller's process group may be signalled in some cases.
 *
 * Note:
 *
 *	The assumption of context (use of u.u_procp), which is valid
 *	in the current streams implementation, but may not be forever.
 */


int osr_bgcheck(osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	struct proc	*p = u.u_procp;
	int		is_ioctl = 0;
	PROC_INTR_VAR(s)

	ASSERT((osr->osr_flags & F_OSR_BLOCK_TTY) == 0);

	switch (osr->osr_flags & F_OSR_TTYBITS) {
	case F_OSR_RTTY_CHECK:		/* read-style check */
		if (sth_isbackground(p, sth)) {
			register struct pgrp	*pg;
			sigset_t		p_sigmask;

			PROC_LOCK(p);
			pg = p->p_pgrp;
			(void)PG_REF(pg);
			PROC_UNLOCK(p);

			BM(PROC_INTR_LOCK(p, s));
			p_sigmask = p->p_sigmask;
			BM(PROC_INTR_UNLOCK(p, s));

			BM(PGRP_READ_LOCK(pg));
			BM(PROC_SDATA_LOCK(p));
			if(sigismember(&p->p_sigignore, SIGTTIN) ||
			   sigismember(&p_sigmask, SIGTTIN) ||
			   pg->pg_jobc == 0) {
				BM(PROC_SDATA_UNLOCK(p));
				BM(PGRP_UNLOCK(pg));
				PG_UNREF(pg);
				return (EIO);
			}
			BM(PROC_SDATA_UNLOCK(p));
			BM(PGRP_UNLOCK(pg));
			pgsignal_tty(pg, SIGTTIN, 1, 0);
			PG_UNREF(pg);
			osr->osr_flags |= F_OSR_BLOCK_TTY;
		}
		return(0);
		/* NOT REACHED */

	case F_OSR_ITTY_CHECK:		/* ioctl-style check */
		is_ioctl = 1;
		/* Fall through */

	case F_OSR_WTTY_CHECK:		/* write-style check */
		if (sth_isbackground(p, sth) &&
		   (is_ioctl || (sth->sth_flags & F_STH_TOSTOP))) {
			register struct pgrp	*pg;
			sigset_t		p_sigmask;

			PROC_LOCK(p);
			pg = p->p_pgrp;
			(void)PG_REF(pg);
			PROC_UNLOCK(p);

			BM(PROC_INTR_LOCK(p, s));
			p_sigmask = p->p_sigmask;
			BM(PROC_INTR_UNLOCK(p, s));

			BM(PGRP_READ_LOCK(pg));
			BM(PROC_SDATA_LOCK(p));
			if(!sigismember(&p->p_sigignore, SIGTTOU) &&
			   !sigismember(&p_sigmask, SIGTTOU)) {
				if (pg->pg_jobc == 0) {
					BM(PROC_SDATA_UNLOCK(p));
					BM(PGRP_UNLOCK(pg));
					PG_UNREF(pg);
					return(EIO);
				}
				BM(PROC_SDATA_UNLOCK(p));
				BM(PGRP_UNLOCK(pg));

				pgsignal_tty(pg, SIGTTOU, 1, 0);
				osr->osr_flags |= F_OSR_BLOCK_TTY;
			}
			PG_UNREF(pg);
		}
		return(0);
		/* NOT REACHED */
	default:
		/* exactly one of the *TTY_CHECK flags should be set */
		panic("osr_bgcheck: unexpected osr_flags");
	}
}
