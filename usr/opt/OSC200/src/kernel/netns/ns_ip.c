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
static char	*sccsid = "@(#)$RCSfile: ns_ip.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/01/13 12:31:18 $";
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
/*
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
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
 *	Base:	ns_ip.c	7.6 (Berkeley) 6/28/90
 */

/*
 * Software interface driver for encapsulating ns in ip.
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/systm.h"
#include "sys/errno.h"
#include "sys/ioctl.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/route.h"
#include "net/netisr.h"
#include "net/if_types.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"

#include "netns/ns.h"
#include "netns/ns_if.h"
#include "netns/idp.h"

#include "net/net_malloc.h"

#ifdef NSIP

struct ifnet_en {
	struct ifnet ifen_ifnet;
	struct route ifen_route;
	struct in_addr ifen_src;
	struct in_addr ifen_dst;
	struct ifnet_en *ifen_next;
};

#define LOMTU	(1024+512);

struct ifnet nsipif;
struct ifnet_en *nsip_list;		/* list of all hosts and gateways or
					broadcast addrs */

struct ifnet_en *
nsipattach()
{
	register struct ifnet_en *m;
	register struct ifnet *ifp;

	if (nsipif.if_mtu == 0) {
		ifp = &nsipif;
		ifp->if_name = "nsip";
		ifp->if_mtu = LOMTU;
		ifp->if_ioctl = nsipioctl;
		ifp->if_output = nsipoutput;
		ifp->if_start = nsipstart;
		ifp->if_flags = IFF_POINTOPOINT;
#if	1
		ifp->if_type = IFT_NSIP;
		ifp->if_hdrlen = 0;
		ifp->if_addrlen = 0;
#endif
	}

	NET_MALLOC((m), struct ifnet_en *, sizeof(*m), M_PCB, M_NOWAIT);
	if (m == NULL) return (NULL);
	m->ifen_next = nsip_list;
	nsip_list = m;
	ifp = &m->ifen_ifnet;

	ifp->if_name = "nsip";
	ifp->if_mtu = LOMTU;
	ifp->if_ioctl = nsipioctl;
	ifp->if_output = nsipoutput;
	ifp->if_start = nsipstart;
	ifp->if_flags = IFF_POINTOPOINT;
	ifp->if_unit = nsipif.if_unit++;
#if	1
	ifp->if_type = IFT_NSIP;
	ifp->if_hdrlen = 0;
	ifp->if_addrlen = 0;
#endif
	if_attach(ifp);

	return (m);
}


/*
 * Process an ioctl request.
 */
/* ARGSUSED */
nsipioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	unsigned int cmd;
	caddr_t data;
{
	int error = 0;
	struct ifreq *ifr;

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		/* fall into: */

	case SIOCSIFDSTADDR:
		/*
		 * Everything else is done at a higher level.
		 */
		break;

	case SIOCSIFFLAGS:
		ifr = (struct ifreq *)data;
		if ((ifr->ifr_flags & IFF_UP) == 0)
			error = nsip_free(ifp);


	default:
		error = EINVAL;
	}
	return (error);
}

struct mbuf *nsip_badlen;
struct mbuf *nsip_lastin;
int nsip_hold_input;

#if	0
idpip_input(m, ifp)
	register struct mbuf *m;
	struct ifnet *ifp;
{
#else
idpip_input(m, iphlen)
	register struct mbuf *m;
	int iphlen;
{
	struct ifnet *ifp;
#endif
	register struct ip *ip;
	register struct idp *idp;
#if	0
	register struct ifqueue *ifq = &nsintrq;
#endif
	int len, s;

	if (nsip_hold_input) {
		if (nsip_lastin) {
			m_freem(nsip_lastin);
		}
		nsip_lastin = m_copym(m, 0, (int)M_COPYALL, M_DONTWAIT);
	}
	/*
	 * Get IP and IDP header together in first mbuf.
	 */
	nsipif.if_ipackets++;
	s = sizeof (struct ip) + sizeof (struct idp);
	if (((m->m_flags & M_EXT) || m->m_len < s) &&
	    (m = m_pullup(m, s)) == 0) {
		nsipif.if_ierrors++;
		return;
	}
	ip = mtod(m, struct ip *);
#if	1
	if (iphlen > sizeof (struct ip)) {
#else
	if (ip->ip_hl > (sizeof (struct ip) >> 2)) {
#endif
		ip_stripoptions(ip, (struct mbuf *)0, (struct ipoption *)0);
		if (m->m_len < s) {
			if ((m = m_pullup(m, s)) == 0) {
				nsipif.if_ierrors++;
				return;
			}
			ip = mtod(m, struct ip *);
		}
	}

	/*
	 * Make mbuf data length reflect IDP length.
	 * If not enough data to reflect IDP length, drop.
	 */
	m->m_data += sizeof (struct ip);
	m->m_len -= sizeof (struct ip);
	m->m_pkthdr.len -= sizeof (struct ip);
	idp = mtod(m, struct idp *);
	len = ntohs(idp->idp_len);
	if (len & 1) len++;		/* Preserve Garbage Byte */
	if (ip->ip_len != len) {
		if (len > ip->ip_len) {
			nsipif.if_ierrors++;
			if (nsip_badlen) m_freem(nsip_badlen);
			nsip_badlen = m;
			return;
		}
		/* Any extra will be trimmed off by the NS routines */
	}

#if	1
	/*
	 * Deliver to NS
	 */
	netisr_input(NETISR_NS, m, (caddr_t)0, 0);
#else
	/*
	 * Place interface pointer before the data
	 * for the receiving protocol.
	 */
	m->m_pkthdr.rcvif = ifp;
	/*
	 * Deliver to NS
	 */
	s = splimp();
	if (IF_QFULL(ifq)) {
		IF_DROP(ifq);
bad:
		m_freem(m);
		splx(s);
		return;
	}
	IF_ENQUEUE(ifq, m);
	schednetisr(NETISR_NS);
	splx(s);
#endif
	return;
}

/* ARGSUSED */
nsipoutput(ifn, m, dst)
	struct ifnet_en *ifn;
	register struct mbuf *m;
	struct sockaddr *dst;
{

	register struct ip *ip;
	register struct route *ro = &(ifn->ifen_route);
	register int len = 0;
	register struct idp *idp = mtod(m, struct idp *);
	int error;

	ifn->ifen_ifnet.if_opackets++;
	nsipif.if_opackets++;


	/*
	 * Calculate data length and make space
	 * for IP header.
	 */
	len =  ntohs(idp->idp_len);
	if (len & 1) len++;		/* Preserve Garbage Byte */
	/* following clause not necessary on vax */
	if (3 & (int)m->m_data) {
		/* force longword alignment of ip hdr */
		struct mbuf *m0 = m_gethdr(MT_HEADER, M_DONTWAIT);
		if (m0 == 0) {
			m_freem(m);
			return (ENOBUFS);
		}
		MH_ALIGN(m0, sizeof (struct ip));
		m0->m_flags = m->m_flags & M_COPYFLAGS;
		m0->m_next = m;
		m0->m_len = sizeof (struct ip);
		m0->m_pkthdr.len = m0->m_len + m->m_len;
		m->m_flags &= ~M_PKTHDR;
	} else {
		M_PREPEND(m, sizeof (struct ip), M_DONTWAIT);
		if (m == 0)
			return (ENOBUFS);
	}
	/*
	 * Fill in IP header.
	 */
	ip = mtod(m, struct ip *);
	*(long *)ip = 0;
	ip->ip_p = IPPROTO_IDP;
	ip->ip_src = ifn->ifen_src;
	ip->ip_dst = ifn->ifen_dst;
	ip->ip_len = (u_short)len + sizeof (struct ip);
	ip->ip_ttl = MAXTTL;

	/*
	 * Output final datagram.
	 */
	error =  (ip_output(m, (struct mbuf *)0, ro, SO_BROADCAST));
	if (error) {
		ifn->ifen_ifnet.if_oerrors++;
		ifn->ifen_ifnet.if_ierrors = error;
	}
	return (error);
bad:
	m_freem(m);
	return (ENETUNREACH);
}

nsipstart(ifp)
struct ifnet *ifp;
{
	panic("nsip_start called\n");
}

struct ifreq ifr = {"nsip0"};

nsip_route(m)
	register struct mbuf *m;
{
	register struct nsip_req *rq = mtod(m, struct nsip_req *);
	struct sockaddr_ns *ns_dst = (struct sockaddr_ns *)&rq->rq_ns;
	struct sockaddr_in *ip_dst = (struct sockaddr_in *)&rq->rq_ip;
	struct route ro;
	struct ifnet_en *ifn;
	struct sockaddr_in *src;

	/*
	 * First, make sure we already have an ns address:
	 */
	if (ns_hosteqnh(ns_thishost, ns_zerohost))
		return (EADDRNOTAVAIL);
	/*
	 * Now, determine if we can get to the destination
	 */
	bzero((caddr_t)&ro, sizeof (ro));
	ro.ro_dst = *(struct sockaddr *)ip_dst;
	rtalloc(&ro);
	if (ro.ro_rt == 0 || ro.ro_rt->rt_ifp == 0) {
		return (ENETUNREACH);
	}

	/*
	 * And see how he's going to get back to us:
	 * i.e., what return ip address do we use?
	 */
	{
		register struct in_ifaddr *ia;
		struct ifnet *ifp = ro.ro_rt->rt_ifp;

		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if (ia->ia_ifp == ifp)
				break;
		if (ia == 0)
			ia = in_ifaddr;
		if (ia == 0) {
			RTFREE(ro.ro_rt);
			return (EADDRNOTAVAIL);
		}
		src = (struct sockaddr_in *)&ia->ia_addr;
	}

	/*
	 * Is there a free (pseudo-)interface or space?
	 */
	for (ifn = nsip_list; ifn; ifn = ifn->ifen_next) {
		if ((ifn->ifen_ifnet.if_flags & IFF_UP) == 0)
			break;
	}
	if (ifn == NULL)
		ifn = nsipattach();
	if (ifn == NULL) {
		RTFREE(ro.ro_rt);
		return (ENOBUFS);
	}
	ifn->ifen_route = ro;
	ifn->ifen_dst =  ip_dst->sin_addr;
	ifn->ifen_src = src->sin_addr;

	/*
	 * now configure this as a point to point link
	 */
	ifr.ifr_name[4] = '0' + nsipif.if_unit - 1;
	ifr.ifr_dstaddr = * (struct sockaddr *) ns_dst;
	(void)ns_control((struct socket *)0, (int)SIOCSIFDSTADDR, (caddr_t)&ifr,
			(struct ifnet *)ifn);
	satons_addr(ifr.ifr_addr).x_host = ns_thishost;
	return (ns_control((struct socket *)0, (int)SIOCSIFADDR, (caddr_t)&ifr,
			(struct ifnet *)ifn));
}

nsip_free(ifp)
struct ifnet *ifp;
{
	register struct ifnet_en *ifn = (struct ifnet_en *)ifp;
	struct route *ro = & ifn->ifen_route;

	if (ro->ro_rt) {
		RTFREE(ro->ro_rt);
		ro->ro_rt = 0;
	}
	ifp->if_flags &= ~IFF_UP;
	return (0);
}

nsip_ctlinput(cmd, sa)
	int cmd;
	struct sockaddr *sa;
{
#if	1
	struct sockaddr_in *sin;
#else
	extern u_char inetctlerrmap[];
	struct sockaddr_in *sin;
	int in_rtchange();
#endif

	if ((unsigned)cmd >= PRC_NCMDS)
		return;
	if (sa->sa_family != AF_INET && sa->sa_family != AF_IMPLINK)
		return;
	sin = (struct sockaddr_in *)sa;
	if (sin->sin_addr.s_addr == INADDR_ANY)
		return;

	switch (cmd) {

	case PRC_ROUTEDEAD:
	case PRC_REDIRECT_NET:
	case PRC_REDIRECT_HOST:
	case PRC_REDIRECT_TOSNET:
	case PRC_REDIRECT_TOSHOST:
		nsip_rtchange(&sin->sin_addr);
		break;
	}
}

nsip_rtchange(dst)
	register struct in_addr *dst;
{
	register struct ifnet_en *ifn;

	for (ifn = nsip_list; ifn; ifn = ifn->ifen_next) {
		if (ifn->ifen_dst.s_addr == dst->s_addr &&
			ifn->ifen_route.ro_rt) {
				RTFREE(ifn->ifen_route.ro_rt);
				ifn->ifen_route.ro_rt = 0;
		}
	}
}
#endif
