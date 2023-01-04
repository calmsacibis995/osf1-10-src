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
 * @(#)$RCSfile: cma_signal.c,v $ $Revision: 4.2.10.3 $ (DEC) $Date: 1993/12/10 20:18:09 $
 */
/*
 *  FACILITY:
 *
 *	CMA services
 *
 *  ABSTRACT:
 *
 *	This module contains routines concerned with handling Unix signals.
 *
 *  AUTHORS:
 *
 *	Webb Scales
 *
 *  CREATION DATE:
 *
 *	16 March 1990
 *
 *  MODIFICATION HISTORY:
 *
 *	001	Webb Scales	29 March 1990
 *		Skip timeslice signal during initialization.
 *	002	Webb Scales	29 March 1990
 *		Replace 'sigset_t' by 'int'.
 *	003	Webb Scales	9 April 1990
 *		Return to 'sigset_t' and incorporate Apollo changes.
 *		Conditionalize code for VMS.
 *	004	Webb Scales	16 April 1990
 *		Fix typo/think-o off-by-one loop index problem.
 *		Remove redundant cma_host.h include.
 *	005	Webb Scales	26 April 1990
 *		Change sigsetmask to sigprocmask.
 *	006	Webb Scales	27 April 1990
 *		Integrate Apollo changes for BL2 (change sigvec to sigaction).
 *	007	Dave Butenhof	28 May 1990
 *		Include new cma_px.h instead of cma_crtlx.h for sigset_t.
 *	008	Webb Scales	30 May 1990
 *		Reorder module initialization so that the sigwait mutex and cv
 *		are created before the signal handlers are set up.
 *	009	Webb Scales	4 June 1990
 *		Establish timeslicer signal handler with other signal handlers.
 *		Add more exceptions (specific instruction and arithmetics).
 *		Allow 'sigwait'ing on SIGQUIT.
 *	010	Webb Scales	5 June 1990
 *		Bring sigwait up to POSIX spec.  Make sigwait code compileable
 *		on VMS.
 *	011	Dave Butenhof	14 June 1990
 *		Replace cma__test_and_set/cma__unset by cma__kernel_set and
 *		cma__kernel_unset.
 *	012	Dave Butenhof	03 August 1990
 *		Include cma_defer.h (new home of deferral code).
 *	013	Paul Curtin	06 August 1990
 *		Replace abort w/ cma__abort
 *		Replace printf w/ cma__put* functions
 *		Replace cma__unblock_all_waiting w/ cma__sem_wake_all
 *	014	Webb Scales	16 August 1990
 *		Integrate Sun port
 *	015	Dave Butenhof	27 August 1990
 *		Change CMA interfaces to pass handles & structures by reference.
 *	016	Webb Scales	29 August 1990
 *		Replaced instances of RAISE with calls to cma_error.
 *	017	Webb Scales	 8 October 1990
 *		Fixed sigwait to use errno instead of exceptions.
 *		Changed SIGTSTP handling to be like SIGQUIT.
 *	018	Dave Butenhof	25 October 1990
 *		Add name to mutexes created at init time.
 *	019	Webb Scales	29 October 1990
 *		sigwait now waits on thread-specific CV.
 *	020	Paul Curtin	11 December 1990
 *		Implemented sigaction for synchronous signals
 *	021	Dave Butenhof	27 December 1990
 *		Rename exceptions.
 *	022	Dave Butenhof	22 January 1991
 *		Fix exception names
 *	023	Dave Butenhof	1 February 1991
 *		Correct order of args to cma__int_wait to conform to external
 *		interface (cv, mutex).
 *	024	Dave Butenhof	20 February 1991
 *		Try to remove an annoying warning message by adding a
 *		spurious cast in an assignment of identically typed pointers.
 *	025	Dave Butenhof	26 February 1991
 *		Make changes for IBM RS/6000 AIX. SIGIOT and SIGLOST are
 *		synonyms,
 *	026	Webb Scales	12 March 1991
 *		Added an alert-test to sigwait to make it a full alert point.
 *	027	Webb Scales	12 March 1991
 *		Merged Apollo changes to CD4
 *	028	Dave Butenhof	05 April 1991
 *		Add OSF/1 signals to get through cma__init_signal.
 *	029	Paul Curtin	08 April 1991
 *		Added code for quick kill.
 *	030	Paul Curtin	06 May 1991
 *		Replaced call to cma_init() with cma__int_init().
 *	031	Dave Butenhof	08 May 1991
 *		Disable the force_dispatch based "quick kill" on kernel
 *		thread implementations, since it won't work... use
 *		cma__vp_interrupt() instead.
 *	032	Paul Curtin	13 May 1991
 *		Replaced a number of external interace calls w/internal
 *		calls/macros.
 *	033	Dave Butenhof	30 May 1991
 *		Add an fflush(stdout) call to cma__abort_process, since
 *		sometimes it doesn't call cma__abort().
 *	034	Dave Butenhof	30 May 1991
 *		Fix wblk structure to use internal CV pointer, since it's
 *		signalled using internal interface (fix to edit 032).
 *	035	Paul Curtin	3  June 1991
 *		Added a new reinit routine.
 *	036	Dave Butenhof	04 June 1991
 *		Augment #034's fix of #032 by also changing the other half of
 *		the CV functions to internal versions (cond_signal_int).
 *	037	Dave Butenhof	05 June 1991
 *		Integrate IBM reverse drop.
 *	038	Paul Curtin	06 June 1991
 *		Re replaced index in for loop, instead of a 1.
 *	039	Webb Scales and Dave Butenhof	    10 June 1991
 *		- Conditionalize inclusion of I/O stuff.
 *		- POSIX-ize signal stuff for OSF/Ultrix
 *	040	Webb Scales	11 July 1991
 *		Fixed a bug which eats all but the first signal if multiple
 *		signals are already pending when sigwait() is called.
 *	041	Paul Curtin	20 August 1991
 *		Conditionalized out the inclusion stdio.h on VMS
 *	042	Paul Curtin	03 September 1991
 *		Insure process signal mask restore on sync term, qar 134.
 *	043	Dave Butenhof	18 September 1991
 *		Merge in IBM, HPUX, and Apollo reverse drops for CMA5.
 *	044	Dave Butenhof	02 October 1991
 *		Integrate changes provided by Alan Peckham to unprotect guard
 *		pages on all stacks before aborting the process; this allows
 *		the UNIX core dump code (which makes the absurd assumption
 *		that the known universe stops just because a page is
 *		protected) to work on threaded processes. Also, integrate code
 *		for an option to allow setting synch. term. signals to abort
 *		the process with a core dump (this may later be enabled
 *		through a cma_debug() command).
 *	045	Dave Butenhof	04 October 1991
 *		Generalize a few conditionals (like checking for whether
 *		signal name is defined rather than specific platforms).
 *		Suggested by Ken Ouellette. There are probably other places
 *		where this could be done, but I don't have access to signal.h
 *		for all the platforms, and don't care to guess whether the
 *		signals are truly not defined, or whether (for whatever
 *		reason) those who ported DECthreads to the platform chose not
 *		to handle them.
 *	046	Webb Scales	18 November 1991
 *		- Corrected routine header for the default signal handler
 *		- Fixed routine returns for the default signal handler
 *		- Removed all references to cma___c_sigacted
 *	047	Dave Butenhof	26 November 1991
 *		Include unistd.h to make ANSI C checking happy.
 *	048	Dave Butenhof	27 November 1991
 *		Conditionalize more code against VMS, since Alpha DEC C is
 *		more picky than VAX C.
 *	049	Dave Butenhof	19 March 1992
 *		Fix some holes in per-thread sigaction, reported by Dale
 *		Tonogai (HP).
 *	050	Webb Scales	6 April 1992
 *		- Modified sigwait to only install signal handlers when they
 *		  are needed.  
 *		- Removed the latent code for termination on synchronous
 *		  terminating signals, as unhandled exceptions will now 
 *		  terminate the process at the point where they are raised.
 *		- Reworked the abort-process routine to be much simpler.
 *		- Simplified unhandled async. term. signal handling.  
 *		  DEC Non-kernel implementations will call abort-process 
 *		  without bothering to switch contexts.
 *		- Reworked the module to allow it to be build on VMS for
 *		  use with C RTL support.
 *	051	Webb Scales	18 May 1992
 *		Cleanse the wblk links before inserting it into the queue.
 *	052	Brian Keane 	21 May 1992
 *		Add missing typecast on queue operation.
 *	053	Dave Butenhof	26 August 1992
 *		Set global 'cma__g_defer_avail' instead of calling cma__defer
 *		for deferred I/O available check.
 *	054	Dave Butenhof	20 November 1992
 *		Fix vp_interrupt call.
 *	054	Brian Keane	26 January 1993
 *		Remove synchronous terminating signals from the blocked mask
 *	        for OSF/1 AXP.  Also, use special sa_flags value to pick
 *		up Shashi's special fix for DEC Ada.
 *	055	Dave Butenhof	12 April 1993
 *		Add argument to cma__int[_timed]_wait() to avoid extra
 *		cma__get_self_tcb() call.
 *	056	Webb Scales	13 April 1993
 *		Move SIGXCPU and SIGXFSZ from "synchronous" to "asynchronous"
 *		category.
 *	057	Dave Butenhof	14 April 1993
 *		Fix cma__queue_remove call for new prototype (to allow use of
 *		INSQUE/REMQUE on VAX).
 *	058	Brian Keane	22 April 1993
 *		Add handlers for sync terminating signals (inadvertantly removed).
 *	059	Webb Scales	 7 May 1993
 *		Revamp sigwait() for DEC OSF/1.
 *	060	Webb Scales	 2 June 1993
 *		Change the sigwait spinlock mechanism to use sigsetmask() on
 *		Ultrix, since sigprocmask() is not atomic there.
 *	061	Webb Scales	 8 June 1993
 *		- Rework sigwait-lock macros slightly.
 *		- Disallow sigwait'ing on SIGINFO.
 *		- Send spurious signal to ensure that any pending signals are
 *		delivered, on OSF/1.  (Work around OS kernel bug.)
 */

/*
 *  INCLUDE FILES
 */

#include <cma.h>
#include <cma_defs.h>
#include <cma_alert.h>
#include <cma_errors.h>
#include <cma_queue.h>
#include <cma_handle.h>
#include <cma_init.h>
#include <cma_condition.h>
#include <cma_mutex.h>
#include <cma_tcb_defs.h>
#include <cma_tcb.h>
#include <cma_stack.h>
#include <cma_defer.h>
#include <cma_timer.h>
#include <cma_sigwait.h>
#include <cma_util.h>
#include <cma_int_errno.h>
#include <cma_assem.h>
#include <cma_vp.h>
#include <cma_signal.h>
#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
# include <cma_thread_io.h>
# include <unistd.h>
#endif
#include <signal.h>
#if _CMA_OS_ == _CMA__UNIX
# include <stdio.h>
#endif
#ifdef _HP_LIBC_R
# include <cma_sigcalls.h>
#endif

/*
 * Global data
 */

#if _CMA_OS_ != _CMA__VMS
cma_t_integer	    cma__g_abort_signal;	/* Signal sent to abort proc */
#endif

#if _CMA_NO_POSIX_SIGNAL_
typedef int sigset_t;

/*
 * Because the Sun cpp doesn't distinguish between macros that take args
 * and those that don't, we have to create a new type name sigaction_t
 * so we can define a macro for the sigaction() function.
 */
typedef struct {
    void     (*sa_handler)();
    sigset_t sa_mask;
    int      sa_flags;
} sigaction_t;

# define SIG_BLOCK    1
# define SIG_UNBLOCK  2
# define SIG_SETMASK  3
 
# define sigaction(sig, act, oact)    cma___sigaction(sig, act, oact)
# define sigprocmask(how, set, oset)  cma___sigprocmask(how, set, oset)
# define sigaddset(maskp, sig)        (*(maskp) |= sigmask(sig), 0)
# define sigdelset(maskp, sig)        (*(maskp) &= ~sigmask(sig), 0)
# define sigemptyset(maskp)           (*(maskp) = 0, 0)
# define sigfillset(maskp)            (*(maskp) = -1, 0)
# define sigismember(maskp, sig)      ((*(maskp) & sigmask(sig)) ? 1 : 0)
#else
/*** See note above as to why we have to create a new type name */
typedef struct sigaction sigaction_t;
#endif

/*
 * LOCAL TYPEDEFS
 */
typedef	    void (*cma___t_sig_hand_ptr) _CMA_PROTOTYPE_((int));

#if (_CMA_PLATFORM_ == _CMA__IBMR2_UNIX) || (_CMA_OSIMPL_ == _CMA__OS_OSF)
typedef void (*cma___t_sighandler)(int, int, struct sigcontext *);
#endif

typedef struct CMA___T_SIGWAIT_BLOCK {
    cma__t_queue	queue;	/* Must be the first field */
    sigset_t		*set;	/* Signals which thread is awaiting */
    cma_t_integer	signal;	/* Signal that the thread received */
    cma__t_int_cv	*cv;	/* Condition variable thread blocks on */
    cma__t_int_tcb	*tcb;	/* Just for debugging */
    } cma___t_sigwait_block;
		    

/*
 * LOCAL DATA
 */

static cma__t_queue	cma___g_sigwait_queue;
static cma_t_integer	cma___g_sigwaiters[NSIG];
static sigset_t		cma___g_sig_wait_mask;
static sigset_t		cma___g_sigact_block_mask;
static sigset_t		cma___g_sig_block_mask;
#if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
static char		ibmr2_vtalrm_stack_area[8192];
static caddr_t		ibmr2_vtalrm_stack = 
			    ((caddr_t) &ibmr2_vtalrm_stack_area[8191]);
#endif

/*
 * LOCAL MACROS
 */

/*
 * No special actions are required for non-terminating signals.
 */
#define	cma___sig_async_nonterm	(cma___t_sig_hand_ptr)cma__sig_deliver
#define	cma___sig_sync_nonterm	(cma___t_sig_hand_ptr)cma__sig_deliver

/*
 * Acquiring the sigwait "interlock" is done by setting the process signal
 * mask to block all signals.  If the "old mask" indicates that all signals
 * were already blocked, then it is deemed that the acquisition failed
 * (because some other thread currently holds the "interlock").  On a kernel
 * threads platform, the operation is retried, like a spinlock, until it
 * succeeds; on other platforms this is a deadlock, so we bugcheck.  Since all
 * signals are blocked automatically during signal delivery, while signal
 * delivery is in progress the signal handler will hold the "interlock".  Note
 * that if some thread currently holds the "interlock", no signal delivery can
 * occur.  So, this form of "interlock" provides synchronization for threads
 * as well as signal delivery.
 */
#if _CMA_THREAD_IS_VP_
# define cma___sigwait_lock(prev_mask) \
    while (1) { \
	if ((prev_mask = sigsetmask (cma___g_sig_block_mask)) == -1) \
	    cma__bugcheck ("cma___sigwait_lock: sigprocmask failed"); \
	if ((prev_mask & cma___g_sig_block_mask) != cma___g_sig_block_mask) \
	    break; \
	cma__vp_yield (); \
	}

# define cma___sigwait_unlock(prev_mask) \
    (sigprocmask (SIG_SETMASK, &prev_mask, cma_c_null_ptr) == -1 \
	? (cma__bugcheck ("cma___sigwait_unlock: sigprocmask failed"), 0) : 0)

# if (_CMA_OSIMPL_ == _CMA__OS_OSF) && (_CMA_VENDOR_ == _CMA__DIGITAL)
#  define cma___sigwait_wakeup()    kill (getpid (), SIGINFO)
# else
#  define cma___sigwait_wakeup()
# endif
#else
# define cma___sigwait_lock(prev_mask) \
    ((prev_mask = sigsetmask (cma___g_sig_block_mask)) == -1\
	? (cma__bugcheck ("cma___sigwait_lock: sigsetmask failed"), 0) \
	: ((prev_mask & cma___g_sig_block_mask) == cma___g_sig_block_mask \
	    ? (cma__bugcheck ("cma___sigwait_lock: deadlock"), 0) \
	    : 0))

# define cma___sigwait_unlock(prev_mask) \
    (sigsetmask (prev_mask) == -1 \
	? (cma__bugcheck ("cma___sigwait_unlock: sigsetmask failed"), 0) : 0)

#  define cma___sigwait_wakeup()
#endif


#if _CMA_OS_ == _CMA__VMS
# define sigmask(sig)			(1 << ((sig)-1))
# define sigaddset(maskp, sig)		(*(maskp) |= sigmask(sig), 0)
# define sigdelset(maskp, sig)		(*(maskp) &= ~sigmask(sig), 0)
# define sigfillset(maskp)		(*(maskp) = -1, 0)
# define sigemptyset(maskp)		(*(maskp) = 0, 0)
# define sigismember(maskp, sig)	(((*(maskp)) & sigmask(sig)) != 0)

# define SIG_BLOCK    1
# define SIG_UNBLOCK  2
# define SIG_SETMASK  3
# define sigprocmask(how, set, oset)	\
	( (how == SIG_BLOCK ? sys$setast (0)  \
	: (how == SIG_UNBLOCK ? sys$setast (1) \
	: (how == SIG_SETMASK ? sys$setast (1) \
	: (cma__bugcheck ("bad value in sigprocmask macro"), 0)))), 0)
#endif

/*
 * LOCAL FUNCTIONS
 */

#if _CMA_OS_ != _CMA__VMS
static void
cma___sig_async_term	_CMA_PROTOTYPE_ ((
	int			sig,
	int			code,
	struct	sigcontext	*scp));

static void
cma___sig_io	_CMA_PROTOTYPE_ ((
	int			sig,
	int			code,
	struct	sigcontext	*scp));

static void
cma___sig_null_handler	_CMA_PROTOTYPE_ ((
	int			sig,
	int			code,
	struct	sigcontext	*scp));

static void
cma___sig_sync_term _CMA_PROTOTYPE_ ((
	int			sig,
	int			code,
	struct	sigcontext	*scp));

# if _CMA_NO_POSIX_SIGNAL_
static int 
cma___sigaction _CMA_PROTOTYPE_ ((
	int                     sig,
	sigaction_t             *act,
	sigaction_t             *oact));

static int 
cma___sigprocmask _CMA_PROTOTYPE_ ((
	int                     how,
	sigset_t                *set,
	sigset_t                *oset));
# endif
#endif


#if _CMA_OS_ != _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_sigaction:  POSIX sigaction function.
 *
 *	This routine allows any thread to establish a handler for
 *	(almost) any synchronous signal.
 *
 *
 *	Note:  as per POSIX, only one thread is awakened per signal delivery.
 *
 *  FORMAL PARAMETERS:
 *
 *	sig -- signal for which the user wishes to establish a handler in
 *		a particular thread.
 *
 *	act -- a sigaction structure containing
 *		    sa_handler	- routine to handle signal
 *		    sa_mask	_ signals to block out while in sa_handler
 *		    sa_flags	- any flags currently set
 *
 *	oact -- old values of act from most recent call to sigaction for the
 *		corresponding thread and signal if any.
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
 *      A 0 return value indicated that the call succeeded.  A -1 return
 *      value indicates an error occurred and errno is set to indicated
 *      the reason.
 *
 *  SIDE EFFECTS:
 *
 *	Establishes handlers for specific signal, removes any previous
 *	handler for this signal.  The user must take care to reestablish
 *	the previous handler if they so wish for it to function at another
 *	time.
 */
extern int
cma_sigaction
#ifdef _CMA_PROTO_
	(
	int sig,
	sigaction_t *act,
	sigaction_t *oact
	)
#else	/* no prototypes */
	(sig, act, oact)
	int    sig;
	sigaction_t *act;
	sigaction_t *oact;
#endif	/* prototype */
    {
    cma__t_int_tcb	*tcb;	    /* get tcb to access sigaction array */
    sigset_t		sig_mask;   /* hold process-wide blocked signals */

    cma__int_init ();
    /*
     * Obtain the mask for signals currently blocked by the process.
     */

    if (sigprocmask (SIG_BLOCK, (sigset_t *)cma_c_null_ptr, &sig_mask) == -1)
	cma__bugcheck ("sigaction:1");

     /*
      * Check to see that the signal requested in this sigaction
      * call is not specifically disallowed by our sigaction mask
      * and that it is currently blocked by the process.
      */

    if (sigismember (&cma___g_sigact_block_mask, sig)
#ifdef currently_undefined_by_POSIX
	    || (!sigismember (&sig_mask, sig))
#endif
	) return (cma__set_errno (EINVAL), -1);

    tcb = cma__get_self_tcb ();

    /*
     * Fix-me: cma_sigaction cannot be called while within a sigaction
     * handler, else a deadlock occurs.  
     * Question: will other threads be searching/changing this sigaction 
     * list someday??  If so locking must be installed.  Fix-me, perhaps.
     */

    if (oact) {				/* If caller wants old handler info */

	if (tcb->sigaction_data[sig].sa_handler != 0) {
	    oact->sa_handler = tcb->sigaction_data[sig].sa_handler;
	    oact->sa_mask = tcb->sigaction_data[sig].sa_mask;
	    oact->sa_flags = tcb->sigaction_data[sig].sa_flags;
	    }
	else {				/* Must be default */
	    oact->sa_handler = SIG_DFL;
	    sigemptyset (&oact->sa_flags);
	    oact->sa_flags = 0;
	    }

	}

    if (act) {
	tcb->sigaction_data[sig].sa_handler = act->sa_handler;
	tcb->sigaction_data[sig].sa_mask = act->sa_mask;
	tcb->sigaction_data[sig].sa_flags = act->sa_flags;
	}

    return (0);

    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma_sigwait:  POSIX sigwait function.
 *
 *	This routine allows any thread to wait for any signal (more or less).
 *
 *	The caller must have blocked (masked out) all the signals specified
 *	to be awaited before calling this routine.  These signals are then
 *	reenabled (unblocked) by this routine until the thread is awakened.
 *	If there are no other threads waiting for a specific signal, then,
 *	when a that signal is delivered and the waiting thread is awakened,
 *	that signal is blocked again before the thread returns.
 *
 *	Note:  as per POSIX, only one thread is awakened per signal delivery.
 *
 *  FORMAL PARAMETERS:
 *
 *	set -- address of a mask containing bits set for each signal the thread
 *		is awaiting.
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
 *	Returns the number of the signal actually caught.
 *
 *  SIDE EFFECTS:
 *
 *	This routine messes around with the signal mask.
 *
 *	NOTE:	A thread may not attempt to catch SIGKILL or SIGSTOP.
 */
extern int
cma_sigwait
#ifdef _CMA_PROTO_
	(
	sigset_t    *set)
#else	/* no prototypes */
	(set)
	sigset_t    *set;
#endif	/* prototype */
    {
    cma_t_integer	    i;
    cma_t_boolean	    sigbitset, one_set = cma_c_false;
    _CMA_VOLATILE_ cma___t_sigwait_block   wblk;
    sigset_t		    sig_mask;
#if _CMA_OS_ != _CMA__VMS
    static sigaction_t	    sig_oact[NSIG];
#endif

    cma__int_init ();

    /*
     * For each signal, check to see if it is set in the user's mask.  If it is
     * also set in the "illegal values" mask, or if it is not set in the current
     * mask, then return an error.  Also, if there are no signals set in the 
     * user's mask, return an error.
     */

#ifdef currently_undefined_by_POSIX
    if (sigprocmask (SIG_BLOCK, (sigset_t *)cma_c_null_ptr, &sig_mask) == -1)
	cma__bugcheck ("sigwait:1");
#endif

    for (i = 1; i < NSIG; i++) {
        if ((sigbitset = sigismember (set, i))
		&& (sigismember (&cma___g_sig_wait_mask, i)
#ifdef currently_undefined_by_POSIX
		|| !sigismember (&sig_mask, i)
#endif
		))
	    return (cma__set_errno (EINVAL), -1);

	cma__assert_fail (
		((sigbitset | 1) == 1),	    /* Check for boolean result */
		"cma_sigwait: sigismember error.\n");

	one_set = one_set || sigbitset;
	}

    if (!one_set)   return (cma__set_errno (EINVAL), -1);

    cma__queue_zero ((cma__t_queue *)(&wblk.queue));
    wblk.set = (sigset_t *)set;		/* copy the pointer */
    wblk.signal = -1;			/* initialize predicate */
    wblk.tcb = cma__get_self_tcb ();
    wblk.cv  = wblk.tcb->tswait_cv;

    /*
     * Establish an interlock which prevents conflict with signal handling
     * and other threads.  (This blocks signals.)
     */
    cma___sigwait_lock (sig_mask);

    cma__queue_insert ((cma__t_queue *)&wblk.queue, &cma___g_sigwait_queue);

    /*
     * Install the appropriate DECthreads signal handler for each signal that 
     * the thread is waiting for, provided that we don't currently have a 
     * handler already in place (i.e., if there are already waiters for a 
     * given signal, then we've already put a handler in place for it).
     */
    for (i = 1; i < NSIG; i++)
	if (sigismember (set, i)) {
#if _CMA_OS_ != _CMA__VMS
	    if (cma___g_sigwaiters[i] == 0) {
		sigaction_t act;


		act.sa_mask = cma___g_sig_block_mask;
		act.sa_flags = 0;
		act.sa_handler = 
		    (i == SIGURG || i == SIGTSTP || i == SIGCONT || 
		    i == SIGCHLD || i == SIGIO || i == SIGWINCH ?
		    (cma___t_sig_hand_ptr)cma___sig_async_nonterm :
		    (cma___t_sig_hand_ptr)cma___sig_async_term);

		if (sigaction (i, &act, &sig_oact[i]) == -1)  
		    cma__bugcheck ("sigwait:3");

		}
#endif
	    /*
	     * Keep a count of how many threads are waiting on each signal 
	     * so we know when to flip bits in the signal mask, install/remove
	     * signal handlers, etc.
	     */
	    cma___g_sigwaiters[i]++;

	    /*
	     * Unblock the signal so it can be delivered.
	     */
	    if (sigdelset (&sig_mask, i) == -1) cma__bugcheck ("sigwait:4");
	    }

    /*
     * Release the interlock.  (This unblocks signals.)
     */
    cma___sigwait_unlock (sig_mask);

    /*
     * Now that the signals are unblocked, make sure any pending ones get
     * delivered.
     */
    cma___sigwait_wakeup();

    /*
     * Lock this for the condition variable wait.
     */
    cma__int_lock (wblk.tcb->tswait_mutex);

    TRY {
	if (wblk.signal == -1)
	    while (wblk.signal == -1)
		cma__int_wait (
			wblk.tcb->tswait_cv,
			wblk.tcb->tswait_mutex,
			wblk.tcb);
	else
	    cma__attempt_delivery (wblk.tcb);
	}
    FINALLY {
	cma__t_queue	*qtmp;


	cma__int_unlock (wblk.tcb->tswait_mutex);

	cma___sigwait_lock (sig_mask);

	cma__queue_remove (&wblk.queue, qtmp, cma__t_queue);

	/*
	 * Leave those signals enabled which still have threads waiting on 
	 * them, and re-block the others in this thread's sigwait set.
	 */
	for (i = 1; i < NSIG; i++)
	    if (sigismember (set, i)) {
		cma___g_sigwaiters[i]--;
#if _CMA_OS_ != _CMA__VMS
		if (cma___g_sigwaiters[i] == 0) {
		    if (sigaddset (&sig_mask, i) == -1)
			cma__bugcheck ("sigwait:8");

		    if (sigaction (
			    i, 
			    &sig_oact[i], 
			    (sigaction_t *)cma_c_null_ptr
			    ) == -1)  
			cma__bugcheck ("sigwait:9");
		    }
#endif
		}

	cma___sigwait_unlock (sig_mask);
	}
    ENDTRY

    return wblk.signal;
    }

#if _CMA_OS_ != _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__abort_process:  abort process execution.
 *
 *  FORMAL PARAMETERS:
 *
 *	Signal to be aborted with
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
 *	Deinstalls signal handlers and causes a terminating signal.
 */
extern void
cma__abort_process
#ifdef _CMA_PROTO_
	(
	cma_t_integer	abort_signal)
#else	/* no prototypes */
	(abort_signal)
	cma_t_integer	abort_signal;
#endif	/* prototype */
    {
    /*
     * We are about to suddenly terminate the process, so make sure that 
     * any pending output makes it.
     */
    (void)fflush (stdout);

#if _CMA_PROTECT_MEMORY_
    /*
     * We will likely be producing a core dump on the way out, so make sure
     * that all the pages (i.e., the guard pages) are accessible, so the
     * dumper produces a useful code dump.
     */
    cma__remap_stack_holes ();
#endif

    /*
     * Set the signal action to the default for the signal which is gonna
     * kill us.
     */
    signal (abort_signal, SIG_DFL);

    /*
     * Send the killing signal, "na zdarovia".
     */
    if (kill (getpid(), abort_signal) == -1)	/* Kill the process */
	cma__bugcheck("cma__abort_process:  could not send signal %d", abort_signal);

    }
#endif

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__init_signal:  Initialize CMA signal handling
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
 *	Registers signal handlers for (almost) all signals.
 */
extern void
cma__init_signal
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    cma_t_integer    i;


    cma__queue_init  (&cma___g_sigwait_queue);

    for (i = 1; i < NSIG; i++)
	cma___g_sigwaiters[i] = 0;

    /*
     * Mask for disallowed signals for sigwait.
     * SIGKILL and SIGSTOP cannot be caught, and therefore cannot be awaited.
     * The others are "synchronous terminating" signals, for which we are 
     * currently imposing the restriction that they cannot be awaited.  There 
     * is no need for this restriction, we are simply waiting for their use to
     * be justified.
     */
    if (sigemptyset (&cma___g_sig_wait_mask) == -1)
	cma__bugcheck ("init_signal:7");
    if (sigaddset (&cma___g_sig_wait_mask, SIGKILL) == -1)
	cma__bugcheck ("init_signal:8");
#ifdef SIGSTOP
    if (sigaddset (&cma___g_sig_wait_mask, SIGSTOP) == -1)
	cma__bugcheck ("init_signal:9");
#endif
    if (sigaddset (&cma___g_sig_wait_mask, SIGILL)  == -1)
	cma__bugcheck ("init_signal:10");
    if (sigaddset (&cma___g_sig_wait_mask, SIGTRAP) == -1)
	cma__bugcheck ("init_signal:11");
    if (sigaddset (&cma___g_sig_wait_mask, SIGIOT)  == -1)
	cma__bugcheck ("init_signal:12");
    if (sigaddset (&cma___g_sig_wait_mask, SIGEMT)  == -1)
	cma__bugcheck ("init_signal:13");
    if (sigaddset (&cma___g_sig_wait_mask, SIGFPE)  == -1)
	cma__bugcheck ("init_signal:14");
    if (sigaddset (&cma___g_sig_wait_mask, SIGBUS)  == -1)
	cma__bugcheck ("init_signal:15");
    if (sigaddset (&cma___g_sig_wait_mask, SIGSEGV) == -1)
	cma__bugcheck ("init_signal:16");
    if (sigaddset (&cma___g_sig_wait_mask, SIGSYS)  == -1)
	cma__bugcheck ("init_signal:17");
    if (sigaddset (&cma___g_sig_wait_mask, SIGPIPE) == -1)
	cma__bugcheck ("init_signal:18");
#if _CMA_VENDOR_ != _CMA__HP
# ifdef SIGXCPU
    if (sigaddset (&cma___g_sig_wait_mask, SIGXCPU) == -1)
	cma__bugcheck ("init_signal:19");
# endif
# ifdef SIGXFSZ
    if (sigaddset (&cma___g_sig_wait_mask, SIGXFSZ) == -1)
	cma__bugcheck ("init_signal:20");
# endif
#endif
#if _CMA_OSIMPL_ == _CMA__OS_OSF
    if (sigaddset (&cma___g_sig_wait_mask, SIGINFO) == -1)
	cma__bugcheck ("init_signal:20.5");
#endif
#if _CMA_VENDOR_ == _CMA__HP
    if (sigaddset (&cma___g_sig_wait_mask, _SIGRESERVE) == -1)
	cma__bugcheck ("init_signal:21");
#endif

#if _CMA_OS_ != _CMA__VMS
    /*
     * Mask for disallowed signals for sigaction.
     * As above SIGKILL AND SIGSTOP cannot be caught, other blocked signals
     * currently consist of asynchronous signals which this 
     * implementation of sigaction does not allow sigaction to handle.
     */

    if (sigemptyset (&cma___g_sigact_block_mask) == -1)
	cma__bugcheck ("init_signal:22");
    if (sigaddset (&cma___g_sigact_block_mask, SIGKILL) == -1)
	cma__bugcheck ("init_signal:23");
    if (sigaddset (&cma___g_sigact_block_mask, SIGSTOP) == -1)
	cma__bugcheck ("init_signal:24");
    if (sigaddset (&cma___g_sigact_block_mask, SIGHUP) == -1) 
	cma__bugcheck ("init_signal:25");
    if (sigaddset (&cma___g_sigact_block_mask, SIGINT) == -1)
	cma__bugcheck ("init_signal:26");
    if (sigaddset (&cma___g_sigact_block_mask, SIGQUIT) == -1)
	cma__bugcheck ("init_signal:27");
    if (sigaddset (&cma___g_sigact_block_mask, SIGALRM) == -1)
	cma__bugcheck ("init_signal:28");
    if (sigaddset (&cma___g_sigact_block_mask, SIGTERM) == -1)
	cma__bugcheck ("init_signal:29");
    if (sigaddset (&cma___g_sigact_block_mask, SIGURG) == -1)
	cma__bugcheck ("init_signal:30");
    if (sigaddset (&cma___g_sigact_block_mask, SIGSTOP) == -1)
	cma__bugcheck ("init_signal:31");
    if (sigaddset (&cma___g_sigact_block_mask, SIGTSTP) == -1)
	cma__bugcheck ("init_signal:32");
    if (sigaddset (&cma___g_sigact_block_mask, SIGCONT) == -1)
	cma__bugcheck ("init_signal:33");
    if (sigaddset (&cma___g_sigact_block_mask, SIGCHLD) == -1)
	cma__bugcheck ("init_signal:34");
    if (sigaddset (&cma___g_sigact_block_mask, SIGTTIN) == -1)
	cma__bugcheck ("init_signal:35");
    if (sigaddset (&cma___g_sigact_block_mask, SIGTTOU) == -1)
	cma__bugcheck ("init_signal:36");
    if (sigaddset (&cma___g_sigact_block_mask, SIGIO) == -1)
	cma__bugcheck ("init_signal:37");
# if _CMA_VENDOR_ != _CMA__HP
    if (sigaddset (&cma___g_sigact_block_mask, SIGXCPU) == -1)
	cma__bugcheck ("init_signal:38a");
    if (sigaddset (&cma___g_sigact_block_mask, SIGXFSZ) == -1)
	cma__bugcheck ("init_signal:38b");
# endif
    if (sigaddset (&cma___g_sigact_block_mask, SIGVTALRM) == -1) 
	cma__bugcheck ("init_signal:39");
    if (sigaddset (&cma___g_sigact_block_mask, SIGPROF) == -1)
	cma__bugcheck ("init_signal:40");
# if _CMA_VENDOR_ != _CMA__HP
    if (sigaddset (&cma___g_sigact_block_mask, SIGWINCH) == -1) 
	cma__bugcheck ("init_signal:41");
# endif
# if SIGIOT != SIGLOST && SIGINFO != SIGLOST
    if (sigaddset (&cma___g_sigact_block_mask, SIGLOST) == -1)
	cma__bugcheck ("init_signal:42");
# endif
    if (sigaddset (&cma___g_sigact_block_mask, SIGUSR1) == -1)
	cma__bugcheck ("init_signal:43");
    if (sigaddset (&cma___g_sigact_block_mask, SIGUSR2) == -1)
	cma__bugcheck ("init_signal:44");
# ifdef SIGCLD
    if (sigaddset (&cma___g_sigact_block_mask, SIGCLD) == -1)
	cma__bugcheck ("init_signal:45");
# endif

    /*
     * Block all signals during signal delivery, except...
     *   SIGKILL, SIGSTOP, and SIGCONT may not be blocked.
     *   SIGTRAP is used to handle debugger breakpoints.
     *   SIGQUIT is useful for rescuing yourself!
     */
    if (sigfillset (&cma___g_sig_block_mask) == -1)
	cma__bugcheck ("init_signal:1");
    if (sigdelset (&cma___g_sig_block_mask, SIGKILL) == -1)
	cma__bugcheck ("init_signal:2");
    if (sigdelset (&cma___g_sig_block_mask, SIGSTOP) == -1)
	cma__bugcheck ("init_signal:3");
    if (sigdelset (&cma___g_sig_block_mask, SIGCONT) == -1)
	cma__bugcheck ("init_signal:4");
    if (sigdelset (&cma___g_sig_block_mask, SIGTRAP) == -1)
	cma__bugcheck ("init_signal:5");
# ifndef NDEBUG
    if (sigdelset (&cma___g_sig_block_mask, SIGQUIT) == -1)
	cma__bugcheck ("init_signal:6");
# endif


    /*
     *   Additional signals to exclude from the blocked mask.  
     *   Blocking these can cause delivery to eventually happen a 
     *	 long way away from the code that caused the signal to occur.
     */
    if (sigdelset (&cma___g_sig_block_mask, SIGILL) == -1)
	cma__bugcheck ("init_signal:7");
    if (sigdelset (&cma___g_sig_block_mask, SIGIOT) == -1)
	cma__bugcheck ("init_signal:8");
    if (sigdelset (&cma___g_sig_block_mask, SIGEMT) == -1)
	cma__bugcheck ("init_signal:9");
    if (sigdelset (&cma___g_sig_block_mask, SIGFPE) == -1)
	cma__bugcheck ("init_signal:10");
    if (sigdelset (&cma___g_sig_block_mask, SIGBUS) == -1)
	cma__bugcheck ("init_signal:11");
    if (sigdelset (&cma___g_sig_block_mask, SIGSEGV) == -1)
	cma__bugcheck ("init_signal:12");
    if (sigdelset (&cma___g_sig_block_mask, SIGSYS) == -1)
	cma__bugcheck ("init_signal:13");
    if (sigdelset (&cma___g_sig_block_mask, SIGPIPE) == -1)
	cma__bugcheck ("init_signal:14");

    {
    sigaction_t act;


    act.sa_mask = cma___g_sig_block_mask;
    act.sa_flags = 0;

    for (i = 1; i < NSIG; i++) {

# if !_CMA_THREAD_IS_VP_
	/*
	 * Timeslice signal 
	 *
	 * (This is handled outside the switch statement, because C doesn't 
	 *  allow multiple duplicate case expressions.)
	 */
	if (i == cma__c_timer_signal) {
#  if _CMA_HARDWARE_ == _CMA__HPPA
	    act.sa_handler = cma__hppa_timer_base;
#  else
#   if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
	    {
	    static struct sigstack vtalrm_sigstack;

	    vtalrm_sigstack.ss_sp = ibmr2_vtalrm_stack;
	    vtalrm_sigstack.ss_onstack  = 0;

	    sigstack(&vtalrm_sigstack, NULL);

	    act.sa_flags |= SA_ONSTACK;
	    }
#   endif
	    act.sa_handler = cma__periodic;
#  endif
	    if (sigaction (i, &act, cma_c_null_ptr) == -1)
		cma__bugcheck ("init_signal:46");

#  if _CMA_PLATFORM_ == _CMA__IBMR2_UNIX
	    act.sa_flags = 0;
#  endif
	    continue;	/* Skip over the switch statement for this iteration */
            }
# endif

	switch (i) {

	    /*
	     * Asynchronous Terminating Signals
	     */
            case SIGHUP:
            case SIGINT:
            case SIGALRM:
            case SIGTERM:
            case SIGVTALRM:
            case SIGPROF:
# if _CMA_VENDOR_ != _CMA__HP
            case SIGXCPU:
            case SIGXFSZ:
# endif
            case SIGUSR1:
            case SIGUSR2:
# if SIGIOT != SIGLOST && SIGINFO != SIGLOST
            case SIGLOST:
# endif
# if _CMA_VENDOR_ == _CMA__HP
	    case SIGPWR:
            case SIGWINDOW:
#  if _CMA_HARDWARE_ == _CMA__M68K
            case SIGDIL:	/* DIL signal */
#  endif
# endif
                {
		/*
		 * These handlers are now installed as needed by sigwait().
		 */
		break;
                }

	    /*
	     * Asynchronous Non-terminating Signals
	     */
            case SIGCONT:
            case SIGCHLD:
# if _CMA_VENDOR_ == _CMA__APOLLO
            case SIGCLD:
            case SIGAPOLLO:
# endif
# if _CMA_VENDOR_ != _CMA__HP
            case SIGWINCH:
# endif
                {
		/*
		 * These handlers are now installed as needed by sigwait().
		 */
		break;
                }

	    /*
	     * Synchronous Terminating Signals
	     */
            case SIGILL:
            case SIGIOT:
            case SIGEMT:
            case SIGFPE:
            case SIGBUS:
            case SIGSEGV:
            case SIGSYS:
            case SIGPIPE:
# if _CMA_PER_THD_SYNC_SIGS_
		{
		/*
		 * These are per-thread signals on OSF/1 systems; the
		 * handling will be initialized by cma__sig_thread_init(),
		 * not here.
		 */
		break;
		}
# endif
                {
		act.sa_handler = (cma___t_sig_hand_ptr)cma___sig_sync_term;
		if (sigaction (i,&act,(sigaction_t *)cma_c_null_ptr) == -1)
		    cma__bugcheck ("init_signal:49");
		break;
                }

	    /*
	     * Synchronous Non-Terminating Signals
	     */
            case SIGTTIN:
            case SIGTTOU:
                {
		act.sa_handler = cma___sig_sync_nonterm;
		if (sigaction (i,&act,(sigaction_t *)cma_c_null_ptr) == -1)
		    cma__bugcheck ("init_signal:50");
		break;
                }

	    /*
	     * I/O Signals.  CMA has special handlers for these.
	     */
            case SIGURG:
            case SIGIO:
                {
		/*
		 * These handlers are now installed as needed by sigwait().
		 */
# if _CMA_AUTO_SIGIO_HANDLING
		/*
		 * If this option is enabled, DECthreads will catch SIGIO and
		 * SIGURG and unblock any threads whose I/O is now ready.
		 */
		act.sa_handler = (cma___t_sig_hand_ptr)cma___sig_io;
		if (sigaction (i,&act,(sigaction_t *)cma_c_null_ptr) == -1)
		    cma__bugcheck ("init_signal:51");
# endif
		break;
                }

	    /*
	     * Signals which CMA does not catch.
	     */
            case SIGKILL:	/* Cannot be caught or ignored. */
            case SIGSTOP:	/* Cannot be caught or ignored. */
	    case SIGTRAP:	/* Don't mess with debugger bpts. */
            case SIGQUIT:	/* Preserve current behavior. */
            case SIGTSTP:
# if _CMA_VENDOR_ == _CMA__HP
            case _SIGRESERVE:	/* Don't catch for now */
# endif
# if _CMA_VENDOR_ == _CMA__IBM
            case 26:           /* Don't catch reserved signals */
            case 37:
            case 38:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
            case 50:
            case 51:
            case 52:
            case 53:
            case 54:
            case 55:
            case 56:
            case 57:
            case 58:
            case 59:
	    case SIGGRANT:
            case SIGRETRACT:
            case SIGSOUND:
            case SIGSAK:
            case SIGMIGRATE:
            case SIGPRE:
            case SIGMSG:
            case SIGPWR:
            case SIGDANGER:
# endif
                {
		/* 
		 * Do nothing for these signals.
		 */
		break;
                }

# if _CMA_OSIMPL_ == _CMA__OS_OSF
	    /*
	     * Special Case
	     *
	     * Work around for bug in OSF/1 kernel in V1.4
	     */
	    case SIGINFO:	/* Information request */
		{
		act.sa_handler = (cma___t_sig_hand_ptr)cma___sig_null_handler;
		if (sigaction (i,&act,(sigaction_t *)cma_c_null_ptr) == -1)
		    cma__bugcheck ("init_signal:51.5");
		break;
		}
# endif
	    /*
	     * All Other Signals
	     *
	     * All signals should be accounted for above:  this case should
	     * never occur.
	     */
            default:
		{
                cma__bugcheck ("init_signal:52");
		break;
		}
	    }
	}
    }
#endif
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__reinit_signal:  Do pre/post re-Initialize 
 *
 *  FORMAL PARAMETERS:
 *
 *	flag
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
cma__reinit_signal
#ifdef _CMA_PROTO_
	(
	cma_t_integer	    flag)
#else	/* no prototypes */
	(flag)
	cma_t_integer	    flag;
#endif	/* prototype */
    {
    cma_t_integer   i;


    if (flag == cma__c_reinit_postfork_clear) {
	cma__queue_init	(&cma___g_sigwait_queue);

	for (i = 1; i < NSIG; i++)
	    cma___g_sigwaiters[i] = 0;

	}
	
    }

/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__sig_deliver:  routine to perform signal "delivery" to sigwait'd
 *	threads and to perform calls to any sigaction handlers in current
 *	thread.
 *
 *	This routine is enabled as a signal handler for most of the signals.
 *
 *  FORMAL PARAMETERS:
 *
 *	sig  -- integer representing the number of the signal
 *	code -- integer representing code for addition hardware information
 *	scp  -- pointer to signal context buffer
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
 *	Boolean value (in case we were called as a function instead of a
 *	signal handler) which indicates that we handled the function (i.e.,
 *	that no additional handling is required).
 *
 *  SIDE EFFECTS:
 *
 *	Threads waiting for this signal are awakened.
 *
 *	NOTE:  This routine assumes that ALL signals are blocked by the system
 *		signal delivery mechanism (ie, which calls this routine).
 *
 *	NOTE:  This routine assumes that it is the only thread running.  We
 *		require mutual exclusion to delete from a queue, but locking a
 *		mutex in a signal handler is problematical.
 */
extern cma_t_boolean
cma__sig_deliver
#ifdef _CMA_PROTO_
	(
	int		    sig,
	int		    code,
	struct sigcontext   *scp)
#else	/* no prototypes */
	(sig, code, scp)
	int		    sig;
	int		    code;
	struct sigcontext   *scp;
#endif	/* prototype */
    {
    cma___t_sigwait_block   *wblk;
    cma__t_int_tcb	    *tcb = cma__get_self_tcb ();
    cma_t_boolean	    resend = cma_c_false;


#if _CMA_OS_ != _CMA__VMS
    if (tcb->sigaction_data[sig].sa_handler == SIG_IGN) {
	return cma_c_true;  /* Ignore this signal */
	}
    else if ((tcb->sigaction_data[sig].sa_handler != 0)
	     && (tcb->sigaction_data[sig].sa_handler != SIG_DFL)) 
	{
# if (_CMA_PLATFORM_ == _CMA__IBMR2_UNIX) || (_CMA_OSIMPL_ == _CMA__OS_OSF)
	cma___t_sighandler handler = 
	    (cma___t_sighandler)tcb->sigaction_data[sig].sa_handler;
	(*handler)(sig, code, scp);
# else
	(tcb->sigaction_data[sig].sa_handler)(sig, code, scp);
# endif
	return cma_c_true;
	}
#endif

/*
 * FIX-ME: make sure sigaction_data is cleared from the tcb upon detach!!
 */
    
#ifdef _CMA_SIGNAL_TRACE_
	{
	extern char	*sys_siglist[];
	char		output[cma__c_buffer_size];


	output[0] = '\0';
	cma__putstring (output, "Signal ");
	cma__putstring (output, sys_siglist[sig]);
	cma__putstring (output, " (#");
	cma__putint    (output, sig);
	cma__putstring (output, ") occurred in thread 0x");
	cma__puthex    (output, cma__get_self_tcb());

	if (code != 0) {
	    cma__putstring  (output, " with code 0x");
	    cma__puthex     (output, code);
	    }
	cma__putstring (output, " at PC 0x");
	cma__puthex    (output, scp->sc_pc);
	cma__putstring (output, ".");
	cma__puteol    (output);
	}
#endif

    wblk = (cma___t_sigwait_block *)cma__queue_next (&cma___g_sigwait_queue);
    while (wblk != (cma___t_sigwait_block*)&cma___g_sigwait_queue
	    && !(sigismember(wblk->set, sig) && (wblk->signal == -1))) {

	/*
	 * Check to see if this waiter is looking for this signal but is
	 * already processing another one.
	 */
	if (sigismember(wblk->set, sig) && (wblk->signal != -1))
	    resend = cma_c_true;

	wblk = (cma___t_sigwait_block *)cma__queue_next (&wblk->queue);
	}

    if (wblk != (cma___t_sigwait_block*)&cma___g_sigwait_queue) {
	wblk->signal = sig;
        cma__int_signal_int (wblk->cv);
	return cma_c_true;
	}
    else
	if (resend) {
#if _CMA_OS_ == _CMA__VMS
	    /*
	     * FIX-ME:  How do we handle this??
	     */
#else
	    /*
	     * We couldn't find a thread to wake up for this signal.  We 
	     * found a thread which could have handled this signal, but
	     * it was "in the process" of handling another signal so it 
	     * couldn't handle this one.  So, since we've now determined
	     * that there are no threads prepared to receive this signal
	     * modify the scp so that this signal will be blocked when 
	     * we return from processing it.  And then send this signal
	     * to us again so it will be pending the next time someone
	     * looks for it.  Oh, and return a "true" from this routine
	     * to say that we took care of this signal, so it doesn't
	     * terminate the process.
	     */
# if (_CMA_PLATFORM_ == _CMA__IBMR2_UNIX)
	    SIGADDSET(scp->sc_mask, sig);
# else
	    scp->sc_mask |= sigmask(sig);
# endif
	    if (kill (getpid(), sig) == -1) cma__abort();
#endif
	    return cma_c_true;
	    }
	else
	    return cma_c_false;
    }

#if _CMA_PER_THD_SYNC_SIGS_
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma__sig_thread_init:  Initialize OSF/1 per-thread signal handling
 *
 *  FORMAL PARAMETERS:
 *
 *	None
 *
 *  IMPLICIT INPUTS:
 *
 *	The signal state of the VP in which function executes
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
 *	Registers signal handlers for thread-synchronous signals.
 */
extern void
cma__sig_thread_init
#ifdef _CMA_PROTO_
	(void)
#else	/* no prototypes */
	()
#endif	/* prototype */
    {
    sigaction_t act;

    /*	 
     *  FIX-ME:
     *
     * The following code is a workaround to an OSF/1 mach threads
     * and Unix signals problem:
     *
     * A new flag SA_NOMASKSELF was added to sigaction() to indicate that
     * signal not to be masked off while processing.  
     *
     * The default BSD behaviour is to disallow the same signal from occuring 
     * to avoid recursion.  System V signal semantics does not block the signal but
     * do not recurse as the handler is ignored and default action is taken
     * if it occurs again.  This change causes the kernel to NOT block the
     * current signal from occuring if the user has requested so.  The end
     * result being that more than one thread in a task can receive and
     * process the same signal at the same time.  Previously, if a second
     * thread receives the same signal as another thread which is already
     * processing it, it wouldn't notice it and continue execution.  The signal
     * would eventually become unblocked, but potentially quite far away
     * from the code that caused the signal.
     *
     */	 

# if _CMA_HARDWARE_ == _CMA__ALPHA && _CMA_OSIMPL_ == _CMA__OS_OSF
    act.sa_flags = 8;	    /* FIX-ME: Special hard coded value for SA_NOMASKSELF */
# else
    act.sa_flags = 0;	    
# endif

    act.sa_mask = cma___g_sig_block_mask;
    act.sa_handler = (cma___t_sig_hand_ptr)cma___sig_sync_term;

    /*
     * Note: SIGTRAP is also per-thread, but we should leave the default and
     * let dbx do it's stuff.
     */
    if (sigaction (SIGILL, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    if (sigaction (SIGIOT, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    if (sigaction (SIGEMT, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    if (sigaction (SIGFPE, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    if (sigaction (SIGBUS, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    if (sigaction (SIGSEGV, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    if (sigaction (SIGSYS, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    if (sigaction (SIGPIPE, &act, (sigaction_t *)cma_c_null_ptr) == -1)
	cma__bugcheck ("sig_thread_init:1");
    }
#endif

#if _CMA_OS_ != _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___sig_async_term:  Signal handler routine for asynchronous,
 *	(process) terminating signals.
 *
 *	This routine is enabled as a signal handler.
 *
 *  FORMAL PARAMETERS:
 *
 *	sig  -- integer representing the number of the signal
 *	code -- integer representing code for addition hardware information
 *	scp  -- pointer to signal context buffer
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
 *	Any thread which is sigwait'ing for this signal is awakened.
 *	If no threads are waiting, the main thread is forced to exit.
 *
 *	NOTE:  This routine assumes that ALL signals are blocked by the system
 *		signal delivery mechanism (ie, which calls this routine).
 */
static void
cma___sig_async_term
#ifdef _CMA_PROTO_
	(
	int		    sig,
	int		    code,
	struct sigcontext   *scp)
#else	/* no prototypes */
	(sig, code, scp)
	int		    sig;
	int		    code;
	struct sigcontext   *scp;
#endif	/* prototype */
    {
    cma__t_int_tcb	*self;

    /*
     * Inform any threads which are sigwait'ing for this signal.
     *
     * If there is at least one waiting thread, simply return.
     */
    if (cma__sig_deliver (sig, code, scp)) return;

    /*
     * Make an attempt to lock the kernel.
     *
     * FIX-ME: This continues regardless of whether it acquires the kernel or
     * some other thread has it.  In an MP implementation, this is surely
     * wrong unless we stop other threads first!  But we can't use a regular
     * cma__enter_kernel(), since this is a signal handler, and the current
     * thread might already own the kernel (there's no way to tell).
     */
    self = cma__get_self_tcb ();
    (void)cma__tryenter_kernel ();

    if (self == &cma__g_def_tcb)	/* If this is main thread */
	cma__abort_process(sig);
    else {
# if _CMA_THREAD_IS_VP_
	/*
	 * Cause the initial thread to abort.  (Hopefully, this will take the
	 * whole process with it.)
	 */
	cma__vp_interrupt (
		cma__g_def_tcb.sched.processor->vp_id,
		(cma__t_vp_handler)cma__abort_process,
		(cma_t_address)sig,
		cma__vp_get_sequence (cma__g_def_tcb.sched.processor->vp_id));
# elif _CMA_HARDWARE_ == _CMA__HPPA
	/*
	 * Transfer to the initial thread (to ensure process termination) and 
	 * resend the signal to abort the process.
	 *
	 * FIX-ME:  Is it really necessary to transfer to the initial thread
	 *	    on HPPA, or could we just call abort process, immediately?
	 */
	cma__g_abort_signal = sig;
	cma__transfer_main_abort();
# elif _CMA_VENDOR_ == _CMA__APOLLO
	/*
	 * Transfer to the main thread (to ensure process termination) and 
	 * resend the signal to abort the process.
	 */
	cma__g_abort_signal = sig;
	cma__cause_force_dispatch (
		self,
		(cma_t_address)cma__force_dispatch,
		scp);
# else
	/*
	 * Abort the process by resending the signal (after some preparation).
	 */
	cma__abort_process (sig);
# endif
	}
    }
#endif

#if _CMA_OS_ != _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___sig_io: handle SIGURG and SIGIO signals
 *
 *  FORMAL PARAMETERS:
 *
 *	none
 *
 *  IMPLICIT INPUTS:
 *
 *	the io data base
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
cma___sig_io
#ifdef _CMA_PROTO_
	(
	int			sig,
	int			code,
	struct	sigcontext	*scp)
#else	/* no prototypes */
	(sig, code, scp)
	int			sig;
	int			code;
	struct	sigcontext	*scp;
#endif	/* prototype */
    {
    /*
     * Inform any threads which are sigwait'ing for this signal.
     */
    cma__sig_deliver (sig, code, scp);

# if !_CMA_THREAD_IS_VP_
    /*
     * An I/O is now pending.  If we can get into the kernel, check and see if 
     * there are any threads which are blocked on this I/O.  If we can't get 
     * into the kernel, defer this action for later.
     */

    if (!cma__tryenter_kernel ()) {
        static struct timeval polling_timeout = {0, 0};

	/*
	 * Since we are not interested in the result of this call, we 
	 * arbitrarily select read and 0.
	 */
	cma__io_available (cma__c_io_read, 0, &polling_timeout);

	cma__unset_kernel ();
	}
    else
	cma__kernel_unset (&cma__g_defer_avail);

# endif
    }
#endif

# if _CMA_OSIMPL_ == _CMA__OS_OSF
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___sig_null_handler:  null signal handler
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
cma___sig_null_handler
#ifdef _CMA_PROTO_
	(
	int			sig,
	int			code,
	struct	sigcontext	*scp)
#else	/* no prototypes */
	(sig, code, scp)
	int			sig;
	int			code;
	struct	sigcontext	*scp;
#endif	/* prototype */
    {
    }
#endif

#if _CMA_OS_ != _CMA__VMS
/*
 *  FUNCTIONAL DESCRIPTION:
 *
 *	cma___sig_sync_term:  Signal handler routine for synchronous,
 *	(process) terminating signals.
 *
 *	This routine is enabled as a signal handler.
 *
 *  FORMAL PARAMETERS:
 *
 *	sig  -- integer representing the number of the signal
 *	code -- integer representing code for addition hardware information
 *	scp  -- pointer to signal context buffer
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
 *	True if a thread was waiting for this signal.
 *
 *  SIDE EFFECTS:
 *
 *	An exception is raised in the calling thread.
 *	Any thread which is sigwait'ing for this signal is awakened.
 *
 *	NOTE:  This routine assumes that ALL signals are blocked by the system
 *		signal delivery mechanism (ie, which calls this routine).
 */
static void
cma___sig_sync_term
#ifdef _CMA_PROTO_
	(
	int		    sig,
	int		    code,
	struct sigcontext   *scp)
#else	/* no prototypes */
	(sig, code, scp)
	int		    sig;
	int		    code;
	struct sigcontext   *scp;
#endif	/* prototype */
    {
    cma_t_integer	exception_status;    

    /*
     * Inform any threads which are sigwait'ing for this signal.
     */
    if ((cma__sig_deliver (sig, code, scp))) return;

    switch (sig) {
	case SIGILL:
            {
	    switch (code) {
# ifdef _CMA_DECODE_SIGNALS_
		case ILL_RESAD_FAULT:
		    {
		    exception_status = exc_s_resaddr;
		    break;
		    }
		case ILL_PRIVIN_FAULT:
		    {
		    exception_status = exc_s_privinst;
		    break;
		    }
		case ILL_RESOP_FAULT:
		    {
		    exception_status = exc_s_resoper;
		    break;
		    }
# endif
		default:
		    {
		    exception_status = exc_s_illinstr;
		    break;
		    }
		}
	    break;
	    }
        case SIGTRAP:
            {
	    exception_status = exc_s_SIGTRAP;
	    break;
	    }
        case SIGIOT:
            {
	    exception_status = exc_s_SIGIOT;
	    break;
	    }
        case SIGEMT:
            {
	    exception_status = exc_s_SIGEMT;
	    break;
	    }
        case SIGFPE:
            {
	    switch (code) {
# ifdef _CMA_DECODE_SIGNALS_
		case FPE_INTOVF_TRAP:
		    {
		    exception_status = exc_s_intovf;
		    break;
		    }
		case FPE_INTDIV_TRAP:
		    {
		    exception_status = exc_s_intdiv;
		    break;
		    }
		case FPE_FLTOVF_TRAP:
		    {
		    exception_status = exc_s_fltovf;
		    break;
		    }
		case FPE_FLTDIV_TRAP:
		    {
		    exception_status = exc_s_fltdiv;
		    break;
		    }
		case FPE_FLTUND_TRAP:
		    {
		    exception_status = exc_s_fltund;
		    break;
		    }
		case FPE_DECOVF_TRAP:
		    {
		    exception_status = exc_s_decovf;
		    break;
		    }
		case FPE_SUBRNG_TRAP:
		    {
		    exception_status = exc_s_subrng;
		    break;
		    }
		case FPE_FLTOVF_FAULT:
		    {
		    exception_status = exc_s_fltovf;
		    break;
		    }
		case FPE_FLTDIV_FAULT:
		    {
		    exception_status = exc_s_fltdiv;
		    break;
		    }
		case FPE_FLTUND_FAULT:
		    {
		    exception_status = exc_s_fltund;
		    break;
		    }
# endif
		default:
		    {
		    exception_status = exc_s_aritherr;
		    break;
		    }
		}
	    break;
	    }
        case SIGBUS:
        case SIGSEGV:
            {
	    exception_status = exc_s_illaddr;
	    break;
	    }
        case SIGSYS:
            {
	    exception_status = exc_s_SIGSYS;
	    break;
	    }
        case SIGPIPE:
            {
	    exception_status = exc_s_SIGPIPE;
	    break;
	    }
#if _CMA_VENDOR_ != _CMA__HP
        case SIGXCPU:
            {
	    exception_status = exc_s_excpu;
	    break;
	    }
        case SIGXFSZ:
            {
	    exception_status = exc_s_exfilsiz;
	    break;
	    }
#endif
        default:
	    {
            cma__bugcheck ("sig_sync_term:1");
	    break;
	    }
	}

    /*
     * Because DECthreads uses _setjmp() & _longjmp() for performance,
     * delivering the exception (via _longjmp()) will not restore the
     * previous signal mask; this would leave signals permanently disabled.
     * To work around the problem without requiring the use of the much
     * slower setjmp() on every TRY block, we'll just manually restore the
     * signal mask from the context block before raising the exception.
     */
    if (sigprocmask (
	    SIG_SETMASK,
	    &scp->sc_mask,
	    (sigset_t *)cma_c_null_ptr)	== -1)
	cma__bugcheck ("sig_sync_term:2");

    cma__error (exception_status);
    }
#endif

#if _CMA_NO_POSIX_SIGNAL_
static int 
cma___sigaction
#ifdef _CMA_PROTO_
	(
	int		sig,
	sigaction_t	*act,
	sigaction_t	*oact)
#else	/* no prototypes */
	(sig, act, oact)
	int		sig;
	sigaction_t	*act;
	sigaction_t	*oact;
#endif
    {
    struct sigvec vec, ovec;
    int result;


    vec.sv_handler = act->sa_handler;
    vec.sv_mask    = act->sa_mask;
    vec.sv_flags   = act->sa_flags; 

    result = sigvec (sig, &vec, &ovec);

    if (oact != NULL) {
	oact->sa_handler = ovec.sv_handler;
	oact->sa_mask    = ovec.sv_mask;
	oact->sa_flags   = ovec.sv_flags;  
	}

    return(result);
    }

static int 
cma___sigprocmask
#ifdef _CMA_PROTO_
	(
	int		how,
	sigset_t	*set,
	sigset_t	*oset)
#else	/* no prototypes */
	(how, set, oset)
	int how;
	sigset_t *set;
	sigset_t *oset;
#endif
    {
    sigset_t mask;
    static sigset_t zero = 0;


    if (set == NULL)
	set = &zero;

    switch (how) {
	case  SIG_BLOCK:
	    if (oset != NULL)
		*oset = sigblock(*set);
	    else
		sigblock(*set);
	    break;
	case  SIG_UNBLOCK:
	    /*** Try to block all signals while setting, approx atomic */
	    mask = sigblock(-1);
	    mask &= ~*set;
	    if (oset != NULL)
		*oset = sigsetmask(mask);
	    else
		sigsetmask(mask);
	    break;
	case  SIG_SETMASK:
	    /*** Try to block all signals while setting, approx atomic */
	    mask = sigblock(-1);
	    mask |= *set;
	    if (oset != NULL)
		*oset = sigsetmask(mask);
	    else
		sigsetmask(mask);
	    break;
        default:
            cma__bugcheck ("sigprocmask:1");
    }
    return(0);
}
#endif

/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SIGNAL.C */
/*  *50   23-JUL-1993 07:37:58 BUTENHOF "Fix typecast problem for MIPS" */
/*  *49   21-JUN-1993 15:20:57 SCALES "Revamp sigwait for OSF/1" */
/*  *48    2-JUN-1993 19:45:59 SCALES "Change sigwait synch to use sigsetmask()" */
/*  *47    7-MAY-1993 18:51:39 SCALES "Fix sigwait() for osf/1" */
/*  *46   21-APR-1993 19:16:59 KEANE "Fix sync terminating signals" */
/*  *45   16-APR-1993 13:05:22 BUTENHOF "Pass TCB to cma__int[_timed]_wait" */
/*  *44    4-FEB-1993 15:46:53 KEANE "Change mask/flags behavior for OSF/1" */
/*  *43   20-NOV-1992 11:18:59 BUTENHOF "Fix vp_interrupt" */
/*  *42   13-OCT-1992 15:54:40 SCALES "Change sigaction structure variable declarations to use sigaction typedef" */
/*  *41    2-SEP-1992 16:26:16 BUTENHOF "Change io_available deferral mechanism" */
/*  *40    9-JUN-1992 14:59:01 KEANE "Only call remap_stack_holes if _CMA_PROTECT_MEMORY_ defined" */
/*  *39   21-MAY-1992 14:47:18 KEANE "Fix typecast problem" */
/*  *38   18-MAY-1992 16:49:43 SCALES "Cleanse wblk queue links" */
/*  *37   16-APR-1992 18:10:46 SCALES "Merge conditional signal handling from 26A1" */
/*  *36   19-MAR-1992 13:17:38 BUTENHOF "Fix sigaction() bugs" */
/*   26A1  7-JAN-1992 16:33:17 SCALES "Install signal handlers only when needed" */
/*  *35   27-NOV-1991 09:25:00 BUTENHOF "Remove more code for VMS" */
/*  *34   26-NOV-1991 16:15:38 BUTENHOF "Fix VAX C compilation error" */
/*  *33   25-NOV-1991 17:55:37 SCALES "fix returns in the default signal handler" */
/*  *32   14-OCT-1991 13:39:59 BUTENHOF "Unprotect guard pages on abort" */
/*  *31   24-SEP-1991 16:27:55 BUTENHOF "Merge CMA5 reverse IBM/HP/Apollo drops" */
/*  *30    3-SEP-1991 16:39:46 CURTIN "fixed qar 134, reset signals upon sync term" */
/*  *29   21-AUG-1991 16:44:38 CURTIN "Removed VMS include of stdio.h" */
/*  *28   11-JUL-1991 18:12:12 SCALES "Fix two-signal bug" */
/*  *27   11-JUN-1991 16:32:33 BUTENHOF "De-prototype some casts for pcc" */
/*  *26   10-JUN-1991 18:23:31 SCALES "Add sccs headers for Ultrix" */
/*  *25   10-JUN-1991 17:55:03 SCALES "Conditionalize inclusion of I/O stuff" */
/*  *24    6-JUN-1991 15:52:39 CURTIN "fixed index in for loop" */
/*  *23    6-JUN-1991 11:18:48 BUTENHOF "Integrate" */
/*  *22    5-JUN-1991 16:14:08 CURTIN "fork work" */
/*  *21    2-JUN-1991 19:36:48 BUTENHOF "Remove reference to cma__periodic for VP" */
/*  *20   31-MAY-1991 13:59:37 BUTENHOF "flush stdout in cma__abort_process" */
/*  *19   29-MAY-1991 17:45:48 BUTENHOF "Support OSF/1 per-thread signals" */
/*  *18   14-MAY-1991 13:50:10 BUTENHOF "use tryenter_kernel & unset_kernel functions" */
/*  *17   14-MAY-1991 13:46:08 BUTENHOF "Integrate changes lost in disk crash" */
/*  *16   14-MAY-1991 13:20:19 CURTIN "Replaced external calls with internal macros." */
/*  *15   10-MAY-1991 11:04:58 BUTENHOF "Use vp_interrupt instead of transfer_main_abort for VPs" */
/*  *14    7-MAY-1991 10:11:45 CURTIN "Replace call to cma_init w/ cma__int_init" */
/*  *13    2-MAY-1991 13:58:57 BUTENHOF "Tin BL3 support" */
/*  *12   12-APR-1991 23:36:46 BUTENHOF "Add OSF/1 signal" */
/*  *11    8-APR-1991 20:30:51 CURTIN "Made changes to cma__abort_process" */
/*  *10    3-APR-1991 15:59:58 BUTENHOF "Support generic OSF/1 platform" */
/*  *9    14-MAR-1991 17:50:43 SCALES "Make sigwait a full alert point" */
/*  *8    27-FEB-1991 16:40:01 BUTENHOF "Fix up for IBM RS/6000" */
/*  *7    20-FEB-1991 22:14:38 BUTENHOF "Try removing type warning" */
/*  *6     5-FEB-1991 00:59:51 BUTENHOF "Improve inlining of internal cv operations" */
/*  *5    24-JAN-1991 00:35:13 BUTENHOF "Fix exception name references" */
/*  *4    28-DEC-1990 01:00:37 BUTENHOF "Fix typo in previous edit" */
/*  *3    28-DEC-1990 00:04:40 BUTENHOF "Change exception names" */
/*  *2    17-DEC-1990 14:34:53 CURTIN "added cma_sigaction" */
/*  *1    12-DEC-1990 21:53:09 BUTENHOF "UNIX signal support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_SIGNAL.C */
