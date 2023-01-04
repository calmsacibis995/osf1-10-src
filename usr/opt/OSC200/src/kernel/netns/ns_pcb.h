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
 *	@(#)$RCSfile: ns_pcb.h,v $ $Revision: 4.2.3.2 $ (DEC) $Date: 1993/06/29 18:39:39 $
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
 * Copyright (c) 1984, 1985, 1986, 1987 Regents of the University of California.
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
 *	Base:	ns_pcb.h	7.4 (Berkeley) 6/28/90
 */

#ifndef _NS_PCB_H_
#define _NS_PCB_H_
/*
 * Ns protocol interface control block.
 */
struct nspcb {
	struct	nspcb *nsp_next;	/* doubly linked list */
	struct	nspcb *nsp_prev;
	struct	nspcb *nsp_head;
	struct	socket *nsp_socket;	/* back pointer to socket */
	struct	ns_addr nsp_faddr;	/* destination address */
	struct	ns_addr nsp_laddr;	/* socket's address */
	caddr_t	nsp_pcb;		/* protocol specific stuff */
	struct	route nsp_route;	/* routing information */
	struct	ns_addr nsp_lastdst;	/* validate cached route for dg socks*/
	long	nsp_notify_param;	/* extra info passed via ns_pcbnotify*/
	short	nsp_flags;
	u_char	nsp_dpt;		/* default packet type for idp_output*/
	u_char	nsp_rpt;		/* last received packet type by
								idp_input() */
};

/* possible flags */

#define NSP_IN_ABORT	0x1		/* calling abort through socket */
#define NSP_RAWIN	0x2		/* show headers on input */
#define NSP_RAWOUT	0x4		/* show header on output */
#define NSP_ALL_PACKETS	0x8		/* Turn off higher proto processing */

#define	NS_WILDCARD	1

#define nsp_lport nsp_laddr.x_port
#define nsp_fport nsp_faddr.x_port

#define	sotonspcb(so)		((struct nspcb *)((so)->so_pcb))

/*
 * Nominal space allocated to a ns socket.
 */
#define	NSSNDQ		2048
#define	NSRCVQ		2048


#ifdef _KERNEL
extern	struct	nspcb nspcb;			/* head of list */
#endif

#endif
