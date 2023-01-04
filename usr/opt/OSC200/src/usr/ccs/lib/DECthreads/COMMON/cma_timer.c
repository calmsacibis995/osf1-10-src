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
static char *rcsid = "@(#)$RCSfile: cma_timer.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:52:50 $";
#endif
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	timer services for the scheduler
 *
 *  AUTHORS:
 *
 *	Hans Oser
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	8 September 1989
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Hans Oser	10 October 1989
 *		cma__tick_wait, cma__tick_delay, cma__tick_wait_insert added;
 *		All timer support moved from cma_dispatch into this module.
 *	002	Hans Oser	11 October 1989
 *		Timer initialisation completed
 *	003	Hans Oser	13 October 1989
 *		Changements due to program review
 *	004	Dave Butenhof	19 October 1989
 *		Modify use of queue operations to use appropriate casts
 *		rather than depending on ANSI C "void *" (sigh).
 *	005	Webb Scales	19 October 1989
 *		Added #include <cma_host.h> for cma__test_and_set/cma__unset
 *	006	Hans Oser	20 October 1989
 *		Additions for ULTRIX nonblocking I/O
 *	007	Webb Scales	20 October 1989
 *		Made it acceptable to MIPS/Ultrix C
 *	008	Hans Oser	26 October 1989
 *		cma__timed_semaphore_decrement corrected.
 *	009	Dave Butenhof	27 October 1989
 *		Make cma__g_tick_time static.
 *	010	Dave Butenhof	All Saints Day 1989
 *		Make use of cma__enter_kernel instead of manually whiling on
 *		test-and-set.
 *	011	Webb Scales	3 November 1989
 *		Temporarily #ifdef'd out semaphore.lock field references
 *	012	Bob Conti	4 November 1989
 *		Removed superfluous include cma_host.
 *	013	Bob Conti	5 November 1989
 *		Renamed time_slice constant to quanta, and
 *		time_slice field to quanta_remaining.
 *		Replaced last_dispatched time-stamp (which caused high
 *		overhead per context-switch) with an incrementing total
 *		runtime (in ticks).  Added constant for one tick.
 *		Rename actual to current everywhere.
 *	014	Bob Conti	6 November 1989
 *		Modify timed_semaphore_decrement to use kernel critical lock
 *		and get its code "mostly right".
 *	015	Webb Scales & Bob Conti		6 November 1989
 *		Moved quantum increment into cma__ready.  Added exit_kernel's
 *		where appropriate.
 *	016	Webb Scales	7 November 1989
 *		Renamed "PC" & "PSL" to "ret_pc" & "ret_psl".
 *		Consolidated calls to cma__AST_force_dispatch and
 *		cma__signal_force_dispatch into a single, os-independent call,
 *		cma__cause_force_dispatch.
 *	017	Dave Butenhof	9 November 1989
 *		Modify to use system time rather than private "ticks", and
 *		rename things for clarity.  Simplify a lot of the algorithms
 *		(especially in the timer AST/signal code).  Add some services
 *		for time conversion (using cma_t_date_time and cma_t_interval
 *		as bases).  (MAJOR REWRITE)
 *	018	Webb Scales	18 November 1989
 *		Changed call to cma__cause_force_dispatch to pass the address
 *		of the signal context block for MIPS/Ultrix.
 *	019	Webb Scales	19 November 1989
 *		Correct a typecast.
 *	020	Webb Scales	8 December 1989
 *		Overhauled non-blocking I/O
 *	021	Webb Scales	10 December 1989
 *		Changed cma__check_io to cma__io_available.
 *	022	Dave Butenhof and Webb Scales	9 January 1990
 *		Fix prototype of cma___periodic. Conditional should be
 *		"unix", not "ultrix".
 *	023	Webb Scales	9 January 1990
 *		Fix cma__add_time & cma__subtract_time to correctly detect and 
 *		handle carries/borrows to/from seconds from/to milliseconds.
 *	024	Dave Butenhof	26 January 1990
 *		Change cma__get_self_tcb_kernel to cma__get_self_tcb (it no
 *		longer locks the kernel, so distinction is irrelevant).
 *	025	Dave Butenhof	15 February 1990
 *		Fix interval_to_time routine; on VMS, it uses RTL routine to
 *		convert to quadword date/time.  A value of 0.0 is an "invalid
 *		time" since no quadword time value for delta-0 can exist.
 *		Bypass call for 0.0, and return quadword of -1 (smallest
 *		delta time).
 *	026	Webb Scales	23 March 1990
 *		Now avoids unnecessary dispatches.
 *	027	Dave Butenhof & Webb Scales	29 March 1990
 *		Add cell for next-timer-queue entry time (for null thread
 *		select).  Make timeslice signal value "global".
 *	028	Dave Butenhof & Webb Scales	6 April 1990
 *		Integrate Apollo changes.
 *	029	Webb Scales	17 April 1990
 *		Catch exceptions raised by get_self_tcb in periodic.
 *	030	Webb Scales	4 June 1990
 *		Establish timeslice signal handler in cma_signal.
 *		Make timeslice routine ("periodic") available to cma_signal.
 *	031	Dave Butenhof	6 June 1990
 *		Finish 030 (make name change of cma___periodic to
 *		cma__periodic consistent).
 *	032	Dave Butenhof	14 June 1990
 *		Use new cma__kernel_[un]set operations to deal with the
 *		kernel lock, instead of cma__test_and_set and cma__unset,
 *		which may now try to lock the kernel (on processors without
 *		interlocked instructions).
 *	033	Webb Scales	15 June 1990
 *		Added priority scheduling
 *		- Added global tick counter (absolute time)
 *		- Tick count is updated by timeslicer or deferred for kernel
 *		- Thread cpu time is now measured in ticks
 *	034	Webb Scales	6 July 1990
 *		Disabled tick-deferral in timeslicer.
 *	035	Dave Butenhof	03 August 1990
 *		Removed semaphore increment from timed condition wait timeout
 *	036	Dave Butenhof	14 August 1990
 *		Implement (primitive) asynchronous alert/cancel by checking
 *		for a pending alert on the current thread at each timer
 *		interrupt, and doing magic to cause the exception if there is
 *		one.
 *	036	Webb Scales	16 August 1990
 *		- Changed #ifdef's to #if's
 *		- Modified Apollo timeslice rate
 *		- Added uniprocessor optimization around get_self in ti*
 *		- Rearranged ifdefs in call to force_dispatch
 *	037	Bob Conti	1 October 1990
 *		Delete cma_c_null_thread
 *	038	Webb Scales & Dave Butenhof	 4 December 1990
 *		Modified check_timer_queue, making it global.
 *		Added a parameter to io_available.
 *	039	Webb Scales	 6 December 1990
 *		Added HP-specific asynch alert and yield actions.
 *	040	Paul Curtin	03 January 1991
 *		Added new feature to cma__check_timer_queue to 
 *		handle case were entry is readied.
 *	041	Dave Butenhof & Webb Scales	3 January 1991
 *		Fix 040: don't write to "next_time" parameter unless it's
 *		there.
 *	042	Dave Butenhof	7 February 1991
 *		Change names of alert state bits.
 *	043	Dave Butenhof	12 March 1991
 *		Add optimization to timer processing: check for (and process)
 *		expired entries at head of queue when inserting new entries.
 *	045	Dave Butenhof	04 April 1991
 *		Change _CMA_UNIPROCESSOR_ to 0/1 instead of ifdef/ifndef
 *	046	Dave Butenhof	24 April 1991
 *		DECthreads' "thread synchronous I/O" wrappers aren't used in
 *		non-multiplexed MP environment (the system is assumed to
 *		support that directly for kernel threads). So don't let the
 *		timer function make a call to io_available.
 *	047	Paul Curtin	31 May 1991
 *		Added a fork() reinitialization routine.
 *	048	Dave Butenhof	31 May 1991
 *		Disable timeslicing and such for _CMA_THREAD_IS_VP_ mode;
 *		just use thread-synchronous kernel timers.
 *	049	Paul Curtin	 5 May 1991
 *		Rearranged flags in reinit routine.
 *	050	Webb Scales and Dave Butenhof	    10 June 1991
 *		Conditionalize inclusion of I/O stuff.
 *	051	Dave Butenhof
 *		Integrate Apollo CMA5 reverse drop: remove special cased
 *		quantum and timer interval for Apollo platforms.
 *	052	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	053	Paul Curtin	18 November 1991
 *		Alpha work: added an include for starlet.h, and a number
 *		of Alpha switches
 *	054	Paul Curtin	10 November 1991
 *		Added an include for cma_util.h
 *	055	Paul Curtin	20 December 1991
 *		Remove starlet.h on VAX
 *	056	Webb Scales	28 January 1992
 *		- Changed exit-kernel to unset-kernel in periodic.
 *		- Reworked the end-quantum preemption on VMS
 *	057	Webb Scales	29 January 1992
 *		Repair damage to async alerts on VAX/VMS.
 *	058	Webb Scales	11 February 1992
 *		Add asynchronous alert code for both Alpha and VAX VMS.
 *	059	Dave Butenhof	10 March 1992
 *		Convert from ftime() function and 'struct timeb' to
 *		gettimeofday() function and 'struct timeval'. The new version
 *		isn't dependent on BSD-isms relegated to libbsd.a on many
 *		systems (such as OSF/1 and AIX).
 *	060	Webb Scales	13 March 1992
 *		Use scheduling policy "run-till block" characteristic instead 
 *		of policy itself to determine whether to do anynch. ctx. switch
 *	061	Dave Butenhof	19 March 1992
 *		Add call to cma__undefer() before all direct calls to
 *		cma__yield_processor() -- since cma__ready() no longer
 *		undefers.
 *	062	Webb Scales	31 March 1992
 *		Rework asynch context switch for U*ix.
 *	063	Webb Scales	30 June 1992
 *		Changed typecast from "unsigned short" to "long" in 
 *		interval-to-time.  Apparently this was missed in the great
 *		ftime-to-gettimeofday conversion.
 *	064	Dave Butenhof	30 July 1992
 *		Add explanation for "odd" return values of check_timer_queue.
 *	065	Dave Butenhof	14 August 1992
 *		Fix some remaining typecast problems in timer functions.
 *	066	Webb Scales and Paul Curtin	3 September 1992
 *		Remove latency in timer expiration wake-up.
 *	067	Dave Butenhof	08 September 1992
 *		Fix name of sys$setimr
 *	068	Paul Curtin	14 September 1992
 *		Move check to cancel timer AST to be done only when
 *		timer event is queued (ie, time not already passed).
 *	069	Dave Butenhof	15 September 1992
 *		insert_timer was setting next_tqe and then setting a timer AST
 *		if the new entry was earlier than next_tqe -- which results
 *		in a distinct lack of timer ASTs and a certain tendency to
 *		hang.
 *	070	Dave Butenhof	17 September 1992
 *		(alternate title, "069 part 2"). The timer queue starts out
 *		empty (surprise), with a next_tqe of 0 -- so the timer AST is
 *		only queued if the new entry is earlier than 17-NOV-1858. Not
 *		entirely surprisingly, this doesn't happen an awful lot. Add
 *		a check for uninitialized next_tqe.
 *	071	Webb Scales	17 September 1992
 *		Moved the timer-reset to a place where it will be done reliably.
 *	072	Webb Scales	18 September 1992
 *		Changed the time slicer back to real time, so that system
 *		services don't completely block the process.
 *	073	Webb Scales	21 September 1992
 *              Change VMS timers to use non-default event flag.
 *	074	Dave Butenhof	22 September 1992
 *		Clear timeout status in TCB when queuing timer! Somewhere in
 *		all this latest spate of changes, we lost the init.
 *	075	Dave Butenhof	24 September 1992
 *		Add entry point to disable timers.
 *	076	Paul Curtin	06 January 1993
 *		Change check_timer_queue to work properly in the
 *		case where current time equals queued time.
 *	077	Paul Curtin	06 January 1993
 *		Added another check to the timer queue.
 *	078	Brian Keane	08 April 1993
 *		Fix cast in cma__interval_to_time, which was incorrect for
 *		64 bit machines.
 *	079	Dave Butenhof	 9 April 1993
 *		Round up microseconds conversion (float -> int) in
 *		cma__interval_to_time, to see if this improves our odd "short
 *		wait" problem on some platforms.
 *	080	Dave Butenhof	14 April 1993
 *		Update queue macro usage to allow VAX builtins.
 *	081	Dave Butenhof	25 May 1993
 *		Remove mgr thread CV -- use direct cma__ready() to schedule.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_timer.h>
#include <cma_tcb.h>
#include <cma_dispatch.h>
#include <cma_stack.h>
#include <cma_condition.h>
#include <cma_assem.h>
#include <cma_util.h>
#include <cma_alert.h>
#include <cma_defer.h>
#include <cma_sched.h>

#if _CMA_OS_ == _CMA__VMS
# include <lib$routines.h>
# include <libdtdef.h>
# if _CMA_HARDWARE_ == _CMA__ALPHA
#  include <starlet.h>
# endif
#endif

#if _CMA_OS_ == _CMA__UNIX
# if !_CMA_THREAD_SYNC_IO_
#  include <cma_thread_io.h>
# endif
# include <sys/time.h>
#endif


/*
 * GLOBAL DATA
 */

cma_t_date_time	cma__g_one_tick;
cma_t_date_time	cma__g_next_tqe;
cma_t_integer	cma__g_time_ticks = 0;

/*
 * LOCAL DATA
 */

static cma_t_date_time	cma___g_last_time;
static cma_t_date_time	cma___g_start_time;
static cma__t_queue	cma___g_timer_queue;
static cma_t_natural	cma___g_timer_efn;

/*
 * The timer interval is .1 seconds.
 */
#if _CMA_OS_ == _CMA__VMS
static cma_t_date_time	cma___g_vms_time_interval = {-1000*1000, -1};
#endif

#if _CMA_OS_ == _CMA__UNIX
# define cma___c_unix_time_interval  100000
#endif

/*
 * LOCAL FUNCTIONS
 */

static void
cma___wake_mgr_async _CMA_PROTOTYPE_ ((void));


/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Add two times.
 *
 *  FORMAL PARAMETERS:
 *
 *	result	Result time
 *
 *	time1	first input time
 *	
 *	time2	second input time
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
cma__add_time
#ifdef _CMA_PROTO_
	(
	cma_t_date_time	*result,
	cma_t_date_time	*time1,
	cma_t_date_time	*time2)
#else	/* no prototypes */
	(result, time1, time2)
	cma_t_date_time	*result;
	cma_t_date_time	*time1;
	cma_t_date_time	*time2;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    lib$add_times (time1, time2, result);
#else
    result->tv_usec = time1->tv_usec + time2->tv_usec;
    result->tv_sec  = time1->tv_sec  + time2->tv_sec;

    if (result->tv_usec >= 1000000) {	/* check for carry */
	result->tv_usec -= 1000000;
	result->tv_sec    += 1;
	}
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Check the timer queue and return time to next entry.
 *
 *  FORMAL PARAMETERS:
 *
 *	next_time : address of a cma_t_date_time to receive the delta time 
 *		    until the next timer queue entry will expire (or 0 if
 *		    a thread was readied).
 *
 *  IMPLICIT INPUTS:
 *
 *	The timer queue.
 *
 *  IMPLICIT OUTPUTS:
 *
 *	The ready queue(s).
 *
 *  FUNCTION VALUE:
 *
 *	The function value is currently only used by the null thread, to
 *	determine whether it should pass an expiration time to
 *	cma__io_available, or generate an infinite time.
 *
 *	The function value thus is customized for that particular use: it is
 *
 *		FALSE	use an infinite timeout
 *
 *		TRUE	use the timeout value returned by this function (may
 *			be the next timer queue entry, or "0 seconds" if one
 *			or more threads were readied).
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#if !_CMA_THREAD_IS_VP_
extern cma_t_boolean
cma__check_timer_queue
#ifdef _CMA_PROTO_
	(
	cma_t_date_time	*next_time)
#else	/* no prototypes */
	(next_time)
	cma_t_date_time *next_time;
#endif	/* prototype */
    {
    cma__t_tcb_time	*tcb_time;
    cma__t_int_tcb	*cur_tcb;
    cma_t_boolean	ret_val;
    cma_t_boolean	readied;
    cma_t_address	*qtmp;


    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "cma___check_timer_queue:  kernel unlocked.");

    readied = cma_c_false;
    cma__get_time (&cma___g_last_time);

    if (!cma__queue_empty (&cma___g_timer_queue)) {
	tcb_time = (cma__t_tcb_time *)cma__queue_next(
		&cma___g_timer_queue);

	while (cma__time_leq (&tcb_time->wakeup_time, &cma___g_last_time)
		&& (&tcb_time->queue != &cma___g_timer_queue)) {
	    cma__queue_remove (&tcb_time->queue, qtmp, cma_t_address);
	    cur_tcb = cma__base (tcb_time, timer, cma__t_int_tcb);

	    /*
	     * Since we timed out before the condition variable was
	     * signalled, remove the TCB from the blocked queue before
	     * putting it on a ready queue.
	     */
	    cma__queue_remove (&cur_tcb->header.queue, cur_tcb, cma__t_int_tcb);
	    cur_tcb->timer.event_status = cma__c_timeout;
	    cma__ready (cur_tcb, cma_c_false);
	    readied = cma_c_true;
	    tcb_time = (cma__t_tcb_time *)cma__queue_next (
		    &cma___g_timer_queue);
	    }

	/*
	 * If there are no more timer queue entries, zero the "next tqe" to
	 * reflect this.  If there are more entries, set the "next tqe" as
	 * appropriate.
	 */
	if (cma__queue_empty (&cma___g_timer_queue)) {
	    cma__zero (&cma__g_next_tqe);
	    }
	else {
	    cma__copy_time (
		    &cma__g_next_tqe,
		    &(((cma__t_tcb_time *)cma__queue_next (
			    &cma___g_timer_queue))->wakeup_time));
#if _CMA_OS_ == _CMA__VMS
	    sys$cantim (
		cma__g_mgr_tcb,
		0);
	    sys$setimr (
		cma___g_timer_efn,
		&cma__g_next_tqe,
		cma___wake_mgr_async,
		cma__g_mgr_tcb,
		0);
#endif
	    }
	    
	if (next_time != (cma_t_date_time *)cma_c_null_ptr)
	    if (readied)
		cma__zero (next_time);
	    else {
		cma__subtract_time (
			next_time,
			&cma__g_next_tqe,
			&cma___g_last_time);
		}
	ret_val = cma_c_true;
	}
    else
	ret_val = cma_c_false;

    return ret_val;
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Disable active timers
 *
 *  FORMAL PARAMETERS:
 *
 *      none
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
cma__disable_timer
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    sys$cantim (cma__g_mgr_tcb, 0);	/* Cancel MGR wakeup, if any */
    sys$cantim (cma__periodic, 0);	/* Cancel timeslicer */
#else
    struct itimerval unix_timer, unix_oldtime;
    unix_timer.it_interval.tv_sec  = 0;
    unix_timer.it_interval.tv_usec = 0;
    unix_timer.it_value.tv_sec  = 0;
    unix_timer.it_value.tv_usec = 0;
    setitimer (cma__c_timer, &unix_timer, &unix_oldtime);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Get current time
 *
 *  FORMAL PARAMETERS:
 *
 *      time	current time
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
cma__get_time
#ifdef _CMA_PROTO_
	(
	cma_t_date_time	*time)		/* Get the current date and time */
#else	/* no prototypes */
	(time)
	cma_t_date_time	*time;		/* Get the current date and time */
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    sys$gettim ((long int *)time);
#endif
#if _CMA_OS_ == _CMA__UNIX
    struct timezone	tmptz;


    /*
     * We don't bother with local time, so just put the timezone info in a
     * temporary and forget it.
     */
    gettimeofday (time, &tmptz);
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Initialize dispatch timer
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
 *	This routine starts the cma_dispatch timer, used for timeslicing
 */
void
cma__init_timer
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
#if !_CMA_THREAD_IS_VP_
# if _CMA_OS_ == _CMA__UNIX
    struct sigvec vec, ovec;
    struct itimerval unix_timer, unix_oldtime;
# endif


    cma__get_time (&cma___g_start_time);

    /*
     * Initialize "time of first timer queue entry" to indicate that there
     * are no entries in the timer queue.
     */
    cma__zero (&cma__g_next_tqe);
    cma__queue_init (&cma___g_timer_queue);

# if _CMA_OS_ == _CMA__VMS
    /*
     * Allocate an event flag.  When the timer expires, it sets the event
     * flag, but when the timer is restarted (first thing in the timer AST
     * routine) the flag is cleared.  This means that this flag is basically
     * always clear...which means that it shouldn't be the default flag
     * because it will interfere with user code...
     *
     * We don't check the status of the lib$get_ef() call, because if it
     * fails we're going to use 0, which is already in cma___g_timer_efn.
     */
    lib$get_ef (&cma___g_timer_efn);

    cma__copy_time (&cma__g_one_tick, &cma___g_vms_time_interval);
    sys$setimr (
	    cma___g_timer_efn,
	    &cma___g_vms_time_interval,
	    cma__periodic,
	    cma__periodic,	/* Easy-to-remember ID for cancellation */
	    0);			/* 1: CPU time units;  0: wall-clock time */
# endif
# if _CMA_OS_ == _CMA__UNIX
    cma__g_one_tick.tv_sec	= 0;
    cma__g_one_tick.tv_usec	= cma___c_unix_time_interval;

    unix_timer.it_interval.tv_sec  = 0;
    unix_timer.it_interval.tv_usec = cma___c_unix_time_interval;
    unix_timer.it_value.tv_sec  = 0;
    unix_timer.it_value.tv_usec = cma___c_unix_time_interval;
    setitimer (cma__c_timer, &unix_timer, &unix_oldtime);
# endif
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Insert entry in timer queue.
 *
 *  FORMAL PARAMETERS:
 *
 *	tcb	TCB to be inserted in queue.
 * 
 *      time	Time at which entry should expire.
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
 *	cma_c_true if inserted; cma_c_false if time is already passed.
 *
 *  SIDE EFFECTS:
 *
 *	none
 */
#if !_CMA_THREAD_IS_VP_
cma_t_boolean
cma__insert_timer
#ifdef _CMA_PROTO_
	(
	cma__t_int_tcb	*tcb,
	cma_t_date_time	*time)
#else	/* no prototypes */
	(tcb, time)
	cma__t_int_tcb	*tcb;
	cma_t_date_time	*time;
#endif	/* prototype */
    {
    cma_t_boolean	value;
    cma__t_tcb_time	*time_list;
    cma_t_date_time	next_tqe;


    cma__assert_warn (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "Call to cma__insert_timer outside of kernel.");

#if _CMA_OS_ != _CMA__VMS
    /*
     * Do this to possibly reduce the latency in processing timer events,
     * since on UNIX we don't have a timer set for the next event.
     */
    (void)cma__check_timer_queue ((cma_t_date_time *)cma_c_null_ptr);
#else
    /*
     * FIX-ME: it should not be necessary to call check_timer_queue in this
     * case, but that appears to cause some hang. The reason must be
     * investigated. Either the _CMA_OS_ != _CMA__VMS conditional should be
     * removed and the call executed unconditionally, or else the bug causing
     * it to be necessary should be fixed. In the meantime, the call to
     * cma__get_time (placed as a replacement for a side-effect of
     * check_timer_queue) need not be executed here. [It is not intended that
     * the _CMA_CHECK_TIMER_QUEUE_FIXED_ macro ever be defined; it merely
     * explains the situation better than the "#if 0" that has been used in
     * some other places.]
     */
# ifndef _CMA_CHECK_TIMER_QUEUE_FIXED_
    (void)cma__check_timer_queue ((cma_t_date_time *)cma_c_null_ptr);
# else
    cma__get_time (&cma___g_last_time);
# endif
#endif

    tcb->timer.event_status = cma__c_notimeout;

    if (cma__time_leq (time, &cma___g_last_time))
	value = cma_c_false;
    else {
	cma__copy_time (&tcb->timer.wakeup_time, time);

	if (cma__queue_empty (&cma___g_timer_queue))
	    time_list = (cma__t_tcb_time *)&cma___g_timer_queue;
	else {				/* queue not empty */
	    time_list = (cma__t_tcb_time *)cma__queue_next (
		    &cma___g_timer_queue);

	    while (cma__time_leq (&time_list->wakeup_time, time)
		    && (&time_list->queue != &cma___g_timer_queue))
		time_list = (cma__t_tcb_time *)cma__queue_next (
			&time_list->queue);

	    }

	cma__queue_insert (&tcb->timer.queue, &time_list->queue);

#if _CMA_OS_ == _CMA__VMS
        /*
         * If our wakeup_time is less than the next tqe then recall the
         * current outstanding timer AST, if there is one, and issue
         * a request for an AST with this wakeup_time.
         */
	if (cma__time_leq (&tcb->timer.wakeup_time, &cma__g_next_tqe)
		|| (cma__g_next_tqe.low == 0 && cma__g_next_tqe.high == 0)) {
	    cma__copy_time (
		    &cma__g_next_tqe,
		    &tcb->timer.wakeup_time);
	    sys$cantim (
		cma__g_mgr_tcb, 
		0);
	    sys$setimr (
		cma___g_timer_efn,
		&tcb->timer.wakeup_time,
		cma___wake_mgr_async,
		cma__g_mgr_tcb,
		0);
	    }
#endif

	value = cma_c_true;
	}

    return value;
    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Convert cma_t_interval to delta cma_t_date_time.
 *
 *  FORMAL PARAMETERS:
 *
 *	interval	The interval
 * 
 *      time		Address to return delta time.
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
 *	raises exception if interval is invalid (negative).
 */
extern void
cma__interval_to_time
#ifdef _CMA_PROTO_
	(
	cma_t_interval	interval,
	cma_t_date_time	*time)
#else	/* no prototypes */
	(interval, time)
	cma_t_interval	interval;
	cma_t_date_time	*time;
#endif	/* prototype */
    {
    cma_t_integer	delta_secs;

    if (interval < 0.0)
	cma__error (cma_s_badparam);

#if _CMA_OS_ == _CMA__VMS
    delta_secs = LIB$K_DELTA_SECONDS_F;

    if (interval == 0.0) {
	time->low = ~0;
	time->high = ~0;
	}
    else {
	cma_t_integer	status;

	
	status = lib$cvtf_to_internal_time (
		&delta_secs,
		&interval,
		time);

	if ((status & 7) != 1)		/* If return isn't NORMAL status */
	    cma__error (cma_s_badparam);

	}
#else
    time->tv_sec = (int)interval;
    time->tv_usec = (int)(((interval - (float)time->tv_sec) * 1000000) + 0.5);
#endif
    }

#if !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Periodic dispatch timer
 *
 *  FORMAL PARAMETERS:
 *
# if _CMA_OS_ == _CMA__VMS
 *	none.
# endif
# if _CMA_OS_ == _CMA__UNIX
 *	sig	- The value of the signal (the signal number)  [not used]
 *	code	- Signal argument (for system signals)  [not used]
#  if (_CMA_HARDWARE_ == _CMA__MIPS) || (_CMA_HARDWARE_ == _CMA__VAX) || (_CMA_VENDOR_ == _CMA__APOLLO)
 *	scp	- The address of a signal context structure  (The value in the
 *		    process signal mask field is saved in the TCB in the event
 *		    of a preemption.)
#  endif
# endif
 *
 *  IMPLICIT INPUTS:
 *
 *	The timer queue.
 *
 *  IMPLICIT OUTPUTS:
 *
 *      none
 *
 *  FUNCTION VALUE:
 *
 *	none
 *
 *  SIDE EFFECTS:
 *
 *	This routine will reactivate itself
 *
 */
extern void
cma__periodic
# if _CMA_OS_ == _CMA__VMS
#  ifdef _CMA_PROTO_
	(void)
#  else 	/* no prototypes */
	()
#  endif	/* prototype */
# else
#  ifdef _CMA_PROTO_
	(
	int		    sig,
	int		    code,
	struct sigcontext   *scp)
#  else 	/* no prototypes */
	(sig, code, scp)
	int		    sig;
	int		    code;
	struct sigcontext   *scp;
#  endif	/* prototype */
# endif
    {
    cma__t_int_tcb  *cur_tcb;
    cma_t_boolean   lock, 
		    notcmastack = cma_c_false, 
		    mgr_wakeup = cma_c_false;


# if _CMA_OS_ == _CMA__VMS
   sys$setimr (				/* restart timer */
	    cma___g_timer_efn,
	    &cma___g_vms_time_interval,
	    cma__periodic,
	    cma__periodic,	/* Easy-to-remember ID for cancellation */
	    0);			/* 1: CPU time units;  0: wall-clock time */
# endif

    /*
     * Try to lock the kernel, and remember whether it was already locked
     * (the value of "lock" is the previous setting of the flag). Even if
     * we're in a non-uniprocessor, we still need to test and save rather
     * than using cma__enter_kernel(), since the timer might have caught the
     * the thread we're running in while it had locked the kernel; and
     * waiting for it to release still wouldn't work very well.
     */
    lock = cma__tryenter_kernel ();

    /*
     * On uniprocessor configurations, getting the current tcb will never
     * cause an exception (since it is stored in a variable), so the TRY
     * block and the error handling are superfluous.
     */
# if _CMA_UNIPROCESSOR_
    cur_tcb = cma__get_self_tcb ();
# else
    TRY {
	cur_tcb = cma__get_self_tcb ();
	}
    CATCH (cma_e_notcmastack) {
	notcmastack = cma_c_true;
	}
    ENDTRY

    /*
     * If the current thread is executing on a stack which was not created by 
     * CMA (and is not the default stack), then the current TCB pointer is 
     * bogus, so unlock the kernel if we locked it and return now.  (The rest 
     * this routine depends upon having access to the correct TCB.)  
     *
     * Do not perform "undefer" actions since they might involve signalling a 
     * condition variable, and we are running at interrupt level.
     */
    if (notcmastack) {
	if (!lock)  cma__unset_kernel ();
	return;
	}
# endif

# if _CMA_OS_ == _CMA__UNIX
    /*
     * Check for pending I/O operations which are now ready, and then
     * wake the manager thread to check the timer queue.
     */
    if (!lock) {
	static struct timeval polling_timeout = {0, 0};


	/* 
	 * The first two parameters are arbitrary; poll (null timeout).
	 */
	(void)cma__io_available (
		cma__c_io_read, 
		0, 
		&polling_timeout);

	if (!cma__queue_empty (&cma___g_timer_queue)) {
	    /*
	     * Remember that we requested a wakeup.
	     */
	    mgr_wakeup = cma_c_true;

	    /*
	     * This actually just defers a wakeup request (readying the mgr
	     * thread to run later).
	     */
	    cma__mgr_wake ();
	    }

	}
# endif

    /*
     * In the current thread, decrement the quanta-remaining. 
     * Force a dispatch if it falls to zero.
     */
    if (cur_tcb->kind != cma__c_thkind_null)
        --cur_tcb->timer.quanta_remaining;

# if 0
    if (!lock)
	cur_tcb->sched.cpu_ticks++;
# endif

    if (!lock) {

	if (!cur_tcb->alert_pending
		&& cur_tcb->alert.a_enable
		&& cur_tcb->alert.g_enable
	    ) {
# if _CMA_HARDWARE_ != _CMA__HPPA
#  if _CMA_OS_ == _CMA__UNIX
	    /*
	     * Save the address of the signal mask.  This is necessary in
	     * order to "dismiss" the interrtupt, and to restor
	     */
	    cur_tcb->async_ctx.interrupt_ctx = (cma_t_address)&scp->sc_mask;
#  endif
	    /*
	     * Dismiss the interrupt and exit the kernel.  Now everything
	     * is "normal", so just request that the alert be delivered.
	     * We know that it is pending, it should cause this frame to
	     * be popped; bugcheck if it doesn't.
	     */
	    cma__clear_interrupt (cur_tcb->async_ctx.interrupt_ctx);
	    cma__unset_kernel ();
	    cma__attempt_delivery (cur_tcb);
	    cma__bugcheck ("cma__periodic: return from attempt-delivery");
# else					/* HPPA */
	    /*
	     * Cause cma___timer_base to process alert.
	     */
	    cma__g_timer_action = cma__c_hppa_timer_alert;

	    /*
	     * We've modified the return address for the timer routine so we
	     * will enter the assembly code for a asynchronous dispatch.
	     * This code needs to have the kernel locked, so return without
	     * executing the unset kernel_critical code at the end of the
	     * routine!
	     */
	    return;
# endif /* HPPA */
	    }

	/*
	 * If the current thread has run out of quanta, then we preempt it.
	 */
	if (((cur_tcb->timer.quanta_remaining <= 0) && (!cur_tcb->sched.rtb))
		|| (cur_tcb->kind == cma__c_thkind_null) || mgr_wakeup
	    ) {
# if _CMA_HARDWARE_ != _CMA__HPPA
#  if _CMA_OS_ == _CMA__UNIX
	    /*
	     * Save the address of the signal mask.  This is necessary in
	     * order to "dismiss" the interrtupt, and to restor
	     */
	    cur_tcb->async_ctx.interrupt_ctx = (cma_t_address)&scp->sc_mask;
#  endif
	    /*
	     * Note the fact that this is an async context switch and then
	     * simply yield.  The dispatch code will deal with the interrupt.
	     */
	    cur_tcb->async_ctx.valid = cma_c_true;
	    cma__undefer ();
	    cma__yield_processor ();
	    cur_tcb->async_ctx.valid = cma_c_false;
# else
	    /*
	     * Cause cma___timer_base to yield processor.
	     */
	    cma__g_timer_action = cma__c_hppa_timer_yield;

	    /*
	     * We've modified the return address for the timer routine so we
	     * will enter the assembly code for a asynchronous dispatch. This
	     * code needs to have the kernel locked, so return without
	     * executing the unset kernel_critical code at the end of the
	     * routine!
	     */
	    return;
# endif /* HPPA */
	    }

	/*
	 * Since we locked the kernel, unlock it before we leave. Do not
	 * perform "undefer" actions since we are running at interrupt level.
	 */
	cma__unset_kernel ();	 
	}

    }
#endif					/* !_CMA_THREAD_IS_VP_ */

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Perform any pre and/or post fork() reinitialization work
 *	for this module.
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
 */
void
cma__reinit_timer
#ifdef _CMA_PROTO_
	(
	cma_t_integer	flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	flag;
#endif	/* prototype */
    {
#if !_CMA_THREAD_IS_VP_

    if (flag == 2) {	    /* Perform child post fork() work, in kernel */
	/*
	 * Initialize "time of first timer queue entry" to indicate that there
	 * are no entries in the timer queue.
	 */
	cma__zero (&cma__g_next_tqe);
	cma__queue_init (&cma___g_timer_queue);
	}

#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Subtract one time from another.
 *
 *  FORMAL PARAMETERS:
 *
 *      difference  - result
 * 
 *	minuend	    - time from which to subtract
 * 
 *	subtrahend  - time to subtract
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
cma__subtract_time
#ifdef _CMA_PROTO_
	(
	cma_t_date_time	*difference,
	cma_t_date_time	*minuend,
	cma_t_date_time	*subtrahend)
#else	/* no prototypes */
	(difference, minuend, subtrahend)
	cma_t_date_time	*difference;
	cma_t_date_time	*minuend;
	cma_t_date_time	*subtrahend;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    lib$sub_times (minuend, subtrahend, difference);
#else
    difference->tv_usec	= minuend->tv_usec - subtrahend->tv_usec;
    difference->tv_sec	= minuend->tv_sec  - subtrahend->tv_sec;

    if (difference->tv_usec < 0) {	/* check for borrow */
	difference->tv_usec += 1000000;
	difference->tv_sec  -= 1;
	}
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Zero time value
 *
 *  FORMAL PARAMETERS:
 *
 *	time	the time to be zeroed
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
cma__zero_time
#ifdef _CMA_PROTO_
	(
	cma_t_date_time	*time)
#else	/* no prototypes */
	(time)
	cma_t_date_time	*time;
#endif	/* prototype */
    {
#if _CMA_OS_ == _CMA__VMS
    time->low		= 0;
    time->high		= 0;
#else
    time->tv_usec	= 0;
    time->tv_sec	= 0;
#endif
    }
#if _CMA_OS_ == _CMA__VMS

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Asynchronously ready and dispatch to the manager thread. This is
 *	roughly equivalent to signal_preempt_int, only there's no condition
 *	variable involved.
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
 */
static void
cma___wake_mgr_async
# ifdef _CMA_PROTO_
	(void)
# else	/* no prototypes */
	()
# endif	/* prototype */
    {
    cma__t_int_tcb	*self = cma__get_self_tcb ();
    cma_t_boolean	lock;


    /*
     * If possible, start the manager thread immediately; otherwise defer a
     * request for the thread currently in the kernel.
     */
    lock = cma__tryenter_kernel ();

    if (!lock) {
	cma__mgr_wake ();		/* Ready the manager thread */
	self->async_ctx.valid = cma_c_true;
	cma__undefer ();
	cma__yield_processor ();
	self->async_ctx.valid = cma_c_false;
	cma__exit_kernel ();
	}
    else
	cma__unset (&cma__g_mgr_wake);

    }
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIMER.C */
/*  *47   27-MAY-1993 14:32:45 BUTENHOF "Change mgr thread wakeup" */
/*  *46   16-APR-1993 13:06:22 BUTENHOF "Update queue operations to allow INSQUE/REMQUE" */
/*  *45    9-APR-1993 10:53:10 BUTENHOF "Round on interval_to_time" */
/*  *44    8-APR-1993 10:51:26 KEANE "Fix 64-bit cast weirdness" */
/*  *43    6-JAN-1993 17:55:56 CURTIN "Add another check timer queue entry" */
/*  *42    6-JAN-1993 15:40:23 CURTIN "Change check_timer_queue to work properly in the case where current time is equal to */
/* queued time." */
/*  *41   28-SEP-1992 11:49:29 BUTENHOF "Add disable timer entry" */
/*  *40   22-SEP-1992 14:18:58 BUTENHOF "Clear timeout status when queuing timer" */
/*  *39   21-SEP-1992 17:55:47 SCALES "Fix event flag handling" */
/*   27A1 21-SEP-1992 17:55:01 SCALES "Fix event flag handling" */
/*  *38   18-SEP-1992 21:09:39 SCALES "Change timeslicer back from CPU to real time" */
/*  *37   17-SEP-1992 19:52:35 SCALES "Fix timer AST reset" */
/*  *36   17-SEP-1992 14:35:32 BUTENHOF "Don't require first time entry to be < 17-NOV-1858 00:00:00.00" */
/*  *35   15-SEP-1992 13:50:50 BUTENHOF "Fix timer AST management" */
/*  *34   14-SEP-1992 16:14:56 CURTIN "Place check to cancel timer AST in appropriate place" */
/*  *33    8-SEP-1992 11:00:29 BUTENHOF "Fix name of sys$setimr" */
/*  *32    4-SEP-1992 16:28:29 CURTIN "Seperate timer actions from time-slice" */
/*  *31    2-SEP-1992 16:26:37 BUTENHOF "Separate semaphores from kernel lock" */
/*  *30   14-AUG-1992 13:45:22 BUTENHOF "Fix timer calculations" */
/*  *29   31-JUL-1992 15:03:11 BUTENHOF "Add explanation to check_timer_queue" */
/*  *28   30-JUN-1992 13:07:53 SCALES "Fix typecast for timeb->timeval conversion" */
/*  *27    3-APR-1992 18:34:13 SCALES "Rework async context switch on U*ix" */
/*  *26   19-MAR-1992 13:18:13 BUTENHOF "Add undefer() calls before yield_processor()" */
/*  *25   18-MAR-1992 19:02:06 SCALES "Parameterize scheduling policies" */
/*  *24   10-MAR-1992 16:27:39 BUTENHOF "Change ftime to gettimeofday" */
/*  *23   11-FEB-1992 16:30:16 SCALES "Asynch alerts for VMS (VAX & Alpha)" */
/*  *22   30-JAN-1992 22:13:05 SCALES "Repair async alerts for VAX/VMS" */
/*  *21   29-JAN-1992 23:48:41 SCALES "Rework async context switch on VMS" */
/*  *20   20-DEC-1991 11:30:57 CURTIN "removed starlet.h on VAX/VMS" */
/*  *19   20-NOV-1991 14:01:35 CURTIN "Added an include for cma_util.h" */
/*  *18   18-NOV-1991 11:42:07 CURTIN "Alpha work" */
/*  *17   14-OCT-1991 13:42:06 BUTENHOF "Refine/fix use of config symbols" */
/*  *16   24-SEP-1991 16:29:33 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *15   11-JUN-1991 17:17:40 BUTENHOF "Remove old cma__traces" */
/*  *14   10-JUN-1991 18:24:50 SCALES "Add sccs headers for Ultrix" */
/*  *13   10-JUN-1991 17:55:44 SCALES "Conditionalize inclusion of I/O stuff" */
/*  *12    5-JUN-1991 16:15:52 CURTIN "fork work" */
/*  *11    2-JUN-1991 19:37:02 BUTENHOF "Stop using timers for _CMA_THREAD_IS_VP_" */
/*  *10   31-MAY-1991 16:44:26 CURTIN "Added a fork() reinitialization routine" */
/*  *9    29-MAY-1991 17:46:05 BUTENHOF "Fiddle with kthread stuff" */
/*  *8    14-MAY-1991 13:43:37 BUTENHOF "Integrate changes lost in disk crash" */
/*  *7     2-MAY-1991 13:59:26 BUTENHOF "Don't let it try thread_io under OSF/1" */
/*  *6    12-APR-1991 23:37:26 BUTENHOF "Change _CMA_UNIPROCESSOR_ to 0/1" */
/*  *5    13-MAR-1991 14:22:52 BUTENHOF "Timer optimization: process expired entries on insertion" */
/*  *4    12-FEB-1991 01:29:41 BUTENHOF "Change to alert control bits" */
/*  *3     4-JAN-1991 01:53:22 BUTENHOF "Fix the replacement bug." */
/*  *2     3-JAN-1991 18:58:57 CURTIN "completed additions" */
/*  *1    12-DEC-1990 21:55:05 BUTENHOF "Timer services" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_TIMER.C */
