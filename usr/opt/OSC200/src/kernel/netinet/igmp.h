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
 *	@(#)$RCSfile: igmp.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/06/29 17:55:37 $
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
 * Internet Group Management Protocol (IGMP) definitions.
 *
 * Written by Steve Deering, Stanford, May 1988.
 *
 * MULTICAST 1.1
 */

#ifndef _IGMP_H_
#define _IGMP_H_
/*
 * IGMP packet format.
 */
struct igmp {
	u_char		igmp_type;	/* version & type of IGMP message  */
	u_char		igmp_code;	/* unused, should be zero          */
	u_short		igmp_cksum;	/* IP-style checksum               */
	struct in_addr	igmp_group;	/* group address being reported    */
};					/*  (zero for queries)             */

#define IGMP_MINLEN		     8

#define IGMP_HOST_MEMBERSHIP_QUERY   0x11  /* message types, incl. version */
#define IGMP_HOST_MEMBERSHIP_REPORT  0x12
#define IGMP_DVMRP		     0x13  /* for experimental multicast   */
					   /*  routing protocol            */

#define IGMP_MAX_HOST_REPORT_DELAY   10    /* max delay for response to    */
					   /*  query (in seconds)          */

#endif
