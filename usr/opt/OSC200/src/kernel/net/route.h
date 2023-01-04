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
 *	@(#)$RCSfile: route.h,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/12/17 00:00:09 $
 */ 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
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
 * Copyright (c) 1980, 1986 Regents of the University of California.
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
 *
 *	Base:	route.h	7.4 (Berkeley) 6/27/88
 *	Merged:	route.h	7.11 (Berkeley) 6/28/90
 */

#ifndef _ROUTE_H_
#define _ROUTE_H_
/*
 * Kernel resident routing tables.
 * 
 * The routing tables are initialized when interface addresses
 * are set by making entries for all directly connected interfaces.
 */

/*
 * A route consists of a destination address and a reference
 * to a routing entry.  These are often held by protocols
 * in their control blocks, e.g. inpcb.
 */
struct route {
	struct	rtentry *ro_rt;
	struct	sockaddr ro_dst;
};

/*
 * These numbers are used by reliable protocols for determining
 * retransmission behavior and are included in the routing structure.
 */
struct rt_metrics {
	u_int	rmx_locks;	/* Kernel must leave these values alone */
	u_int	rmx_mtu;	/* MTU for this path */
	u_int	rmx_hopcount;	/* max hops expected */
	u_int	rmx_expire;	/* lifetime for route, e.g. redirect */
	u_int	rmx_recvpipe;	/* inbound delay-bandwith product */
	u_int	rmx_sendpipe;	/* outbound delay-bandwith product */
	u_int	rmx_ssthresh;	/* outbound gateway buffer limit */
	u_int	rmx_rtt;	/* estimated round trip time */
	u_int	rmx_rttvar;	/* estimated rtt variance */
};

/*
 * rmx_rtt and rmx_rttvar are stored as microseconds;
 * RTTTOPRHZ(rtt) converts to a value suitable for use
 * by a protocol slowtimo counter.
 */
#define	RTM_RTTUNIT	1000000	/* units for rtt, rttvar, as units per sec */
#define	RTTTOPRHZ(r)	((r) / (RTM_RTTUNIT / PR_SLOWHZ))

/*
 * We distinguish between routes to hosts and routes to networks,
 * preferring the former if available.  For each route we infer
 * the interface to use from the gateway address supplied when
 * the route was entered.  Routes that forward packets through
 * gateways are marked so that the output routines know to address the
 * gateway rather than the ultimate destination.
 */
#include "net/radix.h"
struct rtentry {
	struct	radix_node rt_nodes[2];	/* tree glue, and other values */
#define	rt_key(r)	((struct sockaddr *)((r)->rt_nodes->rn_key))
#define	rt_mask(r)	((struct sockaddr *)((r)->rt_nodes->rn_mask))
	struct	sockaddr *rt_gateway;	/* value */
	short	rt_flags;		/* up/down?, host/net */
	short	rt_refcnt;		/* # held references */
	u_int	rt_use;			/* raw # packets forwarded */
	struct	ifnet *rt_ifp;		/* the answer: interface to use */
	struct	ifaddr *rt_ifa;		/* the answer: interface to use */
	struct	sockaddr *rt_genmask;	/* for generation of cloned routes */
	caddr_t	rt_llinfo;		/* pointer to link level info cache */
	struct	rt_metrics rt_rmx;	/* metrics used by rx'ing protocols */
	short	rt_idle;		/* easy to tell llayer still live */
};

/*
 * Following structure necessary for 4.3 compatibility;
 * We should eventually move it to a compat file.
 */
struct ortentry {
	u_int	rt_hash;		/* to speed lookups */
	struct	sockaddr rt_dst;	/* key */
	struct	sockaddr rt_gateway;	/* value */
	short	rt_flags;		/* up/down?, host/net */
	short	rt_refcnt;		/* # held references */
	u_int	rt_use;			/* raw # packets forwarded */
	struct	ifnet *rt_ifp;		/* the answer: interface to use */
};

#define	RTF_UP		0x1		/* route useable */
#define	RTF_GATEWAY	0x2		/* destination is a gateway */
#define	RTF_HOST	0x4		/* host entry (net otherwise) */
#define	RTF_DYNAMIC	0x10		/* created dynamically (by redirect) */
#define	RTF_MODIFIED	0x20		/* modified dynamically (by redirect) */
#define RTF_DONE	0x40		/* message confirmed */
#define RTF_MASK	0x80		/* subnet mask present */
#define RTF_CLONING	0x100		/* generate new routes on use */
#define RTF_XRESOLVE	0x200		/* external daemon resolves name */
#define RTF_LLINFO	0x400		/* generated by ARP or ESIS */


/*
 * Routing statistics.
 */
struct	rtstat {
	short	rts_badredirect;	/* bogus redirect calls */
	short	rts_dynamic;		/* routes created by redirects */
	short	rts_newgateway;		/* routes modified by redirects */
	short	rts_unreach;		/* lookups which failed */
	short	rts_wildcard;		/* lookups satisfied by a wildcard */
#if	defined(_KERNEL) && LOCK_NETSTATS
	simple_lock_data_t rts_lock;		/* statistics lock */
#endif
};

/*
 * Structures for routing messages.
 */
struct rt_msghdr {
	u_short	rtm_msglen;	/* to skip over non-understood messages */
	u_char	rtm_version;	/* future binary compatability */
	u_char	rtm_type;	/* message type */
	u_short	rtm_index;	/* index for associated ifp */
#ifdef __alpha
	int	rtm_pid;	/* identify sender */
#else
	pid_t	rtm_pid;	/* identify sender */
#endif
	int	rtm_addrs;	/* bitmask identifying sockaddrs in msg */
	int	rtm_seq;	/* for sender to identify action */
	int	rtm_errno;	/* why failed */
	int	rtm_flags;	/* flags, incl. kern & message, e.g. DONE */
	int	rtm_use;	/* from rtentry */
	u_int	rtm_inits;	/* which metrics we are initializing */
	struct	rt_metrics rtm_rmx; /* metrics themselves */
};

struct route_cb {
	int	count[AF_MAX];
};
#define	ip_count	count[AF_INET]
#define	ns_count	count[AF_NS]
#define	iso_count	count[AF_ISO]
#define	any_count	count[AF_UNSPEC]

#define RTM_VERSION	2	/* Up the ante and ignore older versions */

#define RTM_ADD		0x1	/* Add Route */
#define RTM_DELETE	0x2	/* Delete Route */
#define RTM_CHANGE	0x3	/* Change Metrics or flags */
#define RTM_GET		0x4	/* Report Metrics */
#define RTM_LOSING	0x5	/* Kernel Suspects Partitioning */
#define RTM_REDIRECT	0x6	/* Told to use different route */
#define RTM_MISS	0x7	/* Lookup failed on this address */
#define RTM_LOCK	0x8	/* fix specified metrics */
#define RTM_OLDADD	0x9	/* caused by SIOCADDRT */
#define RTM_OLDDEL	0xa	/* caused by SIOCDELRT */
#define RTM_RESOLVE	0xb	/* req to resolve dst to LL addr */

#define RTV_MTU		0x1	/* init or lock _mtu */
#define RTV_HOPCOUNT	0x2	/* init or lock _hopcount */
#define RTV_EXPIRE	0x4	/* init or lock _hopcount */
#define RTV_RPIPE	0x8	/* init or lock _recvpipe */
#define RTV_SPIPE	0x10	/* init or lock _sendpipe */
#define RTV_SSTHRESH	0x20	/* init or lock _ssthresh */
#define RTV_RTT		0x40	/* init or lock _rtt */
#define RTV_RTTVAR	0x80	/* init or lock _rttvar */

#define RTA_DST		0x1	/* destination sockaddr present */
#define RTA_GATEWAY	0x2	/* gateway sockaddr present */
#define RTA_NETMASK	0x4	/* netmask sockaddr present */
#define RTA_GENMASK	0x8	/* cloning mask sockaddr present */
#define RTA_IFP		0x10	/* interface name sockaddr present */
#define RTA_IFA		0x20	/* interface addr sockaddr present */
#define RTA_AUTHOR	0x40	/* sockaddr for author of redirect */

#define	RT_ROUNDUP(sa)	\
	((int)(((sa)->sa_len + (sizeof(long) - 1)) & ~(sizeof(long) - 1)))

#ifdef _KERNEL
#if	NETSYNC_LOCK
extern lock_data_t	route_lock;
#define ROUTE_LOCKINIT()	lock_init2(&route_lock, TRUE, LTYPE_ROUTE)
#define	ROUTE_LOCK_DECL()	NETSPL_DECL(_rs)
#define ROUTE_READ_LOCK()	{ NETSPL(_rs,net); lock_read(&route_lock); }
#define ROUTE_READ_UNLOCK()	{ lock_done(&route_lock); NETSPLX(_rs); }
#define ROUTE_WRITE_LOCK() { \
	NETSPL(_rs,net); \
	lock_write(&route_lock); \
	lock_set_recursive(&route_lock); \
}
#define ROUTE_WRITE_UNLOCK() { \
	lock_clear_recursive(&route_lock); \
	lock_done(&route_lock); \
	NETSPLX(_rs); \
}
#define ROUTE_WRITETOREAD_LOCK() { \
	lock_clear_recursive(&route_lock); \
	lock_write_to_read(&route_lock); \
}
#define RTFREE(rt)		rtfree(rt)
#else	/* !NETSYNC_LOCK */
#define ROUTE_LOCKINIT()
#define	ROUTE_LOCK_DECL()	NETSPL_DECL(_rs)
#define ROUTE_READ_LOCK()	NETSPL(_rs,net)
#define ROUTE_READ_UNLOCK()	NETSPLX(_rs)
#define ROUTE_WRITE_LOCK()	NETSPL(_rs,net)
#define ROUTE_WRITE_UNLOCK()	NETSPLX(_rs)
#define ROUTE_WRITETOREAD_LOCK()
#define	RTFREE(rt) \
	if ((rt)->rt_refcnt <= 1) \
		rtfree(rt); \
	else \
		(rt)->rt_refcnt--;
#endif

#ifdef	GATEWAY
#define	RTHASHSIZ	64
#else
#define	RTHASHSIZ	8
#endif
#if	(RTHASHSIZ & (RTHASHSIZ - 1)) == 0
#define RTHASHMOD(h)	((h) & (RTHASHSIZ - 1))
#else
#define RTHASHMOD(h)	((h) % RTHASHSIZ)
#endif
extern	struct	route_cb route_cb;
extern	struct	mbuf *rthost[RTHASHSIZ];
extern	struct	mbuf *rtnet[RTHASHSIZ];
extern	struct	rtstat	rtstat;
#endif

#endif
