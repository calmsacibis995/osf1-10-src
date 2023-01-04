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
static char	*rcsid = "@(#)$RCSfile: kern_clock.c,v $ $Revision: 4.3.17.10 $ (DEC) $Date: 1993/11/19 14:22:32 $";
#endif 
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
 * kern_clock.c
 *
 * Modification History:
 *
 * 04-Feb-92	Jeff Denham
 *	Update POSIX.4 timer references for modified timer structure.
 *
 * 02-Jan-92	Fred Canter
 *	Add code to hardclock to count cache parity errors.
 *
 * 04-Nov-91     Jeff Denham
 *	For P1003.4, in psx4_adjust_callout(), add typecasts to P.4
 *	timer structure type when referencing proc structure fields
 *	via the c_arg field in callout structure. This is for peace of
 *	mind and eliminates compiler warnings.
 *
 * 03-May-91	Peter H. Smith
 *	Change hardcoded priority to a constant.
 *
 * 24-Apr-91     Jeff Denham
 *	For P1003.4, adjust POSIX timers when settimeofday() is called.
 *
 * 4-Apr-91     Lai-Wah Hui
 *      Add P1003.4 required extensions.  
 *      Specifically <rt_timer.h> is now included and if RT_TIMER
 *      is defined  a routine was added to psx_untimeout. psx4_untimeout
 *      will cancel a timer and store the time remaining in the thread structure.
 *
 */

#include <simple_clock.h>
#include <stat_time.h>
#include <mach_co_stats.h>
#include <cpus.h>
#include <sys/dk.h>	/* for SAR counters */

#ifdef	hc
pragma off (optimize);
#endif

/*
 * Copyright (c) 1982, 1986 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 *

 */

#if defined(SVR4_COMPAT)
#	include <dec/svr4_defines.h>
#endif
#include <sys/unix_defs.h>
#include <kern/assert.h>
#include <kern/ast.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/dk.h>
#include <sys/callout.h>
#include <sys/user.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/table.h>
#if	MACH_CO_STATS
#include <mach_debug/callout_statistics.h>
#endif

#if PROFILING && PROFTYPE == 4
#include <sys/gprof.h>
#endif

#include <machine/reg.h>
#include <machine/cpu.h>
#if	!defined(romp) && !defined(mips)
#include <machine/psl.h>
#endif

#include <kern/thread.h>
#include <mach/machine.h>
#include <kern/sched.h>
#include <kern/sched_prim.h>
#include <kern/parallel.h>
#include <mach/time_value.h>
#include <kern/timer.h>
#include <sys/time.h>

#include <mach/boolean.h>
#include <rt_timer.h>

/*
 * The following is a pointer to prfintr() while profiling is enabled.
 * This must be declared as uninitialized data here so that the kernel
 * can be linked with or without the prf pseudo device driver.
 */
void (*prfintr_func)();

#ifndef PROFILING
extern	char *s_lowpc;
#endif

decl_simple_lock_data(,callout_lock)

lock_data_t	profil_lock;

struct callout *callfree, *callout, calltodo;
int ncallout;

static int fixcnt = 0;	/* For systems for which hz doesn't evenly divide
			   1000000, this counts the ticks till we need to
			   correct the time with fixtick */

#if	MACH_CO_STATS
#define	MCO_ASSERT(c)	assert(c)
#define	MCO_STATS(c)	(c)
#else
#define	MCO_ASSERT(c)
#define	MCO_STATS(c)
#endif

#if	UNIX_LOCKS
/*
 * A reminder about timeout handling and psignal.  The very
 * beginning of hardclock is handled on any processor, after
 * which all of the slave processors jump off to a routine
 * that does little more than check resource utilization
 * and possibly call psignal.  In that case, the psignal must
 * be rescheduled to happen on the master processor via
 * psignal_thread.
 *
 * The remainder of hardclock, including timeout handling,
 * is always done on the master processor.  We can avoid
 * passing psignal off to the psignal thread because we
 * "know" that there is no mp synchronization problem.
 *
 * One day, when proc manipulations are parallelized, this
 * ugliness will be fixed.
 *
 * Note that in the meantime, routines setting timeouts are
 * called back on the master processor!
 */
#endif

/*
 * Running the softclock in a thread is highly recommended.
 * So highly so it's not an option, but if desired here it is.
 */
#define SOFTCLOCK_THREAD 1

/*
 * Clock handling routines.
 *
 * This code is written to operate with two timers which run
 * independently of each other. The main clock, running at hz
 * times per second, is used to do scheduling and timeout calculations.
 * The second timer does resource utilization estimation statistically
 * based on the state of the machine phz times a second. Both functions
 * can be performed by a single clock (ie hz == phz), however the
 * statistics will be much more prone to errors. Ideally a machine
 * would have separate clocks measuring time spent in user state, system
 * state, interrupt state, and idle state. These clocks would allow a non-
 * approximate measure of resource utilization.
 */

/*
 * TODO:
 *	time of day, system/user timing, timeouts, profiling on separate timers
 *	allocate more timeout table slots when table overflows.
 */
#define BUMPTIME(t, usec) { \
	extern struct timeval *mach_tv; \
	register struct timeval *tp = (t); \
 \
	tp->tv_usec += (usec); \
	if (tp->tv_usec >= 1000000) { \
		tp->tv_usec -= 1000000; \
		tp->tv_sec++; \
	} \
	if (mach_tv) *mach_tv = time; \
}

/*
 * The hz hardware interval timer.
 * We update the events relating to real time.
 * If this timer is also being used to gather statistics,
 * we run through the statistics gathering routine as well.
 */

#define NTICKS	1

#ifdef  i386
int     dotimein = 0;
#define setsoftclock()  (dotimein++)
#undef	BASEPRI
#define	BASEPRI(X)	(0)
#endif

#ifdef notdef
int watch_dog_on = 0;
int watch_dog_interval = 0;
int watch_dog_time = 0;
int watch_dog_last_event = 0;
int watch_dog_event;
#endif /* notdef */

/*ARGSUSED*/
#ifdef	ibmrt
hardclock(dev,ps,pc)
	register dev_t dev;
	caddr_t pc;
	int ps;
#endif

#ifdef	i386
hardclock(pc,ps,oldpri)
	int oldpri;
	caddr_t pc;
	int ps;
#endif

/* #if	!defined(mips) && !defined(ibmrt) && !defined(i386) */
#if	!defined(ibmrt) && !defined(i386)
hardclock(pc, ps)
	caddr_t pc;
	int ps;
#endif
{
	register struct callout *p1;
	register thread_t	thread;
#if	SIMPLE_CLOCK
#define tick	myticks
	register int myticks;
#endif

	int needsoft = 0;
	int sig;
	extern int tickdelta;
	extern long timedelta;
        extern int fixtick;
#ifndef	multimax
	extern int doresettodr;
#endif

	thread = current_thread();

#if	SIMPLE_CLOCK
	/*
	 *	Simple hardware timer does not restart on overflow, hence
	 *	interrupts do not happen at a constant rate.  Must call
	 *	machine-dependent routine to find out how much time has
	 *	elapsed since last interrupt.
	 */
	myticks = usec_elapsed();

	/*
	 *	NOTE: tick was #define'd to myticks above.
	 */
#endif


	if (thread->state & TH_IDLE) {
#if	STAT_TIME
		timer_bump(&thread->system_timer, NTICKS*tick);
#endif
		clock_tick(NTICKS, CPU_STATE_IDLE);
	} else if (USERMODE(ps)) {
#if	STAT_TIME
		timer_bump(&thread->user_timer, NTICKS*tick);
#endif
		clock_tick(NTICKS, CPU_STATE_USER);
		/*
	 	 * Charge the time out based on the mode the cpu is in.
	 	 * Here again we fudge for the lack of proper interval timers
		 * assuming that the current state has been around at least
		 * one tick.
		 */
		if (u.u_prof.pr_scale > 1) {
			u.u_procp->p_flag |= SOWEUPC;
			aston();
		}
		/*
		 * CPU was in user state.  Increment
		 * user time counter, and process process-virtual time
		 * interval timer.
		 */
		/*
		 * Even though our thread is guaranteed not to be examining
		 * the value of this timer because we know that the thread
		 * was in user mode, some other thread in this task could
		 * be manipulating the virtual timer.
		 */
		/* s = splhigh(); XXX */
		U_TIMER_LOCK();
		if (timerisset(&u.u_timer[ITIMER_VIRTUAL].it_value) &&
		    itimerdecr(&u.u_timer[ITIMER_VIRTUAL], tick) == 0)
			sig = SIGVTALRM;
		else
			sig = 0;
		U_TIMER_UNLOCK();
		/* splx(s); */
		if (sig)
			psignal(u.u_procp, sig);
	} else {
#if	STAT_TIME
		timer_bump(&thread->system_timer, NTICKS*tick);
#endif
		clock_tick(NTICKS, CPU_STATE_SYSTEM);
	}
#if NCPUS > 1
	if (cpu_number() != master_cpu) {
		slave_hardclock(pc, ps);
		return;
	}
#endif
#ifdef	KTRACE
	kern_trace(400,time.tv_sec,time.tv_usec,0);
#endif  /* KTRACE */
#ifdef notdef
	if (watch_dog_on) {
		if (watch_dog_last_event != watch_dog_event) {
			watch_dog_time = 0;
			watch_dog_last_event = watch_dog_event;
		}
		else if (watch_dog_time++ > watch_dog_interval)
			panic("hardclock: watch_dog detected CPU IDLE");
	}
#endif /* notdef */

	/*
	 * Update real-time timeout queue.
	 * At front of queue are some number of events which are ``due''.
	 * The time to these is <= 0 and if negative represents the
	 * number of ticks which have passed since it was supposed to happen.
	 * The rest of the q elements (times > 0) are events yet to happen,
	 * where the time for each is given as a delta from the previous.
	 * Decrementing just the first of these serves to decrement the time
	 * to all events.
	 */
	/* s = splhigh(); XXX */
	simple_lock(&callout_lock);
	p1 = calltodo.c_next;
	while (p1) {
		if (--p1->c_time > 0)
			break;
		needsoft = 1;
		if (p1->c_time == 0)
			break;
		p1 = p1->c_next;
	}
	simple_unlock(&callout_lock);
	/* splx(s); */

	/*
	 * If the cpu is currently scheduled to a process, then
	 * charge it with resource utilization for a tick, updating
	 * statistics which run in (user+system) virtual time,
	 * such as the cpu time limit and profiling timers.
	 * This assumes that the current process has been running
	 * the entire last tick.
	 */
	if (!(thread->state & TH_IDLE))
	{
		if (u.u_rlimit[RLIMIT_CPU].rlim_cur != RLIM_INFINITY) {
		    time_value_t	sys_time, user_time;

		    thread_read_times(thread, &user_time, &sys_time);
		    if ((sys_time.seconds + user_time.seconds + 1) >
		        u.u_rlimit[RLIMIT_CPU].rlim_cur) {
			psignal(u.u_procp, SIGXCPU);
			if (u.u_rlimit[RLIMIT_CPU].rlim_cur <
			    u.u_rlimit[RLIMIT_CPU].rlim_max)
				u.u_rlimit[RLIMIT_CPU].rlim_cur += 5;
			}
		}
		/* s = splhigh(); XXX */
		U_TIMER_LOCK();
		if (timerisset(&u.u_timer[ITIMER_PROF].it_value) &&
		    itimerdecr(&u.u_timer[ITIMER_PROF], tick) == 0)
			sig = SIGPROF;
		else
			sig = 0;
		U_TIMER_UNLOCK();
		/* splx(s); */
		if (sig)
			psignal(u.u_procp, SIGPROF);
		{/* BSD/ULTRIX compatibility */
			register struct rusage * ru = &u.u_ru;
			register pmap_t pmap = thread->task->map->vm_pmap;

			if (ru->ru_maxrss < pmap_resident_count(pmap))
				ru->ru_maxrss = pmap_resident_count(pmap);
			/* assume all of stack is resident */
			ru->ru_isrss += u.u_ssize;
			ru->ru_ixrss += pmap_resident_text(pmap);
			ru->ru_idrss += pmap_resident_count(pmap) -
					 pmap_resident_text(pmap) - u.u_ssize;
		}
	}

	/* call the prf psuedo driver if currently sampling the pc */
	if (prfintr_func)
		(*prfintr_func)(ps, pc, cpu_number());

	/*
	 * If the alternate clock has not made itself known then
	 * we must gather the statistics.
	 */
	if (phz == 0)
		gatherstats(pc, ps);

	/*
	 * Increment the time-of-day, and schedule
	 * processing of the callouts at a very low cpu priority,
	 * so we don't keep the relatively high clock interrupt
	 * priority any longer than necessary.
	 */
	/* s = splhigh(); XXX */
	TIME_WRITE_LOCK();
	/*
	 * If this clocks frequency doesn't divide evenly into a second
         * bump the clock by the remainder once every second
         */
	if (fixtick) {
		fixcnt++;
		if (fixcnt >= hz) {
			BUMPTIME(&time, fixtick);
			fixcnt = 0;
		}
	}
	if (timedelta == 0)
		BUMPTIME(&time, tick)
	else {
		register delta;

		if (timedelta < 0) {
			delta = tick - tickdelta;
			timedelta += tickdelta;
		} else {
			delta = tick + tickdelta;
			timedelta -= tickdelta;
		}
		BUMPTIME(&time, delta);
#if	!defined(multimax)
		if (timedelta == 0 && doresettodr) {
			doresettodr = 0;
			resettodr();
		}
#endif
	}
	TIME_WRITE_UNLOCK();
	/* splx(s); */

#if	SOFTCLOCK_THREAD
	if (needsoft) {
		if (BASEPRI(ps)) {
			/*
			 * Optimization: use thread only if needed.
			 */
			softclock_scan(0);
		} else {
			extern softclock();
			thread_wakeup_one((vm_offset_t)softclock);
			aston();
		}
	}
#else	/* !SOFTCLOCK_THREAD */
/* Code retained for fallback - not recommended */
	if (needsoft) {
#ifdef	i386
		setsoftclock();
#else
		if (BASEPRI(ps)) {
			/*
			 * Save the overhead of a software interrupt;
			 * it will happen as soon as we return, so do it now.
			 */
			(void) splsoftclock();
#if	defined(sun3) || defined(sun4)
			softclock(USERMODE(ps) != 0);
#endif
#if	defined(vax) || defined(ns32000) || defined(ibmrt) || defined(__hp_osf) || defined(mips) || defined (__alpha)
			softclock(pc, ps);
#endif
		} else {
#if	defined(sun3) || defined(sun4)
			int softclock();
			softcall(softclock, USERMODE(ps) != 0);
#else
			setsoftclock();
#endif
		}
#endif	/* !i386 */
	}
#endif	/* !SOFTCLOCK_THREAD */
}
#if	SIMPLE_CLOCK
#undef	tick
#endif	

int	dk_ndrive = DK_NDRIVE;
/*
 * Gather statistics on resource utilization.
 *
 * We make a gross assumption: that the system has been in the
 * state it is in (user state, kernel state, interrupt state,
 * or idle state) for the entire last time interval, and
 * update statistics accordingly.
 */
/*ARGSUSED*/
gatherstats(pc, ps)
	caddr_t pc;
	int ps;
{
	register int cpstate, s;

	/*
	 * Determine what state the cpu is in.
	 */
	if (USERMODE(ps)) {
		/*
		 * CPU was in user state.
		 */
		if (u.u_procp->p_nice > PRIZERO)
			cpstate = CP_NICE;
		else
			cpstate = CP_USER;
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
		if ((current_thread()->state & TH_IDLE) && BASEPRI(ps)) {
			extern int sar_bio_state;
			if (sar_bio_state)
				cpstate = CP_WAIT;
			else
				cpstate = CP_IDLE;
		}

#if PROFILING && PROFTYPE == 4
		{	extern u_int *kcount;
			extern char *s_lowpc;
			extern u_long s_textsize;
			extern int profiling;

			s = pc - s_lowpc;
			if (profiling < 2 && s < s_textsize && kcount)
			{ kcount[s / (HISTFRACTION * sizeof (*kcount))]++; }
		}
#endif	/* PROFILING && PROFTYPE == 4 */
	}
	/*
	 * We maintain statistics shown by user-level statistics
	 * programs:  the amount of time in each cpu state.
	 */
	cp_time[cpstate]++;

	/* waste of time since no DEC disk controllers use the dk_busy
	 * field.  
	for (s = 0; s < DK_NDRIVE; s++)
		if (dk_busy & (1 << s))
			dk_time[s]++;
	*/
}

#if	!SOFTCLOCK_THREAD
/* Code retained for fallback - not recommended */
/*
 * Software priority level clock interrupt.
 * Run periodic events from timeout queue.
 */

#if	defined(sun3) || defined(sun4)
softclock(was_user_mode)
	int	was_user_mode;
#endif

#if	!defined(sun3) && !defined(sun4)
/*ARGSUSED*/
softclock(pc, ps)
	caddr_t pc;
	int ps;
#endif
{
#ifdef	mips
	acksoftclock();
#endif
#ifdef	KTRACE
	kern_trace(401,0,0,0);
#endif  /* KTRACE */
	for (;;) {
		register struct callout *p1;
		register caddr_t arg;
		register int (*func)();
		register int a, s;

		s = splhigh();
		simple_lock(&callout_lock);
		if ((p1 = calltodo.c_next) == 0 || p1->c_time > 0) {
			simple_unlock(&callout_lock);
			splx(s);
			break;
		}
		arg = p1->c_arg; func = p1->c_func; a = p1->c_time;
		calltodo.c_next = p1->c_next;
		p1->c_next = callfree;
		callfree = p1;
		MCO_ASSERT(callout_statistics.cos_current_size > 0);
		MCO_STATS(callout_statistics.cos_num_softclock++);
		MCO_STATS(callout_statistics.cos_current_size--);
		MCO_STATS(callout_statistics.cos_cum_softclock_size += \
			callout_statistics.cos_current_size);
		MCO_ASSERT(callout_statistics_invariant());
		simple_unlock(&callout_lock);
		splx(s);
		(*func)(arg, a);
	}
	/*
	 * If trapped user-mode and profiling, give it
	 * a profiling tick.
	 */
#if	defined(sun3) || defined(sun4)
	if (was_user_mode) {
#else
	if (USERMODE(ps)) {
#endif
		register struct proc *p = u.u_procp;

		if (u.u_prof.pr_scale > 1) {
			p->p_flag |= SOWEUPC;
			aston();
		}
	}
}
#else	/* SOFTCLOCK_THREAD */
/*
 * Software priority level clock "interrupt", handled by a thread.
 * Run periodic events from timeout queue.
 */
softclock()
{
	panic("softclock");
}

softclock_scan(flag)
{
	register struct callout *p1;
	register caddr_t arg;
	register int (*func)(), a;
	int s = splhigh();

	for (;;) {
		simple_lock(&callout_lock);
		if ((p1 = calltodo.c_next) == 0 || p1->c_time > 0)
			break;
		arg = p1->c_arg; func = p1->c_func; a = p1->c_time;
		calltodo.c_next = p1->c_next;
		p1->c_next = callfree;
		callfree = p1;
		MCO_ASSERT(callout_statistics.cos_current_size > 0);
		MCO_STATS(callout_statistics.cos_num_softclock++);
		MCO_STATS(callout_statistics.cos_current_size--);
		MCO_STATS(callout_statistics.cos_cum_softclock_size += \
			callout_statistics.cos_current_size);
		MCO_ASSERT(callout_statistics_invariant());
		simple_unlock(&callout_lock);
		(void) splsoftclock();
		(*func)(arg, a);
		(void) splhigh();
	}
	if (flag)
		assert_wait((vm_offset_t)softclock, FALSE);
	simple_unlock(&callout_lock);
	splx(s);
}

softclock_thread()
{
	thread_t thread = current_thread();

	thread_swappable(thread, FALSE);
/*
 * RT_SCHED: Change hardcoded priority to a constant.  Always valid.
 * The constant is defined in kern/sched.h.
 * Stomping on sched_pri here won't hurt, because the thread is running and
 * therefore not on a run queue.
 */
	thread->priority = thread->sched_pri = BASEPRI_SOFTCLOCK;
	unix_master();		/* XXX signals sent in timeouts */
	(void) spl0();

	for (;;) {
		softclock_scan(1);
		thread_block();
	}
	/* NOTREACHED */
}

thread_t softclock_thread_ptr; /* used for debugging crash dumps */

softclock_init()
{
	extern task_t first_task;

	softclock_thread_ptr = kernel_thread(first_task, softclock_thread);
}
#endif	/* SOFTCLOCK_THREAD */

/*
 * Arrange that (*fun)(arg) is called in t/hz seconds.
 */
timeout(fun, arg, t)
	int (*fun)();
	caddr_t arg;
	register int t;
{
	register struct callout *p1, *p2, *pnew;
	register int s = splhigh();
#if	MACH_CO_STATS
	register int pos = 0;
#endif
#ifdef	KTRACE
	kern_trace(402,fun,arg,t);
#endif  /* KTRACE */
	simple_lock(&callout_lock);
	if (t <= 0)
		t = 1;
	pnew = callfree;
	if (pnew == NULL)
		panic("timeout table overflow");
	callfree = pnew->c_next;
	pnew->c_arg = arg;
	pnew->c_func = fun;
	for (p1 = &calltodo; (p2 = p1->c_next) && p2->c_time < t; p1 = p2) {
		if (p2->c_time > 0)
			t -= p2->c_time;
		MCO_STATS(pos++);
	}
	p1->c_next = pnew;
	pnew->c_next = p2;
	pnew->c_time = t;
	if (p2)
		p2->c_time -= t;
	MCO_STATS(callout_statistics.cos_num_timeout++);
	MCO_STATS(callout_statistics.cos_cum_timeout_size += \
		callout_statistics.cos_current_size);
	MCO_STATS(callout_statistics.cos_cum_timeout_pos += pos);
	MCO_STATS(callout_statistics.cos_current_size++);
	MCO_ASSERT(callout_statistics.cos_current_size > 0);
	MCO_ASSERT(callout_statistics_invariant());
	simple_unlock(&callout_lock);
	splx(s);
}

/*
 * untimeout is called to remove a function timeout call
 * from the callout structure.
 */
untimeout(fun, arg)
	int (*fun)();
	caddr_t arg;
{
	register struct callout *p1, *p2;
	register int s;
#if	MACH_CO_STATS
	register int pos = 0;
#endif
#ifdef	KTRACE
	kern_trace(403,fun,arg,0);
#endif  /* KTRACE */
	s = splhigh();
	simple_lock(&callout_lock);
#if	MACH_CO_STATS
	callout_statistics.cos_num_untimeout++;
#endif
	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
		if (p2->c_func == fun && p2->c_arg == arg) {
			if (p2->c_next && p2->c_time > 0)
				p2->c_next->c_time += p2->c_time;
			p1->c_next = p2->c_next;
			p2->c_next = callfree;
			callfree = p2;
			MCO_ASSERT(callout_statistics.cos_current_size > 0);
			MCO_STATS(callout_statistics.cos_num_untimeout_hit++);
			MCO_STATS(callout_statistics.cos_current_size--);
			MCO_STATS(callout_statistics.cos_cum_untimeout_pos += pos);
			break;
		}
		MCO_STATS(pos++);
	}
	MCO_STATS(callout_statistics.cos_cum_untimeout_size += \
		callout_statistics.cos_current_size);
	MCO_ASSERT(callout_statistics_invariant());
	simple_unlock(&callout_lock);
	splx(s);
}

#if	NCPUS > 1
/*
 * untimeout_try is a multiprocessor version of timeout that returns
 * a boolean indicating whether it successfully removed the entry.
 */
boolean_t
untimeout_try(fun, arg)
	int (*fun)();
	caddr_t arg;
{
	register struct callout *p1, *p2;
	register int s;
	register boolean_t	ret = FALSE;
#if	MACH_CO_STATS
	register int pos = 0;
#endif

	s = splhigh();
	simple_lock(&callout_lock);
#if	MACH_CO_STATS
	callout_statistics.cos_num_untimeout++;
#endif
	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
		if (p2->c_func == fun && p2->c_arg == arg) {
			if (p2->c_next && p2->c_time > 0)
				p2->c_next->c_time += p2->c_time;
			p1->c_next = p2->c_next;
			p2->c_next = callfree;
			callfree = p2;
			MCO_ASSERT(callout_statistics.cos_current_size > 0);
			MCO_STATS(callout_statistics.cos_num_untimeout_hit++);
			MCO_STATS(callout_statistics.cos_current_size--);
			MCO_STATS(callout_statistics.cos_cum_untimeout_pos += pos);
			ret = TRUE;
			break;
		}
		MCO_STATS(pos++);
	}
	MCO_STATS(callout_statistics.cos_cum_untimeout_size +=
		callout_statistics.cos_current_size);
	MCO_ASSERT(callout_statistics_invariant());
	simple_unlock(&callout_lock);
	splx(s);
	return(ret);
}
#endif	/* NCPUS > 1 */

/*
 * Compute number of hz until specified time.
 * Used to compute third argument to timeout() from an
 * absolute time.
 */
hzto(tv)
	struct timeval *tv;
{
	register int ticks;
	register int sec, usec;

	int s = splhigh();
	TIME_READ_LOCK();
	sec = tv->tv_sec - time.tv_sec;
	usec = tv->tv_usec - time.tv_usec;
	TIME_READ_UNLOCK();
	splx(s);

	/*
	 * If number of seconds will fit in 32 bit arithmetic,
	 * then compute number of seconds to time and scale to
	 * ticks.  Otherwise round times greater than representible
	 * to maximum value.
	 *
	 * hz may range from 1 to 2147 without loss of (32-bit) precision.
	 * Maximum value for any timeout depends on hz.
	 * Must potentially correct for roundoff error in tick (1000000/hz)
	 * when passed as tv = { 0, tick }.
	 */
	if (sec + 1 <= INT_MAX / hz) {
		ticks = (sec * hz) + (((usec + tick - 1) * hz) / (1000*1000));
		if (ticks <= 0) ticks = 1;
	} else
		ticks = INT_MAX;
	return (ticks);
}

/* ARGSUSED */
profil(p, args, retval)
	struct proc *p;
	void *args;
	long *retval;
{
	register struct args {
		short	*bufbase;
		unsigned long bufsize;
		unsigned long pcoffset;
		unsigned long pcscale;
	} *uap = (struct args *)args;
	register struct uuprof *upp = &u.u_prof;
	extern lock_data_t profil_lock;

	lock_write(&profil_lock);
	upp->pr_base = uap->bufbase;
	upp->pr_size = uap->bufsize;
	upp->pr_off = uap->pcoffset;
	upp->pr_scale = uap->pcscale;
	lock_done(&profil_lock);
	return (0);
}

void
profil_lock_init()
{
	extern lock_data_t profil_lock;

	lock_init(&profil_lock,TRUE);
}

#if RT

/*
 * psx4_untimeout is called to remove a function timeout call
 * from the callout structure.
 */
psx4_untimeout(fun, arg)
	int (*fun)();
	caddr_t arg;
{
	register struct callout *p1, *p2;
	register int s;
        int cnt;
#if	MACH_CO_STATS
	register int pos = 0;
#endif

	s = splhigh();
	simple_lock(&callout_lock);
#if	MACH_CO_STATS
	callout_statistics.cos_num_untimeout++;
#endif
        cnt = 0;

	for (p1 = &calltodo; (p2 = p1->c_next) != 0; p1 = p2) {
                cnt += p2->c_time; 
		if (p2->c_func == fun && p2->c_arg == arg) {
			if (p2->c_next && p2->c_time > 0)
				p2->c_next->c_time += p2->c_time;
			p1->c_next = p2->c_next;
			p2->c_next = callfree;
			callfree = p2;
			MCO_ASSERT(callout_statistics.cos_current_size > 0);
			MCO_STATS(callout_statistics.cos_num_untimeout_hit++);
			MCO_STATS(callout_statistics.cos_current_size--);
			MCO_STATS(callout_statistics.cos_cum_untimeout_pos += pos);
			break;
		}
		MCO_STATS(pos++);
	}
	MCO_STATS(callout_statistics.cos_cum_untimeout_size += \
		callout_statistics.cos_current_size);
	MCO_ASSERT(callout_statistics_invariant());
	simple_unlock(&callout_lock);
	splx(s);

        return(cnt);
}
#endif

#if RT_TIMER
/*
 * P1003.4 psx4_adjust_callout() -- adjusts .4 timers in callout
 *	queue in response to settimeofday() calls.
 *
 * Environment:
 *  This function is called from setthetime() in bsd/kern_time.c.
 *  It is called at splhigh() with TIME_WRITE_LOCK locked.
 * 
 * Abstract:
 *  If we are in a POSIX timer environment, we want to scan the
 *  callout queue and adjust pending POSIX timers for the change
 *  in the system time. Relative timers retain their position
 *  in the callout queue but have their expected timeout times
 *  adjusted. Absolute timers are moved in the queue to reflect
 *  the change in the system time.
 *
 * Inputs:
 *  tv -- pointer to new timeval specified to settimeofday()
 *
 */

void
psx4_adjust_callout(tv)
struct timeval *tv;
{
	register struct callout abs, *p1, *p2, *a1 = &abs;
	register int pdelta = 0, adelta = 0;
	struct timeval tdelta;
	int tickdelta = 0;

	tdelta = time;
	timevalsub(&tdelta, tv);

	if (tdelta.tv_sec + 1 <= INT_MAX / hz) {
		tickdelta = (tdelta.tv_sec * hz) + (((tdelta.tv_usec + tick - 1) * hz) / (1000*1000));
	} else
		tickdelta = INT_MAX;

	abs.c_next = NULL;

	/*
	 * Step 1: Take absolute POSIX timers out of the callout queue,
	 * readjust tick values, and insert into a temporary list.
	 * POSIX timers with relative waits need to have their
	 * time values likewise adjusted.
	 */

	for (p1 = &calltodo; (p2 = p1->c_next) != NULL; ) {
		if (p2->c_func == psx4_tod_expire) { 	/* if a POSIX timer */
			if (((psx4_timer_t *)p2->c_arg)->psx4t_type & TIMER_ABSTIME) {
				/* This is an absolute wait */
				if (p2->c_next && p2->c_time > 0)
					p2->c_next->c_time += p2->c_time;
				p1->c_next = p2->c_next;
				a1->c_next = p2;
				a1 = p2;
				a1->c_time = (a1->c_time + pdelta) - adelta;
				adelta += a1->c_time;
				a1->c_next = NULL;
			}
			else {
				/* This is a relative wait -- adjust time. */
				PROC_TIMER_LOCK(((psx4_timer_t *)p2->c_arg)->psx4t_p_proc);
				timevalsub(&((psx4_timer_t *)p2->c_arg)->psx4t_timeval.it_value,
					&tdelta);
				PROC_TIMER_UNLOCK(((psx4_timer_t *)p2->c_arg)->psx4t_p_proc);
				pdelta += p2->c_time;
				p1 = p2;
			}
		}
		else {
			/* This isn't any kind of POSIX timer. Go to next. */
			pdelta += p2->c_time;
			p1 = p2;
		} 
	}

	/*
	 * Step 2: Adjust the first entry in the new absolute queue to
	 * reflect change in system time. This is done on a "tick" basis.
	 * If the time was set back, tickdelta is positive.
	 */

	if (abs.c_next != NULL)
		abs.c_next->c_time += tickdelta;

	/*
	 * Step 3: Insert adjusted entries back into callout queue, adjusting
	 * tick counts as necessary. Walk the calltodo list until it's
	 * empty, then see whether there's anything left in the absolute list.
	 */

	p1 = &calltodo;
	a1 = abs.c_next;

	while (((p2 = p1->c_next) != NULL) && a1) {

		/* 
		 * If less, insert and decrement next time by new entry.
		 * If greater, decrement new entry time by current time and
		 *   move to next entry. If there is no next entry, insert.
		 */

			if (a1->c_time < p2->c_time) {
				abs.c_next = a1->c_next; /* remove entry */
				p1->c_next = a1;
				a1->c_next = p2;
				p2->c_time -= a1->c_time;
				p1 = a1;
				a1 = abs.c_next;
			}
			else {
				a1->c_time -= p2->c_time;
				p1 = p2;
			}
	}

	if (a1 != NULL) {
		p1->c_next = a1;
	}

}
#endif /* RT_TIMER */
