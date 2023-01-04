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
static char	*sccsid = "@(#)$RCSfile: raw_ip.c,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/11/23 21:33:12 $";
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
 *	Base:	raw_ip.c	7.6 (Berkeley) 9/20/89
 *	Merged:	raw_ip.c	7.7 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patches.
 *
 */

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
#include "net/raw_cb.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/in_pcb.h"
#include "netinet/ip_var.h"

#include "net/net_malloc.h"

LOCK_ASSERTL_DECL

/*
 * Raw interface to IP protocol.
 */

#if	!NETISR_THREAD
struct	sockaddr_in ripdst = { sizeof(ripdst), AF_INET };
struct	sockaddr_in ripsrc = { sizeof(ripsrc), AF_INET };
struct	sockproto ripproto = { PF_INET };
#endif
/*
 * Setup generic address and protocol structures
 * for raw_input routine, then pass them along with
 * mbuf chain.
 */
void
rip_input(m)
	struct mbuf *m;
{
	register struct ip *ip = mtod(m, struct ip *);
#if	NETISR_THREAD
	extern CONST struct sockaddr_in in_zeroaddr;
	struct	sockaddr_in ripdst, ripsrc;
	struct	sockproto ripproto;

	ripdst = in_zeroaddr;
	ripsrc = in_zeroaddr;
	ripproto.sp_family = PF_INET;
#endif

	ripproto.sp_protocol = ip->ip_p;
	ripdst.sin_addr = ip->ip_dst;
	ripsrc.sin_addr = ip->ip_src;
	if (raw_input(m, &ripproto, (struct sockaddr *)&ripsrc,
	  (struct sockaddr *)&ripdst) == 0) {
		NETSTAT_LOCK(&ipstat.ips_lock);
		ipstat.ips_noproto++;
		ipstat.ips_delivered--;
		NETSTAT_UNLOCK(&ipstat.ips_lock);
	}
}

/*
 * Generate IP header and pass packet to ip_output.
 * Tack on options user may have setup with control call.
 */
#define	satosin(sa)	((struct sockaddr_in *)(sa))
rip_output(m, so)
	register struct mbuf *m;
	struct socket *so;
{
	register struct ip *ip;
	register struct raw_inpcb *rp = sotorawinpcb(so);
	register struct sockaddr_in *sin;

	LOCK_ASSERT("rip_output", SOCKET_ISLOCKED(so));

	/*
	 * If the user handed us a complete IP packet, use it.
	 * Otherwise, allocate an mbuf for a header and fill it in.
	 */
	if (rp->rinp_flags & RINPF_HDRINCL)
		ip = mtod(m, struct ip *);
	else {
		M_PREPEND(m, sizeof(struct ip), M_WAIT);
		ip = mtod(m, struct ip *);
		ip->ip_tos = 0;
		ip->ip_off = 0;
		ip->ip_p = rp->rinp_rcb.rcb_proto.sp_protocol;
		ip->ip_len = m->m_pkthdr.len;
		if (sin = satosin(rp->rinp_rcb.rcb_laddr)) {
			ip->ip_src = sin->sin_addr;
		} else
			ip->ip_src.s_addr = 0;
		if (sin = satosin(rp->rinp_rcb.rcb_faddr))
		    ip->ip_dst = sin->sin_addr;
		ip->ip_ttl = MAXTTL;
	}
	return (ip_output(m,
	   (rp->rinp_flags & RINPF_HDRINCL)? (struct mbuf *)0: rp->rinp_options,
	    &rp->rinp_route, 
	   (so->so_options & SO_DONTROUTE) | IP_ALLOWBROADCAST,
	    rp->rinp_moptions));
}

/*
 * Raw IP socket option processing.
 */
rip_ctloutput(op, so, level, optname, m)
	int op;
	struct socket *so;
	int level, optname;
	struct mbuf **m;
{
	int error = 0;
	register struct raw_inpcb *rp = sotorawinpcb(so);

	if (level != IPPROTO_IP) {
		error = EINVAL;
		if ((op == PRCO_SETOPT) && *m)
			(void)m_free(*m);
	} else switch (op) {

	case PRCO_SETOPT:
		switch (optname) {

		case IP_OPTIONS:
			return (ip_pcbopts(&rp->rinp_options, *m));

		case IP_HDRINCL:
			if (m == 0 || *m == 0 || (*m)->m_len < sizeof (int)) {
				error = EINVAL;
				break;
			}
			if (*mtod(*m, int *))
				rp->rinp_flags |= RINPF_HDRINCL;
			else
				rp->rinp_flags &= ~RINPF_HDRINCL;
			break;

		case IP_MULTICAST_IF:
		case IP_MULTICAST_TTL:
		case IP_MULTICAST_LOOP:
		case IP_ADD_MEMBERSHIP:
		case IP_DROP_MEMBERSHIP:
			error = ip_setmoptions(optname, &rp->rinp_moptions, *m);
			break;
		default:
			error = ip_mrouter_cmd(optname, so, *m);
			break;
		}
		if (*m)
			(void)m_free(*m);
		break;

	case PRCO_GETOPT:
		*m = m_get(M_WAIT, MT_SOOPTS);
		switch (optname) {

		case IP_OPTIONS:
			if (rp->rinp_options) {
				(*m)->m_len = rp->rinp_options->m_len;
				bcopy(mtod(rp->rinp_options, caddr_t),
				    mtod(*m, caddr_t), (unsigned)(*m)->m_len);
			} else
				(*m)->m_len = 0;
			break;

		case IP_HDRINCL:
			(*m)->m_len = sizeof (int);
			*mtod(*m, int *) = rp->rinp_flags & RINPF_HDRINCL;
			break;

		case IP_MULTICAST_IF:
		case IP_MULTICAST_TTL:
		case IP_MULTICAST_LOOP:
		case IP_ADD_MEMBERSHIP:
		case IP_DROP_MEMBERSHIP:
			error = ip_getmoptions(optname, rp->rinp_moptions, m);
			break;

		default:
			error = EINVAL;
			break;
		}
		if (error) {
			(void)m_free(*m);
			*m = 0;
		}
		break;
	}
	return (error);
}

/*ARGSUSED*/
rip_usrreq(so, req, m, nam, control)
	register struct socket *so;
	int req;
	struct mbuf *m, *nam, *control;
{
	register int error = 0;
	register struct raw_inpcb *rp = sotorawinpcb(so);

	LOCK_ASSERT("rip_usrreq", SOCKET_ISLOCKED(so));

	switch (req) {

	case PRU_ATTACH:
		if (rp)
			panic("rip_attach");
		NET_MALLOC(rp, struct raw_inpcb *, sizeof *rp, M_PCB, M_WAITOK);
		if (rp == 0)
			return (ENOBUFS);
		bzero((caddr_t)rp, sizeof *rp);
		so->so_pcb = (caddr_t)rp;
		break;

	case PRU_DETACH:
	    {
		extern struct socket *ip_mrouter;
		if (rp == 0)
			panic("rip_detach");
		if (rp->rinp_options)
			m_freem(rp->rinp_options);
		if (rp->rinp_route.ro_rt)
			RTFREE(rp->rinp_route.ro_rt);
		if (rp->rinp_rcb.rcb_laddr)
			rp->rinp_rcb.rcb_laddr = 0;
		if (so == ip_mrouter)
			ip_mrouter_done();
		if (rp->rinp_moptions)
			ip_freemoptions(rp->rinp_moptions);
		break;
	    }
	case PRU_BIND:
	    {
		struct sockaddr_in *addr = mtod(nam, struct sockaddr_in *);

		if (nam->m_len != sizeof(*addr))
			return (EINVAL);
		if ((ifnet == 0) ||
		    ((addr->sin_family != AF_INET) &&
		     (addr->sin_family != AF_IMPLINK)) ||
		    (addr->sin_addr.s_addr &&
		     ifa_ifwithaddr((struct sockaddr *)addr) == 0))
			return (EADDRNOTAVAIL);
		rp->rinp_rcb.rcb_laddr = (struct sockaddr *)&rp->rinp_laddr;
		rp->rinp_laddr = *addr;
		return (0);
	    }
	case PRU_CONNECT:
	    {
		struct sockaddr_in *addr = mtod(nam, struct sockaddr_in *);

		if (nam->m_len != sizeof(*addr))
			return (EINVAL);
		if (ifnet == 0)
			return (EADDRNOTAVAIL);
		if ((addr->sin_family != AF_INET) &&
		     (addr->sin_family != AF_IMPLINK))
			return (EAFNOSUPPORT);
		rp->rinp_rcb.rcb_faddr = (struct sockaddr *)&rp->rinp_faddr;
		rp->rinp_laddr = *addr;
		soisconnected(so);
		return (0);
	    }
	}
	error =  raw_usrreq(so, req, m, nam, control);

	if (error && (req == PRU_ATTACH) && so->so_pcb)
		NET_FREE(so->so_pcb, M_PCB);
	return (error);
}
