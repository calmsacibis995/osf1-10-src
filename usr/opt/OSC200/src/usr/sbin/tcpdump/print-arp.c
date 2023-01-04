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
static char *rcsid = "@(#)$RCSfile: print-arp.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 18:43:04 $";
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
 * static char rcsid[] = "print-arp.c,v 1.17 92/06/16 21:36:54 leres Exp $ (LBL)";
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>

#include "interface.h"
#include "addrtoname.h"

static u_char ezero[6];

void
arp_print(ap, length, caplen)
	register struct ether_arp *ap;
	int length;
	int caplen;
{
	register struct ether_header *eh;

	if ((u_char *)(ap + 1) > snapend) {
		printf("[|arp]");
		return;
	}
	if (length < sizeof(struct ether_arp)) {
		(void)printf("truncated-arp");
		default_print((u_short *)ap, length);
		return;
	}

	NTOHS(ap->arp_hrd);
	NTOHS(ap->arp_pro);
	NTOHS(ap->arp_op);

	if (ap->arp_hrd != ARPHRD_ETHER
	    || (ap->arp_pro != ETHERTYPE_IP
		&& ap->arp_pro != ETHERTYPE_TRAIL)
	    || ap->arp_hln != sizeof(SHA(ap))
	    || ap->arp_pln != sizeof(SPA(ap))) {
		(void)printf("arp-req #%d for proto #%d (%d) hardware %d (%d)",
				ap->arp_op, ap->arp_pro, ap->arp_pln,
				ap->arp_hrd, ap->arp_hln);
		return;
	}
	if (ap->arp_pro == ETHERTYPE_TRAIL)
		(void)printf("trailer");
	eh = (struct ether_header *)packetp;
	switch (ap->arp_op) {

	case ARPOP_REQUEST:
		(void)printf("arp who-has %s", ipaddr_string(TPA(ap)));
		if (bcmp(ezero, THA(ap), 6) != 0)
			(void)printf(" (%s)", etheraddr_string(THA(ap)));
		(void)printf(" tell %s", ipaddr_string(SPA(ap)));
		if (bcmp(ESRC(eh), SHA(ap), 6) != 0)
			(void)printf(" (%s)", etheraddr_string(SHA(ap)));
		break;

	case ARPOP_REPLY:
		(void)printf("arp reply %s", ipaddr_string(SPA(ap)));
		if (bcmp(ESRC(eh), SHA(ap), 6) != 0)
			(void)printf(" (%s)", etheraddr_string(SHA(ap)));
		(void)printf(" is-at %s", etheraddr_string(SHA(ap)));
		if (bcmp(EDST(eh), THA(ap), 6) != 0)
			(void)printf(" (%s)", etheraddr_string(THA(ap)));
		break;
 
	case REVARP_REQUEST:
		(void)printf("rarp who-is %s tell %s",
			etheraddr_string(THA(ap)),
			etheraddr_string(SHA(ap)));
		break;

	case REVARP_REPLY:
		(void)printf("rarp reply %s at %s",
			etheraddr_string(THA(ap)),
			ipaddr_string(TPA(ap)));
		break;

	default:
		(void)printf("arp-%d", ap->arp_op);
		default_print((u_short *)ap, caplen);
		break;
	}
}
