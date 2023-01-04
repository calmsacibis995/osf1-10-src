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
static char	*sccsid = "@(#)$RCSfile: ip_input.c,v $ $Revision: 4.4.10.5 $ (DEC) $Date: 1993/11/22 23:33:25 $";
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
 * Copyright (c) 1982, 1986, 1988 Regents of the University of California.
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
 *	Base:	ip_input.c	7.14 (Berkeley) 9/20/89
 *	Merged:	ip_input.c	7.16 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 10-Oct-91	Heather Gray
 *	Put quota on reassembly queue
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patches.
 *
 */

#include "machine/endian.h"
#include "net/net_globals.h"
#if	MACH
#include <mach_net.h>
#endif

#include "sys/param.h"
#include "sys/time.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/route.h"
#include "net/netisr.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/in_pcb.h"
#include "netinet/in_var.h"
#include "netinet/ip_var.h"
#include "netinet/ip_icmp.h"
#include "netinet/icmp_var.h"
#include "netinet/udp.h"

#include "net/net_malloc.h"

LOCK_ASSERTL_DECL

int	ipgateway = IPGATEWAY;			/* Are configured as gateway */
int	ipforwarding = IPFORWARDING;		/* Are acting as gateway */
extern	int in_interfaces;			/* Have multiple interfaces */
int	ipsendredirects = IPSENDREDIRECTS;
int	ipdirected_broadcast = IPDIRECTED_BROADCAST;
int	ipsrcroute = 1;		/* Dangerous security hole, but popular option*/

int	ipfragttl = IPFRAGTTL;
int	ipsendreastimo = 0;		/* Send ICMP on reassembly timeout */

u_char	ip_protox[IPPROTO_MAX];
int	ipqmaxlen = IFQ_MAXLEN;
struct	in_ifaddr *in_ifaddr;			/* first inet address */
struct	ifqueue	ipintrq;			/* ip packet input queue */

struct	ipstat	ipstat;
struct	ipreass ipreass;
struct	ipq	ipq;				/* ip reass. queue */
u_short	ip_id;					/* ip packet ctr, for ids */

#if	NETSYNC_LOCK
lock_data_t		ip_frag_lock;
simple_lock_data_t	ip_misc_lock;
simple_lock_data_t	inifaddr_lock;
#endif

/*
 * We need to save the IP options in case a protocol wants to respond
 * to an incoming packet over the same route if the packet got here
 * using IP source routing.  This allows connection establishment and
 * maintenance when the remote end is on a network that is not known
 * to us.
#if	NETISR_THREAD
 * Fix me fix me fix me. ip_stripoptions() is structured to do what
 * we need: extract the options if desired and so they can be passed
 * back down. The global save area is not workable.
#endif
 */
static int ip_nhops;
static struct ip_srcrt ip_srcrt;

/* Gateway */
extern	int if_index;
u_int	*ip_ifmatrix;

extern	int kmempages;

static void
setiproutemask(dst, mask)
	struct sockaddr_in *dst, *mask;
{
	in_sockmaskof(dst->sin_addr, mask);
}

/*
 * IP initialization: fill in IP protocol switch table.
 * All protocols not implemented in kernel go to raw IP protocol handler.
 */
void
ip_init()
{
	register struct protosw *pr;
	register int i;

	pr = pffindproto(PF_INET, IPPROTO_RAW, SOCK_RAW);
	if (pr == 0)
		panic("ip_init");
	for (i = 0; i < IPPROTO_MAX; i++)
		ip_protox[i] = pr - inetsw;
	for (pr = inetdomain.dom_protosw;
	    pr < inetdomain.dom_protoswNPROTOSW; pr++)
		if (pr->pr_domain->dom_family == PF_INET &&
		    pr->pr_protocol && pr->pr_protocol != IPPROTO_RAW)
			ip_protox[pr->pr_protocol] = pr - inetsw;
	ipq.next = ipq.prev = &ipq;
	ip_id = ntohl(iptime());
	in_ifaddr = NULL;
	in_interfaces = 0;
	INIFADDR_LOCKINIT();
	IFQ_LOCKINIT(&ipintrq);
	ipintrq.ifq_maxlen = ipqmaxlen;
	IPFRAG_LOCKINIT();
	IPMISC_LOCKINIT();
	NETSTAT_LOCKINIT(&ipstat.ips_lock);
	NETSTAT_LOCKINIT(&icmpstat.icps_lock);

	if (ipreass.ipr_percent == 0)
	    ipreass.ipr_percent = IPR_DEFAULT;
	ipreass.ipr_max = (kmempages * PAGE_SIZE * ipreass.ipr_percent)/100;
	if (ipreass.ipr_max < (128 * PAGE_SIZE))
		ipreass.ipr_max = (128 * PAGE_SIZE);

	IPREASS_LOCKINIT();
	
/* Problems w/dynamic and ||ization! */
	/* Init ip_ifmatrix in case we become a gateway */
	i = (if_index + 1) * (if_index + 1) * sizeof (u_int);
	NET_MALLOC(ip_ifmatrix, u_int *, i, M_RTABLE, M_WAITOK);
	if (ip_ifmatrix == 0)
		panic("no memory for ip_ifmatrix");
#if	MACH_NET
	mach_net_ipinit();
#endif

	rtinithead(AF_INET, 32, setiproutemask);
	(void) netisr_add(NETISR_IP, ipintr, &ipintrq, &inetdomain);

	/* For now init ARP here... XXX */
	arpinit();
}

#if	!NETISR_THREAD
struct	sockaddr_in ipaddr = { sizeof(ipaddr), AF_INET };
#else
/* Following struct protected by route lock, sort of. */
#endif
struct	route ipforward_rt;
/*
 * Ip input routine.  Checksum and byte swap header.  If fragmented
 * try to reassemble.  Process options.  Pass to next level.
 */
void
ipintr()
{
	register struct ip *ip;
	register struct mbuf *m;
	register struct ipq *fp;
	register struct in_ifaddr *ia;
	register u_int sum;
	int hlen, s;

next:
	/*
	 * Get next datagram off input queue and get IP header
	 * in first mbuf.
	 */
	s = splimp();
	IF_DEQUEUE(&ipintrq, m);
	splx(s);
	if (m == 0)
		return;
if ((m->m_flags & M_PKTHDR) == 0)
panic("ipintr no HDR");
	/*
	 * If no IP addresses have been set yet but the interfaces
	 * are receiving, can't do anything with incoming packets yet.
	 */
	if (in_ifaddr == NULL)
		goto bad;
	NETSTAT_LOCK(&ipstat.ips_lock);
	ipstat.ips_total++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);
	if ((m->m_len < sizeof (struct ip)) &&
	    (m = m_pullup(m, sizeof (struct ip))) == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_toosmall++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto next;
	}
	ip = mtod(m, struct ip *);
	hlen = (ip->ip_vhl & 0x0f) << 2;
	if (hlen < sizeof(struct ip) ||	/* minimum header length */
	    (ip->ip_vhl & 0xf0) != IPVERSION << 4) {	/* wrong stat! */
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_badhlen++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto bad;
	}
	if (hlen > m->m_len) {
		if ((m = m_pullup(m, hlen)) == 0) {
			NETSTAT_LOCK(&ipstat.ips_lock);
			ipstat.ips_badhlen++;
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			goto next;
		}
		ip = mtod(m, struct ip *);
	}

#if	0
	if (ip->ip_sum = in_cksum(m, hlen)) {
#else
	/* Inline in_cksum over IP header. */
#define	ADDSUM(n)	case n:	sum +=	*(u_short *)((caddr_t)ip + n - 2) + \
					*(u_short *)((caddr_t)ip + n - 4)
	sum = 0;
	switch (hlen) {
		ADDSUM(60); ADDSUM(56); ADDSUM(52); ADDSUM(48); ADDSUM(44);
		ADDSUM(40); ADDSUM(36); ADDSUM(32); ADDSUM(28); ADDSUM(24);
		ADDSUM(20); ADDSUM(16); ADDSUM(12); ADDSUM( 8); ADDSUM( 4);
	}
	sum = (sum & 0xffff) + (sum >> 16);
	sum += sum >> 16;
	if (~sum & 0xffff) {
#endif
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_badsum++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto bad;
	}

	/*
	 * Convert fields to host representation.
	 */
	ip->ip_len = ntohs(ip->ip_len);
	if (ip->ip_len < hlen) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_badlen++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto bad;
	}
	ip->ip_id = ntohs(ip->ip_id);
	ip->ip_off = ntohs(ip->ip_off);

	/*
	 * Check that the amount of data in the buffers
	 * is at least as much as the IP header would have us expect.
	 * Trim mbufs if longer than we expect.
	 * Drop packet if shorter than we expect.
	 */
	if (m->m_pkthdr.len < (int)ip->ip_len) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_tooshort++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		goto bad;
	}
	if (m->m_pkthdr.len > (int)ip->ip_len) {
		if (m->m_len == m->m_pkthdr.len) {
			m->m_len = ip->ip_len;
			m->m_pkthdr.len = ip->ip_len;
		} else
			m_adj(m, (int)ip->ip_len - m->m_pkthdr.len);
	}

	/*
	 * Process options and, if not destined for us,
	 * ship it on.  ip_dooptions returns 1 when an
	 * error was detected (causing an icmp message
	 * to be sent and the original packet to be freed).
	 */
	ip_nhops = 0;		/* for source routed packets */
	if (hlen > sizeof (struct ip) && ip_dooptions(m))
		goto next;

	/*
	 * Check our list of addresses, to see if the packet is for us.
	 */
	m->m_flags |= M_WCARD;	/* Accepting a broadcast? */
	for (ia = in_ifaddr; ia; ia = ia->ia_next) {
#define	satosin(sa)	((struct sockaddr_in *)(sa))

		if (IA_SIN(ia)->sin_addr.s_addr == ip->ip_dst.s_addr) {
			m->m_flags &= ~M_WCARD;	/* A direct address */
			goto ours;
		}
		if ((!ipdirected_broadcast || ia->ia_ifp==m->m_pkthdr.rcvif) &&
		    (ia->ia_ifp->if_flags & IFF_BROADCAST)) {
			u_int t;

			if (satosin(&ia->ia_broadaddr)->sin_addr.s_addr ==
			    ip->ip_dst.s_addr)
				goto ours;
			if (ip->ip_dst.s_addr == ia->ia_netbroadcast.s_addr)
				goto ours;
			/*
			 * Look for all-0's host part (old broadcast addr),
			 * either for subnet or net.
			 */
			t = ntohl(ip->ip_dst.s_addr);
			if (t == ia->ia_subnet)
				goto ours;
			if (t == ia->ia_net)
				goto ours;
		}
	}

	if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr))) {
		struct in_multi *inm;
		extern struct socket *ip_mrouter;

		m->m_flags &= ~M_WCARD;
		if (ip_mrouter) {
			/*
			 * If we are acting as a multicast router, all
			 * incoming multicast packets are passed to the
			 * kernel-level multicast forwarding function.
			 * The packet is returned (relatively) intact; if
			 * ip_mforward() returns a non-zero value, the packet
			 * must be discarded, else it may be accepted below.
			 *
			 * (The IP ident field is put in the same byte order
			 * as expected when ip_mforward() is called from
			 * ip_output().)
			 */
			ip->ip_id = htons(ip->ip_id);
			if (ip_mforward(m, m->m_pkthdr.rcvif) != 0) {
				m_freem(m);
				goto next;
			}
			ip->ip_id = ntohs(ip->ip_id);

			/*
			 * The process-level routing demon needs to receive
			 * all multicast IGMP packets, whether or not this
			 * host belongs to their destination groups.
			 */
			if (ip->ip_p == IPPROTO_IGMP)
				goto ours;
		}

		/*
		 * See if we belong to the destination multicast group on the
		 * arrival interface.
		 */
		IN_LOOKUP_MULTI(ip->ip_dst, m->m_pkthdr.rcvif, inm);
		if (inm == NULL) {
			m_freem(m);
			goto next;
		}
		goto ours;
	}

	if (ip->ip_dst.s_addr == (u_int)INADDR_BROADCAST)
		goto ours;
	if (ip->ip_dst.s_addr == INADDR_ANY)
		goto ours;

	/*
	 * Not for us; forward if possible and desirable.
	 */
	if (ipforwarding == 0 || ipgateway == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_cantforward++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		m_freem(m);
	} else {
		m->m_flags &= ~M_WCARD;
		ip_forwardscreen(m, 0);
	}
	goto next;

ours:
	/*
	 * If offset or IP_MF are set, must reassemble.
	 * Otherwise, nothing need be done.
	 * (We could look in the reassembly queue to see
	 * if the packet was previously fragmented,
	 * but it's not worth the time; just let them time out.)
	 */
	if (ip->ip_off &~ IP_DF) {
		/*
		 * Look for queue of fragments
		 * of this datagram.
		 */
		IPFRAG_LOCK();
		for (fp = ipq.next; fp != &ipq; fp = fp->next)
			if (ip->ip_id == fp->ipq_id &&
			    ip->ip_src.s_addr == fp->ipq_src.s_addr &&
			    ip->ip_dst.s_addr == fp->ipq_dst.s_addr &&
			    ip->ip_p == fp->ipq_p)
				goto found;
		fp = 0;
found:
		/*
		 * Adjust ip_len to not reflect header,
		 * use bit in version field (need to save TOS) to 
		 * indicate if more fragments are expected,
		 * convert offset of this to bytes.
		 */
		ip->ip_len -= hlen;
		ip->ip_vhl &= 0x0f;		/* clear version */
		if (ip->ip_off & IP_MF)
			ip->ip_vhl |= IPFRAG_MFF;
		ip->ip_off <<= 3;

		/*
		 * If datagram marked as having more fragments
		 * or if this is not the first fragment,
		 * attempt reassembly; if it succeeds, proceed.
		 */
		if ((ip->ip_vhl & IPFRAG_MFF) || ip->ip_off)
		{
			NETSTAT_LOCK(&ipstat.ips_lock);
			ipstat.ips_fragments++;
			NETSTAT_UNLOCK(&ipstat.ips_lock);

			m = ip_reass(m, fp);
			IPFRAG_UNLOCK();
			if (m == 0)
				goto next;
			else {
				NETSTAT_LOCK(&ipstat.ips_lock);
				ipstat.ips_reassembled++;
				NETSTAT_UNLOCK(&ipstat.ips_lock);
			}
			ip = mtod(m, struct ip *);
		} else {
			if (fp)
				ip_freef(fp);
			IPFRAG_UNLOCK();
		}
	} else
		ip->ip_len -= hlen;

	/*
	 * Switch out to protocol's input routine.
	 */
	NETSTAT_LOCK(&ipstat.ips_lock);
	ipstat.ips_delivered++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);
#if	MACH_NET
	m = mach_net_ipreceive(m, hlen);
	if (m == NULL)
		goto next;
#endif
	(*inetsw[ip_protox[ip->ip_p]].pr_input)(m, hlen);
	goto next;
bad:
	m_freem(m);
	goto next;
}

/*
 * Take incoming datagram fragment and try to
 * reassemble it into whole datagram.  If a chain for
 * reassembly of this datagram already exists, then it
 * is given as fp; otherwise have to make a chain.
 */
struct mbuf *
ip_reass(m, fp)
	register struct mbuf *m;
	register struct ipq *fp;
{
	register struct ip *real_ip = mtod(m, struct ip *);
	register struct ipasfrag *ip, *q;
	register struct mbuf *t;
	int hlen, i, next, length = 0, overquota;

	LOCK_ASSERT("ip_reass", lock_islocked(&ip_frag_lock));

	hlen = (real_ip->ip_vhl & 0x0f) << 2;

	/* Need to perform dtom() on fragq. Real fix later? */
	if (m->m_flags & M_EXT) {
		if (real_ip->ip_p == IPPROTO_UDP) {
			if (real_ip->ip_off == 0)
				m = m_pullup_exact(m, 
				    hlen + sizeof (struct udphdr));
			else
				m = m_pullup_exact(m, hlen);
		}
		else
			m = m_pullup(m, sizeof (struct ip));
		if (m == NULL)
			goto dropfrag1;
		real_ip = mtod(m, struct ip *);
	}

#ifdef __alpha
	ip = (struct ipasfrag *)m->m_ipq;
	ip->ipf_ip = real_ip;
#else /* __alpha */
	ip = mtod(m, struct ipasfrag *);
#endif /* __alpha */

	/*
	 * Presence of header sizes in mbufs
	 * would confuse code below.
	 */
	m->m_data += hlen;
	m->m_len -= hlen;

	/*
	 * Compute amount of buffer space used by this datagram fragment and
	 * check if there is sufficient quota space available.
	 */
	t = m;
	while (t) {
	    length += MSIZE;
	    if (t->m_flags & M_EXT)
		length += t->m_ext.ext_size;
	    t = t->m_next;
	}

	/*
	 * If first fragment to arrive, create a reassembly queue.
	 */
	if (fp == 0) {
	        /*
		 * If this fragment causes the reassembly quota to become
		 * exhausted, don't try to build a new reassembly queue.
		 */
		IPREASS_LOCK();
		if ((ipreass.ipr_inuse + length) > ipreass.ipr_max) {
			ipreass.ipr_drops++;
			IPREASS_UNLOCK();
			goto dropfrag;
		}
		IPREASS_UNLOCK();

		if ((t = m_get(M_DONTWAIT, MT_FTABLE)) == NULL)
			goto dropfrag;
		fp = mtod(t, struct ipq *);
		fp->ipq_ttl = ipfragttl;
		fp->ipq_p = real_ip->ip_p;
		fp->ipq_id = real_ip->ip_id;
		fp->ipq_next = fp->ipq_prev = (struct ipasfrag *)fp;

                fp->ipq_src = real_ip->ip_src;
                fp->ipq_dst = real_ip->ip_dst;

		q = (struct ipasfrag *)fp;
		insque(fp, &ipq);
		goto insert;
	}

	/*
	 * Find a segment which begins after this one does.
	 */
	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = q->ipf_next)
#ifdef __alpha
		if (q->ipf_ip->ip_off > real_ip->ip_off)
			break;
#else
		if (q->ip_off > ip->ip_off)
			break;
#endif

	/*
	 * If there is a preceding segment, it may provide some of
	 * our data already.  If so, drop the data from the incoming
	 * segment.  If it provides all of our data, drop us.
	 */
	if (q->ipf_prev != (struct ipasfrag *)fp) {
#ifdef __alpha
		i = q->ipf_prev->ipf_ip->ip_off + q->ipf_prev->ipf_ip->ip_len - real_ip->ip_off;
#else
		i = q->ipf_prev->ip_off + q->ipf_prev->ip_len - ip->ip_off;
#endif
		if (i > 0) {
			if (i >= real_ip->ip_len)
				goto dropfrag;
			m_adj(m, i);
			real_ip->ip_off += i;
			real_ip->ip_len -= i;
		}
	}

	/*
	 * While we overlap succeeding segments trim them or,
	 * if they are completely covered, dequeue them.
	 */
#ifdef __alpha
	while ( q != (struct ipasfrag *)fp &&
		real_ip->ip_off + real_ip->ip_len > q->ipf_ip->ip_off) {
		i = (real_ip->ip_off + real_ip->ip_len) - q->ipf_ip->ip_off;
		if (i < q->ipf_ip->ip_len) {
			q->ipf_ip->ip_len -= i;
			q->ipf_ip->ip_off += i;
			m_adj(dtom(q), i);
			break;
		}
#else
	while (q != (struct ipasfrag *)fp && ip->ip_off + ip->ip_len > q->ip_off) {
		i = (ip->ip_off + ip->ip_len) - q->ip_off;
		if (i < q->ip_len) {
			q->ip_len -= i;
			q->ip_off += i;
			m_adj(dtom(q), i);
			break;
		}
#endif
		{
		struct ipasfrag *p = q;
		q = q->ipf_next;
		ip_deq(p);
		ip_dealloc(dtom(p));
		}
	}

insert:
	/*
	 * Stick new segment in its place;
	 * check for complete reassembly.
	 */
	ip_enq(ip, q->ipf_prev);
	IPREASS_LOCK();
	ipreass.ipr_inuse += length;
	IPREASS_UNLOCK();

	next = 0;
	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = q->ipf_next) {
#ifdef __alpha
		if (q->ipf_ip->ip_off != next)
		        goto checkquota;
		next += q->ipf_ip->ip_len;
#else
		if (q->ip_off != next)
		        goto checkquota;
		next += q->ip_len;
#endif
	}
#ifdef __alpha
	if (q->ipf_prev->ipf_ip->ip_vhl & IPFRAG_MFF) {
#else
	if (q->ipf_prev->ip_vhl & IPFRAG_MFF) {
#endif
checkquota:	IPREASS_LOCK();
		overquota = ipreass.ipr_inuse > ipreass.ipr_max;
		IPREASS_UNLOCK();
		if (overquota)
			ip_freef(fp);
		return (0);
	}

	/*
	 * Reassembly is complete; concatenate fragments.
	 */
	q = fp->ipq_next;
	length = 0;
	while (q != (struct ipasfrag *)fp) {
		t = dtom(q);
		while (t) {
			length += MSIZE;
			if (t->m_flags & M_EXT)
				length += t->m_ext.ext_size;
			t = t->m_next;
		}
		q = q->ipf_next;
	}
	q = fp->ipq_next;
	m = dtom(q);
	q = q->ipf_next;
	while (q != (struct ipasfrag *)fp) {
		t = dtom(q);
		q = q->ipf_next;
		m_cat(m, t);
	}

	/*
	 * Create header for new ip packet by
	 * modifying header of first packet;
	 * dequeue and discard fragment reassembly header.
	 * Make header visible.
	 */
	ip = fp->ipq_next;
#ifdef __alpha
	real_ip = ip->ipf_ip;
#else
	real_ip = (struct ip *) ip;
#endif
	real_ip->ip_len = next;
        real_ip->ip_src = fp->ipq_src;
        real_ip->ip_dst = fp->ipq_dst;

	remque(fp);
	(void) m_free(dtom(fp));
	m = dtom(ip);
	IPREASS_LOCK();
	ipreass.ipr_inuse -= length;
	IPREASS_UNLOCK();
		
	hlen = ((real_ip->ip_vhl & 0x0f) << 2);
	/* restore version */
	real_ip->ip_vhl = (IPVERSION << 4) | (hlen >> 2);
	m->m_len += hlen;
	m->m_data -= hlen;
	m->m_pkthdr.len = next;
	return m;

dropfrag:
	m_freem(m);
dropfrag1:
	NETSTAT_LOCK(&ipstat.ips_lock);
	ipstat.ips_fragdropped++;
	NETSTAT_UNLOCK(&ipstat.ips_lock);
	return (0);
}

/*
 * Update the reassembly queue quota.
 */
static void
ip_dealloc(m)
	register struct mbuf *m;
{
	IPREASS_LOCK();
	while (m) {
		ipreass.ipr_inuse -= MSIZE;
		if (m->m_flags & M_EXT)
			ipreass.ipr_inuse -= m->m_ext.ext_size;
		m = m_free(m);
	}
	IPREASS_UNLOCK();
}

/*
 * Free a fragment reassembly header and all
 * associated datagrams.
 */
static void
ip_freef(fp)
	struct ipq *fp;
{
	register struct ipasfrag *q, *p;

	LOCK_ASSERT("ip_freef", lock_islocked(&ip_frag_lock));
	for (q = fp->ipq_next; q != (struct ipasfrag *)fp; q = p) {
		p = q->ipf_next;
		ip_deq(q);
		ip_dealloc(dtom(q));
	}
	remque(fp);
	(void) m_free(dtom(fp));
}

/*
 * Put an ip fragment on a reassembly chain.
 * Like insque, but pointers in middle of structure.
 */
static void
ip_enq(p, prev)
	register struct ipasfrag *p, *prev;
{

	p->ipf_prev = prev;
	p->ipf_next = prev->ipf_next;
	prev->ipf_next->ipf_prev = p;
	prev->ipf_next = p;
}

/*
 * To ip_enq as remque is to insque.
 */
static void
ip_deq(p)
	register struct ipasfrag *p;
{

	p->ipf_prev->ipf_next = p->ipf_next;
	p->ipf_next->ipf_prev = p->ipf_prev;
}

/*
 * IP timer processing;
 * if a timer expires on a reassembly
 * queue, discard it.
 */
void
ip_slowtimo()
{
	register struct ipq *fp;

	IPFRAG_LOCK();
	fp = ipq.next;
	if (fp) while (fp != &ipq) {
		--fp->ipq_ttl;
		fp = fp->next;
		if (fp->prev->ipq_ttl == 0) {
			NETSTAT_LOCK(&ipstat.ips_lock);
			ipstat.ips_fragtimeout++;
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			if (ipsendreastimo)
				ip_reastimo(fp->prev);
			ip_freef(fp->prev);
		}
	}
	IPFRAG_UNLOCK();
}

/*
 * Generate an ICMP timeout exceeded on reassembly if first fragment
 * exists.
 */
void
ip_reastimo(fp)
	struct ipq *fp;
{
	register struct ipasfrag *q = fp->ipq_next;
	register struct ip *ip;
	register struct mbuf *m, *mcopy;
	struct in_addr dest;
	int hlen, len;
#ifdef __alpha
	ip = q->ipf_ip;
#else
	ip = (struct ip *)q;
#endif

	if (ip->ip_off == 0) {
		/* Add header length subtracted earlier */
		hlen = ((ip->ip_vhl & 0x0f) << 2);
		ip->ip_len = ip->ip_len + hlen;

		/* Need IP header plus 8 bytes for ICMP message */
		len = ICMP_MINLEN + hlen;
		if (len > ip->ip_len) 
			len = ip->ip_len;

		m = dtom(q);
		m->m_len += hlen;
		m->m_data -= hlen;
		mcopy = m_copym(m, 0, len, M_DONTWAIT);
		if (mcopy == NULL)
			return;
		ip = mtod(mcopy, struct ip *);

		/* Restore original header in copy */
		ip->ip_vhl = (IPVERSION << 4) | (hlen >> 2);
		ip->ip_off = IP_MF;
		ip->ip_id = htons(ip->ip_id);
		ip->ip_src = fp->ipq_src;
		ip->ip_dst = fp->ipq_dst;
		dest.s_addr = 0;
		icmp_error(mcopy, ICMP_TIMXCEED, ICMP_TIMXCEED_REASS, dest);
	}
}

/*
 * Drain off all datagram fragments.
 */
void
ip_drain()
{

	IPFRAG_LOCK();
	while (ipq.next != &ipq) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_fragdropped++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		ip_freef(ipq.next);
	}
	IPFRAG_UNLOCK();
}

/*
 * Do option processing on a datagram,
 * possibly discarding it if bad options are encountered,
 * or forwarding it if source-routed.
 * Returns 1 if packet has been forwarded/freed,
 * 0 if the packet should be processed further.
 */
ip_dooptions(m)
	struct mbuf *m;
{
	register struct ip *ip = mtod(m, struct ip *);
	register u_char *cp;
	register struct ip_timestamp *ipt;
	register struct in_ifaddr *ia;
	int opt, optlen, cnt, off, code, type = ICMP_PARAMPROB, forward = 0;
	struct in_addr *sin;
	n_time ntime;
#if	NETISR_THREAD
	extern CONST struct sockaddr_in in_zeroaddr;
	struct sockaddr_in ipaddr;

	ipaddr = in_zeroaddr;
#endif

	cp = (u_char *)(ip + 1);
	cnt = ((ip->ip_vhl & 0x0f) << 2) - sizeof (struct ip);
	for (; cnt > 0; cnt -= optlen, cp += optlen) {
		opt = cp[IPOPT_OPTVAL];
		if (opt == IPOPT_EOL)
			break;
		if (opt == IPOPT_NOP)
			optlen = 1;
		else {
			optlen = cp[IPOPT_OLEN];
			if (optlen <= 0 || optlen > cnt) {
				code = &cp[IPOPT_OLEN] - (u_char *)ip;
				goto bad;
			}
		}
		switch (opt) {

		default:
			break;

		/*
		 * Source routing with record.
		 * Find interface with current destination address.
		 * If none on this machine then drop if strictly routed,
		 * or do nothing if loosely routed.
		 * Record interface address and bring up next address
		 * component.  If strictly routed make sure next
		 * address is on directly accessible net.
		 */
		case IPOPT_LSRR:
		case IPOPT_SSRR:
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *)ip;
				goto bad;
			}
			ipaddr.sin_addr = ip->ip_dst;
			ia = (struct in_ifaddr *)
				ifa_ifwithaddr((struct sockaddr *)&ipaddr);
			if (ia == 0) {
				if (opt == IPOPT_SSRR) {
					type = ICMP_UNREACH;
					code = ICMP_UNREACH_SRCFAIL;
					goto bad;
				}
				/*
				 * Loose routing, and not at next destination
				 * yet; nothing to do except forward.
				 */
				break;
			}
			off--;			/* 0 origin */
			if (off > optlen - sizeof(struct in_addr)) {
				/*
				 * End of source route.  Should be for us.
				 */
				save_rte(cp, ip->ip_src);
				break;
			}
			/*
			 * locate outgoing interface
			 */
			bcopy((caddr_t)(cp + off), (caddr_t)&ipaddr.sin_addr,
			    sizeof(ipaddr.sin_addr));
			if (opt == IPOPT_SSRR) {
#define	INA	struct in_ifaddr *
#define	SA	struct sockaddr *
				if ((ia = (INA)ifa_ifwithdstaddr((SA)&ipaddr)) == 0)
			    		ia = in_iaonnetof(in_netof(ipaddr.sin_addr));
			} else
				ia = ip_rtaddr(ipaddr.sin_addr);
			if (ia == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_SRCFAIL;
				goto bad;
			}
			ip->ip_dst = ipaddr.sin_addr;
			bcopy((caddr_t)&(IA_SIN(ia)->sin_addr),
			    (caddr_t)(cp + off), sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			forward = 1;
			break;

		case IPOPT_RR:
			if ((off = cp[IPOPT_OFFSET]) < IPOPT_MINOFF) {
				code = &cp[IPOPT_OFFSET] - (u_char *)ip;
				goto bad;
			}
			/*
			 * If no space remains, ignore.
			 */
			off--;			/* 0 origin */
			if (off > optlen - sizeof(struct in_addr))
				break;
			bcopy((caddr_t)(&ip->ip_dst), (caddr_t)&ipaddr.sin_addr,
			    sizeof(ipaddr.sin_addr));
			/*
			 * locate outgoing interface; if we're the destination,
			 * use the incoming interface (should be same).
			 */
			if ((ia = (INA)ifa_ifwithaddr((SA)&ipaddr)) == 0 &&
			    (ia = ip_rtaddr(ipaddr.sin_addr)) == 0) {
				type = ICMP_UNREACH;
				code = ICMP_UNREACH_HOST;
				goto bad;
			}
			bcopy((caddr_t)&(IA_SIN(ia)->sin_addr),
			    (caddr_t)(cp + off), sizeof(struct in_addr));
			cp[IPOPT_OFFSET] += sizeof(struct in_addr);
			break;

		case IPOPT_TS:
			code = cp - (u_char *)ip;
			ipt = (struct ip_timestamp *)cp;
			if (ipt->ipt_len < 5)
				goto bad;
			if (ipt->ipt_ptr > ipt->ipt_len - sizeof (int)) {
				u_char oflw = (ipt->ipt_oflg & 0xf0) + 0x10;
				ipt->ipt_oflg &= ~0xf0;
				ipt->ipt_oflg |= oflw;
				if (oflw == 0)
					goto bad;
				break;
			}
			sin = (struct in_addr *)(cp + ipt->ipt_ptr - 1);
			switch (ipt->ipt_oflg & 0x0f) {

			case IPOPT_TS_TSONLY:
				break;

			case IPOPT_TS_TSANDADDR:
				if (ipt->ipt_ptr + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len)
					goto bad;
				ia = ifptoia(m->m_pkthdr.rcvif);
				bcopy((caddr_t)&IA_SIN(ia)->sin_addr,
				    (caddr_t)sin, sizeof(struct in_addr));
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			case IPOPT_TS_PRESPEC:
				if (ipt->ipt_ptr + sizeof(n_time) +
				    sizeof(struct in_addr) > ipt->ipt_len)
					goto bad;
				bcopy((caddr_t)sin, (caddr_t)&ipaddr.sin_addr,
				    sizeof(struct in_addr));
				if (ifa_ifwithaddr((SA)&ipaddr) == 0)
					continue;
				ipt->ipt_ptr += sizeof(struct in_addr);
				break;

			default:
				goto bad;
			}
			ntime = iptime();
			bcopy((caddr_t)&ntime, (caddr_t)cp + ipt->ipt_ptr - 1,
			    sizeof(n_time));
			ipt->ipt_ptr += sizeof(n_time);
		}
	}
	if (forward) {
		ip_forwardscreen(m, 1);
		return (1);
	} else
		return (0);
bad:
	icmp_error(m, type, code, zeroin_addr);
	return (1);
}

/*
 * Given address of next destination (final or next hop),
 * return internet address info of interface to be used to get there.
 */
struct in_ifaddr *
ip_rtaddr(dst)
	 struct in_addr dst;
{
	register struct sockaddr_in *sin;
	ROUTE_LOCK_DECL()

	sin = (struct sockaddr_in *) &ipforward_rt.ro_dst;

	ROUTE_WRITE_LOCK();
	if (ipforward_rt.ro_rt == 0 || dst.s_addr != sin->sin_addr.s_addr) {
		if (ipforward_rt.ro_rt) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = dst;

		rtalloc(&ipforward_rt);
	}
	ROUTE_WRITE_UNLOCK();
	if (ipforward_rt.ro_rt == 0)
		return ((struct in_ifaddr *)0);
	return ((struct in_ifaddr *) ipforward_rt.ro_rt->rt_ifa);
}

/*
 * Save incoming source route for use in replies,
 * to be picked up later by ip_srcroute if the receiver is interested.
 */
void
save_rte(option, dst)
	u_char *option;
	struct in_addr dst;
{
	unsigned olen;

	if (!ipsrcroute)
		return;
	olen = option[IPOPT_OLEN];
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf("save_rte: olen %d\n", olen);
#endif
	if (olen > sizeof(ip_srcrt) - (1 + sizeof(dst)))
		return;
	bcopy((caddr_t)option, (caddr_t)ip_srcrt.srcopt, olen);
	ip_nhops = (olen - IPOPT_OFFSET - 1) / sizeof(struct in_addr);
	ip_srcrt.dst = dst;
}

/*
 * Retrieve incoming source route for use in replies,
 * in the same form used by setsockopt.
 * The first hop is placed before the options, will be removed later.
 */
struct mbuf *
ip_srcroute()
{
	register struct in_addr *p, *q;
	register struct mbuf *m = NULL;

	if (ip_nhops == 0)
		goto out;

	m = m_get(M_DONTWAIT, MT_SOOPTS);
	if (m == 0)
		goto out;

#define OPTSIZ	(sizeof(ip_srcrt.nop) + sizeof(ip_srcrt.srcopt))

	/* length is (nhops+1)*sizeof(addr) + sizeof(nop + srcrt header) */
	m->m_len = ip_nhops * sizeof(struct in_addr) + sizeof(struct in_addr) +
	    OPTSIZ;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf("ip_srcroute: nhops %d mlen %d", ip_nhops, m->m_len);
#endif

	/*
	 * First save first hop for return route
	 */
	p = &ip_srcrt.route[ip_nhops - 1];
	*(mtod(m, struct in_addr *)) = *p--;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf(" hops %X", ntohl(*mtod(m, struct in_addr *)));
#endif

	/*
	 * Copy option fields and padding (nop) to mbuf.
	 */
	ip_srcrt.nop = IPOPT_NOP;
	ip_srcrt.srcopt[IPOPT_OFFSET] = IPOPT_MINOFF;
	bcopy((caddr_t)&ip_srcrt.nop,
	    mtod(m, caddr_t) + sizeof(struct in_addr), OPTSIZ);
	q = (struct in_addr *)(mtod(m, caddr_t) +
	    sizeof(struct in_addr) + OPTSIZ);
#undef OPTSIZ
	/*
	 * Record return path as an IP source route,
	 * reversing the path (pointers are now aligned).
	 */
	while (p >= ip_srcrt.route) {
#if	INETPRINTFS
		if (inetprintfs > 1)
			printf(" %X", ntohl(*q));
#endif
		*q++ = *p--;
	}
	/*
	 * Last hop goes to final destination.
	 */
	*q = ip_srcrt.dst;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf(" %X\n", ntohl(*q));
#endif
out:
	return (m);
}

/*
 * Strip out IP options, at higher
 * level protocol in the kernel.
 * Third argument is buffer to which options
 * will be moved.
 */
void
ip_stripoptions(m, mopt, ipopt )
	register struct mbuf *m;
	struct mbuf *mopt;
	struct ipoption *ipopt;
{
	register int i;
	struct ip *ip = mtod(m, struct ip *);
	register caddr_t opts;
	int olen;

	olen = ((ip->ip_vhl & 0x0f) << 2) - sizeof (struct ip);
	opts = (caddr_t)(ip + 1);
	if(ipopt) {
                  bcopy(opts, ipopt->ipopt_list , olen);
                  ipopt->oplen = olen ;
        }
	i = m->m_len - (sizeof (struct ip) + olen);
	bcopy(opts  + olen, opts, (unsigned)i);
	m->m_len -= olen;
	if (m->m_flags & M_PKTHDR)
		m->m_pkthdr.len -= olen;
	ip->ip_vhl = (IPVERSION << 4) | sizeof(struct ip) >> 2;
}

CONST u_char inetctlerrmap[PRC_NCMDS] = {
	0,		0,		0,		0,
	0,		EMSGSIZE,	EHOSTDOWN,	EHOSTUNREACH,
	EHOSTUNREACH,	EHOSTUNREACH,	ECONNREFUSED,	ECONNREFUSED,
	EMSGSIZE,	EHOSTUNREACH,	0,		0,
	0,		0,		0,		0,
	ENOPROTOOPT
};

/*
 * Forward a packet.  If some error occurs return the sender
 * an icmp packet.  Note we can't always generate a meaningful
 * icmp message because icmp doesn't have a large enough repertoire
 * of codes and types.
 *
 * If not forwarding, just drop the packet.  This could be confusing
 * if ipforwarding was zero but some routing protocol was advancing
 * us as a gateway to somewhere.  However, we must let the routing
 * protocol deal with that.
 *
 * The srcrt parameter indicates whether the packet is being forwarded
 * via a source route.
 */
void
ip_forward(m, srcrt)
	struct mbuf *m;
	int srcrt;
{
	register struct ip *ip = mtod(m, struct ip *);
	register struct sockaddr_in *sin;
	register struct rtentry *rt;
	int error, type = 0, code;
	struct mbuf *mcopy;
	struct in_addr dest;
	ROUTE_LOCK_DECL()

	dest.s_addr = 0;
#if	INETPRINTFS
	if (inetprintfs > 1)
		printf("forward: src %x dst %x ttl %x\n", ip->ip_src,
			ip->ip_dst, ip->ip_ttl);
#endif
	if (m_broadcast(m) || in_canforward(ip->ip_dst) == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_cantforward++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		m_freem(m);
		return;
	}
	ip->ip_id = htons(ip->ip_id);
	if (ip->ip_ttl <= IPTTLDEC) {
		icmp_error(m, ICMP_TIMXCEED, ICMP_TIMXCEED_INTRANS, dest);
		return;
	}
	ip->ip_ttl -= IPTTLDEC;

	sin = (struct sockaddr_in *)&ipforward_rt.ro_dst;
	ROUTE_WRITE_LOCK();
	if ((rt = ipforward_rt.ro_rt) == 0 ||
	    ip->ip_dst.s_addr != sin->sin_addr.s_addr) {
		if (ipforward_rt.ro_rt) {
			RTFREE(ipforward_rt.ro_rt);
			ipforward_rt.ro_rt = 0;
		}
		sin->sin_family = AF_INET;
		sin->sin_len = sizeof(*sin);
		sin->sin_addr = ip->ip_dst;

		rtalloc(&ipforward_rt);
		if (ipforward_rt.ro_rt == 0) {
			ROUTE_WRITE_UNLOCK();
			icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_HOST, dest);
			return;
		}
		rt = ipforward_rt.ro_rt;
	}
	ROUTE_WRITETOREAD_LOCK();

	/*
	 * Save the IP header plus 64 bits of the packet in case
	 * we need to generate an ICMP message to the src.
	 */
	code = ((ip->ip_vhl & 0x0f) << 2) + 8;	/* borrow code for length */
	if (code > (int)ip->ip_len) code = (int)ip->ip_len;
	mcopy = m_copym(m, 0, code, M_DONTWAIT);

	if (ipgateway)
		ip_ifmatrix[rt->rt_ifp->if_index +
			if_index * m->m_pkthdr.rcvif->if_index]++;

	/*
	 * If forwarding packet using same interface that it came in on,
	 * perhaps should send a redirect to sender to shortcut a hop.
	 * Only send redirect if source is sending directly to us,
	 * and if packet was not source routed (or has any options).
	 * Also, don't send redirect if forwarding using a default route
	 * or a route modified by a redirect.
	 */
#define	satosin(sa)	((struct sockaddr_in *)(sa))
	if (rt->rt_ifp == m->m_pkthdr.rcvif &&
	    (rt->rt_flags & (RTF_DYNAMIC|RTF_MODIFIED)) == 0 &&
	    satosin(rt_key(rt))->sin_addr.s_addr != 0 &&
	    ipsendredirects && !srcrt) {
		struct in_ifaddr *ia;
		u_int src = ntohl(ip->ip_src.s_addr);
		u_int dst = ntohl(ip->ip_dst.s_addr);

		if ((ia = ifptoia(m->m_pkthdr.rcvif)) &&
		   (src & ia->ia_subnetmask) == ia->ia_subnet) {
		    if (rt->rt_flags & RTF_GATEWAY)
			dest = satosin(rt->rt_gateway)->sin_addr;
		    else
			dest = ip->ip_dst;
		    /*
		     * If the destination is reached by a route to host,
		     * is on a subnet of a local net, or is directly
		     * on the attached net (!), use host redirect.
		     * (We may be the correct first hop for other subnets.)
		     */
#define	RTA(rt)	((struct in_ifaddr *)(rt->rt_ifa))
		    type = ICMP_REDIRECT;
		    if ((rt->rt_flags & RTF_HOST) ||
		        (rt->rt_flags & RTF_GATEWAY) == 0)
			    code = ICMP_REDIRECT_HOST;
		    else if (RTA(rt)->ia_subnetmask != RTA(rt)->ia_netmask &&
			(dst & RTA(rt)->ia_netmask) ==  RTA(rt)->ia_net)
			    code = ICMP_REDIRECT_HOST;
		    else
			    code = ICMP_REDIRECT_NET;
#if	INETPRINTFS
		    if (inetprintfs > 1)
		        printf("redirect (%d) to %x\n", code, dest.s_addr);
#endif
		}
	}

	ROUTE_READ_UNLOCK();
	error = ip_output(m, (struct mbuf *)0, &ipforward_rt, IP_FORWARDING,
				(struct ip_moptions *)0);
	NETSTAT_LOCK(&ipstat.ips_lock);
	if (error)
		ipstat.ips_cantforward++;
	else {
		ipstat.ips_forward++;
		if (type)
			ipstat.ips_redirectsent++;
		else {
			NETSTAT_UNLOCK(&ipstat.ips_lock);
			if (mcopy)
				m_freem(mcopy);
			return;
		}
	}
	NETSTAT_UNLOCK(&ipstat.ips_lock);
	if (mcopy == NULL)
		return;
	switch (error) {

	case 0:				/* forwarded, but need redirect */
		/* type, code set above */
		break;

	case ENETUNREACH:		/* shouldn't happen, checked above */
	case EHOSTUNREACH:
	case ENETDOWN:
	case EHOSTDOWN:
	default:
		type = ICMP_UNREACH;
		code = ICMP_UNREACH_HOST;
		break;

	case EMSGSIZE:
		type = ICMP_UNREACH;
		code = ICMP_UNREACH_NEEDFRAG;
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_cantfrag++;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
		break;

	case ENOBUFS:
		type = ICMP_SOURCEQUENCH;
		code = 0;
		break;
	}
	icmp_error(mcopy, type, code, dest);
}

#if	MACH_NET
/*
 * I can't believe I'm putting this support back in. But it's easier
 * to put it here than to fix the NetMsgServer to just use its nice
 * parallel connected transport socket to do the same thing in the
 * way it should. Cf kern/mach_net.c
 */

#include "mach/port.h"		/* For PORT_NULL */

extern int (*send_mach_net_datagram)();	/* Linkage point into IP */

void
mach_net_ipinit()
{
	send_mach_net_datagram = mach_net_ipsend;
}

void
mach_net_ipdone()
{
	send_mach_net_datagram = 0;
}

mach_net_ipsend(p, size)
	caddr_t p;
	register int size;
{
	struct mbuf		*top;
	register struct mbuf	**mp;
	register struct ip	*ip;
	ROUTE_LOCK_DECL()
	static struct route	cached_route;

	ip = (struct ip *)p;
	if (size != ip->ip_len || size <= sizeof (*ip))
		return(FALSE);

	top = 0;
	mp = &top;
	do {
		register struct mbuf *m;
		register unsigned int len;

		if (top == 0) {
			m = m_gethdr(M_WAIT, MT_DATA);
			m->m_pkthdr.len = size;
			m->m_pkthdr.rcvif = 0;
			len = MHLEN;
		} else {
			m = m_get(M_WAIT, MT_DATA);
			len = MLEN;
		}
		if (size > MINCLSIZE) {
			MCLGET(m, M_WAIT);
			if (m->m_flags & M_EXT)
				len = MCLBYTES;
		}
		m->m_len = len = MIN(size, len);
		bcopy(p, mtod(m, caddr_t), len);
		p += len;
		size -= len;
		*mp = m;
		mp = &m->m_next;
	} while (size > 0);

	/*
	 * Ugh. Use cached route if the destination address is the same.
	 * A connected socket would do this for us, and better.
	 */
	ROUTE_WRITE_LOCK();
	if (cached_route.ro_rt && (cached_route.ro_rt->rt_flags & RTF_UP)==0 ||
	    ((struct sockaddr_in *)&cached_route.ro_dst)->sin_addr.s_addr
	    != ip->ip_dst.s_addr) {
		if (cached_route.ro_rt)
			RTFREE(cached_route.ro_rt);
		cached_route.ro_rt = 0;
	}
	ROUTE_WRITE_UNLOCK();

	return ip_output(top, (struct mbuf *)0, &cached_route, 
		(IP_FORWARDING|IP_ALLOWBROADCAST), (struct ip_moptions *)0);
}

/*
 *	Source and destination ports.
 *	Common to both UDP datagrams and TCP segments.
 */
struct ip_ports {
	/* struct ip	ip_header; + options */
	u_short		src_port;
	u_short		dst_port;
};

struct mbuf *
mach_net_ipreceive(m, iphlen)
	struct mbuf *m;
	int iphlen;
{
	register struct ip	*ip;
	struct ip_ports		pp;
	port_t			port;

	if (m->m_pkthdr.len < iphlen + sizeof pp)
		return m;
	if (m->m_len >= iphlen + sizeof pp)
		pp = *(struct ip_ports *)(mtod(m, caddr_t) + iphlen);
	else
		m_copydata(m, iphlen, sizeof pp, (caddr_t)&pp);
	/*
	 * The old kern/mach_net didn't swap the ports, but it seems
	 * it should... Either it never ran on little-endian machines
	 * or the server always used palindromes (e.g. 0).
	 */
	pp.src_port = ntohs(pp.src_port);
	pp.dst_port = ntohs(pp.dst_port);
	ip = mtod(m, struct ip *);
	port = find_listener(ip->ip_src.s_addr, pp.src_port,
				ip->ip_dst.s_addr, pp.dst_port, ip->ip_p);
	if (port != PORT_NULL) {
		/* Not sure if server needs this, but kern/mach_net did it */
		if (iphlen > sizeof (struct ip))
			ip_stripoptions(m, (struct mbuf *)0, (struct ipoption *)0 );
		/* Restore IP header, somewhat */
		ip->ip_len += ((ip->ip_vhl & 0x0f) << 2);
		ip->ip_off >>= 3;
		receive_mach_net_datagram(m);
		m = 0;
	}
	return m;
}
#endif
