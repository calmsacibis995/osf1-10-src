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
static char	*sccsid = "@(#)$RCSfile: ns_output.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/19 22:49:31 $";
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
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
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
 *	Base:	ns_output.c	7.7 (Berkeley) 6/28/90
 */

#include "net/net_globals.h"

#include "sys/param.h"
#include "sys/errno.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/socketvar.h"

#include "net/if.h"
#include "net/route.h"

#include "netns/ns.h"
#include "netns/ns_if.h"
#include "netns/idp.h"
#include "netns/idp_var.h"

int ns_hold_output = 0;
int ns_copy_output = 0;
int ns_output_cnt = 0;
struct mbuf *ns_lastout;

ns_output(m0, ro, flags)
	struct mbuf *m0;
	struct route *ro;
	int flags;
{
	register struct idp *idp = mtod(m0, struct idp *);
	register struct ifnet *ifp = 0;
	int error = 0;
	struct route idproute;
	struct sockaddr_ns *dst;
	extern int idpcksum;

	if (ns_hold_output) {
		if (ns_lastout) {
			(void)m_free(ns_lastout);
		}
		ns_lastout = m_copy(m0, 0, (int)M_COPYALL);
	}
	/*
	 * Route packet.
	 */
	if (ro == 0) {
		ro = &idproute;
		bzero((caddr_t)ro, sizeof (*ro));
	}
	dst = (struct sockaddr_ns *)&ro->ro_dst;
	if (ro->ro_rt == 0) {
		dst->sns_family = AF_NS;
		dst->sns_len = sizeof (*dst);
		dst->sns_addr = idp->idp_dna;
		dst->sns_addr.x_port = 0;
		/*
		 * If routing to interface only,
		 * short circuit routing lookup.
		 */
		if (flags & NS_ROUTETOIF) {
			struct ns_ifaddr *ia = ns_iaonnetof(&idp->idp_dna);

			if (ia == 0) {
				error = ENETUNREACH;
				goto bad;
			}
			ifp = ia->ia_ifp;
			goto gotif;
		}
		rtalloc(ro);
	} else if ((ro->ro_rt->rt_flags & RTF_UP) == 0) {
		/*
		 * The old route has gone away; try for a new one.
		 */
		rtfree(ro->ro_rt);
		ro->ro_rt = NULL;
		rtalloc(ro);
	}
	if (ro->ro_rt == 0 || (ifp = ro->ro_rt->rt_ifp) == 0) {
		error = ENETUNREACH;
		goto bad;
	}
	ro->ro_rt->rt_use++;
	if (ro->ro_rt->rt_flags & (RTF_GATEWAY|RTF_HOST))
		dst = (struct sockaddr_ns *)ro->ro_rt->rt_gateway;
gotif:

	/*
	 * Look for multicast addresses and
	 * and verify user is allowed to send
	 * such a packet.
	 */
	if (dst->sns_addr.x_host.c_host[0]&1) {
		if ((ifp->if_flags & IFF_BROADCAST) == 0) {
			error = EADDRNOTAVAIL;
			goto bad;
		}
		if ((flags & NS_ALLOWBROADCAST) == 0) {
			error = EACCES;
			goto bad;
		}
	}

	if (htons(idp->idp_len) <= ifp->if_mtu) {
		ns_output_cnt++;
		if (ns_copy_output) {
			ns_watch_output(m0, ifp);
		}
		error = (*ifp->if_output)(ifp, m0,
					(struct sockaddr *)dst, ro->ro_rt);
		goto done;
	} else error = EMSGSIZE;


bad:
	if (ns_copy_output) {
		ns_watch_output(m0, ifp);
	}
	m_freem(m0);
done:
	if (ro == &idproute && (flags & NS_ROUTETOIF) == 0 && ro->ro_rt) {
		RTFREE(ro->ro_rt);
		ro->ro_rt = 0;
	}
	return (error);
}
