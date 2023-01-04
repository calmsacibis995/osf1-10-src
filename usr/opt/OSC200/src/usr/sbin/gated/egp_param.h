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
 *	@(#)$RCSfile: egp_param.h,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 06:06:44 $
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
 * COMPONENT_NAME: TCPIP egp_param.h
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

/* egp_param.h
 *
 * Defines various egp parameters
 */

/* Retry counts */

#define NACQ_RETRY	5 	/* Max. No. acquire retransmits before 
				   switch to longer interval */
#define NCEASE_RETRY	3 	/* Max. No. cease retransmits */
#define NPOLL		2	/* Max. No. NR polls to send or receive
				   with same id */

/* Acquire interval constants */

/* MINHELLOINT below is used for neigh. acquire retransmit interval when
 * in state UNACQUIRED or ACQUIRE_SENT or cease retry interval when not
 * acquired
 */
#define LONGACQINT	240	/* Neigh. acquire retransmit interval (sec)
				   when no response after NACQ_RETRY or
				   after ceased */
#define	ACQDELAY	3600	/* Delay (seconds) before try to reacquire
				   neighbor that is misbehaving */

/* Hello interval constants */

#define MINHELLOINT	30	/* Minimum interval for sending and
				   receiving hellos */
#define MAXHELLOINT	120	/* Maximum hello interval, sec. */
#define HELLOMARGIN	2	/* Margin in hello interval to allow for delay
				   variation in the network */
/* Poll interval constants */

#define MINPOLLINT	120	/* Minimum interval for sending and receiving
				   polls */
#define MAXPOLLINT  	480	/* Maximum poll interval, sec. */

/* repoll interval is set to the hello interval for the particular neighbor */

/* Reachability test constants */

#define NCOMMANDS	4	/* No. commands sent on which reachability is
				   based */
#define NRESPONSES	2	/* No. responses expected per NCOMMANDs sent,
				   if down, > NRESPONSES => up,
				   if up, < NRESPONSES => down */
#define NUNREACH	60	/* No. consecutive times neighbor is 
				   unreachbable before ceased */
#define MAXNOUPDATE	3	/* Maximum # successive polls (new id) for
				   which no update was received before cease
				   and try to acquire an alternative :/

/* Command reception rate constants */

#define	CHKCMDTIME	480	/* No. seconds betw. check for recv too many
				   acq., hello or poll commands */
#define NMAXCMD		20	/* Max. # acq., hello and poll commands
				   allowed during CHKCMDTIME seconds */
