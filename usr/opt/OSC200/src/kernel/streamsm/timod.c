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
static char *rcsid = "@(#)$RCSfile: timod.c,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/05/12 15:15:37 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989  Mentat Inc.
 ** timod.c 1.3, last change 1/29/90
 **/

#include <timod.h>		/* XXX dynamic dependency */
#undef	TIMOD

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/sysconfig.h>

#include <streamsm/tiuser.h>
#include <streamsm/tihdr.h>
#include <streamsm/timod.h>
#ifndef staticf
#define staticf static
#endif

#define	MAXPSZ_TO_FORCE_PUTMSG_ERRS	1
#define	MINPSZ_TO_FORCE_PUTMSG_ERRS	32767

typedef	struct iocblk	* IOCP;

typedef struct timod_s {
	uint	timod_mode;	/* TLI or XTI */
	uint	timod_qlen;	/* Saved qlen parameter for XTI */
	mblk_t *	timod_mp;	/* Save mp on IOCTL requests */
} TIMOD, * TIMODP;

/* Definitions for timod_mode */
#define	TIMOD_TLI	0x0000	/* TLI stream */
#define	TIMOD_XTI	0x0001	/* XTI stream */
#define TIMOD1_0	0x0000
#define TIMOD1_1	0x0100  /* new version of timod and libtli */


#ifdef XTI11
staticf	int	timod_close(queue_t * q, int flag, cred_t * cred);
staticf	int	timod_open(queue_t * q, dev_t *dev, int flag, int sflag, cred_t * cred);
#else
staticf	int	timod_close(queue_t * q);
staticf	int	timod_open(queue_t * q, dev_t dev, int flag, int sflag);
#endif
staticf	int	timod_rput(queue_t * q, mblk_t * mp);
staticf	int	timod_wput(queue_t * q, mblk_t * mp);

static struct module_info minfo =  {
#define	MODULE_ID	5006
	MODULE_ID, "timod", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	timod_rput, NULL, timod_open, timod_close, NULL, &minfo
};

static struct qinit winit = {
	timod_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab timodinfo = { &rinit, &winit };

#define STRVERS         OSF_STREAMS_11
#define STRFLAGS        STR_IS_MODULE|STR_SYSV4_OPEN
#define STRSYNCL        SQLVL_QUEUEPAIR
#define STRNAME         "timod"
#define STRCONFIG       timod_configure
#define STRINFO         timodinfo

#include "streamsm/template.c"

#ifdef XTI11
staticf int
timod_close (q, flag, cred)
	queue_t	*	q;
	int		flag;
	cred_t *	cred;
#else
static	caddr_t	timod_g_head;

staticf int
timod_close (queue_t *q)
#endif
{
	TIMODP		timod;

	timod = (TIMODP)q->q_ptr;
	if (timod->timod_mp)
		freemsg(timod->timod_mp);
#ifdef XTI11
	return streams_close_comm(q, flag, cred);
#else
	return mi_close_comm(&timod_g_head, q);
#endif
}

#ifdef XTI11
staticf int
timod_open (q, dev, flag, sflag, cred)
	queue_t	*	q;
	dev_t		*dev;
	int		flag;
	int		sflag;
	cred_t *	cred;
{
	return streams_open_comm(sizeof(TIMOD), q, dev, flag, sflag, cred);
}
#else
staticf int
timod_open (q, dev, flag, sflag)
	queue_t	*	q;
	dev_t		dev;
	int		flag;
	int		sflag;
{
	return mi_open_comm(&timod_g_head, sizeof(TIMOD), q, dev, flag, sflag);
}
#endif
/*
 * Read-side put routine. tmod_rput is mainly interested in M_PROTO and
 * M_PCPROTO messages; these are handled according to their TPI primitive.
 * Other messages are passed through without change.
 */
staticf int
timod_rput (q, mp)
	queue_t	* 		q;
	mblk_t *		mp;
{
	struct iocblk *		iocp;
	mblk_t *		mp1;
	struct T_error_ack *	terr;
	TIMODP			timod;

	timod = (TIMODP)q->q_ptr;
	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
		/* Get the address of the saved M_IOCTL message, if any. */
		mp1 = timod->timod_mp;

		switch (((union T_primitives *)mp->b_rptr)->type) {
		case T_ERROR_ACK:
			if (!mp1) {
				/*
				 * If there was no saved M_IOCTL message,
				 * just pass the message through.
				 */
				putnext(q, mp);
				return;
			}
			/*
			 * Convert the original M_IOCTL message into a
			 * M_IOCACK with the TPI error values encoded
			 * in ioc_rval.
			 */
			terr = (struct T_error_ack *)mp->b_rptr;
			iocp = (struct iocblk *)mp1->b_rptr;
			iocp->ioc_rval = (terr->UNIX_error << 8) | (terr->TLI_error & 0xff);
			iocp->ioc_count = 0;
			freemsg(mp);
			break;
		case T_BIND_ACK:
			timod->timod_qlen = ((struct T_bind_ack *)mp->b_rptr)->CONIND_number;
			/* fallthru */
		case T_OK_ACK:
		case T_OPTMGMT_ACK:
		case T_INFO_ACK:
		case T_GETADDR_ACK:
			if (!mp1) {
				/*
				 * If there was no saved M_IOCTL message,
				 * just pass the message through.
				 */
				putnext(q, mp);
				return;
			}
			/*
			 * Convert the original M_IOCTL message into a
			 * valid M_IOCACK.
			 */
			iocp = (struct iocblk *)mp1->b_rptr;
			mp1->b_cont = mp;
			do {
				mp->b_datap->db_type = M_DATA;
			} while (mp = mp->b_cont);
			iocp->ioc_count = msgdsize(mp1->b_cont);
			break;
		case T_DISCON_IND:
			if (timod->timod_mode & TIMOD_XTI) {
				/*
				 * If we're in XTI mode, we have to make sure
				 * that a subsequent t_snd or similar call
				 * is notified of the pending error.
				 */

				WR(q)->q_minpsz = MINPSZ_TO_FORCE_PUTMSG_ERRS;
				WR(q)->q_maxpsz = MAXPSZ_TO_FORCE_PUTMSG_ERRS;
			}
			/* fallthru */
		case T_CONN_CON:
		case T_CONN_IND:
			if (!(timod->timod_mode & TIMOD1_1))
				goto done;
			if (mp1 = mp->b_cont)
				mp->b_cont = 0;
			putnext(q, mp);
			if (mp1)
				putnext(q, mp1);
			return;
		case T_ORDREL_IND:
		case T_UDERROR_IND:
			if (timod->timod_mode & TIMOD_XTI) {
				/*
				 * If we're in XTI mode, we have to make sure
				 * that a subsequent t_snd or similar call
				 * is notified of the pending error.
				 */
				WR(q)->q_minpsz = MINPSZ_TO_FORCE_PUTMSG_ERRS;
				WR(q)->q_maxpsz = MAXPSZ_TO_FORCE_PUTMSG_ERRS;
			}
			/* fallthru */
		case T_DATA_IND:
		case T_EXDATA_IND:
		case T_UNITDATA_IND:
		default:
			putnext(q, mp);
			return;
		}
		mp = mp1;
		mp->b_datap->db_type = M_IOCACK;
		timod->timod_mp = 0;
		break;
	default:
		break;
	}
done:
	putnext(q, mp);
}

/*
 * Write-side put routine. tmod_wput is only interested in certain
 * M_IOCTL messages which are converted into M_PROTO TPI messages.
 * Other messages are passed through without change.
 */
staticf int
timod_wput (q, mp)
	queue_t	*	q;
	mblk_t * 	mp;
{
	struct iocblk *	iocp;
	long *		lp;
	mblk_t *	mp1;
	TIMODP		timod;

	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
						/* OSF1.0 lib compat */
		timod = (TIMODP)q->q_ptr;
		if (!(timod->timod_mode & TIMOD1_1)
	  	&& (T_FEEDBACK_REQ == ((union T_primitives *)mp->b_rptr)->type)) {
			lp = (long *)mp->b_rptr;
			q = RD(q)->q_next;
			while (q->q_next) {
				q = q->q_next;
			}
			mp->b_rptr += (sizeof(long) * 2);
			if (lp[1] != 0  &&  !(mp1 = getq(q))) {
				freemsg(mp);
				return 0;
			}
			switch (lp[1]) {
			case 0:
				break;
			case MOREDATA:
				linkb(mp, mp1);
				break;
			case MORECTL|MOREDATA:
				linkb(mp, mp1->b_cont);
				/* fall through */;
			case MORECTL:
				mp1->b_cont = mp->b_cont;
				mp->b_cont = mp1;
				break;
			default:
				freemsg(mp);
				break;
			}
			insq(q, q->q_first, mp);
			return 0;
		}
		break;
	case M_IOCTL:
		timod = (TIMODP)q->q_ptr;
		mp1 = mp->b_cont;
		switch (((struct iocblk *)mp->b_rptr)->ioc_cmd) {
		case TI_GETPEERNAME:
		case TI_GETINFO:
		case 2:                 /* OSF1.0 bug compat */
			mp1->b_datap->db_type = M_PCPROTO;
			break;
		case TI_BIND:
		case 1:                 /* OSF1.0 bug compat */
		case TI_UNBIND:
		case 4:                 /* OSF1.0 bug compat */
		case TI_OPTMGMT:
		case 3:                 /* OSF1.0 bug compat */
			mp1->b_datap->db_type = M_PROTO;
			break;
		case TI_XTI_HELLO:
		case 5:                 /* OSF1.0 bug compat */
			/* We're now in XTI mode. */
			timod->timod_mode |= TIMOD_XTI;
			goto ack;
		case TI_XTI_GET_STATE:
		case 6:                 /* OSF1.0 bug compat */
			/* Get extended state info for XTI. */
			iocp = (struct iocblk *)mp->b_rptr;
			mp->b_datap->db_type = M_IOCACK;
			if (iocp->ioc_count >= sizeof(XTIS)) {
				XTISP	xtis = (XTISP)mp1->b_rptr;

				xtis->xtis_qlen = timod->timod_qlen;
				mp1->b_wptr = mp1->b_rptr + sizeof(XTIS);
				iocp->ioc_count = sizeof(XTIS);
			} else
				iocp->ioc_error = EINVAL;
			qreply(q, mp);
			return;
		case TI_XTI_CLEAR_EVENT:
		case 7:                 /* OSF1.0 bug compat */
			q->q_maxpsz = minfo.mi_maxpsz;
			q->q_minpsz = minfo.mi_minpsz;
			goto ack;
			
		case TI_XTI_MODE:
			timod->timod_mode |= TIMOD_XTI;
			/* fallthru */
		case TI_TLI_MODE:
			timod->timod_mode |= TIMOD1_1;
			goto ack;
		default:
			putnext(q, mp);
			return;
		}
		mp->b_cont = 0;
		if ( timod->timod_mp )
			freemsg(timod->timod_mp);
		timod->timod_mp = mp;
		mp = mp1;
		break;
	default:
		break;
	}
	putnext(q, mp);
	return;

ack:	mp->b_datap->db_type = M_IOCACK;
	qreply(q, mp);
	return;
}
