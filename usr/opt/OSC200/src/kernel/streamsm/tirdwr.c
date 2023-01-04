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
static char *rcsid = "@(#)$RCSfile: tirdwr.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/12 15:15:55 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
    
/** Copyright (c) 1990  Mentat Inc.
 ** tirdwr.c 1.3, last change 5/12/90
 **/
/*static	char	sccsid[] = "@(#)tirdwr.c\t\t1.3";*/

#include <tirdwr.h>		/* XXX dynamic dependency */

#include <sys/param.h>
#include <sys/user.h>

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/sysconfig.h>

#include <streamsm/tiuser.h>
#include <streamsm/tihdr.h>
#include <streamsm/timod.h>

#ifndef	staticf
#define staticf static
#endif

staticf	int	tirdwr_close(queue_t *q, int flag, cred_t *cred);
staticf	int	tirdwr_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *cred);
staticf	int	tirdwr_rput(queue_t *q, mblk_t *mp);
staticf	int	tirdwr_wput(queue_t *q, mblk_t *mp);

static struct module_info minfo =  {
	0, "tirdwr", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	tirdwr_rput, NULL, tirdwr_open, tirdwr_close, NULL, &minfo
};

static struct qinit winit = {
	tirdwr_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab tirdwrinfo = { &rinit, &winit };

#define STRVERS         OSF_STREAMS_11
#define STRFLAGS        STR_IS_MODULE|STR_SYSV4_OPEN
#define STRSYNCL        SQLVL_QUEUE
#define STRNAME         "tirdwr"
#define STRCONFIG       tirdwr_configure
#define STRINFO         tirdwrinfo

#include "streamsm/template.c"



staticf mblk_t *
ti_tpi_trailer_alloc (trailer_mp, size, type)
	mblk_t	* trailer_mp;
	int	size;
	int	type;
{
	mblk_t	* mp;

	if (mp = allocb(size, BPRI_MED)) {
		mp->b_cont = trailer_mp;
		mp->b_datap->db_type = M_PROTO;
		((union T_primitives *)mp->b_rptr)->type = type;
		mp->b_wptr += size;
	}
	return mp;
}

staticf mblk_t *
ti_tpi_discon_req (trailer_mp, seqnum)
	mblk_t	* trailer_mp;
	int	seqnum;
{
	mblk_t	* mp;
	struct T_discon_req	* tdr;

	if (mp = ti_tpi_trailer_alloc(trailer_mp, sizeof(struct T_discon_req),T_DISCON_REQ)) {
		tdr = (struct T_discon_req *)mp->b_rptr;
		tdr->SEQ_number = seqnum;
	}
	return mp;
}

staticf mblk_t *
ti_tpi_ordrel_req ()
{
	mblk_t	* mp;

	if (mp = allocb(sizeof(struct T_ordrel_req), BPRI_HI)) {
		mp->b_datap->db_type = M_PROTO;
		((struct T_ordrel_req *)mp->b_rptr)->PRIM_type = T_ORDREL_REQ;
		mp->b_wptr += sizeof(struct T_ordrel_req);
	}
	return mp;
}

/* NOTE: the read queue 'q_ptr' field designates what type of
** request to issue to the underlying transport provider when we close.
*/

staticf int
tirdwr_close (q, flag, cred)
	queue_t	* q;
	int	flag;
	cred_t	* cred;
{
	mblk_t * 	mp;

	switch ((unsigned int)q->q_ptr) {
	case T_ORDREL_REQ:
		mp = ti_tpi_ordrel_req();
		break;
	case T_DISCON_REQ:
		mp = ti_tpi_discon_req( (mblk_t *)0, -1);
		break;
	default:
		return 0;
	}
	if (mp)
		putnext(WR(q), mp);
	return 0;
}

staticf int
tirdwr_open (q, devp, flag, sflag, cred)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* cred;
{
	mblk_t * 	mp;
	mblk_t * 	mp1;
	mblk_t *	mp2;
	queue_t *	sthq;
	union T_primitives	* tprim;

	if (q->q_ptr)
		return 0;
	/* Munge through the stream head queue looking for mblks that
	** 'tirdwr_rput' would not have allowed through in the first place.
	** If any one of these 'untouchables' is present return OPENFAIL.
	*/
	sthq = q->q_next;
	for (mp = sthq->q_first; mp; mp = mp1) {
		mp1 = mp->b_next;
		switch (mp->b_datap->db_type) {
		case M_PROTO:
			tprim = (union T_primitives *)mp->b_rptr;
			if ((mp->b_wptr - mp->b_rptr) < sizeof(tprim->type))
				break;
			if (tprim->type != T_DATA_IND
			&&  tprim->type != T_EXDATA_IND)
				break;
			mp2 = mp->b_cont;
			rmvq(sthq, mp);
			freeb(mp);
			if (!(mp = mp2))
				continue;
			insq(sthq, mp1, mp);
			/* fall through */;
		case M_DATA:
			if (msgdsize(mp) == 0) {
				rmvq(sthq, mp);
				freemsg(mp);
			}
			continue;
		case M_SIG:
		case M_PCSIG:
			continue;
		}
		return EPROTO;
	}
	/* Set for default close action */
	q->q_ptr = (caddr_t)T_DISCON_REQ;
	return 0;
}

staticf int
tirdwr_rput (q, mp)
	queue_t	* q;
	mblk_t * mp;
{
	mblk_t * 	mp1;
	union T_primitives	* tprim;

	switch (mp->b_datap->db_type) {
	case M_DATA:
		/* Blow off zero length pkts so as not to create false EOF */
		if (mp->b_wptr == mp->b_rptr
		&&  msgdsize(mp) == 0) {
			freemsg(mp);
			return 0;
		}
		break;
	case M_PCPROTO:
		freemsg(mp);
		if ((unsigned int)q->q_ptr == T_ORDREL_REQ)
			q->q_ptr = (caddr_t)T_DISCON_REQ;
		return putctl1(q->q_next, M_ERROR, EPROTO);
	case M_PROTO:
		tprim = (union T_primitives *)mp->b_rptr;
		if ((mp->b_wptr - mp->b_rptr) < sizeof(tprim->type)) {
			freemsg(mp);
			return putctl1(q->q_next, M_ERROR, EPROTO);
		}
		switch (tprim->type) {
		case T_EXDATA_IND:
			freemsg(mp);
			return putctl1(q->q_next, M_ERROR, EPROTO);
		case T_UNITDATA_IND:
		case T_DATA_IND:	/* Blow off the control portion */
			if (!(mp1 = mp->b_cont) ||  msgdsize(mp1) == 0) {
				freemsg(mp);
				return 0;
			}
			freeb(mp);
			mp = mp1;
			break;
		case T_DISCON_IND:
			q->q_ptr = (caddr_t)(T_DISCON_REQ ^ T_ORDREL_REQ);
			freemsg(mp);
			return putctl(q->q_next, M_HANGUP);
		case T_ORDREL_IND:
			q->q_ptr = (caddr_t)T_ORDREL_REQ;
			freemsg(mp);
			/* TODO: bufcall, enable service routine ... */
			if (!(mp = allocb(0, BPRI_HI)))
				return 0;
			break;
		default:
			freemsg(mp);
			return putctl1(q->q_next, M_ERROR, EPROTO);
		}
		break;
	default:
		break;
	}
	putnext(q, mp);
	return;
}

staticf int
tirdwr_wput (q, mp)
	queue_t	* q;
	mblk_t * 	mp;
{
	switch (mp->b_datap->db_type) {
	case M_DATA:
		if (mp->b_wptr == mp->b_rptr
		&&  msgdsize(mp) == 0) {
			freemsg(mp);
			return 0;
		}
		break;
	case M_PCPROTO:
	case M_PROTO:
		freemsg(mp);
		return putctl1(RD(q)->q_next, M_ERROR, EPROTO);
	default:
		break;
	}
	putnext(q, mp);
	return;
}
