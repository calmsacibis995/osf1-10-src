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
static char *rcsid = "@(#)$RCSfile: cma_thread.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:52:37 $";
#endif
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	These routines provide the basic thread services.
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	24 August 1989
 *
 *  MODIFICATION HISTORY:
 *
 *  	001	Webb Scales	26 September 1989
 *		Added the thread info routines
 *	002	Webb Scales and Dave Butenhof	27 September 1989
 *		Fix pointer booboo
 *	003	Dave Butenhof	29 September 1989
 *		cma_fork needs to set priority on new thread.
 *	004	Bob Conti	6 October 1989
 *		Add the call to cma__error to make exit work.
 *		Add assertion for cma_assem_vms.
 *	005	Dave Butenhof	16 October 1989
 *		Convert TCB mutex operations to internal form.
 *	006	Dave Butenhof	19 October 1989
 *		Use new type-specific handle validation macros.
 *	007	Dave Butenhof	19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *	008	Webb Scales	20 October 1989
 *		Changed "cma_assem_vms.h" to "cma_assem.h"
 *	009	Dave Butenhof	Halloween 1989
 *		cma_detach needs to remove thread from "known threads" queue
 *		before freeing TCB.
 *	010	Webb Scales	3 November 1989
 *		cma_yield needs to enter the kernel before calling cma__dispatch
 *	011	Webb Scales & Bob Conti	    6 November 1989
 *		Renamed cma__dispatch to cma__yield_processor.  Added an 
 *		exit-kernel call to cma_yeild.
 *	012	Dave Butenhof	1 December 1989
 *		Modify external entries to track POSIX changes to names and
 *		argument ordering.
 *	013	Dave Butenhof	8 December 1989
 *		Signal cma_s_unimp when client attempts to change priority or
 *		scheduling policy.
 *	014	Dave Butenhof	12 December 1989
 *		Add cma__start_thread routine (called by cma__execute_thread
 *		from assembly code); this is responsible for setting up
 *		portable exception handlers, calling the client start
 *		routine, and cleaning up the thread.
 *	015	Dave Butenhof	14 December 1989
 *		- Now that we have exception handling, protect the condition
 *		  wait in cma_thread_join with a TRY/ENDTRY to catch
 *		  cma_e_alerted and unlock the TCB mutex.
 *		- Modify cma__thread_base to report an error on CATCH_ALL,
 *		  using new routine cma__report_error.
 *	016	Dave Butenhof & Bob Conti	15 December 1989
 *		Change cma__report_error to cma_exc_report.
 *	017	Dave Butenhof	15 February 1990
 *		Correct oversight; add priority argument to
 *		cma_thread_set_sched to conform to POSIX pthreads.
 *	018	Dave Butenhof	26 February 1990
 *		Fix set_sched and set_priority to deal with new symbols
 *		correctly.
 *	019	Dave Butenhof & Webb Scales	9 April 1990
 *		When creating a new thread, check for zombie threads.
 *	020	Webb Scales	30 May 1990
 *		Increment a reference count while joining with a thread.
 *		Also changed TRY to catch all exceptions during a join.
 *	021	Webb Scales	15 June 1990
 *		Added priority sceduling
 *		- Rearranged part of the tcb.
 *		- "Enabled" set_sched and added new policies.
 *		- "Enabled" set_prio and added new policies.
 *		- mutexed tcb reads for getting priority and scheduling policy.
 *	022	Paul Curtin	06 August 1990
 *		Replace abort w/ cma__abort.
 *	023	Webb Scales	16 August 1990
 *		Various clean-up stuff.
 *	024	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	025	Bob Conti	10 October 1990
 *		Fix bug in set_priority: unlock before raising error.
 *	026	Dave Butenhof	14 December 1990
 *		Change cma_attributes.h to cma_attr.h (shorten length)
 *	027	Dave Butenhof	1 February 1991
 *		Correct order of args to cma__int_wait to conform to external
 *		interface (cv, mutex).
 *	028	Dave Butenhof	12 March 1991
 *		P1003.4a allows the "status" pointer passed in to
 *		pthread_join to be null, which is a nice convenience if you
 *		don't care about the return status. Make cma_thread_join
 *		equivalent by allowing either pointer to be null.
 *	029	Dave Butenhof	25 March 1991
 *		Change from cma_exception to exc_handling
 *	030	Dave Butenhof	05 April 1991
 *		Support Mach kernel threads (with one-to-one mapping)
 *	031	Paul Curtin	08 April 1991
 *		Adjust call to abort process to pass signal value.
 *	032	Paul Curtin	23 April 1991
 *		Added kernel lock around known thread list access.
 *	033	Dave Butenhof	01 May 1991
 *		Add arguments to cma__bugcheck() calls.
 *	034	Webb Scales and Dave Butenhof	02 May 1991
 *		Fix set_priority and set_scheduler to make the dispatcher
 *		recalculate the thread's priority queue, and yield to allow
 *		appropriate threads a chance to take over.
 *	035	Dave Butenhof	23 May 1991
 *		Under OSF/1 systems, cma__thread_base should set up handling
 *		for the per-thread signals (by calling
 *		cma__sig_thread_init()).
 *	036	Dave Butenhof	29 May 1991
 *		Raise cma_e_unimp on attempts to modify thread
 *		priority or policy.
 *	037	Dave Butenhof	01 July 1991
 *		Improve VP caching for Mach threads by just suspending it
 *		inside cma__thread_base and letting it loop.
 *	038	Dave Butenhof	12 September 1991
 *		Integrate Nat Mishkin's performance improvements to make more
 *		intelligent use of the file descriptor numbers; ULTRIX V4.2
 *		allows 4096 files, but most systems are generated to support
 *		lower numbers, and DECthreads is currently carrying around
 *		all that extra baggage. With this change, it'll only check
 *		and use the ones actually supported on the system. Also,
 *		shrink the TCB by dynamically allocating the file descriptor
 *		mask at thread creation time.
 *	039	Dave Butenhof	06 December 1991
 *		For convenience (I've got no intention of documenting this
 *		for now, but it seems like a nice one-plus), save the status
 *		code for a status exception that terminates a thread.
 *	040	Dave Butenhof	10 February 1992
 *		Add cma__trace call on VP creation.
 *	041	Dave Butenhof	11 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	042	Dave Butenhof	13 March 1992
 *		Since cma__thread_base() doesn't use start routine and arg,
 *		there's no need to pass them into the assembly code: change
 *		call to cma__create_thread() appropriately.
 *	043	Webb Scales	13 March 1992
 *		Parameterized scheduling policies.  Added enter-kernel around
 *		setting policy and priority.
 *	044	Webb Scales and Dave Butenhof	23 March 1992
 *		Fix detach to handle multiple detaches.
 *	045	Dave Butenhof	13 April 1992
 *		Remove most CATCH clauses and CATCH_ALL from cma__thread_base
 *		function so that the new "exception interest" model wi work.
 *		That is, exceptions not explicitly handled will result in a
 *		process dump (on VMS, image exit).
 *	046	Paul Curtin	08 June 1992
 *		Add to cma__thread_base cactus link, for Alpha VMS
 *	047	Paul Curtin	23 June 1992
 *		Merge in generation 25A1 changes (Webb Scales' bug fix for
 *		a window in detach where a thread object could be prematurely
 *		reclaimed.)
 *	048	Dave Butenhof	04 August 1992
 *		Fix OSF/1 bug -- original VP logic cached VP in thread level,
 *		so int_make_thread created new VP only if the thread didn't
 *		have one. When I changed that, I forgot to remove conditional
 *		create (or clear the old vpid), so it started reusing
 *		terminated VP. Ooops.
 *	049	Brian Keane, Paul Curtin, Webb Scales	05 August 1992
 *		Re-worked cma_yield - in the VP case, it was yielding with
 * 		the kernel locked.  It now calls cma__vp_yield directly, and
 * 		doesn't lock the kernel in the VP case.
 *	050	Dave Butenhof	12 August 1992
 *		Remove reference to cma__g_vp_count (which no longer
 *		exists).
 *	051	Dave Butenhof	13 August 1992
 *		Add setup for using PAL unique value on OSF/1 Alpha (there's
 *		currently no value to using it for OpenVMS Alpha, since we
 *		have a single-instruction 'get ID' on uniprocessor dispatcher
 *		anyway).
 *	052	Webb Scales	 3 September 1992
 *		Changed the name of the scheduler header file.
 *	053	Dave Butenhof	10 September 1992
 *		Generalize the check for unsupported POSIX scheduling
 *		functions, using _CMA_POSIX_SCHED_ rather than assuming Mach
 *		thread systems don't support it.
 *	054	Dave Butenhof	16 September 1992
 *		Conditionalize cases for sched. policies that are overmapped
 *		to SCHED_OTHER on RT kernel thread implementation.
 *	055	Webb Scales	23 September 1992
 *		Add the "ada-rtb" scheduling policy.
 *	056	Paul Curtin	 6 October 1992
 *		Change cactus stacking code.
 *	057	Dave Butenhof	13 October 1992
 *		Remove vp_setsched call.
 *	058	Dave Butenhof	25 November 1992
 *		Remove special cases for RT4_KTHREAD policies; all are now
 *		defined uniquely.
 *	059	Dave Butenhof	10 December 1992
 *		Ooops -- int_make_thread doesn't return true/false; check
 *		explicitly for cma_s_normal.
 *	060	Dave Butenhof	 6 January 1993
 *		Decode return from vp_set_sched and raise nopriv only if
 *		appropriate.
 *	061	Dave Butenhof	 7 January 1993
 *		Fix 060.
 *	062	Dave Butenhof	19 January 1993
 *		Add assertion in thread_base that current thread == tcb arg.
 *	063	Dave Butenhof	21 January 1993
 *		Allow up to prio 63 on OSF/1 RT for all realtime policies.
 *	064	Dave Butenhof	29 January 1993
 *		Jeff Denham noticed that "illegal" sched params are stored in
 *		TCB even when RT function fails on OSF/1. Fix it!
 *	065	Brian Keane	 3 March 1993
 *		Raise whatever status cma__int_make_thread returns as an
 *		exception - previously all problems were mapped to exc_s_insfmem.
 *	066	Dave Butenhof	 4 March 1993
 *		Remove ability to set ada_low & ada_rtb threads up to prio
 *		63; not really appropriate, since that should be fifo or rr.
 *		Also, utilize the new policy/priority range table for bounds
 *		checking.
 *	067	Dave Butenhof	10 March 1993
 *		Add a way for dispell_zombies to tell when a VP has actually
 *		terminated without using vp_get_state (which is redundant and
 *		expensive) -- set VP "reset" pointer in creation.
 *	068	Dave Butenhof	25 March 1993
 *		Recode VP thread creation. It doesn't need to lock kernel,
 *		and trace should come before thread is started -- or thread
 *		may be done before it prints. Also, fix trace formatting to
 *		use "0x%lx" for hex values, not "%08x".
 *	069	Brian Keane	30 March 1993
 *		Add self deadlock detection to cma_thread_join.  Remove
 *		unnecessary exit_kernel from error path after calling
 *		cma__vp_set_start.
 *	070	Dave Butenhof	31 March 1993
 *		Use better names for user objects.
 *	071	Dave Buatenhof	 5 April 1993
 *		Remove 'reset' field in VP structure.
 *	072	Dave Butenhof	12 April 1993
 *		Add argument to cma__int[_timed]_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	073	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	074	Dave Butenhof	 4 May 1993
 *		Change locking on thread rundown -- run_down_context no
 *		longer wants TCB locked.
 *	075	Dave Butenhof	11 May 1993
 *		Detach needs to move already-terminated thread after
 *		cma__g_last_thread.
 *	076	Dave Butenhof	14 May 1993
 *		Track first dead thread rather than last living thread
 *		(cma__g_last_thread is now cma__g_dead_zone) -- otherwise,
 *		destroy_tcb and free_tcb would have to enter kernel and check
 *		pointer to avoid moving last_thread onto cache or pool list!
 *	077	Dave Butenhof	11 June 1993
 *		Disable alerts on return from user's thread routine, to avoid
 *		any possible complications.
 *	078	Dave Butenhof	26 July 1993
 *		cma_thread_create() dequeues known TCB on failure, without
 *		locking kernel. Tsk tsk.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_attr.h>
#include <cma_context.h>
#include <cma_kernel.h>
#include <cma_dispatch.h>
#include <cma_handle.h>
#include <cma_init.h>
#include <cma_stack.h>
#include <cma_tcb.h>
#include <cma_thread.h>
#include <cma_assem.h>
#include <cma_mutex.h>
#include <cma_condition.h>
#include <cma_util.h>
#include <cma_deb_core.h>
#include <cma_deb_event.h>
#include <cma_debug_client.h>
#include <cma_vp.h>
#include <cma_vm.h>
#include <cma_sched_defs.h>

#if _CMA_PER_THD_SYNC_SIGS_
# include <cma_signal.h>
#endif

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
# include <libicb.h>
#endif

#if _CMA_OS_ != _CMA__VMS
# include <signal.h>
#endif

/*
 * LOCAL DATA
 */

/*
 * LOCAL FUNCTIONS
 */



/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine grants permission to delete a thread object
 *
 *  FORMAL PARAMETERS:
 *
 *	thread  -  Address of the handle of the thread to detach
 *
 *  IMPLICIT INPUTS:
 *
 *	The thread TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The thread TCB
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	The thread may be freed.
 */
extern void
cma_thread_detach
#ifdef _CMA_PROTO_
	(
	cma_t_thread	*thread)	/* Handle of thread to detach*/
#else	/* no prototypes */
	(thread)
	cma_t_thread	*thread;	/* Handle of thread to detach*/
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;

    /* 
     * Validate the handle, insure that it's a thread, and get the TCB address
     */
    tcb = cma__validate_null_tcb (thread);

    /*
     * If thread is a null handle, do nothing and return
     */
    if (tcb == (cma__t_int_tcb *)cma_c_null_ptr) return;

    cma__int_detach (tcb);

    /*
     * This handle is no longer valid, clear it
     */
    cma__clear_handle (thread);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine causes the thread to terminate prematurely, with an error
 *
 *  FORMAL PARAMETERS:
 *
 *	None
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_exit_error
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb	*self;


    /* 
     * Terminate the thread by raising a special exception.
     */
    self = cma__get_self_tcb ();
    self->exit_status = cma_c_term_error;
    cma__error (cma_s_exit_thread);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine causes the thread to terminate prematurely, with success
 *	status and a return value.
 *
 *  FORMAL PARAMETERS:
 *
 *	result	The thread's result value
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_exit_normal
#ifdef _CMA_PROTO_
	(
	cma_t_address	result)
#else	/* no prototypes */
	(result)
	cma_t_address	result;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self;


    /* 
     * Terminate the thread by raising a special exception.
     */
    self = cma__get_self_tcb ();
    self->exit_status = cma_c_term_normal;
    self->return_value = result;
    cma__error (cma_s_exit_thread);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine creates a new thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	new_thread	- Address of handle to point to new thread (output)
 *	attr		- Address of handle for the attributes object to use
 *			  in creating the thread (and stack)
 *	start_routine	- Address of function which the new thread will execute
 *	arg		- Argument to be passed in the call to the function
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	A new thread is born and possibly scheduled (ie, it possibly preempts
 *	the current (calling) thread).
 */
extern void
cma_thread_create
#ifdef _CMA_PROTO_
	(
	cma_t_thread		*new_thread, /* Handle of new thread */
	cma_t_attr		*attr,	/* Att's of thread & stack */
	cma_t_start_routine	start_routine, /* Function to be executed */
	cma_t_address		arg)	/* Argument to start routine */
#else	/* no prototypes */
	(new_thread, attr, start_routine, arg)
	cma_t_thread		*new_thread; /* Handle of new thread */
	cma_t_attr		*attr;	/* Att's of thread & stack */
	cma_t_start_routine	start_routine; /* Function to be executed */
	cma_t_address		arg;	/* Argument to start routine */
#endif	/* prototype */
    {
    cma__t_int_attr	*new_att;	/* Internal form of attrib. obj */
    cma__t_int_tcb	*new_tcb;	/* New thread's TCB */
    cma_t_status	status;
    cma__t_queue	*tq;


    /*    
     * Get internal pointer to att object (and validate it while we're at it)
     */
    new_att = cma__validate_default_attr (attr);
		
    /* 
     * Get a TCB (and stack) for the new thread
     */
    cma__dispell_zombies ();		/* see if we can free a tcb/stack */
    new_tcb = cma__get_tcb (new_att);

    if ((cma_t_address)new_tcb == cma_c_null_ptr)
	cma__error (exc_s_insfmem);

    status = cma__int_make_thread (
	    new_tcb,
	    new_thread,
	    start_routine,
	    arg);

    if (status != cma_s_normal) {
	cma__enter_kernel ();
	cma__queue_remove (&new_tcb->threads, tq, cma__t_queue);
	cma__g_threadcnt--;
	cma__exit_kernel ();
	cma__free_tcb (new_tcb);
	cma__error (status);
	}

    /*
     * Point user's handle at the new thread object
     */
    cma__object_to_handle ((cma__t_object *)new_tcb, new_thread);
    cma__obj_set_owner (new_tcb, (cma_t_integer)new_thread);
    cma__obj_set_name (new_tcb, "<CMA user@0x%lx>");
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine allows the user to retrive the priority of a given thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	thread	  - The handle for the thread object whose priority is sought
 *	priority  - The address of a varible to recieve the thread's priority
 *
 *  IMPLICIT INPUTS:
 *
 *	The indicated thread's TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_get_priority
#ifdef _CMA_PROTO_
	(
	cma_t_thread	*thread,
	cma_t_priority	*priority)
#else	/* no prototypes */
	(thread, priority)
	cma_t_thread	*thread;
	cma_t_priority	*priority;
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__t_int_tcb  *tcb;

    /*
     * Validate the handle, and retrieve the address of the TCB from it.
     */
    tcb = cma__validate_tcb (thread);

    /*
     * Fetch the priority from the thread's TCB and store it at the address
     * supplied by the user.
     */
    cma__int_lock (tcb->mutex);		    /* Lock for visibility */
    *priority = tcb->sched.priority;
    cma__int_unlock (tcb->mutex);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine allows the user to retrive the scheduling policy of a
 *	given thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		The handle for the thread object whose scheduling
 *			policy is sought.
 *	policy		The address of a varible to recieve the thread's
 *			scheduling policy.
 *
 *  IMPLICIT INPUTS:
 *
 *	The indicated thread's TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_get_sched
#ifdef _CMA_PROTO_
	(
	cma_t_thread		*thread,
	cma_t_sched_policy	*policy)
#else	/* no prototypes */
	(thread, policy)
	cma_t_thread		*thread;
	cma_t_sched_policy	*policy;
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__t_int_tcb  *tcb;

    /*
     * Validate the handle, and retrieve the address of the TCB from it.
     */
    tcb = cma__validate_tcb (thread);

    /*
     * Fetch the priority from the thread's TCB and store it at the address
     * supplied by the user.
     */
    cma__int_lock (tcb->mutex);		    /* Lock for visibility */
    *policy = tcb->sched.policy;
    cma__int_unlock (tcb->mutex);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine allows the user to get a handle for the current thread.
 *
 *  FORMAL PARAMETERS:
 *
 *	thread	- The address of storage to receive a handle for this thread
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_get_self
#ifdef _CMA_PROTO_
	(
	cma_t_thread	*thread)
#else	/* no prototypes */
	(thread)
	cma_t_thread	*thread;
#endif	/* prototype */
    {
    cma__t_int_tcb *tcb;


    /*
     * Get a pointer to the current thread's TCB
     */
    tcb = cma__get_self_tcb ();

    /*
     * Point user's handle at the current thread object
     */
    cma__object_to_handle ((cma__t_object *)tcb, thread);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine causes the calling thread to block until a specific thread
 *	 terminates.  
 *
 *  FORMAL PARAMETERS:
 *
 *	thread	    - Address of the handle of the thread to await
 *	exit_status - Address of variable to receive a value indicating whether
 *			termination was normal, due an error, or due to an alert
 *	result	    - Address of variable to receive the value returned from the
 *			thread's start function, if any
 *
 *  IMPLICIT INPUTS:
 *
 *	The target thread's TCB
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_join
#ifdef _CMA_PROTO_
	(
	cma_t_thread		*thread,	/* Thread to await	    */
	cma_t_exit_status	*exit_status, /* Indicates successful term*/
	cma_t_address		*result) /* Value thread's function  */
#else	/* no prototypes */
	(thread, exit_status, result)
	cma_t_thread		*thread;	/* Thread to await	    */
	cma_t_exit_status	*exit_status; /* Indicates successful term*/
	cma_t_address		*result; /* Value thread's function  */
#endif	/* prototype */
    {
    cma__t_int_tcb  *tcb, *self;


    /*
     * Validate the handle, insure that it's a thread, and get the TCB
     * address
     */
    tcb = cma__validate_tcb (thread);

    /*
     *	Avoid joining with self, which would cause deadlock.
     */
    self = cma__get_self_tcb ();

    if (self == tcb)
	cma__error (cma_s_in_use);

    /*
     * Wait for the thread to finish, looping to catch spurious wake-ups.  Then
     * copy the exit_status and return value into the parameters.
     *
     * Increment a reference count to prevent the thread's zombie from being 
     * freed prematurely.
     */

    cma__int_lock (tcb->mutex);

    tcb->joiners++;
    
    TRY {
	while (!tcb->terminated) {
	    cma__trace ((
		    cma__c_trc_tmp,
		    "(thread_join) thread 0x%lx waiting on 0x%lx",
		    self,
		    tcb));
	    cma__int_wait (tcb->term_cv, tcb->mutex, self);
	    }
	}
    CATCH_ALL {
	cma__trace ((
		cma__c_trc_tmp,
		"(thread_join) exception 0x%lx in thread 0x%lx",
		(THIS_CATCH)->status.status,
		self));
	tcb->joiners--;
	cma__int_unlock (tcb->mutex);
	RERAISE;
	}
    ENDTRY

    cma__trace ((
	    cma__c_trc_tmp,
	    "(thread_join) thread 0x%lx status 0x%lx, result 0x%lx",
	    tcb,
	    tcb->exit_status,
	    tcb->return_value));

    if (exit_status != (cma_t_exit_status *)cma_c_null_ptr)
	*exit_status = tcb->exit_status;

    if (result != (cma_t_address *)cma_c_null_ptr)
	*result = tcb->return_value;

    tcb->joiners--;

    cma__int_unlock (tcb->mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine allows the user to set the priority of a given thread to a
 *	particular value.
 *
 *  FORMAL PARAMETERS:
 *
 *	thread	  - The handle for the thread object whose priority is to change
 *	priority  - The value of the new priority
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The indicated thread's TCB
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_set_priority
#ifdef _CMA_PROTO_
	(
	cma_t_thread	*thread,
	cma_t_priority	priority)
#else	/* no prototypes */
	(thread, priority)
	cma_t_thread	*thread;
	cma_t_priority	priority;
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__t_int_tcb  *tcb;

    /*
     * Validate the handle, and retrieve the address of the TCB from it.
     */
    tcb = cma__validate_tcb (thread);

    /*
     * Lock for read as well as write.
     */
    cma__int_lock (tcb->mutex);

    /*
     * Check the range of the priority value against the allowed range for
     * the specified policy. Note that on the DEC OSF/1 realtime kernel, we
     * allow realtime policies (RR & FIFO) to set up to the maximum kernel
     * priority (63) even though that's above the DECthreads max constant.
     */
    if (priority < cma__g_pri_range[tcb->sched.policy][0]
#if _CMA_RT4_KTHREAD_
	    || priority > 63
	    || (tcb->sched.policy > cma_c_sched_rr
	    && priority > cma__g_pri_range[tcb->sched.policy][2])
#else
	    || priority > cma__g_pri_range[tcb->sched.policy][2]
#endif
	    ) {
	cma__int_unlock (tcb->mutex);
	cma__error (cma_s_badparam);
	}

    /*
     * Store the priority value supplied by the user in the thread's TCB.
     * Enter kernel, first, on uniprocessor systems, to avoid conflicts with
     * the scheduler.
     */
# if !_CMA_THREAD_IS_VP_
    cma__enter_kernel ();
    tcb->sched.priority = priority;
    cma__exit_kernel ();
    cma__int_unlock (tcb->mutex);

    /*
     * Force cma__ready to perform a priority queue calculation 
     */    
    cma__enter_kernel ();
    tcb->sched.adj_time = -(cma__c_prio_interval + 1);
    cma__yield_processor ();		/* Should we give up CPU? */
    cma__exit_kernel ();
# else
#  if _CMA_RT4_KTHREAD_
    {
    cma__t_vp_status	vpstat;
    
    vpstat = cma__vp_set_sched (
	    tcb->sched.processor->vp_id,
	    tcb->sched.policy,
	    priority);

    if (vpstat != cma__c_vp_normal) {
	if (vpstat == cma__c_vp_err_nopriv) {
	    cma__int_unlock (tcb->mutex);
	    cma__error (exc_s_nopriv);
	    }
	else {
	    cma__int_unlock (tcb->mutex);
	    cma__error (cma_s_unimp);
	    }
	}
    else
	tcb->sched.priority = priority;

    }
#  endif
    cma__int_unlock (tcb->mutex);
# endif
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine allows the user to set the scheduling policy of a given
 *	thread to a particular value.
 *
 *  FORMAL PARAMETERS:
 *
 *	thread		The handle for the thread object whose priority is to
 *			change.
 *	
 *	policy		The value of the new scheduling policy
 *
 *	priority	New priority for thread (interpreted under new
 *			scheduling policy).
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The indicated thread's TCB
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_thread_set_sched
#ifdef _CMA_PROTO_
	(
	cma_t_thread		*thread,
	cma_t_sched_policy	policy,
	cma_t_priority		priority)
#else	/* no prototypes */
	(thread, policy, priority)
	cma_t_thread		*thread;
	cma_t_sched_policy	policy;
	cma_t_priority		priority;
#endif	/* prototype */
    {
#if !_CMA_POSIX_SCHED_
    /*
     * If the implementation doesn't support POSIX.4 policy, we can't
     * support this function.
     */
    cma__error (cma_s_unimp);
#else
    cma__t_int_tcb  *tcb;

    /*
     * Validate the handle, and retrieve the address of the TCB from it.
     */
    tcb = cma__validate_tcb (thread);

    if (policy < cma_c_sched_fifo || policy > cma_c_sched_idle)
	cma__error (cma_s_badparam);

    /*
     * Check the range of the priority value against the allowed range for
     * the specified policy. Note that on the DEC OSF/1 realtime kernel, we
     * allow realtime policies (RR & FIFO) to set up to the maximum kernel
     * priority (63) even though that's above the DECthreads max constant.
     */
    if (priority < cma__g_pri_range[policy][0]
#if _CMA_RT4_KTHREAD_
	    || priority > 63
	    || (policy > cma_c_sched_rr
	    && priority > cma__g_pri_range[policy][2])
#else
	    || priority > cma__g_pri_range[policy][2]
#endif
	    )
	cma__error (cma_s_badparam);

# if !_CMA_THREAD_IS_VP_
    /*
     * Force cma__ready to perform a priority queue calculation 
     */    
    cma__int_lock (tcb->mutex);
    cma__enter_kernel ();
    tcb->sched.policy = policy;
    tcb->sched.priority = priority;
    cma__sched_parameterize (tcb, policy);
    cma__exit_kernel ();
    cma__int_unlock (tcb->mutex);
    cma__enter_kernel ();
    tcb->sched.adj_time = -(cma__c_prio_interval + 1);
    cma__yield_processor ();		/* Should we give up CPU? */
    cma__exit_kernel ();
# else
#  if _CMA_RT4_KTHREAD_
    {
    cma__t_vp_status	vpstat;
    
    cma__int_lock (tcb->mutex);
    vpstat = cma__vp_set_sched (
	    tcb->sched.processor->vp_id,
	    policy,
	    priority);

    if (vpstat != cma__c_vp_normal) {

	if (vpstat == cma__c_vp_err_nopriv) {
	    cma__int_unlock (tcb->mutex);
	    cma__error (exc_s_nopriv);
	    }
	else {
	    cma__int_unlock (tcb->mutex);
	    cma__error (cma_s_unimp);
	    }

	}
    else {
	tcb->sched.policy = policy;
	tcb->sched.priority = priority;
	}

    cma__int_unlock (tcb->mutex);
    }
#  endif
# endif
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine allows a thread to notify the scheduler of its willingness
 *	to release its processor to other threads of the same (or higher)
 *	priority.
 *
 *  FORMAL PARAMETERS:
 *
 *	None
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	None
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma_yield
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_THREAD_IS_VP_
    cma__vp_yield ();
#else
    cma__enter_kernel ();
    cma__yield_processor ();
    cma__exit_kernel ();
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine cleans up after a thread has terminated.
 *
 *  FORMAL PARAMETERS:
 *
 *	thread	    - Address of the dead thread TCB
 *
 *  IMPLICIT INPUTS:
 *
 *	None
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The target thread's TCB
 *
 *  FUNCTION VALUE:
 *
 *	None
 *
 *  SIDE EFFECTS:
 *
 *	None
 */
extern void
cma__cleanup_thread
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb		*tcb,	/* Thread to clean up */
	cma_t_address		result,	/* Value thread's function */
	cma_t_exit_status	exit_status) /* Indicates successful term*/
#else	/* no prototypes */
	(tcb, result, exit_status)
	cma__t_int_tcb		*tcb;	/* Thread to clean up */
	cma_t_address		result;	/* Value thread's function */
	cma_t_exit_status	exit_status; /* Indicates successful term*/
#endif	/* prototype */
    {
    cma__run_down_context (tcb);		/* Run PTC destructors */
    cma__int_lock (tcb->mutex);			/* Writer's lock */
    tcb->exit_status = exit_status;		/* Record for posterity */
    tcb->return_value = result;			/* Record for posterity */
    tcb->terminated = cma_c_true;		/* Mark thread terminated */
    cma__int_unlock (tcb->mutex);		/* Visibility */
    cma__trace ((
	    cma__c_trc_obj,
	    "(cleanup_thread) 0x%lx broadcasting to joiners",
	    tcb));
    cma__int_broadcast (tcb->term_cv);		/* Kick the joiners */
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine detaches a thread so it can be cleaned up
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb		Address of the thread's TCB
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
 */
extern void
cma__int_detach
#ifdef _CMA_PROTO_
	(cma__t_int_tcb		*tcb)
#else	/* no prototypes */
	(tcb)
	cma__t_int_tcb		*tcb;
#endif	/* prototype */
    {
    cma__t_queue	*tq;
#ifndef NDEBUG
    cma_t_boolean	termstate = cma_c_false, wasdetached;
    cma_t_boolean	wasmoved = cma_c_false;
#endif


    cma__trace ((
	    cma__c_trc_disp | cma__c_trc_zombie,
	    "(thread_detach) beginning detach of %d(0x%lx)...",
	    tcb->header.sequence,
	    tcb));

    /*
     * Lock the kernel to insure visibility and atomicity. If the thread has
     * terminated, and there are no threads waiting to join,
     * cma__dispell_zombies will reclaim the resources on the next pass.
     *
     * Since dispell_zombies doesn't look beyond the first undetached thread,
     * move it to the beginning of the "zombie list".
     */
    cma__enter_kernel ();

#ifndef NDEBUG
    wasdetached = tcb->detached;
#endif

    if (!tcb->detached) {

	if (tcb->state == cma__c_state_terminated) {
#ifndef NDEBUG
	    termstate = cma_c_true;
#endif
	    tcb->state = cma__c_state_zombie;

	    if (cma__g_dead_zone != &tcb->threads) {
		cma__queue_remove (&tcb->threads, tq, cma__t_queue);
		cma__queue_insert (&tcb->threads, cma__g_dead_zone);
		cma__g_dead_zone = &tcb->threads;
#ifndef NDEBUG
		wasmoved = cma_c_true;
#endif
		}

	    }

	}

    tcb->detached = cma_c_true;		/* Mark it for freeing */
    cma__exit_kernel ();

    cma__trace ((
	    cma__c_trc_disp | cma__c_trc_zombie,
	    "(thread_detach) ...detached %d(0x%lx)[t=%d,d=%d,m=%d]",
	    tcb->header.sequence,
	    tcb,
	    termstate,
	    wasdetached,
	    wasmoved));

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine sets up a thread to execute
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb		Address of the thread's TCB
 *	handle		Address of thread handle (cma_c_null_ptr for internal
 *			threads)
 *	start_routine	Client's thread routine
 *	start_arg	Client's thread argument
 *	start_it	cma_c_true if thread should be started; else it's
 *			left unscheduled (VP suspended if _CMA_THREAD_IS_VP_)
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
 *	status code for failure (or cma_s_normal for success)
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
extern cma_t_status
cma__int_make_thread
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb		*tcb,
	cma_t_thread		*handle,
	cma_t_start_routine	start_routine,
	cma_t_address		start_arg)
#else	/* no prototypes */
	(tcb, handle, start_routine, start_arg)
	cma__t_int_tcb		*tcb;
	cma_t_thread		*handle;
	cma_t_start_routine	start_routine;
	cma_t_address		start_arg;
#endif	/* prototype */
    {
    cma__t_int_stack	*new_stack;	/* New thread's stack */
    cma__t_vp		*vp;		/* VP structure for new VP */
    cma__t_vpid		vpid;		/* ID of VP (if nonmultiplexed mP) */
    cma__t_vp_state	state;		/* State array for new VP */
    cma__t_vp_status	vpstat;


    /*
     * Get address of the new child thread's stack object
     */
    new_stack = (cma__t_int_stack *)cma__queue_next (&tcb->stack);
    tcb->start_code =	start_routine;
    tcb->start_arg =	start_arg;

    /*
     * Remember some stuff for debugging displays
     */
    tcb->debug.start_pc = (cma_t_address)start_routine;
    tcb->debug.object_addr = (cma_t_address)handle;

    /*
     * This isn't necessary, strictly -- but cma_debug() can only get the
     * static context for threads other than the current thread. This ensures
     * that the "thread" command won't incorrectly complain about an invalid
     * SP.
     */
    tcb->static_ctx.sp = (long int)new_stack->stack_base;

    cma__trace ((
	    cma__c_trc_obj,
	    "(int_make_thread) thread 0x%lx (%d), pc 0x%lx (arg 0x%lx), sp 0x%lx, handle 0x%lx",
	    tcb,
	    tcb->header.sequence,
	    start_routine,
	    start_arg,
	    new_stack->stack_base,
	    handle));

    /*
     * Create a runnable thread:  Initialize the stack and context, and
     * notify the scheduler
     */
#if _CMA_THREAD_IS_VP_
    /*
     * If we have kernel threads, and we're enforcing one-to-one mapping,
     * then create a new VP for the thread.
     */
    vpstat = cma__vp_create (&vpid);

    if (vpstat != cma__c_vp_normal) {

	if (vpstat == cma__c_vp_insfmem)
	    return exc_s_insfmem;
	else
	    return exc_s_exquota;

	}

    if ((cma_t_address)tcb->sched.processor == cma_c_null_ptr) {
	vp = cma__alloc_object (cma__t_vp);

	if ((cma_t_address)vp == cma_c_null_ptr) {
	    cma__vp_cache (vpid);
	    return exc_s_insfmem;
	    }

	tcb->sched.processor = vp;
	}
    else
	vp = tcb->sched.processor;

    state.stack = (cma_t_integer)new_stack->stack_base;
    state.tcb = (cma_t_integer)tcb;
    state.start_routine = (cma_t_integer)start_routine;
    state.start_arg = (cma_t_integer)start_arg;
    state.policy = tcb->sched.policy;
    state.priority = tcb->sched.priority;

    vpstat = cma__vp_set_start (vpid, &state);

    if (vpstat != cma__c_vp_normal) {
	cma__vp_cache (vpid);
    
	if (vpstat == cma__c_vp_err_nopriv)
	    return exc_s_nopriv;
	else
	    return cma_s_unimp;

	}

    vp->vp_id = vpid;
    vp->current_thread = tcb;

    cma__trace ((
	    cma__c_trc_obj | cma__c_trc_vp,
	    "(int_make_thread) thread 0x%lx (%d); vp %d allocated",
	    tcb,
	    tcb->header.sequence,
	    vpid->vp));
    cma__start_thread (tcb);
#else
    cma__create_thread (
	    &tcb->static_ctx,		/* Address of base of child context */
	    new_stack->stack_base,	/* Address of base of child stack */
	    tcb);
#endif

    return cma_s_normal;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine begins thread execution, and handles errors.
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb		Address of the thread's TCB
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
 */
extern void
cma__thread_base
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb		*tcb)
#else	/* no prototypes */
	(tcb)
	cma__t_int_tcb		*tcb;
#endif	/* prototype */
    {
    cma_t_start_routine	routine	= tcb->start_code;
    cma_t_address	argument= tcb->start_arg;
    cma_t_address	value;
    cma_t_exit_status	status;
    char	output[128];
#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS
    struct invo_context_blk     invocation_context;
    cma_t_integer               invocation_handle;
#endif

    /*
     * A new thread is started with the kernel locked.  The first thing we
     * need to do is unlock the kernel to allow other business to proceed:
     */
#if !_CMA_THREAD_IS_VP_
    cma__exit_kernel ();
#else
# if _CMA_HARDWARE_ == _CMA__ALPHA && _CMA_OSIMPL_ == _CMA__OS_OSF
    cma__set_unique ((long int)tcb);
# endif
#endif

    cma__trace ((
	    cma__c_trc_obj,
	    "(thread_base) beginning thread 0x%lx (%d) at pc 0x%lx (arg 0x%lx), sp 0x%lx",
	    tcb,
	    tcb->header.sequence,
	    routine,
	    argument,
	    cma__fetch_sp ()));

#if _CMA_PLATFORM_ == _CMA__ALPHA_VMS

    /*
     * Grab out current invocation context.
     */
    lib$get_current_invo_context (&invocation_context);

    /*
     * Grab out previous invocation context.
     */
    lib$get_previous_invo_context (&invocation_context);


    /*
     * Grab invocation handle associated with previous context.
     */
    invocation_handle = lib$get_invo_handle (&invocation_context);
    
    /*
     * Load up an invocation context block with the FP that
     * we wish to use as the link.
     */
    invocation_context.libicb$q_ireg[29] = cma__g_base_frame;

    /*
     * Cactus the current saved context to the base of the main stack.
     */

#if 0
    /*
     * Fix-me***
     * This call needs a third argument... a register mask.
     *
     * Fix-me***: This call is failing with a 0 FP inside of
     * a lib$ routine which is walking down to the proper 
     * invocation.  Need to figure out why.
     *
     * Note: without this call there is no Cactus link on alpha_vms.
     */
    lib$put_invo_registers(invocation_handle, &invocation_context);
#endif

#endif


#if _CMA_PER_THD_SYNC_SIGS_
    cma__sig_thread_init ();
#endif

    status = cma_c_term_normal;		/* Assume it'll be normal */

    TRY {
	if (cma__g_debug_state.events_enabled)
	    cma__debevt_report (cma__c_debevt_activating);
	tcb->debug.substate = cma__c_substa_normal;
	value = (routine) (argument);
	tcb->alert.a_enable = cma_c_false;
	tcb->alert.g_enable = cma_c_false;
	cma__mp_barrier ();

	cma__trace ((
		cma__c_trc_obj,
		"(thread_base) thread 0x%lx completed start routine with 0x%lx",
		tcb,
		value));

	if (cma__g_debug_state.events_enabled)
	    cma__debevt_report (cma__c_debevt_terminating);

	}
    CATCH (cma_e_alerted) {
	tcb->alert.a_enable = cma_c_false;
	tcb->alert.g_enable = cma_c_false;
	cma__mp_barrier ();
	/*
	 * If the thread was terminated by an alert, set the return status
	 * appropriately.
	 */
	cma__trace ((
		cma__c_trc_obj,
		"(thread_base) thread 0x%lx was alerted",
		tcb));

	tcb->debug.substate = cma__c_substa_term_alt;
	status = cma_c_term_alert;

	if (cma__g_debug_state.events_enabled)
	    cma__debevt_report (cma__c_debevt_term_alert);

	}
    CATCH (cma_e_exit_thread) {
	tcb->alert.a_enable = cma_c_false;
	tcb->alert.g_enable = cma_c_false;
	cma__mp_barrier ();
	/*
	 * If the thread was terminated by a voluntary exit, set the
	 * appropriate return status and value.
	 */
	cma__trace ((
		cma__c_trc_obj,
		"(thread_base) thread 0x%lx exited with 0x%lx",
		tcb,
		tcb->return_value));

	tcb->debug.substate = cma__c_substa_term_exc;
	status = tcb->exit_status;
	value = tcb->return_value;

	if (cma__g_debug_state.events_enabled)
	    cma__debevt_report (cma__c_debevt_term_exc);

	}
    ENDTRY

    cma__assert_fail (
	    tcb == cma__get_self_tcb(),
	    "Thread is terminating with wrong TCB.");

    cma__trace ((
	    cma__c_trc_obj,
	    "(thread_base) thread 0x%lx (%d) cleaning up",
	    tcb, tcb->header.sequence));

    TRY {
	cma__cleanup_thread (tcb, value, status);
	cma__trace ((
		cma__c_trc_obj,
		"(thread_base) thread 0x%lx (%d) terminating",
		tcb, tcb->header.sequence));
	cma__terminate_thread (tcb);
	}
    CATCH_ALL {
	exc_report (THIS_CATCH);	/* Generate an error report */
	cma__bugcheck ("thread_base: exception during thread cleanup");
	}
    ENDTRY

    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_THREAD.C */
/*  *62   26-JUL-1993 13:35:20 BUTENHOF "Fix locking error in thread_create error path" */
/*  *61   11-JUN-1993 10:32:29 BUTENHOF "Disable async alerts on return from thread routine" */
/*  *60   14-MAY-1993 15:56:43 BUTENHOF "Detach needs to move terminated thread after g_last_thread" */
/*  *59    4-MAY-1993 11:38:37 BUTENHOF "Reduce locking in rundown" */
/*  *58    3-MAY-1993 13:44:51 BUTENHOF "Maintain known thread list" */
/*  *57   16-APR-1993 13:06:08 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *56    5-APR-1993 14:12:43 BUTENHOF "Remove VP zombie queue" */
/*  *55    1-APR-1993 14:33:31 BUTENHOF "Add names to user objects" */
/*  *54   31-MAR-1993 15:16:42 KEANE "Add self-deadlock detection to join (back propagate from OSF/1)" */
/*  *53   25-MAR-1993 14:43:40 BUTENHOF "Fix creation code" */
/*  *52   10-MAR-1993 13:56:15 BUTENHOF "Set new 'reset' field in VP" */
/*  *51    4-MAR-1993 14:33:23 BUTENHOF "fix bug" */
/*  *50    4-MAR-1993 12:23:52 BUTENHOF "Use priority table, and disallow pri 63 on ada_low & ada_rtb" */
/*  *49    3-MAR-1993 09:57:56 KEANE "Improve status/exception reporting for thread creation" */
/*  *48   29-JAN-1993 12:46:28 BUTENHOF "Don't change TCB sched until it works" */
/*  *47   21-JAN-1993 09:45:49 BUTENHOF "Allow overrun on RT priorities" */
/*  *46   19-JAN-1993 12:56:41 BUTENHOF "Add assertion that TCB is current thread to thread_base" */
/*  *45    7-JAN-1993 14:38:56 BUTENHOF "Fix vp_set_sched error checks" */
/*  *44    6-JAN-1993 15:31:04 BUTENHOF "Raise proper error for vp_set_sched" */
/*  *43   10-DEC-1992 14:34:23 BUTENHOF "Fix error propagation for prioritized thread creation on OSF/1" */
/*  *42    8-DEC-1992 15:14:53 BUTENHOF "Allow FIFO/RR prio about max on RT" */
/*  *41    1-DEC-1992 14:05:52 BUTENHOF "OSF/1 scheduling" */
/*  *40   13-OCT-1992 14:16:12 BUTENHOF "Remove vp_setsched call" */
/*  *39    7-OCT-1992 09:41:22 CURTIN "" */
/*  *38    6-OCT-1992 16:39:51 CURTIN "Add new cactus stack work" */
/*   28A1  6-OCT-1992 16:34:07 CURTIN "Add cactus stack changes" */
/*  *37   24-SEP-1992 08:56:59 SCALES "Add ""ada-rtb"" scheduling policy" */
/*  *36   16-SEP-1992 09:52:49 BUTENHOF "Conditionalize sched. policy switch" */
/*  *35   15-SEP-1992 13:50:42 BUTENHOF "Support Mach scheduling" */
/*  *34    3-SEP-1992 21:28:53 SCALES """Fix sched headerfile name""" */
/*  *33   13-AUG-1992 14:44:20 BUTENHOF "Set and use Alpha pal code unique value" */
/*  *32   12-AUG-1992 12:26:33 BUTENHOF "Remove reference to vp_count" */
/*  *31    5-AUG-1992 21:52:14 KEANE "Make yield on mach platforms avoid entering kernel" */
/*  *30    4-AUG-1992 13:57:24 BUTENHOF "Set saved SP to initial" */
/*  *29    4-AUG-1992 11:04:55 BUTENHOF "Fix int_make_thread" */
/*  *28   26-JUN-1992 14:00:53 CURTIN "Merge in 25A1 changes" */
/*   25A1 10-JUN-1992 22:49:05 SCALES "Apply fixes for BL10+" */
/*  *27    9-JUN-1992 15:01:17 KEANE "Make casts to (int) casts to (cma_t_integer)" */
/*  *26    9-JUN-1992 10:46:48 CURTIN "Add Alpha VMS cactus link in cma__thread_base" */
/*  *25   14-APR-1992 13:14:59 BUTENHOF "Remove exception handlers from thread_base" */
/*  *24   25-MAR-1992 09:34:38 SCALES "Fix to handle multiple detaches" */
/*  *23   18-MAR-1992 19:01:55 SCALES "Parameterize scheduling policies" */
/*  *22   13-MAR-1992 14:10:17 BUTENHOF "Remove excess thread_base arguments" */
/*  *21   18-FEB-1992 15:30:45 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *20   10-FEB-1992 08:51:41 BUTENHOF "Add thread creation trace" */
/*  *19   13-DEC-1991 09:53:56 BUTENHOF "one-plus exception termination: return status as value" */
/*  *18   14-OCT-1991 13:41:34 BUTENHOF "Add thread count, clean up uniproc/OSF conds" */
/*  *17   24-SEP-1991 16:29:10 BUTENHOF "Complete Tin optimizations" */
/*  *16   17-SEP-1991 13:24:36 BUTENHOF "Complete Tin optimizations" */
/*  *15    2-JUL-1991 16:52:59 BUTENHOF "Cache VPs with thread" */
/*  *14   10-JUN-1991 18:24:31 SCALES "Add sccs headers for Ultrix" */
/*  *13    2-JUN-1991 19:36:55 BUTENHOF "Add trace info to cleanup_thead" */
/*  *12   29-MAY-1991 17:45:57 BUTENHOF "Support OSF/1 per-thread signals" */
/*  *11   14-MAY-1991 13:44:15 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *10    3-MAY-1991 11:26:03 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *9    23-APR-1991 11:30:20 CURTIN "added kernel lock around known thread list access" */
/*  *8    12-APR-1991 23:37:06 BUTENHOF "OSF/1 Mach thread support" */
/*  *7     8-APR-1991 20:31:39 CURTIN "added some process abort changes" */
/*  *6     1-APR-1991 18:09:39 BUTENHOF "QAR 93, exception text" */
/*  *5    13-MAR-1991 14:22:43 BUTENHOF "Allow null pointers in thread_join" */
/*  *4     5-FEB-1991 00:59:57 BUTENHOF "Improve inlining of internal cv operations" */
/*  *3    24-JAN-1991 00:35:19 BUTENHOF "Remove assertion for cma_s_bugcheck" */
/*  *2    14-DEC-1990 00:56:02 BUTENHOF "Change module names" */
/*  *1    12-DEC-1990 21:54:25 BUTENHOF "Thread management" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_THREAD.C */
