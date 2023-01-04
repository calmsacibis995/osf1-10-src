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
static char	*sccsid = "@(#)$RCSfile: mach_net.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:25:48 $";
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

#include <sys/secdefines.h>
#if SEC_BASE
#include <sys/security.h>
#endif
#include <sys/param.h>
#include <sys/user.h>
#include <sys/time.h>
#include <sys/mbuf.h>

#include <mach/boolean.h>
#include <kern/kern_port.h>
#include <kern/zalloc.h>
#include <kern/kern_msg.h>

#include <vm/vm_kern.h>	/* kernel_map */
#include <kern/parallel.h>

#include <mach_debug/ipc_statistics.h>

#if	0
/* WRONG WRONG WRONG - SECURITY ALERT */
#define MACH_NET_GID_1	2	/* sys */
#define MACH_NET_GID_2	38	/* network */

#if SEC_BASE
#define MACH_NET_AUTH() (privileged(SEC_REMOTE, 0) || \
				groupmember(MACH_NET_GID_1, u.u_cred) ||  \
				groupmember(MACH_NET_GID_2, u.u_cred))
#else /* !SEC_BASE */
#define MACH_NET_AUTH() (suser(u.u_cred, &u.u_acflag) == 0 || \
				groupmember(MACH_NET_GID_1, u.u_cred) ||  \
				groupmember(MACH_NET_GID_2, u.u_cred))
#endif /* !SEC_BASE */

#else

#if SEC_BASE
#define MACH_NET_AUTH()	(privileged(SEC_REMOTE, 0)) /* XXX wrong priv? */
#else /* !SEC_BASE */
#define MACH_NET_AUTH() (suser(u.u_cred, &u.u_acflag) == 0)
#endif /* !SEC_BASE */

#endif

#define IP_MSG_ID	1959

/* Referenced in order to allow dynamic attach */
int	(*send_mach_net_datagram)();

/*ARGSUSED*/
boolean_t
mach_net_server(in_msg, out_msg)
	msg_header_t	*in_msg;
	msg_header_t	*out_msg;
{
	int (*sendit)() = send_mach_net_datagram;

	if (!sendit || !in_msg->msg_simple ||
	    in_msg->msg_id != IP_MSG_ID ||
	    in_msg->msg_size <= sizeof (*in_msg) ||
	    !MACH_NET_AUTH())
		return(FALSE);

	return (*sendit)((caddr_t)(in_msg + 1),
			 in_msg->msg_size - sizeof *in_msg);
}

/*
 *	Data structures and associated routines
 *	to register IPC ports for incoming datagrams.
 *
 *	Listeners are organized into buckets by the (IP) protocol number
 *	for which they are listening. Within a bucket, the listeners
 *	are linked in a list that is maintained by a move-to-front strategy.
 *
 *	A zero entry in any of the source or destination
 *	fields indicates a wildcard that matches the corresponding field
 *	in any incoming datagram.
 *
 *	The lock field is used for mutual exclusion when adding listeners.
 *	Lookups do not require synchronization because the ipc_port field
 *	(used to tell whether a listener is present) is always updated last,
 *	in a single operation.
 *
 *	The routines to add and remove listeners are called by the user
 *	via a Matchmaker interface. Remove_listener() is also called by
 *	port_deallocate().
 */

struct lbucket {
	struct listener {
		struct listener	*next;
		long		src_addr;
		long		dst_addr;
		unsigned short	src_port;
		unsigned short	dst_port;
		port_t		ipc_port;
	} *head;
	decl_simple_lock_data(,	lock)
} listeners[256];			/* indexed by protocol */

#define LISTENER_MAX	100
#define LISTENER_CHUNK	1

zone_t listener_zone;

#define FOR_EACH_BUCKET(b)	for((b)=listeners; (b) < &listeners[256]; (b)++)
#define FOR_EACH_LISTENER(lp,b)	for((lp)=(b)->head; (lp) != 0; (lp)=(lp)->next)
#define LOCK_BUCKET(b, ipl)	{ ipl = splnet(); simple_lock(&(b)->lock); }
#define UNLOCK_BUCKET(b, ipl)	{ simple_unlock(&(b)->lock); splx(ipl); }

/*
 *	Register a listener.
 *	No checking is done to prevent conflict with existing listeners.
 */
/*ARGSUSED*/
kern_return_t
netipc_listen(server_port, _src_addr, _dst_addr,
			      _src_port, _dst_port, _protocol, ipc_port)
	port_t		server_port;	/* unused */
	int		_src_addr;
	int		_dst_addr;
	int		_src_port;
	int		_dst_port;
	int		_protocol;
	port_t		ipc_port;
{
	register struct listener *lp;
	register struct lbucket *b;
	int ipl;

	/* Translate from int's to network types - XXX! */
	u_long		src_addr = _src_addr;
	u_long		dst_addr = _dst_addr;
	u_short		src_port = _src_port;
	u_short		dst_port = _dst_port;
	u_char		protocol = _protocol;

	if (!MACH_NET_AUTH())
		return(KERN_NO_ACCESS);
	if (ipc_port == PORT_NULL)
		return(KERN_INVALID_ARGUMENT);
	lp = (struct listener *) zalloc(listener_zone);
	lp->src_addr = src_addr;
	lp->src_port = src_port;
	lp->dst_addr = dst_addr;
	lp->dst_port = dst_port;
	port_reference(lp->ipc_port = ipc_port);
	b = &listeners[protocol];
	LOCK_BUCKET(b, ipl);
	lp->next = b->head;
	b->head = lp;
	UNLOCK_BUCKET(b, ipl);
	port_object_set(ipc_port, PORT_OBJECT_NET, 0);
	return(KERN_SUCCESS);
}

/*
 *	Remove all listeners with the specified port.
 */
/*ARGSUSED*/
kern_return_t
netipc_ignore(server_port, ipc_port)
	port_t		server_port;	/* unused */
	register port_t	ipc_port;
{
	register struct lbucket *b;
	register struct listener *prev, *lp;
	kern_return_t result = KERN_FAILURE;
	int ipl;

	if (ipc_port == PORT_NULL)
		return KERN_INVALID_ARGUMENT;
	FOR_EACH_BUCKET (b) {
		LOCK_BUCKET(b, ipl);
		prev = b->head;
		FOR_EACH_LISTENER (lp, b) {
			if (lp->ipc_port == ipc_port) {
				result = KERN_SUCCESS;
				if (lp == b->head) {
					b->head = lp->next;
					zfree(listener_zone, (vm_offset_t) lp);
					if ((prev = lp = b->head) == 0)
						break;
				} else {
					prev->next = lp->next;
					zfree(listener_zone, (vm_offset_t) lp);
					lp = prev;
				}
				port_release(ipc_port);
			} else
				prev = lp;
		}
		UNLOCK_BUCKET(b, ipl);
	}
	return(result);
}

/*
 *	Find a listener for the given source, destination, and protocol.
 */
port_t
find_listener(src_addr, src_port, dst_addr, dst_port, protocol)
	u_long		src_addr;
	u_short		src_port;
	u_long		dst_addr;
	u_short		dst_port;
	u_char		protocol;
{
	register struct lbucket *b;
	register struct listener *prev, *lp;

#define ADDR_MATCH(lp, addr)	((lp)->addr==0 || (lp)->addr==(addr))
#define PORT_MATCH(lp, port)	((lp)->port==0 || (lp)->port==(port))

	b = &listeners[protocol];
	prev = b->head;
	FOR_EACH_LISTENER(lp, b) {
		/*
		 *	The following tests should be ordered
		 *	in increasing likelihood of success.
		 */
		if (PORT_MATCH(lp, dst_port) && PORT_MATCH(lp, src_port) &&
		    ADDR_MATCH(lp, src_addr) && ADDR_MATCH(lp, dst_addr)) {
			if (lp != b->head) {
				/* move to front */
				prev->next = lp->next;
				lp->next = b->head;
				b->head = lp;
			}
			return(lp->ipc_port);
		} else
			prev = lp;
	}
	return(PORT_NULL);
}

/*
 *	Skeletal IPC message header for fast initialization.
 */
static msg_header_t ip_msg_template;

#define MACH_NET_MSG_SIZE_MAX	2048
zone_t	mach_net_kmsg_zone;

void
mach_net_init()
{
	register struct lbucket *b;
	
	listener_zone = zinit(sizeof(struct listener),
				LISTENER_MAX * sizeof(struct listener),
				LISTENER_CHUNK * sizeof(struct listener),
				"net listener zone");
	ip_msg_template.msg_simple = TRUE;
	ip_msg_template.msg_size = MACH_NET_MSG_SIZE_MAX -
				sizeof(struct kern_msg) + sizeof(msg_header_t);
	ip_msg_template.msg_type = MSG_TYPE_NORMAL;
	ip_msg_template.msg_local_port = PORT_NULL;
	ip_msg_template.msg_remote_port = PORT_NULL;
	ip_msg_template.msg_id = IP_MSG_ID;
	FOR_EACH_BUCKET (b) {
		simple_lock_init(&b->lock);
	}

	/*
	 * Create and allocate some space for our private kmsg zone.
	 */
	mach_net_kmsg_zone = zinit(MACH_NET_MSG_SIZE_MAX,
				MACH_NET_MSG_SIZE_MAX * 8,
				MACH_NET_MSG_SIZE_MAX,
				"mach_net messages");
	zcram(mach_net_kmsg_zone, kmem_alloc(kernel_map,
				MACH_NET_MSG_SIZE_MAX * 8),
				MACH_NET_MSG_SIZE_MAX * 8);
}

void
receive_mach_net_datagram(port, m)
	port_t	port;
	struct mbuf *m;
{
#if	!NETISR_THREAD
	/* Cannot lower spl, so don't try. */
	m_freem(m);
	return;
#else
	kern_msg_t		kmsgptr;
	register caddr_t	cp;
	int			space;
	int			s = spl0();	/* Insurance, but dangerous! */

	kmsgptr = (kern_msg_t) zget(mach_net_kmsg_zone);
	if (kmsgptr == KERN_MSG_NULL) {
		m_freem(m);
		splx(s);
		return;
	}
	kmsgptr->home_zone = mach_net_kmsg_zone;

	ipc_event(current);
	ipc_event(ip_data_grams);

	/*
	 *	Copy mbuf chain to kmsg buffer.
	 */
	space = MACH_NET_MSG_SIZE_MAX - sizeof(struct kern_msg);
	cp = (caddr_t)(&kmsgptr->kmsg_header + 1);
	do {
		register unsigned int len = MIN(m->m_len, space);
		bcopy(mtod(m, caddr_t), cp, len);
		cp += len;
		space -= len;
		m = m_free(m);
	} while (m && space);
	if (m)
		m_freem(m);
	/*
	 *	Fill in message header and deliver it.
	 */
	kmsgptr->kmsg_header = ip_msg_template;
	kmsgptr->kmsg_header.msg_size -= space;
	kmsgptr->kmsg_header.msg_local_port = port;

	port_reference(port);
	(void) msg_queue(kmsgptr, SEND_ALWAYS, 0);

	splx(s);
#endif
}
