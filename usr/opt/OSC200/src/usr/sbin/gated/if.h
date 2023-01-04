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
 *	@(#)$RCSfile: if.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:56:54 $
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
 * COMPONENT_NAME: TCPIP if.h
 *
 * FUNCTIONS: 
 *
 * ORIGINS: 10 26 27 39 36
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 *   CENTER FOR THEORY AND SIMULATION IN SCIENCE AND ENGINEERING
 *			CORNELL UNIVERSITY
 *
 *      Portions of this software may fall under the following
 *      copyrights: 
 *
 *	Copyright (c) 1983 Regents of the University of California.
 *	All rights reserved.  The Berkeley software License Agreement
 *	specifies the terms and conditions for redistribution.
 *
 *  GATED - based on Kirton's EGP, UC Berkeley's routing daemon (routed),
 *	    and DCN's HELLO routing Protocol.
 *
 *
 */

/*
 * Interface data definitions.
 *
 * Modified from Routing Table Management Daemon, routed/interface.h
 *
 * Structure interface stores information about a directly attached interface,
 * such as name, internet address, and bound sockets. The interface structures
 * are in a singly linked list pointed to by external variable "ifnet".
 */

/*
 * we will keep a list of active routing gateways per interface.  THis is
 * not pretty, but we need this to properly handle split horizon in the
 * multi-routing protocol environment
 */

struct active_gw {
	int 	proto;			/* routing protocols supported */
	u_int	addr;			/* address of gateway */
	int	timer;			/* age of active gateway */
	struct active_gw *next;		/* ptr to next in list */
	struct active_gw *back;		/* ptr to what precedes me in list */
};

struct interface {
	struct	interface *int_next;
	struct	sockaddr int_addr;		/* address on this host */
	union {
		struct	sockaddr intu_broadaddr;
		struct	sockaddr intu_dstaddr;
	} int_intu;
#define	int_broadaddr	int_intu.intu_broadaddr	/* broadcast address */
#define	int_dstaddr	int_intu.intu_dstaddr	/* other end of p-to-p link */
	u_int	int_net;			/* network # */
	u_int	int_netmask;			/* net mask for addr */
	u_int	int_subnet;			/* subnet # */
	u_int	int_subnetmask;			/* subnet mask for addr */
	int	int_metric;			/* init's routing entry */
	int	int_flags;			/* see below */
	int	int_ipackets;			/* input packets received */
	int	int_opackets;			/* output packets sent */
	char	*int_name;			/* from kernel if structure */
	u_short	int_transitions;		/* times gone up-down */
	int	int_egpsock;			/* egp raw socket */
	int	int_icmpsock;			/* icmp raw socket */
	struct active_gw *int_active_gw;	/* gw's using routing */
	int	int_ripfixedmetric;		/* fixed metric for RIP */
	u_short	int_hellofixedmetric;		/* fixed metric for HELLO */
#ifdef  NSS
        int     int_type;                       /* Inter-NSS/Regional */
        struct sockaddr_in intra_nss_int;       /* Intra-NSS interface  */
#endif /*  NSs */
};

/*
 * 0x1 to 0x10 are reused from the kernel's ifnet definitions,
 * the others agree with the RTS_ flags defined elsewhere.
 *	0x20 - 0x200 were used by the kernel in 4.3 BSD, had to push up
 *	the other defines as shown below.  The corresponding RTS_ flags
 *	were also pushed up.	- fedor
 */
#define	IFF_UP		0x1		/* interface is up */
#define	IFF_BROADCAST	0x2		/* broadcast address valid */
#define	IFF_DEBUG	0x4		/* turn on debugging */
#define	IFF_ROUTE	0x8		/* routing entry installed */
#define	IFF_POINTOPOINT	0x10		/* interface is point-to-point link */

#define IFF_MASK        0x1F            /* Values to read from kernel */

#define IFF_SUBNET	0x800		/* is this a subnet interface? */
#define	IFF_PASSIVE	0x1000		/* can't tell if up/down */
#define	IFF_INTERFACE	0x2000		/* hardware interface */
#define	IFF_REMOTE	0x4000		/* interface isn't on this machine */

#define	IFF_NORIPOUT	0x8000		/* Talk RIP on this interface? */
#define	IFF_NORIPIN	0x10000		/* Listen to RIP on this interface? */
#define	IFF_NOHELLOOUT	0x20000		/* Talk HELLO on this interface? */
#define	IFF_NOHELLOIN	0x40000		/* Listen to HELLO on this interface? */
#define	IFF_NOAGE	0x80000		/* don't time out/age this interface */
#define	IFF_ANNOUNCE	0x100000	/* send info on only what is listed */
#define	IFF_NOANNOUNCE	0x200000	/* send info on everything but listed */
#define IFF_RIPFIXEDMETRIC	0x400000	/* Use a fixed metric for RIP out */
#define IFF_HELLOFIXEDMETRIC	0x800000	/* Use a fixed metric for HELLO out */

#define	IFF_KEEPMASK	0x1ff8000	/* Flags to maintain through an if_check() */
