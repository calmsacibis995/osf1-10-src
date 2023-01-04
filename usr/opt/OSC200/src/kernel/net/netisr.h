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
 *	@(#)$RCSfile: netisr.h,v $ $Revision: 4.2.10.4 $ (DEC) $Date: 1993/06/29 17:32:46 $
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
 * Copyright (C) 1988,1989 Encore Computer Corporation.  All Rights Reserved
 *
 * Property of Encore Computer Corporation.
 * This software is made available solely pursuant to the terms of
 * a software license agreement which governs its use. Unauthorized
 * duplication, distribution or sale are strictly prohibited.
 *
 */
/*
 * Copyright (c) 1980, 1986, 1989 Regents of the University of California.
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
 *	Base:	netisr.h	7.5 (Berkeley) 4/22/89
 *	Merged:	netisr.h	7.6 (Berkeley) 6/28/90
 */

#ifndef _NETISR_H_
#define _NETISR_H_

#ifdef	_KERNEL

#if	NETISR_THREAD
/*
 * The networking code is in separate kernel threads.
 */
#ifndef	LOCORE
#include "kern/thread.h"
#include "kern/sched_prim.h"
#define setsoftnet()	thread_wakeup_one((vm_offset_t)netisr_thread)
#endif

#else	/* e.g. UNIX */

/*
 * The networking code runs off software interrupts.
 *
 * You can switch into the network by doing splnet() and return by splx().
 * The software interrupt level for the network is higher than the software
 * level for the clock (so you can enter the network in routines called
 * at timeout time).
 */
#if defined(vax) || defined(tahoe)
#include "machine/mtpr.h"
#define	setsoftnet()	mtpr(SIRR, 12)
#else
extern void setsoftnet();
#endif

#endif

/*
 * Each ``pup-level-1'' input queue has a bit in a ``netisr'' status
 * word which is used to de-multiplex a single software
 * interrupt used for scheduling the network code to calls
 * on the lowest level routine of each protocol.
 */
#define	NETISR_MB	0		/* using 0 for mbufs */
#define	NETISR_ARP	1		/* using 1 for ARP */
#define	NETISR_IP	2		/* same as AF_INET */
#define	NETISR_IMP	3		/* same as AF_IMPLINK */
#define NETISR_OTHER	4		/* packets otherwise tossed (->dlpi)*/
#define NETISR_STREAMS  5               /* Streams scheduler */
#define	NETISR_NS	6		/* same as AF_NS */
#define	NETISR_ISO	7		/* same as AF_ISO */
#define NETISR_STRTO    8               /* Streams timeout */
#define NETISR_EVL	9		/* X.25 event logging */
#define NETISR_DLO	10		/* used for node 0 in DECnet */
#define NETISR_STRWELD  11              /* Streams "weld" */
#define	NETISR_DN	12		/* same as AF_DECnet */
#define	NETISR_DLI	13		/* same as AF_DLI */
#define	NETISR_LAT	14		/* same as AF_LAT */
#define NETISR_WDD	15		/* used by AF_WAN */

#define NETISR_MAX	16

#if	NETSYNC_LOCK
#define NETISR_LOCKINIT()	simple_lock_init(&netisr_slock)
#define NETISR_LOCK()		simple_lock(&netisr_slock)
#define NETISR_UNLOCK()		simple_unlock(&netisr_slock)
#else
#define NETISR_LOCKINIT()
#define NETISR_LOCK()
#define NETISR_UNLOCK()
#endif

/* Must be called at splimp() */
#define schednetisr(anisr) {		\
	NETISR_LOCK();			\
	++softnet_intr[anisr+1].pending;\
	NETISR_UNLOCK();		\
	setsoftnet();			\
}

#ifndef LOCORE

#if	NETSYNC_LOCK
extern simple_lock_data_t	netisr_slock;
#endif
extern struct softnet_intr {
	short	active;			/* Softnet active count */
	short	pending;		/* Interrupt pending on queue */
	void	(*isr)();		/* Isr to process input */
	struct	ifqueue *ifq;		/* Queue to receive packets, or NULL */
	struct	domain *dom;		/* Domain isr belongs to, or NULL */
	int	flags;			/* Flags word */
#define ISRF_INCHDR	1		/*   Include datalink */
} softnet_intr[NETISR_MAX+1];		/* One extra for the wildcard */

#endif
#endif

#endif
