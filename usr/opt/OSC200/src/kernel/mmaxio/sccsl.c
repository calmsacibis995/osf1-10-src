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
static char	*sccsid = "@(#)$RCSfile: sccsl.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:44:42 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*

 *
 */
#ifdef	SCCS

#endif

#include <sys/unix_defs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/user.h>
#include <sys/ioctl.h>
#include <sys/conf.h>
#include <sys/tty.h>
#include <sys/termios.h>
#include <sys/clist.h>
#include <sys/file.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/poll.h>
#include <sys/lock_types.h>
#include <sys/table.h>
#include <kern/queue.h>
#include <kern/sched_prim.h>
#include <mmax/isr_env.h>

#include <mmaxio/crqdefs.h>
#include <mmax/sccdefs.h>
#include <mmaxio/io.h>
#include <mmaxio/scc_sl.h>

udecl_simple_lock_data(,console_lock)


int	slc_cnt = 4;
struct	tty	slc_tty[4];
struct	devaddr	slc_devaddr[] = {
	{12,	0,	0},
} ;

sccsl_t	sccsl[4];


/*1
 * Multimax SCC serial line driver.
 *	This module implements a interface to the SCC serial lines.
 *	A minor device corresponds to a serial line.
/*/

/*2
 *
 * slinit
 *	SCC serial line initializtion.
 *
.* USAGE:
 *	Routine called at boot time to initialize the transmit and receive crqs.
 *	The immediate command packets are linked to the free list of the 
 *	transmit crq.  The attention packets are linked to the free list of the
 *	receive crq. The read packets are linked to the command queue of the 
 *	receive crq. We don't talk to the scc yet - this is done at open time.
 *	This routine is only called once.
 *	
 */

slcinit()
{
	register int		i, j;		/* loop counters */
	register	sccsl_t	*sp;		/* SCC serial line control blk*/
	short		rd_unit, wrt_unit;	/* read/write unitid's */
	register struct tty	*tp;		/* tty pointer */
	int			unit;

	for (unit=0; unit < slc_cnt; unit++) {
		tp = &slc_tty[unit];
#if	UNIX_LOCKS
		lock_init2(&tp->t_lock, TRUE, LTYPE_SCC_TTY);
#endif
		queue_init(&tp->t_selq);
#if	CMUCS
		tp->t_ttyloc.tlc_hostid = TLC_MYHOST;
		tp->t_ttyloc.tlc_ttyid = unit;
#endif
	}

	usimple_lock_init(&console_lock);
	/*
 	*	Initialize the data structures for each line.
 	*/
	for (i = 0; i < slc_cnt; i++) {	/* for each SCC serial line */
		sp = &sccsl[i];
		/*
	 	 * Initialize the transmit crq.  Then initialize the immediate 
	 	 * commands and store them on the free list, where the device
	 	 * should never look at them.  Also initialize the write 
		 * command packet for the line.
		 * All of these command packets are statically allocated as
		 * part of the SCC serial line control block.
	 	 */

		wrt_unit=MAKEUNITID(0, SCC_SLOT, 0, MAKESLLUN(i, SL_WRITE_LUN));
		init_crq(&sp->xmt_crq, CRQMODE_INTR, 0, wrt_unit, NULL);
			    
		for(j = 0; j < SL_NUM_IMMED; j++) {
			sp->immed_msg[j].abort_hdr.crq_msg_hdr.hdr_size =
						sizeof(crq_abort_msg_t);
			sp->immed_msg[j].abort_hdr.crq_msg_crq =
						(crq_t *)&sp->xmt_crq;
		    	sp->immed_msg[j].abort_hdr.crq_msg_unitid = wrt_unit;
			insque(&sp->immed_msg[j],sp->xmt_crq.crq_free.dbl_bwd);
		}
		sp->write_msg.sl_write_hdr.crq_msg_hdr.hdr_size =
						sizeof(crq_sl_write_msg_t);
		sp->write_msg.sl_write_hdr.crq_msg_hdr.hdr_type =
						TYPE_SL_WRITE_MSG;
		sp->write_msg.sl_write_hdr.crq_msg_hdr.hdr_version =
						SL_WRITE_MSG_REV_LEVEL;
		sp->write_msg.sl_write_hdr.crq_msg_crq =
						(crq_t*)&sp->xmt_crq;
		sp->write_msg.sl_write_hdr.crq_msg_code = CRQOP_SL_WRITE;
		sp->write_msg.sl_write_hdr.crq_msg_unitid = wrt_unit;
		sp->write_msg.sl_write_hdr.crq_msg_refnum = SCCSL_REF_WRITE;

		/*
	 	 * Initialize the receive crq.  It gets two attention packets on
	  	 * the free list. These packets are statically initialized as
		 * part of the SCC serial line control block.
	 	 */
		rd_unit = MAKEUNITID(0, SCC_SLOT, 0, MAKESLLUN(i, SL_READ_LUN));
		 
		init_crq(&sp->rcv_crq, CRQMODE_INTR, 0, rd_unit, NULL);
		for(j = 0; j < SL_NUM_ATTN; j++) {
			sp->attn_msg[j].sl_attn_msg.crq_msg_hdr.hdr_size =
						sizeof(crq_sl_attn_msg_t);
			sp->attn_msg[j].sl_attn_msg.crq_msg_hdr.hdr_type =
						TYPE_SL_ATTN;
			sp->attn_msg[j].sl_attn_msg.crq_msg_hdr.hdr_version =
						SL_ATTN_REV_LEVEL;
			sp->attn_msg[j].sl_attn_msg.crq_msg_crq =
						(crq_t *)&sp->rcv_crq;
			sp->attn_msg[j].sl_attn_msg.crq_msg_unitid = rd_unit;
			insque(&sp->attn_msg[j], sp->rcv_crq.crq_free.dbl_bwd);
		}

		/*
		 * Initialize SL_NUMRDBUF read messages and place them on the
		 * command crq.
		 */
		for(j = 0; j < SL_NUMRDBUF; j++) {
			crq_sl_read_msg_t	*rp;

 			rp =  &sp->read_msg[j];
			insque(rp,sp->rcv_crq.crq_cmd.dbl_bwd);
			rp->sl_read_hdr.crq_msg_hdr.hdr_size = sizeof(*rp);
			rp->sl_read_hdr.crq_msg_hdr.hdr_type = TYPE_SCC_RDPKT;
			rp->sl_read_hdr.crq_msg_hdr.hdr_version =
						SCC_RDPKT_REV_LEVEL;
			rp->sl_read_hdr.crq_msg_crq = (crq_t *)&sp->rcv_crq;
			rp->sl_read_hdr.crq_msg_code = CRQOP_SL_READ;
			rp->sl_read_hdr.crq_msg_unitid = rd_unit;
			rp->sl_read_data_limit = sizeof(sp->read_buffer[j]);
			rp->sl_read_error_limit = 1;
			rp->sl_read_delay = SL_DELAY;
			rp->sl_read_data = &sp->read_buffer[j][0];
			rp->sl_read_error = (long *)(&sp->read_error[j]);
			sp->read_error[j] = 0;
		}

		/*
		 * Not ready to talk to SCC yet.
		 */
		sp->crq_status = SLCRQ_DISCONNECT;

		/*
		 * This queue is used for internal synchronization.
		 */
		mpqueue_init(&sp->ioque);
	}
}

/*2
 * slcopen
 *	The open routine must determine the current state of the crqs with
 *	respect to the scc and, if they are not already connected, must
 *	do create channel operations on both crqs to bring the line up.
 *	Assuming the initialization proceeds without error, the line switch
 *	open routine is invoked.
 *
.* ARGUMENTS:
.*	dev	- serial line minor number
.*	flag	- Determines if open should wait for carrier in serial line.
 *
.* RETURNS:
 *	
 *	EBUSY	 - line already open for exclusive access by another process
 *	EIO	 - error in communicating with the SCC
 *
.* USAGE:
 *	Called on an open system call through the character device switch.
 *
.* ASSUMPTIONS:
 *	
 */

/*ARGSUSED*/
slcopen(dev, flag)
dev_t	dev;
int	flag;
{
	register struct tty	*tp;	/* tty pointer */
	register sccsl_t	*sp;	/* SCC control block */
	register int		unit;	/* SCC serial line number */
	int			ret_val;
	USPLVAR(s)
	extern int	slcstart();
	extern int	slcparam();
 
	unit = minor(dev);
	if (unit >= slc_cnt)
		return (ENXIO);
	tp = &slc_tty[unit];
	TTY_LOCK(tp);
	tp->t_oproc = slcstart;
	tp->t_param = slcparam;
	if ((tp->t_state & TS_ISOPEN) == 0) {
		ttychars(tp);
		tp->t_iflag = TTYDEF_IFLAG;
		tp->t_oflag = TTYDEF_OFLAG;
		tp->t_lflag = TTYDEF_LFLAG;
		tp->t_cflag = CS8|CREAD;
		tp->t_ispeed = tp->t_ospeed = TTYDEF_SPEED;
		ttsetwater(tp);
		(void)slcparam(tp, &tp->t_termios);
	} else if (tp->t_state&TS_XCLUDE && u.u_uid != 0) {
		TTY_UNLOCK(tp);
		return (EBUSY);
	}
	USPLTTY(s);
	while ((tp->t_state & TS_CARR_ON) == 0) {
		tp->t_state |= TS_WOPEN;
		if (ret_val = ttysleep(tp, &tp->t_rawq, 
				       TTIPRI | PCATCH, "slcopen", 0)) {
			USPLX(s);
			TTY_UNLOCK(tp);
			return (ret_val);
		}
	}
	LASSERT(TTY_LOCK_HOLDER(tp));
	USPLX(s);
	ret_val = (*linesw[tp->t_line].l_open)(dev, tp, flag);
	TTY_UNLOCK(tp);
	return ret_val;
}


/*2
 * slcparam
 *	Set up USART options and modes.
 *
.* ARGUMENTS:
 *	tp	- pointer to tty struct
 *	t	- pointer termios struct
 *
.* RETURNS:
 *	Sets error and from lower level functions.
 *
.* USAGE:
 *	Called by open to set up intial USART stuff and ttioctl if
 *	USART options change.
 *
.* ASSUMPTIONS:
 *	
 */
slcparam(tp, t)
	register struct tty *tp;
	register struct termios *t;
/**/
{
	register sccsl_t	*sp;	/* pointer to scc control buffer */
	register crq_t *rcrq, *xcrq;
	int	unit = minor(tp->t_dev);
	int	s, error = 0;
	int	mode;
	extern 	slcintr();

	sp = &sccsl[unit];
	/*
	 *  Note that in our current scheme the tty lock also protects
	 *  the sccsl[] entry.  Furthermore, we expect the caller of
	 *  slcparam() to have already locked the tty structure we're
	 *  going to use.
	 /
	LASSERT(TTY_LOCK_HOLDER(tp));
	/*
	 * See if the crqs have been connected to the scc with a
	 * create channel.  This will not be the case if either this
	 * is the first open at boot time or the terminal line was
	 * closed with the hupcls bit set.
	 * If any operation fails, we clean up at the end.
 	 */
	rcrq = NULL;
	xcrq = NULL;
	if(sp->crq_status == SLCRQ_DISCONNECT) {
		rcrq = &sp->rcv_crq;
		error = alloc_vector(&rcrq->crq_master_vect,
			slcintr, unit*2, INTR_DEVICE);
		if (!error) {
			error = create_chan(rcrq);
			if (!error) {
				xcrq = &sp->xmt_crq;
				error = alloc_vector(&xcrq->crq_master_vect,
					slcintr, unit*2+1, INTR_DEVICE);
				if (!error) {
					error = create_chan(xcrq);
					if (!error)
						goto cont;
					dealloc_vector(&xcrq->crq_master_vect);
				}
				delete_chan(rcrq);
			}
			dealloc_vector(&rcrq->crq_master_vect);
		}
cont:
		if (!error) {
			sp->crq_status = SLCRQ_CONNECT;
			/*
			 * Read commands were linked on rcv crq
			 * in slcinit.
			 */
			send_vector(&rcrq->crq_slave_vect);
		}
	}

	LASSERT(TTY_LOCK_HOLDER(tp));
	USPLTTY(s);
	/*
	 *	check requested parameters.
	 * XXX	For now we only support B9600 so enforce that.
	 */
	if (t->c_ospeed != B9600 ||
	    (t->c_ispeed && t->c_ispeed != t->c_ospeed)) {
		USPLX(s);
		return(EINVAL);
	}

	/* and copy to tty */
	tp->t_ispeed = t->c_ispeed;
	tp->t_ospeed = t->c_ospeed;
	tp->t_cflag = t->c_cflag;

	if(tp->t_ispeed == 0) {
		tp->t_state |= TS_HUPCLS;
		USPLX(s);
		return(error);
	}
	/*
	 * TODO:
	 *	Use parms from tty to set mode.
	 */
	if(!error) {
		LASSERT(TTY_LOCK_HOLDER(tp));
		mode=SL_UART_MODE(B9600,B9600,0,0,SL_1STOP,SL_8BIT);
		/*
	 	*  error = sl_setmode(unit, SL_ATTN_MODE, mode, &mode);
	 	*  if(!error) {
	 	*	if((mode & SLCHAN_DCD) != 0)
	 	*		tp->t_state |= CARR_ON;
	 	*   }
	 	*/
	   	tp->t_state |= TS_CARR_ON; /* 'til set mode works */
	}	    /* second create channel succeeded */
	USPLX(s);
	return(error);
}

/*2
 *
 * slclose
 *	Call the line discipline and tty close
 *	routines.  If the HUPCLS bit is set, also do a disconnect on the
 *	crqs.
 *
.* ARGUMENTS:
.*	dev	- terminal dev #
 *
.* RETURNS:
 *
.* USAGE:
 *	Called on an close system call through the character device switch.
 *
.* ASSUMPTIONS:
 *	none
 */
slcclose(dev)
dev_t	dev;
{
	register struct tty *tp;	/* pointer to tty */
	int	unit = minor(dev);
	int	mode;

	
	tp = SL_DEVTOTTY(unit);
	TTY_LOCK(tp);

	/*
	 *	if((tp->t_cflag&HUPCL) && !(tp->t_state&ISOPEN))) {
	 *		sl_setmode(dev, 0, 0, &mode);
	 *	}
	 */
	(*linesw[tp->t_line].l_close) (tp);
	ttyclose(tp);
	TTY_UNLOCK(tp);
}

/*2
 * slread
 *	Call through the line switch to read characters from the clists into
 *	the users buffer.
 *
.* ARGUMENTS:
.*	dev	- terminal major/minor #
 *
.* RETURN VALUE:
 *	E??	 - error returned by linesw routine
 *
.* USAGE:
 *	Called on an read system call through the character device switch.
 *
.* ASSUMPTIONS:
 *	none
 */

slcread(dev, uio, flag)
dev_t dev;
/**/
{
	register struct tty *tp;
	int ret_val;

	tp = SL_DEVTOTTY(minor(dev));
	TTY_LOCK(tp);
	ret_val = (*linesw[tp->t_line].l_read)(tp, uio, flag);
	TTY_UNLOCK(tp);
	return ret_val;
}

/*2
 * slcwrite
 *	Call through the linesw write routine to take characters from the
 *	users buffer and put them in the output queue.
 *
.* ARGUMENTS:
.*	dev	- terminal major/minor #
 *
.* RETURN VALUE:
 *	E??	 - error returned by linesw write routine
 *
.* USAGE:
 *	Called on an read system call through the character device switch.
 *
.* ASSUMPTIONS:
 *	none
 */

slcwrite(dev, uio, flag)
dev_t dev;
/**/
{
	register struct tty *tp;
	int ret_val;

	tp = SL_DEVTOTTY(minor(dev));
	TTY_LOCK(tp);
	ret_val = (*linesw[tp->t_line].l_write)(tp, uio, flag);
	TTY_UNLOCK(tp);
	return ret_val;
}


/*2
 * slcselect: Select system call
 *
.* ARGUMENTS:
 *
.* dev
 *	SCC serial line
.* sdp
 *	select descriptor
.* rw
 *	Flag indicating read and or write select.
 *
.* RETURNS:
 *	return value from ttselect.
 *	0 if caller will block on indicated request.
 *
.* USAGE:
 *	Called on a select system call through the character device switch.
 *
.* ASSUMPTIONS:
 */
slcselect(dev, events, revents, scanning)
dev_t dev;
short *events, *revents;
int scanning;
{
	register struct tty *tp;
	int	nread;
	USPLVAR(s)

	tp = SL_DEVTOTTY(minor(dev));
	USPLTTY(s);
	TTY_LOCK(tp);

	if (*events & POLLNORM)
		if (scanning) {
			nread = ttnread(tp);
			if (nread > 0 || (!(tp->t_cflag & CLOCAL)
			    && !(tp->t_state & TS_CARR_ON)))
				*revents |= POLLNORM;
			else 
				select_enqueue(&tp->t_selq);
		} else 
			select_dequeue(&tp->t_selq);

	if (*events & POLLOUT)
		if (scanning) {
			if (tp->t_outq.c_cc <= tp->t_lowat)
				*revents |= POLLOUT;
			else 
				select_enqueue(&tp->t_selq);
		} else
			select_dequeue(&tp->t_selq);

	if (scanning)
		if (!(tp->t_state & TS_CARR_ON))
			*revents |= POLLHUP;

	TTY_UNLOCK(tp);
	USPLX(s);
	return (0);

}


#if	0
/*2
 * slcunselect: Select system call
 *
.* ARGUMENTS:
 *
.* dev
 *	SCC serial line
.* proc
 *	process to unselect
.* rw
 *	Flag indicating read and or write select.
 *
.* RETURNS:
 *	return value from ttselect.
 *	0 if caller will block on indicated request.
 *
.* USAGE:
 *	Called on a select system call through the character device switch.
 *
.* ASSUMPTIONS:
 */
slcunselect(dev, proc, rw)
dev_t dev;
struct proc *proc;
int rw;
/**/
{
	register struct tty *tp;
#ifdef	BERKNET
	tp = SL_DEVTOTTY(dev);
	return(ttunselect(tp, proc, rw));
#else
	return(rw);
#endif
}
#endif	/* 0 */


/*2
 * slcioctl
 *	Handle ioctl requests for the scc lines.  Most of the work is done
 *	by ttioctl (tty.c).   Currently, no scc-specific ioctls are
 *	implemented.
 *
.* ARGUMENTS:
.*	dev	- terminal major/minor #
.*	cmd	- ioctl command
.*	data	- buffer of user r/w data for the cmd
.*	mode1	- unused
 *
.* RETURN VALUE:
 *	E??	 - error returned from ttiocom.
 *
.* USAGE:
 *	Called on an ioctl system call through the character device switch.
 *
.* ASSUMPTIONS:
 *	none
 */
slcioctl(dev, cmd, arg, mode1)
dev_t dev;
/**/
{
	register struct tty *tp;
	int	error;


	tp = SL_DEVTOTTY(minor(dev));
	TTY_LOCK(tp);
	error = (*linesw[tp->t_line].l_ioctl)(tp, cmd, arg, mode1);
	if(error >= 0) {
		TTY_UNLOCK(tp);
		return(error);
	}
	error = ttioctl(tp, cmd, arg, mode1);
	TTY_UNLOCK(tp);
	return(error);
}

typedef struct scc_interrupt
{
	queue_chain_t	si_chain;
	int		si_code;
	struct tty	*si_tty;
} scc_interrupt;

#define	NSCCINTRS	300
scc_interrupt	scc_intrs[NSCCINTRS];
mpqueue_head_t	scc_pend_intrs;
mpqueue_head_t	scc_free_intrs;
int		scc_uniprocessor = 1;


/*2
 * slcintr
 *	First level interrupt routine called from trap.s.
 *
.* ARGUMENTS:
 *	code	- encoded unit.
 *		- Unit u corresponds to codes 2*u and 2*u+1.
 *
.* RETURNS:
 *	none
 *
.* USAGE:
 *	This function was specified by the create_channel operations for
 *	both receive and transmit channels. It takes care of the locking 
 *	protocol and calls the interrupt routine.
 *	
.* ASSUMPTIONS:
 *	none
 */
slcintr(ihp)
ihandler_t *ihp;
{
	register int	code = ihp->ih_hparam[0].intparam;
	register int	unit;
	scc_interrupt	*si;

	if (scc_uniprocessor)
		panic ("slcintr:  scc_uniprocessor");

	mpdequeue1 (&scc_free_intrs, &si, QNOWAIT);
	if (si == 0)
		panic("scc:  no free interrupt structs\n");
	si->si_code = code;
	si->si_tty = (struct tty *) 0;
	mpenqueue1 (&scc_pend_intrs, si);
}


/*
 * Called from a clock interrupt.
 */
slcrstrt(tp)
struct tty *tp;
{
	scc_interrupt	*si;

	mpdequeue1 (&scc_free_intrs, &si, QNOWAIT);
	if (si == 0)
		panic("slcrstrt:  no free interrupt structs\n");
	si->si_tty = tp;
	mpenqueue1 (&scc_pend_intrs, si);
}

/*
**  Initialize the two queues, scc_pend_intrs and scc_free_intrs
**  to empty; then free the elemtns of scc_intrs[] onto scc_free_intrs.
*/
void
init_scc_intrs()
{
	register scc_interrupt	*x;

	scc_uniprocessor = 1;		/* disable thread-based SCC intrs */
	mpqueue_init(&scc_pend_intrs);
	mpqueue_init(&scc_free_intrs);
	for (x = scc_intrs; x < &scc_intrs[NSCCINTRS]; ++x)
		mpenqueue1 (&scc_free_intrs, x);
}

void
slcintr_thread()
{
	register int	unit;
	register int	code;
	scc_interrupt	*si;
	thread_t	thread;
	struct tty	*tp;

	init_scc_intrs();

	thread = current_thread();
	thread->priority = thread->sched_pri = 2;
	thread_swappable(thread, FALSE);
#if	MACH_XP
	thread->kernel_only = TRUE;
#endif

	scc_uniprocessor=0;
	for (;;)
	{
		mpdequeue1 (&scc_pend_intrs, &si, QWAIT);
		tp = si->si_tty;
		code = si->si_code;
		mpenqueue1 (&scc_free_intrs, si);

		/*
		 * Service interrupt.  Could be from a clock
		 * interrupt, too.
		 */

		if (tp) {
			ttrstrt(tp);
		} else {
			unit = code >> 1;
			if (IS_ODD(code))
				slcxint(&sccsl[unit]);
			else
				slcrint(&sccsl[unit]);
		}
	}
}


/*2
 * slcrint
 *	Terminal receiver interrupt service routine.  This routine accepts
 *	read and attention packets from the receive crq and processes them.
 *
.* ARGUMENTS:
 *	sp	- SCC control block pointer.
 *
.* RETURNS:
 *	none
 *
.* USAGE:
 *	Called via first level interrupt handler slcintr when
 *	the SCC has recieved data waiting.
 *	
.* ASSUMPTIONS:
 *	none
 */
slcrint(sp)
register sccsl_t	*sp;	/* pointer to units control buffer */
{
	register crq_t	*rcvcrq;
	register crq_msg_t *rsp = (crq_msg_t *)~NULL;
	register crq_msg_t *attn = (crq_msg_t *)~NULL;


	/*
 	 * As long as there is an attention or a read packet to receive, keep
 	 * the queue flowing...
 	 */
	rcvcrq = (crq_t *)&sp->rcv_crq;
	while((rsp != NULL) || (attn != NULL)) {

		/*
	 	 * Look first for attentions.
	 	 */
		if((attn = (crq_msg_t *)rec_attn(rcvcrq)) != NULL) {
			rcvcrq->crq_totattns++;
			if(attn->crq_msg_hdr.hdr_type == TYPE_SL_ATTN) {
				scc_attn(sp,attn);
			}
		}

		/*
	 	 * Now look for read packets.  If there is one, get the lock on
	 	 * the typahead buffer and insert the attention packet on the
	 	 * queue after updating crq statistics.
	 	 */
		if((rsp = (crq_msg_t *)rec_rsp(rcvcrq)) != NULL) {
			rcvcrq->crq_totrsps++;
			if(rsp->crq_msg_hdr.hdr_type == TYPE_SCC_RDPKT) {
				scc_read(sp,rsp);
			}
		}
	}
}


/*2
 * slcxint
 *	This is the transmit interrupt service routine.  It removes packets
 *	from the response queue of the transmit crq until there are none
 *	left. 
 *
.* ARGUMENTS:
 *	sp	- SCC contril block pointer.
 *
.* RETURNS:
 *	none
 *
.* USAGE:
 *	
 *	Called via first level interrupt handler slcintr when
 *	the SCC has has completed an output operation and is ready for more.
 *
.* ASSUMPTIONS:
 *	none
 */

slcxint(sp)
register sccsl_t	*sp;	/* pointer to units control buffer */
{
	register crq_t	*xmtcrq;
	register crq_msg_t *rsp;
	register char	*err_msg;
	register struct tty *tp;

	tp = SL_SPTOTTY(sp);
	TTY_LOCK(tp);
	xmtcrq = &sp->xmt_crq;
	while((rsp = (crq_msg_t *)rec_rsp(xmtcrq)) != NULL) {
		switch(rsp->crq_msg_hdr.hdr_type) {
		case(TYPE_SL_WRITE_MSG):
		    switch(rsp->crq_msg_refnum) {
			default:
			    TTY_UNLOCK(tp);
			    panic("slcxint: bad refnum");
			    break;
			case(SCCSL_REF_WRITE):
			    tp->t_state &= ~TS_BUSY;
			    if (tp->t_state & TS_FLUSH)
				tp->t_state &= ~TS_FLUSH;
			    ndflush(&tp->t_outq, sp->write_msg.sl_write_char_count);
			    if(tp->t_line)
				(*linesw[tp->t_line].l_start)(tp);
			    else
				slcstart(tp);
			    break;
			}
		    break;

		case(TYPE_ERRLOG_MSG):
			insque(rsp, xmtcrq->crq_rsp.dbl_fwd);
			break;

		case(TYPE_CRQ_ABORT_MSG):
		case(TYPE_SL_PAUSE_MSG):
		case(TYPE_SL_RESUME_MSG):
			put_free(rsp, xmtcrq);
			break;

		case(TYPE_SL_MODE_MSG):
			mpenqueue_tail(&sp->ioque, rsp);
			break;

		default:
			panic("Unknown element on sl response queue.");
			
		}
	}
	TTY_UNLOCK(tp);
}


/*****************************************************************************
 *
 * NAME:
 *	slcstop
 *
 * DESCRIPTION:
 *	
 *	Stop output on a line and/or flush typeahead.  The flag value 
 *	indicates what type of action is required:
 *			Operation			Flag value
 *		interrupt, quit (ie ^C)			    3
 *		flush output (^O)			    2
 *		throttle output (^S)			    0
 *
 *	NOTE - the SCC suppports a PAUSE command as well as an ABORT command.
 *	While PAUSE could implement ^S functions and ABORT implement ^C and ^O
 *	functions, for now all output is flushed via ABORT, since the high-
 *	level terminal code already has the intelligence to handle restart
 *	via ^Q.  Trying to handle it here via PAUSE/RESUME just complicates
 *	the driver.
 *
 * ARGUMENTS:
 *	tp	- pointer to the terminal's tty structure
 *	flag	- value indicating whether to stop input, output, or both
 *
 * RETURN VALUE:
 *	none
 *
 * SIDE EFFECTS:
 *	none
 *
 * EXCEPTIONS:
 *	none
 *
 * ASSUMPTIONS:
 *	none
 */
 
slcstop(tp, flag)
register struct tty *tp;
	int	flag;
{
	LASSERT(TTY_LOCK_HOLDER(tp));
}

/*2
 * slcstart
 *	This function starts(restarts) transmission on a line.
 *
 * ARGUMENTS:
 *	tp	- pointer to the terminal's tty structure
 */

slcstart(tp)
register struct tty *tp;
{
	register sccsl_t	*sp;
	register crq_sl_write_msg_t *msg;
	register int	cc, s;
	int	unit;
#if	!UNIX_LOCKS
	extern ttrstrt();
#endif

	sp = SL_TTYTOSP(tp);
	unit = sp - sccsl;

	LASSERT(TTY_LOCK_HOLDER(tp));
	USPLTTY(s);
	if (tp->t_state & (TS_BUSY|TS_TTSTOP|TS_TIMEOUT))
		goto out;
	if (tp->t_outq.c_cc <= tp->t_lowat) {
		if (tp->t_state&TS_ASLEEP) {
			tp->t_state &= ~TS_ASLEEP;
			thread_wakeup ((int) &tp->t_outq);
		}
		select_wakeup(&tp->t_selq);
	}
	if (tp->t_outq.c_cc == 0)
		goto out;
	if (tp->t_flags & (RAW|LITOUT))
		cc = ndqb(&tp->t_outq, 0);
	else {
		cc = ndqb(&tp->t_outq, 0200);
		if (cc == 0) {
			cc = getc(&tp->t_outq) & 0xff;
#if	UNIX_LOCKS
			timeout(slcrstrt, (caddr_t)tp, (cc&0x7f) + 6);
#else
			timeout(ttrstrt, (caddr_t)tp, (cc&0x7f) + 6);
#endif
			tp->t_state |= TS_TIMEOUT;
			goto out;
		}
	}

	msg = (crq_sl_write_msg_t *)&sp->write_msg;
	msg->sl_write_buff = (caddr_t)pmap_resident_extract(pmap_kernel(),
		tp->t_outq.c_cf);
	msg->sl_write_char_count = cc;
	send_cmd(msg, &sp->xmt_crq);
	tp->t_state |= TS_BUSY;
out:
	USPLX(s);
}
 
#ifdef	SL_SETMODE

/*2
 * sl_setmode
 *	Set the mode of a SCC UART.
 *
.* ARGUMENTS:
 *	dev	- terminal major/minor #
 *	attn	- mask of bits to set as the attention mode in which to operate
 *		  the line
 *	modem	- bits defining modem operation - 0 means "do the right thing"
 *	curr_mode - call by reference arg used to pass back SCC mode.
 *
.* RETURNS:
 *	Status of SCC message operation.	
 *
.* USAGE:
 *	Set the serial line into the correct operational mode as given by the
 *	parameters and the information in the tty structure.  This structure
 *	will have been locked by the caller.  This operation is synchronous.
 *	The internal per-line completion queue gets the completed message.
 *
.* ASSUMPTIONS:
 *	none
 */

slc_setmode(dev, attn, uart, curr_mode)
dev_t	dev;
int	attn, uart;
int	*curr_mode;
/**/
{
	register crq_t	*xmtcrq;
	register struct tty *tp;
	register crq_sl_mode_msg_t *msg;
	register sccsl_t	*sp;
	int	unit;
	int	sts;

	msg = (crq_sl_mode_msg_t *) malloc(ptovmap, sizeof(*msg));
	if(msg != NULL) {
	    unit = SL_GETUNIT(dev);
	    sp = &sccsl[unit];
	    xmtcrq = (crq_t *)&sp->xmt_crq;
	    tp = SL_DEVTOTTY(dev);
	    msg->sl_config_hdr.crq_msg_hdr.hdr_type= TYPE_SL_MODE_MSG;
	    msg->sl_config_hdr.crq_msg_code = CRQOP_SL_SET_MODE;
	    msg->sl_config_hdr.crq_msg_unitid = xmtcrq->crq_unitid;
	    msg->sl_config_mode.sl_mode_attn = attn;
	    msg->sl_config_mode.sl_mode_uart = uart;

	    send_cmd(msg, xmtcrq);
	    msg = (crq_sl_mode_msg_t *)get_isr_queue(&sp->ioque);
	    sts = msg->sl_config_hdr.crq_msg_status;
	    *curr_mode = msg->sl_config_status;
	    mfree(ptovmap, sizeof(*msg), msg);
	}
	return(sts);
}

/*2
 * sl_getmode
 *	Retrieve the current status of the line from the scc.  Like setmode,
 *	this operation completes synchronously.
 *
 * ARGUMENTS:
 *	dev	- terminal major/minor #
 *	ptr	- pointer to the longword to receive terminal status info
 *
 * RETURNS:
 *	
 * USAGE:
 *
 * ASSUMPTIONS:
 *	none
 */

slc_getmode(dev, ptr)
dev_t	dev;
int	*ptr;
/**/
{
/*
 *	For now, return fixed bag o' bits...
 */

	*ptr = SL_UART_MODE(B9600, B9600, 0, 0, SL_1STOP, SL_8BIT);
	return(0);
}
#endif

/*2
 * slputc
 *	Put a character out to the console device
 *
 * ARGUMENTS:
 *	c		- Character to output
 *
 * RETURNS:
 *	none
 *
 * USAGE:
 *	Used exclusively by printf (os/prf.c).
 *
 * ASSUMPTIONS:
 *	Mutual exclusion to SCC shared data is guaranteed by a lock in the 
 *	caller.
 */
slputc(c)
char	c;
/**/
{

	unsigned long maxwait;
	/*
	 * Set transmit request in SCC shared memory and load the byte.
	 * when SCC is ready.
	 */
	for(;;) {
		usimple_lock(&console_lock);
		maxwait = 100000;
		while ((*SCC_CONSOLE_XMTREQ) && (maxwait--));
		*SCC_CONSOLE_XMTBUF = c;
		*SCC_CONSOLE_XMTREQ = TRUE;
		usimple_unlock(&console_lock);
		if (c == '\n')
			c = '\r';
		else
			break;
	}
}

/*2
 * scc_read
 *	Transfer data from SCC data buffer to clist.
 *
.* ARGUMENTS:
.*	sp	- SCC control block pointer
.*	rsp	- SCC read message pointer
 *
.* RETURNS:
 *
.* USAGE:
 *	Called on an SCC receive interrupt.
 *
.* ASSUMPTIONS:
 *	
 */
scc_read(sp,rsp)
register sccsl_t	*sp;
register crq_sl_read_msg_t *rsp;
{
	register struct tty *tp = SL_SPTOTTY(sp);
	register char *bp = rsp->sl_read_data; 	/* pointer to data buffer */
	register c_cnt;
	register err_cnt;
	register status;
	u_char	c;

	TTY_LOCK(tp);
	switch(rsp->sl_read_hdr.crq_msg_status) {
	/*
	 * First, deal with the success cases.
	 * Get the character and error
	 * counts.  Get the next character out of the
 	 * buffer.
	 */

	case(STS_SUCCESS):
	case(STS_WARNING):
		err_cnt = rsp->sl_read_data_errors;

		for(c_cnt = 0; c_cnt < rsp->sl_read_data_actual; c_cnt++) {

			c = *bp++;
			/*
			 * If there are more characters with
			 * errors, see if this is one of them.
			 * If so, process framing errors
			 * according to the line state.
			 * Process parity characters depending
			 * on whether or not parity is enabled.
			 */

			if (!(tp->t_state&(TS_ISOPEN|TS_WOPEN)))
				continue;
			/* Handle errors */
			if(err_cnt > 0 && (status & SCC_ERR_INDEX) == c_cnt) {

				status = *rsp->sl_read_error;
				if (status&SCC_ERR_PE)
					if((tp->t_flags & (EVENP|ODDP)) == EVENP ||
						(tp->t_flags & (EVENP|ODDP)) == ODDP)
							continue;
						
				if (status&SCC_ERR_FE)
					if(tp->t_flags & RAW)
						c = 0;
					else
						c = tp->t_cc[VINTR];
			}
			(*linesw[tp->t_line].l_rint)(c, tp);
		}
		break;	/* End success cases */

	/*
	 * If we get an abort, it's presumably because we asked for it.
	 * Just ignore it.
	 */
	case(STS_ABORTED):
	    break;

	case STS_ERROR:
		/* 
		 * Seems to indicate a break. No documetation.
		 */
		if((tp->t_flags & RAW) == 0)
			(*linesw[tp->t_line].l_rint)(tp->t_cc[VINTR], tp);
		break;
	/*
	 * Anything else is an error.
	 */
	default:
	    TTY_UNLOCK(tp);
	    printf("scc_read: unknown msg status 0x%x  on unit %d\n",
			rsp->sl_read_hdr.crq_msg_status, sp-sccsl);
	    TTY_LOCK(tp);
	    break;

	}	/* End of read packet switch */
	/*
	 * Put read command back on command queue.
	 */
	send_cmd(rsp, &sp->rcv_crq);
	TTY_UNLOCK(tp);
}

/*2
 * scc_attn
 *	Process attentions from the SCC
 *
.* ARGUMENTS:
.*	sp	- SCC control block pointer
.*	attn	- SCC attention message pointer
 *
.* RETURNS:
 *
.* USAGE:
 *	Called on an SCC receive interrupt.
 *
.* ASSUMPTIONS:
 *	
 */
scc_attn(sp,attn)
register sccsl_t	*sp;
register crq_sl_attn_msg_t *attn;
{
	register struct tty *tp = SL_SPTOTTY(sp);
	if ((attn->sl_attn_status & SLCHAN_DCD) &&
	    (tp->t_state & TS_WOPEN)) {
		thread_wakeup((int)&tp->t_canq);
	}
	if ((attn->sl_attn_status & SLCHAN_READ_BUFFER_OVERFLOW) != 0) {
		printf("scc_attn: ATTENTION -- scc buffer overflow on line %d\n"
						,sccsl-sp);
	}
	put_free(attn, sp->rcv_crq);
}
