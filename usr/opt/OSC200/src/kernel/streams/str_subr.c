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
static char *rcsid = "@(#)$RCSfile: str_subr.c,v $ $Revision: 4.2.5.4 $ (DEC) $Date: 1993/08/02 20:41:52 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989-1991  Mentat Inc.  **/

#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/stat.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>

STHP
sth_alloc()
{
	STHP	sth;

	STR_MALLOC(sth, STHP, sizeof *sth, M_STRHEAD, M_WAITOK);
	bzero(sth,sizeof(*sth));
	sth->sth_pollq.next = sth->sth_pollq.prev = (POLLQP)&sth->sth_pollq;
	sth->sth_sigsq.next = sth->sth_sigsq.prev = (SIGSQ *)&sth->sth_sigsq;
	simple_lock_init(&sth->sth_ext_flags_lock);
	return sth;
}

void
sth_free (sth)
	STHP	sth;
{
#if	MACH_ASSERT
	if (sth->sth_ext_flags)
		panic("sth_free sth_ext_flags");
#endif
	STR_FREE(sth, M_STRHEAD);
}

queue_t *
q_alloc ()
{
reg	queue_t	* q;
static	queue_t	q_template;

	STR_MALLOC(q, queue_t *, sizeof(queue_t) * 2, M_STRQUEUE, M_WAITOK);
	q[0] = q_template;
	q[1] = q_template;
	q[0].q_other = &q[1];
	q[1].q_other = &q[0];

	/* Allocate the synch queues */
	STR_MALLOC(q[0].q_runq_sq, SQP, 2 * sizeof (SQ), M_STRSQ, M_WAITOK);
	q[1].q_runq_sq = &(q[0].q_runq_sq)[1];

	/* Initialize the Read queue */
	q->q_flag = QUSE | QREADR | QWANTR | QSYNCH;
	sqh_init(&q->q_sqh);
	runq_sq_init(q);
	simple_lock_init(&q->q_qlock);

	/* Initialize the Write queue */
	q = WR(q);
	q->q_flag = QUSE | QWANTR | QSYNCH;
	sqh_init(&q->q_sqh);
	runq_sq_init(q);
	simple_lock_init(&q->q_qlock);

	return RD(q);
}

int
q_free (q)
	queue_t	* q;
{
	flushq(&q[0], FLUSHALL|FLUSH_CAN_CLOSE);
	flushq(&q[1], FLUSHALL);
	/*
	 * Noone should look at this flag after the queue is freed,
	 * but just in case...
	 */
	q[0].q_flag &= ~QUSE;
	q[1].q_flag &= ~QUSE;
	if (q[0].q_bandp)
		STR_FREE(q[0].q_bandp, M_STRQBAND);
	if (q[1].q_bandp)
		STR_FREE(q[1].q_bandp, M_STRQBAND);
#if	MACH_ASSERT
	if (q[0].q_runq_sq->sq_flags || q[1].q_runq_sq->sq_flags)
		panic("q_free q_runq_sq");
#endif
	STR_FREE(q[0].q_runq_sq, M_STRSQ);
	STR_FREE(q, M_STRQUEUE);
	return 0;
}

void
sth_set_queue (q, rinit, winit)
reg	queue_t		* q;
	struct qinit	* rinit;
	struct qinit	* winit;
{
reg	struct module_info * mi;

	if ((q->q_qinfo = rinit) &&  (mi = rinit->qi_minfo)) {
		q->q_minpsz = mi->mi_minpsz;
		q->q_maxpsz = mi->mi_maxpsz;
		q->q_hiwat = mi->mi_hiwat;
		q->q_lowat = mi->mi_lowat;
	}
	q = WR(q);
	if ((q->q_qinfo = winit) &&  (mi = winit->qi_minfo)) {
		q->q_minpsz = mi->mi_minpsz;
		q->q_maxpsz = mi->mi_maxpsz;
		q->q_hiwat = mi->mi_hiwat;
		q->q_lowat = mi->mi_lowat;
	}
}

STHPP
sth_muxid_lookup (sth, muxid, is_persistent)
reg	STHP	sth;
	int	muxid;
	int	is_persistent;
{
reg	STHPP	sthpp;

	sthpp = is_persistent ? &sth->sth_pmux_top : &sth->sth_mux_top;
	if (muxid != MUXID_ALL) {
		for ( ; sth = *sthpp; sthpp = &sth->sth_mux_link) {
			if (sth->sth_muxid == muxid)
				break;
		}
	}
	return sthpp;
}

static	int			iocblk_id;
decl_simple_lock_data(static,iocblk_id_lock)

void
sth_iocblk_init()
{
	simple_lock_init(&iocblk_id_lock);
}

int
sth_iocblk()
{
	int id;

	simple_lock(&iocblk_id_lock);
	if ((id = ++iocblk_id) == 0)
		id = ++iocblk_id;
	simple_unlock(&iocblk_id_lock);
	return id;
}

MBLKP
sth_link_alloc (osr, cmd, muxid, qtop, qbot)
	OSRP	osr;
	int	cmd;
	int	muxid;
	queue_t	* qtop;
	queue_t	* qbot;
{
	MBLKP	mp;
	struct iocblk	* iocp;
	struct linkblk	* linkp;

	if (!(mp = allocb(sizeof(struct iocblk), BPRI_MED)))
		return nil(MBLKP);
	if (!(mp->b_cont = allocb(sizeof(struct linkblk), BPRI_MED))) {
		freeb(mp);
		return nil(MBLKP);
	}
	mp->b_datap->db_type = M_IOCTL;
	iocp = (struct iocblk *)mp->b_rptr;
	mp->b_wptr += sizeof(struct iocblk);
	iocp->ioc_cmd = cmd;
	iocp->ioc_cr = osr->osr_creds;
	iocp->ioc_id = sth_iocblk();
	iocp->ioc_count = sizeof(struct linkblk);
	iocp->ioc_error = 0;
	iocp->ioc_rval = 0;
	linkp = (struct linkblk *)mp->b_cont->b_rptr;
	mp->b_cont->b_wptr += sizeof(struct linkblk);
	linkp->l_index = muxid;
	linkp->l_qtop = qtop;
	linkp->l_qbot = qbot;
	return mp;
}

int
sth_read_reset (osr)
	OSRP	osr;
{
	osr->osr_rw_count  = osr->osr_rw_uio->uio_resid;
	osr->osr_rw_offset = 0;
	osr->osr_rw_total  = 0;
	return 0;
}

int
sth_read_seek (osr, whence, offset)
	OSRP	osr;
	int	whence;
	long	offset;
{
	long newoffset;

	switch (whence) {
	case 0:
		newoffset = offset;
		break;
	case 1:
		newoffset = osr->osr_rw_offset + offset;
		break;
	case 2:
		newoffset = (osr->osr_rw_offset + osr->osr_rw_uio->uio_resid) + offset;
		break;
	default:
		return EINVAL;
	}
	if (newoffset >= 0 && newoffset < osr->osr_rw_uio->uio_resid) {
		osr->osr_rw_offset = newoffset;
		return 0;
	}
	return EINVAL;
}

int
close_wrapper (args)
	struct open_args *args;
{
	int error;

	if (args->a_func == 0)
		return 0;
	if (args->a_queue->q_flag & QOLD)
		error = (*(strclose_V3)(args->a_func))(args->a_queue);
	else
		error = (*(strclose_V4)(args->a_func))
			(args->a_queue, args->a_fflag, args->a_creds);
	/* sanity check */
	switch (error) {
	default:
		if (error >= 0 && error < 128)
			break;
	case EJUSTRETURN:
	case ERESTART:
STR_DEBUG(printf("Streams close: would have returned %d\n", error));
		/* Nothing to restart */
		error = (error == ERESTART) ? EINTR : 0;
	}
	return error;
}

int
open_wrapper (args)
	struct open_args *args;
{
	int error;

	if (args->a_func == 0)
		return ENXIO;
	if (args->a_queue->q_flag & QOLD) {
		int dev;
		/*
		 * V3.2 u.u_error compatibility hack. See sys/stream.h.
		 */
		u.u_error = 0;
		dev = (*(stropen_V3)(args->a_func))
			(args->a_queue, args->a_devp ? *args->a_devp : NODEV,
				args->a_fflag, args->a_sflag);
		if (dev == OPENFAIL) {
			error = (u.u_error ? u.u_error : ENXIO);
		} else {
			error = 0;
			if (args->a_devp)
				*args->a_devp = dev;
		}
	} else
		error = (*(stropen_V4)(args->a_func))
				(args->a_queue, args->a_devp,
				args->a_fflag, args->a_sflag, args->a_creds);
	/* sanity check */
	switch (error) {
	default:
		if (error >= 0 && error < 128)
			break;
	case ERESTART:
	case EJUSTRETURN:
STR_DEBUG(printf("Streams open: would have returned %d\n", error));
		/* Too dangerous to restart with a pathological driver */
		error = (error == ERESTART) ? EINTR : ENXIO;
	}
	return error;
}

/*
 * sth_uiomove() and sth_uiodone().
 *
 * Like uiomove, except they defer altering the iov list until
 * the copies are complete. This is needed for sth_read_seek
 * and sth_read_reset, and used only in osr_read() and osr_write().
 */
int
sth_uiomove (cp, n, osr)
	register caddr_t cp;
	register int n;
	OSRP osr;
{
	register struct iovec *iov;
	register struct uio *uio;
	u_int cnt, resid, offset;
	int error = 0;

	uio = osr->osr_rw_uio;
	if (uio->uio_rw != UIO_READ && uio->uio_rw != UIO_WRITE)
		panic("sth_uiomove: mode");
	resid = osr->osr_rw_count;
	offset = osr->osr_rw_offset;
	iov = uio->uio_iov;
	while (n > 0 && resid > 0) {
		while (offset >= iov->iov_len) {
			offset -= iov->iov_len;
			iov++;
		}
		cnt = iov->iov_len - offset;
		if (cnt > n)
			cnt = n;
		switch (uio->uio_segflg) {

		case UIO_USERSPACE:
		case UIO_USERISPACE:
			if (uio->uio_rw == UIO_READ)
				error = copyout(cp, iov->iov_base + offset, cnt);
			else
				error = copyin(iov->iov_base + offset, cp, cnt);
			if (error)
				return (error);
			break;

		case UIO_SYSSPACE:
			if (uio->uio_rw == UIO_READ)
				bcopy((caddr_t)cp, iov->iov_base + offset, cnt);
			else
				bcopy(iov->iov_base + offset, (caddr_t)cp, cnt);
			break;
		}
		iov++;
		osr->osr_rw_count -= cnt;
		resid -= cnt;
		osr->osr_rw_offset += cnt;
		offset = 0;
		osr->osr_rw_total += cnt;
		cp += cnt;
		n -= cnt;
	}
	return (error);
}

void
sth_uiodone (osr)
	OSRP	osr;
{
	struct uio *uio = osr->osr_rw_uio;
	struct iovec *iov = uio->uio_iov;

	uio->uio_resid -= osr->osr_rw_total;
	uio->uio_offset += osr->osr_rw_total;
	while (osr->osr_rw_total > iov->iov_len) {
		osr->osr_rw_total -= iov->iov_len;
		iov->iov_len = 0;
		iov++;
	}
	iov->iov_len -= osr->osr_rw_total;
	iov->iov_base += osr->osr_rw_total;
}
