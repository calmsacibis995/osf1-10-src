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
static char *rcsid = "@(#)$RCSfile: strtty_gen.c,v $ $Revision: 1.1.5.3 $ (DEC) $Date: 1993/08/19 12:25:52 $";
#endif
/*
 * (c) Copyright 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.2
 */
/*
 * Common #includes
 */

#include <sys/types.h>
#include <sys/signal.h>
#include <sys/conf.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/eucioctl.h>

/*
 * More #includes
 */

#include <kern/xpr.h>
#include <kern/assert.h>
#include <vm/vm_kern.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/sysconfig.h>
#include <sys/table.h>
#include <sys/user.h>

/*
 * STREAMS #includes
 */

#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/str_support.h>
#include <tty/stream_tty.h>     /* common STREAMS tty definitions */
#include <tty/strtty_gen.h>
#include <streams/memdebug.h>

/* A null STT for drivers to use in initializing their tp's */

static STT	Null_stt;
STTP		Null_tp = &Null_stt;

/*
 *
 * Function description:
 *
 * strtty_wsrv:
 *	WRITE queue service routine.  Check for timeouts to process,
 *	then call the driver's "start" routine.
 *
 * Arguments:
 *
 *	q - WRITE queue pointer
 *
 * Return value:
 *
 *	Whatever the driver's start routine returns.
 *
 * Side effects:
 *
 *	None
 *
 */
int strtty_wsrv(
	    queue_t *q
	    )
{
	STTP 		tp = (STTP)q->q_ptr;
	
	if (tp->t_timeout_state) 
		(void) tt_run_jobs(tp->t_tmtab);

	return ((*tp->t_start_proc)(tp));
}

/*
 *
 * Function description:
 *
 * strtty_allocin:
 *	Try to allocate an input message buffer.
 *	If it fails, use bufcall to try again later
 *
 * Arguments:
 *
 *	tp - terminal data structure pointer
 *
 * Return value:
 *
 *	pointer to allocated message block
 *
 * Side effects:
 *
 *	None
 *
 */
mblk_t * strtty_allocin(
	  register STTP tp
	  )
{
	register mblk_t *mp;
	static int size;
	int wanted;
	int s;
	
	if (!(mp = tp->t_inmsg)) {
		if (!size) {
			mp = allocb(1, BPRI_MED);
			if (mp)
				size = mp->b_datap->db_size;
		}
		wanted = max(INTERRUPT_BSIZE, size);
		if (!mp || mp->b_datap->db_size < wanted) {
			if (mp)
				freeb(mp);
			mp = allocb(wanted, BPRI_MED);
		}
	}

	if (mp) {
		s = spltty();
		tp->t_state &= ~S_WAITINMSG;
		tp->t_inmsg = mp;
		splx(s);
		return (mp);
	} else {
		s = spltty();
		tp->t_state |= S_WAITINMSG;
		splx(s);
		if (!bufcall(INTERRUPT_BSIZE, BPRI_MED, strtty_allocin, (caddr_t)tp))
			timeout(strtty_allocin,(caddr_t)tp,hz);
		return ((mblk_t *)0);
	}
}

/*
 *
 * Function description:
 *
 * strtty_flush:
 *     Flush read and/or write data
 *
 * Arguments:
 *
 *	tp - pointer to terminal structure
 *	rw - flag for which data to flush
 *	generate - 0 = external, don't send M_FLUSH
 *	       1 = internal, need to send M_FLUSH
 *
 * Return value:
 *
 *	None
 *
 * Side effects:
 *
 *	Input/output messages deallocated. Queues flushed.
 *
 */
void strtty_flush(
        register STTP tp,
        int rw,
        int generate
	)
{
	mblk_t *mp;
	int s;
	
	s = spltty();
	
	if (rw & FWRITE) {
		flushq(WR(tp->t_queue),FLUSHDATA); /* Clear write queue */
		if (mp = tp->t_outmsg) {
			/* If there's a current output message,
			 * clear it.
			 */
			tp->t_outmsg = (mblk_t *) NULL;
			freemsg(mp);
		}
		(*tp->t_stop_proc)(tp);		/* Clear rest of output */
	}
	if (rw & FREAD) {
		if (mp = tp->t_inmsg)	  /* if there's an input buffer */
			mp->b_wptr = mp->b_rptr; /* clear it */
		flushq(tp->t_queue,FLUSHDATA);
		if (generate) {
			putctl1((tp->t_queue)->q_next,M_FLUSH,FLUSHR);
		}
	}
	splx(s);
} /* end strtty_flush */


/*
 *
 * Function description:
 *
 * strtty_wput:
 *	Write-side put routine handles all messages coming downstream
 *	from above.  Messages are split into out-of-band messages
 *	and in-band messages.
 *
 *	In-band messages are those that must be synchronized with the
 *	output data stream.  These messages are simply queued to the
 *	WRITE queue, so are processed in proper order relative to other
 *	in-band messages.
 *
 *	Out-of-band messages are those that produce immediate action
 *	and do not have any special order relative to the output data stream.
 *	These messages are processed here, then freed.
 *
 * Arguments:
 *
 *	q - WRITE queue pointer
 *	mp - message pointer
 *
 * Return value:
 *
 *	1 - success
 *	0 - failure
 *
 * Side effects:
 *
 *	Messages processed
 *
 */
int
strtty_wput(
	    register queue_t *q, 
	    register mblk_t *mp
	    )
{
	register STTP tp = (STTP)q->q_ptr;
	struct iocblk *iocp;
	mblk_t *mp1;

	switch (mp->b_datap->db_type) {
		/*
		 * In-band messages are simply queued.
		 */
	case M_DATA:
		/*
		 * Break up complex messages into single blocks to simplify
		 * driver output processing.
		 */
		while (mp1 = unlinkb(mp)) {
			putq(q,mp);	/* queue each single block */
			mp = mp1;	/* get pointer to remaining message */
		}		    /* final (or only) piece is queued below */
	/* fall through */
	case M_DELAY:
	case M_BREAK:
		putq(q,mp);
		break;
	case M_STOPI:
		(*tp->t_stopi_proc)(tp, mp);
		break;
	case M_STARTI:
		(*tp->t_starti_proc)(tp, mp);
		break;
	case M_IOCTL:
		/*
		 * M_IOCTL messages are in-band if the command must wait until
		 * output data has drained.  Otherwise they are out-of-band.
		 */
		if (tp->t_qioctl) {
			rmvq(q, tp->t_qioctl);
			freemsg(tp->t_qioctl);
			tp->t_qioctl = (mblk_t *) NULL;
		}

		iocp = (struct iocblk *)mp->b_rptr;
		switch (iocp->ioc_cmd) {
		/* These wait for output to drain */
		case TIOCSETAW: 
		case TIOCSETAF:
		case TCSBRK:
			tp->t_qioctl = mp;
			putq(q,mp); /* so just queue them in order */
			break;
			
		default:
			/* else process the message now */
			(void)(*tp->t_ioctl_proc)(q,mp,tp); 
		}
		break;
		/*
		 * Now handle out-of-band messages
		 */
	case M_STOP:		     /* Stop output */
		if (!(tp->t_state & S_TTSTOP)) {
			tp->t_state |= S_TTSTOP;
			(*tp->t_stop_proc)(tp);
		}
		freemsg(mp);
		break;
		
	case M_START:		     /* Restart output */
		if (tp->t_state & S_TTSTOP) {
			tp->t_state &= ~S_TTSTOP;
			(void)((*tp->t_start_proc)(tp));
		}
		freemsg(mp);
		break;
		
	case M_FLUSH:		     /* Flush */
		{
			int flushflag = 0;
			
			if (*mp->b_rptr & FLUSHW) {
				flushflag |= FWRITE;
				*mp->b_rptr &= ~FLUSHW; /* clear FLUSHW for reply */
			}
			if (*mp->b_rptr & FLUSHR) { /* if it's a READ flush */
				flushflag |= FREAD; /* we will flush our own queues */
				qreply(q,mp); /* reflect the message upstream */
			}
			else
				freemsg(mp); /* otherwise free the message */
			strtty_flush(tp,flushflag,0); /* flush queues */
						      /* according to flags */
		}
		break;
	default:		/* unrecognized message */
		freemsg(mp);
	}
	return 1;
}

/*
 *
 * Function description:
 *
 * strtty_rsrv:
 *	Read service.  Only exists to provide proper STREAMS flow
 *	control.  Just forwards messages from the read queue.
 *
 * Arguments:
 *
 *	q - STREAMS queue pointer
 *
 * Return value:
 *
 *	1
 *
 * Side effects:
 *
 *	None
 *
 */
strtty_rsrv(
	    queue_t *q
	    )
{
	register STTP tp = (STTP)q->q_ptr;
	register mblk_t *mp;

	if (tp->t_timeout_state) 
		(void) tt_run_jobs(tp->t_tmtab);

	while (mp = getq(q)) {
		if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
			putnext(q,mp);
		else {
			putbq(q,mp);
			return 1;
		}
	}
	strtty_sendinputif(tp);	    /* gather any buffered characters */
	if (mp = getq(q)) {
		if ((mp->b_datap->db_type >= QPCTL) || canput(q->q_next))
			putnext(q,mp);
		else {
			putbq(q,mp);
			return 1;
		}
	}
	return 1;
}

/*
 *
 * Function description:
 *
 * strtty_sendinputif:
 *	Send current input upstream if there is any.
 *
 * Arguments:
 *
 *	tp - pointer to terminal structure
 *
 * Return value:
 *
 *	None
 *
 * Side effects:
 *
 *	None
 *
 */
void 
strtty_sendinputif(
              register STTP tp
              )
{
	register mblk_t *mp;
	register int s;
	
	s = spltty();
	if ((mp = tp->t_inmsg) && (mp->b_wptr != mp->b_rptr))
		strtty_sendinput(tp);
	splx(s);
} /* end strtty_sendinputif */

/*
 *
 * Function description:
 *
 * strtty_sendinput:
 *	Try to send input message to the streams read queue
 *	Discard input data if no more room
 *	Allocate new message buffer if needed
 *
 * Arguments:
 *
 *	tp - terminal pointer
 *
 * Return value:
 *
 *	None
 *
 * Side effects:
 *
 *	New message block may be allocated
 *	Input data may be discarded
 */
void 
strtty_sendinput(
            register STTP tp
            )
{
	register queue_t *q;
	register mblk_t *mp;
	
        if (mp = tp->t_inmsg) { /* if there's a message to send */
		q = tp->t_queue;    /* get streams read queue pointer */
                if (!(tp->t_state & S_TBLOCK) && canput(q)) {
                        tp->t_inmsg = (mblk_t *)0;
			(void)strtty_allocin(tp);
			mp->b_flag |= MSGCOMPRESS;
                        putq(q,mp);	    /* send the message */
                } else if (mp->b_wptr == mp->b_datap->db_lim) {
                        /* no more space */

			tp->t_lostinput += mp->b_wptr - mp->b_rptr;
                        mp->b_wptr = mp->b_rptr; /* throw away the data */
                }
        } else
                (void)strtty_allocin(tp);	      /* try to get one */
}

/*
 *
 * Function description:
 *
 * strtty_input:
 *	Put a character into the current input message for
 *	the given line.	 Count lost characters.
 *
 * Arguments:
 *
 *	c - 8 bit character
 *	tp - pointer to terminal data structure
 *
 * Return value:
 *
 * 	0 - if character lost.
 *	1 - if character added to current input message.
 *
 * Side effects:
 *
 *	Count lost input characters
 *
 */
strtty_input(
	register int c,
	register STTP tp
	)
{
	register mblk_t *mp;
	register unit=minor(tp->t_dev);
	
	mp = tp->t_inmsg;	    /* get input message pointer */
	if (!mp)		    /* if there isn't one */
		mp = strtty_allocin(tp);	/* try to get one */
	if (mp && (mp->b_wptr < mp->b_datap->db_lim)) {
		*mp->b_wptr++ = (unsigned char)c; /* store the byte */
		return(1);
	}
	else {
		tp->t_lostinput++; /* count lost characters */
		return(0);
	}
} /* end strtty_input */

int strtty_rstrt(
	register STTP tp
	)
{
	tp->t_state &= ~S_TIMEOUT;
	tp->t_timer = 0;
	(*tp->t_start_proc)(tp);
}

/*
 *
 * Function description: strtty_ioctlbad
 *
 *	Check validity of M_IOCTL message
 *	Allocate mblk for answer if needed
 *
 * Arguments:
 *
 *	mp - pointer to M_IOCTL message
 *
 * Return value:
 *
 *	1 - message is bad, error filled in ioc_error
 *	0 - message is ok
 *
 * Side effects:
 *
 *	Error value filled in message
 *	mblk allocated for answer in case of IOC_OUT
 *
 */
int
strtty_ioctlbad(mblk_t *mp)
{
	struct iocblk *iocp;
	int cmd, len;

	iocp = (struct iocblk *)mp->b_rptr;
	/*
	 * TRANSPARENT ioctls don't want to do a copyout
	 */
	if (iocp->ioc_count == TRANSPARENT) {
		iocp->ioc_count = 0;
	}
	cmd = iocp->ioc_cmd;
	len = IOCPARM_LEN(cmd);
	if (cmd & IOC_IN) {
		/*
		 * IOC_IN commands must have an mblk of the proper size
		 * chained to mp->b_cont
		 */
		if ((iocp->ioc_count != len) || (!mp->b_cont)) {
			iocp->ioc_error = EINVAL;
			goto out;
		}
	}
	if (cmd & IOC_OUT) {
		/*
		 * if an IOC_OUT command comes down without
		 * an attached mblk, allocate one for the answer
		 */
		if (!mp->b_cont)
			mp->b_cont = allocb(len, BPRI_MED);
		if (mp->b_cont)
			iocp->ioc_count = len;
		else {
			iocp->ioc_error = ENOMEM;
			goto out;
		}
	}
      out:
	if (iocp->ioc_error) {
		mp->b_datap->db_type = M_IOCNAK;
		iocp->ioc_count = 0;
		iocp->ioc_rval = 0;
		return (1);
	} else
		return (0);
}

/*
 *
 * Function description: strtty_modem
 *
 *	Send modem indication on a stream (allocate mblk to do so).
 *
 * Arguments:
 *
 *	q 	- queue to send message on.
 *	carr 	- boolean indicator of carrier state
 *
 * Return value:
 *
 *	1 - message handled
 *	0 - message not handled -- memory unavailable
 *
 * Side effects:
 *
 *	mblks allocated
 *
 */

int
strtty_modem(queue_t *q, int carr)
{
	mblk_t *mp;

	if (carr) {
		/* Line discipline doesn't care about carrier coming on */
		return 1;
	}

	mp = allocb(0, BPRI_MED);
        if (mp) {
		mp->b_datap->db_type = M_HANGUP;
		putnext(q, mp);
		return 1;
        }
	return 0;
}
