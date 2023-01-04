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
static char	*sccsid = "@(#)$RCSfile: slave.c,v $ $Revision: 4.3 $ (DEC) $Date: 1992/01/15 01:59:52 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
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
 *	File:	slave.c
 *
 *	Copyright (C) 1985, Avadis Tevanian, Jr.
 *
 *	Misc. slave routines.
 */

#include <cpus.h>
#include <simple_clock.h>
#include <unix_locks.h>

#include <machine/reg.h>
#ifdef	ibmrt
#include <ca/scr.h>
#endif
#if	!defined(ibmrt) && !defined(mips)
#include <machine/psl.h>
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dk.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <machine/cpu.h>
#ifdef	vax
#include <vax/mtpr.h>
#endif

#include <kern/timer.h>

#include <kern/sched.h>
#include <kern/thread.h>
#include <mach/machine.h>

#ifdef	balance
#include <machine/intctl.h>
#endif

#if	!UNIX_LOCKS
/*
 * When UNIX_LOCKS is turned on, there are a variety of ways
 * in which psignal can be called from interrupt level, at
 * which time it is not possible to take unix_master (or any
 * unix locks).  Instead, we shuffle the signal off to a special
 * routine that queues the signal information to a dedicated thread
 * bound to the master processor.
 *
 * Otherwise, just use regular old psignal.
 */
#define	psignal_int		psignal
#endif

slave_main()
{
	slave_config();
	cpu_up(cpu_number());

	timer_init(&kernel_timer[cpu_number()]);
	start_timer(&kernel_timer[cpu_number()]);

	slave_start();
	/*NOTREACHED*/
}

slave_hardclock(pc, ps)
	caddr_t	pc;
	int	ps;
{
	register struct proc 	*p;
	register int 		cpstate;
	register struct utask 	*utaskp;
	thread_t	 	thread;

#if	SIMPLE_CLOCK
#define tick	myticks
	register int myticks;

	/*
	 *	Simple hardware timer does not restart on overflow, hence
	 *	interrupts do not happen at a constant rate.  Must call
	 *	machine-dependent routine to find out how much time has
	 *	elapsed since last interrupt.
	 */
	myticks = usec_elapsed();
#endif
#ifdef	lint
	pc++;
#endif

	thread = current_thread();
	utaskp = thread->u_address.utask;
	p = utaskp->uu_procp;

	/*
	 * Charge the time out based on the mode the cpu is in.
	 * Here again we fudge for the lack of proper interval timers
	 * assuming that the current state has been around at least
	 * one tick.
	 */
	if (USERMODE(ps)) {
		/*
		 * CPU was in user state.  Increment
		 * user time counter, and process process-virtual time
		 * interval timer. 
		 */
		U_TIMER_LOCK();
		if (timerisset(&utaskp->uu_timer[ITIMER_VIRTUAL].it_value) &&
		    itimerdecr(&utaskp->uu_timer[ITIMER_VIRTUAL], tick) == 0) {
			U_TIMER_UNLOCK();
			psignal_int(p, SIGVTALRM);
		} else
			U_TIMER_UNLOCK();
		if (p->p_nice > PRIZERO)
			cpstate = CP_NICE;
		else
			cpstate = CP_USER;

		/*
		 *	Profiling check.
		 */
		if (utaskp->uu_prof.pr_scale > 1) {
			p->p_flag |= SOWEUPC;
			aston();
		}

	} else {
		/*
		 * CPU was in system state.  If profiling kernel
		 * increment a counter.  If no process is running
		 * then this is a system tick if we were running
		 * at a non-zero IPL (in a driver).  If a process is running,
		 * then we charge it with system time even if we were
		 * at a non-zero IPL, since the system often runs
		 * this way during processing of system calls.
		 * This is approximate, but the lack of true interval
		 * timers makes doing anything else difficult.
		 */
		cpstate = CP_SYS;
		if ((thread->state & TH_IDLE) && BASEPRI(ps)) {
			cpstate = CP_IDLE;
		}
	}

	/*
	 * If the cpu is currently scheduled to a process, then
	 * charge it with resource utilization for a tick, updating
	 * statistics which run in (user+system) virtual time,
	 * such as the cpu time limit and profiling timers.
	 * This assumes that the current process has been running
	 * the entire last tick.
	 */
	if (!(thread->state & TH_IDLE)) {
	    if (utaskp->uu_rlimit[RLIMIT_CPU].rlim_cur != RLIM_INFINITY) {
		time_value_t	sys_time, user_time;

		thread_read_times(thread, &user_time, &sys_time);
		if ((sys_time.seconds + user_time.seconds + 1) >
		    utaskp->uu_rlimit[RLIMIT_CPU].rlim_cur) {
			psignal_int(p, SIGXCPU);
			if (utaskp->uu_rlimit[RLIMIT_CPU].rlim_cur <
			    utaskp->uu_rlimit[RLIMIT_CPU].rlim_max)
				utaskp->uu_rlimit[RLIMIT_CPU].rlim_cur += 5;
		}
	    }
	    U_TIMER_LOCK();
	    if (timerisset(&utaskp->uu_timer[ITIMER_PROF].it_value) &&
		itimerdecr(&utaskp->uu_timer[ITIMER_PROF], tick) == 0) {
		    U_TIMER_UNLOCK();
		    psignal_int(p, SIGPROF);
	    } else
		    U_TIMER_UNLOCK();
	}
	cp_time[cpstate]++;

}

#if	SIMPLE_CLOCK
#undef	tick
#endif
