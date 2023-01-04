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
static char	*sccsid = "@(#)$RCSfile: udp_usrreq.c,v $ $Revision: 4.3.9.2 $ (DEC) $Date: 1993/05/16 21:37:39 $";
#endif 
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
 * Copyright (c) 1982, 1986, 1988, 1990 Regents of the University of California.
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
 *	Base:	@(#)udp_usrreq.c	3.2 (Berkeley) 11/4/91
 *	Merged:	udp_usrreq.c	7.17 (Berkeley) 7/1/90
 */

#include "machine/endian.h"
#include "net/net_globals.h"
#if	MACH
#include <sys/secdefines.h>
#endif

#include "sys/param.h"
#include "sys/time.h"
#include "sys/errno.h"
#include "sys/stat.h"

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
#include "netinet/ip_var.h"
#include "netinet/ip_icmp.h"
#include "netinet/udp.h"
#include "netinet/udp_var.h"

#include "net/net_malloc.h"

#if	SEC_ARCH
extern int security_is_on;
#endif

LOCK_ASSERTL_DECL

struct	inpcb udb;
struct	udpstat udpstat;

/*
 * UDP protocol implementation.
 * Per RFC 768, August, 1980.
 */
void
udp_init()
{

	udb.inp_next = udb.inp_prev = &udb;
	INPCBRC_LOCKINIT(&udb);		/* in_pcbnotify frobs */
	udb.inp_refcnt = 2;
	INHEAD_LOCKINIT(&udb);
	NETSTAT_LOCKINIT(&udpstat.udps_lock);
}

int	udpcksum = 1;
int	udp_ttl = UDP_TTL;

#if	!NETISR_THREAD
struct	sockaddr_in udp_in = { sizeof(udp_in), AF_INET };
#endif

void
udp_input(m, iphlen)
	register struct mbuf *m;
	int iphlen;
{
	register struct ip *ip;
	register struct udphdr *uh;
	register struct udpiphdr *ui;
	register struct inpcb *inp;
	register struct socket *so;
	struct mbuf *opts = 0;
	struct mbuf *n;
	int len;
	struct ip save_ip;
	struct ipoption save_ipoption ; 
#if	NETISR_THREAD
	extern CONST struct sockaddr_in in_zeroaddr;
	struct sockaddr_in udp_in;
#endif
	save_ipoption.oplen = 0 ; 

	NETSTAT_LOCK(&udpstat.udps_lock);
	udpstat.udps_ipackets++;
	NETSTAT_UNLOCK(&udpstat.udps_lock);
	/*
	 * Strip IP options, if any; should skip this,
	 * make available to user, and use on returned packets,
	 * but we don't yet have a way to check the checksum
	 * with options still present.
	 */
	if (iphlen > sizeof (struct ip)) {
		ip_stripoptions(m, (struct mbuf *)0, &save_ipoption);
		iphlen = sizeof(struct ip);
	}

	/*
	 * Get IP and UDP header together in first mbuf.
	 */
	ip = mtod(m, struct ip *);
	if (m->m_len < sizeof(struct udpiphdr)) {
		if ((m = m_pullup(m, sizeof(struct udpiphdr))) == 0) {
			NETSTAT_LOCK(&udpstat.udps_lock);
			udpstat.udps_hdrops++;
			NETSTAT_UNLOCK(&udpstat.udps_lock);
			return;
		}
		ip = mtod(m, struct ip *);
	}
	uh = (struct udphdr *)((caddr_t)ip + sizeof (struct ip));

	/*
	 * Make mbuf data length reflect UDP length.
	 * If not enough data to reflect UDP length, drop.
	 */
	len = ntohs((u_short)uh->uh_ulen);
	if (ip->ip_len != len) {
		if (len > (int)ip->ip_len) {
			NETSTAT_LOCK(&udpstat.udps_lock);
			udpstat.udps_badlen++;
			NETSTAT_UNLOCK(&udpstat.udps_lock);
			goto bad;
		}
		m_adj(m, len - (int)ip->ip_len);
		/* ip->ip_len = len; */
	}
	/*
	 * Save a copy of the IP header in case we want to restore it
	 * for sending an ICMP error message in response.
	 */
	save_ip = *ip;

	ui = (struct udpiphdr *) ip;

	if (IN_MULTICAST(ntohl(ip->ip_dst.s_addr)) ||
	    in_broadcast(ip->ip_dst)) {
		struct socket *last;
		/*
		 * Deliver a multicast or broadcast datagram to *all* sockets
		 * for which the local and remote addresses and ports match
		 * those of the incoming datagram.  This allows more than
		 * one process to receive multi/broadcasts on the same port.
		 * (This really ought to be done for unicast datagrams as
		 * well, but that would cause problems with existing
		 * applications that open both address-specific sockets and
		 * a wildcard socket listening to the same port -- they would
		 * end up receiving duplicates of every unicast datagram.
		 * Those applications open the multiple sockets to overcome an
		 * inadequacy of the UDP socket interface, but for backwards
		 * compatibility we avoid the problem here rather than
		 * fixing the interface.  Maybe 4.4BSD will remedy this?)
		 */

		/*
		 * Construct sockaddr format source address.
		 */
#if	NETISR_THREAD
		udp_in = in_zeroaddr;
#endif
		udp_in.sin_port = uh->uh_sport;
		udp_in.sin_addr = ip->ip_src;

		/*
		 * Locate pcb(s) for datagram.
		 * (Algorithm copied from raw_intr().)
		 */
		last = NULL;

		INHEAD_READ_LOCK(&udb);

		for (inp = udb.inp_next; inp != &udb; inp = inp->inp_next) {
			if (inp->inp_lport != uh->uh_dport)
				continue;
			if (inp->inp_laddr.s_addr != INADDR_ANY) {
				if (inp->inp_laddr.s_addr !=
				    ip->ip_dst.s_addr)
					continue;
			}
			if (inp->inp_faddr.s_addr != INADDR_ANY) {
				if (inp->inp_faddr.s_addr !=
				    ip->ip_src.s_addr ||
				    inp->inp_fport != uh->uh_sport)
					continue;
			}
			INPCBRC_REF(inp);

			/*
	 		 * check for buffer space first
	 		 */ 
			so = inp->inp_socket;
			SOCKET_LOCK(so);
			SOCKBUF_LOCK(&so->so_rcv);
			if (sbspace(&so->so_rcv) <  
			      (m->m_pkthdr.len - sizeof (struct udpiphdr))) {
	        	    SOCKBUF_UNLOCK(&so->so_rcv);
			    INPCBRC_UNREF(inp);
                	    SOCKET_UNLOCK(so);
			    NETSTAT_LOCK(&udpstat.udps_lock);
			    udpstat.udps_fullsock++;
			    NETSTAT_UNLOCK(&udpstat.udps_lock);
			    if ((so->so_options & SO_REUSEPORT) == 0) {
				INHEAD_READ_UNLOCK(&udb);
				goto bad;
			    } else
				continue; 
			} 	
			SOCKBUF_UNLOCK(&so->so_rcv);
			SOCKET_UNLOCK(so);

			/*
	 		 * Checksum extended UDP header and data once.
	 		 */
			if (last == NULL) {
#ifdef __alpha
        		    if (udpcksum && ui->ui_sum) {
                		ui->ui_i.fill[0] = ui->ui_i.fill[1] = 0;
                		ui->ui_x1 = 0;
                		ui->ui_len = ui->ui_ulen;
                	        if (ui->ui_sum = 
				    in_cksum(m, len + sizeof (struct ip))) {
#else
        		    if (udpcksum && uh->uh_sum) {
                		((struct ipovly *)ip)->ih_next = 0;
                		((struct ipovly *)ip)->ih_prev = 0;
                		((struct ipovly *)ip)->ih_x1 = 0;
                		((struct ipovly *)ip)->ih_len = uh->uh_ulen;
                		if (uh->uh_sum = 
				    in_cksum(m, len + sizeof (struct ip))) {
#endif
				    SOCKET_LOCK(so);
				    INPCBRC_UNREF(inp);
				    SOCKET_UNLOCK(so);
				    INHEAD_READ_UNLOCK(&udb);
                        	    NETSTAT_LOCK(&udpstat.udps_lock);
                        	    udpstat.udps_badsum++;
                        	    NETSTAT_UNLOCK(&udpstat.udps_lock);
                        	    goto bad;
                		}
        		    }
			} else {
			    struct mbuf *mcopy;

			    if ((mcopy = m_copy(m, 0, M_COPYALL)) != NULL) {
				mcopy->m_len -= sizeof (struct udpiphdr);
				mcopy->m_pkthdr.len -= sizeof (struct udpiphdr);
				mcopy->m_data += sizeof (struct udpiphdr);
				SOCKET_LOCK(last);
				INPCBRC_UNREF(sotoinpcb(last));
				SOCKBUF_LOCK(&last->so_rcv);
				if (sbappendaddr(&last->so_rcv,
					    (struct sockaddr *)&udp_in,
				 	mcopy, (struct mbuf *)0) == 0) {
					SOCKBUF_UNLOCK(&last->so_rcv);
					SOCKET_UNLOCK(last);
					NETSTAT_LOCK(&udpstat.udps_lock);
					udpstat.udps_fullsock++;
					NETSTAT_UNLOCK(&udpstat.udps_lock);
					m_freem(mcopy);
				} else {
					SOCKBUF_UNLOCK(&last->so_rcv);
					sorwakeup(last);
					SOCKET_UNLOCK(last);
				}
			    } else {
				INPCBRC_UNREF(sotoinpcb(last));
				/* no memory, don't try for another pcb */
				INPCBRC_UNREF(sotoinpcb(so));
				INHEAD_READ_UNLOCK(&udb);
				goto bad;
			    }
			}

			/*
			 * Don't look for additional matches if this one
			 * does not have the SO_REUSEPORT socket option set.
			 * This heuristic avoids searching through all pcbs
			 * in the common case of a non-shared port.  It
			 * assumes that an application will never clear
			 * the SO_REUSEPORT option after setting it.
			 */
			last = so;
			if ((last->so_options & SO_REUSEPORT) == 0)
			    break;
		}
		INHEAD_READ_UNLOCK(&udb);

		if (last == NULL) {
			/*
			 * No matching pcb found; discard datagram.
			 * (No need to send an ICMP Port Unreachable
			 * for a broadcast or multicast datgram.)
			 */
			NETSTAT_LOCK(&udpstat.udps_lock);
			udpstat.udps_noport++;
			if (m->m_flags & M_MCAST)
				udpstat.udps_noportmcast++;
			else
				udpstat.udps_noportbcast++;
			NETSTAT_UNLOCK(&udpstat.udps_lock);
			goto bad;
		}

		/* If zero length, and next is EXT, copy header and free */
		m->m_len -= sizeof (struct udpiphdr);
		if ((m->m_len == 0) && (n = m->m_next)) {
			if (n->m_flags & M_EXT) {
			    n->m_pkthdr.len = m->m_pkthdr.len - 
			 		sizeof (struct udpiphdr);
			    n->m_pkthdr.rcvif = m->m_pkthdr.rcvif;
			    n->m_flags = (m->m_flags & M_COPYFLAGS) | M_EXT;
			    m = m_free(m);
			}
		} else {
			m->m_pkthdr.len -= sizeof (struct udpiphdr);
			m->m_data += sizeof (struct udpiphdr);
		}

		SOCKET_LOCK(last);
		INPCBRC_UNREF(sotoinpcb(last));
		SOCKBUF_LOCK(&last->so_rcv);
		if (sbappendaddr(&last->so_rcv, (struct sockaddr *)&udp_in,
		     m, (struct mbuf *)0) == 0) {
			SOCKBUF_UNLOCK(&last->so_rcv);
			SOCKET_UNLOCK(last);
			NETSTAT_LOCK(&udpstat.udps_lock);
			udpstat.udps_fullsock++;
			NETSTAT_UNLOCK(&udpstat.udps_lock);
			goto bad;
		}
		SOCKBUF_UNLOCK(&last->so_rcv);
		sorwakeup(last);
		SOCKET_UNLOCK(last);
		return;
	}  /* End multicast | in_broadcast */

	/*
	 * Locate pcb for datagram.
	 *
	 * BSD "Reno" caches the previous pcb looked up here. This is
	 * problematic with parallelization and thread safety, due to
	 * the synchronization required, and taking/releasing refcounts,
	 * so we push it down to in_pcblookup (where it belongs?).
	 */
	inp = in_pcblookup(&udb, ip->ip_src, uh->uh_sport,
	    ip->ip_dst, uh->uh_dport, INPLOOKUP_WILDCARD|INPLOOKUP_USECACHE);
	if (inp == 0) {
		/* don't send ICMP response for broadcast packet */
		NETSTAT_LOCK(&udpstat.udps_lock);
		udpstat.udps_noport++;
		NETSTAT_UNLOCK(&udpstat.udps_lock);
		if (m_broadcast(m)) {
			NETSTAT_LOCK(&udpstat.udps_lock);
			udpstat.udps_noportbcast++;
			NETSTAT_UNLOCK(&udpstat.udps_lock);
			goto bad;
		}
		/* check the checksum */
#ifdef __alpha
        	if (udpcksum && ui->ui_sum) {
                	ui->ui_i.fill[0] = ui->ui_i.fill[1] = 0;
                	ui->ui_x1 = 0;
                	ui->ui_len = ui->ui_ulen;
                	if (ui->ui_sum = in_cksum(m, len + sizeof (struct ip))) {
#else
        	if (udpcksum && uh->uh_sum) {
                	((struct ipovly *)ip)->ih_next = 0;
                	((struct ipovly *)ip)->ih_prev = 0;
                	((struct ipovly *)ip)->ih_x1 = 0;
                	((struct ipovly *)ip)->ih_len = uh->uh_ulen;
                	if (uh->uh_sum = in_cksum(m, len + sizeof (struct ip))) {
#endif
                        	NETSTAT_LOCK(&udpstat.udps_lock);
                        	udpstat.udps_badsum++;
                        	NETSTAT_UNLOCK(&udpstat.udps_lock);
                        	goto bad;
                	}
        	}

		*ip = save_ip;
		ip->ip_len += iphlen;
		ip->ip_id = htons(ip->ip_id);
		icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_PORT, zeroin_addr);
		return;
	}

	/*
	 * we now can check the socket buffer space before checksum
	 */ 
	so = inp->inp_socket;
	SOCKET_LOCK(so);
	SOCKBUF_LOCK(&so->so_rcv);
	if (sbspace(&so->so_rcv) <  (m->m_pkthdr.len - sizeof (struct udpiphdr))) {
	        SOCKBUF_UNLOCK(&so->so_rcv);
		INPCBRC_UNREF(inp);		/* bumped in in_pcblookup */
                SOCKET_UNLOCK(so);
		NETSTAT_LOCK(&udpstat.udps_lock);
		udpstat.udps_fullsock++;
		NETSTAT_UNLOCK(&udpstat.udps_lock);
		goto bad;
	} 	
	SOCKBUF_UNLOCK(&so->so_rcv);
	SOCKET_UNLOCK(so);

	/*
	 * Checksum extended UDP header and data.
	 */
	
#ifdef __alpha
        if (udpcksum && ui->ui_sum) {
                ui->ui_i.fill[0] = ui->ui_i.fill[1] = 0;
                ui->ui_x1 = 0;
                ui->ui_len = ui->ui_ulen;
                if (ui->ui_sum = in_cksum(m, len + sizeof (struct ip))) {
#else
        if (udpcksum && uh->uh_sum) {
                ((struct ipovly *)ip)->ih_next = 0;
                ((struct ipovly *)ip)->ih_prev = 0;
                ((struct ipovly *)ip)->ih_x1 = 0;
                ((struct ipovly *)ip)->ih_len = uh->uh_ulen;
                if (uh->uh_sum = in_cksum(m, len + sizeof (struct ip))) {
#endif
			SOCKET_LOCK(so);
			INPCBRC_UNREF(inp);	/* bumped in in_pcblookup */
			SOCKET_UNLOCK(so);
                        NETSTAT_LOCK(&udpstat.udps_lock);
                        udpstat.udps_badsum++;
                        NETSTAT_UNLOCK(&udpstat.udps_lock);
                        goto bad;
                }
        }

	/*
	 * Construct sockaddr format source address.
	 * Stuff source address and datagram in user buffer.
	 */
#if	NETISR_THREAD
	udp_in = in_zeroaddr;
#endif
	udp_in.sin_port = uh->uh_sport;
	udp_in.sin_addr = ip->ip_src;
	if (inp->inp_flags & INP_CONTROLOPTS) {
		struct mbuf **mp = &opts;

		if (inp->inp_flags & INP_RECVDSTADDR) {
			*mp = udp_saveopt((caddr_t) &ip->ip_dst,
			    sizeof(struct in_addr), IP_RECVDSTADDR);
			if (*mp)
				mp = &(*mp)->m_next;
		}

		/* support for receiving all the ip options */
		if (inp->inp_flags & INP_RECVOPTS) {
			*mp = udp_saveopt((caddr_t) &save_ipoption.ipopt_list[0],
			    save_ipoption.oplen, IP_RECVOPTS);
			save_ipoption.oplen = 0 ; 
			if (*mp)
				mp = &(*mp)->m_next;
		}
#ifdef notyet
		/* ip_srcroute doesn't do what we want here, need to fix */
		if (inp->inp_flags & INP_RECVRETOPTS) {
			*mp = udp_saveopt((caddr_t) ip_srcroute(),
			    sizeof(struct in_addr), IP_RECVRETOPTS);
			if (*mp)
				mp = &(*mp)->m_next;
		}
#endif
	}
	iphlen += sizeof(struct udphdr);
	m->m_len -= iphlen;
	m->m_pkthdr.len -= iphlen;
	m->m_data += iphlen;

	/* If zero length, and next is EXT, copy header and free */
	if ((m->m_len == 0) && (n = m->m_next))
		if (n->m_flags & M_EXT) {
			n->m_pkthdr.len = m->m_pkthdr.len;
			n->m_pkthdr.rcvif = m->m_pkthdr.rcvif;
			n->m_flags = (m->m_flags & M_COPYFLAGS) | M_EXT;
			m = m_free(m);
		}
		 
	SOCKET_LOCK(so);
	INPCBRC_UNREF(inp);	/* Not necessary to INPCB_LOCK(inp); */
	SOCKBUF_LOCK(&so->so_rcv);
	if (sbappendaddr(&so->so_rcv, (struct sockaddr *)&udp_in,
	    m, opts) == 0) {
		SOCKBUF_UNLOCK(&so->so_rcv);
		SOCKET_UNLOCK(so);
		NETSTAT_LOCK(&udpstat.udps_lock);
		udpstat.udps_fullsock++;
		NETSTAT_UNLOCK(&udpstat.udps_lock);
		goto bad;
	}
	SOCKBUF_UNLOCK(&so->so_rcv);
	sorwakeup(so);
	SOCKET_UNLOCK(so);
	return;
bad:
	m_freem(m);
	if (opts)
		m_freem(opts);
}

/*
 * Create a "control" mbuf containing the specified data
 * with the specified type for presentation with a datagram.
 */
struct mbuf *
udp_saveopt(p, size, type)
	caddr_t p;
	register int size;
	int type;
{
	register struct cmsghdr *cp;
	struct mbuf *m;

	if ((m = m_get(M_DONTWAIT, MT_CONTROL)) == NULL)
		return ((struct mbuf *) NULL);
	cp = (struct cmsghdr *) mtod(m, struct cmsghdr *);
	bcopy(p, (caddr_t)cp + sizeof(struct cmsghdr), size);
	size += sizeof(*cp);
	m->m_len = size;
	cp->cmsg_len = size;
	cp->cmsg_level = IPPROTO_IP;
	cp->cmsg_type = type;
	return (m);
}

/*
 * Notify a udp user of an asynchronous error;
 * just wake up so that it can collect error status.
 */
void
udp_notify(inp, errno)
	struct inpcb *inp;
{
	register struct socket *so = inp->inp_socket;

	LOCK_ASSERT("udp_notify so", SOCKET_ISLOCKED(so));
	so->so_error = errno;
	sorwakeup(so);
	sowwakeup(so);
}

void
udp_ctlinput(cmd, sa, ip)
	int cmd;
	struct sockaddr *sa;
	register struct ip *ip;
{
	register struct udphdr *uh;

	if ((unsigned)cmd > PRC_NCMDS || inetctlerrmap[cmd] == 0)
		return;
	if (ip) {
		uh = (struct udphdr *)((caddr_t)ip + ((ip->ip_vhl&0x0f) << 2));
		in_pcbnotify(&udb, sa, uh->uh_dport, ip->ip_src, uh->uh_sport,
			cmd, udp_notify);
	} else
		in_pcbnotify(&udb, sa, 0, zeroin_addr, 0, cmd, udp_notify);
}

udp_output(inp, m, addr, control)
	register struct inpcb *inp;
	register struct mbuf *m;
	struct mbuf *addr, *control;
{
	register struct udpiphdr *ui;
	register int len = m->m_pkthdr.len;
	struct in_addr laddr;
	int error = 0;
	NETSPL_DECL(s)

	LOCK_ASSERT("udp_output", INPCB_ISLOCKED(inp));
	if (control)
		m_freem(control);		/* XXX */

	if (addr) {
		laddr = inp->inp_laddr;
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			goto release;
		}
		/*
		 * Must block input while temporarily connected.
		 */
		NETSPL(s,net);
		INHEAD_WRITE_LOCK(&udb);
		error = in_pcbconnect(inp, addr);
		if (error) {
			m_freem(m);
			goto nosend;
		}
	} else {
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			goto release;
		}
	}
	/*
	 * Calculate data length and get a mbuf
	 * for UDP and IP headers.
	 */
	M_PREPEND(m, sizeof(struct udpiphdr), M_WAIT);

	/*
	 * Fill in mbuf with extended UDP header
	 * and addresses and length put into network format.
	 */
	ui = mtod(m, struct udpiphdr *);
#ifdef __alpha
	ui->ui_i.fill[0] = ui->ui_i.fill[1] = 0;
#else
	ui->ui_next = ui->ui_prev = 0;
#endif
	ui->ui_x1 = 0;
	ui->ui_pr = IPPROTO_UDP;
	ui->ui_len = htons((u_short)len + sizeof (struct udphdr));
	ui->ui_src = inp->inp_laddr;
	ui->ui_dst = inp->inp_faddr;
	ui->ui_sport = inp->inp_lport;
	ui->ui_dport = inp->inp_fport;
	ui->ui_ulen = ui->ui_len;

	/*
	 * Stuff checksum and output datagram.
	 */
	ui->ui_sum = 0;
	if (udpcksum) {
	    if ((ui->ui_sum = in_cksum(m, sizeof (struct udpiphdr) + len)) == 0)
		ui->ui_sum = 0xffff;
	}
	((struct ip *)ui)->ip_len = sizeof (struct udpiphdr) + len;
	((struct ip *)ui)->ip_ttl = inp->inp_ip.ip_ttl;	/* XXX */
	((struct ip *)ui)->ip_tos = inp->inp_ip.ip_tos;	/* XXX */
	NETSTAT_LOCK(&udpstat.udps_lock);
	udpstat.udps_opackets++;
	NETSTAT_UNLOCK(&udpstat.udps_lock);
	error = ip_output(m, inp->inp_options, &inp->inp_route,
	    inp->inp_socket->so_options & (SO_DONTROUTE | SO_BROADCAST), 
		inp->inp_moptions);

	if (addr) {
		in_pcbdisconnect(inp);
		inp->inp_laddr = laddr;
nosend:
		INHEAD_WRITE_UNLOCK(&udb);
		NETSPLX(s);
	}
	return (error);

release:
	m_freem(m);
	return (error);
}

u_long	udp_sendspace = 9216;		/* really max datagram size */
u_long	udp_recvspace = 40 * (1024 + sizeof(struct sockaddr_in));
					/* 40 1K datagrams */

/*ARGSUSED*/
udp_usrreq(so, req, m, addr, control)
	struct socket *so;
	int req;
	struct mbuf *m, *addr, *control;
{
	struct inpcb *inp = sotoinpcb(so);
	int error = 0;

	LOCK_ASSERT("udp_usrreq", SOCKET_ISLOCKED(so));

	if (req == PRU_CONTROL)
		return (in_control(so, (int)m, (caddr_t)addr,
			(struct ifnet *)control));

#if 	SEC_ARCH
	if (!security_is_on)
#endif
	if (control && control->m_len) {
		error = EINVAL;
		goto release;
	}

	if (inp) {
		INPCB_LOCK(inp);
	} else if (req != PRU_ATTACH) {
		error = EINVAL;
		goto release;
	}

	switch (req) {

	case PRU_ATTACH:
		if (inp != NULL) {
			error = EINVAL;
			break;
		}
		error = in_pcballoc(so, &udb);
		if (error)
			break;
		error = soreserve(so, udp_sendspace, udp_recvspace);
		if (error)
			break;
		((struct inpcb *) so->so_pcb)->inp_ip.ip_ttl = udp_ttl;
		break;

	case PRU_DETACH:
		in_pcbdetach(inp);
		inp = 0;
		break;

	case PRU_BIND:
		error = in_pcbbind(inp, addr);
		break;

	case PRU_LISTEN:
		error = EOPNOTSUPP;
		break;

	case PRU_CONNECT:
		if (inp->inp_faddr.s_addr != INADDR_ANY) {
			error = EISCONN;
			break;
		}
		error = in_pcbconnect(inp, addr);
		if (error == 0)
			soisconnected(so);
		break;

	case PRU_CONNECT2:
		error = EOPNOTSUPP;
		break;

	case PRU_ACCEPT:
		error = EOPNOTSUPP;
		break;

	case PRU_DISCONNECT:
		if (inp->inp_faddr.s_addr == INADDR_ANY) {
			error = ENOTCONN;
			break;
		}
		in_pcbdisconnect(inp);
		inp->inp_laddr.s_addr = INADDR_ANY;
		so->so_state &= ~SS_ISCONNECTED;		/* XXX */
		break;

	case PRU_SHUTDOWN:
		socantsendmore(so);
		break;

	case PRU_SEND:
		error = udp_output(inp, m, addr, control);
		m = 0;
		control = 0;
		break;

	case PRU_ABORT:
		soisdisconnected(so);
		in_pcbdetach(inp);
		inp = 0;
		break;

	case PRU_SOCKADDR:
		in_setsockaddr(inp, addr);
		break;

	case PRU_PEERADDR:
		in_setpeeraddr(inp, addr);
		break;

	case PRU_SENSE:
		/*
		 * stat: don't bother with a blocksize.
		 */
		INPCB_UNLOCK(inp);
		return (0);

	case PRU_SENDOOB:
	case PRU_FASTTIMO:
	case PRU_SLOWTIMO:
	case PRU_PROTORCV:
	case PRU_PROTOSEND:
		error =  EOPNOTSUPP;
		break;

	case PRU_RCVD:
	case PRU_RCVOOB:
		INPCB_UNLOCK(inp);
		return (EOPNOTSUPP);	/* do not free mbuf's */

	default:
		panic("udp_usrreq");
	}
	if (inp)
		INPCB_UNLOCK(inp);
release:
	if (control != NULL)
		m_freem(control);
	if (m != NULL)
		m_freem(m);
	return (error);
}
