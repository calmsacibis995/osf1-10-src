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
 *	@(#)$RCSfile: inet_config.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/06/29 18:11:54 $
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

#ifndef _INET_CONFIG_H_
#define _INET_CONFIG_H_

#define INET_CONFIG_VERSION_1	0x01091590

#define IN_DEFAULT_VALUE	-12345
#define IN_USEVALUE		0x01
#define IN_USEDEFAULTS		0x02

typedef struct inet_config {

	int	version;
	int	errcode;
	int	flags;

	int	inetprintfs;		/* If configured, enable printfs (0) */
	int	useloopback;		/* Use loopback for own packets (1) */

	int	ipgateway;		/* Configure as gateway (0) */
	int	ipforwarding;		/* Act as gateway (0) */
	int	ipsendredirects;	/* Send ICMP redirects if gateway (1) */
	int	ipdirected_broadcast;	/* Broadcasts accepted uniquely (0) */
	int	ipsrcroute;		/* Enable host source routing (1) */
	int	subnetsarelocal;	/* Subnets appear as connected (1) */
	int	ipqmaxlen;		/* Length of IP input queue (512) */

	/* All times in SECONDS */

	int	tcpttl;			/* Default time to live (60) */
	int	tcpmssdflt;		/* Default max segsize (536) */
	int	tcprttdflt;		/* Default initial rtt (1.5) */
	int	tcpkeepidle;		/* Keepalive idle timer (7200) */
	int	tcpkeepintvl;		/* Keepalive interval (75) */
	int	tcpcompat_42;		/* BSD4.2 compat keepalive */
	int	tcprexmtthresh;		/* Retransmit threshold (3) */
	int	tcpconsdebug;		/* If configured, debug printfs (0) */
	u_long	tcp_sendspace;		/* Default send queue (32768) */
	u_long	tcp_recvspace;		/* Default receive queue (32768) */

	int	udpttl;			/* Default time to live (30) */
	int	udpcksum;		/* Enable checksumming (1) */
	u_long	udp_sendspace;		/* Default send queue (9216) */
	u_long	udp_recvspace;		/* Default receive queue (41600) */

	int	arpkillc;		/* Time to remove completed (1200) */
	int	arpkilli;		/* Time to remove incomplete (180) */
	int	arprefresh;		/* Time to refresh entry (120) */
	int	arphold;		/* Time to hold packet (5) */
	int	arplost;		/* Count to broadcast refresh (3) */
	int	arpdead;		/* Count to assume dead (6) */
	int	arpqmaxlen;		/* Length of ARP input queue (512) */
	int	arptabbsiz;		/* Table bucket size (16) */
	int	arptabnb;		/* Number of buckets (37) */
} inet_config_t;

#endif
