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
static char	*sccsid = "@(#)$RCSfile: netisr.c,v $ $Revision: 4.3.15.4 $ (DEC) $Date: 1993/11/03 22:25:39 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, 1991, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0.1
 */
/* 
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
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
 *	netisr.c - Kernel thread(s) for network code.
 *	Also does network initialization.
 *
 *	Revision History:
 *
 * 5-June-91	Heather Gray
 *	OSF 1.0.1 patch.
 *
 * 26-Mar-1991	Matt Thomas
 *	Add the hooks for DLI.
 *
 * 9-Apr-91	Peter .h Smith
 *	The routine net_threadstart is passed a hardcoded priority, which it
 *	assigns to the thread's sched_pri field.  Rather than change all the
 *	callers to use constants for priorities, we adjust the priority as
 *	needed in net_threadstart().  In the future, this should be cleaned
 *	up.
 * 6-Apr-91	Ron Widyono
 *	Ensure consistent lock counts for the RT_PREEMPT case in the
 *	netisr thread lock handoff.
 *
 */

#include <rt_preempt.h>

#include "net/net_globals.h"

#include "sys/param.h"

#include "sys/mbuf.h"
#include "sys/socket.h"
#include "sys/domain.h"
#include "sys/errno.h"
#include "sys/syslog.h"

#include "net/if.h"
#include "net/netisr.h"

#if	MACH
#include "sys/sysconfig.h"
#endif

#include <kern/sched.h>

LOCK_ASSERTL_DECL

struct softnet_intr softnet_intr[NETISR_MAX+1];	/* softnet structs */

#if	NETISR_THREAD
/* Configurable number of netisr threads (always at least 1). */
int netisrthreads = NNETTHREADS;	/* configurable as "pseudo-device" */

extern void (*sr_init_func)();

/* Can't pass parameters to threads, so put them here with protection. */
static vm_offset_t (*net_threadfunc)();
static lock_data_t net_threadlock, net_threadwait;
#endif
#if	NETSYNC_LOCK
simple_lock_data_t	netisr_slock;
#endif

void
netinit()
{
	int s;
	static char initialized;
	extern int trn_units;		/*
					 * Number of Token Ring adapters
					 * globally defined in
					 * net/if_ethersubr.c
					 */
	if (initialized)
		return;
	initialized = 1;

	/* Initialize mbufs and also netisr's (mbufs have one
	 * and are initialized earlier on some systems. */
	mbinit();

	/* Start thread(s) */
#if	NETISR_THREAD
	{
	int i = netisrthreads;
	int p = (i <= 1 ? 0 : 2);	/* High if 1, lower if more */
	lock_init(&net_threadlock, 1);
	lock_init(&net_threadwait, 1);
	netisrthreads = 0;
	do {
		net_threadstart(netisr_thread, p);
		++netisrthreads;
	} while (--i > 0);
	}
#endif

	/* Attach those devices which do not attach themselves */
	/* config() has previously attached hardware */
#include <sl.h>
#if	NSL > 0
	slattach();
#endif
	loattach();

	/* Initialize interface lists, global network data, domains */
	s = splimp();
	ifinit();
	domaininit();
	splx(s);

	/*
	 * Configure the domains. Test for dynamic options and defer
	 * to config manager if set, else look at actual options.
	 */

	/* Raw and routing sockets. Configure unconditionally. */
	route_config();

	/* Token Ring Source Routing option. Configure if Token Ring
	 * adapter presents. */
	if (trn_units)
		(*sr_init_func)();

	/* Unix-local sockets (includes pipes, fifos!) */
#if	defined(MACH) && !defined(UIPC)
#include <uipc.h>
#if	UIPC_DYNAMIC
#undef	UIPC
#define UIPC	0
#endif
#endif

#if	UIPC
	uipc_config();
#endif

	/* TCP/IP protocols */
#if	defined(MACH) && !defined(INET)
#include <inet.h>
#if	INET_DYNAMIC
#undef	INET
#define INET	0
#endif
#endif

#if	INET
#if	MACH
	inet_config(SYSCONFIG_CONFIGURE, (void *)NULL, 0, (void *)NULL, 0);
#else
	inet_config();
#endif
#endif

	/* XNS protocols */
#if	defined(MACH) && !defined(NS)
#include <ns.h>
#if	NS_DYNAMIC
#undef	NS
#define NS	0
#endif
#endif

#if	NS
#if	MACH
	ns_config(SYSCONFIG_CONFIGURE, (void *)NULL, 0, (void *)NULL, 0);
#else
	ns_config();
#endif
#endif

	/* Gateway screen option - gwscreen */
	gw_screen_init();

	/* DLI Family */

#if	defined(MACH) && !defined(DLI)
#include <dli.h>
#if	DLI
	dli_config();
#endif
#endif

	/*
	 * Call to configure LAT driver.  lat_config() is null if LAT
	 * is not configured into config file.
	 */
	lat_config(SYSCONFIG_CONFIGURE, (void *)NULL, 0, (void *)NULL, 0);

	/* Calls config functions for non-base system domains */
	netinit_domains();

}

/*
 * Add/delete isr's in input table. Isr's are specified by number,
 * interrupt routine, and optional input queue and domain.
 */
netisr_add(num, isr, ifq, dp)
	int num;
	void (*isr)();
	struct ifqueue *ifq;
	struct domain *dp;
{
	int s, err = 0;
	struct softnet_intr *softnet;

	if ((unsigned)++num > sizeof softnet_intr / sizeof softnet_intr[0] ||
	    isr == NULL)
		return EINVAL;
	softnet = &softnet_intr[num];
	if (dp)
		DOMAINRC_REF(dp);
	s = splimp();
	NETISR_LOCK();
	if (softnet->isr && softnet->isr != isr)
		err = EEXIST;
	else {
		softnet->active = 0;
		softnet->pending = 0;
		softnet->dom = dp;
		softnet->ifq = ifq;
		softnet->isr = isr;
		if ((num == NETISR_DLI+1) || (num == NETISR_OTHER+1))
		   softnet->flags = ISRF_INCHDR;
	}
	NETISR_UNLOCK();
	splx(s);
	if (err && dp)
		DOMAINRC_UNREF(dp);
	return err;
}

netisr_del(num)
	int num;
{
	int s, err = 0;
	register struct softnet_intr *softnet;
	struct domain *dp = 0;

	if ((unsigned)++num > sizeof softnet_intr / sizeof softnet_intr[0])
		return EINVAL;
	softnet = &softnet_intr[num];
	s = splimp();
	NETISR_LOCK();
	if (softnet->isr == NULL)
		err = ENOENT;
	else if (softnet->active)
		err = EBUSY;
	else {
		if (softnet->ifq) {
			IFQ_LOCK(softnet->ifq);
			for (;;) {
				register struct mbuf *m;
				IF_DEQUEUE_NOLOCK(softnet->ifq, m);
				if (m == NULL) break;
				m_freem(m);
				IF_DROP(softnet->ifq);
			}
			IFQ_UNLOCK(softnet->ifq);
		}
		dp = softnet->dom;
		softnet->active = 0;
		softnet->pending = 0;
		softnet->dom = NULL;
		softnet->ifq = NULL;
		softnet->isr = NULL;
	}
	NETISR_UNLOCK();
	splx(s);
	if (dp)
		DOMAINRC_UNREF(dp);
	return err;
}

/*
 * Receive packet for given isr. Packet is always freed.
 */
/*
 * Isr == -1 means just look for wildcard receiver.
 * Tries to avoid copies in case not deliverable to intended.
 *
 * N.B. I am unconvinced the raw isr is a useful construct, but it
 * doesn't hurt to have it at the moment. What it might try to provide 
 * is a way for passing packets up with no domains attached, and a
 * NIT-style interface. Note many protocol stacks step on the packet
 * buffer during processing so we pullup the headers for insurance.
 * Also note the loopback interface, among others, leaves few clues
 * to the packet's identity.
 */
netisr_input(num, m, header, hdrlen)
	register int num;
	struct mbuf *m;
	caddr_t header;
	int hdrlen;
{
	register int s, wild, err = 0;
	register struct ifqueue *ifq;
	struct softnet_intr *softnet;

	if ((unsigned)++num > sizeof softnet_intr / sizeof softnet_intr[0]) {
		err = EINVAL;
		num = 0;
	}
	softnet = &softnet_intr[num];
	s = splimp();
	NETISR_LOCK();
	wild = (num > 0 && softnet_intr[0].isr != NULL);
	if (softnet->isr == NULL) {
		if (softnet_intr[NETISR_DLI+1].isr) {
			softnet = &softnet_intr[NETISR_DLI+1];
			ifq = softnet->ifq;
			num = NETISR_DLI+1;
		} else {
			err = ENOENT;
		}
	}
	else
		ifq = softnet->ifq;
	NETISR_UNLOCK();
	if (err == 0) {
		if (ifq) {
			IFQ_LOCK(ifq);
			if (IF_QFULL(ifq)) {
				IF_DROP(ifq);	/* bump stat, leave err == 0 */
			} else {
				struct mbuf *mcopy = 0;
				if (wild) {
					mcopy=m_copym(m,0,M_COPYALL,M_DONTWAIT);
					if (mcopy)
					  mcopy = m_pullup(mcopy, MHLEN-hdrlen);
				}
				if ((softnet->flags & ISRF_INCHDR) &&
header && hdrlen > 0) {
					M_PREPEND(m, hdrlen, M_DONTWAIT);
					if (m)
						bcopy(header, mtod(m,
caddr_t), hdrlen);
				}
				if (m)
					IF_ENQUEUE_NOLOCK(ifq, m);
				m = mcopy;
			}
			IFQ_UNLOCK(ifq);
		}
		schednetisr(num-1);
	}
	splx(s);

	if (m) {
		if (wild) {
			if (header && hdrlen > 0) {
				M_PREPEND(m, hdrlen, M_DONTWAIT);
				if (m == NULL)
					return err;
				bcopy(header, mtod(m, caddr_t), hdrlen);
			}
			(void) netisr_input(-1, m, (caddr_t)0, 0);
		} else
			m_freem(m);
	}
	return err;
}

/*
 * Return ISR appropriate for address family (used by loopback).
 */
netisr_af(af)
{
	static int isrs[] =
		{ -1, -1, NETISR_IP, -1, -1, -1, NETISR_NS, NETISR_ISO };

	if ((unsigned)af > sizeof isrs / sizeof isrs[0])
		return -1;
	return isrs[af];
}

/*
 * Process network interrupts by type. May be called from a software
 * interrupt callout, or from thread context (see below).
 */
void
Netintr()
{
	register void (*isr)();
	register struct softnet_intr *softnet;
	register struct domain *dp;
	int s;

	s = splimp();
	NETISR_LOCK();
	for (;;) {
		isr = 0; dp = 0;
		softnet = &softnet_intr[0];
		do {
			if (softnet->pending) {
				isr = softnet->isr;
				dp = softnet->dom;
				++softnet->active;
				softnet->pending = 0;
				break;
			}
		} while (++softnet < &softnet_intr[NETISR_MAX+1]);
		if (!isr)
			break;
		NETISR_UNLOCK();
		splx(s);
#if	NETISR_THREAD
		clear_wait(current_thread(), THREAD_RESTART, FALSE);
#endif
		if (dp) {
			DOMAIN_FUNNEL_DECL(f)
			DOMAIN_FUNNEL(dp, f);
			(*isr)();
			DOMAIN_UNFUNNEL(f);
		} else
			(*isr)();
#if	NETISR_THREAD
		assert_wait((vm_offset_t)netisr_thread, FALSE);
#endif
		s = splimp();
		NETISR_LOCK();
		--softnet->active;
	}
	NETISR_UNLOCK();
	splx(s);
}

#if	NETISR_THREAD

/*
 * Common code for service thread mainlines. 
 * Call with function pointer to cycle periodically, and set
 * function up to return ticks to next call. Priority is optional.
 */

#if	NETSYNC_SPL
#include "kern/parallel.h"	/* for unix_master() */
#endif

void net_threadstart(func, pri)
	vm_offset_t (*func)();
	int pri;
{
	extern task_t first_task;
	thread_t thread;
	static void net_threadmain();

	lock_write(&net_threadlock);
	lock_write(&net_threadwait);
	net_threadfunc = func;
	thread = kernel_thread(first_task, net_threadmain);
	if (thread) {
		if (pri >= 0) {
#if RT_SCHED_RQ
			/* 
			 * Kludge to fix hardcoded priorities.  A priority
			 * below 31 is an old priority, because the realtime
			 * range contains at least 31 priorities.  Bump up
			 * the priority by adding BASEPRI_HIGHEST, which is
			 * the highest allowable system priority.  Right now
			 * it is 32.
			 */
			if (pri <= 31) pri += BASEPRI_HIGHEST;

			/* 
			 * The thread is not suspended, and not the current
			 * thread.  Can't just stomp on sched_pri, because
			 * the thread could be in a run queue.  Don't worry
			 * about thread_max_priority because it was set to
			 * BASEPRI_HIGHEST by kernel_thread.
			 *
			 * It would be more efficient to have each thread set
			 * its own priority, or to use kernel_thread_w_arg to
			 * pass the priority and func pointer to a stub
			 * in this module which then its own priority
			 * and then calls func.
			 */
			thread_priority(thread, pri, FALSE, TRUE);
#else /* RT_SCHED_RQ */
			thread->priority = pri;
			thread->sched_pri = pri;
#endif /* RT_SCHED_RQ */
		}
#if	MACH_LDEBUG
		/* The lock debugging would assert otherwise */
		simple_lock(&net_threadwait.interlock);
		net_threadwait.lthread = (char *) thread;
		dec_lock(&net_threadwait, current_thread());
		simple_unlock(&net_threadwait.interlock);
#elif	RT_PREEMPT
	DEC_LOCK_COUNT;
#endif
		lock_write(&net_threadwait);	/* wait... */
	} else
		log(LOG_ERR, "Can't start network thread 0x%x\n", func);
	lock_done(&net_threadwait);
	lock_done(&net_threadlock);
}

static void
net_threadmain()
{
	thread_t thread;
	int ticks;
	vm_offset_t (*func)();

	func = net_threadfunc;
#if	MACH_LDEBUG
	simple_lock(&net_threadwait.interlock);
	net_threadwait.lthread = (char *) current_thread();
	inc_lock(&net_threadwait, thread);
	simple_unlock(&net_threadwait.interlock);
#endif
	lock_done(&net_threadwait);
	thread = current_thread();
	thread_swappable(thread, FALSE);
#if	NETSYNC_SPL
	/* Bind thread to master cpu and pretend we're softclock() */
	unix_master();
	(void) splsoftclock();
#else
	spl0();
#endif

	/* Run (*func)() forever, waking it up after returned
	 * ticks expire. Note netisr threads don't return... */
	for (;;) {
		ticks = (*func)();
			/* <- TIMING HOLE if thread gets async wakeups */
		assert_wait((vm_offset_t)func, FALSE);
		if (ticks > 0) thread_set_timeout(ticks);
		thread_block();
	}
	/* NOTREACHED */
}

int
netisr_thread()
{
#if	NETSYNC_SPL
	(void) splnet();	/* All XXintr()'s need splnet() if spl */
#endif

	for (;;) {
		assert_wait((vm_offset_t)netisr_thread, FALSE);
		Netintr();		/* Process packets */
		thread_block();
	}
	/* NOTREACHED */
}
#endif
