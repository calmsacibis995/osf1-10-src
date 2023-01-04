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
static char *rcsid = "@(#)$ $ (DEC) $";
#endif
#ifndef lint
static char	*sccsid = "@(#)$RCSfile: ast.c,v $ $Revision: 4.3.3.3 $ (DEC) $Date: 1992/10/20 14:00:12 $";
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
 *
 *	This file contains routines to check whether an ast is needed.
 *
 *	ast_check() - check whether ast is needed for signal, or context
 *	switch.  Usually called by clock interrupt handler.
 *
 *	Revision History:
 *
 * 18-Oct-91 jestabro
 *      Added ALPHA support
 *
 * 02-May-91	Peter H. Smith
 *	Clean up based on code review comments.  Remove panic, platency, and
 *	disabled scheduling code.
 *
 * 9-Apr-91	Peter H. Smith
 *	Changes to sched_prim.c make the run queue hint reliable.  Change
 *	ast_check() to assume that the hint is correct.  Also, add (untested)
 *	MP code to check for a higher-priority fixed priority thread waiting
 *	to run.  Also added some scheduler monitoring code, which is only
 *	enabled when debugging the scheduler.
 *
 * 6-Apr-91	Ron Widyono
 *	Implement Kernel mode ASTs for preemption points.  Per-processor
 *	flag ast_mode[] indicates whether an AST request is User mode or 
 *	Kernel mode.  Change AST requests for preemption to Kernel mode.
 *	Conditionalized under RT_PREEMPT.
 *
 */

#include <cpus.h>
#include <hw_ast.h>
#include <rt_preempt.h>
#include <rt_sched.h>

#include <sys/unix_defs.h>		/* for compat signal code, below */
#include <sys/types.h>
#include <machine/cpu.h>
#include <machine/pcb.h>
#include <kern/ast.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <kern/queue.h>
#include <kern/sched.h>
#include <sys/systm.h>
#include <kern/thread.h>
#include <sys/user.h>
#include <kern/processor.h>
#include <sys/sched_mon.h>

#if	!HW_AST

#if	RT_PREEMPT
int	ast_mode[NCPUS];
#endif
#endif

ast_init()
{
#if	!HW_AST
	register int i;

	for (i=0; i<NCPUS; i++) {
		kernel_async_trap[i].flags.need_ast = 0;
#if	RT_PREEMPT
		ast_mode[i] = USER_AST;
#endif
	}
#endif
}

ast_check()
{
	register struct proc	*p;
	register struct uthread	*uthreadp;
	register int		mycpu = cpu_number();
	register processor_t	myprocessor;
	register thread_t	thread = current_thread();
	register run_queue_t	rq;

	/*
	 *	Check processor state for ast conditions.
	 */
	myprocessor = cpu_to_processor(mycpu);
	switch(myprocessor->state) {
	    case PROCESSOR_OFF_LINE:
	    case PROCESSOR_IDLE:
	    case PROCESSOR_DISPATCHING:
		/*
		 *	No ast.
		 */
	    	break;

#if	NCPUS > 1
	    case PROCESSOR_ASSIGN:
	    case PROCESSOR_SHUTDOWN:
	        /*
		 * 	Need ast to force action thread onto processor.
		 *
		 * XXX  Should check if action thread is already there.
		 */
		RT_SCHED_HIST_SPL(RTS_shutdown, thread, 
				  RT_SCHED_CTRL_POLICY_ALL, myprocessor, 0, 0);
		aston();
		break;
#endif

	    case PROCESSOR_RUNNING:

		/*
		 *	Propagate thread ast to processor.  If we already
		 *	need an ast, don't look for more reasons.
		 */
		ast_propagate(thread, mycpu);
		if (ast_needed(mycpu)) {
#if RT_SCHED_MON
		  if (thread->ast) {
		    RT_SCHED_HIST_SPL(RTS_thast, thread, thread->policy,
				      myprocessor, thread->ast, 0);
		  }
		  else {
		    RT_SCHED_HIST_SPL(RTS_ast, thread, 
				      RT_SCHED_CTRL_POLICY_ALL,
				      myprocessor, 
				kernel_async_trap[(mycpu)].flags.need_ast,0);
		  }
#endif /* RT_SCHED_MON */
		  break;
		}

		/*
		 *	Context switch check.  The csw_needed macro isn't
		 *	used here because the rq->low hint may be wrong,
		 *	and fixing it here avoids an extra ast.
		 *	First check the easy cases.
		 */
		if (thread->state & TH_SUSP || myprocessor->runq.count > 0) {
			RT_SCHED_HIST_SPL(RTS_astcsw, thread, thread->policy,
					  myprocessor, myprocessor->runq.count,
					  0);
			aston();
#if	RT_PREEMPT
			ast_mode[cpu_number()] = KERNEL_AST;
#endif
			break;
		}

		/*
		 *	The following code can be avoided if the processor
		 *	set allows fixed priority (choose_thread maintains
		 *	rq->low), but the extra test isn't worth the effort.
		 */
		rq = &(myprocessor->processor_set->runq);
#if RT_SCHED
		if (myprocessor->first_quantum) {
		  /*
		   * To improve preemption latency, we should check to see if
		   * a higher priority fixed-priority thread is waiting to run.
		   * Do this by looking at the thread at the head of the first
		   * nonempty run queue.  Note that if this is a timeshare
		   * thread, we don't care because even if there are fixed
		   * priority threads behind it, they can't be important or
		   * they wouldn't be down in the timeshare range.
		   */
		  /*
		   * The following code should be considered pseudo-code, since
		   * it hasn't been compiled and tested.  The first compilation
		   * switch keeps it out, so that whoever tries NCPS > 1 first
		   * doesn't have to debug this code too.
		   *
		   * The pseudo-code is here as a reminder that this might be
		   * a desireable feature for MP realtime.  A better solution
		   * would be an interprocessor interrupt and scheduler code
		   * which decides which processor is the best candidate for
		   * the interrupt.
		   */
#if RT_SCHED_NEVER
#if NCPUS > 1
		    /*
		     * If something is in the processor set run queue and we
		     * have enabled the fixed priority policies, start to look
		     * for a higher priority thread.
		     */
		    if ((rq->count > 0)
			&& (myprocessor->policies & (POLICY_FIFO|POLICY_RR))) {
		        register queue_t q;
			register int     s;

			/*
			 * Take out the run queue lock and look at the head of
			 * the highest priority run queue.  Note that if the
			 * count went to 0 before the lock was taken, rq->low
			 * will point to the lowest priority queue and that
			 * queue will be empty.
			 */
			s = splsched();
			simple_lock(&rq->lock);
			q = rq->runq + rq->low;

			/*
			 * Peek at the waiting thread to see if it is running
			 * a policy which requires override of first_quantum.
			 * If the waiting thread should preempt this thread,
			 * save this thread's remaining quantum and turn off
			 * the first_quantum flag so that it will be preempted.
			 *
			 * We don't have to worry about rq->low == 63 and the
			 * queue empty here, because thread->sched_pri >= 63.
			 */
			if (rq->low < thread->sched_pri) {
			  register thread_t waiting_thread;
			  
			  waiting_thread = q->next;
			  thread_lock(waiting_thread);
			  if (waiting_thread->policy & (POLICY_FIFO|
							POLICY_RR)) {
			    RT_SCHED_HIST(RTS_tspreempt, thread, 
					  (thread->policy
					   |waiting_thread->policy),
					  waiting_thread, 0, 0 );
			    thread_unlock(waiting_thread);
			    thread_lock(thread);
			    if (thread->policy & (POLICY_FIFO|POLICY_RR)
				&& (thread->state == TH_RUN)) {
			      myprocessor->was_first_quantum = TRUE;
			      myprocessor->first_quantum = FALSE;
			      RT_SCHED_HIST(RTS_rrqsave5, thread,
					    thread->policy,
					    myprocessor->quantum, 
					    thread->state,
					    (3|((myprocessor->quantum > 
						 0)?4:0)));
			    }
			    thread_unlock(thread);
			    simple_unlock(&rq->lock);
			    (void) splx(s);
			    aston();
			    break;
			  }
			}
			else {
			  thread_unlock(waiting_thread);
			}
		      }
		    simple_unlock(&rq->lock);
		    (void) splx(s);
		  }
#endif NCPUS > 1
#endif /* RT_SCHED_NEVER */
		}
		else if (rq->count > 0) {
#else  /* RT_SCHED */
		if (!(myprocessor->first_quantum) && (rq->count > 0)) {
#endif /* RT_SCHED */
		    register queue_t 	q;
		    /*
		     *	This is not the first quantum, and there may
		     *	be something in the processor_set runq.
		     *	Check whether low hint is accurate.
		     */
#if RT_SCHED_RQ
		    /*
		     * If the RQ option is on, then rq->low and the bitmask
		     * are always correct.  There is no need to check for
		     * accuracy here.
		     *
		     * Caution: We are peeking at rq->low without first taking
		     * out the run queue lock.  This won't work if reading an
		     * int is not atomic.
		     */
#else /* RT_SCHED_RQ */
		    q = rq->runq + rq->low;
		    if (queue_empty(q)) {
			register int i,s;

			/*
			 *	Need to recheck and possibly update hint.
			 */
			s = splsched();
			simple_lock(&rq->lock);
			RT_SCHED_HIST(RTS_rqempty4, rq->count,
				      RT_SCHED_CTRL_POLICY_ALL,
				      rq->mask.bits[0], 
				      rq->mask.bits[1], rq->low);
			q = rq->runq + rq->low;
			if (rq->count > 0) {
			    for (i = rq->low; i < NRQS; i++) {
				if(!(queue_empty(q)))
				    break;
				q++;
			    }
			    rq->low = i;
			}
			simple_unlock(&rq->lock);
			(void) splx(s);
		    }
#endif /* RT_SCHED_RQ */

		    if (rq->low <= thread->sched_pri) {
			RT_SCHED_HIST_SPL(RTS_preempt, thread, thread->policy,
					  rq->low, 0, 0);
			aston();
#if	RT_PREEMPT
			ast_mode[cpu_number()] = KERNEL_AST;
#endif
			break;
		    }
		}

		/*
		 *	XXX Else check for signals.
		 */
		p = u.u_procp;
		uthreadp = current_thread()->u_address.uthread;
		/*
		 * Assumes that memory is nice.		XXX
		 */
		if (p->p_cursig || SHOULDissig(p,uthreadp)) {
			RT_SCHED_HIST_SPL(RTS_sig, thread, thread->policy,
					  uthreadp, 0, 0);
			aston();
		}
		break;

	    default:
	        panic("ast_check: Bad processor state");
	}
}
