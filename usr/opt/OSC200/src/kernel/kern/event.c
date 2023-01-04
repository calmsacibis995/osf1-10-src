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
static char	*sccsid = "@(#)$RCSfile: event.c,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/01/08 17:54:13 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/* 
 * Mach Operating System
 * Copyright (c) 1988 Carnegie-Mellon University
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
 *	File:	kern/event.c
 *
 *	Implementation of a simple event facility.
 */

#include <mach_assert.h>
#include <mach_ldebug.h>

#include <sys/unix_defs.h>
#include <sys/proc.h>
#include <sys/param.h>
#include <sys/user.h>
#include <kern/event.h>
#include <kern/thread.h>
#include <kern/sched_prim.h>

#if	MACH_LDEBUG
extern int check_locks;
#endif

void
event_clear(evp)
event_t	*evp;
{
	int	s;

	s = splhigh();
	simple_lock (&evp->ev_slock);
	evp->ev_event = FALSE;
#if	MACH_ASSERT
#if	MACH_LDEBUG
	if (check_locks)
		evp->ev_thread = (char *) current_thread();
#endif
#ifdef	multimax
	evp->ev_caddr = getpc_fromec();
#endif
#endif
	simple_unlock (&evp->ev_slock);
	splx(s);
}


void
event_init(evp)
event_t	*evp;
{
	simple_lock_init (&evp->ev_slock);
#if	MACH_LDEBUG
	evp->ev_paddr = -1;
#endif
	event_clear(evp);
}

/*
 * If not interruptible, block until awakened normally.
 *
 * If interruptible, call mpsleep, with timo.  If mpsleep is interrupted,
 * it will have called issig() to check signals.  Just pass the error
 * back to the caller.
 *
 * Note that we must call mpsleep at elevated priority, so that the
 * event is caught.  If we simply lowered priority without unlocking the
 * lock, deadlock could result.
 *
 * The timo parameter is ignored if not interruptible.
 */
event_wait(evp, interruptible, timo)
event_t	*evp;
boolean_t interruptible;
int timo;
{
	register thread_t	th;
	int			error = 0, s;

	th = current_thread();

	s = splhigh();
	simple_lock (&evp->ev_slock);
	while (evp->ev_event == FALSE) {
		if (interruptible) {
			error = mpsleep((caddr_t) &evp->ev_event, 
				(PZERO + 1) | PCATCH, "event", timo,
				(void *) simple_lock_addr(evp->ev_slock), 
				MS_LOCK_SIMPLE);
			if (error)
				break;
		} else {
			/*
			 * We won't wake up out of this easily
			 */
			(void) mpsleep((caddr_t) &evp->ev_event, 
				PZERO - 1, "event", 0,
				(void *) simple_lock_addr(evp->ev_slock), 
				MS_LOCK_SIMPLE);
		}
	} /* else -- event already posted */
	if (error == 0)
		simple_unlock (&evp->ev_slock);
	splx(s);
	return (error);
}

int
event_posted(evp)
event_t	*evp;
{
	register boolean_t	saved_event;
	int			s;

	s = splhigh();
	simple_lock(&evp->ev_slock);
	saved_event = evp->ev_event;
	simple_unlock(&evp->ev_slock);
	splx(s);
	return (int) saved_event;
}

void
event_post(evp)
event_t	*evp;
{
	int	s;

	s = splhigh();
	simple_lock (&evp->ev_slock);
	evp->ev_event = TRUE;
#if	MACH_ASSERT
#if	MACH_LDEBUG
	if (check_locks)
		evp->ev_thread = (char *) current_thread();
#endif
#ifdef	multimax
	evp->ev_paddr = getpc_fromep();
#endif
#endif
	simple_unlock(&evp->ev_slock);
	splx(s);
	thread_wakeup((vm_offset_t)&evp->ev_event);
}
