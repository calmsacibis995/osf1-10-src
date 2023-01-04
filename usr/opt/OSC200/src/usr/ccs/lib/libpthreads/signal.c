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
static char	*sccsid = "@(#)$RCSfile: signal.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 04:12:57 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */

#ifndef	lint
#endif	not lint

/*
 * File: signal.c
 *
 * This file contains sigwait() and all that goes to support it. The signal
 * handler sends the signal number in a mach IPC message to a dedicated
 * thread who is created behind the back of a thread calling sigwait().
 * The caller waits on the condition variable and the sigwait thread
 * broadcasts the condition when it gets the message of signal arrival.
 */

#include <pthread.h>
#include "internal.h"
#include <signal.h>

/*
 * Local Variables
 */
private pthread_mutex_t		sigwait_lock;
private pthread_cond_t		signal_arrived;
private	pthread_t		wait_thread;
private int			sig_interest[SIGMAX];
private int			sig_handler[SIGMAX];
private sigset_t		pending;
private	struct sigaction	handler;
private	struct sigaction	old_handler[SIGMAX];

/*
 * Function:
 *	sigwait_handler
 *
 * Parameters:
 *	signo - The signal number that just arrived
 *
 * Description:
 *	This is the signal handler set up by a sigwaiter for all signals
 *	that the thread is interested in. The handler remains installed until
 *	there are no more threads interested in that signal.
 */
private void
sigwait_handler(int signo)
{
	/*
	 * we have go a signal. Just send the signal number in a
	 * message to the sigwait thread.
	 * We know that the wait_thread is valid because this
	 * handler would not be established if it wasn't.
	 */
	vp_event_notify(wait_thread->vp, EVT_SIGWAIT|signo);
}

/*
 * Function:
 *	pthread_sigwait_startup
 *
 * Description:
 *	Called by pthread_init, this function intializes all the global
 *	data and state needed by the sigwait call.
 */
void
pthread_sigwait_startup()
{
	int	i;

	/*
	 * Initialize the mutex/condition pair that protect all
	 * the global data
	 */
	pthread_mutex_init(&sigwait_lock, pthread_mutexattr_default);
	pthread_cond_init(&signal_arrived, pthread_condattr_default);

	/*
	 * There is no sigwait thread running
	 */
	wait_thread = NO_PTHREAD;

	/*
	 * There are no waiting thread and no handlers installed
	 */
	for (i = 1; i < SIGMAX; i++) {
		sig_interest[i] = 0;
		sig_handler[i] = 0;
	}

	/*
	 * The pending mask is also outside the kernel. A signal is pending
	 * if one is set in either the kernel copy or this sigwait pending
	 * mask.
	 */
	sigemptyset(&pending);

	/*
	 * set up the sigaction structure for future calls to establish the
	 * handler
	 */
	handler.sa_handler = sigwait_handler;
	sigemptyset(&handler.sa_mask);
	handler.sa_flags = 0;
}

/*
 * Function:
 *	sigwait_thread
 *
 * Parameters:
 *	arg - not used.
 *
 * Return value:
 *	0	always
 *
 * Description:
 *	This is a thread that is created by the sigwait() function
 *	to manage the distribution of incoming signals to threads that
 *	are waiting for them. Only this thread establishes and removes
 *	signal handlers. The sigwait function marks that someone is interested
 *	in a signal and wakes up this thread.
 */
private void *
sigwait_thread(void *arg)
{
	int		i;
	int		event;
	int		signo;
	int		should_wait;
	sigset_t	set;

	pthread_setname_np(wait_thread, "sigwait_thread");

	pthread_mutex_lock(&sigwait_lock);

	for (;;) {
		/*
		 * scan through all the signals looking for new interest
		 * or loss of interest. These signals will need handlers
		 * established or removed.
		 */
		should_wait = 0;
		for (i = 1; i < SIGMAX; i++) {
			if (!sig_interest[i] && sig_handler[i]) {
				/*
				 * There is no interest in this signal any more
				 * Block the signal again and remove the handler
				 */
				sigemptyset(&set);
				sigaddset(&set, i);
				sigprocmask(SIG_BLOCK, &set, NULL);
				sigaction(i, &old_handler[i], NULL);
				sig_handler[i]--;
			} else if (sig_interest[i]) {
				/*
				 * There is a waiting thread out there so we
				 * still have work to do
				 */
				should_wait++;
				if (!sig_handler[i]) {
					/*
					 * A new handler is needed. The calling
					 * thread should have blocked this
					 * signal. We establish a handler
					 * and unblock is again.
					 */
					sigaction(i, &handler, &old_handler[i]);
					sigemptyset(&set);
					sigaddset(&set, i);
					sigprocmask(SIG_UNBLOCK, &set, NULL);
					sig_handler[i]++;
				}
			}
		}

		/*
		 * If there is no interest in any signals (there will also
		 * be no handlers left established) then we can exit this
		 * thread. Clean up before we do. The wait_thread will be
		 * NULL'd by the detach.
		 */
		if (!should_wait) {
			pthread_detach(&wait_thread);
			pthread_mutex_unlock(&sigwait_lock);
			pthread_exit(0);
		}

		/*
		 * We have done all we can for now so go to sleep
		 * and wait for something to happen. Release the lock
		 * before we do.
		 */
		pthread_mutex_unlock(&sigwait_lock);
		vp_event_wait(wait_thread->vp, &event, NO_TIMEOUT);
		pthread_mutex_lock(&sigwait_lock);

		/*
		 * we have been sent an event. It will be either a signal
		 * arrival or another sigwaiter has joined us.
		 */
		if (event&EVT_SIGWAIT) {
			/*
			 * If there is a signal, make it part of out pending
			 * set and broadcast to all sigwaiters
			 */
			signo = event&~EVT_SIGWAIT;
			sigaddset(&pending, signo);
			pthread_cond_broadcast(&signal_arrived);
		}
#ifdef DEBUG
		/*
		 * Check the only other legal event type
		 */
		else if (event != EVT_WAITER)
			pthread_internal_error("sigwait");
#endif
	}
	/* NOTREACHED */
}

/*
 * Function:
 *	sig_register
 *
 * Parameters:
 *	set - the set of signals that the thread wishes to wait for
 *
 * Return value:
 *	TRUE	the signals are registered
 *	FALSE	The sigwait thread could not be created
 *
 * Description:
 *	Add the set of interested signals to all those being waited for by
 *	all threads. If there are new signals being added to this set then
 *	create the waiter thread if it is not already running or notify it
 *	that there are more signals if it is. The sigwait_lock must be held
 *	when this function is called.
 */
private int
sig_register(sigset_t *set)
{
	int	i;
	int	notify_waiter;

	/*
	 * add all the signals in the set passed to our global set, remembering
	 * if there are any new ones.
	 */
	notify_waiter = 0;
	for (i = 1; i < SIGMAX; i++)
		if (sigismember(set, i))
			if (++sig_interest[i] == 1)
				notify_waiter++;

	if (notify_waiter) {
		/*
		 * We have new signals, create the thread if it is not running
		 */
		if (wait_thread == NO_PTHREAD)
			if (pthread_create(&wait_thread, pthread_attr_default,
					sigwait_thread, NULL) < 0) {
				pthread_mutex_unlock(&sigwait_lock);
				return(FALSE);
			}
		/*
		 * Notify the waiting thread that there has been a change in
		 * the number of signals in the interest set.
		 */
		vp_event_notify(wait_thread->vp, EVT_WAITER);
	}
	return(TRUE);
}

/*
 * Function:
 *	sig_unregister
 *
 * Parameters:
 *	set - the signals to remove from the global interest set
 *
 * Description:
 *	Remove all the signals from the globals set noting if there are
 *	signals that no-one is now interested in. If so we notify the waiter
 *	thread so that it can remove the handler. The sigwait_lock must be
 *	held by the caller of this function.
 */
private void
sig_unregister(sigset_t *set)
{
	int	i;
	int	notify_waiter;

	/*
	 * remove all the signals from the set
	 */
	notify_waiter = 0;
	for (i = 1; i < SIGMAX; i++)
		if (sigismember(set, i))
			if (--sig_interest[i] == 0)
				notify_waiter++;

	/*
	 * If there is a signal that no-one is interested in then wake up
	 * the waiter thread
	 */
	if (notify_waiter)
		vp_event_notify(wait_thread->vp, EVT_WAITER);
}

/*
 * Function:
 *	sig_cleanup
 *
 * Parameters:
 *	set - the set of signals this thread was waiting for.
 *
 * Description:
 *	This is a cancellation cleanup handler in case the sigwaiter is
 *	cancelled. This unregisters the signals that this thread was waiting
 *	for before it was cancelled.
 */
private void
sig_cleanup(sigset_t *set)
{
	/*
	 * sig_unregister() must be called with the sigwait_lock held.
	 */
	pthread_mutex_lock(&sigwait_lock);

	sig_unregister(set);

	pthread_mutex_unlock(&sigwait_lock);
}

/*
 * Function:
 *	sigwait
 *
 * Parameters:
 *	set - the set of signals the calling thread wishes to wait for.
 *
 * Return value:
 *	-1	The signals could not be registered
 *	The signal number of one in set that arrived.
 *
 * Description:
 *	The calling thread registers the signals that it is interested in which
 *	may start a separate thread to catch them. This other thread broadcasts
 *	the arrival of signals (which are put in the pending set) with a
 *	condition variable. When this thread is woken up it scans the pending
 *	set looking for a signal that it is interested in. If there are no
 *	interesting signals the thread waits on the condition again.
 */
int
sigwait(sigset_t *set)
{
	int	mysig;
	int	i;
	int	signal_caught;

	pthread_mutex_lock(&sigwait_lock);

	/*
	 * Try to register the set passed
	 */
	if (!sig_register(set))
		return(-1);

	/*	
	 * The set is registered. Waiting on a condition is a cancellation
	 * point so a cleanup handler is needed to unregister the set before
	 * the thread goes away.
	 */
	pthread_cleanup_push(sig_cleanup, set);

	signal_caught = FALSE;
	while (!signal_caught) {
		/*
		 * Loop until a signal arrives that we are interested in
		 * but first wait for one to arrive
		 */
		pthread_cond_wait(&signal_arrived, &sigwait_lock);
		/*
		 * A signal arrived. Scan the pending set
		 */
		for (i = 1; i < SIGMAX; i++)
			if (sigismember(set, i) && sigismember(&pending, i)) {
				/*
				 * We want this one. Remove it from the pending
				 * set and remember which one it was.
				 */
				sigdelset(&pending, i);
				mysig = i;
				signal_caught = TRUE;
			}
	}

	/*
	 * We have our signal. Remove the cleanup handler and unregister
	 * out set of signals.
	 */
	pthread_cleanup_pop(0);
	sig_unregister(set);

	pthread_mutex_unlock(&sigwait_lock);
	return(mysig);
}
