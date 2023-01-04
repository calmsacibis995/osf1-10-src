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
static char *rcsid = "@(#)$RCSfile: uc_sjis.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:08:57 $";
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

#include <sys/sysconfig.h>
#include <sys/termios.h>
#include <sys/stropts.h>
#include <sys/stream.h>
#include <sys/eucioctl.h>
#include <sys/sjisioctl.h>
#include <tty/stream_tty.h>
#include <tty/stream_sjis.h>

int
uc_sjis_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp)
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
uc_sjis_close(queue_t *q, int flag, cred_t *credp)
{
	struct sjis_s *jp = (struct sjis_s *) q->q_ptr;

	if (jp->rbid)
		unbufcall(jp->rbid);
	if (jp->wbid)
		unbufcall(jp->wbid);

	if (jp->rtid)
		unbufcall(jp->rtid);
	if (jp->wtid)
		unbufcall(jp->wtid);

	if (jp->rspare)
		freemsg(jp->rspare);
	if (jp->wspare)
		freemsg(jp->wspare);

	return(streams_close_comm(q, flag, credp));
}

/*
 * read side should convert from ajec to sjis
 * also peek at the ioctl(M_IOCACK) to look for IEXTEN flag
 */
int
uc_sjis_rput(register queue_t *q, register mblk_t *mp)
{
	struct sjis_s *jp = (struct sjis_s *)q->q_ptr;
	struct iocblk *iocp;

	switch (mp->b_datap->db_type) {
	case M_IOCACK:
		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF: {
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
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			flushq(q, FLUSHDATA);
			jp->asc[0] = jp->asc[1] = 0;
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first || !sjis_readdata(jp, q, mp, conv_ajec2sjis, 1))
			putq(q, mp);
		break;
	default:
		if (canput(q->q_next) || mp->b_datap->db_type >= QPCTL)
			putnext(q, mp);
		else
			putq(q, mp);
	}
	return(0);
}

int
uc_sjis_rsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_readdata(jp, q, mp, conv_ajec2sjis, 1)) {
				putbq(q, mp);
				return(0);
			}
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
 * write side should convert from sjis to ajec and do ioctl
 */
int
uc_sjis_wput(register queue_t *q, register mblk_t *mp)
{
	struct sjis_s *jp;

	jp = (struct sjis_s *) q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_DATA:
		if (q->q_first || !sjis_writedata(jp, q, mp, conv_sjis2ajec, 2))
			putq(q, mp);
		break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW) {
			flushq(q, FLUSHDATA);
			jp->sac = 0;
		}
		putnext(q, mp);
		break;
	case M_IOCTL:
		if (q->q_first || !uc_sjis_ioctl(jp, q, mp))
			putq(q, mp);
		break;
	default:
		if ((mp->b_datap->db_type >= QPCTL) || (canput(q->q_next)))
			putnext(q, mp);
		else
			putq(q, mp);
	}
	return(0);
}

int
uc_sjis_wsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct sjis_s *jp;

	jp = (struct sjis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!sjis_writedata(jp, q, mp, conv_sjis2ajec, 2)) {
				putbq(q, mp);
				return(0);
			}
			break;
		case M_IOCTL:
			if (!uc_sjis_ioctl(jp, q, mp)) {
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

uc_sjis_ioctl(struct sjis_s *jp, queue_t *q, mblk_t *mp)
{
	struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;

	/* First check for flow control on ioctl's which we may want to
	 * send downstream (which means everything but SJIS_UC*).
	 */

	switch (iocp->ioc_cmd) {
	case SJIS_UC_C1SET:
	case SJIS_UC_C1GET:
		break;
	default:
		if (!canput(q->q_next))
			return(0);
		else
			break;
	}

	switch (iocp->ioc_cmd) {
	case TIOCSTI:
		if (sjis_proc_sti(jp, *mp->b_cont->b_rptr, q) < 0) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
		} else {
			iocp->ioc_error = 0;
			mp->b_datap->db_type = M_IOCACK;
		}
		iocp->ioc_count = 0;
		qreply(q, mp);
		return(1);
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
	case SJIS_UC_C1SET:
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
	case SJIS_UC_C1GET:
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

/*
 * process the TIOCSTI ioctl
 * there is SJIS -> AJEC conversion in here too
 */
PRIVATE int
sjis_proc_sti(
	register struct sjis_s *jp,
	unsigned char uc,
	register queue_t *q
	)
{
	unsigned char uc1, n1, n2;
	unsigned char tc[2];

	uc1 = jp->sti_c;
	if (!uc1) {
		if (INRANGE(0x00, 0x7f, uc)) {
			/*
			 * ascii and C0
			 */
			tc[0] = uc;
			return(sjis_send_sti(q, tc, 1));
		}
		if (INRANGE(0xa1, 0xdf, uc)) {
			/*
			 * hankaku-kana
			 */
			tc[0] = SS2;
			tc[1] = uc;
			return(sjis_send_sti(q, tc, 2));
		}
		if (INRANGE(0x81, 0x9f, uc) || INRANGE(0xe0, 0xef, uc)) {
			/*
			 * first byte of kanji
			 */
			jp->sti_c = uc;
			return(0);
		}
		tc[0] = uc & 0x7f;
		return(sjis_send_sti(q, tc, 1));
	}
	if (INRANGE(0x81, 0x9f, uc1)) {
		/*
		 * kanji
		 */
		if (INRANGE(0x40, 0x7e, uc)) {
			n1 = ((uc1 - 0x81)<<1) + 0xa1;
			n2 = uc + 0x61;
		}
		else if (INRANGE(0x80, 0x9e, uc)) {
			n1 = ((uc1 - 0x81)<<1) + 0xa1;
			n2 = uc + 0x60;
		}
		else if (INRANGE(0x9f, 0xfc, uc)) {
			n1 = ((uc1 - 0x81)<<1) + 0xa2;
			n2 = uc + 2;
		}
		else {
			n1 = uc1 & 0x7f;
			n2 = uc & 0x7f;
		}
		jp->sti_c = 0;
		tc[0] = n1;
		tc[1] = n2;
		return(sjis_send_sti(q, tc, 2));
	}
	if (INRANGE(0xe0, 0xef, uc1)) {
		/*
		 * kanji
		 */
		if (INRANGE(0x40, 0x7e, uc)) {
			n1 = ((uc1 - 0xe0)<<1) + 0xdf;
			n2 = uc + 0x61;
		}
		else if (INRANGE(0x80, 0x9e, uc)) {
			n1 = ((uc1 - 0xe0)<<1) + 0xdf;
			n2 = uc + 0x60;
		}
		else if (INRANGE(0x9f, 0xfc, uc)) {
			n1 = ((uc1 - 0xe0)<<1) + 0xe0;
			n2 = uc + 2;
		}
		else {
			n1 = uc1 & 0x7f;
			n2 = uc & 0x7f;
		}
		jp->sti_c = 0;
		tc[0] = n1;
		tc[1] = n2;
		return(sjis_send_sti(q, tc, 2));
	}
	/*
	 * error
	 */
	jp->sti_c = 0;
	tc[0] = n1 & 0x7f;
	tc[1] = n2 & 0x7f;
	return(sjis_send_sti(q, tc, 2));
}

/*
 * send the TIOCSTI M_CTL message down with the given string
 */
PRIVATE int
sjis_send_sti(
	register queue_t *q,
	register unsigned char *ucp,
	register int len
	)
{
	register mblk_t *mp, *mp1;
	register struct iocblk *iocp;

	mp = allocb(sizeof(struct iocblk), BPRI_MED);
	mp1 = allocb(len, BPRI_MED);
	if (!mp || !mp1) {
		if (mp)
			freemsg(mp);
		if (mp1)
			freemsg(mp1);
		return(-1);
	}
	mp->b_datap->db_type = M_CTL;
	mp->b_cont = mp1;
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	iocp = (struct iocblk *)mp->b_rptr;
	iocp->ioc_cmd = TIOCSTI;
	iocp->ioc_count = len;
	while (len--)
		*mp1->b_wptr++ = *ucp++;
	putnext(q, mp);
	return(0);
}
