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
 *	@(#)$RCSfile: cma_defer.h,v $ $Revision: 4.2.7.2 $ (DEC) $Date: 1993/08/18 14:46:13 $
 */
/*
 * FACILITY:
 *
 *	CMA services
 *
 * ABSTRACT:
 *
 *	These routines allow actions which cannot be performed immediately
 *	to be deferred until a time when they can be performed immediately.
 *
 * AUTHOR:
 *
 *	Webb Scales
 *
 * CREATION DATE:
 *
 *	25 July 1990
 *
 * MODIFICATION HISTORY:
 *
 *	001	Dave Butenhof	03 August 1990
 *		Change the names of the semaphore deferral constants to fit
 *		new terms (wake_one and wake_all).
 *	002	Dave Butenhof	09 April 1991
 *		Use new type for "atomic bit" operation target
 *	003	Dave Butenhof	02 May 1991
 *		Remove defer_alert code.
 *	004	Paul Curtin	04 June 1991
 *		Add prototype for reinit routine.
 *	005	Dave Butenhof	21 November 1991
 *		Add prototype for cma__int_signal_int.
 *	006	Dave Butenhof	22 November 1991
 *		Fix 005.
 *	007	Dave Butenhof	26 August 1992
 *		Simplify "global deferral" -- there's now only a single type,
 *		for UNIX thread-synchronous I/O. Condition variable deferral
 *		is handled within each condition variable.
 *	008	Dave Butenhof	15 September 1992
 *		Fix typos on 007.
 *	009	Dave Butenhof	 5 October 1992
 *		Remove cma__init_defer().
 *	010	Webb Scales	 8 February 1993
 *		Add NT-post-wakeup, NT-clear-wakeup, and init-defer.
 *	011	Dave Butenhof	24 May 1993
 *		Add support for new manager thread CV undeferral (and for
 *		waking manager thread under kernel lock instead of using CV
 *		and mutex).
 */


#ifndef CMA_DEFER
#define CMA_DEFER

/*
 * INCLUDE FILES
 */

#include <cma_sched.h>

/*
 * CONSTANTS AND MACROS
 */

/*
 * On kernel thread versions, we don't maintain much of a scheduling
 * database, so there's no need to defer the wakeup -- just do it (for
 * example, on DEC OSF/1, the wakeup is really just sending a message to a
 * VP's "synch" port).
 *
 * On user-mode thread UNIX systems, the null thread blocks in select(),
 * which will terminate when a signal is delivered, with EINTR. The null
 * thread will resume, undefer, and yield... however, there is a window
 * where the signal might arrive before the select() blocks.  By writing
 * to a pipe included in the select(), we make the null thread wake up
 * from the select() immediately.
 *
 * On OpenVMS systems, the null thread is hibernating -- and will resume
 * hibernating when an AST is dismissed. Therefore, we issue a wakeup request
 * for the current process -- the null thread will then continue, undefer,
 * and yield.
 */
#if !_CMA_THREAD_IS_VP_
# if _CMA_OS_ == _CMA__VMS
#  define cma__nt_post_wakeup() (sys$wake (0, 0))
# else
#  define cma__nt_post_wakeup() (write (cma__g_nt_wakeup[1], "X", 1))
# endif
#endif

#if !_CMA_THREAD_IS_VP_
# if (_CMA_OS_ != _CMA__UNIX) || _CMA_THREAD_SYNCH_IO_
#  define cma__check_ioavail() 0
# else
#  define cma__check_ioavail() \
    (cma__kernel_set (&cma__g_defer_avail) ? 0 : \
	cma__io_available (cma__c_io_read, 0, &cma__g_polling_timeout))
# endif
#endif

#if _CMA_THREAD_IS_VP_
# define cma__undefer() (0)
#else
# define cma__undefer() ( \
    cma__assert_fail (cma__tac_isset (&cma__g_kernel_critical), \
	    "Undefer with kernel unlocked"), \
    cma__check_ioavail (), \
    (cma__kernel_set (&cma__g_mgr_wake) \
	? 0 : (cma__mgr_wake (), cma__yield_processor (), 1)))
#endif

/*
 * TYPEDEFS
 */


/*
 * GLOBAL DATA
 */

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_SYNC_IO_
extern cma__t_atomic_bit cma__g_defer_avail;
extern struct timeval cma__g_polling_timeout;
#endif

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_IS_VP_
/*
 * Pipe which prevents the null thread from blocking the process after a 
 * deferred action has taken place, closing a wake-up/waiting race.
 */
extern cma_t_integer	cma__g_nt_wakeup[2];
#endif

/*
 * INTERNAL INTERFACES
 */

#if (_CMA_OS_ == _CMA__UNIX) && !_CMA_THREAD_IS_VP_
void
cma__init_defer _CMA_PROTOTYPE_ ((void));

void
cma__nt_clear_wakeup _CMA_PROTOTYPE_ ((void));
#endif
#endif
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEFER.H */
/*  *15   28-MAY-1993 12:17:44 BUTENHOF "Fix undefer macro" */
/*  *14   27-MAY-1993 14:32:12 BUTENHOF "Signal mgr thd to undefer CVs" */
/*  *13   23-FEB-1993 16:29:31 SCALES "Fix two wake-up waiting races in the null thread" */
/*  *12    5-OCT-1992 15:07:44 BUTENHOF "Move defer bit init" */
/*  *11   15-SEP-1992 09:08:35 BUTENHOF "Fix typos" */
/*  *10    2-SEP-1992 16:24:52 BUTENHOF "Separate semaphores from kernel lock" */
/*  *9    22-NOV-1991 11:55:32 BUTENHOF "Fix prototype " */
/*  *8    21-NOV-1991 13:54:25 BUTENHOF "Add prototype for cma__int_signal_int()" */
/*  *7    10-JUN-1991 19:52:26 SCALES "Convert to stream format for ULTRIX build" */
/*  *6    10-JUN-1991 19:20:32 BUTENHOF "Fix the sccs headers" */
/*  *5    10-JUN-1991 18:21:22 SCALES "Add sccs headers for Ultrix" */
/*  *4     5-JUN-1991 16:16:55 CURTIN "fork work" */
/*  *3     2-MAY-1991 13:58:02 BUTENHOF "Remove defer_alert" */
/*  *2    12-APR-1991 23:35:23 BUTENHOF "Change type of internal locks" */
/*  *1    12-DEC-1990 21:44:21 BUTENHOF "Defer events while kernel locked" */
/*  VAX/DEC CMS REPLACEMENT HISTORY, Element CMA_DEFER.H */
