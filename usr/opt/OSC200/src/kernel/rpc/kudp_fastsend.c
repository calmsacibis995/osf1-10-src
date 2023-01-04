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
static char *rcsid = "@(#)$RCSfile: kudp_fastsend.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/12/01 15:38:22 $";
#endif
#ifdef	KERNEL
#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = 	"@(#)kudp_fastsend.c	1.9 90/07/19 4.1NFSSRC Copyr 1990 Sun Micro";
#endif

/* 
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 *  1.16 88/02/08
 */

#include <rpc/rpc.h>
#include <sys/mbuf.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/if_ether.h>

/* #ifdef notdef */
#define  MMINOFF 12 
int use_fastsend=0;
struct in_ifaddr *ifptoia();

/*
 * struct used to reference count a fragmented mbuf.
 */
struct mhold {
	int mh_ref;		/* holders of pointers to mbuf data */
	struct mbuf *mh_mbuf;	/* mbuf that owns the data */
};

static void
fragbuf_free(mh)
	struct mhold *mh;
{
	int s;

	s = splimp();
	if (--mh->mh_ref) {
		(void) splx(s);
		return;
	}
	m_freem(dtom(mh));
	(void) splx(s);
}

static struct mhold *
fragbuf_alloc(m)
	struct mbuf *m;
{
	struct mbuf *newm;
	struct mhold *mh;

	MGET(newm, M_WAIT, MT_DATA);
	newm->m_next = m;
	/* newm->m_data = MMINOFF; CJXXX */
	newm->m_len = 0;
	m->m_next = NULL;
	mh = mtod(newm, struct mhold *);
	mh->mh_ref = 1;
	mh->mh_mbuf = m;
	return (mh);
}

int Sendtries;
int Sendok;
int route_hit, route_missed;
int fastsend_debug=0;

ku_sendto_mbuf(so, am, to)
	struct socket *so;		/* socket data is sent from */
	register struct mbuf *am;	/* data to be sent */
	struct sockaddr_in *to;		/* destination data is sent to */
{
	register int maxlen;		/* max length of fragment */
	register int curlen;		/* data fragment length */
	register int fragoff;		/* data fragment offset */
	register int grablen;		/* number of mbuf bytes to grab */
	register struct ip *ip;		/* ip header */
	register struct mbuf *m;	/* ip header mbuf */
	int error;			/* error number */
	int s;
	struct ifnet *ifp;		/* interface */
	struct sockaddr_in *dst;	/* packet destination */
	struct inpcb *inp;		/* inpcb for binding */
	struct ip *nextip;		/* ip header for next fragment */
	struct route *ro;

	if (!use_fastsend) 
		return(ku_slow_sendto_mbuf(so, am, to));


	if (fastsend_debug)
		printf("fastsend: called with so 0x%08x am 0x%08x to 0x%08x\n", 
		       so, am, to);
	Sendtries++;
	/*
	 * Determine length of data.
	 * This should be passed in as a parameter.
	 */
	curlen = 0;
	for (m = am; m; m = m->m_next) {
		curlen += m->m_len;
	}
	if (fastsend_debug)
		printf("fastsend: mbuf length %d\n", curlen);
	
	/*
	 * Bind local port if necessary.  This will usually put a route in the
	 * pcb, so we just use that one if we have not changed destinations.
	 */
	inp = sotoinpcb(so);
	if (fastsend_debug)
		printf("fastsend: back from sotoinpcb 0x%08x\n", inp);
	if ((inp->inp_lport == 0) &&
	    (inp->inp_laddr.s_addr == INADDR_ANY)) {
		if (fastsend_debug)
			printf("fastsend: calling in_pcbbind\n");

		(void) in_pcbbind(inp, (struct mbuf *) 0);
	}
	if (fastsend_debug)
		printf("fastsend:back from in_pcbbind\n");
	ro = &inp->inp_route;
	dst = (struct sockaddr_in *) &ro->ro_dst;

	if (fastsend_debug)
		printf("fastsend: dst 0x%08x\n", dst);

	if (ro->ro_rt == 0 || (ro->ro_rt->rt_flags & RTF_UP) == 0 ||
	    dst->sin_addr.s_addr != to->sin_addr.s_addr) {
		if (ro->ro_rt)
			rtfree(ro->ro_rt);
		bzero((caddr_t)ro, sizeof (*ro));
		ro->ro_dst.sa_family = AF_INET;
		dst->sin_addr = to->sin_addr;
		rtalloc(ro);
		route_missed++;
		if (ro->ro_rt == 0 || ro->ro_rt->rt_ifp == 0) {
			(void) m_freem(am);
			if (fastsend_debug)
				printf("fastsend: cannot find route %x\n",
				       to->sin_addr.s_addr);
			return (ENETUNREACH);
		}
	} else route_hit++;
	ifp = ro->ro_rt->rt_ifp;
	ro->ro_rt->rt_use++;
	if (fastsend_debug)
		printf("fastsend: got ifp 0x%08x\n",ifp);
	if (ro->ro_rt->rt_flags & RTF_GATEWAY) {
		dst = (struct sockaddr_in *)&ro->ro_rt->rt_gateway;
	}
	/*
	 * Get mbuf for ip, udp headers.
	 */
	MGET(m, M_WAIT, MT_HEADER);
	if (m == NULL) {
		(void) m_freem(am);
		if (fastsend_debug)
			printf("fastsend: cannot find enough mbufs\n");
		return (ENOBUFS);
	}
	/*
	 * XXX on sparc we must pad to make sure that the packet after
	 * the ethernet header is int aligned.  Also does not hurt to
	 * long-word align on other architectures, and might actually help.
	 */
	m->m_data += sizeof (short) + sizeof (struct ether_header); /* CJXXX */
	m->m_len = sizeof (struct ip) + sizeof (struct udphdr);
	ip = mtod(m, struct ip *);
	if (fastsend_debug)
		printf("fastsend: finished reserving space for enet header\n");

	/*
	 * Create pseudo-header and UDP header, and calculate
	 * the UDP level checksum only if desired.
	 */
	{
		extern int	udpcksum;
#define	ui ((struct udpiphdr *)ip)

		ui->ui_pr = IPPROTO_UDP;
		ui->ui_len = htons((u_short) curlen + sizeof (struct udphdr));
		ui->ui_src = ((struct sockaddr_in *)ifptoia(ifp))->sin_addr;
		ui->ui_dst = to->sin_addr;
		ui->ui_sport = inp->inp_lport;
		ui->ui_dport = to->sin_port;
		ui->ui_ulen = ui->ui_len;
		ui->ui_sum = 0;
		if (udpcksum) {
			/* ui->ui_next = ui->ui_prev = 0; benoy */
			ui->ui_x1 = 0;
			m->m_next = am;
			ui->ui_sum = in_cksum(m,
				sizeof (struct udpiphdr) + curlen);
			if (ui->ui_sum == 0)
				ui->ui_sum = 0xFFFF;
		}
# undef ui
		if (fastsend_debug)
			printf("fastsend: done with udp header\n");

	}
	/*
	 * Now that the pseudo-header has been checksummed, we can
	 * fill in the rest of the IP header except for the IP header
	 * checksum, done for each fragment.
	 */
	ip->ip_vhl = (IPVERSION << 4) | ((sizeof(struct ip)) >> 2);
	ip->ip_tos = 0;
	ip->ip_id = htons((unsigned short)ip_id++);
	ip->ip_off = 0;
	ip->ip_ttl = MAXTTL;

	/*
	 * Fragnemt the data into packets big enough for the
	 * interface, prepend the header, and send them off.
	 */
	maxlen = (ifp->if_mtu - sizeof (struct ip)) & ~7;
	curlen = sizeof (struct udphdr);
	fragoff = 0;
	for (;;) {
		register struct mbuf *mm;
		register struct mbuf *lam;	/* last mbuf in chain */
		if (fastsend_debug)
			printf("fastsend: top of frag code\n");

		/*
		 * Assertion: m points to an mbuf containing a mostly filled
		 * in ip header, while am points to a chain which contains
		 * all the data.
		 * The problem here is that there may be too much data.
		 * If there is, we have to fragment the data (and maybe the
		 * mbuf chain).
		 */
		m->m_next = am;
		lam = m;
		while (am->m_len + curlen <= maxlen) {
			curlen += am->m_len;
			lam = am;
			am = am->m_next;
			if (am == 0) {
				ip->ip_off = htons((u_short) (fragoff >> 3));
				goto send;
			}
		}
		if (curlen == maxlen) {
			/*
			 * Incredible luck: last mbuf exactly
			 * filled out the packet.
			 */
			lam->m_next = 0;
		} else {
			/*
			 * Have to fragment the mbuf chain.  am points
			 * to the mbuf that has too much, so we take part
			 * of its data, point mm to it, and attach mm to
			 * the current chain.  lam conveniently points to
			 * the last mbuf of the current chain.
			 *
			 * We will end up with many type 2 mbufs pointing
			 * into the data for am. We insure that the data
			 * doesn't get freed until all of these mbufs are
			 * freed by replacing am with a new type2 mbuf
			 * which keeps a ref count and frees am when the
			 * count goes to zero.
			 */
			MGET(mm, M_WAIT, MT_DATA);
			if (fastsend_debug)
				printf("fastsend: got another mbuf\n");

			if (m->m_ext.ext_free != fragbuf_free) {
				/*
				 * clone the mbuf and save a pointer
				 * to it and a ref count.
				 */
				mm->m_data = am->m_data; /* CJXXX */
				mm->m_len = am->m_len;
				mm->m_next = am->m_next;
				mm->m_flags |= M_EXT;
				mm->m_ext.ext_free = fragbuf_free;
				mm->m_ext.ext_arg  = (caddr_t)fragbuf_alloc(am);
				mm->m_ext.ext_ref.forw = mm->m_ext.ext_ref.back = &mm->m_ext.ext_ref;
				am = mm;
				MGET(mm, M_WAIT, MT_DATA);
			}
			grablen = maxlen - curlen;
			mm->m_next = NULL;
			mm->m_data = am->m_data;
			mm->m_len = grablen;
			mm->m_flags |= M_EXT;
			mm->m_ext.ext_free = fragbuf_free;
			mm->m_ext.ext_arg  = am->m_ext.ext_arg;
			mm->m_ext.ext_ref.forw = mm->m_ext.ext_ref.back = &mm->m_ext.ext_ref;
			s = splimp();
			((struct mhold *)mm->m_ext.ext_arg)->mh_ref++;
			(void) splx(s);

			lam->m_next = mm;
			am->m_len -= grablen;
			am->m_data += grablen;
			curlen = maxlen;
		}
		if (fastsend_debug)
			printf("fastsend: done fragging\n");
		/*
		 * Assertion: m points to an mbuf chain of data which
		 * can be sent, while am points to a chain containing
		 * all the data that is to be sent in later fragments.
		 */
		ip->ip_off = htons((u_short) ((fragoff >> 3) | IP_MF));
		/*
		 * There are more frags, so we save
		 * a copy of the ip hdr for the next
		 * frag.
		 */
		MGET(mm, M_WAIT, MT_HEADER);
		if (mm == 0) {
			(void) m_free(m);	/* this frag */
			(void) m_freem(am);	/* rest of data */
			if (fastsend_debug)
				printf("fastsend: cannot find enough mbufs(pt2)\n");
			return (ENOBUFS);
		}
		/* 
		 * XXX on sparc we must pad to make sure that the packet after
		 * the ethernet header is int aligned.
		 */
		mm->m_data +=
			sizeof (short) + sizeof (struct ether_header);
		mm->m_len = sizeof (struct ip);
		nextip = mtod(mm, struct ip *);
		if (fastsend_debug)
			printf("fastsend: calling bcopy\n");
		bcopy((caddr_t) ip, (caddr_t) nextip, sizeof (struct ip));
send:
		{
			register int sum;
			/*
			 * Set ip_len and calculate the ip header checksum.
			 * Note ips[5] is skipped below since it IS where the
			 * checksum will eventually go.
			 */
			ip->ip_len = htons(sizeof (struct ip) + curlen);
#define	ips ((u_short *) ip)
			sum = ips[0] + ips[1] + ips[2] + ips[3] + ips[4]
			    + ips[6] + ips[7] + ips[8] + ips[9];
			sum = (sum & 0xFFFF) + (sum >> 16);
			ip->ip_sum = ~((sum & 0xFFFF) + (sum >> 16));
		}
#undef ips
		/*
		 * Send it off to the newtork.
		 */
		if (fastsend_debug)
			printf("fastsend: calling driver - ifp 0x%08x m 0x%08x dst 0x%08x route 0x%08x\n",
			       ifp, m, dst, ro->ro_rt);
		if (error = (*ifp->if_output)(ifp, m, dst, ro->ro_rt)) {
			if (am) {
				(void) m_freem(am);	/* rest of data */
				(void) m_free(mm);	/* hdr for next frag */
			}
			if (fastsend_debug)
				printf("fastsend: driver returns error: %d\n", error);
			return (error);
		}
		if (am == 0) {			/* All fragments sent OK */
			if (fastsend_debug)
				printf("fastsend: all frags out\n");
			Sendok++;
			return (0);
		}
		ip = nextip;
		m = mm;
		fragoff += curlen;
		curlen = 0;
	}
	/*NOTREACHED*/
}
/* #endif notdef */
#ifdef DEBUG
pr_mbuf(p, m)
	char *p;
	struct mbuf *m;
{
	register char *cp, *cp2;
	register struct ip *ip;
	register int len;

	len = 28;
	printf("%s: ", p);
	if (m && m->m_len >= 20) {
		ip = mtod(m, struct ip *);
		printf("hl %d v %d tos %d len %d id %d mf %d off ",
			ip->ip_hl, ip->ip_v, ip->ip_tos, ip->ip_len,
			ip->ip_id, ip->ip_off >> 13, ip->ip_off & 0x1fff);
		printf("%d ttl %d p %d sum %d src %x dst %x\n",
			ip->ip_ttl, ip->ip_p, ip->ip_sum, ip->ip_src.s_addr,
			ip->ip_dst.s_addr);
		len = 0;
		printf("m %x addr %x len %d\n", m, mtod(m, caddr_t), m->m_len);
		m = m->m_next;
	} else if (m) {
		printf("pr_mbuf: m_len %d\n", m->m_len);
	} else {
		printf("pr_mbuf: zero m\n");
	}
	while (m) {
		printf("m %x addr %x len %d\n", m, mtod(m, caddr_t), m->m_len);
		cp = mtod(m, caddr_t);
		cp2 = cp + m->m_len;
		while (cp < cp2) {
			if (len-- < 0) {
				break;
			}
			printf("%x ", *cp & 0xFF);
			cp++;
		}
		m = m->m_next;
		printf("\n");
	}
}
#endif DEBUG
#endif	/* KERNEL */
