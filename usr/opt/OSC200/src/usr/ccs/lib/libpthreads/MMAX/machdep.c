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
static char	*sccsid = "@(#)$RCSfile: machdep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:10 $";
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
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * vp startup for Encore Multimax implementation.
 */

#ifndef	lint
#endif	not lint

/*
 * File: machdep.c
 *
 * This contains all the twiddling needed to start/restart vps and perform
 * asynchronous functino calls. This is machine dependent as it mainly
 * involves mucking about with data on stacks.
 */

#include <pthread.h>
#include "internal.h"

/*
 * Function:
 *	vp_setup
 *
 * Parameters:
 *	vp - the structure describing the MACH thread being set up
 *
 * Description:
 *	Set up the initial state of a MACH thread so that it will
 *	invoke pthread_body(vp) when it is resumed.
 */
void
vp_setup(register vp_t vp)
{
	register int			     *top;
	struct ns32532_thread_state	     state;
	register struct ns32532_thread_state *ts;

	extern	int pthread_body();

	top = (int *) (vp->stackbase + vp->stacksize);
	ts = (struct ns32532_thread_state *) &state;

	/*
	 * Set up ns32532 call frame and registers.
	 * One argument (vp) will be on the stack.
	 * pthread_body will do the enter instruction itself.
	 */
	bzero((char *) &state, sizeof(state));

	ts->pc = (int) (int (*)()) pthread_body;
	ts->fp = (int) top;
	ts->mod = 0x20;		/* must agree with .org in crt0 */
	*--top = (int) vp;	/* argument to function */
	*--top = 0;		/* fake pc from jsb */
	ts->sp = (int) top;

	if (thread_set_state(vp->id, NS32532_THREAD_STATE,
				   (thread_state_t) &state,
				   NS32532_THREAD_STATE_COUNT) != KERN_SUCCESS)
		pthread_internal_error("vp_setup");
}

/*
 * Function:
 *      vp_call_setup
 *
 * Parameters:
 *      vp - the structure describing the target vp to make the call
 *
 * Description:
 *      The MACH thread must be in a suspended state when this function
 *      is called. Registers used by the delivery glue code are pushed
 *      onto the stack and these are set to describe the async call.
 *
 *      pc = delivery glue code
 *      r1 = function to call
 *      r2 = arg to that function
 *      sp = stack pointer beyond the save area
 */
void
vp_call_setup(vp_t vp)
{
	thread_state_data_t		state;
	struct ns32532_thread_state	*statep;
	unsigned int			count;
	int				*sp;
	extern void			vp_call_deliver();

	statep = (struct ns32532_thread_state *)state;

	count = THREAD_STATE_MAX;
	if (thread_get_state(vp->id, NS32532_THREAD_STATE,
			(thread_state_t) &state, &count) != KERN_SUCCESS)
		pthread_internal_error("vp_call_getstate");

	/*
	 * Save old copies on the stack to be possibly restored later
	 * if we come back (unlikely!)
	 */
	sp = (int *)statep->sp;
	*(--sp) = statep->pc;
	*(--sp) = statep->r2;
	*(--sp) = statep->r1;

	/*
	 * set up the glue code environment
	 */
	statep->pc = (int)vp_call_deliver;
	statep->r1 = (int)vp->async_func;
	statep->r2 = (int)vp->async_arg;
	statep->sp = (int)sp;

	if (thread_set_state(vp->id, NS32532_THREAD_STATE,
				   (thread_state_t) &state,
				   NS32532_THREAD_STATE_COUNT) != KERN_SUCCESS)
		pthread_internal_error("vp_call_setstate");
}
