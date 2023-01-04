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
static char	*sccsid = "@(#)$RCSfile: if.c,v $ $Revision: 4.4.7.4 $ (DEC) $Date: 1993/11/03 22:25:29 $";
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
 *	Base:	if.c	7.8 (Berkeley) 5/5/89
 *	Merged: if.c	7.13 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 10-Oct-91	Heather Gray
 *	Add if_isfull() routine
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/kernel.h"
#include "sys/ioctl.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"
#include "sys/protosw.h"

#include "net/if.h"
#include "net/if_dl.h"
#include "net/if_types.h"
#include "net/route.h"
#include "net/gw_screen.h"

#include "net/net_malloc.h"
#include "net/if_trn_sr.h"

LOCK_ASSERTL_DECL

int	ifqmaxlen = IFQ_MAXLEN;

extern int (*sr_ioctl_func)();

/*
 * Network interface utility routines.
 *
 * Routines with ifa_ifwith* names take sockaddr *'s as
 * parameters.
 */

struct	ifnet *ifnet;
extern	struct ifnet loif;
int	if_index, if_indexlim = 8;
struct	ifaddr **ifnet_addrs;
void
ifinit()
{
	register struct ifnet *ifp;

	loinit();		/* Be sure to init loopback */

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_snd.ifq_maxlen == 0)
			ifp->if_snd.ifq_maxlen = ifqmaxlen;
		if (ifp->if_hdrlen > max_linkhdr)
			max_linkhdr = ifp->if_hdrlen;
	}

#if	!NETISR_THREAD
	if_slowtimo();
#else
	net_threadstart(if_slowtimo, -1);
#endif
}

#ifdef	vax
void
ifubareset(uban)
{
	ifreset(uban);
}
#endif

/*
 * Call each interface on a bus reset.
 */
void
ifreset(n)
	int n;
{
	register struct ifnet *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
		if (ifp->if_reset)
			(*ifp->if_reset)(ifp->if_unit, n);
}

static void
sprint_d(cp, n)
	register char *cp;
	u_short n;
{
	register int q, m;
	do {
	    if (n >= 10000) m = 10000;
		else if (n >= 1000) m = 1000;
		else if (n >= 100) m = 100;
		else if (n >= 10) m = 10;
		else m = 1;
	    q = n / m;
	    n -= m * q;
	    if (q > 9) q = 10; /* For crays with more than 100K interfaces */
	    *cp++ = "0123456789Z"[q];
	} while (n > 0);
	*cp++ = 0;
}

/*
 * Attach an interface to the
 * list of "active" interfaces.
 */
void
if_attach(ifp)
	struct ifnet *ifp;
{
	unsigned socksize, ifasize;
	int namelen, unitlen;
	char workbuf[16];
	register struct ifnet **p = &ifnet;
	register struct sockaddr_dl *sdl;
	register struct ifaddr *ifa;

	if (ifp != &loif) {
		IFQ_LOCKINIT(&(ifp->if_snd));
		NETSTAT_LOCKINIT(&(ifp->if_slock));
	}
	while (*p)
		p = &((*p)->if_next);
	*p = ifp;

/* Parallelize me! */
	ifp->if_index = ++if_index;
	if (ifnet_addrs == 0 || if_index >= if_indexlim) {
		unsigned n = (if_indexlim <<= 1) * sizeof(ifa);
		struct ifaddr **q;
		{
		    NET_MALLOC(q, struct ifaddr **, n, M_IFADDR, M_WAITOK);
		    bzero((caddr_t)q, n);
		}

		if (ifnet_addrs) {
			bcopy((caddr_t)ifnet_addrs, (caddr_t)q, n/2);
			{
			    NET_FREE(ifnet_addrs, M_IFADDR);
			}
		}
		ifnet_addrs = q;
	}
	/*
	 * create a Link Level name for this device
	 */
	sprint_d(workbuf, ifp->if_unit);
	namelen = strlen(ifp->if_name);

	unitlen = strlen(workbuf);
#define _offsetof(t, m) ((int)((caddr_t)&((t *)0)->m))
	socksize = _offsetof(struct sockaddr_dl, sdl_data[0]) +
				unitlen + namelen + ifp->if_addrlen;
	socksize = (socksize + (sizeof(long)-1)) & ~(sizeof(long)-1);
	if (socksize < sizeof(*sdl))
		socksize = sizeof(*sdl);
	ifasize = sizeof(*ifa) + 2 * socksize;
	NET_MALLOC(ifa, struct ifaddr *, ifasize, M_IFADDR, M_WAITOK);
	ifnet_addrs[if_index - 1] = ifa;
	bzero((caddr_t)ifa, ifasize);
	sdl = (struct sockaddr_dl *)(ifa + 1);
	ifa->ifa_addr = (struct sockaddr *)sdl;
	ifa->ifa_ifp = ifp;
	sdl->sdl_len = socksize;
	sdl->sdl_family = AF_LINK;
	bcopy(ifp->if_name, sdl->sdl_data, namelen);
	bcopy((caddr_t)workbuf, namelen + (caddr_t)sdl->sdl_data, unitlen);
	sdl->sdl_nlen = (namelen += unitlen);
	sdl->sdl_index = ifp->if_index;
	sdl = (struct sockaddr_dl *)(socksize + (caddr_t)sdl);
	ifa->ifa_netmask = (struct sockaddr *)sdl;
	sdl->sdl_len = socksize - ifp->if_addrlen;
	while (namelen != 0)
		sdl->sdl_data[--namelen] = 0xff;
	ifa->ifa_next = ifp->if_addrlist;
	ifa->ifa_rtrequest = link_rtrequest;
	ifp->if_addrlist = ifa;
}

/*
 * Locate an interface based on a complete address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

#define	equal(a1, a2) \
  (bcmp((caddr_t)(a1), (caddr_t)(a2), ((struct sockaddr *)(a1))->sa_len) == 0)
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != addr->sa_family)
			continue;
		if (equal(addr, ifa->ifa_addr))
			return (ifa);
		if ((ifp->if_flags & IFF_BROADCAST) && ifa->ifa_broadaddr &&
		    equal(ifa->ifa_broadaddr, addr))
			return (ifa);
	}
	return ((struct ifaddr *)0);
}
/*
 * Locate the point to point interface with a given destination address.
 */
/*ARGSUSED*/
struct ifaddr *
ifa_ifwithdstaddr(addr)
	register struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next) 
	    if (ifp->if_flags & IFF_POINTOPOINT)
		for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
			if (ifa->ifa_addr->sa_family != addr->sa_family)
				continue;
			if (equal(addr, ifa->ifa_dstaddr))
				return (ifa);
	}
	return ((struct ifaddr *)0);
}

/*
 * Find an interface on a specific network.  If many, choice
 * is first found.
 */
struct ifaddr *
ifa_ifwithnet(addr)
	struct sockaddr *addr;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;
	u_int af = addr->sa_family;

	if (af >= AF_MAX)
		return (0);
	if (af == AF_LINK) {
		register struct sockaddr_dl *sdl = (struct sockaddr_dl *)addr;
		if (sdl->sdl_index && sdl->sdl_index <= if_index)
			return (ifnet_addrs[sdl->sdl_index - 1]);
	}
	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		register char *cp, *cp2, *cp3;
		register char *cplim;
		if (ifa->ifa_addr->sa_family != af || ifa->ifa_netmask == 0)
			continue;
		cp = addr->sa_data;
		cp2 = ifa->ifa_addr->sa_data;
		cp3 = ifa->ifa_netmask->sa_data;
		cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
		for (; cp3 < cplim; cp3++)
			if ((*cp++ ^ *cp2++) & *cp3)
				break;
		if (cp3 == cplim)
			return (ifa);
	    }
	return ((struct ifaddr *)0);
}

/*
 * Find an interface using a specific address family
 */
struct ifaddr *
ifa_ifwithaf(af)
	register int af;
{
	register struct ifnet *ifp;
	register struct ifaddr *ifa;

	for (ifp = ifnet; ifp; ifp = ifp->if_next)
	    for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		if (ifa->ifa_addr->sa_family == af)
			return (ifa);
	return ((struct ifaddr *)0);
}

/*
 * Find an interface address specific to an interface best matching
 * a given address.
 */
struct ifaddr *
ifaof_ifpforaddr(addr, ifp)
	struct sockaddr *addr;
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;
	register char *cp, *cp2, *cp3;
	register char *cplim;
	struct ifaddr *ifa_maybe = 0;
	u_int af = addr->sa_family;

	if (af >= AF_MAX)
		return (0);
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != af)
			continue;
		ifa_maybe = ifa;
		if (ifa->ifa_netmask == 0) {
			if (equal(addr, ifa->ifa_addr) ||
			    (ifa->ifa_dstaddr && equal(addr, ifa->ifa_dstaddr)))
				return (ifa);
			continue;
		}
		cp = addr->sa_data;
		cp2 = ifa->ifa_addr->sa_data;
		cp3 = ifa->ifa_netmask->sa_data;
		cplim = ifa->ifa_netmask->sa_len + (char *)ifa->ifa_netmask;
		for (; cp3 < cplim; cp3++)
			if ((*cp++ ^ *cp2++) & *cp3)
				break;
		if (cp3 == cplim)
			return (ifa);
	}
	return (ifa_maybe);
}

/*
 * Default action when installing a route with a Link Level gateway.
 * Lookup an appropriate real ifa to point to.
 * This should be moved to /sys/net/link.c eventually.
 */
void
link_rtrequest(cmd, rt, sa)
register struct rtentry *rt;
struct sockaddr *sa;
{
	register struct ifaddr *ifa;
	struct sockaddr *dst;
	struct ifnet *ifp, *oldifnet = ifnet;

	if (cmd != RTM_ADD || ((ifa = rt->rt_ifa) == 0) ||
	    ((ifp = ifa->ifa_ifp) == 0) || ((dst = rt_key(rt)) == 0))
		return;
	if (ifa = ifaof_ifpforaddr(dst, ifp)) {
		rt->rt_ifa = ifa;
		if (ifa->ifa_rtrequest && ifa->ifa_rtrequest != link_rtrequest)
			ifa->ifa_rtrequest(cmd, rt, sa);
	}
}
/*
 * Mark an interface down and notify protocols of
 * the transition.
 * NOTE: must be called at splnet or eqivalent.
 */
void
if_down(ifp)
	register struct ifnet *ifp;
{
	register struct ifaddr *ifa;

	ifp->if_flags &= ~IFF_UP;
	for (ifa = ifp->if_addrlist; ifa; ifa = ifa->ifa_next)
		pfctlinput(PRC_IFDOWN, ifa->ifa_addr);
	if_qflush(&ifp->if_snd);
}

/*
 * Flush an interface queue.
 */
void
if_qflush(ifq)
	register struct ifqueue *ifq;
{
	register struct mbuf *m, *n;
	int s;

	s = splimp();
	IFQ_LOCK(ifq);
	n = ifq->ifq_head;
	while (m = n) {
		n = m->m_act;
		m_freem(m);
	}
	ifq->ifq_head = 0;
	ifq->ifq_tail = 0;
	ifq->ifq_len = 0;
	IFQ_UNLOCK(ifq);
	splx(s);
}

/*
 * Check if there is sufficient space on an interface queue for an
 * additional message.  Used by ip_output() to avoid queuing partial
 * datagrams.  Record drop for interface here.
 */
int
if_isfull(ifp)
	register struct ifnet *ifp;
{
	register int full,s;

	s = splimp();
	IFQ_LOCK(&ifp->if_snd);
	if (full = IF_QFULL(&ifp->if_snd))
	    IF_DROP(&ifp->if_snd);
	IFQ_UNLOCK(&ifp->if_snd);
	splx(s);
	return (full);
}

/*
 * Handle interface watchdog timer routines.  Called
 * from softclock, we decrement timers (if set) and
 * call the appropriate interface routine on expiration.
 */
if_slowtimo()
{
	register struct ifnet *ifp;
	int s = splimp();

	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (ifp->if_timer == 0 || --ifp->if_timer)
			continue;
		if (ifp->if_watchdog)
			(*ifp->if_watchdog)(ifp->if_unit);
	}
	splx(s);
#if	!NETISR_THREAD
	timeout(if_slowtimo, (caddr_t)0, hz / IFNET_SLOWHZ);
#else
	return (hz/IFNET_SLOWHZ);
#endif
}

/*
 * Map interface name to
 * interface structure pointer.
 */
struct ifnet *
ifunit(name)
	register char *name;
{
	register char *cp;
	register struct ifnet *ifp;
	int unit;
	unsigned len;
	char *ep, c;

	for (cp = name; cp < name + IFNAMSIZ && *cp; cp++)
		if (*cp >= '0' && *cp <= '9')
			break;
	if (*cp == '\0' || cp == name + IFNAMSIZ)
		return ((struct ifnet *)0);
	/*
	 * Save first char of unit, and pointer to it,
	 * so we can put a null there to avoid matching
	 * initial substrings of interface names.
	 */
	len = cp - name + 1;
	c = *cp;
	ep = cp;
	for (unit = 0; *cp >= '0' && *cp <= '9'; )
		unit = unit * 10 + *cp++ - '0';
	*ep = 0;

	/*
	 * Following lines for se/ln compatibility
	 */
	if ((len == 3) && (!bcmp(name, "se", 2))) {
		bcopy("ln", name, 2);
	}
	for (ifp = ifnet; ifp; ifp = ifp->if_next) {
		if (bcmp(ifp->if_name, name, len))
			continue;
		if (unit == ifp->if_unit)
			break;
	}
	*ep = c;
	return (ifp);
}

/*
 * Interface ioctls.
 */
ifioctl(so, cmd, data)
	struct socket *so;
	unsigned int cmd;
	caddr_t data;
{
	register struct ifnet *ifp;
	register struct ifreq *ifr;
	int error;

	LOCK_ASSERT("ifioctl", SOCKET_ISLOCKED(so));

	switch (cmd) {

        case SIOCSCREENON:
        case SIOCSCREEN:
        case SIOCSCREENSTATS:
                return(screen_control(so, cmd, data));

	case SIOCGIFCONF:
#ifdef	OSIOCGIFCONF
	case OSIOCGIFCONF:
#endif
		return (ifconf(cmd, data));

	case SIOCSARP:
	case SIOCDARP:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		/* FALL THROUGH */
	case SIOCGARP:
#ifdef	OSIOCGARP
	case OSIOCGARP:
#endif
		return (arpioctl(cmd, data));

	case SIOCSRREQW:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		/* FALL THROUGH */
	case SIOCSRREQR:
		return ((*sr_ioctl_func)(cmd, data));

	}
	ifr = (struct ifreq *)data;
	ifp = ifunit(ifr->ifr_name);
	if (ifp == 0)
		return (ENXIO);

	if ((unsigned)cmd == 0xC01C691C) /* ULTRIX value for SIOCRPHYSADDR */
		cmd = SIOCRPHYSADDR;

	switch (cmd) {

	case SIOCGIFFLAGS:
		ifr->ifr_flags = ifp->if_flags;
		break;

	case SIOCGIFMETRIC:
		ifr->ifr_metric = ifp->if_metric;
		break;

	case SIOCSIFFLAGS:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		if (ifp->if_flags & IFF_UP && (ifr->ifr_flags & IFF_UP) == 0) {
			int s = splimp();
			if_down(ifp);
			splx(s);
		}
		ifp->if_flags = (ifp->if_flags & IFF_CANTCHANGE) |
			(ifr->ifr_flags &~ IFF_CANTCHANGE);
		if (ifp->if_ioctl)
			(void) (*ifp->if_ioctl)(ifp, cmd, data);
		break;

	case SIOCSIFMETRIC:
		if (!(so->so_state & SS_PRIV))
			return (EACCES);
		ifp->if_metric = ifr->ifr_metric;
		break;

        case SIOCDELMULTI:
        case SIOCADDMULTI:
		if (!(so->so_state & SS_PRIV))
			 return (EACCES);
		/* update address in multicast tables */
		if (cmd == SIOCADDMULTI)
			error = if_addmulti(ifp, ifr);
		else
			error = if_delmulti(ifp, ifr);
		return(error);
        case SIOCENABLBACK:
        case SIOCDISABLBACK:
        case SIOCSPHYSADDR:
        case SIOCRDZCTRS:
        case SIOCIFRESET:
        case SIOCIFSETCHAR:
        case SIOCEEUPDATE:
	case SIOCSMACSPEED:
	case SIOCSIPMTU:
                if(!(so->so_state & SS_PRIV))
                        return(EACCES);
        case SIOCRDCTRS:
        case SIOCRPHYSADDR:
	case SIOCRMACSPEED:
                if (ifp->if_ioctl == 0)
                        return (EOPNOTSUPP);
		error =  (*ifp->if_ioctl)(ifp, cmd, data);
		return(error);
	case SIOCRIPMTU:
	  	ifr->ifr_value = ifp->if_mtu;
	break;

	default:
		if (so->so_proto == 0)
			return (EOPNOTSUPP);
#ifndef COMPAT_43
		return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
			cmd, data, ifp));
#else
	    {
		int ocmd = cmd;

		switch (cmd) {

		case SIOCSIFDSTADDR:
		case SIOCSIFADDR:
		case SIOCSIFBRDADDR:
		case SIOCSIFNETMASK:
#if BYTE_ORDER != BIG_ENDIAN
			if (ifr->ifr_addr.sa_family == 0 &&
			    ifr->ifr_addr.sa_len < 16) {
				ifr->ifr_addr.sa_family = ifr->ifr_addr.sa_len;
				ifr->ifr_addr.sa_len = 16;
			}
#else
			if (ifr->ifr_addr.sa_len == 0)
				ifr->ifr_addr.sa_len = 16;
#endif
			break;

#ifdef	OSIOCGIFADDR
		case OSIOCGIFADDR:
			cmd = SIOCGIFADDR;
			break;

		case OSIOCGIFDSTADDR:
			cmd = SIOCGIFDSTADDR;
			break;

		case OSIOCGIFBRDADDR:
			cmd = SIOCGIFBRDADDR;
			break;

		case OSIOCGIFNETMASK:
			cmd = SIOCGIFNETMASK;
#endif
		}
		error =  ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL,
							    cmd, data, ifp));
#ifdef	OSIOCGIFADDR
		switch (ocmd) {

		case OSIOCGIFADDR:
		case OSIOCGIFDSTADDR:
		case OSIOCGIFBRDADDR:
		case OSIOCGIFNETMASK:
			((struct osockaddr *)&ifr->ifr_addr)->sa_family =
							ifr->ifr_addr.sa_family;
		}
#endif
		return (error);

	    }
#endif
	}
	return (0);
}

/*
 * Return interface configuration
 * of system.  List may be used
 * in later ioctl's (above) to get
 * other information.
 */
/*ARGSUSED*/
ifconf(cmd, data)
	unsigned int cmd;
	caddr_t data;
{
	register struct ifconf *ifc = (struct ifconf *)data;
	register struct ifnet *ifp = ifnet;
	register struct ifaddr *ifa;
	register char *cp, *ep;
	struct ifreq ifr, *ifrp;
	int space = ifc->ifc_len, error = 0;

	ifrp = ifc->ifc_req;
	ep = ifr.ifr_name + sizeof (ifr.ifr_name) - 3;	/* name##\0 */
	for (; space > sizeof (ifr) && ifp; ifp = ifp->if_next) {
		bcopy(ifp->if_name, ifr.ifr_name, sizeof (ifr.ifr_name) - 2);
		for (cp = ifr.ifr_name; cp < ep && *cp; cp++)
			;
		if (ifp->if_unit >= 10)		/* >99 units need work */
			*cp++ = '0' + ((ifp->if_unit / 10) % 10);
		*cp++ = '0' + (ifp->if_unit % 10);
		*cp = '\0';
		if ((ifa = ifp->if_addrlist) == 0) {
			bzero((caddr_t)&ifr, sizeof(ifr.ifr_addr));
			error = copyout((caddr_t)&ifr, (caddr_t)ifrp, sizeof (ifr));
			if (error)
				break;
			space -= sizeof (ifr), ifrp++;
		} else 
		    for ( ; space > sizeof (ifr) && ifa; ifa = ifa->ifa_next) {
			register struct sockaddr *sa = ifa->ifa_addr;
#if	defined(COMPAT_43) && defined(OSIOCGIFCONF)
			if (cmd == OSIOCGIFCONF) {
				struct osockaddr *osa =
					 (struct osockaddr *)&ifr.ifr_addr;
				ifr.ifr_addr = *sa;
				osa->sa_family = sa->sa_family;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else
#endif
			if (sa->sa_len <= sizeof(*sa)) {
				ifr.ifr_addr = *sa;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr));
				ifrp++;
			} else {
				space -= sa->sa_len - sizeof(*sa);
				if (space < sizeof (ifr))
					break;
				error = copyout((caddr_t)&ifr, (caddr_t)ifrp,
						sizeof (ifr.ifr_name));
				if (error == 0)
				    error = copyout((caddr_t)sa,
				      (caddr_t)&ifrp->ifr_addr, sa->sa_len);
				ifrp = (struct ifreq *)
					(sa->sa_len + (caddr_t)&ifrp->ifr_addr);
			}
			if (error)
				break;
			space -= sizeof (ifr);
		}
	}
	ifc->ifc_len -= space;
	return (error);
}
