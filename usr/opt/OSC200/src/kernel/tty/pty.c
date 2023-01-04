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
static char *rcsid = "@(#)$RCSfile: pty.c,v $ $Revision: 1.1.6.10 $ (DEC) $Date: 1994/01/07 20:45:48 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
#include <data/pty_data.c>
#include <streams/memdebug.h>
#if NRPTY > 0



/*
 * Streams based pseudo terminal.
 *
 * This file contains a streams based pty slave device (names starting 
 * with "pts_"), plus a streams based, System V style master device (names
 * starting with "ptm_"), plus a non-streams, BSD style master (names
 * starting with "ptr").  The BSD master, while not a streams device
 * itself, is aware of the streams implementation of the slave, and works
 * with it appropriately.  The slave device uses three function pointers
 * loaded by the master at master-open time to cooperate with its master
 * without actually knowing which type the master is.
 *
 * An example of the layout at runtime is this:
 *
 *    +-------------+         +-------------+
 *    |   server    |         | application |
 * ---------------------------------------------
 *    | stream head |         | stream head |
 *    +-------------+         +-------------+
 *       |       ^               |       ^
 *       |       |               |       |
 *       |       |            +-------------+
 *       |       |            | line disc   |
 *       |       |            +-------------+
 *       |       |               |       |
 *       v       |               v       |
 *    +-------------+         +-------------+
 *    |             | <------ |             |
 *    |   master    | ------> |   slave     |
 *    +-------------+         +-------------+
 */

/*
 * Streams control structures.
 */

PRIVATE_STATIC struct module_info pts_info = {
	7608, "pts", 0, INFPSZ, SLAVE_HIWATER, SLAVE_LOWATER
};
PRIVATE_STATIC struct module_info ptm_info = {
	7609, "ptm", 0, INFPSZ, SYSV_MAST_HIWATER, SYSV_MAST_LOWATER
};

PRIVATE_STATIC struct qinit pts_rinit = {
	pts_rput, pts_rsrv, pts_open, pts_close, 0, &pts_info, 0
};
PRIVATE_STATIC struct qinit pts_winit = {
	pts_wput, pts_wsrv, 0, 0, 0, &pts_info, 0
};
PRIVATE_STATIC struct qinit ptm_rinit = {
	ptm_rput, ptm_rsrv, ptm_open, ptm_close, 0, &ptm_info, 0
};
PRIVATE_STATIC struct qinit ptm_winit = {
	ptm_wput, ptm_wsrv, 0, 0, 0, &ptm_info, 0
};

struct streamtab ptsinfo = { &pts_rinit, &pts_winit };
struct streamtab ptminfo = { &ptm_rinit, &ptm_winit };

/* pts_cdev: maj+min for the slave.  Since the masters are at two different
 * major numbers, the slave devno is used by the masters AND the slave in
 * dealing with cdevsw_open_comm() et. al.  The pts_cdev dev_t is passed
 * by the masters to the *ocomm versions of the cdevsw routines.
 */

extern dev_t pts_cdev;

/*
 * Sleep messages.
 */

const static char ptyin[] = "ptyin";
const static char ptyout[] = "ptyout";
const static char ptyopen[] =  "ptyopen";
const static char ptyclose[] =  "ptyclose";
const static char ptyclck[] =  "ptyclck";
const static char ptylck[] =  "ptylck";

/*
 * PTY_OPCL_lock: a single global (across this file) lock to serialize
 * execution of {streams,cdevsw}_{open,close}_*comm().  It also
 * synchronizes access to the "pt_alloc" field in pty_s structures.
 */

static lock_data_t PTY_OPCL_lock;
/*
 * the following define is needed for backwards binary
 * compatibility for staticly linked binaries
 */ 
#define old_isptm	0x40047447

#define flush_switch_flags(flagp)					\
	do {								\
		int tmpflag = *(flagp);					\
									\
		*(flagp) &= ~(*(flagp));				\
		if (tmpflag & FLUSHR)					\
			*(flagp) |= FLUSHW;				\
		if (tmpflag & FLUSHW)					\
			*(flagp) |= FLUSHR;				\
	} while (0)

int
pts_configure(
	sysconfig_op_t	op,
	str_config_t *	indata,
	size_t		indatalen,
	str_config_t *	outdata,
	size_t		outdatalen
	)
{
	dev_t		devno;
	int		indx;
	short       	configured;
	int		ret = 0;
	struct streamadm	ptsadm;

	if (indata != NULL && indatalen == sizeof(str_config_t)
			&& indata->sc_version == OSF_STREAMS_CONFIG_10)
		devno = indata->sc_devnum;
	else
		devno = NODEV;

	if (devno != NODEV) 
		cdevsw_del(devno);


	switch (op) {

	case SYSCONFIG_CONFIGURE:
		/*
		 * Initialize all configurable variables
		 */
		ptsadm.sa_version	= OSF_STREAMS_10;
		ptsadm.sa_flags		= STR_IS_DEVICE | STR_SYSV4_OPEN;
		ptsadm.sa_sync_level	= SQLVL_QUEUEPAIR;
		ptsadm.sa_ttys		= 0;
		ptsadm.sa_sync_info	= 0;
		strcpy(ptsadm.sa_name, "pts");


		PTY_OPCL_LOCK_INIT();

		/* 
		 * Go through the input buffer and set all variables 
		 * to the given values.
		 */

		if ((outdata != NULL) && (indata != NULL)) 
			strcpy(outdata, indata);


		if (strmod_add(devno, &ptsinfo, &ptsadm) == NODEV)
			return ENODEV;
		
		
		if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
			outdata->sc_version = OSF_STREAMS_CONFIG_10;
			outdata->sc_devnum = makedev(major(devno), 0);
			outdata->sc_sa_flags = ptsadm.sa_flags;
			strcpy(outdata->sc_sa_name, ptsadm.sa_name);
		}
		break;


	default:
		ret = EINVAL;
		break;

	} /* switch */

	return(ret);
}

int
ptm_configure(
	sysconfig_op_t	op,
	str_config_t *	indata,
	size_t		indatalen,
	str_config_t *	outdata,
	size_t		outdatalen
	)
{
	struct streamadm	ptmadm;
	dev_t			devno;

	if (op != SYSCONFIG_CONFIGURE)
		return EINVAL;
	if (indata != NULL && indatalen == sizeof(str_config_t)
			&& indata->sc_version == OSF_STREAMS_CONFIG_10)
		devno = indata->sc_devnum;
	else
		devno = NODEV;

	ptmadm.sa_version	= OSF_STREAMS_10;
	ptmadm.sa_flags		= STR_IS_DEVICE|STR_SYSV4_OPEN;
	ptmadm.sa_ttys		= 0;
	ptmadm.sa_sync_level	= SQLVL_QUEUEPAIR;
	ptmadm.sa_sync_info	= 0;
	strcpy(ptmadm.sa_name, "ptm");

	if ( (devno = strmod_add(devno, &ptminfo, &ptmadm)) == NODEV ) {
		return ENODEV;
	}

	if (outdata != NULL && outdatalen == sizeof(str_config_t)) {
		outdata->sc_version = OSF_STREAMS_CONFIG_10;
		outdata->sc_devnum = devno;
		outdata->sc_sa_flags = ptmadm.sa_flags;
		strcpy(outdata->sc_sa_name, ptmadm.sa_name);
	}

	return 0;
}

PRIVATE_STATIC void
pty_init(struct pty_s *ptp)
{
	PTY_LOCK_INIT(ptp);
	simple_lock_init(&ptp->pty_lock);
#if SEC_ARCH
	pty_sec_init(ptp);
#endif /* SEC_ARCH */
}

PRIVATE_STATIC void
pty_wakeup(caddr_t chan)
{
	thread_wakeup_prim((vm_offset_t)chan, FALSE, THREAD_AWAKENED);
}
/*
 * wakeup blocked thread with a wait_result of THREAD_TIMED_OUT
 * this will indicate revoke was called (this works when sleeping
 * w/o timeout) 
 */
pty_revoke_wakeup(caddr_t chan)
{
	thread_wakeup_prim((vm_offset_t)chan, FALSE, THREAD_TIMED_OUT);
}
  
PRIVATE_STATIC int
pts_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp)
{ 
	register struct pty_s *ptp;
	int error, reopen = (q->q_ptr != 0);
#if     SEC_ARCH
        int mode;
#endif
 
	PTY_OPCL_LOCK();
	if (error = streams_open_comm(sizeof(*ptp), q, devp, flag, 
				      sflag, credp)) {
		PTY_OPCL_UNLOCK();
		return(error);
	} else {
		ptp = (struct pty_s *) q->q_ptr;
		ASSERT(ptp != 0);
		if ((ptp->pt_alloc & PFA_ALLOC) == 0)
			pty_init(ptp);
		ptp->pt_alloc |= PFA_SLAVE_ALLOC;
		PTY_OPCL_UNLOCK();
	}
 
	PTY_WRITE_LOCK(ptp);

	simple_lock(&ptp->pty_lock);
	if (ptp->pt_flags & PF_SLOCK) {
		/*
		 * This is a locked SYS V slave (application needs to run 
		 * unlockpt() on master before slave open can succeed).  SVID 
		 * doesn't say which errno should be used in this situation; 
		 * EBUSY seems pretty reasonable.
		 *
		 * NB: If this check moves below the tsleep, a wakeup must
		 * be added to ptm_open().
		 */
		simple_unlock(&ptp->pty_lock);
		error = EBUSY;
		goto out;
	} 

	/* PTY_WRITE_LOCK and simple lock are both held */

	if (!ptp->pt_srq && !(ptp->pt_flags & PF_SWOPEN)) {
		/*
		 * default termios settings
		 */
		bcopy(ttydefchars, ptp->pt_cc, sizeof(ptp->pt_cc));
		ptp->pt_iflag = TTYDEF_IFLAG;
		ptp->pt_oflag = TTYDEF_OFLAG;
		ptp->pt_lflag = TTYDEF_LFLAG;
		/* We never actually use pt_cflag -- we just don't want it to
		 * look ridiculous in "stty -a" output.
		 */
		ptp->pt_cflag = TTYDEF_CFLAG;
		ptp->pt_ospeed = ptp->pt_ispeed = TTYDEF_SPEED;
		simple_unlock(&ptp->pty_lock);
	} else {
		if (ptp->pt_flags & PF_XCLUDE) {
			simple_unlock(&ptp->pty_lock);
			if (drv_priv(credp) != 0) {
				error = EBUSY;
				goto out;
			}
		} else
			simple_unlock(&ptp->pty_lock);
	}

	/* WRITE_LOCK is held; simple lock is not */

	if ((flag & (O_NDELAY | O_NONBLOCK)) == 0) {
		for (;;) {
			if (ptp->pt_mrq)
				break;
			simple_lock(&ptp->pty_lock);
			ptp->pt_flags |= PF_SWOPEN;
			simple_unlock(&ptp->pty_lock);
			assert_wait((vm_offset_t) &ptp->pt_mwq, TRUE);
			PTY_WRITE_UNLOCK(ptp);
			if (error = 
			    tsleep((caddr_t)0, PTYPRI|PCATCH, (char *)ptyopen, 0)) {
			/*
			 * if awoken because of revoke...
			 * let osr_open know about it!
			 */
				if (error == EWOULDBLOCK) 
					return (ECANCELED); 
				goto out2;
			}
			PTY_WRITE_LOCK(ptp);
		}
	} else {
		/* Non-blocking slave opens are now disallowed for
		 * security reasons.
		 */
		if (!ptp->pt_mrq) {
			error = EWOULDBLOCK;
			goto out;
		}
	}

#if	SEC_ARCH
        mode = 0;
        if (flag & FREAD)
                mode |= SP_READACC;
        if (flag & FWRITE)
                mode |= SP_WRITEACC;
        if (SP_ACCESS(SIP->si_tag, &ptp->pt_stag[0], mode, NULL)) {
		error = EACCES;
		goto out;
        }
#endif
	simple_lock(&ptp->pty_lock);
	ptp->pt_flags &= ~PF_SWOPEN;
	ptp->pt_flags |= PF_SOPEN;
	ptp->pt_srq = q;
	if (ptp->pt_mrq)
		ptp->pt_flags |= PF_CARR_ON;
	simple_unlock(&ptp->pty_lock);

	/*
	 * prevent writeq from being scheduled unnecessarily
	 */
	noenable(WR(q));
  
	if (ptp->pt_enable) {
		(*ptp->pt_enable)(ptp, FREAD|FWRITE);
	}
out:
	PTY_WRITE_UNLOCK(ptp);
out2:
	if (error) {
		PTY_OPCL_LOCK();

		/* Free struct if we allocated it */
		if (!reopen)
			ptp->pt_alloc &= ~PFA_SLAVE_ALLOC;
		if ((ptp->pt_alloc & PFA_ALLOC) == 0)
			(void) streams_close_comm(q, flag, credp);

		PTY_OPCL_UNLOCK();
	}

	return(error);
}

PRIVATE_STATIC int
pts_close(queue_t *q, int flag, cred_t *credp)
{
	register struct pty_s * ptp;
	mblk_t *eof_msg;
	int mdrained;
	int err = 0;
  
	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);

	PTY_READ_LOCK(ptp);
	if (ptp->pt_mrq) {
		queue_t *swq = WR(ptp->pt_srq);

		/* Drain output before proceeding with close.
		 */

		simple_lock(&ptp->pty_lock);

		if (!(flag & (FNDELAY|FNONBLOCK))) {
			ptp->pt_flags &= ~PF_TTSTOP;
			for (;;) {
				ASSERT(ptp->pt_mrq != 0);

				if ((mdrained = (*ptp->pt_mdrained)(ptp))
				&& (swq->q_first == 0))
					break;

				if (mdrained) {
					simple_unlock(&ptp->pty_lock);
					(void) pts_start(swq, 0);
					simple_lock(&ptp->pty_lock);
					mdrained = (*ptp->pt_mdrained)(ptp);
				}

				if (!mdrained || swq->q_first) {
					ptp->pt_flags |= PF_WDRAIN;
					assert_wait((vm_offset_t)swq, 1);
					simple_unlock(&ptp->pty_lock);
					PTY_READ_UNLOCK(ptp);
					if (err = tsleep((caddr_t)0,
					    PTYPRI|PCATCH, (char *) ptyclose, 0)) {
						PTY_READ_LOCK(ptp);
						simple_lock(&ptp->pty_lock);
						break;
					} else {
						PTY_READ_LOCK(ptp);
						simple_lock(&ptp->pty_lock);
						if (!ptp->pt_mrq)
							break;
					}
				}
			}
		}
		ptp->pt_flags &= ~PF_WDRAIN;
		simple_unlock(&ptp->pty_lock);

		/* send EOF to master -- this will have the side effect
		 * of waking up any sleeping reads.
		 */
		if (ptp->pt_mrq) {
			/* Get a zero length M_DATA */
			eof_msg = allocb(0, BPRI_WAITOK);

			if (!((*ptp->pt_xfer)(ptp, eof_msg)))
				freemsg(eof_msg);
		}
	}

	if (ptp->pt_srq)
		flushq(WR(ptp->pt_srq), FLUSHALL);

	PTY_RD2WR_LOCK(ptp);

	ptp->pt_srq = NULL;	

	if (ptp->pt_mrq) {
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags &= ~(PF_CARR_ON|PF_SOPEN|PF_SFLOW);
		simple_unlock(&ptp->pty_lock);

		/* Wake up any blocked read's and write's.  They should
		 * do something reasonable if they wake up to discover
		 * that the slave read queue is NULL.
		 */
		PTY_WR2RD_LOCK(ptp);
		(*ptp->pt_enable)(ptp, FREAD | FWRITE);
		PTY_READ_UNLOCK(ptp);
	} else {
		pty_revoke_wakeup((caddr_t) &ptp->pt_mwq);
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags = 0;	/* clear all flag bits */
		simple_unlock(&ptp->pty_lock);
		PTY_WRITE_UNLOCK(ptp);
	}

	PTY_OPCL_LOCK();
	ptp->pt_alloc &= ~PFA_SLAVE_ALLOC;
	if ((ptp->pt_alloc & PFA_ALLOC) == 0)
		(void) streams_close_comm(q, flag, credp);
	PTY_OPCL_UNLOCK();

	return(err);
}

/* Slave write put procedure.
 *
 *	NB: 'q' is noenable'd at slave open time.
 */

PRIVATE_STATIC int
pts_wput(queue_t *q, mblk_t *mp)
{
	register struct pty_s *ptp;
	register struct iocblk *iocp;
	mblk_t *mp1;
	int size, cmd, ret;
	caddr_t data;

	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);

	PTY_READ_LOCK(ptp);
	switch(mp->b_datap->db_type) {
	case M_IOCTL:
		if (ptp->pt_qioctl) {
			rmvq(q, ptp->pt_qioctl);
			freemsg(ptp->pt_qioctl);
			ptp->pt_qioctl = (mblk_t *) NULL;
		}
		if (strtty_ioctlbad(mp)) {
			PTY_READ_UNLOCK(ptp);
			qreply(q, mp);
			goto out;
		}
		switch (((struct iocblk *)mp->b_rptr)->ioc_cmd) {
		case TIOCSETAW: /* these wait */
		case TIOCSETAF: /* for output */
		case TCSBRK:  /* to drain */
			if (q->q_first) {
				ptp->pt_qioctl = mp;
				putq(q, mp); /* so just queue them in order */
				break;
			}

		/* fall through */

		default:
			pty_strioctl_comm(mp, ptp, 0);
			PTY_READ_UNLOCK(ptp);
			qreply(q, mp);
			goto out;
		} 
		break;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);
		if (ptp->pt_mrq)
			(void) ((*ptp->pt_xfer)(ptp, mp));
		else
			if (*mp->b_rptr & FLUSHR) {
				*mp->b_rptr &= ~FLUSHW;
				flushq(q->q_other, FLUSHDATA);
				qreply(q, mp);
			} else
				freemsg(mp);
		break;
	case M_DATA:
		if (q->q_first || !pts_writedata(ptp, mp)) {
			/* If pts_writedata() returned 0, then either
			 * master read queue is full or we've been
			 * explicitly stopped (M_STOP).  We'll be
			 * qenabled when either condition changes.
			 */
			mp->b_flag |= MSGCOMPRESS;
			putq(q, mp);
		}
		break;
	case M_START:
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags &= ~ PF_TTSTOP;
		simple_unlock(&ptp->pty_lock);
		qenable(q);
		if (ptp->pt_mrq)
			(void) ((*ptp->pt_xfer)(ptp, mp));
		else
			freemsg(mp);
		break;
	case M_STOP:
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags |= PF_TTSTOP;
		simple_unlock(&ptp->pty_lock);
		if (ptp->pt_mrq)
			(void) ((*ptp->pt_xfer)(ptp, mp));
		else
			freemsg(mp);
		break;
	case M_STARTI:
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags &= ~ PF_TTINSTOP;
		simple_unlock(&ptp->pty_lock);
		if (ptp->pt_mrq)
			(void) ((*ptp->pt_xfer)(ptp, mp));
		else
			freemsg(mp);
		break;
	case M_STOPI:
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags |= PF_TTINSTOP;
		simple_unlock(&ptp->pty_lock);
		if (ptp->pt_mrq)
			(void) ((*ptp->pt_xfer)(ptp, mp));
		else
			freemsg(mp);
		break;
	case M_CTL:
		if (size = pts_mctl(q, mp)) {
			putbq(q, mp);
			bufcall(size, BPRI_MED, qenable, q);
		}
		break;
	default:
		freemsg(mp);
		break; 
	}
	PTY_READ_UNLOCK(ptp);
out:
	return(0);
}

/* 
 * pts_writedata: 
 *
 *	write data to the master.
 *
 * If the master is up, but we're flow controlled, return 0 telling
 * caller to try sending this same message later.  If the master is
 * down, or if the message transmits successfully, return 1 -- caller
 * should assume that something reasonable was done with the message in
 * this case.
 *
 * The PTY_READ_LOCK should be held on entry to this routine.
 */

PRIVATE_STATIC int
pts_writedata(struct pty_s *ptp, mblk_t *mp)
{
	PTY_READ_LOCK_ASSERT(ptp);
	if (ptp->pt_mrq == 0) {
		freemsg(mp);
		return(1);	/* message ignored */
	}

	simple_lock(&ptp->pty_lock);

	if (ptp->pt_flags & PF_TTSTOP) {
		simple_unlock(&ptp->pty_lock);
		return(0);
	}

	simple_unlock(&ptp->pty_lock);

	return((*ptp->pt_xfer)(ptp, mp));
} 

/* 
 * Slave write service.
 */

PRIVATE_STATIC int
pts_wsrv(queue_t *q)
{
	(void) pts_start(q, 1);
	return(0);
}

/* 
 * Slave start-output routine.  We are called either by pts_wsrv()
 * (from_srvp == 1) or by pts_close() (from_srvp == 0).  A return value
 * of 0 indicates flow slave output flow control (pts_close() will
 * block waiting for data to drain).
 */

PRIVATE_STATIC int
pts_start(queue_t *q, int from_srvp)
{
	register struct pty_s * ptp;	
	register struct iocblk *iocp;
	mblk_t *mp, *mp1;
	int (*func_ptr)();
	int size, cmd, ret;
	caddr_t data;

	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);
	PTY_READ_LOCK(ptp);
	while (mp = getq(q)) {
		switch(mp->b_datap->db_type) {
		case M_DATA:
			if (!pts_writedata(ptp, mp)) {
				/* flow control 
				 */
				PTY_READ_UNLOCK(ptp);
				putbq(q, mp);
				/* Master will qenable/wake us when flow is
				 * back on.
				 */
				return 0;
			}
			continue;
		case M_CTL:
			if (size = pts_mctl(q, mp)) {
				if (from_srvp) {
					putbq(q, mp);	
					bufcall(sizeof(struct termios),
							BPRI_MED, qenable, q);
					PTY_READ_UNLOCK(ptp);
					return(1);
				} else
					freemsg(mp);
			}
			continue;
		case M_IOCTL:
			ASSERT(mp == ptp->pt_qioctl);
			ptp->pt_qioctl = (mblk_t *) NULL;
			pty_strioctl_comm(mp, ptp, 0);
			qreply(q, mp);
			continue; 
		default:
#if MACH_ASSERT
			printf("pts_start: message 0x%x\n", mp);
			panic("pts_start: unexpected message");
#endif
			freemsg(mp);
			continue;
		}
	}
	PTY_READ_UNLOCK(ptp);
	return(1);
}

/*
 * The PTY_READ_LOCK is held on entry to this routine.
 */

PRIVATE_STATIC int
pts_rput(queue_t *q, mblk_t *mp)
{
	switch(mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) 
			flushq(q, FLUSHDATA);
		putnext(q, mp);
		break;
	default:
		if ( (mp->b_datap->db_type >= QPCTL)
		     || (!q->q_first && canput(q->q_next)) )
			putnext(q, mp);
		else {
			mp->b_flag |= MSGCOMPRESS;
			putq(q, mp);
		}
		break;
	}
}

PRIVATE_STATIC int
pts_rsrv(queue_t *q)
{
	register struct pty_s *ptp;
	register mblk_t *mp;

	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);
	while (mp = getq(q)) {
		if (mp->b_datap->db_type >= QPCTL || canput(q->q_next))
			putnext(q, mp);
		else {
			putbq(q, mp);
			return;
		}
		simple_lock(&ptp->pty_lock);
		if ((ptp->pt_flags & PF_MFLOW) && (q->q_count <= q->q_lowat)) {
			ptp->pt_flags &= ~PF_MFLOW;
			simple_unlock(&ptp->pty_lock);
			PTY_READ_LOCK(ptp);
			if (ptp->pt_enable)
				(*ptp->pt_enable)(ptp, FWRITE);
			PTY_READ_UNLOCK(ptp);
		} else
			simple_unlock(&ptp->pty_lock);
	}
}

/* Perform M_CTL processing for slave write side.  READ_LOCK is held
 * on entry to this routine.  A non-zero return value indicates the
 * size of memory request which we're unable to meet in processing
 * this message.  Caller can putbq/bufcall or ignore, as desired.
 */

PRIVATE_STATIC int
pts_mctl(queue_t *q, mblk_t *mp)
{
	struct iocblk *iocp = (struct iocblk *)mp->b_rptr;
	struct pty_s *ptp = (struct pty_s *) q->q_ptr;
	mblk_t *mp1;

	switch(iocp->ioc_cmd) {
	case TIOCGETA:
		if (!mp->b_cont) {
			mp1 = allocb(sizeof(struct termios), BPRI_MED);
			/*
			 * if allocation fails try again later
			 */
			if (!mp1)
				return(sizeof(struct termios));

			mp->b_cont = mp1;
		} else  {
			mp1 = mp->b_cont;
			if ( (mp1->b_datap->db_lim - mp1->b_rptr) 
			     < sizeof(struct termios) ) {
				freemsg(mp);
				return 0;
			}
		}
		simple_lock(&ptp->pty_lock);
		bcopy(&ptp->pt_termios, mp1->b_rptr, sizeof(struct termios));
		simple_unlock(&ptp->pty_lock);
		mp1->b_wptr = mp1->b_rptr + sizeof(struct termios);
		qreply(q, mp);
		return 0;
	default:
		freemsg(mp);
		return 0;
	}
}

/*
 * pty_strioctl_comm:
 *
 * 	Process an M_IOCTL streams message.  The message has already 
 *	been checked (via strtty_ioctlbad()) for validity of pointers,
 *	sizes, etc.
 *
 *	The flag argument indicates the origin of the message:
 *
 *		0 == slave		1 == master
 */

PRIVATE_STATIC void
pty_strioctl_comm(mblk_t *mp, struct pty_s *ptp, int flag)
{
	int 		error;
	caddr_t		data;
	struct iocblk	*iocp = (struct iocblk *)mp->b_rptr;
	int		cmd = iocp->ioc_cmd;

	if (mp->b_cont)
		data  = (caddr_t)mp->b_cont->b_rptr;
	else
		data = NULL;
	if (error = pty_ioctl_comm(&iocp->ioc_rval, cmd, data, ptp, flag)) {
		mp->b_datap->db_type = M_IOCNAK;
		if (mp->b_cont) {
			freemsg(mp->b_cont);
			mp->b_cont = NULL;
		}
		iocp->ioc_count = 0;
		iocp->ioc_error = error;
	} else {
		mp->b_datap->db_type = M_IOCACK;
		iocp->ioc_error = 0;
		iocp->ioc_count = 0;
		if (cmd & IOC_OUT) {
			iocp->ioc_count = IOCPARM_LEN(cmd);
			mp->b_cont->b_wptr = mp->b_cont->b_rptr +
				IOCPARM_LEN(cmd);
		}
	}
}

/*
 * pty_ioctl_comm: 	common ioctl for all the pty drivers
 *
 * The master_flag parameter, if set, indicates that the ioctl came down
 * the master side.  Otherwise, it came down the slave side.
 *
 * Return:
 *
 *	errno on failure
 *
 *	  OR
 *
 *	0, possibly with a return value in *retp.
 *
 * The PTY_READ_LOCK should be held on entry to this routine.
 *
 * (XXX) Users of this routine should be made to cope better with ENOMEM.
 */

PRIVATE_STATIC int
pty_ioctl_comm(
	int *retp,
	register int cmd,
	register caddr_t data,
	register struct pty_s *ptp,
	int master_flag
	)
{
	mblk_t 		*mp;
	int 		error;
	struct termios	t, *termp = &t;
	tcflag_t	compflags, *compflagsp = &compflags;


	PTY_READ_LOCK_ASSERT(ptp);
	error = *retp = 0;

	/*
	 * Verify that master-only ioctl's don't come down the slave.
	 */
	if (!master_flag)
		switch (cmd) {
		case TIOCPKT:
		case TIOCUCNTL:
		case TIOCREMOTE:
		case ISPTM:  
		case old_isptm:
		case ISPTS:  
		case UNLKPT:
		case TIOCSIG:
			return (EINVAL);
		default:
			break;
		}

	switch (cmd) {
/* Master-only ioctls */
	case TIOCPKT:
		simple_lock(&ptp->pty_lock);
		if (*(int *)data) {
			if (ptp->pt_bsdflags & PFB_UCNTL)
				error = EINVAL;
			else
				ptp->pt_bsdflags |= PFB_PKT;
		} else
			ptp->pt_bsdflags &= ~PFB_PKT;
		simple_unlock(&ptp->pty_lock);
		break;
	case TIOCUCNTL:
		simple_lock(&ptp->pty_lock);
		if (*(int *)data) {
			if (ptp->pt_bsdflags & PFB_PKT)
				error = EINVAL;
			else
				ptp->pt_bsdflags |= PFB_UCNTL;
		} else
			ptp->pt_bsdflags &= ~PFB_UCNTL;
		simple_unlock(&ptp->pty_lock);
		break;
	case TIOCREMOTE:
		/* 
		 * Verify that slave is there.
		 */
		if (ptp->pt_srq == 0) {
			error = EINVAL;
			break;
		}

		/*
		 * Now send M_FLUSH up the slave.
		 */
		mp = allocb(1, BPRI_MED);
		if (!mp) {
			error = ENOMEM;
			break;
		}
		mp->b_datap->db_type = M_FLUSH;
		*mp->b_rptr = FLUSHR;
		puthere(ptp->pt_srq, mp);
		if (*(int *)data) {
			mp = pty_ctl(MC_NO_CANON, 0);
			if (!mp) {
				error = ENOMEM;
				break;
			}
			simple_lock(&ptp->pty_lock);
			ptp->pt_flags |= PF_REMOTE;
			simple_unlock(&ptp->pty_lock);
		} else {
			mp = pty_ctl(MC_DO_CANON, 0);
			if (!mp) {
				error = ENOMEM;
				break;
			}
			simple_lock(&ptp->pty_lock);
			ptp->pt_flags &= ~PF_REMOTE;
			simple_unlock(&ptp->pty_lock);
		}
		/*
		 * Now send M_CTL with new setting for line discipline.
		 */
		puthere(ptp->pt_srq, mp);
		break;
	case ISPTM:  
		*retp = (int) ptp->pt_dev; 
		break;
	case old_isptm:
		*(dev_t *)data = ptp->pt_dev;	
		break;
	case ISPTS:  
		*retp = (int) makedev(major(pts_cdev), minor(ptp->pt_dev)); 
		break;
	case UNLKPT:
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags &= ~PF_SLOCK;
		simple_unlock(&ptp->pty_lock);
		break;
	case TIOCSIG:
		/* 
		 * Verify that slave is there.
		 */
		if (ptp->pt_srq == 0) {
			error = EINVAL;
			break;
		}

		/*
		 * TIOCSIG is an IOC_VOID, so we must "validate" ptr here.
		 */
		if (!data) {
			error = EINVAL;
			break;
		}
		if (((int) (*(int *)data)) > NSIG) {
			error = EINVAL;
			break;
		}
		mp = allocb(sizeof(char), BPRI_MED);
		if (!mp) {
			error = ENOMEM;
			break;
		}
		mp->b_datap->db_type = M_PCSIG;
		*mp->b_wptr = *(u_char *)data;
		mp->b_wptr += sizeof(char);
		puthere(ptp->pt_srq, mp);
		break;

/* End master-only ioctls */

	case TIOCGETA:
		/*
		 * fill in values
		 */
		simple_lock(&ptp->pty_lock);
		*(struct termios *)data = ptp->pt_termios;
		simple_unlock(&ptp->pty_lock);
		break;
	case TCSBRK:
		/*
		 * Still need to check data -- no checking is done
		 * on IOC_VOIDs.
		 */
		if (master_flag) {
			if (data && ((int)(*(int *) data)))
				break;
			/* 
			 * Verify that slave is there.
			 */
			if (ptp->pt_srq == 0) {
				error = EINVAL;
				break;
			}
			mp = allocb(0, BPRI_MED);
			if (!mp)
			{
				error = ENOMEM;
				break;
			}
			mp->b_datap->db_type = M_BREAK;
			puthere(ptp->pt_srq, mp);
		}
		break;
	case TIOCSETAF:
	case TIOCSETA:
	case TIOCSETAW: 
	{
		int 		stop;
		cc_t 		*cc;
		mblk_t		*mp;

		termp = (struct termios *)data;
		compflagsp = 0;

seta_postproc:
		/* We may get here from the COMPAT_43 ioctls below,
		 * in which case termp points to a termios on our
		 * local stack (rather than to the "data" arg.),
		 * and compflagsp is non-NULL.
		 */

		/* Fix c_ispeed if zero, per POSIX.
		 */
		if (termp->c_ispeed == 0)
			termp->c_ispeed = termp->c_ospeed;

		/* Allocate an mblk ahead of time, if we'll need one.
		 */
		if (master_flag) {
			if (ptp->pt_srq) {
				mp = pty_ctl(TIOCSETA, sizeof(struct termios));
				if (!mp) {
					error = ENOMEM;
					break;
				}
			}
		} else {
			/*
			 * If speed is to be set to zero, allocate a zero 
			 * length M_DATA msg to send to the master.
			 */
			if ((termp->c_ospeed == 0) && ptp->pt_mrq) {
				mp = allocb(0, BPRI_MED);
				if (!mp) {
					error = ENOMEM;
					break;
				}
			}
		}

		simple_lock(&ptp->pty_lock);
		ptp->pt_termios = *termp;
		if (compflagsp)
			ptp->pt_compatflags = *compflagsp;

		cc = &ptp->pt_cc[0];
		ptp->pt_iflag |= IXOFF;

		/*
		 * START/STOP characters or IXON might have changed
		 * Set appropriate packet mode flags
		 */

		stop = (ptp->pt_iflag & IXON) &&
			CCEQ(cc[VSTOP], CTRL('s')) &&
			CCEQ(cc[VSTART], CTRL('q'));
		if (ptp->pt_bsdflags & PFB_NOSTOP) {
			if (stop) {
				ptp->pt_send &= ~TIOCPKT_NOSTOP;
				ptp->pt_send |= TIOCPKT_DOSTOP;
				ptp->pt_bsdflags &= ~PFB_NOSTOP;
				simple_unlock(&ptp->pty_lock);
				if (ptp->pt_enable)
					(*ptp->pt_enable)(ptp, FREAD);
				simple_lock(&ptp->pty_lock);
			}
		} else {
			if (!stop) {
				ptp->pt_send &= ~TIOCPKT_DOSTOP;
				ptp->pt_send |= TIOCPKT_NOSTOP;
				ptp->pt_bsdflags |= PFB_NOSTOP;
				simple_unlock(&ptp->pty_lock);
				if (ptp->pt_enable)
					(*ptp->pt_enable)(ptp, FREAD);
				simple_lock(&ptp->pty_lock);
			}
		}

		/* Assert: simple lock is held */
		if (master_flag) {
			if (ptp->pt_srq) {
				bcopy(&ptp->pt_termios, mp->b_cont->b_rptr, sizeof(struct termios));
				simple_unlock(&ptp->pty_lock);
				mp->b_cont->b_wptr = mp->b_cont->b_rptr +
					sizeof(struct termios);
				puthere(ptp->pt_srq, mp);
			} else
				simple_unlock(&ptp->pty_lock);
		} else  {
			/*
			 * If speed was set to zero, send a zero length 
			 * M_DATA msg (allocated above) to the master.
			 */
			if ((ptp->pt_ospeed == 0) && ptp->pt_mrq) {
				simple_unlock(&ptp->pty_lock);
				if (!((*ptp->pt_xfer)(ptp, mp)))
					freemsg(mp);
			} else
				simple_unlock(&ptp->pty_lock);
		}
		break;
	}
	case TIOCEXCL:
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags |= PF_XCLUDE;
		simple_unlock(&ptp->pty_lock);
		break;
	case TIOCNXCL:
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags &= ~PF_XCLUDE;
		simple_unlock(&ptp->pty_lock);
		break;
	case TIOCFLUSH:
		break;
	case TIOCGWINSZ:
		simple_lock(&ptp->pty_lock);
		if (!ptp->pt_wnsz.ws_col && (ptp->pt_flags & PF_SV)) {
			/* See V.4 Streams Programmer's Guide, p. 12-18.
			 */
			error = EINVAL;
		} else
			bcopy(&ptp->pt_wnsz, data, sizeof(struct winsize));
		simple_unlock(&ptp->pty_lock);
		break;
	case TIOCSWINSZ:
		simple_lock(&ptp->pty_lock);
		ptp->pt_wnsz = *(struct winsize *)data;
		simple_unlock(&ptp->pty_lock);
		if (master_flag && ptp->pt_srq) {
			mp = pty_ctl(TIOCSWINSZ, sizeof(struct winsize));
			if (!mp) {
				error = ENOMEM;
				break;
			}
			bcopy(data, mp->b_cont->b_rptr, sizeof(struct winsize));
			mp->b_cont->b_wptr = mp->b_cont->b_rptr +
				sizeof(struct winsize);
			puthere(ptp->pt_srq, mp);
		}
		break;
	case TIOCHPCL:
	case FIONBIO:
		break;
	case EUC_WSET:
	case EUC_WGET:
	case EUC_IXLON:
	case EUC_IXLOFF:
	case EUC_OXLON:
	case EUC_OXLOFF:
	case EUC_MSAVE:
	case EUC_MREST:
	case TIOCCONS:
		if (master_flag)
			error = EINVAL;
		break;
#ifdef COMPAT_43
	/* The line discipline converts most of these to "normal"
	 * (i.e., termios-style) ioctls on the way down, but
	 * a few applications send these down the master side,
	 * so we must process them here.
	 */
	case TIOCGETP:
	case TIOCSETP:
	case TIOCSETN:
	case TIOCGETC:
	case TIOCSETC:
	case TIOCSLTC:
	case TIOCGLTC:
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
	case TIOCLGET:
	case OTIOCGETD:
	case OTIOCSETD:
		*termp = ptp->pt_termios;
		*compflagsp = ptp->pt_compatflags;
		if (pty_bsd43_ioctl(cmd, data, termp, compflagsp, master_flag))
			goto seta_postproc;
		break;
#endif /* COMPAT_43 */
	default:
		error = EINVAL;
		break;
	}
	return (error);
}

/*
 * build the specified M_CTL message
 */
PRIVATE_STATIC mblk_t *
pty_ctl(register int command, register int size)
{
	register mblk_t *mp;
	register mblk_t *mp1;
	register struct iocblk *iocp;

	mp = allocb(sizeof(struct iocblk), BPRI_MED);
	if (!mp)
		return(0);
	if (size) {
		mp1 = allocb(size, BPRI_MED);
		if (!mp1) {
			freemsg(mp);
			return(0);
		}
	} else
		mp1 = 0;
	mp->b_datap->db_type = M_CTL;
	mp->b_cont = mp1;
	mp->b_wptr = mp->b_rptr + sizeof(struct iocblk);
	iocp = (struct iocblk *)mp->b_rptr;
	iocp->ioc_cmd = command;
	iocp->ioc_count = size;
	iocp->ioc_error = iocp->ioc_rval = 0;
	return(mp);
}

/*
 * BSD master side
 */

int
ptropen(dev_t dev, 
	int flag, 
	int cflag, 
	dev_t * newdevp,
	struct ucred *cred,
	void **private
	)
{
	register struct pty_s *ptp;
	register struct ptque *ptqp;
	struct que_hd *que;
	mblk_t *mp;
	register unsigned int dev_minor;
	int error;

	PTY_OPCL_LOCK();

	if (minor(*newdevp) >= nptys) {
		if (!(flag & O_DOCLONE)) { 
			PTY_OPCL_UNLOCK();
			return ENXIO;
		}

		*newdevp = makedev(major(*newdevp), nptys - 1);
	}

	error = cdevsw_open_ocomm(pts_cdev, flag, newdevp, 
				  sizeof(struct pty_s), private);
	if (error) {
		PTY_OPCL_UNLOCK();
		return(error);
	} else {
#if MACH_ASSERT
		if (*private == 0)
			panic("ptropen: unexpected null pointer");
#endif
		ptp = (struct pty_s *) *private;
		if ((ptp->pt_alloc & PFA_ALLOC) == 0)
			pty_init(ptp);
		ptp->pt_alloc |= PFA_MASTER_ALLOC;
		PTY_OPCL_UNLOCK();
	}

	PTY_READ_LOCK(ptp);

	/*
	 * Quick check that neither master nor slave is open.  Done
	 * here with just a read lock so that programs that blindly cycle
	 * through /dev/pty[pqrs][0-9a-f] don't need to take a write lock
	 * on every one.
	 */ 

	if (ptp->pt_mrq || ptp->pt_srq) {
		PTY_READ_UNLOCK(ptp);
		return(EIO);
	}

	PTY_RD2WR_LOCK(ptp);

	/*
	 * Check that nothing snuck in during the upgrade.
	 */ 

	if (ptp->pt_mrq || ptp->pt_srq) {
		PTY_WRITE_UNLOCK(ptp);
		return(EIO);
	}


	/*
	 * Initialize master fields.  We start with bzero'd memory,
	 * so fields that should be 0 are omitted here.
	 */
	
	ptqp = &ptp->pt_ptque;
	ptqp->ptq_hiwater = BSD_MAST_HIWATER;
	ptqp->ptq_lowater = BSD_MAST_LOWATER;

	ptp->pt_mrq = (caddr_t)ptqp;
	ptp->pt_xfer = ptrxfer;
	ptp->pt_enable = ptrenable;
	ptp->pt_mdrained = ptrmdrained;
	ptp->pt_dev = *newdevp;

	que = (struct que_hd *)ptp->pt_mrq;
	ptque_init(que);
	que = (struct que_hd *)&ptp->pt_selqhd;
	ptque_init(que);

	/*
	 * Set master open and carrier on to sync with slave.
	 */

	simple_lock(&ptp->pty_lock);
	ptp->pt_flags |= (PF_MOPEN|PF_CARR_ON);
	simple_unlock(&ptp->pty_lock);

	PTY_WRITE_UNLOCK(ptp);

	pty_wakeup((caddr_t) &ptp->pt_mwq);

	return(0);
}

int
ptrclose(
	dev_t 		dev,
	int     	flag,
	int     	mode,   /* unused */
	struct ucred 	*cred,
	void    	*private
	)
{
	register struct pty_s * ptp;
	mblk_t *mp, *bye_msg;
		        
	ptp = (struct pty_s *) private;

	/* Allocate hangup message */
	bye_msg = allocb(0, BPRI_WAITOK);
	bye_msg->b_datap->db_type = M_HANGUP;

	PTY_WRITE_LOCK(ptp);
	while ((mp = (mblk_t *)dequeue_head((struct que_hd *)ptp->pt_mrq)) != 0)
		freemsg(mp);
	ptp->pt_mrq = ptp->pt_mwq = NULL;

	simple_lock(&ptp->pty_lock);
	if (ptp->pt_srq) {
		ptp->pt_flags &= ~(PF_MOPEN|PF_CARR_ON|PF_MFLOW);
		if ((ptp->pt_flags & PF_SFLOW) || (ptp->pt_flags & PF_WDRAIN)) {
			ptp->pt_flags &= ~PF_SFLOW;
			simple_unlock(&ptp->pty_lock);
			qenable(WR(ptp->pt_srq));
			pty_wakeup((caddr_t) WR(ptp->pt_srq));
		} else
			simple_unlock(&ptp->pty_lock);
		PTY_WR2RD_LOCK(ptp);
		putq(ptp->pt_srq, bye_msg);
		PTY_READ_UNLOCK(ptp);
	} else {
		ptp->pt_flags = 0;
		simple_unlock(&ptp->pty_lock);
		PTY_WRITE_UNLOCK(ptp);
		freemsg(bye_msg);
	}

	PTY_OPCL_LOCK();
	ptp->pt_alloc &= ~PFA_MASTER_ALLOC;
	if ((ptp->pt_alloc & PFA_ALLOC) == 0)
		(void) cdevsw_close_comm(ptp);
	PTY_OPCL_UNLOCK();
	return 0;
}

/* A routine to tell slave if there's unread data in the master's 
 * read queue.  The slave uses this to suspend close processing until
 * output (which includes master input) has drained.
 *
 * Simple lock and read lock are both held on entry.
 */

PRIVATE_STATIC int
ptrmdrained(struct pty_s *ptp)
{
	struct ptque	*ptqp = (struct ptque *)ptp->pt_mrq;
	
	return (!(ptqp->ptq_count));
}

int
ptrselect(dev_t dev, short *events, short *revents, int scanning, void *private)
{
	register struct pty_s 	*ptp;
	register struct ptque 	*ptqp;
	int			rtaken;

	ptp = (struct pty_s *) private;
	ptqp = (struct ptque *)ptp->pt_mrq;

	if (scanning && (*events & POLLOUT)) {
		PTY_READ_LOCK(ptp);
		rtaken = 1;
	} else
		rtaken = 0;

	simple_lock(&ptp->pty_lock);

	if (!scanning) {
		select_dequeue(&ptp->pt_selqhd);
		goto out;
	}

	if (*events & POLLNORM) {
		if ((ptp->pt_flags & PF_TTSTOP) == 0 && ptqp->ptq_count)
			*revents |= POLLNORM;
	}

	if (*events & POLLPRI) {
		if (((ptp->pt_bsdflags & PFB_PKT) && ptp->pt_send) ||
		    ((ptp->pt_bsdflags & PFB_UCNTL) && ptp->pt_ucntl)) {
			*revents |= POLLPRI;
			if (*events & POLLNORM)
				*revents |= POLLNORM;
		}
	}

	if ((ptp->pt_flags & PF_CARR_ON) == 0) {
		*revents |= POLLHUP;
		if (*events & POLLNORM)
			*revents |= POLLNORM;
		goto out;
	}

	if (*events & POLLOUT) {
		if ( !(ptp->pt_flags & PF_TTINSTOP) && ptp->pt_srq
		     && canput(ptp->pt_srq) )
			*revents |= POLLOUT;
	}

	if (*revents == 0)
		select_enqueue(&ptp->pt_selqhd);
out:
	simple_unlock(&ptp->pty_lock);
	if (rtaken)
		PTY_READ_UNLOCK(ptp);
	return(0);	                
}

int
ptrread(dev_t dev, struct uio *uio, int flag, void *private)
{
	register struct pty_s *ptp;
	register struct ptque *ptqp;
	mblk_t *mp, *mp1;
	int error = 0, cc, msgsz, tot_msgsz;

	ptp = (struct pty_s *) private;
	ptqp = (struct ptque *)ptp->pt_mrq;

	/*
	 * Continue to transfer data to user space until read is 
	 * satisfied or Packet/User control event is received.
	 */

	PTY_READ_LOCK(ptp);
	for(;;) {
		simple_lock(&ptp->pty_lock);
		if ((ptp->pt_bsdflags & PFB_PKT) && ptp->pt_send) {
			int	c = ptp->pt_send;

			simple_unlock(&ptp->pty_lock);
			if ((error = ureadc(c, uio)) == 0) {
				simple_lock(&ptp->pty_lock);
				ptp->pt_send = 0;
				simple_unlock(&ptp->pty_lock);
			}
			goto out;
		}
		
		if ((ptp->pt_bsdflags & PFB_UCNTL) && ptp->pt_ucntl) {
			int	c = ptp->pt_ucntl;

			simple_unlock(&ptp->pty_lock);
			if ((error = ureadc(c, uio)) == 0) {
				simple_lock(&ptp->pty_lock);
				ptp->pt_ucntl = 0;
				simple_unlock(&ptp->pty_lock);
			}
			goto out;
		}
		if (ptqp->ptq_count && (ptp->pt_flags & PF_TTSTOP) == 0)
			break;

		/* Simple lock is held */
		if ((ptp->pt_flags & PF_CARR_ON) == 0) {
			simple_unlock(&ptp->pty_lock);
			PTY_READ_UNLOCK(ptp);
			return (EIO);
		} else
			simple_unlock(&ptp->pty_lock);
		if (flag & (IO_NDELAY|IO_NONBLOCK|O_NONBLOCK)) {
			PTY_READ_UNLOCK(ptp);
			return(EWOULDBLOCK);
		}
		assert_wait((vm_offset_t) &ptp->pt_mrq, TRUE);
		PTY_READ_UNLOCK(ptp);
		error = tsleep((caddr_t)0, PTYPRI | PCATCH, (char *)ptyin, 0);
		if (error)
			return (error);
		PTY_READ_LOCK(ptp);
	} /* end of for loop  */
	/* Simple lock is held */

	/*
	 * If no slave pt_send or pt_ucntl, transfer a null byte.
	 */
	if (ptp->pt_bsdflags & (PFB_PKT|PFB_UCNTL)) {
		simple_unlock(&ptp->pty_lock);
		if (error = ureadc(0, uio))
			goto out;
		simple_lock(&ptp->pty_lock);
	}

	/*
	 * Transfer to user space for requested read size.
	 * If flow controlled, check low water mark.
	 */

	while (uio->uio_resid) {
		mp = (mblk_t *)dequeue_head(ptqp);
		if (!mp) {
			break;
		}
		tot_msgsz = msgdsize(mp);
		ptqp->ptq_count -= tot_msgsz;

		simple_unlock(&ptp->pty_lock);

		while (tot_msgsz) {
			mp1 = unlinkb(mp);
			msgsz = mp->b_wptr - mp->b_rptr;
			cc = MIN(uio->uio_resid, msgsz);
			uio->uio_rw = UIO_READ;
			error = uiomove((caddr_t)mp->b_rptr, cc, uio);
			if (error) {
				freemsg(mp);
				if (mp1)
					freemsg(mp1);
				break;
			}
			if (cc < msgsz) {
				if (mp1)
					linkb(mp, mp1);
				mp->b_rptr += cc;
				msgsz = msgdsize(mp);
				simple_lock(&ptp->pty_lock);
				ptqp->ptq_count += msgsz;
				enqueue_head((struct que_hd *)ptp->pt_mrq, mp);
				simple_unlock(&ptp->pty_lock);
				break;
			}
			freemsg(mp);
			mp = mp1;
			tot_msgsz -= cc;
		}

		simple_lock(&ptp->pty_lock);

	} /* end of while */

	/* Simple lock is held */

	/*
	 * If slave output is flow controlled and current byte count 
	 * of msgs on master read queue < LOWATER, then qenable the 
	 * slave write queue and clear write flow flag.
	 */

	if ((ptp->pt_flags & PF_SFLOW) || (ptp->pt_flags & PF_WDRAIN)) { 
		ASSERT(ptp->pt_srq);	/* SFLOW and WDRAIN imply this */
		if (ptqp->ptq_count < BSD_MAST_LOWATER) {
			ptp->pt_flags &= ~PF_SFLOW;
			simple_unlock(&ptp->pty_lock);
			qenable(WR(ptp->pt_srq));
			pty_wakeup((caddr_t) WR(ptp->pt_srq));
		} else
			simple_unlock(&ptp->pty_lock);
	} else
		simple_unlock(&ptp->pty_lock);

out:
	PTY_READ_UNLOCK(ptp);
	return(error);
}

/*
 * The ptrxfer routine is responsible for moving data to the BSD master. 
 * If the passed mp is an M_DATA type a return value of 1 indicates the 
 * message was enqueued to the master.  A return value of zero indicates
 * a flow control situation.  The return value is always zero for messages
 * other than M_DATA.
 *
 * This routine should only be called on a pty whose master and slave are 
 * both open.  The PTY_READ_LOCK should be held on entry.  
 */

PRIVATE_STATIC int
ptrxfer(register struct pty_s *ptp, mblk_t *mp)
{
	register struct ptque *ptqp;
	mblk_t *f_mp;
	int ret_val = 0;

	PTY_READ_LOCK_ASSERT(ptp);
	ptqp = (struct ptque *)ptp->pt_mrq;

#if MACH_ASSERT
	if (!(ptp->pt_mrq && ptp->pt_srq)) {
		/* Callers are expected to check before calling */
		panic("ptrxfer: device not open");
	}
#endif

	simple_lock(&ptp->pty_lock);
	/* PTY_LOCK and simple lock are both held */

	switch(mp->b_datap->db_type) {
		case M_DATA:
			if (ptqp->ptq_count >= ptqp->ptq_hiwater) {
				ptp->pt_flags |= PF_SFLOW;
				simple_unlock(&ptp->pty_lock);	
				break;
			}
			enqueue_tail(ptqp, mp);
			ptqp->ptq_count += msgdsize(mp);
			if (ptp->pt_bsdflags & PFB_STOPPED) {
				ptp->pt_bsdflags &= ~PFB_STOPPED;
				ptp->pt_send = TIOCPKT_START;
			}
			select_wakeup(&ptp->pt_selqhd);
			simple_unlock(&ptp->pty_lock);
			pty_wakeup((caddr_t) &ptp->pt_mrq);
			ret_val = 1;
			break;
		case M_FLUSH: 
		{
			int flag = 0;

			if (*mp->b_rptr & FLUSHW) {
				flag |= FREAD;
				while (f_mp = (mblk_t *)dequeue_head(ptqp)) {
					simple_unlock(&ptp->pty_lock);
					freemsg(f_mp);
					simple_lock(&ptp->pty_lock);
				}
				ptqp->ptq_count = 0;
				ptp->pt_bsdflags &= ~PFB_STOPPED;

				ptp->pt_send = *mp->b_rptr;

				if (ptp->pt_flags & PF_SFLOW) {
					ptp->pt_flags &= ~PF_SFLOW;
					simple_unlock(&ptp->pty_lock);
					qenable(WR(ptp->pt_srq));
					simple_lock(&ptp->pty_lock);
				}
				simple_unlock(&ptp->pty_lock);
				*mp->b_rptr &= ~FLUSHW;
			} else
				simple_unlock(&ptp->pty_lock);

			if (*mp->b_rptr & FLUSHR) {
				flag |= FWRITE;
				puthere(ptp->pt_srq, mp);
			} else
				freemsg(mp);
			ptrenable(ptp, flag);
			break;
		}
		case M_STOP:
			ptp->pt_bsdflags |= PFB_STOPPED;
			simple_unlock(&ptp->pty_lock);
			freemsg(mp);
			break;
		case M_START:	
			if (ptp->pt_bsdflags & PFB_STOPPED) {
				ptp->pt_bsdflags &= ~PFB_STOPPED;
				ptp->pt_send = TIOCPKT_START;
			}
			select_wakeup(&ptp->pt_selqhd);
			simple_unlock(&ptp->pty_lock);
			freemsg(mp);
			break;
		case M_STARTI:
			select_wakeup(&ptp->pt_selqhd);
			simple_unlock(&ptp->pty_lock);
			freemsg(mp);
			pty_wakeup((caddr_t) &ptp->pt_mwq);
			break;
		case M_STOPI:
		default:
			simple_unlock(&ptp->pty_lock);
			freemsg(mp);
			break;
	}
	return(ret_val);
}

/*
 * ptrenable: start up input and/or output on master.  Return silently
 * if master is not open.
 *
 * READ_LOCK or WRITE_LOCK should be held on entry to this routine.
 */

PRIVATE_STATIC void
ptrenable(register struct pty_s *ptp, int flag)
{	
	PTY_ANY_LOCK_ASSERT(ptp);
	if (ptp->pt_mrq == 0)
		return;
	else {
		simple_lock(&ptp->pty_lock);
		select_wakeup(&ptp->pt_selqhd);
		simple_unlock(&ptp->pty_lock);

		if (flag & FREAD)
			pty_wakeup((caddr_t) &ptp->pt_mrq);
		if (flag & FWRITE)
			pty_wakeup((caddr_t) &ptp->pt_mwq);

		return;
	}
}

int
ptrwrite(dev_t dev, register struct uio *uio, int flag, void *private)
{
	register struct pty_s *ptp;
	register struct iovec *iov;
	mblk_t *mp;
	int error, cc, ret_val = 0, cnt = 0;

	ptp = (struct pty_s *) private;
	PTY_READ_LOCK(ptp);
	for (;;)  {

		simple_lock(&ptp->pty_lock);

		while ( (ptp->pt_srq) && (uio->uio_iovcnt > 0)
			&& (canput(ptp->pt_srq)) 
			&& ((ptp->pt_flags & PF_TTINSTOP)==0)) {

			iov = uio->uio_iov;
			if (iov->iov_len == 0) {
				uio->uio_iovcnt--;
				uio->uio_iov++;
				continue;
			}

			simple_unlock(&ptp->pty_lock);
			PTY_READ_UNLOCK(ptp);

			cc = MIN(iov->iov_len, STRMSGSZ);

			mp = allocb(cc, BPRI_WAITOK);
			PTY_READ_LOCK(ptp);

			/*
			 * Things may have changed while we were waiting
			 * for allocb() to return.
			 */
			if ( !((ptp->pt_srq) && (canput(ptp->pt_srq))
			    && ((ptp->pt_flags & PF_TTINSTOP)==0)) ) {
				freemsg(mp);
				simple_lock(&ptp->pty_lock);
				break;
			}

			error = uiomove((caddr_t)mp->b_wptr, cc, uio);
			if (error) {
				freemsg(mp);
				PTY_READ_UNLOCK(ptp);
				return(error);
			}
			mp->b_wptr = mp->b_rptr + cc;
			puthere(ptp->pt_srq, mp);
			cnt++;
			simple_lock(&ptp->pty_lock);
		} /* end of "slave open - more data - flow ok" while */

		/* Simple lock is held */

		if ((ptp->pt_flags & PF_CARR_ON) == 0) {
			simple_unlock(&ptp->pty_lock);
			ret_val = EIO;
			break;
		}

		if (!uio->uio_iovcnt) {
			/* no more data -- ESUCCESS */
			simple_unlock(&ptp->pty_lock);
			break;
		}

		if ((flag & (IO_NDELAY|IO_NONBLOCK|O_NONBLOCK)) || (ptp->pt_flags & PF_NBIO)){
			/* Return without blocking */
			simple_unlock(&ptp->pty_lock);
			if (!cnt)
				ret_val = EWOULDBLOCK;
			break;
		}

		ptp->pt_flags |= PF_MFLOW;
		simple_unlock(&ptp->pty_lock);
		assert_wait((vm_offset_t)&ptp->pt_mwq, TRUE);
		PTY_READ_UNLOCK(ptp);
		if (error = tsleep((caddr_t)0, PTYPRI | PCATCH, ptyout, 0))
			return(error);
		PTY_READ_LOCK(ptp);

		/* Slave may have closed while we were asleep (the slave
		 * close code would have woken us up) -- this is checked 
		 * above.
		 */

	} /* end of forever loop */
	PTY_READ_UNLOCK(ptp);
	return(ret_val);
}

int
ptrioctl(
	dev_t           dev,    /* major + minor device number */
	unsigned int    cmd,    /* cmd argument to ioctl system call */
	caddr_t         data,   /* *pointer* to 3rd user argument */
	int             flag,   /* f_flag from file structure */
	struct ucred    *cred,
	void            *private,
	int             *retval
	)
{
	register struct pty_s *ptp;
	struct ptque *ptqp;
	int error, stop;

	/* Handle a couple of cases originating in fcntl */
	if (cmd == FIOASYNC) {
		/* Turning it _on_ is unsupported, for now. */
		return(*(int *)data ? EINVAL : 0);
	}

	ptp = (struct pty_s *) private;
	PTY_READ_LOCK(ptp);
	/*
    	 * mimic the processing of BSD clist master
	 */
	if (cmd == FIONBIO) {
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags |= PF_NBIO;
		simple_unlock(&ptp->pty_lock);
		PTY_READ_UNLOCK(ptp);
		return(ESUCCESS);
	}
	/*
         * handle TIOCOUTQ here for BSD compatibility
	 */
	if (cmd == TIOCOUTQ){
		ptqp = (struct ptque *)ptp->pt_mrq;
		*(int *)data = ptqp->ptq_count;
		PTY_READ_UNLOCK(ptp);
		return (ESUCCESS);
	}
	error =  pty_ioctl_comm(retval, cmd, data, ptp, 1);
	if (error) {
		if (ptp->pt_bsdflags & PFB_UCNTL &&
		   (cmd & ~0xff) == UIOCCMD(0)) {
			if (cmd & 0xff) {
				simple_lock(&ptp->pty_lock);
				ptp->pt_ucntl = (u_char)cmd;
				simple_unlock(&ptp->pty_lock);
				ptrenable(ptp, FREAD);
			}
			PTY_READ_UNLOCK(ptp);
			return(0);
		}
		error = ENOTTY;
	}
	PTY_READ_UNLOCK(ptp);
	return(error);
}

PRIVATE_STATIC int
ptm_open(queue_t *q, dev_t *devp, int flag, int sflag, cred_t *credp)
{
	register int dev;
	mblk_t *mp;
	register struct pty_s *ptp;
	int error;

	PTY_OPCL_LOCK();

	if (minor(*devp) >= nptys) {
		if (!(flag & O_DOCLONE)) { 
			PTY_OPCL_UNLOCK();
			return ENXIO;
		}

		*devp = makedev(major(*devp), nptys - 1);
	}

	error = streams_open_ocomm(pts_cdev, sizeof(struct pty_s), q, 
				   devp, flag, sflag, credp);
	if (error) {
		PTY_OPCL_UNLOCK();
		return(error);
	} else {
#if MACH_ASSERT
		if (q->q_ptr == 0)
			panic("ptm_open: unexpected null pointer");
#endif
		ptp = (struct pty_s *) q->q_ptr;
		if ((ptp->pt_alloc & PFA_ALLOC) == 0)
			pty_init(ptp);
		ptp->pt_alloc |= PFA_MASTER_ALLOC;
		PTY_OPCL_UNLOCK();
	}

	PTY_READ_LOCK(ptp);

	/*
	 * Ensure that neither master nor slave is open -- see comment
	 * in ptropen().
	 */ 

	if (ptp->pt_mrq || ptp->pt_srq) {
		PTY_READ_UNLOCK(ptp);
		return(EIO);
	}

	PTY_RD2WR_LOCK(ptp);

	if (ptp->pt_mrq || ptp->pt_srq) {
		PTY_WRITE_UNLOCK(ptp);
		return(EIO);
	}

	simple_lock(&ptp->pty_lock);
	ptp->pt_flags |= (PF_MOPEN|PF_CARR_ON|PF_SLOCK|PF_SV);
	simple_unlock(&ptp->pty_lock);

	ptp->pt_dev = *devp;
	ptp->pt_xfer = ptm_xfer;
	ptp->pt_enable = ptm_enable;
	ptp->pt_mdrained = ptm_mdrained;
	ptp->pt_mrq = (caddr_t)q;
	ptp->pt_mwq = (caddr_t)WR(q);
	PTY_WRITE_UNLOCK(ptp);

	return (0);
}

PRIVATE_STATIC int
ptm_close(queue_t *q, int flag, cred_t *credp)
{
	register struct pty_s *ptp;	
	mblk_t *bye_msg;

	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);

	bye_msg = allocb(0, BPRI_WAITOK);
	bye_msg->b_datap->db_type = M_HANGUP;

	PTY_WRITE_LOCK(ptp);
	ptp->pt_mwq = ptp->pt_mrq = (caddr_t)NULL;
	simple_lock(&ptp->pty_lock);
	if (ptp->pt_srq) {
		ptp->pt_flags &= ~(PF_MOPEN|PF_CARR_ON|PF_SLOCK|PF_MFLOW);
		if ((ptp->pt_flags & PF_SFLOW) || (ptp->pt_flags & PF_WDRAIN)) {
			ptp->pt_flags &= ~PF_SFLOW;
			simple_unlock(&ptp->pty_lock);
			qenable(WR(ptp->pt_srq));
			pty_wakeup((caddr_t) WR(ptp->pt_srq));
		} else
			simple_unlock(&ptp->pty_lock);
		PTY_WR2RD_LOCK(ptp);
		putq(ptp->pt_srq, bye_msg);
		PTY_READ_UNLOCK(ptp);
	} else {
		ptp->pt_flags = 0;
		simple_unlock(&ptp->pty_lock);
		PTY_WRITE_UNLOCK(ptp);
		freemsg(bye_msg);
	}
	PTY_OPCL_LOCK();
	ptp->pt_alloc &= ~PFA_MASTER_ALLOC;
	if ((ptp->pt_alloc & PFA_ALLOC) == 0)
		(void) streams_close_comm(q, flag, credp);
	PTY_OPCL_UNLOCK();
	return 0;
}

PRIVATE_STATIC int
ptm_mdrained(struct pty_s *ptp)
{
	return 1;    /* mdrained vector a NOP for SV master */
}

PRIVATE_STATIC int
ptm_wput(queue_t *q, mblk_t *mp)
{
	register struct pty_s *ptp;
	int ret_val, cmd;
	u_char l_flush;
	struct iocblk *iocp;
	caddr_t data;

	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);
	PTY_READ_LOCK(ptp);
	switch(mp->b_datap->db_type) {
	case M_IOCTL:
		if (strtty_ioctlbad(mp)) {
			PTY_READ_UNLOCK(ptp);
			qreply(q, mp);
			goto out;
		}
		pty_strioctl_comm(mp, ptp, 1);
		PTY_READ_UNLOCK(ptp);
		qreply(q, mp);
		goto out;
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHW)
			flushq(q, FLUSHDATA);
		if (ptp->pt_srq) {
			flush_switch_flags(mp->b_rptr);
			puthere(ptp->pt_srq, mp);
		} else
			if (*mp->b_rptr & FLUSHR) {
				*mp->b_rptr &= ~FLUSHW;
				flushq(q->q_other, FLUSHDATA);
				qreply(q, mp);
			} else
				freemsg(mp);
		break;
	case M_DATA:
		if (q->q_first || !ptm_write_data(ptp, mp))
			putq(q, mp);
		break;
	default:
		/*
		 * don't expect other msgs 
		 */	
#if	MACH_ASSERT
		printf("pts_wput received a %x msg\n", mp->b_datap->db_type);
#endif
		freemsg(mp);
	}
	PTY_READ_UNLOCK(ptp);
out:
	return;
}

PRIVATE_STATIC int
ptm_wsrv(queue_t *q)
{
	register struct pty_s *ptp;
	register mblk_t *mp;

	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);
	PTY_READ_LOCK(ptp);
	while (mp =  getq(q)) {

		if (mp->b_datap->db_type != M_DATA) {
#if MACH_ASSERT
			printf("ptm_wput: message 0x%x\n", mp);
			panic("ptm_wput: unexpected message");
#endif
			freemsg(mp);
			continue;
		}

		if (!ptm_write_data(ptp, mp)) {
			/* Master output is flow controlled:
			 * we will be qenabled by slave later.
			 */
			putbq(q, mp);
			PTY_READ_UNLOCK(ptp);
			return;
		}
	}

	PTY_READ_UNLOCK(ptp);
}

/* ptm_write_data:
 *
 *	Process an M_DATA message on the master's write side.  A return
 *	value of 1 indicates that something reasonable was done with the
 *	message; a return value of 0 tells the caller to putq()
 *	or putbq() (as appropriate) the message for later processing
 *	by the service routine.
 *
 *	The PTY_READ_LOCK should be held on entry to this routine.
 */

PRIVATE_STATIC int
ptm_write_data(register struct pty_s *ptp, register mblk_t *mp)
{
	PTY_READ_LOCK_ASSERT(ptp);
	ASSERT(mp->b_datap->db_type == M_DATA);

	/* PTY_READ_LOCK is held */

	if (ptp->pt_srq == 0) {
		freemsg(mp);
		return(1);
	}

	simple_lock(&ptp->pty_lock);
	if (ptp->pt_flags & PF_TTINSTOP) {
		simple_unlock(&ptp->pty_lock);
		return(0);
	}

	simple_unlock(&ptp->pty_lock);

	if (!canput(ptp->pt_srq)) {
		simple_lock(&ptp->pty_lock);
		ptp->pt_flags |= PF_MFLOW;
		simple_unlock(&ptp->pty_lock);
		return(0);
	}

	puthere(ptp->pt_srq, mp);
	return(1);
}

/*
 * The PTY_READ_LOCK is held on entry to this routine.
 */

PRIVATE_STATIC int
ptm_rput(queue_t *q, mblk_t *mp)
{
	switch(mp->b_datap->db_type) {
	case M_FLUSH:
		if (*mp->b_rptr & FLUSHR) 
			flushq(q, FLUSHDATA);
		/* Flushing the master write queue, if necessary, will
		 * happen after this message has bounced off the master 
		 * stream head.
		 */
		putnext(q, mp);
		break;
	default:
		if ( (mp->b_datap->db_type >= QPCTL)
		     || (!q->q_first && canput(q->q_next)) )
			putnext(q, mp);
		else
			putq(q, mp);
		break;
	}
}

PRIVATE_STATIC int
ptm_rsrv(queue_t *q)
{
	register struct pty_s *ptp;
	register mblk_t *mp;

	ptp = (struct pty_s *)q->q_ptr;
	ASSERT(ptp != NULL);
	while (mp = getq(q)) {
		if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
			putnext(q, mp);
		else {
			putbq(q, mp);
			return;
		}
		PTY_READ_LOCK(ptp);
		simple_lock(&ptp->pty_lock);
		if (ptp->pt_flags & PF_SFLOW) {

			/* Slave close is expected to clear PF_SFLOW */
			ASSERT(ptp->pt_srq);

			if (q->q_count <= q->q_lowat) {
				ptp->pt_flags &= ~PF_SFLOW;
				simple_unlock(&ptp->pty_lock);
				/* We know slave is open per assertion above.
				 */
				qenable(WR(ptp->pt_srq));
				pty_wakeup((caddr_t) ptp->pt_srq->q_other);
			} else
				simple_unlock(&ptp->pty_lock);
		} else
			simple_unlock(&ptp->pty_lock);
		PTY_READ_UNLOCK(ptp);
	}
}

/*
 * The ptm_xfer routine is responsible for moving data to the System V
 * master. If the passed mp is an M_DATA type a return value of 1 indicates
 * the message was enqueued to the master.  A return value of zero indicates
 * a flow control situation.  The return value is always zero for messages
 * other than M_DATA.
 *
 * This routine should only be called on a pty whose master and slave are 
 * both open.  The PTY_READ_LOCK should be held on entry.  
 */

PRIVATE_STATIC int
ptm_xfer(register struct pty_s *ptp, register mblk_t *mp)
{
	u_char l_flush;
	queue_t *q;

	PTY_READ_LOCK_ASSERT(ptp);
#if MACH_ASSERT
	if (!(ptp->pt_mrq && ptp->pt_srq))
		panic("ptm_xfer: device not open");
#endif

	/* simple lock is NOT held */

	switch(mp->b_datap->db_type) {
	case M_DATA:
		q = (queue_t *)ptp->pt_mrq;
		if (canput(q)) {
			puthere(q, mp);
			return(1);
		} else {
			simple_lock(&ptp->pty_lock);
			ptp->pt_flags |= PF_SFLOW;
			simple_unlock(&ptp->pty_lock);
			return(0);
		}
	case M_FLUSH:
		flush_switch_flags(mp->b_rptr);
		puthere((queue_t *)ptp->pt_mrq, mp);
		break;
	case M_START:	
		freemsg(mp);
		ptm_enable(ptp, FWRITE);
		break;
	case M_STARTI:
		freemsg(mp);
		ptm_enable(ptp, FREAD);
		break;
	case M_STOPI:
	case M_STOP:
	default:
		freemsg(mp);
		break;
	}
	return(0);
}

/*
 * ptm_enable: start up input and/or output on master.  Return silently
 * if master is not open.
 *
 * READ_LOCK or WRITE_LOCK should be held on entry to this routine.
 */

PRIVATE_STATIC void
ptm_enable(register struct pty_s *ptp, int flag)
{
	PTY_ANY_LOCK_ASSERT(ptp);
	/* Non-null pt_mrq implies open master */
	if (ptp->pt_mrq) {
		if (flag & FREAD)
			qenable((queue_t *)ptp->pt_mrq);
		if (flag & FWRITE)
			qenable((queue_t *)ptp->pt_mwq);
	}
	return;
}

#if	SEC_ARCH

PRIVATE_STATIC void
pty_sec_init(struct pty_s *ptp)
{
        /*
         * Initialize security info.
         */

	/*
	 *  POSIX ACLS -- change function arguments
	 */
        SP_OBJECT_CREATE(SIP->si_tag, &ptp->pt_mtag[0], (tag_t) 0,
			 SEC_OBJECT, (dac_t *) 0, (mode_t) 0);
        SP_OBJECT_CREATE(SIP->si_tag, &ptp->pt_stag[0], (tag_t) 0,
			 SEC_OBJECT, (dac_t *) 0, (mode_t) 0);
}

#endif /* SEC_ARCH */

#ifdef COMPAT_43

PRIVATE_STATIC int
pty_bsd43_ioctl(
		int cmd, 
		caddr_t data, 
		struct termios *termp, 
		tcflag_t *compatflagsp, 
		int master_flag
		)
{
	int	ret = 0;
	tcflag_t	tmpflags;

	switch(cmd) {
	case TIOCGETP:
		termios_to_sgttyb(termp, (struct sgttyb *) data);
		break;
	case TIOCSETP:
	case TIOCSETN:
		sgttyb_to_termios((struct sgttyb *) data, termp, compatflagsp);
		ret = (cmd == TIOCSETP) ? TIOCSETAF : TIOCSETA;
		break;
	case TIOCGETC:
		termios_to_tchars(termp, (struct tchars *) data);
		break;
	case TIOCSETC:
		tchars_to_termios((struct tchars *) data, termp);
		ret = TIOCSETA;
		break;
	case TIOCGLTC:
		termios_to_ltchars(termp, (struct ltchars *) data);
		break;
	case TIOCSLTC:
		ltchars_to_termios((struct ltchars *) data, termp);
		ret = TIOCSETA;
		break;
	case TIOCLGET:
		tmpflags = ttcompatgetflags(termp->c_iflag, termp->c_lflag,
                                           termp->c_oflag, termp->c_cflag);

                *(int *)data = tmpflags >> 16;
		break;
	case TIOCLBIS:
	case TIOCLBIC:
	case TIOCLSET:
		flags_to_termios(cmd, *(tcflag_t *) data, termp, compatflagsp);
		ret = TIOCSETA;
		break;
	case OTIOCGETD:
                /*
                 * This ioctl doesn't have much meaning in STREAMS.
                 * It's apparently only used by things wanting to know
                 * if job control is supported, and they view "2" as a yes
                 * answer, so we return 2 for binary compatibility, since
                 * we do support job control.
                 */
		*(int *)data = 2;
		break;
	case OTIOCSETD:
		/* NOOP in streams -- just fall through */
	default:
		break;
	}
	return(ret);
}

#endif /* COMPAT_43 */
#ifdef LOADABLE_DRIVERS

extern dev_t rpty_cdev;

#define DEV_FUNNEL_NULL NULL

static struct cdevsw rpty_cdeventry = {
	ptropen,	ptrclose,	ptrread,	ptrwrite,
	ptrioctl,	nodev,		nodev,		0,
	ptrselect,	nodev,		DEV_FUNNEL_NULL
};

int
rpty_configure(
	 sysconfig_op_t	op,
	 caddr_t	indata,
	 size_t		indatalen,
	 caddr_t	outdata,
	 size_t		outdatalen
	     )
{
	int		i, configured, error = 0;
	dev_t		dev;
	struct subsystem_info	info;

	strcpy(info.subsystem_name, "rpty");
	configured = ((!subsys_reg(SUBSYS_GET_INFO, &info)) &&
		      info.config_flag);
	if ((configured && op == SYSCONFIG_CONFIGURE) ||
	    (!configured &&  op != SYSCONFIG_CONFIGURE))
		return (EALREADY);

	switch (op) {
	case SYSCONFIG_CONFIGURE:
		if ((dev = cdevsw_add(rpty_cdev, &rpty_cdeventry)) == NODEV) {
			printf("rpty: found cdev 0x%x in use\n", rpty_cdev);
			error = ENODEV;
			break;
		}
		break;

	case SYSCONFIG_RECONFIGURE:
	case SYSCONFIG_QUERYSIZE:
	case SYSCONFIG_QUERY:
	case SYSCONFIG_UNCONFIGURE:
		error = EINVAL;
		break;
	default:
		error = EINVAL;
		break;
	}
	return (error);
}

#endif /* LOADABLE_DRIVERS */
#endif /* NRPTY > 0 */
