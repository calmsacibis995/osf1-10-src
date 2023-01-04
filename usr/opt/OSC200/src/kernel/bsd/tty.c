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
static char *rcsid = "@(#)$RCSfile: tty.c,v $ $Revision: 4.4.20.7 $ (DEC) $Date: 1993/10/19 18:47:26 $";
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
 * tty.c
 *
 * Modification History:
 *
 * 24-Feb-92	Fred Canter
 *	In ttyopen(), don't assign a controlling tty if process already
 *	has one. Part of xtrek X server hang fix.
 *
 * 5-Nov-91	Mike Larson
 *	Fix flow control activation in ttyblock().
 *	Base execution of all POSIX implementation defined functions on
 *	state of IEXTEN flag.
 *
 *  6-Nov-91	Philip Cameron
 *	Changed ttyopen() to follow BDS rules and establish a session
 *	and controlling terminal when a terminal that is not already a
 *	controlling terminal is opened by a process that is not a 
 *	session leader or process group leader and the O_NOCTTY flag is
 *	not present. OSF rules are preserved by always asserting O_NOCTTY
 * 	in the open function (vfs/vfs_syscalls.c)
 *
 * 5-Nov-91	Mike Larson
 *	Remove TCGETA from 'switch' statement in ttioctl() that sends
 *	SIGTTOU to a background process.
 *
 * 8-OCT-91	B Harrigan
 *	Disabled NCPUS > 1 code for RT_PREEMPT. Driver is not MP safe on mips.
 *      Removed 30-aug-91 changes to macros, as they have been placed in the 
 *      .h files
 *
 *  8-Oct-91  Philip Cameron
 *    Removed ult_bin_isatty_fix. No longer needed.
 * 15-Aug-91	Philip Cameron
 *	Added TCGETA to the ult_bin_isatty_fix
 *
 * 30-AUG-91    Brian Harrigan
 *      Redefined TTY_LOCK and changed FUNNEL defs for RT_PREEMPT RT MPK
 *	Also removed RT specific calls to mpsleep. This module is not mp
 *     safe as called from MIPS machines.(EFT)
 *     The FUNNEL and TTY_LOCK  Hacks should be fixed in the .h files ...
 *
 *  2-Jul-91	Jim McGinness
 *	Changed "ult_bin" to "ult_bin_isatty_fix", which is now a configurable
 *	parameter.
 *
 * 7-May-91	Ron Widyono
 *	Enabled NCPUS > 1 code for RT_PREEMPT.
 *
 * 21-Jan-91	Fred Canter
 *	Hack for ULTRIX isatty() binary compatibilty.
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
 *	@(#)tty.c	3.4 (Berkeley) 7/2/91
 */

#include <rt_preempt.h>

#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/proc.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/conf.h>
#include <sys/dk.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/poll.h>
#include <machine/reg.h>
#include <kern/parallel.h>
#include <kern/assert.h>
#include <mach/thread_info.h>
#if	SEC_BASE
#include <sys/security.h>
#endif

#include <cputypes.h>
#include <sys/termio.h>

short	tthiwat[NSPEEDS], ttlowat[NSPEEDS];


extern ttin_timeout();

extern cc_t ttydefchars[];

/*
 * Is 'c' a line delimiter ("break" character)?
 */
#define ttbreakc(c) (c == '\n' || CCEQ(cc[VEOF], c) || \
		CCEQ(cc[VEOL], c) || CCEQ(cc[VEOL2], c))

isctty(p, tp)
        struct proc     *p;
        struct tty      *tp;
{
        int rc;

        /* p is current proc */
        PROC_LOCK(p);
        rc = (p->p_pgrp->pg_session == tp->t_session) && (p->p_flag&SCTTY);
        PROC_UNLOCK(p);

        return rc;
}

isbackground(p, tp)
        struct proc     *p;
        struct tty      *tp;
{
        int rc;

        PROC_LOCK(p);
        rc = p->p_pgrp->pg_session == tp->t_session &&
             p->p_flag&SCTTY &&
             p->p_pgrp != tp->t_pgrp;
        PROC_UNLOCK(p);

        return rc;
}
ttychars(tp)
	struct tty *tp;
{
	LASSERT(TTY_LOCK_HOLDER(tp));
	bcopy(ttydefchars, tp->t_cc, NCCS * sizeof(cc_t));
}

/*
 * Wait for output to drain, then flush input waiting.
 */
ttywflush(tp)
	struct tty *tp;
{
	int error;

	LASSERT(TTY_LOCK_HOLDER(tp));
	if ((error = ttywait(tp)) == 0)
		ttyflush(tp, FREAD);
	return (error);
}

/*
 * Wait for output to drain.
 * If closing (TS_DRAIN_CL), wait for a maximum of MAX_WAIT
 */
#define MIN_WAIT hz/4
#define MAX_WAIT 120*hz
ttywait(tp)
	register struct tty *tp;
{
	int cc,tmo,error = 0;
	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	TSPLTTY(s);
	while ((tp->t_outq.c_cc || tp->t_state&TS_BUSY) &&
	    (tp->t_state&TS_CARR_ON || tp->t_cflag&CLOCAL) &&
	    tp->t_oproc) {
		(*tp->t_oproc)(tp);
		tp->t_state |= TS_ASLEEP;
		if ((tp->t_state & TS_DRAIN_CL) == 0) { 

			if (error = ttysleep(tp, (caddr_t)&tp->t_outq, TTOPRI | PCATCH, ttyout))
				break;
		}
		else {
			cc = tp->t_outq.c_cc;
			tmo = cc*(hz/4);  /* allow .25 second per character */
			if (!tmo)
				tmo  = MIN_WAIT;
			else if (tmo > MAX_WAIT)
				tmo = MAX_WAIT;
			if (error = ttysleep_tmo(tp, (caddr_t)&tp->t_outq, TTOPRI | PCATCH, ttyout,tmo)) {
				if (error != EWOULDBLOCK)
					break;
				if (cc == tp->t_outq.c_cc) 
					break;
			}
		} 
	}
	TSPLX(s);
	return (error);
}

/*
 * Flush all TTY queues
 */
ttyflush(tp, rw)
	register struct tty *tp;
	long rw;
{
	int error;

	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	TSPLTTY(s);
	if (rw & FREAD) {
		while (getc(&tp->t_canq) >= 0)
			;
		ttwakeup(tp);
	}
	if (rw & FWRITE) {
		thread_wakeup((vm_offset_t)&tp->t_outq);
		tp->t_state &= ~TS_TTSTOP;
		CDEVSW_STOP(major(tp->t_dev), tp, rw, error);
		while (getc(&tp->t_outq) >= 0)
			;
	}
	if (rw & FREAD) {
		while (getc(&tp->t_rawq) >= 0)
			;
		tp->t_rocount = 0;
		tp->t_rocol = 0;
		tp->t_state &= ~TS_LOCAL;
	}
	TSPLX(s);
	select_wakeup(&tp->t_selq);
	select_dequeue_all(&tp->t_selq);
}

/*
 * Send stop character on input overflow.
 */
ttyblock(tp)
	register struct tty *tp;
{
	register x;

	LASSERT(TTY_LOCK_HOLDER(tp));
	x = tp->t_rawq.c_cc + tp->t_canq.c_cc;
	if (tp->t_rawq.c_cc > tp->t_hog) {
		ttyflush(tp, FREAD|FWRITE);
		tp->t_state &= ~TS_TBLOCK;
	}
	/*
	 * Block further input iff:
	 * Current input > threshold AND input is available to user program
	 */
	if (x >= tp->t_hog/2 &&
	    (!(tp->t_lflag&ICANON) || (tp->t_canq.c_cc > 0)) &&
	    tp->t_cc[VSTOP] != _POSIX_VDISABLE) {
		if ((tp->t_state & TS_NOFLOWCHARS) || putc(tp->t_cc[VSTOP], &tp->t_outq)==0) {
			tp->t_state |= TS_TBLOCK;
			ttstart(tp);
		}
	}
	LASSERT(TTY_LOCK_HOLDER(tp));
}

/*
 * Restart typewriter output following a delay
 * timeout.
 * The name of the routine is passed to the timeout
 * subroutine and it is called during a clock interrupt.
 *
 * WARNING:  this routine must always be called in thread
 * context and so can't be called directly from a clock interrupt!
 */
ttrstrt(tp)
	struct tty *tp;
{
	TSPLVAR(ipl)

	TSPLTTY(ipl);
	TTY_LOCK(tp);
	if (tp == 0)
		panic("ttrstrt");
	tp->t_state &= ~TS_TIMEOUT;
	ttstart(tp);
	TTY_UNLOCK(tp);
	TSPLX(ipl);
}

/*
 * Start output on the typewriter. It is used from the top half
 * after some characters have been put on the output queue,
 * from the interrupt routine to transmit the next
 * character, and after a timeout has finished.
 */
ttstart(tp)
	struct tty *tp;
{
	LASSERT(TTY_LOCK_HOLDER(tp));
	if (tp->t_oproc)		/* kludge for pty */
		(*tp->t_oproc)(tp);
}

/*
 * Common code for tty ioctls.
 */

/*ARGSUSED*/
ttioctl(tp, com, data, flag)
	register struct tty *tp;
	unsigned int com;
	caddr_t data;
	long flag;
{
	extern int nldisp;
	int error;
        register struct proc    *p = u.u_procp;
	TSPLVAR(s)
	PROC_INTR_VAR(ps);

	LASSERT(TTY_LOCK_HOLDER(tp));
	/*
	 * If the ioctl involves modification,
	 * hang if in the background.
	 */
	switch (com) {

	case TIOCSETD:
	case TIOCFLUSH:
	case TIOCSPGRP:
	case TIOCSTI:
	case TIOCSWINSZ:
	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF:
#ifdef COMPAT_43
	case TIOCSETP:
	case TIOCSETN:
	case TIOCSETC:
	case TIOCSLTC:
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
	case OTIOCSETD:
#endif

/* SVID ioctls */
  	case TCXONC:
	case TCFLSH:
	case TCSETAW:
	case TCSETAF:
	case TCSETA:
        case TCSBREAK:
		while (ISBACKGROUND(p, tp))  {
		   register struct pgrp *pg;
                   int                     dosignal;
                   sigset_t                p_sigmask;
                        /*
                         * Take a reference on our pgrp.  This is to keep
                         * the pgrp alive across the pgsignal, just in case
                         * the parent or another thread moved us to another
                         * and this one got deleted, or worse, reallocated.
                         */


		
		   PROC_LOCK(p);
		   pg = p->p_pgrp;
		   (void)PG_REF(pg);
		   PROC_UNLOCK(p);

                   p_sigmask = p->p_sigmask;

                   dosignal = pg->pg_jobc &&
                           !sigismember(&p->p_sigignore, SIGTTOU) &&
                           !sigismember(&p_sigmask, SIGTTOU);
		   if (dosignal) {
			pgsignal_tty(pg, SIGTTOU, 1,1);
			PG_UNREF(pg);
			if (error = ttysleep(tp, (caddr_t)&lbolt,
				TTOPRI | PCATCH, ttybg)) {
				return (error);
			}
		   } else {
			PG_UNREF(pg);
			break;
		   }

		}
		break;
	}

	/*
	 * Process the ioctl.
	 */
	switch (com) {

	/* get discipline number */
	case TIOCGETD:
#ifdef	balance
		if (tp->t_line == 2)
			*(int *)data = 1;
		else
#else
		*(int *)data = tp->t_line;
#endif
		break;

	/* set line discipline */
	case TIOCSETD: {
		register int t = *(int *)data;
		dev_t dev = tp->t_dev;
#ifdef	balance
		if (t == 1)
			t = 2;
#endif

		if ((unsigned)t >= nldisp)
			return (ENXIO);
		if (t != tp->t_line) {
			TSPLTTY(s);
			(*linesw[tp->t_line].l_close)(tp);
			error = (*linesw[t].l_open)(dev, tp, flag);
			if (error) {
				(void)(*linesw[tp->t_line].l_open)(dev, tp, flag);
				TSPLX(s);
				return (error);
			}
			tp->t_line = t;
			TSPLX(s);
		}
		break;
	}

	/* prevent more opens on channel */
	case TIOCEXCL:
		TSPLTTY(s);
		tp->t_state |= TS_XCLUDE;
		TSPLX(s);
		break;

	case TIOCNXCL:
		TSPLTTY(s);
		tp->t_state &= ~TS_XCLUDE;
		TSPLX(s);
		break;

	case TIOCHPCL:
		TSPLTTY(s);
		tp->t_cflag |= HUPCL;
		TSPLX(s);
		break;

	case TIOCFLUSH: {
		register int flags = *(int *)data;

		if (flags == 0)
			flags = FREAD|FWRITE;
		else
			flags &= FREAD|FWRITE;
		ttyflush(tp, flags);
		break;
	}

	case FIOASYNC:
		if (*(int *)data)
			tp->t_state |= TS_ASYNC;
		else
			tp->t_state &= ~TS_ASYNC;
		break;

	case FIONBIO:
		break;	/* XXX remove */

	/* return number of characters immediately available */
	case FIONREAD:
		TSPLTTY(s);
		*(off_t *)data = ttnread(tp);
		TSPLX(s);
		break;

	case TIOCOUTQ:
		*(int *)data = tp->t_outq.c_cc;
		break;

	case TIOCSTOP:
		TSPLTTY(s);
		if ((tp->t_state&TS_TTSTOP) == 0) {
			tp->t_state |= TS_TTSTOP;
			CDEVSW_STOP(major(tp->t_dev), tp, 0, error);
		}
		TSPLX(s);
		break;

	case TIOCSTART:
		TSPLTTY(s);
		if ((tp->t_state&TS_TTSTOP) || (tp->t_lflag&FLUSHO)) {
			tp->t_state &= ~TS_TTSTOP;
			tp->t_lflag &= ~FLUSHO;
			ttstart(tp);
		}
		TSPLX(s);
		break;

	/*
	 * Simulate typing of a character at the terminal.
	 */
	case TIOCSTI:
#if	SEC_BASE
		if ((flag & FREAD) == 0 && !privileged(SEC_ALLOWDACACCESS, 0))
			return (EPERM);
		if (!ISCTTY(p, tp) && !privileged(SEC_ALLOWDACACCESS,0)) {
			return (EACCES);
		}
#else
		if (u.u_uid && (flag & FREAD) == 0)
			return (EPERM);
		if (u.u_uid && !ISCTTY(p, tp)) {
			return (EACCES);
		}
#endif
		TSPLTTY(s);
		(*linesw[tp->t_line].l_rint)(*(char *)data, tp);
		TSPLX(s);
		break;

	case TIOCGETA: {
		struct termios *t = (struct termios *)data;

		bcopy(&tp->t_termios, t, sizeof(struct termios));
		break;
	}

	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF: {
		register struct termios *t = (struct termios *)data;
		int permanent = 0;

		TSPLTTY(s);
		if (com == TIOCSETAW || com == TIOCSETAF) {
			if (error = ttywait(tp)) {
				TSPLX(s);
				return (error);
			}
			if (com == TIOCSETAF)
				ttyflush(tp, FREAD);
		}
		if ((tp->t_cflag&CLOCAL) != (t->c_cflag&CLOCAL)) {
			if(t->c_cflag & CLOCAL) {
				tp->t_state &= ~TS_MODEM_ON;
				cdevsw[major(tp->t_dev)].d_ioctl(tp->t_dev,
				       TIOCNMODEM,&permanent,0);
			} 
			else {
				tp->t_state |= TS_MODEM_ON;
				cdevsw[major(tp->t_dev)].d_ioctl(tp->t_dev,
				       TIOCMODEM,&permanent,0);
			}
		}

		/*
		 * set device hardware - if necessary
		 * Also param routine moves speed settings
	         * and cflag to tty struct so don't have to 
		 * it here
		 */

		if (t->c_ispeed == 0)
			t->c_ispeed = t->c_ospeed;
		if ((tp->t_cflag != t->c_cflag) || 
		    (tp->t_ospeed != t->c_ospeed) || 
		    (tp->t_ispeed != t->c_ispeed)) {
			if (tp->t_param && (error = (*tp->t_param)(tp, t))) {
				TSPLX(s);
				return (error);
			}
		}
		ttsetwater(tp);
		if (com != TIOCSETAF) {
			if ((t->c_lflag&ICANON) != (tp->t_lflag&ICANON))
				if (t->c_lflag&ICANON) {	
					tp->t_lflag |= PENDIN;
					ttwakeup(tp);
				}
				else {
					struct clist tq;

					catq(&tp->t_rawq, &tp->t_canq);
					tq = tp->t_rawq;
					tp->t_rawq = tp->t_canq;
					tp->t_canq = tq;
				}
		}
		bcopy(t, &tp->t_termios, sizeof(struct termios));
		tp->t_shad_time = t->c_cc[VTIME] * hz / 10;
		TSPLX(s);
		break;
	}

	/*
	 * Set controlling terminal.
	 * Session ctty vnode pointer set in vnode layer.
	 */
	case TIOCSCTTY: {
		struct session *s;
		int	spl;

		PROC_LOCK(p);
		/*
		 * No other threads in this session will be here since we
		 * have the tty lock.
		 * It is possible that another thread can change the
		 * process group while are looking at it, but it will
		 * always be in the same session.  Session leader is a
		 * read only field for the process.
		 */
		s = p->p_session;		/* save it now */
		if(p->p_flag & SCTTY) {
			PROC_UNLOCK(p);
			return (EPERM);
		}
		
		SESS_LOCK(s);		/* for s_ttyvp */
		if(s->s_leader != p || (s->s_ttyvp || tp->t_session) &&
		  (tp->t_session != s)) {
			SESS_UNLOCK(s);
			PROC_UNLOCK(p);
			return (EPERM);
		}
		s->s_fpgrpp = &tp->t_pgrp;
		tp->t_session = s;
		spl = splhigh();
		SESS_FPGRP_LOCK(s);
		tp->t_pgrp = p->p_pgrp;
		SESS_FPGRP_UNLOCK(s);
		splx(spl);
		SESS_UNLOCK(s);
		p->p_flag |= SCTTY;
		PROC_UNLOCK(p);
		break;
	}

		
	/*
	 * Set terminal process group.
	 */
	case TIOCSPGRP: {
		register struct pgrp *pgrp;
		struct session	*s;
		int	spl;

		if (*(int *)data == -1) {
			return (EINVAL);
		}
		pgrp = pgfind(*(int *)data);

		/*
		 * It doesn't matter if this proc's pgrp changes, since
		 * its session will be the same.
		 */
		s = p->p_pgrp->pg_session;

		if (!ISCTTY(p, tp)) {
			return (ENOTTY);
		}
#ifdef COMPAT_43
		/*
		 * C-shell tries to set the tty's pgrp before it
		 * makes itself a pgrp leader.  Therefore, we now
		 * create a new process group, in the expectation
		 * that the program is about to do a setpgrp(0, pid).
		 * The fact that the C shell uses such a sequence
		 * is wrong, but we're trying to maintain compatibility.
		 */
		else if (pgrp == NULL && *(int *)data == p->p_pid) {
			pgmv(p, p->p_pid, 0);
			pgrp = p->p_pgrp;
			if (pgrp == NULL) {
				return(EPERM);
			}
		}
#endif /* COMPAT_43 */
		else if (pgrp == NULL || pgrp->pg_session != s) {
			return (EPERM);
		}
		SESS_LOCK(s);
		spl = splhigh();
		SESS_FPGRP_LOCK(s);
		tp->t_pgrp = pgrp;
		SESS_FPGRP_UNLOCK(s);
		splx(spl);
		SESS_UNLOCK(s);
		break;
	}


 	case TIOCGSID:
 		if (!ISCTTY(p, tp)) {
 			return (ENOTTY);
 		}
 		*(pid_t *)data = tp->t_session->s_id;
 		break;

	case TIOCGPGRP:
		if (!ISCTTY(p, tp)) {
			return (ENOTTY);
		}
		*(int *)data = tp->t_pgrp? tp->t_pgrp->pg_id: PID_RSRVD;
		break;

	case TIOCSWINSZ:
		if (bcmp((caddr_t)&tp->t_winsize, data,
		    sizeof (struct winsize))) {
			tp->t_winsize = *(struct winsize *)data;
			pgsignal_tty(tp->t_pgrp, SIGWINCH, 1,1);
		}
		break;

	case TIOCGWINSZ:
		*(struct winsize *)data = tp->t_winsize;
		break;

	case TIOCMODEM:
		if (*(int *)data) {
#if	SEC_BASE
			if (!privileged(SEC_ALLOWDACACCESS, 0))
				return (EPERM);
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				return (error);
#endif
		}
		return(-1);
		break;

	case TIOCNMODEM:
		if (*(int *)data) {
#if	SEC_BASE
			if (!privileged(SEC_ALLOWDACACCESS, 0))
				return (EPERM);
#else
			if (error = suser(u.u_cred, &u.u_acflag))
				return (error);
#endif
		}
		return(-1);
		break;

	case TIOCCONS:
		/* now handled in vn_ioctl - just ACK here */
		break;

#ifdef COMPAT_43
	case TIOCGETP:
	case TIOCSETP:
	case TIOCSETN:
	case TIOCGETC:
	case TIOCSETC:
	case TIOCSLTC:
	case TIOCGLTC:
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
	case TIOCLGET:
	case OTIOCGETD:
	case OTIOCSETD:
		return(ttcompat(tp, com, data, flag));
#endif

/* SVID ioctls */
  	case TCXONC:
	case TCFLSH:
	case TCGETA:
	case TCSETAW:
	case TCSETAF:
	case TCSETA:
		return(tt_sysv_compat(tp, com, data, flag));
		
	case TCSBREAK:
		if (error = ttywait(tp))
                        return (error);
                if (*(int *)data >= 0)
                        return (-1);
                break;
                
	default:
		return (-1);
	}
	return (0);
}

ttnread(tp)
	struct tty *tp;
{
	int nread = 0;

	LASSERT(TTY_LOCK_HOLDER(tp));
	if (tp->t_lflag & PENDIN)
		ttypend(tp);
	nread = tp->t_canq.c_cc;
	if ((tp->t_lflag & ICANON) == 0)
		nread += tp->t_rawq.c_cc;
	return (nread);
}

int
ttselect(dev, events, revents, scanning)
	int scanning;
	dev_t dev;
	short *events, *revents;
{
	register struct tty *tp;
	int nread;

	CDEVSW_TTYS(major(dev), minor(dev), tp);
	DEVSW_FUNNEL(c,major(dev));
	TTY_LOCK(tp);

	tpselect(tp, events, revents, scanning);

	TTY_UNLOCK(tp);
	DEVSW_UNFUNNEL(c,major(dev));
	return (0);
}

/*
 * select routine which takes a pointer to tty struct instead of
 * a dev_t
 */
tpselect(tp, events, revents, scanning)
	register struct tty *tp;
	short *events, *revents;
	int scanning;
{
	int nread;
	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	TSPLTTY(s);
	if (*events & POLLNORM) {
		if (scanning) {
			nread = ttnread(tp);
			if (nread > 0 ||
			   (!(tp->t_cflag&CLOCAL) && !(tp->t_state&TS_CARR_ON)))
				*revents |= POLLNORM;
			else
				select_enqueue(&tp->t_selq);
		} else
			select_dequeue(&tp->t_selq);
	}
	if (*events & POLLOUT) {
		if (scanning) {
			if (tp->t_outq.c_cc <= tp->t_lowat)
				*revents |= POLLOUT;
			else
				select_enqueue(&tp->t_selq);
		} else
			select_dequeue(&tp->t_selq);
	}

	if (scanning)
		if (!(tp->t_state&TS_CARR_ON))
			*revents |= POLLHUP;


	TSPLX(s);
	return (0);
}

/*
 * Initial open of tty, or (re)entry to line discipline.
 */
ttyopen(dev, tp, flags)
	dev_t dev;
	register struct tty *tp;
	long flags;
{

#ifdef	COMPAT_43
	register struct proc *pp;
#endif

	LASSERT(TTY_LOCK_HOLDER(tp));
	tp->t_dev = dev;
	tp->t_shad_time = tp->t_cc[VTIME] * hz / 10;
	if (!tp->t_hog)
		tp->t_hog = TTYHOG;

#ifdef COMPAT_43
	pp = u.u_procp;
        PROC_LOCK(pp);  /* for p->p_pgrp (and therefore for session too) */
	if (!(flags&O_NOCTTY) && !(pp->p_flag&SCTTY) && tp->t_session==NULL) {
		if (!SESS_LEADER(pp) && pp->p_pgrp->pg_id != pp->p_pid) {
                        PROC_UNLOCK(pp);
			pgmv(pp, pp->p_pid, 1);
                } else
                        PROC_UNLOCK(pp);
                (void) ttioctl(tp, TIOCSCTTY, (caddr_t)0, 0);
        } else {
                PROC_UNLOCK(pp);
        }
#endif /* COMPAT_43 */

	tp->t_state &= ~TS_WOPEN;
	if ((tp->t_state & TS_ISOPEN) == 0) {
		tp->t_state |= TS_ISOPEN;
		bzero((caddr_t)&tp->t_winsize, sizeof(tp->t_winsize));
	}
	return (0);
}

/*
 * "close" a line discipline
 */
ttylclose(tp)
	register struct tty *tp;
{

	LASSERT(TTY_LOCK_HOLDER(tp));
	/* Restart output if blocked */
	ttioctl(tp, TIOCSTART, 0, 0);
	tp->t_state |= TS_DRAIN_CL;
	ttywflush(tp);
	tp->t_state &= ~TS_DRAIN_CL;
}

/*
 * clean tp on last close
 */
ttyclose(tp)
	register struct tty *tp;
{
	struct session *s;

	LASSERT(TTY_LOCK_HOLDER(tp));
	ttyflush(tp, FREAD|FWRITE);
	if (s = tp->t_session) {
		tp->t_session = NULL;
	}
	tp->t_pgrp = NULL;
	tp->t_hog = 0;
	tp->t_state = 0;
	tp->t_col = tp->t_rocol = 0;
	tp->t_gen++;
	return (0);
}

/*
 * Handle modem control transition on a tty.
 * Flag indicates new state of carrier.
 * Returns 0 if the line should be turned off, otherwise 1.
 */
ttymodem(tp, flag)
	register struct tty *tp;
	long flag;
{

	int error;
	struct proc  *p;

	LASSERT(TTY_LOCK_HOLDER(tp));
	if ((tp->t_state&TS_WOPEN) == 0 && (tp->t_lflag & MDMBUF)) {
		/*
		 * MDMBUF: do flow control according to carrier flag
		 */
		if (flag) {
			tp->t_state &= ~TS_TTSTOP;
			ttstart(tp);
		} else if ((tp->t_state&TS_TTSTOP) == 0) {
			tp->t_state |= TS_TTSTOP;
			CDEVSW_STOP(major(tp->t_dev), tp, 0, error);
		}
	} else if (flag == 0) {
		/*
		 * Lost carrier.
		 */
		struct session *s = tp->t_session;
                tp->t_state &= ~TS_CARR_ON;
                if(tp->t_state&TS_ISOPEN && (tp->t_cflag&CLOCAL) == 0) {
                        if(s) {
                                if(p = s->s_leader) {
                                        if(P_REF(p)) {
                                                psignal_tty(p, SIGHUP);
                                                P_UNREF(p);
                                        }
                                } 
                        }
                        ttyflush(tp, FREAD|FWRITE);
                        return (0);
                }
	} else {
		/*
		 * Carrier now on.
		 */
		tp->t_state |= TS_CARR_ON;
		ttwakeup(tp);
	}
	return (1);
}

/*
 * Default modem control routine (for other line disciplines).
 * Return argument flag, to turn off device on carrier drop.
 */
nullmodem(tp, flag)
	register struct tty *tp;
	long flag;
{
	struct proc *p;
	LASSERT(TTY_LOCK_HOLDER(tp));
	if (flag)
		tp->t_state |= TS_CARR_ON;
	else {
		tp->t_state &= ~TS_CARR_ON;
		if ((tp->t_cflag & CLOCAL) == 0) {
                        if(tp->t_session) {
                                if(p = tp->t_session->s_leader) {
                                        if(P_REF(p)) {
                                                psignal_tty(p, SIGHUP);
                                                P_UNREF(p);
                                        }
                                        return (0);
                                } 
                        }
                }
        }
        return (1);
}

/*
 * reinput pending characters after state switch
 * call at spltty().
 */
ttypend(tp)
	register struct tty *tp;
{
	struct clist tq;
	register c;

	LASSERT(TTY_LOCK_HOLDER(tp));
	tp->t_lflag &= ~PENDIN;
	tp->t_state |= TS_TYPEN;
	tq = tp->t_rawq;
	tp->t_rawq.c_cc = 0;
	tp->t_rawq.c_cf = tp->t_rawq.c_cl = 0;
	while ((c = getc(&tq)) >= 0)
		ttyinput(c, tp);
	tp->t_state &= ~TS_TYPEN;
}

/*
 *
 * Place a character on raw TTY input queue,
 * putting in delimiters and waking up top
 * half as needed.  Also echo if required.
 * The arguments are the character and the
 * appropriate tty structure.
 */
ttyinput(c, tp)
	register long c;
	register struct tty *tp;
{
	register int iflag = tp->t_iflag;
	register int lflag = tp->t_lflag;
	register u_char *cc = tp->t_cc;
	int i, err;

	LASSERT(TTY_LOCK_HOLDER(tp));
	/*
	 * If input is pending take it first.
	 */
	if (lflag&PENDIN)
		ttypend(tp);
	/*
	 * Gather stats.
	 */
	tk_nin++;
	if (lflag&ICANON) {
		tk_cancc++;
		tp->t_cancc++;
	} else {
		tk_rawcc++;
		tp->t_rawcc++;
	}
	/*
	 * Handle exceptional conditions (break, parity, framing).
	 */
	if (err = (c&TTY_ERRORMASK)) {
		c &= ~TTY_ERRORMASK;
		if ((err&TTY_FE) && !c) {	/* break */
			if (iflag&IGNBRK)
				goto endcase;
                        else if (iflag&BRKINT) {
                                ttyflush(tp, FREAD|FWRITE);
                                pgsignal_tty(tp->t_pgrp, SIGINT, 1,0);
                                goto endcase;
                        }
			else {
				c = 0;
				if (iflag&PARMRK)
					goto parmrk;
			}
		} else if (((err&TTY_PE) && (iflag&INPCK)) || (err&TTY_FE)) {
			if (iflag&IGNPAR)
				goto endcase;
			else if (iflag&PARMRK) {
parmrk:
				putc(0377|TTY_QUOTE, &tp->t_rawq);
				putc(0|TTY_QUOTE, &tp->t_rawq);
				putc(c|TTY_QUOTE, &tp->t_rawq);
				goto endcase;
			} else
				c = 0;
		}
	}
	/*
	 * In tandem mode, check high water mark.
	 */
	if (iflag&IXOFF)
		ttyblock(tp);
	if ((tp->t_state&TS_TYPEN) == 0 && (iflag&ISTRIP))
		c &= 0177;
	else {
		if ((iflag & PARMRK) && !(iflag & ISTRIP) && c == 0377) {
			putc(0377|TTY_QUOTE, &tp->t_rawq);
			putc(0377|TTY_QUOTE, &tp->t_rawq);
			goto endcase;
		}
	}		
			
	/*
	 * Check for literal nexting very first
	 */
	if (tp->t_state&TS_LNCH) {
		c |= TTY_QUOTE;
		tp->t_state &= ~TS_LNCH;
	}
	/*
	 * Scan for special characters.  This code
	 * is really just a big case statement with
	 * non-constant cases.  The bottom of the
	 * case statement is labeled ``endcase'', so goto
	 * it after a case match, or similar.
	 */
	/*
	 * Control chars which aren't controlled
	 * by ICANON, ISIG, or IXON.
	 */
	if (lflag&IEXTEN) {
		if (CCEQ(cc[VLNEXT],c)) {
			if (lflag&ECHO) {
				if (lflag&ECHOE)
					ttyoutstr("^\b", tp);
				else
					ttyecho(c, tp);
			}
			tp->t_state |= TS_LNCH;
			goto endcase;
		}
		if (CCEQ(cc[VDISCARD],c)) {
			if (lflag&FLUSHO)
				tp->t_lflag &= ~FLUSHO;
			else {
				ttyflush(tp, FWRITE);
				ttyecho(c, tp);
				if (tp->t_rawq.c_cc + tp->t_canq.c_cc)
					ttyretype(tp);
				tp->t_lflag |= FLUSHO;
			}
			goto startoutput;
		}
	}
	/*
	 * Signals.
	 */
	if (lflag&ISIG) {
		if (CCEQ(cc[VINTR], c) || CCEQ(cc[VQUIT], c)) {
			if ((lflag&NOFLSH) == 0)
				ttyflush(tp, FREAD|FWRITE);
			ttyecho(c, tp);
			pgsignal_tty(tp->t_pgrp,
				CCEQ(cc[VINTR],c) ? SIGINT : SIGQUIT, 1,0);
			goto endcase;
		}		
		if (CCEQ(cc[VSUSP],c)) {
			if ((lflag&NOFLSH) == 0)
				ttyflush(tp, FREAD);
			ttyecho(c, tp);
			pgsignal_tty(tp->t_pgrp, SIGTSTP, 1,0);
			goto endcase;
		}
		if (CCEQ(cc[VSTATUS],c) && (lflag & IEXTEN)) {
			pgsignal_tty(tp->t_pgrp, SIGINFO, 1,0);
			if ((lflag & NOKERNINFO) == 0)
				ttyinfo(tp);
			goto endcase;
		}
	}
	/*
	 * Handle start/stop characters.
	 */
	if (iflag&IXON) {
		if (CCEQ(cc[VSTOP],c)) {
			if ((tp->t_state&TS_TTSTOP) == 0) {
				tp->t_state |= TS_TTSTOP;
				CDEVSW_STOP(major(tp->t_dev), tp, 0, err);
				return;
			}
			if (!CCEQ(cc[VSTART], c))
				return;
			/*
			 * if VSTART == VSTOP then toggle
			 */
			goto endcase;
		}
		if (CCEQ(cc[VSTART], c))
			goto restartoutput;
	}
	/*
	 * IGNCR, ICRNL, & INLCR
	 */
	if (c == '\r') {
		if (iflag&IGNCR)
			goto endcase;
		else if (iflag&ICRNL)
			c = '\n';
	}
	else if (c == '\n' && iflag&INLCR)
		c = '\r';

				/* Map Upper case to lower case */
	if (iflag&IUCLC && 'A' <= c && c <= 'Z')
	    c += 'a' - 'A';

	/*
	 * Non canonical mode; don't process line editing
	 * characters; check high water mark for wakeup.
	 *
	 */
	if (!(lflag&ICANON)) {
		if (tp->t_rawq.c_cc > tp->t_hog) {
			if (iflag&IMAXBEL) {
				if (tp->t_outq.c_cc < tp->t_hiwat)
					(void) ttyoutput(CTRL('g'), tp);
			} else
				ttyflush(tp, FREAD | FWRITE);
		} else {
			if (putc(c, &tp->t_rawq) >= 0) {
				ttyecho(c, tp);
				if (tp->t_rawq.c_cc >= tp->t_cc[VMIN])
				    ttwakeup(tp);
				else if (tp->t_shad_time > 0) {
 					if (tp->t_state & TS_INTIMEOUT)
						untimeout(ttin_timeout, tp);
					timeout(ttin_timeout, (caddr_t) tp,
						tp->t_shad_time);
					tp->t_state |= TS_INTIMEOUT;
				}
				
			}
		}
		goto endcase;
	}
	/*
	 * From here on down canonical mode character
	 * processing takes place.
	 */
	/*
	 * erase (^H / ^?)
	 */
	if (CCEQ(cc[VERASE], c)) {
		if (tp->t_rawq.c_cc)
			ttyrub(unputc(&tp->t_rawq), tp);
		goto endcase;
	}
	/*
	 * kill (^U)
	 */
	if (CCEQ(cc[VKILL], c)) {
		if (lflag&ECHOKE && tp->t_rawq.c_cc == tp->t_rocount &&
		    !(lflag&ECHOPRT)) {
			while (tp->t_rawq.c_cc)
				ttyrub(unputc(&tp->t_rawq), tp);
		} else {
			ttyecho(c, tp);
			if (lflag&ECHOK || lflag&ECHOKE)
				ttyecho('\n', tp);
			while (getc(&tp->t_rawq) > 0)
				;
			tp->t_rocount = 0;
		}
		tp->t_state &= ~TS_LOCAL;
		goto endcase;
	}
	/*
	 * word erase (^W)
	 */
	if (CCEQ(cc[VWERASE], c) && (lflag & IEXTEN)) {	
		int ctype;

#define CTYPE(c) ((lflag&ALTWERASE) ? (partab[(c)&TTY_CHARMASK]&0100) : 0)
		/*
		 * erase whitespace
		 */
		while ((c = unputc(&tp->t_rawq)) == ' ' || c == '\t')
			ttyrub(c, tp);
		if (c == -1)
			goto endcase;
		/*
		 * special case last char of token
		 */
		ttyrub(c, tp);
		c = unputc(&tp->t_rawq);
		if (c == -1 || c == ' ' || c == '\t') {
			if (c != -1)
				(void) putc(c, &tp->t_rawq);
			goto endcase;
		}
		/*
		 * erase rest of token
		 */
		ctype = CTYPE(c);
		do {
			ttyrub(c, tp);
			c = unputc(&tp->t_rawq);
			if (c == -1)
				goto endcase;
		} while (c != ' ' && c != '\t' && CTYPE(c) == ctype);
		(void) putc(c, &tp->t_rawq);
		goto endcase;
#undef CTYPE
	}
	/*
	 * reprint line (^R)
	 */
	if (CCEQ(cc[VREPRINT], c) && (lflag & IEXTEN)) {
		ttyretype(tp);
		goto endcase;
	}
	/*
	 * Check for input buffer overflow
	 */
	if (tp->t_rawq.c_cc+tp->t_canq.c_cc > tp->t_hog) {
		if (iflag&IMAXBEL) {
			if (tp->t_outq.c_cc < tp->t_hiwat)
				(void) ttyoutput(CTRL('g'), tp);
		} else
			ttyflush(tp, FREAD | FWRITE);
		goto endcase;
	}

	/*
	 * Put data char in q for user and
	 * wakeup on seeing a line delimiter.
	 */
	if (putc(c, &tp->t_rawq) >= 0) {
		if (ttbreakc(c)) {
			tp->t_rocount = 0;
			catq(&tp->t_rawq, &tp->t_canq);
			ttwakeup(tp);
		} else if (tp->t_rocount++ == 0)
			tp->t_rocol = tp->t_col;
		if (tp->t_state&TS_ERASE) {
			/*
			 * end of prterase \.../
			 */
			tp->t_state &= ~TS_ERASE;
			(void) ttyoutput('/', tp);
		}
		i = tp->t_col;
		ttyecho(c, tp);
		if (CCEQ(cc[VEOF], c) && lflag&ECHO) {
			/*
			 * Place the cursor over the '^' of the ^D.
			 */
			i = MIN(2, tp->t_col - i);
			while (i > 0) {
				(void) ttyoutput('\b', tp);
				i--;
			}
		}
	}
endcase:
	/*
	 * IXANY means allow any character to restart output.
	 */
	if ((tp->t_state&TS_TTSTOP) && !(iflag&IXANY)
	    && cc[VSTART] != cc[VSTOP])
		return;
restartoutput:
	tp->t_state &= ~TS_TTSTOP;
	tp->t_lflag &= ~FLUSHO;
startoutput:
	ttstart(tp);
}

/*
 * Put character on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * This is called both from the top half for output,
 * and from interrupt level for echoing.
 * The arguments are the character and the tty structure.
 * Returns < 0 if putc succeeds, otherwise returns char to resend
 * Must be recursive.
 */
ttyoutput(c, tp)
	register long c;
	register struct tty *tp;
{
	register char *colp;
	register ctype;
	register long oflag = tp->t_oflag;
	
	LASSERT(TTY_LOCK_HOLDER(tp));

	if (!(oflag&OPOST) || (c & TTY_QUOTE)) {
		if (tp->t_lflag&FLUSHO)
			return (-1);
		if (putc(c & TTY_CHARMASK, &tp->t_outq))
			return (c);
		tk_nout++;
		tp->t_outcc++;
		return (-1);
	}
	c &= TTY_CHARMASK;
	/*
	 * Turn tabs to spaces as required
	 */

	if (c == '\t' && ((oflag&OXTABS)|| ((oflag & TABDLY) == TAB3))) {
		TSPLVAR(s)

		c = 8 - (tp->t_col&7);
		if ((tp->t_lflag&FLUSHO) == 0) {
			TSPLTTY(s);		/* don't interrupt tabs */
			c -= b_to_q("        ", c, &tp->t_outq);
			tk_nout += c;
			tp->t_outcc += c;
			TSPLX(s);
		}
		tp->t_col += c;
		return (c ? -1 : '\t');
	}
	if (c == CEOT && (oflag&ONOEOT))
		return(-1);
	tk_nout++;
	tp->t_outcc++;
	/*
	 * Generate escapes for upper-case-only terminals.
	 */
	if (tp->t_lflag&XCASE) {
		colp = "({)}!|^~'`\\\\";
		while(*colp++)
		    if (c == *colp++) {
			    ttyoutput('\\'|TTY_QUOTE, tp);
			    c = colp[-2];
			    break;
		    }
		if ('A' <= c && c <= 'Z')
		    ttyoutput('\\' | TTY_QUOTE, tp);
	}

	if (oflag&OLCUC && 'a' <= c && c <= 'z')
	    c += 'A' - 'a';
	
	/*
	 * turn <nl> to <cr><lf> if desired.
	 */
	if (c == '\n' && (oflag&ONLCR) && ttyoutput('\r', tp) >= 0)
		return (c);

	if (c == '\r' && (oflag&ONOCR) && tp->t_col == 0)
		return(-1);
	if (c == '\r' && (oflag&OCRNL)) {
		c = '\n';
		if ((tp->t_lflag&FLUSHO) == 0 && putc(c, &tp->t_outq))
			return ('\r');
	}
	else {
		if ((tp->t_lflag&FLUSHO) == 0 && putc(c, &tp->t_outq))
			return (c);

		if (c == '\n' && (oflag & ONLRET)) {
			c = '\r';
		}
	}
	    
	/*
	 * Calculate delays.
	 * The numbers here represent clock ticks
	 * and are not necessarily optimal for all terminals.
	 *
	 * SHOULD JUST ALLOW USER TO SPECIFY DELAYS
	 *
	 * (actually, should THROW AWAY terminals which need delays)
	 */
	colp = &tp->t_col;
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {

	case ORDINARY:
		(*colp)++;

	case CONTROL:
		break;


	/*
	 * This macro is close enough to the correct thing;
	 * it should be replaced by real user settable delays
	 * in any event...
	 */
#define mstohz(ms)	((((ms) * hz) >> 10) & 0X0FF)
	case BACKSPACE:
		if (oflag & BSDLY)
			if (oflag & OFILL)
				c = 1;
			else
				 c = mstohz(100);
			
		if (*colp)
			(*colp)--;
		break;
	case NEWLINE:
		ctype = oflag & NLDLY;
		if (ctype == NL2) { /* tty 37 */
			if (*colp > 0) {
				c = (((unsigned)*colp) >> 4) + 3;
				if ((unsigned)c > 6)
					c = mstohz(60);
				else
					c = mstohz(c * 10);
			}
		}
		else if (ctype == NL1) /* vt05 */
			if (oflag & OFILL)
				c = 2;
			else
				c = mstohz(100);
		/* *colp = 0; */
		break;

	case TAB:
		ctype = oflag & TABDLY;
		if (ctype == TAB1) { /* tty 37 */
			c = 1 - (*colp | ~07);
			if (c < 5)
				c = 0;
			else
			    c = mstohz(10 * c);
			
		}
		else if (ctype == TAB2)
			c = mstohz(200);
				/* TAB3 is handled earlier. */
		else
			c = 0;
		if (ctype && (oflag & OFILL))
		    c = 2;
		
		*colp |= 07;
		(*colp)++;
		break;

	case VTAB:
		if (oflag&VTDELAY) /* tty 37 */
			c =  177;
		break;

	case RETURN:
		ctype = oflag & CRDLY;
		if (ctype == CR2) /* tn 300 */
			if (oflag & OFILL)
				c = 2;
			else
				c = mstohz(100);
		else if (ctype == CR3) /* ti 700 */
			c = mstohz(166);
		else if (ctype == CR1) { /* concept 100 */
			    
			if (oflag & OFILL)
				c = 4;
			else {
				c = (*colp >>4) + 3;
				c = c < 6 ? 6 : c;
				c = mstohz(c * 10);
			}
		    
			
		}
		*colp = 0;
		break;
		
	case FF:
		if (oflag & FFDLY)
			c = 0177;
		break;
			
	}
	
	if (c && (tp->t_lflag&FLUSHO) == 0) {
		if (oflag & OFILL) {
			ctype = oflag & OFDEL ? '\177' : '\0';
			for(;c > 0;c--)
				(void)putc(ctype, &tp->t_outq);
		}
		else 
			(void) putc(c|TTY_QUOTE, &tp->t_outq);
	}
	return (-1);
}
	
#undef mstohz

struct{
	char from;
	char to;
	}xcase_map[] = 
{
	{'\'', '`'},
	{'!', '|'},
	{'^', '~'},    
	{'(', '{'},    
	{')', '}'},
	{'\\', '\\'},
	{'\0', '\0'}
	 
};


/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp, uio, flag)
	register struct tty *tp;
	struct uio *uio;
	long flag;
{
	register struct clist *qp;
	register int c;
	register long lflag;
	register struct proc *p;
	register u_char *cc = tp->t_cc;
	int first, error = 0;
	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	first = 1;
	
loop:
	lflag = tp->t_lflag;
	TSPLTTY(s);
	/*
	 * take pending input first
	 */
	if (lflag&PENDIN)
		ttypend(tp);
	/*
	 * Handle carrier.
	 */
	if (!(tp->t_state&TS_CARR_ON) && !(tp->t_cflag&CLOCAL)) {
		if (tp->t_state&TS_ISOPEN) {
			TSPLX(s);
			return (0);	/* EOF */
		} else if (flag & (IO_NDELAY|IO_NONBLOCK)) {
			TSPLX(s);
			return (EWOULDBLOCK);
		} else {
			/*
			 * sleep awaiting carrier
			 */
			error = ttysleep(tp, (caddr_t)&tp->t_rawq, 
				TTIPRI | PCATCH, ttyin);
			TSPLX(s);
			if (error)
				return (error);
			goto loop;
		}
	}
	TSPLX(s);
	/*
	 * Hang process if it's in the background.
	 */
        p = u.u_procp;
        if(ISBACKGROUND(p, tp)) {
                register struct pgrp    *pg;
                         sigset_t       p_sigmask;

                PROC_LOCK(p);
                pg = p->p_pgrp;
                (void)PG_REF(pg);
                PROC_UNLOCK(p);

                p_sigmask = p->p_sigmask;
                if (sigismember(&p->p_sigignore, SIGTTIN) ||
                    sigismember(&p_sigmask, SIGTTIN)      ||
                   pg->pg_jobc == 0) {
                        PG_UNREF(pg);
                        return (EIO);
                }

                pgsignal_tty(pg, SIGTTIN, 1, 1);
                PG_UNREF(pg);
                if(error = ttysleep(tp, (caddr_t)&lbolt,
                                    TTIPRI | PCATCH, ttybg))
                        return (error);
                goto loop;
        }
	/*
	 * If canonical, use the canonical queue,
	 * else use the raw queue.
	 *
	 * XXX - should get rid of canonical queue.
	 * (actually, should get rid of clists...)
	 */
	if (lflag&ICANON) {
		qp = &tp->t_canq;
		
		TSPLTTY(s);
		if (qp->c_cc <= 0) {
			/** XXX ??? ask mike why TS_CARR_ON was (once) necessary here
			  if ((tp->t_state&TS_CARR_ON) == 0 ||
			  (tp->t_state&TS_NBIO)) {
			  TSPLX(s);
			  return (EWOULDBLOCK);
			  }
			  **/
			if (flag & (IO_NDELAY|IO_NONBLOCK)) {
				TSPLX(s);
				return (EWOULDBLOCK);
			}
			error = ttysleep(tp, (caddr_t)&tp->t_rawq, 
				TTIPRI | PCATCH, ttyin);
			TSPLX(s);
			if (error)
				return (error);
			goto loop;
		}
		TSPLX(s);
	}
	else {
		qp = &tp->t_rawq;
		TSPLTTY(s);
		if (qp->c_cc < tp->t_cc[VMIN] || qp->c_cc == 0) {
			/** XXX ??? ask mike why TS_CARR_ON was (once) necessary here
			  if ((tp->t_state&TS_CARR_ON) == 0 ||
			  (tp->t_state&TS_NBIO)) {
			  TSPLX(s);
			  return (EWOULDBLOCK);
			  }
			  **/
			if (flag & (IO_NDELAY|IO_NONBLOCK)) {
				TSPLX(s);
				return (EWOULDBLOCK);
			}
			if (tp->t_cc[VMIN] == 0 &&
			    (tp->t_shad_time == 0 || !first)) {
				/*
				 * If there are no chars and the caller
				 * doesn't want to wait or has waited once
				 * then return
				 */
				TSPLX(s);
				return(error);
			}
			if (tp->t_shad_time > 0 && !(tp->t_state&TS_INTIMEOUT) 
			    && (qp->c_cc > 0 || tp->t_cc[VMIN] == 0)) {
				first = 0;
				timeout(ttin_timeout, tp, tp->t_shad_time);
				tp->t_state |= TS_INTIMEOUT;
			}
			if (error = ttysleep (tp, (caddr_t)&tp->t_rawq, 
					      TTIPRI | PCATCH, ttyin)) {
				TSPLX(s);
				return (error);
			}
			if (qp->c_cc <= 0) {
				/* If there are no characters wait again. */
				TSPLX(s);
				goto loop;
			}
		}

		TSPLX(s);
	}		
	/*
	 * Input present, check for input mapping and processing.
	 */
	first = 1;
	while ((c = getc(qp)) >= 0) {
		/*
		 * delayed suspend (^Y)
		 */
		if (CCEQ(cc[VDSUSP], c) && (lflag&ISIG) &&
		    (lflag & IEXTEN)) {
			pgsignal_tty(tp->t_pgrp, SIGTSTP, 1,1);
			if (first) {
				if (error = ttysleep(tp, (caddr_t)&lbolt, 
					TTIPRI | PCATCH, ttybg))
					break;
				goto loop;
			}
			break;
		}
		/*
		 * Interpret EOF only in canonical mode.
		 */
		if (CCEQ(cc[VEOF], c) && lflag&ICANON)
			break;
			
		/*
		 * Connical upper/lower presentation
		 */
		if((lflag&ICANON) && (lflag&XCASE)) {
			int next_char;	
			/*
			 * If this character is a quote and
			 * if the next character is one we want to
			 * remap then discard the \\ and send new
			 * remap character otherwise send the \\
			 */
			if ((c == '\\') && (qp->c_cc > 0)) {
				next_char = *qp->c_cf;
				if ( next_char >= 'a' && next_char <= 'z') {
					(void) getc(qp);
					c = next_char - 'a' + 'A';
				}
				else {
					int cnt;
					cnt = 0;
					while(xcase_map[cnt].from) {
						if (xcase_map[cnt].from == next_char) {
							(void) getc(qp);
							c = xcase_map[cnt].to;
							break;
						}
						cnt++;
					}
				}
			}
		}
		
		/*
		 * Give user character.
		 */
 		error = ureadc(c , uio);
		if (error)
			break;
 		if (uio->uio_resid == 0)
			break;
		/*
		 * In canonical mode check for a "break character"
		 * marking the end of a "line of input".
		 */
		if ((lflag&ICANON) && ttbreakc(c)) {
			break;
		}
		first = 0;
	}
	/*
	 * Look to unblock output now that (presumably)
	 * the input queue has gone down.
	 */
	if (tp->t_state&TS_TBLOCK && tp->t_rawq.c_cc < tp->t_hog/5) {
		if (cc[VSTART] != _POSIX_VDISABLE
		   && ((tp->t_state & TS_NOFLOWCHARS) || putc(cc[VSTART], &tp->t_outq) == 0)) {
			TSPLTTY(s);
			tp->t_state &= ~TS_TBLOCK;
			TSPLX(s);
			ttstart(tp);
		}
	}
	return (error);
}

/*
 * Check the output queue on tp for space for a kernel message
 * (from uprintf/tprintf).  Allow some space over the normal
 * hiwater mark so we don't lose messages due to normal flow
 * control, but don't let the tty run amok.
 * Sleeps here are not interruptible, but we return prematurely
 * if new signals come in.
 */
ttycheckoutq(tp, wait)
	register struct tty *tp;
	long wait;
{
	int hiwat, oldsig;
	extern int wakeup();
	TSPLVAR(s)

	TTY_LOCK(tp);
	hiwat = tp->t_hiwat;
	TSPLTTY(s);
	oldsig = u.u_procp->p_sig;
	if (tp->t_outq.c_cc > hiwat + 200)
		while (tp->t_outq.c_cc > hiwat) {
			ttstart(tp);
			if (wait == 0 || u.u_procp->p_sig != oldsig) {
				TSPLX(s);
				TTY_UNLOCK(tp);
				return (0);
			}
			tp->t_state |= TS_ASLEEP;
#if	UNIX_LOCKS && ((NCPUS > 1) )
			(void) mpsleep((caddr_t)&tp->t_outq, PZERO-1, ttyout, 
					hz, &tp->t_lock, 
					MS_LOCK_WRITE|MS_LOCK_ON_ERROR);
#else
			(void) mpsleep((caddr_t)&tp->t_outq, PZERO-1, ttyout, 
					hz, (void *)NULL, 0);
#endif
		}
	TSPLX(s);
	TTY_UNLOCK(tp);
	return (1);
}

/*
 * Called from the device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(tp, uio, flag)
	register struct tty *tp;
	register struct uio *uio;
	long flag;
{
	register char *cp;
	register int cc = 0, ce;
	register struct proc *p;
	int i, hiwat, cnt, error, s;
	char obuf[OBUFSIZ];

	LASSERT(TTY_LOCK_HOLDER(tp));
	hiwat = tp->t_hiwat;
	cnt = uio->uio_resid;
	error = 0;

loop:
	TSPLTTY(s);
	if (!(tp->t_state&TS_CARR_ON) && !(tp->t_cflag&CLOCAL)) {
		if (tp->t_state&TS_ISOPEN) {
			TSPLX(s);
			return (EIO);
		} else if (flag & (IO_NDELAY|IO_NONBLOCK)) {
			TSPLX(s);
			error = EWOULDBLOCK;
			goto out;
		} else {
			/*
			 * sleep awaiting carrier
			 */
			error = ttysleep(tp, (caddr_t)&tp->t_rawq, 
					TTIPRI | PCATCH, ttopen);
			TSPLX(s);
			if (error)
				goto out;
			goto loop;
		}
	}
	TSPLX(s);
	/*
	 * Hang the process if it's in the background.
	 */
        p = u.u_procp;
        if (ISBACKGROUND(p, tp)) {
                register struct pgrp    *pg;
                sigset_t                p_sigmask;

                PROC_LOCK(p);
                pg = p->p_pgrp;
                (void)PG_REF(pg);
                PROC_UNLOCK(p);
                p_sigmask = p->p_sigmask;
                if ((tp->t_lflag & TOSTOP) &&
                   !sigismember(&p->p_sigignore, SIGTTOU) &&
                   !sigismember(&p_sigmask, SIGTTOU)) {
                        if(pg->pg_jobc == 0) {
                                PG_UNREF(pg);
                                return (EIO);
                        }

                        pgsignal_tty(pg, SIGTTOU, 1, 1);
                        PG_UNREF(pg);
                        if (error = ttysleep(tp, (caddr_t)&lbolt,
                                                     TTIPRI | PCATCH, ttybg))
                                goto out;
                        goto loop;
                }
                PG_UNREF(pg);
        }
	/*
	 * Process the user's data in at most OBUFSIZ
	 * chunks.  Perform any output translation.
	 * Keep track of high water mark, sleep on overflow 
	 * awaiting device aid in acquiring new space.
	 */
	while (uio->uio_resid > 0 || cc > 0) {
		if (tp->t_lflag&FLUSHO) {
			uio->uio_resid = 0;
			return (0);
		}
		if (tp->t_outq.c_cc > hiwat)
			goto ovhiwat;
		/*
		 * Grab a hunk of data from the user.
		 * unless we have some left over from last time.
		 */
		if (cc == 0) {
			cc = min(uio->uio_resid, OBUFSIZ);
			cp = obuf;
			error = uiomove(cp, cc, uio);
			if (error) {
				cc = 0;
				break;
			}
		}
		/*
		 * If nothing fancy need be done, grab those characters we
		 * can handle without any of ttyoutput's processing and
		 * just transfer them to the output q.  For those chars
		 * which require special processing (as indicated by the
		 * bits in partab), call ttyoutput.  After processing
		 * a hunk of data, look for FLUSHO so ^O's will take effect
		 * immediately.
		 */
		while (cc > 0) {
			if (!(tp->t_oflag&OPOST))
				ce = cc;
			else {
				if ((tp->t_oflag & OLCUC) ||
				    (tp->t_lflag & XCASE))
					/* 
					 * Process all the characters 
					 * one by one 
					 */
					ce = 0;
				else
					ce = cc - scanc((unsigned)cc,
							(u_char *)cp,
							(u_char *)partab, 077);
				/*
				 * If ce is zero, then we're processing
				 * a special character through ttyoutput.
				 */
				if (ce == 0) {
					tp->t_rocount = 0;
					if (ttyoutput(*cp, tp) >= 0) {
					    /* no c-lists, wait a bit */
					    ttstart(tp);
					    if (error = ttysleep(tp, 
						(caddr_t)&lbolt,
						 TTOPRI | PCATCH, ttybuf))
						    break;
					    goto loop;
					}
					cp++, cc--;
					if ((tp->t_lflag&FLUSHO) ||
					    tp->t_outq.c_cc > hiwat)
						goto ovhiwat;
					continue;
				}
			}
			/*
			 * A bunch of normal characters have been found,
			 * transfer them en masse to the output queue and
			 * continue processing at the top of the loop.
			 * If there are any further characters in this
			 * <= OBUFSIZ chunk, the first should be a character
			 * requiring special handling by ttyoutput.
			 */
			tp->t_rocount = 0;
			i = b_to_q(cp, ce, &tp->t_outq);
			ce -= i;
			tp->t_col += ce;
			cp += ce, cc -= ce, tk_nout += ce;
			tp->t_outcc += ce;
			if (i > 0) {
				/* out of c-lists, wait a bit */
				ttstart(tp);
				if (error = ttysleep(tp, (caddr_t)&lbolt,
					    TTOPRI | PCATCH, ttybuf))
					break;
				goto loop;
			}
			if (tp->t_lflag&FLUSHO || tp->t_outq.c_cc > hiwat)
				break;
		}
		ttstart(tp);
	}
out:
	/*
	 * If cc is nonzero, we leave the uio structure inconsistent,
	 * as the offset and iov pointers have moved forward,
	 * but it doesn't matter (the call will either return short
	 * or restart with a new uio).
	 */
	uio->uio_resid += cc;
	return (error);

ovhiwat:
	ttstart(tp);
	TSPLTTY(s);
	/*
	 * This can only occur if FLUSHO is set in t_lflag,
	 * or if ttstart/oproc is synchronous (or very fast).
	 */
	if (tp->t_outq.c_cc <= hiwat) {
		TSPLX(s);
		goto loop;
	}
	if (flag & (IO_NDELAY|IO_NONBLOCK)) {
		TSPLX(s);
		uio->uio_resid += cc;
		if (uio->uio_resid == cnt)
			return (EWOULDBLOCK);
		return (0);
	}
	tp->t_state |= TS_ASLEEP;
	error =  ttysleep(tp, &tp->t_outq, TTOPRI | PCATCH, ttyout);
	TSPLX(s);
	if (error)
		goto out;
	goto loop;
}

/*
 * Rubout one character from the rawq of tp
 * as cleanly as possible.
 */
ttyrub(c, tp)
	register long c;
	register struct tty *tp;
{
	register char *cp;
	register int savecol;
	char *nextc();
	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	if ((tp->t_lflag&ECHO) == 0)
		return;
	tp->t_lflag &= ~FLUSHO;	
	if (tp->t_lflag&ECHOE) {
		if (tp->t_rocount == 0) {
			/*
			 * Screwed by ttwrite; retype
			 */
			ttyretype(tp);
			return;
		}
		if (c == ('\t'|TTY_QUOTE) || c == ('\n'|TTY_QUOTE))
			ttyrubo(tp, 2);
		else switch (partab[c&=0377]&077) {

		case ORDINARY:
			ttyrubo(tp, 1);
			break;

		case VTAB:
		case BACKSPACE:
		case CONTROL:
		case RETURN:
		case NEWLINE:
		case FF:
			if (tp->t_lflag&ECHOCTL)
				ttyrubo(tp, 2);
			break;

		case TAB: {
			int c;

			if (tp->t_rocount < tp->t_rawq.c_cc) {
				ttyretype(tp);
				return;
			}
			TSPLTTY(s);
			savecol = tp->t_col;
			tp->t_state |= TS_CNTTB;
			tp->t_lflag |= FLUSHO;
			tp->t_col = tp->t_rocol;
			cp = tp->t_rawq.c_cf;
			if (cp)
				c = *cp;	/* XXX FIX NEXTC */
			for (; cp; cp = nextc(&tp->t_rawq, cp, &c))
				ttyecho(c, tp);
			tp->t_lflag &= ~FLUSHO;
			tp->t_state &= ~TS_CNTTB;
			TSPLX(s);
			/*
			 * savecol will now be length of the tab
			 */
			savecol -= tp->t_col;
			tp->t_col += savecol;
			if (savecol > 8)
				savecol = 8;		/* overflow screw */
			while (--savecol >= 0)
				(void) ttyoutput('\b', tp);
			break;
		}

		default:
			/* XXX */
			printf("ttyrub: would panic c = %d, val = %d\n",
				c, partab[c&=0377]&077);
			/*panic("ttyrub");*/
		}
	} else if (tp->t_lflag&ECHOPRT) {
		if ((tp->t_state&TS_ERASE) == 0) {
			(void) ttyoutput('\\', tp);
			tp->t_state |= TS_ERASE;
		}
		ttyecho(c, tp);
	} else
		ttyecho(tp->t_cc[VERASE], tp);
	tp->t_rocount--;
}

/*
 * Crt back over cnt chars perhaps
 * erasing them.
 */
ttyrubo(tp, cnt)
	register struct tty *tp;
	long cnt;
{

	LASSERT(TTY_LOCK_HOLDER(tp));
	while (--cnt >= 0)
		ttyoutstr("\b \b", tp);
}

/*
 * Reprint the rawq line.
 * We assume c_cc has already been checked.
 */
ttyretype(tp)
	register struct tty *tp;
{
	register char *cp;
	char *nextc();
	int c;
	TSPLVAR(s)

	LASSERT(TTY_LOCK_HOLDER(tp));
	if (tp->t_cc[VREPRINT] != _POSIX_VDISABLE)
		ttyecho(tp->t_cc[VREPRINT], tp);
	(void) ttyoutput('\n', tp);
	TSPLTTY(s);
	/*** XXX *** FIX *** NEXTC IS BROKEN - DOESN'T CHECK QUOTE
	  BIT OF FIRST CHAR ****/
	for (cp = tp->t_canq.c_cf, c=(cp?*cp:0); cp; cp = nextc(&tp->t_canq, cp, &c)) {
		ttyecho(c, tp);
	}
	for (cp = tp->t_rawq.c_cf, c=(cp?*cp:0); cp; cp = nextc(&tp->t_rawq, cp, &c)) {
		ttyecho(c, tp);
	}
	tp->t_state &= ~TS_ERASE;
	TSPLX(s);
	tp->t_rocount = tp->t_rawq.c_cc;
	tp->t_rocol = 0;
}

/*
 * Echo a typed character to the terminal.
 */
ttyecho(c, tp)
	register long c;
	register struct tty *tp;
{
	LASSERT(TTY_LOCK_HOLDER(tp));
	if ((tp->t_state&TS_CNTTB) == 0)
		tp->t_lflag &= ~FLUSHO;
	if ((tp->t_lflag&ECHO) == 0 && !(tp->t_lflag&ECHONL && c == '\n'))
		return;
	if (tp->t_lflag&ECHOCTL) {
		if ((c&TTY_CHARMASK)<=037 && c!='\t' && c!='\n' || c==0177) {
			(void) ttyoutput('^', tp);
			c &= TTY_CHARMASK;
			if (c == 0177)
				c = '?';
			else
				c += 'A' - 1;
		}
	}
	
        /*
	 * If we are doing XCASE processing then send '\\' to ttyoutput
	 * quoted so it won't be double echoed.
         */
	if ((tp->t_lflag & XCASE) && c == '\\') {
		(void) ttyoutput(c | TTY_QUOTE, tp);
		return;
	}
		    
	/*
	 * Do not echo non-printing control characters.  Mainly
	 * to stop causing special actions on most terminals.
	 */
#if 0
	/*
	 * This code prevents the driver from echoing 8 bit characters
	 * which are legal in other languages such as latin-1.  If this
	 * causes problems it will have to be made another flag.
	 */
	c &= 0177;
	if ((040 <= c && c <= 0176) || (07 <= c && c <= 012) || c == 015)
#endif
	    (void) ttyoutput(c, tp);
}

/*
 * send string cp to tp
 */
ttyoutstr(cp, tp)
	register char *cp;
	register struct tty *tp;
{
	register char c;

	LASSERT(TTY_LOCK_HOLDER(tp));
	while (c = *cp++)
		(void) ttyoutput(c, tp);
}

ttwakeup(tp)
	struct tty *tp;
{
	LASSERT(TTY_LOCK_HOLDER(tp));
	
	if (tp->t_state & TS_INTIMEOUT) {
		untimeout(ttin_timeout, tp);
		tp->t_state &= ~TS_INTIMEOUT;
	}
	
	select_wakeup(&tp->t_selq);
	if (tp->t_state & TS_ASYNC) {
		pgsignal_tty(tp->t_pgrp, SIGIO, 1,0);
	}
	thread_wakeup((vm_offset_t)&tp->t_rawq);
}

/*
 * set tty hi and low water marks
 *
 * Try to arrange the dynamics so there's about one second
 * from hi to low water.
 *
 */
ttsetwater(tp)
	struct tty *tp;
{
	register cps = tp->t_ospeed / 10;
	register x;

#define clamp(x, h, l) ((x)>h ? h : ((x)<l) ? l : (x))
	tp->t_lowat = x = clamp(cps/2, TTMAXLOWAT, TTMINLOWAT);
	x += cps;
	x = clamp(x, TTMAXHIWAT, TTMINHIWAT);
	tp->t_hiwat = roundup(x, CBSIZE);
#undef clamp
}


int ttyhostname = 0;
/*
 * (^T)
 * Report on state of foreground process group.
 * This routine assumes that the utask and thread strcutures  are not paged.
 */
/* Look closely at this one -- XXX -- gmf */
ttyinfo(tp)
	struct tty *tp;
{
	register struct proc *p, *pick = NULL;
	register struct utask *ut;
	register thread_t th;
	register char *cp = hostname;
	pid_t pid;
	int x, s;
	time_value_t utime, stime;
	char ucomm[MAXCOMLEN+1];
	char *outstring;
#define	pgtok(a)	(((a)*NBPG)/1024)

	/*
	 * Allow some space over the normal hiwater mark so
	 * we don't lose messages due to normal flow
	 * control, but don't let the tty run amok.
	 */
	if (tp->t_outq.c_cc > tp->t_hiwat + 200)
		return;
	/* 
	 * hostname.  Not locked, but the worst that can
	 * happen is that we print a garbled hostname.
	 * Doesn't seem worth it.
	 */
	if (ttyhostname) {
		if (*cp == '\0')
			ttyoutstr("amnesia", tp);
		else
			while (*cp && *cp != '.')
				tputchar(*cp++, tp);
		tputchar(' ', tp);
	}
	/* 
	 * load average 
	 */
	x = (avenrun[0] * 100 + FSCALE/2) >> FSHIFT;
	ttyoutstr("load: ", tp);
	ttyoutint(x/100, 10, 1, tp);
	tputchar('.', tp);
	ttyoutint(x%100, 10, 2, tp);
	if (tp->t_session == NULL)
		ttyoutstr(" not a controlling terminal\n", tp);
	else if (tp->t_pgrp == NULL)
		ttyoutstr(" no foreground process group\n", tp);
        PGRP_READ_LOCK(tp->t_pgrp);
        if((p = tp->t_pgrp->pg_mem) == NULL) {
                ttyoutstr(" empty foreground process group\n", tp);
                PGRP_UNLOCK(tp->t_pgrp);
                return;
        }

        /* pick interesting process */
        for(pick = NULL; p != NULL; p = p->p_pgrpnxt) {
                if (proc_compare(pick, p)) {
                        if (P_REF(p)) {
                                if (pick)
                                        P_UNREF(pick);
                                pick = p;
                        }
                }
        }
        PGRP_UNLOCK(tp->t_pgrp);

        if (pick == NULL) {
                ttyoutstr(" no information on current process\n", tp);
                return;
        }
        pid = pick->p_pid;
        task_lock(pick->task);
        th = (thread_t)queue_first(&pick->task->thread_list);

        if (th != THREAD_NULL) {
                /* in-line thread_reference */
                s = splsched();
                thread_lock(th);
                th->ref_count++;
                outstring = (th->wait_mesg ? th->wait_mesg : "running");
                thread_unlock(th);
                splx(s);
        }
        task_unlock(pick->task);
        ut = pick->utask;
        if (pid <= 0 || th == NULL || ut == NULL) {
                ttyoutstr(" no information on current process\n", tp);
                thread_deallocate(th);  /* lose reference, if acquired */
                P_UNREF(pick);
                return;
        }

        usimple_lock(&ut->uu_handy_lock);
        bcopy(ut->uu_comm, ucomm, MAXCOMLEN+1);
        usimple_unlock(&ut->uu_handy_lock);
        ttyoutstr("  cmd: ", tp);
        ttyoutstr(ucomm, tp);
        tputchar(' ', tp);
        ttyoutint(pid, 10, 1, tp);
        ttyoutstr(" [", tp);
        ttyoutstr(outstring, tp);
        ttyoutstr("] ", tp);

        /*
         * cpu time
         */
        if (u.u_procp == pick)
                s = splclock();
        thread_read_times(th, &utime, &stime);
        if (u.u_procp == pick)
                splx(s);
        /* user time */
        x = utime.microseconds / 10000; /* scale to 100's */
        ttyoutint(utime.seconds, 10, 1, tp);
        tputchar('.', tp);
        ttyoutint(x, 10, 2, tp);
        tputchar('u', tp);
        tputchar(' ', tp);
        /* system time */
        x = stime.microseconds / 10000; /* scale to 100's */
        ttyoutint(stime.seconds, 10, 1, tp);
        tputchar('.', tp);
        ttyoutint(x, 10, 2, tp);
        tputchar('s', tp);
        tputchar(' ', tp);
        /*
         * pctcpu
         */
        x = th->cpu_usage / (TIMER_RATE/TH_USAGE_SCALE);
        x = (x * 3) / 5;
#if     SIMPLE_CLOCK
        /*
         *      Clock drift compensation.
         */
        x = (x * 1000000)/sched_usec;
#endif  /*SIMPLE_CLOCK*/
        ttyoutint(x/100, 10, 1, tp);
        ttyoutstr("% ",tp);
        /*
         * RSS
         */
        if (pick->task && pick->task->map) {
                vm_map_t map = pick->task->map;
                ttyoutint(pgtok(map->vm_pmap->stats.resident_count), 10, 1, tp);
                ttyoutstr("k\n", tp);
        }
        thread_deallocate(th);
        P_UNREF(pick);

}

ttyoutint(n, base, min, tp)
	register long n, base, min;
	register struct tty *tp;
{
	char info[16];
	register char *p = info;

	while (--min >= 0 || n) {
		*p++ = "0123456789abcdef"[n%base];
		n /= base;
	}
	while (p > info)
		ttyoutput(*--p, tp);
}

/*
 * Returns 1 if p2 is "better" than p1
 *
 * The algorithm for picking the "interesting" process is thus:
 *
 *	1) (Only foreground processes are eligable - implied)
 *	2) Runnable processes are favored over anything
 *	   else.  The runner with the highest cpu
 *	   utilization is picked (p_cpu).  Ties are
 *	   broken by picking the highest pid.
 *	3  Next, the sleeper with the shortest sleep
 *	   time is favored.  With ties, we pick out
 *	   just "short-term" sleepers (thread->interruptible == 0).
 *	   Further ties are broken by picking the highest
 *	   pid.
 *
 */
#define isrun(th)	(((th)->state & TH_RUN) == TH_RUN)
#define TESTAB(a, b)    ((a)<<1 | (b))
#define ONLYA   2
#define ONLYB   1
#define BOTH    3

proc_compare(p1, p2)
	register struct proc *p1, *p2;
{
	thread_t th1, th2;
	pid_t pid1, pid2;
	long slptime1, slptime2;
	
	if (p1 == NULL)
		return (1);

	if ((th1 = p1->thread) == NULL || (pid1 = p1->p_pid) <= 0)
		return(1);	/* The process has exited. */
	thread_reference(th1);

	if ((th2 = p2->thread) == NULL || (pid2 = p2->p_pid) <= 0) {
		thread_deallocate(th1);
		return(0);	/* The process has exited. */
	}
	thread_reference(th2);

#undef RETURN
#define RETURN(x) thread_deallocate(th1); \
	thread_deallocate(th2); \
	    return(x);
	
	/*
	 * see if at least one of them is runnable
	 */
	switch (TESTAB(isrun(th1), isrun(th2))) {
	case ONLYA:
		RETURN (1);
	case ONLYB:
		RETURN (0);
	case BOTH:
		/*
		 * tie - favor one with highest recent cpu utilization
		 */
		if (th2->cpu_usage > th1->cpu_usage) {
			RETURN (1);
		}
		
		if (th1->cpu_usage > th2->cpu_usage) {
			RETURN (0);
		}
		
		RETURN (pid2 > pid1);	/* tie - return highest pid */
	}
	/* 
	 * pick the one with the smallest sleep time
	 */
	slptime1 = sched_tick - th1->sleep_stamp;
	slptime2 = sched_tick - th2->sleep_stamp;
	if (slptime2 > slptime1) {
		RETURN (0);
	}
	
	if (slptime1 > slptime2) {
		RETURN (1);
	}
	
	/*
	 * favor one sleeping in a non-interruptible sleep
	 */
	if (th1->interruptible && (th2->interruptible) == 0) {
		RETURN (1);
	}
	
	if (th2->interruptible && (th1->interruptible) == 0) {
		RETURN (0);
	}
	
	RETURN(pid2 > pid1);		/* tie - return highest pid */

#undef RETURN	
}

/*
 * Output char to tty; console putchar style.
 */
tputchar(c, tp)
	long c;
	struct tty *tp;
{
	TSPLVAR(s)

	TSPLTTY(s);
	if ((tp->t_state & (TS_CARR_ON | TS_ISOPEN))
	    == (TS_CARR_ON | TS_ISOPEN)) {
		if (c == '\n')
			(void) ttyoutput('\r', tp);
		(void) ttyoutput(c, tp);
		ttstart(tp);
		TSPLX(s);
		return (0);
	}
	TSPLX(s);
	return (-1);
}

ttin_timeout(tp)
register struct tty *tp;
{
	
/*
 * This function performs a wakeup on the rawq.  It is used for
 * inter-character timings, and is called by timeout.
 */
	TSPLVAR(ipl)

	TSPLTTY(ipl);
	TTY_LOCK(tp);
	if (tp == 0)
		panic("ttin_timeout");
	tp->t_state &= ~TS_INTIMEOUT;
	ttwakeup(tp);
	TTY_UNLOCK(tp);
	TSPLX(ipl);
}

/*
 * Takes a locked struct tty.
 * If no errors, returns it locked; otherwise it's unlocked.
 */
ttysleep(tp, chan, pri, wmesg)
	register struct tty *tp;
	caddr_t chan;
	long pri;
	char *wmesg;
{
	int error;
	short gen = tp->t_gen;

	LASSERT(TTY_LOCK_HOLDER(tp));
#if	UNIX_LOCKS && ((NCPUS > 1) )
	if (error = mpsleep(chan, pri, wmesg, 0, 
			    &tp->t_lock, MS_LOCK_WRITE|MS_LOCK_ON_ERROR))
#else
	if (error = tsleep(chan, pri, wmesg, 0))
#endif
		return (error);
	if (tp->t_gen != gen) {
		return (ERESTART);
	}
	return (0);
}
ttysleep_tmo(tp, chan, pri, wmesg,tmo)
	register struct tty *tp;
	caddr_t chan;
	long pri;
	char *wmesg;
	int tmo;
{
	int error;
	short gen = tp->t_gen;

	LASSERT(TTY_LOCK_HOLDER(tp));
#if	UNIX_LOCKS && ((NCPUS > 1) )
	error = mpsleep(chan, pri, wmesg, tmo, 
			    &tp->t_lock, MS_LOCK_WRITE|MS_LOCK_ON_ERROR);
#else
	error = tsleep(chan, pri, wmesg, tmo);
#endif
	if ((error) && (error != EWOULDBLOCK))
		return (error);
	if (tp->t_gen != gen) {
		return (ERESTART);
	}
	return (0);
}
