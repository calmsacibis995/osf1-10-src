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
static char	*sccsid = "@(#)$RCSfile: if_loop.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/05/16 19:52:44 $";
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
 *	Base:	if_loop.c	7.9 (Berkeley) 9/20/89
 *	Merged:	if_loop.c	7.10 (Berkeley) 6/28/90
 */

/*
 * Loopback interface driver for protocol testing and timing.
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/time.h"
#include "sys/ioctl.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"

#include "net/if.h"
#include "net/if_types.h"
#include "net/netisr.h"
#include "net/route.h"

#define	LOMTU	(1024+512)

LOCK_ASSERTL_DECL

struct	ifnet loif;

void
loinit()
{
	IFQ_LOCKINIT(&(loif.if_snd));
	NETSTAT_LOCKINIT(&(loif.if_slock));
}

void
loattach()
{
	register struct ifnet *ifp = &loif;

	ifp->if_name = "lo";
	ifp->if_version = "Local Loopback Interface.";
	ifp->if_mtu = LOMTU;
	ifp->if_flags = IFF_LOOPBACK | IFF_MULTICAST;
	ifp->if_ioctl = loioctl;
	ifp->if_output = looutput;
	ifp->if_type = IFT_LOOP;
	ifp->if_hdrlen = 0;
	ifp->if_addrlen = 0;
	if_attach(ifp);
}

/* ARGSUSED */
looutput(ifp, m, dst, rt)
	struct ifnet *ifp;
	register struct mbuf *m;
	struct sockaddr *dst;
	struct rtentry *rt;
{
	int s;

	if ((m->m_flags & M_PKTHDR) == 0)
		panic("looutput no HDR");
	m->m_pkthdr.rcvif = ifp;

	s = splimp();
	NETSTAT_LOCK(&ifp->if_slock);
	ifp->if_opackets++;
	ifp->if_obytes += m->m_pkthdr.len;
	ifp->if_ipackets++;
	ifp->if_ibytes += m->m_pkthdr.len;
	NETSTAT_UNLOCK(&ifp->if_slock);
	splx(s);

	return netisr_input(netisr_af((int)dst->sa_family), m, (caddr_t)0, 0);
}

/*
 * Process an ioctl request.
 */
/* ARGSUSED */
loioctl(ifp, cmd, data)
	register struct ifnet *ifp;
	unsigned int cmd;
	caddr_t data;
{
	struct ifreq *ifr;
	int error = 0;

	switch (cmd) {

	case SIOCSIFADDR:
		ifp->if_flags |= IFF_UP;
		/*
		 * Everything else is done at a higher level.
		 */
		break;

	case SIOCADDMULTI:
	case SIOCDELMULTI:
		ifr = (struct ifreq *)data;
		if (ifr == 0) {
			error = EINVAL;
			break;
		}

		switch (ifr->ifr_addr.sa_family) {

		case AF_INET:
			break;
		default:
			error = EAFNOSUPPORT;
			break;
		}

		break;

	default:
		error = EINVAL;
	}
	return (error);
}
