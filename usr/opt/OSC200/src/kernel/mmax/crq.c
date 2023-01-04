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
static char	*sccsid = "@(#)$RCSfile: crq.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:18 $";
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

/*1
 * crq.c
 *
 * 	This module contains a set of command/response queue primitives used 
 *	primarily by device driver code and system code that needs to 
 *	communicate with I/O devices.
 */

#include <mmax_xpc.h>
#include <mmax_apc.h>
#include <mmax_dpc.h>

#include <sys/param.h>
#include <sys/user.h>
#include <sys/sysconfig.h>
#include <kern/queue.h>
#include <mmaxio/io.h>
#include <mmaxio/crqdefs.h>
#include <mmax/cpudefs.h>
#include <mmax/sccdefs.h>
#include <mmax/isr_env.h>
#include <mmax/handler.h>
#if	MMAX_XPC || MMAX_APC
#include <mmax/icu.h>
#endif
#include <emc.h>
#include <msd.h>

#include <kern/assert.h>

crq_msg_t * get_isr_queue();

crq_t *Slot_crqs[NUM_SLOT];

crq_msg_t *rec_polled_rsp();
void	restore_mode(), spurious_polled_rsp();

/*
 * Stray interrupt message.
 */

static	iostray (chdev)
{
	printf ("stray interrupt - channel: %d, device: %d, cpu: %d\n",
		chdev >> 8, chdev & 0xFF, getcpuid());
}

static	crq_create_chan_msg_t create_chan_cmd;
static	mpqueue_head_t	create_chan_q;
static	lock_data_t	create_chan_lock;

static	crq_delete_chan_msg_t delete_chan_cmd;
static	mpqueue_head_t	delete_chan_q;
static	lock_data_t	delete_chan_lock;

static	crq_warm_msg_t	warm_cmd;
static	mpqueue_head_t	warm_q;
static	lock_data_t	warm_lock;

/*2
.* initcrq - initialize CRQ communication
 *
.* ARGUMENTS: None
 *
.* USAGE:
 *	Called once during initialization to establish CRQs.
 */

initcrq()
{
	mpqueue_init(&create_chan_q);
	lock_init(&create_chan_lock, TRUE);

	mpqueue_init(&delete_chan_q);
	lock_init(&delete_chan_lock, TRUE);

	mpqueue_init(&warm_q);
	lock_init(&warm_lock, TRUE);
}


/*2
.* init_crq - initialize a command/response queue
 *
.* ARGUMENTS:
 *
.* crq - Pointer to the CRQ to be initialized.
 *
.* mode - CRQ mode for the crq_mode field.
 *
.* opt - CRQ options for the crq_opt field.
 *
.* unitid - CRQ unit identifier of slave for the crq_unitid field.
 *
.* stats - Pointer to CRQ statistics block (if proper option in crq_opt
 *	field is set.
 *
.* USAGE:
 *	This routine is called to initialize a CRQ after the CRQ has
 *	been allocated. It must be done for every CRQ before doing
 *	a create_chan() on the CRQ.
 */

init_crq (crq, mode, opt, unitid, stats)
register crq_t	*crq;
char		mode;
char		opt;
short		unitid;
crq_stats_t	*stats;
{

	/*
	 * Initialize crq links and its header.
	 */
	crq->crq_links.dbl_fwd = (dbl_link_t *) crq;
	crq->crq_links.dbl_bwd = (dbl_link_t *) crq;
	crq->crq_hdr.hdr_type = TYPE_CRQ;
	crq->crq_hdr.hdr_size = sizeof(crq_t);

	/*
	 * Initialize the CRQ lock.
	 */
	crq_lock_init(&crq->crq_slock);

	/*
	 * Set up mode, options, and unit identifier.
	 */
	crq->crq_mode = mode;
	crq->crq_opt = opt;
	crq->crq_unitid = unitid;

	/*
	 * Initialize the message queues as empty.
	 */
	crq->crq_cmd.dbl_fwd = &crq->crq_cmd;
	crq->crq_cmd.dbl_bwd = &crq->crq_cmd;
	crq->crq_immedcmd.dbl_fwd = &crq->crq_immedcmd;
	crq->crq_immedcmd.dbl_bwd = &crq->crq_immedcmd;
	crq->crq_rsp.dbl_fwd = &crq->crq_rsp;
	crq->crq_rsp.dbl_bwd = &crq->crq_rsp;
	crq->crq_attn.dbl_fwd = &crq->crq_attn;
	crq->crq_attn.dbl_bwd = &crq->crq_attn;
	crq->crq_free.dbl_fwd = &crq->crq_free;
	crq->crq_free.dbl_bwd = &crq->crq_free;

	/*
	 * Initialize the default statistics
	 */
	crq->crq_totcmds =
	 crq->crq_totrsps =
	 crq->crq_totattns =
	 crq->crq_totvects = 0;

	/*
	 * Initialize the optional statistics.
	 */
	if (crq->crq_opt & CRQOPT_EXTSTATS) {
		crq->crq_stats = stats;
		stats->crs_maxcmd =
		 stats->crs_curcmd =
		 stats->crs_maxrsp =
		 stats->crs_currsp =
		 stats->crs_maxattn =
		 stats->crs_curattn =
		 stats->crs_maxfree =
		 stats->crs_curfree =
		 stats->crs_vectattempts = 0;
	} else {
		crq->crq_stats = (crq_stats_t *) NULL;
	}

}

/*2
.* create_chan - this function creates an I/O communications channel.
 *
.* ARGUMENTS:
 *
.* crq - Pointer to an initialized CRQ to be used by the channel.
 *
.* isr - Pointer to an interrupt service routine for this channel.
 *
.* param - Parameter to pass to isr.
 *
.* name - Pointer to channel name.
 *
.* USAGE:
 *	This routine is used to create communications channels between
 *	the DPC and either an SCC or an EMC card on a MULTIMAX. It is 
 *	called numerous times to create these channels by EMC and SCC 
 *	drivers. On non-MULTIMAX systems it is used to create a
 *	CRQ to be used for communication between layers of hardware -
 *	i.e. to create channels for multiple disks belonging to one
 *	disk controller.
 *
.* ASSUMPTIONS:
 *	The CRQ has been initialized via init_crq.
 */
create_chan(crq)
register crq_t	*crq;
{
	int	error = 0;

	lock_write(&create_chan_lock);	/* lock the routine */
	/*
	 * Initialize the create channel message. The reference number is set
	 * to the address of the per process I/O queue that the SCC Requester
	 * CRQ interrupt service routine will queue the response message.
	 */
	create_chan_cmd.creat_chan_hdr.crq_msg_code = CRQOP_CREATE_CHANNEL;
	create_chan_cmd.creat_chan_hdr.crq_msg_refnum = (long) &create_chan_q;
	create_chan_cmd.creat_chan_crq = crq;
	/*
	 * Send the create channel command message to the slot level CRQ
	 * on which the unit exists.
	 */
	send_slot_cmd(&create_chan_cmd, GETSLOT(crq->crq_unitid));
	/*
	 * Synchronize with completion
	 */
	if (crq->crq_mode & CRQMODE_INTR) {
		if((crq_create_chan_msg_t *)get_isr_queue(&create_chan_q)
			!= &create_chan_cmd)
				error = EIO;
	}
	else {
		if((crq_create_chan_msg_t *)rec_polled_rsp(REQ_CRQ(SCC_SLOT,0))
			!= &create_chan_cmd)
				error = EIO;
	}

	/*
	 * Verify that the channel was successfully created.
	 */
	if (create_chan_cmd.creat_chan_hdr.crq_msg_status != STS_SUCCESS)
		error = EIO;

	/*
	 * Just return on any error.
	 */
err:
	lock_done(&create_chan_lock);	/* unlock the routine */
	return (error);
}

/*2
.* delete_chan - given an existing CRQ, delete the associated channel.
 *
.* ARGUMENTS:
 *
.* crq - Pointer to the CRQ whose channel is to be deleted.
 *
.* USAGE:
 *	Called when closing down communications channels with
 *	either the SCC or the EMC on MULTIMAX systems, or closing
 *	down a channel between disks/tapes and a controller.
 */

delete_chan(crq)
register crq_t	*crq;
{
	int	error = 0;

	lock_write(&delete_chan_lock);	/* lock the routine */

	/*
	 * Initialize the delete channel message. The reference number is set
	 * to the address of the per process I/O queue that the SCC Requester
	 * CRQ interrupt service routine will queue the response message.
	 */
	delete_chan_cmd.del_chan_hdr.crq_msg_code = CRQOP_DELETE_CHANNEL;
	delete_chan_cmd.del_chan_hdr.crq_msg_refnum = (long) &delete_chan_q;
	delete_chan_cmd.del_chan_crq = crq;

	/*
	 * Send the delete channel command message to the slot level CRQ
	 * on which the unit exists.
	 */
	send_slot_cmd(&delete_chan_cmd, GETSLOT(crq->crq_unitid));

	/*
	 * Synchronize with completion
	 */
	if ((crq_delete_chan_msg_t *) 
		get_isr_queue(delete_chan_cmd.del_chan_hdr.crq_msg_refnum) ==
		&delete_chan_cmd) {

		/*
		 * Verify that the channel was successfully deleted.
		 */
		if (delete_chan_cmd.del_chan_hdr.crq_msg_status != STS_SUCCESS)
			error = EIO;
		else {
			/*
			 * Assumes that associated vector has already been
			 * deallocated.
			 */
			crq->crq_master_vect = VEC_INVALID;
		}

	} else
		error = EIO;
	lock_done(&delete_chan_lock);	/* lock the routine */
	return(error);
}

/*2
.* warm_restart - Issue a warm restart to the specified I/O controller.
 *
.* ARGUMENTS:
 *
.* slot - Slot number of the I/O controller.
 *
.* USAGE:
 * 	Issue a warm restart to the specified I/O controller. A warm restart
 *	command causes the controller to re-initialize including 'forgetting'
 *	any and all I/O channels and operations. Used when rebooting the
 *	system and in the standalone system when about to boot the OS.
 */

warm_restart(slot)
int	slot;
{
int	error = 0;

	lock_write(&warm_lock);	/* lock the routine */

	/*
	 * Initialize the warm restart message.
	 */
	warm_cmd.warm_hdr.crq_msg_code = CRQOP_WARM;
	warm_cmd.warm_hdr.crq_msg_refnum = (long) &warm_q;

	/*
	 * Sending a warm restart command is always done via the requester
	 * CRQ on the MULTIMAX. On other systems there is no requestor
	 * CRQ, so the command is sent directly to the slot CRQ.
	 */
	send_req_cmd(&warm_cmd, slot);

	/*
	 * Synchronize with completion
	 */
	if ((crq_warm_msg_t *)
		get_isr_queue(warm_cmd.warm_hdr.crq_msg_refnum) == &warm_cmd) {

		/*
		 * Verify successful warm restart
		 */
		if (warm_cmd.warm_hdr.crq_msg_status != STS_SUCCESS)
			error = ENXIO;
	} else
		error = EIO;

	lock_done(&warm_lock);	/* unlock the routine */
	return(error);
}

/*2
.* send_slot_cmd - This function sends a specified command to a specified 
 *	slot.
 *
.* ARGUMENTS:
 *
.* cmd - Pointer to a CRQ_MSG that is a command
 *
.* slot - Slot number to send a command to.
 *
.* USAGE:
 *	This function sends a specified command to a specified slot. On a
 *	MULTIMAX, if the slot does not yet have a slot-level channel/CRQ 
 *	associated, the message is routed through the SCC Requester Slot.
 *	On other systems, there must be as slot-level channel/CRQ
 *	associated with the slot.
 *
.* ASSUMPTIONS:
 *	The slot number is a valid slot number. On non-MULTIMAX systems,
 *	there must be a slot-level CRQ associated with the slot.
 */

send_slot_cmd(cmd, slot)
crq_msg_t *cmd;
int	slot;
{

	register crq_t	*crq;

	assert(slot <= NUM_SLOT);

	/*
	 * Determine which CRQ to use based on existance. If the slot CRQ
	 * doesn't exist, use the SCC Requester slot. We also set the unitid
	 * of the channel that will process the command based on the CRQ that
	 * we use.
	 */
	if (Slot_crqs[slot] == NULL) {
		crq = REQ_CRQ(SCC_SLOT, 0);
		cmd->crq_msg_unitid = MAKEUNITID(0, slot, 0, REQ_LUN);
	}
	else {
		crq = Slot_crqs[slot];
		cmd->crq_msg_unitid = MAKEUNITID(0, slot, 0, SLOT_LUN);
	}

	/*
	 * Send the message and return to caller. The caller must synchronize
	 * with completion.
	 */
	send_cmd(cmd, crq);
}

/*2
.* send_req_cmd This function sends a specified command to a specified 
 *	slot.
 *
.* ARGUMENTS:
 *
.* cmd - Pointer to a CRQ_MSG that is a command
 *
.* slot - Slot number to send the command to
 *
.* USAGE:
 *	This function sends a specified command to a specified slot. Unlike
 *	send_slot_cmd, the message is always routed through the SCC Requester
 *	Slot.
 *
 * ASSUMPTIONS:
 *	The slot number is a valid slot number. This routine is only
 *	used by MULTIMAX systems.
 */

send_req_cmd(cmd, slot)
crq_msg_t *cmd;
int slot;
{

	register crq_t *crq;

	assert(slot <= NUM_SLOT);

	/*
	 * Send the message and return to caller. The caller must synchronize
	 * with completion.
	 */
	crq = REQ_CRQ(SCC_SLOT, 0);
	cmd->crq_msg_unitid = MAKEUNITID(0, slot, 0, REQ_LUN);
	send_cmd(cmd, crq);
}

/*2
.* send_cmd - this functions sends the specified command to the slave
 *	associated with the specified CRQ.
 *
.* ARGUMENTS:
 *
.* cmd - Pointer to message (command) to send
 *
.* crq - Pointer crq for channel to send over
 *
.* USAGE:
 *	This function performs the master side of the send command message
 *	protocol.
 */

send_cmd(cmd, crq)
register crq_msg_t *cmd;
register crq_t *crq;
{
	register crq_stats_t *stats;
	int i;
	unsigned s;

	/*
	 * Fill in remaining Message fields (only a backpointer to the CRQ
	 * and timestamp).  The caller must do the rest.
	 */
	cmd->crq_msg_crq = crq;
	cmd->crq_msg_timestamp[0] = 0;
	cmd->crq_msg_timestamp[1] = 0;

	/*
	 * Lock the CRQ.
	 */
	s = spl7();
	crq_lock(&crq->crq_slock);

	/*
	 * Insert the command on the command queue. We need to remember
	 * whether we are adding a command to an empty queue.
	 */
	i = ((dbl_link_t *)&crq->crq_cmd.dbl_fwd == crq->crq_cmd.dbl_fwd);
	insque(cmd, crq->crq_cmd.dbl_bwd);
	cmd->crq_msg_status = STS_QUEUED;

	/*
	 * Update statistics
	 */
	crq->crq_totcmds++;
	if ((i) && (crq->crq_mode == CRQMODE_INTR))
		crq->crq_totvects++;
	if(crq->crq_opt == CRQOPT_EXTSTATS) {
		stats = crq->crq_stats;
		if(++stats->crs_curcmd > stats->crs_maxcmd)
			stats->crs_maxcmd++;
	}
	/*
	 * Release lock
	 */
	crq_unlock(&crq->crq_slock);

	/*
	 * If the CRQ is an interrupt-driven CRQ and the queue was previously
	 * empty, send the slave an interrupt vector.
	 */
	if (i)
		send_vector(&crq->crq_slave_vect);
	splx(s);

	/*
	 * For benefit of 4.2 network emulation, return 0
	 */
	return(0);
}

/*2
.* send_immedcmd - this function sends an immediate command to the slave
 *	processor associated with a CRQ.
 *
.* ARGUMENTS:
 *
.* cmd - Pointer to message (command) to send
 *
.* crq - Pointer to crq for channel to send over
 *
.* USAGE:
 *	This function performs the master side of the send command message
 *	protocol for the immediate command queue.
 */

send_immedcmd(cmd, crq)
register crq_msg_t *cmd;
register crq_t *crq;
{
	register crq_stats_t *stats;
	int i;
	unsigned s;

	/*
	 * Fill in remaining Message fields (only a backpointer to the CRQ
	 * and timestamp).  The caller must do the rest.
	 */
	cmd->crq_msg_crq = crq;
	cmd->crq_msg_timestamp[0] = 0;
	cmd->crq_msg_timestamp[1] = 0;

	/*
	 * Lock the CRQ.
	 */
	s = spl7();
	crq_lock(&crq->crq_slock);

	/*
	 * Insert the command on the command queue. We need to remember
	 * whether we are adding a command to an empty queue.
	 */
	i = ((dbl_link_t *)&crq->crq_immedcmd.dbl_fwd
	 			== crq->crq_immedcmd.dbl_fwd);
	insque(cmd, crq->crq_immedcmd.dbl_bwd);
	cmd->crq_msg_status = STS_QUEUED;

	/*
	 * Update statistics
	 */
	crq->crq_totcmds++;
	if ((i) && (crq->crq_mode == CRQMODE_INTR))
		crq->crq_totvects++;
	if(crq->crq_opt == CRQOPT_EXTSTATS) {
		stats = crq->crq_stats;
		if(++stats->crs_curcmd > stats->crs_maxcmd)
			stats->crs_maxcmd++;
	}

	/*
	 * Release lock
	 */
	crq_unlock(&crq->crq_slock);

	/*
	 * If the CRQ is an interrupt-driven CRQ and the queue was previously
	 * empty, send the slave an interrupt vector.
	 */
	if ((i) && (crq->crq_mode == CRQMODE_INTR))
		send_vector(&crq->crq_slave_vect);
	splx(s);

	/*
	 * For benefit of 4.2 network emulation, return 0
	 */
	return(0);
}

/*2
.* rec_rsp - receive a response from a CRQ.
 *
.* ARGUMENTS:
 *
.* crq - Pointer to the CRQ the response is to be received from.
 *
.* RETURN VALUE:
 *	Pointer to a response that was queued. Null if no response is 
 *	queued.
 *
.* USAGE:
 *	This routine is used to remove the response from the CRQ by
 *	interrupt service routines.
 */

crq_msg_t *
rec_rsp(crq)
register crq_t	*crq;
{

	crq_msg_t *rsp;
	unsigned s;

	/*
	 * Lock the CRQ
	 */
	s = spl7();
	crq_lock(&crq->crq_slock);

	/*
	 * If a response is queued, dequeue the response, update statistics,
	 * and return a pointer to it. Otherwise, return a NULL.
	 */
	if (crq->crq_rsp.dbl_fwd == (dbl_link_t *)(&crq->crq_rsp.dbl_fwd))
		rsp = NULL;
	else
		switch(((crq_msg_t *)crq->crq_rsp.dbl_fwd)->crq_msg_status) {

		case STS_FREE:
		case STS_QUEUED:
		case STS_PENDING:
			printf("Rcv_rsp: Status= %d\n",
			   ((crq_msg_t *)crq->crq_rsp.dbl_fwd)->crq_msg_status);
			crq_unlock(&crq->crq_slock);
			splx(s);
			panic("Rcv_rsp: response queued with bad status.");
			break;

		default:
			rsp = (crq_msg_t *)remque(crq->crq_rsp.dbl_fwd);
			crq->crq_totrsps++;
			if(crq->crq_opt == CRQOPT_EXTSTATS)
				crq->crq_stats->crs_currsp--;
			break;
		}

	/*
	 * Unlock the CRQ lock.
	 */
	crq_unlock(&crq->crq_slock);
	splx(s);
	
	return(rsp);
}

/*2
.* rec_polled_rsp - Receive a response from a CRQ by polling a CRQ
 *
.* ARGUMENTS:
 *
.* crq - Pointer to the CRQ to be polled
 *
.* RETURN VALUE:
 *	Pointer to a response from the CRQ. If we poll for a "long time"
 *	then without a response, NULL is returned. ("Long time" is
 *	3000 retries with a 10000 iteration loop to waste time between
 *	retries.
 *
.* USAGE:
 *	Receive a response by polling a CRQ. Wait a long time for an
 *	item to appear on the queue (i.e. do not wait forever). Used for
 *	waiting for an EMC/SCC to respond to a command for CRQs that
 *	are not interrupt-driven.
 *
.* ASSUMPTIONS:
 *	Truly nonsense for non-MULTIMAX systems that do not have I/O
 *	processors.
 */

#if	MMAX_XPC
static int polldelay = 30000;
static int pollcount = 10000;
#endif

#if	MMAX_APC || MMAX_DPC
static int polldelay = 10000;
static int pollcount = 3000;
#endif

crq_msg_t *
rec_polled_rsp(crq)
register crq_t *crq;
{

	crq_msg_t *rsp;
	register int i, j;
	unsigned s;
	int	opcode, unitid, refnum;

	/*
	 * Wait for a long time for i to goto zero.  If a response is queued
	 * in the meantime, dequeue it and return a pointer.  Otherwise, the
	 * default is to return a NULL.
	 */
	for(i = pollcount; i > 0; i--) {

		/*
		 * Lock the CRQ
		 */
		s = spl7();
		crq_lock(&crq->crq_slock);
		if(crq->crq_attn.dbl_fwd != 
				(dbl_link_t *)(&crq->crq_attn.dbl_fwd)) {
			rsp = (crq_msg_t *)remque(crq->crq_attn.dbl_fwd);
			j = rsp->crq_msg_code;
			opcode = (int) rsp->crq_msg_code;
			unitid = (int) rsp->crq_msg_unitid;
			refnum = (int) rsp->crq_msg_refnum;
			insque((int *)rsp, (int *)crq->crq_free.dbl_bwd);
			printf("...drained attention from unit 0x%x, code 0x%x, unitid 0x%x, refnum 0x%x\n",
			       crq->crq_unitid, opcode, unitid, refnum);
		}
		if(crq->crq_rsp.dbl_fwd == 
				(dbl_link_t *)(&crq->crq_rsp.dbl_fwd)) {
			crq_unlock(&crq->crq_slock);
			splx(s);
			for(j = polldelay; j > 0; j--)	/* Wait for resp */
				;
		} else {
			rsp = (crq_msg_t *)remque(crq->crq_rsp.dbl_fwd);
			crq->crq_totrsps++;
			if(crq->crq_opt == CRQOPT_EXTSTATS)
				crq->crq_stats->crs_currsp--;
			crq_unlock(&crq->crq_slock);
			splx(s);
			return(rsp);
		}
	}
	return(NULL);
}

/*2
.* rec_attn - receive an attention from the specified CRQ.
 *
.* ARGUMENTS:
 *
.* crq- Pointer to the CRQ to receive the attention from.
 *
.* RETURN VALUE:
 *	Pointer to attention command that was received. NULL if there
 *	was no attention command on the CRQ.
 *
.* USAGE:
 *	This function is used to receive an attention from the
 *	specified CRQ (if it is present at the time of the call). It is
 *	used by interrupt service routines to remove attentions
 *	responses from the queue.
 */

crq_msg_t *
rec_attn(crq)
register crq_t *crq;
{

	crq_msg_t *attn;
	unsigned s;

	/*
	 * Lock the CRQ
	 */
	s = spl7();
	crq_lock(&crq->crq_slock);

	/*
	 * If an attention is queued, dequeue the attention and return a
	 * pointer to it. Otherwise, return a NULL.
	 */
	if (crq->crq_attn.dbl_fwd == (dbl_link_t *)(&crq->crq_attn.dbl_fwd))
		attn = NULL;
	else {
		attn = (crq_msg_t *)remque(crq->crq_attn.dbl_fwd);
		crq->crq_totattns++;
		if(crq->crq_opt == CRQOPT_EXTSTATS)
			crq->crq_stats->crs_curattn--;
	}

	/*
	 * Unlock the CRQ
	 */
	crq_unlock(&crq->crq_slock);
	splx(s);

	return(attn);
}


/*2
.* put_free - put a message on the free queue
 *
.* ARGUMENTS:
 *
.* msg - Pointer to the message (command) to free
 *
.* crq - Pointer to the CRQ whose free list the message is to be put on.
 *
.* USAGE:
 *	Put the specified message (command) on the free queue of the
 *	specified CRQ. This routine is used by interrrupt service routines
 *	to put attention messages back on the free queue after they have
 *	been processed.
 */

put_free(msg, crq)
register crq_msg_t *msg;
register crq_t *crq;
{
	register crq_stats_t *stats;
	unsigned s;

	msg->crq_msg_status = STS_FREE;
	/*
	 * Lock the CRQ.
	 */
	s = spl7();
	crq_lock(&crq->crq_slock);

	/*
	 * Queue the free message to the free queue and update statistics.
	 */
	insque(msg, crq->crq_free.dbl_bwd);
	if(crq->crq_opt == CRQOPT_EXTSTATS) {
		stats = crq->crq_stats;
		if(++stats->crs_curfree > stats->crs_maxfree)
			stats->crs_maxfree++;
	}

	/*
	 * Unlock the CRQ.
	 */
	crq_unlock(&crq->crq_slock);
	splx(s);
}

/*2
.* get_free - get a message from the free queue of a CRQ.
 *
.* ARGUMENTS:
 *
.* crq - Pointer to the CRQ whose free queue is to be used.
 *
.* RETURN VALUE:
 *	Returns a pointer to the message (command) that
 *	was removed from the free queue of the CRQ. If the
 *	free queue of the CRQ is empty, NULL is returned.
 *
.* USAGE:
 *	This routine is used to get a message from the free
 *	queue of the specified CRQ. Unknown who calls this.
 */

crq_msg_t *
get_free(crq)
register crq_t *crq;
{

	register crq_msg_t *free;
	unsigned s;

	/*
	 * Lock the CRQ.
	 */
	s = spl7();
	crq_lock(&crq->crq_slock);

	/*
	 * Dequeue a free message or return a null pointer.
	 */
	if (crq->crq_free.dbl_fwd == (dbl_link_t *)(&crq->crq_free.dbl_fwd))
		free = NULL;
	else
		free = (crq_msg_t *)remque(crq->crq_free.dbl_fwd);
		if(crq->crq_opt == CRQOPT_EXTSTATS)
			crq->crq_stats->crs_curfree--;

	/*
	 * Unlock the CRQ
	 */
	crq_unlock(&crq->crq_slock);
	splx(s);

	return(free);
}

/*2
.* put_isr_queue - Put a response on the isr queue and wake someone up.
 *
.* ARGUMENTS:
 *
.* rsp - response that is to be put into the ISR queue.
 *
.* queue - isr_queue that the response is to be put upon and which has
 *	the semaphore to wake someone up.
 *
.* USAGE:
 *	This routine is used by interrupt service routines to put
 *	a response to a command on the isr_queue and wake up the
 *	proc waiting for this response. This is used by interrupt
 *	service routines for commands that use the isr_queue interface.
 *
.* ASSUMPTIONS:
 *	The proc is waiting (or about to wait) via a get_isr_queue()
 *	call.
 */

put_isr_queue(rsp, queue)
crq_msg_t *rsp;
isr_queue_t *queue;
{
	mpenqueue1(queue, rsp);
}


/*2
.* get_isr_queue - Get a response from the isr queue once there is
 *	something on the queue.
 *
.* ARGUMENTS:
 *
.* queue - isr_queue that the response is to be put upon and which has
 *	the semaphore to wake someone up.
 *
.* RETURN VALUE:
 *	The response that was put onto the queue is returned. In the
 *	unlikely event that this proc was woken up by mistake (i.e.
 *	if something is broken), then NULL will be returned.
 *
.* USAGE:
 *	This routine is used by SCC/EMC command requestors (on a
 *	MULTIMAX) and disk/tape drivers (on a non-MULTIMAX) to wait
 *	for a response (if they decide to use the isr_queue
 *	interface).
 */

crq_msg_t *
get_isr_queue(queue)
isr_queue_t *queue;
{
	crq_msg_t *rsp;

	mpdequeue1(queue, &rsp, QWAIT);
	return(rsp);
}

/*
 * Private array of ihandler structs for use by alloc_vector()/
 * dealloc_vector() interface to itable.
 *
 * We take a guess here at the maximum number we may need, which
 * we take to be the sum of:
 *	NEMC - one per EMC/MSC slot
 *	NMSD - one per potential EMC/MSC disk
 *	2 - for SCC slot/master CRQ's
 *	8 - 2 for each of 4 SCC serial lines
 *	4 - for other clients (real-time clock, soft clock, remote ASTs,
 *	    physical page mapper)
 */
#define NIHENTS	(NEMC+NMSD+2+8+4)

ihandler_t ihandlers[NIHENTS];
simple_lock_data_t ihandlers_lock;

/*2
.* alloc_vector - allocate an interrupt vector in the interrupt table.
 *
.* ARGUMENTS:
 *
.* vector - output parameter. Pointer to some space for the interrupt
 *	vector that is to be filled in.
 *
.* isr - interrupt service routine for this interrupt vector.
 *
.* param - parameter that is to be passed to the interrupt service routine
 *	when an interrupt occurs.
 *
.* type - type of interrupt generated for this ISR.
 *
 *
.* USAGE:
 *	This routine is used by create_chan() and other, special-purpose
 *  clients, to allocate an interrupt vector for a CRQ they wish to
 *	create. On MULTIMAX systems, the interrupt is allocated in the
 *	"real" interrupt table (itable) from which interrupt service
 *  routines are directly called. On non-MULTIMAX systems, the
 *	interrupt is allocated in the pseudo-interrupt table (pseudoitbl),
 *	which is used by the low-level (i.e. controller) interrupt
 *	routines to call higher level (i.e. device) interrupt routines.
 */
alloc_vector(vector, isr, param, type)
long *vector;
void (*isr)();
unsigned int param;
int type;
{
	extern int sys_resolve();
	register ihandler_t *pih;
	register i;
	ihandler_id_t *pid;
	static char firstcall = 1;

	if (firstcall) {
		simple_lock_init(&ihandlers_lock);
		pih = ihandlers;
		for (i = 0; i < NIHENTS; ++i, ++pih)
			pih->ih_handler = NULL;
		firstcall = 0;
	}
	/*
	 * Lock the table against any updates.
	 */
	simple_lock(&ihandlers_lock);
	/*
	 * Go through the table looking for an empty slot. If one is
	 * found then fill in the itbl entry and the interrupt
	 * vector.
	 */
	pih = ihandlers;
	for (i = 0; i < NIHENTS; i++, pih++)
		if (pih->ih_handler == NULL) {
			bzero(pih, sizeof (*pih));
			pih->ih_handler = isr;
			pih->ih_hparam[0].intparam = (int)param;
			pih->ih_resolver = sys_resolve;
			pih->ih_rparam[0].intparam = IH_VEC_DYNAMIC_OK;
			pih->ih_flags = IH_VEC_PASS_ISP;
			pih->ih_next = NULL;
#if	SER_COMPAT
			pih->ih_funnel = FUNNEL_NULL;
#endif
			pih->ih_stats.ihs_type = type;
			simple_unlock(&ihandlers_lock);
			break;
		}
	if (NIHENTS <= i)
		printf("alloc_vector: no local ihandler\n");
	else {
		if ((pid = handler_add(pih)) != NULL) {
			if (handler_enable(pid) == 0) {
				MAKEVECTOR(vector, MASTER_CLASS, pih->ih_level);
				return(0);
			}
			(void)handler_del(pid);
		}
		simple_lock(&ihandlers_lock);
		pih->ih_handler = NULL;
	}
	simple_unlock(&ihandlers_lock);
	return(EIO);
}

/*2
.* dealloc_vector - deallocate an interrupt vector from the interrupt table.
 *
.* ARGUMENTS:
 *
.* vector - Pointer to some interrupt vector that is to be deallocated (removed)
 *	from the interrupt table.
 *
.* USAGE:
 *	This routine is used by create_chan() to to abort an unsuccessful
 *	creation of a channel and by delete_chan) to deallocate the interrupt
 *	vector for the CRQ it is deleting the channel for.
 */

dealloc_vector(vector)
long *vector;
{
	register ihandler_t *pih;
	register i;

	/*
	 * Lock the table against any updates.
	 */
	simple_lock(&ihandlers_lock);
	/*
	 * If the specified interrupt table entry is already
	 * empty then return an error. Otherwise make it an empty
	 * table entry.
	 */
	i = GETVECTORNUM(vector);
	pih = &ihandlers[i];
	if (pih->ih_handler == NULL) {
		simple_unlock(&ihandlers_lock);
		return(EIO);
	}
	else {
		pih->ih_handler = NULL;
	}
	/*
	 * Unlock the interrupt table.
	 */
	simple_unlock(&ihandlers_lock);
	return(0);
}

/*2
.* send_vector - send a vector on the vector bus for MULTIMAX systems.
 *
.* ARGUMENTS:
 *
.* vector - pointer to the vector to be sent.
 *
.* USAGE:
 *	This routine is used to send a vector on the vector bus in
 *	order to generate an interrupt on some device in some
 *	slot. It is used by the send_cmd() routines to cause the
 *	devices the command is being sent to to be interrupted (if the
 *	CRQ is non-empty), and is used to generate inter-processor
 *	interrupts. On non-MULTIMAX systems this routine is a simple
 *	macro that directly calls vector_isr(), which informs
 *	the controller that an I/O is queued up.
 *
.* ASSUMPTIONS:
 *	Interrupts are disabled when this routine is called (otherwise
 *	the test for whether the interrupt can be sent or not cannot
 *	be done accurately).
 */

#if	MMAX_XPC
/*
 * There is one vector bus gate array per board, shared between the two
 * CPUs.  Due to a small glitch in the VBGA, however, the CPUs must
 * serialize before sending a vector.
 */
vb_xmit_lock_t		vb_xmit_lock[20]; /* locks for per-board vbga's */
#endif

int vector_count = 0;
#define	VBTIMEOUT	100000

send_vector(vectorp)
long *vectorp;
{
	int	i, s, max_delay, success;
#if	MMAX_XPC
	int	slot;
#endif

	vector_count++;		/* Count total vectors sent */

#if	MMAX_XPC
	slot = GETCPUSLOT;
#endif
	for (i = 0; i < 10; ++i) {
		if (i)
			printf("send_vector:  cpu %d xmit retry vector=0x%x\n",
			       getcpuid(), *vectorp);
		max_delay = VBTIMEOUT;
		s = splhigh();
#if	MMAX_XPC
		simple_lock(&vb_xmit_lock[slot].lock);
#endif
		while (VB_BUSY && --max_delay)
			;
		if (max_delay == 0) {
#if	MMAX_XPC
			simple_unlock(&vb_xmit_lock[slot].lock);
			splx(s);
#endif
			printf("send_vector:  cpu %d vecbus busy stuck?\n",
			       getcpuid());
			continue;	/* error - VB_BUSY bit stuck? */
		}

		/* Send the vector. */
#if	MMAX_XPC
		*XPCVB_XMIT_1 = (unsigned short) *vectorp;
		*XPCVB_XMIT_2 = (unsigned short) (*vectorp >> 16);
#endif
#if	MMAX_APC
		*APCREG_VBXMIT = ((apcvbxmit_t *) vectorp)->l;
#endif
#if	MMAX_DPC
		*((long *)DPCREG_SENDVEC) =
			((dpcsendvec_t *)vectorp)->l^DPCSENDVEC_FIX;
		success = TRUE;
#endif

#if	MMAX_XPC || MMAX_APC
		/* Wait for busy to clear so we can check for ACK. */
		max_delay = VBTIMEOUT;
		while (VB_BUSY && --max_delay)
			;

		/* Check for successful transmission of vector. */
#if	MMAX_XPC
		success = ((*XPCVB_STATUS) & XPCVB_STAT_VECT_ACK);
		simple_unlock(&vb_xmit_lock[slot].lock);
#endif
#if	MMAX_APC
		if ((success = (!(*(char *)(S_ICU_BASE+RBIAS+IPND) &
			     ((ICU_VB_NAK >> 24) & 0xff)))) == 0)
			clr_vbnak();
#endif	/* MMAX_APC */
#endif	/* MMAX_XPC */
		splx(s);
		if (success)
			return;
	}
	panic("send_vector:  persistent vectorbus NAKs");
}


#if	MMAX_APC
clr_vbnak()
{
	*(char *)(S_ICU_BASE + IMCTL) =		/* freeze pend */
		CFRZ | FRZ | NTAR;
	*(char *)(S_ICU_BASE+RBIAS+IELTG) =	/* set level trig */
		((ELTG_S_VBNAK >> 8) & 0xff);
	*(char *)(S_ICU_BASE+RBIAS+IPND) =	/* clear pend */
		0x09;
	*(char *)(S_ICU_BASE+RBIAS+IELTG) =	/* set edge trig */
		((ELTG_S >> 8) & 0xff);
	*(char *)(S_ICU_BASE+IMCTL) =		/* unfreeze pend */
		NTAR;
}
#endif	/* MMAX_APC */



/*
 * Messages that are used during a polled situation. They
 * are statically allocated so that calls to malloc() do not have to
 * be made at init or panic time.
 */
crq_create_chan_msg_t pld_create_msg;
crq_delete_chan_msg_t pld_delete_msg;
crq_warm_msg_t pld_warm_msg;

/*2
.* polled_warm_restart - Issue a warm restart to the specified I/O controller
 *	during a polled situation.
 *
.* ARGUMENTS:
 *
.* slot - Slot number of the I/O controller.
 *
.* USAGE:
 * 	Issue a warm restart to the specified I/O controller. A warm restart
 *	command causes the controller to re-initialize including 'forgetting'
 *	any and all I/O channels and operations. Called during polled() to
 *	restart EMCs and SCCs.
 */

polled_warm_restart(slot)
int	slot;
{
	crq_warm_msg_t	*cmd, *rsp;
	crq_t		*scc_crq;
	int		old_mode, error = 0;

	/*
	 * Allocate a command packet to use.
	 */
	cmd = &pld_warm_msg;

	/*
	 * Initialize the warm restart message.
	 */
	cmd->warm_hdr.crq_msg_code = CRQOP_WARM;

	scc_crq = REQ_CRQ(SCC_SLOT,0);
	old_mode = set_polled_mode(scc_crq);

	/*
	 * Sending a warm restart command via the requester
	 * CRQ.
	 */
	send_req_cmd(cmd, slot);

	/*
	 * Synchronize with completion
	 */
        do {
		rsp = (crq_warm_msg_t *) rec_polled_rsp(scc_crq);
		if (rsp == cmd) {
			/*
			 * Verify successful warm restart
			 */
			if (cmd->warm_hdr.crq_msg_status != STS_SUCCESS)
			  error = (int) cmd->warm_hdr.crq_msg_status;
	        } else {
			if (rsp != 0)
				spurious_polled_rsp("polled_warm_restart",rsp);
			else
				error = ENXIO; /* Not the best, but... */
		}
        } while ( (rsp != 0) && (rsp != cmd) );

	restore_mode(scc_crq, old_mode);
	return(error);
}

/*2
.* polled_create_chan - this function creates an I/O communications channel
 *	in a polled situation.
 *
.* ARGUMENTS:
 *
.* crq - Pointer to an initialized CRQ to be used by the channel.
 *
.* isr - Pointer to an interrupt service routine for this channel.
 *
.* param - Parameter to pass to isr.
 *
.* name - Pointer to channel name.
 *
.* USAGE:
 *	This routine is used to create communications channels between
 *	the DPC and disks on the EMC card. It is called by the d_dump
 *	routines of the bdevsw drivers to create channels for each and
 *	every IO.
 *
.* ASSUMPTIONS:
 *	The CRQ has been initialized via init_crq.
 */

polled_create_chan(crq, isr, param)
register crq_t	*crq;
int	(*isr)();
int	param;
{
	crq_create_chan_msg_t	*cmd, *rsp;
	crq_t			*scc_crq;
	int			old_mode, error = 0;

	/*
	 * Allocate a command packet.
	 */
	cmd = &pld_create_msg;

	/*
	 * Initialize the create channel message. The reference number is set
	 * to the address of the per process I/O queue that the SCC Requester
	 * CRQ interrupt service routine will queue the response message. Note,
	 * for standalone mode a single thread of control (and therefore queue)
	 * is assumed.
	 */
	cmd->creat_chan_hdr.crq_msg_code = CRQOP_CREATE_CHANNEL;
	cmd->creat_chan_crq = crq;

	scc_crq = REQ_CRQ(SCC_SLOT,0);
	old_mode = set_polled_mode(scc_crq);

	/*
	 * Send the create channel command message to the requestor CRQ.
	 */
	send_req_cmd(cmd, GETSLOT(crq->crq_unitid));

	/*
	 * Synchronize with completion
	 */
	do {
		rsp = (crq_create_chan_msg_t *)	rec_polled_rsp(scc_crq);
		if (rsp == cmd) {
			/*
			 * Verify that the channel was successful created.
			 */
			if (cmd->creat_chan_hdr.crq_msg_status != STS_SUCCESS)
				error = EIO;
		} else {
			if (rsp != 0)
				spurious_polled_rsp("polled_create_chan", rsp);
			else 
				error = ENXIO; /* Not the best, but... */
		}
	} while ( (rsp != 0) && (rsp != cmd) );

	restore_mode(scc_crq, old_mode);
	return(error);
}

/*2
.* polled_delete_chan - given an existing CRQ, delete the associated channel in
 *	a polled situation.
 *
.* ARGUMENTS:
 *
.* crq - Pointer to the CRQ whose channel is to be deleted.
 *
.* USAGE:
 *	Called after completing every IO by the d_dump routines of
 *	all the bdevsw drivers.
 */

polled_delete_chan(crq)
register crq_t	*crq;
{
	crq_delete_chan_msg_t	*cmd, *rsp;
	crq_t			*scc_crq;
	int			old_mode, error = 0;

	/*
	 * Allocate a command packet to use.
	 */
	cmd = &pld_delete_msg;

	/*
	 * Initialize the delete channel message.
	 */
	cmd->del_chan_hdr.crq_msg_code = CRQOP_DELETE_CHANNEL;
	cmd->del_chan_crq = crq;

	scc_crq = REQ_CRQ(SCC_SLOT,0);
	old_mode = set_polled_mode(scc_crq);

	/*
	 * Send the delete channel command to the requestor CRQ.
	 */
	send_req_cmd(cmd, GETSLOT(crq->crq_unitid));

	/*
	 * Synchronize with completion
	 */
	do {
		rsp = (crq_delete_chan_msg_t *)	rec_polled_rsp(scc_crq);
		if (rsp == cmd) {
			/*
			 * Verify that the channel was successfully deleted.
			 */
			if (cmd->del_chan_hdr.crq_msg_status != STS_SUCCESS)
				error = cmd->del_chan_hdr.crq_msg_status;
		} else {
			if (rsp != 0)
				spurious_polled_rsp("polled_delete_chan", rsp);
			else 
				error = ENXIO; /* Not the best, but... */
		}
	} while ((rsp != 0) && (rsp != cmd));

	restore_mode(scc_crq, old_mode);
	return(error);
}


static int
set_polled_mode(crq)
crq_t	*crq;
{
	register int	s, old_mode;

	s = splhigh();
	crq_lock(&crq->crq_slock);
	old_mode = crq->crq_mode & CRQMODE_INTR;
	crq->crq_mode &= (~CRQMODE_INTR);
	crq_unlock(&crq->crq_slock);
	splx(s);
	return old_mode;
}


static void
restore_mode(crq, mode)
crq_t	*crq;
int	mode;
{
	register int	s;

	if (mode == CRQMODE_INTR) {
		s = splhigh();
		crq_lock(&crq->crq_slock);
		crq->crq_mode |= CRQMODE_INTR;
		crq_unlock(&crq->crq_slock);
		splx(s);
	}
}


static void
spurious_polled_rsp(routine, rsp)
char		*routine;
crq_msg_t	*rsp;
{
	int	i, *ip;

	printf("%s:  spurious reply, message address 0x%8x:\n", rsp);
	ip = (int *) rsp;
	for (i=0; i< (MAX_ATTN_SIZE/4); i++) {
		printf("0x%8x", *ip++);
		if (((i % 4) == 0) && (i != 0))
			printf("\n");
		else
			printf("\t");
	}
}
