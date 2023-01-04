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
static char	*sccsid = "@(#)$RCSfile: msg.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 93/03/02 16:10:03 $";
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

#include <mach/kern_return.h>
#include <mach/message.h>
#include <sys/time.h>

extern msg_return_t msg_send_trap();
extern msg_return_t msg_receive_trap();
extern msg_return_t msg_rpc_trap();

msg_return_t	msg_send(header, option, timeout)
	msg_header_t	*header;
	msg_option_t	option;
	int		timeout;	/* milli seconds */
{
	register msg_return_t	result;
	struct timeval		time;
	long			start_time, elapsed_time;
	int			otimeout;

	if (option & SEND_TIMEOUT) {
		(void) gettimeofday(&time, 0);
		start_time = time.tv_sec * 1000 + time.tv_usec / 1000;
		otimeout = timeout;
	}

	while ((result = msg_send_trap(header, option, header->msg_size,
						timeout)) == SEND_INTERRUPTED) {
		if (option & SEND_INTERRUPT)
			break;
		if (option & SEND_TIMEOUT) {
			(void) gettimeofday(&time, (struct timezone *)0);
			elapsed_time = (time.tv_sec*1000 + time.tv_usec/1000)
								- start_time;
			if (elapsed_time >= 0 && elapsed_time < otimeout)
				timeout = otimeout - elapsed_time;
			else
                                return RCV_TIMED_OUT;
		}
	}

	return result;
}

msg_return_t	msg_receive(header, option, timeout)
	msg_header_t	*header;
	msg_option_t	option;
	int		timeout;	/* milli seconds */
{
	register msg_return_t	result;
	struct timeval		time;
	long			start_time, elapsed_time; /* milli seconds */
	int			otimeout; /* in milli seconds */

	if (option & RCV_TIMEOUT) {
		(void) gettimeofday(&time, 0);
		start_time = time.tv_sec * 1000 + time.tv_usec / 1000;
		otimeout = timeout;
	}

	while ((result = msg_receive_trap(header, option, header->msg_size,
					  header->msg_local_port, timeout))
							== RCV_INTERRUPTED) {
		if (option & RCV_INTERRUPT)
			break;
		if (option & RCV_TIMEOUT) {
			(void) gettimeofday(&time, (struct timezone *)0);
			elapsed_time = (time.tv_sec*1000 + time.tv_usec/1000)
								- start_time;
			if (elapsed_time >= 0 && elapsed_time < otimeout)
				timeout = otimeout - elapsed_time;
			else
                                return RCV_TIMED_OUT;
		}
	}

	return result;
}

msg_return_t	msg_rpc(header, option, rcv_size, send_timeout, rcv_timeout)
	msg_header_t	*header;
	msg_option_t	option;
	int		rcv_size;
	int		send_timeout;	/* milli seconds */
	int		rcv_timeout;	/* milli seconds */
{
	register msg_return_t	result;
	struct timeval		time;
	long			start_time, elapsed_time; /* milli seconds */
	int			osend_timeout, orcv_timeout;

	if (option & (SEND_TIMEOUT|RCV_TIMEOUT)) {
		(void) gettimeofday(&time, 0);
		start_time = time.tv_sec * 1000 + time.tv_usec / 1000;
		osend_timeout = send_timeout;
		orcv_timeout  = rcv_timeout;
	}

	while ((result = msg_rpc_trap(header, option, header->msg_size,
				      rcv_size, send_timeout, rcv_timeout))
							== SEND_INTERRUPTED) {
		if (option & SEND_INTERRUPT)
			break;
		if (option & SEND_TIMEOUT) {
			(void) gettimeofday(&time, (struct timezone *)0);
			elapsed_time = (time.tv_sec*1000 + time.tv_usec/1000)
								- start_time;
			if (elapsed_time >= 0 && elapsed_time < osend_timeout)
				send_timeout = osend_timeout - elapsed_time;
			else
                                return SEND_TIMED_OUT;
		}
	}

	if ((result == RCV_INTERRUPTED) && !(option & RCV_INTERRUPT)) do {
		if (option & RCV_TIMEOUT) {
			(void) gettimeofday(&time, (struct timezone *)0);
			elapsed_time = (time.tv_sec*1000 + time.tv_usec/1000)
								- start_time;
			if (elapsed_time >= 0 && elapsed_time < orcv_timeout)
				rcv_timeout = orcv_timeout - elapsed_time;
			else
                                return RCV_TIMED_OUT;
		}
		result = msg_receive_trap(header, option, rcv_size,
						  header->msg_local_port,
						  rcv_timeout);
	} while (result == RCV_INTERRUPTED);

	return result;
}
