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
static char *rcsid = "@(#)$RCSfile: cma_dispatch.c,v $ $Revision: 4.2.10.2 $ (DEC) $Date: 1993/08/18 14:46:55 $";
#endif
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	These routines provide the basic cma dispatcher
 *
 *  AUTHORS:
 *
 *	Hans Oser
 *	Webb Scales
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	1 September 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof & Webb Scales	27 September 1989
 *		Fix cma__init_dispatch to initialize dispatcher queues.
 *	002	Dave Butenhof & Webb Scales	27 September 1989
 *		Fix cma__unset to use interlocked instructions.
 *	003	Hans Oser			28 September 1989
 *		Fix wrong test for empty queues.
 *	004	Hans Oser			29 September 1989
 *		Fix time slice bug, add null-thread initialisation
 *		and function cma__get_async_info.
 *	005	Hans Oser and Webb Scales	03 October 1989
 *		Add ctx_buffer parameter to cma__get_async_info
 *	006	Hans Oser			06 October 1989
 *		Timer support transferred to module cma_timer
 *	007	Hans Oser			13 October 1989
 *		Changements due to program review
 *	008	Hans Oser			17 October 1989
 *		To correct loss of thread in cma__terminate_thread
 *	009	Dave Butenhof			19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *	010	Dave Butenhof			19 October 1989
 *		Replace #pragmas with use of new cma_host.h functions.
 *	011	Webb Scales			20 October 1989
 *		Changed "cma_assem_vms.h" to "cma_assem.h".  Added typecasts.
 *	012	Webb Scales			20 October 1989
 *		Placed definition (ie, type and initializer) of 
 *		cma__g_kernel_critical in this file and placed 
 *		the declaration (ie, "extern") in cma_dispatch.h
 *	013	Webb Scales			23 October 1989
 *		Changed definiton of cma__force_dispatch to a routine, changed
 *		"..._PSL" to "..._ps"
 *	014	Dave Butenhof			Halloween 1989
 *		Implement queue of all threads (chained at create_thread
 *		time) so debug code can find them all.
 *	015	Dave Butenhof			All Saints Day 1989
 *		Remove cma__enter_kernel (it's now a macro).
 *	016	Dave Butenhof			1 November 1989
 *		Move known TCB init to cma__init_static routine for
 *		consistency with other known foo inits.
 *	017	Dave Butenhof			2 November 1989
 *		Fix init to avoid doubly-inserting default TCB on known
 *		threads queue.
 *	018	Dave Butenhof and Bob Conti	3 November 1989
 *		Fix undefer logic to avoid deadlocking on entry from
 *		cma__assign; also change name of "dispatch_defer" flag to
 *		"do_redispatch" for clarity.
 *	019	Webb Scales			3 November 1989
 *		Revamp cma__defer and cma__undefer.
 *	020	Webb Scales			3 November 1989
 *		Fix cma___t_defer_q_entry declaration, and cma__t_semaphore
 *		typecasts in cma__incr_sem arguments.
 *	021	Webb Scales			3 November 1989
 *		Make call to "cma_error" into "cma__error"
 *	022	Bob Conti			5 November 1989
 *		Add undefer calls to *all* callers of cma__assign to 
 *		compensate for our having moved undefer out of cma__assign.
 *		Remove the now-superfluous include of cma_host.
 *		Replace inefficient per-dispatch time-stamping by 
 *		incrementing the current thread's runtime in cma__periodic.
 *		Delete middle (time) parameter from cma__assign.
 *		Add the flag defers_processed that prevents useless calls to 
 *		undefer.
 *	023	Dave Butenhof			6 November 1989
 *		Fix some comments (especially function descriptions).
 *	024	Webb Scales & Bob Conti		6 November 1989
 *		Made cma__ready into a macro and put the quatum increment into
 *		it.  Remove parameter from cma__dispatch.  Removed kernel lock 
 *		form cma__AST_forced_dispatch.  Renamed cma__dispatch to 
 *		cma__yield_processor.  Renamed cma__assign to cma__dispatch.
 *	025	Webb Scales	7 November 1989
 *		Changed name of cma__AST_force_dispatch to 
 *		cma__cause_force_dispatch.  Added include of cma_vm.h
 *	026	Dave Butenhof	17 November 1989
 *		Changes to comply with new timer module.
 *	027	Webb Scales	18 November 1989
 *		Changed interface to cma__get_async_info to single parameter.
 *		Made parameters to cma__cause_force_dispatch system specific.
 *	028	Webb Scales	19 November 1989
 *		Add include for signal header file on ultrix.
 *	029	Dave Butenhof	21 November 1989
 *		Fix debugging hooks so "threads" command can work.
 *	030	Dave Butenhof	1 December 1989
 *		Initialize scheduling policy on null thread.
 *	031	Webb Scales	10 December 1989
 *		Add deferal action for Unix non-blocking I/O.
 *	032	Dave Butenhof	11 December 1989
 *		Fix deferral... since defer advances before inserting,
 *		undefer needs to advance before removing.
 *	033	Webb Scales	11 December 1989
 *		Re-alphabetized the routines.
 *		Moved the call to cma__undefer to inside cma__dispatch.
 *		Fixed undefer to advance before removing.
 *	034	Webb Scales	12 December 1989
 *		Corrected cma__augment_defer_queue, so that it now works.
 *	035	Webb Scales & Dave Butenhof	14 December 1989
 *		undefer unblock_all should generate pending spurious wakeup.
 *	036	Dave Butenhof	26 January 1990
 *		Change cma__get_self_tcb_kernel to cma__get_self_tcb (it no
 *		longer locks the kernel, so distinction is irrelevant).
 *	037	Dave Butenhof	13 February 1990
 *		Context switch errno global cell.
 *	038	Dave Butenhof	26 February 1990
 *		Change use of priority symbol to new standard.
 *	039	Dave Butenhof	2 March 1990
 *		Integrate Kevin Ackley's changes for Apollo M68000 port.
 *	040	Dave Butenhof	21 March 1990
 *		Make null thread do pause in loop.
 *	041	Webb Scales & Dave Butenhof	29 March 1990
 *		Make null thread $hiber on VAX/VMS and call 'select' on
 * 		U*ix.
 *	042	Dave Butenhof	6 April 1990
 *		Null thread must RERAISE debug exception.
 *	043	Dave Butenhof	9 April 1990
 *		Back off 042---TRY/ENDTRY catching doesn't work for
 *		continuable exceptions, and correct fix is now in exception
 *		code (SS$_DEBUG isn't passed to C exception package).
 *	044	Dave Butenhof	9 April 1990
 *		Use new "known_object" structure for known condition queue
 *		(includes mutex).
 *	045	Dave Butenhof & Webb Scales	9 April 1990
 *		Implement zombies and anti-zombie spells.  This allows a
 *		terminating thread to survive until context switched out,
 *		while ensuring that it's eventually dispelled by the
 *		appropriate incantation.
 *	046	Webb Scales	30 May 1990
 *		Add reference count to tcb for joiners to prevent zombies from
 *		being dispelled prematurely.
 *	047	Webb Scales	15 June 1990
 *		Added priority scheduling:
 *		- Made the ready list an array of queue headers.
 *		- Added global "next ready queue" pointer.
 *		- Changed cma__ready from a macro to a routine.
 *		- Modified dispatch routine.
 *		- Added a bunch of local macros.
 *		- Added defer's for tick updates
 *	048	Webb Scales	26 June 1990
 *		Fixed a bug which occurs when a thread dispatches to itself.
 *	049	Webb Scales	25 July 1990
 *		Moved defer functions to their own module.
 *	050	Dave Butenhof	14 August 1990
 *		Generalize cma__cause_force_dispatch so it can be used for
 *		asynchronous alerts as well as timeslicing.
 *	051	Webb Scales	15 August 1990
 *		Incorporated Apollo changes, adding HPPA and SUN 68K platforms.
 *	052	Bob Conti	9 September 1990
 *		Add code to report key state-transition events to the debugger.
 *	053	Dave Butenhof	28 November 1990
 *		Integrate IBM RIOS changes provided by Dave Mehaffy.
 *	054	Dave Butenhof	5 February 1991
 *		Rename "errno" field.
 *	055	Dave Butenhof	6 February 1991
 *		Modify null thread loop to attempt to yield the processor
 *		before returning to $hiber (on VAX/VMS). This allows the
 *		cma_cond_signal_int function to get better performance (when
 *		other threads may be idle) by performing a $wake.
 *	056	Webb Scales	18 March 1991
 *		Changed priority-adjusting scheduling to do priority calculation
 *		only when there is new "data".
 *	057	Dave Butenhof	25 March 1991
 *		Change exception interface names
 *	058	Dave Butenhof	05 April 1991
 *		Support parallel processing via Mach threads...
 *		_CMA_KTHREADS_ defines type of kernel threads (currently,
 *		_CMA__MACH and _CMA__NONE are supported). _CMA_MULTIPLEX_
 *		defines whether user->kernel thread mapping is one-on-one or
 *		multiplexed (currently only one-on-one is supported). The
 *		_CMA_MULTIPLEX_ symbol must be 0 if _CMA_UNIPROCESSOR_ is
 *		true (so it's really "multiprocessor-multiplexed"). The
 * 		symbol _CMA_THREAD_IS_VP_ is set when kernel thread mapping
 *		is one-to-one, for convenience.
 *	059	Paul Curtin	15 April 1991
 *		Added include of thread.h
 *	060	Dave Butenhof	13 May 1991
 *		Use cma__trace_kernel() where kernel is locked, to avoid a
 *		deadlock.
 *	061	Dave Butenhof	23 May 1991
 *		Under OSF/1 systems, the default thread/VP needs to set up
 *		default handling for the per-thread signals by calling
 *		cma__sig_thread_init().
 *	062	Paul Curtin	31 May 1991
 *		Added a reinit routine.
 *	063	Dave Butenhof	31 May 1991
 *		Change locking in cma__terminate_thread.
 *	064	Dave Butenhof	06 June 1991
 *		Integrate IBM reverse-drop changes (RS/6000 path of
 *		cma__cause_force_dispatch()).
 *	065	Webb Scales and Dave Butenhof	    10 June 1991
 *		Conditionalize inclusion of I/O stuff.
 *	066	Dave Butenhof	05 September 1991
 *		Add call to new function cma__vp_yield() to implement
 *		cma__yield_processor() on VP-based DECthreads.
 *	067	Dave Butenhof	18 September 1991
 *		Merge Apollo CMA5 reverse drop. They reimplement the standard
 *		DECthreads logic to switch to main thread and deliver a
 *		terminating signal; instead of using a signal exit trampoline
 *		function, force the next context switch to the main thread,
 *		and when the main thread is scheduled check for a pending
 *		termination signal (this isn't an optimal general solution,
 *		since it adds a test on the "fast path", so I'll leave it in
 *		Apollo condition code).
 *	068	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	069	Paul Curtin	18 November 1991
 *		Alpha work: added include for stdlib and an Alpha switch.
 *	070	Dave Butenhof	27 November 1991
 *		Modify errno context switch code for DEC C.
 *	071	Dave Butenhof	05 December 1991
 *		For a little recreational diversion from Alpha, implement
 *		priority-ordering of realtime threads on a block queue (i.e.,
 *		for mutex lock and condition wait).
 *	072	Dave Butenhof	09 January 1992
 *		Support Alpha hardware in cma__cause_force_dispatch.
 *	073	Dave Butenhof	21 January 1992
 *		Add support for DEC OSF/1 shared library version by saving
 *		some extra registers picie needs in cause_force_dispatch.
 *	074	Dave Butenhof	24 January 1992
 *		Fix a type mismatch (MIPS C) on 019.
 *	075	Webb Scales	28 January 1992
 *		Reworked end-of-quantum preemption on VMS.
 *	076	Dave Butenhof	30 January 1992
 *		Remove cma__trace_kernel calls.
 *	077	Webb Scales	30 January 1992
 *		- Implement our own private $CLRAST.
 *		- Move the undefer which is done during dispatching so that
 *		  it is done before the out-bound thread is queued anywhere.
 *		- Repaired the damage to cause-force-dispatch on VAX/VMS so
 *		  that async alerts will work...until we do them for Alpha.
 *	078	Webb Scales and Dave Butenhof	6 February 1992
 *		Removed unneeded initialization of a/synch contexts
 *	079	Dave Butenhof	10 February 1992
 *		Call cma__vp_dump() before bugcheck if cma__vp_delete()
 *		returns. Also, adjust to new VM protocol.
 *	080	Webb Scales	18 February 1992
 *		- Make code for picie conditionally compiled for picie only.
 *		- Change the exit-kernel to unset-kernel in the Apollo-specific
 *		    abort-process code.
 *		- Removed explicit calls to undefer.
 *	081	Webb Scales	25 February 1992
 *		Reorganized the errno-switching code.
 *	082	Dave Butenhof	10 March 1992
 *		Change timeb field references to timeval, since
 *		cma_t_date_time has been changed to avoid requiring libbsd.a
 *		on AIX and OSF/1.
 *	083	Dave Butenhof	12 March 1992
 *		Use correct symbol in reinit code rather than literal "2".
 *	084	Webb Scales	13 March 1992
 *		Parameterize scheduling policies.
 *	085	Dave Butenhof	19 March 1992
 *		Add call to cma__undefer() before all direct calls to
 *		cma__yield_processor() -- since cma__ready() no longer
 *		undefers.
 *	086	Webb Scales	31 March 1992
 *		Rework asynch context switch for U*ix.
 *	087	Dave Butenhof	15 April 1992
 *		Add check for resheduled terminated thread, and bugcheck.
 *	088	Dave Butenhof	20 April 1992
 *		Modify 033 -- bugcheck only if state is terminated; the
 *		terminated flag is unreliable (it's set without kernel
 *		interlock, and is only used as a predicate for join).
 *	089	Webb Scales	18 May 1992
 *		Initialize temp queue headers in sort-ready-queues.
 *	090	Webb Scales	19 May 1992
 *		Cleanse the null thread's queue links during re-init after fork.
 *	091	Dave Butenhof	04 August 1992
 *		Clear vpid when VP is deleted.
 *	092	Dave Butenhof	11 August 1992
 *		Remove the final remains of cma__ready() for thread_is_vp
 *		case, since it doesn't do anything useful.
 *	093	Dave Butenhof	13 August 1992
 *		For Alpha OSF/1, initialize main thread's PAL unique value.
 *	094	Dave Butenhof	14 August 1992
 *		Fix locking bug in dispell_zombies. Add info to some bugchecks.
 *	095	Dave Butenhof	28 August 1992
 *		Break up cma__block into macros to save time.
 *		Also, change several scheduler functions so they don't
 *		require kernel to be locked for _CMA_THREAD_IS_VP_.
 *	096	Webb Scales	 3 September 1992
 *		Moved most of the scheduling (as opposed to dispatching) code
 *		to cma_sched.c.
 *	097	Webb Scales	23 September 1992
 *		Update the current VP's current TCB pointer during context 
 *		switch.
 *	098	Webb Scales	23 September 1992
 *		Make low priority Ada policy scheduling uniform with the rest
 *		of the background class.
 *		Also, add a name for the "last thread" condition variable.
 *	099	Dave Butenhof	15 October 1992
 *		On a kernel thread implementation, unlock the kernel during
 *		thread termination, before calling cma__dispatch (which no
 *		longer expects the kernel to be locked for kthreads).
 *	100	Webb Scales	19 November 1992
 *		Added code to the context switch which will swap the TIS
 *		table of errno cells.
 *	101	Brian Keane	19 March 1993
 *		Fix problem in VP-specific logic in dispell_zombies - 
 *		check vp_id for null, not non-null.
 *	102	Dave Butenhof	25 March 1993
 *		Fix trace format strings to use "0x%lx" rather than "%08x".
 *	103	Dave Butenhof	 5 April 1993
 *		Change zombie VP processing: instead of using "reset" pointer
 *		for asynch clear by VP layer, use new synchronous call
 *		cma__vp_reclaim() from dispell_zombies.
 *	104	Webb Scales	 7 April 1993
 *		Conditionalize clearing a field in the TCB-VP structure so that
 *		it is only done if there are VPs.
 *	105	Dave Butenhof	12 April 1993
 *		Add argument to cma__int_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	106	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	107	Dave Butenhof	27 April 1993
 *		Add trace call to dispell_zombies, and fix logic: try to
 *		reclaim the VP even if there are joiners.
 *	108	Dave Butenhof	30 April 1993
 *		More zombie tracing to track down OSF/1 problem...
 *	109	Dave Butenhof	30 April 1993
 *		Put terminated threads on zombie list unconditionally
 *		(instead of only detached threads), so that VPs can be
 *		reclaimed.
 *	110	Dave Butenhof	 3 May 1993
 *		Reverse 109: instead, leave zombie threads on known thread
 *		list (moving them to the end). This satisfies the requirement
 *		that lead to 109 (being able to reclaim undetached VPs), but
 *		also allows retaining existing VMS DEBUG interface
 *		capabilities.
 *	111	Dave Butenhof	 4 May 1993
 *		Fix queue corrupter in dispell_zombies (it now uses known
 *		thread list, with tcb->threads link, but was still requeueing
 *		unreclaimed thread on tcb->header.queue).
 *	112	Dave Butenhof	11 May 1993
 *		I removed a "dirty hack" in queue manipulation from
 *		dispell_zombies (inserting local queue header onto list, then
 *		removing it)
 *	113	Dave Butenhof	14 May 1993
 *		Track first dead thread rather than last living thread
 *		(cma__g_last_thread is now cma__g_dead_zone) -- otherwise,
 *		destroy_tcb and free_tcb would have to enter kernel and check
 *		pointer to avoid moving last_thread onto cache or pool list!
 *	114	Dave Butenhof	17 May 1993
 *		VAX ULTRIX can't handle comparisons other than "==" and "!="
 *		on enums, so cast new "<" and ">=" comparisons to (int)
 *		[due to addition of "zombie" state].
 *	115	Dave Butenhof	26 May 1993
 *		Fix dispatch bugcheck (when no ready thread is found, don't
 *		say "no vp").
 *	116	Dave Butenhof	28 May 1993
 *		Change name of "last thread" CV for consistency.
 *	117	Dave Butenhof	 4 June 1993
 *		Ah ha! Stupid bug in the spell to dispell zombies caused a
 *		thread to be buried while its motor was still running.
 *	118	Dave Butenhof	11 June 1993
 *		Another magic bug -- kernel threads always need to go at the
 *		front of the dead zone, so the VP can be reclaimed (how'd
 *		this get broken?)
 *	119	Brian Keane	1 July 1993
 *		Minor touch-ups to eliminate warnings with DEC C on OpenVMS AXP.
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_queue.h>
#include <cma_stack.h>
#include <cma_tcb.h>
#include <cma_assem.h>
#include <cma_dispatch.h>
#include <cma_timer.h>
#include <cma_vm.h>
#include <cma_int_errno.h>
#if _CMA_OS_ == _CMA__UNIX
# include <signal.h>
# if !_CMA_THREAD_SYNC_IO_
#  include <cma_thread_io.h>
# endif
#endif
#include <cma_deb_core.h>
#include <cma_deb_event.h>
#include <cma_sched.h>
#include <cma_sched_defs.h>
#include <cma_vp.h>
#include <cma_defer.h>
#include <cma_debug_client.h>
#include <cma_thread.h>
#include <cma_defer.h>
#include <cma_kernel.h>
#include <stdlib.h>

/*
 * GLOBAL DATA
 */

/*
 * Array of ready queues
 */
cma__t_queue	cma__g_ready_list[cma__c_prio_n_tot];

/*
 * Next ready queue to check
 */
cma_t_integer	cma__g_next_ready_queue = 0;

cma_t_integer	cma__g_threadcnt;	/* Number of threads */

/*
 * LOCAL DATA
 */

static cma__t_int_cv	*cma___g_lastthread;	/* Wait for last thread */

/*
 * LOCAL MACROS
 */

/*
 * Number of null threads (one-to-one mapped kernel thread implementations
 * generally don't have any; n to m mapped kernel thread implementations
 * might need more than one).
 */
#if _CMA_THREAD_IS_VP_
# define cma___c_null_thd_cnt	0
#else
# define cma___c_null_thd_cnt	1
#endif

/*
 * LOCAL FUNCTIONS
 */


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine is called by a signal or AST service routine to initiate
 *	an asynchronous context switch.
 *
 *	First, any context which is not readily available outside of the
 *	signal or AST environment is saved.  Then the return PC and processor
 *	status are set so that, when the signal or AST routine returns,
 *	execution will resume in the asynchronous context switch code, instead
 *	of at the point of interruption, with the processor in a known state.
 *
 *  FORMAL PARAMETERS:
 *
 *	cur_tcb	    The address of the current thread's TCB (read only)
#if (_CMA_OS_ == _CMA__UNIX) && (_CMA_HARDWARE_ != _CMA__VAX)
 *	scp	    The address of the signal context block.  Several fields
 *			in this block are copied into the TCB, and the sc_pc
 *			field is set to the address of cma__force_dispatch.
#endif
#if _CMA_HARDWARE_ == _CMA__VAX
 *	saved_pc    The address of the instruction at the point of 
 *			interruption.  The value of this parameter is 
 *			stored in the TCB and replaced by the address of 
 *			cma__force_dispatch.
 *
 *	saved_ps    The value of the processor status register at the point of 
 *			interruption.  The value of this parameter is stored 
 *			in the TCB and replaced by a known value.
#endif
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
void
cma__cause_force_dispatch
#if (_CMA_OS_ == _CMA__UNIX) && (_CMA_HARDWARE_ != _CMA__VAX) && (_CMA_HARDWARE_ != _CMA__ALPHA)
# ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	    *cur_tcb,
	cma_t_address	    target_pc,
	struct sigcontext   *scp)
# else
	(cur_tcb, target_pc, scp)
	cma__t_int_tcb	    *cur_tcb;
	cma_t_address	    target_pc;
	struct sigcontext   *scp;
# endif
#endif
#if (_CMA_HARDWARE_ == _CMA__VAX) || (_CMA_HARDWARE_ == _CMA__ALPHA)
# ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	    *cur_tcb,
	cma_t_address	    target_pc,
	cma_t_address	    *saved_pc,
	cma_t_address	    *saved_ps)
# else
	(cur_tcb, target_pc, saved_pc, saved_ps)
	cma__t_int_tcb	    *cur_tcb;
	cma_t_address	    target_pc;
	cma_t_address	    *saved_pc;
	cma_t_address	    *saved_ps;
# endif
#endif
    {
    cma__assert_fail(
	    cma__tac_isset (&cma__g_kernel_critical),
	    "In cma__cause_force_dispatch with kernel unlocked");

#if 0
#if _CMA_HARDWARE_ == _CMA__MIPS
    /*
     * Save the information which we have now but will need later, when it is
     * hard to get to.
     */
    cur_tcb->async_ctx.sig_mask     = (long int)scp->sc_mask;	/* Save signal mask */
    cur_tcb->async_ctx.restart_pc   = (long int)scp->sc_pc;	/* Save PC at signal */
    cur_tcb->async_ctx.used_fpc     = (long int)scp->sc_ownedfp;	/* Used the FPC */
    cur_tcb->async_ctx.fpc_csr      = (long int)scp->sc_fpc_csr;	/* The FPC C/S reg */
    cur_tcb->async_ctx.fpc_eir      = (long int)scp->sc_fpc_eir;	/* The FPC EI reg */
    cur_tcb->async_ctx.cp0_cause    = (long int)scp->sc_cause;	/* The CP0 cause reg */
    cur_tcb->async_ctx.cp0_badvaddr = (long int)scp->sc_badvaddr;	/* The CP0 bad VA */
    cur_tcb->async_ctx.cpu_badpaddr = (long int)scp->sc_badpaddr;	/* The CPU bd bad PA */

#ifdef  _CMA_SHLIB_
    /*
     * Save some context which picie modifies before we have a chance to
     * stash it in cma__force_dispatch. Also set up the $r25 register for the
     * call into cma__force_dispatch since picie assumes it has been entered
     * via a "jal ra, r25".
     */
    cur_tcb->async_ctx.t9           = (long int)scp->sc_regs[25];
    cur_tcb->async_ctx.gp           = (long int)scp->sc_regs[28];
    scp->sc_regs[25]                = (long int)target_pc;
#endif

    /*
     * Cause execution to resume at cma__forced_dispatch instead of at the
     * point of interruption.
     */
    *((cma_t_address *)(&scp->sc_pc)) = target_pc;
#endif
#if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
	{
	int toc = cma__get_toc ();

	bcopy(scp, &cur_tcb->async_ctx.ibmr2_scp, sizeof(struct sigcontext));
	scp->sc_jmpbuf.jmp_context.iar = *(long *)target_pc;
	toc = cma__get_toc();
	scp->sc_jmpbuf.jmp_context.gpr[1] =
	    &cur_tcb->ctx_stack[cma__c_ibmr2_ctx_stack_top];
	scp->sc_jmpbuf.jmp_context.gpr[2] = toc;
	}
#endif
#if (_CMA_HARDWARE_ == _CMA__VAX) || (_CMA_HARDWARE_ == _CMA__ALPHA)
# if _CMA_OS_ != _CMA__VMS
    /*
     * Save the interrupted's thread PC and PS in its tcb.
     */
    cur_tcb->async_ctx.pc  = (long int)*saved_pc;
    cur_tcb->async_ctx.psl = (long int)*saved_ps;
# endif

    /*
     * Cause execution to resume at cma__forced_dispatch instead of at the
     * point of interruption (after placing the processor in a known state).
     */
    *saved_pc = target_pc;
    *saved_ps = (cma_t_address)cma__c_default_ps;
#endif
#if _CMA_HARDWARE_ == _CMA__M68K
    cur_tcb->async_ctx.restart_pc   = scp->sc_pc;	/* Save PC at signal */
    cur_tcb->async_ctx.restart_ccr  = (short)scp->sc_ps;	/* Save CCR at signal */

    /*
     * Cause execution to resume at cma__forced_dispatch instead of at the
     * point of interruption.
     */
    *((cma_t_address *)(&scp->sc_pc)) = target_pc;
#endif
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine resumes the next ready thread
 *
 *  FORMAL PARAMETERS:
 *
 *	old_tcb		The current running TCB (which is being replaced)
 *
 *	save_context	cma_c_true causes current context to be saved.
 *			cma_c_false is used if the current context has already
 *			been saved (a timesliced thread, possibly) or if the
 *			context need not be saved (a terminating thread).
 *	milliseconds	Number of milliseconds to block before timeout
 *			(used for timed semaphore wait in VP implementation)
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
 *	cma_c_true if returns normally; cma_c_false if suspend timed out.
 *
 *  SIDE EFFECTS:
 *
 *	The first thread from the highest priority ready list will become
 *	the running thread.
 */
extern cma_t_boolean
cma__dispatch
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*old_tcb,
	cma_t_boolean	save_context,
	cma_t_integer	milliseconds)
#else	/* no prototypes */
	(old_tcb, save_context, milliseconds)
	cma__t_int_tcb	*old_tcb;
	cma_t_boolean	save_context;
	cma_t_integer	milliseconds;
#endif	/* prototype */
    {
    cma_t_boolean	status;
    cma_t_address	*qtmp;


#if !_CMA_THREAD_IS_VP_
    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "In cma__dispatch with kernel unlocked");

    cma__sched_update_time (
	    old_tcb->sched.cpu_time, 
	    old_tcb->sched.cpu_ticks + 1);


    while (cma__queue_empty (&cma__g_ready_list[cma__g_next_ready_queue])) {

	if (cma__g_next_ready_queue > 0)
	    cma__g_next_ready_queue--;
	else {
# if _CMA_MULTIPLEX_
	    cma__queue_remove (
		    &old_tcb->sched.processor->queue,
		    qtmp,
		    cma_t_address);
	    cma__exit_kernel ();

	    if (cma__vp_delete (old_tcb->sched.processor->vp_id) != cma__c_vp_normal)
		cma__bugcheck ("dispatch: VP delete failed");

	    cma__enter_kernel ();
# else
	    cma__bugcheck (
		    "Can't find null thread (context switch from interrupt?)");
# endif
	    }

	}

# if _CMA_VENDOR_ == _CMA__APOLLO
    /*
     * If there's an abort signal pending, we're going to switch to the
     * default thread no matter what.  Otherwise, pick the next ready
     * thread.
     */
    if (cma__g_abort_signal != 0) {
        cma__g_current_thread = &cma__g_def_tcb;
        }
    else 
# endif
	cma__queue_dequeue (
		&cma__g_ready_list[cma__g_next_ready_queue],
		cma__g_current_thread,
		cma__t_int_tcb);

    if ((int)cma__g_current_thread->state >= (int)cma__c_state_terminated)
	cma__bugcheck ("dispatch: about to dispatch to terminated thread");

    cma__g_current_thread->state = cma__c_state_running;
    cma__g_current_thread->sched.cpu_ticks = 0;

    /*
     * If there were no other ready threads of higher priority, then we are
     * rescheduling the thread which just ran, and there is no more work to 
     * do.  Otherwise, save and restore thread states, etc.
     */
    if (cma__g_current_thread != old_tcb) {
	/*
	 * Move the per-processor data to the new tcb.
	 */
	cma__g_current_thread->sched.processor = old_tcb->sched.processor;
	cma__g_current_thread->sched.processor->current_thread = 
	    cma__g_current_thread;
	old_tcb->sched.processor = (cma__t_vp *)cma_c_null_ptr;

/* 
 * FIX-ME: At some point we will compile this file with and 
 * without the _CMA_DEBEVT_ flag defined, so that there can be
 * absolutely no overhead for reporting debugging events when the debugger 
 * until the debugger is first invoked by the user.  This will require
 * more work.  For now, the run-time check for any-debug-events is an
 * 80% solution.
 */
# ifdef _CMA_DEBEVT_
	/*
	 * Remember information in the new thread about what thread got 
	 * preempted from the processor. The preemption event is reported
	 * except when the prior thread voluntarily blocked or terminated.
	 */
	if (cma__g_debug_state.events_enabled
		&& ((int)old_tcb->state < (int)cma__c_state_blocked)) {
	    cma__g_current_thread->debug.did_preempt = cma_c_true;
	    cma__g_current_thread->debug.preempted_tcb = old_tcb;
	    }
# endif

	/*
	 * Save the current value of the global errno cell into the outgoing 
	 * TCB, and set it from the incoming TCB.  This is a uniprocessor 
	 * "compromise" to full support for per-thread errno (which requires 
	 * all libraries and client code to use a new errno macro that 
	 * references the TCB cell directly).
	 */
# if _CMA_ERRNO_TYPE_ != _CMA__INTERN_FUNC
	old_tcb->thd_errno = cma__get_errno();
	cma__set_errno (cma__g_current_thread->thd_errno);
#  if _CMA_OS_ == _CMA__VMS
	old_tcb->thd_vmserrno = vaxc$errno;
	vaxc$errno = cma__g_current_thread->thd_vmserrno;
#  endif
# else
#  if _CMA_OS_ == _CMA__VMS
	/*
	 * On VMS, the TIS image manages a list of errno cells which are used
	 * (in addition to the per-thread errno cell) by old code.  Save and 
	 * restore each one if there are any.
	 */
	if (*cma__g_errno_tbl) {
	    cma_t_errno **addr_tbl;   /* Pointer into TIS table of addresses */
	    cma_t_errno *old_val_tbl; /* Pointer into old tcb's table of vals */
	    cma_t_errno *new_val_tbl; /* Pointer into new tcb's table of vals */

	    old_val_tbl = old_tcb->errno_tbl;
	    new_val_tbl = cma__g_current_thread->errno_tbl;
	    for (addr_tbl = cma__g_errno_tbl; *addr_tbl; addr_tbl++) {
		*old_val_tbl++ = **addr_tbl;
		**addr_tbl = *new_val_tbl++;
		}

	    old_val_tbl = old_tcb->vmserrno_tbl;
	    new_val_tbl = cma__g_current_thread->vmserrno_tbl;
	    for (addr_tbl = cma__g_vmserrno_tbl; *addr_tbl; addr_tbl++) {
		*old_val_tbl++ = **addr_tbl;
		**addr_tbl = *new_val_tbl++;
		}
	    }
#  endif
# endif

	/*
	 * If this dispatch is an asynchronous context switch (the result of an 
	 * end-of-quantum event) then we are currently running at AST level.
	 * If we are switching to a thread which was switched out asynchronously
	 * then everything is fine: restore it and let it return from the AST.
	 * But if we are switching to a thread which was switched sychronously
	 * then we need to get rid of the AST.
	 */
	if (old_tcb->async_ctx.valid)
	    if (!cma__g_current_thread->async_ctx.valid) {
		/* 
		 * Async-to-Sync:
		 *
		 * "Dismiss" the interrupt context.  (Allow other interrupts
		 * to be delivered.)
		 */
		cma__clear_interrupt (old_tcb->async_ctx.interrupt_ctx);
		}
# if _CMA_OS_ == _CMA__UNIX
	    else {
		/*
		 * Async-to-Async:
		 *
		 * The thread that we are switching to was also interrupted.
		 * As part of restoring it, the system will reset the process
		 * signal mask, so provide it with an appropriate value.
		 */
		*(sigset_t *)cma__g_current_thread->async_ctx.interrupt_ctx = 
		    *(sigset_t *)old_tcb->async_ctx.interrupt_ctx;
		}
	else
	    if (cma__g_current_thread->async_ctx.valid) {
		/* 
		 * Sync-to-Async:
		 *
		 * The thread that we are switching to was also interrupted.
		 * As part of restoring it, the system will reset the process
		 * signal mask, so provide it with an appropriate value.
		 */
		if (sigprocmask (
			SIG_SETMASK, 
			cma_c_null_ptr, 
			cma__g_current_thread->async_ctx.interrupt_ctx) == -1)
		    cma__bugcheck("cma_dispatch: Sync-to-Async");
		}

	    /*
	     * Note that no action is required for the Sync-to-Sync case.
	     */
# endif

	/*
	 * Switch to new thread context
	 */
	if (save_context)
	    cma__transfer_thread_ctx (
		    &old_tcb->static_ctx,
		    &cma__g_current_thread->static_ctx);
	else
	    cma__restore_thread_ctx (
		    &cma__g_current_thread->static_ctx);

# ifdef _CMA_DEBEVT_
	
	/* 
	 * We are about to run.  Notify the debugger of the interesting 
	 * dispatch events.
	 * 
	 * We must report the debugging events here because they must
	 * be reported in the about-to-run thread after it is on its
	 * own stack.
	 * 
	 * NOTES: 
	 * 1) A  "minor" discrepancy in new state is unavioidable due to the 
	 *    fact that we must borrow a few of the thread's registers to
	 *    implement events! The important thing to get right is the stack.
	 * 2) Assumptions: the new thread called cma__dispatch previously to 
	 *    save its context; declaring the event will not destroy any
	 *    aspect of the thread's state unless the user explicitly 
	 *    does it (declaring an event must be "modular").
	 * 3) For now, as a uniform rule, events are all reported while in 
	 *    the kernel.  
	 *    This limits what the user can do when an event is reported, but 
	 *    declaring events outside the kernel is difficult to 
	 *    implement. *WARNING* don't try to leave the kernel and 
	 *    then reenter it to report events; this can break our callers
	 *    who expect that the kernel lock will be held while in 
	 *    the context of the thread that called cma__dispatch.
	 */

	/*
	 * If the debugger created an inconsistency in the dispatch
	 * database, then yield the processor, which knows how to 
	 * clean things back up.  Note this enforces the invariant that
	 * we never leave dispatch with an inconsistency.
	 */
	while (cma__g_debug_state.is_inconsistency) {
	    cma__undefer ();
	    cma__yield_processor ();
	    }


	if (cma__g_debug_state.events_enabled) {
	    cma__t_int_tcb	*cur_tcb;

	    cur_tcb = cma__get_self_tcb ();

	    /* 
	     * If we were requested to run by debugger, checkin.
	     */
	    if (cur_tcb->debug.notify_debugger) {
		cur_tcb->debug.notify_debugger	= cma_c_false;
		cma__debevt_notify ();
		};		

	    /* 
	     * Report the preempting event
	     */
	    if (cur_tcb->debug.did_preempt) {
		cur_tcb->debug.did_preempt = cma_c_false;
		cma__debevt_report (cma__c_debevt_preempting);
		}

	    /* 
	     * Report the running event
	     */
	    cma__debevt_report (cma__c_debevt_running);

	    }
# endif
	}
#else					/* else _CMA_THREAD_IS_VP_ */
    /*
     * FIX-ME: Currently, all of the DEBEVT code is inside the multiplexing
     * conditionals; OK since we don't have debug event support except on
     * VMS yet anyway, but we need to think through the debugging
     * mechanisms for kernel thread systems!
     */

    /*
     * If we're mapping threads one-to-one, then we need to suspend the VP
     * until we're unblocked.
     */
    if (save_context) {
	status = cma__vp_suspend (old_tcb->sched.processor->vp_id, milliseconds);

	if (status == cma__c_vp_normal || status == cma__c_vp_err_timeout)
	    return (status == cma__c_vp_normal);
	else
	    cma__bugcheck ("dispatch: failed to suspend VP");

	}
    else {
	cma__trace ((
		cma__c_trc_vp | cma__c_trc_obj,
		"(dispatch) thread %d(0x%lx) [vp %d] about to delete itself",
		old_tcb->header.sequence,
		old_tcb,
		old_tcb->sched.processor->vp_id->vp));
	cma__vp_delete (old_tcb->sched.processor->vp_id);
	cma__bugcheck ("dispatch: delete failed to terminate execution");
	}

#endif

#if _CMA_VENDOR_ == _CMA__APOLLO
    /*
     * If there's an abort signal pending and we're the default thread, 
     * abort the process now!
     */
    if (cma__g_abort_signal != 0 && cma__g_current_thread == &cma__g_def_tcb) {
	cma__unset_kernel ();
        cma__abort_process (cma__g_abort_signal);
        }
#endif

    return cma_c_true;

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	See if there are any terminated threads (zombies) which haven't yet
 *	been laid to rest.  If there are, free them.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma__g_known_threads queue
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
extern void
cma__dispell_zombies
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;
    cma__t_queue	tempq, *qptr, *nextq, *tq;
#ifndef NDEBUG
    cma_t_integer	freecnt = 0, mvcnt = 0, scancnt = 0, exitdetached = 0;
    cma_t_integer	tofree = 0, firstdead;
    cma__t_int_tcb	*self = cma__get_self_tcb ();
#endif


    cma__trace ((
	    cma__c_trc_disp | cma__c_trc_zombie,
	    "(dispell_zombies) wizard %d(0x%lx) prepares to hunt zombies...",
	    self->header.sequence, self));

    cma__queue_init (&tempq);

    /*
     * Dispell zombie TCBs.
     *
     * When a thread terminates, it must free its TCB before actually
     * completing termination.  This is done by placing it at the end of the
     * known thread queue after it has locked the kernel for its final
     * context switch. When the context switch is finished and the kernel is
     * unlocked, this routine is called to release any threads in the "dead
     * zone" that are no longer in use.
     *
     * Detached threads immediately follow the last living thread: undetached
     * threads are at the end. (On a VP implementation, undetached threads
     * are put at the beginning of the "dead zone", but are moved to the end
     * once the VP has been reclaimed). When the traversal finds an
     * undetached thread (without a VP), the traversal can safely stop.
     */
    cma__enter_kernel ();
    qptr = cma__g_dead_zone;
#ifndef NDEBUG
    if (qptr != &cma__g_known_threads.queue) {
	cma__t_int_tcb	*fd;

	fd = cma__base (qptr, threads, cma__t_int_tcb);
	firstdead = fd->header.sequence;
	}
    else
	firstdead = 0;
#endif

    while (qptr != &cma__g_known_threads.queue) {
	cma_t_boolean	freed_tcb, zombie;
	cma_t_integer	reclaimed = 0;


	nextq = cma__queue_next (qptr);
	tcb = cma__base (qptr, threads, cma__t_int_tcb);
	zombie = (tcb->state == cma__c_state_zombie);

	/*
	 * Check the state of the thread's virtual processor. If the VP is
	 * still assigned, but now idle, the state is "reclaimed". Otherwise,
	 * if the VP had already been reclaimed earlier, remember that it was
	 * "already reclaimed". On a uniprocessor, it's always "already
	 * reclaimed".
	 */
#if _CMA_THREAD_IS_VP_
	if (tcb->sched.processor->vp_id != (cma__t_vpid)0) {

	    if (cma__vp_reclaim (tcb->sched.processor->vp_id)) {
		tcb->sched.processor->vp_id = (cma__t_vpid)0;
		reclaimed = 1;
		}
	    else
		reclaimed = 0;

	    }
	else {

	    if (!zombie) {
# ifndef NDEBUG
		exitdetached = tcb->header.sequence;
# endif
		break;
		}

	    reclaimed = 2;
	    }
#else
	if (!zombie) {
# ifndef NDEBUG
	    exitdetached = tcb->header.sequence;
# endif
	    break;
	    }

	reclaimed = 2;
#endif

#ifndef NDEBUG
	scancnt++;
#endif

	/*
	 * We can only reclaim the resources associated with a thread if
	 * there are no joiners still waiting for it (it may take some time
	 * after termination for all joiners to awaken, if the system is busy
	 * or a joiner is low priority), and the VP associated with the
	 * thread (if any) has been reclaimed. So check all of those
	 * conditions...
	 */
	if (zombie && tcb->joiners == 0 && reclaimed > 0) {
#ifndef NDEBUG
	    tofree++;
#endif
	    if (cma__g_dead_zone == &tcb->threads)
		cma__g_dead_zone = nextq;

	    cma__queue_remove (&tcb->threads, tq, cma__t_queue);
	    cma__queue_insert (&tcb->header.queue, &tempq);
	    }
#if _CMA_THREAD_IS_VP_
	else {
	    /*
	     * On a VP implementation, if we just reclaimed the VP from a
	     * non-detached thread, move it to the end of the list so we
	     * don't have to look at it again.
	     */
	    if (reclaimed == 1) {
# ifndef NDEBUG
		mvcnt++;
# endif
		if (nextq != &cma__g_known_threads.queue) {

		    if (cma__g_dead_zone == &tcb->threads)
			cma__g_dead_zone = nextq;

		    cma__queue_remove (&tcb->threads, tq, cma__t_queue);
		    cma__queue_insert (&tcb->threads, &cma__g_known_threads.queue);
		    }

		}

	    }
#endif

	qptr = nextq;			/* Advance */
	}

    cma__exit_kernel ();

    cma__trace ((
	    cma__c_trc_disp | cma__c_trc_zombie,
	    "(dispell_zombies)  %d(0x%lx)<%d>[%d] scan %d, free %d, reclaim %d",
	    self->header.sequence, self,
	    firstdead, exitdetached, scancnt, tofree, mvcnt));

    /*
     * Now, if there are any TCBs that we can free, they're on the local
     * temporary queue.  Free them all.
     */
    while (!cma__queue_empty (&tempq)) {
	cma__t_int_tcb	*tcbq;


	cma__queue_dequeue (&tempq, tcbq, cma__t_int_tcb);
	cma__trace ((
		cma__c_trc_disp | cma__c_trc_zombie,
		"(dispell_zombies)   %d(0x%lx) lays zombie %d(0x%lx) to rest",
		self->header.sequence,
		self,
		tcbq->header.sequence,
		tcbq));
	cma__free_tcb (tcbq);
#ifndef NDEBUG
	freecnt++;
#endif
	}

    cma__trace ((
	    cma__c_trc_disp | cma__c_trc_zombie,
	    "(dispell_zombies) %d(0x%lx) retires, having freed %d",
	    self->header.sequence, self,
	    freecnt));
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Return address of asynchronous context block in the current tcb
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	current tcb
 *
 *  IMPLICIT OUTPUTS:
 *
 *	none
 *
 *  FUNCTION VALUE:
 *
 *	The address of the asynchronous context block in the current tcb
 *
 *  SIDE EFFECTS:
 *
 *	none
 * 
 */
cma__t_async_ctx *
cma__get_async_info
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb *cur_tcb;


    cur_tcb = cma__get_self_tcb ();

    return &cur_tcb->async_ctx;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	initialize the dispatcher 
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	cma__ready_list	    - CMA ready list
 *
 *  IMPLICIT OUTPUTS:
 *
 *	cma__ready_list	    - CMA ready list
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 * This routine will initialize the default thread and dispatch database.
 * 
 */
void
cma__init_dispatch
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer   i;


    cma___g_lastthread = cma__get_cv (&cma__g_def_attr);
    cma__obj_set_name (cma___g_lastthread, "last thread");

#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && !_CMA_UNIPROCESSOR_
    /*
     * If this is OSF/1 (which supports some per-thread signals), initialize
     * the standard handlers for those signals within the default thread/vp.
     */
    cma__sig_thread_init ();
#endif

#if !_CMA_THREAD_IS_VP_
    cma__trace ((
	    cma__c_trc_init | cma__c_trc_disp,
	    "(init_dispatch) initializing ready lists"));

    for (i = 0; i < cma__c_prio_n_tot; i++)
	cma__queue_init (&cma__g_ready_list[i]);
#endif
    }

#if !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Place a thread in the appropriate ready-queue
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb	    		    - Address of the tcb
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
 * 	This routine will add the thread tcb at the end of the appropriate
 * 	cma__g_ready_list[i].
 */
void
cma__ready
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*tcb,
	cma_t_boolean	preempt)
#else	/* no prototypes */
	(tcb, preempt)
	cma__t_int_tcb	*tcb;
	cma_t_boolean	preempt;
#endif	/* prototype */
    {
    cma_t_integer   cur_ticks;		/* Current absolute time in ticks */
    cma_t_integer   p_i,		/* Current priority (scaled) */
		    q_i;		/* Queue to be readied on */


    /*
     * State transition:  from "running" to "ready"
     */
    tcb->state = cma__c_state_ready;

    /*
     * If this thread was preempted, it will finish its quanta later.  
     * Otherise, it has finished its quanta, so give it a new one for next 
     * time.
     */
    if (!preempt)
        tcb->timer.quanta_remaining += cma__c_quanta;

    /*
     * Ensure that a thread never "accumulates" quanta.
     */
    if (tcb->timer.quanta_remaining > cma__c_quanta)
        tcb->timer.quanta_remaining = cma__c_quanta;

    cur_ticks = cma__get_time_ticks ();

    cma__sched_update_time (
	    tcb->sched.tot_time, 
	    cur_ticks - tcb->sched.time_stamp);
    tcb->sched.time_stamp = cur_ticks;

    /*
     * If the thread has been put on_hold, "ready" the thread to the Hold List.
     */
    if (tcb->debug.on_hold) {
	cma__queue_insert (
	    &(tcb->header.queue), 
	    &cma__g_hold_list);
	/*
	 * NOTE:  We are returning here, thus, all code that applies to both
	 * threads on_hold and not on_hold *must* be performed ABOVE.
	 */
	return;
	}

    /*
     * Check to see if we should recalculate the proper queue.
     */
    if (cur_ticks - tcb->sched.adj_time > cma__c_prio_interval) {
	/*
	 * Update the "last priority adjustment" timestamp.
	 */
	tcb->sched.adj_time = cur_ticks;

	/*
	 * Find the appropriate queue, based on policy and priority.
	 */
	switch (tcb->sched.class) {
	    case cma__c_class_fore:
		/*
		 * We currently assume that all foreground class threads
		 * have variable priorities.
		 */
		{
		p_i = cma__sched_priority (tcb);

		if (p_i < cma__g_prio_p_2)
		    if (p_i < cma__g_prio_p_1) {
			if (p_i < cma__g_prio_fg_min) 
			    p_i = cma__g_prio_fg_min;
			q_i = cma__scale_dn (cma__scale_dn (cma__g_prio_m_0 
				* p_i)) + cma__g_prio_b_0;
			}
		    else
			q_i = cma__scale_dn (cma__scale_dn (cma__g_prio_m_1 
				* p_i)) + cma__g_prio_b_1;
		else
		    if (p_i < cma__g_prio_p_3)
			q_i = cma__scale_dn (cma__scale_dn (cma__g_prio_m_2
				* p_i)) + cma__g_prio_b_2;
		    else {
			if (p_i > cma__g_prio_fg_max) 
			    p_i = cma__g_prio_fg_max;
			q_i = cma__scale_dn (cma__scale_dn (cma__g_prio_m_3
				* p_i)) + cma__g_prio_b_3;
			}

		break;
		}
	    case cma__c_class_back:
		{
		p_i = cma__sched_priority (tcb);

		if (p_i < cma__g_prio_bg_min)  p_i = cma__g_prio_bg_min;
		if (p_i > cma__g_prio_bg_max)  p_i = cma__g_prio_bg_max;

		q_i = (cma__c_prio_n_bg * (p_i - cma__g_prio_bg_min))
			/ (cma__g_prio_bg_max - cma__g_prio_bg_min)
			+ cma__c_prio_o_bg;
		break;
		}
	    case cma__c_class_rt:
		{
		/*
		 * If we implement more than one realtime queue, choose which 
		 * queue here and set q_i appropriately.
		 */
		q_i = cma__c_prio_o_rt;

		break;
		}
	    case cma__c_class_idle:
		{
		q_i = cma__c_prio_o_id;
		break;
		}
	    default:
		{
		cma__bugcheck ("ready: bad sched class %d", tcb->sched.class);
		break;
		}
	    }  /* switch */

	/* 
	 * "Remember" which queue we're going on.
	 */
	tcb->sched.q_num = q_i;
	} /* if */
    else
	/*
	 * No need to update the queue (not enough time has passed to make it
	 * worth the expense of the calculation), just use the same queue as 
	 * last time.
	 */
	q_i = tcb->sched.q_num;

    if (q_i > cma__g_next_ready_queue)	cma__g_next_ready_queue = q_i;

    if (tcb->sched.class != cma__c_class_rt) {
	/*
	 * FIX-ME: The preempted thread should be the next to run...for example
	 *	    if it is a throughput thread, it should be put in the top
	 *	    throughput queue without regard to its calculated priority.
	 */
	if (preempt)
	    cma__queue_insert_after (
		    &(tcb->header.queue), 
		    &cma__g_ready_list[q_i]);
	else
	    cma__queue_insert (
		    &(tcb->header.queue), 
		    &cma__g_ready_list[q_i]);
	}
    else {
	/*
	 * The real-time queue is kept in sorted order.
	 */
	cma__t_queue    *tmp_q;
	cma__t_int_tcb  *tmp_tcb;


	tmp_q = cma__queue_next (&cma__g_ready_list[q_i]);
	tmp_tcb = (cma__t_int_tcb *)tmp_q;

	while ((tmp_q != &cma__g_ready_list[q_i])
		&& (tcb->sched.priority <= tmp_tcb->sched.priority)) {

	    /*
	     * If this is a preempt, quit searching when we get to the
	     * first of equal priority, and then insert before it.  
	     * Otherwise, the loop will exit when we hit one of lower
	     * priority, and we'll insert before it, instead.
	     */
	    if (preempt && tcb->sched.priority == tmp_tcb->sched.priority)
		break;

	    tmp_q = cma__queue_next (tmp_q);
	    tmp_tcb = (cma__t_int_tcb *)tmp_q;
	    }

	cma__queue_insert (&(tcb->header.queue), tmp_q);
	}
    }
#endif					/* !_CMA_THREAD_IS_VP_ */


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
 *	cma__ready_list	    - CMA ready list
 *
 *  IMPLICIT OUTPUTS:
 *
 *	cma__ready_list	    - CMA ready list
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 * This routine will initialize the default thread and dispatch database.
 * 
 */
extern void
cma__reinit_dispatch
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)		/* Indicate pre (1) or post (0) fork */
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;		/* Indicate pre (1) or post (0) fork */
#endif	/* prototype */
    {
    cma_t_integer   i;

    if (flag == cma__c_reinit_postfork_clear) {	/* Post-fork child handling */
	cma__g_threadcnt = 1;		/* Default thread */

#if !_CMA_THREAD_IS_VP_
	for (i = 0; i < cma__c_prio_n_tot; i++)
	    cma__queue_init (&cma__g_ready_list[i]);
#endif
	}

    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Start the thread 
 *
 *  FORMAL PARAMETERS:
 *
 *	thread_tcb		    - Address of the tcb
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
cma__start_thread
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*thread_tcb)
#else	/* no prototypes */
	(thread_tcb)
	cma__t_int_tcb	*thread_tcb;
#endif	/* prototype */
    {
#if !_CMA_THREAD_IS_VP_
    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "In cma__start_thread with kernel unlocked");
#endif

    /*
     * Force cma__ready to perform a priority queue calculation 
     */    
    thread_tcb->sched.adj_time = -(cma__c_prio_interval + 1);

    thread_tcb->sched.tot_time = 
	    thread_tcb->sched.priority * cma__c_init_cpu_time;
    thread_tcb->sched.time_stamp = cma__get_time_ticks ();
    thread_tcb->sched.cpu_time = cma__c_init_cpu_time;
#if !_CMA_THREAD_IS_VP_
    thread_tcb->sched.processor = (cma__t_vp *)cma_c_null_ptr;
#endif
    thread_tcb->timer.quanta_remaining = 0;	/* cma__ready increments */

    cma__ready (thread_tcb, cma_c_false);
    cma__try_run (thread_tcb);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Terminate a thread 
 *
 *  FORMAL PARAMETERS:
 *
 *	thread_tcb	    - Address of the tcb
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
 *	none	(this routine never returns.)
 *
 *  SIDE EFFECTS:
 *
 *	This routine never returns, the thread running it is simply never 
 *	resecheduled.
 */
void
cma__terminate_thread
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*thread_tcb)
#else	/* no prototypes */
	(thread_tcb)
	cma__t_int_tcb	*thread_tcb;
#endif	/* prototype */
    {
    cma_t_address	*qtmp;


    cma__trace ((
	    cma__c_trc_obj | cma__c_trc_disp | cma__c_trc_zombie,
	    "(terminate_thread) thread %d(0x%lx) terminating, %sdetached",
	    thread_tcb->header.sequence,
	    thread_tcb,
	    (thread_tcb->detached ? "" : "not ")));

    /*
     * Dispell any current undead before we add this thread to the queue.
     * This prevents too many from accumulating, while ensuring that this
     * particular zombie will survive until after context switch.
     */
    cma__dispell_zombies ();

    cma__enter_kernel ();
    cma__g_threadcnt--;			/* One less thread running */

    /*
     * Dequeue the thread, and move it immediately following the last living
     * thread (if detached) or to the end of the known threads list. This
     * allows the zombie list scanner to stop when it hits a non-detached
     * thread.
     *
     * All threads on the known thread list from cma__g_dead_zone on are
     * "zombies" -- terminated threads kept around until it's safe to reclaim
     * their context (stack, VP, and TCB). A thread can't actually be
     * reclaimed until the VP is off the stack and all joiners have finished
     * with the thread. The "zombie" thread state is used only for threads
     * that are both terminated and detached.
     */
    cma__queue_remove (&thread_tcb->threads, qtmp, cma_t_address);

    if (thread_tcb->detached) {
	thread_tcb->state = cma__c_state_zombie;
	cma__queue_insert (&thread_tcb->threads, cma__g_dead_zone);
	cma__g_dead_zone = &thread_tcb->threads;
	}
    else {
	thread_tcb->state = cma__c_state_terminated;
#if _CMA_THREAD_IS_VP_
	/*
	 * For one-to-one kernel thread systems, dispell_zombies also takes
	 * care of reclaiming the VP -- since the spell stops at a
	 * non-detached thread without a VP, we *always* put VPed threads at
	 * the front of the dead zone (this code is the same as for
	 * (thread_tcb->detached), above, except for the thread state).
	 */
	cma__queue_insert (&thread_tcb->threads, cma__g_dead_zone);
	cma__g_dead_zone = &thread_tcb->threads;
#else
	cma__queue_insert (&thread_tcb->threads, &cma__g_known_threads.queue);

	if (cma__g_dead_zone == &cma__g_known_threads.queue)
	    cma__g_dead_zone = &thread_tcb->threads;
#endif

	}

    /*
     * If this is the default (initial) thread, then don't completely exit
     * until all other threads have exited; then exit the process.
     */
    if (thread_tcb->kind == cma__c_thkind_initial) {
	cma__unset_kernel ();		/* Unlock the kernel */
	cma__int_lock (thread_tcb->mutex);

	while (cma__g_threadcnt > cma___c_null_thd_cnt) {
	    TRY {
		cma__int_wait(cma___g_lastthread,thread_tcb->mutex,thread_tcb);
		}
	    CATCH_ALL {		
		}
	    ENDTRY
	    }

	cma__int_unlock (thread_tcb->mutex);
	/*
	 * P1003.4a/D4 doesn't specify the process exit status when all
	 * threads have exited. P1003.4a/D5 specifies that the status is
	 * "unspecified". A value of "0" seems reasonable (1 on OpenVMS) to
	 * signify normal termination.
	 */
#if _CMA_OS_ != _CMA__VMS
	exit (0);
#else
	sys$exit (1);
#endif
	}
    else {

	if (cma__g_threadcnt <= cma___c_null_thd_cnt) {
	    cma__unset_kernel ();	/* Unlock the kernel */
	    cma__int_signal (cma___g_lastthread);
	    cma__enter_kernel ();
	    }

	}

#if _CMA_THREAD_IS_VP_
    /*
     * In multiplexed implementations, the kernel stays locked through the
     * context switch into another thread. In one-to-one VP implementations,
     * however, we can safely release the kernel now.
     */
    cma__unset_kernel ();		/* Unlock the kernel */
#endif

    /*
     * Select next ready thread
     */
    cma__dispatch (
	    thread_tcb,
	    cma_c_false,
	    0);

    cma__bugcheck ("terminate_thread: continued after termination");
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	This routine performs a context switch
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	the current thread tcb, the cma__g_ready_list
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
 *	The current thread's tcb will be appended to the cma__g_ready_list,
 *	the first tcb will be taken from the ready list and its thread
 *	will become the running thread.  
 */
void
cma__yield_processor
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma__t_int_tcb	*cur_tcb;	/* Current thread's TCB  */


    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "In cma__yield_processor with kernel unlocked");

#if !_CMA_THREAD_IS_VP_
    cur_tcb = cma__get_self_tcb ();

    /* 
     * Insert the current thread on the rear of the appropriate ready queue.
     */
    cma__ready (cur_tcb, cma_c_false);

    /*
     * If the debugger changed anything (e.g. priority or hold for one
     * or more threads), sort the ready lists and hold queue to make the 
     * dispatcher database consistent.
     */
    if (cma__g_debug_state.is_inconsistency) {
	cma__g_debug_state.is_inconsistency = cma_c_false;
 	cma___sort_ready_list (); 
	}


    /*
     * Select next ready thread
     */
    cma__dispatch (
	    cur_tcb,
	    cma_c_true,			/* Save current context */
	    0);
#else
    cma__exit_kernel ();
    cma__vp_yield ();
    cma__enter_kernel ();
#endif					/* _CMA_THREAD_IS_VP_ */
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DISPATCH.C */
/*  *80    2-JUL-1993 09:43:04 KEANE "Fix DEC C warnings: add return statements to non-void routines" */
/*  *79   11-JUN-1993 10:32:22 BUTENHOF "Fix mach reclamation bug" */
/*  *78    4-JUN-1993 22:41:13 BUTENHOF "Don't free TCB with running VP!" */
/*  *77   28-MAY-1993 12:17:46 BUTENHOF "Fix last thread cv name" */
/*  *76   27-MAY-1993 14:32:19 BUTENHOF "Fix bugcheck message" */
/*  *75   17-MAY-1993 09:57:44 BUTENHOF "Fix VAX ULTRIX compilation error" */
/*  *74   14-MAY-1993 15:55:22 BUTENHOF "Reinstate local zombie queue hack" */
/*  *73    4-MAY-1993 11:17:08 BUTENHOF "Fix queue corrupter" */
/*  *72    3-MAY-1993 13:44:19 BUTENHOF "Merge zombies with known threads" */
/*  *71   30-APR-1993 18:13:57 BUTENHOF "More zombie tracing" */
/*  *70   27-APR-1993 14:37:23 BUTENHOF "Log dispell" */
/*  *69   16-APR-1993 13:03:26 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *68    7-APR-1993 12:54:51 SCALES "Touch up dispell-zombies for non-VP case" */
/*  *67    5-APR-1993 14:12:38 BUTENHOF "Remove VP zombie queue" */
/*  *66   25-MAR-1993 14:43:25 BUTENHOF "Fix trace hexes" */
/*  *65   19-MAR-1993 17:49:36 KEANE "Fix dispell_zombies for VP" */
/*  *64   10-MAR-1993 13:56:10 BUTENHOF "Don't get mach state in dispell_zombies" */
/*  *63   17-FEB-1993 14:35:25 BUTENHOF "Fix 'no vp' bugcheck text" */
/*  *62   24-NOV-1992 01:52:46 SCALES "Add support for TIS multiple errno table" */
/*  *61   15-OCT-1992 14:04:27 BUTENHOF "Don't leave kernel locked on kthread terminate" */
/*  *60   24-SEP-1992 08:56:31 SCALES "Replace ada-constant with scheduling class constant" */
/*  *59   23-SEP-1992 17:10:25 SCALES "Update the VP's TCB pointer during context switch" */
/*  *58    4-SEP-1992 15:52:57 SCALES "Separate into two modules" */
/*  *57    2-SEP-1992 16:24:56 BUTENHOF "Separate semaphores from kernel lock" */
/*  *56   21-AUG-1992 13:41:48 BUTENHOF "Use spinlocks on kernel thread semaphores instead of kernel_critical" */
/*  *55   14-AUG-1992 13:45:13 BUTENHOF "Fix dispell_zombie problem" */
/*  *54   13-AUG-1992 14:43:51 BUTENHOF "Set default thread unique value" */
/*  *53   11-AUG-1992 13:15:48 BUTENHOF "Nullify cma__ready on kernel thread" */
/*  *52    4-AUG-1992 11:04:41 BUTENHOF "Clear vpid when VP is deleted" */
/*  *51   19-MAY-1992 14:20:46 SCALES "Cleanse null threads' queue pointers during re-init" */
/*  *50   18-MAY-1992 16:48:39 SCALES "Cleanse queue links in sort-ready-queues" */
/*  *49   20-APR-1992 10:07:29 BUTENHOF "Modify bugcheck for terminated" */
/*  *48   15-APR-1992 14:43:20 BUTENHOF "Bugcheck on dispatch to terminated thread" */
/*  *47    3-APR-1992 18:33:51 SCALES "Rework async context switch for U*ix" */
/*  *46   31-MAR-1992 13:30:32 BUTENHOF "Alpha OSF" */
/*  *45   19-MAR-1992 13:17:07 BUTENHOF "Add undefer() calls before yield_processor()" */
/*  *44   18-MAR-1992 19:01:04 SCALES "Parameterize scheduling policies" */
/*  *43   13-MAR-1992 11:09:02 BUTENHOF "Clean up reinit" */
/*  *42   10-MAR-1992 16:25:20 BUTENHOF "Change timeb to timeval" */
/*  *41   26-FEB-1992 19:14:56 SCALES "errno reorganization" */
/*   37A1 26-FEB-1992 13:43:50 KEANE "Fixed shared lib prob in BL9" */
/*  *40   19-FEB-1992 13:48:53 SCALES "Perform undeferal on enter-kernel" */
/*  *39   18-FEB-1992 15:28:41 BUTENHOF "Call vp_dump in case of terminate bugcheck" */
/*  *38   10-FEB-1992 11:29:25 SCALES "Remove unneeded initializations" */
/*  *37   30-JAN-1992 22:12:24 SCALES "Implement our own $CLRAST" */
/*  *36   30-JAN-1992 11:55:51 BUTENHOF "Get rid of trace_kernel" */
/*  *35   29-JAN-1992 23:47:24 SCALES "Rework async context switch on VMS" */
/*  *34   24-JAN-1992 10:02:01 BUTENHOF "Fix type mismatch" */
/*  *33   23-JAN-1992 14:23:23 BUTENHOF "Integrate DEC OSF/1 shared library support" */
/*   22A1 20-JAN-1992 17:15:19 SCALES "Integrate vp_yield for Tin" */
/*  *32    9-JAN-1992 14:25:49 BUTENHOF "Support Alpha in cause_force_dispatch" */
/*  *31    6-DEC-1991 07:19:26 BUTENHOF "Sort realtime threads on blocked queues" */
/*  *30   27-NOV-1991 09:24:41 BUTENHOF "Change errno access" */
/*  *29   26-NOV-1991 16:15:10 BUTENHOF "Get rid of exit() call on VMS" */
/*  *28   22-NOV-1991 11:55:45 BUTENHOF "Add casts" */
/*  *27   18-NOV-1991 11:04:00 CURTIN "Added include for stdlib and an Alpha switch" */
/*  *26   14-OCT-1991 13:38:27 BUTENHOF "Refine/fix use of config symbols" */
/*  *25   24-SEP-1991 16:26:58 BUTENHOF "Apollo-specific quick kill" */
/*  *24   17-SEP-1991 13:23:23 BUTENHOF "Implement ""yield"" for Mach kernel threads" */
/*  *23    2-JUL-1991 16:52:36 BUTENHOF "Cache VPs with thread" */
/*  *22   21-JUN-1991 11:59:18 BUTENHOF "Fix some type mismatch errors" */
/*  *21   11-JUN-1991 17:16:45 BUTENHOF "Clean out some old cma__traces" */
/*  *20   10-JUN-1991 18:21:29 SCALES "Add sccs headers for Ultrix" */
/*  *19   10-JUN-1991 17:54:04 SCALES "Conditionalize inclusion of I/O stuff" */
/*  *18    5-JUN-1991 18:37:42 BUTENHOF "Add cast to free_mem() call" */
/*  *17    5-JUN-1991 17:31:24 BUTENHOF "Clean up some disabled code" */
/*  *16    3-JUN-1991 17:12:57 BUTENHOF "More debugging for an error condition..." */
/*  *15    2-JUN-1991 19:36:24 BUTENHOF "Remove null thread and timer code for VPs" */
/*  *14   31-MAY-1991 14:12:26 CURTIN "Added a new reinit routine" */
/*  *13   29-MAY-1991 17:14:28 BUTENHOF "Minimize null thread kernel locking (MP)" */
/*  *12   14-MAY-1991 13:43:17 BUTENHOF "Continue OSF/1 debugging" */
/*  *11    3-MAY-1991 11:25:47 BUTENHOF "Continue OSF/1 debugging" */
/*  *10   24-APR-1991 11:11:08 BUTENHOF "Continue OSF/1 debugging" */
/*  *9    15-APR-1991 15:54:24 CURTIN "added an include" */
/*  *8    12-APR-1991 23:35:30 BUTENHOF "Implement VP layer to support Mach threads" */
/*  *7     1-APR-1991 18:08:19 BUTENHOF "Integrate exception changes" */
/*  *6    28-MAR-1991 17:22:21 SCALES "Improve variable priority dispatch" */
/*  *5    14-MAR-1991 13:45:36 SCALES "Convert to stream format for ULTRIX build" */
/*  *4    12-MAR-1991 19:22:50 SCALES "Merge Apollo changes to CD4" */
/*  *3     6-FEB-1991 18:57:07 BUTENHOF "Improve response of signal_int operation" */
/*  *2     6-FEB-1991 01:32:51 BUTENHOF "Change errno field name" */
/*  *1    12-DEC-1990 21:44:53 BUTENHOF "Thread dispatcher" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DISPATCH.C */
