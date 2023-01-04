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
static char *rcsid = "@(#)$RCSfile: str_scalls.c,v $ $Revision: 4.2.10.5 $ (DEC) $Date: 1994/01/07 20:45:43 $";
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
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/vnode.h>
#include <sys/stat.h>
#include <sys/user.h>		
#include <sys/proc.h>
#include <sys/poll.h>

#include <streams/str_stream.h>
#include <streams/str_proto.h>
#include <streams/str_debug.h>
#include <sys/stropts.h>
#include <sys/ioctl.h>

#include <tty/tty_common.h>
#include <sys/termio.h>

/*
 * The initial open of the console will need
 * at least a simple line discipline!
 */
const static struct strapush console_apush =
	{ SAP_ONE, 0, 0, 0, 1, { "ldterm" } };

extern	struct streamtab sthinfo;
extern  dev_t consdev;

OSRP
osr_alloc (sth, size, pri)
	STHP sth;
	int  size, pri;
{
	OSRP	osr = nilp(OSR);

	if (sth && size == 0) {
		simple_lock(&sth->sth_ext_flags_lock);
		if ((sth->sth_ext_flags & F_STH_OSR_INUSE) == 0) {
			sth->sth_ext_flags |= F_STH_OSR_INUSE;
			osr = &sth->sth_osr;
		}
		simple_unlock(&sth->sth_ext_flags_lock);
	}
	if ( !osr ) {
		if (pri == BPRI_WAITOK) {
		     STR_MALLOC(osr, OSRP, sizeof *osr + size, M_STROSR, M_WAITOK);
		} else {
		     STR_MALLOC(osr, OSRP, sizeof *osr + size, M_STROSR, M_NOWAIT);
		}
	}
	if (osr) {
		static OSR init_osr;
		*osr = init_osr;
		sq_init(&osr->osr_sq);
		if ( size )
			bzero((caddr_t)(osr + 1), size);
		osr->osr_sth = sth;
	}
	return osr;
}

#ifdef	__GNUC__
__inline__
#endif
void
osr_free(osr)
	OSRP	osr;
{
#if	MACH_ASSERT
	if (!osr->osr_sth)
		panic("osr_free sth");
#endif
	if (osr == &osr->osr_sth->sth_osr) {
		simple_lock(&osr->osr_sth->sth_ext_flags_lock);
#if	MACH_ASSERT
		if (!(osr->osr_sth->sth_ext_flags & F_STH_OSR_INUSE))
			panic("osr_free sth_ext_flags");
#endif
		osr->osr_sth->sth_ext_flags &= ~F_STH_OSR_INUSE;
		simple_unlock(&osr->osr_sth->sth_ext_flags_lock);
	} else
		STR_FREE(osr, M_STROSR);
}

/*
 * We need to keep track of active streams for finding
 * abandoned streams under links. We now keep all of
 * them on these lists, but we could just keep ones
 * which we didn't close due to F_STH_LINKED.
 */

#define	STH_HASH_TBL_SIZE	32
#define	STH_HASH(dev)		&sth_open_streams\
	[((unsigned)major(dev) ^ (unsigned)minor(dev)) % STH_HASH_TBL_SIZE]

STHP	sth_open_streams[STH_HASH_TBL_SIZE];
decl_simple_lock_data(static,streams_open_lock)

void
str_open_init ()
{
	simple_lock_init(&streams_open_lock);
}

static STHP
sth_get_sth (dev, errp)
reg	dev_t	dev;
	int *	errp;
{
reg	STHP	sth;

	if (dev == NODEV)
		return 0;
	simple_lock(&streams_open_lock);
	for (sth = *STH_HASH(dev); sth; sth = sth->sth_next)
		if (sth->sth_dev == dev)
			break;
	if (sth) {
		/* Check for races. Not much to do but fail. */
		if (sth->sth_flags & F_STH_CLOSING) {
			*errp = EINPROGRESS;
			sth = 0;
		}
	}
	simple_unlock(&streams_open_lock);
	return sth;
}

/*
 * pse_open - normal open of a streams device.
 *
 * This level distinguishes the various open cases
 *
 *		first open of a new device
 *		reopen of an already open device
 *
 */

int
pse_open (dev, flags, id, newdev, cred, private)
	dev_t	dev;
	int	flags;
	int	id;		/* ignored */
	dev_t	*newdev;	/* ignored for non-clones */
	struct ucred *cred;
	void	**private;
{
	OSRP	osr;
	STHP	sth;
	int	indx, error = 0;

	ENTER_FUNC(pse_open, dev, flags, id, 0);

	sth = (STHP)*private;
	if (flags & O_DOCLONE) {
		/* Clones cannot be reopened. */
		if (sth) {
			error = EIO;
			goto out;
		}
	} else {
		STHP sth2 = sth_get_sth(dev, &error);
		if (error)
			goto out;
		if (sth2) {
			/* Check for abandoned stream. */
			if ( sth == 0 )
				sth = sth2;
			/* Check for races and broken clonable drivers. */
			else if (sth != sth2) {
				error = ENXIO;
				goto out;
			}
		}
	}

	if ( sth == 0 && (indx = dcookie_to_dindex(major(dev))) < 0 ) {
		error = ENODEV;
		goto out;
	}

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_open_dev    = dev;
	osr->osr_open_dindex = indx;
	osr->osr_open_fflag  = flags;
	if (cred != NOCRED)
		osr->osr_creds = cred;
	if (flags & O_NDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (flags & O_NONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;

	/*
	 * Reopen of existing stream.
	 *
	 * In order to guarantee synchronization with ongoing activities,
	 * this request is run via the ioctl_osrq. This synchronization
	 * is necessary in order to prevent nasty things like I_PUSH and
	 * I_POP to happen while we are handling this call.
	 *
	 * We could prevent this by permanently holding the stream head lock,
	 * but that may not be a good idea, in case any of the qi_qopen
	 * routines decide to sleep. This way, while we are on the ioctl_osrq,
	 * further ioctl requests will have to wait.
	 */
	if ( sth ) {
		/* For SVID compatability, check the modes on opens of
		 * FIFO's to make sure there are readers/writers.
		 *
		 * Implement the behavior described in the SVID open(BA_OS)
		 * pages.
		 */
		if(sth->sth_flags & F_STH_FIFO) {
			error = osr_fifo_sleep(sth,flags);
			if(error)
			{
				osr_free(osr);
				goto out;
			}
		}
		osr->osr_flags  |= F_OSR_NEED_MULT_SQH;
		osr->osr_handler = osr_reopen;
		osr->osr_osrq    = &sth->sth_ioctl_osrq;
		error = osr_run(osr);
		osr_free(osr);
	/*
	 * New open goes via osr_open, possibly with CLONEOPEN flag.
	 */
	} else if ((error = osr_open(osr)) == 0) {
		sth = osr->osr_sth;
		osr_free(osr);
	}

	if (error == 0) {
		*private = (void *)sth;
		if (flags & O_DOCLONE) {
			/*
			 * Returning ECLONEME again indicates our desire
			 * for an "anonymous" alias, e.g. for Streams pipes.
			 * See spec_clone() in vfs/spec_vnops.c.
			 */
			if ((*newdev = sth->sth_dev) == NODEV) {
				error = ECLONEME;
				*newdev = dev;
			}
		}
		/*
		 * While we were opening the stream, someone might have
		 * notified us that we are a controlling tty.
		 * We then have to do some special business...
		 */
		if ( !(flags & O_NOCTTY) && (sth->sth_flags & F_STH_ISATTY) ) {
			csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
			if ( sth->sth_flags & F_STH_ISATTY )
				sth_ttyopen(sth, flags);
			csq_release(&sth->sth_rq->q_sqh);
		}
		DB_isopen(sth);
		DB_check_streams("OPEN");

		/* If this was an open of a FIFO wake up any other openers
		 * waiting.  Bump read and write reference count.  These
		 * counts don't need to be accurate, as long as they remain
		 * non-zero as long as there is at least one reader or
		 * writer on the FIFO.
		 */
		if(sth->sth_flags & F_STH_FIFO) {
			simple_lock(&sth->sth_ext_flags_lock);
			if(flags & FREAD) {
				sth->sth_ext_flags &= ~F_STH_FIFO_RO;
				wakeup(&sth->sth_read_mode);
				sth->sth_rdcnt++;
			}
			if(flags & FWRITE) {
				sth->sth_ext_flags &= ~F_STH_FIFO_WO;
				wakeup(&sth->sth_write_mode);
				sth->sth_wrcnt++;
			}
			simple_unlock(&sth->sth_ext_flags_lock);
		}
	}
/*
 * if error is ECANCELED, driver open routine realized
 * that a revoke was in process  so just return close/revoke
 * will take care of the rest 
 */
	if (error == ECANCELED)
		error = 0;  /* pretend nothing happened! */
out:
	LEAVE_FUNC(pse_open, error);
	return error;
}

/* Co-ordinate opens of a FIFO per SVID.
 * If a FIFO is opened O_RDONLY or O_WRONLY then the opener must wait
 * for the other side of the FIFO to open.  sth->sth_ext_flags_lock
 * is used to synchronize access to the read and write counts.
 */

osr_fifo_sleep(sth,flags)
reg	STHP	sth;
	int	flags;
{
	register int error = 0;

	if(flags & FWRITE)
	    while(error == 0) {
		if(flags & FREAD)
		    return 0;
		simple_lock(&sth->sth_ext_flags_lock);
		if(sth->sth_ext_flags & F_STH_FIFO_RO || sth->sth_rdcnt) {
		    simple_unlock(&sth->sth_ext_flags_lock);
		    return 0;
		}
		if(flags & (O_NDELAY|O_NONBLOCK)) {
		    simple_unlock(&sth->sth_ext_flags_lock);
		    return ENXIO;
		}
		sth->sth_ext_flags |= F_STH_FIFO_WO;
		simple_unlock(&sth->sth_ext_flags_lock);
		assert_wait((vm_offset_t)&sth->sth_read_mode,TRUE);
		error = tsleep(0,(PZERO+1)|PCATCH,0,0);
	    }

	if(flags & FREAD)
	    while(error == 0) {
		if(flags & FWRITE)
		    return 0;
		simple_lock(&sth->sth_ext_flags_lock);
		if((sth->sth_ext_flags & F_STH_FIFO_WO) || sth->sth_wrcnt ||
			(flags & (O_NDELAY|O_NONBLOCK))) {
		    simple_unlock(&sth->sth_ext_flags_lock);
		    return 0;
		}
		sth->sth_ext_flags |= F_STH_FIFO_RO;
		simple_unlock(&sth->sth_ext_flags_lock);
		assert_wait((vm_offset_t)&sth->sth_write_mode,TRUE);
		error = tsleep(0,(PZERO+1)|PCATCH,0,0);
	    }
	return error;
}



/*
 * Open of streams device (clone open or open of specific device).
 * It is assumed that the cdevsw interface has ensured serialization
 * of open requests on a per device basis. If this becomes no longer
 * the case, it can be achieved by acquiring the open synch queue
 * in the dmodsw entry for this device.
 */
int
osr_open (osr)
reg	OSRP	osr;
{
	dev_t	dev;
	int	error;
	int	i;
	int 	pre_hash = 0;
	OSRQ	close_osrq;
	queue_t	* q = nilp(queue_t);
	struct streamtab * str;
	struct strapush st, * stra;
reg	STHP	sth;
	STHPP	sthp;
	struct open_args open_args;

	ENTER_FUNC(osr_open, osr, 0, 0, 0);

	if (!(str = dindex_to_str(osr->osr_open_dindex))
	||  !str->st_rdinit
	||  !str->st_rdinit->qi_qopen
	||  !modsw_ref(str->st_rdinit, 1)) {
		error = ENXIO;
		goto out;
	}
	if (!(sth = sth_alloc())
	||  !(sth->sth_rq = q_alloc())
	||  !(q = q_alloc())) {
		if ( sth ) {
			if ( sth->sth_rq )
				q_free(sth->sth_rq);
			sth_free(sth);
		}
		(void) modsw_ref(str->st_rdinit, -1);
		error = EAGAIN;
		goto out;
	}
	osr->osr_sth = sth;
	sth->sth_wq = WR(sth->sth_rq);
	sth->sth_rq->q_sqh.sqh_parent = &sth->sth_rq->q_sqh;
	sth->sth_wq->q_sqh.sqh_parent = &sth->sth_rq->q_sqh;
	sth_set_queue(sth->sth_rq, sthinfo.st_rdinit, sthinfo.st_wrinit);
	noenable(sth->sth_wq);
	sth->sth_rq->q_ptr = (caddr_t)sth;
	sth->sth_wq->q_ptr = (caddr_t)sth;
	sth->sth_close_wait_timeout = 15000L;
	sth->sth_read_mode = RNORM | RPROTNORM;
	sth->sth_write_mode = SNDZERO;
	osrq_init(&sth->sth_read_osrq);
	osrq_init(&sth->sth_write_osrq);
	osrq_init(&sth->sth_ioctl_osrq);
	
	/*
	 * Initialize and set flow control pointers and linkage in
	 * the new queue and the Stream head.
	 */
	(void) sqh_set_parent(q, str);
	(void) sqh_set_parent(WR(q), str);
	sth_set_queue(q, str->st_rdinit, str->st_wrinit);

	q->q_ffcp = sth->sth_rq;
	q->q_bfcp = sth->sth_rq->q_bfcp;
	sth->sth_rq->q_bfcp = q;
	WR(q)->q_ffcp = nilp(queue_t);
	WR(q)->q_bfcp = sth->sth_wq;
	sth->sth_wq->q_ffcp = WR(q);
	q->q_next = sth->sth_rq;
	WR(q)->q_next = nilp(queue_t);
	sth->sth_wq->q_next = WR(q);
	dev = osr->osr_open_dev;
	if ((!(osr->osr_open_fflag & O_DOCLONE)) && (dev != NODEV)) { 
		/* Add the new stream into the open streams hash table  */
		/* See comment at sth_get_sth(). 			*/
		/* need to do this here to compensate for revoke being  */
		/* called while the driver is sleeping in open ...	*/
		/* this enables pse_close to get a sth via sth_get_sth()*/
		pre_hash++;
		sth->sth_dev = dev;
		simple_lock(&streams_open_lock);
		sthp = STH_HASH(dev);
		sth->sth_next = *sthp;
		*sthp = sth;
		simple_unlock(&streams_open_lock);
	}

	/*
	 * Open the driver.
	 */
	open_args.a_func  = q->q_qinfo->qi_qopen;
	open_args.a_queue = q;
	open_args.a_devp  = &dev;
	open_args.a_fflag = osr->osr_open_fflag;
	open_args.a_sflag = (osr->osr_open_fflag & O_DOCLONE) ? CLONEOPEN : 0;
	open_args.a_creds = osr->osr_creds;
	mult_sqh_acquire(osr);
	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
	error = csq_protect(q, WR(q),
		(csq_protect_fcn_t)open_wrapper,
		(csq_protect_arg_t)&open_args,
		&osr->osr_sq, TRUE);
	mult_sqh_release(osr);
	csq_release(&sth->sth_rq->q_sqh);

	if (error) {
		/*
		 * a revoke has been issued
		 * just let close do the cleanup
		 */
		if (error == ECANCELED)
			goto out;
		if (pre_hash) {
			simple_lock(&streams_open_lock);
			sthp = STH_HASH(sth->sth_dev);
			if (*sthp == sth)
				*sthp = sth->sth_next;
			else for ( ; *sthp; sthp = &((*sthp)->sth_next)) {
				if ((*sthp)->sth_next == sth) {
					(*sthp)->sth_next = sth->sth_next;
					break;
				}
			}
			simple_unlock(&streams_open_lock);
		}
		q_free(q);
		q_free(sth->sth_rq);
		sth_free(sth);
		(void) modsw_ref(str->st_rdinit, -1);
	} else if (dev == NODEV) {
		/* This is a pipe which does not want to identify itself. */
		sth->sth_dev = NODEV;
	} else {
		if (osr->osr_open_fflag & O_DOCLONE)
			osr->osr_open_dev =
				makedev(major(osr->osr_open_dev), minor(dev));
		dev = sth->sth_dev = osr->osr_open_dev;
		/* Add the new stream into the open streams hash table */
		/* See comment at sth_get_sth(). */
		/* now take care of the clone case */
		if (! pre_hash) {
			simple_lock(&streams_open_lock);
			sthp = STH_HASH(dev);
			sth->sth_next = *sthp;
			*sthp = sth;
			simple_unlock(&streams_open_lock);
		}

		/* Check for autopushes */
		if (sad_get_autopush((long)major(dev), (long)minor(dev), &st))
			stra = &st;
		else if ( dev == consdev )
			stra = (struct strapush *)&console_apush;
		else
			stra = NULL;
		if ( stra && stra->sap_npush ) {
			/* Re-use osr as the push osr. */
			osr->osr_handler     = osr_push;
			osr->osr_osrq        = &sth->sth_ioctl_osrq;
			osr->osr_flags      |= F_OSR_NEED_MULT_SQH;
			osr->osr_closeout    = RWHL_ERROR_FLAGS;
			osr->osr_ioctl_arg1  = osr->osr_open_fflag;
			for (i = 0; i < stra->sap_npush; i++) {
				osr->osr_ioctl_arg0p = stra->sap_list[i];
				if (error = osr_run(osr)) {
					if (stra == &console_apush)
						error = 0;
					else {
						osr->osr_ioctl_arg1= O_NONBLOCK;
						osrq_init(&close_osrq);
						osrq_insert(&close_osrq, osr);
						(void) osr_close_subr(&close_osrq);
						osr = 0;
					}
					break;
				}
			}
		}
	}
out:
	if (error && osr)
		osr_free(osr);
	LEAVE_FUNC(osr_open, error);
	return error;
}

/*
 * osr_reopen - the actual reopen process in the form of an osr_handler.
 *
 * The environment calls us with mult_sqh and stream head locked.
 * Crawl down the stream and call the open routines.
 */
int
osr_reopen (osr)
	OSRP	osr;
{
	STHP	sth = osr->osr_sth;
	struct open_args open_args;
	int	error = 0;
	queue_t	* wq;
	queue_t * rq;

	ENTER_FUNC(osr_reopen, osr, 0, 0, 0);

	csq_release(&sth->sth_rq->q_sqh);
	wq = sth->sth_wq;
	open_args.a_devp  = 0;
	open_args.a_fflag = osr->osr_open_fflag;
	open_args.a_sflag = MODOPEN;
	open_args.a_creds = osr->osr_creds;

	while (!STREAM_END(wq)) {
		wq = wq->q_next;
		open_args.a_queue = rq = RD(wq);
		open_args.a_func  = rq->q_qinfo->qi_qopen;
		if (STREAM_END(wq)) {
			/*
			 * Device reopen - this may be wrong in the
			 * presence of welded modules...
			 */
			open_args.a_devp  = &osr->osr_open_dev;
			open_args.a_sflag = 0;
		}
		error = csq_protect(rq, wq,
			(csq_protect_fcn_t)open_wrapper,
			(csq_protect_arg_t)&open_args,
			&osr->osr_sq, TRUE);
		if (error)
			break;
	}
	if (error == 0) {
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
		sth->sth_flags &= ~F_STH_CLOSED;
	}

	LEAVE_FUNC(osr_reopen, error);
	return error;
}

int
pse_close (dev, flag, mode, cred, private)
	dev_t	dev;
	int	flag;
	int	mode;	/* unused */
	struct ucred *cred;
	void	*private;
{
	OSRP	osr;
	STHP	sth;
	int	error;
	OSRQ	close_osrq;

	ENTER_FUNC(pse_close, dev, flag, 0, 0);

	sth = (STHP)private;
	if (! sth) {
		sth = sth_get_sth(dev, &error);
		if (! sth) 
			return(0); /* XXX fail silently */	
	}
			/* Check for races and broken clonable drivers. */
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	osr->osr_osrq = &sth->sth_ioctl_osrq;
	if (cred != NOCRED)
		osr->osr_creds = cred;
	if (flag & FNDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (flag & FNONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;
	osr->osr_ioctl_arg1 = flag;

	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
	sth->sth_flags |= F_STH_CLOSED;
	csq_release(&sth->sth_rq->q_sqh);

	/*
	 * Chase other callers out...
	 */
	osrq_cancel(&sth->sth_ioctl_osrq);
	osrq_cancel(&sth->sth_read_osrq);
	osrq_cancel(&sth->sth_write_osrq);

	csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);

	select_wakeup(&sth->sth_pollq);
	select_dequeue_all(&sth->sth_pollq);

	/* There may be signal structures left. */
	while ( sth_sigs_active(sth) ) {
		SIGSP ss = (SIGSP)sth->sth_sigsq.next;
		remque(&ss->ss_link);
		remque(&ss->ss_proclink);
		STR_FREE(ss, M_STRSIGS);
	}

	sth_ttyclose(sth);

	if ( sth->sth_flags & F_STH_LINKED ) {
		/*
		 * ... then don't proceed any further. This
		 * stream will be wiped out during unlink.
		 * See comment at sth_get_sth().
		 */
		csq_release(&sth->sth_rq->q_sqh);
		osr_free(osr);
		error = 0;
	} else {
		csq_release(&sth->sth_rq->q_sqh);
		osrq_init(&close_osrq);
		osrq_insert(&close_osrq, osr);
		error = osr_close_subr(&close_osrq);
	}
	DB_check_streams("CLOSE");
	LEAVE_FUNC(pse_close, error);

	REPORT_FUNC();
	return error;
}

/*
 * osr_close_subr - dismantle one or more streams
 *
 * The standard call to this routine will be with a list
 * of one element, right from pse_close (see above).
 *
 * Another call is possible from osr_unlink which hands
 * over a list of streams which have become obsolete since
 * they were already closed, and have now been unlinked.
 *
 * If there are persistent links under this stream, it is not
 * closed.
 *
 * Furthermore, at some point during close processing, we have
 * to unlink our lower streams (if any), which may lead to more
 * streams to be closed. The osr_unlink_subr, which we use for
 * that, is prepared to report back such streams.
 *
 * On entry, no resources are held.
 * On exit, no resources are held, and all stream heads and OSR's
 * have been deallocated. (THIS IS DIFFERENT, compared to the other
 * cases, for obvious reasons.)
 */
int
osr_close_subr (osrq)
	OSRQP	osrq;
{
	STHP		sth;
	OSRP		osr;
	queue_t *	q;
	int		err, error = 0, closecount = 0;
	int		tmo;
	STHPP		sthp;

	ENTER_FUNC(osr_close_subr, osrq, 0, 0, 0);

	while ( (osr = osrq_remove(osrq)) != nil(OSRP) ) {
		sth = osr->osr_sth;
		osrq_insert(&sth->sth_ioctl_osrq, osr);	/* for synch&wakeups */
		osr->osr_flags |= F_OSR_NEED_MULT_SQH;
		mult_sqh_acquire(osr);
		csq_acquire(&sth->sth_rq->q_sqh, &osr->osr_sq);
		sth->sth_flags |= F_STH_CLOSING;

		/*
		 * Unlink lower streams, if there are any. This might
		 * add work to our list of streams to be closed...
		 */
		if ( sth->sth_mux_top ) {
			err = osr_unlink_subr(osr, MUXID_ALL, I_UNLINK, osrq);
STR_DEBUG(if (err) printf("streams: error %d during close/unlink\n", err));
		}

		/*
		 * If persistent links still exist, become a orphan.
		 */
		if ( sth->sth_pmux_top ) {
			mult_sqh_release(osr);
			if (osr != osrq_remove(&sth->sth_ioctl_osrq))
				panic("osr_close_subr osr");
			sth->sth_flags &= ~F_STH_CLOSING;
			csq_release(&sth->sth_rq->q_sqh);
			osr_free(osr);
			continue;
		}

		q = sth->sth_wq;
		/*
		 * If this stream is one end of a pipe, discard its stat
		 * and fattach buffer, and if the other end has not been
		 * closed, send an M_HANGUP message down the stream.
		 */
		if ( (sth->sth_flags & (F_STH_FIFO|F_STH_PIPE|F_STH_HANGUP)) == F_STH_PIPE
		&&   q->q_next )
			putctl(q->q_next, M_HANGUP);

		/*
		 * Time to pop the modules. The spec requires that we wait 15
		 * seconds per non-empty module. We take the stream head into
		 * this consideration in the first round. We now allow
		 * interrupts during the wait, and take that as indication
		 * not to wait any more during the following pop operations.
		 * Each queue is polled every second until empty.
		 * Should we bother to wait on linked streamheads?
		 */
		err = 0;

		while ( q = sth->sth_wq->q_next ) {

			if ( (q->q_first || sth->sth_wq->q_first)
			&&   sth->sth_close_wait_timeout > 0
			&&   !(osr->osr_flags & F_OSR_NBIO) ) {
				tmo = MS_TO_TICKS(sth->sth_close_wait_timeout);
				if (tmo == 0) tmo = 1;
				for (;;) {
					err = osr_sleep(osr, TRUE, MIN(hz,tmo));
					if (err == ETIME) {
						err = 0;
						tmo -= hz;
					}
					if (err) {
						sth->sth_close_wait_timeout = 0;
						break;
					}
					if (tmo <= 0 || !(q->q_first
					     || sth->sth_wq->q_first))
						break;
				}
			}

			/*
			 * Check for any welds before popping the next queue!
			 */
			if ( sth->sth_wq->q_flag & QWELDED )	/* backq(q) */
				unweldq_exec(sth->sth_wq, q, &osr->osr_sq);
			q = OTHERQ(q);
			if ( q->q_next && (q->q_flag & QWELDED) )
				unweldq_exec(q, q->q_next, &osr->osr_sq);
			if ((q = sth->sth_wq->q_next) == nilp(queue_t)) {
				err = 0;
				break;
			}

			/* 
			 * The last error will be the driver's.
			 */
			err = osr_pop_subr(osr, q);

			/*
			 * After the sth_wq wait, flush it to avoid data
			 * out of sequence and further clogs.
			 */
			flushq(sth->sth_wq, FLUSHALL);
		}
		/*
		 * Call osr_pop_subr for the stream head queue pair.
		 * Note osr_pop_subr drops the mult_sqh and sth locks
		 * when it sees a stream head queue. Then remember
		 * the main stream's error for return.
		 */
		(void) osr_pop_subr(osr, sth->sth_wq);
		if (closecount++ == 0)
			error = err;
		osr_free(osr);

		/*
		 * Now remove the stream from the open streams hash table.
		 */
		if (sth->sth_dev != NODEV) {
			simple_lock(&streams_open_lock);
			sthp = STH_HASH(sth->sth_dev);
			if (*sthp == sth)
				*sthp = sth->sth_next;
			else for ( ; *sthp; sthp = &((*sthp)->sth_next)) {
				if ((*sthp)->sth_next == sth) {
					(*sthp)->sth_next = sth->sth_next;
					break;
				}
			}
			simple_unlock(&streams_open_lock);
		}

		/*
		 * Time to chuck pipe info and fdetach other end.
		 */
		if (sth->sth_flags & F_STH_PIPE)
			sth_update_times(sth, FSYNC, (struct stat *)0);

		DB_isclosed(sth);
		sth_free(sth);
	}
	if (error == ERESTART)
		error = EINTR;
	LEAVE_FUNC(osr_close_subr, error);
	return error;
}

int
pse_read (dev, uiop, fflag, private)
	dev_t		dev;
	struct uio	*uiop;
	int		fflag;
	void		*private;
{
	OSRP	osr;
	STHP	sth;
	int	error;

	ENTER_FUNC(pse_read, dev, uiop, fflag, 0);

	sth = (STHP)private;

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	if (fflag & IO_NDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (fflag & IO_NONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;
	osr->osr_flags   |= F_OSR_RTTY_CHECK;
	osr->osr_rw_uio   = uiop;
	osr->osr_rw_count = uiop->uio_resid;
	osr->osr_handler  = osr_read;
	osr->osr_osrq     = &sth->sth_read_osrq;
	osr->osr_closeout = RL_ERROR_FLAGS;

	error = osr_run(osr);

	if (osr->osr_rw_total)
		sth_uiodone(osr);

	if (osr->osr_flags & F_OSR_BLOCK_TTY) {
		osr_free(osr);
		ASSERT(error == 0);
		if (!(error = tsleep((caddr_t)&lbolt, TTIPRI|PCATCH, ttybg, 0)))
			error = ERESTART;	/* Go around to check revoke */
	} else
		osr_free(osr);

	DB_check_streams("READ");
	LEAVE_FUNC(pse_read, error);
	return error;
}

int
pse_write (dev, uiop, fflag, private)
	dev_t		dev;
	struct uio	*uiop;
	int		fflag;
	void		*private;
{
	OSRP	osr;
	STHP	sth;
	int	error;

	ENTER_FUNC(pse_write, dev, uiop, fflag, 0);

	sth = (STHP)private;
	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	if (fflag & IO_NDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (fflag & IO_NONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;
	osr->osr_flags   |= F_OSR_WTTY_CHECK;
	osr->osr_rw_uio   = uiop;
	osr->osr_rw_count = uiop->uio_resid;
	osr->osr_handler  = osr_write;
	osr->osr_osrq     = &sth->sth_write_osrq;
	osr->osr_closeout = WHL_ERROR_FLAGS;

	error = osr_run(osr);

	if (osr->osr_rw_total)
		sth_uiodone(osr);

	if (osr->osr_flags & F_OSR_BLOCK_TTY) {
		osr_free(osr);
		ASSERT(error == 0);
		if (!(error = tsleep((caddr_t)&lbolt, TTIPRI|PCATCH, ttybg, 0)))
			error = ERESTART;	/* Go around to check revoke */
	} else
		osr_free(osr);

	DB_check_streams("WRITE");
	LEAVE_FUNC(pse_write, error);
	return error;
}

/* Ioctl data buffer - large enough for largest copyin/out */
union streams_ioctl_buffer {
	char			c[FMNAMESZ+1];
	int			i;
	long			l;
	caddr_t			p;
	struct stat		sb;
	struct bandinfo		b;
	struct str_list		s1;
	struct strfdinsert	s2;
	struct strfdinsert_attr	s2a;
	struct strioctl		s3;
	struct strioctl_attr	s3a;
	struct strpmsg		s4;
	struct strpmsg_attr	s4a;
	struct strpeek		s5;
	struct strpeek_attr	s5a;
	struct strrecvfd	s6;
	struct strrecvfd_attr	s6a;
	/* there is no struct strsendfd */
	struct strsendfd_attr	s7a;
};

/* Ioctl permissions */
#define MUSTHAVE(flag, errno) do {	\
	if ((fflag & (flag)) != (flag)	\
	&&  drv_priv(cred) != 0) {	\
		error = (errno);	\
		goto done;		\
	}				\
} while (0)

int
pse_ioctl (dev, cmd, data, fflag, cred, private, retval)
	dev_t		dev;	/* major + minor device number */
	int		cmd;	/* cmd argument to ioctl system call */
	caddr_t		data;	/* *pointer* to 3rd user argument */
	int		fflag;	/* f_flag from file structure */
	struct ucred	*cred;
	int		*retval;
	void		*private;
{
	STHP		sth;
	OSRP		osr;
	int		error = 0;
	int		len = 0;
	char *		buf;
	union streams_ioctl_buffer ioctl_buf;

	ENTER_FUNC(pse_ioctl, dev, cmd, data, fflag);

	sth = (STHP)private;

	if(!sth)
	{
	     *retval = 0;
	     return EIO;
	}

	osr = osr_alloc(sth, 0, BPRI_WAITOK);
	if (cred != NOCRED)
		osr->osr_creds = cred;
	if (fflag & FNDELAY)
		osr->osr_flags |= F_OSR_NDELAY;
	if (fflag & FNONBLOCK)
		osr->osr_flags |= F_OSR_NONBLOCK;
	osr->osr_ioctl_arg0p = buf = (char *)&ioctl_buf;
	bzero(buf, sizeof ioctl_buf);

	/*
	 * Streams ioctl's - trickery speeds switch.
	 */
	if (((cmd & ~0xff) ^ _IO('S', 0)) == 0)
	switch ((unsigned char)(cmd & 0xff)) {

	case I_ATMARK & 0xff:
		osr->osr_handler    = osr_atmark;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case I_CANPUT & 0xff:
		osr->osr_handler    = osr_canput;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case I_CKBAND & 0xff:
		osr->osr_handler    = osr_ckband;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case I_FDINSERT & 0xff:
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler     = osr_fdinsert;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags      |= 
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strfdinsert));
#if SEC_BASE
		((struct strfdinsert_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_FDINSERT_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler     = osr_fdinsert;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags      |=
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strfdinsert_attr));
#else
		error = EINVAL;
#endif
		break;

	case I_FIFO & 0xff:
		osr->osr_handler    = osr_fifo;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		break;

	case I_FIND & 0xff:
		osr->osr_handler     = osr_find;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_closeout    = RWL_ERROR_FLAGS;
		len = FMNAMESZ + 1;
		while ((error = copyin(*(caddr_t *)data, buf, len)) && --len)
			;
		buf[len] = '\0';
		len = 0;
		break;

	case I_FLUSH & 0xff:
		osr->osr_handler    = osr_flush;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		if (osr->osr_ioctl_arg1 & FLUSHR)
			MUSTHAVE(FREAD, EACCES);
		if (osr->osr_ioctl_arg1 & FLUSHW)
			MUSTHAVE(FWRITE, EACCES);
		break;

	case I_FLUSHBAND & 0xff:
		osr->osr_handler    = osr_flushband;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct bandinfo));
		if (error == 0) {
			if (((struct bandinfo *)data)->bi_flag & FLUSHR)
				MUSTHAVE(FREAD, EACCES);
			if (((struct bandinfo *)data)->bi_flag & FLUSHW)
				MUSTHAVE(FWRITE, EACCES);
		}
		break;

	case I_GETBAND & 0xff:
		osr->osr_handler    = osr_getband;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(long);
		break;

	case I_GETCLTIME & 0xff:
		osr->osr_handler    = osr_getcltime;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(long);
		break;

	case I_GETMSG & 0xff:
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler     = osr_getmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpeek);
		error = copyin(*(caddr_t *)data, buf, len);
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.maxlen = -1;
#endif
		break;

	case I_GETMSG_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler     = osr_getmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpeek_attr);
		error = copyin(*(caddr_t *)data, buf, len);
#else
		error = EINVAL;
#endif
		break;

	case I_GETPMSG & 0xff:
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler     = osr_getpmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpmsg);
		error = copyin(*(caddr_t *)data, buf, len);
#if SEC_BASE
		((struct strpmsg_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_GETPMSG_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler     = osr_getpmsg;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len		     = sizeof(struct strpmsg_attr);
		error = copyin(*(caddr_t *)data, buf, len);
#else
		error = EINVAL;
#endif
		break;

	case I_GETSIG & 0xff:
		osr->osr_handler    = osr_getsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)u.u_procp;
		len		    = sizeof(long);
		break;

	case I_GRDOPT & 0xff:
		osr->osr_handler    = osr_grdopt;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(long);
		break;

	case I_GWROPT & 0xff:
		osr->osr_handler    = osr_gwropt;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(long);
		break;

	case I_ISASTREAM & 0xff:
		osr->osr_handler    = osr_isastream;
		osr->osr_osrq       = &sth->sth_read_osrq;
		break;

	case I_LINK & 0xff:
	case I_PLINK & 0xff:
		MUSTHAVE(FREAD|FWRITE, EACCES);
		osr->osr_handler    = osr_link;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		osr->osr_ioctl_arg2 = cmd;
		break;

	case I_LIST & 0xff:
		osr->osr_handler     = osr_list;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_flags      |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout    = RWL_ERROR_FLAGS;
		if ( *(caddr_t *)data )
			error = copyin(*(caddr_t *)data, buf, sizeof(struct str_list));
		else
			osr->osr_ioctl_arg0p = nilp(char);
		break;

	case I_LOOK & 0xff:
		osr->osr_handler    = osr_look;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		break;

	case I_NREAD & 0xff:
		osr->osr_handler    = osr_nread;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		len		    = sizeof(long);
		break;

	case I_POP & 0xff:
		MUSTHAVE(FREAD|FWRITE, EACCES);
		osr->osr_handler    = osr_pop;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = fflag;
		break;

	case I_PEEK & 0xff:
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler     = osr_peek;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strpeek));
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.maxlen = -1;
#endif
		len = sizeof(struct strpeek);
		break;

	case I_PEEK_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler     = osr_peek;
		osr->osr_osrq        = &sth->sth_read_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout    = RL_ERROR_FLAGS;
		len = sizeof(struct strpeek_attr);
		error = copyin(*(caddr_t *)data, buf, len);
#else
		error = EINVAL;
#endif
		break;

	case I_PIPE & 0xff:
		osr->osr_handler    = osr_pipe;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case I_PUSH & 0xff:
		MUSTHAVE(FREAD|FWRITE, EACCES);
		osr->osr_handler     = osr_push;
		osr->osr_osrq        = &sth->sth_ioctl_osrq;
		osr->osr_flags      |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout    = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1  = fflag;
		len = FMNAMESZ + 1;
		while ((error = copyin(*(caddr_t *)data, buf, len)) && --len)
			;
		buf[len] = '\0';
		len = 0;
		break;

	case I_PUTMSG & 0xff:
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler     = osr_putmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strpeek));
#if SEC_BASE
		((struct strpeek_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_PUTMSG_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler     = osr_putmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strpeek_attr));
#else
		error = EINVAL;
#endif
		break;

	case I_PUTPMSG & 0xff:
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler     = osr_putpmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strpmsg));
#if SEC_BASE
		((struct strpmsg_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_PUTPMSG_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler     = osr_putpmsg;
		osr->osr_osrq        = &sth->sth_write_osrq;
		osr->osr_flags	    |= F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout    = WHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strpmsg_attr));
#else
		error = EINVAL;
#endif
		break;

	case I_RECVFD & 0xff:
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler    = osr_recvfd;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_flags	   |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1p= *(caddr_t *)data;
		len = sizeof(struct strrecvfd);
#if SEC_BASE
		/* In/out struct in SEC_BASE - zap both lengths */
		((struct strrecvfd_attr *)buf)->attrbuf.maxlen = -1;
		((struct strrecvfd_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_RECVFD_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FREAD, EBADF);
		osr->osr_handler    = osr_recvfd;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_flags	   |= F_OSR_AUDIT_READ|F_OSR_RTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1p= *(caddr_t *)data;
		len = sizeof(struct strrecvfd_attr);
		error = copyin(*(caddr_t *)data, buf, len);
		if (error)
			len = 0; /* don't try to copyout() below */
#else
		error = EINVAL;
#endif
		break;

	case I_SENDFD & 0xff:
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler    = osr_sendfd;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_flags     |=
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
#if SEC_BASE
		((struct strsendfd_attr *)buf)->fd = osr->osr_ioctl_arg1;
		((struct strsendfd_attr *)buf)->attrbuf.len = -1;
#endif
		break;

	case I_SENDFD_ATTR & 0xff:
#if SEC_BASE
		MUSTHAVE(FWRITE, EBADF);
		osr->osr_handler    = osr_sendfd;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_flags     |=
			F_OSR_NEED_MULT_SQH|F_OSR_AUDIT_WRITE|F_OSR_WTTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(struct strsendfd_attr));
		osr->osr_ioctl_arg1 = ((struct strsendfd_attr *)buf)->fd;
#else
		error = EINVAL;
#endif
		break;

	case I_SETCLTIME & 0xff:
		osr->osr_handler    = osr_setcltime;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		error = copyin(*(caddr_t *)data, buf, sizeof(int));
		break;

	case I_SETSIG & 0xff:
		osr->osr_handler    = osr_setsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)u.u_procp;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case I_SRDOPT & 0xff:
		osr->osr_handler    = osr_srdopt;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case I_STR & 0xff:
		len = sizeof(struct strioctl);
		if ( error = copyin(*(caddr_t *)data, buf, len) )
			break;
		goto other;

	case I_STR_ATTR & 0xff:
#if SEC_BASE
		len = sizeof(struct strioctl_attr);
		if ( error = copyin(*(caddr_t *)data, buf, len) )
			break;
		goto other;
#else
		error = EINVAL;
#endif
		break;

	case I_SWROPT & 0xff:
		osr->osr_handler    = osr_swropt;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case I_UNLINK & 0xff:
	case I_PUNLINK & 0xff:
		MUSTHAVE(FREAD|FWRITE, EACCES);
		osr->osr_handler    = osr_unlink;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg1 = *(int *)data;
		osr->osr_ioctl_arg2 = cmd;
		break;

	default:
		/*
		 * Group 'S' is reserved.
		 */
		error = EINVAL;
		break;
	}

	/*
	 * TTY ioctl's
	 */
	else if (IOCGROUP(cmd) == 't') switch (cmd) {

	case TIOCGPGRP:
		osr->osr_handler    = osr_tiocgpgrp;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)u.u_procp;
		break;

	case TIOCGSID:
		osr->osr_handler    = osr_tiocgsid;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)u.u_procp;
		break;

	case TIOCSPGRP:
		MUSTHAVE(FREAD, EACCES);
		osr->osr_handler    = osr_tiocspgrp;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_ITTY_CHECK;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)u.u_procp;
		osr->osr_ioctl_arg1 = *(int *)data;
		break;

	case TIOCSCTTY:
		osr->osr_handler    = osr_tiocsctty;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		osr->osr_ioctl_arg2p= (char *)u.u_procp;
		break;

	case TIOCCONS:
		osr->osr_handler    = osr_tioccons;
		osr->osr_osrq       = &sth->sth_write_osrq;
		osr->osr_closeout   = RWHL_ERROR_FLAGS;
		break;

	default:
		goto other;
	}

	/*
	 * File ioctl's
	 */
	else if (IOCGROUP(cmd) == 'f') switch (cmd) {

	case FIOASYNC:
		/* Hack this into a funny I_SETSIG */
		osr->osr_handler    = osr_setsig;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		*(int *)osr->osr_ioctl_arg0p = 1;
		if (*(int *)data)
			osr->osr_ioctl_arg1 = S_INPUT|S_OUTPUT|S_ERROR|S_HANGUP;
		osr->osr_ioctl_arg2p= (char *)u.u_procp;
		break;

	case FIOFATTACH:
	case FIOFDETACH:
		MUSTHAVE(FKERNEL, EINVAL);
		osr->osr_handler    = osr_fattach;
		osr->osr_osrq       = &sth->sth_ioctl_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_ioctl_arg1 = (cmd == (int)FIOFATTACH);
		osr->osr_ioctl_arg2p= (char *)(*(void **)data);
		break;

	case FIONREAD:
		osr->osr_handler    = osr_fionread;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		break;

	case FIOPIPESTAT:
		osr->osr_handler    = osr_pipestat;
		osr->osr_osrq       = &sth->sth_read_osrq;
		osr->osr_flags     |= F_OSR_NEED_MULT_SQH;
		osr->osr_closeout   = RWL_ERROR_FLAGS;
		break;

	default:
		goto other;
	}

	/*
	 * Others
	 */
	else
other:	{
		struct strioctl *stri = (struct strioctl *)buf;

		/*
		 * If len set, then is I_STR; else an unknown
		 * ioctl gets mapped to a TRANSPARENT ioctl.
		 */
		if ( len ) {
			osr->osr_ioctl_arg1p = stri->ic_dp;
			osr->osr_ioctl_arg1_len = stri->ic_len;
			if ( stri->ic_len == TRANSPARENT )
				osr->osr_ioctl_arg1_len = 0;
			else if ( stri->ic_len < 0 ) {
				error = EINVAL;
				goto done;
			}
			/* SVID says I_STR always blocks */
			osr->osr_flags &= ~F_OSR_NBIO;
		} else {
			stri->ic_cmd    = cmd;
			stri->ic_timout = -1;
			if ( cmd & (IOC_IN | IOC_OUT) ) {
				/* ioctl() code passes kernel address here */
				stri->ic_dp             = data;
				osr->osr_ioctl_arg1p    = data;
				stri->ic_len            = SEMI_TRANSPARENT;
				osr->osr_ioctl_arg1_len =
					(cmd & IOC_IN) ? IOCPARM_LEN(cmd) : 0;
			} else {
				stri->ic_dp             = *(caddr_t *)data;
				osr->osr_ioctl_arg1p    = *(caddr_t *)data;
				stri->ic_len            = FULLY_TRANSPARENT;
				osr->osr_ioctl_arg1_len = sizeof(caddr_t);
			}
		}
		osr->osr_handler  = osr_str;
		osr->osr_osrq     = &sth->sth_ioctl_osrq;
		osr->osr_closeout = RWHL_ERROR_FLAGS;
#if SEC_BASE
		if ( len != sizeof (struct strioctl_attr) ) {
			/* In/out struct in SEC_BASE - zap both lengths */
			((struct strioctl_attr *)buf)->attrbuf.maxlen = -1;
			((struct strioctl_attr *)buf)->attrbuf.len = -1;
		}
#endif

		/* Protect TTY ioctl's in both I_STR and transparent paths */
		if (IOCGROUP(stri->ic_cmd) == 't') switch (stri->ic_cmd) {
		case TIOCSTI:
			MUSTHAVE(FREAD, EPERM);
			/* No need to lock sth here... */
			if (sth_isctty(u.u_procp, sth) == 0
			&&  drv_priv(cred) != 0) {
				error = EACCES;
				goto done;
			}
			osr->osr_flags |= F_OSR_ITTY_CHECK;
			break;

		case TIOCSETD:
		case TIOCFLUSH:
		case TIOCSWINSZ:
		case TIOCSETA:
		case TIOCSETAW:
		case TIOCSETAF:
#ifdef COMPAT_43
		case TIOCSETP:
		case TIOCSETN:
		case TIOCSETC:
		case TIOCSLTC:
		case TIOCLBIS:
		case TIOCLBIC:
		case TIOCLSET:
		case OTIOCSETD:
#endif
		/* SVID ioctls */
		case TCXONC:
		case TCFLSH:
		case TCSETAW:
		case TCSETAF:
		case TCSETA:
		case TCSBREAK:
			/*
			 * If the tty ioctl involves modification,
			 * request to hang if in the background.
			 */
			MUSTHAVE(FREAD, EACCES);
			osr->osr_flags |= F_OSR_ITTY_CHECK;
			break;
		}
	}

	if ( error || (error = osr_run(osr)) ||
	     (osr->osr_flags & F_OSR_BLOCK_TTY))
		goto done;

	/*
	 * Tail-end processing of non-copyout ioctl's here. There are
	 * four ways for an ioctl to return data to the user:
	 * 1)  via M_COPYOUT messages from driver/module.
	 * 2a) by setting arg0p and arg0_len (usually arg0p => ioctl_buf).
	 * 2b) by setting arg0p and len above (overrides arg0_len).
	 * 3)  by setting arg1p and placing an mblk chain in ioctl_data.
	 *
	 * (2a and 2b are mutually exclusive, others are not)
	 */
	if ( len == 0 && osr->osr_ioctl_arg0p )
		len = osr->osr_ioctl_arg0_len;
	if ( len > 0 ) {
		if ( cmd & (IOC_IN | IOC_OUT) ) {
#if	MACH_ASSERT
			/* These must be trustworthy */
			if (!(cmd & IOC_OUT) || len > IOCPARM_LEN(cmd))
				panic("pse_ioctl arg0p");
#endif
			bcopy((caddr_t)osr->osr_ioctl_arg0p, data, len);
		} else
			error = copyout((caddr_t)osr->osr_ioctl_arg0p,
					*(caddr_t *)data, len);
	}
	if ( osr->osr_ioctl_data ) {
		if (error == 0 && (len = osr->osr_ioctl_arg1_len) > 0) {
			MBLKP mp = osr->osr_ioctl_data;
			if ((cmd & (IOC_IN|IOC_OUT))
			&&  (!(cmd & IOC_OUT) || len > IOCPARM_LEN(cmd)))
				error = EFAULT;
			else do {
				if (!mp || mp->b_datap->db_type != M_DATA ||
				    (unsigned)(len = mp->b_wptr - mp->b_rptr) >
						osr->osr_ioctl_arg1_len) {
					error = EFAULT;
					break;
				}
				if (cmd & IOC_OUT)
					bcopy((caddr_t)mp->b_rptr,
					    (caddr_t)osr->osr_ioctl_arg1p, len);
				else if (error = copyout((caddr_t)mp->b_rptr,
					    (caddr_t)osr->osr_ioctl_arg1p, len))
					break;
				mp = mp->b_cont;
				osr->osr_flags |= F_OSR_AUDIT_READ;
				osr->osr_ioctl_arg1p += len;
			} while ((osr->osr_ioctl_arg1_len -= len) > 0);
			if (error == 0 && osr->osr_ioctl_arg1_len)
				error = EFAULT;
		}
		freemsg(osr->osr_ioctl_data);
	}

#if SEC_BASE
	/*
	 * Make sure that the ioctl commands that perform data transfers
	 * get audited as reads and writes.
	 */
	if (error == 0 &&
	    (osr->osr_flags & (F_OSR_AUDIT_READ|F_OSR_AUDIT_WRITE))) {
		int fd;
		STHP nsth;
		/*
		 * We need the fd this ioctl is operating on for the
		 * audit record. We no longer have u.u_arg so we use
		 * sth_fd_to_sth() until we match our sth. While we
		 * might get it wrong if we've dup'd the fd, we're at
		 * least close.
		 */
		if (u.utask->uu_fd) {
			for (fd = 0; fd <= u.utask->uu_fd->fd_lastfile; fd++)
				if (sth_fd_to_sth(fd, &nsth) == 0
					&& sth == nsth)
					break;
			if (fd <= u.utask->uu_fd->fd_lastfile) {
				if (osr->osr_flags & F_OSR_AUDIT_READ)
					audstub_rdwr1(UIO_READ, fd);
				if (osr->osr_flags & F_OSR_AUDIT_WRITE)
					audstub_rdwr1(UIO_WRITE, fd);
			}
		}
	}
#endif
	if (error == 0)
		*retval = osr->osr_ret_val;

done:	DB_check_streams("IOCTL");

	if (osr->osr_flags & F_OSR_BLOCK_TTY) {
		osr_free(osr);
		ASSERT(error == 0);
		if (!(error = tsleep((caddr_t)&lbolt, TTIPRI|PCATCH, ttybg, 0)))
			error = ERESTART;	/* Go around to check revoke */
	} else
		osr_free(osr);
out:
	LEAVE_FUNC(pse_ioctl, error);
	return error;
}

/*
 *	pse_select - select (poll) system call
 *
 *	Parameter	In/Out		Semantics
 *
 *	dev		in		device number, used to locate stream
 *	events		in		events that are requested
 *	revents		out		reported events (immediate hits)
 *	scanning	in		FALSE means that this is a cleanup call
 */

int
pse_select (dev, events, revents, scanning, private)
	dev_t	dev;
	short *	events;
	short * revents;
	int	scanning;
	void *	private;
{
	STHP		sth;
	MBLKP		mp;
	int		error = 0;
	SQP		sq = nil(SQP);

	ENTER_FUNC(pse_select, dev, events, revents, scanning);

	sth = (STHP)private;
	simple_lock(&sth->sth_ext_flags_lock);
	if ((sth->sth_ext_flags & F_STH_OSR_INUSE) == 0) {
		sth->sth_ext_flags |= F_STH_OSR_INUSE;
		sth->sth_osr.osr_sth = sth;
		sq = &sth->sth_osr.osr_sq;
	}
	simple_unlock(&sth->sth_ext_flags_lock);
	if (sq == nil(SQP))
		STR_MALLOC(sq, SQP, sizeof *sq, M_STRSQ, M_WAITOK);
	sq_init(sq);
	csq_acquire(&sth->sth_rq->q_sqh, sq);

	if ( scanning ) {
		/*
		 * The manual says:
		 *
		 * POLLIN  - non-priority message available (or M_PASSFP)
		 * POLLPRI - priority message available
		 * POLLRDNORM - band 0 message available
		 * POLLRDBAND - non-0 band message available
		 * POLLOUT - non-priority messages can be sent
		 * POLLWRNORM - same as POLLOUT
		 * POLLWRBAND - non-0 band message may be sent
		 * POLLMSG - SIGPOLL has reached front of q (note: only
		 *		this signal is queued by sth_rput)
		 * POLLERR - error message has arrived
		 * POLLHUP - hangup condition
		 * POLLNVAL - bogus file descriptor
		 *
		 * POLLIN and POLLPRI are mutually exclusive on return.
		 * POLLERR and POLLHUP are not requested, only reported.
		 * if POLLHUP is true, then POLLOUT can't be true.
		 *
		 * For consistency with the handling of requests, we add:
		 * POLLERR is mutually exclusive with any other return value!
		 */ 

		if ( sth->sth_flags & (F_STH_READ_ERROR | F_STH_WRITE_ERROR) ){
			*revents = POLLERR;
			goto done;
		}

		if ( sth->sth_flags & F_STH_LINKED ) {
			*revents = POLLNVAL;
			goto done;
		}

		if ( (*events & (POLLPRI|POLLIN|POLLRDNORM|POLLRDBAND|POLLMSG) )
		&&   (mp = sth->sth_rq->q_first) ) {
			if ( mp->b_datap->db_type >= QPCTL ) {
				if (*events & POLLPRI)
					*revents |= POLLPRI;
				if ((*events & POLLMSG)
				&&  mp->b_datap->db_type == M_PCSIG
				&&  (int)*mp->b_rptr == SIGPOLL)
					*revents |= POLLMSG;
			} else {
				if (*events & POLLIN)
					*revents |= POLLIN;
				if ((*events & POLLMSG)
				&&  mp->b_datap->db_type == M_SIG
				&&  (int)*mp->b_rptr == SIGPOLL)
					*revents |= POLLMSG;
				if ( mp->b_band == 0 ) {
					if (*events & POLLRDNORM)
						*revents |= POLLRDNORM;
				} else if (*events & POLLRDBAND)
					*revents |= POLLRDBAND;
			}
		}

		if ( sth->sth_flags & F_STH_HANGUP ) {
			*revents |= POLLHUP;
			goto done;
		}

		if (*events & (POLLOUT | POLLWRNORM | POLLWRBAND)) {
			if (sth_canput(sth, 0)) {
				if (*events & POLLOUT)
					*revents = POLLOUT;
				if (*events & POLLWRNORM)
					*revents |= POLLWRNORM;
			}
			if (*events & POLLWRBAND) {
				int i1 = sth->sth_wq->q_nband;
				/* the stream head creates a band for any
				 * messages sent down stream.
				 */
				for ( ; i1 >= 1; i1--){
					if (sth_canput(sth, i1)) {
						*revents |= POLLWRBAND;
						break;
					}
				}
			}
		}

		if ( *revents == 0 )
			select_enqueue(&sth->sth_pollq);
	} else
		select_dequeue(&sth->sth_pollq);

done:
	csq_release(&sth->sth_rq->q_sqh);
	if (sq == &sth->sth_osr.osr_sq) {
		simple_lock(&sth->sth_ext_flags_lock);
		sth->sth_ext_flags &= ~F_STH_OSR_INUSE;
		simple_unlock(&sth->sth_ext_flags_lock);
	} else
		STR_FREE(sq, M_STRSQ);
out:
	LEAVE_FUNC(pse_select, error);
	return error;
}

/*
 * Clone open. When cloning, shift minor to major and call device open.
 * We pass in a "huge" minor  for two reasons. In case the driver doesn't
 * check O_DOCLONE, this is as far from device 0 as we can get. Second, a
 * clonable driver that uses cdevsw_open_comm() may clip this to its own
 * smaller minor limit, whereas default is the implementation-defined max.
 */

#define minorlimit      (minor(~0))

int
clone_open(
        dev_t   dev,            /* major = clone major, minor = device major */
        int     mode,
        int     id,
        dev_t   *newdev,
        struct ucred *cred,
        void    **privatep)
{
        long    maj;
        int     error;

        if (!(mode & O_DOCLONE))
                return ECLONEME;
        maj = minor(dev);
        dev = makedev(maj, minorlimit);
        if (newdev) *newdev = dev;
        if (id == S_IFCHR)
                CDEVSW_OPEN(maj, dev, mode, id, newdev, error, cred, privatep);

        else
                BDEVSW_OPEN(maj, dev, mode, id, error, cred, privatep);
        return error;
}
