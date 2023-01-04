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
static char	*sccsid = "@(#)$RCSfile: if.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:26:21 $";
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

#if !defined(lint) && !defined(_NOIDENT)

#endif
/*

*/
/* 
 * COMPONENT_NAME: TCPIP if.c
 * 
 * FUNCTIONS: if_iflookup, if_ifwithaddr, if_ifwithdstaddr, 
 *            if_ifwithnet 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint
static char sccsid[] = "if.c	5.6 (Berkeley) 6/1/90";
#endif  not lint */

/*
 * Routing Table Management Daemon
 */
#include "defs.h"

extern	struct interface *ifnet;

/*
 * Find the interface with address addr.
 */
struct interface *
if_ifwithaddr(addr)
	struct sockaddr *addr;
{
	register struct interface *ifp;

#define	same(a1, a2) \
	(bcmp((caddr_t)((a1)->sa_data), (caddr_t)((a2)->sa_data), 14) == 0)
	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_REMOTE)
			continue;
		if (ifp->int_addr.sa_family != addr->sa_family)
			continue;
		if (same(&ifp->int_addr, addr))
			break;
		if ((ifp->int_flags & IFF_BROADCAST) &&
		    same(&ifp->int_broadaddr, addr))
			break;
	}
	return (ifp);
}

/*
 * Find the point-to-point interface with destination address addr.
 */
struct interface *
if_ifwithdstaddr(addr)
	struct sockaddr *addr;
{
	register struct interface *ifp;

	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if ((ifp->int_flags & IFF_POINTOPOINT) == 0)
			continue;
		if (same(&ifp->int_dstaddr, addr))
			break;
	}
	return (ifp);
}

/*
 * Find the interface on the network 
 * of the specified address.
 */
struct interface *
if_ifwithnet(addr)
	register struct sockaddr *addr;
{
	register struct  interface *ifp;
	register int af = addr->sa_family;
	register int (*netmatch)();

	if (af >= af_max)
		return (0);
	netmatch = afswitch[af].af_netmatch;
	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_flags & IFF_REMOTE)
			continue;
		if (af != ifp->int_addr.sa_family)
			continue;
		if ((*netmatch)(addr, &ifp->int_addr))
			break;
	}
	return (ifp);
}

/*
 * Find an interface from which the specified address
 * should have come from.  Used for figuring out which
 * interface a packet came in on -- for tracing.
 */
struct interface *
if_iflookup(addr)
	struct sockaddr *addr;
{
	register struct interface *ifp, *maybe;
	register int af = addr->sa_family;
	register int (*netmatch)();

	if (af >= af_max)
		return (0);
	maybe = 0;
	netmatch = afswitch[af].af_netmatch;
	for (ifp = ifnet; ifp; ifp = ifp->int_next) {
		if (ifp->int_addr.sa_family != af)
			continue;
		if (same(&ifp->int_addr, addr))
			break;
		if ((ifp->int_flags & IFF_BROADCAST) &&
		    same(&ifp->int_broadaddr, addr))
			break;
		if ((ifp->int_flags & IFF_POINTOPOINT) &&
		    same(&ifp->int_dstaddr, addr))
			break;
		if (maybe == 0 && (*netmatch)(addr, &ifp->int_addr))
			maybe = ifp;
	}
	if (ifp == 0)
		ifp = maybe;
	return (ifp);
}
