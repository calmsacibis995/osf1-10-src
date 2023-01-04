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
 *	@(#)$RCSfile: igmp_var.h,v $ $Revision: 4.2.3.3 $ (DEC) $Date: 1993/06/29 17:55:46 $
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
 * Internet Group Management Protocol (IGMP),
 * implementation-specific definitions.
 *
 * Written by Steve Deering, Stanford, May 1988.
 *
 * MULTICAST 1.1
 */

#ifndef _IGMP_VAR_H_
#define _IGMP_VAR_H_

struct igmpstat {
	u_int	igps_rcv_total;		/* total IGMP messages received    */
	u_int	igps_rcv_tooshort;	/* received with too few bytes     */
	u_int	igps_rcv_badsum;	/* received with bad checksum      */
	u_int	igps_rcv_queries;	/* received membership queries     */
	u_int	igps_rcv_badqueries;	/* received invalid queries        */
	u_int	igps_rcv_reports;	/* received membership reports     */
	u_int	igps_rcv_badreports;	/* received invalid reports        */
	u_int	igps_rcv_ourreports;	/* received reports for our groups */
	u_int	igps_snd_reports;	/* sent membership reports         */
#if	defined(_KERNEL) && LOCK_NETSTATS
	simple_lock_data_t igps_lock;	/* statistics lock */
#endif
};

#ifdef _KERNEL
extern struct igmpstat igmpstat;

/*
 * Macro to compute a random timer value between 1 and (IGMP_MAX_REPORT_DELAY
 * * countdown frequency).  We generate a "random" number by adding
 * the total number of IP packets received, our primary IP address, and the
 * multicast address being timed-out.  The 4.3 random() routine really
 * ought to be available in the kernel!
 */
#define IGMP_RANDOM_DELAY(multiaddr)					\
	/* struct in_addr multiaddr; */					\
	( (u_int)						 	\
	  (ipstat.ips_total +						\
	   ntohl(IA_SIN(in_ifaddr)->sin_addr.s_addr) +			\
	   ntohl((multiaddr).s_addr)					\
	  )								\
	  % (IGMP_MAX_HOST_REPORT_DELAY * PR_FASTHZ) + 1		\
	)

#endif

#endif
