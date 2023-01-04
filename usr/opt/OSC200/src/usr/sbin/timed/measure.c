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
static char	*sccsid = "@(#)$RCSfile: measure.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/13 17:53:36 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/* 
 * COMPONENT_NAME: TCPIP measure.c
 * 
 * FUNCTIONS: in_cksum, measure 
 *
 * ORIGINS: 10  26  27 
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989
 * All Rights Reserved
 * Licensed Materials - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1983 Regents of the University of California.
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
/* measure.c	1.3  com/sockcmd/timed,3.1,9021 10/8/89 17:46:35 */
/*
#ifndef lint
static char sccsid[] = "measure.c	2.6 (Berkeley) 6/18/88";
#endif  not lint */

#include "globals.h"
#include <protocols/timed.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

#define BIASP	 	43199999
#define BIASN		-43200000
#define MODULO	 	86400000
#define PROCESSING_TIME	5 	/* ms. to reduce error in measurement */

#define PACKET_IN	1024

extern int id;
int measure_delta;
extern int sock_raw;
static n_short seqno = 0;

/*
 * Measures the differences between machines' clocks using
 * ICMP timestamp messages.
 */

measure(wait, addr)
struct timeval *wait;
struct sockaddr_in *addr;
{
	int length;
	int status;
	int msgcount, trials;
	int cc, count;
	fd_set ready;
	int sendtime, recvtime, histime;
	int min1, min2, diff;
	register int delta1, delta2;
	struct timeval tv1, tout;
	u_char packet[PACKET_IN], opacket[64];
	register struct icmp *icp = (struct icmp *) packet;
	register struct icmp *oicp = (struct icmp *) opacket;
	struct ip *ip = (struct ip *) packet;

	min1 = min2 = 0x7fffffff;
	status = HOSTDOWN;
	measure_delta = HOSTDOWN;

/* empties the icmp input queue */
	FD_ZERO(&ready);
empty:
	tout.tv_sec = tout.tv_usec = 0;
	FD_SET(sock_raw, &ready);
	if (select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout)) {
		length = sizeof(struct sockaddr_in);
		cc = recvfrom(sock_raw, (char *)packet, PACKET_IN, 0, 
		    (struct sockaddr_in *)NULL, &length);
		if (cc < 0)
			return(-1);
		goto empty;
	}

	/*
	 * To measure the difference, select MSGS messages whose round-trip
	 * time is smaller than RANGE if ckrange is 1, otherwise simply
	 * select MSGS messages regardless of round-trip transmission time.
	 * Choose the smallest transmission time in each of the two directions.
	 * Use these two latter quantities to compute the delta between
	 * the two clocks.
	 */

	length = sizeof(struct sockaddr_in);
	oicp->icmp_type = ICMP_TSTAMP;
	oicp->icmp_code = 0;
	oicp->icmp_cksum = 0;
	oicp->icmp_id = id;
	oicp->icmp_rtime = 0;
	oicp->icmp_ttime = 0;
	FD_ZERO(&ready);
	msgcount = 0;
	for (trials = 0; msgcount < MSGS && trials < TRIALS; ++trials) {
		oicp->icmp_seq = ++seqno;
		oicp->icmp_cksum = 0;

		tout.tv_sec = wait->tv_sec;
		tout.tv_usec = wait->tv_usec;

    		(void)gettimeofday (&tv1, (struct timezone *)0);
		sendtime = oicp->icmp_otime = (tv1.tv_sec % (24*60*60)) * 1000 
							+ tv1.tv_usec / 1000;
		oicp->icmp_cksum = in_cksum((u_short *)oicp, sizeof(*oicp));
	
		count = sendto(sock_raw, (char *)opacket, sizeof(*oicp), 0, 
				addr, sizeof(struct sockaddr_in));
		if (count < 0) {
			status = UNREACHABLE;
			return(-1);
		}
		for (;;) {
			FD_SET(sock_raw, &ready);
			if ((count = select(FD_SETSIZE, &ready, (fd_set *)0,
			    (fd_set *)0, &tout)) <= 0)
				break;
			cc = recvfrom(sock_raw, (char *)packet, PACKET_IN, 0, 
			    (struct sockaddr_in *)NULL, &length);
			(void)gettimeofday(&tv1, (struct timezone *)0);
			if (cc < 0)
				return(-1);
			icp = (struct icmp *)(packet + (ip->ip_hl << 2));
			if((icp->icmp_type == ICMP_TSTAMPREPLY) &&
			    icp->icmp_id == id && icp->icmp_seq == seqno)
				break;
		}
		if (count <= 0)
			continue;		/* resend */
		recvtime = (tv1.tv_sec % (24*60*60)) * 1000 +
		    tv1.tv_usec / 1000;
		diff = recvtime - sendtime;
		/*
		 * diff can be less than 0 aroud midnight
		 */
		if (diff < 0)
			continue;
		msgcount++;
		histime = ntohl(icp->icmp_rtime);
		/*
		 * a hosts using a time format different from 
		 * ms. since midnight UT (as per RFC792) should
		 * set the high order bit of the 32-bit time
		 * value it transmits.
		 */
		if ((histime & 0x80000000) != 0) {
			status = NONSTDTIME;
			break;
		}
		status = GOOD;
		delta1 = histime - sendtime;
		/*
		 * Handles wrap-around to avoid that around 
		 * midnight small time differences appear 
		 * enormous. However, the two machine's clocks
		 * must be within 12 hours from each other.
		 */
		if (delta1 < BIASN)
			delta1 += MODULO;
		else if (delta1 > BIASP)
			delta1 -= MODULO;
		delta2 = recvtime - histime;
		if (delta2 < BIASN)
			delta2 += MODULO;
		else if (delta2 > BIASP)
			delta2 -= MODULO;
		if (delta1 < min1)  
			min1 = delta1;
		if (delta2 < min2)
			min2 = delta2;
		if (diff < RANGE) {
			min1 = delta1;
			min2 = delta2;
			break;
		}
	}

	/*
	 * If no answer is received for TRIALS consecutive times, 
	 * the machine is assumed to be down
	 */
	 if (status == GOOD) {
		measure_delta = (min1 - min2)/2 + PROCESSING_TIME;
	}
	return(status);
}
in_cksum(addr, len)
u_short *addr;
int len;
{
	register int nleft = len;
	register u_short *w = addr, tmp;
	register u_short answer;
	register int sum = 0;

	/*
	 *  Our algorithm is simple, using a 32 bit accumulator (sum),
	 *  we add sequential 16 bit words to it, and at the end, fold
	 *  back all the carry bits from the top 16 bits into the lower
	 *  16 bits.
	 */
	while( nleft > 1 )  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if( nleft == 1 ) {
		tmp = *(u_char *)w;
		sum += (tmp << 8);
	}

	/*
	 * add back carry outs from top 16 bits to low 16 bits
	 */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return (answer);
}
