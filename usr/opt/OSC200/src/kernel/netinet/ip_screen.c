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
static char *rcsid = "@(#)$RCSfile: ip_screen.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/12/09 15:12:14 $";
#endif

/*
 *	16 December 1988	Jeffrey Mogul/DECWRL
 *		Created.
 *	Copyright (c) 1988 by Digital Equipment Corporation
 */
/*
 * IP Screening mechanism
 *	Basically, just an interface to protocol-independent
 *	stuff in net/gw_screen.c
 *
 */

#include "sys/param.h"
#include "net/net_globals.h"
#include "netinet/in.h" 
#include "netinet/in_systm.h"
#include "netinet/ip.h"
#include "netinet/ip_icmp.h"
#include "sys/mbuf.h"
#include "netinet/proto_inet.h"
#include "sys/socket.h"

/*
 * This procedure is instead of ip_forward() from ipintr().
 */
void
ip_forwardscreen(m, srcrt)
	struct mbuf *m;
	int srcrt;
{
	gw_forwardscreen(m, srcrt, AF_INET, ip_forward, ip_gwbounce);
}

/*
 * Called from gateway forwarding code if the packet is rejected
 * and the sender should be notified.
 */
void
ip_gwbounce(m, srcrt)
	struct mbuf *m;
	int srcrt;
{
	struct ip *ip = mtod(m, struct ip *);
	struct in_addr dest;
		    
	dest.s_addr = 0;
	ip->ip_id = htons(ip->ip_id);
	icmp_error(m, ICMP_UNREACH, ICMP_UNREACH_HOST, dest);
}
