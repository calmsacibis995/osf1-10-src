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
static char	*rcsid = "@(#)$RCSfile: subr_log.c,v $ $Revision: 4.4.10.4 $ (DEC) $Date: 1993/08/02 20:40:15 $";
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

/*
 * Error log buffer for kernel printf's.
 */

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/ioctl.h>
#include <sys/msgbuf.h>
#include <sys/file.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <sys/poll.h>
#include <sys/lwc.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>
#include <dec/binlog/binlog.h>

long msgbuf_size = sizeof(struct msgbuf);/* can be overridden during boot */
struct msgbuf *pmsgbuf;			/* pointer to main message buffer */
struct msgbuf *pconbuf;			/* pointer to deferred console buf */
decl_simple_lock_data(,msgbuf_lock)	/* lock on writing to either msgbuf */

int log_open;				/* klog open flag (for subr_prf.c) */
int con_open;				/* kcon open flag (for subr_prf.c) */
int log_wait;				/* klog read wait (for wakeup) */
int con_wait;				/* kcon read wait (for wakeup) */
struct queue_entry log_selq;		/* klog select thread queue */
struct queue_entry con_selq;		/* kcon select thread queue */
decl_simple_lock_data(,log_lock)	/* lock on above state data */
lwc_id_t log_lwcid;			/* lightweight context id */

/*ARGSUSED*/
logopen(dev)
	dev_t dev;
{
	int s;

	switch (minor(dev)) {
	case 0:
		/* minor 0 is used for the /dev/klog pseudo-device */
		if (!pmsgbuf)
			return(ENXIO);
		s = spl1();
		simple_lock(&log_lock);
		if (log_open) {
			simple_unlock(&log_lock);
			splx(s);
			return(EBUSY);
		}
		log_open = 1;
		simple_unlock(&log_lock);
		splx(s);
		break;
	case 1:
		/* minor 1 is used for the /dev/kcon pseudo-device */
		if (!pconbuf)
			return(ENXIO);
		s = spl1();
		simple_lock(&log_lock);
		if (con_open) {
			simple_unlock(&log_lock);
			splx(s);
			return(EBUSY);
		}
		con_open = 1;
		simple_unlock(&log_lock);
		splx(s);
		break;
	default:
		return(ENXIO);
	}
	return(0);
}

/*ARGSUSED*/
logclose(dev)
	dev_t dev;
{
	int s;

	s = spl1();
	simple_lock(&log_lock);
	if (minor(dev) == 0) {
		log_open = log_wait = 0;
		select_wakeup(&log_selq);
		select_dequeue_all(&log_selq);
	} else {
		con_open = con_wait = 0;
		select_wakeup(&con_selq);
		select_dequeue_all(&con_selq);
	}
	simple_unlock(&log_lock);
	splx(s);
	return(0);
}

/*ARGSUSED*/
logread(dev, uio, flag)
	dev_t dev;
	struct uio *uio;
	int flag;
{
	register struct msgbuf *mp;
	register long i, j, len;
	int *wp, s, error;
	long mbsize;

	if (minor(dev) == 0) {
		mp = pmsgbuf;
		wp = &log_wait;
	} else {
		mp = pconbuf;
		wp = &con_wait;
	}

	s = spl1();
	simple_lock(&log_lock);
	while (mp->msg_bufr == mp->msg_bufx) {
		if (flag & (IO_NDELAY|IO_NONBLOCK)) {
			simple_unlock(&log_lock);
			splx(s);
			return(EWOULDBLOCK);
		}
		*wp = 1; /* sets either log_wait or con_wait */
		if (error = mpsleep((caddr_t)wp, (PZERO + 1) | PCATCH, "klog",
		    0, (void *)simple_lock_addr(log_lock), MS_LOCK_SIMPLE)) {
			/* log_lock is not held in this path */
			splx(s);
			return(error);
		}
	}
	*wp = 0; /* clears either log_wait or con_wait */
	simple_unlock(&log_lock);
	splx(s);

	/* no locking needed for reading out of either msgbuf */
	mbsize = msgbuf_size - (sizeof(struct msgbuf) - MSG_BSIZE);
	while (uio->uio_resid > 0) {
		i = mp->msg_bufr;
		j = mp->msg_bufx;
		len = (j >= i) ? j - i : mbsize - i;
		if (len > uio->uio_resid)
			len = uio->uio_resid;
		if (len <= 0)
			break;
		if (error = uiomove((caddr_t)&mp->msg_bufc[i], (int)len, uio))
			return(error);
		if ((i += len) < 0 || i >= mbsize)
			i = 0;
		mp->msg_bufr = i;
	}
	return(0);
}

/*ARGSUSED*/
logselect(dev, events, revents, scanning)
dev_t	dev;
short	*events, *revents;
int	scanning;
{
	struct queue_entry *qp;
	struct msgbuf *mp;
	int *wp, s;

	if (minor(dev) == 0) {
		mp = pmsgbuf;
		qp = &log_selq;
		wp = &log_wait;
	} else {
		mp = pconbuf;
		qp = &con_selq;
		wp = &con_wait;
	}

	s = spl1();
	simple_lock(&log_lock);
	if (scanning) {
		if (*events & POLLNORM) {
			if (mp->msg_bufr == mp->msg_bufx) {
				select_enqueue(qp);
				*wp = 1; /* sets either log_wait or con_wait */
			} else
				*revents |= POLLNORM;
		}
	} else
		select_dequeue(qp);

	simple_unlock(&log_lock);
	splx(s);
	return(0);
}

/*ARGSUSED*/
logioctl(dev, com, data, flag)
	dev_t dev;
	unsigned int com;
	caddr_t data;
	long flag;
{
	struct msgbuf *mp;
	long len, mbsize;

	mbsize = msgbuf_size - (sizeof(struct msgbuf) - MSG_BSIZE);
	mp = (minor(dev) == 0) ? pmsgbuf : pconbuf;

	switch (com) {
	case FIONREAD:
		if ((len = mp->msg_bufx - mp->msg_bufr) < 0L)
			len += mbsize;
		*(off_t *)data = len;
		break;
	default:
		return(EINVAL);
	}
	return(0);
}

/*
 * Called from subr_prf.c to put a null-terminated string into the
 * main message buffer, and optionally into the deferred console i/o
 * buffer (first).  This routine must be called at splextreme().
 */
log_puts(str, both)
	u_char *str;
	int both;
{
	register u_char c, *sp, *dp;
	register long i;
	long j, mbsize;
	struct msgbuf *mp;
	extern int binlogpanic;

	mbsize = msgbuf_size - (sizeof(struct msgbuf) - MSG_BSIZE);

	simple_lock(&msgbuf_lock);
	mp = (both) ? pconbuf : pmsgbuf;
	do {
		i = j = mp->msg_bufx;
		sp = str;
		dp = (u_char *)&mp->msg_bufc[i];
		while (c = *sp++) {
			if (c == '\r' || c == 0177)
				continue;
			*dp++ = c;
			if (++i >= mbsize) {
				i = 0L;
				dp = (u_char *)&mp->msg_bufc[0];
			}
		}
		mp->msg_bufx = i;
		if (mp == pmsgbuf)
			break;
		mp = pmsgbuf;
	} while (1);
	simple_unlock(&msgbuf_lock);

	if (!binlogpanic) {
		/* the following is normally a no-op */
		binlog_logmsg(ELMSGT_INFO, str, 0);
	} else {
		binlogpanic = 0;
		binlog_logmsg(ELMSGT_PANIC, str, 0);
	}
}

logwakeup()
{
	if (log_open || con_open)
		lwc_interrupt(log_lwcid);
}

void
logwakeup_lwc()
{
	int s;

	lwc_rfc(log_lwcid);

	s = spl1();
	simple_lock(&log_lock);
	if (log_wait) {
		log_wait = 0;
		select_wakeup(&log_selq);
		wakeup((caddr_t)&log_wait);
	}
	if (con_wait) {
		con_wait = 0;
		select_wakeup(&con_selq);
		wakeup((caddr_t)&con_wait);
	}
	simple_unlock(&log_lock);
	splx(s);
}

long
log_bootstrap(mp)
	register struct msgbuf *mp;
{
	extern int printstate;

	if (msgbuf_size == 1L || msgbuf_size > 1024L * 1024L)
		msgbuf_size = sizeof(struct msgbuf);
	else if (msgbuf_size < 1024L)
		msgbuf_size = 0L;
	else if (msgbuf_size & (sizeof(long) - 1))
		msgbuf_size &= ~(sizeof(long) - 1);
	if (!msgbuf_size)
		return(0L);

	bzero((caddr_t)mp, 2L * msgbuf_size);
	(pmsgbuf = mp)->msg_magic = MSG_MAGIC;
	mp = (struct msgbuf *)((caddr_t)mp + msgbuf_size);
	(pconbuf = mp)->msg_magic = MSG_MAGIC;
	simple_lock_init(&msgbuf_lock);
	simple_lock_init(&log_lock);
	printstate |= MEMPRINT;
	return(2L * msgbuf_size);
}

void
log_init()
{
	if (!(log_lwcid = lwc_create(LWC_PRI_SYSLOG, logwakeup_lwc)))
		panic("log_init: lwc creation failure");
	queue_init(&log_selq);
	queue_init(&con_selq);
}

void
log_startup()
{
	struct msgbuf *mp;
	int i;

	if ((mp = pmsgbuf) && (i = (int)mp->msg_bufx) > 0)
		binlog_logmsg(ELMSGT_SU, &mp->msg_bufc[0], i);
}
