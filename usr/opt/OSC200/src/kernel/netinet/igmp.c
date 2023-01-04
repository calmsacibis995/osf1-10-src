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
static char	*sccsid = "@(#)$RCSfile: igmp.c,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/16 21:10:28 $";
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
 * Internet Group Management Protocol (IGMP) routines.
 *
 * Written by Steve Deering, Stanford, May 1988.
 *
 * MULTICAST 1.1
 */

#include "machine/endian.h"
#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/net_malloc.h"
#include "net/route.h"

#include "netinet/in.h"
#include "netinet/in_var.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_var.h"
#include "netinet/igmp.h"
#include "netinet/igmp_var.h"

extern struct ifnet loif;

CONST static struct sockproto   igmpproto = { AF_INET, IPPROTO_IGMP };
#if	NETISR_THREAD
extern CONST struct sockaddr_in in_zeroaddr;
#else
struct sockaddr_in igmpsrc = { sizeof (igmpsrc), AF_INET };
struct sockaddr_in igmpdst = { sizeof (igmpdst), AF_INET };
#endif

static int		igmp_timers_are_running = 0;
static u_int		igmp_all_hosts_group;

struct igmpstat igmpstat;
struct socket *ip_mrouter;

#if	NETSYNC_LOCK
lock_data_t	global_igmp_lock;
#define IGMP_LOCKINIT()	lock_init2(&global_igmp_lock, TRUE, LTYPE_IGMP)
#define IGMP_LOCK()	{ NETSPL(s,net); lock_write(&global_igmp_lock); }
#define IGMP_UNLOCK()	{ lock_done(&global_igmp_lock); NETSPLX(s); }
#else
#define IGMP_LOCKINIT()
#define IGMP_LOCK()	NETSPL(s,net)
#define IGMP_UNLOCK()	NETSPLX(s)
#endif

void
igmp_init()
{
	/*
	 * To avoid byte-swapping the same value over and over again.
	 */
	igmp_all_hosts_group = htonl(INADDR_ALLHOSTS_GROUP);
	IGMP_LOCKINIT();
}

void
igmp_input(m, iphlen)
	register struct mbuf *m;
	int iphlen;
{
	register struct igmp *igmp;
	register struct ip *ip;
	register int igmplen;
	register int minlen;
	struct ifnet *ifp;
	struct in_multi *inm;
	struct in_multistep step;
	struct in_ifaddr *ia;
#if	NETISR_THREAD
	struct sockaddr_in igmpsrc, igmpdst;
#endif
	NETSPL_DECL(s)

	NETSTAT_LOCK(&igmpstat.igps_lock);
	++igmpstat.igps_rcv_total;
	NETSTAT_UNLOCK(&igmpstat.igps_lock);

	ip = mtod(m, struct ip *);
	igmplen = ip->ip_len;

	/*
	 * Validate lengths
	 */
	if (igmplen < IGMP_MINLEN) {
		NETSTAT_LOCK(&igmpstat.igps_lock);
		++igmpstat.igps_rcv_tooshort;
		NETSTAT_UNLOCK(&igmpstat.igps_lock);
		m_freem(m);
		return;
	}
	minlen = iphlen + IGMP_MINLEN;
	if (m->m_len < minlen && (m = m_pullup(m, minlen)) == 0) {
		NETSTAT_LOCK(&igmpstat.igps_lock);
		++igmpstat.igps_rcv_tooshort;
		NETSTAT_UNLOCK(&igmpstat.igps_lock);
		return;
	}

	/*
	 * Validate checksum
	 */
	m->m_data += iphlen;
	m->m_len -= iphlen;
	igmp = mtod(m, struct igmp *);
	if (in_cksum(m, igmplen)) {
		NETSTAT_LOCK(&igmpstat.igps_lock);
		++igmpstat.igps_rcv_badsum;
		NETSTAT_UNLOCK(&igmpstat.igps_lock);
		m_freem(m);
		return;
	}
	m->m_data -= iphlen;
	m->m_len += iphlen;
	ifp = m->m_pkthdr.rcvif;
	ip = mtod(m, struct ip *);

	IGMP_LOCK();
	switch (igmp->igmp_type) {

	case IGMP_HOST_MEMBERSHIP_QUERY:
		NETSTAT_LOCK(&igmpstat.igps_lock);
		++igmpstat.igps_rcv_queries;
		NETSTAT_UNLOCK(&igmpstat.igps_lock);

		if (ifp == &loif)
			break;

		if (ip->ip_dst.s_addr != igmp_all_hosts_group) {
			NETSTAT_LOCK(&igmpstat.igps_lock);
			++igmpstat.igps_rcv_badqueries;
			NETSTAT_UNLOCK(&igmpstat.igps_lock);
			goto out;
		}

		/*
		 * Start the timers in all of our membership records for
		 * the interface on which the query arrived, except those
		 * that are already running and those that belong to the
		 * "all-hosts" group.
		 */
		IN_FIRST_MULTI(step, inm);
		while (inm != NULL) {
			if (inm->inm_ifp == ifp && inm->inm_timer == 0 &&
			    inm->inm_addr.s_addr != igmp_all_hosts_group) {
				inm->inm_timer =
					IGMP_RANDOM_DELAY(inm->inm_addr);
				igmp_timers_are_running = 1;
			}
			IN_NEXT_MULTI(step, inm);
		}

		break;

	case IGMP_HOST_MEMBERSHIP_REPORT:
		NETSTAT_LOCK(&igmpstat.igps_lock);
		++igmpstat.igps_rcv_reports;
		NETSTAT_UNLOCK(&igmpstat.igps_lock);

		if (ifp == &loif)
			break;

		if (!IN_MULTICAST(ntohl(igmp->igmp_group.s_addr)) ||
		    igmp->igmp_group.s_addr != ip->ip_dst.s_addr) {
			NETSTAT_LOCK(&igmpstat.igps_lock);
			++igmpstat.igps_rcv_badreports;
			NETSTAT_UNLOCK(&igmpstat.igps_lock);
			goto out;
		}

		/*
		 * KLUDGE: if the IP source address of the report has an
		 * unspecified (i.e., zero) subnet number, as is allowed for
		 * a booting host, replace it with the correct subnet number
		 * so that a process-level multicast routing demon can
		 * determine which subnet it arrived from.  This is necessary
		 * to compensate for the lack of any way for a process to
		 * determine the arrival interface of an incoming packet.
		 */
		if ((ntohl(ip->ip_src.s_addr) & IN_CLASSA_NET) == 0) {
			IFP_TO_IA(ifp, ia);
			if (ia) ip->ip_src.s_addr = htonl(ia->ia_subnet);
		}

		/*
		 * If we belong to the group being reported, stop
		 * our timer for that group.
		 */
		IN_LOOKUP_MULTI(igmp->igmp_group, ifp, inm);
		if (inm != NULL) {
			inm->inm_timer = 0;
			NETSTAT_LOCK(&igmpstat.igps_lock);
			++igmpstat.igps_rcv_ourreports;
			NETSTAT_UNLOCK(&igmpstat.igps_lock);
		}

		break;
	}
	IGMP_UNLOCK();

	/*
	 * Pass all valid IGMP packets up to any process(es) listening
	 * on a raw IGMP socket.
	 */
#if	NETISR_THREAD
	igmpsrc = in_zeroaddr;
	igmpdst = in_zeroaddr;
#endif
	igmpsrc.sin_addr = ip->ip_src;
	igmpdst.sin_addr = ip->ip_dst;
	raw_input(m, &igmpproto,
		    (struct sockaddr *)&igmpsrc, (struct sockaddr *)&igmpdst);
	return;

out:
	IGMP_UNLOCK();
	m_freem(m);
}

void
igmp_joingroup(inm)
	struct in_multi *inm;
{
	NETSPL_DECL(s)

	IGMP_LOCK();
	if (inm->inm_addr.s_addr == igmp_all_hosts_group ||
	    inm->inm_ifp == &loif)
		inm->inm_timer = 0;
	else {
		igmp_sendreport(inm);
		inm->inm_timer = IGMP_RANDOM_DELAY(inm->inm_addr);
		igmp_timers_are_running = 1;
	}
	IGMP_UNLOCK();
}

void
igmp_leavegroup(inm)
	struct in_multi *inm;
{
	/*
	 * No action required on leaving a group.
	 */
}

void
igmp_fasttimo()
{
	register struct in_multi *inm;
	struct in_multistep step;
	NETSPL_DECL(s)

	IGMP_LOCK();
	if (igmp_timers_are_running) {
		igmp_timers_are_running = 0;
		IN_FIRST_MULTI(step, inm);
		while (inm != NULL) {
			if (inm->inm_timer == 0)
				;	/* do nothing */
			else if (--inm->inm_timer == 0)
				igmp_sendreport(inm);
			else
				igmp_timers_are_running = 1;
			IN_NEXT_MULTI(step, inm);
		}
	}
	IGMP_UNLOCK();
}

static void
igmp_sendreport(inm)
	struct in_multi *inm;
{
	struct mbuf *m;
	struct igmp *igmp;
	struct ip *ip;
	struct ip_moptions *imo;
	NETSPL_DECL(s)

	MGETHDR(m, M_DONTWAIT, MT_HEADER);
	if (m == NULL)
		return;
	NET_MALLOC(imo, struct ip_moptions *, sizeof *imo, 
			M_IPMOPTS, M_NOWAIT);
	if (imo == NULL) {
		m_free(m);
		return;
	}
	MH_ALIGN(m, IGMP_MINLEN);
	m->m_len = IGMP_MINLEN;
	igmp = mtod(m, struct igmp *);
	IGMP_LOCK();
	igmp->igmp_type = IGMP_HOST_MEMBERSHIP_REPORT;
	igmp->igmp_code = 0;
	igmp->igmp_group = inm->inm_addr;
	igmp->igmp_cksum = 0;
	igmp->igmp_cksum = in_cksum(m, IGMP_MINLEN);

	m->m_data -= sizeof(struct ip);
	m->m_len  += sizeof(struct ip);
	m->m_pkthdr.len = m->m_len;
	ip = mtod(m, struct ip *);
	ip->ip_tos = 0;
	ip->ip_len = sizeof(struct ip) + IGMP_MINLEN;
	ip->ip_off = 0;
	ip->ip_p = IPPROTO_IGMP;
	ip->ip_src.s_addr = INADDR_ANY;
	ip->ip_dst = igmp->igmp_group;

	imo->imo_multicast_ifp = inm->inm_ifp;
	imo->imo_multicast_ttl = 1;
	/*
	 * Request loopback of the report if we are acting as a multicast
	 * router, so that the process-level routing demon can hear it.
	 */
	imo->imo_multicast_loop = (ip_mrouter != NULL);
	IGMP_UNLOCK();

	ip_output(m, (struct mbuf *)0, (struct route *)0, 0, imo);

	NET_FREE(imo, M_IPMOPTS);
	NETSTAT_LOCK(&igmpstat.igps_lock);
	++igmpstat.igps_snd_reports;
	NETSTAT_UNLOCK(&igmpstat.igps_lock);
}

