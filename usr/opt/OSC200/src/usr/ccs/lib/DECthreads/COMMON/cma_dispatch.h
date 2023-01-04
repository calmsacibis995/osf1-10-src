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
 * @(#)$RCSfile: cma_dispatch.h,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:47:13 $
 */
/*
 * FACILITY:
 *
 *	CMA services
 *
 * ABSTRACT:
 *
 *	This module defines the dispaching of the threads.
 *
 * AUTHORS:
 *
 *	Hans Oser
 *
 * CREATION DATE:
 *
 *	23 August 1989
 *
 * MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof & Webb Scales	27 September 1989
 *		Fix spelling error (deferral instead of deferal)
 *	002	Hans Oser			03 October 1989
 *		Function cma__get_asynch_info added, cma__dispatch
 *		has now a parameter specifing the mode.
 *	003	Hans Oser and Webb Scales	03 October 1989
 *		Add ctx_buffer parameter to cma__get_async_info
 *	004	Hans Oser			10 October 1989
 *		Move Timer support to cma_timer; make
 *		cma__get_current_tcb available to others.
 *	005	Hans Oser			13 October 1989
 *		changements due to program review.
 *	006	Webb Scales			17 October 1989
 *		Corrected case of #include filenames
 *	007	Dave Butenhof & Webb Scales	19 October 1989
 *		Remove extern defs for test_and_set and unset (which have
 *		moved to cma_host_*.h).
 *	008	Webb Scales			20 October 1989
 *		Placed definition (ie, type and initializer) of 
 *		cma__g_kernel_critical in cma_dispatch.c and placed 
 *		the declaration (ie, "extern") in this file.
 *	009	Dave Butenhof			Halloween 1989
 *		Define queue of known threads.
 *	010	Dave Butenhof			All Saints Day 1989
 *		Convert cma__enter_kernel to macro.
 *	011	Dave Butenhof/Webb Scales/Bob Conti	3 November 1989
 *		Change exit_kernel macro to call undefer before releasing
 *		lock.
 *	012	All				3 November 1989
 *		Redesign deferral.
 *	013	Webb Scales			3 November 1989
 *		Add assertion to exit_kernel
 *	014	Bob Conti			5 November 1989
 *		Delete time argument from cma__assign
 *		Add flag defers_processed to avoid useless calls to undefer.
 *		Change cma__undefer to return an integer so it can be
 *		used in conditionals in a macro where a value is expected.
 *	015	Webb Scales & Bob Conti		6 November 1989
 *		Made cma__ready into a macro and put the quatum increment into
 *		it.
 *	016	Webb Scales	7 November 1989
 *		Changed name of cma__AST_force_dispatch to 
 *		cma__cause_force_dispatch.
 *	017	Webb Scales	18 November 1989
 *		Changed interface to cma__get_async_info
 *	018	Webb Scales	19 November 1989
 *		Corrected proto for cma__cause_force_dispatch
 *	019	Dave Butenhof	4 December 1989
 *		Include cma_tcb_defs.h instead of cma_tcb.h
 *	020	Webb Scales	10 December 1989
 *		Add deferal action for Unix non-blocking I/O.
 *	021	Dave Butenhof	2 March 1990
 *		Integrate Kevin Ackley's changes for Apollo M68000 port.
 *	022	Dave Butenhof	9 April 1990
 *		Remove definition of known thread queue header; it's now
 *		in cma_debugger.
 *	023	Webb Scales	15 June 1990
 *		Added priority scheduling:
 *		- Changed cma__ready from a macro to a routine.
 *		- Made the ready list an array of queue headers.
 *		- Added global "next ready queue" pointer.
 *		- Added defer's for tick updates.
 *	024	Webb Scales	25 July 1990
 *		Moved defer functions to their own module.
 *	025	Dave Butenhof	14 August 1990
 *		Generalize cma__cause_force_dispatch so it can be used for
 *		asynchronous alerts as well as timeslicing.
 *	026	Webb Scales	15 August 1990
 *		Cleaned up some, accomodating HPPA & Sun-68K platforms
 *	027	Dave Butenhof	04 April 1991
 *		Add convenient macros for kernel threads.
 *	028	Paul Curtin	31 May 1991
 *		Added a prototype for a reinit routine
 *	029	Dave Butenhof	31 May 1991
 *		Add timeout argument to cma__block and cma__dispatch (only
 *		used for _CMA_THREAD_IS_VP_).
 *	030	Dave Butenhof	07 October 1991
 *		Add cma__g_threadcnt.
 *	031	Paul Curtin	18 November 1991
 *		Added the use of an Alpha switch.
 *	032	Webb Scales	13 March 1992
 *		Moved VP definitions to cma_vp_defs.h
 *	033	Dave Butenhof	11 August 1992
 *		Move cma__try_run macro from cma_vp_defs.h to here.
 *	034	Dave Butenhof	28 August 1992
 *		Change cma__block into several macros for performance and
 * 		flexibility.
 *	035	Webb Scales	 3 September 1992
 *		Moved most of the scheduling (as opposed to dispatching) code
 *		to cma_sched.c.
 *	036	Dave Butenhof	30 September 1992
 *		Since some functions previously in dispatch were moved to
 *		sched, dispatch.h ought to include sched.h to avoid
 *		perturbing other modules.
 *	037	Dave Butenhof	25 March 1993
 *		cma__try_run prototype doesn't belong here -- it's already in
 *		cma_sched.h.
 *	038	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	039	Dave Butenhof	30 April 1993
 *		Zombie queue is now extern.
 *	040	Dave Butenhof	 3 May 1993
 *		Remove zombie list.
 */


#ifndef CMA_DISPATCH
#define CMA_DISPATCH

/*
 * INCLUDE FILES
 */

#include <cma.h>
#include <cma_queue.h>
#include <cma_tcb_defs.h>
#include <cma_sched.h>

/*
 * CONSTANTS AND MACROS
 */

#if _CMA_THREAD_IS_VP_
# if _CMA_DEBEVT_
#  define cma__prepare_block(_tcb_) { \
    (_tcb_)->state = cma__c_state_blocked; \
    if (cma__g_debug_state.events_enabled) \
	cma__debevt_report (cma__c_debevt_blocking); }
# else
#  define cma__prepare_block(_tcb_) (_tcb_)->state = cma__c_state_blocked
# endif
# define cma__finish_block(_tcb_,_status_) \
    if (!(_status_)) {cma_t_address qtmp; \
    cma__queue_remove (&(_tcb_)->header.queue, qtmp, cma_t_address);}
# define cma__ready(_tcb_,dummy) { \
    (_tcb_)->state = cma__c_state_running; \
    if (cma__vp_resume ((_tcb_)->sched.processor->vp_id) != cma__c_vp_normal) \
	cma__bugcheck ("ready: vp_resume thread %d", (_tcb_)->header.sequence); }
#else
# if _CMA_DEBEVT_
#  define cma__prepare_block(_tcb_) { \
    cma__assert_fail (cma__tac_isset (&cma__g_kernel_critical),"Blocking with kernel unlocked"); \
    if ((_tcb_)->timer.quanta_remaining > 0) (_tcb_)->timer.quanta_remaining = 0; \
    (_tcb_)->state = cma__c_state_blocked; \
    if (cma__g_debug_state.events_enabled) \
	cma__debevt_report (cma__c_debevt_blocking); }
# else
#  define cma__prepare_block(_tcb_) { \
    cma__assert_fail (cma__tac_isset (&cma__g_kernel_critical),"Blocking with kernel unlocked"); \
    if ((_tcb_)->timer.quanta_remaining > 0) (_tcb_)->timer.quanta_remaining = 0; \
    (_tcb_)->state = cma__c_state_blocked; }
# endif
# define cma__finish_block(_tcb_,_status_)
#endif

/*
 * If the thread has a realtime policy, sort it into the beginning of the
 * block queue. Otherwise, just insert it at the end.
 *
 * FIX-ME: This doesn't currently sort ADA_LOW threads, which probably
 * violates their definition as "low priority realtime". I won't put them at
 * the front, because I don't want them to have priority over throughput
 * threads. I won't sort them at the end (which would be the "right"
 * solution) because that would mean locating the right place to put
 * throughput and background threads in the middle, and I don't want them to
 * pay the overhead (this could be solved by expanding the block queue to
 * have a start-of-ada_low pointer, but I don't think it's worth the effort
 * right now).
 */
#if _CMA_POSIX_SCHED_
# define cma__insert_by_sched(__tcb__,__q__) \
    if ((__tcb__)->sched.class == cma__c_class_rt) { \
	cma__t_queue	*__p__ = cma__queue_next ((__q__)); \
	int		__pr__ = (__tcb__)->sched.priority; \
	while (__p__ != (__q__)) { \
	    cma__t_int_tcb	*__t__ = (cma__t_int_tcb *)__p__; \
	    cma__t_sched_class	__sc__ = __t__->sched.class; \
	    int			__sp__ = __t__->sched.priority; \
	    if ((__sc__ != cma__c_class_rt) || (__sp__ < __pr__)) break; \
	    __p__ = cma__queue_next (__p__); } \
	cma__queue_insert (&(__tcb__)->header.queue, __p__); } \
    else cma__queue_insert (&(__tcb__)->header.queue, (__q__))
#else
# define cma__insert_by_sched(__tcb__,__q__) \
    cma__queue_insert (&(__tcb__)->header.queue, (__q__))
#endif

/*
 * TYPEDEFS
 */

/*
 * GLOBAL DATA
 */

extern cma__t_queue	cma__g_ready_list[cma__c_prio_n_tot];
extern cma_t_integer	cma__g_next_ready_queue; /* Next queue to check */
extern cma_t_integer	cma__g_threadcnt;	/* Number of threads */

/*
 * INTERNAL INTERFACES
 */

/*
 * start a forced dispatch from a signal or AST routine
 */
extern void
cma__cause_force_dispatch
#if (_CMA_OS_ == _CMA__UNIX) && (_CMA_HARDWARE_ != _CMA__VAX) && (_CMA_HARDWARE_ != _CMA__ALPHA)
	_CMA_PROTOTYPE_ ((
	cma__t_int_tcb	    *cur_tcb,
	cma_t_address	    target_pc,
	struct sigcontext   *scp));
#endif
#if (_CMA_HARDWARE_ == _CMA__VAX) || (_CMA_HARDWARE_ == _CMA__ALPHA)
	_CMA_PROTOTYPE_ ((
	cma__t_int_tcb	    *cur_tcb,
	cma_t_address	    target_pc,
	cma_t_address	    *saved_pc,
	cma_t_address	    *saved_ps));
#endif

/*
 * The next ready thread will be made running
 *
 * (save_context is set to cma_c_false for unscheduling dead threads.)
 */
extern cma_t_boolean
cma__dispatch _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb,
	cma_t_boolean	save_context,
	cma_t_integer	milliseconds));

/*
 * Dispell zombie threads (threads which have terminated but have not yet
 * been freed).
 */
extern void
cma__dispell_zombies _CMA_PROTOTYPE_ ((void));

/*
 * Get information saved during a timeslice AST or signal
 */
extern cma__t_async_ctx *
cma__get_async_info _CMA_PROTOTYPE_ ((void));

/*
 * Initialize the dispatcher: the initial executing process becomes a thread.
 */
extern void
cma__init_dispatch _CMA_PROTOTYPE_ ((void));

#if !_CMA_THREAD_IS_VP_
/*
 * Ready the thread: added it to the tail of the appropriate ready list.
 */
extern void
cma__ready _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*tcb,
	cma_t_boolean	preempt));
#endif

/*
 * Perform pre and/or post fork() reinitialization work.
 */
extern void
cma__reinit_dispatch _CMA_PROTOTYPE_ ((
	cma_t_integer	flag));

/*
 * Start the thread: the thread is added at the tail of the ready list.
 * The tcb is initialized.
 */
extern void
cma__start_thread _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*thread_tcb));

/*
 * Set thread to idle status (= non dispatchable)
 */
extern void
cma__terminate_thread _CMA_PROTOTYPE_ ((
	cma__t_int_tcb	*thread_tcb));

/*
 * The current thread is put at the tail of the ready list and the topmost
 * thread of the ready list is resumed. 
 */ 
extern void
cma__yield_processor _CMA_PROTOTYPE_ ((void));

#endif

/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DISPATCH.H */
/*  *22    2-JUL-1993 14:37:49 BUTENHOF "Don't dequeue on finish_block" */
/*  *21    3-MAY-1993 13:44:27 BUTENHOF "Merge zombies with known threads" */
/*  *20   30-APR-1993 18:14:05 BUTENHOF "Add zombie queue" */
/*  *19   16-APR-1993 13:03:42 BUTENHOF "Modify queue remove & dequeue" */
/*  *18   25-MAR-1993 14:43:34 BUTENHOF "Remove try_run prototype (it's in cma_sched.h)" */
/*  *17   30-SEP-1992 15:20:16 BUTENHOF "Include sched.h" */
/*  *16    4-SEP-1992 15:53:12 SCALES "Separate into two modules" */
/*  *15    2-SEP-1992 16:25:09 BUTENHOF "Separate semaphores from kernel lock" */
/*  *14   11-AUG-1992 13:15:57 BUTENHOF "Nullify cma__ready on kernel thread" */
/*  *13   31-MAR-1992 13:30:55 BUTENHOF "Alpha OSF" */
/*  *12   18-MAR-1992 19:01:22 SCALES "Move VP stuff to cma_vp_defs.h" */
/*  *11   18-NOV-1991 11:00:19 CURTIN "Added the use of an Alpha switch" */
/*  *10   14-OCT-1991 13:38:39 BUTENHOF "Add thread count, clean up uniproc/OSF conds" */
/*  *9    10-JUN-1991 19:52:54 SCALES "Convert to stream format for ULTRIX build" */
/*  *8    10-JUN-1991 19:20:38 BUTENHOF "Fix the sccs headers" */
/*  *7    10-JUN-1991 18:21:36 SCALES "Add sccs headers for Ultrix" */
/*  *6     2-JUN-1991 19:36:34 BUTENHOF "Add arguments" */
/*  *5    31-MAY-1991 14:12:12 CURTIN "Added a proto for a new reinit routine" */
/*  *4    14-MAY-1991 13:38:53 BUTENHOF "Fix try_run declaration" */
/*  *3     3-MAY-1991 11:25:57 BUTENHOF "Fix try_run declaration" */
/*  *2    12-APR-1991 23:35:39 BUTENHOF "Implement VP layer to support Mach threads" */
/*  *1    12-DEC-1990 21:44:57 BUTENHOF "Thread dispatcher" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DISPATCH.H */
