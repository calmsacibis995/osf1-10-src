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
static char *rcsid = "@(#)$RCSfile: lc_sjis.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:04:41 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/systm.h>

#include <sys/termios.h>
#include <sys/sysconfig.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/eucioctl.h>
#include <sys/sjisioctl.h>
#include <tty/stream_tty.h>
#include <tty/stream_sjis.h>

int
lc_sjis_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp)
{
	register struct sjis_s *jp;
	int error;

	if (q->q_ptr)
		return(0);
	if (error = streams_open_comm(sizeof(*jp), q, devp, flag, sflag, credp))
		return(error);
	else
		jp = (struct sjis_s *) q->q_ptr;

	/*
	 * Default values for contents of struct sjis_s (NULL fields
	 * already taken care of via bzero() in streams_open_comm()).
	 */
	jp->flags_save = jp->flags = KS_ICONV | KS_OCONV | KS_IEXTEN;
	jp->c1state = SJIS_C1_C0;

	return(0);
}

int
lc_sjis_close(queue_t *q, int flag, cred_t *credp)
{
	struct sjis_s *jp = (struct sjis_s *) q->q_ptr;

	if (jp->rbid)
		unbufcall(jp->rbid);
	if (jp->wbid)
		unbufcall(jp->wbid);

	if (jp->rtid)
		untimeout(jp->rtid);
	if (jp->wtid)
		untimeout(jp->wtid);

	if (jp->rspare)
		freemsg(jp->rspare);
	if (jp->wspare)
		freemsg(jp->wspare);

	return(streams_close_comm(q, flag, credp));
}

/*
 * read side should convert from sjis to ajec
 * also peek at the ioctl(M_IOCACK) to look for IEXTEN flag
 */
int
lc_sjis_rput(
	register queue_t *q,
	register mblk_t *mp
	)
{
	register struct sjis_s *jp = (struct sjis_s *)q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_IOCACK:
	{
		register struct iocblk *iocp = (struct iocblk *)mp->b_rptr;

		switch (iocp->ioc_cmd) {
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF:
		{
			register struct termios *tp;

			if (!mp->b_cont)
				break;
			tp = (struct termios *)mp->b_cont->b_rptr;
			if (tp->c_lflag & IEXTEN)
				jp->flags |= KS_IEXTEN;
			else
				jp->flags &= ~KS_IEXTEN;
			break;
		}
		default:
			break;
		}
		putnext(q, mp);
		break;
	}
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			flushq(q, FLUSHDATA);
			jp->sac = 0;
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first || !sjis_readdata(jp, q, mp, conv_sjis2ajec, 2))
			putq(q, mp);
		break;
	default: 
		if (mp->b_datap->db_type >= QPCTL)
			putnext(q, mp);
		else
			putq(q, mp);
		break;
	}
	return(0);
}

int
lc_sjis_rsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_readdata(jp, q, mp, conv_sjis2ajec, 2)) {
				putbq(q, mp);
				return(0);
			}
			break;
		default:
			ASSERT(mp->b_datap->db_type < QPCTL);
			if (!canput(q->q_next)) {
				putbq(q, mp);
				return(0);
			}
			putnext(q, mp);
			break;
		}
	}
	return(0);
}

/*
 * write side should convert from ajec to sjis and do ioctl
 */
int
lc_sjis_wput(
	register queue_t *q,
	register mblk_t *mp
	)
{
	struct sjis_s *jp = (struct sjis_s *) q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW) {
			flushq(q, FLUSHDATA);
			jp->asc[0] = jp->asc[1] = 0;
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first || !sjis_writedata(jp, q, mp, conv_ajec2sjis, 1))
			putq(q, mp);
		break;
	case M_IOCTL:
		if (q->q_first || !lc_sjis_ioctl(jp, q, mp))
			putq(q, mp);
		break;
	default:
		if ((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp);
		else
			putq(q, mp);
		break;
	}
	return(0);
}

int
lc_sjis_wsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_writedata(jp, q, mp, conv_ajec2sjis, 1)) {
				putbq(q, mp);
				return(0);
			}
			break;
		case M_IOCTL:
			if (!lc_sjis_ioctl(jp, q, mp)) {
				putbq(q, mp);
				return(0);
			}
			break;
		default:
			ASSERT(mp->b_datap->db_type < QPCTL);
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

PRIVATE int
lc_sjis_ioctl(struct sjis_s *jp, queue_t *q, mblk_t *mp)
{
	struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;

	/* First check for flow control on ioctl's which we may want to
	 * send downstream (which means everything but SJIS_LC*).
	 */

	switch (iocp->ioc_cmd) {
	case SJIS_LC_C1SET:
	case SJIS_LC_C1GET:
		break;
	default:
		if (!canput(q->q_next))
			return(0);
		else
			break;
	}

	switch (iocp->ioc_cmd) {
	case EUC_IXLON:
		jp->flags |= KS_ICONV;
		break;
	case EUC_IXLOFF:
		jp->flags &= ~KS_ICONV;
		break;
	case EUC_OXLON:
		jp->flags |= KS_OCONV;
		break;
	case EUC_OXLOFF:
		jp->flags &= ~KS_OCONV;
		break;
	case EUC_MSAVE:
		jp->flags_save = jp->flags;
		jp->flags &= ~(KS_ICONV|KS_OCONV);
		break;
	case EUC_MREST:
		jp->flags = jp->flags_save;
		break;
	case SJIS_LC_C1SET:
		if (!mp->b_cont) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		if (iocp->ioc_count < sizeof(int)) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		jp->c1state = *(int *)mp->b_cont->b_rptr;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return(1);
	case SJIS_LC_C1GET:
		if (!mp->b_cont) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		if (iocp->ioc_count < sizeof(int)) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		*(int *)mp->b_cont->b_rptr = jp->c1state;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return(1);
	default:
		break;
	}

	putnext(q, mp);
	return(1);
}
