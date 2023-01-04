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
 * @(#)$RCSfile: cma_kernel.h,v $ $Revision: 4.2.2.5 $ (DEC) $Date: 1992/12/10 18:16:31 $
 */

/*
 * FACILITY:
 *
 *	CMA services
 *
 * ABSTRACT:
 *
 *	This module defines the interface for locking and unlocking the kernel
 *	scheduling database.
 *
 * AUTHORS:
 *
 *	Dave Butenhof
 *
 * CREATION DATE:
 *
 *	14 June 1990
 *
 * MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof & Webb Scales	03 August 1990
 *		Include cma_defer.h for deferral code
 *	002	Dave Butenhof	09 April 1991
 *		Use new type for "atomic bit" operation target
 *	003	Dave Butenhof	26 April 1991
 *		Optimize cma__enter_kernel() for uniprocessors.
 *	004	Dave Butenhof	08 May 1991
 *		Add new test macro to assert that kernel isn't already locked
 *		on uniprocessors (but evaporate on multiprocessors).
 *	005	Dave Butenhof	13 May 1991
 *		Conditionalize extern declarations for recorded kernel
 *		operations.
 *	006	Dave Butenhof	03 June 1991
 *		Uniprocessor cma__enter_kernel() call will bugcheck if kernel
 *		was already locked.
 *	007	Dave Butenhof	11 June 1991
 *		Add formatting function for kernel trace array.
 *	008	Webb Scales	13 February 1992
 *		Perform undeferrals on entering the kernel
 *	009	Brian Keane, Paul Curtin, Webb Scales	05 August 1992
 *		Provide for enter_kernel to yield when it can't get in, in
 *		the non _CMA_UNIPROCESSOR_ case.
 *	010	Dave Butenhof	19 August 1992
 *		Clean up -- limit enter_kernel by _CMA_SPINLOCKYIELD_ rather
 *		than a local symbol, to make it easier to control.
 *	011	Brian Keane	25 August 1992
 *		Modify enter_kernel to use swtch_pri(0), rather than cma__vp_yield().
 *	012	Dave Butenhof	 6 November 1992
 *		Add __FILE__ & __LINE__ to bugcheck.
 */

#ifndef CMA_KERNEL
#define CMA_KERNEL

/*
 * INCLUDE FILES
 */

#include <cma_defs.h>
#include <cma_defer.h>

/*
 * CONSTANTS AND MACROS
 */

#if _CMA_UNIPROCESSOR_
    /*
     * On a uniprocessor, DECthreads does all context switching of threads in
     * user mode within a single kernel context (process). It is impossible
     * for two threads to contend for the kernel lock simultaneously.
     * Therefore, when a thread attempts to lock the kernel, the lock must
     * always be clear.  Even if the thread is preempted by a timeslice
     * between reading the value of the lock and setting the lock (on
     * hardware without an atomic test-and-set), the lock must have been
     * restored to 0 before it continues at the set operation (or it could
     * not be running).
     *
     * Therefore, a loop is useless; the only way the lock can already be
     * set is if it's going to hang (because the *current thread* already has
     * it locked).  Instead, we'll declare a bugcheck if the lock is already
     * set.  This probably isn't as easy to diagnose as a hang (which leaves
     * the state intact), but we can't use a loop because the MIPS
     * uniprocessor cma__test_and_set() function uses cma__enter_kernel()
     * within a comma-list expression, and a loop isn't allowed.
     *
     * Additionally, define a macro that will test whether the kernel is
     * currently locked, and declare an assertion error if so: this is useful
     * on uniprocessors to detect programming errors that could result in
     * DECthreads hangs, but they are incorrect on multiprocessor
     * implementations where another thread may hold the kernel lock.
     */
# ifdef _CMA_TRACE_KERNEL_
#  define cma__enter_kernel() cma__enter_kern_record (__LINE__, __FILE__)
#  define cma__set_kernel() cma__set_kern_record (__LINE__, __FILE__)
#  define cma__tryenter_kernel() cma__tryenter_kern_record (__LINE__, __FILE__)
# else
#  define cma__enter_kernel() \
	(cma__kernel_set (&cma__g_kernel_critical) ? \
	    (cma__bugcheck ("enter_kernel: deadlock at %s:%d", \
	    __FILE__,__LINE__), 0) : cma__undefer ())
#  define cma__set_kernel() \
	(cma__kernel_set (&cma__g_kernel_critical) ? \
	    (cma__bugcheck ("set_kernel: deadlock at %s:%d", \
	    __FILE__,__LINE__), 0) : 0)
#  define cma__tryenter_kernel() (cma__kernel_set (&cma__g_kernel_critical))
# endif
# define cma__assert_not_kernel() \
    cma__assert_fail ( \
	    !cma__tac_isset (&cma__g_kernel_critical), \
	    "About to enter kernel when already in kernel.");
#else
# ifdef _CMA_TRACE_KERNEL_
#  define cma__enter_kernel() cma__enter_kern_record (__LINE__, __FILE__)
#  define cma__set_kernel() cma__set_kern_record (__LINE__, __FILE__)
#  define cma__tryenter_kernel() cma__tryenter_kern_record (__LINE__, __FILE__)
# else
#  define cma__enter_kernel() {int __limit__ = _CMA_SPINLOCKYIELD_; \
	while (cma__kernel_set (&cma__g_kernel_critical)) { \
	    if (__limit__ <= 0) {swtch_pri(0); \
		__limit__ = _CMA_SPINLOCKYIELD_;} \
	    else --__limit__; } \
	cma__undefer (); }
#  define cma__set_kernel() {int __limit__ = _CMA_SPINLOCKYIELD_; \
	while (cma__kernel_set (&cma__g_kernel_critical)) { \
	    if (__limit__ <= 0) {swtch_pri(0); \
		__limit__ = _CMA_SPINLOCKYIELD_;} \
	    else --__limit__; }}
#  define cma__tryenter_kernel() (cma__kernel_set (&cma__g_kernel_critical))
# endif
# define cma__assert_not_kernel()
#endif

#ifdef _CMA_TRACE_KERNEL_
# define cma__exit_kernel() cma__exit_kern_record (__LINE__, __FILE__)
# define cma__unset_kernel() cma__unset_kern_record (__LINE__, __FILE__)
#else
# define cma__exit_kernel() ( \
    cma__assert_fail ( \
	    cma__tac_isset (&cma__g_kernel_critical), \
	    "cma_exit_kernel:  kernel critical already unlocked"), \
    cma__undefer (), \
    cma__kernel_unset (&cma__g_kernel_critical))
# define cma__unset_kernel() (cma__kernel_unset (&cma__g_kernel_critical))
#endif

/*
 * TYPEDEFS
 */

/*
 * GLOBAL DATA
 */

extern cma__t_atomic_bit	cma__g_kernel_critical;	/* CMA in krnl */

/*
 * INTERNAL INTERFACES
 */

# ifdef _CMA_TRACE_KERNEL_
extern void
cma__enter_kern_record _CMA_PROTOTYPE_ ((
	cma_t_integer	line,
	char		*file));

extern void
cma__exit_kern_record _CMA_PROTOTYPE_ ((
	cma_t_integer	line,
	char		*file));

extern void
cma__format_karray _CMA_PROTOTYPE_ ((void));

extern cma_t_boolean
cma__tryenter_kern_record _CMA_PROTOTYPE_ ((
	cma_t_integer	line,
	char		*file));

extern void
cma__unset_kern_record _CMA_PROTOTYPE_ ((
	cma_t_integer	line,
	char		*file));
# endif

#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_KERNEL.H */
/*  *19    4-DEC-1992 12:41:43 BUTENHOF "Change OSF/1 yield" */
/*  *18    6-NOV-1992 12:50:36 BUTENHOF "Add info to deadlock bugcheck" */
/*  *17    2-SEP-1992 16:25:19 BUTENHOF "Fix undeferral" */
/*  *16   25-AUG-1992 20:45:46 KEANE "Modify enter_kernel to use swtch_pri(0), rather than cma__vp_yield()" */
/*  *15   21-AUG-1992 13:42:06 BUTENHOF "Limit spinlock by config symbol" */
/*  *14    5-AUG-1992 21:51:58 KEANE "Attempt to optimize enter_kernel" */
/*  *13   19-FEB-1992 13:50:29 SCALES "Undefer on enter-kernel" */
/*  *12   14-OCT-1991 13:39:13 BUTENHOF "Refine/fix use of config symbols" */
/*  *11   11-JUN-1991 17:17:10 BUTENHOF "Add & use functions to dump kernel/sem trace arrays" */
/*  *10   10-JUN-1991 19:54:01 SCALES "Convert to stream format for ULTRIX build" */
/*  *9    10-JUN-1991 19:21:03 BUTENHOF "Fix the sccs headers" */
/*  *8    10-JUN-1991 18:22:16 SCALES "Add sccs headers for Ultrix" */
/*  *7     3-JUN-1991 17:13:08 BUTENHOF "Uniproc enter_kernel() should bugcheck if was set" */
/*  *6    29-MAY-1991 17:14:50 BUTENHOF "Change definition of kernel record symbol" */
/*  *5    14-MAY-1991 13:43:29 BUTENHOF "Add kernel test macro" */
/*  *4    10-MAY-1991 16:18:52 BUTENHOF "Add kernel test macro" */
/*  *3     2-MAY-1991 13:58:28 BUTENHOF "Optimize enter_kernel for uniprocessor" */
/*  *2    12-APR-1991 23:36:02 BUTENHOF "Change type of internal locks" */
/*  *1    12-DEC-1990 21:46:52 BUTENHOF "Kernel lock support" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_KERNEL.H */
