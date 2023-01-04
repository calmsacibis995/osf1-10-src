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
static char	*sccsid = "@(#)$RCSfile: in_pcb.c,v $ $Revision: 4.3.5.4 $ (DEC) $Date: 1993/12/21 21:50:23 $";
#endif
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *	Base:	@(#)in_pcb.c	9.2 (Berkeley) 6/3/91
 *	Merged:	in_pcb.c	7.13 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patches.
 *
 */

#include "machine/endian.h"
#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/route.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/in_pcb.h"
#include "netinet/in_var.h"
#include "netinet/ip_var.h"

#include "net/net_malloc.h"

LOCK_ASSERTL_DECL

CONST struct	in_addr zeroin_addr;
int in_pcbcachehit = 0; 
int in_pcbcachemiss = 0;

in_pcballoc(so, head)
	struct socket *so;
	struct inpcb *head;
{
	register struct inpcb *inp;

	NET_MALLOC(inp, struct inpcb *, sizeof *inp, M_PCB, M_NOWAIT);
	if (inp == NULL)
		return (ENOBUFS);
	bzero((caddr_t)inp, sizeof *inp);
	inp->inp_head = head;
	inp->inp_socket = so;
	INPCB_LOCKINIT(inp);
	INPCBRC_LOCKINIT(inp);
	inp->inp_refcnt = 1;
	INHEAD_WRITE_LOCK(head);
	insque(inp, head);
	so->so_pcb = (caddr_t)inp;
	INHEAD_WRITE_UNLOCK(head);
	return (0);
}
	
in_pcbbind(inp, nam)
	register struct inpcb *inp;
	struct mbuf *nam;
{
	register struct socket *so = inp->inp_socket;
	register struct inpcb *head = inp->inp_head;
	register struct sockaddr_in *sin;
	int error = 0;
	u_short lport = 0;
#define	ret(err)	{ error = (err); goto out; }

	LOCK_ASSERT("in_pcbbind", INPCB_ISLOCKED(inp));
	if (in_ifaddr == 0)
		return (EADDRNOTAVAIL);
	if (inp->inp_lport || inp->inp_laddr.s_addr != INADDR_ANY)
		return (EINVAL);
	if (nam == 0) {
		INHEAD_WRITE_LOCK(head);
		goto noname;
	}
	sin = mtod(nam, struct sockaddr_in *);
	if (nam->m_len != sizeof (*sin))
		return (EINVAL);
	if (sin->sin_addr.s_addr != INADDR_ANY) {
		lport = sin->sin_port;
		sin->sin_port = 0;		/* yech... */
		if (ifa_ifwithaddr((struct sockaddr *)sin) == 0) {
			if (!(IN_MULTICAST(ntohl(sin->sin_addr.s_addr))))
				return (EADDRNOTAVAIL);
		}
		sin->sin_port = lport;
	} else
		lport = sin->sin_port;
	INHEAD_WRITE_LOCK(head);
	if (lport) {
		u_short aport = ntohs(lport);
		int wild = 0;
		struct inpcb *inp2;

		/* GROSS */
		if (aport < IPPORT_RESERVED && !(so->so_state & SS_PRIV))
			ret(EACCES);
		/* even GROSSER, but this is the Internet */
		if ((so->so_options & SO_REUSEADDR) == 0 &&
		    ((so->so_proto->pr_flags & PR_CONNREQUIRED) == 0 ||
		     (so->so_options & SO_ACCEPTCONN) == 0))
			wild = INPLOOKUP_WILDCARD;
		inp2 = in_pcblookup(head,
		    zeroin_addr, 0, sin->sin_addr, lport, wild);
		if (inp2 && !((so->so_options & inp2->inp_socket->so_options)
				& SO_REUSEPORT)) {
			INPCBRC_UNREF(inp2);
			ret(EADDRINUSE);
		}
	}
	inp->inp_laddr = sin->sin_addr;
noname:
	if (lport == 0) {
		u_short firstport = 0;
		do {
			if (head->inp_lport++ < IPPORT_RESERVED ||
			    head->inp_lport > IPPORT_USERRESERVED)
				head->inp_lport = IPPORT_RESERVED;
			if (firstport == 0)
				firstport = head->inp_lport;
			else if (firstport == head->inp_lport)
				ret(EADDRINUSE);
			lport = htons(head->inp_lport);
		} while (in_pcbmatch(head,
			    zeroin_addr, 0, inp->inp_laddr, lport));
	}
	inp->inp_lport = lport;
out:
	INHEAD_WRITE_UNLOCK(head);
	return (error);
}

/*
 * Connect from a socket to a specified address.
 * Both address and port must be specified in argument sin.
 * If don't have a local address for this socket yet,
 * then pick one.
 */
in_pcbconnect(inp, nam)
	register struct inpcb *inp;
	struct mbuf *nam;
{
	struct in_ifaddr *ia;
	struct sockaddr_in *ifaddr;
	register struct sockaddr_in *sin = mtod(nam, struct sockaddr_in *);
	struct protosw	*pswp;

	LOCK_ASSERT("in_pcbconnect", INPCB_ISLOCKED(inp));
	if (nam->m_len != sizeof (*sin))
		return (EINVAL);
	if (sin->sin_family != AF_INET)
		return (EAFNOSUPPORT);
	if (sin->sin_port == 0)
		return (EADDRNOTAVAIL);
	if (in_ifaddr) {
		/*
		 * If the destination address is INADDR_ANY,
		 * use the primary local address.
		 * If the supplied address is INADDR_BROADCAST,
		 * and the primary interface supports broadcast,
		 * choose the broadcast address for that interface.
		 */
#define	satosin(sa)	((struct sockaddr_in *)(sa))
		if (sin->sin_addr.s_addr == INADDR_ANY)
		    sin->sin_addr = IA_SIN(in_ifaddr)->sin_addr;
		else if (sin->sin_addr.s_addr == (u_int)INADDR_BROADCAST &&
		  (in_ifaddr->ia_ifp->if_flags & IFF_BROADCAST))
		    sin->sin_addr = satosin(&in_ifaddr->ia_broadaddr)->sin_addr;
	}
	if (inp->inp_laddr.s_addr == INADDR_ANY) {
		register struct route *ro;
		struct ifnet *ifp;
		ROUTE_LOCK_DECL()

		ia = (struct in_ifaddr *)0;
		/* 
		 * If route is known or can be allocated now,
		 * our src addr is taken from the i/f, else punt.
		 */
		ROUTE_WRITE_LOCK();
		ro = &inp->inp_route;
		if (ro->ro_rt &&
		    (satosin(&ro->ro_dst)->sin_addr.s_addr !=
			sin->sin_addr.s_addr || 
		    inp->inp_socket->so_options & SO_DONTROUTE)) {
			RTFREE(ro->ro_rt);
			ro->ro_rt = (struct rtentry *)0;
		}
		if ((inp->inp_socket->so_options & SO_DONTROUTE) == 0 && /*XXX*/
		    (ro->ro_rt == (struct rtentry *)0 ||
		    ro->ro_rt->rt_ifp == (struct ifnet *)0)) {
			/* No route yet, so try to acquire one */
			ro->ro_dst.sa_family = AF_INET;
			ro->ro_dst.sa_len = sizeof(struct sockaddr_in);
			((struct sockaddr_in *) &ro->ro_dst)->sin_addr =
				sin->sin_addr;
			rtalloc(ro);
		}
		ROUTE_WRITE_UNLOCK();
		/*
		 * If we found a route, use the address
		 * corresponding to the outgoing interface
		 * unless it is the loopback (in case a route
		 * to our address on another net goes to loopback).
		 */
		if (ro->ro_rt && (ifp = ro->ro_rt->rt_ifp) &&
		    (ifp->if_flags & IFF_LOOPBACK) == 0)
			for (ia = in_ifaddr; ia; ia = ia->ia_next)
				if (ia->ia_ifp == ifp)
					break;
		if (ia == 0) {
			u_short fport = sin->sin_port;

			sin->sin_port = 0;
			ia = (struct in_ifaddr *)
			    ifa_ifwithdstaddr((struct sockaddr *)sin);
			sin->sin_port = fport;
			if (ia == 0)
				ia = in_iaonnetof(in_netof(sin->sin_addr));
			if (ia == 0)
				ia = in_ifaddr;
			if (ia == 0)
				return (EADDRNOTAVAIL);
		}
		/*
		 * If the destination address is multicast and an outgoing
		 * interface has been set as a multicast option, use the
		 * address of that interface as our source address.
		 */
		if (IN_MULTICAST(ntohl(sin->sin_addr.s_addr)) &&
		    inp->inp_moptions != NULL) {
			struct ip_moptions *imo;

			imo = inp->inp_moptions;
			if (imo->imo_multicast_ifp != NULL) {
				ifp = imo->imo_multicast_ifp;
				for (ia = in_ifaddr; ia; ia = ia->ia_next)
					if (ia->ia_ifp == ifp)
						break;
				if (ia == 0)
					return (EADDRNOTAVAIL);
			}
		}
		ifaddr = (struct sockaddr_in *)&ia->ia_addr;
	}
	INHEAD_WRITE_LOCK(inp->inp_head);
	if (in_pcbmatch(inp->inp_head,
	    sin->sin_addr,
	    sin->sin_port,
	    inp->inp_laddr.s_addr ? inp->inp_laddr : ifaddr->sin_addr,
	    inp->inp_lport)) {
		INHEAD_WRITE_UNLOCK(inp->inp_head);
		return (EADDRINUSE);
	}
	inp->inp_faddr = sin->sin_addr;
	inp->inp_fport = sin->sin_port;
	if (inp->inp_laddr.s_addr == INADDR_ANY) {
		if (inp->inp_lport == 0)
			(void)in_pcbbind(inp, (struct mbuf *)0);
		inp->inp_laddr = ifaddr->sin_addr;
	}
	/* 
	 * Now check if we are setting us up for a nasty infinite loop 
	 * (local and foreign addr/port same).
	 *
	 */
	pswp = inp->inp_socket->so_proto;
	if ((pswp->pr_flags & PR_CONNREQUIRED) && 
	    (sin->sin_port == inp->inp_lport) &&
	    (inp->inp_laddr.s_addr == sin->sin_addr.s_addr)) {
		INHEAD_WRITE_UNLOCK(inp->inp_head);
		return(ECONNREFUSED);
	}
	INHEAD_WRITE_UNLOCK(inp->inp_head);
	return (0);
}

void
in_pcbdisconnect(inp)
	struct inpcb *inp;
{
	LOCK_ASSERT("in_pcbdisconnect", INPCB_ISLOCKED(inp));
	INHEAD_WRITE_LOCK(inp->inp_head);
	inp->inp_faddr.s_addr = INADDR_ANY;
	inp->inp_fport = 0;
	if (inp->inp_socket->so_state & SS_NOFDREF)
		in_pcbdetach(inp);
	INHEAD_WRITE_UNLOCK(inp->inp_head);
}

void
in_pcbdetach(inp)
	struct inpcb *inp;
{
	LOCK_ASSERT("in_pcbdetach", INPCB_ISLOCKED(inp));
	LOCK_ASSERT("in_pcbdetach so", SOCKET_ISLOCKED(inp->inp_socket));

	INHEAD_WRITE_LOCK(inp->inp_head);
	if (inp == inp->inp_head->inp_head)	/* Check cached pcb */
		inp->inp_head->inp_head = 0;
	remque(inp);
	inp->inp_next = inp->inp_prev = 0;
	INHEAD_WRITE_UNLOCK(inp->inp_head);
	INPCB_UNLOCK(inp);
	INPCBRC_UNREF(inp);		/* Does in_pcbfree if unref */
}

void
in_pcbfree(inp)
	struct inpcb *inp;
{
	int refcnt;

	LOCK_ASSERT("in_pcbfree so", SOCKET_ISLOCKED(inp->inp_socket));

	INHEAD_WRITE_LOCK(inp->inp_head);
	if (inp->inp_next)
		panic("in_pcbfree");
	INPCBRC_LOCK(inp);
	refcnt = inp->inp_refcnt;
	INPCBRC_UNLOCK(inp);
	INHEAD_WRITE_UNLOCK(inp->inp_head);
	if (refcnt <= 1) {			/* Check for race */
		inp->inp_socket->so_pcb = 0;
		sofree(inp->inp_socket);
		if (inp->inp_options)
			(void)m_free(inp->inp_options);
		if (inp->inp_route.ro_rt)
			rtfree(inp->inp_route.ro_rt);
		ip_freemoptions(inp->inp_moptions);
		NET_FREE(inp, M_PCB);
	}
}

void
in_setsockaddr(inp, nam)
	register struct inpcb *inp;
	struct mbuf *nam;
{
	register struct sockaddr_in *sin;
	
	LOCK_ASSERT("in_setsockaddr", INPCB_ISLOCKED(inp));
	nam->m_len = sizeof (*sin);
	sin = mtod(nam, struct sockaddr_in *);
	bzero((caddr_t)sin, sizeof (*sin));
	sin->sin_family = AF_INET;
	sin->sin_len = sizeof(*sin);
	sin->sin_port = inp->inp_lport;
	sin->sin_addr = inp->inp_laddr;
}

void
in_setpeeraddr(inp, nam)
	struct inpcb *inp;
	struct mbuf *nam;
{
	register struct sockaddr_in *sin;
	
	LOCK_ASSERT("in_setpeeraddr", INPCB_ISLOCKED(inp));
	nam->m_len = sizeof (*sin);
	sin = mtod(nam, struct sockaddr_in *);
	bzero((caddr_t)sin, sizeof (*sin));
	sin->sin_family = AF_INET;
	sin->sin_len = sizeof(*sin);
	sin->sin_port = inp->inp_fport;
	sin->sin_addr = inp->inp_faddr;
}

/*
 * Pass some notification to all connections of a protocol
 * associated with address dst.  The local address and/or port numbers
 * may be specified to limit the search.  The "usual action" will be
 * taken, depending on the ctlinput cmd.  The caller must filter any
 * cmds that are uninteresting (e.g., no error in the map).
 * Call the protocol specific routine (if any) to report
 * any errors for each matching socket.
 */
void
#ifndef _NO_PROTO
in_pcbnotify(
	struct inpcb *head,
	struct sockaddr *dst,
	u_short fport,
	struct in_addr laddr,
	u_short lport,
	int cmd,
	void (*notify)())
#else
in_pcbnotify(head, dst, fport, laddr, lport, cmd, notify)
	struct inpcb *head;
	struct sockaddr *dst;
	u_short fport, lport;
	struct in_addr laddr;
	int cmd;
	void (*notify)();
#endif
{
	register struct inpcb *inp, *inpnxt;
	register struct socket *so;
	struct in_addr faddr;
	int errno;

	if ((unsigned)cmd > PRC_NCMDS || dst->sa_family != AF_INET)
		return;
	faddr = ((struct sockaddr_in *)dst)->sin_addr;
	if (faddr.s_addr == INADDR_ANY)
		return;

	/*
	 * Redirects go to all references to the destination,
	 * and use in_rtchange to invalidate the route cache.
	 * Dead host indications: notify all references to the destination.
	 * Otherwise, if we have knowledge of the local port and address,
	 * deliver only to that socket.
	 */
	if (PRC_IS_REDIRECT(cmd) || cmd == PRC_HOSTDEAD) {
		fport = 0;
		lport = 0;
		laddr.s_addr = 0;
		if (cmd != PRC_HOSTDEAD)
			notify = in_rtchange;
	}
	if (notify == 0)
		return;
	errno = inetctlerrmap[cmd];

resync:	/* XXX */
	INHEAD_READ_LOCK(head);
	if (inp = head->inp_next) {
		INPCBRC_REF(inp);
		for(; inp != head; inp = inpnxt) {
			inpnxt = inp->inp_next;
			INPCBRC_REF(inpnxt);
			if (inp->inp_faddr.s_addr != faddr.s_addr ||
			    inp->inp_socket == 0 ||
			    (lport && inp->inp_lport != lport) ||
			    (laddr.s_addr && inp->inp_laddr.s_addr != laddr.s_addr) ||
			    (fport && inp->inp_fport != fport)) {
				INPCBRC_UNREF(inp);
				continue;
			}
			INHEAD_READ_UNLOCK(head);
			so = inp->inp_socket;
			SOCKET_LOCK(so);
			INPCB_LOCK(inp);
			(*notify)(inp, errno);	/* <- */
			INPCB_UNLOCK(inp);
			INPCBRC_UNREF(inp);
			SOCKET_UNLOCK(so);
			INHEAD_READ_LOCK(head);
			/* We only resync if we lose our next inpcb */
			if (inpnxt->inp_next == 0) {
				INHEAD_READ_UNLOCK(head);
				so = inpnxt->inp_socket;
				SOCKET_LOCK(so);
				INPCBRC_UNREF(inpnxt);
				SOCKET_UNLOCK(so);
				goto resync;
			}
		}
		INPCBRC_UNREF(inp);	/* inp == head here */
	}
	INHEAD_READ_UNLOCK(head);
}

/*
 * Check for alternatives when higher level complains
 * about service problems.  For now, invalidate cached
 * routing information.  If the route was created dynamically
 * (by a redirect), time to try a default gateway again.
 */
void
in_losing(inp)
	struct inpcb *inp;
{
	register struct rtentry *rt;

	LOCK_ASSERT("in_losing", INPCB_ISLOCKED(inp));
	if ((rt = inp->inp_route.ro_rt)) {
		rt_missmsg(RTM_LOSING, &inp->inp_route.ro_dst,
			    rt->rt_gateway, (struct sockaddr *)rt_mask(rt),
			    (struct sockaddr *)0, rt->rt_flags, 0);
		if (rt->rt_flags & RTF_DYNAMIC)
			(void) rtrequest(RTM_DELETE, rt_key(rt),
				rt->rt_gateway, rt_mask(rt), rt->rt_flags, 
				(struct rtentry **)0);
		inp->inp_route.ro_rt = 0;
		rtfree(rt);
		/*
		 * A new route can be allocated
		 * the next time output is attempted.
		 */
	}
}

#if	NETSYNC_LOCK
void
in_losing_lock(inp)
	struct inpcb *inp;
{
	INPCB_LOCK(inp);
	in_losing(inp);
	INPCB_UNLOCK(inp);
}
#endif

/*
 * After a routing change, flush old routing
 * and allocate a (hopefully) better one.
 */
void
in_rtchange(inp)
	register struct inpcb *inp;
{
	LOCK_ASSERT("in_rtchange", INPCB_ISLOCKED(inp));
	if (inp->inp_route.ro_rt) {
		rtfree(inp->inp_route.ro_rt);
		inp->inp_route.ro_rt = 0;
		/*
		 * A new route can be allocated the next time
		 * output is attempted.
		 */
	}
}

struct inpcb *
#ifndef _NO_PROTO
in_pcblookup(
	struct inpcb *head,
	struct in_addr faddr,
	u_short fport,
	struct in_addr laddr,
	u_short lport,
	int flags)
#else
in_pcblookup(head, faddr, fport, laddr, lport, flags)
	struct inpcb *head;
	struct in_addr faddr, laddr;
	u_short fport, lport;
	int flags;
#endif
{
	register struct inpcb *inp, *match = 0;
	int matchwild = 3, wildcard;

	INHEAD_READ_LOCK(head);
	if ((flags & INPLOOKUP_USECACHE) && (inp = head->inp_head) &&
	    inp->inp_lport == lport && inp->inp_fport == fport &&
	    inp->inp_faddr.s_addr == faddr.s_addr &&
	    inp->inp_laddr.s_addr == laddr.s_addr) {
		match = inp;			/* Cache hit */
		in_pcbcachehit++;
	}
	else {
	    in_pcbcachemiss++;
	    for (inp = head->inp_next; inp != head; inp = inp->inp_next) {
		if (inp->inp_lport != lport)
			continue;
		wildcard = 0;
		if (inp->inp_laddr.s_addr != INADDR_ANY) {
			if (laddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->inp_laddr.s_addr != laddr.s_addr)
				continue;
		} else {
			if (laddr.s_addr != INADDR_ANY)
				wildcard++;
		}
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			if (faddr.s_addr == INADDR_ANY)
				wildcard++;
			else if (inp->inp_faddr.s_addr != faddr.s_addr ||
			    inp->inp_fport != fport)
				continue;
		} else {
			if (faddr.s_addr != INADDR_ANY)
				wildcard++;
		}
		if (wildcard && (flags & INPLOOKUP_WILDCARD) == 0)
			continue;
		if (wildcard < matchwild) {
			match = inp;
			matchwild = wildcard;
			if (matchwild == 0)
				break;
		}
	    }
	}
	if (match) {
		INPCBRC_REF(match);
		if (flags & INPLOOKUP_USECACHE)
			head->inp_head = match;	/* save for next cache */
	}
	INHEAD_READ_UNLOCK(head);
	return (match);
}

/* This is just in_pcblookup with no wildcard flags. It's a bit faster. */
/* If NETSYNC_LOCK, it does not take locks or return a referenced INPCB. */
struct inpcb *
#ifndef _NO_PROTO
in_pcbmatch(
	struct inpcb *head,
	struct in_addr faddr,
	u_short fport,
	struct in_addr laddr,
	u_short lport)
#else
in_pcbmatch(head, faddr, fport, laddr, lport)
	struct inpcb *head;
	struct in_addr faddr, laddr;
	u_short fport, lport;
#endif
{
	register struct inpcb *inp;

	for (inp = head->inp_next; inp != head; inp = inp->inp_next) {
		if (inp->inp_lport != lport)
			continue;
		if (inp->inp_laddr.s_addr != INADDR_ANY) {
			if (laddr.s_addr == INADDR_ANY)
				continue;
			else if (inp->inp_laddr.s_addr != laddr.s_addr)
				continue;
		} else {
			if (laddr.s_addr != INADDR_ANY)
				continue;
		}
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			if (faddr.s_addr == INADDR_ANY)
				continue;
			else if (inp->inp_faddr.s_addr != faddr.s_addr ||
			    inp->inp_fport != fport)
				continue;
		} else {
			if (faddr.s_addr != INADDR_ANY)
				continue;
		}
		return inp;
	}
	return 0;
}
