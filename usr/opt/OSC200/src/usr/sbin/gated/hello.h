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
 *	@(#)$RCSfile: hello.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:56:34 $
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
 * COMPONENT_NAME: TCPIP hello.h
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

#ifndef IPPROTO_HELLO
#define		IPPROTO_HELLO	63
#endif
#define		DELAY_INFINITY	30000		/* in ms */
#define		HELLO_TIMERRATE	15		/* in seconds */
#define		HELLO_HYST(s)	(int)(s*.25)	/* 25% of old route, in ms */

#define		HELLO_DEFAULT	0		/* net 0 as default */

#define		METRIC_DIFF(x,y)	(x > y ? x - y : y - x)

/*		Define the DCN HELLO protocol packet			*/

struct hellohdr {
		u_short	h_cksum;
		u_short h_date;
		time_t	h_time;
		u_short	h_tstp;
		} ;

/*
struct m_hdr {
		u_char	m_count ;
		u_char	m_type ;
		} ;
*/
struct type0pair {
		u_short d0_delay;
		u_short	d0_offset;
		} ;
			

struct type1pair {
		struct in_addr d1_dst;
		u_short	d1_delay ;
		short	d1_offset ;
		} ;


