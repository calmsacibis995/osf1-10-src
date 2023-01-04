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
static char	*sccsid = "@(#)$RCSfile: emc.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:40:35 $";
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

/*1
 * emc.c
 *
 *	This file contains the routines used for sending messages
 *	to the EMC from the DPC.
 */

#include <sys/param.h>
#include <sys/user.h>
#include <mmaxio/crqdefs.h>
#include <mmax/sccdefs.h>
#include <mmaxio/emcdefs.h>
#include <mmax/boot.h>
#include <mmax/isr_env.h>

#include <emc.h>


extern	crq_t *Slot_crqs[NUM_SLOT];
int emc_isr();

/*
 * Messages and status of messages/commands dealing with checking whether
 * EMC is alive or not.
 */

int emc_msgreceived[NUM_SLOT];
crq_msg_t *emc_noop_cmd[NUM_SLOT];


/*
 *  Determine if the board type represented by 'type' is capable of
 *  holding disk drives and such.  Will need to be modified when
 *  we add MSC's and understand what we want to do.
 */

is_emc(type)
int type;
{
	return(type == EMC || type == EMCII || type == EMCDIF || type == MSC);
}


/*2
.* init_emc - initilize the EMC for communication with the host.
 *
.* ARGUMENTS:
 *
.* slot - slot on the backplane this EMC is located in.
 *
.* USAGE:
 *	Initialize the EMC for communications with the host. This includes
 *	allocating and initializing a CRQ for the EMC slot. Called once early 
 *	in the initialization for each EMC in the system. Must be called 
 *	BEFORE any other messages are sent to EMC's.
 */

init_emc(slot, emc_num)
unsigned slot;
int emc_num;
{
	static crq_t slot_crq[NEMC];
	static emc_atn_msg_t free[NUM_EMC_ATTNS][NEMC];
	static crq_nop_msg_t cmd[NEMC];
	register i;

	init_crq(&slot_crq[emc_num], CRQMODE_INTR, 0,
		MAKEUNITID(0, slot, 0, SLOT_LUN), NULL);

	if(alloc_vector(&slot_crq[emc_num].crq_master_vect,
		emc_isr, slot, INTR_DEVICE))
		panic("init_emc: slot alloc vector failed\n");

	/*
	 * Populate the free message queue for attentions.
	 */
	for (i = 0; i < NUM_EMC_ATTNS; i++) {
		put_free(&free[i][emc_num], &slot_crq[emc_num]);
	}

	/*
	 * Create a channel to the EMC slot with the newly created
	 * CRQ. If successful, the slot crq is copied to the
	 * system array of per slot CRQs.
	 */
	if(polled_create_chan(&slot_crq[emc_num], emc_isr, slot))
		panic("init_emc: create channel failed\n");
	Slot_crqs[slot] = &slot_crq[emc_num];

	/*
	 * Initialize the noop message used to check
	 * periodically whether the EMC is alive or not. Then send
	 * the message to the EMC for the 1st time.
	 */
	cmd[emc_num].nop_hdr.crq_msg_code = CRQOP_NOP;
	cmd[emc_num].nop_hdr.crq_msg_refnum = 0;
	emc_msgreceived[slot] = 1;
	emc_noop_cmd[slot] = &(cmd[emc_num].nop_hdr);
	emc_timeout(slot);
}


/*2
.* emc_isr - EMC slot level interrupt routine.
 *
.* ARGUMENTS:
 *
.* slot - EMC slot requesting service.
 *
.* USAGE;
 *	This routine is the slot level interrupt routine. All attentions
 *	and responses are queued to the proper isr queue.
 */
emc_isr(ihp)
ihandler_t *ihp;
{
	int slot = ihp->ih_hparam[0].intparam;
	register emc_atn_msg_t *attn;
	register crq_msg_t *rsp;
	register crq_t *crq;

	/*
	 * Set the CRQ address based on the slot number.
	 */
	crq = Slot_crqs[slot];

	/*
	 * Dequeue and process attention packets.
	 */
	while ((attn = (emc_atn_msg_t *)rec_attn(crq)) != NULL) {

		printf("EMC attention received slot %d. See error log.\n",slot);
		put_free(attn, crq);
	}

	/*
	 * Dequeue all packets. If the packet is a noop, then indicate
	 * the noop was received. For all other packets, pass responses to 
	 * post-processing queue.
	 */
	while ((rsp = (crq_msg_t *)rec_rsp(crq)) != NULL) {
		if (rsp->crq_msg_code == CRQOP_NOP && rsp->crq_msg_refnum == 0)
			emc_msgreceived[slot] = 1;
		else
			put_isr_queue(rsp, rsp->crq_msg_refnum);
	}
}


/*2
.* emc_timeout - timeout() routine called once a minute for each EMC
 *
.* ARGUMENTS:
 *
.* slot - slot number of this EMC
 *
.* USAGE:
 *	This routine is used to check whether the NOOP message sent to
 *	the EMC by this routine one minute ago was replied to. If it was
 *	replied to, then send another message and make sure this routine
 *	gets called again in a minute. If it didn't get replied to
 *	then cause the system to panic.
 */

emc_timeout(slot)
{
extern	time_t	hz;
extern  char *panicstr;

	if(emc_msgreceived[slot]) {
		if( !panicstr ) {
			emc_msgreceived[slot] == 0;
			timeout(emc_timeout,slot,60*hz);
			send_slot_cmd(emc_noop_cmd[slot],slot);
		}
	} else {
		printf("emc_timeout: EMC in slot %d not responding\n", slot);
		panic("emc_timeout");
	}
}
