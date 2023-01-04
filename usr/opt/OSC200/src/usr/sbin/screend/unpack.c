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
static char *rcsid = "@(#)$RCSfile: unpack.c,v $ $Revision: 1.1.3.4 $ (DEC) $Date: 1992/12/11 17:47:08 $";
#endif

/*
 * unpack.c
 * Take an IP datagram header stack and "unpack" it into our canonical
 * representation.
 *
 * 21 December 1988	Jeffrey Mogul/DECWRL
 *	Created.
 *	Copyright 1989, 1990 Digital Equipment Corporation
 */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include "screentab.h"

#define	IPOFF_MASK	(0x1FFF)	/* should be in <netinet/in.h> */

extern int debug;

/*
 * NOTE:
 *	Before this packet left the kernel, ipintr() in netinet/ip_input.c
 *	had already converted ip_off, ip_len, and ip_id into HOST order.
 */

UnpackIP(bufp, buflen, unpp)
register char *bufp;
register int buflen;
register struct unpacked_hdrs *unpp;
{
	register struct ip *ipp;
	struct tcphdr *tcpp;
	struct udphdr *udpp;
	struct icmp *icmpp;
	register int remlen;
	register char *ipdata;

	remlen = buflen - sizeof(*ipp);
	if (remlen < 0) {
	    if (debug)
	    	fprintf(stderr, "Runt, len %d\n", buflen);
	    return(0);
	}
	ipp = (struct ip *)bufp;
	ipdata = &(bufp[ipp->ip_hl * sizeof(int)]);

	unpp->src.addr = ipp->ip_src;
	unpp->dst.addr = ipp->ip_dst;
	unpp->proto = ipp->ip_p;

	/*
	 * Sleazy kludge to avoid being hacked via source routes;
	 * lazy assumption that any packet with options might have
	 * a source route, and that if one cares about source routes
	 * then one cares about the destination, so pretending that
	 * the packet destination is 255.255.255.255 (broadcast) will
	 * be a conservative estimate.
	 */
	if (ipp->ip_hl != 5) {	/* IP header options present */
	    /* Should parse options and extract info from any source route */
	    unpp->dst.addr.s_addr = 0xFFFFFFFF;
	}

	if (ipp->ip_off & IPOFF_MASK) {
	    /* do we have to look for fragment leader */
	    switch (ipp->ip_p) {
	    case IPPROTO_UDP:
	    case IPPROTO_TCP:
	    case IPPROTO_ICMP:
		/*
		 * For these protocols, we parse into the "ip data"
		 * segment, so we need information out of fragment
		 * 0.
		 */
		return(FindFragLeader(ipp, unpp));
		 
	    default:
		/* Other protocols: all fragments have enough info */
		break;		/* drop through */
	    }
	}
	
	switch (ipp->ip_p) {
	case IPPROTO_UDP:
	    if (remlen < sizeof(*udpp)) {
		if (debug)
		    fprintf(stderr, "Runt UDP header (%d short)\n",
				sizeof(*udpp) - remlen);
		return(0);
	    }
	    udpp = (struct udphdr *)ipdata;
	    unpp->src.port = udpp->uh_sport;
	    unpp->dst.port = udpp->uh_dport;
	    break;
	case IPPROTO_TCP:
	    if (remlen < sizeof(*tcpp)) {
		if (debug)
		    fprintf(stderr, "Runt TCP header (%d short)\n",
			sizeof(*tcpp) - remlen);
		return(0);
	    }
	    tcpp = (struct tcphdr *)ipdata;
	    unpp->src.port = tcpp->th_sport;
	    unpp->dst.port = tcpp->th_dport;
	    break;
	/* known but not handled protocols: */
	case IPPROTO_ICMP:
	    if (remlen < sizeof(*icmpp)) {
		if (debug)
		    fprintf(stderr, "Runt ICMP header (%d short)\n",
			sizeof(*icmpp) - remlen);
		return(0);
	    }
	    icmpp = (struct icmp *)ipdata;
	    unpp->src.port = icmpp->icmp_type;
	    unpp->dst.port = icmpp->icmp_type;
	    break;
	default:
	    break;
	}

	/* See if we expect more fragments */
	if (ipp->ip_off & IP_MF) {
	    /*
	     * This is the first of several fragments (we already
	     * know the offset is zero) so remember it if this is
	     * a protocol that needs the info.
	     */
	    switch (ipp->ip_p) {
	    case IPPROTO_UDP:
	    case IPPROTO_TCP:
	    case IPPROTO_ICMP:
		RecordFragLeader(ipp, unpp);
		break;
	    default:
		/* nothing worth saving for this protocol */
		break;
	    }
	}

	return(1);
}

PrintAnnotatedHdrs(ahp)
register struct annotated_hdrs *ahp;
{
	char *ProtoNumberToName();
	char *protoname;

	printf("[%s]", inet_ntoa(ahp->hdrs.src.addr));
	if (ahp->srcnote.net.s_addr) {
	    printf(" net %s", inet_ntoa(ahp->srcnote.net));
	}
	if (ahp->srcnote.subnet.s_addr) {
	    printf(" subnet %s", inet_ntoa(ahp->srcnote.subnet));
	}
	printf("->");
	printf("[%s]", inet_ntoa(ahp->hdrs.dst.addr));
	if (ahp->dstnote.net.s_addr) {
	    printf(" net %s", inet_ntoa(ahp->dstnote.net));
	}
	if (ahp->dstnote.subnet.s_addr) {
	    printf(" subnet %s", inet_ntoa(ahp->dstnote.subnet));
	}
	
	protoname = ProtoNumberToName(ahp->hdrs.proto);
	if (protoname) {
	    printf(" %s", protoname);
	}
	else
	    printf(" proto %d", ahp->hdrs.proto);

	switch (ahp->hdrs.proto) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
		printf(" (%d->%d)",
			ntohs(ahp->hdrs.src.port), ntohs(ahp->hdrs.dst.port));
		break;
	case IPPROTO_ICMP:
		printf(" (%d)", ahp->hdrs.src.port);
		break;
	default:
		break;
	}
}

PrintUnpackedHdrs(uhp)
register struct unpacked_hdrs *uhp;
{
	char *ProtoNumberToName();
	char *protoname;

	printf("[%s]", inet_ntoa(uhp->src.addr));
	printf("->");
	printf("[%s]", inet_ntoa(uhp->dst.addr));

	protoname = ProtoNumberToName(uhp->proto);
	if (protoname) {
	    printf(" %s", protoname);
	}
	else
	    printf(" proto %d", uhp->proto);

	switch (uhp->proto) {
	case IPPROTO_UDP:
	case IPPROTO_TCP:
		printf(" (%d->%d)",
			ntohs(uhp->src.port), ntohs(uhp->dst.port));
		break;
	case IPPROTO_ICMP:
		printf(" (%d)", uhp->src.port);
		break;
	default:
		break;
	}
}
