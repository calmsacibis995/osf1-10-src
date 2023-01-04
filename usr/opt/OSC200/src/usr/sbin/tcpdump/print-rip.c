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
static char *rcsid = "@(#)$RCSfile: print-rip.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 18:47:05 $";
#endif
/*
 * Copyright (c) 1988-1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Based on:
 * static char rcsid[] = "print-rip.c,v 1.12 91/04/19 10:46:46 mccanne Exp $ (LBL)";
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <protocols/routed.h>

#include <errno.h>

#include "interface.h"
#include "addrtoname.h"

static void
rip_entry_print(ni)
	register struct netinfo *ni;
{
	if (ntohs(ni->rip_dst.sa_family) != AF_INET) {
		register int i;

		printf(" [family %d:", ntohs(ni->rip_dst.sa_family));
		for (i = 0; i < 14; i += 2)
			printf(" %02x%02x", ni->rip_dst.sa_data[i],
				ni->rip_dst.sa_data[i+1]);
		printf("]");
	} else {
		register struct sockaddr_in *sin = 
				(struct sockaddr_in *)&ni->rip_dst;
		printf(" %s", ipaddr_string(&sin->sin_addr));
		if (sin->sin_port)
			printf(" [port %d]", sin->sin_port);
	}
	printf("(%d)", ntohl(ni->rip_metric));
}

void
rip_print(dat, length)
	u_char *dat;
	int length;
{
	register struct rip *rp = (struct rip *)dat;
	register struct netinfo *ni;
	register int amt = (u_char *)snapend - dat;
	register int i = min(length, amt) -
			 (sizeof(struct rip) - sizeof(struct netinfo));
	int j;
	int trunc;
	
	if (i < 0)
		return;

	switch (rp->rip_cmd) {

	case RIPCMD_REQUEST:
		printf(" rip-req %d", length);
		break;
	case RIPCMD_RESPONSE:
		j = length / sizeof(*ni);
		if (j * sizeof(*ni) != length - 4)
			printf(" rip-resp %d[%d]:", j, length);
		else
			printf(" rip-resp %d:", j);
		trunc = ((i / sizeof(*ni)) * sizeof(*ni) != i);
		for (ni = rp->rip_nets; (i -= sizeof(*ni)) >= 0; ++ni)
			rip_entry_print(ni);
		if (trunc)
			printf("[|rip]");
		break;
	case RIPCMD_TRACEON:
		printf(" rip-traceon %d: \"%s\"", length, rp->rip_tracefile);
		break;
	case RIPCMD_TRACEOFF:
		printf(" rip-traceoff %d", length);
		break;
	case RIPCMD_POLL:
		printf(" rip-poll %d", length);
		break;
	case RIPCMD_POLLENTRY:
		printf(" rip-pollentry %d", length);
		break;
	default:
		printf(" rip-%d ?? %d", rp->rip_cmd, length);
		break;
	}
	if (rp->rip_vers != RIPVERSION)
		printf(" [vers %d]", rp->rip_vers);
}
