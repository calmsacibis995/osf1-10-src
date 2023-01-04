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
static char	*sccsid = "@(#)$RCSfile: clock.c,v $ $Revision: 4.2.3.4 $ (DEC) $Date: 1992/10/13 14:48:52 $";
#endif 
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint

#endif
*/
# include "sendmail.h"
# include <signal.h>

/*
**  SETEVENT -- set an event to happen at a specific time.
**
**	Events are stored in a sorted list for fast processing.
**	An event only applies to the process that set it.
**
**	Parameters:
**		intvl -- intvl until next event occurs.
**		func -- function to call on event.
**		arg -- argument to func on event.
**
**	Returns:
**		none.
**
**	Side Effects:
**		none.
*/

EVENT *
setevent(intvl, func, arg)
	time_t intvl;
	int (*func)();
	int arg;
{
	register EVENT **evp;
	register EVENT *ev;
	auto time_t now;
	extern tick();

	if (intvl <= 0)
	{
		syserr("setevent: intvl=%ld\n", intvl);
		return (NULL);
	}

	(void) time(&now);

	/* search event queue for correct position */
	for (evp = &EventQueue; (ev = *evp) != NULL; evp = &ev->ev_link)
	{
		if (ev->ev_time >= now + intvl)
			break;
	}

	/* insert new event */
	ev = (EVENT *) xalloc(sizeof *ev);
	ev->ev_time = now + intvl;
	ev->ev_func = func;
	ev->ev_arg = arg;
	ev->ev_pid = getpid();
	ev->ev_link = *evp;
	*evp = ev;

	if (tTd(5, 5))
		printf("setevent: intvl=%ld, for=%ld, func=%x, arg=%d, ev=%x\n",
			intvl, now + intvl, func, arg, ev);

	tick();
	return (ev);
}
/*
**  CLREVENT -- remove an event from the event queue.
**
**	Parameters:
**		ev -- pointer to event to remove.
**
**	Returns:
**		none.
**
**	Side Effects:
**		arranges for event ev to not happen.
*/

clrevent(ev)
	register EVENT *ev;
{
	register EVENT **evp;

	if (tTd(5, 5))
		printf("clrevent: ev=%x\n", ev);
	if (ev == NULL)
		return;

	/* find the parent event */
	(void) signal(SIGALRM, SIG_IGN);
	for (evp = &EventQueue; *evp != NULL; evp = &(*evp)->ev_link)
	{
		if (*evp == ev)
			break;
	}

	/* now remove it */
	if (*evp != NULL)
	{
		*evp = ev->ev_link;
		free((char *) ev);
	}

	/* restore clocks and pick up anything spare */
	tick();
}
/*
**  TICK -- take a clock tick
**
**	Called by the alarm clock.  This routine runs events as needed.
**
**	Parameters:
**		none.
**
**	Returns:
**		none.
**
**	Side Effects:
**		calls the next function in EventQueue.
*/

tick()
{
	register time_t now;
	register EVENT *ev;
	int mypid = getpid();

	(void) signal(SIGALRM, SIG_IGN);
	(void) alarm(0);
	now = curtime();

	if (tTd(5, 4))
		printf("tick: now=%ld\n", now);

	while ((ev = EventQueue) != NULL &&
	       (ev->ev_time <= now || ev->ev_pid != mypid))
	{
		int (*f)();
		int arg;
		int pid;

		/* process the event on the top of the queue */
		ev = EventQueue;
		EventQueue = EventQueue->ev_link;
		if (tTd(5, 6))
			printf("tick: ev=%x, func=%x, arg=%d, pid=%d\n", ev,
				ev->ev_func, ev->ev_arg, ev->ev_pid);

		/* we must be careful in here because ev_func may not return */
		(void) signal(SIGALRM, (void (*)(int))tick);
#ifdef SIGVTALRM
		/* reset 4.2bsd signal mask to allow future alarms */
		(void) sigsetmask(sigblock(0) & ~sigmask(SIGALRM));
#endif /* SIGVTALRM */

		f = ev->ev_func;
		arg = ev->ev_arg;
		pid = ev->ev_pid;
		free((char *) ev);
		if (pid != getpid())
			continue;
		if (EventQueue != NULL)
		{
			if (EventQueue->ev_time > now)
				(void) alarm((unsigned) (EventQueue->ev_time - now));
			else
				(void) alarm(3);
		}
		(*f)(arg);
		(void) alarm(0);
		now = curtime();
	}
	(void) signal(SIGALRM, (void (*)(int))tick);
	if (EventQueue != NULL)
		(void) alarm((unsigned) (EventQueue->ev_time - now));
}
/*
**  SLEEP -- a version of sleep that works with this stuff
**
**	Because sleep uses the alarm facility, I must reimplement
**	it here.
**
**	Parameters:
**		intvl -- time to sleep.
**
**	Returns:
**		none.
**
**	Side Effects:
**		waits for intvl time.  However, other events can
**		be run during that interval.
*/

static bool	SleepDone;
static endsleep();

sleep(intvl)
	unsigned int intvl;
{

	if (intvl == 0)
		return;
	SleepDone = FALSE;
	(void) setevent((time_t) intvl, endsleep, 0);
	while (!SleepDone)
		pause();
}

static
endsleep()
{
	SleepDone = TRUE;
}
