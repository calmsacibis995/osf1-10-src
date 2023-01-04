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
static char *rcsid = "@(#)$RCSfile: sched_prim.c,v $ $Revision: 4.3.25.12 $ (DEC) $Date: 1993/11/19 14:23:03 $";
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
 *	File:	sched_prim.c
 *	Author:	Avadis Tevanian, Jr.
 *
 *	Copyright (C) 1986, Avadis Tevanian, Jr.
 *
 *	Scheduling primitives
 *
 * Historical summary:
 *
 *	Redo priority recomputation. [dlb, 29 feb 88]
 *	New accurate timing. [dlb, 19 feb 88]
 *	Simplified choose_thread and thread_block. [dlb, 18 dec 87]
 *	Add machine-dependent hooks in idle loop. [dbg, 24 nov 87]
 *	Quantum scheduling changes. [dlb, 14 oct 87]
 *	Replaced scheduling logic with a state machine, and included
 *	 timeout handling. [dbg, 05 oct 87]
 *	Deactivate kernel pmap in idle_thread. [dlb, 23 sep 87]
 *	Favor local_runq in choose_thread. [dlb, 23 sep 87]
 *	Hacks for master processor handling. [rvb, 12 sep 87]
 *	Improved idle cpu and idle threads logic. [dlb, 24 aug 87]
 *	Priority computation improvements. [dlb, 26 jun 87]
 *	Quantum-based scheduling. [avie, dlb, apr 87]
 *	Improved thread swapper. [avie, 13 mar 87]
 *	Lots of bug fixes. [dbg, mar 87]
 *	Accurate timing support. [dlb, 27 feb 87]
 *	Reductions in scheduler lock contention. [dlb, 18 feb 87]
 *	Revise thread suspension mechanism. [avie, 17 feb 87]
 *	Real thread handling [avie, 31 jan 87]
 *	Direct idle cpu dispatching. [dlb, 19 jan 87]
 *	Initial processor binding. [avie, 30 sep 86]
 *	Initial sleep/wakeup. [dbg, 12 jun 86]
 *	Created. [avie, 08 apr 86]
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 02-May-91	Peter H. Smith
 *	Clean up based on code review comments.
 *
 * 21-Apr-91	Ron Widyono
 *	Remove RT_PREEMPT debugging.
 *
 * 11-Apr-91     Lai-Wah
 *      Add P1003.4 required extensions.  
 *      Specifically <rt_timer> is now included and if RT_TIMER
 *      is defined the clear_wait will call psx4_untimeout instead
 *      of timeout if a sleep interrupted to get the time remaining.
 *
 * 9-Apr-91	Peter H. Smith
 *	Add fixed priority policies POLICY_FIFO and POLICY_RR, extend to
 *	64 run queues, and add a run queue bitmask.  Maintain correctness
 *	of runq->low and the bitmask at all times.
 *	The new policies require modifications to allow preemption back to the
 *	head of the run queue, quantum preservation and completion, and making
 *	sure that do_thread_scan() leaves fixed priority threads alone.
 *	Add some scheduler monitoring code.
 *
 * 6-Apr-91	Ron Widyono
 *	Modify preemption requests to use Kernel Mode ASTs for preemption
 *	points.  Conditionalized by RT_PREEMPT.
 *	
 */

#include <cpus.h>
#include <simple_clock.h>
#include <mach_host.h>
#include <mach_km.h>
#ifndef	mips		/* DEC mips always uses fast csw interfaces */
#include <fast_csw.h>
#endif
#include <hw_footprint.h>
#include <mach_ldebug.h>
#include <rt_preempt.h>
#include <rt_preempt_debug.h>
#include <rt_sched.h>		/* Holds setting of RT_SCHED. */
#include <rt_sched_opt.h>	/* Holds setting of RT_SCHED_OPT. */

#include <sys/types.h>

#include <kern/ast.h>
#include <kern/queue.h>
#include <kern/thread.h>
#include <kern/lock.h>
#include <mach/policy.h>
#include <kern/sched.h>
#include <mach/machine.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <vm/vm_umap.h>
#include <kern/parallel.h>
#include <machine/machparam.h>	/* For def'n of splsched() */
#include <machine/pmap.h>

#include <machine/cpu.h>

#include <kern/processor.h>
#include <kern/sched_prim.h>
#include <kern/thread_swap.h>
#include <kern/task.h>
#include <sys/user.h>
#include <kern/task_swap.h>

#include <kern/macro_help.h>
#include <sys/sched_mon.h>	/* RT_SCHED_MON+ */
#include <sys/lwc.h>
int kernel_thread_fixed_pri_disable;

extern int hz, thread_swap_tick;

extern	int 	atintr_level;		/* Interrupt nesting level */

long		swtch_tsk_ctxt_cnt;	/* Task context switch count */
long		swtch_thrd_ctxt_cnt;	/* Thread context switch count */

int		round_robin_switch_rate; /* switchs per second */
int		min_quantum;	/* defines max context switch rate */

#if RT_SCHED
/*
 * Define a separate global for the POSIX 1003.4 SCHED_RR round-robin
 * interval.  For now, this is assigned min_quantum converted to milliseconds.
 * In the future, we may want to make this be settable or at least 
 * configurable, either on a per-system basis (this mechanism), per-processor
 * basis (use pset->min_quantum as default), per-thread basis (put it in the
 * sched_param structure), or table driven basis (ala SVID3).
 */
int		rt_sched_rr_interval;	/* Round-Robin interval */
#endif /* RT_SCHED */

unsigned	sched_tick;

#if	SIMPLE_CLOCK
int		sched_usec;
#endif

#if	MACH_KM
#include <kern/kern_mon.h>
#endif

thread_t	sched_thread_id;

/*
 *	State machine
 *
 * states are combinations of:
 *  R	running
 *  W	waiting (or on wait queue)
 *  S	suspended (or will suspend)
 *  N	non-interruptible
 *  O	swapped out
 *
 * init	action 								 swap
 *	assert_wait	thread_block	clear_wait	suspend	resume	out in
 *
 * R	RW, RWN		R;   setrun	-		RS	-	-
 * RS	RWS, RWNS	S;  wake_active	-		-	R	-
 * RN	RWN		RN;  setrun	-		RNS	-	-
 * RNS	RWNS		RNS; setrun	-		-	RN	-
 *
 * RW			W		R		RWS	-	-
 * RWN			WN		RN		RWNS	-	-
 * RWS			WS; wake_active	RS		-	RW	-
 * RWNS			WNS		RNS		-	RWN	-
 *
 * W					R;   setrun	WS	-	WO
 * WN					RN;  setrun	WNS	-	-
 * WNS					RNS; setrun	-	WN	-
 * WO					RO;  swapin	WSO	-	-
 *
 * S					-		-	R	SO
 * SO					-		-	RO	-
 * WS					S		-	W	WSO
 * WSO					SO		-	WO	-
 *
 * RO					-		RSO	-	     R
 * RSO					-		-	RO	     RS
 */

/*
 *	Waiting protocols and implementation:
 *
 *	Each thread may be waiting for exactly one event; this event
 *	is set using assert_wait().  That thread may be awakened either
 *	by performing a thread_wakeup_prim() on its event,
 *	or by directly waking that thread up with clear_wait().
 *
 *	The implementation of wait events uses a hash table.  Each
 *	bucket is queue of threads having the same hash function
 *	value; the chain for the queue (linked list) is the run queue
 *	field.  [It is not possible to be waiting and runnable at the
 *	same time.]
 *
 *	Locks on both the thread and on the hash buckets govern the
 *	wait event field and the queue chain field.  Because wakeup
 *	operations only have the event as an argument, the event hash
 *	bucket must be locked before any thread.
 *
 *	Scheduling operations may also occur at interrupt level; therefore,
 *	interrupts below splsched() must be prevented when holding
 *	thread or hash bucket locks.
 *
 *	The wait event hash table declarations are as follows:
 */

#define NUMQUEUES	59

queue_head_t		wait_queue[NUMQUEUES];
decl_simple_lock_data(,	wait_lock[NUMQUEUES])

#define wait_hash(event) \
	(((long)(event) < 0 ? ((event) ^ -1L) : (event))%NUMQUEUES)

#if RT_SCHED_RQ
/*
 * RT_SCHED_UPDATE_RQ and RT_SCHED_UPDATE_RQ_LOW maintain the consistency of
 * the run queue bitmask and runq->low.  RT_SCHED_UPDATE_RQ_LOW is passed the
 * address of the run queue and the address of the queue header which is
 * indexed by the current value of runq->low.  (Note that the second argument
 * is redundant, but is there for optimization.)  If the queue header
 * represents an empty queue, the bit corresponding to the current setting of
 * runq->low is cleared, and runq->low is adjusted to hold the index of the
 * first nonzero bit in the mask (which corresponds to the first nonempty
 * run queue).
 *
 * RT_SCHED_UPDATE_RQ is similar, but it updates an arbitrary queue.  The
 * second and third arguments are the address of the queue header, and the
 * index of the queue header.  If the queue header and index correspond to
 * the current setting of runq->low, then the behavior is the same as that
 * of RT_SCHED_UPDATE_RQ_LOW.  Otherwise, the specified queue header is
 * checked and the bit corresonding to the index is cleared, but runq->low
 * is not changed.
 *
 * In an effort to improve performance, the code to keep the run queue mask
 * and run queue low mark consistent was made an assembler routine.  This
 * was undertaken because the gnu compiler generated an awful lot of code.
 * The machine code should not be needed when decent optimizing compilers
 * are used.  The RT_SCHED_OPT option enables the machine-coded routines.
 */
#if RT_SCHED_OPT
extern int runq_update(run_queue_t,int);
#define RT_SCHED_UPDATE_RQ_LOW(rq,q) runq_update_low(rq);
#define RT_SCHED_UPDATE_RQ(rq,q,pri) runq_update(rq,pri);
#else /* RT_SCHED_OPT */
#define RT_SCHED_UPDATE_RQ_LOW(rq,q)					\
       if (queue_empty(q)) {						\
	 RT_SCHED_CLEAR_RUNQ_BIT(&(rq)->mask,(rq)->low);		\
	 (rq)->low = find_first_runq_bit_set(&(rq)->mask);		\
       }
#define RT_SCHED_UPDATE_RQ(rq,q,pri)					\
       if (queue_empty(q)) {						\
	 RT_SCHED_CLEAR_RUNQ_BIT(&(rq)->mask,pri);			\
	 if ((pri) == (rq)->low) {					\
	   (rq)->low = find_first_runq_bit_set(&(rq)->mask);		\
	 }								\
       }
#endif /* RT_SCHED_OPT */
#else /* RT_SCHED_RQ */
#define RT_SCHED_UPDATE_RQ_LOW(rq,q)
#define RT_SCHED_UPDATE_RQ(rq,q,pri)
#endif /* RT_SCHED_RQ */

void sched_init()
{

	if ((round_robin_switch_rate > 0) && (round_robin_switch_rate  <= hz))
		min_quantum = hz / round_robin_switch_rate;
	else 
		min_quantum =  hz/10;		

#if RT_SCHED
	/*
	 * The quantum passed to thread_policy is in units of milliseconds.
	 * The units of min_quantum are ticks.
	 */
	rt_sched_rr_interval = min_quantum * 1000 / hz;
#endif /* RT_SCHED */
	wait_queue_init();
	pset_sys_bootstrap();		/* initialize processer mgmt. */
	queue_init(&action_queue);
	simple_lock_init(&action_lock);
	sched_tick = 0;
#if	SIMPLE_CLOCK
	sched_usec = 0;
#endif
	ast_init();
#if RT_SCHED_MON
	/*
	 * If monitoring is enabled, initialize the necessary data structures.
	 */
	rt_sched_mon_init();
#endif /* RT_SCHED_MON */
}

#if RT_SCHED_MON
/*
 * rt_sched_mon_init
 *
 * Initialize the scheduler monitor data structures.
 */

rt_sched_mon_init()
{
  register int i;
  register rt_sched_hist_entry_t entry;
	  
  /* 
   * Initialize the scheduler history lock.  This lock is used for both the
   * history buffer and the counters.
   */
  simple_lock_init(&rt_sched_hist_lock);

  /*
   * Initialize the history counters.  There is a catchall counter at array
   * index 0, which is why we initialize one extra entry.
   */
  for (i = 0; i < RT_SCHED_HIST_EVTS + 1; i++) {
    rt_sched_hist_counts[i] = 0;
  }

  /* 
   * Initalize the history buffer.  Put ctrl_len in the buffer so that the
   * length can be queried at runtime.  Zero out the buffer just to be
   * thorough.
   */
  rt_sched_hist.count = 1;	/* Number of NEXT event. */
  rt_sched_hist.next = 0;	/* Index of next/current event to write. */
  rt_sched_hist.last = 0;	/* Index of last event written. */
  rt_sched_hist.ctrl_len = sizeof( rt_sched_hist.ctrl );
  rt_sched_hist.fill2 = 0;
  for( i = 0; i < RT_SCHED_HIST_LEN; i++) {
    entry = &rt_sched_hist.buf[i];
    entry->event = 0;
    entry->duplicates = 0;
    entry->d1 = 0;
    entry->d2 = 0;
    entry->d3 = 0;
  }

  /*
   * Initialize the control bytes.  Turn everything off, then turn on events
   * which record execution of error code paths.
   */
  for( i = 0; i < sizeof( rt_sched_hist.ctrl ); i++ ) {
    rt_sched_hist.ctrl[i] = 0;
  }
  rt_sched_hist.ctrl[RTS_rqempty1] = RT_SCHED_CTRL_DEFAULT;
  rt_sched_hist.ctrl[RTS_rqempty2] = RT_SCHED_CTRL_DEFAULT;
  rt_sched_hist.ctrl[RTS_rqempty3] = RT_SCHED_CTRL_DEFAULT;
  rt_sched_hist.ctrl[RTS_rqempty4] = RT_SCHED_CTRL_DEFAULT;
  rt_sched_hist.ctrl[RTS_rqempty5] = RT_SCHED_CTRL_DEFAULT;
  rt_sched_hist.ctrl[RTS_rqmask] = 
    RT_SCHED_CTRL_DEFAULT & ~RT_SCHED_CTRL_MERGE;
  rt_sched_hist.ctrl[RTS_rqinvalid] = 
    RT_SCHED_CTRL_DEFAULT & ~RT_SCHED_CTRL_MERGE;
}
#endif /* RT_SCHED_MON */

wait_queue_init()
{
	register int i;

	for (i = 0; i < NUMQUEUES; i++) {
		queue_init(&wait_queue[i]);
		simple_lock_init(&wait_lock[i]);
	}
}

#if	DEBUG
int	sched_debug = 0;
#endif

/*
 *	Thread timeout routine, called when timer expires.
 *	Called at splhigh.
 */
/*
 * RT:
 *
 *	Modify this routine to raise ipl to splsched.  Despite the comment
 *	above, this routine is actually called at splsoftclock.  Taking the
 *	thread lock at a different ipl could cause deadlock.  This wasn't
 *	bracketed with compilation switches because it is always required.
 */
thread_timeout(_thread, t)
	caddr_t	_thread;
	int	t;
{
	register thread_t	thread = (thread_t) _thread;
	register int s;

	s = splsched();
#ifdef	lint
	t++;
#endif
	thread_lock(thread);
	thread->timer_set = FALSE;
	thread_unlock(thread);
	clear_wait(thread, THREAD_TIMED_OUT, FALSE);
	splx(s);
}

/*
 *	thread_set_timeout:
 *
 *	Set a timer for the current thread, if the thread
 *	is ready to wait.  Must be called between assert_wait()
 *	and thread_block().
 */
 
void thread_set_timeout(t)
	int	t;	/* timeout interval in ticks */
{
	register thread_t	thread = current_thread();
	register int s;

	s = splsched();
	thread_lock(thread);
	if ((thread->state & TH_WAIT) != 0) {
		thread->timer_set = TRUE;
		timeout(thread_timeout, (caddr_t)thread, t);
	}
	thread_unlock(thread);
	splx(s);
}

/*
 *	assert_wait:
 *
 *	Assert that the current thread is about to go to
 *	sleep until the specified event occurs.
 */
void assert_wait(event, interruptible)
	vm_offset_t	event;
	boolean_t	interruptible;
{
	register queue_t	q;
	register int		index;
	register thread_t	thread;
#if	MACH_SLOCKS
	register simple_lock_t	lock;
#endif
	int			s;

        if (AT_INTR_LVL())
                panic("assert_wait: waiting in an interrupt service routine");

	thread = current_thread();
#if	DEBUG
	if (sched_debug)
		printf("assert_wait: thread = 0x%lx, event = 0x%lx\n", thread, event);
#endif
	if (thread->wait_event != 0) {
#if	DEBUG
		printf("assert_wait: already asserted event 0x%lx\n",
			thread->wait_event);
#endif
		panic("assert_wait");
	}
 	s = splsched();
	RT_SCHED_HIST(RTS_await, thread, thread->policy, event, 0, 0);
	if (event != 0) {
		index = wait_hash(event);
		q = &wait_queue[index];
#if	MACH_SLOCKS
		lock = &wait_lock[index];
#endif
		simple_lock(lock);
		thread_lock(thread);
		enqueue_tail(q, (queue_entry_t) thread);
		thread->wait_event = event;
		thread->state |= TH_WAIT;
		thread->sleep_stamp = sched_tick;
		thread->interruptible = interruptible;
		thread_unlock(thread);
		simple_unlock(lock);
	} else {
		thread_lock(thread);
		thread->state |= TH_WAIT;
		thread->sleep_stamp = sched_tick;
		thread->interruptible = interruptible;
		thread_unlock(thread);
	}
	splx(s);
}

/*
 *	clear_wait:
 *
 *	Clear the wait condition for the specified thread.  Start the thread
 *	executing if that is appropriate.
 *
 *	parameters:
 *	  thread		thread to awaken
 *	  result		Wakeup result the thread should see
 *	  interrupt_only	Don't wake up the thread if it isn't
 *				interruptible.
 */
void clear_wait(thread, result, interrupt_only)
	register thread_t	thread;
	int			result;
	boolean_t		interrupt_only;
{
	register int		index;
	register queue_t	q;
#if	MACH_SLOCKS
	register simple_lock_t	lock;
#endif
	register vm_offset_t	event;
	int			s;

	s = splsched();
	thread_lock(thread);
	if (interrupt_only && !thread->interruptible) {
		/*
		 *	can't interrupt thread
		 */
		thread_unlock(thread);
		splx(s);
		return;
	}

	event = thread->wait_event;
	if (event != 0) {
		thread_unlock(thread);
		index = wait_hash(event);
		q = &wait_queue[index];
#if	MACH_SLOCKS
		lock = &wait_lock[index];
#endif
		simple_lock(lock);
		/*
		 *	If the thread is still waiting on that event,
		 *	then remove it from the list.  If it is waiting
		 *	on a different event, or no event at all, then
		 *	someone else did our job for us.
		 */
		thread_lock(thread);
		if (thread->wait_event == event) {
			remqueue(q, (queue_entry_t)thread);
			thread->wait_event = 0;
			event = 0;		/* cause to run below */
		}
		simple_unlock(lock);
	}
	if (event == 0) {
		register int	state = thread->state;

		if (thread->timer_set) {
			thread->timer_set = FALSE;
#if RT
			if(thread->psx4_sleep == TRUE)
			   thread->time_remaining = psx4_untimeout(thread_timeout, (caddr_t)thread);

                        else
#endif /* RT */

			untimeout(thread_timeout, (caddr_t)thread);
		}
		/* End of /proc code */
		switch (state) {
		    case TH_WAIT | TH_SUSP:
			/*
			 *	Suspend thread if interruptible
			 */
			if (thread->interruptible) {
			    thread->state = TH_SUSP;
			    thread->wait_result = result;
			    break;
			}
			/* fall through */
		    case TH_WAIT:
			/*
			 *	Sleeping and not suspendable - put
			 *	on run queue.
			 */
			thread->state = (state & ~TH_WAIT) | TH_RUN;
			thread->wait_result = result;
			thread_setrun(thread, TRUE);
			break;

		    case TH_WAIT | TH_SWAPPED:
			/*
			 *	Thread is swapped out, but runnable
			 */
			thread->state = TH_RUN | TH_SWAPPED;
			thread->wait_result = result;
			thread_swapin(thread);
			break;

		    default:
			/*
			 *	Either already running, or suspended.
			 */
			if (state & TH_WAIT) {
				thread->state = state & ~TH_WAIT;
				thread->wait_result = result;
				thread->interruptible = TRUE;
			}
			break;
		}
	}
	thread_unlock(thread);
	splx(s);
}

/*
 *	thread_wakeup_prim:
 *
 *	Common routine for thread_wakeup, thread_wakeup_with_result,
 *	and thread_wakeup_one.
 *
 */
void thread_wakeup_prim(event, one_thread, result)
	register vm_offset_t	event;
	boolean_t		one_thread;
	int			result;
{
	register queue_t	q;
	register int		index;
	register thread_t	thread, next_th;
#if	MACH_SLOCKS
	register simple_lock_t	lock;
#endif
	int			s;
	register int		state;

	index = wait_hash(event);
	q = &wait_queue[index];
	s = splsched();
#if	MACH_SLOCKS
	lock = &wait_lock[index];
#endif
	simple_lock(lock);
	thread = (thread_t) queue_first(q);
	while (!queue_end(q, (queue_entry_t)thread)) {
		next_th = (thread_t) queue_next((queue_t) thread);

		if (thread->wait_event == event) {
			thread_lock(thread);

#if	DEBUG
			if (sched_debug)
				printf("thread_wakeup: thread 0x%lx woken, event = 0x%lx\n", thread, event);
#endif
			RT_SCHED_HIST(RTS_wake, thread, thread->policy,
				      event, one_thread, result);
			remqueue(q, (queue_entry_t) thread);
			thread->wait_event = 0;
			if (thread->timer_set) {
				thread->timer_set = FALSE;
				untimeout(thread_timeout, (caddr_t)thread);
			}
			state = thread->state;
			switch (state) {
			    case TH_WAIT | TH_SUSP:
				if (thread->interruptible) {
				    thread->state = TH_SUSP;
				    thread->wait_result = result;
				    break;
				}
				/* fall through */
			    case TH_WAIT:
				thread->state = (state & ~TH_WAIT) | TH_RUN;
				thread->wait_result = result;
				thread_setrun(thread, TRUE);
				break;
				
			    case TH_WAIT | TH_SWAPPED:
				simple_unlock(lock);
				thread->state = TH_RUN | TH_SWAPPED;
				thread->wait_result = result;
				thread_swapin(thread);
				thread_unlock(thread);

				if (one_thread) {
					/*
					 *	Done!
					 */
					splx(s);
					return;
				}

				/*
				 * Restart the search, since we had
				 * to unlock for thread_swapin (it calls
				 * thread_wakeup, which may find the
				 * swapin thread on the same queue).
				 */
				simple_lock(lock);
				thread = (thread_t) queue_first(q);
				continue;

			    default:
				if ((state & TH_WAIT) == 0)
				    panic("thread_wakeup");
				thread->state = state & ~TH_WAIT;
				thread->wait_result = result;
				thread->interruptible = TRUE;
				break;
			}
			thread_unlock(thread);
			if (one_thread)
				break;
		}
		thread = next_th;
	}
	simple_unlock(lock);
	splx(s);
}


/*
 *	thread_wakeup_hi:
 *
 *	routine for thread_wakeup_high:
 *	This routine wakes the highest priority thread in the queue that is
 *	waiting for the event.
 *
 */

void thread_wakeup_hi(event,  result)
	register vm_offset_t	event;
	int			result;
{
	register queue_t	q;
	register int	index;
	register thread_t thread, hipri_th;
#if MACH_SLOCKS 	
	register simple_lock_t lock;
#endif 	
	int	    s;
	register int	state;

	/*
	 * Lock a list in the event wait queue array.  This allows 
	 * us to be fast and loose with the queue manipulations.
	 * Raise spl to sched to block isr scheduler activity.
	 */

	index = wait_hash(event);
	q = &wait_queue[index];
	s = splsched();  
#if MACH_SLOCKS 	
	lock = &wait_lock[index];
#endif 	

	simple_lock(lock);

	/*
	 * Locate the highest priority thread on this queue
	 * which matches event.
	 */

	thread = (thread_t) queue_first(q);
	hipri_th = (thread_t)NULL;  
	    
	while (!queue_end(q, (queue_entry_t)thread)) {

	      if(thread->wait_event == event){   /* the event?  */

		if(hipri_th == (thread_t)NULL)   /* first found */
		  hipri_th = thread;             /* save as hi  */
	        else                             /* not first   */
		  if(thread->sched_pri < hipri_th->sched_pri)
		    hipri_th = thread;           /* save new hi */
	      }
	      thread = (thread_t) queue_next((queue_t) thread);
	    }

	/*
         * if no thread was found for the event return.
         */

	if(hipri_th == (thread_t)NULL){
	   simple_unlock(lock);
	   splx(s); 
	   return;
	 }

	/*
	 * Now that the highest priority thread has been located
	 * lock it, remove it from the wait queue and release 
	 * the wait queue lock.
	 */

	thread = hipri_th;
	thread_lock(thread);

	remqueue(q, (queue_entry_t) thread);
	simple_unlock(lock);

	thread->wait_event = 0;
	if (thread->timer_set) {
	   thread->timer_set = FALSE;
	   untimeout(thread_timeout, (caddr_t)thread);
	}

	state = thread->state;
	switch (state) {

		case TH_WAIT | TH_SUSP:
				if (thread->interruptible) {
				thread->state = TH_SUSP;
				thread->wait_result = result;
				break;
				}
				/* fall through */
		case TH_WAIT:
				thread->state = (state & ~TH_WAIT) | TH_RUN;
				thread->wait_result = result;
				thread_setrun(thread, TRUE);
				break;


		case TH_WAIT | TH_SWAPPED:
				thread->state = TH_RUN | TH_SWAPPED;
				thread->wait_result = result;
				thread_swapin(thread);
				break;
		default:
				if ((state & TH_WAIT) == 0)
				      panic("thread_wakeup_high");
				thread->state = state & ~TH_WAIT;
				thread->wait_result = result;
				thread->interruptible = TRUE;
				break;
		}

	thread_unlock(thread);
	splx(s); 
}


/*
 *	thread_sleep:
 *
 *	Cause the current thread to wait until the specified event
 *	occurs.  The specified lock is unlocked before releasing
 *	the cpu.  (This is a convenient way to sleep without manually
 *	calling assert_wait).
 */
void thread_sleep(event, lock, interruptible)
	vm_offset_t	event;
	simple_lock_t	lock;
	boolean_t	interruptible;
{
#if	DEBUG
	if (sched_debug)
		printf("thread_sleep: event = 0x%lx\n", event);
#endif
	assert_wait(event, interruptible);	/* assert event */
	simple_unlock(lock);			/* release the lock */
#ifdef	KTRACE
	kern_trace(900,event,interruptible,0);
#endif  /* KTRACE */
	thread_block();				/* block ourselves */
}

/*
 *	thread_bind:
 *
 *	Force a thread to execute on the specified processor.
 *	If the thread is currently executing, it may wait until its
 *	time slice is up before switching onto the specified processor.
 *
 *	A processor of PROCESSOR_NULL causes the thread to be unbound.
 *	xxx - DO NOT export this to users.
 */
thread_bind(thread, processor)
	register thread_t	thread;
	processor_t		processor;
{
	int		s;

	s = splsched();
	thread_lock(thread);
	thread->bound_processor = processor;
	thread_unlock(thread);
#ifdef	KTRACE
	kern_trace(901,thread,processor,0);
#endif  /* KTRACE */
	(void) splx(s);
}

/*
 * FAST_CSW mode is the mode DEC mips context switch functions
 * always work.  Wire it as so below, by ensuring Nothing is
 * done to redefine the switch_thread_context(), switch_task_context(),
 * and switch_context() routines.
 */
#ifndef	mips
#ifdef __alpha
#define switch_thread_context(thread,new_thread)                        \
	MACRO_BEGIN                                                     \
                swtch_thrd_ctxt_cnt++;                                  \
                switch_context(thread,new_thread);                      \
	MACRO_END

#define switch_task_context(thread,new_thread)                          \
	MACRO_BEGIN                                                     \
                swtch_tsk_ctxt_cnt++;                                   \
                switch_context(thread,new_thread);                       \
	MACRO_END
#else

#if     FAST_CSW
/* Nothing */
#else
/*
 *	Macros to substitute for missing hardware support.
 */

#ifdef	mips	/* XXX */
#define set_active_thread(new_thread) /* done in load_context */
#else
#define set_active_thread(new_thread) active_threads[cpu_number()] = new_thread 
#endif

void	thread_continue();

#define switch_thread_context(thread,new_thread)			\
	MACRO_BEGIN							\
		swtch_thrd_ctxt_cnt++;					\
		switch_context(thread,new_thread);			\
	MACRO_END

#define switch_task_context(thread,new_thread)	 			\
	MACRO_BEGIN							\
		swtch_tsk_ctxt_cnt++;					\
		switch_context(thread,new_thread)			\
	MACRO_END

#define switch_context(thread,new_thread)	 			\
	if (save_context() == 0) {					\
		thread_continue(thread);				\
		set_active_thread(new_thread);				\
		load_context(new_thread);				\
		/*NOTREACHED*/						\
    	}
#endif  /* FAST_CSW */
#endif	/* alpha*/
#endif	/* !mips */

int	same_task_count = 0;
int	same_thread_count = 0;
/*
 *	thread_block:
 *
 *	Block the current thread.  If the thread is runnable
 *	then someone must have woken it up between its request
 *	to sleep and now.  In this case, it goes back on a
 *	run queue.
 */
void thread_block()
{
	register thread_t thread = current_thread();
	register thread_t	new_thread;
	register int		mycpu = cpu_number();
	register processor_t	myprocessor;
	int		s;

	myprocessor = cpu_to_processor(mycpu);

#if	DEBUG
	if (sched_debug)
		printf("thread_block: thread = 0x%x\n", thread);
#endif

	s = splsched();
	/* give threads waiting on an event a priority boost.  This boost
	 * is to queues 6 to 8 for non-interruptible waits (block i/o) and
	 * queues 9 to 11 for interruptible sleeps (tty,signals...etc...).
	 * This priority  boost lasts until the thread gets scheduled to
	 * a processor.
	 */
	if ((thread->state & TH_WAIT) && (thread->sched_pri >= BASEPRI_USER)
	    && (thread->policy == POLICY_TIMESHARE)) {
		if (thread->interruptible) {
			thread->sched_pri = BASEPRI_USER - 3;
		} else {
			thread->sched_pri = BASEPRI_USER - 6;
		}
	}
	RT_SCHED_HIST(RTS_block, thread, thread->policy, 0, 0, 0);
#if	RT_PREEMPT
	/*
	 * If the slock_count has not returned to 0, coerce it there
	 * and then check whether the anomaly should crash the system.
	 */
	if (slock_count[cpu_number()] != 0)
		PREEMPTION_FIXUP();
#endif
#ifdef	KTRACE
	kern_trace(902,thread,0,0);
#endif  /* KTRACE */
	/*
	 *	Check for obvious simple case; local runq is
	 *	empty and global runq has entry at hint.
	 */
	if (myprocessor->runq.count > 0) {
#if RT_SCHED
		/* Preserve remaining RR thread quantum. */
		if (myprocessor->was_first_quantum) {
		  thread->rmng_quantum = myprocessor->quantum;
		  RT_SCHED_HIST(RTS_rrqsave1, thread, thread->policy,
				myprocessor->quantum, thread->state,
				(((thread->state == TH_RUN)?1:0)
				 |((myprocessor->first_quantum)?2:0)
				 |((myprocessor->quantum > 0)?4:0)));
		}
#endif /* RT_SCHED */
		new_thread = choose_thread(myprocessor);
		if (new_thread->policy == POLICY_TIMESHARE) {
			myprocessor->quantum = min_quantum;
			myprocessor->first_quantum = TRUE;
		}
		else {
#if RT_SCHED
			/*
			 * Restore partially completed quantum.  If thread
			 * completed its last quantum, get a fresh quantum
			 * from sched_data.  If running one of the POSIX
			 * policies, turn on first_quantum flag.  (For some
			 * reason POLICY_FIXEDPRI doesn't set first_quantum).
			 * Clear rmng_quantum and was_first_quantum so that
			 * the thread doesn't end up back on the head of the
			 * run queue by accident.
			 */
                	if (new_thread->rmng_quantum > 0) {
			    myprocessor->quantum = new_thread->rmng_quantum;
			    new_thread->rmng_quantum = 0;
			    RT_SCHED_HIST(RTS_rrqrest1, new_thread,
					  new_thread->policy,
					  myprocessor->quantum, 0, 0 );
			}
			else {
			    myprocessor->quantum = new_thread->sched_data;
			}
			myprocessor->first_quantum =
			  ((new_thread->policy & (POLICY_FIFO|POLICY_RR)) != 0);
		}
		myprocessor->was_first_quantum = FALSE;
#else /* RT_SCHED */
			/*
			 *	POLICY_FIXEDPRI
			 */
			myprocessor->quantum = new_thread->sched_data;
			myprocessor->first_quantum = FALSE;
		}
#endif /* RT_SCHED */
	}
	else {
		register run_queue_t	rq;

#if	MACH_HOST
		rq = &(myprocessor->processor_set->runq);
#else
		rq = &default_pset.runq;
#endif
		if (rq->count == 0) {
			/*
			 *	Nothing else runnable.  Return if this
			 *	thread is still runnable on this processor.
			 *	Check for priority update if required.
			 */
			if ((thread->state == TH_RUN) &&
			    ((thread->bound_processor == PROCESSOR_NULL) ||
			     (thread->bound_processor == myprocessor))) {
				same_thread_count++;
				if (thread->sched_stamp != sched_tick)
				    update_priority(thread);
				splx(s);
				return;
			}
			else {
				new_thread = choose_thread(myprocessor);
			}
		}
		else {
			register queue_t	q;
		
			/*
			 *	If there is a thread at hint, grab it,
			 *	else call choose_thread to do it the hard way.
			 */
			simple_lock(&rq->lock);
			q = rq->runq + rq->low;

			if (queue_empty(q)) {
				RT_SCHED_HIST(RTS_rqempty1, rq->count,
					      RT_SCHED_CTRL_POLICY_ALL,
					      rq->mask.bits[0], 
					      rq->mask.bits[1], rq->low);
				simple_unlock(&rq->lock);
				new_thread = choose_thread(myprocessor);
			}
			else {
				new_thread = (thread_t) dequeue_head(q);
				new_thread->runq = RUN_QUEUE_NULL;
				rq->count--;
				RT_SCHED_UPDATE_RQ_LOW(rq,q);
				simple_unlock(&rq->lock);
			}
		}
#if RT_SCHED
		/* 
		 * Preserve remaining RR thread quantum.
		 */
		if (myprocessor->was_first_quantum) {
		  thread->rmng_quantum = myprocessor->quantum;
		  RT_SCHED_HIST(RTS_rrqsave2, thread, thread->policy,
				myprocessor->quantum, thread->state,
				(((thread->state == TH_RUN)?1:0)
				 |((myprocessor->first_quantum)?2:0)
				 |((myprocessor->quantum > 0)?4:0)));
		}
#endif /* RT_SCHED */
		if (new_thread->policy == POLICY_TIMESHARE) {
#if	MACH_HOST
			myprocessor->quantum =
				myprocessor->processor_set->set_quantum;
#else
			myprocessor->quantum = default_pset.set_quantum;
#endif
			myprocessor->first_quantum = TRUE;
		}
		else {
#if RT_SCHED
			/*
			 * Restore partially completed quantum.  If thread
			 * completed its last quantum, get a fresh quantum
			 * from sched_data.  If running one of the POSIX
			 * policies, turn on first_quantum flag.  (For some
			 * reason POLICY_FIXEDPRI doesn't set first_quantum).
			 * Clear rmng_quantum and was_first_quantum so that
			 * the thread doesn't end up back on the head of the
			 * run queue by accident.
			 */
			if (new_thread->rmng_quantum > 0) {
			  myprocessor->quantum = new_thread->rmng_quantum;
			  new_thread->rmng_quantum = 0;
			  RT_SCHED_HIST(RTS_rrqrest2, new_thread,
					new_thread->policy,
					myprocessor->quantum, 0, 0 );
			}
			else {
			    myprocessor->quantum = new_thread->sched_data;
			}
			myprocessor->first_quantum =
			  ((new_thread->policy & (POLICY_FIFO|POLICY_RR)) != 0);
		}
		myprocessor->was_first_quantum = FALSE;
#else /* RT_SCHED */
			/*
			 *	POLICY_FIXEDPRI
			 */
			myprocessor->quantum = new_thread->sched_data;
			myprocessor->first_quantum = FALSE;
		}
#endif /* RT_SCHED */
	}

	/*
	 *	Thread is now interruptible.
	 */
	new_thread->interruptible = TRUE;

#if	HW_FOOTPRINT
	new_thread->last_processor = myprocessor;
#endif

	/*
	 *	Set up ast context of new thread and switch to its timer.
	 */
	ast_context(new_thread, mycpu);
#if	RT_PREEMPT
	ast_mode[cpu_number()] = USER_AST;
#endif
	timer_switch(&new_thread->system_timer);

	/*
	 *	WARNINGS:
	 *	    1.  switch_{task,thread}_context must do
	 *		active_threads[mycpu] = new_thread;
	 *		load_context(new_thread) should do this also.
	 *
	 *	    2.  On hardware that can take page faults during
	 *		a context_save, PMAP_DEACTIVATE and PMAP_ACTIVATE
	 *		must not change the pmap.  Assembly language should
	 *		only change it after all operations that could
	 *		cause a page fault are completed.
	 *
	 *	All of this nonsense is necessitated by bizarre hardware
	 *	on which saving context can cause a trap or fault.  Handlers
	 *	for these traps and faults must not block (e.g. don't call
	 *	vm_fault).
	 */

#if     MACH_KM		
	kern_mon_thread_sensor(thread,new_thread,mycpu);
#endif

#ifdef	notyet
	LASSERT(slck_dbg[cpu_number()].count == 0);
#endif

	/*
	 *	A context switch is about to occur.  If the current thread
	 *	is in either W or S state, the context switch is classified 
	 *	as voluntary, otherwise the context switch is classified as 
	 *	involuntary.
	 */
 
	if (thread->state & (TH_WAIT | TH_SUSP)) 
		u.u_ru.ru_nvcsw++;
	else
		u.u_ru.ru_nivcsw++;

	if (thread->task == new_thread->task) {
		RT_SCHED_HIST(RTS_exec, new_thread, 
			      new_thread->policy | thread->policy, thread,
			      ((new_thread->policy << 16) 
			       | new_thread->sched_pri), TRUE);
		same_task_count++;
		PMAP_CONTEXT(vm_map_pmap(new_thread->task->map), new_thread);

		/* check for priority boost... if so, recalculate set
		   non-boosted priority */
		if (new_thread->sched_pri < new_thread->priority ) {
			if (new_thread->sched_stamp != sched_tick)
				update_priority(new_thread);
			else
				compute_my_priority(new_thread);
		}
		switch_thread_context(thread, new_thread);
	}
	else {
	    RT_SCHED_HIST(RTS_exec, new_thread, 
			  new_thread->policy | thread->policy, thread,
			  ((new_thread->policy << 16)
			   | new_thread->sched_pri), FALSE);
	    if (thread->task->kernel_vm_space == FALSE) {
		PMAP_DEACTIVATE(vm_map_pmap(thread->task->map), thread,
			mycpu);
	    }
	    if (new_thread->task->kernel_vm_space == FALSE) {
		PMAP_ACTIVATE(vm_map_pmap(new_thread->task->map),
			new_thread, mycpu);
	    }

	    /* check for priority boost... if so, recalculate set
	       non-boosted priority */
	    if (new_thread->sched_pri < new_thread->priority ) {
		if (new_thread->sched_stamp != sched_tick)
			update_priority(new_thread);
		else
			compute_my_priority(new_thread);
	    }
	    switch_task_context(thread, new_thread);

	}
	splx(s);

}

/*
 *	Thread continue dispatches a running thread that is not
 *	on a runq.  It is called by machine-dependent context switch
 *	code.  Caller must disable interrupts.
 */

void thread_continue(thread)
thread_t	thread;
{
register int		state;

#if RT_SCHED
	ASSERT(issplsched());
#endif /* RT_SCHED */

	/*
	 *	See if we should go back on a run queue.
	 */
#ifdef	KTRACE
        kern_trace(903,thread,thread->state,0);
#endif  /* KTRACE */
	thread_lock(thread);
	state = thread->state;
	switch (state) {

	    case TH_RUN | TH_SUSP:
		if (thread->interruptible) {
		    /*
		     *	Suspend the thread
		     */
		    thread->state = TH_SUSP;
		    if (thread->wake_active) {
			thread->wake_active = FALSE;
			thread_unlock(thread);
			thread_wakeup((vm_offset_t)&thread->wake_active);
			return;
		    }
		    break;
		}
		/* fall through */

	    case TH_RUN:
		/*
		 *	No reason to stop.  Put back on a run queue.
		 */
		thread_setrun(thread, FALSE);
		break;

	    case TH_RUN | TH_WAIT | TH_SUSP:
		if (thread->interruptible) {
		    /*
		     *	Suspended and interruptible.
		     */
		    thread->state = TH_WAIT | TH_SUSP;
		    thread->sleep_stamp = sched_tick;
		    if (thread->wake_active) {
			/*
			 *	Someone wants to know when this thread
			 *	really stops.
			 */
			thread->wake_active = FALSE;
			thread_unlock(thread);
			thread_wakeup((vm_offset_t)&thread->wake_active);
			return;
		    }
		    break;
		}
		/* fall through */

	    case TH_RUN | TH_WAIT:
		/*
		 *	Waiting, and not suspended or not interruptible.
		 */
		thread->state = state & ~TH_RUN;
		break;

	    default:
		/*
		 *	Drop idle thread -- it is already in
		 *	idle_thread_array.
		 */
		if (state != (TH_RUN | TH_IDLE)) {
		    /*
		     *	Not running - oops
		     */
		    panic("thread_continue");
		}
		break;
	}
	thread_unlock(thread);
}

/*
 *	thread_run:
 *
 *	Switch directly from the current thread to a specified
 *	thread.  Both the current and new threads must be
 *	runnable.
 */
void thread_run(new_thread)
	register thread_t	new_thread;
{
	register thread_t	thread = current_thread();
	register int		s, mycpu;

	mycpu = cpu_number();

#ifdef	KTRACE
	kern_trace(904,thread,new_thread,0);
#endif  /* KTRACE */
#if	DEBUG
	if (sched_debug)
		printf("thread_run: thread = 0x%x\n", thread);
#endif

	s = splsched();

	/*
	 *	Thread is now interruptible.
	 */
	new_thread->interruptible = TRUE;

#if	HW_FOOTPRINT
	new_thread->last_processor = current_processor();
#endif

	/*
	 *	New thread inherits old one's quantum and first_quantum flag.
	 *	Fixed priority threads must be fixed by caller.
	 */

	timer_switch(&new_thread->system_timer);

	ast_context(new_thread, mycpu);
#if	RT_PREEMPT
	ast_mode[cpu_number()] = USER_AST;
#endif

#if     MACH_KM		
	kern_mon_thread_sensor(thread,new_thread,mycpu);
#endif

	u.u_ru.ru_nvcsw++;

	if (thread->task == new_thread->task) {
		RT_SCHED_HIST(RTS_run, new_thread, 
			      thread->policy|new_thread->policy, thread,
			      ((new_thread->policy << 16)
			       | new_thread->sched_pri), TRUE);
		same_task_count++;
		PMAP_CONTEXT(vm_map_pmap(new_thread->task->map), new_thread);
		/* check for priority boost... if so, recalculate set
		   non-boosted priority */
		if (new_thread->sched_pri < new_thread->priority ) {
			if (new_thread->sched_stamp != sched_tick)
				update_priority(new_thread);
			else
				compute_my_priority(new_thread);
		}
		switch_thread_context(thread, new_thread);
	}
	else {
	    RT_SCHED_HIST(RTS_run, new_thread, 
			  thread->policy|new_thread->policy, thread,
			  ((new_thread->policy << 16)
			   | new_thread->sched_pri), FALSE);
	    if (thread->task->kernel_vm_space == FALSE) {
		PMAP_DEACTIVATE(vm_map_pmap(thread->task->map), thread,
			mycpu);
	    }
	    if (new_thread->task->kernel_vm_space == FALSE) {
		PMAP_ACTIVATE(vm_map_pmap(new_thread->task->map),
			new_thread, mycpu);
	    }

	    /* check for priority boost... if so, recalculate set
	       non-boosted priority */
	    if (new_thread->sched_pri < new_thread->priority ) {
		if (new_thread->sched_stamp != sched_tick)
			update_priority(new_thread);
		else
			compute_my_priority(new_thread);
	    }
	    switch_task_context(thread, new_thread);
	}

	splx(s);
}


/*
 *	Define shifts for simulating (5/8)**n
 */

shift_data_t	wait_shift[32] = {
	{1,1},{1,3},{1,-3},{2,-7},{3,5},{3,-5},{4,-8},{5,7},
	{5,-7},{6,-10},{7,10},{7,-9},{8,-11},{9,12},{9,-11},{10,-13},
	{11,14},{11,-13},{12,-15},{13,17},{13,-15},{14,-17},{15,19},{16,18},
	{16,-19},{17,22},{18,20},{18,-20},{19,26},{20,22},{20,-22},{21,-27}};

/*
 *	set_pri:
 *
 *	Set the priority of the specified thread to the specified
 *	priority.  This may cause the thread to change queues.
 *
 *	The thread *must* be locked by the caller.
 */

#if RT_SCHED
/*
 * Clearing the thread rmng_quantum field forces the thread to the tail of
 * the run queue, even if it hasn't finished its first quantum (or even if
 * it is FIFO).  This should be revisited -- should setting the priority
 * of a thread to the same priority cause yield-like behavior, or should it
 * leave the thread alone?
 */
#define RT_SCHED_CLEAR_RMNG_QUANTUM(th) (th)->rmng_quantum = 0;
#else /* RT_SCHED */
#define RT_SCHED_CLEAR_RMNG_QUANTUM(th)
#endif /* RT_SCHED */

#define set_pri(th, pri, may_reschedule)		\
	MACRO_BEGIN					\
	register struct run_queue	*rq;		\
							\
	rq = rem_runq((th));				\
	if (((th)->task->kernel_vm_space == FALSE)  ||	\
	    ((th)->priority >= BASEPRI_USER)	||	\
	    (kernel_thread_fixed_pri_disable)) 	{	\
		(th)->sched_pri = pri;			\
	}						\
	if (rq != RUN_QUEUE_NULL) {			\
		if (may_reschedule)			\
			RT_SCHED_CLEAR_RMNG_QUANTUM(th)	\
		thread_setrun(th, TRUE);		\
	}						\
	MACRO_END

/*
 * RT_SCHED:  The hardcoded priorities were replaced with BASEPRI_LOWEST
 * below.  This was done without conditionalizing the code, since it would
 * have doubled the number of macro definitions.  It never hurts to use a
 * a mnemonic instead of a constant, anyway...
 */

int rss_adjust_pri = 2;
extern int ubc_pages, vm_page_inactive_count, vm_page_active_count;
extern int task_inswapped_queue_count, vm_page_free_target, vm_page_free_count;
extern pmap_t kernel_pmap;

#define	rss_adjust(TH) 							\
	(((vm_page_free_count <= vm_page_free_target) &&		\
	 ((TH)->task->map->vm_pmap != kernel_pmap) &&			\
	 ((pmap_resident_count((TH)->task->map->vm_pmap)) > 		\
          ((TH)->task->u_address->uu_rlimit[RLIMIT_RSS].rlim_cur)))	\
		? rss_adjust_pri : 0)

#ifdef	PRI_SHIFT_2
#if	PRI_SHIFT_2 > 0
#define do_priority_computation(th, pri)				\
	MACRO_BEGIN							\
	(pri) = (th)->priority	/* start with base priority */		\
	    + (((th)->sched_usage + (th)->sched_delta) >> (PRI_SHIFT + SCHED_SHIFT))            \
            + (((th)->sched_usage + (th)->sched_delta) >> (PRI_SHIFT_2 + SCHED_SHIFT)) + rss_adjust(th); \
	if ((pri) > BASEPRI_LOWEST) (pri) = BASEPRI_LOWEST;		\
	MACRO_END
#else	/* PRI_SHIFT_2 <= 0 */
#define do_priority_computation(th, pri)				\
	MACRO_BEGIN							\
	(pri) = (th)->priority	/* start with base priority */		\
	    + (((th)->sched_usage + (th)->sched_delta) >> (PRI_SHIFT + SCHED_SHIFT))            \
            - (((th)->sched_usage + (th)->sched_delta) >> (SCHED_SHIFT - PRI_SHIFT_2)) + rss_adjust(th); \
	if ((pri) > BASEPRI_LOWEST) (pri) = BASEPRI_LOWEST;		\
	MACRO_END
#endif	/* PRI_SHIFT_2 <= 0 */
#else	/* !defined(PRI_SHIFT_2) */
#define do_priority_computation(th, pri)				\
	MACRO_BEGIN							\
	(pri) = (th)->priority	/* start with base priority */		\
	    + (((th)->sched_usage+(th)->sched_delta)>>(PRI_SHIFT + SCHED_SHIFT)) + rss_adjust(th);\
	if ((pri) > BASEPRI_LOWEST) (pri) = BASEPRI_LOWEST;		\
	MACRO_END
#endif	/* !defined(PRI_SHIFT_2) */

/*
 *	compute_priority:
 *
 *	Compute the effective priority of the specified thread.
 *	The effective priority computation is as follows:
 *
 *	Take the base priority for this thread and add
 *	to it an increment derived from its cpu_usage.
 *
 *	The thread *must* be locked by the caller. 
 */

compute_priority(thread, may_reschedule)
	register thread_t	thread;
	boolean_t		may_reschedule;
{
	register int	pri;

#if RT_SCHED
	/*
	 * It is safe to call this routine with no lock taken, if the thread
	 * is not known to the rest of the system and it has not been put in
	 * a run queue yet.  This is taken advantage of in procdup().
	 */
#if (MACH_ASSERT|MACH_LASSERT)
	if (thread->runq != RUN_QUEUE_NULL) {
	  ASSERT(issplsched());
	  LASSERT(thread->lock.lock_data != 0);
	}
#endif /* (MACH_ASSERT|MACH_LASSERT) */
#endif /* RT_SCHED */

	if (thread->policy == POLICY_TIMESHARE) {
	    do_priority_computation(thread, pri);
	    if (thread->depress_priority < 0)
		set_pri(thread, pri, may_reschedule);
	    else
		thread->depress_priority = pri;
	}
	else {
		set_pri(thread, thread->priority, may_reschedule);
	}
}

/*
 *	compute_my_priority:
 *
 *	Version of compute priority for current thread or thread
 *	being manipulated by scheduler (going on or off a runq).
 *	Only used for priority updates.  Policy or priority changes
 *	must call compute_priority above.  Caller must know thread
 *	is not depressed and have thread locked.
 */

compute_my_priority(thread)
	register thread_t	thread;
#define compute_my_priority(thread) 				\
	MACRO_BEGIN						\
	register int temp_pri;					\
								\
	if (((thread)->task->kernel_vm_space == FALSE)  ||	\
	    ((thread)->priority >= BASEPRI_USER)	||	\
	    (kernel_thread_fixed_pri_disable)) 		{	\
		do_priority_computation(thread,temp_pri);	\
		(thread)->sched_pri = temp_pri;			\
	}							\
	MACRO_END

{
#if RT_SCHED
	ASSERT(issplsched());
	LASSERT(thread->lock.lock_data != 0);
	ASSERT(thread->runq == RUN_QUEUE_NULL);
	ASSERT(thread->depress_priority >= 0);
#endif /* RT_SCHED */

    	compute_my_priority(thread);
}

/*
 *	recompute_priorities:
 *
 *	Update the priorities of all threads periodically.
 */
recompute_priorities()
{
#if	SIMPLE_CLOCK
	int	new_usec;
#endif

    	thread_swap_tick++;	/* for swapper */
#if	!MACH_HOST
	sched_tick++;
	update_runq_pri(&default_pset.runq);
#endif

	timeout(recompute_priorities, (caddr_t) 0, hz);
#if	SIMPLE_CLOCK
	/*
	 *	Compensate for clock drift.  sched_usec is an
	 *	exponential average of the number of microseconds in
	 *	a second.  It decays in the same fashion as cpu_usage.
	 */
	new_usec = sched_usec_elapsed();
	sched_usec = (5*sched_usec + 3*new_usec)/8;
#endif
	/*
	 *	Wakeup scheduler thread.
	 */
	if (sched_thread_id != THREAD_NULL) {
		clear_wait(sched_thread_id, THREAD_AWAKENED, FALSE);
	}
}

/*
 *	update_priority
 *
 *	Cause the priority computation of a thread that has been 
 *	sleeping or suspended to "catch up" with the system.  Thread
 *	*MUST* be locked by caller.  If thread is running, then this
 *	can only be called by the thread on itself.
 */
/*
 * Split update_priority into two macros/routines.  The update_usage macro
 * is used by do_thread_scan -- it updates the usage information without
 * calling compute_my_priority.  This avoids calling compute_my_priority on
 * a thread which is in a run queue.
 */
update_usage(thread)
	register thread_t	thread;

#define update_usage(thread)						\
	MACRO_BEGIN							\
	register unsigned int	ticks;					\
	register shift_t	shiftp;					\
									\
	ticks = sched_tick - (thread)->sched_stamp;			\
									\
	if (ticks == 0) panic("Bad update");				\
									\
	/*								\
	 *	If asleep for more than 30 seconds forget all		\
	 *	cpu_usage, else catch up on missed aging.		\
	 *	5/8 ** n is approximated by the two shifts		\
	 *	in the wait_shift array.				\
	 */								\
	(thread)->sched_stamp += ticks;					\
	thread_timer_delta((thread));					\
	if (ticks >  30) {						\
		(thread)->cpu_usage = 0;				\
		(thread)->sched_usage = 0;				\
	}								\
	else {								\
		(thread)->cpu_usage += (thread)->cpu_delta;		\
		shiftp = &wait_shift[ticks];				\
		if (shiftp->shift2 > 0) {				\
		    (thread)->cpu_usage =				\
			((thread)->cpu_usage >> shiftp->shift1) +	\
			((thread)->cpu_usage >> shiftp->shift2);	\
		    (thread)->sched_usage =				\
			((thread)->sched_usage >> shiftp->shift1) +	\
			((thread)->sched_usage >> shiftp->shift2);	\
		}							\
		else {							\
		    (thread)->cpu_usage =				\
			((thread)->cpu_usage >> shiftp->shift1) -	\
			((thread)->cpu_usage >> -(shiftp->shift2));	\
		    (thread)->sched_usage =				\
			((thread)->sched_usage >> shiftp->shift1) -	\
			((thread)->sched_usage >> -(shiftp->shift2));	\
		}							\
		(thread)->sched_usage += (thread)->sched_delta;	        \
	}								\
	(thread)->cpu_delta = 0;					\
	(thread)->sched_delta = 0;					\
	MACRO_END
{
    	update_usage(thread);
}

#if RT_SCHED
update_priority(thread)
	register thread_t	thread;

#define update_priority(thread)						\
	MACRO_BEGIN							\
	update_usage(thread);						\
	if (((thread)->policy == POLICY_TIMESHARE) &&			\
	    ((thread)->depress_priority < 0)) {				\
		compute_my_priority(thread);				\
	}								\
	MACRO_END
{
	ASSERT(issplsched());
	LASSERT(thread->lock.lock_data != 0);
	ASSERT(thread->runq == RUN_QUEUE_NULL);
#else /* RT_SCHED */
update_priority(thread)
	register thread_t	thread;

#define update_priority(thread)						\
	MACRO_BEGIN							\
	register unsigned int	ticks;					\
	register shift_t	shiftp;					\
									\
	ticks = sched_tick - (thread)->sched_stamp;			\
									\
	if (ticks == 0) panic("Bad update");				\
									\
	/*								\
	 *	If asleep for more than 30 seconds forget all		\
	 *	cpu_usage, else catch up on missed aging.		\
	 *	5/8 ** n is approximated by the two shifts		\
	 *	in the wait_shift array.				\
	 */								\
	(thread)->sched_stamp += ticks;					\
	thread_timer_delta((thread));					\
	if (ticks >  30) {						\
		(thread)->cpu_usage = 0;				\
		(thread)->sched_usage = 0;				\
	}								\
	else {								\
		(thread)->cpu_usage += (thread)->cpu_delta;		\
		shiftp = &wait_shift[ticks];				\
		if (shiftp->shift2 > 0) {				\
		    (thread)->cpu_usage =				\
			((thread)->cpu_usage >> shiftp->shift1) +	\
			((thread)->cpu_usage >> shiftp->shift2);	\
		    (thread)->sched_usage =				\
			((thread)->sched_usage >> shiftp->shift1) +	\
			((thread)->sched_usage >> shiftp->shift2);	\
		}							\
		else {							\
		    (thread)->cpu_usage =				\
			((thread)->cpu_usage >> shiftp->shift1) -	\
			((thread)->cpu_usage >> -(shiftp->shift2));	\
		    (thread)->sched_usage =				\
			((thread)->sched_usage >> shiftp->shift1) -	\
			((thread)->sched_usage >> -(shiftp->shift2));	\
		}							\
	        (thread)->sched_usage += (thread)->sched_delta;		\
	}								\
	(thread)->cpu_delta = 0;					\
	(thread)->sched_delta = 0;					\
	if (((thread)->policy == POLICY_TIMESHARE) &&			\
	    ((thread)->depress_priority < 0)) {				\
		compute_my_priority(thread);				\
	}								\
	MACRO_END
{
#endif /* RT_SCHED */
    	update_priority(thread);
}

#if RT_SCHED
/*
 * RT_SCHED_ENQUEUE is invoked from within run_queue_enqueue.  It was added
 * so that two slightly different definitions of run_queue_enqueue would not
 * be required.  When running the Realtime policies, need to be able to
 * push preempted threads back to the head of the run queue.
 *
 * Note that a third parameter, tail, had to be added to run_queue_enqueue
 * in order to pass the tail expression to this macro.  This parameter is
 * passed the undefined value RT_SCHED_UNUSED when RT_SCHED is 0.
 */
#define RT_SCHED_ENQUEUE(th,whichq,rq,tail)				\
	if (tail) {							\
	  enqueue_tail(&(rq)->runq[whichq], (queue_entry_t) (th));	\
	  RT_SCHED_HIST(RTS_totail1, (th), (th)->policy,		\
			whichq, 0, 0 );  				\
	}			       					\
	else {								\
	  enqueue_head(&(rq)->runq[whichq], (queue_entry_t) (th));	\
	  RT_SCHED_HIST(RTS_tohead1, (th), (th)->policy,		\
			whichq, 0, 0 );					\
	}								\
	RT_SCHED_SET_RUNQ_BIT(&(rq)->mask,whichq)
#else /* RT_SCHED */
#define RT_SCHED_ENQUEUE(th,whichq,rq,tail)				\
	enqueue_tail(&(rq)->runq[whichq], (queue_entry_t) (th))
#endif /* RT_SCHED */
/*
 *	run_queue_enqueue macro for thread_setrun().
 */
#if	DEBUG
#define run_queue_enqueue(rq, th, tail)					\
	MACRO_BEGIN							\
	    register unsigned int	whichq;				\
									\
	    whichq = (th)->sched_pri;					\
	    if (whichq >= NRQS) {					\
	printf("thread_setrun: pri too high (%d)\n", (th)->sched_pri);  \
		whichq = NRQS - 1;					\
	    }								\
									\
	    simple_lock(&(rq)->lock);	/* lock the run queue */	\
	    RT_SCHED_ENQUEUE(th, whichq, rq, tail);			\
									\
	    if (whichq < (rq)->low || (rq)->count == 0) 		\
		 (rq)->low = whichq;	/* minimize */			\
									\
	    (rq)->count++;						\
	    (th)->runq = (rq);						\
	    thread_check((th), (rq));					\
	    checkrq((rq));						\
	    simple_unlock(&(rq)->lock);					\
	MACRO_END
#else	/* !DEBUG */
#define run_queue_enqueue(rq, th, tail)					\
	MACRO_BEGIN							\
	    register unsigned int	whichq;				\
									\
	    whichq = (th)->sched_pri;					\
	    if (whichq >= NRQS) {					\
		whichq = NRQS - 1;					\
	    }								\
									\
	    simple_lock(&(rq)->lock);	/* lock the run queue */	\
	    RT_SCHED_ENQUEUE(th, whichq, rq, tail);			\
									\
	    if (whichq < (rq)->low || (rq)->count == 0) 		\
		 (rq)->low = whichq;	/* minimize */			\
									\
	    (rq)->count++;						\
	    (th)->runq = (rq);						\
	    simple_unlock(&(rq)->lock);					\
	MACRO_END
#endif	/* !DEBUG */
/*
 *	thread_setrun:
 *
 *	Make thread runnable; dispatch directly onto an idle processor
 *	if possible.  Else put on appropriate run queue (processor
 *	if bound, else processor set.  Caller must have lock on thread.
 *	This is always called at splsched.
 */

void thread_setrun(th, may_preempt)
	register thread_t	th;
	boolean_t		may_preempt;
{
	register processor_t	processor;
	register run_queue_t	rq;
#if RT_SCHED
	register thread_t	c_th;
	register processor_t	myprocessor;
#endif /* RT_SCHED */
#if	NCPUS > 1
	register processor_set_t	pset;
#endif

#if RT_SCHED
	ASSERT(issplsched());
	LASSERT(th->lock.lock_data != 0);
#endif /* RT_SCHED */

	/*
	 *      Update priority if needed.  Do not recalculate priorities
	 *      for threads that have boosted priorities due to an event
	 *      wait.
	 */
	if (th->sched_stamp != sched_tick &&
	   (th->sched_pri >= th->priority)) {
		update_priority(th);
	}

	if (th->runq != RUN_QUEUE_NULL) {
#if	DEBUG
	    printf("thread_setrun: thread 0x%x already on run queue 0x%x.\n",
		th, th->runq);
#endif
	    panic("thread_setrun");
	}
	RT_SCHED_HIST(RTS_setrun, th, th->policy, 
		      (th->sched_pri << 16) | (th->state & 0xFFFF),
		      th->rmng_quantum,
		      ((may_preempt?1:0)
		       |((th->policy & ~(POLICY_RR|POLICY_FIFO))?2:0)
		       |((th->state != TH_RUN)?4:0)
		       |((th->rmng_quantum <= 0)?8:0)));

	/*
	 *	Debugging code to find zombie walks panic
	 */
	if ((th->ast & AST_TERMINATE) && th->halted) {
	    panic("zombie awakened!!");
	}

#if	NCPUS > 1
	/*
	 *	Try to dispatch the thread directly onto an idle processor.
	 */
	if ((processor = th->bound_processor) == PROCESSOR_NULL) {
	    /*
	     *	Not bound, any processor in the processor set is ok.
	     */
	    pset = th->processor_set;
#if	HW_FOOTPRINT
	    /*
	     *	But first check the last processor it ran on.
	     */
	    processor = th->last_processor;
	    if (processor->state == PROCESSOR_IDLE) {
		    simple_lock(&processor->lock);
		    simple_lock(&pset->idle_lock);
		    if ((processor->state == PROCESSOR_IDLE)
#if	MACH_HOST
			&& (processor->processor_set == pset)
#endif
			) {
			    queue_remove(&pset->idle_queue, processor,
			        processor_t, processor_queue);
			    pset->idle_count--;
			    processor->next_thread = th;
			    processor->state = PROCESSOR_DISPATCHING;
			    simple_unlock(&pset->idle_lock);
			    simple_unlock(&processor->lock);
		            return;
		    }
		    simple_unlock(&pset->idle_lock);
		    simple_unlock(&processor->lock);
	    }
#endif	/* HW_FOOTPRINT */

	    if (pset->idle_count > 0) {
		simple_lock(&pset->idle_lock);
		if (pset->idle_count > 0) {
		    processor = (processor_t) queue_first(&pset->idle_queue);
		    queue_remove(&(pset->idle_queue), processor, processor_t,
				processor_queue);
		    pset->idle_count--;
		    processor->next_thread = th;
		    processor->state = PROCESSOR_DISPATCHING;
		    simple_unlock(&pset->idle_lock);
		    return;
		}
		simple_unlock(&pset->idle_lock);
	    }
	    rq = &(pset->runq);
#if RT_SCHED
	    run_queue_enqueue(rq,th,(may_preempt
				     || (th->rmng_quantum <= 0)
				     || (th->state != TH_RUN)));
#else /* RT_SCHED */
	    run_queue_enqueue(rq,th,RT_SCHED_UNUSED);
#endif /* RT_SCHED */
	    /*
	     * Preempt check
	     */
#if RT_SCHED
	    c_th = current_thread();
	    if (may_preempt && (c_th->sched_pri > rq->low)) {
	    		/* Preserve remaining quantum if policy requires it. */
			myprocessor = current_processor();
			if ((c_th->policy & (POLICY_RR|POLICY_FIFO))
			    && (myprocessor->first_quantum)) {
			  myprocessor->was_first_quantum = TRUE;
			  RT_SCHED_HIST(RTS_rrqsave5, c_th, c_th->policy,
					myprocessor->quantum, c_th->state,
					(((c_th->state == TH_RUN)?1:0)
					 |((myprocessor->first_quantum)?2:0)
					 |((myprocessor->quantum > 0)?4:0)));
			}
			myprocessor->first_quantum = FALSE;
#else /* RT_SCHED */
	    if (may_preempt && (current_thread()->sched_pri > rq->low)) {
			/*
			 *	Turn off first_quantum to allow csw.
			 */
			current_processor()->first_quantum = FALSE;
#endif /* RT_SCHED */
			aston();
#if	RT_PREEMPT
			ast_mode[cpu_number()] = KERNEL_AST;
#endif
	    }
	}
	else {
	    /*
	     *	Bound, can only run on bound processor.  Have to lock
	     *  processor here because it may not be the current one.
	     */
	    if (processor->state == PROCESSOR_IDLE) {
		simple_lock(&processor->lock);
		pset = processor->processor_set;
		simple_lock(&pset->idle_lock);
		if (processor->state == PROCESSOR_IDLE) {
		    queue_remove(&pset->idle_queue, processor,
			processor_t, processor_queue);
		    pset->idle_count--;
		    processor->next_thread = th;
		    processor->state = PROCESSOR_DISPATCHING;
		    simple_unlock(&pset->idle_lock);
		    simple_unlock(&processor->lock);
		    return;
		}
		simple_unlock(&pset->idle_lock);
		simple_unlock(&processor->lock);
	    }
	    rq = &(processor->runq);
#if RT_SCHED
	    run_queue_enqueue(rq,th,(may_preempt
				     || (th->rmng_quantum <= 0)
				     || (th->state != TH_RUN)));
#else /* RT_SCHED */
	    run_queue_enqueue(rq,th,RT_SCHED_UNUSED);
#endif /* RT_SCHED */

	    /*
	     *	Cause ast on processor if processor is on line.
	     *
	     *	XXX Don't do this remotely to master because this will
	     *	XXX send an interprocessor interrupt, and that's too
	     *  XXX expensive for all the unparallelized U*x code.
	     */
	    if (processor == current_processor()) {
		aston();
#if	RT_PREEMPT
		ast_mode[cpu_number()] = KERNEL_AST;
#endif
	    }
	    else if ((processor != master_processor) &&
	    	     (processor->state != PROCESSOR_OFF_LINE)) {
			cause_ast_check(processor);
	    }
	}
#else	/* NCPUS <= 1 */
	/*
	 *	XXX should replace queue with a boolean in this case.
	 */
	if (default_pset.idle_count > 0) {
	    processor = (processor_t) queue_first(&default_pset.idle_queue);
	    queue_remove(&default_pset.idle_queue, processor,
		processor_t, processor_queue);
	    default_pset.idle_count--;
	    processor->next_thread = th;
	    processor->state = PROCESSOR_DISPATCHING;
	    return;
	}
	if (th->bound_processor == PROCESSOR_NULL) {
	    	rq = &(default_pset.runq);
	}
	else {
		rq = &(master_processor->runq);
		aston();
#if	RT_PREEMPT
		ast_mode[cpu_number()] = KERNEL_AST;
#endif
	}
#if RT_SCHED
	run_queue_enqueue(rq,th,(may_preempt
				 || (th->rmng_quantum <= 0)
				 || (th->state != TH_RUN)));
#else /* RT_SCHED */
	run_queue_enqueue(rq,th,RT_SCHED_UNUSED);
#endif /* RT_SCHED */

	/*
	 * Preempt check
	 */
#if RT_SCHED
	c_th = current_thread();
	if (may_preempt && (c_th->sched_pri > rq->low)) {
		/* Preserve remaining quantum if policy requires it. */
		myprocessor = current_processor();
		if ((c_th->policy & (POLICY_RR|POLICY_FIFO))
		    && (myprocessor->first_quantum)) {
		  myprocessor->was_first_quantum = TRUE;
		  RT_SCHED_HIST(RTS_rrqsave4, c_th, c_th->policy,
				myprocessor->quantum, c_th->state,
				(((c_th->state == TH_RUN)?1:0)
				 |((myprocessor->first_quantum)?2:0)
				 |((myprocessor->quantum > 0)?4:0)));
		}
		myprocessor->first_quantum = FALSE;
#else /* RT_SCHED */
	if (may_preempt && (current_thread()->sched_pri > rq->low)) {
		/*
		 *	Turn off first_quantum to allow context switch.
		 */
		current_processor()->first_quantum = FALSE;
#endif /* RT_SCHED */
		aston();
#if	RT_PREEMPT
		ast_mode[cpu_number()] = KERNEL_AST;
#endif
	}
#endif	/* NCPUS <= 1 */
}

/*
 *	rem_runq:
 *
 *	Remove a thread from its run queue.
 *	The run queue that the process was on is returned
 *	(or RUN_QUEUE_NULL if not on a run queue).  Thread *must* be locked
 *	before calling this routine.  Unusual locking protocol on runq
 *	field in thread structure makes this code interesting; see thread.h.
 */

struct run_queue *rem_runq(th)
	thread_t		th;
{
	register struct run_queue	*rq;
#if RT_SCHED_RQ
	register queue_head_t		*q;
	register int			pri;
#endif /* RT_SCHED_RQ */

	rq = th->runq;
	/*
	 *	If rq is RUN_QUEUE_NULL, the thread will stay out of the
	 *	run_queues because the caller locked the thread.  Otherwise
	 *	the thread is on a runq, but could leave.
	 */
	if (rq != RUN_QUEUE_NULL) {
#if RT_SCHED
		/* 
		 * It is OK to call this at lower spl if the thread is not
		 * known to the rest of the system (this is taken advantage
		 * of by procdup()).  Moving this assertion inside the
		 * rq test weakens it enough so that procdup can do its thing.
		 */
		ASSERT(issplsched());
#endif /* RT_SCHED */
		simple_lock(&rq->lock);
#if	DEBUG
		checkrq(rq);
#endif
		if (rq == th->runq) {
			/*
			 *	Thread is in a runq and we have a lock on
			 *	that runq.
			 */
#if	DEBUG
			checkrq(rq);
			thread_check(th, rq);
#endif
#if RT_SCHED_RQ
			/*
			 * To keep the bitmask and rq->low correct at all
			 * times, it is necessary to know what queue the
			 * thread is being removed from.  Use sched_pri to
			 * determine the queue.  Note that this means that
			 * sched_pri must be kept correct at all times, too.
			 * Modifications to do_thread_scan accomplish this.
			 */
			pri = th->sched_pri;
			q = &rq->runq[pri];
			remqueue(q, (queue_entry_t) th);
			RT_SCHED_UPDATE_RQ(rq,q,pri);

			RT_SCHED_HIST(RTS_remq, th, th->priority,
				      th->links.next, th->links.prev, 
				      th->sched_pri);
#else /* RT_SCHED_RQ */
			remqueue(&rq->runq[0], (queue_entry_t) th);
#endif /* RT_SCHED_RQ */
			rq->count--;
#if	DEBUG
			checkrq(rq);
#endif
			th->runq = RUN_QUEUE_NULL;
			simple_unlock(&rq->lock);
		}
		else {
			/*
			 *	The thread left the runq before we could
			 * 	lock the runq.  It is not on a runq now, and
			 *	can't move again because this routine's
			 *	caller locked the thread.
			 */
			simple_unlock(&rq->lock);
			rq = RUN_QUEUE_NULL;
		}
	}

	return(rq);
}


/*
 *	choose_thread:
 *
 *	Choose a thread to execute.  The thread chosen is removed
 *	from its run queue.  Note that this requires only that the runq
 *	lock be held.
 *
 *	Strategy:
 *		Check local runq first; if anything found, run it.
 *		Else check global runq; if nothing found, return idle thread.
 */

thread_t choose_thread(myprocessor)
processor_t myprocessor;
{
	thread_t th;
	register queue_t q;
	register run_queue_t runq;
	register int i;
	processor_set_t	pset;

	runq = &myprocessor->runq;
#ifdef	KTRACE
	kern_trace(905,0,0,0);
#endif  /* KTRACE */
	if (runq->count > 0) {
	    simple_lock(&runq->lock);
	    if (runq->count > 0) {
		q = runq->runq + runq->low;
#if RT_SCHED_RQ
		/*
		 * When the run queue bitmask is compiled in, both the bitmask
		 * and runq->low are kept accurate at all times.  This means
		 * that runq->low will always point to a nonempty queue if
		 * runq->count > 0, so no need to update it here.
		 *
		 * Fall through to panic if the run queue is corrupt.
		 * The test and panic may be removed to improve performance.
		 */
		th = (thread_t) dequeue_head(q);
		if (th != THREAD_NULL) {
		  th->runq = RUN_QUEUE_NULL;
		  runq->count--;
		  RT_SCHED_UPDATE_RQ_LOW(runq,q);
		  simple_unlock(&runq->lock);
		  return(th);
		}
#else /* RT_SCHED_RQ */
		for (i = runq->low; i < NRQS ; i++) {
		    if (!queue_empty(q)) {
			th = (thread_t) dequeue_head(q);
			th->runq = RUN_QUEUE_NULL;
			runq->count--;
			runq->low = i;
			simple_unlock(&runq->lock);
			return(th);
		    }
		    else {
			q++;
		    }
		}
#endif /* RT_SCHED_RQ */
		panic("choose_thread 1");
		/*NOTREACHED*/
	    }
	    else {
		simple_unlock(&runq->lock);
	    }
	}

	pset = myprocessor->processor_set;
	runq = &(pset->runq);

	if (runq->count > 0) {
	    simple_lock(&runq->lock);
	    if (runq->count > 0) {
		q = runq->runq + runq->low;
#if RT_SCHED_RQ
		/*
		 * When the run queue bitmask is compiled in, both the bitmask
		 * and runq->low are kept accurate at all times.  This means
		 * that runq->low will always point to a nonempty queue if
		 * runq->count > 0, so no need to update it here.
		 *
		 * Fall through to panic if the run queue is corrupt.
		 * The test and panic may be removed to improve performance.
		 */
		th = (thread_t) dequeue_head(q);
		if (th != THREAD_NULL) {
		  th->runq = RUN_QUEUE_NULL;
		  runq->count--;
		  RT_SCHED_UPDATE_RQ_LOW(runq,q);
		  simple_unlock(&runq->lock);
		  return(th);
		}
#else /* RT_SCHED_RQ */
		for (i = runq->low; i < NRQS ; i++) {
		    if (!queue_empty(q)) {
			th = (thread_t) dequeue_head(q);
			th->runq = RUN_QUEUE_NULL;
			runq->count--;
			/*
			 *	For POLICY_FIXEDPRI, runq->low must be
			 *	accurate!
			 */
			if (runq->count > 0) {
#if RT_SCHED
			    if (pset->policies & (POLICY_FIXEDPRI
						  |POLICY_RR|POLICY_FIFO)) {
#else /* RT_SCHED */
			    if (pset->policies & POLICY_FIXEDPRI) {
#endif /* RT_SCHED */
			        while ((i < NRQS) && queue_empty(q)) {
				    q++;
				    i++;
				}
			    }
			    runq->low = i;
			    RT_SCHED_HIST(RTS_rqlow, 0, pset->policies,
					  i, 0, 0 );
			}
			simple_unlock(&runq->lock);
			return(th);
		    }
		    else {
			q++;
		    }
		}
#endif /* RT_SCHED_RQ */
		panic("choose_thread 2");
		/*NOTREACHED*/
	    }
	    simple_unlock(&runq->lock);
	}
	/*
	 *	Nothing is runnable, so set this processor idle if it
	 *	was running.  If it was in an assignment or shutdown,
	 *	leave it alone.  Return its idle thread.
	 */
	simple_lock(&pset->idle_lock);
	if (myprocessor->state == PROCESSOR_RUNNING) {
	    myprocessor->state = PROCESSOR_IDLE;
	    /*
	     *	XXX Until it goes away, put master on end of queue, others
	     *	XXX on front so master gets used last.
	     */
	    if (myprocessor == master_processor) {
		queue_enter(&(pset->idle_queue), myprocessor,
			processor_t, processor_queue);
	    }
	    else {
		queue_enter_first(&(pset->idle_queue), myprocessor,
			processor_t, processor_queue);
	    }

	    pset->idle_count++;
	}
	simple_unlock(&pset->idle_lock);

	return(myprocessor->idle_thread);
}

/*
 *	This is the idle thread, which just looks for other threads
 *	to execute.
 */


/*
 *	no_dispatch_count counts number of times processors go non-idle
 *	without being dispatched.  This should be very rare.
 */
int	no_dispatch_count = 0;

void idle_thread()
{
#ifdef	__alpha
	volatile thread_t *threadp;
#else
	volatile register thread_t *threadp;
#endif
	volatile register int *gcount, *lcount;
	register thread_t new_thread;
	register processor_t	myprocessor;
	register int	state;
	struct thread *th;
	int mycpu;

	extern	int vm_zeroed_pages_wanted;
	extern	void vm_page_zeroer();

	(void) splsched();
	th = current_thread();			/* who am I? */
	mycpu = cpu_number();			/* where am I? */
	myprocessor = cpu_to_processor(mycpu);

	/*
	 * RT_SCHED:  This was assigning 31 to the priority fields.  Replaced
	 *	      that with a constant, which is always defined in
	 *	      kern/sched.h.
	 */
	th->priority = BASEPRI_LOWEST;
	th->sched_pri = BASEPRI_LOWEST;

	/*
	 *	Set the idle flag to indicate that this is an idle thread,
	 *	enter ourselves in the idle array, and thread_block() to get
	 *	out of the run queues (and set the processor idle when we
	 *	run next time).
	 */
	thread_lock(th);
	th->state |= TH_IDLE;
	thread_unlock(th);
	myprocessor->idle_thread = th;
	thread_block();

	threadp = (volatile thread_t *)&myprocessor->next_thread;
	lcount =  (volatile int *)&(myprocessor->runq.count);

#if	!MACH_HOST
	gcount = &default_pset.runq.count;
#endif

	for (;;) {

#ifdef	MARK_CPU_IDLE
		MARK_CPU_IDLE(mycpu);
#else
		/*
		 *	Deactivate kernel pmap to avoid shootdown
		 *	interupts.
		 */
		PMAP_DEACTIVATE(kernel_pmap, th, mycpu);
#endif
#if	MACH_HOST
		gcount = &(myprocessor->processor_set->runq.count);
#endif

		spl0();		/* for idle loop */
/*
 *	This cpu will be dispatched (by thread_setrun) by setting next_thread
 *	to the value of the thread to run next.  Also check runq counts
 *	and should_exit.
 *
 */
		while (*threadp == THREAD_NULL && !*gcount && !*lcount &&
			!kernel_async_trap[cpu_number()].flags.lwc_pending)
			if (vm_zeroed_pages_wanted)
				vm_page_zeroer();

		if (kernel_async_trap[cpu_number()].flags.lwc_pending) {
			lwc_schedule();
			continue;
		}

		(void) splsched();
#ifdef	MARK_CPU_ACTIVE
		MARK_CPU_ACTIVE(mycpu);
#else
		PMAP_ACTIVATE(kernel_pmap, th, mycpu);
#endif

		/*
		 *	This is not a switch statement to avoid the
		 *	bounds checking code in the common case.
		 */
retry:
		state = myprocessor->state;
		if (state == PROCESSOR_DISPATCHING) {
			/*
			 *	Commmon case -- cpu dispatched.
			 */
			new_thread = *threadp;
			*threadp = THREAD_NULL;
			myprocessor->state = PROCESSOR_RUNNING;
			/*
			 *	set up quantum for new thread.
			 */
			if (new_thread->policy == POLICY_TIMESHARE) {
				/*
				 *  Just use set quantum.  No point in
				 *  checking for shorter local runq quantum;
				 *  csw_needed will handle correctly.
				 */
#if	MACH_HOST
				myprocessor->quantum = new_thread->
					processor_set->set_quantum;
#else
				myprocessor->quantum =
					default_pset.set_quantum;
#endif

				myprocessor->first_quantum = TRUE;
			}
			else {
#if RT_SCHED
				/* Restore rmng quantum if required. */
				if (new_thread->rmng_quantum > 0) {
				  myprocessor->quantum = 
				    new_thread->rmng_quantum;
				  new_thread->rmng_quantum = 0;
				  RT_SCHED_HIST(RTS_rrqrest3, new_thread,
						new_thread->policy,
						myprocessor->quantum, 0, 0);
				}
				else {
				  myprocessor->quantum = 
				    new_thread->sched_data;
				}
				myprocessor->first_quantum =
				  (new_thread->policy != POLICY_FIXEDPRI);

			}
			myprocessor->was_first_quantum = FALSE;
#else /* RT_SCHED */
				/*
				 *	POLICY_FIXEDPRI
				 */
				myprocessor->quantum = new_thread->sched_data;
				myprocessor->first_quantum = FALSE;
			}
#endif /* RT_SCHED */
			thread_run(new_thread);
		}
		else if (state == PROCESSOR_IDLE) {
			register processor_set_t pset;

			pset = myprocessor->processor_set;
			simple_lock(&pset->idle_lock);
			if (myprocessor->state != PROCESSOR_IDLE) {
				/*
				 *	Something happened, try again.
				 */
				simple_unlock(&pset->idle_lock);
				goto retry;
			}
			/*
			 *	Processor was not dispatched (Rare).
			 *	Set it running again.
			 */
			no_dispatch_count++;
			pset->idle_count--;
			queue_remove(&pset->idle_queue, myprocessor,
				processor_t, processor_queue);
			myprocessor->state = PROCESSOR_RUNNING;
			simple_unlock(&pset->idle_lock);
			thread_block();
		}
		else if ((state == PROCESSOR_ASSIGN) ||
			 (state == PROCESSOR_SHUTDOWN)) {
			/*
			 *	Changing processor sets, or going off-line.
			 *	Release next_thread if there is one.  Actual
			 *	thread to run in on a runq.
			 */
			if ((new_thread = *threadp)!= THREAD_NULL) {
				*threadp = THREAD_NULL;
				thread_setrun(new_thread, FALSE);
			}

			thread_block();
		}
		else {
#if	DEBUG
			printf(" Bad processor state %d (Cpu %d)\n",
				cpu_state(mycpu), mycpu);
#endif
			panic("idle_thread");
		}
	}
}
		
/*
 *	sched_thread: scheduler thread.
 *
 *	This thread handles periodic calculations in the scheduler that
 *	we don't want to do at interrupt level.  This allows us to
 *	avoid blocking 
 */
void sched_thread()
{
    extern void sar_runq_update();

    sched_thread_id = current_thread();

    while (TRUE) {
	/*
	 *	Sleep on event 0, recompute_priorities() will awaken
	 *	us by calling clear_wait().
	 */
	assert_wait((vm_offset_t)0, FALSE);
	thread_block();
	(void) compute_mach_factor();
	sar_runq_update();
#if MACH_HOST
	sched_tick++;
	update_pset_runq_pri();
#endif /* MACH_HOST */
    }
}

#if MACH_HOST
/*
 * update_pset_runq_pri is called only if muliple processor sets are
 * supported.  Its function is to recalcute priorities for all threads
 * on run queues for all processor sets. This routine can not block and
 * must be called at splnone. 
 */
update_pset_runq_pri() {
	register processor_set_t	pset;

	ASSERT(getspl() == 0);

	simple_lock(&all_psets_lock);
	pset = (processor_set_t) queue_first(&all_psets);
	while (!queue_end(&all_psets, (queue_entry_t) pset)) {
		update_runq_pri(&pset->runq))
		pset = (processor_set_t) queue_next(&pset->all_psets);
	}
	simple_unlock(&all_psets_lock);

}
#endif /* MACH_HOST */

/*
 * update_runq_pri recalculates priorities for jobs on a run 
 * queue with the policy of timeshare.  This routine does not
 * block and can be called at splsched or lower.
 */
update_runq_pri(runq)
run_queue_t	runq;
{
	register queue_t	q;
	register thread_t	thread;
	register thread_t	thread_next;
	register int		count;
	int			s;

	s = splsched();
	simple_lock(&runq->lock);
	if((count = runq->count) > 0) {
	    q = runq->runq + runq->low;
	    while (count > 0) {
		thread = (thread_t) queue_first(q);
		while(!queue_end(q, (queue_entry_t)thread)) {
		    thread_next = (thread_t) queue_next((queue_t)thread);

		    if (simple_lock_try(&thread->lock)) {
#if RT_SCHED
		    	/*
		    	 * Only mess with timesharing threads.  Trying to update
		    	 * the priority of a fixed priority policy thread won't
		    	 * help anyway.
		    	 */
		    	if ((sched_tick != thread->sched_stamp)
			    && (thread->sched_pri > thread->priority)
			    && (thread->policy == POLICY_TIMESHARE)) {
#else /* RT_SCHED */
		    	if ((thread->sched_pri > thread->priority) &&
			    (sched_tick != thread->sched_stamp )) {
#endif /* RT_SCHED */
			    int new_pri;

			    update_usage(thread);
			    do_priority_computation(thread, new_pri);
			    if (new_pri != thread->sched_pri) {
				if (thread->depress_priority < 0) {
				    remque((queue_entry_t)thread);
				    RT_SCHED_UPDATE_RQ(runq,&runq->runq[thread->sched_pri],thread->sched_pri);
				    thread->sched_pri = new_pri;
				    RT_SCHED_ENQUEUE(thread,new_pri,runq,TRUE);
				    if (new_pri < runq->low || runq->count ==0)
					runq->low = new_pri;
				} else {
				    thread->depress_priority = new_pri;
				}
			    }
			}
			thread_unlock(thread);
		    }
		    thread = thread_next;
		    count--;
		}
		q++;
	    }
	}
	simple_unlock(&runq->lock);
	splx(s);

}
		
/*
 *	Just in case someone doesn't use the macro
 */
#undef	thread_wakeup
void		thread_wakeup(x)
	register vm_offset_t	x;
{
	thread_wakeup_with_result(x, THREAD_AWAKENED);
}

#if	someday
/*
 *	create_wait(event, persistent)
 *
 *	Create and initialize a new wait structure corresponding to the
 *	specified event.  A persistence is specified to determine if the
 *	wait structure should ever be deallocated.  (Persistent wait
 *	structures are used for frequently used events).
 *
 *	The wait structure is entered into the event hash table.
 */

wait_t create_wait(event, persistent)
	event_t		event;
	boolean_t	persistent;
{
	wait_t		wait;
	wait_bucket_t	bucket;

	/*
	 *	Allocate and initialize and wait structure.
	 */

	wait = (wait_t) zalloc(wait_zone);
	wait->event = event;
	wait->persistent = persistent;
	queue_init(&wait->thread_list);
	simple_lock_init(&wait->lock);

	/*
	 *	Insert the wait structure into the hash table.
	 */

	bucket = wait_buckets[wait_hash(event)];
	lock_write(&bucket->lock);
	queue_enter(&bucket->waitq, wait, wait_t, listq);
	lock_write_done(&bucket->lock);
}

/*
 *	assert_wait(event)
 *
 *	assert that the current thread wants to wait for the specified
 *	event.  The thread does not actually wait at this time, but it
 *	should wait sometime in the near future.
 */

assert_wait(event)
	event_t		event;
{
	wait_t		wait;
	wait_bucket_t	bucket;
	boolean_t	found;
	thread_t	thread;

	/*
	 *	Find the bucket for this event.
	 */

	bucket = wait_buckets[wait_hash(event)];
	found = FALSE;

	/*
	 *	See if there is already a wait structure for the
	 *	event.
	 */

	lock_read(&bucket->lock);
	wait = (wait_t) queue_first(&bucket->waitq);
	while (!queue_end(&bucket->waitq, (queue_entry_t) wait)) {
		if (wait->event == event) {
			found = TRUE;
			break;
		}
		wait = (wait_t) queue_next(&wait->listq);
	}
	lock_read_done(&bucket->lock);

	/*
	 *	If there was no such entry, then create (and insert)
	 *	a non-persistent wait structure.
	 */

	if (!found)
		wait = create_wait(FALSE);
	}

	/*
	 *	Now we have a wait structure corresponding to our event
	 *	(which is in the hash table).  We must now insert our thread
	 *	into the list of threads waiting for this event, which means
	 *	we create yet another structure to represent it (because a
	 *	thread may be waiting for more than one event).
	 *
	 *	Then, link the thread wait structure into the thread list
	 *	of what it is waiting on.
	 */

	thread = current_thread();
	twait = (thread_wait_t) zalloc(thread_wait_zone);
	twait->thread = thread;

	simple_lock(&wait->lock);
	queue_enter(&wait->thread_list, twait, thread_wait_t, threadq);
	queue_enter(&thread->wait_list, twait, thread_wait_t, waitq);
	simple_unlock(&wait->lock);
}
#endif	/* someday */

#if	DEBUG
checkrq(rq)
	run_queue_t	rq;
{
	register queue_t	q1;
	register int		i, j;
	register queue_entry_t	e;

	j = 0;
	q1 = rq->runq;
	for (i = 0; i < NRQS; i++) {
		if (q1->next == q1) {
#if RT_SCHED_RQ
			if (RT_SCHED_RUNQ_BIT_VALUE(&rq->mask,i))
				panic("checkrq-mask-on");
#endif /* RT_SCHED_RQ */
			if (q1->prev != q1->next)
				panic("checkrq");
		}
#if RT_SCHED_RQ
		else if (!RT_SCHED_RUNQ_BIT_VALUE(&rq->mask,i))
			panic("checkrq-mask-off")
#endif /* RT_SCHED_RQ */
		for (e = q1->next; e != q1; e = e->next) {
			j++;
			if (e->next->prev != e)
				panic("checkrq-2");
			if (e->prev->next != e)
				panic("checkrq-3");
		}
		q1++;
	}
	if (j != rq->count)
		panic("checkrq-count");
#if RT_SCHED_RQ
	if (rq->low != find_first_runq_bit_set(&rq->mask))
		panic("checkrq-low");
#endif /* RT_SCHED_RQ */
}

thread_check(th, rq)
	register thread_t	th;
	register run_queue_t	rq;
{
	register unsigned int 	whichq;

	whichq = th->sched_pri;
	if (whichq >= NRQS) {
		printf("thread_check: priority too high\n");
		whichq = NRQS-1;
	}
	if ((th->links.next == &rq->runq[whichq]) &&
		(rq->runq[whichq].prev != (queue_entry_t)th))
			panic("thread_check");
}
#endif	/* DEBUG */

#if RT_SCHED_RQ
#if !RT_SCHED_OPT
#if !defined( __alpha)

/*
 * find_first_runq_bit_set
 *
 * This procedure returns the priority of the first runq which contains an
 * entry, based on the values in the runq bitmask.  Returns the lowest
 * priority if no bits are set.
 *
 * Because this routine is time critical, it is not generalized for an
 * arbitrary number of priorities.
 *
 * This procedure should be called at splsched, and the runq structure which
 * holds the bitmask should be locked.
 *
 * NOTE: This routine may run faster if it is changed to use a 128 byte lookup
 *	 table, the way the version in locore.s does.
 *
 * NOTE: on alpha the routine is in locore.s.
 */

int find_first_runq_bit_set(mask)
  register struct run_queue_bitmask *mask;
{
  register long bits;
  register int offset;
  register int base;

  /*
   * Find the first nonzero longword in the bitmask and start search there.
   */

  bits = mask->bits[0];
  if (bits) {
    base = 0;
  }
  else {
    base = LONG_BIT;
    bits = mask->bits[1];
  }

  /*
   * Perform an ordered binary search to find the first nonzero bit.  Set
   * "offset" to the (little endian) offset of that bit.  Set "offset" to
   * 31 if all bits are nonzero.  Note that this procedure is not dependent
   * on (or consistent with) little-endian bitstrings.
   *
   * No matter which bit is the first set, five longword mask operations
   * are required.
   */

  if (bits & 0xFFFF) {
    if (bits & 0xFF) {
      if (bits & 0xF) {
	if (bits & 0x3) {
	  offset = (bits & 0x1) ? 0 : 1;
	}
	else { /* (bits & 0xC) */
	  offset = (bits & 0x4) ? 2 : 3;
	}
      }
      else { /* (bits & 0xF0) */
	if (bits & 0x30) {
	  offset = (bits & 0x10) ? 4 : 5;
	}
	else { /* (bits & 0xC0) */
	  offset = (bits & 0x40) ? 6 : 7;
	}
      }
    }
    else { /* (bits & 0xFF00) */
      if (bits & 0xF00) {
	if (bits & 0x300) {
	  offset = (bits & 0x100) ? 8 : 9;
	}
	else { /* (bits & 0xC00) */
	  offset = (bits & 0x400) ? 10 : 11;
	}
      }
      else { /* (bits & 0xF000) */
	if (bits & 0x3000) {
	  offset = (bits & 0x1000) ? 12 : 13;
	}
	else {  /* (bits & 0xC000) */
	  offset = (bits & 0x4000) ? 14 : 15;
	}
      }
    }
  }
  else { /* (bits & 0xFFFF0000) || (bits == 0) */
    if (bits & 0xFF0000) {
      if (bits & 0xF0000) {
	if (bits & 0x30000) {
	  offset = (bits & 0x10000) ? 16 : 17;
	}
	else { /* (bits & 0xC0000) */
	  offset = (bits & 0x40000) ? 18 : 19;
	}
      }
      else { /* (bits & 0xF00000) */
	if (bits & 0x300000) {
	  offset = (bits & 0x100000) ? 20 : 21;
	}
	else { /* (bits & 0xC00000) */
	  offset = (bits & 0x400000) ? 22 : 23;
	}
      }
    }
    else { /* (bits & 0xFF000000) */
      if (bits & 0xF000000) {
	if (bits & 0x3000000) {
	  offset = (bits & 0x1000000) ? 24 : 25;
	}
	else { /* (bits & 0xC000000) */
	  offset = (bits & 0x4000000) ? 26 : 27;
	}
      }
      else { /* (bits & 0xF0000000) */
	if (bits & 0x30000000) {
	  offset = (bits & 0x10000000) ? 28 : 29;
	}
	else {  /* (bits & 0xC0000000) */
	  offset = (bits & 0x40000000) ? 30 : 31;
	}
      }
    }
  }

  return (base + offset);
}
#endif /* !defined( __alpha) */
#endif /* !RT_SCHED_OPT */
#endif /* RT_SCHED_RQ */

