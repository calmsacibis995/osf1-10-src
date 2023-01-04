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
/*
 *	@(#)$RCSfile: server_loop.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:59:46 $
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
 *	File:	kern/server_loop.c
 *
 *	A common server loop for builtin pagers.
 */

/*
 *	Must define symbols for:
 *		SERVER_NAME		String name of this module
 *		TERMINATE_FUNCTION	How to handle port death
 *		SERVER_LOOP		Routine name for the loop
 *		SERVER_DISPATCH		MiG function(s) to handle message
 *
 *	May optionally define:
 *		RECEIVE_OPTION		Receive option (default NONE)
 *		RECEIVE_TIMEOUT		Receive timeout (default 0)
 *		TIMEOUT_FUNCTION	Timeout handler (default null)
 *
 *	Must redefine symbols for pager_server functions.
 */

#ifndef	RECEIVE_OPTION
#define RECEIVE_OPTION	MSG_OPTION_NONE
#endif	RECEIVE_OPTION

#ifndef	RECEIVE_TIMEOUT
#define RECEIVE_TIMEOUT	0
#endif	RECEIVE_TIMEOUT

#ifndef	TIMEOUT_FUNCTION
#define TIMEOUT_FUNCTION
#endif	TIMEOUT_FUNCTION

#include <mach/boolean.h>
#include <mach/port.h>
#include <mach/message.h>
#include <mach/notify.h>

void		SERVER_LOOP(rcv_set, do_notify)
	port_t		rcv_set;
	boolean_t	do_notify;
{
	msg_return_t	r;
	port_t		my_notify;
	port_t		my_self;
	vm_offset_t	messages;
	register
	msg_header_t	*in_msg;
	msg_header_t	*out_msg;

	/*
	 *	Find out who we are.
	 */

	my_self = task_self();

	if (do_notify) {
		my_notify = task_notify();

		if (port_set_add(my_self, rcv_set, my_notify)
						!= KERN_SUCCESS) {
			printf("%s: can't enable notify port", SERVER_NAME);
			panic(SERVER_NAME);
		}
	}

	/*
	 *	Allocate our message buffers.
	 *	[The buffers must reside in kernel space, since other
	 *	message buffers (in the mach_user_external module) are.]
	 */

	if ((messages = kmem_alloc(kernel_map, 2 * MSG_SIZE_MAX)) == 0) {
		printf("%s: can't allocate message buffers", SERVER_NAME);
		panic(SERVER_NAME);
	}
	in_msg = (msg_header_t *) messages;
	out_msg = (msg_header_t *) (messages + MSG_SIZE_MAX);

	/*
	 *	Service loop... receive messages and process them.
	 */

	for (;;) {
		in_msg->msg_local_port = rcv_set;
		in_msg->msg_size = MSG_SIZE_MAX;
		if ((r = msg_receive(in_msg, RECEIVE_OPTION, RECEIVE_TIMEOUT)) != RCV_SUCCESS) {
			if (r == RCV_TIMED_OUT) {
				TIMEOUT_FUNCTION ;
			} else {
				printf("%s: receive failed, 0x%x.\n", SERVER_NAME, r);
			}
			continue;
		}
		if (do_notify && (in_msg->msg_local_port == my_notify)) {
			notification_t	*n = (notification_t *) in_msg;
			switch(in_msg->msg_id) {
				case NOTIFY_PORT_DELETED:
					TERMINATE_FUNCTION(n->notify_port);
					break;
				default:
					printf("%s: wierd notification (%d)\n", SERVER_NAME, in_msg->msg_id);
					printf("port = 0x%x.\n", n->notify_port);
					break;
			}
			continue;
		}
		if (SERVER_DISPATCH(in_msg, out_msg) &&
		    (out_msg->msg_remote_port != PORT_NULL))
			msg_send(out_msg, MSG_OPTION_NONE, 0);
	}
}
