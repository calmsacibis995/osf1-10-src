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
static char *rcsid = "@(#)$RCSfile: log.c,v $ $Revision: 4.2.5.3 $ (DEC) $Date: 1993/07/27 17:14:02 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/** Copyright (c) 1990, 1991  Mentat Inc.
 ** log.c 2.3, last change 11/24/90
 **/

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/strstat.h>
#include <sys/strlog.h>

extern int mi_iprintf();
extern int mi_mpprintf_putc();

#ifndef staticf
#define staticf static
#endif

typedef struct log_ctl  * LOGP;
typedef struct trace_ids * TRACEP;

typedef struct logi_s {
        int     logi_sid;
} LOGI, * LOGIP;

staticf	int		log_close(queue_t *, int, cred_t *);
staticf	int		log_deliver(queue_t *, mblk_t *, int);
static 	int		log_open(queue_t *, dev_t *, int, int, cred_t *);
staticf	queue_t	*	log_should_deliver(short, short, char, short);
staticf	int		log_wput(queue_t *, mblk_t *);

static	queue_t	*	log_error_q;
static	mblk_t *	log_trace_blks;
static	queue_t	*	log_trace_q;
static	int		trace_seq_num;
static	int		error_seq_num;
decl_simple_lock_data(static,log_lock)

#define	MODULE_ID	44
static struct module_info minfo =  {
	MODULE_ID, "LOG", 0, INFPSZ, 2048, 128
};

static struct qinit rinit = {
	NULL, NULL, log_open, log_close, NULL, &minfo
};

static struct qinit winit = {
	log_wput, NULL, NULL, NULL, NULL, &minfo
};

struct streamtab loginfo = { &rinit, &winit };
void
strlog_init()
{
	simple_lock_init(&log_lock);
}

/** Submit messages for logging */
int
strlog ( mid, sid, level, flags, fmt, va_alist)
	short mid;
	short sid;
	char  level;
	unsigned short flags; 
	char * fmt;
	va_dcl
{
        va_list ap;
	queue_t	* deliver_q;
	mblk_t *  mp;
	LOGP	lctlp;
	int s = splstr();

	simple_lock(&log_lock);
	/*
	 * Increment message numbers even if this one doesn't go anywhere,
	 * or we can't get memory.
	 */
	if (flags & SL_TRACE)
		trace_seq_num++;
	if (flags & SL_ERROR)
		error_seq_num++;

	/* No log or trace programs open, so quit early */
	if (!log_trace_q  &&  !log_error_q  &&  !(flags & SL_CONSOLE))
		goto out;
	/* Find out if we should deliver this message */
	if (!(deliver_q = log_should_deliver(mid, sid, level, (short)flags)))
		goto out;
	if (!(mp = allocb(sizeof(struct log_ctl), BPRI_MED)))
		goto out;
	if (!(mp->b_cont = allocb(LOGMSGSZ, BPRI_MED))) {  
		freeb(mp);
		goto out;
	}
        if (fmt) {
		va_start(ap);
                mi_iprintf(fmt, ap, mi_mpprintf_putc, (char *)mp->b_cont);
                mi_mpprintf_putc((char *)mp->b_cont, '\0');
		va_end(ap);
	}
	mp->b_datap->db_type = M_PROTO;
	mp->b_wptr += sizeof(struct log_ctl);
	lctlp = (LOGP)mp->b_rptr;
	lctlp->sid = sid;
	lctlp->mid = mid;
	lctlp->level = level;
	lctlp->flags = flags;
	log_deliver(deliver_q, mp, flags);
out:
	simple_unlock(&log_lock);
	splx(s);
	return 0;
}

staticf int
log_close (q, flag, credp)
	queue_t	* q;
	int	flag;
	cred_t	* credp;
{
	int s = splstr();

	simple_lock(&log_lock);
	if (q == log_error_q)
		log_error_q = 0;
	if (q == log_trace_q) {
		log_trace_q = 0;
		if (log_trace_blks) {
			freemsg(log_trace_blks);
			log_trace_blks = 0;
		}
	}
	simple_unlock(&log_lock);
	splx(s);
	return streams_close_comm(q, flag, credp);
}

staticf int
log_deliver (q, mp, flags)
	queue_t	* 	q;
	mblk_t *	mp;
	int		flags;
{
	LOGP		logp;
	mblk_t *	mp1;

	logp = (LOGP)mp->b_rptr;
	logp->ltime = lbolt;
	logp->ttime = time;
	logp->seq_no = (flags & SL_ERROR) ? error_seq_num : trace_seq_num;
	if (flags & SL_CONSOLE) {
		simple_unlock(&log_lock);
		printf("%s\r\n", mp->b_cont->b_rptr);
		simple_lock(&log_lock);
		flags &= ~SL_CONSOLE;
		if (flags == 0)
			goto out;
	}
	if ((flags & SL_ERROR)  &&  log_error_q) {
		if (q  &&  q == log_trace_q)
			mp1 = dupmsg(mp);
		else
			mp1 = 0;
		putnext(log_error_q, mp);
		mp = mp1;
	}
	if (q  &&  q == log_trace_q  &&  mp) {
		putnext(q, mp);
		mp = 0;
	}
out:
	if (mp)
		freemsg(mp);
	return 0;
}

staticf int
log_open (q, devp, flag, sflag, credp)
	queue_t	* q;
	dev_t	* devp;
	int	flag;
	int	sflag;
	cred_t	* credp;
{
	int	err;

	if (q->q_ptr)
		return ENXIO;
	err = streams_open_comm(sizeof(LOGI), q, devp, flag, sflag, credp);
	if (!err)
		((LOGIP)q->q_ptr)->logi_sid = minor(*devp);
	return err;
}

staticf queue_t *
log_should_deliver (
	short		mid,
	short		sid,
	char		level,
	short		flags)
{
	queue_t *	q = 0;
	mblk_t *	mp;
	TRACEP		tracep;

	if (flags & SL_TRACE) {
		for (mp = log_trace_blks; mp; mp = mp->b_cont) {
			for (tracep = (TRACEP)mp->b_rptr;
			     tracep < (TRACEP)mp->b_wptr;
			     tracep++) {
				if ((tracep->ti_mid == -1 ||
						tracep->ti_mid == mid)
				&& (tracep->ti_sid == -1 ||
					tracep->ti_sid == sid)
				&& (tracep->ti_level == -1 ||
						tracep->ti_level >= level)) {
					q = log_trace_q;
					goto out;
				}
			}
		}
	}
out:
	if (q == 0 && (flags & SL_ERROR))
		q = log_error_q;
	if (q == 0 && (flags & SL_CONSOLE))
		q = (queue_t *)-1;
	return q;
}

staticf int
log_wput (q, mp)
	queue_t	* 	q;
	mblk_t *	mp;
{
	struct iocblk * iocp;
	LOGP		logp;
	mblk_t *	mp1;
	int		s;

	switch (mp->b_datap->db_type) {
	case M_PCPROTO:
	case M_PROTO:
		if ((mp->b_wptr - mp->b_rptr) < sizeof(struct log_ctl)
		||  !mp->b_cont)
			break;
		logp = (LOGP)mp->b_rptr;
		logp->mid = MODULE_ID;
		logp->sid = ((LOGIP)q->q_ptr)->logi_sid;
		s = splstr();
		simple_lock(&log_lock);
		if (logp->flags & SL_TRACE)
			trace_seq_num++;
		if (logp->flags & SL_ERROR)
			error_seq_num++;
		q = log_should_deliver(logp->mid, logp->sid, logp->level, logp->flags);
		if (q)
			log_deliver(q, mp, logp->flags);
		simple_unlock(&log_lock);
		splx(s);
		if (q)
			return 0;
		break;
	case M_DATA:
		break;
	case M_IOCTL:
		iocp = (struct iocblk *)mp->b_rptr;
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = ENXIO;
		s = splstr();
		simple_lock(&log_lock);
		switch (iocp->ioc_cmd) {
		case I_TRCLOG:
			if (iocp->ioc_count < sizeof(struct trace_ids)
			||  (iocp->ioc_count % sizeof(struct trace_ids)) != 0
			||  !mp->b_cont)
				break;
			if (log_trace_q  &&  log_trace_q != RD(q))
				break;
			if (!log_trace_q)
				log_trace_q = RD(q);
			mp1 = mp->b_cont;
			mp->b_cont = 0;
			mp1->b_cont = log_trace_blks;
			log_trace_blks = mp1;
			iocp->ioc_error = 0;
			break;
		case I_ERRLOG:
			if (mp->b_cont) {
				freemsg(mp->b_cont);
				mp->b_cont = 0;
			}
			if (log_error_q  &&  log_error_q != RD(q))
				break;
			if (!log_error_q)
				log_error_q = RD(q);
			iocp->ioc_error = 0;
			break;
		default:
			mp->b_datap->db_type = M_IOCNAK;
			break;
		}
		simple_unlock(&log_lock);
		splx(s);
		iocp->ioc_count = 0;
		qreply(q, mp);
		return 0;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) {
			*mp->b_rptr &= ~FLUSHW;
			qreply(q, mp);
			return 0;
		}
		break;
	}
	freemsg(mp);
	return 0;
}
