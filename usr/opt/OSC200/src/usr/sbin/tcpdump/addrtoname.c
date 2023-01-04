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
/*
 * Copyright (c) 1988, 1990 The Regents of the University of California.
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
 *
 *  Internet, ethernet, port, and protocol string to address
 *  and address to string conversion routines
 */
/*
 * Based on:
 * static char rcsid[] = "addrtoname.c,v 1.4 92/07/02 13:12:18 mogul Exp $ (LBL)";
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <signal.h>

#include "interface.h"
#include "addrtoname.h"
#include "nametoaddr.h"
#include "etherent.h"
#include "llc.h"

/*
 * hash tables for whatever-to-name translations
 */

#define HASHNAMESIZE 4096

struct hnamemem {
	u_long addr;
	char *name;
	struct hnamemem *nxt;
};

struct hnamemem hnametable[HASHNAMESIZE];
struct hnamemem tporttable[HASHNAMESIZE];
struct hnamemem uporttable[HASHNAMESIZE];
struct hnamemem eprototable[HASHNAMESIZE];
struct hnamemem dnaddrtable[HASHNAMESIZE];
struct hnamemem llcsaptable[HASHNAMESIZE];

struct enamemem {
	u_short e_addr0;
	u_short e_addr1;
	u_short e_addr2;
	char *e_name;
	u_char *e_nsap;			/* used only for nsaptable[] */
	struct enamemem *e_nxt;
};

struct enamemem enametable[HASHNAMESIZE];
struct enamemem nsaptable[HASHNAMESIZE];

struct protoidmem {
	u_long p_oui;
	u_short p_proto;
	char *p_name;
	struct protoidmem *p_nxt;
};

struct protoidmem protoidtable[HASHNAMESIZE];

/*
 * A faster replacement for inet_ntoa().
 */
char *
intoa(addr)
	u_long addr;
{
	register char *cp;
	register u_int byte;
	register int n;
	static char buf[sizeof(".xxx.xxx.xxx.xxx")];

	NTOHL(addr);
	cp = &buf[sizeof buf];
	*--cp = '\0';

	n = 4;
	do {
		byte = addr & 0xff;
		*--cp = byte % 10 + '0';
		byte /= 10;
		if (byte > 0) {
			*--cp = byte % 10 + '0';
			byte /= 10;
			if (byte > 0)
				*--cp = byte + '0';
		}
		*--cp = '.';
		addr >>= 8;
	} while (--n > 0);

	return cp + 1;
}

static u_long f_netmask;
static u_long f_localnet;
static u_long netmask;

/*
 * "getname" is written in this atrocious way to make sure we don't
 * wait forever while trying to get hostnames from yp.
 */
#include <setjmp.h>

jmp_buf getname_env;

static void
nohostname(unused)
int unused;
{
	longjmp(getname_env, 1);
}

/*
 * Return a name for the IP address pointed to by ap.  This address
 * is assumed to be in network byte order.
 */
char *
getname(ap)
	u_char *ap;
{
	register struct hnamemem *p;
	register struct hostent *hp;
	register char *cp;
	u_int32 addr;

#ifndef TCPDUMP_ALIGN
	addr = *(u_int32 *)ap;
#else
	/*
	 * Deal with alignment.
	 */
	switch ((int)ap & 3) {

	case 0:
		addr = *(u_int32 *)ap;
		break;

	case 2:
#if BYTE_ORDER == LITTLE_ENDIAN
		addr = ((u_long)*(u_short *)(ap + 2) << 16) | 
			(u_long)*(u_short *)ap;
#else
		addr = ((u_long)*(u_short *)ap << 16) | 
			(u_long)*(u_short *)(ap + 2);
#endif
		break;

	default:
#if BYTE_ORDER == LITTLE_ENDIAN
		addr = ((u_long)ap[0] << 24) |
			((u_long)ap[1] << 16) |
			((u_long)ap[2] << 8) |
			(u_long)ap[3];
#else
		addr = ((u_long)ap[3] << 24) |
			((u_long)ap[2] << 16) |
			((u_long)ap[1] << 8) |
			(u_long)ap[0];
#endif
		break;
	}
#endif
	p = &hnametable[addr & (HASHNAMESIZE-1)]; 
	for (; p->nxt; p = p->nxt) {
		if (p->addr == addr)
			return (p->name);
	}
	p->addr = addr;
	p->nxt = (struct hnamemem *)calloc(1, sizeof (*p));

	/*
	 * Only print names when:
	 * 	(1) -n was not given.
	 *	(2) Address is foreign and -f was given.  If -f was not 
	 *	    present, f_netmask and f_local are 0 and the second
	 *	    test will succeed.
	 *	(3) The host portion is not 0 (i.e., a network address).
	 *	(4) The host portion is not broadcast.
	 */
	if (!nflag && (addr & f_netmask) == f_localnet
	    && (addr &~ netmask) != 0 && (addr | netmask) != 0xffffffff) {
		if (!setjmp(getname_env)) {
			(void)signal(SIGALRM, nohostname);
			(void)alarm(20);
			hp = gethostbyaddr((char *)&addr, 4, AF_INET);
			(void)alarm(0);
			if (hp) {
				char *dotp;	
				u_int len = strlen(hp->h_name) + 1;
				p->name = (char *)malloc(len);
				(void)strcpy(p->name, hp->h_name);
				if (Nflag) {
					/* Remove domain qualifications */
					dotp = (char *) index(p->name, '.');
					if (dotp)
						*dotp = 0;
				}
				return (p->name);
			}
		}
	}
	cp = intoa(addr);
	p->name = (char *)malloc((unsigned)(strlen(cp) + 1));
	(void)strcpy(p->name, cp);
	return (p->name);
}

static char hex[] = "0123456789abcdef";


/* Find the hash node that corresponds the ether address 'ep'. */

static inline struct enamemem *
lookup_emem(ep)
	u_char *ep;
{
	register u_int i, j, k;
	struct enamemem *tp;

	k = (ep[0] << 8) | ep[1];
	j = (ep[2] << 8) | ep[3];
	i = (ep[4] << 8) | ep[5];

	tp = &enametable[(i ^ j) & (HASHNAMESIZE-1)];
	while (tp->e_nxt)
		if (tp->e_addr0 == i &&
		    tp->e_addr1 == j &&
		    tp->e_addr2 == k)
			return tp;
		else
			tp = tp->e_nxt;
	tp->e_addr0 = i;
	tp->e_addr1 = j;
	tp->e_addr2 = k;
	tp->e_nxt = (struct enamemem *)calloc(1, sizeof(*tp));

	return tp;
}

/* Find the hash node that corresponds the NSAP 'nsap'. */

static inline struct enamemem *
lookup_nsap(nsap)
	register u_char *nsap;
{
	register u_int i, j, k;
	int nlen = *nsap;
	struct enamemem *tp;
	u_char *ensap = nsap + nlen - 6;

	if (nlen > 6) {
	    k = (ensap[0] << 8) | ensap[1];
	    j = (ensap[2] << 8) | ensap[3];
	    i = (ensap[4] << 8) | ensap[5];
	}
	else {
	    i = j = k = 0;
	}
    
	tp = &nsaptable[(i ^ j) & (HASHNAMESIZE-1)];
	while (tp->e_nxt)
		if (tp->e_addr0 == i &&
		    tp->e_addr1 == j &&
		    tp->e_addr2 == k &&
		    tp->e_nsap[0] == nlen &&
		    bcmp(&(nsap[1]), &(tp->e_nsap[1]), nlen) == 0)
			return tp;
		else
			tp = tp->e_nxt;
	tp->e_addr0 = i;
	tp->e_addr1 = j;
	tp->e_addr2 = k;
	tp->e_nsap = (u_char *) calloc(1, nlen + 1);
	bcopy(nsap, tp->e_nsap, nlen + 1);
	tp->e_nxt = (struct enamemem *)calloc(1, sizeof(*tp));

	return tp;
}

/* Find the hash node that corresponds the protoid 'pi'. */

static inline struct protoidmem *
lookup_protoid(pi)
	u_char *pi;
{
	register u_int i, j;
	struct protoidmem *tp;

	/* 5 octets won't be aligned */
	i = (((pi[0] << 8) + pi[1]) << 8) + pi[2];
	j =   (pi[3] << 8) + pi[4];
	/* XXX should be endian-insensitive, but do big-endian testing  XXX */

	tp = &protoidtable[(i ^ j) & (HASHNAMESIZE-1)];
	while (tp->p_nxt)
		if (tp->p_oui == i && tp->p_proto == j)
			return tp;
		else
			tp = tp->p_nxt;
	tp->p_oui = i;
	tp->p_proto = j;
	tp->p_nxt = (struct protoidmem *)calloc(1, sizeof(*tp));

	return tp;
}

char *
etheraddr_string(ep)
	register u_char *ep;
{
	register u_int i, j;
	register char *cp;
	register struct enamemem *tp;

	tp = lookup_emem(ep);
	if (tp->e_name)
		return tp->e_name;

#ifdef ETHER_SERVICE
	if (!nflag) {
		cp = ETHER_ntohost(ep);
		if (cp) {
			tp->e_name = cp;
			return cp;
		}
	}
#endif		
	tp->e_name = cp = (char *)malloc(sizeof("00:00:00:00:00:00"));

	if (j = *ep >> 4)
		*cp++ = hex[j];
	*cp++ = hex[*ep++ & 0xf];
	for (i = 5; (int)--i >= 0;) {
		*cp++ = ':';
		if (j = *ep >> 4)
			*cp++ = hex[j];
		*cp++ = hex[*ep++ & 0xf];
	}
	*cp = '\0';
	return (tp->e_name);
}

char *
etherproto_string(port)
	u_short port;
{
	register char *cp;
	register struct hnamemem *tp;
	register u_long i = port;

	for (tp = &eprototable[i & (HASHNAMESIZE-1)]; tp->nxt; tp = tp->nxt)
		if (tp->addr == i)
			return (tp->name);

	tp->name = cp = (char *)malloc(sizeof("0000"));
	tp->addr = i;
	tp->nxt = (struct hnamemem *)calloc(1, sizeof (*tp));

	NTOHS(port);
	*cp++ = hex[port >> 12 & 0xf];
	*cp++ = hex[port >> 8 & 0xf];
	*cp++ = hex[port >> 4 & 0xf];
	*cp++ = hex[port & 0xf];
	*cp++ = '\0';
	return (tp->name);
}

char *
protoid_string(pi)
	register u_char *pi;
{
	register u_int i, j;
	register char *cp;
	register struct protoidmem *tp;

	tp = lookup_protoid(pi);
	if (tp->p_name)
		return tp->p_name;

	tp->p_name = cp = (char *)malloc(sizeof("00:00:00:00:00"));

	if (j = *pi >> 4)
		*cp++ = hex[j];
	*cp++ = hex[*pi++ & 0xf];
	for (i = 4; (int)--i >= 0;) {
		*cp++ = ':';
		if (j = *pi >> 4)
			*cp++ = hex[j];
		*cp++ = hex[*pi++ & 0xf];
	}
	*cp = '\0';
	return (tp->p_name);
}

char *
llcsap_string(sap)
	u_char sap;
{
	register char *cp;
	register struct hnamemem *tp;
	register u_long i = sap;

	for (tp = &llcsaptable[i & (HASHNAMESIZE-1)]; tp->nxt; tp = tp->nxt)
		if (tp->addr == i)
			return (tp->name);

	tp->name = cp = (char *)malloc(sizeof("sap 00"));
	tp->addr = i;
	tp->nxt = (struct hnamemem *)calloc(1, sizeof (*tp));

	(void)strcpy(cp, "sap ");
	cp += strlen(cp);
	*cp++ = hex[sap >> 4 & 0xf];
	*cp++ = hex[sap & 0xf];
	*cp++ = '\0';
	return (tp->name);
}

char *
isonsap_string(nsap)
	u_char *nsap;
{
	register u_int i, j, nlen = nsap[0];
	register char *cp;
	register struct enamemem *tp;

	tp = lookup_nsap(nsap);
	if (tp->e_name)
		return tp->e_name;

	tp->e_name = cp = (char *)malloc(nlen * 2 + 2);

	nsap++;
	*cp++ = '/';
	for (i = nlen; (int)--i >= 0;) {
		*cp++ = hex[*nsap >> 4];
		*cp++ = hex[*nsap++ & 0xf];
	}
	*cp = '\0';
	return (tp->e_name);
}

char *
tcpport_string(port)
	u_short port;
{
	register struct hnamemem *tp;
	register int i = port;

	for (tp = &tporttable[i & (HASHNAMESIZE-1)]; tp->nxt; tp = tp->nxt)
		if (tp->addr == i)
			return (tp->name);

	tp->name = (char *)malloc(sizeof("00000"));
	tp->addr = i;
	tp->nxt = (struct hnamemem *)calloc(1, sizeof (*tp));

	(void)sprintf(tp->name, "%d", i);
	return (tp->name);
}

char *
udpport_string(port)
	register u_short port;
{
	register struct hnamemem *tp;
	register int i = port;

	for (tp = &uporttable[i & (HASHNAMESIZE-1)]; tp->nxt; tp = tp->nxt)
		if (tp->addr == i)
			return (tp->name);

	tp->name = (char *)malloc(sizeof("00000"));
	tp->addr = i;
	tp->nxt = (struct hnamemem *)calloc(1, sizeof(*tp));

	(void)sprintf(tp->name, "%d", i);

	return (tp->name);
}

static void
init_servarray()
{
	struct servent *sv;
	register struct hnamemem *table;
	register int i;

	while (sv = getservent()) {
		NTOHS(sv->s_port);
		i = sv->s_port & (HASHNAMESIZE-1);
		if (strcmp(sv->s_proto, "tcp") == 0)
			table = &tporttable[i];
		else if (strcmp(sv->s_proto, "udp") == 0)
			table = &uporttable[i];
		else
			continue;

		while (table->name)
			table = table->nxt;
		if (nflag) {
			char buf[32];

			(void)sprintf(buf, "%d", sv->s_port);
			table->name = (char *)malloc((unsigned)strlen(buf)+1);
			(void)strcpy(table->name, buf);
		} else {
			table->name =
				(char *)malloc((unsigned)strlen(sv->s_name)+1);
			(void)strcpy(table->name, sv->s_name);
		}
		table->addr = sv->s_port;
		table->nxt = (struct hnamemem *)calloc(1, sizeof(*table));
	}
	endservent();
}

#include "etherproto.h"

/* Static data base of ether protocol types. */
struct eproto eproto_db[] = { 
	{ "pup", ETHERTYPE_PUP },
	{ "xns", ETHERTYPE_NS },
	{ "ip", ETHERTYPE_IP },
	{ "arp", ETHERTYPE_ARP },
	{ "rarp", ETHERTYPE_REVARP },
	{ "sprite", ETHERTYPE_SPRITE },
	{ "mopdl", ETHERTYPE_MOPDL },
	{ "moprc", ETHERTYPE_MOPRC },
	{ "decnet", ETHERTYPE_DN },
	{ "lat", ETHERTYPE_LAT },
	{ "lanbridge", ETHERTYPE_LANBRIDGE },
	{ "vexp", ETHERTYPE_VEXP },
	{ "vprod", ETHERTYPE_VPROD },
	{ "atalk", ETHERTYPE_ATALK },
	{ "atalkarp", ETHERTYPE_AARP },
	{ "loopback", ETHERTYPE_LOOPBACK },
	{ "decdts", ETHERTYPE_DECDTS },
	{ "decdns", ETHERTYPE_DECDNS },
	{ (char *)0, 0 }
};

static void
init_eprotoarray()
{
	register int i;
	register struct hnamemem *table;

	for (i = 0; eproto_db[i].s; i++) {
		int j = ntohs(eproto_db[i].p) & (HASHNAMESIZE-1);
		table = &eprototable[j];
		while (table->name)
			table = table->nxt;
		table->name = eproto_db[i].s;
		table->addr = ntohs(eproto_db[i].p);
		table->nxt = (struct hnamemem *)calloc(1, sizeof(*table));
	}
}

/*
 * SNAP proto IDs with org code 0:0:0 are actually encapsulated Ethernet
 * types.
 */
static void
init_protoidarray()
{
	register int i;
	register struct protoidmem *tp;
	u_char protoid[5];

	protoid[0] = 0;
	protoid[1] = 0;
	protoid[2] = 0;
	for (i = 0; eproto_db[i].s; i++) {
		u_short etype = htons(eproto_db[i].p);
		bcopy((char *)&(etype), (char *)&(protoid[3]), 2);
		tp = lookup_protoid(protoid);
		tp->p_name = (char *)malloc(strlen(eproto_db[i].s) + 1);
		strcpy(tp->p_name, eproto_db[i].s);
	}
}

static void
init_etherarray()
{
#ifndef ETHER_SERVICE
	FILE *fp;
	struct etherent *ep;
	struct enamemem *tp;

	fp = fopen(ETHERS_FILE, "r");
	if (fp == 0)
		/* No data base; will have to settle for 
		   numeric addresses. */
		return;

	while (ep = next_etherent(fp)) {
		tp = lookup_emem(ep->addr);
		tp->e_name = (char *)malloc((unsigned)strlen(ep->name)+1);
		strcpy(tp->e_name, ep->name);
	}
#endif
}

struct llcsap llcsap_db[] = {
	{ LLCSAP_NULL, "null" },
	{ LLCSAP_8021B_I, "802.1b-gsap" },
	{ LLCSAP_8021B_G, "802.1b-isap" },
	{ LLCSAP_IP, "ip-sap" },
	{ LLCSAP_PROWAYNM, "proway-nm" },
	{ LLCSAP_8021D, "802.1d" },
	{ LLCSAP_RS511, "eia-rs511" },
	{ LLCSAP_ISO8208, "x.25/llc2" },
	{ LLCSAP_PROWAY, "proway" },
	{ LLCSAP_ISONS, "iso-clns" },
	{ LLCSAP_GLOBAL, "global" },
	{ 0, (char *) 0 }
};

static void
init_llcsaparray()
{
	register int i;
	register struct hnamemem *table;

	for (i = 0; llcsap_db[i].s; i++) {
		table = &llcsaptable[llcsap_db[i].sap];
		while (table->name)
			table = table->nxt;
		table->name = llcsap_db[i].s;
		table->addr = llcsap_db[i].sap;
		table->nxt = (struct hnamemem *)calloc(1, sizeof(*table));
	}
}

/*
 * Initialize the address to name translation machinery.  We map all
 * non-local IP addresses to numeric addresses if fflag is true (i.e.,
 * to prevent blocking on the nameserver).  localnet is the IP address
 * of the local network.  mask is its subnet mask.
 */
void
init_addrtoname(fflag, localnet, mask)
	int fflag;
	u_long localnet;
	u_long mask;
{
	netmask = mask;
	if (fflag) {
		f_localnet = localnet;
		f_netmask = mask;
	}
	if (nflag)
		/*
		 * Simplest way to suppress names.
		 */
		return;

	init_etherarray();
	init_servarray();
	init_eprotoarray();
	init_llcsaparray();
	init_protoidarray();
}

char *
dnaddr_string(dnaddr)
unsigned short dnaddr;
{
	register struct hnamemem *tp;
	char *dnname_string();
	char *dnnum_string();
	
	for (tp = &dnaddrtable[dnaddr & (HASHNAMESIZE-1)];
					tp->nxt; tp = tp->nxt)
		if (tp->addr == dnaddr)
			return (tp->name);

	tp->addr = dnaddr;
	tp->nxt = (struct hnamemem *)calloc(1, sizeof(*tp));
	if (nflag)
	    tp->name = dnnum_string(dnaddr);
	else
	    tp->name = dnname_string(dnaddr);
	return(tp->name);
}
