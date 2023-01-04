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
 *	@(#)$RCSfile: cma_tcb_defs.h,v $ $Revision: 4.2.5.2 $ (DEC) $Date: 1993/05/25 19:59:48 $
 */
/*
 *  FACILITY:
 *
 *	Concert Multithread Architecture (CMA) services
 *
 *  ABSTRACT:
 *
 *	TCB-related type definitions.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	4 December 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	5 December 1989
 *		Remove initial_test attribute.
 *	002	Dave Butenhof	8 December 1989
 *		Add pointer to base of thread's exception stack.
 *	003	Webb Scales	11 January 1990
 *		Renamed the select field from nfds to nfound, as these names
 *		each have particular connotations, and the wrong one was
 * 		used.
 *	004	Dave Butenhof	12 February 1990
 *		Add field for per-thread errno value.
 *	005	Webb Scales	30 May 1990
 *		Add reference count of joiners to prevent zombies from being
 *              dispelled prematurely.
 *	006	Webb Scales	15 June 1990
 *		Added priority scheduling:  Created new tcb substructure for 
 *		scheduling and moved cpu_time, policy, and priority into it.
 *	007	Webb Scales	16 August 1990
 *		Replaced #ifdef's with #if's
 *	008	Bob Conti	9 September 1990
 *		Add debug context and standard TCB prolog so that all
 *		task control blocks (ours and clients as well) can be 
 *		identified while the debugger is active.  Also added, 
 *		for Webb, a condition variable tswait_cv to be used for all
 *		thread-synchronous waits (aka self-waits) done by the thread.
 *		New include files have been added so that more types can be 
 *		stored in the TCB; all of cma.h is now available.
 *	009	Webb Scales	13 September 1990
 *		Changed tswait to internal format.
 *	010	Webb Scales	20 September 1990
 *		Added a mutex for thread-synchronous wait CV.
 *	011	Webb Scales	25 September 1990
 *		Added a separate queue node for select.
 *	012	Bob Conti	28 September 1990
 *		Add cast to fix build bug.
 *	013	Paul Curtin	11 December 1990
 *		Added sigaction_data array to tcb
 *	014	Dave Butenhof	5 February 1991
 *		Rename "errno" field.
 *	015	Dave Butenhof	7 February 1991
 *		Redefine alert state fields.
 *	016	Webb Scales	18 March 1991
 *		Added sched.q_num to improve variable priority scheduling.
 *	017	Dave Butenhof	25 March 1991
 *		Change exception interface names
 *	018	Dave Butenhof	05 June 1991
 *		Integrate IBM changes
 *	019	Dave Butenhof and Webb Scales	05 June 1991
 *		Conditionalize vacuous (forward) structure defs, since MIPS C
 *		V2.1 doesn't like (or, apparently, need).
 *	020	Paul Curtin	05 June 1991
 *		Remove include of exc_handling.h, gets rid of a 
 *		circularity problem.  We get it from cma.h any way.
 *	021	Webb Scales and Dave Butenhof	    10 June 1991
 *		Conditionalize inclusion of I/O stuff.
 *	022	Dave Butenhof	01 July 1991
 *		Add thread start function and argument to TCB so that I can
 *		make cma__thread_base loop rather than creating new threads.
 *	023	Dave Butenhof	12 September 1991
 *		Shrink TCB by dynamically allocating file masks (which are
 *		now 4096 bits on ULTRIX V4.2).
 *	024	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	025	Paul Curtin	20 November 1991
 *		Made exc_t_context volatile
 *	026	Dave Butenhof	27 November 1991
 *		Add thd_vmserrno field for DEC C and VAX C "vaxc$errno" (this
 *		is long overdue :-) ).
 *	027	Dave Butenhof	09 December 1991
 *		Align the count field of the alert structure to longword
 *		boundary.
 *	028	Dave Butenhof	13 December 1991
 *		Change name of exc_t_context to exc_context_t.
 *	029	Webb Scales	13 March 1992
 *		Parameterize scheduling policies.
 *	030	Dave Butenhof and Webb Scales	17 April 1992
 *		Remove "volatile" from exc_stack -- we don't think it's
 *		needed.
 *	031	Brian Keane	08 June 1992
 *		Change erron types to cma_t_errno.
 *	032	Dave Butenhof	27 August 1992
 *		Add value for tcb->event_status here (previous values were
 *		spread out through cma_dispatch and cma_timer -- and are now
 *		obsolete anyway).
 *	033	Webb Scales	 3 September 1992
 *		Change to include cma_sched_defs.h instead of cma_sched.h;
 *		add a new thread "kind" for the scheduling manager thread.
 *	034	Dave Butenhof	24 September 1992
 *		Adjust debug padding for addition of field to object header.
 *	035	Webb Scales	19 November 1992
 *		Add errno tables.
 */


#ifndef CMA_TCB_DEFS
#define CMA_TCB_DEFS

/*
 *  INCLUDE FILES
 */
#include <cma.h>
#include <cma_debug_client.h>
#include <cma_attr.h>
#include <cma_defs.h>
#include <cma_handle.h>
#include <cma_queue.h>
#if _CMA_OS_ == _CMA__UNIX
# if !_CMA_THREAD_SYNC_IO_
#  include <cma_thread_io.h>
# endif
# include <signal.h>
# ifndef NSIG
#  define NSIG sizeof(sigset_t)*8
# endif
#endif
#include <cma_sched_defs.h>
#if _CMA_OS_ == _CMA__VMS
# include <cma_tis_int.h>
#endif

/*
 * CONSTANTS AND MACROS
 */

#if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
# define cma__c_ibmr2_ctx_stack_size	2048
# define cma__c_ibmr2_ctx_stack_top	(cma__c_ibmr2_ctx_stack_size - 1)
#endif

/*
 * Define a value for tcb->timer.event_status, which cma__check_timer_queue
 * can set to indicate to a timed condition wait that the wait timed out.
 */
#define cma__c_notimeout	0
#define cma__c_timeout		1

/*
 * TYPEDEFS
 */

#ifndef __STDC__
struct CMA__T_INT_CV;
struct CMA__T_INT_MUTEX;
struct CMA__T_INT_TCB;
#endif

typedef cma_t_address		*cma__t_context_list;

typedef struct CMA__T_TCB_PAD {
    /*
     * Adjust to align the tcb prolog at byte 32. 8 bytes are required as
     * object header is currently  24 bytes long.
     */
    cma_t_integer	pad1;		/* pad bytes */
    cma_t_integer	pad2;		/* pad bytes */
    } cma__t_tcb_pad;

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
typedef struct CMA__T_TCB_SELECT {
    cma__t_queue	queue;
    cma__t_file_mask	*rfds;
    cma__t_file_mask	*wfds;
    cma__t_file_mask	*efds;
    cma_t_integer	nfound;
    } cma__t_tcb_select;
#endif

typedef struct CMA__T_TCB_TIME {
    cma__t_queue	queue;		/* must be first entry! */
    struct CMA__T_INT_CV *cv;		/* Used for timed waits */
    cma_t_integer	event_status;	/* Status of timed condition wait */
    cma_t_integer	quanta_remaining;
    cma_t_date_time	wakeup_time;
    } cma__t_tcb_time;

typedef enum CMA__T_DEBEVT {
	cma__c_debevt_activating = 1,	/* First transition to running */
	cma__c_debevt_running = 2,	/* Any transition to running */
	cma__c_debevt_preempting = 3,	/* Preemted (replaced) another thread */
	cma__c_debevt_blocking = 4,	/* Any transition to blocked */
	cma__c_debevt_terminating = 5,	/* Final state transition */
	cma__c_debevt_term_alert = 6,	/* Terminated due to alert/cancel */
	cma__c_debevt_term_exc = 7,	/* Terminated due to exception */
	cma__c_debevt_exc_handled = 8	/* Exception is being handled */
	} cma__t_debevt;

#define cma__c_debevt__first ((cma_t_integer)cma__c_debevt_activating)
#define cma__c_debevt__last  ((cma_t_integer)cma__c_debevt_exc_handled)
#define cma__c_debevt__dim (cma__c_debevt__last + 1)

/* 
 * Type defining thread substate, which is used by the debugger.
 * If the state is blocked, substate indicates WHY the thread is blocked.
 */
typedef enum CMA__T_SUBSTATE {
    cma__c_substa_normal = 0,
    cma__c_substa_mutex = 1,
    cma__c_substa_cv = 2,
    cma__c_substa_timed_cv = 3,
    cma__c_substa_term_alt = 4,
    cma__c_substa_term_exc = 5,
    cma__c_substa_delay =6,
    cma__c_substa_not_yet_run = 7
    } cma__t_substate;
#define cma__c_substa__first ((cma_t_integer)cma__c_substa_normal)
#define cma__c_substa__last  ((cma_t_integer)cma__c_substa_not_yet_run)
#define cma__c_substa__dim (cma__c_substa__last + 1)


/*
 * Per-thread state for the debugger
 */
typedef struct CMA__T_TCB_DEBUG {
    cma_t_boolean	on_hold;	/* Thread was put on hold by debugger */
    cma_t_boolean	activated;	/* Activation event was reported */
    cma_t_boolean	did_preempt;	/* Thread preempted prior one */
    cma_t_address	start_pc;	/* Start routine address */
    cma_t_address	object_addr;	/* Addr of thread object */
    cma__t_substate	substate;	/* Reason blocked, terminated, etc.*/
    cma_t_boolean	notify_debugger;/* Notify debugger thread is running */
    cma_t_address	SPARE2;		/* SPARE */
    cma_t_address	SPARE3;		/* SPARE */
    struct CMA__T_INT_TCB	
			*preempted_tcb;	/* TCB of thread that got preempted */
    cma_t_boolean	flags[cma__c_debevt__dim]; 	
					/* Events enabled for this thread */
    } cma__t_tcb_debug;

typedef struct CMA__T_TCB_SCHED {
    cma_t_integer	adj_time;	/* Abs. time in ticks of last prio adj */
    cma_t_integer	tot_time;	/* Weighted ave in ticks (scaled) */
    cma_t_integer	time_stamp;	/* Abs. time in ticks of last update */
    cma_t_integer	cpu_time;	/* Weighted average in ticks */
    cma_t_integer	cpu_ticks;	/* # of ticks while comp. (scaled) */
    cma_t_integer	q_num;		/* Number of last ready queue on */
    cma_t_priority	priority;	/* Thread priority */
    cma_t_sched_policy  policy;         /* Scheduling policy of thread */
    cma_t_boolean	rtb;		/* "Run 'Till Block" scheduling */
    cma_t_boolean	spp;		/* "Strict Priority Preemption" sched */
    cma_t_boolean	fixed_prio;	/* Fixed priority */
    cma__t_sched_class	class;		/* Scheduling class */
    struct CMA__T_VP	*processor;	/* Current processor (if running) */
    } cma__t_tcb_sched;

typedef struct CMA__T_INT_ALERT {
    cma_t_boolean	g_enable : 1;	/* general delivery state */
    cma_t_boolean	a_enable : 1;	/* asynchronous delivery state */
    cma_t_integer	spare : 30;	/* Pad to longword */
    cma_t_natural	count;		/* Alert scope nesting count */
    } cma__t_int_alert;

typedef enum CMA__T_STATE {
    cma__c_state_running = 0,		/* For consistency with initial TCB */
    cma__c_state_ready	= 1,
    cma__c_state_blocked = 2,		/* Before here is runnable: after not */
    cma__c_state_terminated = 3,
    cma__c_state_zombie = 4
    } cma__t_state;
#define cma__c_state__first cma__c_state_running
#define cma__c_state__last  cma__c_state_zombie
#define cma__c_state__dim (cma__c_state__last + 1)

typedef enum CMA__T_THKIND {
    cma__c_thkind_initial = 0,		/* Initial thread */
    cma__c_thkind_normal = 1,		/* Normal thread */
    cma__c_thkind_null	= 2,		/* A null thread */
    cma__c_thkind_mgr 	= 3		/* The scheduling manager thread */
    } cma__t_thkind;
#define cma__c_thkind__first ((cma_t_integer)cma__c_thkind_initial)
#define cma__c_thkind__last  ((cma_t_integer)cma__c_thkind_mgr)
#define cma__c_thkind__dim (cma__c_thkind__last + 1)

typedef struct CMA__T_INT_TCB {
    /* 
     * Fixed part of TCB.
     *   Modifications to the following three fields must be coordinated.  
     *   The object header must always be first, and the prolog must always 
     *   remain at the same offset (32) for all time. Thus the object header
     *   must never grow beyond a maximum of 32 bytes.
     */
    cma__t_object	header;		/* Common object header */
    cma__t_tcb_pad	pad1;		/* Pad required to align prolog */
    cma_t_tcb_prolog	prolog;		/* Standard prolog for tasks, threads */

    /* 
     * Floating part of TCB (fields here on are free to be moved and resized).
     */
    cma__t_queue	threads;	/* List of all known threads */
    cma__t_int_attr	*attributes;	/* Backpointer to attr obj */
    cma__t_state	state;		/* Current state of thread */
    cma__t_thkind	kind;		/* Which kind of thread */
    struct CMA__T_INT_MUTEX *mutex;	/* Mutex to control TCB access */
    struct CMA__T_INT_CV *term_cv;	/* CV for join */
    struct CMA__T_INT_MUTEX *tswait_mutex;	/* Mutex for thread-synchronous waits */
    struct CMA__T_INT_CV *tswait_cv;	/* CV for thread-synchronous waits */
    cma_t_start_routine	start_code;	/* Address of start routine */
    cma_t_address	start_arg;	/* Argument to pass to start_code */
    cma__t_queue	stack;		/* Queue header for stack descr. */
    cma_t_natural	context_count;	/* Size of context array */
    cma__t_context_list	contexts;	/* Context value array pointer */
    cma_t_exit_status	exit_status;	/* Exit status of thread */
    cma_t_address	return_value;	/* Thread's return value */
    cma__t_async_ctx	async_ctx;	/* Asynchronous context switch info */
    cma__t_static_ctx	static_ctx;	/* Static context switch information */
    cma__t_tcb_time	timer;		/* Time info for dispatcher */
    cma__t_tcb_sched	sched;		/* Scheduler info */
    cma__t_tcb_debug	debug;		/* Debugger info */
#if _CMA_OS_ == _CMA__UNIX
# if !_CMA_THREAD_SYNC_IO_
    cma__t_tcb_select	select;		/* Select info for timed selects */
# endif
    struct sigaction	sigaction_data[NSIG];
#endif
    cma_t_boolean	detached;	/* Set if already detached */
    cma_t_boolean	terminated;	/* Set if terminated */
    cma_t_integer	joiners;	/* Count of joiners, for zombie frees */
    cma__t_int_alert	alert;		/* Current alert enable state */
    cma__t_atomic_bit	alert_pending;	/* CLEAR when alert is pending */
    struct CMA__T_INT_CV *wait_cv;	/* CV thread is currently waiting on */
    struct CMA__T_INT_MUTEX *wait_mutex;	/* Mutex thread is waiting on */
    struct EXC_CONTEXT_T *exc_stack;	/* Top of exception stack */
#if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
    char		ctx_stack[cma__c_ibmr2_ctx_stack_size];
#endif
    cma_t_errno		thd_errno;	/* Per-thread errno value */
#if _CMA_OS_ == _CMA__VMS
    cma_t_errno		thd_vmserrno;	/* Per-thread VMS errno value */
    cma_t_errno		errno_tbl[cma_tis__c_errno_table_size];
    cma_t_errno		vmserrno_tbl[cma_tis__c_errno_table_size];
#endif
    } cma__t_int_tcb;

#endif

/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TCB_DEFS.H */
/*  *28   14-MAY-1993 15:56:37 BUTENHOF "Add 'zombie' state" */
/*  *27   24-NOV-1992 01:53:07 SCALES "Add support for TIS multiple errno table" */
/*  *26   28-SEP-1992 11:49:26 BUTENHOF "Add 'owner sequence' field" */
/*  *25   15-SEP-1992 13:50:39 BUTENHOF "Support Mach scheduling" */
/*  *24    3-SEP-1992 21:28:50 SCALES """Fix sched include""" */
/*  *23    2-SEP-1992 16:26:33 BUTENHOF "Add 'event_status' values" */
/*  *22   15-JUN-1992 16:51:34 KEANE "Make errno be type cma_t_errno" */
/*  *21   17-APR-1992 11:11:44 BUTENHOF "Remove volatile on exc_stack" */
/*  *20   18-MAR-1992 19:01:47 SCALES "Parameterize scheduling policies" */
/*  *19   13-DEC-1991 10:12:45 BUTENHOF "Fix exc symbols" */
/*  *18   13-DEC-1991 09:53:49 BUTENHOF "Align nesting count in alert structure" */
/*  *17   27-NOV-1991 09:25:11 BUTENHOF "Add field for VMS errno on VAX C & DEC C" */
/*  *16   20-NOV-1991 14:45:36 CURTIN "Added a volatile" */
/*  *15   14-OCT-1991 13:41:00 BUTENHOF "Refine/fix use of config symbols" */
/*  *14   17-SEP-1991 13:21:30 BUTENHOF "Handle variable # of FDs more efficiently" */
/*  *13    2-JUL-1991 16:52:53 BUTENHOF "Add & use start func/arg fields in TCB" */
/*  *12   10-JUN-1991 19:57:11 SCALES "Convert to stream format for ULTRIX build" */
/*  *11   10-JUN-1991 19:21:59 BUTENHOF "Fix the sccs headers" */
/*  *10   10-JUN-1991 18:24:15 SCALES "Add sccs headers for Ultrix" */
/*  *9    10-JUN-1991 17:55:33 SCALES "Conditionalize inclusion of I/O stuff" */
/*  *8     5-JUN-1991 18:57:21 CURTIN "removed exc_handling.h include: axe circularity" */
/*  *7     5-JUN-1991 17:31:52 BUTENHOF "Integrate IBM changes" */
/*  *6     1-APR-1991 18:09:33 BUTENHOF "Integrate exception changes" */
/*  *5    28-MAR-1991 17:22:35 SCALES "Improve variable priority dispatch" */
/*  *4    12-FEB-1991 01:29:35 BUTENHOF "Redefine alert state" */
/*  *3     6-FEB-1991 01:33:22 BUTENHOF "Change errno field name" */
/*  *2    17-DEC-1990 14:35:16 CURTIN "added sigaction stuff" */
/*  *1    12-DEC-1990 21:53:54 BUTENHOF "Thread bookkeeping" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TCB_DEFS.H */
