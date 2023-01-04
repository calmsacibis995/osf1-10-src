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
static char	*sccsid = "@(#)$RCSfile: in.c,v $ $Revision: 4.3.16.3 $ (DEC) $Date: 1993/07/13 19:29:49 $";
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
 *	Base:	in.c	7.13 (Berkeley) 9/20/89
 *	Merged:	in.c	7.15 (Berkeley) 6/28/90
 */
/*
 *	Revision History:
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patches.
 *
 */

#ifdef __alpha
#include "machine/endian.h"
#endif

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/errno.h"
#include "sys/ioctl.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"

#include "net/if.h"
#include "net/route.h"

#include "netinet/in.h"
#include "netinet/in_systm.h"
#include "netinet/in_var.h"

#include "net/net_malloc.h"

LOCK_ASSERTL_DECL

#if	INETPRINTFS
int inetprintfs = INETPRINTFS;		/* 0=none, !0=errors, >1=info+errors */
#endif

/*
 * Formulate an Internet address from network + host.
 */
struct in_addr
in_makeaddr(net, host)
	u_int net, host;
{
	register struct in_ifaddr *ia;
	register u_int mask;
	u_int addr;

	if (IN_CLASSA(net))
		mask = IN_CLASSA_HOST;
	else if (IN_CLASSB(net))
		mask = IN_CLASSB_HOST;
	else
		mask = IN_CLASSC_HOST;
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if ((ia->ia_netmask & net) == ia->ia_net) {
			mask = ~ia->ia_subnetmask;
			break;
		}
	addr = htonl(net | (host & mask));
	return (*(struct in_addr *)&addr);
}

/*
 * Return the network number from an internet address.
 */
u_int
in_netof(in)
	struct in_addr in;
{
	register u_int i = ntohl(in.s_addr);
	register u_int net;
	register struct in_ifaddr *ia;

	if (IN_CLASSA(i))
		net = i & IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		net = i & IN_CLASSB_NET;
	else if (IN_CLASSC(i))
		net = i & IN_CLASSC_NET;
	else if (IN_CLASSD(i))
		net = i & IN_CLASSD_NET;
	else
		return (0);

	/*
	 * Check whether network is a subnet;
	 * if so, return subnet number.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (net == ia->ia_net)
			return (i & ia->ia_subnetmask);
	return (net);
}

/*
 * Compute and save network mask as sockaddr from an internet address.
 */
void
in_sockmaskof(in, sockmask)
	struct in_addr in;
	register struct sockaddr_in *sockmask;
{
	register u_int net;
	register u_int mask;
    {
	register u_int i = ntohl(in.s_addr);

	if (i == 0)
		net = 0, mask = 0;
	else if (IN_CLASSA(i))
		net = i & IN_CLASSA_NET, mask = IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		net = i & IN_CLASSB_NET, mask = IN_CLASSB_NET;
	else if (IN_CLASSC(i))
		net = i & IN_CLASSC_NET, mask = IN_CLASSC_NET;
	else
		net = i, mask = -1;
    }
    {
	register struct in_ifaddr *ia;
	/*
	 * Check whether network is a subnet;
	 * if so, return subnet number.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (net == ia->ia_net)
			mask =  ia->ia_subnetmask;
    }
    {
	register char *cpbase = (char *)&(sockmask->sin_addr);
	register char *cp = (char *)(1 + &(sockmask->sin_addr));

	sockmask->sin_addr.s_addr = htonl(mask);
	sockmask->sin_family = AF_INET;
	sockmask->sin_len = 0;
	while (--cp >= cpbase)
		if (*cp) {
			sockmask->sin_len = 1 + cp - (caddr_t)sockmask;
			break;
		}
    }
}

/*
 * Return the host portion of an internet address.
 */
u_int
in_lnaof(in)
	struct in_addr in;
{
	register u_int i = ntohl(in.s_addr);
	register u_int net, host;
	register struct in_ifaddr *ia;

	if (IN_CLASSA(i)) {
		net = i & IN_CLASSA_NET;
		host = i & IN_CLASSA_HOST;
	} else if (IN_CLASSB(i)) {
		net = i & IN_CLASSB_NET;
		host = i & IN_CLASSB_HOST;
	} else if (IN_CLASSC(i)) {
		net = i & IN_CLASSC_NET;
		host = i & IN_CLASSC_HOST;
	} else if (IN_CLASSD(i)) {
		net = i & IN_CLASSD_NET;
		host = i & IN_CLASSD_HOST;
	} else
		return (i);

	/*
	 * Check whether network is a subnet;
	 * if so, use the modified interpretation of `host'.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (net == ia->ia_net)
			return (host &~ ia->ia_subnetmask);
	return (host);
}

int subnetsarelocal = SUBNETSARELOCAL;

/*
 * Return 1 if an internet address is for a ``local'' host
 * (one to which we have a connection).  If subnetsarelocal
 * is true, this includes other subnets of the local net.
 * Otherwise, it includes only the directly-connected (sub)nets.
 */
in_localaddr(in)
	struct in_addr in;
{
	register u_int i = ntohl(in.s_addr);
	register struct in_ifaddr *ia;

	if (subnetsarelocal) {
		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if ((i & ia->ia_netmask) == ia->ia_net)
				return (1);
	} else {
		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if ((i & ia->ia_subnetmask) == ia->ia_subnet)
				return (1);
	}
	return (0);
}

/*
 * Determine whether an IP address is in a reserved set of addresses
 * that may not be forwarded, or whether datagrams to that destination
 * may be forwarded.
 */
in_canforward(in)
	struct in_addr in;
{
	register u_int i = ntohl(in.s_addr);
	register u_int net;

	if (IN_EXPERIMENTAL(i))
		return (0);
	if (IN_CLASSA(i)) {
		net = i & IN_CLASSA_NET;
		if (net == 0 || net == IN_LOOPBACKNET)
			return (0);
	}
	return (1);
}

int	in_interfaces;		/* number of external internet interfaces */
extern	struct ifnet loif;

/*
 * Generic internet control operations (ioctl's).
 * Ifp is 0 if not an interface-specific ioctl.
 */
#if     NETSYNC_LOCK
/*
 * Note: the lock around the "in_ifaddr" chain is here to prevent
 *  multiple threads from corrupting the interface chain.
 *  There are many places in the remainder of the code which read
 *  this chain, but which do not take a lock.  Mostly, this is done
 *  for performance reasons.  We must therefore be careful here to
 *  make sure that intermediate values within the chain not be corrupted.
 *  To do this, for new entries, we do not link into the chain until we're
 *  through manipulating the structure.  For existing entries, we unlink
 *  then relink the structure into the chain when all changes are made.
 *
 * This is from Encore. I am considering putting r/w semantics on the
 * list for completeness and accuracy. The read locks could be no-ops
 * to match this code.
 */
#endif
#define returnerror(e)	{ error = (e); goto ret; }
/* ARGSUSED */
in_control(so, cmd, data, ifp)
	struct socket *so;
	unsigned int cmd;
	caddr_t data;
	register struct ifnet *ifp;
{
	register struct ifreq *ifr = (struct ifreq *)data;
	register struct in_ifaddr *ia = 0;
	register struct ifaddr *ifa;
	struct in_ifaddr *oia;
	struct in_aliasreq *ifra = (struct in_aliasreq *)data;
	struct sockaddr_in oldaddr;
	int error, hostIsNew, maskIsNew;
	u_int i;

	INIFADDR_LOCK();

	/*
	 * Find address for this interface, if it exists.
	 */
	if (ifp)
		for (ia = in_ifaddr; ia; ia = ia->ia_next)
			if (ia->ia_ifp == ifp)
				break;

	switch ((int)cmd) {

#ifdef	SIOCAIFADDR
	case SIOCPIFADDR:
	case SIOCAIFADDR:
	case SIOCDIFADDR:
		/* Try to locate an existing entry for the address.  Try
                 * to determine the correct entry for not AF_INET addresses.
                 * This must be done to at least try to protect against calls
                 * with a badly formatted sockaddr (which would cause the
                 * rest of this routine to operate on the first entry for
                 * the interface rather than determining that the address
                 * was bogus).
                 */
                if (ifra->ifra_addr.sin_family == AF_INET) {
                    for (oia = ia; ia; ia = ia->ia_next) {
                        if (ia->ia_ifp == ifp  &&
                            ia->ia_addr.sin_addr.s_addr ==
                                ifra->ifra_addr.sin_addr.s_addr)
                            break;
                    }
                } else {
                    for (oia = ia; ia; ia = ia->ia_next)
                        if (ia->ia_ifp == ifp  &&
                            !bcmp(ia->ia_addr,ifra->ifra_addr,
                                sizeof(struct sockaddr)))
                                        break;
                }

		if (cmd == SIOCDIFADDR && ia == 0)
			returnerror (EADDRNOTAVAIL);
		/* FALLTHROUGH */
#endif
	case SIOCSIFADDR:
	case SIOCSIFNETMASK:
	case SIOCSIFDSTADDR:
		if (!(so->so_state & SS_PRIV))
			returnerror (EACCES);

		if (ifp == 0)
			panic("in_control");
		if (ia == (struct in_ifaddr *)0) {
			register struct in_ifaddr *iaold;
			NET_MALLOC(ia, struct in_ifaddr *, sizeof *ia,
							M_IFADDR, M_WAITOK);
			if (ia == 0)
				returnerror (ENOBUFS);
			bzero((caddr_t)ia, sizeof *ia);
			if (iaold = in_ifaddr) {
				for ( ; iaold->ia_next; iaold = iaold->ia_next)
					continue;
				iaold->ia_next = ia;
			} else
				in_ifaddr = ia;
                        if(cmd == SIOCPIFADDR) {
                                /* When setting the primary address, place
                                 * the address at the head of the interface's
                                 * list.  Skip over the link name entry
                                 * if it exists.
                                 */
                                ifa = ifp->if_addrlist;
                                if(ifa) {
                                        ia->ia_ifa.ifa_next = ifa->ifa_next;
                                        ifa->ifa_next = (struct ifaddr *)ia;
                                } else {
                                        ia->ia_ifa.ifa_next =
                                            (struct ifaddr *)ifp->if_addrlist;
                                        ifp->if_addrlist = (struct ifaddr *)ia;
                                }
                        } else {
                            if (ifa = ifp->if_addrlist) {
                                    for ( ; ifa->ifa_next; ifa = ifa->ifa_next)
                                            ;
                                    ifa->ifa_next = (struct ifaddr *) ia;
                            } else
                                ifp->if_addrlist = (struct ifaddr *) ia;
                        }
			ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;
			ia->ia_ifa.ifa_dstaddr
					= (struct sockaddr *)&ia->ia_dstaddr;
			ia->ia_ifa.ifa_netmask
					= (struct sockaddr *)&ia->ia_sockmask;
			ia->ia_sockmask.sin_len = 8;

			ia->ia_broadaddr.sin_len = sizeof(ia->ia_addr);
			ia->ia_broadaddr.sin_family = AF_INET;

			ia->ia_ifp = ifp;
			if (ifp != &loif)
				in_interfaces++;
		}
		break;

	case SIOCSIFBRDADDR:
		if (!(so->so_state & SS_PRIV))
			returnerror (EACCES);
		/* FALLTHROUGH */

	case SIOCGIFADDR:
	case SIOCGIFNETMASK:
	case SIOCGIFDSTADDR:
	case SIOCGIFBRDADDR:
		if (ia == (struct in_ifaddr *)0)
			returnerror (EADDRNOTAVAIL);
		break;

	default:
		returnerror (EOPNOTSUPP);
		/*NOTREACHED*/
	}
	switch ((int)cmd) {

	case SIOCGIFADDR:
		*((struct sockaddr_in *)&ifr->ifr_addr) = ia->ia_addr;
		break;

	case SIOCGIFBRDADDR:
		if ((ifp->if_flags & IFF_BROADCAST) == 0)
			returnerror (EINVAL);
		*((struct sockaddr_in *)&ifr->ifr_dstaddr) = ia->ia_broadaddr;
		break;

	case SIOCGIFDSTADDR:
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			returnerror (EINVAL);
		*((struct sockaddr_in *)&ifr->ifr_dstaddr) = ia->ia_dstaddr;
		break;

	case SIOCGIFNETMASK:
		*((struct sockaddr_in *)&ifr->ifr_addr) = ia->ia_sockmask;
		break;

	case SIOCSIFDSTADDR:
		if ((ifp->if_flags & IFF_POINTOPOINT) == 0)
			returnerror (EINVAL);
		oldaddr = ia->ia_dstaddr;
		ia->ia_dstaddr = *(struct sockaddr_in *)&ifr->ifr_dstaddr;
		if (ifp->if_ioctl &&
		    (error = (*ifp->if_ioctl)(ifp, SIOCSIFDSTADDR, ia))) {
			ia->ia_dstaddr = oldaddr;
			returnerror (error);
		}
		if (ia->ia_flags & IFA_ROUTE) {
			ia->ia_ifa.ifa_dstaddr = (struct sockaddr *)&oldaddr;
			rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
			ia->ia_ifa.ifa_dstaddr =
					(struct sockaddr *)&ia->ia_dstaddr;
			rtinit(&(ia->ia_ifa), (int)RTM_ADD, RTF_HOST|RTF_UP);
		}
		break;

	case SIOCSIFBRDADDR:
		if ((ifp->if_flags & IFF_BROADCAST) == 0)
			returnerror (EINVAL);
		ia->ia_broadaddr = *(struct sockaddr_in *)&ifr->ifr_broadaddr;
		break;

	case SIOCSIFADDR:
                error = in_ifinit(ifp, ia,
                    (struct sockaddr_in *) &ifr->ifr_addr, 1);
                if(!error && ifr->ifr_addr.sa_family == AF_INET)
                    arpaliaswhohas((struct arpcom *)ifp,&ia->ia_addr.sin_addr.s_addr);
                goto ret;
                break;


	case SIOCSIFNETMASK:
		i = ifra->ifra_addr.sin_addr.s_addr;
		ia->ia_subnetmask = ntohl(ia->ia_sockmask.sin_addr.s_addr = i);
		break;

#ifdef	SIOCAIFADDR
        case SIOCPIFADDR:
            /* if this is the primary getting set then get rid of any route
             * entries belonging to aliases.
             */
            ifa = ifp->if_addrlist;
            for(; ifa; ifa = ifa->ifa_next) {
                oia = (struct in_ifaddr *)ifa;
                if(oia->ia_ifa.ifa_addr->sa_family == ifra->ifra_addr.sin_family
 &&
                    (oia->ia_ifa.ifa_flags & IFA_ROUTE)) {
                        in_ifscrub(ifp, oia);
                        break;
                }
            }
            /* FALLTHROUGH */


	case SIOCAIFADDR:
		maskIsNew = 0; hostIsNew = 1; error = 0;
		if (ia->ia_addr.sin_family == AF_INET) {
			if (ifra->ifra_addr.sin_len == 0) {
				ifra->ifra_addr = ia->ia_addr;
				hostIsNew = 0;
			} else if (ifra->ifra_addr.sin_addr.s_addr ==
					       ia->ia_addr.sin_addr.s_addr)
				hostIsNew = 0;
		}
		if (ifra->ifra_mask.sin_len) {
			in_ifscrub(ifp, ia);
			ia->ia_sockmask = ifra->ifra_mask;
			ia->ia_subnetmask =
			     ntohl(ia->ia_sockmask.sin_addr.s_addr);
			maskIsNew = 1;
		}
		if ((ifp->if_flags & IFF_POINTOPOINT) &&
		    (ifra->ifra_dstaddr.sin_family == AF_INET)) {
			in_ifscrub(ifp, ia);
			ia->ia_dstaddr = ifra->ifra_dstaddr;
			maskIsNew  = 1; /* We lie; but the effect's the same */
		}
		if (ifra->ifra_addr.sin_family == AF_INET &&
		    (hostIsNew || maskIsNew))
			error = in_ifinit(ifp, ia, &ifra->ifra_addr, 0);
		if ((ifp->if_flags & IFF_BROADCAST) &&
		    (ifra->ifra_broadaddr.sin_family == AF_INET))
			ia->ia_broadaddr = ifra->ifra_broadaddr;
                if (ifra->ifra_addr.sin_family == AF_INET)
                    arpaliaswhohas((struct arpcom *)ifp,
                                &ia->ia_addr.sin_addr.s_addr);
		returnerror (error);

	case SIOCDIFADDR:
                i = ia->ia_ifa.ifa_flags & IFA_ROUTE;
		in_ifscrub(ifp, ia);
		if ((ifa = ifp->if_addrlist) == (struct ifaddr *)ia)
			ifp->if_addrlist = ifa->ifa_next;
		else {
			while (ifa->ifa_next &&
			       (ifa->ifa_next != (struct ifaddr *)ia))
				    ifa = ifa->ifa_next;
			if (ifa->ifa_next)
				ifa->ifa_next = ((struct ifaddr *)ia)->ifa_next;
#if	INETPRINTFS
			else if (inetprintfs)
				printf("Couldn't unlink inifaddr from ifp\n");
#endif
		}
		oia = ia;
		if (oia == (ia = in_ifaddr))
			in_ifaddr = ia->ia_next;
		else {
			while (ia->ia_next && (ia->ia_next != oia))
				ia = ia->ia_next;
			if (ia->ia_next)
				ia->ia_next = oia->ia_next;
#if	INETPRINTFS
			else if (inetprintfs)
				printf("Couldn't unlink inifaddr from in_ifaddr\n");
#endif
		}
                /* make sure that there is a route table entry for the
                 * network if alias address are present.  If the deleted
                 * entry had a route associated with it, then look for
                 * the first address of the same address family with no
                 * route and try to add the route. The add will fail if
                 * a route already exists.  This prevents the deletion of
                 * the primary from leaving no routes for the address family.
                 */
                if(i) {
                    ifa = ifp->if_addrlist;
                    for(; ifa; ifa = ifa->ifa_next) {
                        int flags = RTF_UP;

                        ia = (struct in_ifaddr *)ifa;
                        if(ia->ia_ifa.ifa_addr->sa_family == oia->ia_ifa.ifa_addr->sa_family &&
                                !(ia->ia_ifa.ifa_flags & IFA_ROUTE)) {
                            if (ifp->if_flags & IFF_LOOPBACK) {
                                flags |= RTF_HOST;
                            } else if (ifp->if_flags & IFF_POINTOPOINT) {
                                if (ia->ia_dstaddr.sin_family != AF_INET)
                                    break;
                                flags |= RTF_HOST;
                            }
                            if (rtinit(ia, (int)RTM_ADD, flags) == 0)
                                    ia->ia_flags |= IFA_ROUTE;
                            break;
                        }
                    }
                }
		NET_FREE(oia, M_IFADDR);
		break;
#endif

	default:
		if (ifp == 0 || ifp->if_ioctl == 0)
			returnerror (EOPNOTSUPP);
		returnerror ((*ifp->if_ioctl)(ifp, cmd, data));
	}
	error = 0;
ret:
	INIFADDR_UNLOCK();
	return error;
}

/*
 * Delete any existing route for an interface.
 */
void
in_ifscrub(ifp, ia)
	register struct ifnet *ifp;
	register struct in_ifaddr *ia;
{

	if ((ia->ia_flags & IFA_ROUTE) == 0)
		return;
	if (ifp->if_flags & (IFF_LOOPBACK|IFF_POINTOPOINT))
		rtinit(&(ia->ia_ifa), (int)RTM_DELETE, RTF_HOST);
	else
		rtinit(&(ia->ia_ifa), (int)RTM_DELETE, 0);
	ia->ia_flags &= ~IFA_ROUTE;
}

/*
 * Initialize an interface's internet address
 * and routing table entry.
 */
in_ifinit(ifp, ia, sin, scrub)
	register struct ifnet *ifp;
	register struct in_ifaddr *ia;
	struct sockaddr_in *sin;
	int scrub;
{
	register u_int i = ntohl(sin->sin_addr.s_addr);
	struct sockaddr_in oldaddr;
	int error, flags = RTF_UP;
	u_int addr;
	NETSPL_DECL(NETSPL(s,imp))

	oldaddr = ia->ia_addr;
	ia->ia_addr = *sin;
	/*
	 * Give the interface a chance to initialize
	 * if this is its first address,
	 * and to validate the address if necessary.
	 */
	if (ifp->if_ioctl && (error = (*ifp->if_ioctl)(ifp, SIOCSIFADDR, ia))) {
		NETSPLX(s);
		ia->ia_addr = oldaddr;
		return (error);
	}
	NETSPLX(s);
	if (scrub) {
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&oldaddr;
		in_ifscrub(ifp, ia);
		ia->ia_ifa.ifa_addr = (struct sockaddr *)&ia->ia_addr;
	}
	if (IN_CLASSA(i))
		ia->ia_netmask = IN_CLASSA_NET;
	else if (IN_CLASSB(i))
		ia->ia_netmask = IN_CLASSB_NET;
	else
		ia->ia_netmask = IN_CLASSC_NET;
	ia->ia_net = i & ia->ia_netmask;
	/*
	 * The subnet mask includes at least the standard network part,
	 * but may already have been set to a larger value.
	 */
	ia->ia_subnetmask |= ia->ia_netmask;
	ia->ia_subnet = i & ia->ia_subnetmask;
	ia->ia_sockmask.sin_addr.s_addr = htonl(ia->ia_subnetmask);
	ia->ia_sockmask.sin_family = AF_INET;
	{
		register char *cp = (char *) (1 + &(ia->ia_sockmask.sin_addr));
		register char *cpbase = (char *) &(ia->ia_sockmask.sin_addr);
		while (--cp >= cpbase)
			if (*cp) {
				ia->ia_sockmask.sin_len =
					1 + cp - (char *) &(ia->ia_sockmask);
				break;
			}
	}
	/*
	 * Add route for the network.
	 */
	if (ifp->if_flags & IFF_LOOPBACK) {
		ia->ia_ifa.ifa_dstaddr = ia->ia_ifa.ifa_addr;
		flags |= RTF_HOST;
	} else if (ifp->if_flags & IFF_POINTOPOINT) {
		if (ia->ia_dstaddr.sin_family != AF_INET)
			return (0);
		flags |= RTF_HOST;
	} else {
		addr = htonl( ~ia->ia_subnetmask | ia->ia_subnet );  
		ia->ia_broadaddr.sin_addr = (*(struct in_addr *)&addr);  
		if (ifp->if_flags & IFF_BROADCAST) {
			ia->ia_netbroadcast.s_addr = htonl(ia->ia_net | 
			    (INADDR_BROADCAST &~ ia->ia_netmask));
		}
	}
	if ((error = rtinit(&(ia->ia_ifa), (int)RTM_ADD, flags)) == 0)
		ia->ia_flags |= IFA_ROUTE;
	/*
	 * If the interface supports multicast, join the "all hosts"
	 * multicast group on that interface.
	 */
	if (ifp->if_flags & IFF_MULTICAST) {
		struct in_addr addr;

		addr.s_addr = htonl(INADDR_ALLHOSTS_GROUP);
		(void)in_addmulti(&addr, ifp);
	}
        /* The fact that a route already exists is not necessarily an error.
         * This is especially true when configuring aliases.  Don't
         * return EEXIST to the user if the route is a duplicate.  This
         * will stop the confusing error message from ifconfig (while
         * still permitting recognition of EEXIST errors from other
         * places in the system call).
         */
        if(error == EEXIST)
                error = 0;
	return (error);
}

/*
 * Return address info for specified internet network.
 */
struct in_ifaddr *
in_iaonnetof(net)
	u_int net;
{
	register struct in_ifaddr *ia;

	for (ia = in_ifaddr; ia; ia = ia->ia_next)
		if (ia->ia_subnet == net)
			return (ia);
	return ((struct in_ifaddr *)0);
}

/*
 * Return 1 if the address might be a local broadcast address.
 */
in_broadcast(in)
	struct in_addr in;
{
	register struct in_ifaddr *ia;
	u_int t;

	/*
	 * Look through the list of addresses for a match
	 * with a broadcast address.
	 */
	for (ia = in_ifaddr; ia; ia = ia->ia_next)
	    if (ia->ia_ifp->if_flags & IFF_BROADCAST) {
		if (ia->ia_broadaddr.sin_addr.s_addr == in.s_addr)
		     return (1);
		/*
		 * Check for old-style (host 0) broadcast.
		 */
		if ((t = ntohl(in.s_addr)) == ia->ia_subnet || t == ia->ia_net)
		    return (1);
	}
	if (in.s_addr == INADDR_BROADCAST || in.s_addr == INADDR_ANY)
		return (1);
	return (0);
}

/*
 * Add an address to the list of IP multicast addresses for a 
 * given interface.
 */
struct in_multi *
in_addmulti(ap, ifp)
	register struct in_addr *ap;
	register struct ifnet *ifp;
{
	register struct in_multi *inm;
	struct ifreq ifr;
	struct in_ifaddr *ia;
	int error;
	NETSPL_DECL(NETSPL(s,net))

	/*
	 * See if address already in list.
	 */
	IN_LOOKUP_MULTI(*ap, ifp, inm);
	if (inm != NULL) {
		/*
		 * Found it; just increment the reference count.
		 */
		++inm->inm_refcount;
	}
	else {
		/*
		 * New address; allocate a new multicast record
		 * and link it into the interface's multicast list.
		 */
		NET_MALLOC(inm, struct in_multi *, sizeof *inm,
						M_IPMADDR, M_NOWAIT);
		if (inm == NULL) {
			NETSPLX(s);
			return (NULL);
		}
		inm->inm_addr = *ap;
		inm->inm_ifp = ifp;
		inm->inm_refcount = 1;
		IFP_TO_IA(ifp, ia);
		if (ia == NULL) {
			NET_FREE(inm, M_IPMADDR);
			NETSPLX(s);
			return (NULL);
		}
		inm->inm_ia = ia;
		inm->inm_timer = 0;
		inm->inm_next = ia->ia_multiaddrs;
		/*
		 * Ask link layer to update its multicast table for 
		 * the new address.
		 */
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_family = AF_INET;
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr = *ap;

		error = if_addmulti(ifp, &ifr);
		if (error != 0) {
			NET_FREE(inm, M_IPMADDR);
			NETSPLX(s);
			return (NULL);
		}
		ia->ia_multiaddrs = inm; 
		/*
		 * Let IGMP know that we have joined a new IP multicast group.
		 */
		igmp_joingroup(inm);
	}
	NETSPLX(s);
	return (inm);
}

/*
 * Delete a multicast address record.
 */
int
in_delmulti(inm)
	register struct in_multi *inm;
{
	register struct in_multi **p;
	struct ifreq ifr;
	NETSPL_DECL(NETSPL(s,net))

	if (--inm->inm_refcount == 0) {
		/*
		 * No remaining claims to this record; let IGMP know that
		 * we are leaving the multicast group.
		 */
		igmp_leavegroup(inm);
		/*
		 * Unlink from list.
		 */
		for (p = &inm->inm_ia->ia_multiaddrs;
		     *p != NULL && *p != inm;
		     p = &(*p)->inm_next)
			continue;
		if (*p != inm)
			panic("in_delmulti");
		*p = (*p)->inm_next;
		/*
		 * Notify the link layer to update its multicast table.
		 */
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_family = AF_INET;
		((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr =
								inm->inm_addr;
		(void)if_delmulti(inm->inm_ifp, &ifr);
		NET_FREE(inm, M_IPMADDR);
	}
	NETSPLX(s);
}
