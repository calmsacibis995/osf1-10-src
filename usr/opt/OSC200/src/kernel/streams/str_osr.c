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
static char *rcsid = "@(#)$RCSfile: str_osr.c,v $ $Revision: 4.2.11.12 $ (DEC) $Date: 1993/10/27 22:31:37 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1989-1991  Mentat Inc.
 **/

#include <sys/secdefines.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <sys/mode.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/buf.h>	/* for B_WRITE, sigh */
#include <sys/mbuf.h>
#include <sys/syslimits.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>
#include <strpush.h>
#include <streams/memdebug.h>

#if SEC_BASE
extern mblk_t	*str_copyin_attrs(struct strbuf *, int *);
extern int	 str_copyout_attrs(mblk_t *, struct strbuf *);
#include <kern/assert.h>
#endif

typedef	struct iocblk	* IOCP;

#ifdef NSTRPUSH
int nstrpush = NSTRPUSH;		/* defined in config file */
#else
int nstrpush = 9;			/* default to 9 */
#endif

/*
 * Types and structures which are local to this source file.
 */

/*
 * We must ensure no data are headed for the
 * stream head from below. We need a more
 * reliable method than this...
 */
static int hangup_tries = 10;

/*
 * Test for out-of-range write, putmsg, or putpmsg.
 */
#define	bad_put_length(len, q)		( \
	((q)->q_next->q_minpsz > 0 && (len) < (q)->q_next->q_minpsz) || \
	((q)->q_next->q_maxpsz != INFPSZ && (len) > (q)->q_next->q_maxpsz) \
)

#define	bad_write_length(len, q)	( \
	((q)->q_next->q_minpsz > 0 && (len) < (q)->q_next->q_minpsz) || \
	((q)->q_next->q_maxpsz != INFPSZ && (len) > (q)->q_next->q_maxpsz && \
		 ((q)->q_next->q_minpsz > 0 || (q)->q_next->q_maxpsz <= 0)) \
)

/*
 * Structure of an M_PASSFP message - stream head business only.
 */
struct	strpfp {
	struct file_cookie	pass_file_cookie; /* file 'pointer' */
	uid_t			pass_uid;	  /* user id of sender */
	gid_t			pass_gid;	  /* group id of sender */
};

/*
 * Minor exported interface.
 *
 * discard_passfp is an interface for flushq. This allows us to keep
 * the structure of M_PASSFP messages local to this file.
 */
void
discard_passfp(mp)
	MBLKP		mp;
{
	cookie_destroy(&((struct strpfp *)(mp->b_rptr))->pass_file_cookie);
}

/*
 * Getmsg/putmsg subroutines. Used by {get,put}[p]msg(), peek, fdinsert.
 */

staticf int
osr_getmsg_subr (
	OSRP		osr,
	struct strbuf *	databuf,
	struct strbuf *	ctlbuf,
#if SEC_BASE
	struct strbuf *	attrbuf,
#endif
	long *		flags,
	int *		band,
	int		peeking)
{
	STHP		sth = osr->osr_sth;
	int		error = 0;
reg	MBLKP		mp;
	MBLKP		bmp;
	MBLKP		start_mp;
	int		ctl_to_get;
	caddr_t		ctl;
	int		data_to_get;
	caddr_t		data;
	int		cnt;
	int		rval;
	int		can_delete;
	int		tryhard = hangup_tries;
	long		notifycnt = 0;

	if (sth->sth_flags & F_STH_PIPE)
		sth_update_times(sth, FREAD, (struct stat *)0);

#if SEC_BASE
	attrbuf->len = -1;
#endif
beginning:
	for (;;) {
		if (mp = sth->sth_rq->q_first) {
			if (mp->b_datap->db_type >= QPCTL) {
				*flags = MSG_HIPRI;
				*band = 0;
				break;
			}
			if (*flags == MSG_BAND) {
				/* V.4 compat, do not set band */
				if (mp->b_band >= *band)
					break;
			} else if (*flags != MSG_HIPRI) {
				*flags = MSG_BAND;
				*band = mp->b_band;
				break;
			}
		} else if (sth->sth_flags & F_STH_HANGUP) {
			if (tryhard-- > 0) {
				osr->osr_flags |= F_OSR_NONBLOCK;
				(void) osr_sleep(osr, FALSE, 0);
				continue;
			}
			ctlbuf->len = 0;
			databuf->len = 0;
			return 0;
		}
		if (error = osr_sleep(osr, TRUE, 0))
			return error;
	}

	if (peeking) {
	    if ((mp = dupmsg(mp)) == NULL)  {
	        /*
		 *  If dupmsg fails - do an allocate with BPRI_WAITOK
		 *  to grow the free pool.  This raises the chance
		 *  of success next time.
		 */
		csq_release(&sth->sth_rq->q_sqh);
		mp = allocb(0, BPRI_WAITOK);
		if (mp) freeb(mp);
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);	
	        goto beginning;	      
	    }
	} else
	    mp = sth_getq(sth);
	csq_release(&sth->sth_rq->q_sqh);

	bmp = start_mp = mp;
	ctl_to_get = ctlbuf->maxlen;
	ctl = ctlbuf->buf;
	ctlbuf->len = -1;
	data_to_get = databuf->maxlen;
	data = databuf->buf;
	databuf->len = -1;
	rval = 0;
#if SEC_BASE
	if (mp->b_attr && (error = str_copyout_attrs(mp->b_attr, attrbuf)))
		goto putback;
#endif
	do {
		cnt = mp->b_wptr - mp->b_rptr;
		switch (mp->b_datap->db_type) {
		case M_HPDATA:
			/* Handle data originally part of a high priority
			 * message whose control portion has been consumed.
			 */
			*flags = RS_HIPRI;
			/* FALLS THROUGH */
		case M_DATA:
			if (cnt > data_to_get) {
				cnt = data_to_get;
				rval |= MOREDATA;
			}
			can_delete = (databuf->maxlen >= 0);
			if (databuf->len == -1  &&  can_delete)
				databuf->len = 0;
			if (cnt > 0) {
				error = copyout((caddr_t)mp->b_rptr, data, cnt);
				if (error == 0) {
					data_to_get -= cnt;
					data += cnt;
					databuf->len += cnt;
					mp->b_rptr += cnt;
					if (mp->b_flag & MSGNOTIFY)
						notifycnt += cnt;
				}
			}
			break;
		case M_PCPROTO:
			*flags = RS_HIPRI;
			/* FALLS THROUGH */
		case M_PROTO:
			if (cnt > ctl_to_get) {
				cnt = ctl_to_get;
				rval |= MORECTL;
			}
			can_delete = (ctlbuf->maxlen >= 0);
			if (ctlbuf->len == -1  &&  can_delete)
				ctlbuf->len = 0;
			if (cnt > 0) {
				error = copyout((caddr_t)mp->b_rptr, ctl, cnt);
				if (error == 0) {
					ctl_to_get -= cnt;
					ctl += cnt;
					ctlbuf->len += cnt;
					mp->b_rptr += cnt;
					if (mp->b_flag & MSGNOTIFY)
						notifycnt += cnt;
				}
			}
			break;
		default:
			error = EBADMSG;
			goto done;
		}
		if (!peeking && can_delete && mp->b_rptr >= mp->b_wptr) {
			if (start_mp == mp) {
#if SEC_BASE
				/* Move the attributes up to the new head. */
				if (mp->b_cont) {
					ASSERT(mp->b_cont->b_attr == NULL);
					mp->b_cont->b_attr = mp->b_attr;
					mp->b_attr = NULL;
				}
#endif
				start_mp = bmp = mp->b_cont;
			} else
				bmp->b_cont = mp->b_cont;
			freeb(mp);
			mp = bmp;
		} else {
			bmp = mp;
			mp = mp->b_cont;
		}
	} while (!error  &&  mp);

done:
	if (!peeking && (start_mp || notifycnt))
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
	if (start_mp) {
		/* If we ate all of the first part of a high priority
		 * message, but not all the data, we have to fix the
		 * remaining blocks so we can set MSG_HIPRI and band
		 * if they are subsequently read using get[p]msg.
		 */
		if (!peeking && rval == MOREDATA) {
			mp = start_mp;
			if (mp->b_datap->db_type == M_DATA 
			&&  *flags == MSG_HIPRI)
				mp->b_datap->db_type = M_HPDATA;
			mp->b_band = (mp->b_datap->db_type < QPCTL) ? *band : 0;
		}
putback:
		/* Band may have changed - check putbq return */
		if (peeking || putbq(sth->sth_rq, start_mp) == 0)
			freemsg(start_mp);
	}
	if (error == 0)
		osr->osr_ret_val = rval;
	if (!peeking && notifycnt) {
		while ((mp = allocb(sizeof(long), BPRI_HI)) == NULL)
			(void)osr_bufcall(osr, FALSE, 0, sizeof(long), BPRI_HI);
		*(long *)mp->b_rptr = notifycnt;
		mp->b_wptr += sizeof (long);
		mp->b_datap->db_type = M_NOTIFY;
		putnext(sth->sth_wq, mp);
	}
	return error;
}

staticf int
osr_putmsg_subr (
	OSRP		osr,
	struct strbuf *	databuf,
	struct strbuf *	ctlbuf,
#if SEC_BASE
	struct strbuf *	attrbuf,
#endif
	long		flags,
	int		band,
	queue_t *	q,
	int		qoff)
{
	STHP		sth = osr->osr_sth;
	MBLKP		mp = nil(MBLKP);
reg	MBLKPP		mpp = &mp;
	int		totlen;
reg	int		len;
	int		error;
	char *          dataptr;

	if (sth->sth_flags & F_STH_PIPE)
		sth_update_times(sth, FWRITE, (struct stat *)0);

	if ((len = databuf->len) >= 0 && ((STRMSGSZ > 0 && len > STRMSGSZ)
	||   bad_put_length(len, sth->sth_wq)))
		return ERANGE;

	for (totlen = ctlbuf->len; totlen >= 0; ) {
		/*
		 * Control is passed in a single mblk. This is from B-7
		 * the SPG. If STRCTLSZ is 0 (not mentioned in V.4 SPG)
		 * then limit it as protection against huge sizes and
		 * allocb max.
		 */
		len = MAX(totlen, MIN_CTL_BUF);
		if (STRCTLSZ > 0) {
			if (len > STRCTLSZ || (STRMSGSZ > 0 && len > STRMSGSZ))
				return ERANGE;
		} else if (len > PAGE_SIZE || (STRMSGSZ > 0 && len > STRMSGSZ))
			return ERANGE;
		if (!(mp = allocb(len, BPRI_WAITOK))) {
			if (q)	/* fdinsert - don't wait */
				return EAGAIN;
			if (error = osr_bufcall(osr, TRUE, 0, len, BPRI_LO))
			{
				return(error);
			}
		}
		if (totlen > 0) {
			csq_release(&sth->sth_rq->q_sqh);
			if (error = copyin((caddr_t)ctlbuf->buf,
					(caddr_t)mp->b_rptr, totlen)) {
				freemsg(mp);
				return error;
			}
			if (q)	/* fdinsert - qoff checked there */
				*((queue_t **)&mp->b_rptr[qoff]) = q;
			mp->b_wptr += totlen;
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
		}
		mp->b_band = band;
		mp->b_datap->db_type = (flags == MSG_HIPRI) ?
						M_PCPROTO : M_PROTO;
		mpp = &mp->b_cont;
		break;
	}

	dataptr = databuf->buf;
	for (totlen = databuf->len; totlen >= 0; ) {
		/*
		 * Chain data into mblk's of limited size. The total
		 * length of the chain is limited only by STRMSGSZ and
		 * bad_put checks above... this could be dangerous.
		 */
		len = totlen;
		if (totlen == databuf->len)
			len += sth->sth_wroff;
		if (len > MCLBYTES)
			len = MCLBYTES;
		if ((*mpp = allocb(len, BPRI_WAITOK)) == NULL) {
			if (q) {/* fdinsert - don't wait */
				if (mp) freemsg(mp);
				return EAGAIN;
			}
			if (error = osr_bufcall(osr, TRUE, 0, len, BPRI_LO))
				return(error);
			continue;
		}
		if (totlen == databuf->len && len - sth->sth_wroff > 0) {
			(*mpp)->b_wptr = ((*mpp)->b_rptr += sth->sth_wroff);
			len -= sth->sth_wroff;
		}
		if (len > 0) {
			csq_release(&sth->sth_rq->q_sqh);
			if (error = copyin((caddr_t)dataptr,
					(caddr_t)((*mpp)->b_rptr), len)) {
				freemsg(mp);
				return error;
			}
			dataptr += len;
			(*mpp)->b_wptr += len;
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
		}
		(*mpp)->b_band = band;
		mpp = &(*mpp)->b_cont;
		if ((totlen -= len) <= 0)
			break;
	}

	if (mp) {
#if SEC_BASE
		ASSERT(mp->b_attr == NULL);
		csq_release(&sth->sth_rq->q_sqh);
		mp->b_attr = str_copyin_attrs(attrbuf, &error);
		if (error) {
			freemsg(mp);
			return error;
		}
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
#endif
		/* If the band does not exist on the stream head, create it.
		 * This must be done to keep a record of which bands are
		 * active on the stream.  Otherwise, it is possible that
		 * if a message is send down stream on a band no queue
		 * will ever create a band since the bands only get
		 * created on a putmsg.
		 */
		if(band) {
			SIMPLE_LOCK_DECL
			SIMPLE_LOCK(&sth->sth_wq->q_qlock);
			if (band > sth->sth_wq->q_nband  &&  
				!alloc_qband(sth->sth_wq, band)) {
				SIMPLE_UNLOCK(&sth->sth_wq->q_qlock);
				return 0;
			}
			SIMPLE_UNLOCK(&sth->sth_wq->q_qlock);
		}


		if (mp->b_datap->db_type >= QPCTL) {
			putnext(sth->sth_wq, mp);
		} else for (;;) {
			switch (sth_canput(sth, band)) {
			default:
				putnext(sth->sth_wq, mp);
				return 0;
			case 2:
			case 1:
				if (putq_owned(sth->sth_wq, mp) == 0) {
					freemsg(mp);
					return EAGAIN;
				}
				return 0;
			case 0:
				if ( error = osr_sleep(osr, TRUE, 0) ) {
					freemsg(mp);
					return error;
				}
				continue;
			}
		}
	}
	return 0;
}

/*
 * osr_handler routines: execute the request specified by the OSR
 *
 * The common layout for all osr_handlers:
 *
 * -	On entry, they are serialized with other OS requests for
 *	the same stream and the same queue. In some cases (i.e.
 *	when run on the ioctl_osrq) they are serialized, else
 *	they may be interleaved with other osr's under protection
 *	of the stream head lock.
 *
 * -	The calling level locked the stream head, and the mult_sqh,
 *	if they need it. They are free to clear the F_NEED_MULT_SQH
 *	flag, after they don't need it anymore. If they block after
 *	that, they will not re-acquire the mult_sqh automatically.
 *
 * -	All blocking is done using the subroutine osr_sleep. This
 *	service takes care of releasing and re-acquiring the stream head
 *	and the mult_sqh. The handlers are responsible for any other
 *	locks themselves. The handlers must release all such locks before
 *	return, and in fact may also release the stream head lock.
 *
 * -	osr_sleep takes care of synchronization on the osr.
 *
 * -	osr_sleep can be configured to check for interrupts, and to set
 *	a time limit. The return values will be EINTR and ETIME, respectively.
 *	It also checks for relevant "global" stream head errors, and returns
 *	any. A return of 0 does not imply the osr is complete.
 *
 * -	An osr_handler returns when the OSR is handled, or when an
 *	error occurred. The return value is an error indication.
 */

int
osr_atmark (osr)
	OSRP	osr;
{
	MBLKP	mp;
	int	check_mark;

	check_mark = osr->osr_ioctl_arg1;
	if (check_mark != ANYMARK  &&  check_mark != LASTMARK)
		return EINVAL;
	mp = osr->osr_sth->sth_rq->q_first;
	if (!mp  ||  !(mp->b_flag & MSGMARK))
		return 0;

	if (check_mark != ANYMARK) {
		/* Make sure there are no other marked messages on the queue  */
		while (mp = mp->b_next) {
			if (mp->b_flag & MSGMARK)
				return 0;
		}
	}
	osr->osr_ret_val = 1;
	return 0;
}

int
osr_canput (osr)
	OSRP	osr;
{
	int	band;
	int	ret_val;

	band = osr->osr_ioctl_arg1;
	if ((unsigned)band > MAX_QBAND)
		return EINVAL;
	osr->osr_ret_val = (sth_canput(osr->osr_sth, band) != 0);
	return 0;
}

int
osr_ckband (osr)
	OSRP	osr;
{
	int		band;
	qband_t		* qb;
	queue_t		* q;
	SIMPLE_LOCK_DECL

	band = osr->osr_ioctl_arg1;
	if ((unsigned)band > MAX_QBAND)
		return EINVAL;
	q = osr->osr_sth->sth_rq;
	if (band == 0) {
		if (q->q_first)
			osr->osr_ret_val = 1;
	} else {
		SIMPLE_LOCK(&q->q_qlock);
		if (band <= q->q_nband) {
			qb = &q->q_bandp[band-1];
			if (qb->qb_first)
				osr->osr_ret_val = 1;
		}
		SIMPLE_UNLOCK(&q->q_qlock);
	}
	return 0;
}

int
osr_fattach (osr)
reg	OSRP	osr;
{
	STHP	sth = osr->osr_sth;

	/*
	 * Nothing to do if not a pipe, else alloc fattach struct.
	 */
	if (!(sth->sth_flags & (F_STH_PIPE|F_STH_FIFO)))
		return 0;
	return sth_fattach(sth, osr->osr_ioctl_arg1, (void *)osr->osr_ioctl_arg2p);
}

int
osr_fdinsert (osr)
	OSRP	osr;
{
	queue_t	* q;
	STHP	other_sth;
#if SEC_BASE
	struct strfdinsert_attr * strfd =
			(struct strfdinsert_attr *)osr->osr_ioctl_arg0p;
#else
	struct strfdinsert * strfd = (struct strfdinsert *)osr->osr_ioctl_arg0p;
#endif

	if ( (strfd->flags != 0  &&  strfd->flags != RS_HIPRI)
	||   strfd->ctlbuf.len < sizeof(queue_t *)
	||   strfd->offset < 0  
	||   ALIGNMENT(strfd->offset) != 0
	||   (strfd->offset + sizeof(queue_t *)) > strfd->ctlbuf.len
	||   sth_fd_to_sth(strfd->fildes, &other_sth) != 0 )
		return EINVAL;

	/*
	 * Open issue: there is no reference on strfd->fildes, and
	 * other_sth (with its associated driver queue) may close at
	 * any time. Unfortunately there is no positive way to know
	 * when the fdinsert has completed. It might be considered
	 * to stamp the mblk and check in freeb()...
	 */
	for (q = other_sth->sth_wq; !STREAM_END(q); q = q->q_next)
		;
	q = RD(q);
	mult_sqh_release(osr);
	osr->osr_flags &= ~F_OSR_NEED_MULT_SQH;

	/* Fdinsert doesn't wait (especially with a queue looked up) */
	osr->osr_flags |= F_OSR_NONBLOCK;

#if SEC_BASE
	return osr_putmsg_subr(osr, &strfd->databuf, &strfd->ctlbuf,
			&strfd->attrbuf, strfd->flags, 0, q, strfd->offset);
#else
	return osr_putmsg_subr(osr, &strfd->databuf, &strfd->ctlbuf,
			strfd->flags, 0, q, strfd->offset);
#endif
}

/*
 * The I_FIFO ioctl should only be done from the mkfifo library call.
 * We assume here that we have a dedicated stream and that we
 * don't need to worry about anyone else using it. If there is,
 * they're going to be rudely surprised!
 */
int
osr_fifo (osr)
reg	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	queue_t	* q = sth->sth_wq->q_next;

	/*
	 * Verify we have an anonymous, unattatched stream
	 * head with no modules pushed.
	 */
	if (sth->sth_dev != NODEV || sth->sth_next || !STREAM_END(q))
		return EINVAL;

	/* Cross-connect the _driver_ */
	q->q_next = OTHERQ(q);
	q->q_flag |= QWELDED;
	sth->sth_wq->q_ffcp = sth->sth_rq;
	sth->sth_rq->q_bfcp = sth->sth_wq;

	/* Set appropriate stream head state */
	sth->sth_flags |= (F_STH_PIPE | F_STH_FIFO | F_STH_NDELON);
	sth->sth_write_mode = 0;
	sth->sth_read_mode |= RPROTCOMPRESS;
	sth->sth_rq->q_hiwat = PIPE_BUF;
	sth->sth_rq->q_lowat = PIPE_BUF - 1;
	sth_update_times(sth, FCREAT, (struct stat *)0);

	return 0;
}

int
osr_find (osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	char	* fmodsw_name;
	char	* name;
reg	queue_t	* q;

	if (!(name = osr->osr_ioctl_arg0p)[0])
		return EINVAL;
	
	for (q = sth->sth_wq->q_next; q; q = q->q_next) {
		if ((fmodsw_name = qinfo_to_name(RD(q)->q_qinfo))
		&&  strcmp(fmodsw_name, name) == 0) {
			osr->osr_ret_val = 1;
			return 0;
		}
	}

	if (fname_to_str((char *)name))
		return 0;

	return EINVAL;
}

int
osr_fionread (osr)
	OSRP	osr;
{
	queue_t *	q = osr->osr_sth->sth_rq;
reg	int		nread;
reg	mblk_t *	mp;

	nread = 0;
	for (mp = q->q_first; mp; mp = mp->b_next)
		nread += msgdsize(mp);
	osr->osr_ioctl_arg0_len = sizeof (int);
	*((int *)osr->osr_ioctl_arg0p) = nread;
	return 0;
}

/*
 * osr_flush - send a M_FLUSH message downstream.
 *
 * Special for M_PASSFP messages: Must flush them manually, since
 * the stream head service routine is not able to do that.
 * We do that AFTER sending the M_FLUSH message, since we prefer
 * to leave things unaltered if we have to return ENOSR.
 */

int
osr_flush (osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	int	flags = osr->osr_ioctl_arg1;

	switch (flags) {
	case FLUSHW:
		flushq(sth->sth_wq, FLUSHALL);
		break;
	case FLUSHR:
		flushq(sth->sth_rq, FLUSHALL | FLUSH_CAN_CLOSE);
		break;
	case FLUSHRW:
		flushq(sth->sth_rq, FLUSHALL | FLUSH_CAN_CLOSE);
		flushq(sth->sth_wq, FLUSHALL);
		break;
	default:
		return EINVAL;
	}
	if (putctl1(sth->sth_wq->q_next, M_FLUSH, flags))
		return 0;
	return ENOSR;
}

int
osr_flushband (osr)
	OSRP	osr;
{
	struct bandinfo	* bi;
	STH	* sth;

	bi = (struct bandinfo *)osr->osr_ioctl_arg0p;
	if ((unsigned)bi->bi_pri > MAX_QBAND)
		return EINVAL;
	sth = osr->osr_sth;
	switch (bi->bi_flag) {
	case FLUSHW:
		flushband(sth->sth_wq, bi->bi_pri, FLUSHALL);
		break;
	case FLUSHR:
		flushband(sth->sth_rq, bi->bi_pri, FLUSHALL | FLUSH_CAN_CLOSE);
		break;
	case FLUSHRW:
		flushband(sth->sth_rq, bi->bi_pri, FLUSHALL | FLUSH_CAN_CLOSE);
		flushband(sth->sth_wq, bi->bi_pri, FLUSHALL);
		break;
	default:
		return EINVAL;
	}
	if (putctl2(sth->sth_wq->q_next, M_FLUSH, bi->bi_flag | FLUSHBAND, bi->bi_pri))
		return 0;
	return ENOSR;
}

int
osr_getband (osr)
	OSRP	osr;
{
	MBLKP	mp;

	if (mp = osr->osr_sth->sth_rq->q_first) {
		*((int *)osr->osr_ioctl_arg0p) = mp->b_band;
		return 0;
	}
	return ENODATA;
}

int
osr_getcltime (osr)
	OSRP	osr;
{
	*((long *)osr->osr_ioctl_arg0p) = osr->osr_sth->sth_close_wait_timeout;
	return 0;
}

int
osr_getmsg (osr)
	OSRP	osr;
{
	int	error, band;
#if SEC_BASE
	struct strpeek_attr *strp = (struct strpeek_attr *)osr->osr_ioctl_arg0p;
#else
	struct strpeek *strp = (struct strpeek *)osr->osr_ioctl_arg0p;
#endif

	if (strp->flags != 0  &&  strp->flags != RS_HIPRI)
		return EINVAL;

#if SEC_BASE
	error = osr_getmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			&strp->attrbuf, &strp->flags, &band, 0);
#else
	error = osr_getmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			&strp->flags, &band, 0);
#endif
	if (strp->flags != RS_HIPRI)
		strp->flags = 0;
	return error;
}

int
osr_getpmsg (osr)
	OSRP	osr;
{
#if SEC_BASE
	struct strpmsg_attr *strp = (struct strpmsg_attr *)osr->osr_ioctl_arg0p;
#else
	struct strpmsg *strp = (struct strpmsg *)osr->osr_ioctl_arg0p;
#endif

	if ((strp->flags != MSG_HIPRI  &&  strp->flags != MSG_BAND
		&&  strp->flags != MSG_ANY)
	||  (strp->flags == MSG_HIPRI  &&  strp->band != 0))
		return EINVAL;

#if SEC_BASE
	return osr_getmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			&strp->attrbuf, &strp->flags, &strp->band, 0);
#else
	return osr_getmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			&strp->flags, &strp->band, 0);
#endif
}

int
osr_getsig (osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
reg	SIGSP	sigs;
	struct proc *p = (struct proc *)osr->osr_ioctl_arg2p;

	for (sigs  = (SIGSP)sth->sth_sigsq.next;
	     sigs != (SIGSP)&sth->sth_sigsq;
	     sigs  = (SIGSP)sigs->ss_link) {
		if (sigs->ss_procp == p) {
			*((int *)osr->osr_ioctl_arg0p) = sigs->ss_mask;
			return 0;
		}
	}
	return EINVAL;
}

int
osr_grdopt (osr)
	OSRP	osr;
{
	*((int *)osr->osr_ioctl_arg0p) = osr->osr_sth->sth_read_mode;
	return 0;
}

int
osr_gwropt (osr)
	OSRP	osr;
{
	*((int *)osr->osr_ioctl_arg0p) = osr->osr_sth->sth_write_mode;
	return 0;
}

int
osr_isastream (osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;

	if (!(sth->sth_flags & F_STH_PIPE))
		osr->osr_ret_val = I_ISASTREAM;
	else if (!(sth->sth_flags & F_STH_FIFO))
		osr->osr_ret_val = I_PIPE;
	else
		osr->osr_ret_val = I_FIFO;
	return 0;
}

/*
 * osr_link
 *
 *	Holds stream head and mult_sqh on entry.
 *
 * This routine handles both regular and persistent links.
 *	
 */
int
osr_link (osr)
	OSRP			osr;
{
	STHP			sth = osr->osr_sth;
	IOCP			iocp;
	int			is_persistent;
	STHP			lsth;
	MBLKP			mp;
	queue_t	* 		q;
	queue_t *		back_q;
	struct streamtab *	st;
	int			error;
	int                     muxid;

	if ( error = sth_fd_to_sth(osr->osr_ioctl_arg1, &lsth) )
		return error;		/* bogus file descriptor or file */

	/* Is this a persistent link or a regular one? */
	is_persistent = (osr->osr_ioctl_arg2 == I_PLINK);

	for (q = sth->sth_wq; !STREAM_END(q); q = q->q_next)
		;
	if (!q
	||  !(st = dqinfo_to_str(RD(q)->q_qinfo))
	||  !st->st_muxrinit
	||  !st->st_muxwinit)
		return EINVAL;		/* not a mux */

	/*
	 * Now we grab the module below the lower stream head to
	 * prevent anything from coming up while we are modifying
	 * it, and then, the lower stream head itself.
	 */
	if ( back_q = backq(lsth->sth_rq) )
		csq_acquire(&back_q->q_sqh, &osr->osr_sq);
	csq_acquire(&lsth->sth_rq->q_sqh, &osr->osr_sq);

	if ( lsth->sth_flags & RWHL_ERROR_FLAGS )
		error = EINVAL;
	else {
	        muxid = sth_muxid();
		mp = sth_link_alloc(osr, osr->osr_ioctl_arg2, muxid, q, lsth->sth_wq);
		if (mp == 0)
			error = EAGAIN;
	}
	if (error) {
		mult_sqh_release(osr);
		csq_release(&lsth->sth_rq->q_sqh);
		if ( back_q )
			csq_release(&back_q->q_sqh);
		return error;
	}

	lsth->sth_flags |= F_STH_LINKED;
	lsth->sth_read_error = EINVAL;
	lsth->sth_write_error = EINVAL;
	lsth->sth_muxid = muxid;
	lsth->sth_rq->q_ptr = lsth->sth_wq->q_ptr = 0;
	sth_set_queue(lsth->sth_rq, st->st_muxrinit, st->st_muxwinit);
	enableok(lsth->sth_wq);

	/*
	 * Acquire the lower stream head again before reparenting,
	 * since it is done separately for the read and write sides.
	 *
	 * After the link, the lower streamhead queues will be at
	 * the same synchronization level as the mux they are linked
	 * under. When the unlink happens, synchronization will be
	 * restored to QUEUEPAIR level, as for all stream heads.
	 */
	csq_acquire(&lsth->sth_wq->q_sqh, &osr->osr_sq);
	csq_newparent(osr, lsth->sth_wq, st);
	csq_newparent(osr, lsth->sth_rq, st);

	if ( !lsth->sth_rq->q_first )
		lsth->sth_rq->q_flag |= QWANTR;
	if (is_persistent) {
		lsth->sth_mux_link = sth->sth_pmux_top;
		sth->sth_pmux_top = lsth;
	} else {
		lsth->sth_mux_link = sth->sth_mux_top;
		sth->sth_mux_top = lsth;
	}
	osr->osr_ret_val = lsth->sth_muxid; /* save for later return */
	if (sth->sth_ioc_mp)
		freemsg(sth->sth_ioc_mp);
	sth->sth_ioc_id = ((struct iocblk *)mp->b_rptr)->ioc_id;
	sth->sth_ioc_mp = nil(MBLKP);

	puthere(q, mp);

	/* Lower module read remains locked until operation complete. */
	csq_release(&lsth->sth_wq->q_sqh);
	csq_release(&lsth->sth_rq->q_sqh);

	/* Release the bottom stream since the multiplexor might want to
	 * send messages to configure the bottom stream prior to replying
	 * to the I_LINK.  This type of operation is permitted (though
	 * only by the implementation, not explicitly by the specification)
	 * by SVR4 and by OSF/1 1.0 so we should maintain compatability.
	 */
	if ( back_q )
		csq_release(&back_q->q_sqh);

	error = osr_sleep(osr, FALSE, 0);	/* V.4 specifies no timeouts */

	sth->sth_ioc_id = 0;
	if ( !(mp = sth->sth_ioc_mp) ) {
		if (error == 0)
			error = ETIME;
	} else {
		sth->sth_ioc_mp = nil(MBLKP);
		iocp = (IOCP)mp->b_rptr;
		error = iocp->ioc_error;
		if (!error && mp->b_datap->db_type != M_IOCACK)
			error = EINVAL;
		/* sanity check */
		switch (error) {
		default:
			if (error >= 0 && error < 128)
				break;
		case ERESTART:
		case EJUSTRETURN:
STR_DEBUG(printf("osr_link_subr: would have returned %d\n", error));
			error = EINVAL;
		}
		freemsg(mp);
	}
	if ( error ) {
		if ( back_q )
			csq_acquire(&back_q->q_sqh, &osr->osr_sq);
		csq_acquire(&lsth->sth_rq->q_sqh, &osr->osr_sq);
		csq_acquire(&lsth->sth_wq->q_sqh, &osr->osr_sq);
		
		lsth->sth_flags &= ~F_STH_LINKED;
		lsth->sth_read_error = 0;
		lsth->sth_write_error = 0;
		lsth->sth_muxid = 0;
		sth_set_queue(	lsth->sth_rq,
				sthinfo.st_rdinit,
				sthinfo.st_wrinit);
		lsth->sth_rq->q_ptr = (caddr_t)lsth;
		lsth->sth_wq->q_ptr = (caddr_t)lsth;
		if (is_persistent)
			sth->sth_pmux_top = lsth->sth_mux_link;
		else
			sth->sth_mux_top = lsth->sth_mux_link;
		lsth->sth_mux_link = nil(STHP);

		/*
		 * reset queue parenting
		 */
		noenable(lsth->sth_wq);
		csq_newparent(osr, lsth->sth_wq, &sthinfo);
		csq_newparent(osr, lsth->sth_rq, &sthinfo);

		csq_release(&lsth->sth_wq->q_sqh);
		csq_release(&lsth->sth_rq->q_sqh);
		if ( back_q )
			csq_release(&back_q->q_sqh);
	}
	return error;
}

int
osr_list (osr)
	OSRP	osr;
{
	int		cnt;
	int		err;
	MBLKP		mp;
	queue_t		* q;
	struct str_list	* sl;

	sl = (struct str_list *)osr->osr_ioctl_arg0p;
	q = osr->osr_sth->sth_wq->q_next;
	for (cnt = 1; !STREAM_END(q); cnt++)
		q = q->q_next;
	if (!sl) {
		osr->osr_ret_val = cnt;
		return 0;
	}
	if (sl->sl_nmods < 1)
		return EINVAL;
	sl->sl_nmods = MIN(sl->sl_nmods, cnt);
	cnt = sl->sl_nmods * sizeof (struct str_mlist);
	if ((mp = allocb(cnt, BPRI_MED)) == 0)
		return EAGAIN;
	bzero((caddr_t)mp->b_rptr, cnt);
	osr->osr_ioctl_arg1p = (caddr_t)sl->sl_modlist;
	osr->osr_ioctl_arg1_len = cnt;
	osr->osr_ioctl_data = mp;
	q = osr->osr_sth->sth_wq->q_next;
	cnt = 0;
	for (;;) {
		bcopy(q->q_qinfo->qi_minfo->mi_idname,
		      ((struct str_mlist *)mp->b_wptr)->l_name, FMNAMESZ + 1);
		mp->b_wptr += sizeof (struct str_mlist);
		if (++cnt >= sl->sl_nmods)
			break;
		if (STREAM_END(q))
			break;
		q = q->q_next;
	}
	osr->osr_ioctl_arg0_len = sizeof(struct str_list);
	osr->osr_ret_val = 0;
	return 0;
}

int
osr_look (osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	char	* name;
	queue_t	* q;

	if ( !(q = sth->sth_wq->q_next) || STREAM_END(q) )
		return EINVAL;
	else if ( (name = qinfo_to_name(RD(q)->q_qinfo)) == NULL )
		return ENXIO;	/* shouldn't happen */
	osr->osr_ioctl_arg0_len = strlen(name) + 1;
	bcopy(name, osr->osr_ioctl_arg0p, osr->osr_ioctl_arg0_len);
	return 0;
}

int
osr_nread (osr)
	OSRP	osr;
{
reg	queue_t	* q = osr->osr_sth->sth_rq;

	*((int *)osr->osr_ioctl_arg0p) = (q->q_first == nil(MBLKP))
					? 0 : msgdsize(q->q_first);
	osr->osr_ret_val = qsize(q);
	return 0;
}

int
osr_peek (osr)
	OSRP	osr;
{
	int	error;
	int	band = 0;
#if SEC_BASE
	struct strpeek_attr *strp = (struct strpeek_attr *)osr->osr_ioctl_arg0p;
#else
	struct strpeek *strp = (struct strpeek *)osr->osr_ioctl_arg0p;
#endif

	if (strp->flags != RS_HIPRI  &&  strp->flags != 0)
		return EINVAL;

	/* We want EAGAIN when no data. */
	osr->osr_flags |= F_OSR_NONBLOCK;

#if SEC_BASE
	error = osr_getmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			&strp->attrbuf, &strp->flags, &band, 1);
#else
	error = osr_getmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			&strp->flags, &band, 1);
#endif
	if (error == 0) {
		if (strp->databuf.len > 0 || strp->ctlbuf.len > 0) {
			if (strp->flags != RS_HIPRI)
				strp->flags = 0;
			osr->osr_ret_val = 1;
		} else
			osr->osr_ret_val = 0;
	} else if (error == EAGAIN) {
		osr->osr_ret_val = 0;
		error = 0;
	}
	return error;
}

/*
 * The I_PIPE ioctl should only be done from the pipe library call.
 * We assume here that we have two dedicated streams and that we
 * don't need to worry about anyone else using them. If there are,
 * they're going to be rudely surprised!
 */
int
osr_pipe (osr)
reg	OSRP	osr;
{
	int	error;
	queue_t	* q1;
	queue_t	* q2;
	STHP	sth1 = osr->osr_sth;
	STHP	sth2;

	/* Get the STH for the second stream */
	if ( (error = sth_fd_to_sth(osr->osr_ioctl_arg1, &sth2)) != 0 )
		return error;
	csq_acquire(&sth2->sth_rq->q_sqh, &osr->osr_sq);

	/*
	 * Verify that we have two different, anonymous, errorless,
	 * unpushed, unconnected, unattached stream heads.
	 */
	if (sth1 == sth2
	||  sth1->sth_dev != NODEV
	||  sth2->sth_dev != NODEV
	||  sth1->sth_next
	||  sth2->sth_next
	||  (sth2->sth_flags & RWHL_ERROR_FLAGS)
	||  !STREAM_END(sth1->sth_wq->q_next)
	||  !STREAM_END(sth2->sth_wq->q_next)) {
		csq_release(&sth2->sth_rq->q_sqh);
		return EINVAL;
	}

	/* No need to acquire these special driver queues. */
	q1 = sth1->sth_wq->q_next;
	q2 = sth2->sth_wq->q_next;
	
	/* Cross-connect the stream heads */
	sth1->sth_wq->q_next = sth2->sth_rq;
	sth1->sth_wq->q_flag |= QWELDED;
	sth1->sth_wq->q_ffcp = sth2->sth_rq;
	sth2->sth_rq->q_bfcp = sth1->sth_wq;

	sth2->sth_wq->q_next = sth1->sth_rq;
	sth2->sth_wq->q_flag |= QWELDED;
	sth2->sth_wq->q_ffcp = sth1->sth_rq;
	sth1->sth_rq->q_bfcp = sth2->sth_wq;

	/* Set appropriate stream head state */
	sth1->sth_flags |= (F_STH_PIPE | F_STH_NDELON);
	sth1->sth_write_mode = 0;
	sth1->sth_read_mode |= RPROTCOMPRESS;
	sth1->sth_rq->q_hiwat = PIPE_BUF;
	sth1->sth_rq->q_lowat = PIPE_BUF - 1;
	sth2->sth_flags |= (F_STH_PIPE | F_STH_WPIPE | F_STH_NDELON);
	sth2->sth_write_mode = 0;
	sth2->sth_read_mode |= RPROTCOMPRESS;
	sth2->sth_rq->q_hiwat = PIPE_BUF;
	sth2->sth_rq->q_lowat = PIPE_BUF - 1;
	sth_update_times(sth2, FCREAT, (struct stat *)0);

	mult_sqh_release(osr);
	csq_release(&sth2->sth_rq->q_sqh);
	csq_release(&sth1->sth_rq->q_sqh);

	/* Now we can ditch the drivers. */
	if (!(RD(q1)->q_flag & QOLD))
		(void)(*(strclose_V4)(RD(q1)->q_qinfo->qi_qclose))
			(RD(q1), 0, osr->osr_creds);
	else
		(void)(*(strclose_V3)(RD(q1)->q_qinfo->qi_qclose))(RD(q1));
	(void) modsw_ref(RD(q1)->q_qinfo, -1);
	q_free(RD(q1));

	if (!(RD(q2)->q_flag & QOLD))
		(void)(*(strclose_V4)(RD(q2)->q_qinfo->qi_qclose))
			(RD(q2), 0, osr->osr_creds);
	else
		(void)(*(strclose_V3)(RD(q2)->q_qinfo->qi_qclose))(RD(q2));
	(void) modsw_ref(RD(q2)->q_qinfo, -1);
	q_free(RD(q2));
	
	return 0;
}

/*
 * Stat on a pipe or fifo needs to return all data in the pipe.
 *
 * A bidirectional pipe must always yield the same value from each
 * side, so we use the F_STH_WPIPE bit and start from there.
 */
int
osr_pipestat (osr)
reg	OSRP	osr;
{
	STHP	sth1 = osr->osr_sth;
	STHP	sth2;
	MBLKP	mp;
	int	nwrite = 0, nread = 0;
	queue_t	* q;
	struct stat *sb;

	if ( !(sth1->sth_flags & F_STH_PIPE) )
		return EINVAL;
	/* Walk forward to the possible other stream head. */
	for (q = sth1->sth_wq; q->q_next; q = q->q_next)
		;
	if (q->q_qinfo->qi_putp == sth_rput) {
		sth2 = (STHP)q->q_ptr;
		csq_acquire(&sth2->sth_rq->q_sqh, &osr->osr_sq);
	} else
		sth2 = NULL;
	mult_sqh_release(osr);

	if (sth1 == sth2 || (sth1->sth_flags & F_STH_WPIPE)) {
		/* Count up this side's read q */
		if (q->q_flag & QREADR)
			for (mp = q->q_first; mp; mp = mp->b_next)
				nread += msgdsize(mp);
		/* And other side's write q */
		for (mp = sth1->sth_wq->q_first; mp; mp = mp->b_next)
			nwrite += msgdsize(mp);
	} else {
		/* Count up this side's write q */
		if (q->q_flag & QREADR)
			for (mp = OTHERQ(q)->q_first; mp; mp = mp->b_next)
				nwrite += msgdsize(mp);
		/* And other side's read q */
		for (mp = sth1->sth_rq->q_first; mp; mp = mp->b_next)
			nread += msgdsize(mp);
	}
	if (sth2)
		csq_release(&sth2->sth_rq->q_sqh);
	sb = (struct stat *)osr->osr_ioctl_arg0p;
	osr->osr_ioctl_arg0_len = sizeof *sb;
	bzero(sb, sizeof *sb);
	sb->st_blksize = nread + nwrite;
	sb->st_size = nread;
	sb->st_dev = NODEV;
	if (sth1 == sth2)
		sb->st_mode = S_IFIFO;
	sth_update_times(sth1, 0, sb);
	return 0;
}

/*
 * osr_pop - handler for I_POP requests.
 *
 * Acts as a front-end to the internal pop subroutine which
 * is also used when dismantling a stream at close time.
 */

int
osr_pop (osr)
reg	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
reg	queue_t	* q = sth->sth_wq->q_next;

	if ( !q
	||   (!(sth->sth_flags & F_STH_PIPE) && STREAM_END(q))
	||   (sth->sth_wq->q_flag & QWELDED)
	||   (RD(q)->q_flag & QWELDED) )
		return EINVAL;
	(void) osr_pop_subr(osr, q);
	return 0;
}

/*
 * osr_pop_subr - internal pop subroutine
 *
 * This is a common subroutine used during I_POP requests
 * and when dismantling a stream at close time. It is
 * called with mult_sqh and the sth locks held, and
 * returns with both released.
 */
int
osr_pop_subr (osr, wq)
	OSRP		osr;
	queue_t	*	wq;		/* module's write queue */
{
	queue_t *	rq = RD(wq);	/* module's read queue */
	queue_t *	bq = backq(rq);	/* lower module's read queue */
	queue_t *	hrq;		/* stream head's read queue */
	queue_t	*	hwq;		/* stream head's write queue */
	queue_t *	qq;		/* for scanning q chains */
	int		error;

	/*
	 * Lock the module to be popped.
	 * If there is a module below that one, we have to lock
	 * that too, in order to have it after the qi_qclose
	 * returns (preventing messages entering a closed queue).
	 */
	if ( bq  &&  wq->q_next != rq  &&  wq->q_next != rq->q_next )
		csq_acquire(&bq->q_sqh, &osr->osr_sq);
	else
		bq = nilp(queue_t);

	/*
	 * Call the module's close procedure (if any).
	 */
	if (rq->q_qinfo->qi_qclose) {
		struct open_args close_args;

		close_args.a_func = rq->q_qinfo->qi_qclose;
		close_args.a_queue = rq;
		close_args.a_fflag = osr->osr_ioctl_arg1;
		close_args.a_creds = osr->osr_creds;
		error = csq_protect(rq, wq, (csq_protect_fcn_t)close_wrapper,
			(csq_protect_arg_t)&close_args, &osr->osr_sq, FALSE);
	} else {
		csq_acquire(&wq->q_sqh, &osr->osr_sq);
		csq_acquire(&rq->q_sqh, &osr->osr_sq);
		error = 0;
	}
	osr->osr_sth->sth_push_cnt--;

	/*
	 * We have the stream head, the lower module, and
	 * the module itself (from csq_protect(FALSE)).
	 *
	 * Now remove the module from the stream.
	 */
	if ( hrq = rq->q_next ) {
		hwq = WR(hrq);

		if (wq->q_flag & QWELDED ) {
			/*
			 * Pipe!  Pull the weld indication back into sth_wq.
			 */
			hwq->q_flag |= QWELDED;
		}

		/*
		 * Time to fix up any flow control pointers to make sure no
		 * one is referencing the queue we are popping.
		 */
		if ( hrq->q_bfcp == rq && (qq = hrq->q_bfcp = rq->q_bfcp) ) {
			do {
				csq_acquire(&qq->q_sqh, &osr->osr_sq);
				qq->q_ffcp = rq->q_ffcp;
				csq_release(&qq->q_sqh);
			} while ( (qq = qq->q_next) != rq );
		}

		if ( hwq->q_ffcp == wq && (hwq->q_ffcp = wq->q_ffcp) ) {
			qq = wq;
			do {
				qq = qq->q_next;
				csq_acquire(&qq->q_sqh, &osr->osr_sq);
				qq->q_bfcp = wq->q_bfcp;
				csq_release(&qq->q_sqh);
			} while ( qq != wq->q_ffcp );
		}
		/*
		 * Check for simple stream loops, don't propagate them.
		 * (We won't find anything funny here if custom plumbing
		 * has been done using weldq: osr_close will call unweldq
		 * before popping a welded queue, and osr_pop will fail
		 * an attempt to pop a welded queue.)
		 */
		if ( bq == nilp(queue_t) ) {
			hwq->q_next = nilp(queue_t);
			hwq->q_ffcp = nilp(queue_t);	/* ??? */
			hrq->q_bfcp = nilp(queue_t);
		} else {
			hwq->q_next = wq->q_next;
			bq->q_next = hrq;
		}
		wq->q_next = rq->q_next = nilp(queue_t);	/* paranoid */
	}
	/*
	 * The target queue is now out of the stream.
	 * Time to release any synch queues we grabbed.
	 * If "popping" a stream head, release locks taken
	 * in caller to avoid deadlocks.
	 */
	if ( rq == osr->osr_sth->sth_rq ) {
		mult_sqh_release(osr);
		csq_release(&osr->osr_sth->sth_rq->q_sqh);
	}
	if ( bq )
		csq_release(&bq->q_sqh);

	/*
	 * Make sure that the queues aren't on the run queue,
	 * then blow off anything else that may have sneaked in.
	 * (Any queued messages remaining will be freed in q_free().)
	 */
	runq_remove(rq);
	runq_remove(wq);

	csq_cleanup(&wq->q_sqh);
	csq_release(&wq->q_sqh);
	csq_cleanup(&rq->q_sqh);
	csq_release(&rq->q_sqh);

	/*
	 * Drop reference on this module, then free the queues.
	 */
	if (hrq)
		(void) modsw_ref(rq->q_qinfo, -1);
	q_free(rq);
	return error;
}

int
osr_push (osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	int		error = 0;
	queue_t	*	q;
	queue_t	*	bq;
	queue_t *	back_q;
	queue_t *	front_q;
	struct streamtab * str;
	struct open_args open_args;

	if ( !osr->osr_ioctl_arg0p[0]
	||   !(str = fname_to_str(osr->osr_ioctl_arg0p)) )
		return EINVAL;
	if ( !(back_q = backq(sth->sth_rq)) )
		return ENOSTR;
	if ( !str->st_rdinit || !str->st_rdinit->qi_qopen )
		return ENXIO;
	if ( sth->sth_push_cnt >= nstrpush )
		return EINVAL;
	if ( !modsw_ref(str->st_rdinit, 1) )
		return EINVAL;
	if ( !(q = q_alloc()) ) {
		(void) modsw_ref(str->st_rdinit, -1);
		return EAGAIN;
	}

	(void) sqh_set_parent(q, str);
	(void) sqh_set_parent(WR(q), str);
	sth_set_queue(q, str->st_rdinit, str->st_wrinit);

	/*
	 * The lower module must be locked first.
	 */
	front_q = sth->sth_wq->q_next;
	csq_acquire(&back_q->q_sqh, &osr->osr_sq);
	csq_acquire(&front_q->q_sqh, &osr->osr_sq);

	/*
	 * Set the queue and flow control pointers, along with any
	 * welds, to bring the new queue into the stream.
	 */
	back_q->q_next = q;
	q->q_next = sth->sth_rq;
	WR(q)->q_next = front_q;
	sth->sth_wq->q_next = WR(q);
	if ( sth->sth_wq->q_flag & QWELDED ) {
		WR(q)->q_flag |= QWELDED;
		sth->sth_wq->q_flag &= ~QWELDED;
	}

	q->q_ffcp = sth->sth_rq;
	q->q_bfcp = sth->sth_rq->q_bfcp;
	if ( q->q_qinfo->qi_srvp ) {
		sth->sth_rq->q_bfcp = q;
		bq = q->q_bfcp;
		do {
			csq_acquire(&bq->q_sqh, &osr->osr_sq);
			bq->q_ffcp = q;
			csq_release(&bq->q_sqh);
		} while ( (bq = bq->q_next) != q );
	}
	WR(q)->q_ffcp = sth->sth_wq->q_ffcp;
	WR(q)->q_bfcp = sth->sth_wq;
	if ( WR(q)->q_qinfo->qi_srvp ) {
		sth->sth_wq->q_ffcp = WR(q);
		bq = WR(q);
		do {
			bq = bq->q_next;
			csq_acquire(&bq->q_sqh, &osr->osr_sq);
			bq->q_bfcp = WR(q);
			csq_release(&bq->q_sqh);
		} while ( bq != WR(q)->q_ffcp );
	}

	open_args.a_func  = q->q_qinfo->qi_qopen;
	open_args.a_queue = q;
	open_args.a_devp  = 0;
	open_args.a_fflag = osr->osr_ioctl_arg1;
	open_args.a_sflag = MODOPEN;
	open_args.a_creds = osr->osr_creds;
	error = csq_protect(q, WR(q), (csq_protect_fcn_t)open_wrapper,
			(csq_protect_arg_t)&open_args, &osr->osr_sq, FALSE);

	/* Success! */
	if (error == 0) {
		sth->sth_push_cnt++;
		mult_sqh_release(osr);
		csq_release(&q->q_sqh);
		csq_release(&WR(q)->q_sqh);
		csq_release(&back_q->q_sqh);
		csq_release(&front_q->q_sqh);
		csq_release(&sth->sth_rq->q_sqh);
		return 0;
	}

	/* Get the failed module out of the stream. */
	if ( q->q_qinfo->qi_srvp ) {
		sth->sth_rq->q_bfcp = q->q_bfcp;
		bq = q->q_bfcp;
		do {
			csq_acquire(&bq->q_sqh, &osr->osr_sq);
			bq->q_ffcp = sth->sth_rq;
			csq_release(&bq->q_sqh);
		} while ( (bq = bq->q_next) != q );
	}
	if ( WR(q)->q_qinfo->qi_srvp ) {
		sth->sth_wq->q_ffcp = WR(q)->q_ffcp;
		bq = WR(q);
		do {
			bq = bq->q_next;
			csq_acquire(&bq->q_sqh, &osr->osr_sq);
			bq->q_bfcp = sth->sth_wq;
			csq_release(&bq->q_sqh);
		} while ( bq != WR(q)->q_ffcp );
	}
	if ( WR(q)->q_flag & QWELDED )
		sth->sth_wq->q_flag |= QWELDED;
	sth->sth_wq->q_next = front_q;
	back_q->q_next = sth->sth_rq;
	runq_remove(q);
	runq_remove(WR(q));
	csq_cleanup(&WR(q)->q_sqh);
	csq_release(&WR(q)->q_sqh);
	csq_cleanup(&q->q_sqh);
	csq_release(&q->q_sqh);
	q_free(q);
	(void) modsw_ref(str->st_rdinit, -1);
	mult_sqh_release(osr);
	csq_release(&back_q->q_sqh);
	csq_release(&front_q->q_sqh);
	csq_release(&sth->sth_rq->q_sqh);
	return error;
}

int
osr_putmsg (osr)
	OSRP	osr;
{
#if SEC_BASE
	struct strpeek_attr *strp = (struct strpeek_attr *)osr->osr_ioctl_arg0p;
#else
	struct strpeek *strp = (struct strpeek *)osr->osr_ioctl_arg0p;
#endif

	if ( (strp->flags != 0  &&  strp->flags != RS_HIPRI)
	||   (strp->ctlbuf.len < 0  &&  strp->flags == RS_HIPRI) )
		return EINVAL;

#if SEC_BASE
	return osr_putmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			&strp->attrbuf, strp->flags, 0, (queue_t *)0, 0);
#else
	return osr_putmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
			strp->flags, 0, (queue_t *)0, 0);
#endif
}

int
osr_putpmsg (osr)
	OSRP	osr;
{
#if SEC_BASE
	struct strpmsg_attr *strp = (struct strpmsg_attr *)osr->osr_ioctl_arg0p;
#else
	struct strpmsg *strp = (struct strpmsg *)osr->osr_ioctl_arg0p;
#endif

	if ((strp->flags != MSG_HIPRI  &&  strp->flags != MSG_BAND)
	|| (strp->flags == MSG_HIPRI  &&  (strp->ctlbuf.len < 0 || strp->band))
	|| (strp->flags == MSG_BAND  &&  (unsigned)strp->band > MAX_QBAND) )
		return EINVAL;

#if SEC_BASE
	return osr_putmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
		&strp->attrbuf, strp->flags, strp->band, (queue_t *)0, 0);
#else
	return osr_putmsg_subr(osr, &strp->databuf, &strp->ctlbuf,
		strp->flags, strp->band, (queue_t *)0, 0);
#endif
}

int
osr_read (osr)
reg	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	queue_t	*	q = sth->sth_rq;
reg	int		cnt;
reg	MBLKP		mp;
	MBLKP		mp1;
	int		to_get;
	int		error;
	int		read_mode;
	long		notifycnt = 0;
	int		mread_sent = 0;
	int		tryhard = hangup_tries;

	if (sth->sth_flags & F_STH_PIPE)
		sth_update_times(sth, FREAD, (struct stat *)0);
	if (osr->osr_rw_count == 0)
		return 0;

	for (;;) {
		read_mode = sth->sth_read_mode;
		while (!(mp = sth_getq(sth))) {
			if(sth->sth_flags & F_STH_FIFO)
			{
				simple_lock(&sth->sth_ext_flags_lock);
				cnt = sth->sth_wrcnt;
				simple_unlock(&sth->sth_ext_flags_lock);
				if(cnt == 0) {
					error = 0;
					goto out;
				}
			}
			if (sth->sth_flags & F_STH_HANGUP) {
				if (tryhard-- > 0 && !osr->osr_rw_total) {
					osr->osr_flags |= F_OSR_NONBLOCK;
					(void) osr_sleep(osr, FALSE, 0);
					continue;
				}
				error = 0;
				goto out;
			}
			if (osr->osr_rw_total && !(read_mode & RFILL))
				goto out;
			if ( sth->sth_flags & F_STH_MREADON && !mread_sent ) {
				if ( !(mp = allocb(sizeof(long), BPRI_HI)) ) {
					if (osr_bufcall(osr, TRUE, 0,
							sizeof(long), BPRI_HI))
					{
						goto out; /* isn't EAGAIN */
					}
					continue;
				}
				*(long *)mp->b_rptr = osr->osr_rw_count;
				mp->b_wptr += sizeof (long);
				mp->b_datap->db_type = M_READ;
				++mread_sent;
				putnext(sth->sth_wq, mp);
			}
			if (error = osr_sleep(osr, TRUE, 0)) {
				if (error != EAGAIN)
					goto out;
				if (mp && (mp = sth_getq(sth)))	/* mread sent */
					break;
				if (osr->osr_rw_total
				||  ((sth->sth_flags & F_STH_NDELON) &&
				     (osr->osr_flags & F_OSR_NBIO) ==
						F_OSR_NDELAY))
					error = 0;
				goto out;
			}
		}
		error = 0;

		switch (mp->b_datap->db_type) {
		case M_DATA:
		case M_HPDATA:
			break;
		case M_MI: {
			ulong * up = (ulong *)mp->b_rptr;
			if ( up[0] & M_MI_READ_RESET )
				sth_read_reset(osr);
			if ( up[0] & M_MI_READ_SEEK )
				error = sth_read_seek(osr, (int)up[1], (long)up[2]);
			if ( (up[0] & M_MI_READ_END)  &&  osr->osr_rw_total ) {
				freemsg(mp);
				goto out;
			}
			freemsg(mp);
			if ( error )
				goto out;
			}
			continue;
		case M_PROTO:
		case M_PCPROTO:
			if (read_mode & RPROTDIS) {
				/* Trash all M_PROTO and M_PCPROTO message blocks */
				do {
					if (mp->b_flag & MSGNOTIFY)
						notifycnt += mp->b_wptr - mp->b_rptr;
					mp1 = mp;
					mp = mp->b_cont;
					freeb(mp1);
					if (!mp)
						break;
				} while (mp->b_datap->db_type == M_PROTO
					|| mp->b_datap->db_type == M_PCPROTO);
				if (!mp)
					continue;
				break;
			}
			if (read_mode & RPROTDAT) {
				/* Convert all M_PROTO and M_PCPROTO messages to M_DATA */
				mp1 = mp;
				do {
					if (mp->b_datap->db_type == M_PROTO
					||  mp->b_datap->db_type == M_PCPROTO)
						mp->b_datap->db_type = M_DATA;
				} while (mp = mp->b_cont);
				mp = mp1;
				break;
			}
			/* FALL THROUGH */
		default:
			error = EBADMSG;
			goto out2;
		}
		/* If we already have data from a previous message,
		 * and a zero-length message comes up, complete the current
		 * read and handle the zero-length message next time.
		 * This can only happen in non-RMSG* modes.
		 */
		if (osr->osr_rw_total && msgdsize(mp) == 0)
			goto out2;

		csq_release(&q->q_sqh);
		to_get = osr->osr_rw_count;
		do {
			cnt = MIN(mp->b_wptr - mp->b_rptr, osr->osr_rw_count);
			if (cnt > 0) {
				error = sth_uiomove((caddr_t)mp->b_rptr, cnt, osr);
				if (error)
					goto out3;
				mp->b_rptr += cnt;
				if (mp->b_flag & MSGNOTIFY)
					notifycnt += cnt;
			}
			if (mp->b_rptr >= mp->b_wptr) {
				mp1 = mp;
				mp = mp->b_cont;
#if SEC_BASE
				/* Move the attributes up to the new head. */
				if (mp) {
					ASSERT(mp->b_attr == NULL);
					mp->b_attr = mp1->b_attr;
					mp1->b_attr = NULL;
				}
#endif
				freeb(mp1);
			}
		} while (osr->osr_rw_count  &&  mp);

		if (mp) {
			/* More stuff left in this message.
			 * In message discard mode, ditch the remainder of
			 * the message; otherwise put it back on the queue
			 * for the next read.
			 */
			if (read_mode & RMSGD) do {
				if (mp->b_flag & MSGNOTIFY)
					notifycnt += mp->b_wptr - mp->b_rptr;
				mp1 = mp;
				mp = mp->b_cont;
				freeb(mp1);
			} while (mp);
			goto out3;
		}
		/*
		 * If we got everything or nothing, complete the read now.
		 * Also complete reads for RMSG* modes.
		 */
		if ( osr->osr_rw_count == 0
		||   osr->osr_rw_count == to_get
		||   (read_mode & (RMSGD|RMSGN)) )
			goto out;
		csq_acquire(&q->q_sqh, &osr->osr_sq);
	}
out3:
	csq_acquire(&q->q_sqh, &osr->osr_sq);
out2:
	if (mp && putbq(q, mp) == 0)
		freemsg(mp);
out:
	if (notifycnt) {
		while ((mp = allocb(sizeof(long), BPRI_HI)) == NULL)
			(void)osr_bufcall(osr, FALSE, 0, sizeof(long), BPRI_HI);
		*(long *)mp->b_rptr = notifycnt;
		mp->b_wptr += sizeof (long);
		mp->b_datap->db_type = M_NOTIFY;
		putnext(sth->sth_wq, mp);
	}
	return error;
}

/*
 * osr_recvfd - expect a M_PASSFP message on the stream and handle it.
 *
 * The normal path through this routine is quite easy.
 * We block waiting for a PASSFP message (depending on ONDELAY).
 * If another kind of message shows up, return EBADMSG, else
 * we use the file system interface routine fd_alloc to map
 * the passed file into this process's descriptor space.
 * If that should fail (typically ENFILE), we have to drop the
 * reference passed with the file cookie via cookie_destroy().
 * (Question: should we could put it back instead?)
 *
 * The tricky thing is that dropping the reference could lead
 * to an immediate callback via the OS layer: if our M_PASSFP
 * message happened to be the last reference to the passed stream,
 * then a close operation will be initiated. Theoretically this
 * could be our own stream, but osr_sendfd guarantees this won't be.
 */

int
osr_recvfd (osr)
	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	MBLKP		mp;
	struct strpfp *	pfp;
	int		error;
#if SEC_BASE
	struct strrecvfd_attr *strrfd =
		(struct strrecvfd_attr *)osr->osr_ioctl_arg0p;
#else
	struct strrecvfd * strrfd = (struct strrecvfd *)osr->osr_ioctl_arg0p;
#endif

	while ( !(mp = sth->sth_rq->q_first) )
		if ( error = osr_sleep(osr, TRUE, 0) )
			return error;

	if (mp->b_datap->db_type != M_PASSFP)
		return EBADMSG;
	mp = sth_getq(sth);
	csq_release(&sth->sth_rq->q_sqh);

	/*
	 * Check writable before allocating fd slot.
	 */
	if (!useracc(osr->osr_ioctl_arg1p, sizeof *strrfd, B_WRITE)) {
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
		(void) putbq(sth->sth_rq, mp);
		return EFAULT;
	}
#if SEC_BASE
	strrfd->attrbuf.len = -1;
	if (mp->b_attr) {
		if (error = str_copyout_attrs(mp->b_attr, &strrfd->attrbuf)) {
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
			(void) putbq(sth->sth_rq, mp);
			return error;
		}
	}
#endif
	pfp = (struct strpfp *)mp->b_rptr;
	strrfd->uid = pfp->pass_uid;
	strrfd->gid = pfp->pass_gid;
	if ( error = fd_alloc(&pfp->pass_file_cookie, &strrfd->fd) )
		cookie_destroy(&pfp->pass_file_cookie);
	freemsg(mp);

	return error;
}


/*
 * osr_sendfd - construct a M_PASSFP message containing a reference
 * to an open file, and drop it at the other end of a stream pipe.
 *
 * The file descriptor to be passed is contained in osr_ioctl_arg1,
 * and there is a interface routine "fd_to_cookie" which converts
 * this into a file reference which can be used to construct the
 * access path at the other end.
 *
 * We rely on the reference count in the file layer - it is incremented
 * when we call fd_to_cookie(). This is necessary to prevent the
 * file from being closed while we hold this message.
 *
 * We hold the mult_sqh in order to prevent the streams plumbing from
 * changing while we are walking through the stream pipe.
 */
int
osr_sendfd (osr)
	OSRP	osr;
{
	STHP			sth = osr->osr_sth;
	int			fd = osr->osr_ioctl_arg1;
	MBLKP			mp;
	STHP			passed_sth;	/* If a stream */
	struct strpfp *		pfp;
	queue_t	* 		q;
	int			error = 0;
#if SEC_BASE
	struct strsendfd_attr	* strsnd =
			(struct strsendfd_attr *)osr->osr_ioctl_arg0p;
#endif

	if (osr->osr_creds == NULL)
		return EACCES;
	if ( (mp = allocb(sizeof(struct strpfp), BPRI_LO)) == NULL )
		return EAGAIN;

	mp->b_datap->db_type = M_PASSFP;
	pfp = (struct strpfp *)mp->b_rptr;
	pfp->pass_uid = osr->osr_creds->cr_uid;
	pfp->pass_gid = osr->osr_creds->cr_gid;
	mp->b_wptr += sizeof(struct strpfp);
	if (error = fd_to_cookie(fd, &pfp->pass_file_cookie)) {
		freemsg(mp);
		return error;
	}

#if SEC_BASE
	ASSERT(mp->b_attr == NULL);
	mult_sqh_release(osr);
	csq_release(&sth->sth_rq->q_sqh);
	mp->b_attr = str_copyin_attrs(&strsnd->attrbuf, &error);
	if (error) {
		freemsg(mp);
		return error;
	}
	mult_sqh_acquire(osr);
	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
#endif

	if ( sth_fd_to_sth(fd, &passed_sth) )
		passed_sth = 0;

	q = sth->sth_wq;
	while (q->q_next)
		q = q->q_next;
	/*
	 * Now there are some more reasons which could prevent us from
	 * sending our readily available message:
	 *	- it is not a streams pipe, i.e. we didn't arrive at
	 *	   a stream head
	 *	- we are trying to pass a stream to itself, which is illegal
	 *	   (The unprotected accesss to the passed stream's sth_rq is
	 *	   OK, since that will only change during close, and that won't
	 *	   happen as long as we hold the reference to the file in our
	 *	   cookie.)
	 *	- flow control blocks the receiving queue.
	 *
	 * Should we verify the receiving sth error flags?
	 */
	if ( q->q_qinfo->qi_putp != sth_rput
	||   (passed_sth && passed_sth->sth_rq == q) )
		error = EINVAL;
	else if ( !canput(q) )
		error = EAGAIN;
	else
		puthere(q, mp);
	mult_sqh_release(osr);
	csq_release(&sth->sth_rq->q_sqh);

	if ( error ) {
		cookie_destroy(&pfp->pass_file_cookie);
		freemsg(mp);
	}
	return error;
}

int
osr_setcltime (osr)
	OSRP	osr;
{
	long	wait_time;

	wait_time = *((long *)osr->osr_ioctl_arg0p);
	if (wait_time < 0)
		return EINVAL;
	if (wait_time < MIN_CLOSE_WAIT_TIMEOUT)
		wait_time = MIN_CLOSE_WAIT_TIMEOUT;
	osr->osr_sth->sth_close_wait_timeout = wait_time;
	return 0;
}

int
osr_setsig (osr)
	OSRP	osr;
{
        STHP    sth = osr->osr_sth;
reg     SIGSP   sigs;
        int     arg = osr->osr_ioctl_arg1;
        struct proc *p = (struct proc *)osr->osr_ioctl_arg2p;

        if (arg & ~(S_INPUT | S_RDNORM | S_RDBAND | S_HIPRI
                        | S_OUTPUT | S_WRNORM | S_WRBAND | S_MSG
                        | S_ERROR | S_HANGUP | S_BANDURG))
                return EINVAL;

        for (sigs  = (SIGSP)sth->sth_sigsq.next;
             sigs != (SIGSP)&sth->sth_sigsq;
             sigs  = (SIGSP)sigs->ss_link)
                if (sigs->ss_procp == p)
                        break;

        if (arg == 0) {
                /*
                 * FIOASYNC sets the arg0p to non-0.
                 */
                if (*(int *)osr->osr_ioctl_arg0p) {
                        if (sigs == (SIGSP)&sth->sth_sigsq)
                                return 0;
                        sigs->ss_async = 0;
                } else {
                        if (sigs == (SIGSP)&sth->sth_sigsq
                        ||  sigs->ss_mask == 0)
                                return EINVAL;
                        sigs->ss_mask = 0;
                }
                if (sigs->ss_mask == 0 && sigs->ss_async == 0) {
                        remque(&sigs->ss_link);
                        remque(&sigs->ss_proclink);
                        STR_FREE(sigs, M_STRSIGS);
                }
                return 0;
        }

        /*
         * Allocate a new SIGS structure for this process if necessary.
         * If there is already a SIGS for this process, all we have
         * to do is assign in the new mask.
         */
        if (sigs == (SIGSP)&sth->sth_sigsq) {
                STR_MALLOC(sigs, SIGSP, sizeof *sigs, M_STRSIGS, M_NOWAIT);
                if (!sigs)
                        return EAGAIN;
                sigs->ss_procp = p;
                sigs->ss_sth = sth;
                sigs->ss_async = sigs->ss_mask = 0;
                insque(&sigs->ss_link, &((SIGSP)sth->sth_sigsq.prev)->ss_link);
                insque(&sigs->ss_proclink, p->p_strsigs.prev);
        }
        if (*(int *)osr->osr_ioctl_arg0p)
                sigs->ss_async = arg;
        else
                sigs->ss_mask = arg;
        return 0;
}

/*
 * Called from exit() to clean out residual sigs.
 */
void
streams_sigexit (p)
	struct proc *p;
{
	SIGSP	sigs;
	SIGSQ *	sigq;
	STHP	sth;
	SQP	sq;

	STR_MALLOC(sq, SQP, sizeof *sq, M_STRSQ, M_WAITOK);
	sq_init(sq);
	while ((sigq = (SIGSQ *)p->p_strsigs.next) != (SIGSQ *)&p->p_strsigs) {
		remque(sigq);
		sigs = (SIGSP)((char *)sigq - (int)&((SIGSP)0)->ss_proclink);
		sth = sigs->ss_sth;
		csq_acquire(&sth->sth_wq->q_sqh, sq);
		remque(&sigs->ss_link);
		csq_release(&sth->sth_wq->q_sqh);
		STR_FREE(sigs, M_STRSIGS);
	}
	STR_FREE(sq, M_STRSQ);
}

int
osr_srdopt (osr)
	OSRP	osr;
{
	int	read_mode;

	read_mode = osr->osr_ioctl_arg1;
	if (read_mode & ~READ_OPTIONS_MASK)
		return EINVAL;
	/*
	 * If either the read mode or the proto action part is empty,
	 * default to the current mode recorded in the Stream head.
	 */
	if ((read_mode & READ_MODE_MASK) == 0)
		read_mode |= (osr->osr_sth->sth_read_mode & READ_MODE_MASK);
	if ((read_mode & READ_PROTO_MASK) == 0)
		read_mode |= (osr->osr_sth->sth_read_mode & READ_PROTO_MASK);

	/* Make sure we were given reasonable stuff. */
	switch ( read_mode & READ_MODE_MASK ) {
	case RNORM:
	case RMSGD:
	case RMSGN:
	case RFILL:
		break;
	default:
		return EINVAL;
	}
	switch ( read_mode & READ_PROTO_MASK ) {
	case RPROTNORM:
	case RPROTDIS:
	case RPROTDAT:
	/* RPROTCOMPRESS not settable by user */
		break;
	default:
		return EINVAL;
	}
	osr->osr_sth->sth_read_mode = read_mode;
	return 0;
}

int
osr_str (osr)
	OSRP	osr;
{
	STHP			sth = osr->osr_sth;
	IOCP			iocp;
	int			tmo, len;
reg	MBLKP			mp;
reg	MBLKP			mp1;
	int			error = 0;
	struct copyreq *	cqp;
#if SEC_BASE
	struct strioctl_attr *  stri = (struct strioctl_attr *)osr->osr_ioctl_arg0p;
#else
	struct strioctl	*	stri = (struct strioctl *)osr->osr_ioctl_arg0p;
#endif

	ENTER_FUNC(osr_str, osr, 0, 0, 0);

	if (stri->ic_timout < -1) {
		error = EINVAL;
		mp = 0;
		goto out;
	}
	if (stri->ic_timout == 0)
		stri->ic_timout = 15;
	if (stri->ic_timout < 0)
		stri->ic_timout = 0;
	tmo = MS_TO_TICKS(stri->ic_timout * 1000);

	if ( !(mp = allocb(sizeof(struct iocblk), BPRI_LO)) ) {
		error = EAGAIN;
		goto out;
	}
	mp->b_datap->db_type = M_IOCTL;
	mp->b_wptr += sizeof(struct iocblk);
	iocp = (IOCP)mp->b_rptr;

	if ( len = osr->osr_ioctl_arg1_len ) {
		if ( !(mp->b_cont = allocb(len, BPRI_LO)) ) {
			error = EAGAIN;
			goto out;
		}
		switch (stri->ic_len) {
		case FULLY_TRANSPARENT:
			/* len set to sizeof(caddr_t) by pse_ioctl() */
			*((caddr_t *)mp->b_cont->b_wptr) = stri->ic_dp;
			iocp->ioc_count = TRANSPARENT;
			break;
		case SEMI_TRANSPARENT:
			bcopy(osr->osr_ioctl_arg1p, mp->b_cont->b_wptr, len);
			iocp->ioc_count = len;
			break;
		default:
			csq_release(&sth->sth_rq->q_sqh);
			if (error = copyin(osr->osr_ioctl_arg1p,
					(caddr_t)mp->b_cont->b_wptr, len))
				goto out;
			iocp->ioc_count = len;
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
			break;
		}
		mp->b_cont->b_wptr += len;
	} else
		iocp->ioc_count = 0;
#if SEC_BASE
	ASSERT(mp->b_attr == NULL);
	csq_release(&sth->sth_rq->q_sqh);
	mp->b_attr = str_copyin_attrs(&stri->attrbuf, &error);
	if (error)
		goto out;
	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
#endif
	if (sth->sth_ioc_mp)
		freemsg(sth->sth_ioc_mp);
	sth->sth_ioc_mp      = nil(MBLKP);
	sth->sth_ioc_id      = sth_iocblk();
	iocp->ioc_cmd        = stri->ic_cmd;
	iocp->ioc_cr         = osr->osr_creds;
	iocp->ioc_id         = sth->sth_ioc_id;
	iocp->ioc_error      = 0;
	iocp->ioc_rval       = 0;

	/*
	 * The following loop iterates as long as we have to send more
	 * M_IOCDATA messages downstream. We don't care about canput(sth_wq)
	 * if we need to use it, because at most one extra will go there.
	 * Normally however we try to avoid it entirely and send the ioctl
	 * promptly. It's queued only to keep it in sequence. When queued
	 * it must be the M_IOCTL, which is band 0 - no need to check.
	 *
	 * Note: the upcoming M_COPY* message is re-used as M_IOCDATA message.
	 *	This makes it necessary to set the cp_rval field only after
	 *	we are done with the cq_* fields.
	 */
	for (;;) {
		if (mp->b_datap->db_type >= QPCTL
		||  sth_canput(sth, 0) >= 2
		||  sth->sth_wq->q_first == 0)
			putnext(sth->sth_wq, mp);
		else
			(void) putq_owned(sth->sth_wq, mp);
		error = osr_sleep(osr, TRUE, tmo);
		if ( !(mp = sth->sth_ioc_mp) ) {
			if (error == 0)
				error = ETIME;
			goto done;
		}
		sth->sth_ioc_mp = nil(MBLKP);

		switch (mp->b_datap->db_type) {
			caddr_t cp;
			unsigned cnt, size;
		case M_IOCACK:
			iocp = (IOCP)mp->b_rptr;
			error = iocp->ioc_error;
			osr->osr_ret_val = iocp->ioc_rval;
			goto check;
		case M_IOCNAK:
			iocp = (IOCP)mp->b_rptr;
			if ( !(error = iocp->ioc_error) )
				error = EINVAL;
			goto check;
		case M_COPYIN:
			if (mp->b_cont) {
				freemsg(mp->b_cont);
				mp->b_cont = 0;
			}
			if (mp->b_wptr - mp->b_rptr != sizeof(struct copyreq)) {
				error = EFAULT;
				goto done;
			}
			osr->osr_flags |= F_OSR_AUDIT_WRITE;
			mp->b_datap->db_type = M_IOCDATA;
			cqp = (struct copyreq *)mp->b_rptr;
			cp = cqp->cq_addr;
			size = cqp->cq_size;
			if ( !(mp1 = allocb(size, BPRI_LO)) ) {
				((struct copyresp *)cqp)->cp_rval = ENOSR;
				break;
			}
			error = copyin(cp, (caddr_t)mp1->b_rptr, size);
			if (error) {
				freeb(mp1);
				((struct copyresp *)cqp)->cp_rval = EFAULT;
				break;
			}
			mp1->b_wptr += size;
			mp->b_cont = mp1;
			((struct copyresp *)cqp)->cp_rval = 0;
			break;
		case M_COPYOUT:
			if (mp->b_wptr - mp->b_rptr != sizeof(struct copyreq)) {
				error = EFAULT;
				goto done;
			}
			csq_release(&sth->sth_rq->q_sqh);
			osr->osr_flags |= F_OSR_AUDIT_READ;
			mp->b_datap->db_type = M_IOCDATA;
			error = 0;
			cqp = (struct copyreq *)mp->b_rptr;
			cp = cqp->cq_addr;
			size = cqp->cq_size;
			mp1 = mp->b_cont;
			while (size > 0) {
				if (mp1 == NULL
				||  mp1->b_datap->db_type != M_DATA
				||  (cnt = mp1->b_wptr - mp1->b_rptr) > size)
					error = EFAULT;
				else if (cnt)
					error = copyout((caddr_t)mp1->b_rptr,
								cp, cnt);
				if (error)
					break;
				size -= cnt;
				cp += cnt;
				mp1 = mp1->b_cont;
			}
			if (error == 0 && mp1)
				error = EFAULT;
			freemsg(mp->b_cont);
			mp->b_cont = nil(MBLKP);
			((struct copyresp *)cqp)->cp_rval = error;
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
			break;
		default:
			error = EFAULT;
			goto done;
		} /* switch */
	} /* for */

check:
	/* sanity check */
	switch (error) {
	default:
		if (error >= 0 && error < 128)
			break;
	case ERESTART:
	case EJUSTRETURN:
STR_DEBUG(printf("osr_str: would have returned %d\n", error));
		error = EINVAL;
	}

done:
	/*
	 * Clear sth_ioc_id because we're not waiting for an answer anymore.
	 */
	if (sth->sth_ioc_mp) {
		freemsg(sth->sth_ioc_mp);
		sth->sth_ioc_mp = nil(MBLKP);
	}
	sth->sth_ioc_id = 0;
	csq_release(&sth->sth_rq->q_sqh);
	if ( error == 0 ) {
		/* Return M_IOCACK via pse_ioctl */
		osr->osr_ioctl_data = mp->b_cont;
		mp->b_cont = NULL;
		stri->ic_len = osr->osr_ioctl_arg1_len =
					((IOCP)mp->b_rptr)->ioc_count;
#if SEC_BASE
		stri->attrbuf.len = -1;
		if (mp->b_attr)
			(void) str_copyout_attrs(mp->b_attr, &stri->attrbuf);
#endif
	}
out:
	if (mp)
		freemsg(mp);
	LEAVE_FUNC(osr_str, error);
	return error;
}

int
osr_swropt (osr)
	OSRP	osr;
{
	switch (osr->osr_ioctl_arg1) {
	case SNDZERO:
	case 0:
		osr->osr_sth->sth_write_mode = osr->osr_ioctl_arg1;
		break;
	default:
		return EINVAL;
	}
	return 0;
}

/*
 * osr_unlink - handler for I_UNLINK requests
 *
 * Calls the internal unlink subroutine, since this functionality
 * is used by during close processing also.
 *
 * The subroutine may find that one or more of the lower streams have
 * to be closed, since this link was the last reference to them. For
 * this purpose, we hand it a list of OSR's (which is initially empty),
 * into which it will enter (close) OSR's for those streams. It will be
 * this level's responsibility to get those streams closed.
 */

int
osr_unlink (osr)
	OSRP	osr;
{
	OSRQ	close_osrq;
	int	error;

	osrq_init(&close_osrq);

	error = osr_unlink_subr(osr, osr->osr_ioctl_arg1,
			osr->osr_ioctl_arg2, &close_osrq);
	mult_sqh_release(osr);
	csq_release(&osr->osr_sth->sth_rq->q_sqh);

	if (close_osrq.osrq_first)
		(void) osr_close_subr(&close_osrq);

	return error;
}

int
osr_unlink_subr (osr, muxid, cmd, close_osrq)
	OSRP	osr;
	int	muxid;
	int	cmd;
	OSRQP	close_osrq;
{
	STHP	sth = osr->osr_sth;
	STHP	lsth;
	int	error = 0;
	int	is_persistent;
	MBLKP	mp;
	queue_t	* q;
	STHPP	sthpp;
	OSRP	close_osr;

	/* mult_sqh and sth locked on entry and return */
	osr->osr_closeout = (RWHL_ERROR_FLAGS & ~F_STH_CLOSED);	/* insurance */
	is_persistent = (cmd == I_PUNLINK);
	for (;;) {
		sthpp = sth_muxid_lookup(sth, muxid, is_persistent);
		lsth = *sthpp;
		if ( !lsth ) {
			if (muxid != MUXID_ALL)
				error = EINVAL;
			break;
		}
		for (q = sth->sth_wq; !STREAM_END(q); q = q->q_next)
			;
		mp = sth_link_alloc(osr, cmd, lsth->sth_muxid, q, lsth->sth_wq);
		if (mp == 0) {
			error = osr_bufcall(osr, TRUE, 0,
					sizeof (struct iocblk), BPRI_MED);
			if (error == 0)
				continue;
			if ( !(sth->sth_flags & F_STH_CLOSING) )
				return error;
			error = ENOSR;
		}
		if (mp) {
			if (sth->sth_ioc_mp)
				freemsg(sth->sth_ioc_mp);
			sth->sth_ioc_mp = nil(MBLKP);
			sth->sth_ioc_id = ((struct iocblk *)mp->b_rptr)->ioc_id;
			puthere(q, mp);
			error = osr_sleep(osr, FALSE,
				(sth->sth_flags & F_STH_CLOSING) ?
					MS_TO_TICKS(15000) : 0);
		}
		mp = sth->sth_ioc_mp;
		sth->sth_ioc_mp = nil(MBLKP);
		sth->sth_ioc_id = 0;
		if (sth->sth_flags & F_STH_CLOSING) {
STR_DEBUG(if (error || !mp) \
	printf("osr_unlink_subr: ioctl failure %d, closing anyway\n", error));
			error = 0;
		} else if (error == 0 && mp) {
			error = ((struct iocblk *)mp->b_rptr)->ioc_error;
			if (!error && mp->b_datap->db_type != M_IOCACK)
				error = EINVAL;
			/* sanity check */
			switch (error) {
			default:
				if (error >= 0 && error < 128)
					break;
			case ERESTART:
			case EJUSTRETURN:
STR_DEBUG(printf("osr_unlink_subr: would have returned %d\n", error));
				error = EINVAL;
			}
		} else if (error == 0)
			error = ETIME;
		if (mp)
			freemsg(mp);
		if (error)
			break;

		/*
		 * Now we must lock up the lower stream head
		 */
		csq_acquire(&lsth->sth_rq->q_sqh, &osr->osr_sq);
		csq_acquire(&lsth->sth_wq->q_sqh, &osr->osr_sq);

		/*
		 * Get out of the mux sth list
		 */
		sthpp[0] = lsth->sth_mux_link;
		lsth->sth_mux_link = nil(STHP);
		/* Reset the field values as they were before we linked in */
		lsth->sth_flags &= ~F_STH_LINKED;
		lsth->sth_read_error = 0;
		lsth->sth_write_error = 0;
		lsth->sth_muxid = 0;
		noenable(lsth->sth_wq);
		sth_set_queue(	lsth->sth_rq,
				sthinfo.st_rdinit,
				sthinfo.st_wrinit);
		lsth->sth_rq->q_ptr = (caddr_t)lsth;
		lsth->sth_wq->q_ptr = (caddr_t)lsth;

		/*
		 * If the lower stream is already closed, arrange to have it
		 * processed by osr_close_subr() now.
		 */
		if (lsth->sth_flags & F_STH_CLOSED) {
			/*
			 * Add the lower stream to
			 * the list of streams to close.
			 */
			lsth->sth_flags |= F_STH_CLOSING;
			close_osr = osr_alloc(lsth, 0, BPRI_WAITOK);
			close_osr->osr_creds = osr->osr_creds;
			close_osr->osr_osrq = &lsth->sth_ioctl_osrq;
			close_osr->osr_flags = F_OSR_NONBLOCK;
			close_osr->osr_ioctl_arg1 = O_NONBLOCK;
			osrq_insert(close_osrq, close_osr);
		}
		/*
		 * reset queue parenting to stream head synchronization
		 */
		csq_newparent(osr, lsth->sth_wq, &sthinfo);
		csq_newparent(osr, lsth->sth_rq, &sthinfo);
		csq_release(&lsth->sth_wq->q_sqh);
		csq_release(&lsth->sth_rq->q_sqh);

		if (muxid != MUXID_ALL ||
		    (is_persistent ? !sth->sth_pmux_top : !sth->sth_mux_top))
			break;
	}
	return error;
}

int
osr_write (osr)
reg	OSRP	osr;
{
	STHP		sth = osr->osr_sth;
	queue_t	*	q = sth->sth_wq;
reg	int		cnt;
	MBLKP		mp;
	int		maxpsz;
	u_long		wroff;
	int		error = 0;

	if (sth->sth_flags & F_STH_PIPE)
		sth_update_times(sth, FWRITE, (struct stat *)0);

	if (osr->osr_rw_count == 0  &&  !(sth->sth_write_mode & SNDZERO))
		return 0;
	if (bad_write_length(osr->osr_rw_count, q))
		return ERANGE;
	wroff = sth->sth_wroff;

	/*
	 * Write buffering: not larger than q_maxpsz and STRMSGSZ.
	 * If q_maxpsz indeterminate, aim for half of q_hiwat.
	 * If STRMSGSZ indeterminate, use a reasonable number.
	 */
	maxpsz = q->q_next->q_maxpsz;
	if (maxpsz == INFPSZ) {
		maxpsz = (STRMSGSZ > 0) ? STRMSGSZ : MCLBYTES;
		cnt = q->q_next->q_hiwat / 2;
		if (cnt > 0 && cnt < maxpsz && cnt >= q->q_next->q_minpsz)
			maxpsz = cnt;
		/*
		 * If multiple messages will be sent, "smoothing" their
		 * sizes is shown to improve throughput for most modules.
		 * Maxpsz is artificially decreased, but sensibly.
		 */
		if (osr->osr_rw_count > maxpsz) {
			cnt = (osr->osr_rw_count + maxpsz - 1) / maxpsz;
			cnt = (osr->osr_rw_count + cnt - 1) / cnt; /* size */
			maxpsz = (int) ALIGN(cnt);		   /* roundup */
		}
	}
	/* Chain buffers as in putmsg()? For now, clip. */
	else if (STRMSGSZ > 0 && maxpsz > STRMSGSZ)
		maxpsz = STRMSGSZ;

	do {
		cnt = MIN(osr->osr_rw_count, maxpsz);
		if (cnt < q->q_next->q_minpsz) {
 			/* Don't launch small residual message. */
			if (osr->osr_rw_total == 0)
				error = ERANGE;
			break;
		}
		if (STRMSGSZ > 0 && cnt + wroff > STRMSGSZ)
			wroff = STRMSGSZ - cnt;
		for (;;) {
			if (sth_canput(sth, 0) == 0)
				error = osr_sleep(osr, TRUE, 0);
			else if ((mp = allocb(cnt + wroff, BPRI_LO)) == NULL)
				error = osr_bufcall(osr, TRUE, 0,
							cnt + wroff, BPRI_LO);
			else
				break;
			if (error) {
				if (error == EAGAIN && (osr->osr_rw_total ||
				    ((sth->sth_flags & F_STH_NDELON) &&
				     (osr->osr_flags & F_OSR_NBIO) ==
						F_OSR_NDELAY)) )
					error = 0;
				return error;
			}
		}
		if (wroff)
			mp->b_rptr = (mp->b_wptr += wroff);
#if !SEC_BASE
		if (cnt > 0) {
			csq_release(&sth->sth_rq->q_sqh);
			if (error = sth_uiomove((caddr_t)mp->b_wptr, cnt, osr)){
				freemsg(mp);
				return error;
			}
			mp->b_wptr += cnt;
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
		}
#else
		csq_release(&sth->sth_rq->q_sqh);
		if (cnt > 0) {
			if (error = sth_uiomove((caddr_t)mp->b_wptr, cnt, osr)){
				freemsg(mp);
				return error;
			}
			mp->b_wptr += cnt;
		}
		ASSERT(mp->b_attr == NULL);
		mp->b_attr = str_copyin_attrs(NULL, &error);
		if (error) {
			freemsg(mp);
			return error;
		}
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
#endif
		for (;;) {
			switch (sth_canput(sth, 0)) {
			default:
				putnext(q, mp);
				goto out;
			case 2:
			case 1:
				(void) putq_owned(q, mp);
				goto out;
			case 0:
				if ( error = osr_sleep(osr, TRUE, 0) ) {
					/* count, offset unimportant */
					osr->osr_rw_total -= cnt;
					freemsg(mp);
					return error;
				}
				continue;
			}
		}
out:		;
	} while ( osr->osr_rw_count );
	return error;
}

/* Routines for assigning unique MUX id's.  This must be MP safe.
 * Make sure that the muxid never goes negative or is 0.
 * A simple counter should be sufficient since this will provide
 * 2 billion ID's before they get reused.
 */

decl_simple_lock_data(static,muxid_lock);
static int unique_muxid;

sth_muxid_init()
{
	simple_lock_init(&muxid_lock);
	unique_muxid = 1;
}

sth_muxid()
{
	register int muxid;

	simple_lock(&muxid_lock);
	if(unique_muxid < 1)
		unique_muxid = 1;
	muxid = unique_muxid++;
	simple_unlock(&muxid_lock);
	return muxid;
}
