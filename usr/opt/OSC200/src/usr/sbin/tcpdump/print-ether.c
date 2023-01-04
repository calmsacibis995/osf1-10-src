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
static char *rcsid = "@(#)$RCSfile: print-ether.c,v $ $Revision: 1.1.2.2 $ (DEC) $Date: 1993/06/22 18:44:09 $";
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
 * static char rcsid[] = "print-ether.c,v 1.6 92/07/01 17:49:49 mogul Exp $ (LBL)";
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>

#include "interface.h"
#include "addrtoname.h"
#include "etherproto.h"

u_char *packetp;
u_char *snapend;

static inline void
ether_print(ep, length)
	register struct ether_header *ep;
	int length;
{
	if (qflag)
		(void)printf("%s %s %d: ",
			     etheraddr_string(ESRC(ep)),
			     etheraddr_string(EDST(ep)),
			     length);
	else
		(void)printf("%s %s %s %d: ",
			     etheraddr_string(ESRC(ep)),
			     etheraddr_string(EDST(ep)),
			     etherproto_string(ep->ether_type), 
			     length);
}

/*
 * This is the top level routine of the printer.  'p' is the points
 * to the ether header of the packet, 'tvp' is the timestamp, 
 * 'length' is the length of the packet off the wire, and 'caplen'
 * is the number of bytes actually captured.
 */
void
ether_if_print(p, tvp, length, caplen)
	u_char *p;
	struct timeval *tvp;
	int length;
	int caplen;
{
	struct ether_header *ep;
	register int i;
	u_short ether_type;
	extern u_short extracted_ethertype;

	ts_print(tvp);

	if (caplen < sizeof(struct ether_header)) {
		printf("[|ether]");
		goto out;
	}

	if (eflag)
		ether_print((struct ether_header *)p, length);

	/*
	 * Some printers want to get back at the ethernet addresses,
	 * and/or check that they're not walking off the end of the packet.
	 * Rather than pass them all the way down, we set these globals.
	 */
	packetp = p;
	snapend = p + caplen;
	
	length -= sizeof(struct ether_header);
	caplen -= sizeof(struct ether_header);
	ep = (struct ether_header *)p;
	p += sizeof(struct ether_header);

	ether_type = ntohs(ep->ether_type);

	/*
	 * Is it (gag) an 802.3 encapsulation?
	 */
	extracted_ethertype = 0;
	if (ether_type < ETHERMTU) {
	    /* Try to print the LLC-layer header & higher layers */
	    if (llc_print(p, length, caplen, ESRC(ep), EDST(ep)) == 0) {
		/* ether_type not known, print raw packet */
		if (!eflag)
			ether_print(ep, length);
		if (extracted_ethertype) {
			printf("(LLC %s) ",
				etherproto_string(htons(extracted_ethertype)));
		}
		if (!xflag && !qflag)
			default_print((u_short *)p, caplen);
	    }
	}
	else if (ether_encap_print(ether_type, p, length, caplen) == 0) {
		/* ether_type not known, print raw packet */
		if (!eflag)
			ether_print(ep, length);
		if (!xflag && !qflag)
			default_print((u_short *)p, caplen);
	}

	if (xflag)
		default_print((u_short *)p, caplen);
 out:
	putchar('\n');
}

/*
 * Prints the packet encapsulated in an Ethernet data segment
 * (or an equivalent encapsulation), given the Ethernet type code.
 *
 * Returns non-zero if it can do so, zero if the ethertype is unknown.
 *
 * Hides the ether type in a global for the benefit of lower layers
 * that might want to know what it is.
 */

u_short	extracted_ethertype;

ether_encap_print(ethertype, p, length, caplen)
u_short ethertype;		/* in host byte-order */
u_char *p;			/* ptr to encapsulated packet */
int length;			/* length of encapsulated packet */
int caplen;			/* remaining length actually captured */
{
	extracted_ethertype = ethertype;

	switch (ethertype) {

	case ETHERTYPE_IP:
		ip_print((struct ip *)p, length);
		return(1);

	case ETHERTYPE_ARP:
	case ETHERTYPE_REVARP:
		arp_print((struct ether_arp *)p, length, caplen);
		return(1);

	case ETHERTYPE_DN:
		decnet_print(p, length, caplen);
		return(1);

	case ETHERTYPE_LAT:
	case ETHERTYPE_MOPRC:
	case ETHERTYPE_MOPDL:
		/* default_print for now */
	default:
		return(0);
	}
}

