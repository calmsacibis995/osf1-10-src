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
static char	*sccsid = "@(#)$RCSfile: machdep.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:11:58 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * derived from vp startup for Encore Multimax implementation.
 */

#include <pthread.h>
#include "internal.h"


/*
 * Set up the initial state of a MACH thread
 * so that it will invoke pthread_body(vp)
 * when it is resumed.
 */
void
vp_setup(vp)
register vp_t	vp;
{
	register int			     *top;
	struct i386_thread_state	     state;
	register struct i386_thread_state *ts;

	extern	int pthread_body();

	top = (int *) (vp->stackbase + vp->stacksize);
	ts = (struct i386_thread_state *) &state;

	/*
	 * Set up i386 call frame and registers.
	 * One argument (vp) will be on the stack.
	 * pthread_body will do the enter instruction itself.
	 */
	bzero((char *) &state, sizeof(state));

	ts->eip = (int) (int (*)()) pthread_body;
	*--top = (int) vp;	/* argument to function */
	*--top = 0;		/* fake pc from jsb */
/*  Note: No return address is really saved on stack	*/
	ts->esp = (int) top;

	if (thread_set_state(vp->id, i386_THREAD_STATE,
				   (thread_state_t) &state,
				   i386_THREAD_STATE_COUNT) != KERN_SUCCESS)
		pthread_internal_error("vp_setup");
}

vp_call_setup(vp)
vp_t	vp;
{
	thread_state_data_t		state;
	struct i386_thread_state	*statep;
	unsigned int			count;
	int				*sp;
	extern void			vp_call_deliver();

	statep = (struct i386_thread_state *)state;

	count = THREAD_STATE_MAX;
	if (thread_get_state(vp->id, i386_THREAD_STATE,
			(thread_state_t) &state, &count) != KERN_SUCCESS)
		pthread_internal_error("vp_call_getstate");

	sp = (int *)statep->uesp;
	sp = (int *)statep->esp;
	*(--sp) = statep->eip;
	*(--sp) = statep->ecx;

	statep->eip = (int)vp_call_deliver;
	statep->ecx = (int)vp->async_func;
	statep->eax = (int)vp->async_arg;
	statep->esp = (int)sp;

	if (thread_set_state(vp->id, i386_THREAD_STATE,
				   (thread_state_t) &state,
				   i386_THREAD_STATE_COUNT) != KERN_SUCCESS)
		pthread_internal_error("vp_call_setstate");
}
