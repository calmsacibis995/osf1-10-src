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
static char *rcsid = "@(#)$RCSfile: printsubs.c,v $ $Revision: 1.1.3.3 $ (DEC) $Date: 1992/12/11 17:46:07 $";
#endif

/*
 * printsubs.c
 * Debugging printout functions
 *
 * 19 December 1988	Jeffrey Mogul/DECWRL
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

char *inet_ntoa();

PrintNetmaskData(nmp)
register struct NetmaskData *nmp;
{
	printf("NetmaskData: mask for %s", inet_ntoa(nmp->network));
	printf(" is %s\n", inet_ntoa(nmp->mask));
}

PrintActionSpec(ap)
register struct ActionSpec *ap;
{
	printf("from [");
	PrintObjectSpec(&(ap->from));
	printf("] to [");
	PrintObjectSpec(&(ap->to));
	printf("]");
	if (ap->action & ASACTION_ACCEPT)
		printf(" accept");
	else 
		printf(" reject");
	if (ap->action & ASACTION_NOTIFY)
		printf(" notify");
	if (ap->action & ASACTION_LOG)
		printf(" log");
	printf("\n");
}

PrintObjectSpec(osp)
register struct ObjectSpec *osp;
{
	PrintAddrSpec(&(osp->aspec), osp->flags);
	printf("/");
	PrintPortSpec(&(osp->pspec), osp->flags);
}

PrintAddrSpec(asp, flags)
register struct AddrSpec *asp;
int flags;
{
	if (flags & OSF_NOTADDR)
	    printf("not ");
	switch (asp->addrtype) {
	case ASAT_ANY:
		printf("any host");
		break;
	case ASAT_NET:
		printf("net %s", inet_ntoa(asp->aval.network));
		break;
	case ASAT_SUBNET:
		printf("subnet %s", inet_ntoa(asp->aval.subnet));
		break;
	case ASAT_HOST:
		printf("host %s", inet_ntoa(asp->aval.host));
		break;
	default:
		printf("bad addrtype");
		break;
	}
}

PrintPortSpec(psp, flags)
register struct PortSpec *psp;
int flags;
{
	char *ICMPNumberToName();
	char *icmpname;

	if (flags & OSF_NOTPROTO)
	    printf("not ");
	switch (psp->proto) {
	case IPPROTO_UDP:
		printf("UDP ");
		if (flags & OSF_NOTPORT)
		    printf("not ");
		PortValPrint(psp->pval.port);
		break;
	case IPPROTO_TCP:
		printf("TCP ");
		if (flags & OSF_NOTPORT)
		    printf("not ");
		PortValPrint(psp->pval.port);
		break;
	case IPPROTO_ICMP:
		if (flags & OSF_NOTPORT)
		    printf("not ");
		if (psp->pval.code == ICMPV_ANY) {
		    printf("any ICMP");
		    break;
		}
		if (psp->pval.code == ICMPV_INFOTYPE) {
		    printf("any `informational' ICMP");
		    break;
		}
		icmpname = ICMPNumberToName(psp->pval.code);
		if (icmpname)
		    printf("ICMP %s", icmpname);
		else
		    printf("ICMP code %d", psp->pval.code);
		break;
	case 0:
		printf("any proto");
		if (psp->pval.port.discrim != PORTV_ANY)
		    PortValPrint(psp->pval.port);
		break;
	default:
		printf("proto %d", psp->proto);
		break;
	}
}

PortValPrint(pval)
struct PortValue pval;
{
	char *PortNumberToName();
	char *portnumber;

	switch (pval.discrim) {
	case PORTV_ANY:
	    printf("any port");
	    break;

	case PORTV_RESERVED:
	    printf("any reserved port");
	    break;

	case PORTV_XSERVER:
	    printf("any xserver port");
	    break;

	case PORTV_EXACT:
	    portnumber = PortNumberToName(pval.value);
	    if (portnumber) {
		printf("%s", portnumber);
	    }
	    else
		printf("port %d", ntohs(pval.value));
	    break;

	default:
	    printf("!bogus port value!");
	    break;
	}
}


/*
 * Prints an IP header
 */
PrintIPHeader(tvp, bufp, len)
struct timeval *tvp;
char *bufp;
int len;
{
	struct ip *ipp;
	struct tcphdr *tcpp;
	struct udphdr *udpp;
	int remlen;
	char *ipdata;

	remlen = len - sizeof(*ipp);
	if (remlen < 0) {
	    printf("Runt, len %d\n", len);
	    return;
	}
	ipp = (struct ip *)bufp;
	ipdata = &(bufp[sizeof(*ipp)]);
	if (ipp->ip_hl != 5) {
	    printf("Options present, hl %d ", ipp->ip_hl);
	}
	if (ipp->ip_off) {
	    printf("Not first frag, off %d\n", ntohs(ipp->ip_off));
	    return;
	}
	
	/* looks like a keeper? */
	printf("(%d.%06d) ",
		tvp->tv_sec,tvp->tv_usec);
	printf("[%s] -> ", inet_ntoa(ipp->ip_src));
	printf("[%s]", inet_ntoa(ipp->ip_dst));

	switch (ipp->ip_p) {
	case IPPROTO_UDP:
	    if (remlen < sizeof(*udpp)) {
		printf("Runt UDP header (%d short)\n",
			sizeof(*udpp) - remlen);
		return;
	    }
	    udpp = (struct udphdr *)ipdata;
	    printf(" UDP (%d->%d)\n",
			ntohs(udpp->uh_sport), ntohs(udpp->uh_dport));
	    break;
	case IPPROTO_TCP:
	    if (remlen < sizeof(*tcpp)) {
		printf("Runt TCP header (%d short)\n",
			sizeof(*tcpp) - remlen);
		return;
	    }
	    tcpp = (struct tcphdr *)ipdata;
	    printf(" TCP (%d->%d)\n",
			ntohs(tcpp->th_sport), ntohs(tcpp->th_dport));
	    break;
	/* known but not handled protocols: */
	case IPPROTO_ICMP:
	    printf(" ICMP");
	    break;
	default:
	    printf(" Unknown protocol %d", ipp->ip_p);
	}
	printf("\n");
}

/*
 * Formats an IP header into a string
 */
FormatIPHeader(bufp, len, msgbuf)
char *bufp;
int len;
char *msgbuf;
{
	struct ip *ipp;
	struct tcphdr *tcpp;
	struct udphdr *udpp;
	int remlen;
	char *ipdata;
	static char srcstr[32];
	static char opstr[64];
	struct icmp *icmpp;
	char *icmpname;
	char *ICMPNumberToAbbrev();

	remlen = len - sizeof(*ipp);
	if (remlen < 0) {
	    sprintf(msgbuf, "Runt, len %d", len);
	    return;
	}
	ipp = (struct ip *)bufp;
	ipdata = &(bufp[sizeof(*ipp)]);
	if (ipp->ip_hl != 5) {
	    sprintf(opstr, " Options present, hl %d", ipp->ip_hl);
	}
	else
	    sprintf(opstr, "");
	if (ipp->ip_off) {
	    sprintf(msgbuf, "Not first frag, off %d", ntohs(ipp->ip_off));
	    return;
	}
	
	/* looks like a keeper? */
	sprintf(srcstr, "%s", inet_ntoa(ipp->ip_src));

	switch (ipp->ip_p) {
	case IPPROTO_UDP:
	    if (remlen < sizeof(*udpp)) {
		sprintf(msgbuf, "Runt UDP header (%d short)",
			sizeof(*udpp) - remlen);
		return;
	    }
	    udpp = (struct udphdr *)ipdata;
	    sprintf(msgbuf,
		"UDP [%s]->[%s](%d->%d)%s", srcstr, inet_ntoa(ipp->ip_dst),
			ntohs(udpp->uh_sport), ntohs(udpp->uh_dport),
			opstr);
	    break;
	case IPPROTO_TCP:
	    if (remlen < sizeof(*tcpp)) {
		sprintf(msgbuf, "Runt TCP header (%d short)",
			sizeof(*tcpp) - remlen);
		return;
	    }
	    tcpp = (struct tcphdr *)ipdata;
	    sprintf(msgbuf,
		"TCP [%s]->[%s](%d->%d)%s", srcstr, inet_ntoa(ipp->ip_dst),
			ntohs(tcpp->th_sport), ntohs(tcpp->th_dport),
			opstr);
	    break;
	/* known but not handled protocols: */
	case IPPROTO_ICMP:
	    if (remlen < sizeof(*icmpp)) {
		sprintf(msgbuf, "Runt TCP header (%d short)",
			sizeof(*icmpp) - remlen);
		return;
	    }
	    icmpp = (struct icmp *)ipdata;
	    icmpname = ICMPNumberToAbbrev(icmpp->icmp_type);
	    if (icmpname)
	        sprintf(msgbuf, "ICMP %s [%s]->[%s]%s", icmpname,
			srcstr, inet_ntoa(ipp->ip_dst), opstr
			);
	    else
	        sprintf(msgbuf, "ICMP code-%d [%s]->[%s]%s", icmpp->icmp_type,
			srcstr, inet_ntoa(ipp->ip_dst), opstr
			);
	    break;
	default:
	    sprintf(msgbuf, "Unknown proto %d [%s]->[%s]%s", ipp->ip_p,
			srcstr, inet_ntoa(ipp->ip_dst), opstr
			);
	}
}

/*
 * Formats an "unpacked" IP header into a string
 */
FormatUnpackedIPHeader(uhp, msgbuf)
struct unpacked_hdrs *uhp;
char *msgbuf;
{
	struct ip *ipp;
	struct tcphdr *tcpp;
	struct udphdr *udpp;
	int remlen;
	char *ipdata;
	static char srcstr[32];
	struct icmp *icmpp;
	char *icmpname;
	char *ICMPNumberToAbbrev();

	sprintf(srcstr, "%s", inet_ntoa(uhp->src.addr));

	switch (uhp->proto) {
	case IPPROTO_UDP:
	    sprintf(msgbuf,
		"UDP [%s]->[%s](%d->%d)", srcstr, inet_ntoa(uhp->dst.addr),
			ntohs(uhp->src.port), ntohs(uhp->dst.port));
	    break;
	case IPPROTO_TCP:
	    sprintf(msgbuf,
		"TCP [%s]->[%s](%d->%d)", srcstr, inet_ntoa(uhp->dst.addr),
			ntohs(uhp->src.port), ntohs(uhp->dst.port));
	    break;
	/* known but not handled protocols: */
	case IPPROTO_ICMP:
	    icmpname = ICMPNumberToAbbrev(uhp->src.port);
	    if (icmpname)
	        sprintf(msgbuf, "ICMP %s [%s]->[%s]", icmpname,
			srcstr, inet_ntoa(uhp->dst.addr)
			);
	    else
	        sprintf(msgbuf, "ICMP code-%d [%s]->[%s]", uhp->src.port,
			srcstr, inet_ntoa(uhp->dst.addr)
			);
	    break;
	default:
	    sprintf(msgbuf, "Unknown proto %d [%s]->[%s]", uhp->proto,
			srcstr, inet_ntoa(uhp->dst.addr)
			);
	}
}
