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
static char *rcsid = "@(#)$RCSfile: lc_jis.c,v $ $Revision: 1.1.5.2 $ (DEC) $Date: 1993/05/12 18:04:32 $";
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
#include <sys/jisioctl.h>
#include <tty/stream_tty.h>
#include <tty/stream_jis.h>

int
lc_jis_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp)
{
	register struct jis_s *jp;
	int error;

	if (q->q_ptr)
		return(0);
	if (error = streams_open_comm(sizeof(*jp), q, devp, flag, sflag, credp))
		return(error);
	else
		jp = (struct jis_s *) q->q_ptr;

	/*
	 * Default values for contents of struct jis_s.  Mem. is bzero'd
	 * to start with, so no initialization of zero fields here.
	 */
	jp->flags_save = jp->flags = KS_ICONV | KS_OCONV | KS_IEXTEN;
	jp->c1state = JIS_C1_C0;
	jp->kanastate = JIS_KANA_7BIT;
	strcpy(jp->ki, LC_DEF_KI);
	strcpy(jp->k2i, LC_DEF_K2I);
	strcpy(jp->ko, LC_DEF_KO);

	return(0);
}

int
lc_jis_close(queue_t *q, int flag, cred_t *credp)
{
	struct jis_s *jp = (struct jis_s *) q->q_ptr;

	if (jp->rspare)
		freemsg(jp->rspare);
	if (jp->wspare)
		freemsg(jp->wspare);

	if (jp->rbid)
		unbufcall(jp->rbid);
	if (jp->wbid)
		unbufcall(jp->wbid);
	
	if (jp->rtid)
		untimeout(jp->rtid);
	if (jp->wtid)
		untimeout(jp->wtid);

	return(streams_close_comm(q, flag, credp));
}

/*
 * read side should convert from jis to ajec
 * also peek at the ioctl(M_IOCACK) to look for IEXTEN flag
 */
int
lc_jis_rput(register queue_t *q, register mblk_t *mp)
{
	register struct jis_s *jp = (struct jis_s *)q->q_ptr;

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
			jp->jastate = 0;
			bzero(jp->jac, 8);
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first || !jis_readdata(jp, q, mp, conv_jis2ajec, JA_COEFF, JA_ADDEND))
			putq(q, mp);
		break;
	default:
		if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
			putnext(q, mp);
		else
			putq(q, mp);
	}
	return(0);
}

int
lc_jis_rsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct jis_s *jp;

	jp = (struct jis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!jis_readdata(jp, q, mp, conv_jis2ajec, JA_COEFF, JA_ADDEND)) {
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
 * write side should convert from ajec to jis and do ioctl
 */
int
lc_jis_wput(register queue_t *q, register mblk_t *mp)
{
	struct jis_s *jp = (struct jis_s *) q->q_ptr;

	switch (mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW) {
			flushq(q, FLUSHDATA);
			jp->ajstate = 0;
			jp->ajc[0] = 0;
		}
		putnext(q, mp);
		break;
	case M_DATA:
		if (q->q_first || !jis_writedata(jp, q, mp, conv_ajec2jis, AJ_COEFF, AJ_ADDEND))
			putq(q, mp);
		break;
	case M_IOCTL:
		if (q->q_first || !lc_jis_ioctl(jp, q, mp))
			putq(q, mp);
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

int
lc_jis_wsrv(register queue_t *q)
{
	register mblk_t *mp;
	register struct jis_s *jp;

	jp = (struct jis_s *)q->q_ptr;
	while (mp = getq(q)) {
		switch (mp->b_datap->db_type) {
		case M_DATA:
			if (!jis_writedata(jp, q, mp, conv_ajec2jis, AJ_COEFF, AJ_ADDEND)) {
				putbq(q, mp);
				return(0);
			}
			break;
		case M_IOCTL:
			if (!lc_jis_ioctl(jp, q, mp)) {
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

PRIVATE int
lc_jis_ioctl(struct jis_s *jp, queue_t *q, mblk_t *mp)
{
	struct iocblk *iocp;

	iocp = (struct iocblk *)mp->b_rptr;

	/* First check for flow control on ioctl's which we may want to
	 * send downstream (which means everything but JIS_LC*).
	 */
	
	switch (iocp->ioc_cmd) {
	case JIS_LC_KOUTSET:
	case JIS_LC_KINSET:
	case JIS_LC_K2INSET:
	case JIS_LC_KOUTGET:
	case JIS_LC_KINGET:
	case JIS_LC_K2INGET:
	case JIS_LC_C1SET:
	case JIS_LC_KANASET:
	case JIS_LC_C1GET:
	case JIS_LC_KANAGET:
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
	case JIS_LC_KOUTSET:
	case JIS_LC_K2INSET:
	case JIS_LC_KINSET: {
		register unsigned char *cp, *cp2;

		if (!mp->b_cont || (strlen(mp->b_cont->b_rptr) >= JA_SZ)) {
			iocp->ioc_error = EINVAL;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}

		cp = mp->b_cont->b_rptr;
		if (iocp->ioc_cmd == JIS_LC_KINSET)
			cp2 = jp->ki;
		else if (iocp->ioc_cmd == JIS_LC_K2INSET)
			cp2 = jp->k2i;
		else
			cp2 = jp->ko;

		/* size checked above */
		while (*cp)
			*cp2++ = *cp++;

		*cp2 = 0;
		iocp->ioc_error = 0;
		iocp->ioc_count = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return(1);
	}
	case JIS_LC_KOUTGET:
	case JIS_LC_K2INGET:
	case JIS_LC_KINGET: {
		register unsigned char *cp, *cp2;

		if (!mp->b_cont) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		if (iocp->ioc_count < JA_SZ) {
			iocp->ioc_error = ENOMEM;
			mp->b_datap->db_type = M_IOCNAK;
			qreply(q, mp);
			return(1);
		}
		if (iocp->ioc_cmd == JIS_LC_KINGET)
			cp = jp->ki;
		else if (iocp->ioc_cmd == JIS_LC_K2INGET)
			cp = jp->k2i;
		else
			cp = jp->ko;
		cp2 = mp->b_cont->b_rptr;
		while (*cp)
			*cp2++ = *cp++;
		*cp2++ = 0;
		mp->b_cont->b_wptr = cp2;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return(1);
	}
	case JIS_LC_C1SET:
	case JIS_LC_KANASET:
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
		if (iocp->ioc_cmd == JIS_LC_C1SET)
			jp->c1state = *(int *)mp->b_cont->b_rptr;
		else
			jp->kanastate = *(int *)mp->b_cont->b_rptr;
		iocp->ioc_error = 0;
		mp->b_datap->db_type = M_IOCACK;
		qreply(q, mp);
		return(1);
	case JIS_LC_C1GET:
	case JIS_LC_KANAGET:
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
		if (iocp->ioc_cmd == JIS_LC_C1SET)
			*(int *)mp->b_cont->b_rptr = jp->c1state;
		else
			*(int *)mp->b_cont->b_rptr = jp->kanastate;
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
