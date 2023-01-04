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
static char	*sccsid = "@(#)$RCSfile: if_ether.c,v $ $Revision: 4.3.17.9 $ (DEC) $Date: 1993/09/30 19:18:24 $";
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
 *	Base:	if_ether.c	7.10 (Berkeley) 4/22/89
 *	Merged:	if_ether.c	7.12 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patches.
 *
 */

/*
 * Ethernet address resolution protocol.
 * TODO:
 *	link entries onto hash chains, keep free list
 *	add "inuse/lock" bit (or ref. count) along with valid bit
 */

#ifdef __alpha
#include "machine/endian.h"
#endif

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/kernel.h"
#include "sys/errno.h"
#include "sys/ioctl.h"
#include "sys/syslog.h"

#include "sys/mbuf.h"
#include "sys/socket.h"

#include "net/if.h"
#include "net/route.h"
#include "net/netisr.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/in_var.h"
#include "netinet/if_ether.h"

LOCK_ASSERTL_DECL

struct	arptab *arptab;
int	arptab_size;				/* count of arptab structs */
int	arptab_bsiz = 16, arptab_nb = 37;	/* tabs per bucket, # buckets */
static	char anypublished;

static	struct arptab arptabXXX[37*16];	/* To avoid malloc, for now */

struct	ifqueue arpintrq;
int	arpqmaxlen = IFQ_MAXLEN;
int	useloopback = 1;	/* use loopback interface for local traffic */


#if	NETSYNC_LOCK
lock_data_t	global_arp_lock;
#define ARP_LOCKINIT()	lock_init2(&global_arp_lock, TRUE, LTYPE_ARP)
#define ARP_LOCK()	{ NETSPL(s,net); lock_write(&global_arp_lock); }
#define ARP_UNLOCK()	{ lock_done(&global_arp_lock); NETSPLX(s); }
#else
#define ARP_LOCKINIT()
#define ARP_LOCK()	NETSPL(s,net)
#define ARP_UNLOCK()	NETSPLX(s)
#endif

/*
 * ARP trailer negotiation.  Trailer protocol is not IP specific,
 * but ARP request/response use IP addresses.
 */
#define ETHERTYPE_IPTRAILERS ETHERTYPE_TRAIL

#define	ARPTAB_HASH(a) \
	((u_int)(a) % arptab_nb)

#define	ARPTAB_LOOK(at,addr,ifp) { \
	register short n; \
	at = &arptab[ARPTAB_HASH(addr) * arptab_bsiz]; \
	for (n = 0 ; n < arptab_bsiz; n++,at++) \
		if (at->at_if == (ifp) && at->at_iaddr.s_addr == (addr)) \
			break; \
	if (n >= arptab_bsiz) \
		at = 0; \
}
void haddr_convert();
/*
 * ARP (RFC826) protocol.
 *
 * Enhancements over traditional BSD:
 *	Single shared table for all if's
 *	Hardware (interface) independent
 *	Stale detection with unicast refresh
 *	Anti-flooding of resolves with backoff
 *	Early freeing of unresolved mbufs
 *
 * Primary limitation: per-interface output routine required (arpoutput()).
 */

/* timer values */
#define	ARPT_AGE	1	/* aging timer, 1 sec. */

/* Settable values (in SECONDS) used by arptimer() */
int	arpkillc   = 20*60;	/* kill completed entry in 20 mins. */
int	arpkilli   = 3*60;	/* kill incomplete entry in 3 minutes */
int	arprefresh = 2*60;	/* time to refresh entry */
int	arphold	   = 5;		/* time to keep at_hold packet */
/* Settable counts used by arpresolve() */
int	arplost	   = 3;		/* retry threshold for broadcast */
int	arpdead	   = 6;		/* retry threshold for backoff */

extern struct ifnet loif;

arphasmbuf(m)
struct mbuf *m;
{
        register struct arptab *at;
        register int i;
	NETSPL_DECL(s)

        ARP_LOCK();
        at = &arptab[0];
        for (i = 0; i < arptab_size; i++, at++) {
		if (at->at_flags & ATF_PERM)
			continue;
		if (at->at_hold == m || (at->at_hold && at->at_hold->m_next == m)) {
			if ((at->at_flags & ATF_COM) == 0)
				arptfree(at);
			break;
		}
	}
        ARP_UNLOCK();
}

/*
 * Timeout routine.  Update arp_tab entries once a second.
 */
arptimer()
{
	register struct arptab *at;
	register i;
	NETSPL_DECL(s)

	ARP_LOCK();
	at = &arptab[0];
	for (i = 0; i < arptab_size; i++, at++) {
		if (at->at_flags == 0)
			continue;
		/* release held packet after hold time */
		if (++at->at_timer >= arphold && at->at_hold) {
			m_freem(at->at_hold);
			at->at_hold = 0;
		}
		/* no further action if perm */
		if (at->at_flags & ATF_PERM)
			continue;
		/* set stale bit if need refresh */
		if (++at->at_valid >= arprefresh)
			at->at_flags |= ATF_STALE;
		/* check expiration time */
		if (at->at_timer < ((at->at_flags&ATF_COM)?arpkillc:arpkilli))
			continue;
		/* timer has expired, clear entry */
		arptfree(at);
	}
	ARP_UNLOCK();
#if	!NETISR_THREAD
	timeout(arptimer, (caddr_t)0, ARPT_AGE * hz);
#else
	return (ARPT_AGE * hz);
#endif
}

/*
 * Send an ARP packet, asking who has addr on interface ac.
 */

static void arpdorequest();

static void
arprequest(ac, addr, dest)
        register struct arpcom *ac;
        struct in_addr *addr;
        u_char *dest;           /* Dest hwaddr - bcast/etc */
{
        struct ifaddr *ifa;

        /* Use the first found, compatible with previous behavior */
        for (ifa = ac->ac_if.if_addrlist; ifa; ifa = ifa->ifa_next)
                if (ifa->ifa_addr->sa_family == AF_INET)
                        break;
        if (ifa == 0)
                return;
        arpdorequest(ac,addr,dest,
            (struct in_addr *)&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr);

}


/* This routine gets called when an interfaces physical address gets changed.
 * It causes an ARP request to be sent for each AF_INET address configured
 * on the interface.  This will cause other hosts to update their ARP
 * caches with the new physical address.
 */
static void
arprequestall(ac, addr, dest)
        register struct arpcom *ac;
        struct in_addr *addr;
        u_char *dest;           /* Dest hwaddr - bcast/etc */
{
        struct ifaddr *ifa;

        /* send out an ARP packet identifying each AF_INET address on
         * the interface.
         */
        for (ifa = ac->ac_if.if_addrlist; ifa; ifa = ifa->ifa_next)
                if (ifa->ifa_addr->sa_family == AF_INET)
                    arpdorequest(ac,
                        (struct in_addr *)&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr,
                        dest,
                        (struct in_addr *)&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr);

}

/* This routine is called when an alias address is configured.  It sends out
 * and ARP request identifying the new IP address/physical address pair.
 * This differs slightly from the normal arpwhohas in that both the source
 * and reply protocol address will be that of the alias (instead of the
 * reply address being the primary).
 */
static void
arpaliasrequest(ac, addr, dest)
        register struct arpcom *ac;
        struct in_addr *addr;
        u_char *dest;           /* Dest hwaddr - bcast/etc */
{
        arpdorequest(ac,addr,dest,addr);
}

static void
arpdorequest(ac, addr, dest,spa)
	register struct arpcom *ac;
	struct in_addr *addr;
	u_char *dest;		/* Dest hwaddr - bcast/etc */
	struct in_addr *spa;    /* source protocol address to use */
{
	register struct mbuf *m;
	register struct arphdr *ah;
	int hln, pln;

	hln = ac->ac_if.if_addrlen;
	pln = sizeof (*addr);
if (hln <= 0 || sizeof (*ah) + (2*hln) + (2*pln) > MHLEN) return;
	if ((m = m_gethdr(M_DONTWAIT, MT_DATA)) == NULL)
		return;
	m->m_pkthdr.len = m->m_len = sizeof(*ah) + (2*hln) + (2*pln);
	m->m_pkthdr.rcvif = 0;
	MH_ALIGN(m, m->m_len);
	ah = mtod(m, struct arphdr *);
	ah->ar_pro = htons(ETHERTYPE_IP);
	ah->ar_hln = hln;	/* hardware address length */
	ah->ar_pln = pln;	/* protocol address length */
	ah->ar_op = htons(ARPOP_REQUEST);
	bcopy((caddr_t)ac->ac_hwaddr, (caddr_t)AR_SHA(ah), hln);
	bcopy((caddr_t)spa, (caddr_t)AR_SPA(ah), pln);
	bzero((caddr_t)AR_THA(ah), hln);
	bcopy((caddr_t)addr, (caddr_t)AR_TPA(ah), pln);
	if (ac->ac_arphrd == ARPHRD_802 && ac->ac_if.if_type == IFT_ISO88025) {
		/*
		 * Convert the source address to the non-canonical form
		 * for Token Ring interfaces.
		 * All addresses stored in the system are in the
		 * canonical form.
		 */
		haddr_convert((caddr_t)AR_SHA(ah));
	}
	arpoutput(ac, m, dest, ac->ac_arphrd);
}

/*
 * Resolve an IP address into a hardware address.  If success, 
 * desten is filled in.  If there is no entry in arptab,
 * set one up and broadcast a request for the IP address.
 * Hold onto this mbuf and resend it once the address
 * is finally resolved.  A return value of 1 indicates
 * that desten has been filled in and the packet should be sent
 * normally; a 0 return indicates that the packet has been
 * taken over here, either now or for later transmission.
 *
 * We now raise interrupts only to splnet, since arpinput no
 * longer runs as a packet interrupt routine.
 */
static
arpresolve_local(ac, m, destip, desten, usetrailers)
	register struct arpcom *ac;
	struct mbuf *m;
	register struct in_addr *destip;
	register u_char *desten;
	int *usetrailers;
{
	register struct arptab *at;
	struct sockaddr_in sin;
	register struct in_ifaddr *ia;
	int retval = 0;
	NETSPL_DECL(s)

/* XXX Compat for non-ported drivers XXX check IFT_ETHER */
if (ac->ac_if.if_addrlen == 0)
	ac->ac_if.if_addrlen = 6;
if (ac->ac_if.if_addrlen == 6 && !ac->ac_bcastaddr)
	ac->ac_bcastaddr = etherbroadcastaddr;
if (ac->ac_if.if_addrlen == 6 && ac->ac_arphrd == 0)
	ac->ac_arphrd = ARPHRD_ETHER;

	*usetrailers = 0;
	if (m->m_flags & M_BCAST) {	/* broadcast */
		if (!ac->ac_bcastaddr) {
			m_freem(m);
			return (0);
		}
		bcopy(ac->ac_bcastaddr, (caddr_t)desten,
		    (int)ac->ac_if.if_addrlen);
		return (1);
	}

	if (m->m_flags & M_MCAST) {
		ETHER_MAP_IP_MULTICAST(destip, desten);
		return(1);
	}

	/* if for us, use software loopback driver if up */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
	    if ((ia->ia_ifp == &ac->ac_if) &&
		(destip->s_addr == ia->ia_addr.sin_addr.s_addr)) {
		/*
		 * This test used to be
		 *	if (loif.if_flags & IFF_UP)
		 * It allowed local traffic to be forced
		 * through the hardware by configuring the loopback down.
		 * However, it causes problems during network configuration
		 * for boards that can't receive packets they send.
		 * It is now necessary to clear "useloopback"
		 * to force traffic out to the hardware.
		 */
		if (useloopback) {
			sin.sin_family = AF_INET;
			sin.sin_len = sizeof(sin);
			sin.sin_addr = *destip;
			(void) looutput(&loif, m, (struct sockaddr *)&sin,						(struct rtentry *)0);
			/*
			 * The packet has already been sent and freed.
			 */
			return (0);
		} else {
			bcopy((caddr_t)ac->ac_hwaddr, (caddr_t)desten,
			    (int)ac->ac_if.if_addrlen);
			return (1);
		}
	}
	ARP_LOCK();
	ARPTAB_LOOK(at, destip->s_addr, &ac->ac_if);
	if (at == 0) {			/* not found */
		if (ac->ac_if.if_flags & IFF_NOARP) {
			u_int lna; int len;
			ARP_UNLOCK();
			lna = in_lnaof(*destip);
			len = ac->ac_if.if_addrlen;
			bcopy((caddr_t)ac->ac_hwaddr, (caddr_t)desten, len);
			desten[--len] = lna & 0xff;
			if (len > 0) desten[--len] = (lna >> 8) & 0xff;
			if (len > 0) desten[--len] = (lna >> 16) & 0x7f;
			return (1);
		}
		at = arptnew(destip, &ac->ac_if);
		if (at == 0)
			panic("arpresolve: no free entry");
		at->at_hold = m;
		ARP_UNLOCK();
		arprequest(ac, destip, ac->ac_bcastaddr);
		return (0);
	}
	at->at_timer = 0;		/* restart the timer */
	if (at->at_flags & ATF_COM) {	/* entry IS complete */
		bcopy((caddr_t)at->at_hwaddr, (caddr_t)desten,
		    (int)at->at_if->if_addrlen);
		if (at->at_flags & ATF_USETRAILERS)
			*usetrailers = 1;
		retval = 1;
	} else {
		/*
		 * There is an arptab entry, but no hardware address
		 * response yet.  Replace the held mbuf with this
		 * latest one.
		 */
		if (at->at_hold)
			m_freem(at->at_hold);
		at->at_hold = m;
	}
	/* Prevent ARP flooding of unresolved entries. ARP depends on
	 * transport level sends to retry, but backs off independently */
	if ((!retval || (at->at_flags & ATF_STALE)) &&
	    at->at_sent <= at->at_valid) {
		u_char hwaddr[sizeof at->at_hwaddr];
		if (!(at->at_flags & ATF_DEAD) && ++at->at_retry >= arpdead)
			at->at_flags |= ATF_DEAD;	/* no further backoff */
		/* Backoff 2 sec, 4 sec, 8 sec ... (64 sec default) */
		at->at_sent = at->at_valid + (1 << at->at_retry);
		/* Unicast if known and < 3 tries, else broadcast */
		/* Copy out known address so can unlock for send */
		if (retval && at->at_retry < arplost)
		    bcopy(at->at_hwaddr, hwaddr, (int)ac->ac_if.if_addrlen);
		else if (ac->ac_bcastaddr)
		    bcopy(ac->ac_bcastaddr, hwaddr, (int)ac->ac_if.if_addrlen);
		else
		    goto out;
		/* Time to refresh the modify bits, too */
		at->at_flags &= ~(ATF_USETRAILERS|ATF_USE802);
		ARP_UNLOCK();
		arprequest(ac, destip, hwaddr);
		return retval;
	}
out:
	ARP_UNLOCK();
	return retval;
}

/* Used in logmsg below */
char *
arp_sprintf(a, hwaddr, len)
	register char *a;
	register u_char *hwaddr;
	register int len;
{
	register int i, j;
	CONST static char tohex[] = "0123456789ABCDEF";

	for (i = j = 0; i < len; ++i) {
		a[j++] = tohex[hwaddr[i] >> 4];
		a[j++] = tohex[hwaddr[i] & 0x0f];
		a[j++] = ':';
	}
	if (i == 0) {
		a[0] = '?'; a[1] = '?'; a[2] = '?';
		j = 4;
	}
	a[j-1] = '\0';
	return a;
}

/*
 * Called as softnet_intr routine when ARP packets received.
 * Common length and type checks are done here,
 * then the protocol-specific routine is called.
 */
void
arpintr()
{
	register struct mbuf *m;
	int s;

	for (;;) {
		s = splimp();
		IF_DEQUEUE(&arpintrq, m);
		splx(s);
		if (m == 0)
			return;
if ((m->m_flags & M_PKTHDR) == 0)
panic("arpintr no HDR");
		arpinput((struct arpcom *)m->m_pkthdr.rcvif, m);
	}
}

void
arpinput(ac, m)
	struct arpcom *ac;
	struct mbuf *m;
{
	register struct arphdr *ar;

	if (ac->ac_if.if_flags & IFF_NOARP)
		goto out;
	if (m->m_len < sizeof(struct arphdr))
		goto out;
	ar = mtod(m, struct arphdr *);
	if (m->m_len < sizeof(struct arphdr) + 2 * ar->ar_hln + 2 * ar->ar_pln)
		goto out;

	switch (ntohs(ar->ar_pro)) {

	case ETHERTYPE_IPTRAILERS:
		/* Ignore trailers in BOTH directions if disabled */
		if (ac->ac_if.if_flags & IFF_NOTRAILERS)
			break;
		/* FALL THROUGH */
	case ETHERTYPE_IP:
		in_arpinput(ac, m);
		return;

	default:
		break;
	}
out:
	m_freem(m);
}

/*
 * ARP for Internet protocols on (e.g. 10 Mb/s Ethernet).
 * Algorithm is that given in RFC 826.
 * In addition, a sanity check is performed on the sender
 * protocol address, to catch impersonators.
 * We also handle negotiations for use of trailer protocol:
 * ARP replies for protocol type ETHERTYPE_TRAIL are sent
 * along with IP replies if we want trailers sent to us,
 * and also send them in response to IP replies.
 * This allows either end to announce the desire to receive
 * trailer packets.
 * We reply to requests for ETHERTYPE_TRAIL protocol as well,
 * but don't normally send requests.
 */
void
in_arpinput(ac, m)
	register struct arpcom *ac;
	struct mbuf *m;
{
	register struct arphdr *ah;
	register struct arptab *at;
	register struct in_ifaddr *ia;
	struct in_ifaddr *maybe_ia = 0;
	struct mbuf *mcopy = 0;
	struct in_addr isaddr, itaddr, myaddr;
	int proto, op, completed = 0;
	u_char dest[16];

	NETSPL_DECL(s)

	ah = mtod(m, struct arphdr *);
	if (ah->ar_hln != ac->ac_if.if_addrlen ||
	    ah->ar_pln != sizeof (itaddr) ||
	    (ntohs(ah->ar_hrd) != ARPHRD_ETHER && 
			ntohs(ah->ar_hrd) != ARPHRD_802 )) 
		goto out;
	/*
	 * If this is a 802 format packet from a token ring interface convert to
	 * canonical format for internal data manupilations.
	 */
	if (ntohs(ah->ar_hrd) == ARPHRD_802 && 
		ac->ac_if.if_type == IFT_ISO88025) {
	     haddr_convert((caddr_t)AR_SHA(ah));
	     haddr_convert((caddr_t)AR_THA(ah));
	}
	proto = ntohs(ah->ar_pro);
	op = ntohs(ah->ar_op);
	bcopy((caddr_t)AR_SPA(ah), (caddr_t)&isaddr, sizeof (isaddr));
	bcopy((caddr_t)AR_TPA(ah), (caddr_t)&itaddr, sizeof (itaddr));

	/* broadcast src, drop it */ 
	if (isaddr.s_addr == INADDR_BROADCAST ) {
#ifdef FUTURE
		char a[2 * sizeof ac->ac_hwaddr + sizeof ac->ac_hwaddr + 1];
		char *badipaddr = (char *)inet_ntoa(isaddr.s_addr); 
		log(LOG_ERR,
		    "arp: illegal IP address %s is used by hardware address %s!\n",
			badipaddr, arp_sprintf(a, AR_SHA(ah), (int)ah->ar_hln));
#endif
		goto out; 
	} 
			
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_ifp == &ac->ac_if) {
			maybe_ia = ia;
			if ((itaddr.s_addr == ia->ia_addr.sin_addr.s_addr) ||
			     (isaddr.s_addr == ia->ia_addr.sin_addr.s_addr))
				break;
		}
	if (maybe_ia == 0)
		goto out;
	myaddr = ia ? ia->ia_addr.sin_addr : maybe_ia->ia_addr.sin_addr;
	if (!bcmp((caddr_t)AR_SHA(ah), (caddr_t)ac->ac_hwaddr, (int)ah->ar_hln))
		goto out;	/* it's from me, or published, ignore it. */
	/* Reject reply and log message if hwaddr == broadcast */
	if (ac->ac_bcastaddr &&
	    !bcmp((caddr_t)AR_SHA(ah), ac->ac_bcastaddr, (int)ah->ar_hln)) {
		char *badipaddr = (char *)inet_ntoa(isaddr.s_addr); 
		log(LOG_ERR,
		    "arp: hardware address is broadcast for IP address %s!\n",
			badipaddr);
		goto out;
	}
	/* Log a message if someone else reponds with our IP address */
	if (isaddr.s_addr == myaddr.s_addr) {
		char a[2 * sizeof ac->ac_hwaddr + sizeof ac->ac_hwaddr + 1];
		char *badipaddr = (char *)inet_ntoa(isaddr.s_addr); 
		log(LOG_ERR,
		   "arp: local IP address %s in use by hardware address %s\n",
			badipaddr, arp_sprintf(a, AR_SHA(ah), (int)ah->ar_hln));
		itaddr = myaddr;
		if (op == ARPOP_REQUEST)
			goto reply;
		goto out;
	}
	ARP_LOCK();
	ARPTAB_LOOK(at, isaddr.s_addr, &ac->ac_if);
	if (at) {
		bcopy((caddr_t)AR_SHA(ah), (caddr_t)at->at_hwaddr,
		    (int)ah->ar_hln);
		if ((at->at_flags & ATF_COM) == 0)
			completed = 1;
		at->at_flags |= ATF_COM;
		at->at_flags &= ~(ATF_STALE|ATF_DEAD);
		at->at_retry = at->at_valid = at->at_sent = 0;
		if (at->at_hold) {
			struct sockaddr_in sin;
			struct mbuf *om = at->at_hold;
			at->at_hold = 0;
			ARP_UNLOCK();
			sin.sin_family = AF_INET;
			sin.sin_len = sizeof(sin);
			sin.sin_addr = isaddr;
			(*ac->ac_if.if_output)(&ac->ac_if, 
			    om, (struct sockaddr *)&sin, (struct rtentry *)0);
			goto reply;
		}
	} else if (itaddr.s_addr == myaddr.s_addr) {
		/* ensure we have a table entry */
		if (at = arptnew(&isaddr, &ac->ac_if)) {
			bcopy((caddr_t)AR_SHA(ah), (caddr_t)at->at_hwaddr,
			    (int)ah->ar_hln);
			completed = 1;
			at->at_flags |= ATF_COM;
		}
	}
	ARP_UNLOCK();
reply:
	switch (proto) {

	case ETHERTYPE_IPTRAILERS:
		/* partner says trailers are OK */
		if (at) {
			ARP_LOCK();
			/* Must recheck in case table changed */
			if (at->at_if == &ac->ac_if &&
			    at->at_iaddr.s_addr == isaddr.s_addr)
				at->at_flags |= ATF_USETRAILERS;
			ARP_UNLOCK();
		}
		/*
		 * Reply to request iff we want trailers.
		 */
		if (op != ARPOP_REQUEST || ac->ac_if.if_flags & IFF_NOTRAILERS)
			goto out;
		break;

	case ETHERTYPE_IP:
		/*
		 * Reply if this is an IP request,
		 * or if we want to send a trailer response.
		 * Send the latter only to the IP response
		 * that completes the current ARP entry.
		 */
		if (op != ARPOP_REQUEST &&
		    (completed == 0 || ac->ac_if.if_flags & IFF_NOTRAILERS))
			goto out;
	}
	if (itaddr.s_addr == myaddr.s_addr) {
		/* I am the target */
		bcopy((caddr_t)AR_SHA(ah), (caddr_t)AR_THA(ah),
		    (int)ah->ar_hln);
		bcopy((caddr_t)ac->ac_hwaddr, (caddr_t)AR_SHA(ah),
		    (int)ah->ar_hln);
	} else if (anypublished) {
		ARP_LOCK();
		ARPTAB_LOOK(at, itaddr.s_addr, &ac->ac_if);
		if (at == NULL || (at->at_flags & ATF_PUBL) == 0) {
			ARP_UNLOCK();
			goto out;
		}
		bcopy((caddr_t)AR_SHA(ah), (caddr_t)AR_THA(ah),
		    (int)ah->ar_hln);
		bcopy((caddr_t)at->at_hwaddr, (caddr_t)AR_SHA(ah),
		    (int)ah->ar_hln);
		ARP_UNLOCK();
	} else
		goto out;

	m->m_pkthdr.len = m->m_len =
	    sizeof(*ah) + (2*(int)ah->ar_hln) + sizeof(isaddr) + sizeof(itaddr);
	bcopy((caddr_t)AR_SPA(ah), (caddr_t)AR_TPA(ah), sizeof(isaddr));
	bcopy((caddr_t)&itaddr, (caddr_t)AR_SPA(ah), sizeof(itaddr));
	ah->ar_op = htons(ARPOP_REPLY); 
	/*
	 * If this is a 802 format packet convert the arp data from
	 * canonical format to non-canonical before sending the packet out 
         * on the wire. This is done only if the interface is 802.5.
	 * The destination address passed in the mac header
	 * should be in the canonical form. Save it first.
	 */
	bcopy((caddr_t)AR_THA(ah), dest, (int)ah->ar_hln);

	if (ntohs(ah->ar_hrd) == ARPHRD_802 && 
		ac->ac_if.if_type == IFT_ISO88025) {
	     haddr_convert((caddr_t)AR_SHA(ah));
	     haddr_convert((caddr_t)AR_THA(ah));
	}
	/*
	 * If incoming packet was an IP reply,
	 * we are sending a reply for type IPTRAILERS.
	 * If we are sending a reply for type IP
	 * and we want to receive trailers,
	 * send a trailer reply as well.
	 */
	if (op == ARPOP_REPLY)
		ah->ar_pro = htons(ETHERTYPE_IPTRAILERS);
	else if (proto == ETHERTYPE_IP &&
	    (ac->ac_if.if_flags & IFF_NOTRAILERS) == 0)
		mcopy = m_copym(m, 0, (int)M_COPYALL, M_DONTWAIT);
	arpoutput(ac, m, dest, ntohs(ah->ar_hrd));
	if (mcopy) {
		ah = mtod(mcopy, struct arphdr *);
		ah->ar_pro = htons(ETHERTYPE_IPTRAILERS);
		arpoutput(ac, mcopy, dest, ntohs(ah->ar_hrd));
	}
	return;

out:
	m_freem(m);
	return;
}

/*
 * ARP output. This code should be per-interface.
 */
void
arpoutput(ac, m, dest, arphrd)
	struct arpcom *ac;
	struct mbuf *m;
	u_char *dest;
	u_short arphrd;
{
	struct sockaddr sa;

	switch (arphrd) {
case 0:				/*XXX*/
arphrd = ARPHRD_ETHER;
	case ARPHRD_802:	/*XXX - needs per-host semantic*/
	case ARPHRD_ETHER: {
		struct ether_header *eh = (struct ether_header *)sa.sa_data;
		sa.sa_family = AF_UNSPEC;
		sa.sa_len = sizeof(sa);
if (dest == NULL) dest = etherbroadcastaddr; /* XXX */
		bzero((caddr_t)eh->ether_shost, sizeof (eh->ether_shost));
		bcopy(dest, (caddr_t)eh->ether_dhost, sizeof(eh->ether_dhost));
		eh->ether_type = ETHERTYPE_ARP;	/* ether_output will swap */
		break;
	}
	default:
		printf("arpoutput: can't handle type %d\n", (int)arphrd);
		m_freem(m);
		return;
	}
	mtod(m, struct arphdr *)->ar_hrd = htons(arphrd);
	(*ac->ac_if.if_output)(&ac->ac_if, m, &sa, (struct rtentry *)0);
}

/*
 * Free an arptab entry.
 * Must be called at splnet and/or ARP_LOCK held.
 */
void
arptfree(at)
	register struct arptab *at;
{
	if (at->at_hold)
		m_freem(at->at_hold);
	bzero((caddr_t)at, sizeof *at);
}

/*
 * Enter a new address in arptab, pushing out the oldest entry 
 * from the bucket if there is no room.
 * This always succeeds since no bucket can be completely filled
 * with permanent entries (except from arpioctl when testing whether
 * another permanent entry will fit).
 * MUST BE CALLED AT SPLNET (and/or) WITH ARP_LOCK HELD.
 */
struct arptab *
arptnew(addr, ifp)
	struct in_addr *addr;
	struct ifnet *ifp;
{
	register n;
	int oldest = -1;
	register struct arptab *at, *ato = NULL;

	at = &arptab[ARPTAB_HASH(addr->s_addr) * arptab_bsiz];
	for (n = 0; n < arptab_bsiz; n++,at++) {
		if (at->at_flags == 0)
			goto out;	 /* found an empty entry */
		if (at->at_flags & ATF_PERM)
			continue;
		if ((int) at->at_timer > oldest) {
			oldest = at->at_timer;
			ato = at;
		}
	}
	if (ato == NULL)
		return (NULL);
	at = ato;
	arptfree(at);
out:
	at->at_if = ifp;
	at->at_iaddr = *addr;
	at->at_flags = ATF_INUSE;
	return (at);
}

static int
arpioctl_local(cmd, data)
	unsigned int cmd;
	caddr_t data;
{
	register struct arpreq *ar = (struct arpreq *)data;
	register struct arptab *at;
	register struct sockaddr_in *sin;
	struct ifaddr *ifa;
	NETSPL_DECL(s)

	sin = (struct sockaddr_in *)&ar->arp_ha;
#if defined(COMPAT_43) && BYTE_ORDER != BIG_ENDIAN
	if (sin->sin_family == 0 && sin->sin_len < 16)
		sin->sin_family = sin->sin_len;
#endif
	sin->sin_len = sizeof(ar->arp_ha);
	sin = (struct sockaddr_in *)&ar->arp_pa;
#if defined(COMPAT_43) && BYTE_ORDER != BIG_ENDIAN
	if (sin->sin_family == 0 && sin->sin_len < 16)
		sin->sin_family = sin->sin_len;
#endif
	sin->sin_len = sizeof(ar->arp_pa);
	if (ar->arp_pa.sa_family != AF_INET ||
	    ar->arp_ha.sa_family != AF_UNSPEC)
		return (EAFNOSUPPORT);
	ARP_LOCK();
	/* Tricky - don't know ifp, so match any - XXX use ifa_withnet? */
	ARPTAB_LOOK(at, sin->sin_addr.s_addr, at->at_if);
	if (at == NULL) {		/* not found */
		if (cmd != SIOCSARP) {
			ARP_UNLOCK();
			return (ENXIO);
		}
		if ((ifa = ifa_ifwithnet(&ar->arp_pa)) == NULL) {
			ARP_UNLOCK();
			return (ENETUNREACH);
		}
	}
	switch (cmd) {

	case SIOCSARP:		/* set entry */
		if (at == NULL) {
			at = arptnew(&sin->sin_addr, ifa->ifa_ifp);
			if (at == NULL) {
				ARP_UNLOCK();
				return (EADDRNOTAVAIL);
			}
			if (ar->arp_flags & ATF_PERM) {
			/* never make all entries in a bucket permanent */
				register struct arptab *tat;
				
				/* try to re-allocate */
				tat = arptnew(&sin->sin_addr, ifa->ifa_ifp);
				if (tat == NULL) {
					arptfree(at);
					ARP_UNLOCK();
					return (EADDRNOTAVAIL);
				}
				arptfree(tat);
			}
		}
		bcopy((caddr_t)ar->arp_ha.sa_data, (caddr_t)at->at_hwaddr,
		    sizeof (at->at_hwaddr));
		at->at_flags = ATF_COM | ATF_INUSE |
			(ar->arp_flags & ~ATF_CANTCHANGE);
		if (at->at_flags & ATF_PUBL)
			anypublished = 1;
		break;

	case SIOCDARP:		/* delete entry */
		arptfree(at);
		break;

	case SIOCGARP:		/* get entry */
#ifdef	OSIOCGARP
	case OSIOCGARP:
#endif
		bcopy((caddr_t)at->at_hwaddr, (caddr_t)ar->arp_ha.sa_data,
		    sizeof (ar->arp_ha.sa_data));
#if	defined(COMPAT_43) && defined(OSIOCGARP)
		if (cmd == OSIOCGARP)
			((struct osockaddr *)&ar->arp_ha)->sa_family =
							ar->arp_ha.sa_family;
#endif
		ar->arp_flags = at->at_flags;
		break;
	}
	ARP_UNLOCK();
	return (0);
}

void
arpinit()
{
	/* Stubs for dynamic attach - in net/if_ethersubr.c */
	extern void (*arpreq)();
	extern int  (*arpres)();
	extern int  (*arpctl)();
	extern void  (*arpreqall)();
	extern void  (*arpalias)();

	ARP_LOCKINIT();
	if (!arptab)
		arptab = arptabXXX;
 	if (arptab_bsiz <= 0)
		arptab_bsiz = (ipgateway ? 16 : 9);
 	if (arptab_nb <= 0)
		arptab_nb = (ipgateway ? 37 : 19);
	arptab_size = arptab_bsiz * arptab_nb;
	if (arptab == arptabXXX &&
	    sizeof (struct arptab) * arptab_size > sizeof arptabXXX)
		panic("arpinit");
	bzero((caddr_t)arptab, sizeof (struct arptab) * arptab_size);
	IFQ_LOCKINIT(&arpintrq);
	arpintrq.ifq_maxlen = arpqmaxlen;
	arpctl = arpioctl_local;
	arpres = arpresolve_local;
	arpreq = arprequest;
	arpreqall = arprequestall;
	arpalias = arpaliasrequest;
	(void) netisr_add(NETISR_ARP, arpintr, &arpintrq, &inetdomain);
#if	!NETISR_THREAD
	arptimer();
#else
	net_threadstart(arptimer, 0);
#endif
}


/*
 * haddr_convert();
 * --------------
 * 
 * Converts a non-canonical address to a canonical address and a canonical 
 * address to a canonical address.
 *
 * Uses a sizeable translation table to do the conversion.
 *
 * INPUTS:
 *  adr - pointer to six byte string to convert (unsigned char *)
 *
 * OUTPUTS:
 *  The string is updated to contain the converted address.
 *
 * CALLER:
 *  arpinput, arprequest, and device drivers which have to deal with 
 *  non-canonical addresses as opposed to the hardware dealing with it.
 *
 */

static unsigned long con_table[256] = {
0x00,   /* 0x00 */  0x80,   /* 0x01 */  0x40,   /* 0x02 */  0xc0,   /* 0x03 */
0x20,   /* 0x04 */  0xa0,   /* 0x05 */  0x60,   /* 0x06 */  0xe0,   /* 0x07 */
0x10,   /* 0x08 */  0x90,   /* 0x09 */  0x50,   /* 0x0a */  0xd0,   /* 0x0b */
0x30,   /* 0x0c */  0xb0,   /* 0x0d */  0x70,   /* 0x0e */  0xf0,   /* 0x0f */
0x08,   /* 0x10 */  0x88,   /* 0x11 */  0x48,   /* 0x12 */  0xc8,   /* 0x13 */
0x28,   /* 0x14 */  0xa8,   /* 0x15 */  0x68,   /* 0x16 */  0xe8,   /* 0x17 */
0x18,   /* 0x18 */  0x98,   /* 0x19 */  0x58,   /* 0x1a */  0xd8,   /* 0x1b */
0x38,   /* 0x1c */  0xb8,   /* 0x1d */  0x78,   /* 0x1e */  0xf8,   /* 0x1f */
0x04,   /* 0x20 */  0x84,   /* 0x21 */  0x44,   /* 0x22 */  0xc4,   /* 0x23 */
0x24,   /* 0x24 */  0xa4,   /* 0x25 */  0x64,   /* 0x26 */  0xe4,   /* 0x27 */
0x14,   /* 0x28 */  0x94,   /* 0x29 */  0x54,   /* 0x2a */  0xd4,   /* 0x2b */
0x34,   /* 0x2c */  0xb4,   /* 0x2d */  0x74,   /* 0x2e */  0xf4,   /* 0x2f */
0x0c,   /* 0x30 */  0x8c,   /* 0x31 */  0x4c,   /* 0x32 */  0xcc,   /* 0x33 */
0x2c,   /* 0x34 */  0xac,   /* 0x35 */  0x6c,   /* 0x36 */  0xec,   /* 0x37 */
0x1c,   /* 0x38 */  0x9c,   /* 0x39 */  0x5c,   /* 0x3a */  0xdc,   /* 0x3b */
0x3c,   /* 0x3c */  0xbc,   /* 0x3d */  0x7c,   /* 0x3e */  0xfc,   /* 0x3f */
0x02,   /* 0x40 */  0x82,   /* 0x41 */  0x42,   /* 0x42 */  0xc2,   /* 0x43 */
0x22,   /* 0x44 */  0xa2,   /* 0x45 */  0x62,   /* 0x46 */  0xe2,   /* 0x47 */
0x12,   /* 0x48 */  0x92,   /* 0x49 */  0x52,   /* 0x4a */  0xd2,   /* 0x4b */
0x32,   /* 0x4c */  0xb2,   /* 0x4d */  0x72,   /* 0x4e */  0xf2,   /* 0x4f */
0x0a,   /* 0x50 */  0x8a,   /* 0x51 */  0x4a,   /* 0x52 */  0xca,   /* 0x53 */
0x2a,   /* 0x54 */  0xaa,   /* 0x55 */  0x6a,   /* 0x56 */  0xea,   /* 0x57 */
0x1a,   /* 0x58 */  0x9a,   /* 0x59 */  0x5a,   /* 0x5a */  0xda,   /* 0x5b */
0x3a,   /* 0x5c */  0xba,   /* 0x5d */  0x7a,   /* 0x5e */  0xfa,   /* 0x5f */
0x06,   /* 0x60 */  0x86,   /* 0x61 */  0x46,   /* 0x62 */  0xc6,   /* 0x63 */
0x26,   /* 0x64 */  0xa6,   /* 0x65 */  0x66,   /* 0x66 */  0xe6,   /* 0x67 */
0x16,   /* 0x68 */  0x96,   /* 0x69 */  0x56,   /* 0x6a */  0xd6,   /* 0x6b */
0x36,   /* 0x6c */  0xb6,   /* 0x6d */  0x76,   /* 0x6e */  0xf6,   /* 0x6f */
0x0e,   /* 0x70 */  0x8e,   /* 0x71 */  0x4e,   /* 0x72 */  0xce,   /* 0x73 */
0x2e,   /* 0x74 */  0xae,   /* 0x75 */  0x6e,   /* 0x76 */  0xee,   /* 0x77 */
0x1e,   /* 0x78 */  0x9e,   /* 0x79 */  0x5e,   /* 0x7a */  0xde,   /* 0x7b */
0x3e,   /* 0x7c */  0xbe,   /* 0x7d */  0x7e,   /* 0x7e */  0xfe,   /* 0x7f */
0x01,   /* 0x80 */  0x81,   /* 0x81 */  0x41,   /* 0x82 */  0xc1,   /* 0x83 */
0x21,   /* 0x84 */  0xa1,   /* 0x85 */  0x61,   /* 0x86 */  0xe1,   /* 0x87 */
0x11,   /* 0x88 */  0x91,   /* 0x89 */  0x51,   /* 0x8a */  0xd1,   /* 0x8b */
0x31,   /* 0x8c */  0xb1,   /* 0x8d */  0x71,   /* 0x8e */  0xf1,   /* 0x8f */
0x09,   /* 0x90 */  0x89,   /* 0x91 */  0x49,   /* 0x92 */  0xc9,   /* 0x93 */
0x29,   /* 0x94 */  0xa9,   /* 0x95 */  0x69,   /* 0x96 */  0xe9,   /* 0x97 */
0x19,   /* 0x98 */  0x99,   /* 0x99 */  0x59,   /* 0x9a */  0xd9,   /* 0x9b */
0x39,   /* 0x9c */  0xb9,   /* 0x9d */  0x79,   /* 0x9e */  0xf9,   /* 0x9f */
0x05,   /* 0xa0 */  0x85,   /* 0xa1 */  0x45,   /* 0xa2 */  0xc5,   /* 0xa3 */
0x25,   /* 0xa4 */  0xa5,   /* 0xa5 */  0x65,   /* 0xa6 */  0xe5,   /* 0xa7 */
0x15,   /* 0xa8 */  0x95,   /* 0xa9 */  0x55,   /* 0xaa */  0xd5,   /* 0xab */
0x35,   /* 0xac */  0xb5,   /* 0xad */  0x75,   /* 0xae */  0xf5,   /* 0xaf */
0x0d,   /* 0xb0 */  0x8d,   /* 0xb1 */  0x4d,   /* 0xb2 */  0xcd,   /* 0xb3 */
0x2d,   /* 0xb4 */  0xad,   /* 0xb5 */  0x6d,   /* 0xb6 */  0xed,   /* 0xb7 */
0x1d,   /* 0xb8 */  0x9d,   /* 0xb9 */  0x5d,   /* 0xba */  0xdd,   /* 0xbb */
0x3d,   /* 0xbc */  0xbd,   /* 0xbd */  0x7d,   /* 0xbe */  0xfd,   /* 0xbf */
0x03,   /* 0xc0 */  0x83,   /* 0xc1 */  0x43,   /* 0xc2 */  0xc3,   /* 0xc3 */
0x23,   /* 0xc4 */  0xa3,   /* 0xc5 */  0x63,   /* 0xc6 */  0xe3,   /* 0xc7 */
0x13,   /* 0xc8 */  0x93,   /* 0xc9 */  0x53,   /* 0xca */  0xd3,   /* 0xcb */
0x33,   /* 0xcc */  0xb3,   /* 0xcd */  0x73,   /* 0xce */  0xf3,   /* 0xcf */
0x0b,   /* 0xd0 */  0x8b,   /* 0xd1 */  0x4b,   /* 0xd2 */  0xcb,   /* 0xd3 */
0x2b,   /* 0xd4 */  0xab,   /* 0xd5 */  0x6b,   /* 0xd6 */  0xeb,   /* 0xd7 */
0x1b,   /* 0xd8 */  0x9b,   /* 0xd9 */  0x5b,   /* 0xda */  0xdb,   /* 0xdb */
0x3b,   /* 0xdc */  0xbb,   /* 0xdd */  0x7b,   /* 0xde */  0xfb,   /* 0xdf */
0x07,   /* 0xe0 */  0x87,   /* 0xe1 */  0x47,   /* 0xe2 */  0xc7,   /* 0xe3 */
0x27,   /* 0xe4 */  0xa7,   /* 0xe5 */  0x67,   /* 0xe6 */  0xe7,   /* 0xe7 */
0x17,   /* 0xe8 */  0x97,   /* 0xe9 */  0x57,   /* 0xea */  0xd7,   /* 0xeb */
0x37,   /* 0xec */  0xb7,   /* 0xed */  0x77,   /* 0xee */  0xf7,   /* 0xef */
0x0f,   /* 0xf0 */  0x8f,   /* 0xf1 */  0x4f,   /* 0xf2 */  0xcf,   /* 0xf3 */
0x2f,   /* 0xf4 */  0xaf,   /* 0xf5 */  0x6f,   /* 0xf6 */  0xef,   /* 0xf7 */
0x1f,   /* 0xf8 */  0x9f,   /* 0xf9 */  0x5f,   /* 0xfa */  0xdf,   /* 0xfb */
0x3f,   /* 0xfc */  0xbf,   /* 0xfd */  0x7f,   /* 0xfe */  0xff,   /* 0xff */
} ;

void 
haddr_convert(addr)
u_char *addr;
{
  u_long i;

  for (i=0; i<6; i++) {
        *addr = con_table[*addr] & 0xFF;
        addr++;
  };
}

