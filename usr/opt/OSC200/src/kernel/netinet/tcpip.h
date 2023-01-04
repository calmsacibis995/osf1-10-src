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
 *	@(#)$RCSfile: tcpip.h,v $ $Revision: 4.3.7.2 $ (DEC) $Date: 1993/06/29 18:14:39 $
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
 *	Base:	tcpip.h	7.3 (Berkeley) 6/29/88
 *	Merged:	tcpip.h	7.4 (Berkeley) 6/28/90
 */

#ifndef _NETINET_TCPIP_H_
#define _NETINET_TCPIP_H_
/*
 * Tcp+ip header, after ip options removed.
 */
struct tcpiphdr {
	struct 	ipovly ti_i;		/* overlaid ip structure */
	struct	tcphdr ti_t;		/* tcp header */
};

#ifdef __alpha
/* Structure used to store the pointers linking the
 * protocol reassembly queues. The 4 pointers overlay the contents of
 * the mbuf's mun1_ihp[4] field. 
 */
struct ovtcpiphdr {			/* used for reassembly queues */
	caddr_t ti_next, ti_prev;	/* for protocol sequence q's */
	struct tcpiphdr *ti_ipovly; 	/* where to find the real tcp+ip
					 * header, after ip options removed */
	struct mbuf *ti_rmb;		/* for use by REASS_MBUF() */
};

#else  /* not __alpha */

#define	ti_next		ti_i.ih_next
#define	ti_prev		ti_i.ih_prev

#endif /* __alpha */

#define	ti_x1		ti_i.ih_x1
#define	ti_pr		ti_i.ih_pr
#define	ti_len		ti_i.ih_len
#define	ti_src		ti_i.ih_src
#define	ti_dst		ti_i.ih_dst
#define	ti_sport	ti_t.th_sport
#define	ti_dport	ti_t.th_dport
#define	ti_seq		ti_t.th_seq
#define	ti_ack		ti_t.th_ack
#if	defined(_KERNEL) || defined(_NO_BITFIELDS) || (__STDC__ == 1)
#define	ti_xoff		ti_t.th_xoff
#else
#define	ti_x2		ti_t.th_x2
#define	ti_off		ti_t.th_off
#endif
#define	ti_flags	ti_t.th_flags
#define	ti_win		ti_t.th_win
#define	ti_sum		ti_t.th_sum
#define	ti_urp		ti_t.th_urp

#endif
