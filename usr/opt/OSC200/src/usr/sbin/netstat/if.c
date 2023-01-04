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
static char	*sccsid = "@(#)$RCSfile: if.c,v $ $Revision: 4.2.7.3 $ (DEC) $Date: 1993/09/29 19:54:51 $";
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
 * if.c
 *
 *	Modification History:
 *
 * 01-Apr-91	Fred Canter
 *	MIPS C 2.20+, changes for -std
 *
 */
/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
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

#ifndef lint


#endif /* not lint */

#include <sys/types.h>
#define	_SOCKADDR_LEN
#include <sys/socket.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/if_ether.h>
#include <netns/ns.h>
#include <netns/ns_if.h>

#include <stdio.h>
#include <strings.h>
#include <signal.h>

#define	YES	1
#define	NO	0

extern	int kmem;
extern	int tflag;
extern	int dflag;
extern	int nflag;
extern	int aflag;
extern	char *interface;
extern	int unit;
extern	char *routename(), *netname(), *ns_phost(), *etherprint();

/*
 * Print a description of the network interfaces.
 */
intpr(interval, ifnetaddr)
	int interval;
	off_t ifnetaddr;
{
	struct ifnet ifnet;
	union {
		struct ifaddr ifa;
		struct in_ifaddr in;
		struct ns_ifaddr ns;
	} ifaddr;
	off_t ifaddraddr;
	struct sockaddr sa;
	char name[16];
	off_t ifaddrfound = 0;
	off_t ifnetfound = 0;
	int found=0;

	if (ifnetaddr == 0) {
		printf("ifnet: symbol not defined\n");
		return;
	}
	if (interval) {
		sidewaysintpr((unsigned)interval, ifnetaddr);
		return;
	}
	klseek(kmem, ifnetaddr, 0);
	read(kmem, (char *)&ifnetaddr, sizeof ifnetaddr);
	ifaddraddr = 0;
	while (ifnetaddr || ifaddraddr) {
		struct sockaddr_in *sin;
		register char *cp;
		int n, m;
		struct in_addr inet_makeaddr();

		ifnetfound = ifnetaddr;
		if (ifaddraddr == 0) {
			klseek(kmem, ifnetaddr, 0);
			read(kmem, (char *)&ifnet, sizeof ifnet);
			klseek(kmem, (off_t)ifnet.if_name, 0);
			read(kmem, name, 16);
			name[15] = '\0';
			ifnetaddr = (off_t) ifnet.if_next;
			if (interface != 0 &&
			    (strcmp(name, interface) != 0 || unit != ifnet.if_unit))
				continue;
			cp = index(name, '\0');
			*cp++ = ifnet.if_unit + '0';
			if ((ifnet.if_flags&IFF_UP) == 0)
				*cp++ = '*';
			*cp = '\0';
			ifaddraddr = (off_t)ifnet.if_addrlist;
		}
		if (found == 0) {
			found = 1;
			printf("%-5.5s %-5.5s %-11.11s %-15.15s %8.8s %5.5s %8.8s %5.5s",
			"Name", "Mtu", "Network", "Address", "Ipkts", "Ierrs", "Opkts", "Oerrs");
			printf(" %5s", "Coll");
			if (tflag)
				printf(" %s", "Time");
			if (dflag)
				printf(" %s", "Drop");
			putchar('\n');
		}
		printf("%-5.5s %-5d ", name, ifnet.if_mtu);
		ifaddrfound = ifaddraddr;
		if (ifaddraddr == 0) {
			printf("%-11.11s ", "none");
			printf("%-15.15s ", "none");
		} else {
			klseek(kmem, ifaddraddr, 0);
			read(kmem, (char *)&ifaddr, sizeof ifaddr);
			klseek(kmem, ifaddr.ifa.ifa_addr, 0);
			read(kmem, (char *)&sa, sizeof sa);
			switch (sa.sa_family) {
			case AF_UNSPEC:
				printf("%-11.11s ", "none");
				printf("%-15.15s ", "none");
				break;
			case AF_INET:
				sin = (struct sockaddr_in *)&sa;
#ifdef notdef
				/* can't use inet_makeaddr because kernel
				 * keeps nets unshifted.
				 */
				in = inet_makeaddr(ifaddr.in.ia_subnet,
					INADDR_ANY);
				printf("%-11.11s ", netname(in));
#else
				printf("%-11.11s ",
					netname(htonl(ifaddr.in.ia_subnet),
						ifaddr.in.ia_subnetmask));
#endif
				printf("%-15.15s ", routename(sin->sin_addr));
				break;
			case AF_NS:
				{
				struct sockaddr_ns *sns =
					(struct sockaddr_ns *)&sa;
				u_int net;
				char netnum[8];
				char *ns_phost();

				*(union ns_net *) &net = sns->sns_addr.x_net;
		sprintf(netnum, "%lxH", ntohl(net));
				upHex(netnum);
				printf("ns:%-8s ", netnum);
				printf("%-15s ", ns_phost(sns));
				}
				break;
			case AF_OSI:
				{
				 printf("%-12.12s%-15.15s ","OSI","none");
				 break;
				}
			case AF_LAT:
				{
				 printf("%-12.12s%-15.15s ","LAT","none");
				 break;
				}
			case AF_DLI:
				{
				printf("%-12.12s%-15.15s ","DLI","none");
				break;
				}
			case AF_LINK:
				{
				struct sockaddr_dl *sdl =
					(struct sockaddr_dl *)&sa;
				    cp = (char *)LLADDR(sdl);
				    n = sdl->sdl_alen;
				}
				m = printf("<Link>");
				goto hexprint;
			default:
				m = printf("(%d)", sa.sa_family);
				for (cp = sa.sa_len + (char *)&sa;
					--cp > sa.sa_data && (*cp == 0);) {}
				n = cp - sa.sa_data + 1;
				cp = sa.sa_data;
			hexprint:
				while (--n >= 0)
					m += printf("%x%c", *cp++ & 0xff,
						    n > 0 ? '.' : ' ');
				m = 28 - m;
				while (m-- > 0)
					putchar(' ');
				break;
			}
			ifaddraddr = (off_t)ifaddr.ifa.ifa_next;
		}
		printf("%8d %5d %8d %5d %5d",
		    ifnet.if_ipackets, ifnet.if_ierrors,
		    ifnet.if_opackets, ifnet.if_oerrors,
		    ifnet.if_collisions);
		if (tflag)
			printf(" %3d", ifnet.if_timer);
		if (dflag)
			printf(" %3d", ifnet.if_snd.ifq_drops);
		putchar('\n');

		if (aflag && ifaddrfound) {
			/*
			 * print any internet multicast addresses
			 */
			switch (sa.sa_family) {
			case AF_INET:
			    {
				off_t multiaddr;
				struct in_multi inm;

				multiaddr = (off_t)ifaddr.in.ia_multiaddrs;
				while (multiaddr != 0) {
					klseek(kmem, multiaddr, 0);
					read(kmem, (char *)&inm, sizeof inm);
					multiaddr = (off_t)inm.inm_next;
					printf("%23s %-19.19s\n", "",
						routename(inm.inm_addr));
				}
				break;
			    }
			default:
				break;
			}
		}
		if (aflag && ifaddraddr == 0) {
			/*
			 * print link-level addresses
			 */
			if (ifnet.if_flags & IFF_MULTICAST) {
				off_t multiaddr;
				struct arpcom ac;
				struct ifmulti ifm;

				klseek(kmem, ifnetfound, 0);
				read(kmem, (char *)&ac, sizeof ac);
				printf("%23s %s\n", "",
					etherprint(ac.ac_enaddr));
				multiaddr = (off_t)ifnet.if_multiaddrs;
				while (multiaddr != 0) {
					klseek(kmem, multiaddr, 0);
					read(kmem, (char *)&ifm, sizeof ifm);
					multiaddr = (off_t)ifm.ifm_next;
					printf("%23s %s", "",
						etherprint(ifm.ifm_addrlo));
					if (bcmp(ifm.ifm_addrlo,
						 ifm.ifm_addrhi, 6) != 0)
						printf(" to %s",
						etherprint(ifm.ifm_addrhi));
					printf("\n");
				}
			}
		}
	}
	if (!found)
		printf("%s%d: No such device\n", interface, unit);
}

#define	MAXIF	64
struct	iftot {
	char	ift_name[16];		/* interface name */
	int	ift_ip;			/* input packets */
	int	ift_ie;			/* input errors */
	int	ift_op;			/* output packets */
	int	ift_oe;			/* output errors */
	int	ift_co;			/* collisions */
	int	ift_dr;			/* drops */
} iftot[MAXIF];

u_char	signalled;			/* set if alarm goes off "early" */

/*
 * Called if an interval expires before sidewaysintpr has completed a loop.
 * Sets a flag to not wait for the alarm.
 */
void catchalarm()
{
	signalled = YES;
}

/*
 * Print a running summary of interface statistics.
 * Repeat display every interval seconds, showing statistics
 * collected over that interval.  Assumes that interval is non-zero.
 * First line printed at top of screen is always cumulative.
 */
sidewaysintpr(interval, off)
	unsigned interval;
	off_t off;
{
	void catchalarm();
	struct ifnet ifnet;
	off_t firstifnet;
	register struct iftot *ip, *total;
	register int line;
	struct iftot *lastif, *sum, *interesting;
	int oldmask;
	int found=0;

	klseek(kmem, off, 0);
	read(kmem, (char *)&firstifnet, sizeof (off_t));
	lastif = iftot;
	sum = iftot + MAXIF - 1;
	total = sum - 1;
	interesting = iftot;
	for (off = firstifnet, ip = iftot; off;) {
		char *cp;

		klseek(kmem, off, 0);
		read(kmem, (char *)&ifnet, sizeof ifnet);
		klseek(kmem, (off_t)ifnet.if_name, 0);
		ip->ift_name[0] = '(';
		read(kmem, ip->ift_name + 1, 15);
		if (interface) {
		    if (!found && strcmp(ip->ift_name + 1, interface) == 0 &&
		    	unit == ifnet.if_unit) {
			interesting = ip;
			found = 1;
		    }
		} else 
		    if (!found && (ifnet.if_flags & IFF_UP) && 
			(ifnet.if_flags & IFF_RUNNING)) {
				interesting = ip;
				found = 1;
			}
		ip->ift_name[15] = '\0';
		cp = index(ip->ift_name, '\0');
		sprintf(cp, "%d)", ifnet.if_unit);
		ip++;
		if (ip >= iftot + MAXIF - 2)
			break;
		off = (off_t) ifnet.if_next;
	}
	if (interface && !found ) {
		printf("%s%d: No such device\n", interface, unit);
		return;
	}
	lastif = ip;

	(void)signal(SIGALRM, catchalarm);
	signalled = NO;
	(void)alarm(interval);
banner:
	printf("   input    %-6.6s    output       ", interesting->ift_name);
	if (lastif - iftot > 0) {
		if (dflag)
			printf("      ");
		printf("     input   (Total)    output");
	}
	for (ip = iftot; ip < iftot + MAXIF; ip++) {
		ip->ift_ip = 0;
		ip->ift_ie = 0;
		ip->ift_op = 0;
		ip->ift_oe = 0;
		ip->ift_co = 0;
		ip->ift_dr = 0;
	}
	putchar('\n');
	printf("%8.8s %5.5s %8.8s %5.5s %5.5s ",
		"packets", "errs", "packets", "errs", "colls");
	if (dflag)
		printf("%5.5s ", "drops");
	if (lastif - iftot > 0)
		printf(" %8.8s %5.5s %8.8s %5.5s %5.5s",
			"packets", "errs", "packets", "errs", "colls");
	if (dflag)
		printf(" %5.5s", "drops");
	putchar('\n');
	fflush(stdout);
	line = 0;
loop:
	sum->ift_ip = 0;
	sum->ift_ie = 0;
	sum->ift_op = 0;
	sum->ift_oe = 0;
	sum->ift_co = 0;
	sum->ift_dr = 0;
	for (off = firstifnet, ip = iftot; off && ip < lastif; ip++) {
		klseek(kmem, off, 0);
		read(kmem, (char *)&ifnet, sizeof ifnet);
		if (ip == interesting) {
			printf("%8d %5d %8d %5d %5d",
				ifnet.if_ipackets - ip->ift_ip,
				ifnet.if_ierrors - ip->ift_ie,
				ifnet.if_opackets - ip->ift_op,
				ifnet.if_oerrors - ip->ift_oe,
				ifnet.if_collisions - ip->ift_co);
			if (dflag)
				printf(" %5d",
				    ifnet.if_snd.ifq_drops - ip->ift_dr);
		}
		ip->ift_ip = ifnet.if_ipackets;
		ip->ift_ie = ifnet.if_ierrors;
		ip->ift_op = ifnet.if_opackets;
		ip->ift_oe = ifnet.if_oerrors;
		ip->ift_co = ifnet.if_collisions;
		ip->ift_dr = ifnet.if_snd.ifq_drops;
		sum->ift_ip += ip->ift_ip;
		sum->ift_ie += ip->ift_ie;
		sum->ift_op += ip->ift_op;
		sum->ift_oe += ip->ift_oe;
		sum->ift_co += ip->ift_co;
		sum->ift_dr += ip->ift_dr;
		off = (off_t) ifnet.if_next;
	}
	if (lastif - iftot > 0) {
		printf("  %8d %5d %8d %5d %5d",
			sum->ift_ip - total->ift_ip,
			sum->ift_ie - total->ift_ie,
			sum->ift_op - total->ift_op,
			sum->ift_oe - total->ift_oe,
			sum->ift_co - total->ift_co);
		if (dflag)
			printf(" %5d", sum->ift_dr - total->ift_dr);
	}
	*total = *sum;
	putchar('\n');
	fflush(stdout);
	line++;
	oldmask = sigblock(sigmask(SIGALRM));
	if (! signalled) {
		sigpause(0);
	}
	sigsetmask(oldmask);
	signalled = NO;
	(void)alarm(interval);
	if (line == 21)
		goto banner;
	goto loop;
	/*NOTREACHED*/
}

/*
 * Return a printable string representation of an Ethernet address.
 */
char *
etherprint(enaddr)
	char enaddr[6];
{
	static char string[18];
	unsigned char *en = (unsigned char *)enaddr;

	sprintf(string, "%02x:%02x:%02x:%02x:%02x:%02x",
		en[0], en[1], en[2], en[3], en[4], en[5] );
	string[17] = '\0';
	return(string);
}
