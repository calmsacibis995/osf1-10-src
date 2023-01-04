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
static char	*sccsid = "@(#)$RCSfile: tcp_subr.c,v $ $Revision: 4.3.5.3 $ (DEC) $Date: 1993/11/22 23:34:29 $";
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
 */
/*
 *	Revision History:
 *
 * 3-June-91	Heather Gray
 *	Interim OSF patch. Init tcp_sendspace to reasonable minimum value.
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
#include "netinet/ip_var.h"
#include "netinet/ip_icmp.h"
#include "netinet/tcp.h"
#include "netinet/tcpip.h"
#include "netinet/tcp_fsm.h"
#include "netinet/tcp_seq.h"
#include "netinet/tcp_timer.h"
#include "netinet/tcp_var.h"

#include "net/net_malloc.h"

LOCK_ASSERTL_DECL

struct	inpcb tcb;		/* head of queue of active tcpcb's */
struct	tcpstat tcpstat;	/* tcp statistics */
tcp_seq	tcp_iss;

/* patchable/settable parameters for tcp */
int	tcp_ttl = TCP_TTL;
int 	tcp_mssdflt = TCP_MSS;
int 	tcp_rttdflt = TCPTV_SRTTDFLT / PR_SLOWHZ;
int	tcp_compat_42 = TCP_COMPAT_42;
int	tcp_urgent_42 = 1;  
int	tcp_dont_winscale = 0;
int 	tcp_rptr2dflt = 0;
int	tcp_connr2_conf = 1;		/* Connect R2 conforms to 1122 */
int	tcp_connr2_shift = 0;		/* Negative gives old behaviour
					   if conformance not true */

#if	NETSYNC_LOCK
simple_lock_data_t	misc_tcp_lock;
#endif

#ifdef __alpha
#define reasstruct ovtcpiphdr
#else  /* not __alpha */
#define reasstruct tcpiphdr
#endif /* __alpha */
/*
 * Tcp initialization
 */
void
tcp_init()
{
	extern u_long tcp_sendspace;

	tcp_iss = 1;		/* wrong */
	tcb.inp_next = tcb.inp_prev = &tcb;
	if (max_protohdr < sizeof(struct tcpiphdr))
		max_protohdr = sizeof(struct tcpiphdr);
	if (max_linkhdr + sizeof(struct tcpiphdr) > MHLEN)
		panic("tcp_init");
	if (tcp_sendspace < 2 * MCLBYTES)
		tcp_sendspace = 2 * MCLBYTES;
	INPCBRC_LOCKINIT(&tcb);		/* in_pcbnotify, tcp_slowtimo frob */
	tcb.inp_refcnt = 2;
	INHEAD_LOCKINIT(&tcb);
	TCPMISC_LOCKINIT();
	NETSTAT_LOCKINIT(&tcpstat.tcps_lock);
}

/*
 * Fill in template to be used to send tcp packets on a connection.
 * Skeletal tcp/ip header, minimizing the amount of work
 * necessary when the connection is used.
 */
void
tcp_template(tp)
	struct tcpcb *tp;
{
	register struct inpcb *inp = tp->t_inpcb;
	register struct tcpiphdr *n = &tp->t_template;

#ifdef __alpha
	n->ti_i.fill[0] = n->ti_i.fill[1] = 0;
#else
	n->ti_next = n->ti_prev = 0;
#endif
	n->ti_x1 = 0;
	n->ti_pr = IPPROTO_TCP;
	n->ti_len = htons(sizeof (struct tcpiphdr) - sizeof (struct ip));
	n->ti_src = inp->inp_laddr;
	n->ti_dst = inp->inp_faddr;
	n->ti_sport = inp->inp_lport;
	n->ti_dport = inp->inp_fport;
	n->ti_seq = 0;
	n->ti_ack = 0;
	n->ti_xoff = sizeof (struct tcphdr) << 2;
	n->ti_flags = 0;
	n->ti_win = 0;
	n->ti_sum = 0;
	n->ti_urp = 0;
}

/*
 * Send a single message to the TCP at address specified by
 * the given TCP/IP header.  If m == 0, then we make a copy
 * of the tcpiphdr at ti and send directly to the addressed host.
 * This is used to force keep alive messages out using the TCP
 * template for a connection tp->t_template.  If flags are given
 * then we send a message back to the TCP which originated the
 * segment ti, and discard the mbuf containing it and any other
 * attached mbufs.
 *
 * In any case the ack and sequence number of the transmitted
 * segment are as specified by the parameters.
 */
void
tcp_respond(tp, ti, m, ack, seq, flags)
	struct tcpcb *tp;
	register struct tcpiphdr *ti;
	register struct mbuf *m;
	tcp_seq ack, seq;
	int flags;
{
	int win = 0, tlen;
	struct route *ro = 0;

	if (tp) {
		win = sbspace(&tp->t_inpcb->inp_socket->so_rcv);
		ro = &tp->t_inpcb->inp_route;
	}
	if (m == 0) {
		m = m_gethdr(M_DONTWAIT, MT_HEADER);
		if (m == NULL)
			return;
		if (tcp_compat_42)
			tlen = 1;
		else
			tlen = 0;
		m->m_data += max_linkhdr;
		*mtod(m, struct tcpiphdr *) = *ti;
		ti = mtod(m, struct tcpiphdr *);
		flags = TH_ACK;
	} else {
		m_freem(m->m_next);
		m->m_next = 0;
		m->m_data = (caddr_t)ti;
		tlen = 0;
#define xchg(a,b,type) { type t; t=a; a=b; b=t; }
		xchg(ti->ti_dst.s_addr, ti->ti_src.s_addr, u_int);
		xchg(ti->ti_dport, ti->ti_sport, u_short);
#undef xchg
	}
	m->m_len = sizeof (struct tcpiphdr) + tlen;
#ifdef __alpha
	ti->ti_i.fill[0] = ti->ti_i.fill[1] = 0;
#else
	ti->ti_next = ti->ti_prev = 0;
#endif
	ti->ti_x1 = 0;
	ti->ti_len = htons((u_short)(sizeof (struct tcphdr) + tlen));
	ti->ti_seq = htonl(seq);
	ti->ti_ack = htonl(ack);
	ti->ti_xoff = sizeof (struct tcphdr) << 2;
	ti->ti_flags = flags;
        if ( tp )
                ti->ti_win = htons( (u_short) (win >> tp->rcv_scale) );
        else
                ti->ti_win = htons((u_short)win);
	ti->ti_urp = 0;
	ti->ti_sum = in_cksum(m, sizeof (struct tcpiphdr) + tlen);
	((struct ip *)ti)->ip_len = sizeof (struct tcpiphdr) + tlen;
	((struct ip *)ti)->ip_ttl = tcp_ttl;
	if (m->m_flags & M_PKTHDR) {
		m->m_pkthdr.rcvif = 0;
		m->m_pkthdr.len = ((struct ip *)ti)->ip_len;
	} else
		panic("tcp_respond");
	(void) ip_output(m, (struct mbuf *)0, ro, 0, (struct ip_moptions *)0);
}

/*
 * Create a new TCP control block, making an
 * empty reassembly queue and hooking it to the argument
 * protocol control block.
 */
struct tcpcb *
tcp_newtcpcb(inp)
	struct inpcb *inp;
{
	register struct tcpcb *tp;

	LOCK_ASSERT("tcp_newtcpcb", INPCB_ISLOCKED(inp));
	NET_MALLOC(tp, struct tcpcb *, sizeof *tp, M_PCB, M_NOWAIT);
	if (tp == NULL)
		return ((struct tcpcb *)0);
	bzero((caddr_t)tp, sizeof *tp);
	tp->seg_next = tp->seg_prev = (struct reasstruct *)tp;
	tp->t_maxseg = tcp_mssdflt;
	tp->t_rptr2rxt = tcp_rptr2dflt;
	tp->t_rptr2cur = 0;

	tp->t_flags = 0;		/* sends options! */
	tp->t_inpcb = inp;
	/*
	 * Init srtt to TCPTV_SRTTBASE (0), so we can tell that we have no
	 * rtt estimate.  Set rttvar so that srtt + 2 * rttvar gives
	 * reasonable initial retransmit time.
	 */
	tp->t_srtt = TCPTV_SRTTBASE;
	tp->t_rttvar = tcp_rttdflt * PR_SLOWHZ << 2;
	tp->t_rttmin = TCPTV_MIN;
	TCPT_RANGESET(tp->t_rxtcur, 
	    ((TCPTV_SRTTBASE >> 2) + (TCPTV_SRTTDFLT << 2)) >> 1,
	    TCPTV_MIN, TCPTV_REXMTMAX);
	tp->snd_cwnd = TCP_MAXWIN<<TCP_MAX_WINSHIFT;
	tp->snd_ssthresh = TCP_MAXWIN<<TCP_MAX_WINSHIFT;
	inp->inp_ip.ip_ttl = tcp_ttl;

        if ( ! tcp_dont_winscale )
                tp->t_flags |= TF_REQ_SCALE;
	tp->t_timestamp = tcp_iss;
	inp->inp_ppcb = (caddr_t)tp;
	return (tp);
}

/*
 * Drop a TCP connection, reporting
 * the specified error.  If connection is synchronized,
 * then send a RST to peer.
 */
struct tcpcb *
tcp_drop(tp, errno)
	register struct tcpcb *tp;
	int errno;
{
	struct socket *so = tp->t_inpcb->inp_socket;

	if (TCPS_HAVERCVDSYN(tp->t_state)) {
		tp->t_state = TCPS_CLOSED;
		(void) tcp_output(tp);
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_drops++;
	} else {
		NETSTAT_LOCK(&tcpstat.tcps_lock);
		tcpstat.tcps_conndrops++;
	}
	NETSTAT_UNLOCK(&tcpstat.tcps_lock);
	if (errno == ETIMEDOUT && tp->t_softerror)
		errno = tp->t_softerror;
	so->so_error = errno;
	return (tcp_close(tp));
}

/*
 * Close a TCP control block:
 *	discard all space held by the tcp
 *	discard internet protocol block
 *	wake up any sleepers
 */
struct tcpcb *
tcp_close(tp)
	register struct tcpcb *tp;
{
	register struct reasstruct *t;
	struct inpcb *inp = tp->t_inpcb;
	struct socket *so = inp->inp_socket;
	register struct mbuf *m;
#ifdef RTV_RTT
	register struct rtentry *rt;
#endif
	ROUTE_LOCK_DECL()

	LOCK_ASSERT("tcp_close", INPCB_ISLOCKED(inp));

#ifdef RTV_RTT
	/*
	 * If we sent enough data to get some meaningful characteristics,
	 * save them in the routing entry.  'Enough' is arbitrarily 
	 * defined as 4K (default tcp_sendspace) * 16.  This would
	 * give us 16 rtt samples assuming we only get one sample per
	 * window (the usual case on a long haul net).  16 samples is
	 * enough for the srtt filter to converge to within 5% of the correct
	 * value; fewer samples and we could save a very bogus rtt.
	 *
	 * Don't update the default route's characteristics and don't
	 * update anything that the user "locked".
	 */
	ROUTE_WRITE_LOCK();
	if (SEQ_LT(tp->iss+(4096*16), tp->snd_max) &&
	    (rt = inp->inp_route.ro_rt) &&
	    ((struct sockaddr_in *) rt_key(rt))->sin_addr.s_addr !=
	    INADDR_ANY) {
		register u_int i;

		if ((rt->rt_rmx.rmx_locks & RTV_RTT) == 0) {
			i = tp->t_srtt *
			    (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTT_SCALE));
			if (rt->rt_rmx.rmx_rtt && i)
				/*
				 * filter this update to half the old & half
				 * the new values, converting scale.
				 * See route.h and tcp_var.h for a
				 * description of the scaling constants.
				 */
				rt->rt_rmx.rmx_rtt =
				    (rt->rt_rmx.rmx_rtt + i) / 2;
			else
				rt->rt_rmx.rmx_rtt = i;
		}
		if ((rt->rt_rmx.rmx_locks & RTV_RTTVAR) == 0) {
			i = tp->t_rttvar *
			    (RTM_RTTUNIT / (PR_SLOWHZ * TCP_RTTVAR_SCALE));
			if (rt->rt_rmx.rmx_rttvar && i)
				rt->rt_rmx.rmx_rttvar =
				    (rt->rt_rmx.rmx_rttvar + i) / 2;
			else
				rt->rt_rmx.rmx_rttvar = i;
		}
		/*
		 * update the pipelimit (ssthresh) if it has been updated
		 * already or if a pipesize was specified & the threshhold
		 * got below half the pipesize.  I.e., wait for bad news
		 * before we start updating, then update on both good
		 * and bad news.
		 */
		if ((rt->rt_rmx.rmx_locks & RTV_SSTHRESH) == 0 &&
		    (i = tp->snd_ssthresh) && rt->rt_rmx.rmx_ssthresh ||
		    i < (rt->rt_rmx.rmx_sendpipe / 2)) {
			/*
			 * convert the limit from user data bytes to
			 * packets then to packet data bytes.
			 */
			i = (i + tp->t_maxseg / 2) / tp->t_maxseg;
			if (i < 2)
				i = 2;
			i *= (u_int)(tp->t_maxseg + sizeof (struct tcpiphdr));
			if (rt->rt_rmx.rmx_ssthresh)
				rt->rt_rmx.rmx_ssthresh =
				    (rt->rt_rmx.rmx_ssthresh + i) / 2;
			else
				rt->rt_rmx.rmx_ssthresh = i;
		}
	}
	ROUTE_WRITE_UNLOCK();
#endif RTV_RTT
	/* free the reassembly queue, if any */
	t = tp->seg_next;
	while (t != (struct reasstruct *)tp) {
		t = (struct reasstruct *)t->ti_next;
#ifdef __alpha
		m = REASS_MBUF(((struct ovtcpiphdr *)t->ti_prev)->ti_ipovly);
#else
		m = REASS_MBUF((struct tcpiphdr *)t->ti_prev);
#endif /* __alpha */
		remque(t->ti_prev);
		m_freem(m);
	}
	NET_FREE(tp, M_PCB);
	inp->inp_ppcb = 0;
	soisdisconnected(so);
	in_pcbdetach(inp);
	NETSTAT_LOCK(&tcpstat.tcps_lock);
	tcpstat.tcps_closed++;
	NETSTAT_UNLOCK(&tcpstat.tcps_lock);
	return ((struct tcpcb *)0);
}

void
tcp_drain()
{

}

/*
 * Notify a tcp user of an asynchronous error;
 * store error as soft error, but wake up user
 * (for now, won't do anything until can select for soft error).
 */
void
tcp_notify(inp, error)
	struct inpcb *inp;
	int error;
{
	register struct socket *so = inp->inp_socket;
	register struct tcpcb *tp = (struct tcpcb *)inp->inp_ppcb;

	LOCK_ASSERT("tcp_notify so", SOCKET_ISLOCKED(so));
	LOCK_ASSERT("tcp_notify inpcb", INPCB_ISLOCKED(inp));

	/*
	 * If conformance to RFC 1122 is negated :
	 * If connection hasn't completed, and if configured to ignore
	 * R2 (by a negative connr2) or has retransmitted "several" times,
	 * and receives a second error, give up now.  This is better
	 * than waiting a long time to establish a connection that
	 * can never complete.
	 */
	if (!tcp_connr2_conf) {
		if (tp->t_state < TCPS_ESTABLISHED && 
		    tcp_connr2_shift < 0 ||
		    (tp->t_rxtshift > tcp_connr2_shift && tp->t_softerror)) {
			so->so_error = error;
			tp->t_softerror = 0;
		} else
			tp->t_softerror = error;
	}
	else
		tp->t_softerror = error;
	wakeup((caddr_t) &so->so_timeo);
	sorwakeup(so);
	sowwakeup(so);
}

void
tcp_ctlinput(cmd, sa, ip)
	int cmd;
	struct sockaddr *sa;
	register struct ip *ip;
{
	register struct tcphdr *th;
	void (*notify)() = tcp_notify;

	if (cmd == PRC_QUENCH)
		notify = tcp_quench;
	else if ((unsigned)cmd > PRC_NCMDS || inetctlerrmap[cmd] == 0)
		return;
	if (ip) {
		th = (struct tcphdr *)((caddr_t)ip + ((ip->ip_vhl&0x0f) << 2));
		in_pcbnotify(&tcb, sa, th->th_dport, ip->ip_src, th->th_sport,
			cmd, notify);
	} else
		in_pcbnotify(&tcb, sa, 0, zeroin_addr, 0, cmd, notify);
}

/*
 * When a source quench is received, close congestion window
 * to one segment.  We will gradually open it again as we proceed.
#if	NETSYNC_LOCK
 * Entered with INPCB lock held.
#endif
 */
void
tcp_quench(inp)
	struct inpcb *inp;
{
	struct tcpcb *tp = intotcpcb(inp);

	if (tp)
		tp->snd_cwnd = tp->t_maxseg;
}