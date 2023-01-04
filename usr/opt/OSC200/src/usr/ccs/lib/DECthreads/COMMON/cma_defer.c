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
static char *rcsid = "@(#)$RCSfile: cma_defer.c,v $ $Revision: 4.2.9.2 $ (DEC) $Date: 1993/08/18 14:45:55 $";
#endif
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	These routines allow actions which cannot be performed immediately
 *	to be deferred until a time when they can be performed immediately.
 *
 *  AUTHOR:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	25 July 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	03 August 1990
 *		Update semaphore deferral to new style, and fix signal
 *		deferral accordingly.
 *	002	Dave Butenhof	27 August 1990
 *		Change name of Webb's "interrupt level signal" to
 *		cma_cond_signal_int rather than more awkward
 *		cma_cond_signal_interrupt (since he put the shorter form into
 *		the transfer vector, and has used it in cmalib).
 *	003	Dave Butenhof	27 August 1990
 *		Change interfaces to pass handles & structures by reference.
 *	004	Webb Scales	 2 October 1990
 *		Removed trailing characters from endifs
 *	005	Webb Scales	 4 December 1990
 *		Added new parameter to io_available.
 *	006	Dave Butenhof	4 February 1991
 *		Rearrange signal_int deferral a little, and add internal
 *		interface.
 *	007	Dave Butenhof	6 February 1991
 *		On VAX/VMS, cma_cond_signal_int and cma__int_signal_int will
 *		now use the $wake kernel service to interrupt the null
 *		thread's hibernate loop, which will cause it to yield.  This
 *		attempts to fix the latency in processing deferred signals
 *		which can occur if no other threads are doing things that
 *		cause CMA to exit from the kernel (and process deferrals).
 *	008	Dave Butenhof	09 April 1991
 *		Use new type for "atomic bit" operation target
 *	009	Dave Butenhof	01 May 1991
 *		Add arguments to cma__bugcheck() calls.
 *	010	Dave Butenhof	02 May 1991
 *		Cut down (dramatically) on the number of cma__alloc_mem()
 *		calls during initialization by allocating the entire set of
 *		defer queue entries for the "augment_queue" functions with a
 *		single call and then chopping it up.
 *	011	Dave Butenhof	14 May 1991
 *		Convert to dynamic init of atomic bit types.
 *	012	Dave Butenhof	30 May 1991
 *		Conditionalize out io_avail defer type for one-to-one kernel
 *		thread implementations (where i/o is already thread
 *		synchronous).
 *	013	Paul Curtin	 4 June 1991
 *		Added a fork() reinit routine.
 *	014	Webb Scales and Dave Butenhof	    10 June 1991
 *		Conditionalize inclusion of I/O stuff.
 *	015	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	016	Dave Butenhof	10 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	017	Webb Scales	10 June 1992
 *		Triple the size of the CV-signal-from-interrupt queue.
 *	018	Dave Butenhof	18 August 1992
 *		Change signal_int functions to use special pending wake form
 *		of semaphore signal.
 *	019	Dave Butenhof	20 August 1992
 *		Add code in signal_int for OSF/1 -- since there is no timer
 *		signal or null thread, try to do the signal directly if the
 *		kernel isn't already locked.
 *	020	Dave Butenhof	26 August 1992
 *		Most of the "global deferral" mechanism is now obsolete. Only
 *		generic UNIX "thread synchronous I/O" is globally deferred,
 *		and that's done mostly within a macro. The only code
 *		remaining is for initialization (which could really be
 *		static) and re-initialization (which could be done directly
 *		in cma_fork()).
 *	021	Dave Butenhof	15 September 1992
 *		Add cma_kernel.h
 *	022	Dave Butenhof	 5 October 1992
 *		Remove cma__init_defer().
 *	023	Webb Scales	 8 February 1993
 *		Add routines to solve the wake-up/waiting race on U*ix.
 *		Added condition variable undefer to cma__undefer().
 *	024	Dave Butenhof	25 February 1993
 *		Improve bugcheck in 023 code.
 *	025	Dave Butenhof	24 May 1993
 *		Add support for new manager thread CV undeferral (and for
 *		waking manager thread under kernel lock instead of using CV
 *		and mutex).
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_kernel.h>
#include <cma_defer.h>
#include <cma_condition.h>
#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
# include <cma_thread_io.h>
#endif
#ifndef NDEBUG
# include <errno.h>
#endif

/*
 * GLOBAL DATA
 */

/*
 * Clear when defers are pending
 */
#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
cma__t_atomic_bit cma__g_defer_avail;
struct timeval cma__g_polling_timeout = {0, 0};
#endif

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_IS_VP_
/*
 * Pipe which prevents the null thread from blocking the process after a 
 * deferred action has taken place, closing a wake-up/waiting race.
 */
cma_t_integer	cma__g_nt_wakeup[2];
#endif
/*
 * LOCAL DATA
 */

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */


#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Module initialization -- set up the "wake-up" pipe.
 *
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
cma__init_defer
#ifdef _CMA_PROTO_
    (void)
#else
    ()
#endif
    {

    /*
     * Create the pipe and set up all of the necessary data structures so that
     * DECthreads will poll this in io_available; set the pipe to non-blocking
     * so that deferral activities don't block the process.
     */
    cma__create_nt_pipe (cma__g_nt_wakeup);
    }
#endif

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Clear all pending wake-ups for the null thread.  (I.e., empty the
 *	"wake-up" pipe.)
 *
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
 *	Empties the wake-up pipe.
 * 
 */
void
cma__nt_clear_wakeup
#ifdef _CMA_PROTO_
    (void)
#else
    ()
#endif
    {
    char buffer[512];
    int	 c, l = 0, t = 0;


    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "Clear-wakeup with kernel critical unlocked");

    do {
	c = read (cma__g_nt_wakeup[0], buffer, sizeof(buffer));
#ifndef NDEBUG
	if (c > 0) {
	    l += 1;
	    t += c;
	    }
	else if ((c == -1) && ((errno == EWOULDBLOCK) || (errno == EAGAIN)))
	    break;
	else
	    cma__bugcheck(
		    "Clear-wakeup read (%d,%d); iterations %d, undefers %d.", 
		    c, (c == -1 ? errno : 0), l, t);
#endif
	} while (c > 0);
    }
#endif

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_IS_VP_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Perform fork() reinitialization work.
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
cma__reinit_defer
#ifdef _CMA_PROTO_
	(
	cma_t_integer	    flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	    flag;
#endif	/* prototype */
    {

    if (flag == cma__c_reinit_postfork_clear) {
	cma__close_nt_pipe (cma__g_nt_wakeup);
	cma__create_nt_pipe (cma__g_nt_wakeup);
	}

    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Undefer operations which couldn't be completed due to kernel lock.
 *
 *	NOTE: C code uses the "cma__undefer()" macro rather than calling this
 *	function: the function version is present to simplify assembly code.
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
#undef cma__undefer
void
cma__undefer
#ifdef _CMA_PROTO_
    (void)
#else
    ()
#endif
    {
#if !_CMA_THREAD_IS_VP_
    cma__assert_fail (
	    cma__tac_isset (&cma__g_kernel_critical),
	    "Undefer call with kernel unlocked");

# if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
    if (!cma__kernel_set (&cma__g_defer_avail))
	cma__io_available (cma__c_io_read, 0, &cma__g_polling_timeout);
# endif

    if (!cma__kernel_set (&cma__g_mgr_wake)) {
	cma__mgr_wake ();
	cma__yield_processor ();
	}

#endif
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEFER.C */
/*  *22   27-MAY-1993 14:32:09 BUTENHOF "Undefer routine == undefer macro" */
/*  *21   25-FEB-1993 14:31:06 BUTENHOF "Improve bugcheck" */
/*  *20   23-FEB-1993 16:30:59 SCALES "Fix two wake-up waiting races in the null thread" */
/*  *19    5-OCT-1992 15:07:39 BUTENHOF "Move defer bit init" */
/*  *18   15-SEP-1992 13:49:35 BUTENHOF "Add header" */
/*  *17    2-SEP-1992 16:24:45 BUTENHOF "Separate semaphores from kernel lock" */
/*  *16   21-AUG-1992 13:41:39 BUTENHOF "Modify signal_int functions" */
/*  *15   10-JUN-1992 22:44:41 SCALES "Increase the size of the signal deferral queue" */
/*  *14   18-FEB-1992 15:28:33 BUTENHOF "Adapt to new alloc_mem protocol" */
/*  *13   14-OCT-1991 13:38:14 BUTENHOF "Refine/fix use of config symbols" */
/*  *12   10-JUN-1991 18:20:26 SCALES "Add sccs headers for Ultrix" */
/*  *11   10-JUN-1991 17:53:49 SCALES "Conditionalize inclusion of I/O stuff" */
/*  *10    5-JUN-1991 17:31:18 BUTENHOF "Add cast to alloc_mem()" */
/*  *9     5-JUN-1991 16:12:31 CURTIN "fork work" */
/*  *8    31-MAY-1991 13:59:31 BUTENHOF "Remove io_avail defer for k-thread impls" */
/*  *7    14-MAY-1991 16:18:17 BUTENHOF "Remove use of ""cma__c_tac_static_clear""" */
/*  *6     2-MAY-1991 16:46:01 BUTENHOF "Fix bug in previous change" */
/*  *5     2-MAY-1991 13:57:55 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *4    12-APR-1991 23:35:18 BUTENHOF "Change type of internal locks" */
/*  *3     6-FEB-1991 18:57:00 BUTENHOF "Improve response of signal_int operation" */
/*  *2     5-FEB-1991 00:59:37 BUTENHOF "Add internal interface to signal_int" */
/*  *1    12-DEC-1990 21:44:19 BUTENHOF "Defer events while kernel locked" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEFER.C */
