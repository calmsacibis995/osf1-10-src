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
 * @(#)$RCSfile: cma_ux.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/06/21 14:26:05 $
 */

/*
 *  FACILITY:
 *
 *	DECthreads services
 *
 *  ABSTRACT:
 *
 *	Wrappers for UNIX process control functions.
 *
 *  AUTHORS:
 *
 *	Dave Butenhof
 *
 *  CREATION DATE:
 *
 *	12 February 1991
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	13 February 1991
 *		Fix initial bugs
 *	002	Dave Butenhof	26 February 1991
 *		Bugcheck on errors in child code after fork.
 *	003	Dave Butenhof	01 May 1991
 *		Add arguments to cma__bugcheck() calls.
 *	004	Paul Curtin	05 June 1991
 *		Total rewrite of fork: it now clears all existence of threads
 *		other than the forking thread in the forked process. ('cept
 *		the null thread of course)
 *	005	Paul Curtin	06 June	1991
 *		Added Al Simon's atfork work.
 *	006	Webb Scales	10 June 1991
 *		Removed cma_errno include.
 *	007	Dave Butenhof	11 June 1991
 *		Conditionalize thread I/O reinit (not used on multiprocessor
 *		implementations).  Also include cma_init.h to get
 *		cma__int_init() macro.
 *	008	Dave Butenhof	10 September 1991
 *		Modify cma_fork() wrapper to skip all the atfork() and reinit
 *		stuff and just call "raw" fork() if DECthreads hasn't been
 *		initialized yet (there's no need for the other stuff).
 *	009	Dave Butenhof	19 September 1991
 *		Integrate HPUX CMA5 reverse drop: include HP-specific header
 *		if using HP reentrant libc.
 *	010	Dave Butenhof	04 October 1991
 *		Clean up use of _CMA_UNIPROCESSOR_
 *	011	Dave Butenhof	07 January 1992
 *		Build in modification of OSF/1's malloc.c from libpthreads.a;
 *		it has pre/post fork functions that need to be called.
 *	012	Dave Butenhof	11 February 1992
 *		A law of nature has just been changed: cma__alloc_mem now
 *		returns cma_c_null_ptr on an allocation failure, rather than
 *		raising an exception. This allows greater efficiency, since
 *		TRY is generally expensive. Anyway, apply the process of
 *		evolution: adapt or die.
 *	013	Webb Scales	18 February 1992
 *		Since defered actions are now performed on both enter- and
 *		exit-kernel, replace these macros where undeferrals are
 *		unwanted.
 *	014	Dave Butenhof	12 March 1992
 *		Add tracing.
 *	015	Brian Keane	23 April 1992
 *		Allow atfork handlers to be null.
 *	016	Jerry Feldman	25 July	1992
 *		Reorder some of the reinits for child process to prevent a
 *		child thread in the child process from being dispatched. For
 *		child process, the kernel is kept locked until after all the
 *		reinits are completed.
 *	017	Webb Scales	 3 September 1992
 *		Add scheduler module reinitialization.
 *	018	Dave Butenhof	 5 October 1992
 *		Remove cma__reinit_defer()
 *	019	Dave Butenhof	Friday the 13th, November 1992
 *		Reinit the child's dispatch database (ready queues)
 *		immediately; then unlock the kernel and let the rest run.
 *		After reinit_dispatch, timeslicing won't hurt (there aren't
 *		any ready threads).
 *	020	Brian Keane	4 December 1992
 *		More fork work.  Add additional tracing calls, courtesy of
 *		Dave.  Move vp child reinitialization down after mutex
 *		reinit, since it uses a mutex.  Save tcb prior to fork, so
 *		child can reestablish identity after the fork.
 *	021	Brian Keane	19 January 1993
 *		Modified cma_fork to call cma__mach_fork in the OSF/1 AXP
 *		case, so we can provide our own version of fork.
 *	022	Webb Scales	18 February 1993
 *		Added defer module reinitialization.
 *	023	Dave Butenhof	23 March 1993
 *		Don't reinitialize VP module unless we have VPs.
 *	024	Dave Butenhof	28 March 1993
 *		When I implemented 023, I seem to have accidentally
 *		conditionalized the prefork reinit_debug as well as
 *		reinit_vp. Which ain't too cool. Re-fix it.
 */


/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_attr.h>
#include <cma_condition.h>
#include <cma_context.h>
#include <cma_deb_core.h>
#include <cma_defs.h>
#include <cma_dispatch.h>
#include <cma_sched.h>
#include <cma_stack.h>
#include <cma_signal.h>
#include <cma_defer.h>
#include <cma_once.h>
#include <cma_tcb.h>
#include <cma_timer.h>
#include <cma_kernel.h>
#include <cma_mutex.h>
#include <cma_queue.h>
#include <cma_vm.h>
#include <cma_init.h>
#if _CMA_OS_ == _CMA__UNIX
# if !_CMA_THREAD_SYNC_IO_
#  include <cma_thread_io.h>
# endif
# include <sys/time.h>
# include <signal.h>
# include <cma_ux.h>
#endif
#include <stdio.h>
#ifdef _HP_LIBC_R
# include <cma_uxcalls.h>
#endif

/*
 * GLOBAL DATA
 */

/*
 * LOCAL DATA
 */

#define CMA___C_PRE_FORK 0		/* cma_atfork definitions */
#define CMA___C_PARENT_POST_FORK 1
#define CMA___C_CHILD_POST_FORK 2
#define CMA___C_MAX_FORK_RTNS 3                           

static cma__t_queue	cma___g_fork_handler_queue;
static cma__t_int_mutex	*cma___g_fork_queue_mutex;

typedef struct CMA___T_FORK_RTNS {
    cma__t_queue queue;                 /* Front and back links           */
    cma_t_address user_state;           /* User state parameter           */
    cma_t_fork_rtn routines[CMA___C_MAX_FORK_RTNS];
                                        /* Array of fork routine pointers */
                                        /* Pre-fork, Parent post-fork     */
                                        /* Child post_fork                */
    } cma___t_fork_rtns;

/*
 * LOCAL MACROS
 */

/*
 * LOCAL FUNCTIONS
 */

static void
cma___do_fork _CMA_PROTOTYPE_ ((
        cma_t_natural which_case ));
                                     

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Perform UNIX fork(2) function, and shut down CMA in the child process
 *	(turn off timeslicing, and lock kernel to prevent erroneous use).
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
 *	The pid of the new process, or 0 in the child
 *
 *  SIDE EFFECTS:
 *
 *	Shuts down all but the forking thread in the child process.
 */
extern int
cma_fork
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    int	pid;
# if _CMA_HARDWARE_ == _CMA__ALPHA && _CMA_OSIMPL_ == _CMA__OS_OSF
    cma__t_int_tcb  *pre_fork_tcb;

    /*
     * We grab the TCB prior to doing the fork, because fork()
     * causes the child to lose identity on OSF/1 AXP; apparently
     * the PALcode unique value isn't preserved across fork().
     */
    pre_fork_tcb = cma__get_self_tcb ();
# endif

    /*
     * We're putting a lot of complication around the fork() routine to
     * handle thread system rundown and reinitialization. None of that is
     * necessary if DECthreads hasn't been initialized yet. So first, test
     * whether initialization has been done... and if not, just call fork()
     * directly and return.
     */
    if (!cma__tac_isset (&cma__g_init_started))
	return fork ();

    cma__trace ((
	    cma__c_trc_wrp,
	    "(fork) beginning prefork actions"));

    /*
     * Call do_fork specifying that we're just prior to fork.
     * This for atfork() functionality.
     */
    cma___do_fork (CMA___C_PRE_FORK);

    /*
     * Call reinit routines passing 0 to indicate pre-fork reinitialize.  For
     * the most part these routines simply perform a lock on internal locks.
     */

    cma__reinit_debug (cma__c_reinit_prefork_lock);
#if _CMA_KTHREADS_ != _CMA__NONE
    cma__reinit_vp (cma__c_reinit_prefork_lock);
#endif
    cma__reinit_mutex (cma__c_reinit_prefork_lock);
    cma__reinit_signal (cma__c_reinit_prefork_lock);
    cma__reinit_memory (cma__c_reinit_prefork_lock);
    cma__reinit_timer (cma__c_reinit_prefork_lock);
#if !_CMA_THREAD_SYNC_IO_ && (_CMA_OS_ == _CMA__UNIX)
    cma__reinit_thread_io (cma__c_reinit_prefork_lock);
#endif
    cma__reinit_tcb (cma__c_reinit_prefork_lock);
    cma__reinit_once (cma__c_reinit_prefork_lock);
    cma__reinit_dispatch (cma__c_reinit_prefork_lock);
    cma__reinit_sched (cma__c_reinit_prefork_lock);
    cma__reinit_stack (cma__c_reinit_prefork_lock);
    cma__reinit_context (cma__c_reinit_prefork_lock);
    cma__reinit_cv (cma__c_reinit_prefork_lock);
    cma__reinit_attr (cma__c_reinit_prefork_lock);
#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
    /*
     * If we're on OSF/1 system and using libc_r.a instead of wrappers, then
     * we're also using cma_malloc.c, and need to call its prefork reinit.
     */
    cma__reinit_malloc (cma__c_reinit_prefork_lock);
#endif
    cma__trace ((
	    cma__c_trc_wrp,
	    "(fork) finished prefork actions"));

    cma__int_lock (cma__g_global_lock);
    cma__set_kernel ();		/* No interferance! */

    /*
     * On OSF/1 AXP systems, we provide  fork(), so we call
     * the internal routine to do the actual system fork stuff.
     */
#if _CMA_HARDWARE_ == _CMA__ALPHA && _CMA_OSIMPL_ == _CMA__OS_OSF
    pid = cma__mach_fork();
#else
    pid = fork();
#endif

    if (pid != 0) {				/* parent process */

	/*
	 * Exit kernel and unlock all locks in the parent process.
	 */
	cma__unset_kernel ();
	cma__int_unlock (cma__g_global_lock);
#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
	/*
	 * If we're on OSF/1 system and using libc_r.a instead of wrappers,
	 * then we're also using cma_malloc.c, and need to call its prefork
	 * reinit.
	 */
	cma__reinit_malloc (cma__c_reinit_postfork_unlock);
#endif
	cma__reinit_attr (cma__c_reinit_postfork_unlock);
	cma__reinit_cv (cma__c_reinit_postfork_unlock);
	cma__reinit_context (cma__c_reinit_postfork_unlock);
	cma__reinit_stack (cma__c_reinit_postfork_unlock);
	cma__reinit_dispatch (cma__c_reinit_postfork_unlock);
	cma__reinit_sched (cma__c_reinit_postfork_unlock);
	cma__reinit_once (cma__c_reinit_postfork_unlock);
	cma__reinit_tcb (cma__c_reinit_postfork_unlock);
#if !_CMA_THREAD_SYNC_IO_ && (_CMA_OS_ == _CMA__UNIX)
	cma__reinit_thread_io (cma__c_reinit_postfork_unlock);
#endif
	cma__reinit_timer (cma__c_reinit_postfork_unlock);
	cma__reinit_memory (cma__c_reinit_postfork_unlock);
	cma__reinit_signal (cma__c_reinit_postfork_unlock);
	cma__reinit_mutex (cma__c_reinit_postfork_unlock);
#if _CMA_KTHREADS_ != _CMA__NONE
	cma__reinit_vp (cma__c_reinit_postfork_unlock);
#endif
	cma__reinit_debug (cma__c_reinit_postfork_unlock);

         /*
          * Call do fork specifying that we're in parent after fork.
          * This for atfork() functionality.
          */
	cma___do_fork (CMA___C_PARENT_POST_FORK);
	cma__trace ((
		cma__c_trc_wrp,
		"(fork) parent completed postfork actions"));
	}
    else {					/* child process */

	/*
	 * Reinitialize required data structures, and remove all other
	 * threads from the known threads list in forked process.
	 */

	/*
	 * First order of business is to restore the saved tcb using the
	 * PALcode call.
	 */

# if _CMA_HARDWARE_ == _CMA__ALPHA && _CMA_OSIMPL_ == _CMA__OS_OSF
	cma__set_unique ((long int)pre_fork_tcb);
# endif

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
	cma__tac_set (&cma__g_defer_avail);
#endif
	cma__tac_set (&cma__g_cv_defer);

	/*
	 * FIX-ME:
	 *
	 * In most cases, the child needs to call both the clear and unlock
	 * reinit function codes for each module. Those could be implemented
	 * by having "clear" fall through (in sequential cases of a switch
	 * statement) to "unlock" (which needs to be executed without the
	 * clear in the parent). However, some clear and unlock operations
	 * need to be performed independently (such as CV).
	 */
	cma__reinit_dispatch (cma__c_reinit_postfork_clear);
	cma__reinit_timer (cma__c_reinit_postfork_clear);
	cma__reinit_sched (cma__c_reinit_postfork_clear);
	cma__reinit_cv (cma__c_reinit_postfork_clear);
	cma__unset_kernel ();	
	cma__trace ((
		cma__c_trc_wrp,
		"(fork) child reinit timer & dispatch complete"));
	cma__int_unlock (cma__g_global_lock);
#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
	/*
	 * If we're on OSF/1 system and using libc_r.a instead of wrappers,
	 * then we're also using cma_malloc.c, and need to call its prefork
	 * reinit.
	 */
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit malloc"));
	cma__reinit_malloc (cma__c_reinit_postfork_unlock);
#endif

	cma__trace ((cma__c_trc_wrp, "(fork) child reinit attr"));
	cma__reinit_attr (cma__c_reinit_postfork_clear);
	cma__reinit_attr (cma__c_reinit_postfork_unlock);
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit cv"));
	cma__reinit_cv (cma__c_reinit_postfork_unlock);
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit context"));
	cma__reinit_context (cma__c_reinit_postfork_clear);
	cma__reinit_context (cma__c_reinit_postfork_unlock);
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit stack"));
	cma__reinit_stack (cma__c_reinit_postfork_clear);
	cma__reinit_stack (cma__c_reinit_postfork_unlock);
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit once"));
	cma__reinit_once (cma__c_reinit_postfork_clear);
	cma__reinit_once (cma__c_reinit_postfork_unlock);
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit tcb"));
	cma__reinit_tcb (cma__c_reinit_postfork_clear);
	cma__reinit_tcb (cma__c_reinit_postfork_unlock);
#if !_CMA_THREAD_SYNC_IO_ && (_CMA_OS_ == _CMA__UNIX)
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit thread_io"));
	cma__reinit_thread_io (cma__c_reinit_postfork_clear);
	cma__reinit_thread_io (cma__c_reinit_postfork_unlock);
#endif
#if !_CMA_THREAD_IS_VP_ && (_CMA_OS_ == _CMA__UNIX)
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit defer"));
	cma__reinit_defer (cma__c_reinit_postfork_clear);
	cma__reinit_defer (cma__c_reinit_postfork_unlock);
#endif
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit memory"));
	cma__reinit_memory (cma__c_reinit_postfork_clear);
	cma__reinit_memory (cma__c_reinit_postfork_unlock);
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit signal"));
	cma__reinit_signal (cma__c_reinit_postfork_clear);
	cma__reinit_signal (cma__c_reinit_postfork_unlock);
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit mutex"));
	cma__reinit_mutex (cma__c_reinit_postfork_clear);
	cma__reinit_mutex (cma__c_reinit_postfork_unlock);
#if _CMA_KTHREADS_ != _CMA__NONE
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit vp"));
	cma__reinit_vp (cma__c_reinit_postfork_clear);
	cma__reinit_vp (cma__c_reinit_postfork_unlock);
#endif
	cma__trace ((cma__c_trc_wrp, "(fork) child reinit debug"));
	cma__reinit_debug (cma__c_reinit_postfork_clear);
	cma__reinit_debug (cma__c_reinit_postfork_unlock);

        /*
         * Allow libc_r to reinitialize its locks.
         */
#if (_CMA_OSIMPL_ == _CMA__OS_OSF) && _CMA_REENTRANT_CLIB_
        cma__trace ((cma__c_trc_wrp, "(fork) child reinit libc_r"));
        libc_locks_reinit();
#endif
 
	/*
	 * Call do fork specifying that we're in child just after fork. This
	 * for atfork() functionality.
	 */
	cma__trace ((cma__c_trc_wrp, "(fork) child user post fork"));
	cma___do_fork (CMA___C_CHILD_POST_FORK);
	cma__trace ((
		cma__c_trc_wrp,
		"(fork) child completed postfork actions"));
	}

    return pid;
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Register caller's fork handler routines.
 *
 *  FORMAL PARAMETERS:
 *
 *      user_state - the pointer to user state--passed to each routine.
 *	pre_fork - the routine to be called before performing the fork.
 *      parent_fork - the routine to be called after, in the parent.
 *      child_fork - the routine to be called after, in the child.
 *
 *  IMPLICIT INPUTS:
 *
 *	cma___g_fork_handler_queue, cma___g_fork_queue_mutex
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
 *	allocates memory.
 */
extern void
cma_atfork
#ifdef _CMA_PROTO_
        (
	cma_t_address user_state,
	cma_t_fork_rtn pre_fork,
        cma_t_fork_rtn parent_fork,
        cma_t_fork_rtn child_fork)
#else   /* no prototypes */
        (user_state, pre_fork, parent_fork, child_fork)
	cma_t_address user_state;
        cma_t_fork_rtn pre_fork;
        cma_t_fork_rtn parent_fork;
        cma_t_fork_rtn child_fork;
#endif  /* prototype */
    {
    cma___t_fork_rtns * entry;

    /*
     * Initialize CMA if it has not already been done.
     */
    cma__int_init();

    /*
     * Allocate a new fork routines structure to hold them.
     */
    entry = cma__alloc_object (cma___t_fork_rtns);

    /*
     * FIX-ME:
     *
     * It doesn't seem right for "atfork" to return exceptions. I'm not going
     * to change the prototype to return a value right now, but we ought to
     * consider that later.
     */
    if ((cma_t_address)entry == cma_c_null_ptr)
	cma__error (exc_s_insfmem);

    /*
     * Populate the new entry.
     */
    entry->user_state = user_state;
    entry->routines [CMA___C_PRE_FORK] = pre_fork;
    entry->routines [CMA___C_PARENT_POST_FORK] = parent_fork;
    entry->routines [CMA___C_CHILD_POST_FORK] = child_fork;

    /*
     * Take the lock to assure consistency and visibility.
     */
    cma__int_lock (cma___g_fork_queue_mutex);

    /*
     * Insert the entry at the head of the queue so we get LIFO order.
     */
    cma__queue_insert_after ((cma__t_queue *)entry, &cma___g_fork_handler_queue);

    /*
     * Release the lock.
     */
    cma__int_unlock (cma___g_fork_queue_mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Call the registered fork handlers for the appropriate fork case.
 *
 *  FORMAL PARAMETERS:
 *
 *	which_case - whether this is pre-fork, post-fork parent, or 
 *                   post-fork child.
 *
 *  IMPLICIT INPUTS:
 *
 *	the fork handler queue and fork handler mutex.
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
cma___do_fork
#ifdef _CMA_PROTO_
	(
	cma_t_natural which_case)
#else	/* no prototypes */
	(which_case)
	cma_t_natural which_case;
#endif	/* prototype */
    {
    cma___t_fork_rtns * entry;

    /*
     * Take the lock on the queue to assure data visibility and 
     * consistency.
     */
    cma__int_lock (cma___g_fork_queue_mutex);

    /*
     * If the queue is empty we have nothing to do.
     */
    if (!cma__queue_empty (&cma___g_fork_handler_queue)) {
	entry = (cma___t_fork_rtns *) cma__queue_next (&cma___g_fork_handler_queue);

	do {
	    if ((cma_t_address)entry->routines[which_case] != cma_c_null_ptr)
		(*(entry->routines[which_case]))(entry->user_state);
	    entry = (cma___t_fork_rtns *) cma__queue_next (
			(cma__t_queue *) entry);
	    } while (entry != (cma___t_fork_rtns *) &cma___g_fork_handler_queue);
	}

    /*
     * Release the lock.
     */
    cma__int_unlock (cma___g_fork_queue_mutex);
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	Init the fork handlers mechanism.
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	the fork handler queue and fork handler mutex.
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
cma__init_atfork
#ifdef _CMA_PROTO_
        (void)
#else   /* no prototypes */
        ()
#endif  /* prototype */
    {
    cma___g_fork_queue_mutex = cma__get_mutex (&cma__g_def_attr);
    cma__obj_set_name (cma___g_fork_queue_mutex, "atfork queue");
    cma__queue_init (&cma___g_fork_handler_queue);
    }
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_UX.C */
/*  *27   28-MAR-1993 14:30:43 BUTENHOF "Move reinit_debug out of KTHREAD conditional :-) " */
/*  *26   23-MAR-1993 12:51:04 BUTENHOF "Don't call reinit_vp on non-kthread" */
/*  *25   23-FEB-1993 16:42:24 SCALES "Fix two wake-up waiting races in the null thread" */
/*  *24   19-JAN-1993 16:06:34 KEANE "Modified cma_fork to call cma__mach_fork on OSF/1 AXP" */
/*  *23    5-DEC-1992 10:17:46 KEANE "Fix various OSF/1 AXP fork problems" */
/*  *22   13-NOV-1992 13:01:34 BUTENHOF "Fix fork wrapper" */
/*  *21    5-OCT-1992 15:06:18 BUTENHOF "Fix defer init" */
/*  *20    5-OCT-1992 15:03:53 SCALES "Add re/initialization calls for scheduler module" */
/*  *19   30-JUN-1992 16:28:41 G_FELDMAN "Changed the ordering of the child fork reinits" */
/*  *18   21-MAY-1992 10:57:19 KEANE "Allow null atfork handlers to be specified" */
/*   14A1 17-APR-1992 08:34:06 KEANE "Apply undeferral fix to improved BL9" */
/*  *17   13-MAR-1992 14:10:46 BUTENHOF "Add tracing" */
/*  *16   19-FEB-1992 13:50:37 SCALES "Perform undeferal on enter-kernel" */
/*  *15   18-FEB-1992 15:31:27 BUTENHOF "Adapt to new alloc_mem protocol" */
/*   10A1 20-JAN-1992 17:15:27 SCALES "Integrate changes for Tin malloc()" */
/*  *14    7-JAN-1992 17:27:35 BUTENHOF "Call malloc pre/post fork functions on OSF/1 kthreads" */
/*  *13   14-OCT-1991 13:42:19 BUTENHOF "Refine/fix use of config symbols" */
/*  *12   24-SEP-1991 16:29:52 BUTENHOF "Call raw fork() in cma_fork if CMA not inited" */
/*  *11   17-SEP-1991 13:24:58 BUTENHOF "Call raw fork() in cma_fork if CMA not inited" */
/*  *10   12-JUN-1991 12:21:55 BUTENHOF "Catch another call to reinit_thread_io" */
/*  *9    11-JUN-1991 16:32:46 BUTENHOF "Fix reinit code" */
/*  *8    10-JUN-1991 18:25:06 SCALES "Add sccs headers for Ultrix" */
/*  *7    10-JUN-1991 17:56:03 SCALES "Remove include of cma_errno" */
/*  *6     6-JUN-1991 11:22:14 CURTIN "Added Al Simon's atfork work" */
/*  *5     5-JUN-1991 16:16:08 CURTIN "fork work" */
/*  *4     2-MAY-1991 13:59:54 BUTENHOF "Add argument to cma__bugcheck() calls" */
/*  *3    27-FEB-1991 15:48:33 BUTENHOF "Bugcheck on errors in child reset code" */
/*  *2    14-FEB-1991 23:51:14 BUTENHOF "Fix it up" */
/*  *1    13-FEB-1991 00:04:34 BUTENHOF "UNIX process control wrappers" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_UX.C */
