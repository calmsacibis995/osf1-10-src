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
static char	*sccsid = "@(#)$RCSfile: machdep.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/05/21 10:59:50 $";
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
 * File: machdep.c
 *
 * This contains all the twiddling needed to start/restart vps and perform
 * asynchronous functino calls. This is machine dependent as it mainly
 * involves mucking about with data on stacks.
 */

#ifndef	lint
#endif	not lint

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
	struct alpha_thread_state		state;
	register struct alpha_thread_state	*ts;

	extern		pthread_body();
#ifndef OSF_ROSE
	extern long	_gp;		/* ld(1) defines this */
#endif

	ts = &state;

	/*
	 * Set up alpha call frame and registers.
	 */
	bzero((char *)ts, sizeof(struct alpha_thread_state));
	/*
	 * Set pc to procedure entry, pass vp in a0 (r16).
	 * Give as GP value to the vp the same we have.
	 */
	ts->pc  = (long)pthread_body;
	ts->r16 = (long)vp;
	ts->r30 = vp->stackbase + vp->stacksize;
#ifndef OSF_ROSE
	ts->r27 = (long)pthread_body;	/* for gp calculation */
	ts->r29 = (long)&_gp;
#endif

	if (thread_set_state(vp->id, ALPHA_THREAD_STATE, (thread_state_t)ts,
				   ALPHA_THREAD_STATE_COUNT) != KERN_SUCCESS)
		pthread_internal_error("vp_setup");
}

/*
 * Function:
 *	vp_call_setup
 *
 * Parameters:
 *	vp - the structure describing the target vp to make the call
 *
 * Description:
 *	The MACH thread must be in a suspended state when this function
 *	is called. Registers used by the delivery glue code are pushed 
 *	onto the stack and these are set to describe the async call.
 *
 *	pc = delivery glue code
 *	a0 = function to call
 *	a1 = arg to that function
 *	sp = stack pointer beyond the save area
 */
void
vp_call_setup(vp_t vp)
{
	thread_state_data_t		state;
	struct alpha_thread_state	*statep;
	unsigned int			count;
	long				*sp;
	void				vp_call_deliver();

	statep = (struct alpha_thread_state *)state;

	count = THREAD_STATE_MAX;
	if (thread_get_state(vp->id, ALPHA_THREAD_STATE, (thread_state_t)state,
				   &count) != KERN_SUCCESS)
		pthread_internal_error("vp_call_getstate");

	/*
	 * Save old copies on the stack to be possibly restored later
	 * if we come back (unlikely!)
	 */
	sp = (long *)statep->r30;
	*(--sp) = statep->r17;	/* a1 */
	*(--sp) = statep->r16;	/* a0 */
	*(--sp) = statep->pc;	/* ra */

 	/*
	 * set up the glue code environment
	 */
	statep->pc  = (long)vp_call_deliver;
	statep->r27 = (long)vp_call_deliver;	/* pv for gp calculation */
	statep->r16 = (long)vp->async_func;	/* a0 */
	statep->r17 = (long)vp->async_arg;	/* a1 */
	statep->r30 = (long)sp;

	if (thread_set_state(vp->id, ALPHA_THREAD_STATE, (thread_state_t)state,
				   ALPHA_THREAD_STATE_COUNT) != KERN_SUCCESS)
		pthread_internal_error("vp_call_setstate");
}
