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
static char	*sccsid = "@(#)$RCSfile: evt.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/11/20 12:10:33 $";
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
 * File: evt.c
 *
 * This file contains the functions to suspend a vp and wake a vp for a
 * specified reason. This is simply a cover for send a message on a port.
 * When a vp is allocated, and event port is also allocated.
 */

#define POSIX_4D9
#include <pthread.h>
#include <sys/timers.h>
#include <mach/message.h>
#include <errno.h>
#include "internal.h"

#define	SECONDS_TO_MILLISECONDS(s)	((s) * 1000)
#define	SECONDS_TO_NANOSECONDS(s)	((s) * 1000000000)
#define	NANOSECONDS_TO_MILLISECONDS(n)	(((n) + 999999) / 1000000)
#define	MILLISECONDS_TO_NANOSECONDS(m)	((m) * 1000000)

/*
 * Function:
 *	vp_event_wait
 *
 * Parameters:
 *	vp - the vp structure of the calling thread.
 *	message - a pointer to where to store the event message
 *	timeout - A pointer to a timeout value. This may be NULL.
 *
 * Description:
 *	The action of wating for an event simply involves waiting on
 *	the vp's event port. There may be a timeout specified in
 *	absolute time in the struct timespec. If this timeout is specified
 *	and now then the call returns without waiting on the port.
 */
void
vp_event_wait(vp_t vp, int *message, struct timespec *timeout)
{
	kern_return_t	status;
	msg_header_t	msg;
	msg_option_t	option;
	msg_timeout_t	wait;
	struct timespec	now;
	long		secs, nsecs;

	/*
	 * set up the message structure
	 */
	msg.msg_size = sizeof(msg);
	msg.msg_local_port = vp->event_port;
	/*
	 * Calculate the timeout period if specified.
	 */
	if (timeout != NO_TIMEOUT) {
		if (getclock(TIMEOFDAY, &now) != 0 )
			pthread_internal_error("vp_event_wait: getclock failed");
		nsecs = timeout->tv_nsec - now.tv_nsec;
		if (timeout->tv_nsec >= now.tv_nsec) {
			timeout->tv_sec--;
			nsecs += SECONDS_TO_NANOSECONDS(1);
		}
		secs = timeout->tv_sec - now.tv_sec;
		if (secs < 0)
			wait = 0;
		else
			wait = SECONDS_TO_MILLISECONDS(secs) +
				NANOSECONDS_TO_MILLISECONDS(nsecs);

		if (wait == 0)
			wait = 1;

		option = RCV_TIMEOUT;
	} else {
		wait = 0;
		option = MSG_OPTION_NONE;
	}
	/*
	 * Wait for the event. When it arrives check for a timeout, if
	 * not the event is the message id.
	 */
	status = msg_receive(&msg, option, wait);
	switch (status) {
	case	RCV_TIMED_OUT:
		*message = EVT_TIMEOUT;
		break;
	case	RCV_SUCCESS:
		*message = msg.msg_id;
		break;
	default:
		pthread_internal_error("vp_event_wait");
	}
}

/*
 * Function:
 *	vp_event_notify
 *
 * Parameters:
 *	vp - the vp structure of the suspended vp
 *	message - the event that the vp is to receive
 *
 * Description:
 *	The message is sent on the event port of the target vp. The message
 *	is parcelled up in a msg_header and sent as the ipc message id.
 */
void
vp_event_notify(vp_t vp, int message)
{
	kern_return_t	status;
	msg_header_t	msg;

	/*
	 * set up the ipc message header. Put the event message in the
	 * ipc message id field.
	 */
	msg.msg_simple = TRUE;
	msg.msg_size = sizeof(msg);
	msg.msg_type = MSG_TYPE_NORMAL;
	msg.msg_local_port = PORT_NULL;
	msg.msg_remote_port = vp->event_port;
	msg.msg_id = message;
	/*
	 * Send the message to the target vp and check that the send was good.
	 * It does not matter whether the target vp is blocked yet
	 * or not as this is a queue. We know that it will call
	 * vp_event_wait sometime.
	 */
	status = msg_send(&msg, SEND_TIMEOUT, 0);
	if (status != SEND_SUCCESS && status != SEND_TIMED_OUT) {
		mach_error("msg_send", status);
		pthread_internal_error("vp_event_notify");
	}
}

/*
 * Function:
 *	vp_event_flush
 *
 * Parameters:
 *	vp - the vp structure of the calling thread.
 *
 * Description:
 *	We are expecting (normally one) bogus event on the queue caused by
 *	a race between timeouts and cancellation. Get all the messages
 *	and throw them away until there aren't any left.
 */
void
vp_event_flush(vp_t vp)
{
	struct timespec timeout;
	int		message;

	/*
	 * What we really want is an option to msg_receive which returns if
	 * the port is empty but that doesn't exist so we use a timeout instead.
	 */
	if (getclock(TIMEOFDAY, &timeout) != 0 )
		pthread_internal_error("vp_event_flush: getclock failed");

	timeout.tv_nsec += MILLISECONDS_TO_NANOSECONDS(50);
	if (timeout.tv_nsec >= SECONDS_TO_NANOSECONDS(1)) {
		timeout.tv_sec++;
		timeout.tv_nsec -= SECONDS_TO_NANOSECONDS(1);
	}

	do {
		vp_event_wait(vp, &message, &timeout);
	} while (message != EVT_TIMEOUT);
}

/*
 * Function:
 *	allocate_event_port
 *
 * Parameters:
 *	port - pointer to where the port id should be stored.
 *
 * Return value:
 *	TRUE	Success
 *	FALSE	The port could not be allocated
 *
 * Description:
 *	Allocate a port and set it up to be used.
 *
 */
int
allocate_event_port(port_t *port)
{
	/*
	 * allocate a new port on which to receive events
	 */
	if (port_allocate(task_self(), port) != KERN_SUCCESS) {
		set_errno(EAGAIN);
		return(FALSE);
	}

	if (port_disable(task_self(), *port) != KERN_SUCCESS)
		pthread_internal_error("port_disable");

	/*
	 * only allow one outstanding event to be sent to a thread
	 * at a time.
	 */
	if (port_set_backlog(task_self(), *port, 1) != KERN_SUCCESS)
		pthread_internal_error("port_set_backlog");

	return(TRUE);
}

/*
 * Function:
 *	deallocate_event_port
 *
 * Parameters:
 *	port - the id of the port to be deallocated
 *
 * Description:
 *	Return the event port to the system.
 */
void
deallocate_event_port(port_t port)
{
	/*
	 * This may fail as we could be called during the cleanup after
	 * a fork() in which case the port no longer belongs to our task.
	 * So we ignore any error that could occur.
	 */
	port_deallocate(task_self(), port);
}


/*
 * Function:
 *	sleep
 *
 * Parameters:
 *	secs - the number of seconds to sleep
 *
 * Return value:
 *	always 0
 *
 * Description:
 *	wait on the event port with a timeout, knowing that a message will
 *	not arrive
 */
unsigned int
sleep(unsigned int secs)
{
	struct timespec timeout;
	int		message;
	vp_t		vp;

	if (getclock(TIMEOFDAY, &timeout) != 0 )
		pthread_internal_error("vp_event_flush: getclock failed");

	timeout.tv_sec += secs;

	vp = vp_self();
	vp_event_wait(vp, &message, &timeout);
	return(0);
}
