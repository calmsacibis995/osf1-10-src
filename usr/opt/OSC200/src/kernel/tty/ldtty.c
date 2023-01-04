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
static char *rcsid = "@(#)$RCSfile: ldtty.c,v $ $Revision: 1.1.5.4 $ (DEC) $Date: 1993/08/19 12:25:34 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
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
 *
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
#include <streams/memdebug.h>

/*
 * module for the EUC line discipline
 */
#define MODULE_ID	7701
#define MODULE_NAME	"ldterm"

static struct module_info minfo = {
	MODULE_ID, MODULE_NAME, 0, INFPSZ,
	LDTTY_STREAMS_HIWAT, LDTTY_STREAMS_LOWAT
};

static struct qinit rinit = {
	ldtty_rput, ldtty_rsrv, ldtty_open, ldtty_close, 0, &minfo, 0};

static struct qinit winit = {
	ldtty_wput, ldtty_wsrv, 0, 0, 0, &minfo, 0};

struct streamtab ldttyinfo = { &rinit, &winit };

int
ldtty_configure(
	sysconfig_op_t	op,
	str_config_t *	indata,
	size_t		indatalen,
	str_config_t *	outdata,
	size_t		outdatalen
	)
{
	struct streamadm	sa;
	dev_t			devno;

	if (op != SYSCONFIG_CONFIGURE)
		return EINVAL;

	if (indata != NULL && indatalen == sizeof(str_config_t)
			&& indata->sc_version == OSF_STREAMS_CONFIG_10)
		devno = indata->sc_devnum;
	else
		devno = NODEV;

	sa.sa_version		= OSF_STREAMS_10;
	sa.sa_flags		= STR_IS_MODULE | STR_SYSV4_OPEN;
	sa.sa_ttys		= (caddr_t)NULL;
	sa.sa_sync_level	= SQLVL_QUEUEPAIR;
	sa.sa_sync_info		= (caddr_t)NULL;
	strcpy(sa.sa_name,	"ldtty");

	if ((devno = strmod_add(devno, &ldttyinfo, &sa)) == NODEV) {
		return ENODEV;
	}

	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
		outdata->sc_version = OSF_STREAMS_CONFIG_10;
		outdata->sc_devnum = devno;
		outdata->sc_sa_flags = sa.sa_flags;
		strcpy(outdata->sc_sa_name, sa.sa_name);
	}

	return 0;
}

/*
 * module specific structures/definitions
 */

/*
 * open
 */
PRIVATE_STATIC int
ldtty_open(
	register queue_t *q,
	register dev_t *devp,
	register int flag,
	register int sflag,
        cred_t *credp
	)
{
	register struct ldtty *tp;
	register mblk_t *mp;
	register struct stroptions *sop;
	int error;

	if (q->q_ptr) {
		/* reopen */
		tp = (struct ldtty *) q->q_ptr;
		ASSERT(tp->t_state & TS_ISOPEN);
		return(0);
	} 

	/* else first open */

	if (error = streams_open_comm(sizeof(struct ldtty), 
					q, devp, flag, sflag, credp))
		return(error);

	tp = (struct ldtty *) q->q_ptr;
	tp->t_queue = q;

	mp = allocb(sizeof(struct stroptions), BPRI_HI);
	if (!mp) {
		streams_close_comm(q, flag, credp);
		return(ENOMEM);
	}

	/*
	 * send the M_SETOPS to the stream head
	 */

	mp->b_datap->db_type = M_SETOPTS;
	sop = (struct stroptions *)mp->b_rptr;
	mp->b_wptr =  mp->b_rptr + sizeof(struct stroptions);
	sop->so_flags = SO_READOPT |
			SO_MREADOFF | SO_NDELON | SO_ISTTY;
	sop->so_readopt = RMSGN | RPROTNORM;
	putnext(q, mp);

	/*
	 * Default values for contents of ldtty -- streams_open_comm()
	 * returns bzero'ed memory, so only nonzero fields filled in here.
	 */

	tp->t_state = TS_ISOPEN;

	simple_lock_init(&tp->t_intimeout_lock);

	tp->t_iflag = TTYDEF_IFLAG;
	tp->t_oflag = TTYDEF_OFLAG;
	tp->t_lflag = TTYDEF_LFLAG;
	/*
	 * t_cflag and speeds will be provided by driver
	 * if/when we need them
	 */
	bcopy((caddr_t)ttydefchars, (caddr_t)tp->t_cc, sizeof(tp->t_cc));
	tp->t_shad_time = (tp->t_cc[VTIME] * hz) / 10;

	tp->t_cswidth.eucw[0] = 1;
	tp->t_cswidth.eucw[1] = 1;
	tp->t_cswidth.scrw[0] = 1;
	tp->t_cswidth.scrw[1] = 1;
	return(0);
}

/*
 * close
 */
PRIVATE_STATIC int
ldtty_close(register queue_t *q, int flag, cred_t *credp)
{
	register struct ldtty *tp;
	int error = 0;
        mblk_t *mp;

	tp = (struct ldtty *)q->q_ptr;
        /*
         * Undo stream head changes we did when first opened
         */
        mp = allocb(sizeof(struct stroptions), BPRI_MED);
        if (mp) {
                struct stroptions *sop;

		/*
		 * send the M_SETOPS to the stream head
		 */
		mp->b_datap->db_type = M_SETOPTS;
		sop = (struct stroptions *)mp->b_rptr;
		mp->b_wptr =  mp->b_rptr + sizeof(struct stroptions);
		sop->so_flags = SO_READOPT | SO_TONSTOP |
				SO_MREADOFF | SO_NDELOFF | SO_ISNTTY;
		sop->so_readopt = RNORM;
		putnext(q, mp);
	}
	if (tp->t_state & TS_TTSTOP) {
		tp->t_state &= ~TS_TTSTOP;
		putctl(WR(tp->t_queue)->q_next, M_START);
	}
	while (ldtty_msgdsize(tp->t_outbuf) || WR(tp->t_queue)->q_first) {
		/* One last chance for output to drain */
		tp->t_state &= ~TS_WAITOUTPUT;
		if (tp->t_state & TS_WAITOUTBUF) {
			/*
			 * Some previous call to allocb() for
			 * t_outmsg failed.
			 */
			ASSERT(WR(tp->t_queue)->q_first);
			tp->t_state &= ~TS_WAITOUTBUF;
			unbufcall(tp->t_outbid);

			if (!ldtty_getoutbuf(tp))
				goto slp;
		}

		if (ldtty_start(tp) || WR(tp->t_queue)->q_first) {
	slp:
			if (flag & (FNDELAY|FNONBLOCK))
				break;
			tp->t_state |= TS_ASLEEP;
			if (error = tsleep((caddr_t)&tp->t_outbuf,
					TTIPRI | PCATCH, "ldclose", 0))
				break;
		}
	}
	(void) ldtty_flush(tp, FREAD|FWRITE, 0);
	/*
	 * free struct ldtty and stuff inside it
	 */
	if (tp->t_intimeout)
		untimeout(tp->t_intimeout);
	if (tp->t_outbid)
		unbufcall(tp->t_outbid);
	if (tp->t_ackbid)
		unbufcall(tp->t_ackbid);
	if (tp->t_hupbid)
		unbufcall(tp->t_hupbid);
	if (tp->t_rawbuf)
		freemsg(tp->t_rawbuf);
	if (tp->t_outbuf)
		freemsg(tp->t_outbuf);
	if (tp->t_sparebuf)
		freemsg(tp->t_sparebuf);
	streams_close_comm(q, flag, credp);
	return(error);
}

/*
 * read side should process the characters coming in from the terminal
 * and other miscellaneous messages
 */
PRIVATE_STATIC int
ldtty_rput(register queue_t *q, register mblk_t *mp)
{
	register struct ldtty 	*tp;

	tp = (struct ldtty *)q->q_ptr;
	switch (mp->b_datap->db_type) {
        case M_DATA:
                if (tp->t_state & TS_NOCANON) {
			if (canput(q->q_next))
                        	putnext(q, mp);
			else
				freemsg(mp);
                } else {
			mp->b_flag |= MSGCOMPRESS;
			if ((tp->t_state & TS_RAWBACKUP) || q->q_first
			|| (mp = ldtty_readdata(tp, mp))) {
				/* mp may have advanced from where it
				 * pointed a few lines above, so
				 * set MSGCOMPRESS bit again.
				 */
				mp->b_flag |= MSGCOMPRESS;
				putq(q, mp);
			}
		}
                break;
        case M_BREAK:
                ldtty_break(tp, mp);
                break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR)
			ldtty_flush(tp, FREAD, 0);
		putnext(q, mp);
		break;
        case M_IOCACK:
                if (!ldtty_ioctl_ack(tp, q, mp)) {
			/* Out of memory -- we have to change this
			 * to a normal priority message to keep
			 * q from getting instantly enabled.  Service
			 * routine knows to undo this.  M_IOCTL is
			 * suitable as the temporary type since
			 * M_IOCTL's never come from downstream
			 * (per streams spec).
			 */
			mp->b_datap->db_type = M_IOCTL;
			putbq(q, mp);
		}
                break;
        case M_CTL:
                if (!ldtty_mctl(tp, q, mp))
			putbq(q, mp);
                break;
	case M_HANGUP:
		if (!ldtty_mhangup(tp, q, mp)) {
			/* Hi-pri, so putq would qenable immediately.
			 * We'll just send the message along without
			 * the M_ERROR that ldtty_hangup was to insert.
			 * The effect at the stream head will be that
			 * write(2)'s get ENXIO instead of EIO - seems
			 * preferable to delaying the hangup condition
			 * indefinitely.
			 */
			putnext(tp->t_queue, mp);
		}
		break;
        default:
                if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
			putnext(q, mp);
		else
			putq(q, mp);
                break;
        }
	return(0);
}

PRIVATE_STATIC int
ldtty_rsrv(register queue_t *q)
{
	register mblk_t *mp;
	struct ldtty	*tp = (struct ldtty *) q->q_ptr;
	int		need_wakeup = 0;

	ldtty_check_intimeout(tp, &need_wakeup);
	if (need_wakeup)
		ldtty_wakeup(tp);
	if ((tp->t_state & TS_TBLOCK) && (tp->t_lflag & ICANON))
		ldtty_tblock(tp);

	while (mp = getq(q)) {
		switch(mp->b_datap->db_type) {
		case M_DATA:
			if (mp = ldtty_readdata(tp, mp)) {
				tp->t_state |= TS_RAWBACKUP;
				putbq(q, mp);
				return(0);
			}
			break;
		case M_IOCTL:
			/* This is actually an M_IOCACK -- see note
			 * in ldtty_rput().
			 */
			mp->b_datap->db_type = M_IOCACK;
			if (!ldtty_ioctl_ack(tp, q, mp)) {
				mp->b_datap->db_type = M_IOCTL;
				putbq(q, mp);
				return(0);
			}
			break;
		case M_CTL:
			if (!ldtty_mctl(tp, q, mp)) {
				putbq(q, mp);
				return(0);
			}
			break;
		default:
			ASSERT(mp->b_datap->db_type < QPCTL);
			if (canput(q->q_next))
				putnext(q, mp);
			else {
				putbq(q, mp);
				return;
			}
		}
	}
}

/*
 * write side should process characters coming from the application
 * and ioctl's and other miscellaneous messages.
 */
PRIVATE_STATIC int
ldtty_wput(register queue_t *q, register mblk_t *mp)
{
	register struct ldtty *tp;

	tp = (struct ldtty *)q->q_ptr;
	switch (mp->b_datap->db_type) {
	case M_DATA:
		mp->b_flag |= MSGCOMPRESS;
		if (q->q_first || (!canput(q->q_next))) {
			putq(q, mp);
		} else
			(void)ldtty_write(tp, mp);
		break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			ldtty_flush(tp, FWRITE, 0);
		putnext(q, mp);
		break;
	case M_READ:
		ldtty_unset_intimeout(tp);
		/*
		 * Rig up timeout or send raw data as appropriate if !ICANON.
		 * If t_shcc is non-zero, then the M_READ must have been
		 * sent while data was in flight from the line discipline
		 * to the stream head.  Just ignore the M_READ in this case:
		 * if the stream head needs more data after the in-flight data
		 * is consumed, it will ask again (with a new M_READ); if it
		 * doesn't, and if we don't ignore this M_READ, then we're at
		 * risk of sending a zero length M_DATA upstream when VMIN is
		 * greater than zero.
		 */
		if (!(tp->t_lflag & ICANON) && (tp->t_shcc == 0)) {
			if (tp->t_shad_time > 0) {
				/*
				 * We're using VTIME
				 * Flag that a read is pending
				 */
				tp->t_state |= TS_VTIME_FLAG;
				if (tp->t_rawcc || (tp->t_cc[VMIN] == 0)) {
					/*
					 * Start the timer now
					 */
					ldtty_set_intimeout(tp,tp->t_shad_time);
				}
			} else if (tp->t_cc[VMIN] == 0)
				ldtty_wakeup(tp);
		}
		putnext(q, mp);
		break;
	case M_NOTIFY:
		if (tp->t_lflag & ICANON)
			freemsg(mp);
		else
			ldtty_mnotify(tp, mp);
		break;
	case M_IOCTL: {
		register struct iocblk *iocp;

		if (tp->t_qioctl) {
			rmvq(q, tp->t_qioctl);
			freemsg(tp->t_qioctl);
			tp->t_qioctl = (mblk_t *) NULL;
		}
		/*
		 * See if it's one we recognize.  If not, pass the message 
		 * along.  See if the size of the data is correct.  If not, 
		 * NAK it.  If all ok, process it.
		 */
		iocp = (struct iocblk *)mp->b_rptr;
		tp->t_ioctl_cmd = 0;

		switch (iocp->ioc_cmd) {
		case TIOCSETD:
		case TIOCFLUSH:
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF:
		case TIOCSWINSZ:
		case TIOCGETD:
		case TIOCOUTQ:
		case TIOCGETA:
		case TIOCGWINSZ:
		case TIOCSTI:
		case EUC_WSET:
		case EUC_WGET:
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
#endif /* COMPAT_43 */
/* SVID start */
		case TCXONC:
		case TCFLSH:
		case TCSETA:
		case TCSETAW:
		case TCSETAF:
		case TCGETA:
		case TCSBRK:
/* SVID end */
			if (strtty_ioctlbad(mp)) {
				qreply(q,mp);
				return (0);
			}
			break;
		case TIOCEXCL:
		case TIOCNXCL:
		case TIOCHPCL:
		case TIOCSTOP:
		case TIOCSTART:
		case TIOCSBRK:
		case TIOCCBRK:
			/*
			 * These may come down as TRANSPARENT ioctls
			 * Make sure ioc_count is zeroed for return
			 */
			iocp->ioc_count = 0;
			if (mp->b_cont) {
				freemsg(mp->b_cont);
				mp->b_cont = NULL;
			}
			break;
		default:
			putnext(q, mp);
			return(0);
		}
		/*
		 * A few ioctls must be queued behind data messages so
		 * data will drain before the ioctls are executed.
		 */
		iocp->ioc_error = 0;
		switch (iocp->ioc_cmd) {
		case TIOCSETAW:
		case TIOCSETAF:
/* SVID start */
		case TCSETAW:
		case TCSETAF:
		case TCSBRK:
/* SVID end */
		    	if (q->q_first) {
				tp->t_qioctl = mp;
				putq(q, mp);
				break;
			}
		/* else fall through */
		default:
			ldtty_ioctl(tp, q, mp);
		}
	}
		break;
	case M_CTL:
		ldtty_mctl(tp, q, mp);
		break;
	default:
		if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
			putnext(q, mp);
		else
			putq(q, mp);
		break;
	}
	return(0);
}

/*
 * write service routine
 * data may encounter flow control
 */
PRIVATE_STATIC int
ldtty_wsrv(register queue_t *q)		/* write q */
{
	register struct ldtty *tp;
	register mblk_t *mp;

	tp = (struct ldtty *)q->q_ptr;

	if (tp->t_state & TS_WAITOUTBUF) {
		/*
		 * Some previous call to allocb() for t_outmsg failed.
		 */
		tp->t_state &= ~TS_WAITOUTBUF;
		
		if (!ldtty_getoutbuf(tp))
			return(0);
	}

	if (tp->t_state & TS_WAITOUTPUT) {
		/*
		 * Some previous call to ldtty_start() was flow controlled.
		 */
		tp->t_state &= ~TS_WAITOUTPUT;
		if (ldtty_start(tp) != 0) {
			/* still flow controlled */
			return(0);
		}
	}

	if (tp->t_state & TS_WAITEUC) {
		tp->t_state &= ~TS_WAITEUC;
		if (euctty_out(tp) != 0) {
			/* still can't copy multibyte char to outbuf */
			return(0);
		}
	}

	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (ldtty_write(tp, mp))
				return(0);	/* can't do more output now */
			break;
		case M_IOCTL:
			ASSERT(mp == tp->t_qioctl);
			tp->t_qioctl = (mblk_t *) NULL;
			ldtty_ioctl(tp, q, mp);
			break;
		default:
			if (!canput(q->q_next)) {
				putbq(q, mp);
				return(0);
			}
			putnext(q, mp);
			break;
		} /* switch */
	} /* while */
	return(0);
}

/*
 * ldtty_readdata - Hand characters of a message one by one to ldtty_input(),
 * then initiate post processing.  If ldtty_input() finds that the raw input
 * buffer is at MAX_INPUT, it will return a T_POST_BACKUP bit, which tells
 * us to return the unfinished mblk_t to our caller.  In the absence of
 * T_POST_BACKUP, we'll always return a null pointer.  T_POST_BACKUP can
 * only happen in non-ICANON mode.
 */

PRIVATE_STATIC mblk_t *
ldtty_readdata(register struct ldtty *tp, register mblk_t *mp)
{
        register int post_wakeup = 0;

        while (mp) {
        	mblk_t *mp1;

		mp1 = mp->b_cont;

		while (mp->b_rptr < mp->b_wptr) {
			post_wakeup |= ldtty_input(tp, *mp->b_rptr);
			if (post_wakeup & T_POST_FLUSH)
				post_wakeup = 0;
			if (post_wakeup & T_POST_BACKUP) {
				tp->t_state |= TS_RAWBACKUP;
				goto out;
			}
			++mp->b_rptr;
		}

		/* Attempt to recycle memory that we'll need again soon.
		 * This will save a typical vi session one allocb() per
		 * character typed.
		 */
		if (tp->t_sparebuf)
			freeb(mp);
		else {
			mp->b_cont = NULL;
			tp->t_sparebuf = mp;
		}
                mp = mp1;
        }
out:
        ldtty_post_input(tp, post_wakeup);
	if (post_wakeup & T_POST_BACKUP)
		return(mp);
	else
		return((mblk_t *) 0);
}

/*
 * tty ioctls
 *
 * some ioctls are forwarded, some are acked
 */
PRIVATE_STATIC void
ldtty_ioctl(
	register struct ldtty *tp, 
	register queue_t *q,
	register mblk_t *mp
	)
{
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {
	case TIOCSETD:
		/* this ioctl doesn't have much meaning in STREAMS */
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	case TIOCGETD:
		/* 
		 * This ioctl doesn't have much meaning in STREAMS.
		 * It's apparently only used by things wanting to know
		 * if job control is supported, and they view "2" as a yes
		 * answer, so we return 2 for binary compatibility, since
		 * we do support job control.
		 */
		*(int *)mp->b_cont->b_rptr = 2;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
		ldtty_iocack_msg(iocp, sizeof(int), mp);
		qreply(q, mp);
		break;
	case TIOCFLUSH: {
		register int flags = *(int *)mp->b_cont->b_rptr;

		if (flags)
			flags &= FREAD|FWRITE;
		else
			flags = FREAD|FWRITE;
		(void) ldtty_flush(tp, flags, 1);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	}
	case TIOCOUTQ:
		*(int *)mp->b_cont->b_rptr = ldtty_msgdsize(tp->t_outbuf);
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
		ldtty_iocack_msg(iocp, sizeof(int), mp);
		qreply(q, mp);
		break;
	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF:
		/*
		 * these will be handled on the way up, M_IOCACK
		 */
		putnext(q, mp);
		break;
	case TIOCGETA:
		/*
		 * we fill in the info on the way up, M_IOCACK
		 */
		putnext(q, mp);
		break;
	case TIOCSWINSZ:
		ldtty_swinsz(tp, mp);
		putnext(q, mp);
		break;
	case TIOCGWINSZ:
		*(struct winsize *)mp->b_cont->b_rptr = tp->t_winsize;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct winsize);
		ldtty_iocack_msg(iocp, sizeof(struct winsize), mp);
		qreply(q, mp);
		break;
	case TIOCHPCL: {
		/*
		 * convert this one into a TIOCSETA for the driver
		 * then handle it on the way up
		 */
		struct termios term;

		term = tp->t_termios;
		term.c_cflag |= HUPCL;
		if (ldtty_compat_to_termios(tp, mp, TIOCSETA, &term)) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		putnext(q, mp);
	}
		break;
	case TIOCSTOP:
		if ((tp->t_state & TS_TTSTOP) == 0) {
			tp->t_state |= TS_TTSTOP;
			putctl(q->q_next, M_STOP);
		}
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	case TIOCSTART:
		if ((tp->t_state & TS_TTSTOP) || (tp->t_lflag & FLUSHO)) {
			tp->t_state &= ~TS_TTSTOP;
			putctl(q->q_next, M_START);
			tp->t_lflag &= ~FLUSHO;
			(void)ldtty_start(tp);
		}
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
		break;
	case EUC_WSET: {
		eucioc_t	*eu = (eucioc_t *)mp->b_cont->b_rptr;

		if ((eu->eucw[1] > 8) || (eu->eucw[2] > 8) || (eu->eucw[3] >8)){
			ldtty_iocnak_msg(iocp, EINVAL, mp);
			qreply(q, mp);
			return;
		}

		tp->t_cswidth = *eu;
		/* Not clear from spec. whether we should NAK attempt
		 * to set class 0 to something other than 1,1.  We ignore
		 * it and just take values for classes 1, 2, and 3.
		 */
		tp->t_cswidth.eucw[0] = 1;
		tp->t_cswidth.scrw[0] = 1;
                if ((tp->t_cswidth.eucw[1] > 1) ||
                    (tp->t_cswidth.scrw[1] > 1) ||
                    (tp->t_cswidth.eucw[2] > 1) ||
                    (tp->t_cswidth.scrw[2] > 1) ||
                    (tp->t_cswidth.eucw[3] > 1) ||
                    (tp->t_cswidth.scrw[3] > 1))
			tp->t_state |= TS_MBENABLED;
		else
			tp->t_state &= ~TS_MBENABLED;
		putnext(q, mp);
	}
		break;
	case EUC_WGET:
		*(eucioc_t *)mp->b_cont->b_rptr = tp->t_cswidth;
		mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(eucioc_t);
		ldtty_iocack_msg(iocp, sizeof(eucioc_t), mp);
		qreply(q, mp);
		break;
	case TIOCSBRK: {
		/*
		 * allocate M_BREAK and send a 1 to the driver
		 */
		register mblk_t *mp1;

		mp1 = allocb(sizeof(int), BPRI_MED);
		if (!mp1) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		mp1->b_datap->db_type = M_BREAK;
		*(int *)mp1->b_wptr++ = 1;
		putnext(q, mp1);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
		break;
	case TIOCCBRK: {
		/*
		 * allocate M_BREAK and send a 0 to the driver
		 */
		register mblk_t *mp1;

		mp1 = allocb(sizeof(int), BPRI_MED);
		if (!mp1) {
			ldtty_iocnak_msg(iocp, ENOMEM, mp);
			qreply(q, mp);
			return;
		}
		mp1->b_datap->db_type = M_BREAK;
		*(int *)mp1->b_wptr++ = 0;
		putnext(q, mp1);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
		break;
	case TIOCSTI: {
		/*
		 * Simulate typed input character
		 */
		ldtty_sti(tp, mp);
		ldtty_iocack_msg(iocp, 0, mp);
		qreply(q, mp);
	}
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
		ldtty_bsd43_ioctl(tp, q, mp);
		break;
#endif /* COMPAT_43 */
/* SVID starts */
	case TCXONC:
	case TCFLSH:
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
	case TCGETA:
		ldtty_svid_ioctl(tp, q, mp);
		break;
/* SVID ends */
	default:
		putnext(q, mp);
	}
}

/*
 * tty ioctl's
 *
 * For handling acks on the way up -- return 1 if ack sent, else 0.
 * M_IOCACK is high priority, so we'll only fail to send upstream
 * if we're waiting on memory.
 *
 * A TIOCSETA M_CTL can also come through here.
 */

PRIVATE_STATIC int
ldtty_ioctl_ack(
	register struct ldtty *tp,
	register queue_t *q,
	register mblk_t *mp
	)
{
	register struct iocblk *iocp;
	register struct termios *t;

	iocp = (struct iocblk *)mp->b_rptr;
	if (iocp->ioc_error == 0) switch (iocp->ioc_cmd) {
	case TIOCGETA:
		/*
		 * copy the speeds and cflag from the driver
		 */
		if (!mp->b_cont)
			break;
		t = (struct termios *)mp->b_cont->b_rptr;
		tp->t_ospeed = t->c_ospeed;
		tp->t_ispeed = t->c_ispeed;
		tp->t_cflag = t->c_cflag;
		*t = tp->t_termios;
		if (!tp->t_ioctl_cmd)
			break;
		switch(tp->t_ioctl_cmd) {
			case TIOCGETP: {
				register struct sgttyb *sg;

				sg = (struct sgttyb *)mp->b_cont->b_rptr;
				termios_to_sgttyb(&tp->t_termios, sg);
				mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(struct sgttyb);
				iocp->ioc_count = sizeof(struct sgttyb);
				break;
			}
			case TIOCLGET: {
				int ret = ttcompatgetflags(tp->t_iflag, tp->t_lflag,
							   tp->t_oflag, tp->t_cflag);
			
				*(int *)mp->b_cont->b_rptr = ret >> 16;
				mp->b_cont->b_wptr = mp->b_cont->b_rptr + sizeof(int);
				iocp->ioc_count = sizeof(int);
				break;
			}
		}
		iocp->ioc_cmd = tp->t_ioctl_cmd;
		tp->t_ioctl_cmd = 0;
		break;
				
	case TIOCSETA:
	case TIOCSETAW:
	case TIOCSETAF: {
		register int cmd = iocp->ioc_cmd;
                register mblk_t *flush_mp, *strop_mp;
                register struct stroptions *sop;

		/* Aside from copying the new termios, there are a few
		 * things to take care of here:
		 *
		 *	[1] If we're transitioning from canon to noncanon
		 *	or vice-versa, or if the TOSTOP bit is changing, we'll
		 *	need to modify some stream head settings (M_SETOPTS).
		 *
		 *	[2] If we're changing from canon to noncanon, we'll
		 * 	need to initialize tp->t_shcc to zero.  This counter
		 *	tracks the size of the stream head's copy of unread
		 * 	raw data.
		 *
		 *	[3] If we're changing from noncanon to canon, we'll
		 *	need to flush the stream head's copy (if any) of 
		 *	unread data and cook our local copy.
		 *
		 *	[4] If we're staying in noncanon but VMIN is about to
		 *	become "unsatisfied", we'll need to remove any unread
		 *	data from the stream head.
		 *
		 *	[5] If we're leaving IXON mode and have previously
		 *	received a VSTOP character, we'll need to restart
		 *	output.
		 *
		 * Items [3] and [4] will require an M_FLUSH to be allocated,
		 * as will any invocation of the SETAF form of the ioctl.
		 * We allocate the M_FLUSH up front so that the ioctl can
		 * be deferred on memory failure (bufcall, etc.) without
		 * leaving things half baked.
		 */

		if (!mp->b_cont)
			if (mp->b_datap->db_type == M_CTL)
				return(1);
			else
				break;

		t = (struct termios *)mp->b_cont->b_rptr;

		/* Attempt to allocate an M_FLUSH if we'll need one. */

                if (ldtty_need_flush(cmd, tp, t)) {
			flush_mp = allocb(1, BPRI_HI);
			if (flush_mp == NULL) {
				if (tp->t_ackbid)
					unbufcall(tp->t_ackbid);
				tp->t_ackbid = bufcall(1, BPRI_HI, qenable,
						tp->t_queue);
				return(0);
			}
		} else
			flush_mp = NULL;

		iocp->ioc_count = 0;

		if (ldtty_need_setopts(&tp->t_termios, t)) {
			/*
			 * Send an M_SETOPTS message to reflect the new
			 * settings.
			 */

			if (!canput(tp->t_queue->q_next)) {
				if (flush_mp)
					freemsg(flush_mp);
				return(0);
			}

			strop_mp = allocb(sizeof (struct stroptions),BPRI_MED);
			if (strop_mp == NULL) {
				/*
				 * No memory. Delay this ack until later.
				 * Process issuing the ioctl will stall.
				 */
				tp->t_ackbid = 
					bufcall(sizeof(struct stroptions),
					        BPRI_MED, qenable, tp->t_queue);
				if (flush_mp)
					freemsg(flush_mp);
				return(0);
			}
			/*
			 * send the M_SETOPTS to the stream head
			 */
			strop_mp->b_datap->db_type = M_SETOPTS;
			sop = (struct stroptions *)strop_mp->b_rptr;
			strop_mp->b_wptr = strop_mp->b_rptr +
				sizeof(struct stroptions);
			if (t->c_lflag & ICANON) {
				sop->so_readopt = RMSGN | RPROTNORM;
				sop->so_flags = SO_MREADOFF | SO_READOPT;
			} else {
				sop->so_readopt = RNORM;
#ifdef	RPROTCOMPRESS
				sop->so_readopt |= RPROTCOMPRESS;
#endif
				sop->so_flags = SO_MREADON | SO_READOPT;
			}

			if (t->c_lflag & TOSTOP)
				sop->so_flags |= SO_TOSTOP;
			else
				sop->so_flags |= SO_TONSTOP;

			putnext(q, strop_mp);
		}
                /*
                 * for TIOCSETAW and TIOCSETAF,
                 * the output has already been drained by the driver
                 */
                if (cmd == TIOCSETAF) {
                        ldtty_flush_mp(tp, FREAD, flush_mp, 0);
			flush_mp = NULL;
		}

		/* Check if we're switching into ICANON -- we'll
		 * need to cook buffered data in this case.
		 */
		if (!(tp->t_lflag & ICANON) && (t->c_lflag & ICANON)) {
			ldtty_unset_intimeout(tp);
			t->c_lflag |= PENDIN;
			tp->t_state &= ~TS_RAWBACKUP;
			if (tp->t_shcc) {
				ASSERT(flush_mp != (mblk_t *) 0);
				(void) ldtty_flush_shead(tp, flush_mp);
				flush_mp = NULL;
			}
			/* Call ldtty_tblock() since flow control
			 * works differently in ICANON/!ICANON. */
			ldtty_tblock(tp);
		}

		if (!(t->c_lflag & ICANON)) {
			if (tp->t_lflag & ICANON) {
				/* Switching out of ICANON -- start keeping
				 * track of byte count at stream head.
				 */
				tp->t_shcc = 0;
			} else {
				if (tp->t_shcc && (t->c_cc[VMIN] > tp->t_shcc)){
					/* VMIN no longer satisfied -- remove
					 * unread data from stream head.
					 */
					ASSERT(flush_mp != (mblk_t *) 0);
					(void)ldtty_flush_shead(tp, flush_mp);
					flush_mp = NULL;
				}
				if ((t->c_cc[VTIME] == 0)
				|| (t->c_cc[VMIN] > 0 && tp->t_rawcc == 0))
					ldtty_unset_intimeout(tp);
			}
		}

		if ((tp->t_state & TS_TTSTOP) &&
		    (tp->t_iflag & IXON) && !(t->c_iflag & IXON)) {
			tp->t_state &= ~TS_TTSTOP;
			putctl(WR(tp->t_queue)->q_next, M_START);
			(void)ldtty_start(tp);
		}

		ASSERT(flush_mp == (mblk_t *) 0);
		if (flush_mp)
			freemsg(flush_mp);
                /*
                 * set device information
		 */
		tp->t_termios = *t;
		tp->t_shad_time = (tp->t_cc[VTIME] * hz) / 10;

		/*
		 * if we need to "recanonicalize" data, do processing
		 */
		if (tp->t_lflag & PENDIN)
			ldtty_pend(tp);

		/* This may have been an M_CTL resulting from an ioctl
		 * down the master side (i.e., the other side) of a pty.
		 * If so, we're done.
		 */
		if (mp->b_datap->db_type == M_CTL)
			return(1);

		/*
		 * Restore cmd if this really was a BSD or SYSV ioctl
		 */
		if (tp->t_ioctl_cmd) {
			iocp->ioc_cmd = tp->t_ioctl_cmd;
			tp->t_ioctl_cmd = 0;
		}
		break;
	}
	case TIOCSWINSZ:
	case TIOCHPCL:
	case EUC_WSET:
		iocp->ioc_count = 0;
		break;
	}
	putnext(q, mp);
	return(1);
}

PRIVATE_STATIC int
ldtty_need_flush(int cmd, struct ldtty *tp, struct termios *t)
{
	if (cmd == TIOCSETAF) {
		/*
		 * always need it for SETAF
		 */
		return(1);
	} else if (t->c_lflag & ICANON) {
		if (!(tp->t_lflag & ICANON) && tp->t_shcc) {
			/*
			 * have to cook buffered data
			 */
			return(1);
		}
	} else if (!(tp->t_lflag & ICANON)
		    && tp->t_shcc && (t->c_cc[VMIN] > tp->t_shcc)) {
			/*
			 * VMIN was satisfied under prior settings,
			 * but is not satisfied under new settings.
			 */
			return(1);
	}
	return(0);
}

/*
 * process pending characters
 */
PRIVATE_STATIC void
ldtty_pend(struct ldtty *tp)
{
	mblk_t *mp;

	tp->t_lflag &= ~PENDIN;
	tp->t_state |= TS_TYPEN;
	mp = tp->t_rawbuf;      		/* get current message */
	ldtty_bufclr(tp); 			/* clear input buffer */
        if (mp)
                (void) ldtty_readdata(tp, mp);  /* reprocess input */
	tp->t_state &= ~TS_TYPEN;
}

/*
 * process input characters
 * returns flag bits describing postprocessing
 * T_POST_WAKEUP set if ldtty_wakeup() is required
 * T_POST_START set if ldtty_start() is required
 * returns 0 if no postprocessing is required
 */
int
ldtty_input(register struct ldtty *tp, register int c)
{
	register int iflag = tp->t_iflag;
	register int lflag = tp->t_lflag;
	register u_char *cc = tp->t_cc;
	register int i;
	register int post_wakeup = 0;
        int err;

	/*
	 * accumulate statistics
	 */
	tk_nin++;
	if (lflag & ICANON)
		tk_cancc++;
	else
		tk_rawcc++;
	/*
	 * in tandem mode, check high water mark
	 */
	if (iflag & IXOFF)
		ldtty_tblock(tp);
	/*
	 * Strip the character if we have not already processed it
	 * once. (Characters coming in while TS_TYPEN is set have
	 * already been through here once while we were !ICANON,
	 * but are now being cooked after a switch (TIOCSETA*) back
	 * to ICANON).
	 */
	if (((tp->t_state & TS_TYPEN) == 0) && ((c & TTY_QUOTE) == 0)) {
		if  (iflag & ISTRIP)
			c &= 0x7f;
		else if ((iflag & PARMRK) && (c == 0377)) {

			/*
			 * POSIX: If ISTRIP is not set (just checked
			 * above), and PARMRK is set, deliver a legitimate
			 * \0377 character as \0377, \0377.  Do the first
			 * by sending a quoted \0377 through ldtty_input().  
			 * Do the second by just allowing the \0377 that 
			 * we already have to fall through the rest of this 
			 * function.
			 */

			ldtty_input(tp,(TTY_QUOTE | 0377));
		}
	}
	/*
	 * check for literal next
	 */
	if (tp->t_state & TS_LNCH) {
		c |= TTY_QUOTE;
		tp->t_state &= ~TS_LNCH;
	}
	/*
	 * control chars which aren't controlled by ICANON, ISIG, IXON
	 */
	if (lflag & IEXTEN) {
		if (CCEQ(cc[VLNEXT], c)) {
			if (lflag & ECHO) {
				if (lflag & ECHOE) {
					ldtty_output(tp, '^');
					ldtty_output(tp, '\b');
				} else {
					ldtty_echo(tp, c);
				}
			}
			tp->t_state |= TS_LNCH;
			goto endcase;
		}
#ifndef VDISCARD
#define VDISCARD VFLUSHO
#endif /* VDISCARD */
		if (CCEQ(cc[VDISCARD], c)) {
			if (lflag & FLUSHO) {
				tp->t_lflag &= ~FLUSHO;
			} else {
				(void) ldtty_flush(tp, FWRITE, 1);
				ldtty_echo(tp, c);
				if (tp->t_rawcc)
					ldtty_retype(tp);
				tp->t_lflag |= FLUSHO;
			}
			goto startoutput;
		}
	}
	/*
	 * signals
	 */
	if (lflag & ISIG) {
		if (CCEQ(cc[VINTR], c) || CCEQ(cc[VQUIT], c)) {
			if ((lflag & NOFLSH) == 0) {
				post_wakeup |= T_POST_FLUSH;
				(void) ldtty_flush(tp, FREAD|FWRITE, 1);
			}
			ldtty_echo(tp, c);
			if (CCEQ(cc[VINTR], c))
				putctl1(tp->t_queue->q_next, M_PCSIG, SIGINT);
			else
				putctl1(tp->t_queue->q_next, M_PCSIG, SIGQUIT);
			goto endcase;
		}
		if (CCEQ(cc[VSUSP], c)) {
			if ((lflag & NOFLSH) == 0) {
				post_wakeup |= T_POST_FLUSH;
				(void) ldtty_flush(tp, FREAD|FWRITE, 1);
			}
			ldtty_echo(tp, c);
			putctl1(tp->t_queue->q_next, M_PCSIG, SIGTSTP);
			goto endcase;
		}
#ifndef VSTATUS
#define VSTATUS VINFO
#endif /* VSTATUS */
		if (CCEQ(cc[VSTATUS], c)) {
			putctl1(tp->t_queue->q_next, M_PCSIG, SIGINFO);
#ifdef LDTTY_DEBUG
			if ((lflag & NOKERNINFO) == 0)
				ldtty_info(tp);
#endif
			goto endcase;
		}
	}
	/*
	 * start/stop chars
	 */
	if (iflag & IXON) {
		if (CCEQ(cc[VSTOP], c)) {
			if ((tp->t_state & TS_TTSTOP) == 0) {
				tp->t_state |= TS_TTSTOP;
				putctl(WR(tp->t_queue)->q_next, M_STOP);
				return(post_wakeup);
			}
			if (!CCEQ(cc[VSTART], c))
				return(post_wakeup);
			/*
			 * if VSTART == VSTOP then toggle
			 */
			goto endcase;
		}
		if (CCEQ(cc[VSTART], c))
			goto restartoutput;
	}

	/*
	 * IGNCR, ICRNL, INLCR
	 */
	if (c == '\r') {
		if (iflag & IGNCR)
			goto endcase;
		else if (iflag & ICRNL)
			c = '\n';
	} else if ((c == '\n') && (iflag & INLCR))
		c = '\r';
	/*
	 * map upper case to lower case
	 */
	if ((iflag & IUCLC) && ('A' <= c) && (c <= 'Z'))
		c += 'a' - 'A';

	/*
	 * non canonical mode
	 */
	if ((lflag & ICANON) == 0) {
		if (tp->t_rawcc >= MAX_INPUT) {
			if (iflag & IMAXBEL) {
				if (ldtty_msgdsize(tp->t_outbuf) < LDTTYMAX)
					ldtty_output(tp, CTRL('g'));
			}
			post_wakeup |= T_POST_BACKUP;
		} else if (ldtty_stuffc(c & TTY_CHARMASK, tp) >= 0) {
			ldtty_echo(tp, c);
			if (tp->t_rawcc >= tp->t_cc[VMIN]) {

				/*
				 * Set flag but wait until caller is 
				 * done processing the whole M_DATA 
				 * message before sending.
				 */

				post_wakeup |= T_POST_WAKEUP;
			} else
                                /*
                                 * Caller may need to (re)start timer
                                 */
                                post_wakeup |= T_POST_TIMER;
		}
		goto endcase;
	}

	/*
	 * canonical mode
	 */

	/*
	 * erase
	 */
	if (CCEQ(cc[VERASE], c)) {
		if (ldtty_mbenabled(tp)) {
			euctty_erase(tp);
			goto endcase;
		}
		/*
		 * single byte erase
		 */
		if (tp->t_rawcc) {
			unsigned char rub_c;

			rub_c = ldtty_unstuffc(tp);
			ldtty_rub(tp, rub_c);
		}
		goto endcase;
	}
	/*
	 * kill
	 */
	if (CCEQ(cc[VKILL], c)) {
		if (ldtty_mbenabled(tp)) {
			euctty_kill(tp);
			goto endcase;
		}
		/*
		 * single byte kill
		 */
		if ((lflag & ECHOE) &&
		    (tp->t_rawcc == tp->t_rocount) &&
		    !(lflag & ECHOPRT)) {
			while (tp->t_rawcc) {
				unsigned char rub_c;

				rub_c = ldtty_unstuffc(tp);
				ldtty_rub(tp, rub_c);
			}
		} else {
			ldtty_echo(tp, c);
			if ((lflag & ECHOK) || (lflag & ECHOKE))
				ldtty_echo(tp, '\n');
			if (tp->t_rawbuf)
				ldtty_bufreset(tp);
			tp->t_rocount = 0;
		}
		tp->t_state &= ~TS_LOCAL;
		goto endcase;
	}
	/*
	 * word erase
	 */
	if (CCEQ(cc[VWERASE], c)) {
		if (ldtty_mbenabled(tp)) {
			euctty_werase(tp);
			goto endcase;
		}
		/*
		 * single byte word erase
		 */
		{
		register int ctype;
		int rub_c;

		/*
		 * erase white space
		 */
		while ((rub_c = ldtty_unstuffc(tp)) == ' ' ||
		       rub_c == '\t')
			ldtty_rub(tp, rub_c);
		if (rub_c == -1)
			goto endcase;
		/*
		 * special case last char of token
		 */
		ldtty_rub(tp, rub_c);
		rub_c = ldtty_unstuffc(tp);
		if ((rub_c == -1) || (rub_c == ' ') || (rub_c == '\t')) {
			if (rub_c != -1)
				ldtty_stuffc(rub_c, tp);
			goto endcase;
		}
		/*
		 * erase rest of token
		 */
#define CTYPE(c) ((lflag&ALTWERASE) ? (partab[(c)&TTY_CHARMASK]&0100) : 0)
		ctype = CTYPE(rub_c);
		do {
			ldtty_rub(tp, rub_c);
			rub_c = ldtty_unstuffc(tp);
			if (rub_c == -1)
				goto endcase;
		} while ((rub_c != ' ') && (rub_c != '\t') &&
			 (CTYPE(rub_c) == ctype));
		ldtty_stuffc(rub_c, tp);
		goto endcase;
#undef CTYPE
		} /* single byte word erase */
	}
	/*
	 * reprint
	 */
	if (CCEQ(cc[VREPRINT], c)) {
		ldtty_retype(tp);
		goto endcase;
	}
	/*
	 * check for input overflow
	 */
	if (tp->t_rawcc >= MAX_INPUT) {
		if (iflag & IMAXBEL) {
			if (ldtty_msgdsize(tp->t_outbuf) < LDTTYMAX)
				ldtty_output(tp, CTRL('g'));
		}
		goto endcase;
	}
	/*
	 * put data in tp->t_rawbuf
	 * wakeup if it is a line delimiter
	 */
	if (ldtty_stuffc(c & TTY_CHARMASK, tp) >= 0) {
                if (ldtty_breakc(c)) {
                        tp->t_rocount = 0;
                        /*
                         * At this point, we just want to send the
			 * rawbuf upstream.
                         */
                        ldtty_sendcanon(tp);
                } else if (tp->t_rocount++ == 0)
                        tp->t_rocol = tp->t_col;
                if (tp->t_state & TS_ERASE) {
                        /*
                         * end of prterase \.../
                         */
                        tp->t_state &= ~TS_ERASE;
                        ldtty_output(tp, '/');
                }
                i = tp->t_col;
                ldtty_echo(tp, c);
                if (CCEQ(cc[VEOF], c) && (lflag & ECHO)) {
                        /*
                         * place the cursor over the '^' of the ^D
                         */
                        i = MIN(2, tp->t_col - i);
                        while (i > 0) {
                                ldtty_output(tp, '\b');
                                i--;
                        }
                }
        }
endcase:
	/*
	 * IXANY means allow any character to restart output
	 */
	if ((tp->t_state & TS_TTSTOP) && !(iflag & IXANY) &&
	    (cc[VSTART] != cc[VSTOP]))
		return(post_wakeup);
restartoutput:
        /*
         * Only send an M_START message if we need to
         */
        if (tp->t_state & TS_TTSTOP) {
                tp->t_state &= ~TS_TTSTOP;
                putctl(WR(tp->t_queue)->q_next, M_START);
        }
	tp->t_lflag &= ~FLUSHO;
startoutput:
        /*
         * Caller needs to start output
         */
        post_wakeup |= T_POST_START;
	return(post_wakeup);
}

PRIVATE_STATIC int
ldtty_flush_shead(struct ldtty *tp, register mblk_t *mp)
{
	if (!mp) {
		mp = allocb(1, BPRI_HI);
		if (!mp)
			return(0);
	}
	mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
	mp->b_flag |= MSGNOTIFY;
	mp->b_datap->db_type = M_FLUSH;
	*mp->b_wptr++ = FLUSHR;
	tp->t_shcc = 0;
	tp->t_unsent = tp->t_rawbuf;
	if (tp->t_rawbuf)
		tp->t_unsent_ndx = 
			tp->t_rawbuf->b_rptr - tp->t_rawbuf->b_datap->db_base;
	else
		tp->t_unsent_ndx = 0;
	putnext(tp->t_queue, mp);
	return(1);
}

/*
 * flush the tty queues
 *
 * this is called when a M_FLUSH is received and also
 * from various places in the line discipline
 * if generate, then generate M_FLUSH messages
 *
 * Notice that a READ flush sends a FLUSHR message downstream.
 * The driver at the bottom will reflect it back upstream, causing
 * all modules to flush their read queues in their rput procedures.
 * The stream head will eat the FLUSHR message when it gets to the top.
 *
 * Conversely, a WRITE flush sends a FLUSHW message upstream.
 * The stream head will reflect this message downstream so all modules
 * will flush their write queues in their wput routines.
 * The driver will eat the FLUSHW message at the bottom.
 *
 */

int
ldtty_flush(
	register struct ldtty *tp,
	register int flags,
	register int generate
	)
{
	mblk_t	*rmp, *wmp;

	rmp = wmp = (mblk_t *) 0;
	if (generate) {

		/* We will punt if operation would be half baked */

		if (flags & FREAD) {
			rmp = allocb(1, BPRI_HI);
			if (!rmp)
				return(0);
		}
		if (flags & FWRITE) {
			if (wmp = tp->t_outbuf) {
				tp->t_outbuf = NULL;
				if (wmp->b_cont) {
					freemsg(wmp->b_cont);
					wmp->b_cont = NULL;
				}
				wmp->b_rptr = wmp->b_wptr =
					wmp->b_datap->db_base;
			} else {
				wmp = allocb(1, BPRI_HI);
				if (!wmp) {
					if (rmp)
						freeb(rmp);
					return(0);
				}
			}
		}
	}

	ldtty_flush_mp(tp, flags, rmp, wmp);
	return(flags);
}

PRIVATE_STATIC void
ldtty_flush_mp(struct ldtty *tp, int flags, mblk_t *rmp, mblk_t *wmp)
{
	if (flags & FREAD) {
		flushq(tp->t_queue, FLUSHDATA);
		tp->t_state &= ~TS_RAWBACKUP;
		if (rmp) {
			rmp->b_datap->db_type = M_FLUSH;
			*rmp->b_wptr++ = FLUSHR;
			putnext(WR(tp->t_queue), rmp);
		}
		if (tp->t_rawbuf)
			ldtty_bufreset(tp);
		if (tp->t_cc[VMIN] > 0)
			ldtty_unset_intimeout(tp);
		tp->t_shcc = 0;
		tp->t_rocount = 0;
		tp->t_rocol = 0;
		tp->t_state &= ~TS_LOCAL;
		ldtty_tblock(tp);
	}
	if (flags & FWRITE) {
		flushq(WR(tp->t_queue), FLUSHDATA);
		if (tp->t_outbuf) {
			freemsg(tp->t_outbuf);
			tp->t_outbuf = NULL;
		}
		if (wmp) {
			wmp->b_datap->db_type = M_FLUSH;
			*wmp->b_wptr++ = FLUSHW;
			putnext(tp->t_queue, wmp);
		}
	}
}

/*
 * echo the character
 */
void
ldtty_echo(register struct ldtty *tp, register int c)
{
	if ((tp->t_state & TS_CNTTB) == 0)
		tp->t_lflag &= ~FLUSHO;
	if (((tp->t_lflag & ECHO) == 0) &&
	    !((tp->t_lflag & ECHONL) && (c == '\n')))
		return;
	/*
	 * multi-byte EUC echo processing
	 */
	if (ldtty_mbenabled(tp)) {
		euctty_echo(tp, c);
		return;
	}
	if (tp->t_lflag & ECHOCTL) {
		/*
		 * echo C0
		 */
		if (((c & TTY_CHARMASK) < 0x20) && (c != '\t') && (c != '\n') ||
		    (c == 0x7f)) {
			(void)ldtty_output(tp, '^');
			c &= TTY_CHARMASK;
			if (c == 0x7f)
				c = '?';
			else
				c += 'A' - 1;
		}
		/*
		 * we won't echo C1 characters as M-^something.
		 * a lower converter module should have converted
		 * the C1 to its C0 equivalent.
		 */
	}
	if ((tp->t_lflag & XCASE) && (c == '\\')) {
		(void)ldtty_output(tp, c | TTY_QUOTE);
		return;
	}
	(void)ldtty_output(tp, c);
}

/*
 * rubout one character as cleanly as possible
 *
 * the character has already been taken out of the buffer
 */
void
ldtty_rub(register struct ldtty *tp, register int c)
{
	if ((tp->t_lflag & ECHO) == 0)
		return;
	tp->t_lflag &= ~FLUSHO;
	if (tp->t_lflag & ECHOE) {
		if (tp->t_rocount == 0) {
			/*
			 * screwed by write, retype the line
			 */
			ldtty_retype(tp);
			return;
		}
		switch (partab[c &= TTY_CHARMASK] & 077) {
		case ORDINARY:
			ldtty_rubo(tp, 1);
			break;
		case VTAB:
		case BACKSPACE:
		case CONTROL:
		case RETURN:
		case NEWLINE:
		case FF:
			if (tp->t_lflag & ECHOCTL)
				ldtty_rubo(tp, 2);
			break;
		case TAB: {
			register int savecol;
			register unsigned char *cp;
			register mblk_t *mp;
			/*
			 * if the column position got screwed, retype
			 */
			if (ldtty_mbenabled(tp)) {
				if (tp->t_rocount < euctty_rocount(tp)) {
					ldtty_retype(tp);
					return;
				}
			} else if (tp->t_rocount < tp->t_rawcc) {
				ldtty_retype(tp);
				return;
			}
			savecol = tp->t_col;
			tp->t_state |= TS_CNTTB;
			tp->t_lflag |= FLUSHO;
			tp->t_col = tp->t_rocol;
			mp = tp->t_rawbuf;
			while (mp) {
			    cp = mp->b_rptr;
			    while (cp < mp->b_wptr)
				ldtty_echo(tp, *cp++);
			    mp = mp->b_cont;
			}
			tp->t_lflag &= ~FLUSHO;
			tp->t_state &= ~TS_CNTTB;
			savecol -= tp->t_col;
			tp->t_col += savecol;
			if (savecol > 8)
				savecol = 8;
			while (--savecol >= 0)
				ldtty_output(tp, '\b');
		}
			break;
		break;
			ldtty_retype(tp);
			return;
		}
	} else if (tp->t_lflag & ECHOPRT) {
		if ((tp->t_state & TS_ERASE) == 0) {
			ldtty_output(tp, '\\');
			tp->t_state |= TS_ERASE;
		}
		ldtty_echo(tp, c);
	} else
		ldtty_echo(tp, tp->t_cc[VERASE]);
	tp->t_rocount--;
}

void
ldtty_rubo(register struct ldtty *tp, register int ct)
{
	while (--ct >= 0) {
		(void)ldtty_output(tp, '\b');
		(void)ldtty_output(tp, ' ');
		(void)ldtty_output(tp, '\b');
	}
}

PRIVATE_STATIC int
ldtty_putc(register int c, struct ldtty *tp)
{
	register mblk_t		*mp;

	if ((!tp->t_outbuf) && (!ldtty_getoutbuf(tp)))
		return(-1);

	mp = tp->t_outbuf;
	if (mp->b_wptr < mp->b_datap->db_lim) {
		*mp->b_wptr++ = (unsigned char)c;
		return(0);
	}
	return(-1);
}

/*
 * process output to the terminal
 * adding delays, expanding tabs, CR/NL, etc..
 * returns < 0 if successful
 * must be recursive
 */
int
ldtty_output(register struct ldtty *tp, register int c)
{
	register int ctype;
	register long oflag = tp->t_oflag;

	if (!(oflag & OPOST) || (c & TTY_QUOTE)) {
		if (tp->t_lflag & FLUSHO)
			return(-1);
		if (ldtty_putc(c & TTY_CHARMASK, tp))
			return(c);
		return(-1);
	}
	c &= TTY_CHARMASK;
	/*
	 * turn tabs to spaces as required
	 */
	if ((c == '\t') && ((oflag & OXTABS) || ((oflag & TABDLY) == TAB3))) {
		c = 8 - (tp->t_col & 7);
		if ((tp->t_lflag & FLUSHO) == 0) {
			register int i = 0;

			while (i < c) {
				if (ldtty_putc(' ', tp))
					break;
				i++;
			}
			c = i;
		}
		tp->t_col += c;
		return(tp->t_col & 7 ? '\t' : -1);
	}
	if ((c == CEOT) && (oflag & ONOEOT))
		return(-1);
	/*
	 * generate escapes for upper-case-only terminals
	 */
	if (tp->t_lflag & XCASE) {
		const char *ccolp = "({)}!|^~'`\\\\";
		while (*ccolp++)
			if (c == *ccolp++) {
				(void)ldtty_output(tp, '\\' | TTY_QUOTE);
				c = ccolp[-2];
				break;
			}
		if (('A' <= c) && (c <= 'Z'))
			(void)ldtty_output(tp, '\\' | TTY_QUOTE);
	}
	if ((oflag & OLCUC) && ('a' <= c) && (c <= 'z'))
		c += 'A' - 'a';
	/*
	 * turn <nl> to <cr><lf> if desired
	 */
	if ((c == '\n') && (oflag & ONLCR) && (ldtty_output(tp, '\r') >= 0))
		return(c);
	if ((c == '\r') && (oflag & ONOCR) && (tp->t_col == 0))
		return(-1);
	if ((c == '\r') && (oflag & OCRNL)) {
		c = '\n';
		if (((tp->t_lflag & FLUSHO) == 0) && ldtty_putc(c, tp))
			return('\r');
	} else {
		if (((tp->t_lflag & FLUSHO) == 0) && ldtty_putc(c, tp))
			return(c);
		if ((c == '\n') && (oflag & ONLRET))
			c = '\r';
	}

	/*
	 * calculate delays (taken from BSD tty.c)
	 *
	 * should not support any terminals which need this stuff!
	 */
	ctype = partab[c];
	c = 0;
	switch (ctype&077) {
	case ORDINARY:
		tp->t_col++;
	case CONTROL:
		break;
#define mstohz(ms)	((((ms) * hz) >> 10) & 0x0ff)
	case BACKSPACE:
		if (oflag & BSDLY)
			if (oflag & OFILL)
				c = 1;
			else
				c = mstohz(100);
		if (tp->t_col)
			tp->t_col--;
		break;
	case NEWLINE:
		ctype = oflag & NLDLY;
		if (ctype == NL2) {
			if (tp->t_col > 0) {
				c = (((unsigned int)tp->t_col) >> 4) + 3;
				if ((unsigned int)c > 6)
					c = mstohz(60);
				else
					c = mstohz(c * 10);
			}
		} else if (ctype == NL1)
			if (oflag & OFILL)
				c = 2;
			else
				c = mstohz(100);
		break;
	case TAB:
		ctype = oflag & TABDLY;
		if (ctype == TAB1) {
			c = 1 - (tp->t_col | ~07);
			if (c < 5)
				c = 0;
			else
				c = mstohz(10 * c);
		} else if (ctype == TAB2) {
			c = mstohz(200);
		} else
			c = 0;
		if (ctype && (oflag & OFILL))
			c = 2;
		tp->t_col |= 07;
		tp->t_col++;
		break;
	case VTAB:
		if (oflag & VTDELAY)
			c = 177;
		break;
	case RETURN:
		if ((oflag & ONOCR) && (tp->t_col == 0))
			return(-1);
		ctype = oflag & CRDLY;
		if (ctype == CR2)
			if (oflag & OFILL)
				c = 2;
			else
				c = mstohz(100);
		else if (ctype == CR3)
			c = mstohz(166);
		else if (ctype == CR1) {
			if (oflag & OFILL)
				c = 4;
			else {
				c = (tp->t_col >> 4) + 3;
				c = c < 6 ? 6 : c;
				c = mstohz(c * 10);
			}
		}
		tp->t_col = 0;
		break;
	case FF:
		if (oflag & FFDLY)
			c = 0177;
		break;
	}
	if (c && ((tp->t_lflag & FLUSHO) == 0)) {
		if (oflag & OFILL) {
			ctype = oflag & OFDEL ? 0x7f : 0;
			for (; c > 0; c--)
				(void)ldtty_putc(ctype, tp);
		} else {
			/*
			 * send an M_DELAY to the driver
			 * after sending any pending output
			 */
			register mblk_t *mp;

			if (!ldtty_start(tp)) {
				mp = allocb(sizeof(int), BPRI_MED);
				if (mp) {
					mp->b_datap->db_type = M_DELAY;
					*(int *)mp->b_wptr++ = c;
					putnext(WR(tp->t_queue), mp);
				}
			}
		}
	}
	return(-1);
}

/*
 * start output
 * returns 0 on success, -1 if flow controlled or waiting on buffer
 */
int
ldtty_start(register struct ldtty *tp)
{
	queue_t *q = WR(tp->t_queue);
	register mblk_t *mp;
	int size;

	/*
	 * Don't do anything here if we'll be called later
	 * by wsrv or rput.
	 */
	if (tp->t_state & (TS_WAITOUTPUT | TS_WAITOUTBUF | TS_TTSTOP))
		return(-1);
	/*
	 * Stop for driver flow control
	 */
	if (!canput(q->q_next)) {
		tp->t_state |= TS_WAITOUTPUT;
		return(-1);
	}
	/*
	 * Send any non-empty message
	 */
	mp = tp->t_outbuf;
	if (size = ldtty_msgdsize(mp)) {
		/*
		 * send chars in outbuf down to the driver
		 */
		tp->t_outbuf = NULL;
		mp->b_datap->db_type = M_DATA;
		/*
		 * Count output bytes
		 */
		tk_nout += size;
		putnext(q, mp);
	}
	/*
	 * Awaken sleepers when drained.
	 */
	if (q->q_first)
		qenable(q);
	else if (tp->t_state & TS_ASLEEP) {
		tp->t_state &= ~TS_ASLEEP;
		wakeup(&tp->t_outbuf);	    
	}
	return(0);
}

/*
 * retype the characters in the raw buffer
 */
void
ldtty_retype(register struct ldtty *tp)
{
	register unsigned char *cp;
	register mblk_t *mp;

	if (tp->t_cc[VREPRINT] != ((cc_t)_POSIX_VDISABLE))
		ldtty_echo(tp, tp->t_cc[VREPRINT]);
	ldtty_output(tp, '\n');
	mp = tp->t_rawbuf;
	while (mp) {
	    	cp = mp->b_rptr;
	    	while (cp < mp->b_wptr)
			ldtty_echo(tp, *cp++);
	    	mp = mp->b_cont;
	}
	tp->t_state &= ~TS_ERASE;
	if (ldtty_mbenabled(tp))
		tp->t_rocount = euctty_rocount(tp);
	else
		tp->t_rocount = tp->t_rawcc;
	tp->t_rocol = 0;
}

/*
 * Block input if current buffer is getting full, unblock if
 * space is returning.
 *
 * Special treatment for ICANON is for a bug (#5894) in which
 * IXOFF under ICANON hung, since we didn't know whether there
 * was any data at all at the stream head, that is, whether the
 * opportunity existed for the rawcc to return to the low water
 * mark without further keyboard input (such as ^C or ^U).
 * The mechanism for handling IXOFF correctly in raw mode
 * operates with M_NOTIFY messages awakening us when the
 * stream head handles data, so this is just an ICANON problem.
 * For now, we can only perform such flow control when the
 * upstream queues are full, and we can restart in service.
 */
PRIVATE_STATIC void
ldtty_tblock(register struct ldtty *tp)
{
	queue_t *q = tp->t_queue;
	int will_restart = (tp->t_lflag & ICANON) ?
				!canput(q->q_next) : (tp->t_shcc > 0);

	if (tp->t_state & TS_TBLOCK) {
		if ((tp->t_rawcc < LDTTYLOWAT || !will_restart) &&
		    putctl(WR(q)->q_next, M_STARTI))
			tp->t_state &= ~TS_TBLOCK;
	} else {
		if (tp->t_rawcc >= LDTTYHIWAT && will_restart &&
		    putctl(WR(q)->q_next, M_STOPI))
			tp->t_state |= TS_TBLOCK;
	}
}

/*
 *
 * Function description: ldtty_wakeup
 *
 *      Called when we want to send non-canonical input upstream
 *	Never called in the case of canonical input
 * 
 * Arguments:
 *
 *      tp - pointer to ldtty structure
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *	pending read timeout cancelled
 *      current rawbuf sent upstream.
 * 	new buffer allocated.
 *
 */
PRIVATE_STATIC void
ldtty_wakeup(register struct ldtty *tp)
{
	ldtty_unset_intimeout(tp);
        tp->t_state &= ~TS_VTIME_FLAG;
	/*
	 * We're not buffering input characters
	 * Just send current buffer upstream
	 */
	ldtty_sendraw(tp);
}

/*
 * called by timeout
 */
PRIVATE_STATIC void
ldtty_intimeout(register struct ldtty *tp)
{
	/* The timeout state (consisting of a non-NULL tp->t_intimeout field
	 * and a set TS_INTIMEOUT bit in tp->t_state) will be cleared
	 * when the service procedure calls ldtty_check_intimeout().  We
	 * can't do it here since we're not protected by streams locking.
	 */
	ldtty_post_intimeout(tp);
	qenable(tp->t_queue);
}

/*
 * write user data
 * if we completely process the message, free it and return 0
 * if we cannot completely process the message because of flow control
 * or allocb() failure, put the unprocessed part of message back on
 * the queue and return -1
 */
PRIVATE_STATIC int
ldtty_write(register struct ldtty *tp, register mblk_t *mp)
{
	mblk_t 		*mp1;
	queue_t 	*q = WR(tp->t_queue);

	if (tp->t_lflag & FLUSHO) {
		freemsg(mp);
		return(0);
	}
	/*
	 * if no output processing required, just forward the message
	 */
	if (!(tp->t_oflag & OPOST)) {
		/*
		 * send the accumulated message in t_outbuf first
		 */
		if (ldtty_start(tp))
			goto flow;
		tp->t_rocount = 0;
		/*
		 * Count output bytes
		 */
		tk_nout += msgdsize(mp);

		putnext(q, mp);
		return(0);
	}
	/*
	 * multi-byte EUC write processing
	 */
	if (ldtty_mbenabled(tp))
		return(euctty_write(tp, mp));
	/*
	 * some output processing required
	 */
	while (mp) {
                register int 	cc;
		register int	ce;
		register int	i;

		cc = mp->b_wptr - mp->b_rptr;
		while (cc > 0) {
                        if ((tp->t_oflag & OLCUC) ||
                            (tp->t_lflag & XCASE))
                                /* 
                                 * Process all the characters 
                                 * one by one 
                                 */
                                ce = 0;
                        else {
				/* Try to grab a bunch of normal
				 * chars to process all at once.
				 */
                                ce = cc - scanc((unsigned)cc,
                                                mp->b_rptr,
                                                (u_char *)partab, 077);
			}

			tp->t_rocount = 0;
                        if (ce == 0) {
                                i = (unsigned char)*mp->b_rptr;
                                if ((i = ldtty_output(tp, i)) >= 0) {
					/* If output buffer is full, send
					 * it down stream and try this char
					 * again, but punt if there's still
					 * some holdup (could be allocb()
					 * failure or flow control).
					 */
                                        if (ldtty_start(tp)
					|| ((i = ldtty_output(tp, i)) >= 0)) {
                                                *mp->b_rptr = (unsigned char)i;
						goto flow;
                                        }
                                }
                                mp->b_rptr++;
                                cc--;
                        } else {
                                /*
                                 * A bunch of normal characters have
                                 * been found, transfer them en masse
                                 * to the output buffer and continue
                                 * processing at the top of the loop.
                                 * If there are any further characters
                                 * in this message block, the first
                                 * should be a character requiring
                                 * special handling by ldtty_output.
                                 */
                                i = ldtty_b_to_m(mp->b_rptr, ce, tp);
				if ((i == ce) && (tp->t_state & TS_WAITOUTBUF))
					goto flow;
                                ce -= i;
                                tp->t_col += ce;
                                mp->b_rptr += ce;
                                cc -= ce;
				/*
				 * If output full, try to send it
				 */
                                if (i > 0 && ldtty_start(tp))
					goto flow;
                        }
		}

		mp1 = mp;
		mp = mp->b_cont;
		freeb(mp1);
	}
	if (ldtty_start(tp) == 0)
		return 0;
flow:
	/* flow controlled, return */
	if (mp)
		putbq(q, mp);
	return (-1);
}

/*
 *
 * Function description: ldtty_sendcanon
 *
 *      Send a completed line upstream.
 *      Strip the trailing EOF character if there is one.
 *
 * Arguments:
 *
 *      tp - pointer to current ldtty struct
 *
 * Return value:
 *
 *      None
 *
 * Side effects:
 *
 *	Input blockage may be cleared
 *
 */
PRIVATE_STATIC void
ldtty_sendcanon(register struct ldtty *tp)
{
	register mblk_t *mp, *mp1;
	register u_char c;
	int cc;

	mp = tp->t_rawbuf;
	mp1 = tp->t_rawtail;
	cc = tp->t_rawcc;

	ldtty_bufclr(tp);

	if (cc) {
		c = *(mp1->b_wptr - 1);      /* Look at last character */
		if (CCEQ(tp->t_cc[VEOF], c)) /* If it's EOF */
			mp1->b_wptr--;       /* Remove it */
		mp->b_flag &= ~MSGCOMPRESS;
                putnext(tp->t_queue, mp);
	}
        ldtty_tblock(tp);
}

/*
 *
 * Function description: ldtty_sendraw
 * 
 * Send the raw buffer upstream.
 * 
 * In order to perform IXOFF processing correctly, and in order to cook
 * unread raw data if an application switches into canon mode, the line
 * discipline needs to keep its hands on unread raw data, even once VMIN
 * has been satisfied.  In order to perform select, read, and getmsg
 * processing correctly, the stream head also needs to have its hands on
 * unread raw data, but only once VMIN is satisfied.  In other words, both
 * the line discipline and the stream head would like to "own" the raw
 * input buffer.
 * 
 * The solution to this problem is to hold the buffer in the line
 * discipline until VMIN is satisfied, and then maintain the buffer in
 * both places (i.e., in duplicate) until a read or flush brings the
 * buffer size below VMIN, at which time the line discipline reclaims
 * sole ownership.
 * 
 * To accomplish this, we always retain unread data, but send a copy to
 * the stream head (note that we won't be called at all until VMIN is
 * satisfied or VTIME has expired).  The copy is marked as special via the
 * MSGNOTIFY bit in the M_DATA message.  When the stream head delivers
 * marked data to a user (e.g., in read(2) processing), it sends an
 * M_NOTIFY message downstream with a count of how many marked characters
 * were read.  This tells the line discipline (ldtty_mnotify()) to chop
 * off the beginning portion of the local copy of the raw buffer
 * corresponding to the data that was just read.  Note that M_NOTIFY
 * is not a standard V.4 message type.
 * 
 * Upon each entry to the ldtty_sendraw() function, the raw buffer
 * consists of some number of bytes (possibly 0) of which we've already
 * sent a copy to the stream head, followed by some number of "newly
 * arrived" bytes (also possibly 0).  The t_unsent and t_unsent_ndx
 * fields in the ldtty structure together identify the start of the newly
 * arrived region, which is the portion that ldtty_sendraw() must send.
 *
 * Arguments:
 *
 *      tp - pointer to current ldtty struct
 *
 * Return value:
 *
 *	None
 *
 * Side effects:
 *
 *	Input blockage may be cleared.
 *
 */

PRIVATE_STATIC void
ldtty_sendraw(register struct ldtty *tp)
{
	register mblk_t *mp, *mp1;
	register int wanted = tp->t_rawcc - tp->t_shcc;

	mp = tp->t_sparebuf;
	if (tp->t_rawbuf && wanted > 0) {
		if (mp && (mp->b_datap->db_size >= wanted)) {
#if MACH_ASSERT
			tp->t_sparehit++;
			tp->t_rawtrace = 1;
#endif
			tp->t_sparebuf = NULL;
		} else {
#if MACH_ASSERT
			tp->t_sparemiss++;
			tp->t_rawtrace = 2;
#endif
			if (!(mp = allocb(wanted, BPRI_MED)))
				return;
		}
		ldtty_copymsg(tp->t_unsent, tp->t_unsent_ndx, wanted, mp);
		mp1 = tp->t_unsent = tp->t_rawtail;
		tp->t_unsent_ndx = (mp1->b_wptr - mp1->b_datap->db_base);
	} else {
#if MACH_ASSERT
		tp->t_rawtrace = 3;
#endif
		/*
		 * In theory, this should be an assertion, however
		 * experience shows it to be not uncommon on MP's.
		 * In any event, with data at the stream head,
		 * there's nothing to do.
		 */
		if (tp->t_shcc)
			return;

		/* Terminate the read with a 0-length message. */
		if (mp) {
			tp->t_sparebuf = NULL;
			mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		} else if (!(mp = allocb(0, BPRI_MED)))
			return;
	}
	tp->t_shcc = tp->t_rawcc;
	ASSERT(msgdsize(mp) == wanted);
	putnext(tp->t_queue, mp);
}

/*
 * ldtty_mnotify:
 *
 *	Remove front portion of raw input buffer to sync up w/ stream
 *	head.  See comment above ldtty_sendraw().
 */

PRIVATE_STATIC void
ldtty_mnotify(struct ldtty *tp, mblk_t *mp)
{
	mblk_t	*mp1;
	int	count = *((long *)mp->b_rptr);
	
	ASSERT(count <= tp->t_rawcc);
	ASSERT(count <= tp->t_shcc);

	mp1 = tp->t_rawbuf;

	if (count >= tp->t_rawcc) {
		/* Delete entire raw buffer */
		ASSERT(tp->t_rawbuf != ((mblk_t *) 0));
		ldtty_bufreset(tp);
	} else {
		if (!(adjmsg(tp->t_rawbuf, count)))
			panic("ldtty_mnotify: adjmsg");

		/* Trim off empty messages that resulted from the adjmsg().
		 * Since we've determined count to be less than t_rawcc,
		 * we're "guaranteed" to exit the following while loop
		 * before tp->t_rawbuf becomes NULL.
		 */
		while (tp->t_rawbuf->b_wptr == tp->t_rawbuf->b_rptr) {
			mp1 = tp->t_rawbuf;
			tp->t_rawbuf = tp->t_rawbuf->b_cont;
			freeb(mp1);
		}
		tp->t_rawcc -= count;
	}
	if ((tp->t_shcc -= count) < 0)
		tp->t_shcc = 0;

	/* If stream head count has become "unsatisfied", then
	 * flush it -- we don't want reads or selects to find data there.
	 */
	if (tp->t_shcc && (tp->t_shcc < tp->t_cc[VMIN]))
		ldtty_flush_shead(tp, mp);
	else
		freemsg(mp);

	if ((tp->t_state & TS_RAWBACKUP) && (tp->t_shcc < LDTTYLOWAT)) {
		tp->t_state &= ~TS_RAWBACKUP;
		qenable(tp->t_queue);
	}
        ldtty_tblock(tp);
}

/*
 * Process M_BREAK message from read service routine
 */
PRIVATE_STATIC void
ldtty_break(struct ldtty *tp, mblk_t *mp)
{
        register int c;
        int err;
        int post_wakeup = 0;
        int iflag = tp->t_iflag;

        /*
         * This is either parity error or BREAK character
         * If message doesn't contain an int,
         * assume it's just a plain BREAK event
         * Can't use msgdsize() because it doesn't work
         * on M_BREAK message blocks
         */
        if ((mp->b_wptr - mp->b_rptr) < sizeof(int))
                c = TTY_FE; /* if no data, it's a BREAK */
        else
                c = *(int *)mp->b_rptr;
	freemsg(mp);

        err = (c&TTY_ERRORMASK);
        c &= TTY_CHARMASK;
        if (err) {
                if ((err&TTY_FE) && !c) {	/* break */
                        if (iflag&IGNBRK)
                                return;
                        else if (iflag&BRKINT) {
                                (void) ldtty_flush(tp, FREAD|FWRITE, 1);
                                putctl1(tp->t_queue->q_next, M_PCSIG, SIGINT);
                                return;
                        } else
                                goto parmrk;
                } else if (((err&TTY_PE) && (iflag&INPCK)) || (err&TTY_FE)) {
                        if (iflag&IGNPAR)
                                return;
                        else if ((iflag&PARMRK) == 0)
                                c = 0;
                        else
                                c |= TTY_QUOTE;
parmrk:                 if (iflag&PARMRK) {
                        	post_wakeup |= ldtty_input(tp, 0377|TTY_QUOTE);
                                post_wakeup |= ldtty_input(tp, 000|TTY_QUOTE);
                        }
                }
        }
        post_wakeup |= ldtty_input(tp, c);
	if ((post_wakeup & T_POST_FLUSH) == 0)
        	ldtty_post_input(tp, post_wakeup);
}

PRIVATE_STATIC int
ldtty_mhangup(
	register struct ldtty *tp,
	register queue_t *q,
	register mblk_t *mp
	)
{
	mblk_t *new_mp = allocb(2, BPRI_HI);

	if (!new_mp) {
		if (tp->t_hupbid)
			unbufcall(tp->t_hupbid);
		tp->t_hupbid = bufcall(2, BPRI_HI, qenable, q);
		return(0);
	}
	new_mp->b_datap->db_type = M_ERROR;
	*new_mp->b_wptr++ = 0;
	*new_mp->b_wptr++ = EIO;
	putnext(q, new_mp);
	putnext(q, mp);
	return(1);
}

/*
 * Process the M_CTL messages from the read put routine
 * or from the write put routine (in case of TIOCSTI).  The
 * calling put routine has already checked canput(q->q_next).
 * Return 1 if message processed, 0 otherwise.  The zero return
 * would come from a memory request failing below us.
 */

PRIVATE_STATIC int
ldtty_mctl(
	register struct ldtty *tp,
	register queue_t *q,
	register mblk_t *mp
	)
{
	register struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;
	switch (iocp->ioc_cmd) {
	case MC_DO_CANON:
		/*
		 * the pty will be out of remote mode
		 */
                tp->t_state &= ~TS_NOCANON;
		putnext(q, mp);
		return(1);
	case MC_NO_CANON:
		/*
		 *  the pty will be in remote mode
		 */
                tp->t_state |= TS_NOCANON;
		putnext(q, mp);
		return(1);
        case TIOCSETA:
		/*
		 * Pty master wants us to change termios settings.
		 */
		if (ldtty_ioctl_ack(tp, q, mp)) {
			freemsg(mp);
			return(1);
		} else
			return(0);
        case TIOCSWINSZ:
		/*
		 * Driver (probably a pty) wants us to change window size.
		 */
		if (ldtty_swinsz(tp, mp)) {
			freemsg(mp);
			return(1);
		} else
			return(0);
	case TIOCSTI:
		/*
		 * Simulate typein (sent by upstream character conversion
		 * module)
		 */
		ldtty_sti(tp, mp);
		freemsg(mp);
		return(1);
	default:
		putnext(q, mp);
		return(1);
	}
}

/*
 * Attempt to copy cc characters from cp to tp->t_outbuf and return
 * the residual count.  A residual may exist due either to flow control
 * or failed ldtty_getoutbuf().  If it's the latter, our caller
 * will know by the TS_WAITOUTBUF bit in the tp structure.
 */
PRIVATE_STATIC int
ldtty_b_to_m(register unsigned char *cp, register int cc, struct ldtty *tp)
{
        register int	nc;
	mblk_t		*mp;

	if ((!tp->t_outbuf) && (!ldtty_getoutbuf(tp)))
		return(cc);

	mp = tp->t_outbuf;
        if (cc <= 0)
                return (0);
        nc = MIN(cc, (mp->b_datap->db_lim - mp->b_wptr));
        if (nc > 0) {
                bcopy(cp, mp->b_wptr, nc);
                mp->b_wptr += nc;
                cc -= nc;
        }
        return (cc);
}

int
ldtty_stuffc(register int c, struct ldtty *tp)
{
	mblk_t *mp = tp->t_rawtail;

	if (!mp) {
		/*
     		 * No message blocks yet
     		 */
		if (tp->t_sparebuf) {
			mp = tp->t_sparebuf;
			tp->t_sparebuf = NULL;
			mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		} else {
			mp = allocb(LDTTYCHUNKSIZE, BPRI_MED);
			if (!mp)
			return (-1);
		}
		ldtty_newrawbuf(tp, mp);
	} else if (mp->b_wptr >= mp->b_datap->db_lim) {
		/* No room in current mblk
     		 * allocate a new one
     		 */
		if (tp->t_sparebuf) {
			mp = tp->t_sparebuf;
			tp->t_sparebuf = NULL;
			mp->b_rptr = mp->b_wptr = mp->b_datap->db_base;
		} else {
			mp = allocb(LDTTYCHUNKSIZE, BPRI_MED);
			if (!mp)
			return (-1);
		}
		tp->t_rawtail->b_cont = mp;
		tp->t_rawtail = mp;
	}
	*mp->b_wptr++ = (unsigned char)c;
	tp->t_rawcc++;
	return (0);
}

int
ldtty_unstuffc(struct ldtty *tp)
{
	register int c;
	mblk_t *mp = tp->t_rawtail;

	if (tp->t_rawcc == 0)
		c = -1;
	else {
		c = *--mp->b_wptr;
		if (--tp->t_rawcc == 0) {
			/*
       			 * No characters left.
       			 */
			ldtty_bufreset(tp);
		} else if (mp->b_wptr <= mp->b_rptr) {
			/*
       			 * Took last character out of this mblk,
       			 * Deallocate it.  Have to walk forward
       			 * through the chain starting with rawbuf
       			 */
			mblk_t *mp1 = tp->t_rawbuf;

			freeb(mp);
			while (mp1->b_cont != mp)
				mp1 = mp1->b_cont;
			mp1->b_cont = NULL;
			tp->t_rawtail = mp1;
		}
	}
	return (c);
}

/*
 *
 * Function description: ldtty_post_input
 *
 *	Perform common operations after looping through ldtty_input.
 * 	Operations are:
 * 		Start output
 * 		Send input upstream (ldtty_wakeup)
 * 		(re)start inter-character timer
 *
 * Arguments:
 *
 *      tp - pointer to current ldtty structure
 * 	post_wakeup - flags that indicate what to do
 *
 * Return value:
 *
 *	None
 *
 * Side effects:
 *
 *	Listed under "operations" above
 *
 */
PRIVATE_STATIC void
ldtty_post_input(struct ldtty *tp, int post_wakeup)
{
        if (post_wakeup & T_POST_START)
                /*
                 * Start output
                 */
                (void)ldtty_start(tp);

        if (post_wakeup & T_POST_WAKEUP)
                /*
                 * Send input upstream
                 * No need to set timer in this case
                 */
                ldtty_wakeup(tp);

        else if (post_wakeup & T_POST_TIMER) {
                /*
                 * If we're in non-canonical mode,
                 * and VTIME > 0 and a read() has been issued
                 * then (re)start intercharacter timer.
                 */
                if (!(tp->t_lflag & ICANON) &&       /* non-canonical */
                    (tp->t_shad_time > 0) &&         /* and VTIME > 0 */
                    (tp->t_state & TS_VTIME_FLAG)) { /* and read issued */
			ldtty_set_intimeout(tp, tp->t_shad_time);
                }
        }
}

PRIVATE_STATIC int
ldtty_swinsz(struct ldtty *tp, mblk_t *mp)
{
	if (bcmp(&tp->t_winsize, (struct winsize *)mp->b_cont->b_rptr,
		 				sizeof(struct winsize)))
		if (putctl1(tp->t_queue->q_next, M_PCSIG, SIGWINCH)) {
			tp->t_winsize = *(struct winsize *)mp->b_cont->b_rptr;
			return(1);
		} else
			return(0);
	return(1);
}

PRIVATE_STATIC void
ldtty_sti(struct ldtty *tp, mblk_t *mp)
{
	mblk_t *mp1;

	mp1 = unlinkb(mp);
	if (mp1)
		puthere(tp->t_queue, mp1);
}

/*
 * ldtty_copymsg:
 * 
 * Copy some number of bytes from a supplied mblk chain to a target,
 * possibly omitting some beginning portion of the source mblk in the
 * copy. This routine is tailored to the needs of ldtty_sendraw().
 * 
 * The caller is responsible for verifying that the supplied target mblk
 * is big enough.  The target's b_cont pointer should be NULL on entry to
 * this routine; we don't do b_cont processing for the target.
 */

PRIVATE_STATIC void
ldtty_copymsg(register mblk_t *from, int from_index, int from_len, register mblk_t *to)
{
	register int	count;

	ASSERT(to->b_cont == NULL);
	to->b_rptr = to->b_wptr = to->b_datap->db_base;
	to->b_flag |= MSGNOTIFY;	/* COMPRESS is done at stream head */
	do {
		count = (from->b_wptr - from->b_datap->db_base) - from_index;
		ASSERT(count >= 0);
		ASSERT((to->b_wptr + count) <= to->b_datap->db_lim);
		bcopy(from->b_datap->db_base + from_index, to->b_wptr, count);
		to->b_wptr += count;
		from_len -= count;
		from_index = 0;
	} while (from = from->b_cont);
	ASSERT(from_len == 0);
}

int
ldtty_getoutbuf(struct ldtty *tp)
{
	if (tp->t_outbuf)
		return(1);

	if (tp->t_state & TS_WAITOUTBUF)
		return(0);

	if (tp->t_outbid)
		unbufcall(tp->t_outbid);

	if (tp->t_outbuf = allocb(LDTTYMAX, BPRI_MED)) {
		tp->t_outbid = 0;
		return(1);
	} else {
		tp->t_state |= TS_WAITOUTBUF;
		tp->t_outbid = 
			bufcall(LDTTYMAX, BPRI_MED, qenable, WR(tp->t_queue));
		return(0);
	}
}

#ifdef LDTTY_DEBUG
/*
 * print tty info
 */
PRIVATE_STATIC void
ldtty_info(register struct ldtty *tp)
{
	ldtty_putstr(tp, "ldtty status\n");
	ldtty_putstr(tp, "t_termios");
	ldtty_putstr(tp, " c_iflag=0x");  ldtty_putint(tp, tp->t_iflag, 16);
	ldtty_putstr(tp, " c_oflag=0x");  ldtty_putint(tp, tp->t_oflag, 16);
	ldtty_putstr(tp, " c_cflag=0x");  ldtty_putint(tp, tp->t_cflag, 16);
	ldtty_putstr(tp, " c_lflag=0x");  ldtty_putint(tp, tp->t_lflag, 16);
	ldtty_putstr(tp, "\n");

	ldtty_putstr(tp, "t_cswidth=");
	ldtty_putint(tp, tp->t_cswidth.eucw[1], 10);  ldtty_putstr(tp, ":");
	ldtty_putint(tp, tp->t_cswidth.scrw[1], 10);  ldtty_putstr(tp, ",");
	ldtty_putint(tp, tp->t_cswidth.eucw[2], 10);  ldtty_putstr(tp, ":");
	ldtty_putint(tp, tp->t_cswidth.scrw[2], 10);  ldtty_putstr(tp, ",");
	ldtty_putint(tp, tp->t_cswidth.eucw[3], 10);  ldtty_putstr(tp, ":");
	ldtty_putint(tp, tp->t_cswidth.scrw[3], 10);  ldtty_putstr(tp, "\n");

	ldtty_putstr(tp, "t_state=0x");
	ldtty_putint(tp, tp->t_state, 16);
	ldtty_putstr(tp, " t_shad_time=");
	ldtty_putint(tp, tp->t_shad_time, 10);
	ldtty_putstr(tp, "\n");
}

PRIVATE_STATIC void
ldtty_putstr(register struct ldtty *tp, register char *s)
{
	while (*s)
		ldtty_output(tp, *s++);
}

PRIVATE_STATIC void
ldtty_putint(
	register struct ldtty *tp,
	register unsigned int x, 	/* the int to print */
	register int base		/* the base of the displayed value */
	)
{
	char info[16];
	register char *p = info;

	if (!x) {
		ldtty_output(tp, '0');
		return;
	}
	while (x) {
		*p++ = "0123456789abcdef"[x % base];
		x /= base;
	}
	while (p > info)
		ldtty_output(tp, *--p);
}
#endif /* LDTTY_DEBUG */
