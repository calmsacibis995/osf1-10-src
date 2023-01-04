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
 *	@(#)$RCSfile: if_ether.h,v $ $Revision: 4.2.5.5 $ (DEC) $Date: 1993/07/27 15:50:54 $
 */ 
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
 * Copyright (c) 1982, 1986 Regents of the University of California.
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
 *	Base:	if_ether.h	7.4 (Berkeley) 2/17/89
 *	Merged:	if_ether.h	7.5 (Berkeley) 6/28/90
 */

#ifndef _IF_ETHER_H_
#define _IF_ETHER_H_
/*
 * Modification History:
 *
 *	Matt Thomas	 2/15/90
 *	Add ETHERTYPE_LAT and ETHERTYPE_DECnet for LAT and DLI.
 */

/*
 * Structure of a 10Mb/s Ethernet header.
 */
struct	ether_header {
	u_char	ether_dhost[6];
	u_char	ether_shost[6];
	u_short	ether_type;
};

#ifndef _KERNEL
struct ether_addr {
	u_char  ether_addr_octet[6];
};
#endif

/*
 * Note that PUP is <= 1500 and since falls in the range of valid 802.3
 * frames.  Hence, PUP is not actually usuable.
 */
#define	ETHERTYPE_PUP		0x0200	/* PUP protocol */
#define	ETHERTYPE_IP		0x0800	/* IP protocol */
#define ETHERTYPE_ARP		0x0806	/* Addr. resolution protocol */
#define ETHERTYPE_LAT		0x6004	/* Local Area Transport (LAT) */
#define ETHERTYPE_DECnet	0x6003	/* Phase IV DECnet */
#define ETHERTYPE_MOPRC		0x6002	/* MOP CCR protocol type */
#define ETHERTYPE_MOPDL		0x6001	/* MOP Downline Load protocol type */
#define ETHERTYPE_LBACK		0x9000	/* MOP loopback protocol type */

/*
 * The ETHERTYPE_NTRAILER packet types starting at ETHERTYPE_TRAIL have
 * (type-ETHERTYPE_TRAIL)*512 bytes of data followed
 * by an ETHER type (as given above) and then the (variable-length) header.
 */
#define	ETHERTYPE_TRAIL		0x1000		/* Trailer packet */
#define	ETHERTYPE_NTRAILER	16

#if 0
/* #if defined(__alpha) && defined(_KERNEL) */
/* This is needed to support networking with ADUs */
#ifndef ALPHA_ADU
#include <hal/cpuconf.h>
#endif
extern int cpu;
#define	ETHERMTU	((cpu == ALPHA_ADU) ? (1500-14) : 1500)
#else
#define	ETHERMTU	1500
#endif	/* __alpha */
#define	ETHERMIN	(60-14)

/*
 * Ethernet Bandwidth.
 */
#define ETHER_BANDWIDTH_10MB	10000000        /* Ethernet - 10Mbs */

/*
 * Ethernet Address Resolution Protocol.
 *
 * See RFC 826 for protocol description.  Structure below is adapted
 * to resolving internet addresses.  Field names used correspond to 
 * RFC 826.
 */
struct	ether_arp {
	struct	arphdr ea_hdr;	/* fixed-size header */
	u_char	arp_sha[6];	/* sender hardware address */
	u_char	arp_spa[4];	/* sender protocol address */
	u_char	arp_tha[6];	/* target hardware address */
	u_char	arp_tpa[4];	/* target protocol address */
};
#define	arp_hrd	ea_hdr.ar_hrd
#define	arp_pro	ea_hdr.ar_pro
#define	arp_hln	ea_hdr.ar_hln
#define	arp_pln	ea_hdr.ar_pln
#define	arp_op	ea_hdr.ar_op

/*
 * Structure shared between the ethernet driver modules and
 * the address resolution code.  For example, each ec_softc or il_softc
 * begins with this structure.
 */
#define NISAPS	128
struct	arpcom {
	struct 	ifnet ac_if;		/* network-visible interface */
	u_char	ac_hwaddr[14];		/* hardware address (sizeof sa_data) */
	u_short	ac_arphrd;		/* arp hardware type (net/if_arp.h) */
	u_char	*ac_bcastaddr;		/* pointer to link broadcast or 0 */
	struct in_addr ac_ipaddr;	/* copy of ip address- XXX */
	struct dli_ifnet *ac_dlif;	/* pointer to the DLI data */
};
#define ac_enaddr	ac_hwaddr	/* one size fits many */

/*
 * Internet to ethernet address resolution table.
 */
struct	arptab {
	struct	in_addr at_iaddr;	/* internet address */
	struct	ifnet *at_if;		/* interface pointer */
	u_char	at_hwaddr[14];		/* hardware address (len in at_if) */
	u_short	at_flags;		/* flags (net/if_arp.h) */
	short	at_timer;		/* ticks since last referenced */
	short	at_valid;		/* ticks since last valid */
	short	at_retry;		/* request send count */
	short	at_sent;		/* at_valid at last send */
	struct	mbuf *at_hold;		/* last packet until resolved/timeout */
};
#define at_enaddr	at_hwaddr	/* as for arpcom */

#ifdef	_KERNEL
/*
 * Macro to map an IP multicast address to an Ethernet multicast address.
 * The high-order 25 bits of the Ethernet address are statically assigned,
 * and the low-order 23 bits are taken from the low end of the IP address.
 */
#define ETHER_MAP_IP_MULTICAST(ipaddr, enaddr) \
	/* struct in_addr *ipaddr; */ \
	/* u_char enaddr[6];	   */ \
{ \
	(enaddr)[0] = 0x01; \
	(enaddr)[1] = 0x00; \
	(enaddr)[2] = 0x5e; \
	(enaddr)[3] = ((u_char *)ipaddr)[1] & 0x7f; \
	(enaddr)[4] = ((u_char *)ipaddr)[2]; \
	(enaddr)[5] = ((u_char *)ipaddr)[3]; \
}

extern CONST u_char etherbroadcastaddr[6];
#endif

#endif
