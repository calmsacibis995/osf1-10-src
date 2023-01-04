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
 *	@(#)$RCSfile: rt_control.h,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/04/20 15:58:26 $
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
 * COMPONENT_NAME: TCPIP rt_control.h
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

#define MAXINTERFACE	25		/* Maximum number of interfaces */

struct restricthash {
	struct	restrictlist *rt_forw;
	struct	restrictlist *rt_back;
};

struct restrictlist {
	struct	restrictlist *rt_forw;
	struct	restrictlist *rt_back;
	u_int	rhash;
	struct	sockaddr_in rdst;
	int	rproto;
	int	regpmetric;
	int	flags;
	u_int  rintf[MAXINTERFACE];
};

#define RT_ANNOUNCE	0x1		/* announce control restriction */
#define RT_NOLISTEN	0x2		/* listen control restriction */
#define RT_SRCLISTEN	0x4		/* listen from source */
#define RT_NOANNOUNCE	0x8		/* noannounce control restriction */

struct as_entry {
	struct as_entry *next;
	u_short as;
	u_short flags;
};

struct as_list {
	struct as_list *next;
	u_short as;
	u_short flags;
	struct as_entry *as_ptr;
};

#define	AS_SEND		0x1		/* Can send to this AS */
#define AS_DONOTSEND	0x2		/* Can not send to this AS */
#define	AS_RESTRICT	0x4		/* Announcement restrictions apply */

struct as_valid {
	struct as_valid *next;
	struct in_addr dst;
	u_short as;
	u_short metric;
};

#define MARTIAN_NETS	static char *martian_nets[7] = {\
	"127.0.0.0",\
	"128.0.0.0",\
	"191.255.0.0",\
	"192.0.0.0",\
	"223.255.255.0",\
	"224.0.0.0",\
	(char *)0 }		/* This is the end of the table, not default net */
