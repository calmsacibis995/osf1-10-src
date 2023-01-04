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
static char *rcsid = "@(#)$RCSfile: cma_sched.c,v $ $Revision: 1.1.6.2 $ (DEC) $Date: 1993/08/18 14:50:45 $";
#endif
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	These routines are used in performing scheduling (as distinct from
 *	dispatching) tasks.
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	28 August 1992
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	14 September 1992
 *		Add name to internal objects.
 *	002	Dave Butenhof, Paul Curtin, Webb Scales	15 September 1992
 *		Rearrange code in manager thread.
 *	003	Webb Scales	15 September 1992
 *		Add UNIX-conditional include of IO stuff; conditionalize-out
 *		'most everything for thread-is-VP case.
 *	004	Webb Scales	18 September 1992
 *		Temporarily (?) changed the special threads' sequence numbers
 *		from negative to big positive.
 *	005	Dave Butenhof	24 September 1992
 *		Change sequences back to negative -- header.sequence is now
 *		unsigned, so things should be OK.
 *	006	Dave Butenhof	 6 November 1992
 *		Undefer things (like sigwait) before going into io_available.
 *		There's still a hole where sigwait() can get lost, that needs
 *		to be addressed, but this will narrow it a little.
 *	007	Dave Butenhof	25 November 1992
 *		VP struct no longer has an 'interrupts' queue.
 *	008	Dave Butenhof	29 January 1993
 *		On DEC OSF/1 with realtime support, use the actual policy and
 *		priority (from the Mach thread) when setting up the default
 *		TCB.
 *	009	Dave Butenhof	25 March 1993
 *		Fix trace format strings to use "0x%lx" rather than "%08x".
 *	010	Dave Butenhof	12 April 1993
 *		Add argument to cma__int[_timed]_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	011	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	012	Dave Butenhof	24 May 1993
 *		Modify manager thread control -- transfer to it
 *		responsibility for undeferring CV operations, and wake it up
 *		by explicitly scheduling it rather than signalling a CV (so
 *		it can be done within the kernel).
 *	013	Dave Butenhof	28 May 1993
 *		Fix up state of manager thread before blocking (normally,
 *		cma__ready does that).
 *	014	Brian Keane	1 July 1993
 *		Minor touch-ups to eliminate warnings with DEC C on OpenVMS AXP.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_sched.h>
#include <cma_tcb.h>
#include <cma_queue.h>
#include <cma_timer.h>
#include <cma_thread.h>
#include <cma_condition.h>
#include <cma_vp.h>
#include <cma_vm.h>
#include <cma_dispatch.h>
#include <cma_deb_core.h>
#if _CMA_OS_ == _CMA__UNIX && !_CMA_THREAD_IS_VP_
# include <cma_thread_io.h>
#endif

/*
 * GLOBAL DATA
 */

/*
 * Variable priority scheduling parameters
 */
cma_t_integer	cma__g_prio_bg_min,
  		cma__g_prio_bg_max,
		cma__g_prio_fg_min,
  		cma__g_prio_fg_max,
  		cma__g_prio_m_0,
  		cma__g_prio_m_1,
  		cma__g_prio_m_2,
  		cma__g_prio_m_3,
  		cma__g_prio_b_0,
  		cma__g_prio_b_1,
  		cma__g_prio_b_2,
  		cma__g_prio_b_3,
  		cma__g_prio_p_1,
  		cma__g_prio_p_2,
  		cma__g_prio_p_3;

cma__t_queue	cma__g_hold_list;	/* Threads put on hold by debugger */
cma__t_queue	cma__g_run_vps;		/* Currently running processors */

cma__t_int_tcb		*cma__g_mgr_tcb;	/* Scheduling manager thread */
cma__t_atomic_bit	cma__g_mgr_sort_rq;	/* Request to sort ready queues */
cma__t_atomic_bit	cma__g_mgr_running;	/* Mgr thread running? */
cma__t_atomic_bit	cma__g_mgr_wake;	/* Defer mgr wakeup */

/*
 * LOCAL DATA
 */

static cma__t_int_tcb	*cma___g_null_tcb;	/* Null thread */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */

cma_t_address
cma___mgr_thread  _CMA_PROTOTYPE_ ((void));

cma_t_address
cma___null_thread  _CMA_PROTOTYPE_ ((void));

cma__t_vp *
cma___preempt_victim  _CMA_PROTOTYPE_ ((cma__t_int_tcb	*ptcb));

void
cma___sort_ready_list _CMA_PROTOTYPE_ ((void));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	initialize the scheduler
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
void
cma__init_sched
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_vp		    *def_vp;
    cma__t_int_tcb	    *def_tcb;
#if !_CMA_THREAD_IS_VP_
    static cma__t_string    null_thread = "null thread";
    static cma__t_string    mgr_thread = "manager thread";


    cma__tac_set (&cma__g_mgr_sort_rq);
    cma__tac_set (&cma__g_mgr_running);
#endif
    cma__queue_init (&(cma__g_hold_list));		

    cma__trace ((
	    cma__c_trc_init | cma__c_trc_sched,
	    "(init_dispatch) creating default VP"));
    def_vp = cma__alloc_object (cma__t_vp);

    if ((cma_t_address)def_vp == cma_c_null_ptr)
	cma__bugcheck ("init: can't allocate vp struc");

    def_tcb = &cma__g_def_tcb;
    def_vp->current_thread = &cma__g_def_tcb;
    def_vp->vp_id = cma__vp_get_id ();
    def_tcb->sched.processor = def_vp;
    def_tcb->state = cma__c_state_running;
#if _CMA_RT4_KTHREAD_
    def_tcb->sched.policy = def_vp->vp_id->policy;
    def_tcb->sched.priority = def_vp->vp_id->priority;
#endif

    cma__trace ((
	    cma__c_trc_init | cma__c_trc_sched,
	    "(init_dispatch) initializing VP queues"));
    cma__queue_init (&cma__g_run_vps);

    cma__queue_insert (&def_vp->queue, &cma__g_run_vps);

#if !_CMA_THREAD_IS_VP_
    /*
     * Force cma__ready to perform a priority queue calculation 
     */    
    def_tcb->timer.quanta_remaining = cma__c_quanta;

    def_tcb->sched.adj_time = -(cma__c_prio_interval + 1);

    def_tcb->sched.tot_time = 
	def_tcb->sched.priority *	cma__c_init_cpu_time;
    def_tcb->sched.time_stamp = 0;
    def_tcb->sched.cpu_time = cma__c_init_cpu_time;

    cma__g_prio_bg_min = cma__scale_up (cma_c_prio_back_min);	/* Temporary choice of inits */
    cma__g_prio_bg_max = cma__scale_up (cma_c_prio_back_max);
    cma__g_prio_fg_min = cma__scale_up (cma_c_prio_through_min);
    cma__g_prio_fg_max = cma__scale_up (cma_c_prio_through_max);

    cma__g_prio_m_0 = 4 * cma__scale_up (cma__scale_up (cma__c_prio_n_0))
	    / (cma__g_prio_fg_max - cma__g_prio_fg_min);
    cma__g_prio_m_1 = 4 * cma__scale_up (cma__scale_up (cma__c_prio_n_1))
	    / (cma__g_prio_fg_max - cma__g_prio_fg_min);
    cma__g_prio_m_2 = 4 * cma__scale_up (cma__scale_up (cma__c_prio_n_2))
	    / (cma__g_prio_fg_max - cma__g_prio_fg_min);
    cma__g_prio_m_3 = 4 * cma__scale_up (cma__scale_up (cma__c_prio_n_3))
	    / (cma__g_prio_fg_max - cma__g_prio_fg_min);

    cma__g_prio_b_0 = cma__c_prio_n_0 
	    * (0 * cma__g_prio_fg_min + 4 * cma__g_prio_fg_max)
	    / (cma__g_prio_fg_min - cma__g_prio_fg_max) + cma__c_prio_o_0;
    cma__g_prio_b_1 = cma__c_prio_n_1 
	    * (1 * cma__g_prio_fg_min + 3 * cma__g_prio_fg_max)
	    / (cma__g_prio_fg_min - cma__g_prio_fg_max) + cma__c_prio_o_1;
    cma__g_prio_b_2 = cma__c_prio_n_2 
	    * (2 * cma__g_prio_fg_min + 2 * cma__g_prio_fg_max)
	    / (cma__g_prio_fg_min - cma__g_prio_fg_max) + cma__c_prio_o_2;
    cma__g_prio_b_3 = cma__c_prio_n_3 
	    * (3 * cma__g_prio_fg_min + 1 * cma__g_prio_fg_max)
	    / (cma__g_prio_fg_min - cma__g_prio_fg_max) + cma__c_prio_o_3;

    cma__g_prio_p_1 = cma__g_prio_fg_min
	    + 1 * (cma__g_prio_fg_max - cma__g_prio_fg_min)/4;
    cma__g_prio_p_2 = cma__g_prio_fg_min
	    + 2 * (cma__g_prio_fg_max - cma__g_prio_fg_min)/4;
    cma__g_prio_p_3 = cma__g_prio_fg_min
	    + 3 * (cma__g_prio_fg_max - cma__g_prio_fg_min)/4;

    /*
     * Create scheduling manager thread
     */
    cma__g_known_threads.sequence = -2;	/* Special numbers for mgr & null */
    cma__trace ((
	    cma__c_trc_init | cma__c_trc_sched,
	    "(init_sched) creating manager thread tcb"));
    cma__g_mgr_tcb		= cma__get_tcb (&cma__g_def_attr);

    /*
     * Change from normal TCB defaults to those of the scheduling manager thread
     */
    cma__g_mgr_tcb->kind		= cma__c_thkind_mgr; 
    cma__g_mgr_tcb->sched.priority	= cma_c_prio_fifo_max + 1;
    cma__g_mgr_tcb->sched.policy	= cma_c_sched_fifo;
    cma__g_mgr_tcb->sched.rtb		= cma_c_true;
    cma__g_mgr_tcb->sched.spp		= cma_c_true;
    cma__g_mgr_tcb->sched.fixed_prio	= cma_c_true;
    cma__g_mgr_tcb->sched.class	= cma__c_class_rt;
    cma__obj_set_name (cma__g_mgr_tcb, mgr_thread);

    cma__trace ((
	    cma__c_trc_init | cma__c_trc_sched,
	    "(init_sched) manager thread tcb is 0x%lx; creating thread...",
	    cma__g_mgr_tcb));

    if (cma__int_make_thread (
	    cma__g_mgr_tcb,
	    (cma_t_thread *)cma_c_null_ptr,
	    (cma_t_start_routine)cma___mgr_thread,
	    cma_c_null_ptr) != cma_s_normal)
	cma__bugcheck ("init: can't make manager thread");

    /*
     * Create null thread
     */
    cma__trace ((
	    cma__c_trc_init | cma__c_trc_sched,
	    "(init_sched) creating null thread tcb"));
    cma___g_null_tcb		= cma__get_tcb (&cma__g_def_attr);

    /*
     * Change from normal TCB defaults to those of a null thread
     */
    cma___g_null_tcb->kind		= cma__c_thkind_null; 
    cma___g_null_tcb->sched.priority	= cma_c_prio_idle_min;
    cma___g_null_tcb->sched.policy	= cma_c_sched_idle;
    cma___g_null_tcb->sched.rtb		= cma_c_true;
    cma___g_null_tcb->sched.spp		= cma_c_false;
    cma___g_null_tcb->sched.fixed_prio	= cma_c_true;
    cma___g_null_tcb->sched.class	= cma__c_class_idle;
    cma__obj_set_name (cma___g_null_tcb, null_thread);

    cma__trace ((
	    cma__c_trc_init | cma__c_trc_sched,
	    "(init_sched) null thread tcb is 0x%lx; creating thread...",
	    cma___g_null_tcb));

    if (cma__int_make_thread (
	    cma___g_null_tcb,
	    (cma_t_thread *)cma_c_null_ptr,
	    (cma_t_start_routine)cma___null_thread,
	    cma_c_null_ptr) != cma_s_normal)
	cma__bugcheck ("init: can't make null thread");
#endif

    cma__g_known_threads.sequence = 2;	/* First user thread is #2 */
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Performs pre- or post- `fork() reinitialization' work.
 *
 *  FORMAL PARAMETERS:
 *
 *	flag
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
void
cma__reinit_sched
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
    {
    cma_t_integer   i;

    if (flag == cma__c_reinit_postfork_clear) {	/* Post-fork child handling */
#if 0
	cma__queue_init (&cma__g_run_vps);
	cma__queue_insert (&def_vp->queue, &cma__g_run_vps);
#endif
	cma__queue_init (&cma__g_hold_list);		
#if !_CMA_THREAD_IS_VP_
	cma__g_threadcnt += 2;		/* Count the null & manager threads */
	cma__queue_zero (&(cma___g_null_tcb->header.queue));
	cma__ready (cma___g_null_tcb, cma_c_false);
	cma__queue_zero (&(cma__g_mgr_tcb->header.queue));
	cma__ready (cma__g_mgr_tcb, cma_c_false);
#endif
	}
    }



#if !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Try to find a processor for just readied thread
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb	- Address of the tcb
 *
 *  IMPLICIT INPUTS:
 *
 *	cma__g_ready_list[]	    - CMA ready lists
 *
 *  IMPLICIT OUTPUTS:
 *
 *	cma__g_ready_list[]	    - CMA ready lists
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 * 	This routine will add the thread tcb at the end of the
 * 	cma__g_ready_list.
 */
void
cma__try_run 
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*tcb)
#else	/* no prototypes */
	(tcb)
	cma__t_int_tcb	*tcb;
#endif	/* prototype */
    {
    cma__t_vp	    *vp;		/* Pointer to processor */

    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "In cma__try_run with kernel unlocked");

    /*
     * Since we readied a thread, let's see if we can wake up
     * some suspended vp.
     */
# if _CMA_MULTIPLEX_
    if (cma__vp_create (&vp->vp_id) != cma__c_vp_normal)
# endif					/* _CMA_MULTIPLEX_ */
	if (vp = cma___preempt_victim (tcb)) {
	    cma__t_int_tcb *cur_tcb;


	    cur_tcb = cma__get_self_tcb ();

	    if (vp == cur_tcb->sched.processor) {
		cma__undefer ();
		cma__yield_processor ();
		}
	    else
# if _CMA_MULTIPLEX_
		/*
		 * FIX-ME: This won't compile, since the
		 * cma__cause_force_dispatch function needs additional
		 * arguments.  Will need to be redesigned when multiprocessor
		 * multiplexing is implemented.
		 */
		cma__vp_interrupt (vp, cma__cause_force_dispatch, tcb);
# else
		cma__bugcheck ("try_run: spurious vp");
# endif

	    }

    }
	
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Manager thread function (declared to match cma_t_start_routine)
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma_t_address
cma___mgr_thread
# ifdef _CMA_PROTO_
	(void)
# else	/* no prototypes */
	()
# endif	/* prototype */
    {

    while (cma_c_true) {

	TRY {

	    while (cma_c_true) {
		cma__trace ((
			cma__c_trc_sched | cma__c_trc_manager,
			"(mgr_thread) top of loop"));

		cma__trace ((
			cma__c_trc_sched | cma__c_trc_manager,
			"(mgr_thread) about to wait"));

		cma__set_kernel ();	/* Enter kernel */

		/*
		 * Now that we have the kernel locked, check whether there
		 * are more CV operations to defer, or if someone needs the
		 * manager thread to wake up. If so, loop again without
		 * dispatching. Note that we can't use the normal undefer
		 * macro because it readies the manager thread and yields to
		 * it -- the yield would simply return, and the manager
		 * thread would dispatch to somewhere else with the mgr_wake
		 * flag reset.
		 */
		if (cma__kernel_set (&cma__g_mgr_wake)) {

		    /*
		     * NOTE: The manager thread does NOT live on any
		     * scheduling queue when not running. Nor does it ever
		     * block. Instead, it is explicitly "readied" when
		     * needed, and simply dispatches to another thread
		     * (without readying itself on some queue) when done with
		     * a loop.
		     */
		    cma__unset (&cma__g_mgr_running);	/* Not running */
		    cma__g_mgr_tcb->state = cma__c_state_blocked;
		    cma__dispatch (cma__g_mgr_tcb, cma_c_true, 0);
		    }

		cma__exit_kernel ();

		if (!cma__test_and_set (&cma__g_mgr_sort_rq)) {
		    cma__trace ((
			    cma__c_trc_sched | cma__c_trc_manager, 
			    "(mgr_thread) sorting ready list"));
		    cma__enter_kernel ();
		    cma___sort_ready_list();
		    cma__exit_kernel ();
		    }

		cma__trace ((
			cma__c_trc_sched | cma__c_trc_manager,
			"(mgr_thread) checking timer queue"));
		cma__enter_kernel ();
		(void)cma__check_timer_queue (
			(cma_t_date_time *)cma_c_null_ptr);
		cma__exit_kernel ();

		if (!cma__test_and_set (&cma__g_cv_defer)) {
		    cma__trace ((
			    cma__c_trc_sched | cma__c_trc_manager,
			    "(mgr_thread) undefering CV ops"));
		    cma__enter_kernel ();
		    cma__undefer_cv ();
		    cma__exit_kernel ();
		    }

		}

	    }
	CATCH_ALL {			/* Catch and drop all exceptions */
#ifndef NDEBUG
	    cma__assert_warn (0, "Exception in Manager thread.");
	    exc_report (THIS_CATCH);
#endif
	    }
	ENDTRY

	}

    return cma_c_null_ptr;

    }
	
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Null thread function (declared to match cma_t_start_routine)
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma_t_address
cma___null_thread
# ifdef _CMA_PROTO_
	(void)
# else	/* no prototypes */
	()
# endif	/* prototype */
    {
    while (cma_c_true) {
	TRY {
# if _CMA_OS_ == _CMA__VMS
	    while (cma_c_true) {
		extern sys$hiber _CMA_PROTOTYPE_ ((void));
		cma__trace ((cma__c_trc_null, "(null_thread) hibernating"));
		sys$hiber ();
		cma__trace ((cma__c_trc_null, "(null_thread) yielding"));
		cma_yield ();		/* See if other threads need to run */
		}
# endif
# if _CMA_OS_ == _CMA__UNIX
	    cma__enter_kernel ();

	    while (cma_c_true) {
		cma_t_date_time		delta;
		struct timeval		wait_time, *wait_ptr;


		/*
		 * Check the timer queue. Ready any threads whose entries have
		 * expired.  Determine if the queue is now empty, and get the
		 * delta time to the next expiration, if it is not.
		 *
		 * If the timer queue is now empty, set the wait pointer to 
		 * null indicating an infinite timeout.
		 */
		if (!cma__check_timer_queue (&delta)) {
#  if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
		    /*
		     * select() on the RS/6000 returns EINVAL when the
		     * timeout is NULL and no file descriptors are selected,
		     * as often happens with sigwait(). So we set up a wait
		     * time of approx. 68 years to simulate the infinite wait
		     * implied by wait_ptr = NULL
		     */
		    wait_time.tv_sec = 0x7FFFFFFF;
		    wait_time.tv_usec = 0;
		    wait_ptr = &wait_time;
#  else
		    wait_ptr = (struct timeval *)cma_c_null_ptr;
#  endif
		    }
		else {
		    wait_time.tv_sec = delta.tv_sec;
		    wait_time.tv_usec = delta.tv_usec;
		    wait_ptr = &wait_time;
		    }

		/* 
		 * Block at a select(2) call until a timer queue entry expires.
		 * (The first two parameters are arbitrary.)
		 */
		cma__undefer ();
		(void)cma__io_available (cma__c_io_read, 0, wait_ptr);

		/*
		 * Since there is now a file ready for I/O, or a timer queue
		 * entry ready to expire, yield the processor to let things
		 * happen.
		 */
		cma__undefer ();
		cma__yield_processor ();
	        }

#  if 0
	    /*
	     * The following statement ought to be here, but needn't be, as
	     * it's never executed due to the infinite loop. Its ghost is
	     * left here to avoid confusing anyone...
	     */
	    cma__exit_kernel ();
#  endif
# endif
	    }
	CATCH_ALL {			/* Catch and drop all exceptions */
# if _CMA_OS_ == _CMA__UNIX
	    cma__exit_kernel ();
# endif
# ifndef NDEBUG
	    cma__assert_warn (0, "Exception in null thread.");
	    exc_report (THIS_CATCH);
# endif
	    }
	ENDTRY
	}

    return cma_c_null_ptr;
    }



/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Search for a preemption victim.
 *
 *  FORMAL PARAMETERS:
 *
 *	ptcb	    - Address of the tcb requesting the preempt
 *
 *  IMPLICIT INPUTS:
 *
 *	cma__g_run_vps	    - Preemptable processors
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	Address of victim's VP data structure or null for no victim.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
cma__t_vp *
cma___preempt_victim
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*ptcb)
#else	/* no prototypes */
	(ptcb)
	cma__t_int_tcb	*ptcb;
#endif	/* prototype */
    {
    cma__t_queue	*vp;
    cma__t_int_tcb	*ttcb, 
			*vtcb = (cma__t_int_tcb *)cma_c_null_ptr;
    cma_t_boolean	victim;


    vp = cma__queue_next (&cma__g_run_vps);
    while (vp != &cma__g_run_vps) {
	ttcb = ((cma__t_vp *)vp)->current_thread;

	if (ptcb->sched.spp)
	    switch (ptcb->sched.class) {
		case cma__c_class_rt:
		    {
		    victim = (ttcb->sched.class == cma__c_class_rt
			    ? (cma__sched_prio_rt(ptcb) > cma__sched_prio_rt(ttcb))
			    : cma_c_true);
		    break;
		    }
		case cma__c_class_fore:
		    {
		    switch (ttcb->sched.class) {
			case cma__c_class_rt:
			    {
			    victim = cma_c_false;
			    break;
			    }
			case cma__c_class_fore:
			    {
			    victim = (cma__sched_prio_fore(ptcb)
				    > cma__sched_prio_fore(ttcb));
			    break;
			    }
			case cma__c_class_back:
			case cma__c_class_idle:
			    {
			    victim = cma_c_true;
			    break;
			    }
			default:
			    {
			    cma__bugcheck (
				    "preempt_victim1: bad preempt policy %d",
				    ttcb->sched.class);
			    break;
			    }
			}

		    break;
		    }
		case cma__c_class_back:
		    {
		    switch (ttcb->sched.class) {
			case cma__c_class_rt:
			case cma__c_class_fore:
			    {
			    victim = cma_c_false;
			    break;
			    }
			case cma__c_class_back:
			    {
			    victim = (cma__sched_priority(ptcb)
				    > cma__sched_priority(ttcb));
			    break;
			    }
			case cma__c_class_idle:
			    {
			    victim = cma_c_true;
			    break;
			    }
			default:
			    {
			    cma__bugcheck (
				    "preempt_victim2: bad target policy %d",
				    ttcb->sched.class);
			    break;
			    }
			}

		    break;
		    }
		/*
		 * Idle threads should never be the cause of a preemption.
		 */
		default:
		    {
		    cma__bugcheck ("preempt_victim3: idle thread caused preemption");
		    break;
		    }
		}
	else			    
	    /* 
	     * A non-preemptive scheduling policy thread preempts 
	     * only the Null Thread.
	     */
	    victim = (ttcb->sched.class == cma__c_class_idle);


	if (victim)
	    if (!vtcb)
	        vtcb = ttcb;
	    else
		switch (ttcb->sched.class) {
		    case cma__c_class_rt:
			{
			if (vtcb->sched.class == cma__c_class_rt)
			    if (cma__sched_prio_rt(ttcb) 
				    < cma__sched_prio_rt(vtcb))
			        vtcb = ttcb;

			break;
			}
		    case cma__c_class_fore:
			{
			switch (vtcb->sched.class) {
			    case cma__c_class_rt:
				{
				vtcb = ttcb;
				break;
				}
			    case cma__c_class_fore:
				{
				if (cma__sched_prio_fore(ttcb)
					< cma__sched_prio_fore(vtcb))
				    vtcb = ttcb;
				break;
				}
			    case cma__c_class_back:
			    case cma__c_class_idle:
				{
				break;
				}
			    default:
				{
				cma__bugcheck (
					"preempt_victim4: bad victim class %d",
					vtcb->sched.class);
				break;
				}
			    }

			break;
	        	}
		    case cma__c_class_back:
			{
			switch (vtcb->sched.class) {
			    case cma__c_class_rt:
			    case cma__c_class_fore:
				{
				vtcb = ttcb;
				break;
				}
			    case cma__c_class_back:
				{
		        	if (cma__sched_priority(ttcb)
					< cma__sched_priority(vtcb))
				    vtcb = ttcb;

				break;
				}
			    case cma__c_class_idle:
				{
				break;
				}
			    default:
				{
				cma__bugcheck (
					"preempt_victim5: bad victim class %d",
					vtcb->sched.class);
				break;
				}
			    }

			break;
		        }
		    case cma__c_class_idle:
			{
			vtcb = ttcb;
			break;
		        }
		    default:
			{
			cma__bugcheck (
				"preempt_victim6: bad sched class %d",
				ttcb->sched.class);
			break;
			}
		    }

	vp = cma__queue_next (vp);
	}

    return (vtcb ? vtcb->sched.processor : (cma__t_vp *)cma_c_null_ptr);
    }



/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___sort_ready_list is used to remove all inconsistencies
 *	in the dispatcher database.  Inconsistencies can arrive from
 *	two sources: the debugger has changed a thread's priority or
 *	even policy, or the scheduler has modified priorities without
 *	moving the thread.
 *
 *	While scanning the queues, this function also looks for a "next thread"
 *	that the debugger may have requested.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	none
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
void
cma___sort_ready_list
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer	i;
    cma__t_queue	temp_ready_list[cma__c_prio_n_tot];
    cma__t_queue	temp_hold_list;
    cma__t_queue	temp_next_thread;
    cma_t_address	*qtmp;
    cma__t_int_tcb	*tcb;

    cma__assert_fail(
	    cma__tac_isset (&cma__g_kernel_critical),
	    "In cma___sort_ready_list with kernel unlocked");

    /*
     * Move the ready and hold lists to temp-ready, temp-hold (by the hack
     * of inserting the new queue head on the list, then removing the
     * original queue head and re-initializing it to "empty").
     */
    for (i = 0; i < cma__c_prio_n_tot; i++) {
	cma__queue_zero (&temp_ready_list[i]);
	cma__queue_insert_after (&temp_ready_list[i], &cma__g_ready_list[i]);
	cma__queue_remove (&cma__g_ready_list[i], qtmp, cma_t_address);
	cma__queue_init (&cma__g_ready_list[i]);
	}

    cma__queue_zero (&temp_hold_list);
    cma__queue_insert_after (&temp_hold_list,&cma__g_hold_list);
    cma__queue_remove (&cma__g_hold_list, qtmp, cma_t_address);
    cma__queue_init (&cma__g_hold_list);

    /*
     * Init the queue to hold the next_thread
     */
    cma__queue_init (&temp_next_thread);

    /*
     * Scan the temp hold list: ready thread unless debugger wants it run
     */
    while (!cma__queue_empty (&temp_hold_list)) {
	cma__queue_dequeue (&temp_hold_list, tcb, cma__t_int_tcb);

	if (tcb == cma__g_debug_state.next_to_run) {
	    cma__g_debug_state.next_to_run = (cma__t_int_tcb *)cma_c_null_ptr;
	    cma__queue_insert(&(tcb->header.queue), &temp_next_thread);
	    }
	else
	    cma__ready (tcb, cma_c_false);

	}

    /*
     * Scan the ready hold list: ready thread unless debugger wants it run
     */
    for (i = 0; i < cma__c_prio_n_tot; i++) {

	while (!cma__queue_empty (&temp_ready_list[i])) {
	    cma__queue_dequeue (&temp_ready_list[i], tcb, cma__t_int_tcb);

	    if (tcb == cma__g_debug_state.next_to_run) {
		cma__g_debug_state.next_to_run =
		    (cma__t_int_tcb *)cma_c_null_ptr;
		cma__queue_insert(&(tcb->header.queue), &temp_next_thread);
		}
	    else
		cma__ready (tcb, cma_c_false);

	    }

	}
	
    /*
     * Finally, insert the debugger-requested thread at the very top of ready.
     */
    if (!cma__queue_empty (&temp_next_thread)) {
	cma__queue_dequeue (&temp_next_thread, tcb, cma__t_int_tcb);
	cma__g_next_ready_queue	= cma__c_prio_o_rt;
	cma__g_debug_state.events_enabled   = cma_c_true;
	tcb->debug.notify_debugger = cma_c_true;
	cma__queue_insert(
	    &(tcb->header.queue),
	    &cma__g_ready_list[cma__g_next_ready_queue]);
	}

    }
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SCHED.C */
/*  *14    2-JUL-1993 09:43:22 KEANE "Fix DEC C warnings: add return statements to non-void routines" */
/*  *13   28-MAY-1993 12:17:53 BUTENHOF "Mgr thread needs to set state on block" */
/*  *12   27-MAY-1993 14:32:30 BUTENHOF "Teach mgr thd to undefer CVs" */
/*  *11   16-APR-1993 13:05:13 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *10   25-MAR-1993 14:43:37 BUTENHOF "Fix trace hexes" */
/*  *9    23-FEB-1993 17:19:58 SCALES "Fix up preemption" */
/*  *8    29-JAN-1993 12:46:26 BUTENHOF "Set default vp/thread to actual Mach sched. state" */
/*  *7     1-DEC-1992 14:05:46 BUTENHOF "Remove interrupts queue" */
/*  *6     6-NOV-1992 12:50:39 BUTENHOF "Undefer before io_avail in null thread" */
/*  *5    28-SEP-1992 11:49:16 BUTENHOF "Fix mgr/null sequence numbers" */
/*  *4    18-SEP-1992 21:32:14 SCALES "change special thread sequence numbers" */
/*  *3    15-SEP-1992 22:42:56 SCALES "Make safe to build for kernel threads" */
/*  *2    15-SEP-1992 13:50:16 BUTENHOF "Add name to thread manager CV" */
/*  *1     4-SEP-1992 15:55:45 SCALES "Scheduler code" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SCHED.C */
