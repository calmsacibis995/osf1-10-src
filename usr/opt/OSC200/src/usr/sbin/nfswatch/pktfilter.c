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
static char *rcsid = "@(#)$RCSfile: pktfilter.c,v $ $Revision: 1.1.2.4 $ (DEC) $Date: 1993/12/21 23:35:50 $";
#endif
/* Based on:
 * /home/harbor/davy/system/nfswatch/RCS/pktfilter.c,v 4.0 1993/03/01 19:59:00 davy Exp $";
 */

#include "os.h"

/*
 * pktfilter.c - filters to count the packets.
 *
 * David A. Curry				Jeffrey C. Mogul
 * Purdue University				Digital Equipment Corporation
 * Engineering Computer Network			Western Research Laboratory
 * 1285 Electrical Engineering Building		250 University Avenue
 * West Lafayette, IN 47907-1285		Palo Alto, CA 94301
 * davy@ecn.purdue.edu				mogul@decwrl.dec.com
 *
 * log: pktfilter.c,v
 * Revision 4.0  1993/03/01  19:59:00  davy
 * NFSWATCH Version 4.0.
 *
 * Revision 3.7  1993/02/24  17:44:45  davy
 * Added -auth mode, changes to -proc mode, -map option, -server option.
 *
 * Revision 3.6  1993/01/16  19:08:59  davy
 * Corrected Jeff's address.
 *
 * Revision 3.5  1993/01/15  19:33:39  davy
 * Miscellaneous cleanups.
 *
 * Revision 3.4  1993/01/15  15:43:36  davy
 * Assorted changes for porting to Solaris 2.x/SVR4.
 *
 * Revision 3.3  1993/01/13  21:25:18  davy
 * Assorted IRIX porting changes.
 *
 * Revision 3.2  1993/01/13  20:18:17  davy
 * Put in OS-specific define scheme, and merged in Tim Hudson's code for
 * SGI systems (as yet untested).
 *
 * Revision 3.1  1992/07/24  18:47:57  mogul
 * Added FDDI support
 *
 * Revision 3.0  1991/01/23  08:23:17  davy
 * NFSWATCH Version 3.0.
 *
 * Revision 1.3  91/01/04  15:56:25  davy
 * Bug fix from Jeff Mogul.
 * 
 * Revision 1.2  90/08/17  15:47:42  davy
 * NFSWATCH Version 2.0.
 * 
 * Revision 1.1  88/11/29  11:20:49  davy
 * NFSWATCH Release 1.0
 * 
 */
#include <sys/param.h>
#include <sys/socket.h>
#include <net/if.h>
#ifndef ultrix
#include <net/if_arp.h>
#endif
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_var.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp_var.h>
#include <errno.h>
#include <stdio.h>

#include "nfswatch.h"
#include "externs.h"
#include "screen.h"

#ifdef SUNOS4
#define NND	1
#include <sun/ndio.h>
#endif

#ifdef ultrix
#include "ultrix.map.h"
#include "ipports.h"
#endif

#ifdef __alpha
#include "alphaosf.map.h"
#include "ipports.h"
#endif

#ifdef sgi
#include <rpc/types.h>
#include "sgi.map.h"
#include "ipports.h"
#endif

/*
 * Ethernet broadcast address.
 */
static	struct ether_addr ether_broadcast = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void pkt_dispatch();

/*
 * pkt_filter_ether - Parse Ethernet header,
 *			pass rest of packet to pkt_dispatch()
 */
void
pkt_filter_ether(cp, length, tstamp)
struct timeval *tstamp;
u_int length;
char *cp;
{
	int packet[PACKETSIZE];
	register int bdcst;
	struct ether_header eheader;

	/*
	 * Extract the ethernet header.
	 */
	(void) bcopy(cp, (char *) &eheader, sizeof(struct ether_header));
	(void) bcopy(cp + sizeof(struct ether_header), (char *) packet,
		(int) (length - sizeof(struct ether_header)));

	/*
	 * See if it's a broadcast packet.
	 */
#if defined(ultrix) || defined(sgi) || defined(__osf__)
	bdcst = !bcmp((char *) eheader.ether_dhost, (char *) &ether_broadcast,
			sizeof(struct ether_addr));
#else
	bdcst = !bcmp((char *) &eheader.ether_dhost, (char *) &ether_broadcast,
			sizeof(struct ether_addr));
#endif

	/*
	 * Figure out what kind of packet it is, and pass
	 * it off to the appropriate filter.
	 */
	pkt_dispatch(packet, length, bdcst, eheader.ether_type, tstamp);
}

#ifndef	LLC_SNAP_LSAP
#define LLC_UI		0x3
#define LLC_SNAP_LSAP	0xaa
#endif

/*
 * pkt_filter_fddi - Parse FDDI and LLC headers,
 *			pass rest of packet to pkt_dispatch()
 */
void
pkt_filter_fddi(cp, length, tstamp)
struct timeval *tstamp;
u_int length;
char *cp;
{
	int packet[PACKETSIZE];
	register int bdcst;
	struct fddi_header {
	        u_char	fddi_fc;
	        u_char	fddi_dhost[6];
	        u_char	fddi_shost[6];
	} fheader;
	struct llc_header {
		u_char	llc_dsap;
		u_char	llc_ssap;
		u_char	llc_control;
		u_char	llc_org_code[3];
		u_short	llc_ether_type;
	} lheader;
	u_short etype;

#define	FDDI_LLC_LEN (sizeof(struct fddi_header) + sizeof(struct llc_header))

	if (length <= FDDI_LLC_LEN)
	    return;	/* runt */

	/*
	 * Extract the FDDI and LLC headers.
	 */
	(void) bcopy(cp, (char *) &fheader, sizeof(struct fddi_header));
	(void) bcopy(cp + sizeof(struct fddi_header), (char *) &lheader,
			sizeof(struct llc_header));
	(void) bcopy(cp + FDDI_LLC_LEN, (char *) packet,
			(int) (length - FDDI_LLC_LEN));

	/* Should check FDDI frame control ... */

	/*
	 * See if it's a broadcast packet.
	 */
	bdcst = !bcmp((char *) fheader.fddi_dhost, (char *) &ether_broadcast,
			sizeof(struct ether_addr));

	/*
	 * Check LLC encapsulation type, extract Ethernet type if SNAP
	 */
	if ((lheader.llc_dsap == LLC_SNAP_LSAP)
		&& (lheader.llc_ssap == LLC_SNAP_LSAP)
		&& (lheader.llc_control == LLC_UI)
		&& (lheader.llc_org_code[0] == 0)
		&& (lheader.llc_org_code[1] == 0)
		&& (lheader.llc_org_code[2] == 0)) {
	    etype = lheader.llc_ether_type;
	}
	else
	    etype = 0;

	/*
	 * Figure out what kind of packet it is, and pass
	 * it off to the appropriate filter.
	 */
	pkt_dispatch(packet, length, bdcst, etype, tstamp);
}

/*
 * pkt_dispatch - count a packet, and pass it off to the appropriate filter.
 *		Caller tells us the Ethernet type code and if the packet
 *		was a broadcast.
 */
void
pkt_dispatch(packet, length, bdcst, etype, tstamp)
struct timeval *tstamp;
int *packet;			/* address of IP or ARP header */
u_int length;
int bdcst;			/* was packet a LAN broadcast? */
u_short etype;			/* still in network byte-order */
{
	struct ip *ip;
	struct ether_arp *arp;
	register int want;

	/*
	 * Count this packet in the network totals.
	 */
	int_pkt_total++;
	pkt_total++;

	/*
	 * See if it's a broadcast packet, and count it if it is.
	 */
	if (bdcst) {
		pkt_counters[PKT_BROADCAST].pc_interval++;
		pkt_counters[PKT_BROADCAST].pc_total++;
	}

	/*
	 * Figure out what kind of packet it is, and pass
	 * it off to the appropriate filter.
	 */
	switch (ntohs(etype)) {
	case ETHERTYPE_IP:		/* IP packet			*/
		ip = (struct ip *) packet;
		want = want_packet(ip->ip_src.s_addr, ip->ip_dst.s_addr);

		/*
		 * If we want this packet, count it in the host
		 * totals and pass it off.
		 */
		if (bdcst || want) {
			int_dst_pkt_total++;
			dst_pkt_total++;

			ip_filter(ip, length, ip->ip_src.s_addr,
				ip->ip_dst.s_addr, tstamp);
		}

		break;
	case ETHERTYPE_ARP:		/* Address Resolution Protocol	*/
		arp = (struct ether_arp *) packet;
		want = want_packet(arp->arp_spa, arp->arp_tpa);

		/*
		 * If we want this packet, count it in the host
		 * totals and then count it in the packet
		 * type counters.
		 */
		if (bdcst || want) {
			int_dst_pkt_total++;
			dst_pkt_total++;

			pkt_counters[PKT_ARP].pc_interval++;
			pkt_counters[PKT_ARP].pc_total++;
		}

		break;
	case ETHERTYPE_REVARP:		/* Reverse Addr Resol Protocol	*/
		arp = (struct ether_arp *) packet;
		want = want_packet(arp->arp_spa, arp->arp_tpa);

		/*
		 * If we want this packet, count it in the host
		 * totals and then count it in the packet
		 * type counters.
		 */
		if (bdcst || want) {
			int_dst_pkt_total++;
			dst_pkt_total++;

			pkt_counters[PKT_RARP].pc_interval++;
			pkt_counters[PKT_RARP].pc_total++;
		}

		break;
#ifdef notdef
	case ETHERTYPE_PUP:		/* Xerox PUP			*/
#endif
	default:			/* who knows...			*/
		int_dst_pkt_total++;
		dst_pkt_total++;

		pkt_counters[PKT_OTHER].pc_interval++;
		pkt_counters[PKT_OTHER].pc_total++;
		break;
	}
}

/*
 * ip_filter - strip off the IP header and pass off to the appropriate
 *	       filter.
 */
void
ip_filter(ip, length, src, dst, tstamp)
struct timeval *tstamp;
register struct ip *ip;
ipaddrt src, dst;
u_int length;
{
	register int *data;
	register int datalength;

	data = (int *) ip;
	data += ip->ip_hl;
	datalength = ntohs(ip->ip_len) - (4 * ip->ip_hl);

	/*
	 * Figure out what kind of IP packet this is, and
	 * pass it off to the appropriate filter.
	 */
	switch (ip->ip_p) {
	case IPPROTO_TCP:		/* transmission control protocol*/
		tcp_filter((struct tcphdr *) data, datalength,
			src, dst, tstamp);
		break;
	case IPPROTO_UDP:		/* user datagram protocol	*/
		udp_filter((struct udphdr *) data, datalength,
			src, dst, tstamp);
		break;
	case IPPROTO_ND:		/* Sun Network Disk protocol	*/
		nd_filter((char *) data, datalength, src, dst, tstamp);
		break;
	case IPPROTO_ICMP:		/* control message protocol	*/
		icmp_filter((struct icmp *) data, datalength,
			src, dst, tstamp);
		break;
#ifdef notdef
	case IPPROTO_IGMP:		/* group message protocol	*/
	case IPPROTO_GGP:		/* gateway-gateway protocol	*/
	case IPPROTO_EGP:		/* exterior gateway protocol	*/
	case IPPROTO_PUP:		/* Xerox pup protocol		*/
	case IPPROTO_IDP:		/* XNS IDP			*/
#endif
	default:			/* who knows...			*/
		break;
	}
}

/*
 * tcp_filter - count TCP packets.
 */
void
tcp_filter(tcp, length, src, dst, tstamp)
register struct tcphdr *tcp;
struct timeval *tstamp;
ipaddrt src, dst;
u_int length;
{
	/*
	 * Just count the packet.
	 */
	pkt_counters[PKT_TCP].pc_interval++;
	pkt_counters[PKT_TCP].pc_total++;
}

/*
 * udp_filter - count UDP packets, pass RPC packets to the RPC filter.
 */
void
udp_filter(udp, length, src, dst, tstamp)
register struct udphdr *udp;
struct timeval *tstamp;
ipaddrt src, dst;
u_int length;
{
	/*
	 * Count as a UDP packet.
	 */
	pkt_counters[PKT_UDP].pc_interval++;
	pkt_counters[PKT_UDP].pc_total++;

	/*
	 * See what type of packet it is.  Pass off
	 * anything we don't recognize to the RPC
	 * filter.
	 */
	switch (ntohs(udp->uh_sport)) {
	case IPPORT_ROUTESERVER:	/* routing control protocol	*/
		pkt_counters[PKT_ROUTING].pc_interval++;
		pkt_counters[PKT_ROUTING].pc_total++;
		break;
#ifdef notdef
					/* network standard functions	*/
	case IPPORT_ECHO:		/* packet echo server		*/
	case IPPORT_DISCARD:		/* packet discard server	*/
	case IPPORT_SYSTAT:		/* system stats			*/
	case IPPORT_DAYTIME:		/* time of day server		*/
	case IPPORT_NETSTAT:		/* network stats		*/
	case IPPORT_FTP:		/* file transfer		*/
	case IPPORT_TELNET:		/* remote terminal service	*/
	case IPPORT_SMTP:		/* simple mail transfer protocol*/
	case IPPORT_TIMESERVER:		/* network time synchronization	*/
	case IPPORT_NAMESERVER:		/* domain name lookup		*/
	case IPPORT_WHOIS:		/* white pages			*/
	case IPPORT_MTP:		/* ???				*/
					/* host specific functions	*/
	case IPPORT_TFTP:		/* trivial file transfer	*/
	case IPPORT_RJE:		/* remote job entry		*/
	case IPPORT_FINGER:		/* finger			*/
	case IPPORT_TTYLINK:		/* ???				*/
	case IPPORT_SUPDUP:		/* SUPDUP			*/
					/* UNIX TCP services		*/
	case IPPORT_EXECSERVER:		/* rsh				*/
	case IPPORT_LOGINSERVER:	/* rlogin			*/
	case IPPORT_CMDSERVER:		/* rcmd				*/
					/* UNIX UDP services		*/
	/* case IPPORT_BIFFUDP:		/* biff mail notification	*/
	/* case IPPORT_WHOSERVER:	/* rwho				*/
#endif
	default:			/* might be an RPC packet	*/
		rpc_filter((char *) udp + sizeof(struct udphdr),
			ntohs(udp->uh_ulen) - sizeof(struct udphdr),
			src, dst, tstamp);
		break;
	}
}

/*
 * nd_filter - count Sun ND packets.
 */
void
nd_filter(data, length, src, dst, tstamp)
struct timeval *tstamp;
ipaddrt src, dst;
u_int length;
char *data;
{
#ifdef SUNOS4
	register struct ndpack *nd;

	nd = (struct ndpack *) (data - sizeof(struct ip));

	/*
	 * Figure out whether it's a read or a write.
	 */
	switch (nd->np_op & NDOPCODE) {
	case NDOPREAD:
		pkt_counters[PKT_NDREAD].pc_interval++;
		pkt_counters[PKT_NDREAD].pc_total++;
		break;
	case NDOPWRITE:
		pkt_counters[PKT_NDWRITE].pc_interval++;
		pkt_counters[PKT_NDWRITE].pc_total++;
		break;
	case NDOPERROR:
	default:
		pkt_counters[PKT_OTHER].pc_interval++;
		pkt_counters[PKT_OTHER].pc_total++;
		break;
	}
#else /* SUNOS4 */
	pkt_counters[PKT_OTHER].pc_interval++;
	pkt_counters[PKT_OTHER].pc_total++;
#endif /* SUNOS4 */
}

/*
 * icmp_filter - count ICMP packets.
 */
void icmp_filter(icp, length, src, dst, tstamp)
register struct icmp *icp;
struct timeval *tstamp;
ipaddrt src, dst;
u_int length;
{
	pkt_counters[PKT_ICMP].pc_interval++;
	pkt_counters[PKT_ICMP].pc_total++;
}
