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
 *	@(#)$RCSfile: in.h,v $ $Revision: 4.3.6.3 $ (DEC) $Date: 1993/10/04 21:14:07 $
 */
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/*
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
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
 *	Base:	in.h	7.8 (Berkeley) 9/13/89
 *	Merged:	in.h	7.10 (Berkeley) 6/28/90
 */

#ifndef	_NETINET_IN_H_
#define	_NETINET_IN_H_

#include <sys/types.h>

/*
 * Constants and structures defined by the internet system,
 * Per RFC 790, September 1981.
 */

/*
 * Protocols
 */
#define	IPPROTO_IP		0		/* dummy for IP */
#define	IPPROTO_ICMP		1		/* control message protocol */
#define IPPROTO_IGMP		2		/* group mgmt protocol */
#define	IPPROTO_GGP		3		/* gateway^2 (deprecated) */
#define	IPPROTO_TCP		6		/* tcp */
#define	IPPROTO_EGP		8		/* exterior gateway protocol */
#define	IPPROTO_PUP		12		/* pup */
#define	IPPROTO_UDP		17		/* user datagram protocol */
#define	IPPROTO_IDP		22		/* xns idp */
#define	IPPROTO_TP		29 		/* tp-4 w/ class negotiation */
#define	IPPROTO_EON		80		/* ISO cnlp */

#define	IPPROTO_RAW		255		/* raw IP packet */
#define	IPPROTO_MAX		256


/*
 * Local port number conventions:
 * Ports < IPPORT_RESERVED are reserved for
 * privileged processes (e.g. root).
 * Ports > IPPORT_USERRESERVED are reserved
 * for servers, not necessarily privileged.
 */
#define	IPPORT_RESERVED		1024
#define	IPPORT_USERRESERVED	5000

/* From Sun 4.1 sources, needed for SUN rpc */
#define    IPPORT_TIMESERVER       37

/*
 * Internet address (a structure for historical reasons)
 */
struct in_addr {
	u_int  s_addr;
};

/*
 * Definitions of bits in internet address integers.
 * On subnets, the decomposition of addresses to host and net parts
 * is done according to subnet mask, not the masks here.
 */
#define	IN_CLASSA(i)		(((int)(i) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		0x00ffffff
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(i)		(((int)(i) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		0x0000ffff
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(i)		(((int)(i) & 0xe0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		0x000000ff

#define	IN_CLASSD(i)		(((int)(i) & 0xf0000000) == 0xe0000000)
#define IN_CLASSD_NET		0xf0000000	/* These ones aren't really */
#define IN_CLASSD_NSHIFT	28		/* net and host fields, but */
#define IN_CLASSD_HOST		0x0fffffff	/* routing needn't know.  */
#define	IN_MULTICAST(i)		IN_CLASSD(i)

#define	IN_EXPERIMENTAL(i)	(((int)(i) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(i)		(((int)(i) & 0xf0000000) == 0xf0000000)

#define	INADDR_ANY		(u_int)0x00000000
#define	INADDR_BROADCAST	(u_int)0xffffffff	/* must be masked */
#define INADDR_LOOPBACK         (u_int)0x7F000001

#define INADDR_UNSPEC_GROUP	(u_int)0xe0000000	/* 224.0.0.0   */
#define INADDR_ALLHOSTS_GROUP	(u_int)0xe0000001	/* 224.0.0.1   */
#define INADDR_MAX_LOCAL_GROUP	(u_int)0xe00000ff	/* 224.0.0.255 */

#ifndef _KERNEL
#define	INADDR_NONE		0xffffffff		/* -1 return */
#endif

#define	IN_LOOPBACKNET		127			/* official! */

/*
 * Socket address, internet style.
 */
#if	defined(_SOCKADDR_LEN) || defined(_KERNEL)
struct sockaddr_in {
	u_char	sin_len;
	u_char	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};
#else
struct sockaddr_in {
	u_short	sin_family;
	u_short	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};
#endif

/*
 * Structure used to describe IP options.
 * Used to store options internally, to pass them to a process,
 * or to restore options retrieved earlier.
 * The ip_dst is used for the first-hop gateway when using a source route
 * (this gets put into the header proper).
 */
struct ip_opts {
	struct	in_addr ip_dst;		/* first hop, 0 w/o src rt */
#if defined(__cplusplus)
	char    Ip_opts[40];            /* cannot have same name as class */
#else
	char	ip_opts[40];		/* actually variable in size */
#endif
};

/*
 * Options for use with [gs]etsockopt at the IP level.
 * First word of comment is data type; bool is stored in int.
 */
#define	IP_OPTIONS	1	/* buf/ip_opts; set/get IP per-packet options */
#define	IP_HDRINCL	2	/* int; header is included with data (raw) */
#define	IP_TOS		3	/* int; IP type of service and precedence */
#define	IP_TTL		4	/* int; IP time to live */
#define	IP_RECVOPTS	5	/* bool; receive all IP options w/datagram */
#define	IP_RECVRETOPTS	6	/* bool; receive IP options for response */
#define	IP_RECVDSTADDR	7	/* bool; receive IP dst addr w/datagram */
#define	IP_RETOPTS	8	/* ip_opts; set/get IP per-packet options */
#define IP_MULTICAST_IF	   9	/* in_addr; IP multicast interface */
#define IP_MULTICAST_TTL   10	/* u_char; IP multicast timetolive */
#define IP_MULTICAST_LOOP  11	/* u_char; IP multicast loopback */
#define IP_ADD_MEMBERSHIP  12	/* ip_mreq; add  an IP group membership */
#define IP_DROP_MEMBERSHIP 13	/* ip_mreq; drop an IP group membership */

#define IP_DEFAULT_MULTICAST_TTL   1	/* normally limit m'casts to 1 hop  */
#define IP_DEFAULT_MULTICAST_LOOP  1	/* normally hear sends if a member  */
#define IP_MAX_MEMBERSHIPS	   20	/* per socket */

/*
 * Argument structure for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP.
 */
struct ip_mreq {
	struct in_addr	imr_multiaddr;	/* IP multicast address of group */
	struct in_addr	imr_interface;	/* local IP address of interface */
};

#ifdef _KERNEL
extern	struct domain inetdomain;
extern	CONST struct protosw inetsw[];
extern	int inetprintfs;
extern	int ipgateway, ipforwarding;
#endif
#endif
