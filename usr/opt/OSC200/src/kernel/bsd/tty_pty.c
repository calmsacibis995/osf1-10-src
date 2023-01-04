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
static char *rcsid = "@(#)$RCSfile: tty_pty.c,v $ $Revision: 4.6.25.8 $ (DEC) $Date: 1994/01/21 23:26:29 $";
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1982, 1986, 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *

 */

/*
 * Pseudo-teletype Driver
 * (Actually two drivers, requiring two entries in 'cdevsw')
 *
 * Modification History:
 *
 *
 * 31-Aug-91	Fred Canter
 *	Make NPTY configurable.
 *
 * 7-sep-1991   Brian Harrigan
 *              Undid changes of 30-aug as macros were changed in .h file
 *
 * 30-AUG-91    Brian Harrigan
 *              CHanged TTY_LOCK macro to avoid race condition in RT code
 *              for EFT. Hack. SHould be changed in .h file instead... 
 *
 * 6-Aug-91	Fred Canter
 *	Hardwire NPTY to 176 for EFT. It will be configurable in BL7.
 *	The maximum number of pseudo ttys /dev/MAKEDEV can make is 176.
 *
 * 7-May-91	Ron Widyono
 *	Enabled NCPUS > 1 code for RT_PREEMPT.
 */

#include <data/tty_pty_data.c>
#include <rt_preempt.h>


#if	NPTY > 0

#define BUFSIZ		100		/* Chunk size iomoved to/from user */
#define CLONE_DEV	minor(NODEV)

#define PF_NBIO		0x04
#define PF_PKT		0x08		/* packet mode */
#define PF_STOPPED	0x10		/* user told stopped */
#define PF_REMOTE	0x20		/* remote and flow controlled input */
#define PF_NOSTOP	0x40
#define PF_UCNTL	0x80		/* user control mode */

#define pt_tty_to_pt_ioctl(x) ((struct pt_ioctl *)((char *)(x) + \
				((char *)&((struct pt_data *)0)->pt_ioctl - \
				 (char *)&((struct pt_data *)0)->pt_tty)))
#define old_isptm	0x40047447

int pty_choose(dev_t *devp);
int pty_alloc(dev_t dev, struct pt_data **ptdpp);
void pty_free(struct pt_data *ptdp, dev_t dev);
int pty_ref(dev_t dev, struct pt_data **ptdpp);
void pty_unref(struct pt_data *ptdp);

/*ARGSUSED*/
ptsopen(dev, flag)
	dev_t dev;
	long flag;
{
	register struct tty *tp;
	struct pt_data *ptdp;
	int error;
#if	SEC_ARCH
	int mode;
#endif

	if (error = pty_alloc(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	TTY_LOCK(tp);
	if ((tp->t_state & TS_ISOPEN) == 0) {
		ttychars(tp);		/* Set up default chars */
		bzero((caddr_t)&tp->t_winsize, sizeof(tp->t_winsize));
		tp->t_iflag = TTYDEF_IFLAG;
		tp->t_oflag = TTYDEF_OFLAG;
		tp->t_lflag = TTYDEF_LFLAG;
		tp->t_cflag = TTYDEF_CFLAG;
		tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
		ttsetwater(tp);		/* would be done in xxparam() */
	} else if (tp->t_state&TS_XCLUDE
#if	SEC_BASE
		   && !privileged(SEC_ALLOWDACACCESS, 0)
#else
		   && u.u_uid != 0
#endif
					) {
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
		return(EBUSY);
	}
	if (tp->t_oproc)			/* Ctrlr still around. */
		tp->t_state |= TS_CARR_ON;
	/* Let X work:  Don't wait if O_NDELAY is on. */
	/* was if (flag & O_NDELAY) */
	if (flag & (O_NDELAY|O_NONBLOCK)) {
		tp->t_state |= TS_ONDELAY;
	} else {
		while ((tp->t_state & TS_CARR_ON) == 0) {
			tp->t_state |= TS_WOPEN;
			/* was if (flag&FNDELAY) */
			if (flag&(FNDELAY|FNONBLOCK))
			        break;
			if (error = ttysleep(tp, (caddr_t)&tp->t_rawq, 
					     TTIPRI | PCATCH, ttopen))
				goto out;
		}
	}
#if	SEC_ARCH
	if (security_is_on) {
		mode = 0;
		if (flag & FREAD)
			mode |= SP_READACC;
		if (flag & FWRITE)
			mode |= SP_WRITEACC;
		if (SP_ACCESS(SIP->si_tag, PTSTAG(dev, 0), mode, NULL))
			error = u.u_error;
			goto out;
		}
	}
#endif
	error = (*linesw[tp->t_line].l_open)(dev, tp, flag);
	ptcwakeup(tp, FREAD|FWRITE);
	if (error)
		goto out;
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(0);
out:
	if (!tp->t_oproc) {
		TTY_UNLOCK(tp);
		pty_free(ptdp, dev);
	} else {
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
	}
	return(error);
}

ptsclose(dev)
	dev_t dev;
{
	register struct tty *tp;
	struct pt_data *ptdp;
	int error;

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	TTY_LOCK(tp);
	(*linesw[tp->t_line].l_close)(tp);
	ttyclose(tp);
	ptcwakeup(tp, FREAD|FWRITE);
	if (!tp->t_oproc) {
		TTY_UNLOCK(tp);
		pty_free(ptdp, dev);
	} else {
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
	}
	return(0);
}

ptsselect(dev, events, revents, scanning)
	dev_t dev;
	short *events, *revents;
	int scanning;
{
	register struct tty *tp;
	struct pt_data *ptdp;
	int error;

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	TTY_LOCK(tp);
	tpselect(tp,events,revents,scanning);
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(0);
}

ptsread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	long flag;
{
	register struct tty *tp;
	register struct proc *p;
	struct pt_data *ptdp;
	int error;

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	TTY_LOCK(tp);
again:
	if (ptdp->pt_ioctl.pt_flags & PF_REMOTE) {
                p = u.u_procp;
                while (ISBACKGROUND(p, tp)) {
                        register struct pgrp    *pg;
                        sigset_t                p_sigmask;

                        pg = p->p_pgrp;
                        (void)PG_REF(pg);

                        p_sigmask = p->p_sigmask;
                        if (sigismember(&p->p_sigignore, SIGTTIN) ||
                            sigismember(&p_sigmask, SIGTTIN) ||
                            pg->pg_jobc == 0) {
                                PG_UNREF(pg);
                                TTY_UNLOCK(tp);
                                return (EIO);
                        }

                        pgsignal_tty(pg, SIGTTIN, 1, 1);
                        PG_UNREF(pg);
                        if (error = ttysleep(tp, (caddr_t)&lbolt,
                                        TTIPRI | PCATCH, ttybg)) {
                                goto out;
                        }
		}
		if (tp->t_canq.c_cc == 0) {
			if (flag & (IO_NDELAY|IO_NONBLOCK)) {
				error = EWOULDBLOCK;
				goto out;
			}
			if (error = ttysleep(tp, (caddr_t)&tp->t_canq,
					     TTIPRI | PCATCH, ttyin))
				goto out;
			goto again;
		}
		while (tp->t_canq.c_cc > 1 && uio->uio_resid > 0)
			if (ureadc(getc(&tp->t_canq), uio) < 0) {
				error = EFAULT;
				break;
			}
		if (tp->t_canq.c_cc == 1)
			(void) getc(&tp->t_canq);
		if (tp->t_canq.c_cc)
			goto out;
	} else
		if (tp->t_oproc)
			error = (*linesw[tp->t_line].l_read)(tp, uio, flag);
	/*
	 * this last check is need in case l_read was interrupted
	 * by a signal
	 */
	if (tp->t_oproc)
		ptcwakeup(tp, FWRITE);
out:
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(error);
}

/*
 * Write to pseudo-tty.
 * Wakeups of controlling tty will happen
 * indirectly, when tty driver calls ptsstart.
 */
ptswrite(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	long flag;
{
	register struct tty *tp;
	struct pt_data *ptdp;
	int error;

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	TTY_LOCK(tp);
	if (!tp->t_oproc) {
		error = EIO;
		goto out;
	}
	error = (*linesw[tp->t_line].l_write)(tp, uio, flag);
out:
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(error);
}

/*
 * Start output on pseudo-tty.
 * Wake up process selecting or sleeping for input from controlling tty.
 */
ptsstart(tp)
	struct tty *tp;
{
	struct pt_ioctl *pti;

	LASSERT(TTY_LOCK_HOLDER(tp));
	if (tp->t_state & TS_TTSTOP)
		return;
	pti = pt_tty_to_pt_ioctl(tp);
	if (pti->pt_flags & PF_STOPPED) {
		pti->pt_flags &= ~PF_STOPPED;
		pti->pt_send = TIOCPKT_START;
	}
	ptcwakeup(tp, FREAD);
}

ptcwakeup(tp, flag)
	struct tty *tp;
	long flag;
{
	LASSERT(TTY_LOCK_HOLDER(tp));
	select_wakeup(&pt_tty_to_pt_ioctl(tp)->pt_selq);
	if (flag & FREAD)
		thread_wakeup((vm_offset_t)&tp->t_outq.c_cf);
	if (flag & FWRITE)
		thread_wakeup((vm_offset_t)&tp->t_rawq.c_cf);
}

/*ARGSUSED*/
ptcopen(dev, flag, cflag, newdevp)
	dev_t	dev;
	long	flag, cflag;
	dev_t	*newdevp;
{
	register struct tty *tp;
	struct pt_data *ptdp;
	int error;

	if (minor(dev) == CLONE_DEV) {
		if ((flag & O_DOCLONE) == 0)
			return(ECLONEME);
		if (error = pty_choose(&dev))
			return(error);
		*newdevp = dev;
	}
	if (error = pty_alloc(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	TTY_LOCK(tp);
	if (tp->t_oproc) {
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
		return(EIO);
	}
#if	SEC_ARCH
	if (security_is_on) {
	bzero(PTCTAG(dev, 0), SEC_NUM_TAGS * sizeof(tag_t));
	SP_OBJECT_CREATE(SIP->si_tag, PTCTAG(dev, 0), (tag_t) 0, SEC_OBJECT,
		(dac_t *) 0, (mode_t) 0);
	bzero(PTSTAG(dev, 0), SEC_NUM_TAGS * sizeof(tag_t));
	SP_OBJECT_CREATE(SIP->si_tag, PTSTAG(dev, 0), (tag_t) 0, SEC_OBJECT,
		(dac_t *) 0, (mode_t) 0);
	}
#endif
	tp->t_oproc = ptsstart;
	tp->t_dev = dev; /* needed for ISPTM ioctl before slave is opened */
	(void)(*linesw[tp->t_line].l_modem)(tp, 1);
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(0);
}

ptcclose(dev)
	dev_t dev;
{
	register struct tty *tp;
	struct pt_data *ptdp;
	struct session *s;
	int error;

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	TTY_LOCK(tp);
	(void)(*linesw[tp->t_line].l_modem)(tp, 0);
	tp->t_state &= ~TS_CARR_ON;
	tp->t_oproc = NULL;		/* mark closed */
	if (s = tp->t_session) {
		tp->t_session = NULL;
	}
	if (!tp->t_state) {
		TTY_UNLOCK(tp);
		pty_free(ptdp, dev);
	} else {
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
	}
	return(0);
}

ptcread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	long flag;
{
	register struct tty *tp;
	struct pt_ioctl *pti;
	struct pt_data *ptdp;
	char buf[BUFSIZ];
	int error, cc;

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	pti = &ptdp->pt_ioctl;
	TTY_LOCK(tp);
	/*
	 * We want to block until the slave
	 * is open, and there's something to read;
	 * but if we lost the slave or we're NBIO,
	 * then return the appropriate error instead.
	 */
	for (;;) {
		if (tp->t_state&TS_ISOPEN) {
			if (pti->pt_flags&PF_PKT && pti->pt_send) {
				error = ureadc((int)pti->pt_send, uio);
				if (!error)
					pti->pt_send = 0;
				goto out;
			}
			if (pti->pt_flags&PF_UCNTL && pti->pt_ucntl) {
				error = ureadc((int)pti->pt_ucntl, uio);
				if (!error)
					pti->pt_ucntl = 0;
				goto out;
			}
			if (tp->t_outq.c_cc && (tp->t_state&TS_TTSTOP) == 0)
				break;
		}
		if ((tp->t_state&TS_CARR_ON) == 0) {
			error = EIO;	/*  EIO to be compatible w/BSD4.3 */
			goto out;
		}
		if (flag & (IO_NDELAY|IO_NONBLOCK)) {
			error = EWOULDBLOCK;
			goto out;
		}
		if (error = ttysleep (tp, (caddr_t)&tp->t_outq.c_cf, 
				      TTIPRI | PCATCH, ttyin))
			goto out;
	}
	if (pti->pt_flags & (PF_PKT|PF_UCNTL))
		error = ureadc(0, uio);
	while (uio->uio_resid > 0 && error == 0) {
		cc = q_to_b(&tp->t_outq, buf, MIN(uio->uio_resid, BUFSIZ));
		if (cc <= 0)
			break;
		uio->uio_rw = UIO_READ;
		error = uiomove(buf, cc, uio);
	}
	if (tp->t_outq.c_cc <= tp->t_lowat) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			thread_wakeup((vm_offset_t)&tp->t_outq);
		}
		select_wakeup(&tp->t_selq);
	}
out:
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(error);
}

ptsstop(tp, flush)
	register struct tty *tp;
	long flush;
{
	struct pt_ioctl *pti;
	int flag;

	LASSERT(TTY_LOCK_HOLDER(tp));
	pti = pt_tty_to_pt_ioctl(tp);
	/* note: FLUSHREAD and FLUSHWRITE already ok */
	if (flush == 0) {
		flush = TIOCPKT_STOP;
		pti->pt_flags |= PF_STOPPED;
	} else
		pti->pt_flags &= ~PF_STOPPED;
	pti->pt_send |= flush;
	/* change of perspective */
	flag = 0;
	if (flush & FREAD)
		flag |= FWRITE;
	if (flush & FWRITE)
		flag |= FREAD;
	ptcwakeup(tp, flag);
}

ptcselect(dev, events, revents, scanning)
	dev_t dev;
	short *events, *revents;
	int scanning;
{
	register struct tty *tp;
	struct pt_ioctl *pti;
	struct pt_data *ptdp;
	int error;
	TSPLVAR(s)

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	pti = &ptdp->pt_ioctl;
	TTY_LOCK(tp);
	if (!scanning) {
		select_dequeue(&pti->pt_selq);
		goto selout;
	}

	if ((tp->t_state&TS_CARR_ON) == 0) {
		*revents |= POLLHUP;
		if (*events & POLLNORM)
			*revents |= POLLNORM;
		goto selout;
	}

	if (*events & (POLLNORM | POLLPRI)) {
		if (*events & POLLNORM) {
			/*
			 * Need to block timeouts (ttrstart).
			 */
			TSPLTTY(s);
			if ((tp->t_state&TS_ISOPEN) &&
			    tp->t_outq.c_cc && (tp->t_state&TS_TTSTOP) == 0) {
				TSPLX(s);
				*revents |= POLLNORM;
				goto selout;
			}
			TSPLX(s);
		}

		/* FALLTHROUGH */
	
		if ((tp->t_state&TS_ISOPEN) &&
		    (pti->pt_flags&PF_PKT && pti->pt_send ||
		    pti->pt_flags&PF_UCNTL && pti->pt_ucntl)) {
			if (*events & POLLNORM)
				*revents |= POLLNORM;
			if (*events & POLLPRI)
				*revents |= POLLPRI;
			goto selout;
		}
		select_enqueue(&pti->pt_selq);
	}

	if (*events & POLLOUT) {
		if (tp->t_state&TS_ISOPEN) {
			if (pti->pt_flags & PF_REMOTE) {
				if (tp->t_canq.c_cc == 0) {
					*revents |= POLLOUT;
					goto selout;
				}
			} else {
				if (tp->t_rawq.c_cc + tp->t_canq.c_cc < TTYHOG-2) {
					*revents |= POLLOUT;
					goto selout;
				}
				if (tp->t_canq.c_cc == 0 && (tp->t_iflag&ICANON)) {
					*revents |= POLLOUT;
					goto selout;
				}
			}
		}
		select_enqueue(&pti->pt_selq);
	}

selout:
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(0);
}

ptcwrite(dev, uio, flag)
	dev_t dev;
	register struct uio *uio;
	long flag;
{
	register struct tty *tp;
	register struct pt_ioctl *pti;
	register struct iovec *iov = (struct iovec *)0;
	register char *cp;
	register int cc = 0;
	struct pt_data *ptdp;
	char locbuf[BUFSIZ];
	int cnt = 0;
	int error;

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	pti = &ptdp->pt_ioctl;
	TTY_LOCK(tp);
again:
	if ((tp->t_state&TS_ISOPEN) == 0)
		goto block;
	if (pti->pt_flags & PF_REMOTE) {
		if (tp->t_canq.c_cc)
			goto block;
		while (uio->uio_iovcnt > 0 && tp->t_canq.c_cc < TTYHOG - 1) {
			iov = uio->uio_iov;
			if (iov->iov_len == 0) {
				uio->uio_iovcnt--;	
				uio->uio_iov++;
				continue;
			}
			if (cc == 0) {
				cc = MIN(iov->iov_len, BUFSIZ);
				cc = MIN(cc, TTYHOG - 1 - tp->t_canq.c_cc);
				cp = locbuf;
				uio->uio_rw = UIO_WRITE;
				if (error = uiomove(cp, cc, uio))
					goto out;
				/* check again for safety */
				if ((tp->t_state&TS_ISOPEN) == 0) {
					error = EIO;
					goto out;
				}
			}
			if (cc) {
				(void) b_to_q(cp, cc, &tp->t_canq);
			}
			cc = 0;
		}
		(void) putc(0, &tp->t_canq);
		ttwakeup(tp);
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
		thread_wakeup((vm_offset_t)&tp->t_canq);
		return (0);
	}
	while (uio->uio_iovcnt > 0) {
		iov = uio->uio_iov;
		if (cc == 0) {
			if (iov->iov_len == 0) {
				uio->uio_iovcnt--;	
				uio->uio_iov++;
				continue;
			}
			cc = MIN(iov->iov_len, BUFSIZ);
			cp = locbuf;
			uio->uio_rw = UIO_WRITE;
			error = uiomove(cp, cc, uio);
			if (error)
				goto out;
			/* check again for safety */
			if ((tp->t_state&TS_ISOPEN) == 0) {
				error = EIO;
				goto out;
			}
		}
		while (cc > 0) {
			if ((tp->t_rawq.c_cc + tp->t_canq.c_cc) >= TTYHOG - 2 &&
			   (tp->t_canq.c_cc > 0 || !(tp->t_iflag&ICANON))) {
				thread_wakeup((vm_offset_t)&tp->t_rawq);
				goto block;
			}
			(*linesw[tp->t_line].l_rint)(*cp++&0377, tp);
			cnt++;
			cc--;
		}
		cc = 0;
	}
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return (0);
block:
	/*
	 * Come here to wait for slave to open, for space
	 * in outq, or space in rawq.
	 */
	if ((tp->t_state&TS_CARR_ON) == 0) {
		error = EIO;
		goto out;
	}
	if ((pti->pt_flags & PF_NBIO) || (flag & (IO_NDELAY|IO_NONBLOCK))) {
		if (iov) {
			iov->iov_base -= cc;
			iov->iov_len += cc;
			uio->uio_resid += cc;
			uio->uio_offset -= cc;
		}
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
		if (cnt == 0)
			return (EWOULDBLOCK);
		return (0);
	}
	if (!(error = ttysleep(tp, (caddr_t)&tp->t_rawq.c_cf, 
			     TTOPRI | PCATCH, ttyout)))
		goto again;
out:
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return(error);
}

/*ARGSUSED*/
ptyioctl(dev, cmd, data, flag, cred, private, retval)
	dev_t dev;
	unsigned int cmd;
	caddr_t data;
	long flag;
	struct ucred    *cred;
	void            *private;
	int             *retval;
{
	register struct tty *tp;
	register struct pt_ioctl *pti;
	register u_char *cc;
	struct pt_data *ptdp;
	int stop, error;
	extern ttyinput();
#if NKJI > 0
	extern kji_ttyinput();
#endif

	if (error = pty_ref(dev, &ptdp))
		return(error);

	tp = &ptdp->pt_tty;
	pti = &ptdp->pt_ioctl;
	TTY_LOCK(tp);
	cc = tp->t_cc;
	/*
	 * IF CONTROLLER STTY THEN MUST FLUSH TO PREVENT A HANG.
	 * ttywflush(tp) will hang if there are characters in the outq.
	 */
	if (cdevsw[major(dev)].d_open == ptcopen)
		switch (cmd) {

		case TIOCSCTTY:
			goto einval;

		case TIOCPKT:
			if (*(int *)data) {
				if (pti->pt_flags & PF_UCNTL) {
					goto einval;
				}
				pti->pt_flags |= PF_PKT;
			} else
				pti->pt_flags &= ~PF_PKT;
			TTY_UNLOCK(tp);
			pty_unref(ptdp);
			return (0);
		case TIOCUCNTL:
			if (*(int *)data) {
				if (pti->pt_flags & PF_PKT) {
					goto einval;
				}
				pti->pt_flags |= PF_UCNTL;
			} else
				pti->pt_flags &= ~PF_UCNTL;
			TTY_UNLOCK(tp);
			pty_unref(ptdp);
			return (0);

		case TIOCREMOTE:
			if (*(int *)data)
				pti->pt_flags |= PF_REMOTE;
			else
				pti->pt_flags &= ~PF_REMOTE;
			ttyflush(tp, FREAD|FWRITE);
			TTY_UNLOCK(tp);
			pty_unref(ptdp);
			return (0);

		case ISPTM:
			*retval = tp->t_dev;	
			pty_unref(ptdp);
			return(0);

		case old_isptm:
			*(dev_t *)data = tp->t_dev;	
			pty_unref(ptdp);
			return(0);

		case FIONBIO:
			if (*(int *)data)
				pti->pt_flags |= PF_NBIO;
			else
				pti->pt_flags &= ~PF_NBIO;
			TTY_UNLOCK(tp);
			pty_unref(ptdp);
			return (0);

		case TIOCSETP:
		case TIOCSETN:
		case TIOCSETD:
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF:
			while (getc(&tp->t_outq) >= 0)
				;
			break;
		}

#ifdef	sun	/* Yes, this is a property of SunOS */

/*
 * The following code was added in order to allow Suntools to run under Mach.
 * Note that TIOCSSIZE has the same ioctl number as _O_TIOCSSIZE and
 * TIOCSWINSZ.  Thus a call specifying any of these ioctl's will be
 * handled here.
 */
	switch (cmd) {

	case TIOCSSIZE:	/* This isn't very pretty, but what driver is? */
		/*
		 * Check to make sure this is not a TIOCSWINSZ which has
		 * the same ioctl number.  We distinguish between the two
		 * by examining the "ts_lines" field in the ttysize structure.
		 * If the upper 16 bits of this field are non-zero, we assume
		 * it is a TIOCSWINSZ. TIOCSWINSZ is handled by the regular
		 * tty driver.
		 */
		if ((((struct swsize *)data)->ts_lines&0xffff0000) != 0)
			break;

	case _N_TIOCSSIZE:
		tp->t_winsize.ws_row = ((struct swsize *)data)->ts_lines;
		tp->t_winsize.ws_col = ((struct swsize *)data)->ts_cols;
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
		return (0);

	/*
	 * There is no way to distinguish between a real TIOCGSIZE as
	 * in SunOS and the 4.3 TIOCGWINSZ since the kluge used above
	 * can't be used (we don't know what the user is doing).  We'll
	 * assume that it's a TIOCGWINSZ and punt to the tty driver.
	 */
	case _N_TIOCGSIZE:
		((struct swsize *)data)->ts_lines = tp->t_winsize.ws_row;
		((struct swsize *)data)->ts_cols = tp->t_winsize.ws_col;
		TTY_UNLOCK(tp);
		pty_unref(ptdp);
		return (0);
        }

#endif	/* sun */

	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, data, flag);
	if (error < 0)
		 error = ttioctl(tp, cmd, data, flag);
	/*
	 * Since we use the tty queues internally,
	 * pty's can't be switched to disciplines which overwrite
	 * the queues.  We can't tell anything about the discipline
	 * from here...
	 * kji_ttyinput is also "blessed" - kanji discipline.
	 */
#if NKJI > 0
	if ((linesw[tp->t_line].l_rint != ttyinput) &&
	    (linesw[tp->t_line].l_rint != kji_ttyinput)) {
#else
	if (linesw[tp->t_line].l_rint != ttyinput) {
#endif
		(*linesw[tp->t_line].l_close)(tp);
		tp->t_line = 0;
		(void)(*linesw[tp->t_line].l_open)(dev, tp, flag);
		error = ENOTTY;
	}
	if (error < 0) {
		if (pti->pt_flags & PF_UCNTL &&
		    (cmd & ~0xff) == UIOCCMD(0)) {
			if (cmd & 0xff) {
				pti->pt_ucntl = (u_char)cmd;
				ptcwakeup(tp, FREAD);
			}
			TTY_UNLOCK(tp);
			pty_unref(ptdp);
			return (0);
		}
		error = ENOTTY;
	}
	stop = (tp->t_iflag & IXON) && CCEQ(cc[VSTOP], CTRL('s'))
		&& CCEQ(cc[VSTART], CTRL('q'));
	if (pti->pt_flags & PF_NOSTOP) {
		if (stop) {
			pti->pt_send &= ~TIOCPKT_NOSTOP;
			pti->pt_send |= TIOCPKT_DOSTOP;
			pti->pt_flags &= ~PF_NOSTOP;
			ptcwakeup(tp, FREAD);
		}
	} else {
		if (!stop) {
			pti->pt_send &= ~TIOCPKT_DOSTOP;
			pti->pt_send |= TIOCPKT_NOSTOP;
			pti->pt_flags |= PF_NOSTOP;
			ptcwakeup(tp, FREAD);
		}
	}
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return (error);
einval:
	TTY_UNLOCK(tp);
	pty_unref(ptdp);
	return EINVAL;
}

decl_simple_lock_data(static,PTY_LOCK)

void
pty_initialization()
{
	usimple_lock_init(&PTY_LOCK);
}

int
pty_choose(devp)
dev_t *devp;
{
	register int i, j;
	minor_t pty;

	usimple_lock(&PTY_LOCK);
	for (i = 0; i < nptymask; i++) {
		if (pt_mask[i] != ~0L)
			break;
	}
	if (i >= nptymask) {
		usimple_unlock(&PTY_LOCK);
		return(ENXIO);
	}
	j = ffs_l(~pt_mask[i]) - 1;
	pty = (i * LONG_BIT) + j;
	if (j < 0 || pty >= npty) {
		usimple_unlock(&PTY_LOCK);
		return(ENXIO);
	}
	pt_mask[i] |= (1L << j); /* must reserve pty slot while under lock */
	usimple_unlock(&PTY_LOCK);
	*devp = makedev(major(*devp), pty);
	return(0);
}

int
pty_alloc(dev, ptdpp)
dev_t dev;
struct pt_data **ptdpp;
{
	register struct pt_data *ptdp;
	struct pt_data *ptdp2;
	minor_t pty;

	if ((pty = minor(dev)) >= npty) {
		*ptdpp = NULL;
		return(ENXIO);
	}

	/* indicate pty slot is being used and check for existing pt_data */
	usimple_lock(&PTY_LOCK);
	pt_mask[pty / LONG_BIT] |= (1L << (pty % LONG_BIT));
	if (ptdp = pt_datap[pty]) {
		/* here if the data structure has already been allocated */
		ptdp->pt_ioctl.pt_count++;
		usimple_unlock(&PTY_LOCK);
		*ptdpp = ptdp;
		return(0);
	}
	usimple_unlock(&PTY_LOCK);

	/* attempt allocation of new pt_data (might wait for free memory) */
	if (!(ptdp = (struct pt_data *)kalloc(sizeof(struct pt_data)))) {
		/* here if we couldn't get one (shouldn't happen) */
		usimple_lock(&PTY_LOCK);
		if (ptdp = pt_datap[pty]) {
			/* here if another process put one in place */
			ptdp->pt_ioctl.pt_count++;
			usimple_unlock(&PTY_LOCK);
			*ptdpp = ptdp;
			return(0);
		}
		/* here to fail the open if the allocation failed */
		pt_mask[pty / LONG_BIT] &= ~(1L << (pty % LONG_BIT));
		usimple_unlock(&PTY_LOCK);
		*ptdpp = NULL;
		return(ENOMEM);
	}

	/* here to initialize the newly allocated pt_data structure */
	bzero(ptdp, sizeof(struct pt_data));
	ptdp->pt_ioctl.pt_count = 2; /* one for caller and one for table */
#if UNIX_LOCKS && (NCPUS > 1 || RT_PREEMPT)
	lock_init2(&ptdp->pt_tty.t_lock, TRUE, LTYPE_PTY);
#endif
	queue_init(&ptdp->pt_tty.t_selq);
	queue_init(&ptdp->pt_ioctl.pt_selq);

	/* put new structure in place under appropriate synchronization */
	usimple_lock(&PTY_LOCK);
	if (ptdp2 = pt_datap[pty]) {
		/* here if another process put one in place first */
		ptdp2->pt_ioctl.pt_count++;
		usimple_unlock(&PTY_LOCK);
		ptdp->pt_ioctl.pt_count = 0; /* defend against misuse */
		kfree(ptdp, sizeof(struct pt_data));
		*ptdpp = ptdp2;
		return(0);
	}
	pt_datap[pty] = ptdp;
	usimple_unlock(&PTY_LOCK);
	*ptdpp = ptdp;
	return(0);
}

void
pty_free(ptdp, dev)
register struct pt_data *ptdp;
dev_t dev;
{
	minor_t pty;

	pty = minor(dev);
	usimple_lock(&PTY_LOCK);
	if (ptdp == pt_datap[pty]) {
		/* here if pty slot should be freed (normal last close) */
		pt_mask[pty / LONG_BIT] &= ~(1L << (pty % LONG_BIT));
		pt_datap[pty] = NULL;
		if ((ptdp->pt_ioctl.pt_count -= 2) > 0) {
			/* here if pt_data structure is still referenced */
			usimple_unlock(&PTY_LOCK);
			return;
		}
	} else {
		/* here if pty slot is still in use or has been reused */
		if (--ptdp->pt_ioctl.pt_count > 0) {
			/* here if pt_data structure is still referenced */
			usimple_unlock(&PTY_LOCK);
			return;
		}
	}
	/* here to free pt_data structure on release of final reference */
	usimple_unlock(&PTY_LOCK);
	kfree(ptdp, sizeof(struct pt_data));
}

int
pty_ref(dev, ptdpp)
dev_t dev;
struct pt_data **ptdpp;
{
	struct pt_data *ptdp;
	minor_t pty;

	pty = minor(dev);
	usimple_lock(&PTY_LOCK);
	if (!(ptdp = pt_datap[pty])) {
		/* here if pty slot has been closed */
		usimple_unlock(&PTY_LOCK);
		*ptdpp = NULL;
		return(EIO);
	}
	/* here to increment structure reference count */
	ptdp->pt_ioctl.pt_count++;
	usimple_unlock(&PTY_LOCK);
	*ptdpp = ptdp;
	return(0);
}

void
pty_unref(ptdp)
struct pt_data *ptdp;
{
	usimple_lock(&PTY_LOCK);
	if (--ptdp->pt_ioctl.pt_count > 0) {
		/* here if there is still another structure reference */
		usimple_unlock(&PTY_LOCK);
		return;
	}
	/* here if pty slot was closed while we had an outstanding ref */
	usimple_unlock(&PTY_LOCK);
	kfree(ptdp, sizeof(struct pt_data));
}

#endif	/* NPTY > 0 */
