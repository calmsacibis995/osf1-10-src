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
static char *rcsid = "@(#)$RCSfile: ldtty_compat.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:05:38 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * process BSD4.3 and SVID ioctls
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/user.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/file.h>

#include <sys/stropts.h>
#include <sys/stream.h>
#include <tty/stream_tty.h>
#include <tty/ldtty.h>

#include <sys/termio.h>

/*
 * ldtty_compat_to_termios
 *
 * convert a BSD or SYSV M_IOCTL message to a termios M_IOCTL message.
 * this way the drivers will only see the termios ioctl's.
 * on the way up, the ldtty has to make sure that the original ioctl goes up.
 */
int
ldtty_compat_to_termios(
	register struct ldtty *tp,
	register mblk_t *mp,
	register int cmd,
	register struct termios *termp
	)
{
	register mblk_t *mp1;
	register struct iocblk *iocp;

	/*
	 * allocate the termios data
	 */
	if (!mp->b_cont ||
	    mp->b_cont->b_datap->db_size < sizeof(struct termios)) {
		if ((mp1 = allocb(sizeof(struct termios), BPRI_MED)) == NULL)
			return(-1);
		if (mp->b_cont)
			freemsg(mp->b_cont);
	} else {
		mp1 = mp->b_cont;
		mp1->b_rptr = mp1->b_datap->db_base;
	}
	iocp = (struct iocblk *)mp->b_rptr;
	/*
	 * save the original M_IOCTL command, replace the data.
	 */
	tp->t_ioctl_cmd = iocp->ioc_cmd;
	mp->b_cont = mp1;
	/*
	 * copy the termios info
	 */
	*((struct termios *) mp1->b_rptr) = *termp;
	mp1->b_wptr = mp1->b_rptr + sizeof(struct termios);
	/*
	 * fix up the iocblk
	 */
	iocp->ioc_count = sizeof(struct termios);
	iocp->ioc_cmd = cmd;
	return(0);
}

#ifdef COMPAT_43
/*
 * process BSD4.3 compatibility tty ioctl's
 */
void
ldtty_bsd43_ioctl(
	register struct ldtty *tp,
	register queue_t *q,
	register mblk_t *mp
	)
{
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {
	case TIOCLGET:
	case TIOCGETP: {
		struct termios term;

		term = tp->t_termios;
		if (ldtty_compat_to_termios(tp, mp, TIOCGETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q,mp);
	}
		break;
	case TIOCSETP:
	case TIOCSETN: {
		int		cmd;
		struct termios	term;

		term = tp->t_termios;
		sgttyb_to_termios((struct sgttyb *) mp->b_cont->b_rptr,
					&term, (tcflag_t *)&tp->t_flags);
		cmd = iocp->ioc_cmd == TIOCSETP ? TIOCSETAF : TIOCSETA;

		if (ldtty_compat_to_termios(tp, mp, cmd, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case TIOCGETC:
		termios_to_tchars(&tp->t_termios, 
				(struct tchars *) mp->b_cont->b_rptr);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct tchars);
		ldtty_iocack_msg(iocp, sizeof(struct tchars), mp);
		qreply(q, mp);
		break;
	case TIOCSETC: {
		struct termios term;

		term = tp->t_termios;
		tchars_to_termios((struct tchars *) mp->b_cont->b_rptr, &term);
		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case TIOCGLTC:
		termios_to_ltchars(&tp->t_termios, 
				(struct ltchars *) mp->b_cont->b_rptr);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + 
						sizeof(struct ltchars);
		ldtty_iocack_msg(iocp, sizeof(struct ltchars), mp);
		qreply(q, mp);
		break;
	case TIOCSLTC: {
		struct termios term;

		term = tp->t_termios;
		ltchars_to_termios((struct ltchars *)mp->b_cont->b_rptr, &term);
		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET: {
		register int com = iocp->ioc_cmd;
		register unsigned int newflags = *(unsigned int *)mp->b_cont->b_rptr;
		struct termios term;

		term = tp->t_termios;
		flags_to_termios(com, newflags, &term, &tp->t_flags);

		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case OTIOCGETD:
		/* this ioctl doesn't have much meaning in STREAMS
		 * But, since anyone who makes this call is attempting
		 * to discover if we are running the "new" job control
		 * line discipline, we lie and tell them we are.
		 */
		*(int *)mp->b_cont->b_rptr = 2;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
		ldtty_iocack_msg(iocp, sizeof(int), mp);
		qreply(q, mp);
		break;
	case OTIOCSETD:
		/* this ioctl doesn't have much meaning in STREAMS */
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	default:
		ldtty_iocnak_msg(iocp, EINVAL, mp);
		qreply(q, mp);
	}
}
#endif /* COMPAT_43 */

/* SVID starts */
/*
 * process SVID tty ioctl's
 */
void
ldtty_svid_ioctl(
	register struct ldtty *tp,
	register queue_t *q,		/* write q */
	register mblk_t *mp
	)
{
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {
	case TCXONC: {
		switch (*(int *)mp->b_cont->b_rptr) {
		case TCOOFF:
			/*
			 * should be the same as TIOCSTOP
			 */
			if ((tp->t_state & TS_TTSTOP) == 0) {
				tp->t_state |= TS_TTSTOP;
				putctl(q->q_next, M_STOP);
			    }
			break;
		case TCOON:
			/*
			 * should be the same as TIOCSTART
			 */
			if ((tp->t_state & TS_TTSTOP) || (tp->t_lflag & FLUSHO)) {
				tp->t_state &= ~TS_TTSTOP;
				putctl(q->q_next, M_START);
				tp->t_lflag &= ~FLUSHO;
				(void)ldtty_start(tp);
			    }
			break;
		case TCIOFF:
			if (!(tp->t_state & TS_TBLOCK) &&
			    putctl(q->q_next, M_STOPI))
				tp->t_state |= TS_TBLOCK;
			break;
		case TCION:
			if ((tp->t_state & TS_TBLOCK) &&
			    putctl(q->q_next, M_STARTI))
				tp->t_state &= ~TS_TBLOCK;
			break;
		default:
			ldtty_iocnak_msg(iocp, EINVAL, mp);
			qreply(q, mp);
			return;
		}
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	}
	case TCFLSH: {
		register int flags;

		switch (*(int *)mp->b_cont->b_rptr) {
		case TCIFLUSH:
			flags = FREAD;
			break;
		case TCOFLUSH:
			flags = FWRITE;
			break;
		case TCIOFLUSH:
			flags = FREAD | FWRITE;
			break;
		default:
			ldtty_iocnak_msg(iocp, EINVAL, mp);
			qreply(q, mp);
			return;
		}
		(void) ldtty_flush(tp, flags, 1);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
		break;

	case TCGETA: {
		register struct termio *termiop;
		register int speed;

		termiop = (struct termio *)mp->b_cont->b_rptr;
		termiop->c_iflag = tp->t_iflag;
		termiop->c_oflag = tp->t_oflag;
		speed = ttspeedtab(tp->t_ospeed, ttcompatspeeds);
		speed = speed == -1 ? B0 : speed;
		termiop->c_cflag = tp->t_cflag | speed;
		termiop->c_lflag = tp->t_lflag;
		if (tp->t_lflag & NOFLSH) {
			termiop->c_lflag |= VNOFLSH;
		}
		termiop->c_line = 0;	/* this is STREAMS */
		termiop->c_cc[VVINTR] = tp->t_cc[VINTR];
		termiop->c_cc[VVQUIT] = tp->t_cc[VQUIT];
		termiop->c_cc[VVERASE] = tp->t_cc[VERASE];
		termiop->c_cc[VVKILL] = tp->t_cc[VKILL];
		termiop->c_cc[VVEOL2] = tp->t_cc[VEOL2];
		termiop->c_cc[VVSWTCH] = tp->t_cc[VSUSP];
		if (tp->t_lflag & ICANON) {
			termiop->c_cc[VVEOL] = tp->t_cc[VEOL];
			termiop->c_cc[VVEOF] = tp->t_cc[VEOF];
		}
		else {
			termiop->c_cc[VVMIN] = tp->t_cc[VMIN];
			termiop->c_cc[VVTIME] = tp->t_cc[VTIME];
		}
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct termio);
		ldtty_iocack_msg(iocp, sizeof(struct termio), mp);
		qreply(q, mp);
	}
		break;
	case TCSETA:
	case TCSETAW:
	case TCSETAF: {
		register struct termio *termiop;
		struct termios term;
		register int speed, i, cmd;

		/* Note that since the termio structure uses shorts and */
		/* the termios structure uses longs any flag set in the */
		/* upper half of the word will be cleared. However, this */
		/* is the correct behavior. Note that we have to move */
		/* around the NOFLSH flag. */
		termiop = (struct termio *)mp->b_cont->b_rptr;
		term.c_iflag = termiop->c_iflag;
		term.c_oflag = termiop->c_oflag;
		term.c_cflag = termiop->c_cflag & ~0xf;
		term.c_ispeed = term.c_ospeed =
					ttcompatspcodes[termiop->c_cflag & 0xf];
		term.c_lflag = termiop->c_lflag & ~VNOFLSH;
		if (termiop->c_lflag & VNOFLSH) {
			term.c_lflag |= NOFLSH;
        	}
		/* Initialize the whole termios cc array since its bigger */
		for (i = 0; i < NCCS; i++) {
			term.c_cc[i] = tp->t_cc[i];
		}
		term.c_cc[VINTR] = termiop->c_cc[VVINTR];
		term.c_cc[VQUIT] = termiop->c_cc[VVQUIT];
		term.c_cc[VERASE] = termiop->c_cc[VVERASE];
		term.c_cc[VKILL] = termiop->c_cc[VVKILL];
		term.c_cc[VEOL2] = termiop->c_cc[VVEOL2];
		term.c_cc[VSUSP] = termiop->c_cc[VVSWTCH];
		if (termiop->c_lflag & ICANON) {
			term.c_cc[VEOL] = termiop->c_cc[VVEOL];
			term.c_cc[VEOF] = termiop->c_cc[VVEOF];
		}
		else {
			term.c_cc[VMIN] = termiop->c_cc[VVMIN];
			term.c_cc[VTIME] = termiop->c_cc[VVTIME];
		}
		switch (iocp->ioc_cmd) {
		case TCSETAW:
			cmd = TIOCSETAW;
			break;
		case TCSETAF:
			cmd = TIOCSETAF;
			break;
		case TCSETA:
			cmd = TIOCSETA;
			break;
		}

		if (ldtty_compat_to_termios(tp, mp, cmd, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	default:
		ldtty_iocnak_msg(iocp, EINVAL, mp);
		qreply(q, mp);
	}
}
/* SVID ends */
